/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/util/ufilearchive.cpp
    begin             : Sat Jan 26 2002
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

#include "ufo/util/ufilearchive.hpp"

#include "ufo/utoolkit.hpp"

#include "ufo/util/ustring.hpp"

#include <algorithm>

// we need some posix functionality to access directories
#ifdef UFO_OS_UNIX
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>
#endif

using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UFileArchive, UObject)

UFileArchive * UFileArchive::sm_instance = NULL;

//
// public static methods
//

bool
UFileArchive::exists(const std::string & fileName) {
	std::ifstream fileStream(fileName.c_str());

	if (fileStream.good()) {
		return true;
	}
	return false;
}

// this code is inspired PhysicsFS
// by Ryan C. Gordon -- http://icculus.org/physfs/
std::vector<std::string>
UFileArchive::readDir(const std::string & dirName) {
	std::vector<std::string> ret;
#ifdef UFO_OS_UNIX
	DIR * dir;
	struct dirent * ent;
	dir = opendir(dirName.c_str());
	if (dir == NULL) {
		return ret;
	}

	while (1) {
		ent = readdir(dir);
		if (ent == NULL) {
			// we're done.
			break;
		}
		if (strcmp(ent->d_name, ".") == 0 ||
				strcmp(ent->d_name, "..") == 0) {
			// omit . and ..
			continue;
		}
		// FIXME: should we omit sym links?
		ret.push_back(ent->d_name);
    } // while
	closedir(dir);
#elif defined(UFO_OS_WIN32) // !UFO_OS_UNIX
	HANDLE dir;
	WIN32_FIND_DATA ent;
	std::string searchPath(dirName);

	// if there's no '\\' at the end of the path, stick one in there.
	if (searchPath[searchPath.length() - 1] != '\\') {
		searchPath.append(1, '\\');
	} // if

	// Append the "*" to the end of the string
	searchPath.append(1, '*');

	dir = FindFirstFile(searchPath.c_str(), &ent);
	if (dir == INVALID_HANDLE_VALUE) {
		return ret;
	}

	do
	{
		if (strcmp(ent.cFileName, ".") == 0 ||
				strcmp(ent.cFileName, "..") == 0) {
			// omit . and ..
			continue;
		}
		ret.push_back(ent.cFileName);

	} while (FindNextFile(dir, &ent) != 0);

	FindClose(dir);
#endif
	return ret;
}

std::string
UFileArchive::dirName(const std::string & path) {
	std::string::size_type pos = path.size();
	// eliminate trailing slashes
	if (path[path.size() - 1] == '/') {
		pos = path.find_last_not_of('/');
	}
	// search directory slash
	pos = path.find_last_of('/', pos);
	// eliminate all slashes
	pos = path.find_last_not_of('/', pos);

	if (pos == std::string::npos) {
		return "/";
	}
	return path.substr(0, pos + 1);
}

bool
UFileArchive::isDirectory(const std::string & path) {
#if defined(UFO_OS_UNIX)
	struct stat statbuf;
	if (stat(path.c_str(), &statbuf) == -1) {
		// FIXME: process strerror(errno)?
		return false;
	}
	return S_ISDIR(statbuf.st_mode);
#elif defined(UFO_OS_WIN32) // !UFO_OS_UNIX
    return ((GetFileAttributes(path.c_str()) & FILE_ATTRIBUTE_DIRECTORY) != 0);
#else
	return false;
#endif
}

//
// c'tors
//

UFileArchive::UFileArchive() : m_archives() {}
UFileArchive::UFileArchive(const std::string & searchPath) : m_archives() {
	addSearchPath(searchPath);
}

//
// public methods
//

void
UFileArchive::addArchive(const std::string & arc) {
	if (arc.length()) {
		// removing trailing slash
		if (arc[arc.length() - 1] == '/') {
			m_archives.push_back(arc.substr(0, arc.length() - 1));
		} else {
			m_archives.push_back(arc);
		}
	}
}
bool
UFileArchive::removeArchive(const std::string & arc) {
	std::vector<std::string>::iterator pos =
		std::find(m_archives.begin(), m_archives.end(), arc);
	if (pos == m_archives.end() ) {
		return false;
	}

	m_archives.erase(pos);
	return true;
}
void
UFileArchive::removeAllArchives() {
	m_archives.clear();
}

void
UFileArchive::addSearchPath(const std::string & searchPath) {
	std::vector<std::string> archives = UString(searchPath).tokenize(':');

	for (std::vector<std::string>::const_iterator iter = archives.begin();
			iter != archives.end();
			++iter) {
		addArchive(*iter);
	}
}

std::string
UFileArchive::getSearchPath() const {
	std::string ret;
	std::vector<std::string>::const_iterator iter = m_archives.begin();
	if (iter != m_archives.end()) {
		ret.append(*iter);
		++iter;
	}

	for (; iter != m_archives.end(); ++iter) {
		ret.append(1, ':').append(*iter);
	}
	return ret;
}

std::vector<std::string>
UFileArchive::getSearchPathAsVector() const {
	return m_archives;
}

bool
UFileArchive::existsInArchive(const std::string & fileName) {
	return (getAbsolutePath(fileName) != "");
}


std::string
UFileArchive::getAbsolutePath(const std::string & fileNameA) const {
	for (std::vector<std::string>::const_iterator iter = m_archives.begin();
			iter != m_archives.end(); ++iter) {
		std::string newFileName(*iter);
		newFileName.append(1, '/').append(fileNameA);
		if (exists(newFileName)) {
			return newFileName;
		}
	}
	return "";
}

// FIXME
// we can't create this instance at the very beginning of the run time
// as we need the toolkit object
// Therefore we use createInstance
UFileArchive *
UFileArchive::createInstance() {
	UFileArchive * archive = new UFileArchive();

	// add . as default directory
	archive->addArchive("./");
	//archive->addSearchPath(UFO_DATADIR);
	archive->addSearchPath(UToolkit::getToolkit()->getProperty("data_dir"));

	return archive;
}
