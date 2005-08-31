/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/util/ufilearchive.hpp
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

#ifndef UFILEARCHIVE_HPP
#define UFILEARCHIVE_HPP

#include "../uobject.hpp"

#include <vector>
#include <fstream>

namespace ufo {

/** @short A simple abstraction to read files.
  * @ingroup core
  *
  * This class provides two static methods to check the content of a directory
  * and to check whether a file exists.
  * Furthermore, there is one default instance for file archives to load
  * data from.
  * @author Johannes Schmidt
  */

class UFO_EXPORT UFileArchive : public UObject  {
	UFO_DECLARE_DYNAMIC_CLASS(UFileArchive)
public: // Public static methods
	/** Checks if a file exists
	  * @param fileNameA file to check
	  * @return true if the file exists
	  */
	static bool exists(const std::string & fileName);
	/** Reads the contents of the given directory.
	  * Returns an empty vector when the given directory is a file.
	  */
	static std::vector<std::string> readDir(const std::string & dirName);

	static UFileArchive * getDefault();
	static void setDefault(UFileArchive * archive);

public: // c'tors
	/** Creates an empty file archive. */
	UFileArchive();
	/** Creates a file archive with the given colon separated search path. */
	UFileArchive(const std::string & searchPath);

public: // Public methods
	/** adds some generic search directories */
	//static void init();

	/** adds a further directory to the search path. */
	void addArchive(const std::string & arc);
	/** removes the given directory from the search path */
	bool removeArchive(const std::string & arc);
	/** cleans up the search path */
	void removeAllArchives();

	/** Adds a colon separated list to the search path. */
	void addSearchPath(const std::string & searchPath);
	/** Returns a colon separated list of the search path.
	  * Can be tokenized via UString tokenizer.
	  * @see UString
	  */
	std::string getSearchPath() const;
	/** Returns the search path as vector.
	  * @see UString
	  */
	std::vector<std::string> getSearchPathAsVector() const;

	/** Checks if a file exists in the archive
	  * @param fileNameA file to check
	  * @return true if the file exists
	  */
	bool existsInArchive(const std::string & fileNameA);

	/** Returns the full path name of the given file name
	  * if found in the archive.
	  * Otherwise returns an empty string.
	  */
	std::string getAbsolutePath(const std::string & fileNameA) const;

	/** Creates a file stream by teh given attributes.
	  * This is done by iterating through the search path.
	  * This method does not return NULL, but may return an
	  * file stream which fails on operations
	  * (i.e. ifstream::fail returns true).
	  *
	  * Please note: The created file stream has to be closed and deleted
	  * by destroyFileStream!
	  * @see destroyFileStream
	  */
	std::ifstream * createFileStream(const std::string & fileNameA,
		std::ios_base::openmode modeA = std::ios_base::in);

	/** Closes and destroys a file stream object created by createFileStream.
	  */
	void destroyFileStream(std::ifstream * fstream);

private: // Private static methods
	static UFileArchive * createInstance();

private: // Private static attributes
	static UFileArchive * sm_instance;

private: // Private attributes
	std::vector<std::string> m_archives;
};

inline UFileArchive *
UFileArchive::getDefault() {
	if (!sm_instance) {
		sm_instance = createInstance();
	}
	return sm_instance;
}

inline void
UFileArchive::setDefault(UFileArchive * instance) {
	if (instance) {
		sm_instance = instance;
	}
}

} // namespace ufo

#endif // UFILEARCHIVE_HPP
