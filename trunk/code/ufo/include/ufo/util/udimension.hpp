/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/util/udimension.hpp
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#ifndef UDIMENSION_HPP
#define UDIMENSION_HPP

#include "../uobject.hpp"

namespace ufo {

/**
  *@author Johannes Schmidt
  */
class UFO_EXPORT UDimension {
public:
	UDimension();
	UDimension(int w, int h);

	int getWidth() const;
	int getHeight() const;

	void setSize(int w, int h);
	/** for convenience */
	UDimension getSize() const;
	bool isEmpty() const;
public: // Public operators
	bool operator()() { return !(isEmpty()); }
	bool operator!() { return isEmpty(); }

	/** Adds dimension dim to this dimension
	  * @return Reference to this dimension.
	  */
	UDimension & operator+=(const UDimension & dim);
	/** Subtracts dimension dim from this dimension
	  * @return Reference to this dimension.
	  */
	UDimension & operator-=(const UDimension & dim);
	/** Multiplies c with both width and height of this dimension.
	  * @return Reference to this dimension.
	  */
	UDimension & operator*=(int c);
	/** Multiplies c with both width and height of this dimension.
	  * @return Reference to this dimension.
	  */
	UDimension & operator*=(double c);
	/** Divides both width and height by c.
	  * @return Reference to this dimension.
	  */
	UDimension & operator/=(int c);
	/** Divides both width and height by c.
	  * @return Reference to this dimension.
	  */
	UDimension & operator/=(double c);

	friend std::ostream & operator<<(std::ostream & os, const UDimension & o);
public:  // Public attributes
	/**  width */
	int w;
	/**  height */
	int h;
};

//
// public operators
//
UFO_EXPORT UDimension operator+(const UDimension & dim1, const UDimension & dim2);
UFO_EXPORT UDimension operator-(const UDimension & dim1, const UDimension & dim2);
UFO_EXPORT UDimension operator*(const UDimension & dim, int c);
UFO_EXPORT UDimension operator*(int c, const UDimension & p);
UFO_EXPORT UDimension operator*(const UDimension & dim, double c);
UFO_EXPORT UDimension operator*(double c, const UDimension & p);
UFO_EXPORT UDimension operator/(const UDimension & dim, int c);
UFO_EXPORT UDimension operator/(const UDimension & dim, double c);

/// Equality
UFO_EXPORT bool operator==(const UDimension & dim1,const UDimension & dim2);
UFO_EXPORT bool operator!=(const UDimension & dim1,const UDimension & dim2);


/** wrapper class for UDimension which is derived from UObject.
  *@author Johannes Schmidt
  */
class UDimensionObject : public UDimension, public UObject {
	UFO_DECLARE_DYNAMIC_CLASS(UDimensionObject)
public:
	UDimensionObject();
	UDimensionObject(const UDimension & dim);
	UDimensionObject(int w, int h);

	//
	// overrides UObject
	//
	virtual unsigned int hashCode() const;
	virtual bool equals(const UObject * obj);
	virtual bool equals(const UDimension * obj);
	virtual UObject * clone() const;

protected:  // Protected methods
	virtual std::ostream & paramString(std::ostream & os) const;
};


//
// UDimension
// inline implementation
//

inline
UDimension::UDimension() : w(0), h(0) {}

inline
UDimension::UDimension(int w, int h) : w(w), h(h) {}


inline int
UDimension::getWidth() const { return w; }

inline int
UDimension::getHeight() const { return h; }



inline void
UDimension::setSize(int w, int h) {
	this->w = w;
	this->h = h;
}

inline UDimension
UDimension::getSize() const {
	return UDimension(w, h);
}


inline bool
UDimension::isEmpty() const {
	return (!(w && h));
}


inline UDimension &
UDimension::operator+=(const UDimension & dim) {
	w += dim.w;
	h += dim.h;
	return *this;
}

inline UDimension &
UDimension::operator-=(const UDimension & dim) {
	w -= dim.w;
	h -= dim.h;
	return *this;
}

inline UDimension &
UDimension::operator*=(int c) {
	w *= c;
	h *= c;
	return *this;
}

inline UDimension &
UDimension::operator*=(double c) {
	// FIXME: should we round or truncate?
	w = int(w * c);
	h = int(h * c);
	return *this;
}

inline UDimension &
UDimension::operator/=(int c) {
	// FIXME: should we round or truncate?
	w = int(w / c);
	h = int(h / c);
	return *this;
}

inline UDimension &
UDimension::operator/=(double c) {
	// FIXME: should we round or truncate?
	w = int(w / c);
	h = int(h / c);
	return *this;
}

inline std::ostream &
operator<<(std::ostream & os, const UDimension & o) {
	return os << "UDimension[" << o.w << "x" << o.h << "]";
}

//
// public operators
// inline implementation
//
inline UDimension operator+(const UDimension & dim1, const UDimension & dim2) {
	UDimension ret(dim1);
	return ret += dim2;
}

inline UDimension operator-(const UDimension & dim1, const UDimension & dim2) {
	UDimension ret(dim1);
	return ret -= dim2;
}

inline UDimension operator*(const UDimension & dim, int c) {
	UDimension ret(dim);
	return ret *= c;
}

inline UDimension operator*(int c, const UDimension & dim) {
	UDimension ret(dim);
	return ret *= c;
}

inline UDimension operator*(const UDimension & dim, double c) {
	UDimension ret(dim);
	return ret *= c;
}

inline UDimension operator*(double c, const UDimension & dim) {
	UDimension ret(dim);
	return ret *= c;
}

inline UDimension operator/(const UDimension & dim, int c) {
	UDimension ret(dim);
	return ret /= c;
}

inline UDimension operator/(const UDimension & dim, double c) {
	UDimension ret(dim);
	return ret /= c;
}


inline bool operator==(const UDimension & dim1,const UDimension & dim2) {
	return dim1.w == dim2.w && dim1.h == dim2.h;
}

inline bool operator!=(const UDimension & dim1,const UDimension & dim2) {
	return dim1.w != dim2.w || dim1.h != dim2.h;
}

//
// UDimensionObject
// inline implementation
//

inline
UDimensionObject::UDimensionObject() {}

inline
UDimensionObject::UDimensionObject(const UDimension & dim) : UDimension(dim) {}

inline
UDimensionObject::UDimensionObject(int w, int h) : UDimension(w, h) {}

inline unsigned int
UDimensionObject::hashCode() const {
	// FIXME
	return (((w << 8) | h) ^ w);
}

inline bool
UDimensionObject::equals(const UObject * obj) {
	if (const UDimension * dim = dynamic_cast<const UDimension*>(obj)) {
		return (w == dim->w) && (h == dim->h);
	}
	return false;
}

inline bool
UDimensionObject::equals(const UDimension * obj) {
	if (obj) {
		return (w == obj->w) && (h == obj->h);
	}
	return false;
}

inline UObject *
UDimensionObject::clone() const {
	return new UDimensionObject(w, h);
}

//
// protected methods
//

inline std::ostream &
UDimensionObject::paramString(std::ostream & os) const {
	return os << w << "x" << h;
}

} // namespace ufo

#endif // UDIMENSION_HPP
