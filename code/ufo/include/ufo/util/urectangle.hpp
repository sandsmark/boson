/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#ifndef URECTANGLE_HPP
#define URECTANGLE_HPP

#include "../uobject.hpp"

#include "upoint.hpp"
#include "udimension.hpp"

namespace ufo {

/**
  * @author Johannes Schmidt
  */

class UFO_EXPORT URectangle {
public:
	URectangle();
	URectangle(int x, int y, int w, int h);
	URectangle(const UPoint & p, const UDimension & d);
	/** Computes a rectangle between two points. */
	URectangle(const UPoint & p1, const UPoint & p2);

	UPoint getLocation() const;
	UDimension getSize() const;

	void setBounds(int x, int y, int w, int h);
	void setBounds(const URectangle & rect);

	/** Returns true when the given point pos is inside the rectangle
	  * or on the edge of the rectangle
	  */
	bool contains(const UPoint & pos);
	
	/** Computes the union which contains both rectangles.
	  */
	URectangle computeUnion(const URectangle & src) const;
	
	/** computes the rectangle that contains both src rectangles and
	  * saves the values in the dest rectangle within creating a new one.
	  * It is allowed to use one source rectangle as dest rectangle, e.g.
	  * <code>URectangle::computeUnion(src, src2, src);</code>
	  * @return dest
	  */
	static URectangle * computeUnion(const URectangle & src1,
		const URectangle & src2, URectangle * dest);
	
	/** returns true if both, width and height are != 0
	  */
	bool isEmpty() const;

public: // Public operators
	bool operator()() { return !(isEmpty()); }
	bool operator!() { return isEmpty(); }


	/** Moves this rectangle using the coordinates of the given point
	  * @return Reference to this rectangle.
	  */
	URectangle & operator+=(const UPoint & p);
	/** Moves this rectangle using the coordinates of the given point
	  * @return Reference to this rectangle.
	  */
	URectangle & operator-=(const UPoint & p);
	
	/** Increases this rectangle using the given dimension
	  * @return Reference to this rectangle.
	  */
	URectangle & operator+=(const UDimension & dim);
	/** Shrinks this rectangle using the given dimension
	  * @return Reference to this rectangle.
	  */
	URectangle & operator-=(const UDimension & dim);

	friend std::ostream & operator<<(std::ostream & os, const URectangle & o);
/*
	UObject * clone() const;
protected:  // Protected methods
	std::ostream & paramString(std::ostream & os) const;
*/
public:  // Public attributes
	int x;
	int y;
	int w;
	int h;
};


//
// public operators
//
UFO_EXPORT URectangle operator+(const URectangle & rect, const UPoint & p);
UFO_EXPORT URectangle operator-(const URectangle & rect, const UPoint & p);

UFO_EXPORT URectangle operator+(const URectangle & rect, const UDimension & dim);
UFO_EXPORT URectangle operator-(const URectangle & rect, const UDimension & dim);

/// Equality
UFO_EXPORT bool operator==(const URectangle & r1,const URectangle & r2);
UFO_EXPORT bool operator!=(const URectangle & r1,const URectangle & r2);

/** wrapper class for URectangle derived from UObject
  * @author Johannes Schmidt
  */
class UFO_EXPORT URectangleObject : public URectangle, public UObject {
	UFO_DECLARE_DYNAMIC_CLASS(URectangleObject)
public:
	URectangleObject();
	URectangleObject(const URectangle & rect);
	URectangleObject(int x, int y, int w, int h);
	URectangleObject(const UPoint & p, const UDimension & d);
	/** Computes a rectangle between two points. */
	URectangleObject(const UPoint & p1, const UPoint & p2);

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
	// FIXME
	// hm, what about std::min und std:abs ?
	// but: what about ms vc6
	if (p2.x > p1.x) {
		x = p1.x;
		w = p2.x - p1.x;
	} else {
		x = p2.x;
		w = p1.x - p2.x;
	}

	if (p2.y > p1.y) {
		y = p1.y;
		h = p2.y - p1.y;
	} else {
		y = p2.y;
		h = p1.y - p2.y;
	}
}


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


inline bool
URectangle::contains(const UPoint & pos) {
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
URectangle::isEmpty() const {
	return (!(w && h));
}

inline std::ostream &
operator<<(std::ostream & os, const URectangle & o) {
	return os << "URectangle[" << o.x << "," << o.y
		<< "," << o.w << "x" << o.h << "]";
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
