/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/util/ucolor.hpp
    begin             : Wed May 16 2001
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

#ifndef UCOLOR_HPP
#define UCOLOR_HPP

#include "../uobject.hpp"
#include "../ufo_types.hpp"

namespace ufo {

/** @short An abstraction of a four float array (red, green, blue and alpha)
  *  describing a color.
  * @ingroup appearance
  * @ingroup drawing
  *
  * This class is not part of the @ref UObject inheritance structure.
  * Use instead @ref UColorObject if you need a color derived from UObject.
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UColor {
	//UFO_DECLARE_DYNAMIC_CLASS(UColor)
public: // Public static colors
	static const UColor red;
	static const UColor green;
	static const UColor blue;

	static const UColor darkRed;
	static const UColor darkGreen;
	static const UColor darkBlue;

	static const UColor white;
	static const UColor black;

	static const UColor gray;
	static const UColor darkGray;
	static const UColor lightGray;

	static const UColor cyan;
	static const UColor magenta;
	static const UColor yellow;

	static const UColor darkCyan;
	static const UColor darkMagenta;
	static const UColor darkYellow;

	//static const UColor * orange;
	//static const UColor * pink;

public:
	/** Creates a black color. */
	UColor();
	UColor(const UColor & col);
	/** Creates a color decoding the given string.
	  * You can either use a hex string if prefixed with '#' or
	  * a comma or blank space separated list of integer values.
	  * <p>
	  * Valid strings are (all white): "#ffffffff", "#0xffffffff",
	  * "255,255,255,255", "255, 255, 255, 255", "255 255 255 255"
	  * @param colorString A character string in one of the above
	  *  representations
	  */
	explicit UColor(const std::string & colorString);
	explicit UColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
	explicit UColor(const uint8_t rgba[], bool hasAlpha = false);
	explicit UColor(int r, int g, int b, int a = 255);
	explicit UColor(const int rgba[], bool hasAlpha = false);
	explicit UColor(float r, float g, float b, float a = 1.0f);
	explicit UColor(const float rgba[], bool hasAlpha = false);
	explicit UColor(double r, double g, double b, double a = 1.0);
	explicit UColor(const double rgba[], bool hasAlpha = false);

	/** The returned data is in system memory and should not be deleted.
	  * @return the color values as float array
	  */
	const float * getFloat() const;
	/** The returned data is in system memory and should not be deleted.
	  * @return the color values as float array
	  */
	float * getFloat();

	float getRed() const;
	float getGreen() const;
	float getBlue() const;
	float getAlpha() const;
	
	uint32_t getArgb() const;
	uint32_t getRgba() const;

	/** returns a darker version of the color. The Alpha value is untouched.
	  * Multiplies each rgb value with FACTOR.
	  */
	UColor darker() const;
	/** returns a darker version of the color. The Alpha value is untouched.
	  * Multiplies each rgb value with 1 / FACTOR.
	  */
	UColor brighter() const;

	/** @return True if all color components are exactly 0. */
	bool isBlack() const;
	/** @return True if all color components are exactly 1.0f. */
	bool isWhite() const;

public: // operators
	friend bool operator==(const UColor & col1, const UColor & col2);
	friend bool operator!=(const UColor & col1, const UColor & col2);

	friend std::ostream & operator<<(std::ostream & os, const UColor & col);

private:  // Private attributes
	/** A float array representing the color values red, green, blue and alpha
	  * (in exactly this order).
	  */
	float m_farr[4];

	/**  */
	static const float FACTOR;
	static const float MIN_VAL;
};


/** A color class derived from @ref UObject and @ref UColor.
  * It implements the missing @ref UObject methods and borrows all
  * important methods from @ref UColor .
  * @author Johannes Schmidt
  */
class UFO_EXPORT UColorObject : public UColor, public UObject {
	UFO_DECLARE_DYNAMIC_CLASS(UColorObject)
public:
	UColorObject();
	explicit UColorObject(const std::string & colorString);
	explicit UColorObject(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
	explicit UColorObject(const uint8_t rgba[], bool hasAlpha = false);
	explicit UColorObject(int r, int g, int b, int a = 255);
	explicit UColorObject(const int rgba[], bool hasAlpha = false);
	explicit UColorObject(float r, float g, float b, float a = 1.0f);
	explicit UColorObject(const float rgba[], bool hasAlpha = false);
	explicit UColorObject(double r, double g, double b, double a = 1.0);
	explicit UColorObject(const double rgba[], bool hasAlpha = false);

public: // overrides UObject
	virtual unsigned int hashCode() const;
	virtual bool equals(const UObject * obj);
	virtual bool equals(const UColorObject * obj);
	virtual UObject * clone() const;

protected:  // Protected methods
	virtual std::ostream & paramString(std::ostream & os) const;
};

//
// UColor
// inline implementation
//


inline const float *
UColor::getFloat() const {
	return m_farr;
}

inline float *
UColor::getFloat() {
	return m_farr;
}

inline float
UColor::getRed() const {
	return m_farr[0];
}

inline float
UColor::getGreen() const {
	return m_farr[1];
}

inline float
UColor::getBlue() const {
	return m_farr[2];
}

inline float
UColor::getAlpha() const {
	return m_farr[3];
}

inline bool
UColor::isBlack() const {
	return ((m_farr[0] == 0.f) && (m_farr[1] == 0.f) && (m_farr[2] == 0.f));
}

inline bool
UColor::isWhite() const {
	return ((m_farr[0] == 1.f) && (m_farr[1] == 1.f) && (m_farr[2] == 1.f));
}


inline bool operator==(const UColor & col1, const UColor & col2) {
	return ((col1.m_farr[0] == col2.m_farr[0]) &&
		(col1.m_farr[1] == col2.m_farr[1]) &&
		(col1.m_farr[2] == col2.m_farr[2]) &&
		(col1.m_farr[3] == col2.m_farr[3])
	);
}

inline bool operator!=(const UColor & col1, const UColor & col2) {
	return !(operator==(col1, col2));
}

inline std::ostream &
operator<<(std::ostream & os, const UColor & col) {
	return os << "UColor["
	<< col.m_farr[0] << ","
	<< col.m_farr[1] << ","
	<< col.m_farr[2] << ","
	<< col.m_farr[3]
	<< "]";
}

//
// UColorObject
// inline implementation
//

inline
UColorObject::UColorObject() {}

inline
UColorObject::UColorObject(const std::string & colorString)
	: UColor(colorString)
{}

inline
UColorObject::UColorObject(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	: UColor(r, g, b, a)
{}

inline
UColorObject::UColorObject(const uint8_t rgba[], bool hasAlpha)
	: UColor(rgba, hasAlpha)
{}

inline
UColorObject::UColorObject(int r, int g, int b, int a)
	: UColor(r, g, b, a)
{}

inline
UColorObject::UColorObject(const int rgba[], bool hasAlpha)
	: UColor(rgba, hasAlpha)
{}

inline
UColorObject::UColorObject(float r, float g, float b, float a)
	: UColor(r, g, b, a)
{}

inline
UColorObject::UColorObject(const float rgba[], bool hasAlpha)
	: UColor(rgba, hasAlpha)
{}

inline
UColorObject::UColorObject(double r, double g, double b, double a)
	: UColor(r, g, b, a)
{}

inline
UColorObject::UColorObject(const double rgba[], bool hasAlpha)
	: UColor(rgba, hasAlpha)
{}

} // namespace ufo

#endif // UCOLOR_HPP
