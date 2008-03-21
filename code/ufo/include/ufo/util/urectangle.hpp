/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/util/urectangle.hpp
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA *
 ***************************************************************************/

#ifndef URECTANGLE_HPP
#define URECTANGLE_HPP

#include "../uobject.hpp"

#include "upoint.hpp"
#include "udimension.hpp"
#include "uinsets.hpp"

namespace ufo {

/** @short An abstraction of a rectangle (x, y, width and height).
  * @ingroup appearance
  *
  * This class is not part of the @ref UObject inheritance structure.
  * Use instead @ref URectangleObject if you need a rectangle derived
  * from UObject.
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT URectangle {
public:
	inline URectangle();
	inline URectangle(int x, int y, int w, int h);
	inline URectangle(const UPoint & p, const UDimension & d);
	/** Computes a rectangle between two points. */
	inline URectangle(const UPoint & p1, const UPoint & p2);
	/** Creates a rectangle with locatin 0,0 and the given size. */
	inline URectangle(const UDimension & d);

	inline UPoint getLocation() const;
	inline UDimension getSize() const;

	inline void setBounds(int x, int y, int w, int h);
	inline void setBounds(const URectangle & rect);

	/** @return True when the given point pos is inside the rectangle
	  * or on the edge of the rectangle
	  */
	inline bool contains(const UPoint & pos) const;

	/** @return True if both, width and height are not equal to @p invalid
	  * @see invalid
	  */
	inline bool isValid() const;
	/** @return True if the width and height are equal to @p invalid
	  * @see invalid
	  */
	inline bool isInvalid() const;
	/** @return True if width or height is equal to 0. */
	inline bool isEmpty() const;

	/** Clamps this URectangle to have at most the dimension of
	  * the given @p maxDim. Does nothing if maxDim is smaller than
	  * the size of this rectangle.
	  */
	inline void clamp(const UDimension & maxDim);
	/** Expands this URectangle to have at least the dimension of
	  * the given @p minDim. Does nothing if minDim is bigger than
	  * the size of this rectangle.
	  */
	inline void expand(const UDimension & minDim);

	/** Intersects this rectangle with the given rectangle
	  */
	inline void intersect(const URectangle & rect);
	/** Unites this rectangle with the given rectangle.
	  * @see computeUnion
	  */
	inline void unite(const URectangle & rect);

	/** @return The union of this rectangle and the given rectangle
	  */
	inline URectangle computeUnion(const URectangle & src) const;

	/** computes the rectangle that contains both src rectangles and
	  * saves the values in the dest rectangle within creating a new one.
	  * It is allowed to use one source rectangle as dest rectangle, e.g.
	  * <code>URectangle::computeUnion(src, src2, src);</code>
	  * @return dest
	  */
	inline static URectangle * computeUnion(const URectangle & src1,
		const URectangle & src2, URectangle * dest);

public: // Public operators
	/** @see isValid
	  * @return True if width and height have both "valid" values.
	  */
	inline bool operator()() const;
	/** @see isInvalid
	  * @return True if width or height is "invalid".
	  */
	inline bool operator!() const;


	/** Moves this rectangle using the coordinates of the given point
	  * @return Reference to this rectangle.
	  */
	inline URectangle & operator+=(const UPoint & p);
	/** Moves this rectangle using the coordinates of the given point
	  * @return Reference to this rectangle.
	  */
	inline URectangle & operator-=(const UPoint & p);

	/** Increases this rectangle using the given dimension
	  * @return Reference to this rectangle.
	  */
	inline URectangle & operator+=(const UDimension & dim);
	/** Shrinks this rectangle using the given dimension
	  * @return Reference to this rectangle.
	  */
	inline URectangle & operator-=(const UDimension & dim);

	/** Increases this rectangle using the given insets
	  * @return Reference to this rectangle.
	  */
	inline URectangle & operator+=(const UInsets & insets);
	/** Shrinks this rectangle using the given insets
	  * @return Reference to this rectangle.
	  */
	inline URectangle & operator-=(const UInsets & insets);

	inline friend std::ostream & operator<<(std::ostream & os, const URectangle & o);
public:  // Public attributes
	int x;
	int y;
	int w;
	int h;
public: // Public static attributes
	static URectangle invalid;
};


//
// public operators
//
inline URectangle operator+(const URectangle & rect, const UPoint & p);
inline URectangle operator-(const URectangle & rect, const UPoint & p);

inline URectangle operator+(const URectangle & rect, const UDimension & dim);
inline URectangle operator-(const URectangle & rect, const UDimension & dim);

