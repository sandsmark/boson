/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/util/uinsets.hpp
    begin             : Sun May 27 2001
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

#ifndef UINSETS_HPP
#define UINSETS_HPP

#include "../uobject.hpp"

namespace ufo {

/**
  *@author Johannes Schmidt
  */

class UFO_EXPORT UInsets {
public:
	UInsets();
	UInsets(int top, int left, int bottom, int right);

	int getTop() const;
	int getLeft() const;
	int getBottom() const;
	int getRight() const;

	/** @return The total insets in horizontal direction, i.e. left + right. */
	int getHorizontal() const;
	/** @return The total insets in vertical direction, i.e. top + bottom. */
	int getVertical() const;

	void grow(const UInsets & add);

	bool isEmpty() const;

public: // Public operators
	bool operator()() { return !(isEmpty()); }
	bool operator!() { return isEmpty(); }

	/** Adds point p to this point
	  * @return Reference to this point.
	  */
	UInsets & operator+=(const UInsets & in);
	/** Subtracts point p from this point
	  * @return Reference to this point.
	  */
	UInsets & operator-=(const UInsets & in);

	friend std::ostream & operator<<(std::ostream & os, const UInsets & o);

public:  // Public attributes
	int top;
	int left;
	int bottom;
	int right;
};


//
// public operators
//
UFO_EXPORT UInsets operator+(const UInsets & in1, const UInsets & in2);
UFO_EXPORT UInsets operator-(const UInsets & in1, const UInsets & in2);

/// Equality
UFO_EXPORT bool operator==(const UInsets & in1,const UInsets & in2);
UFO_EXPORT bool operator!=(const UInsets & in1,const UInsets & in2);


/** wrapper class for UInsets derived from UObject
  * @author Johannes Schmidt
  */
class UFO_EXPORT UInsetsObject : public UInsets, public UObject {
	UFO_DECLARE_DYNAMIC_CLASS(UInsetsObject)
public:
	UInsetsObject();
	UInsetsObject(const UInsets & insets);
	UInsetsObject(int top, int left, int bottom, int right);

	//
	// overrides UObject
	//
	virtual unsigned int hashCode() const;
	virtual bool equals(const UObject * obj);
	virtual bool equals(const UInsets * obj);
	virtual UObject * clone() const;

protected:  // Protected methods
	virtual std::ostream & paramString(std::ostream & os) const;
};


//
// UInsets
// inline implementation
//


inline UInsets::UInsets() : top(0), left(0), bottom(0), right(0) {}

inline UInsets::UInsets(int top, int left, int bottom, int right)
	: top(top)
	, left(left)
	, bottom(bottom)
	, right(right)
{}


inline int
UInsets::getTop() const {
	return top;
}
inline int
UInsets::getLeft() const {
	return left;
}
inline int
UInsets::getBottom() const {
	return bottom;
}
inline int
UInsets::getRight() const {
	return right;
}

inline int
UInsets::getHorizontal() const {
	return left + right;
}
inline int
UInsets::getVertical() const {
	return top + bottom;
}



inline void
UInsets::grow(const UInsets & add) {
	top += add.top;
	left += add.left;
	bottom += add.bottom;
	right += add.right;
}

inline bool
UInsets::isEmpty() const {
	return (!(top && left && bottom && right));
}


inline UInsets &
UInsets::operator+=(const UInsets & in) {
	top += in.top;
	left += in.left;
	bottom += in.bottom;
	right += in.right;
	return *this;
}

inline UInsets &
UInsets::operator-=(const UInsets & in) {
	top -= in.top;
	left -= in.left;
	bottom -= in.bottom;
	right -= in.right;
	return *this;
}

inline std::ostream &
operator<<(std::ostream & os, const UInsets & o) {
	return os << "UInsets["
		<< "top=" << o.top
		<< ", left=" << o.left
		<< ", bottom=" << o.bottom
		<< ", right=" << o.right
		<< "]";
}

inline UInsets operator+(const UInsets & in1, const UInsets & in2) {
	UInsets ret(in1);
	return ret += in2;
}
inline UInsets operator-(const UInsets & in1, const UInsets & in2) {
	UInsets ret(in1);
	return ret -= in2;
}

/// Equality
inline bool operator==(const UInsets & in1,const UInsets & in2) {
	return ((in1.top == in2.top) &&
		(in1.left == in2.left) &&
		(in1.bottom == in2.bottom) &&
		(in1.right == in2.right));
}
inline bool operator!=(const UInsets & in1,const UInsets & in2) {
	return ((in1.top != in2.top) &&
		(in1.left != in2.left) &&
		(in1.bottom != in2.bottom) &&
		(in1.right != in2.right));
}

//
// URectangleObject
// inline implementation
//

inline
UInsetsObject::UInsetsObject() {}

inline
UInsetsObject::UInsetsObject(const UInsets & insets) : UInsets(insets) {}

inline
UInsetsObject::UInsetsObject(int top, int left, int bottom, int right)
	: UInsets(top, left, bottom, right) {}

//
// overrides UObject
//
inline unsigned int
UInsetsObject::hashCode() const {
	// FIXME
	// doh, too lazy
	int temp =  ((top << 16) | bottom);
	temp &= ~(right | left);
	return temp;
}

inline bool
UInsetsObject::equals(const UObject * obj) {
	if (const UInsets * insets = dynamic_cast<const UInsets*>(obj)) {
		return ((top == insets->top) &&
			(left == insets->left) &&
			(bottom == insets->bottom) &&
			(right == insets->right));
	}
	return false;
}

inline bool
UInsetsObject::equals(const UInsets * obj) {
	return ((top == obj->top) &&
		(left == obj->left) &&
		(bottom == obj->bottom) &&
		(right == obj->right));
}

inline UObject *
UInsetsObject::clone() const {
	return new UInsetsObject(top, left, bottom, right);
}


//
// Protected methods
//

inline std::ostream &
UInsetsObject::paramString(std::ostream & os) const {
	return os << "top=" << top
		<< ", left=" << left
		<< ", bottom=" << bottom
		<< ", right=" << right;
}


} // namespace ufo

#endif // UINSETS_HPP
