/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/font/ufontinfo.hpp
    begin             : Sat May 3 2003
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

#ifndef UFONTINFO_HPP
#define UFONTINFO_HPP

//#include "ufont.hpp"
#include "../uobject.hpp"
#include "../util/ustring.hpp"

namespace ufo {

/** @short A struct describing a font and its attributes.
  * @ingroup text
  * @ingroup drawing
  *
  * It is used to query for and load fonts and to get font attributes.
  * <p>
  * The default way to query for font is to use the UFont class.
  * <p>
  * If you want to use UFontInfo objects, create them separately
  * via the constructors.
  * You should not access the members of UFontInfo directly.
  *
  * @see UFont
  * @author Johannes Schmidt
  */
class UFontInfo /*: public UObject*/ {
public: // Public types
	/** A null font without any font information,i.e. any style */
	static const UFontInfo nullFont;

	/** Some font families. These font families are logical font descriptors which
	  * are mapped to valid font faces by the font renderer.
	  */
	enum Family {
		/** default font face */
		DefaultFamily = 0,
		/** decorative font face */
		Decorative,
		/** serif font face */
		Serif,
		/** sans-serif font face */
		SansSerif,
		//Swiss = SansSerif,
		/** A synonym for Serif */
		//Roman = Serif,
		/** handwriting font face */
		Script,
		/** fixed font face */
		MonoSpaced
		//Modern = Courier
	};
	/** The font weight. */
	enum Weight {
		AnyWeight = 0,
		Light = 250,
		Normal = 400,
		DemiBold = 600,
		Bold = 700,
		Black = 900
	};

	/** Some style hints. */
	enum Style {
		AnyStyle = 0,
		Plain = 1 << 0,
		Italic = 1 << 1,
		Underline = 1 << 2,
		StrikeOut = 1 << 3,
		AntiAliased = 1 << 4,
		NotAntiAliased = 1 << 5
	};

	/** The character encoding. FIXME: very incomplete */
	enum Encoding {
		/** default font encoding for the current system */
		Encoding_System = 0,
		/** The default encoding. */
		Encoding_Default,
		Encoding_ISO8859_1,       // West European (Latin1)
		Encoding_ISO8859_2,       // Central and East European (Latin2)
		Encoding_ISO8859_3,       // Esperanto (Latin3)
		Encoding_ISO8859_4,       // Baltic (old) (Latin4)
		Encoding_ISO8859_5,       // Cyrillic
		Encoding_ISO8859_6,       // Arabic
		Encoding_ISO8859_7,       // Greek
		Encoding_ISO8859_8,       // Hebrew
		Encoding_ISO8859_9,       // Turkish (Latin5)
		Encoding_ISO8859_10,      // Variation of Latin4 (Latin6)
		Encoding_ISO8859_11,      // Thai
		Encoding_ISO8859_12,      // Unused
		Encoding_ISO8859_13,      // Baltic (Latin7)
		Encoding_ISO8859_14,      // Latin8
		Encoding_ISO8859_15,      // Latin9 (a.k.a. Latin0, includes euro)
		Encoding_ISO8859_MAX,
		Encoding_UTF8,
		Encoding_Unicode
	};

public: // Public attributes
	Family family;
	std::string face;
	float pointSize;
	int weight;
	int style;
	Encoding encoding;

public:
	/** Creates the null font info struct. */
	UFontInfo();

	UFontInfo(const std::string & face, float pointSize, int weight,
		int style = Plain, Encoding encoding = Encoding_Default);
	UFontInfo(Family family, float pointSize, int weight,
		int style = Plain, Encoding encoding = Encoding_Default);
	UFontInfo(Family family, const std::string & face, float pointSize, int weight,
		int style, Encoding encoding);

	/** generates a hash code for this font info */
	unsigned int hashCode() const;

public: // Public operators
	friend std::ostream & operator<<(std::ostream & os, const UFontInfo & info);
};


/// Equality operators
UFO_EXPORT bool operator==(const UFontInfo & info1,const UFontInfo & info2);
UFO_EXPORT bool operator!=(const UFontInfo & info1,const UFontInfo & info2);

UFO_EXPORT bool operator<(const UFontInfo & info1,const UFontInfo & info2);

//
// inline implementation
//

inline
UFontInfo::UFontInfo()
	: family(DefaultFamily)
	, face("")
	, pointSize(0)
	, weight(AnyWeight)
	, style(AnyStyle)
	, encoding(Encoding_Default)
{}

inline
UFontInfo::UFontInfo(const std::string & face, float pointSize, int weight,
		int style, Encoding encoding)
	: family(DefaultFamily)
	, face(face)
	, pointSize(pointSize)
	, weight(weight)
	, style(style)
	, encoding(encoding)
{}

inline
UFontInfo::UFontInfo(Family family, float pointSize, int weight,
		int style, Encoding encoding)
	: family(family)
	, face("")
	, pointSize(pointSize)
	, weight(weight)
	, style(style)
	, encoding(encoding)
{}

inline
UFontInfo::UFontInfo(Family family, const std::string & face, float pointSize, int weight,
		int style, Encoding encoding)
	: family(family)
	, face(face)
	, pointSize(pointSize)
	, weight(weight)
	, style(style)
	, encoding(encoding)
{}

inline unsigned int
UFontInfo::hashCode() const {
	// FIXME !
	// quick and dirty, probably even stupid
	unsigned int ret = UString::hash(face.c_str());
	ret = ret << encoding;
	ret |= int(pointSize * 10);
	ret += weight;
	ret += style;
	return ret;
}

inline std::ostream &
operator<<(std::ostream & os, const UFontInfo & info) {
	return os << "UFontInfo["
	<< "family " << info.family
	<< "; face " << info.face
	<< "; point size " << info.pointSize
	<< "; weight " << info.weight
	<< "; style " << info.style
	<< "; encoding " << info.encoding
	<< "]";
}

inline bool operator==(const UFontInfo & info1,const UFontInfo & info2) {
	return (
		(info1.face == info2.face) &&
		(info1.family == info2.family) &&
		(info1.pointSize == info2.pointSize) &&
		(info1.weight == info2.weight) &&
		(info1.style == info2.style) &&
		(info1.encoding == info2.encoding)
	);
}

inline bool operator!=(const UFontInfo & info1,const UFontInfo & info2) {
	return (
		(info1.face != info2.face) ||
		(info1.family != info2.family) ||
		(info1.pointSize != info2.pointSize) ||
		(info1.weight != info2.weight) ||
		(info1.style != info2.style) ||
		(info1.encoding != info2.encoding)
	);
}

inline bool operator<(const UFontInfo & info1,const UFontInfo & info2) {
	return info1.hashCode() < info2.hashCode();
}

} // namespace ufo

#endif // UFONTINFO_HPP
