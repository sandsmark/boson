/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/util/ustring.cpp
    begin             : Sat Aug 11 2001
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

#include "ufo/util/ustring.hpp"

#include <cctype>

using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UString, UObject)

UString::UString(char * str) : m_stringRef(new std::string(str)) {}
UString::UString(const char * str) : m_stringRef(new std::string(str)) {}
UString::UString(const std::string & str) : m_stringRef(new std::string(str)) {}
/*
bool
UString::operator ==(UString & str) {
	return !(m_string->compare( str.getString() ));
}
bool
UString::operator ==(const UString & str) {
	return !(m_string->compare( str.getString() ));
}
*/

UString &
UString::operator =(char * str) {
	m_stringRef = new std::string(str);
	return *this;
}
UString &
UString::operator =(const char * str) {
	m_stringRef = new std::string(str);
	return *this;
}
UString &
UString::operator =(const std::string & str) {
	m_stringRef = new std::string(str);
	return *this;
}


void
UString::detach() {
	if (m_stringRef.refCount() > 1) {
	// FIXME
	// whee! does this work?
	m_stringRef = new std::string(*m_stringRef);
	}
}

std::vector<std::string>
UString::tokenize(char delimiter) const {
	std::vector<std::string> ret;

	std::string::size_type begin = 0;
	std::string::size_type end = (*m_stringRef).find(delimiter, begin);
	while (end != std::string::npos) {
		ret.push_back((*m_stringRef).substr(begin, end - begin));
		begin = end + 1;
		end = (*m_stringRef).find(delimiter, begin);
	}
	if (begin < (*m_stringRef).length()) {
		ret.push_back((*m_stringRef).substr(begin, (*m_stringRef).length()));
	}

	return ret;
}


UString
UString::upperCase() const {
	std::string ret(*m_stringRef);
	for (std::string::iterator iter = ret.begin();
			iter != ret.end();
			++iter) {
		(*iter) = /*std::*/toupper(*iter);
	}
	return ret;
}

UString
UString::lowerCase() const {
	std::string ret(*m_stringRef);
	for (std::string::iterator iter = ret.begin();
			iter != ret.end();
			++iter) {
		(*iter) = /*std::*/tolower(*iter);
	}
	return ret;
}

//
// static methods
//

unsigned int
UString::hash(const char * cstr) {
	unsigned long ret = 0;

	// #1
	// a standard approach
	for (; *cstr; ++cstr)
		ret = *cstr + 5 * ret;
	// #2
	// this is very good for short strings or strings with different endings
	//for (; *cstr; ++cstr)
	//ret = (ret << 5) - ret + *cstr;
	// #3
	// this seems to be the best, but it is a bit slower.
	//static unsigned int const shift = 6;
	//static unsigned int const mask = ~0U << (bitsizeof(unsigned int) - shift);
	//for (; *cstr; ++cstr) {
	//	ret =  (ret & mask) ^ (ret << shift) ^ *cstr;
	//}
	return (unsigned int)(ret);
}

int
UString::toInt(const std::string & str) {
	UIStringStream stream(str);
	int ret = 0;
	stream >> ret;
	return ret;
}

unsigned int
UString::toUnsignedInt(const std::string & str) {
	UIStringStream stream(str);
	unsigned int ret = 0;
	stream >> ret;
	return ret;
}

double
UString::toDouble(const std::string & str) {
	UIStringStream stream(str);
	double ret = 0;
	stream >> ret;
	return ret;
}

//
// Overrides UObject
//


unsigned int
UString::hashCode() const {
	return hash((*m_stringRef).c_str());
}

bool
UString::equals(const UObject * obj) const {
	if (const UString * stringObj = dynamic_cast<const UString *>(obj)) {
		return !(m_stringRef->compare(stringObj->str()));
	}
	return false;
}

bool
UString::equals(const UString * str) const {
	if (str) {
		return !(m_stringRef->compare(str->str()));
	}
	return false;
}

std::string
UString::toString() const {
	return *m_stringRef;
}

UObject *
UString::clone() const {
	return new UString(*m_stringRef);
}

//
// protected
//
std::ostream &
UString::paramString(std::ostream & os) const {
	return os << m_stringRef;
}
