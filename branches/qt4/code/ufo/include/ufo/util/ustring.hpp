/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/util/ustring.hpp
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA *
 ***************************************************************************/

#ifndef USTRING_HPP
#define USTRING_HPP

#include "../uobject.hpp"
#include "../usharedptr.hpp"

#include <string>
#include <vector>

#include "../ufo_types.hpp" // UStringStream

namespace ufo {

/** @short A simple wrapper for std::string which is derived from UObject.
  * @ingroup core
  *
  * Uses implicit data sharing.
  * Implements some usefule functions like toString(Type & T), tokenize(),
  * unsigned int hash(const char*), etc.
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UString : public UObject {
	UFO_DECLARE_DYNAMIC_CLASS(UString)
private: // Private attributes
	USharedPtr<std::string> m_stringRef;
public:
	UString(char * str);
	UString(const char * str);
	UString(const std::string & str);

public: // operators
	inline bool operator ==(UString & str) const;
	inline bool operator ==(const UString & str) const;

	inline bool operator ==(const std::string & str) const;

	UString & operator =(char * str);
	UString & operator =(const char * str);
	UString & operator =(const std::string & str);

	operator std::string() const {
		return *m_stringRef;
	}

public: // Public methods
	/** Returns a const ref to the internal string.
	  * This string should not be stored as it may be invalid when the UString
	  * objet gets out of scope.
	  */
	inline const std::string & str() const;
	/** Returns a reference to the internal string.
	  * Be careful, this changes the internal string for all UString objects
	  * which are using the shared string.
	  * Use detach() if you want to modify only this UString object.
	  * This string should not be stored as it may be invalid when the UString
	  * objet gets out of scope.
	  * @see detach
	  */
	inline std::string & str();

	/** Returns a const char pointer to the shared string object. */
	inline const char * c_str() const;

	/** Creates a local copy of the shared string. */
	void detach();

	/** The string is cut at every occurence of the character delimiter.
	  * The resulting array of strings (without delimiter) is returned.
	  * @return A tokenized vector of strings.
	  */
	std::vector<std::string> tokenize(char delimiter) const;

	/** Returns a new string with all characters in upper case. */
	UString upperCase() const;
	/** Returns a new string with all characters in lower case. */
	UString lowerCase() const;

public: // Public static methods
	/** returns a string representation of the given type */
#ifndef _MSC_VER
	template <typename TYPE>
	static std::string toString(const TYPE & t);
#else
	template <typename TYPE>
	static std::string toString(const TYPE & t) {
		UOStringStream os;
		os << t;
		return os.str();
	}
#endif
	static int toInt(const std::string & str);
	static unsigned int toUnsignedInt(const std::string & str);
	static double toDouble(const std::string & str);

	/** Computes a hash value using horner's rule.
	  */
	static unsigned int hash(const char * cstr);

public: // Overrides UObject

	virtual unsigned int hashCode() const;

	/** checks if this object is the same object than obj */
	virtual bool equals(const UString * str) const;

	/** checks if this object is the same object than obj */
	virtual bool equals(const UObject * obj) const;
	/** Returns only the string */
	virtual std::string toString() const;

	virtual UObject * clone() const;

protected: // Overrides UObject
	virtual std::ostream & paramString(std::ostream & os) const;
};

//
// inline implementation
//


inline bool
UString::operator ==(UString & str) const {
	return !(m_stringRef->compare(str.str()));
}
inline bool
UString::operator ==(const UString & str) const {
	return !(m_stringRef->compare(str.str()));
}

inline bool
UString::operator ==(const std::string & str) const {
	return !(m_stringRef->compare(str));
}

inline const std::string &
UString::str() const {
	return *m_stringRef;
}

inline std::string &
UString::str() {
	return *m_stringRef;
}

inline const char *
UString::c_str() const {
	return m_stringRef->c_str();
}

#ifndef _MSC_VER
template <typename TYPE> std::string
UString::toString(const TYPE & t) {
	UOStringStream os;
	os << t;
	return os.str();
}
#endif


} // namespace ufo

#endif // USTRING_HPP