inline URectangle operator+(const URectangle & rect, const UInsets & in);
inline URectangle operator-(const URectangle & rect, const UInsets & in);

/// Equality
inline bool operator==(const URectangle & r1,const URectangle & r2);
inline bool operator!=(const URectangle & r1,const URectangle & r2);

/** wrapper class for URectangle derived from UObject
  * @author Johannes Schmidt
  */
class UFO_EXPORT URectangleObject : public URectangle, public UObject {
	UFO_DECLARE_DYNAMIC_CLASS(URectangleObject)
public:
	inline URectangleObject();
	inline URectangleObject(const URectangle & rect);
	inline URectangleObject(int x, int y, int w, int h);
	inline URectangleObject(const UPoint & p, const UDimension & d);
	/** Computes a rectangle between two points. */
	inline URectangleObject(const UPoint & p1, const UPoint & p2);

	//
	// overrides UObject
	//
	virtual unsigned int hashCode() const;
	virtual bool equals(const UObject * obj);
	virtual bool equals(const URectangle * obj);
	virtual UObject * clone() const;

protected:  // Protected methods
	virtual std::ostream & paramString(std::ostream & os) const;
};

//
// URectangle
// inline implementation
//

inline URectangle::URectangle() : x(0), y(0), w(0), h(0) {}

inline URectangle::URectangle(int x, int y, int w, int h)
	: x(x), y(y), w(w), h(h) {}

inline URectangle::URectangle(const UPoint & p, const UDimension & d)
	: x(p.x)
	, y(p.y)
	, w(d.w)
	, h(d.h)
{}

inline URectangle::URectangle(const UPoint & p1, const UPoint & p2) {
	x = std::min(p1.x, p2.x);
	y = std::min(p1.y, p2.y);
	w = std::abs(p2.x - p1.x);
	h = std::abs(p2.y - p1.y);
}

inline URectangle::URectangle(const UDimension & d)
	: x(0)
	, y(0)
	, w(d.w)
	, h(d.h)
{}


inline UPoint
URectangle::getLocation() const {
	return UPoint(x, y);
}

inline UDimension
URectangle::getSize() const {
	return UDimension(w, h);
}


inline void
URectangle::setBounds(int x, int y, int w, int h) {
	this->x = x;
	this->y = y;
	this->w = w;
	this->h = h;
}

inline void
URectangle::setBounds(const URectangle & rect) {
	x = rect.x;
	y =  rect.y;
	w =  rect.w;
	h =  rect.h;
}

inline void
URectangle::clamp(const UDimension & maxDim) {
	w = std::min(w, maxDim.w);
	h = std::min(h, maxDim.h);
}

inline void
URectangle::expand(const UDimension & minDim) {
	w = std::max(w, minDim.w);
	h = std::max(h, minDim.h);
}

inline void
URectangle::intersect(const URectangle & rect) {
	int x1 = std::max(x, rect.x);
	int y1 = std::max(y, rect.y);
	int x2 = std::min(x + w, rect.x + rect.w);
	int y2 = std::min(y + h, rect.y + rect.h);

	setBounds(x1, y1, x2 - x1, y2 - y1);
	w = std::max(w, 0);
	h = std::max(h, 0);
}

inline void
URectangle::unite(const URectangle & rect) {
	int x1 = std::min(x, rect.x);
	int y1 = std::min(y, rect.y);
	int x2 = std::max(x + w, rect.x + rect.w);
	int y2 = std::max(y + h, rect.y + rect.h);

	setBounds(x1, y1, x2 - x1 + 1, y2 - y1 + 1);
}

inline bool
URectangle::contains(const UPoint & pos) const {
	return (
		pos.x >= x && pos.x < x + w &&
		pos.y >= y && pos.y < y + h
	);
}

inline URectangle
URectangle::computeUnion(const URectangle & src) const {
	URectangle ret;
	URectangle::computeUnion(*this, src, &ret);
	return ret;
}

inline URectangle *
URectangle::computeUnion(const URectangle & src1,
		const URectangle & src2, URectangle * dest) {
	if (dest) {
		// allow using src rectangle as dest rectangle
		int x = std::min(src1.x, src2.x);
		int y = std::min(src1.y, src2.y);
		dest->w = std::max(src1.x + src1.w, src2.x + src2.w) - x;
		dest->h = std::max(src1.y + src1.h, src2.y + src2.h) - y;
		dest->x = x;
		dest->y = y;
	}
	return dest;
}

