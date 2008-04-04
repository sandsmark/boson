/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/util/uproperties.cpp
    begin             : Wed Feb 6 2002
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

#include "ufo/util/uproperties.hpp"

#include <iostream>
#include <fstream>
#include <string>

#include <sstream>

using namespace ufo;


UFO_IMPLEMENT_DEFAULT_DYNAMIC_CLASS(UProperties, UObject)

UProperties::UProperties()
	: m_reader(new UINIReader())
	, m_writer(new UINIWriter())
{}

UProperties::UProperties(const std::string & fileNameA)
	: m_reader(new UINIReader())
	, m_writer(new UINIWriter())
{
	load(fileNameA);
}


UObject *
UProperties::clone() {
	UProperties * ret = NULL;
	// use copy constructor
	*ret = *this;

	for (childMap::iterator iter = m_children.begin();
			iter != m_children.end();
			++iter) {
		(*iter).second = static_cast<UProperties*>(((*iter).second)->clone());
	}
	return ret;
}

void
UProperties::setReader(Reader * p) {
	if (p) {
		m_reader = p;
	}
}

UProperties::Reader *
UProperties::getReader() {
	return m_reader;
}


void
UProperties::setWriter(Writer * p) {
	if (p) {
		m_writer = p;
	}
}

UProperties::Writer *
UProperties::getWriter() {
	return m_writer;
}



void
UProperties::load(const std::string & fileNameA) {
	std::ifstream stream(fileNameA.c_str());
	load(stream);
}

void
UProperties::load(std::istream & streamA) {
	m_reader->read(streamA, this);
}


void
UProperties::save(const std::string & fileNameA) {
	std::ofstream stream(fileNameA.c_str());
	save(stream);
}

void
UProperties::save(std::ostream & streamA) {
	m_writer->write(streamA, this);
}



void
UProperties::clear() {
	m_hash.clear();
	m_children.clear();
}

void
UProperties::putChild(const std::string & keyA, UProperties * propA) {
	if (propA) {
		m_children[keyA] = propA;
	}
}

UProperties *
UProperties::getChild(const std::string & keyA) {
	childMap::const_iterator iter;
	iter = m_children.find(keyA);

	if (iter != m_children.end()) {
		return (*iter).second;
	} else {
		return NULL;
	}
}


std::vector<std::string>
UProperties::getChildKeys() {
	std::vector<std::string> ret;

	for(childMap::const_iterator iter = m_children.begin();
			iter != m_children.end();
			++iter) {
		ret.push_back((*iter).first);
	}
	return ret;
}



//
// class UINIReader
//

void
UINIReader::read(std::istream & streamA, UProperties * propA) {
	m_localProp = propA;
	m_nestedProp = NULL;

	// we are using a reasonable size
	char buffer[256];

	//std::cerr << "parsing stream for prop " << propA << std::hex << uint32_t(propA) << std::endl;

	while (streamA) {
		streamA.getline(buffer, 256);

		parse(buffer, streamA.gcount());
	}
}

void
UINIReader::parse(char * buffer, unsigned int n) {
	if (!n) {
		return;
	}

	char * pArr = buffer;
	int size = n;

	// remove trailing spaces and comments
	while (pArr[0] == ' ' && size) {
		pArr++;
		size--;
	}
	if (size == 0) { return; }

	if (pArr[0] == '#') {
		return;
	}

	// new element, nested property
	if (pArr[0] == '[') {
		int length = 0;
		pArr++;
		size--;
		while (pArr[length] != ']' && (size - length) >= 0) {
			length++;
		}

		if (size < 0) { return; }

		std::string name(pArr, length);
		m_nestedProp = m_localProp->getChild(name);
		if (m_nestedProp == NULL) {
			m_nestedProp = new UProperties();
			m_localProp->putChild(name, m_nestedProp);
		}
	} else {

		std::string test(pArr);

		std::string::size_type pos = test.find('=');
		std::string::size_type end = test.find('#');
		end = std::min(end, test.length());

		std::string::size_type endKey = pos;
		std::string::size_type beginValue = pos + 1;
		// remove white space
		while (pArr[endKey - 1] == ' ') {
			endKey--;
		}
		while (pArr[beginValue] == ' ') {
			beginValue++;
		}

		if (pos != std::string::npos && end > pos) {

			std::string key = test.substr(0, endKey);

			std::string value = test.substr(beginValue, end);

			if (m_nestedProp) {
				m_nestedProp->put(key, value);
			} else {
				m_localProp->put(key, value);
			}
		}
	}
}

void
UINIWriter::write(std::ostream & streamA, UProperties * propA) {
	std::vector<std::string> keys = propA->getKeys();

	for (std::vector<std::string>::const_iterator key_iter = keys.begin();
			key_iter != keys.end();
			key_iter++) {
		streamA << *key_iter << "=" << propA->get(*key_iter) << std::endl;
	}

	streamA << "\n";


	// and now save recursively the nested property objects
	std::vector<std::string> childKeys = propA->getChildKeys();

	for(std::vector<std::string>::const_iterator iter = childKeys.begin();
			iter != childKeys.end();
			iter++) {

		// write sub prop
		streamA << "[" << (*iter) << "]" << std::endl;

		UProperties * child = propA->getChild(*iter);

		child->save(streamA);
	}
}

// FIXME: this should really be in other files

#include "ufo/util/upoint.hpp"
#include "ufo/util/udimension.hpp"
#include "ufo/util/urectangle.hpp"
#include "ufo/util/uinsets.hpp"

UFO_IMPLEMENT_DEFAULT_DYNAMIC_CLASS(UPointObject, UPoint)
UFO_IMPLEMENT_DEFAULT_DYNAMIC_CLASS(UDimensionObject, UDimension)
UFO_IMPLEMENT_DEFAULT_DYNAMIC_CLASS(URectangleObject, URectangle)
UFO_IMPLEMENT_DEFAULT_DYNAMIC_CLASS(UInsetsObject, UInsets)

// FIXME: Use configure detection for limits header
/*
#ifdef HAVE_LIMITS
#include <limits>
static int ufo_max_int = std::numeric_limits<int>::max();
#else
*/
#include <climits>
static int ufo_max_int = INT_MAX;
//#endif

UPoint UPoint::invalid(ufo_max_int, ufo_max_int);
UDimension UDimension::invalid(ufo_max_int, ufo_max_int);
UDimension UDimension::maxDimension(ufo_max_int, ufo_max_int);
URectangle URectangle::invalid(0, 0, ufo_max_int, ufo_max_int);
