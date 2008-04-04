/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/usharedlib.hpp
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA *
 ***************************************************************************/

#ifndef USHAREDLIB_HPP
#define USHAREDLIB_HPP

#include "uobject.hpp"

namespace ufo {

/** @short Loads a dynamic link library at runtime.
  * @ingroup plugin
  *
  * @author Johannes Schmidt
  */
class UFO_EXPORT USharedLib : public UObject {
	UFO_DECLARE_DYNAMIC_CLASS(USharedLib)
public: // Public types
	/** Function binding mode */
	enum ldmode_t {
		bindNow,  /** Bind library and symbols now */
		bindLazy  /** Bind lazy (on first call) */
	};

public:
	USharedLib();
	USharedLib(const std::string & fileName, ldmode_t mode = bindLazy);
	~USharedLib();

	/** Returns the file name for the loaded shared lib. */
	std::string getFileName() const;

	/** Loads the given symbol from the loaded object file. */
	void * symbol(const std::string & sym);
	/** Loads the given symbol from the loaded object file. */
	void * operator[](const std::string & sym) {
		return symbol(sym);
	}

	/** Loads the given shared library. */
	bool load(const std::string & fileName, ldmode_t mode = bindLazy);
	/** Unloads the loaded library (if any). */
	void unload();

private:
	/** disable copy constructors */
	USharedLib(const USharedLib &);
	USharedLib & operator=(const USharedLib &);

private: // Private attributes
	std::string m_fileName;

	/** The dll handle */
	void * m_handle;
};

} // namespace ufo

#endif // USHAREDLIB_HPP
