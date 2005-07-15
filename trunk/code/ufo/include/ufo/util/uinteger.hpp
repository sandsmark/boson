/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/util/uinteger.hpp
    begin             : Mon Aug 13 2001
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

#ifndef UINTEGER_HPP
#define UINTEGER_HPP

#include "../uobject.hpp"

namespace ufo {

/** @short A class representing an integer, derived from UObject.
  * @ingroup core
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UInteger : public UObject {
	UFO_DECLARE_DYNAMIC_CLASS(UInteger)
private: // Private attributes
	int m_integer;
public:
	UInteger(const int integer = 0);

	bool operator ==(UInteger & integer);
	bool operator ==(const UInteger & integer);

	UInteger & operator =(int integer);
	operator int() {
		return m_integer;
	}

	int toInt() const;
	static int toInt(const std::string & stringA, bool * ok = NULL);

	/** checks if this object is the same object than obj */
	bool equals(const UInteger * integer) const;

	/** checks if this object is the same object than obj */
	bool equals(const UObject * obj) const;
	/** string representation */
	std::string toString() const;
	/** tries to make a new object with the same internal data.
	 */
	UObject * clone() const;

protected: // Protected methods
	std::ostream & paramString(std::ostream & os) const;
};

} // namespace ufo

#endif // UINTEGER_HPP
