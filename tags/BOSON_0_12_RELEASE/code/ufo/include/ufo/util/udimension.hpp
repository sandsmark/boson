/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
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

#include "uinsets.hpp"

namespace ufo {

/** @short An abstraction to dimension (width and height).
  * @ingroup appearance
  *
  * This class is not part of the @ref UObject inheritance structure.
  * Use instead @ref UDimensionObject if you need a dimension derived
  * from UObject.
  *
  * @author Johannes Schmidt
  */
class UFO_EXPORT UDimension {
public:
	/** Creates an empty dimension (width == 0 and height == 0).
	  */
	inline UDimension();
	/** Creates a dimension with the given width and height.
	  * @param w The width
	  * @param h The height
	  */
	inline UDimension(int w, int h);

	/** @return The width of this dimension. */
	inline int getWidth() const;
	/** @return The height of this dimension. */
	inline int getHeight() const;


	/** @return True if both, width and height are equal to @p invalid
	  * @see invalid
	  */
	inline bool isInvalid() const;
	/** @return True if both, width and height are not equal to @p invalid
	  * @see invalid
	  */
	inline bool isValid() const;
	/** @return True if width or height is equal to 0. */
	inline bool isEmpty() const;

	/** Clamps this UDimension to have at most the dimension of
	  * the given @p maxDim. Does nothing if maxDim is smaller than
	  * this UDimension.
	  */
	inline void clamp(const UDimension & maxDim);
	/** Expands this UDimension to have at least the dimension of
	  * the given @p minDim. Does nothing if minDim is bigger than
	  * this UDimension.
	  */
	inline void expand(const UDimension & minDim);

	/** If width resp. height of this dimension is equal to @p invalid,
	  * it is replaced by the width resp. height of the given dimension.
	  * @param dim The dimension which should be used for updates
	  *  of invalid values
	  */
	inline void update(const UDimension & dim);
	/** If width resp. height of the given dimension is not euqal to
	  * @p invalid, the width resp. height of this dimension is replaced by
	  * this value.
	  * @param dim The dimension whose valid values replaces the values
	  *  of this dimension.
	  */
	inline void transcribe(const UDimension & dim);

	/** Sets the size of this dimension to the given values.
	  * @param w The new width
	  * @param h The new height
	  */
	inline void setSize(int w, int h);
	/** This method is for convenience and mimics the API of widget types.
	  * @return A copy of this dimension object.
	  */
	inline UDimension getSize() const;
public: // Public operators
	/** @see isValid
	  * @return True if width and height have both "valid" values.
	  */
	inline bool operator()() const;
	/** @see isInvalid
	  * @return True if width or height is "invalid".
	  */
	inline bool operator!() const;

	/** Adds dimension @p dim to this dimension
	  * @return Reference to this dimension.
	  */
	inline UDimension & operator+=(const UDimension & dim);
	/** Subtracts dimension @p dim from this dimension
	  * @return Reference to this dimension.
	  */
	inline UDimension & operator-=(const UDimension & dim);

	/** Increases this dimension using the given insets
	  * @return Reference to this rectangle.
	  */
	inline UDimension & operator+=(const UInsets & insets);
	/** Shrinks this dimension using the given insets
	  * @return Reference to this rectangle.
	  */
	inline UDimension & operator-=(const UInsets & insets);

	/** Multiplies @p c with both width and height of this dimension.
	  * @return Reference to this dimension.
	  */
	inline UDimension & operator*=(int c);
	/** Multiplies @p c with both width and height of this dimension.
	  * @return Reference to this dimension.
	  */
	inline UDimension & operator*=(double c);
	/** Divides both width and height by @p c.
	  * @return Reference to this dimension.
	  */
	inline UDimension & operator/=(int c);
	/** Divides both width and height by @p c.
	  * @return Reference to this dimension.
	  */
	inline UDimension & operator/=(double c);

	inline friend std::ostream & operator<<(std::ostream & os, const UDimension & o);
public:  // Public attributes
	/** The width of this dimension object. */
	int w;
	/** The height of this dimension object. */
	int h;
public: // Public static attributes
	static UDimension maxDimension;
	static UDimension invalid;
};

//
// public operators
//
inline UDimension operator+(const UDimension & dim1, const UDimension & dim2);
inline UDimension operator-(const UDimension & dim1, const UDimension & dim2);
inline UDimension operator+(const UDimension & dim, const UInsets & in);
inline UDimension operator-(const UDimension & dim, const UInsets & in);
inline UDimension operator*(const UDimension & dim, int c);
inline UDimension operator*(int c, const UDimension & p);
inline UDimension operator*(const UDimension & dim, double c);
inline UDimension operator*(double c, const UDimension & p);
inline UDimension operator/(const UDimension & dim, int c);
inline UDimension operator/(const UDimension & dim, double c);

/// Equality
inline bool operator==(const UDimension & dim1,const UDimension & dim2);
inline bool operator!=(const UDimension & dim1,const UDimension & dim2);


/** wrapper class for UDimension which is derived from UObject.
  *@author Johannes Schmidt
  */
class UFO_EXPORT UDimensionObject : public UDimension, public UObject {
	UFO_DECLARE_DYNAMIC_CLASS(UDimensionObject)
public:
	inline UDimensionObject();
	inline UDimensionObject(const UDimension & dim);
	inline UDimensionObject(int w, int h);

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

inline bool
UDimension::isInvalid() const {
	return (*this == UDimension::invalid);
}

inline bool
UDimension::isValid() const {
	return (w != UDimension::invalid.w &&
		h != UDimension::invalid.h);
}

inline bool
UDimension::isEmpty() const {
	return (!(w && h));
}

inline void
UDimension::clamp(const UDimension & maxDim) {
	w = std::min(w, maxDim.w);
	h = std::min(h, maxDim.h);
}

inline void
UDimension::expand(const UDimension & minDim) {
	w = std::max(w, minDim.w);
	h = std::max(h, minDim.h);
}

inline void
UDimension::update(const UDimension & dim) {
	if (w == invalid.w) {
		w = dim.w;
	}
	if (h == invalid.h) {
		h = dim.h;
	}
}

inline void
UDimension::transcribe(const UDimension & dim) {
	if (dim.w != invalid.w) {
		w = dim.w;
	}
	if (dim.h != invalid.h) {
		h = dim.h;
	}
}

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
UDimension::operator()() const {
	return isValid();
}

inline bool
UDimension::operator!() const {
	return !isValid();
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
UDimension::operator+=(const UInsets & insets) {
	w += insets.getHorizontal();
	h += insets.getVertical();
	return *this;
}

inline UDimension &
UDimension::operator-=(const UInsets & insets) {
	w -= insets.getHorizontal();
	h -= insets.getVertical();
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

inline UDimension operator+(const UDimension & dim, const UInsets & in) {
	UDimension ret(dim);
	return ret += in;
}

inline UDimension operator-(const UDimension & dim, const UInsets & in) {
	UDimension ret(dim);
	return ret -= in;
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
