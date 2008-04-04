/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/util/uproperties.hpp
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

#ifndef UPROPERTIES_HPP
#define UPROPERTIES_HPP

#include "../uobject.hpp"

#include <vector>
#include <map>

namespace ufo {

/** @short A set of properties which can be saved to or loaded from a stream.
  * @ingroup core
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UProperties : public UObject {
	UFO_DECLARE_DYNAMIC_CLASS(UProperties)
public: // Public types
	/** A Properties parser */
	class Reader : public virtual UObject {
	public:
		/** Returns a UProperties which contains the data of the stream.
		  * May contain nested properties.
		  */
		virtual void read(std::istream & streamA, UProperties * propA) = 0;
	};

	class Writer : public virtual UObject {
	public:
		/** Writes the data of the properties object in a special format.
		  */
		virtual void write(std::ostream & streamA, UProperties * propA) = 0;
	};

public:
	UProperties();
	UProperties(const std::string & fileNameA);

	// overrides UObject
	UObject * clone();

	void setReader(Reader * p);
	Reader * getReader();

	void setWriter(Writer * p);
	Writer * getWriter();

	void load(const std::string & fileNameA);
	void load(std::istream & streamA);

	void save(std::ostream & streamA);
	void save(const std::string & fileNameA);

	/** Returns value specified by keyA. */
	std::string get(const std::string & keyA);
	void put(const std::string & keyA, const std::string & valueA);

	/** Returns all available keys in this properties set. */
	std::vector<std::string> getKeys();

	/** clears all entries and child properties */
	void clear();

	//
	//nested properties functions
	//
	void putChild(const std::string & keyA, UProperties * propA);
	UProperties * getChild(const std::string & keyA);

	/** Returns all keys for children. */
	std::vector<std::string> getChildKeys();

public: // Protected types
	typedef std::map<std::string, std::string> propertiesMap;
	typedef std::map<std::string, UProperties*> childMap;

private: // Private attributes
	propertiesMap m_hash;
	childMap m_children;

	Reader * m_reader;
	Writer * m_writer;
};

class UINIReader : public UProperties::Reader {
public:
	void read(std::istream & streamA, UProperties * propA);

protected:
	void parse(char * buffer, unsigned int n);

	UProperties * m_localProp;
	UProperties * m_nestedProp;
};

class UINIWriter : public UProperties::Writer {
public:
	void write(std::ostream & streamA, UProperties * propA);
};



//
// inline implementation
//


inline std::string
UProperties::get(const std::string & keyA) {
	// avoid creation of empty key, value pair
	propertiesMap::const_iterator iter;
	iter = m_hash.find(keyA);

	if (iter != m_hash.end()) {
		return (*iter).second;
	} else {
		return "";
	}
}

inline void
UProperties::put(const std::string & keyA, const std::string & valueA) {
	m_hash[keyA] = valueA;
	//(*((m_hash.insert(value_type(keyA, data_type()))).first)).second = valueA;
}


inline std::vector<std::string>
UProperties::getKeys() {
	std::vector<std::string> ret;

	for(propertiesMap::const_iterator iter = m_hash.begin();
			iter != m_hash.end();
			++iter) {
		ret.push_back((*iter).first);
	}
	return ret;
}



} // namespace ufo

#endif // UPROPERTIES_HPP
