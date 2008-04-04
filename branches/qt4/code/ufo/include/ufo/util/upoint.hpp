/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/util/upoint.hpp
    begin             : Sun May 13 2001
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

#ifndef UPOINT_HPP
#define UPOINT_HPP

#include "../uobject.hpp"

namespace ufo {

/** @short An abstraction of a two dimensional point (x and y).
  * @ingroup appearance
  *
  * This class is not part of the @ref UObject inheritance structure.
  * Use instead @ref UPointObject if you need a point derived from UObject.
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UPoint {
public:
	inline UPoint();
	inline UPoint(int x, int y);

	inline int getX() const;
	inline void setX(int x);

	inline int getY() const;
	inline void setY(int y);

	/** for convenience */
	inline void setLocation(const UPoint & p);
	/** Returns a copy. For convenience */
	inline UPoint getLocation() const;

	/** This method is for convenience. It adds p to this point.*/
	inline void translate(const UPoint & p);

	/** @return True if the width and height are equal to @p invalid
	  * @see invalid
	  */
	inline bool isInvalid() const;
	/** @return True if both, x and y are exactly 0. */
	inline bool isNull() const;

	/** Adds point p to this point
	  * @return Reference to this point.
	  */
	inline UPoint & operator+=(const UPoint & p);
	/** Subtracts point p from this point
	  * @return Reference to this point.
	  */
	inline UPoint & operator-=(const UPoint & p);
	/** Multiplies c to both x and y coordinate of this point.
	  * @return Reference to this point.
	  */
	inline UPoint & operator*=(int c);
	/** Multiplies c to both x and y coordinate of this point.
	  * Truncates the values to integers.
	  * @return Reference to this point.
	  */
	inline UPoint & operator*=(double c);
	/** Divides both x and y by c.
	  * Truncates the values to integers.
	  * @return Reference to this point.
	  */
	inline UPoint & operator/=(int c);
	/** Divides both x and y by c.
	  * Truncates the values to integers.
	  * @return Reference to this point.
	  */
	inline UPoint & operator/=(double c);

	inline friend std::ostream & operator<<(std::ostream & os, const UPoint & o);

public:  // Public attributes
	int x;
	int y;
public: // Public static attributes
	static UPoint invalid;
};

//
// public operators
//
inline UPoint operator+(const UPoint & p1, const UPoint & p2);
inline UPoint operator-(const UPoint & p1, const UPoint & p2);
// equivalent to UPoint(0, 0) - p.
inline UPoint operator-(const UPoint & p);
inline UPoint operator*(const UPoint & p, int c);
inline UPoint operator*(int c, const UPoint & p);
inline UPoint operator*(const UPoint & p, double c);
inline UPoint operator*(double c, const UPoint & p);
inline UPoint operator/(const UPoint & p, int c);
inline UPoint operator/(const UPoint & p, double c);

/// Equality
inline bool operator==(const UPoint & p1,const UPoint & p2);
inline bool operator!=(const UPoint & p1,const UPoint & p2);

/** wrapper class for UDimension which is derived from UObject.
  *@author Johannes Schmidt
  */
class UFO_EXPORT UPointObject : public UPoint, public UObject {
	UFO_DECLARE_DYNAMIC_CLASS(UPointObject)
public:
	inline UPointObject();
	inline UPointObject(const UPoint & p);
	inline UPointObject(int w, int h);

	//
	// overrides UObject
	//
	virtual unsigned int hashCode() const;
	virtual bool equals(const UObject * obj);
	virtual bool equals(const UPoint * obj);
	virtual UObject * clone() const;

protected:  // Protected methods
	virtual std::ostream & paramString(std::ostream & os) const;
};



//
// UPoint
// inline implementation
//


inline UPoint::UPoint() : x(0), y(0) {}

inline UPoint::UPoint(int x, int y) : x(x), y(y) {}

inline int
UPoint::getX() const {
	return x;
}
inline void
UPoint::setX(int x) {
	this->x = x;
}
inline int
UPoint::getY() const {
	return y;
}
inline void
UPoint::setY(int y) {
	this->y = y;
}