inline bool
URectangle::isInvalid() const {
	return (*this == URectangle::invalid);
}

inline bool
URectangle::isValid() const {
	return (x != URectangle::invalid.x &&
		y != URectangle::invalid.y &&
		w != URectangle::invalid.w &&
		h != URectangle::invalid.h);
}

inline bool
URectangle::isEmpty() const {
	return (!(w && h));
}

inline std::ostream &
operator<<(std::ostream & os, const URectangle & o) {
	return os << "URectangle[" << o.x << "," << o.y
		<< "," << o.w << "x" << o.h << "]";
}

inline bool
URectangle::operator()() const {
	return isValid();
}

inline bool
URectangle::operator!() const {
	return !isValid();
}

inline URectangle &
URectangle::operator+=(const UPoint & p) {
	this->x += p.x;
	this->y += p.y;
	return *this;
}

inline URectangle &
URectangle::operator-=(const UPoint & p) {
	this->x -= p.x;
	this->y -= p.y;
	return *this;
}


inline URectangle &
URectangle::operator+=(const UDimension & dim) {
	this->w += dim.w;
	this->h += dim.h;
	return *this;
}

inline URectangle &
URectangle::operator-=(const UDimension & dim) {
	this->w -= dim.w;
	this->h -= dim.h;
	return *this;
}

inline URectangle &
URectangle::operator+=(const UInsets & insets) {
	x -= insets.left;
	y -= insets.top;
	w += insets.getHorizontal();
	h += insets.getVertical();
	return *this;
}

inline URectangle &
URectangle::operator-=(const UInsets & insets) {
	x += insets.left;
	y += insets.top;
	w -= insets.getHorizontal();
	h -= insets.getVertical();
	return *this;
}


//
// public operators
// inline implementation
//
inline URectangle operator+(const URectangle & rect, const UPoint & p) {
	URectangle ret(rect);
	return ret += p;
}

inline URectangle operator-(const URectangle & rect, const UPoint & p) {
	URectangle ret(rect);
	return ret -= p;
}

inline URectangle operator+(const URectangle & rect, const UDimension & dim) {
	URectangle ret(rect);
	return ret += dim;
}

inline URectangle operator-(const URectangle & rect, const UDimension & dim) {
	URectangle ret(rect);
	return ret -= dim;
}

inline URectangle operator+(const URectangle & rect, const UInsets & in) {
	URectangle ret(rect);
	return ret += in;
}

inline URectangle operator-(const URectangle & rect, const UInsets & in) {
	URectangle ret(rect);
	return ret -= in;
}


inline bool operator==(const URectangle & r1,const URectangle & r2) {
	return ((r1.x == r2.x) &&
		(r1.y == r2.y) &&
		(r1.w == r2.w) &&
		(r1.h == r2.w));
}

inline bool operator!=(const URectangle & r1,const URectangle & r2) {
	return !(operator==(r1, r2));
}

//
// URectangleObject
// inline implementation
//


inline
URectangleObject::URectangleObject() {}

inline
URectangleObject::URectangleObject(const URectangle & rect)
	: URectangle(rect) {}

inline
URectangleObject::URectangleObject(int x, int y, int w, int h)
	: URectangle(x, y, w, h) {}

inline
URectangleObject::URectangleObject(const UPoint & p, const UDimension & d)
	: URectangle(p, d) {}

inline
URectangleObject::URectangleObject(const UPoint & p1, const UPoint & p2)
	: URectangle(p1, p2) {}

//
// overrides UObject
//
inline unsigned int
URectangleObject::hashCode() const {
	// FIXME
	// doh, too lazy
	int temp =  ((w << 16) | h);
	temp &= ~(x | y);
	return temp;
}

inline bool
URectangleObject::equals(const UObject * obj) {
	if (const URectangle * rect = dynamic_cast<const URectangle*>(obj)) {
		return ((x == rect->x) &&
			(y == rect->y) &&
			(w == rect->w) &&
			(h == rect->h));
	}
	return false;
}

inline bool
URectangleObject::equals(const URectangle * obj) {
	return ((x == obj->x) &&
		(y == obj->y) &&
		(w == obj->w) &&
		(h == obj->h));
}

inline UObject *
URectangleObject::clone() const {
	return new URectangleObject(x, y, w, h);
}


//
// Protected methods
//

inline std::ostream &
URectangleObject::paramString(std::ostream & os) const {
	return os << x << "," << y << "," << w << "x" << h;
}

} // namespace ufo

#endif // URECTANGLE_HPP
