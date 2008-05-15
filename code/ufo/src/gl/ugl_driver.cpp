/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/gl/ugl_driver.cpp
    begin             : Sat Nov 1 2003
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

#include "ufo/gl/ugl_driver.hpp"

#include "ufo/usharedlib.hpp"

#ifdef UFO_OS_WIN32
#define UFO_GL_LIB "OPENGL32.DLL"
#else
#define UFO_GL_LIB "libGL.so.1"
#endif

#include <cstdlib>

ufo::UGL_Driver * ufo::ugl_driver = NULL;//new ufo::UGL_Driver("");

ufo::UGL_Driver::UGL_Driver(const char * glPath) {
#if 0
	m_glLib = new USharedLib();

	// Unix style: load linked libs first
	bool valid = m_glLib->load("");
	if(!valid || m_glLib->symbol("glBegin") == NULL) {
		m_glLib->unload();

		valid = m_glLib->load(UFO_GL_LIB);
	}
	// FIXME: fail if GL lib is invalid

//#ifdef UFO_TARGET_OPENGL
#define UFO_GL_PROC(ret,func,params) \
{ \
	func = (ret (WINAPI *)params)(getProcAddress(#func)); \
	if (!func) { \
		std::cerr << "Couldn't load GL function: " << #func << "\n"; \
		exit(0); \
	} \
}
#include "ufo/gl/ugl_prototypes.hpp"
#undef UFO_GL_PROC
//#endif // UFO_TARGET_OPENGL
#else

#define UFO_GL_PROC(ret,func,params) \
{ \
	func = ::func; \
	if (!func) { \
		std::cerr << "Couldn't load GL function: " << #func << "\n"; \
		exit(0); \
	} \
}
#include "ufo/gl/ugl_prototypes.hpp"
#undef UFO_GL_PROC

#endif // 0
}


void *
ufo::UGL_Driver::getProcAddress(const char* proc) {
	return m_glLib->symbol(proc);
}