inline UPoint
UPoint::getLocation() const {
	return UPoint(x, y);
}

inline void
UPoint::setLocation(const UPoint & p) {
	x = p.x;
	y = p.y;
}


inline void
UPoint::translate(const UPoint & p) {
	x += p.x;
	y += p.y;
}


inline bool
UPoint::isInvalid() const {
	return (*this == UPoint::invalid);
}

inline bool
UPoint::isNull() const {
	return !(x || y);
}

inline UPoint &
UPoint::operator+=(const UPoint & p) {
	x += p.x;
	y += p.y;
	return *this;
}

inline UPoint &
UPoint::operator-=(const UPoint & p) {
	x -= p.x;
	y -= p.y;
	return *this;
}

inline UPoint &
UPoint::operator*=(int c) {
	x *= c;
	y *= c;
	return *this;
}

inline UPoint &
UPoint::operator*=(double c) {
	// FIXME: should we round or truncate?
	x = int(x * c);
	y = int(y * c);
	return *this;
}

inline UPoint &
UPoint::operator/=(int c) {
	// FIXME: should we round or truncate?
	x = int(x / c);
	y = int(y / c);
	return *this;
}

inline UPoint &
UPoint::operator/=(double c) {
	// FIXME: should we round or truncate?
	x = int(x / c);
	y = int(y / c);
	return *this;
}

inline std::ostream &
operator<<(std::ostream & os, const UPoint & o) {
	return os << "UPoint[" << o.x << "," << o.y << "]";
}

//
// public operators
// inline implementation
//
inline UPoint operator+(const UPoint & p1, const UPoint & p2) {
	UPoint ret(p1);
	return ret += p2;
}

inline UPoint operator-(const UPoint & p1, const UPoint & p2) {
	UPoint ret(p1);
	return ret -= p2;
}

inline UPoint operator-(const UPoint & p) {
	return UPoint(-p.x, -p.y);
}

inline UPoint operator*(const UPoint & p, int c) {
	UPoint ret(p);
	return ret *= c;
}

inline UPoint operator*(int c, const UPoint & p) {
	UPoint ret(p);
	return ret *= c;
}

inline UPoint operator*(const UPoint & p, double c) {
	UPoint ret(p);
	return ret *= c;
}

inline UPoint operator*(double c, const UPoint & p) {
	UPoint ret(p);
	return ret *= c;
}

inline UPoint operator/(const UPoint & p, int c) {
	UPoint ret(p);
	return ret /= c;
}

inline UPoint operator/(const UPoint & p, double c) {
	UPoint ret(p);
	return ret /= c;
}


inline bool operator==(const UPoint & p1,const UPoint & p2) {
	return p1.x == p2.x && p1.y == p2.y;
}

inline bool operator!=(const UPoint & p1,const UPoint & p2) {
	return p1.x != p2.x || p1.y != p2.y;
}

//
// UPointObject
// inline implementation
//


inline
UPointObject::UPointObject() : UPoint() {}

inline
UPointObject::UPointObject(const UPoint & p) : UPoint(p) {}

inline
UPointObject::UPointObject(int x, int y) : UPoint(x, y) {}

inline unsigned int
UPointObject::hashCode() const {
	// FIXME
	return (((x << 8) | y) ^ y);
}

inline bool
UPointObject::equals(const UObject * obj) {
	if (const UPointObject * p = dynamic_cast<const UPointObject*>(obj)) {
		return (x == p->x) && (y == p->y);
	}
	return false;
}
inline bool
UPointObject::equals(const UPoint * obj) {
	if (obj) {
		return (x == obj->x) && (y == obj->y);
	}
	return false;
}

inline UObject *
UPointObject::clone() const {
	return new UPointObject(x, y);
}

//
// protected methods
//

inline std::ostream &
UPointObject::paramString(std::ostream & os) const {
	return os << x << "," << y;
}

} // namespace ufo

#endif // UPOINT_HPP
