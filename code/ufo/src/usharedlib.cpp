/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/usharedlib.cpp
    begin             : Tue Feb 11 2003
    $Id$
 ***************************************************************************/

/***************************************************************************
 *  This library is free software; you can redistribute it and/or          *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This library is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this library; if not, write to the Free Software     *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#include "ufo/usharedlib.hpp"

// great parts of this code are taken from libSDL (http://www.libsdl.org)
// thanks to Sam Lantinga

#if defined(HAVE_DLFCN_H)
# include <dlfcn.h>
# include <errno.h>
#elif defined(UFO_OS_WIN32)
# include <windows.h>
#elif defined(UFO_OS_BEOS)
# include <be/kernel/image.h>
#elif defined(UFO_OS_MAC)
# include <string.h>
# include <Strings.h>
# include <CodeFragments.h>
# include <Errors.h>
#else
/*#error Unsupported dynamic link environment*/
#endif /* system type */

namespace ufo {

UFO_IMPLEMENT_DYNAMIC_CLASS(USharedLib, UObject)

USharedLib::USharedLib() : m_handle(NULL) {}

USharedLib::USharedLib(const std::string & fileName, ldmode_t mode)
	: m_fileName(fileName)
	, m_handle(NULL)
{
	load(fileName, mode);
}

USharedLib::~USharedLib() {
	unload();
}


std::string
USharedLib::getFileName() const {
	return m_fileName;
}


void *
USharedLib::symbol(const std::string & symbol) {
	void * object = NULL;
	std::string loaderror;

#if defined(HAVE_DLFCN_H)
/* * */
	object = dlsym(m_handle, symbol.c_str());
	if (object == NULL) {
		loaderror = dlerror();
	}
#elif defined(WIN32)
/* * */
	char errbuf[512];

	object = (void *) GetProcAddress((HMODULE) m_handle, symbol.c_str());
	if (object == NULL) {
		FormatMessage((FORMAT_MESSAGE_IGNORE_INSERTS |
					FORMAT_MESSAGE_FROM_SYSTEM),
				NULL, GetLastError(),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				errbuf, 512, NULL);
		loaderror = errbuf;
	}
#elif defined(__BEOS__)
/* * */
	image_id library_id = (image_id) m_handle;
	if (get_image_symbol(library_id,
		symbol.c_str(), B_SYMBOL_TYPE_TEXT, &object) != B_NO_ERROR ) {
		loaderror = "Symbol not found";
	}
#elif defined(macintosh)
/* * */
	CFragSymbolClass symclass;
	CFragConnectionID library_id = (CFragConnectionID) m_handle;

	if (FindSymbol(library_id, C2PStr(symbol.c_str()),
			(char **)&object, &symclass) != noErr) {
		loaderror = "Symbol not found";
	}
#endif /* system type */

	if (object == NULL) {
		//uWarning() << "USharedLib: Failed loading "
		//<< symbol << "  " << loaderror << "\n";
	}
	return (object);
}


bool
USharedLib::load(const std::string & fileName, ldmode_t mode) {
	std::string loaderror;
#ifdef HAVE_DLFCN_H
	int flags = 0;
	switch (mode) {
		case bindLazy:
			flags = RTLD_LAZY;
		break;
		case bindNow:
			flags = RTLD_NOW;
		break;
	}
	flags |= RTLD_GLOBAL;

	m_handle = (void *) dlopen(fileName.c_str(), flags);
	if(!m_handle) {
		//throw SystemError(errno, "Could not load shared library", P_SOURCEINFO);
	}
#elif defined(WIN32)
/* * */
	char errbuf[512];

	m_handle = (void *) LoadLibrary(fileName.c_str());

	/* Generate an error message if all loads failed */
	if (m_handle == NULL) {
		FormatMessage((FORMAT_MESSAGE_IGNORE_INSERTS |
					FORMAT_MESSAGE_FROM_SYSTEM),
				NULL, GetLastError(),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				errbuf, sizeof(errbuf)/*SDL_TABLESIZE(errbuf)*/, NULL);
		loaderror = errbuf;
	}
#elif defined(__BEOS__)
/* * */
	image_id library_id;

	library_id = load_add_on(fileName);
	if (library_id == B_ERROR) {
		loaderror = "BeOS error";
	} else {
		m_handle = (void *) (library_id);
	}
#elif defined(macintosh)
/* * */
	CFragConnectionID library_id;
	Ptr mainAddr;
	Str255 errName;
	OSErr error;

	error = GetSharedLibrary(fileName.c_str(), kCompiledCFragArch,
			kLoadCFrag, &library_id, &mainAddr, errName);
	switch (error) {
		case noErr:
			break;
		case cfragNoLibraryErr:
			loaderror = "Library not found";
			break;
		case cfragUnresolvedErr:
			loaderror = "Unabled to resolve symbols";
			break;
		case cfragNoPrivateMemErr:
		case cfragNoClientMemErr:
			loaderror = "Out of memory";
			break;
		default:
			loaderror = "Unknown Code Fragment Manager error";
			break;
	}
	if (loaderror.empty()) {
		m_handle = (void *) (library_id);
	}
#endif /* system type */

	if (m_handle == NULL) {
		// FIXME: if loading fails, throw warning? exception?
		//uWarning() << "Failed loading " << fileName << " : " << loaderror << "\n";
		return false;
	}
	m_fileName = fileName;
	return true;
}

void
USharedLib::unload() {
	#if defined(__BEOS__)
	image_id library_id;
#endif

	if (m_handle == NULL) {
		return;
	}

#if defined(HAVE_DLFCN_H)
/* * */
	dlclose(m_handle);
#elif defined(WIN32)
/* * */
	FreeLibrary((HMODULE) m_handle);
#elif defined(__BEOS__)
/* * */
	library_id = (image_id) m_handle;
	unload_add_on(library_id);
#elif defined(macintosh)
/* * */
	CFragConnectionID library_id = (CFragConnectionID) m_handle;
	CloseConnection(library_id);
#endif /* system type */
}

} // namespace ufo
