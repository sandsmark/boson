/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/util/uinteger.cpp
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

#include "ufo/util/uinteger.hpp"

#include "ufo/util/ustring.hpp"


using namespace ufo;

UFO_IMPLEMENT_DEFAULT_DYNAMIC_CLASS(UInteger, UObject)

UInteger::UInteger(const int integer) : m_integer(integer) {}

bool
UInteger::operator ==(UInteger & integer) {
	return (m_integer == integer.m_integer);
}
bool
UInteger::operator ==(const UInteger & integer) {
	return (m_integer == integer.m_integer);
}

UInteger &
UInteger::operator =(int integer) {
	m_integer = integer;
	return *this;
}
int
UInteger::toInt() const {
	return m_integer;
}

int
UInteger::toInt(const std::string & stringA) {
	UIStringStream stream(stringA);

	int ret = 0;
	stream >> ret;

	return ret;
}

bool
UInteger::equals(const UInteger * integer) const {
	return (m_integer == integer->m_integer);
}

bool
UInteger::equals(const UObject * obj) const {
	if (const UInteger * intObj = dynamic_cast<const UInteger*>(obj)) {
		return (m_integer == intObj->m_integer);
	}
	return false;
}


std::string
UInteger::toString() const {
	return UString::toString(m_integer);
}


UObject *
UInteger::clone() const {
	return new UInteger(m_integer);
}

//
// protected methods
//

std::ostream &
UInteger::paramString(std::ostream & os) const {
	return os << m_integer;
}
