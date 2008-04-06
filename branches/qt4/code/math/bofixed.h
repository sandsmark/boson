/*
    This file is part of the Boson game
    Copyright (C) 2004-2005 Andreas Beckermann (b_mann@gmx.de)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

// note the copyright above: this is LGPL!

#ifndef BOFIXED_H
#define BOFIXED_H

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <qglobal.h> // qint32, ...

// Define this when you want bofixed to use normal floats (so that essentially
//  bofixed = float)
//#define BOFIXED_IS_FLOAT



// we have 32 bits available, the bits before the decimal point are responible
// for how large the numbers can be (10 bits mean up to 2^(10-1)-1). the bits
// after the decimal point are responsible for how precise we are.
// AB: i believe 15 bits before the point are sufficient for boson, since
// canvas/world/cell coordinates are always <= 500. 15 bits support numbers up
// to 16383.
#define BITS_AFTER_POINT (32-15)
// 2^(BITS_AFTER_POINT) -> this is required for float conversion which doesn't
// like shifting. multiplying by BITS_POW is equal to shifting left by
// BITS_AFTER_POINT.
#define BITS_POW (0x1<<BITS_AFTER_POINT)

// AB: on fixed point arithmetics:
// conversion from/to int is easy - just shift to the left/right by the number
//   of bits after (!) the point.
// addition is easy in fixed point arithmetics, as we can use usual int addition.
// subtraction is trivial (negative addition).
// multiplication and division however need a bit more work. the cpu "sees"
//   BITS_AFTER_POINT additional digits that have no actual meaning. e.g. we have
//   "010.00" for a 5 bit type with 2 bits after the point. this is a decimal 2,
//   but interpreted as an int it's a 8. square this number and we should get 4
//   (i.e. "100.00"), but what we get is "10000.00", which needs more than our 5
//   bits and is obviously not the correct value. we can fix that by shifting
//   back by BITS_AFTER_POINT bits, however we need a type that can take all the
//   bits temporarily (64 bits, as we use 32 bits in bofixed) before we shift.

/**
 * @short A fixed point type
 *
 * This class is a replacement for the standard float datatype. It uses fixed
 * point arithmetic (float and double use floating point arithmetics) and is
 * based on integer calculations, therefore the results of the calculations are
 * not dependand on the FPU of your computer. This is necessary because boson
 * often depends on the clients to calculate exactly the same values, however
 * when using floating point calculations this is not always the case (although
 * all FPUs involved are IEEE complient).
 *
 * The bofixed class is meant to be used like any other type, in particular it
 * should be used a simple drop in replacement for floats. You should be able to
 * just replace the declaration of "float" by "bofixed".
 *
 * Most of this class consists of overloading operators for the different types.
 * Therefore you should easily be able to do things like
 * <pre>
 * float a = my_value;
 * bofixed b = ma_2nd_value;
 * b += a;
 * b += 0.15f;
 * b += (int)10
 * </pre>
 * without explicitly converting values to bofixed.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
#ifndef BOFIXED_IS_FLOAT
class bofixed
{
public:
	/**
	 * Construct a fixed point variable with initial value 0.0
	 **/
	bofixed()
		: mValue(0)
	{
	}
	bofixed(const bofixed& f)
		: mValue(f.mValue)
	{
	}
	bofixed(qint32 f)  : mValue(f << BITS_AFTER_POINT) { }
	bofixed(quint32 f) : mValue(f << BITS_AFTER_POINT) { }
	bofixed(qint8 f)   : mValue(((qint32)f) << BITS_AFTER_POINT) { }
	bofixed(quint8 f)  : mValue(((qint32)f) << BITS_AFTER_POINT) { }
	bofixed(qint16 f)  : mValue(((qint32)f) << BITS_AFTER_POINT) { }
	bofixed(quint16 f) : mValue(((qint32)f) << BITS_AFTER_POINT) { }
	bofixed(long f)   : mValue(((qint32)f) << BITS_AFTER_POINT) { }
	bofixed(unsigned long f)  : mValue(((qint32)f) << BITS_AFTER_POINT) { }
	bofixed(float f)    : mValue((qint32)(nearbyintf(f * BITS_POW))) { }
	bofixed(double f)   : mValue((qint32)(nearbyintf(f * BITS_POW))) { }

	inline bool equals(const bofixed& f) const         { return (mValue == f.mValue); }
	inline bool isGreater(const bofixed& f) const      { return (mValue >  f.mValue); }
	inline bool isGreaterEqual(const bofixed& f) const { return (mValue >= f.mValue); }
	inline bool isLess(const bofixed& f) const         { return (mValue <  f.mValue); }
	inline bool isLessEqual(const bofixed& f) const    { return (mValue <= f.mValue); }

	/**
	 * @return The fixed point value castet to integer. This behaves just
	 * like casting a floating point number to int, i.e. the digits after
	 * the decimal point are "cut off".
	 **/
	inline qint32 toInt() const { return mValue >> BITS_AFTER_POINT; }
	inline float toFloat() const { return ((float)mValue) / BITS_POW; }
	inline double toDouble() const { return (double)toFloat(); }

	/**
	 * @return The internal representation of the fixed point value. This
	 * may be useful for debugging-
	 **/
	qint32 rawInt() const { return mValue; }
	void setFromRawInt(qint32 r) { mValue = r; }

	bofixed& operator=(const bofixed& f) { mValue = f.mValue; return *this; }
	bofixed& operator=(qint32 f)  { mValue = f << BITS_AFTER_POINT; return *this; }
	bofixed& operator=(quint32 f) { mValue = f << BITS_AFTER_POINT; return *this; }
	bofixed& operator=(qint8 f)   { mValue = ((qint32)f) << BITS_AFTER_POINT; return *this; }
	bofixed& operator=(quint8 f)  { mValue = ((qint32)f) << BITS_AFTER_POINT; return *this; }
	bofixed& operator=(qint16 f)  { mValue = f << BITS_AFTER_POINT; return *this; }
	bofixed& operator=(quint16 f) { mValue = f << BITS_AFTER_POINT; return *this; }
	bofixed& operator=(long f)   { mValue = f << BITS_AFTER_POINT; return *this; }
	bofixed& operator=(unsigned long f)  { mValue = f << BITS_AFTER_POINT; return *this; }
	bofixed& operator=(float f)    { mValue = (qint32)(nearbyintf(f * BITS_POW)); return *this; }
	bofixed& operator=(double f)   { mValue = (qint32)(nearbyintf(f * BITS_POW)); return *this; }

	inline operator float() const { return toFloat(); }

	// AB: providing dedicated casting operators for several types is _not_
	// a good idea. for example things like if (!x) where x is a bofixed
	// causes a compiler error, as g++ doesn't know what to cast to (since
	// unless we would provide a bool cast operator, too).
	// providing float only means that if (!x) does exactly what we would
	// expect.
	//
	// disadvantage: things like domElement.setAttribute("x", x) work
	// without changes - we store a float here. I believe it would be nicer
	// if we'd store bofixed as a rawInt() in xml files.

	inline bofixed operator-() const { bofixed f(*this); f.mValue *= -1; return f; }

	inline bofixed& operator+=(const bofixed& f) { mValue += f.mValue; return *this; }
	inline bofixed& operator+=(qint32 f)  { return operator+=(bofixed(f)); }
	inline bofixed& operator+=(quint32 f) { return operator+=(bofixed(f)); }
	inline bofixed& operator+=(qint16 f)  { return operator+=(bofixed(f)); }
	inline bofixed& operator+=(quint16 f) { return operator+=(bofixed(f)); }
	inline bofixed& operator+=(qint8 f)   { return operator+=(bofixed(f)); }
	inline bofixed& operator+=(quint8 f)  { return operator+=(bofixed(f)); }
	inline bofixed& operator+=(long f)   { return operator+=(bofixed(f)); }
	inline bofixed& operator+=(unsigned long f)  { return operator+=(bofixed(f)); }
	inline bofixed& operator+=(float f)    { return operator+=(bofixed(f)); }
	inline bofixed& operator+=(double f)   { return operator+=(bofixed(f)); }

	inline bofixed& operator-=(const bofixed& f) { mValue -= f.mValue; return *this; }
	inline bofixed& operator-=(qint32 f)  { return operator-=(bofixed(f)); }
	inline bofixed& operator-=(quint32 f) { return operator-=(bofixed(f)); }
	inline bofixed& operator-=(qint16 f)  { return operator-=(bofixed(f)); }
	inline bofixed& operator-=(quint16 f) { return operator-=(bofixed(f)); }
	inline bofixed& operator-=(qint8 f)   { return operator-=(bofixed(f)); }
	inline bofixed& operator-=(quint8 f)  { return operator-=(bofixed(f)); }
	inline bofixed& operator-=(long f)   { return operator-=(bofixed(f)); }
	inline bofixed& operator-=(unsigned long f)  { return operator-=(bofixed(f)); }
	inline bofixed& operator-=(float f)    { return operator-=(bofixed(f)); }
	inline bofixed& operator-=(double f)   { return operator-=(bofixed(f)); }

	inline bofixed& operator*=(const bofixed& f)
	{
		qint64 tmp = ((qint64)mValue) * f.mValue;
		mValue = (tmp >> BITS_AFTER_POINT);
		return *this;
	}
	inline bofixed& operator*=(qint32 f)  { return operator*=(bofixed(f)); }
	inline bofixed& operator*=(quint32 f) { return operator*=(bofixed(f)); }
	inline bofixed& operator*=(qint16 f)  { return operator*=(bofixed(f)); }
	inline bofixed& operator*=(quint16 f) { return operator*=(bofixed(f)); }
	inline bofixed& operator*=(qint8 f)   { return operator*=(bofixed(f)); }
	inline bofixed& operator*=(quint8 f)  { return operator*=(bofixed(f)); }
	inline bofixed& operator*=(long f)   { return operator*=(bofixed(f)); }
	inline bofixed& operator*=(unsigned long f)  { return operator*=(bofixed(f)); }
	inline bofixed& operator*=(float f)    { return operator*=(bofixed(f)); }
	inline bofixed& operator*=(double f)   { return operator*=(bofixed(f)); }

	inline bofixed& operator/=(const bofixed& f)
	{
		// AB: the result of mValue/f.mValue must be shifted left by
		// BITS_AFTER_POINT bits. however that would mean losing a lot
		// of precision, so instead we shift the value that we divide to
		// the right first.
		qint64 tmp = ((qint64)mValue) << BITS_AFTER_POINT;
		tmp /= f.mValue;
		mValue = tmp;
		return *this;
	}
	inline bofixed& operator/=(qint32 f)  { return operator/=(bofixed(f)); }
	inline bofixed& operator/=(quint32 f) { return operator/=(bofixed(f)); }
	inline bofixed& operator/=(qint16 f)  { return operator/=(bofixed(f)); }
	inline bofixed& operator/=(quint16 f) { return operator/=(bofixed(f)); }
	inline bofixed& operator/=(qint8 f)   { return operator/=(bofixed(f)); }
	inline bofixed& operator/=(quint8 f)  { return operator/=(bofixed(f)); }
	inline bofixed& operator/=(long f)   { return operator/=(bofixed(f)); }
	inline bofixed& operator/=(unsigned long f)  { return operator/=(bofixed(f)); }
	inline bofixed& operator/=(float f)    { return operator/=(bofixed(f)); }
	inline bofixed& operator/=(double f)   { return operator/=(bofixed(f)); }

private:
	qint32 mValue;
};
#else
class bofixed
{
public:
	/**
	 * Construct a fixed point variable with initial value 0.0
	 **/
	bofixed()
		: mValue(0.0f)
	{
	}
	bofixed(const bofixed& f)
		: mValue(f.mValue)
	{
	}
	bofixed(qint32 f)  : mValue(f) { }
	bofixed(quint32 f) : mValue(f) { }
	bofixed(qint8 f)   : mValue(f) { }
	bofixed(quint8 f)  : mValue(f) { }
	bofixed(qint16 f)  : mValue(f) { }
	bofixed(quint16 f) : mValue(f) { }
	bofixed(long f)   : mValue(f) { }
	bofixed(unsigned long f)  : mValue(f) { }
	bofixed(float f)    : mValue(f) { }
	bofixed(double f)   : mValue(f) { }

	inline bool equals(const bofixed& f) const         { return (mValue == f.mValue); }
	inline bool isGreater(const bofixed& f) const      { return (mValue >  f.mValue); }
	inline bool isGreaterEqual(const bofixed& f) const { return (mValue >= f.mValue); }
	inline bool isLess(const bofixed& f) const         { return (mValue <  f.mValue); }
	inline bool isLessEqual(const bofixed& f) const    { return (mValue <= f.mValue); }

	/**
	 * @return The fixed point value castet to integer. This behaves just
	 * like casting a floating point number to int, i.e. the digits after
	 * the decimal point are "cut off".
	 **/
	inline qint32 toInt() const { return (qint32)mValue; }
	inline float toFloat() const { return mValue; }
	inline double toDouble() const { return (double)mValue; }

#if 0
	/**
	 * @return The internal representation of the fixed point value. This
	 * may be useful for debugging-
	 **/
	qint32 rawInt() const { return *((quint32*)&mValue); }
	void setFromRawInt(qint32 r) { mValue = *((float*)&r); }
#endif

	bofixed& operator=(const bofixed& f) { mValue = f.mValue; return *this; }
	bofixed& operator=(qint32 f)  { mValue = f; return *this; }
	bofixed& operator=(quint32 f) { mValue = f; return *this; }
	bofixed& operator=(qint8 f)   { mValue = f; return *this; }
	bofixed& operator=(quint8 f)  { mValue = f; return *this; }
	bofixed& operator=(qint16 f)  { mValue = f; return *this; }
	bofixed& operator=(quint16 f) { mValue = f; return *this; }
	bofixed& operator=(long f)   { mValue = f; return *this; }
	bofixed& operator=(unsigned long f)  { mValue = f; return *this; }
	bofixed& operator=(float f)    { mValue = f; return *this; }
	bofixed& operator=(double f)   { mValue = f; return *this; }

	inline operator float() const { return toFloat(); }

	// AB: providing dedicated casting operators for several types is _not_
	// a good idea. for example things like if (!x) where x is a bofixed
	// causes a compiler error, as g++ doesn't know what to cast to (since
	// unless we would provide a bool cast operator, too).
	// providing float only means that if (!x) does exactly what we would
	// expect.
	//
	// disadvantage: things like domElement.setAttribute("x", x) work
	// without changes - we store a float here. I believe it would be nicer
	// if we'd store bofixed as a rawInt() in xml files.

	inline bofixed operator-() const { bofixed f(*this); f.mValue *= -1; return f; }

	inline bofixed& operator+=(const bofixed& f) { mValue += f.mValue; return *this; }
	inline bofixed& operator+=(qint32 f)  { return operator+=(bofixed(f)); }
	inline bofixed& operator+=(quint32 f) { return operator+=(bofixed(f)); }
	inline bofixed& operator+=(qint16 f)  { return operator+=(bofixed(f)); }
	inline bofixed& operator+=(quint16 f) { return operator+=(bofixed(f)); }
	inline bofixed& operator+=(qint8 f)   { return operator+=(bofixed(f)); }
	inline bofixed& operator+=(quint8 f)  { return operator+=(bofixed(f)); }
	inline bofixed& operator+=(long f)   { return operator+=(bofixed(f)); }
	inline bofixed& operator+=(unsigned long f)  { return operator+=(bofixed(f)); }
	inline bofixed& operator+=(float f)    { return operator+=(bofixed(f)); }
	inline bofixed& operator+=(double f)   { return operator+=(bofixed(f)); }

	inline bofixed& operator-=(const bofixed& f) { mValue -= f.mValue; return *this; }
	inline bofixed& operator-=(qint32 f)  { return operator-=(bofixed(f)); }
	inline bofixed& operator-=(quint32 f) { return operator-=(bofixed(f)); }
	inline bofixed& operator-=(qint16 f)  { return operator-=(bofixed(f)); }
	inline bofixed& operator-=(quint16 f) { return operator-=(bofixed(f)); }
	inline bofixed& operator-=(qint8 f)   { return operator-=(bofixed(f)); }
	inline bofixed& operator-=(quint8 f)  { return operator-=(bofixed(f)); }
	inline bofixed& operator-=(long f)   { return operator-=(bofixed(f)); }
	inline bofixed& operator-=(unsigned long f)  { return operator-=(bofixed(f)); }
	inline bofixed& operator-=(float f)    { return operator-=(bofixed(f)); }
	inline bofixed& operator-=(double f)   { return operator-=(bofixed(f)); }

	inline bofixed& operator*=(const bofixed& f)
	{
		mValue *= f.mValue;
		return *this;
	}
	inline bofixed& operator*=(qint32 f)  { return operator*=(bofixed(f)); }
	inline bofixed& operator*=(quint32 f) { return operator*=(bofixed(f)); }
	inline bofixed& operator*=(qint16 f)  { return operator*=(bofixed(f)); }
	inline bofixed& operator*=(quint16 f) { return operator*=(bofixed(f)); }
	inline bofixed& operator*=(qint8 f)   { return operator*=(bofixed(f)); }
	inline bofixed& operator*=(quint8 f)  { return operator*=(bofixed(f)); }
	inline bofixed& operator*=(long f)   { return operator*=(bofixed(f)); }
	inline bofixed& operator*=(unsigned long f)  { return operator*=(bofixed(f)); }
	inline bofixed& operator*=(float f)    { return operator*=(bofixed(f)); }
	inline bofixed& operator*=(double f)   { return operator*=(bofixed(f)); }

	inline bofixed& operator/=(const bofixed& f)
	{
		mValue /= f.mValue;
		return *this;
	}
	inline bofixed& operator/=(qint32 f)  { return operator/=(bofixed(f)); }
	inline bofixed& operator/=(quint32 f) { return operator/=(bofixed(f)); }
	inline bofixed& operator/=(qint16 f)  { return operator/=(bofixed(f)); }
	inline bofixed& operator/=(quint16 f) { return operator/=(bofixed(f)); }
	inline bofixed& operator/=(qint8 f)   { return operator/=(bofixed(f)); }
	inline bofixed& operator/=(quint8 f)  { return operator/=(bofixed(f)); }
	inline bofixed& operator/=(long f)   { return operator/=(bofixed(f)); }
	inline bofixed& operator/=(unsigned long f)  { return operator/=(bofixed(f)); }
	inline bofixed& operator/=(float f)    { return operator/=(bofixed(f)); }
	inline bofixed& operator/=(double f)   { return operator/=(bofixed(f)); }

private:
	float mValue;
};
#endif

#ifndef BOFIXED_IS_FLOAT
inline bofixed operator+(const bofixed& f1, const bofixed& f2) { bofixed f(f1); f += f2; return f; }
inline bofixed operator+(const bofixed& f1, qint32 f2)  { bofixed f(f1); f += f2; return f; }
inline bofixed operator+(const bofixed& f1, quint32 f2) { bofixed f(f1); f += f2; return f; }
inline bofixed operator+(const bofixed& f1, qint16 f2)  { bofixed f(f1); f += f2; return f; }
inline bofixed operator+(const bofixed& f1, quint16 f2) { bofixed f(f1); f += f2; return f; }
inline bofixed operator+(const bofixed& f1, qint8 f2)   { bofixed f(f1); f += f2; return f; }
inline bofixed operator+(const bofixed& f1, quint8 f2)  { bofixed f(f1); f += f2; return f; }
inline bofixed operator+(const bofixed& f1, long f2 )  { bofixed f(f1); f += f2; return f; }
inline bofixed operator+(const bofixed& f1, unsigned long f2)  { bofixed f(f1); f += f2; return f; }
inline bofixed operator+(const bofixed& f1, float f2)    { bofixed f(f1); f += f2; return f; }
inline bofixed operator+(const bofixed& f1, double f2)   { bofixed f(f1); f += f2; return f; }
inline bofixed operator+(qint32 f1, const bofixed& f2)  { bofixed f(f1); f += f2; return f; }
inline bofixed operator+(quint32 f1, const bofixed& f2) { bofixed f(f1); f += f2; return f; }
inline bofixed operator+(qint8 f1, const bofixed& f2)   { bofixed f(f1); f += f2; return f; }
inline bofixed operator+(quint8 f1, const bofixed& f2)  { bofixed f(f1); f += f2; return f; }
inline bofixed operator+(qint16 f1, const bofixed& f2)  { bofixed f(f1); f += f2; return f; }
inline bofixed operator+(quint16 f1, const bofixed& f2) { bofixed f(f1); f += f2; return f; }
inline bofixed operator+(long f1, const bofixed& f2)   { bofixed f(f1); f += f2; return f; }
inline bofixed operator+(unsigned long f1, const bofixed& f2)  { bofixed f(f1); f += f2; return f; }
inline bofixed operator+(float f1, const bofixed& f2)    { bofixed f(f1); f += f2; return f; }
inline bofixed operator+(double f1, const bofixed& f2)   { bofixed f(f1); f += f2; return f; }

inline bofixed operator-(const bofixed& f1, const bofixed& f2) { bofixed f(f1); f -= f2; return f; }
inline bofixed operator-(const bofixed& f1, qint32 f2)  { bofixed f(f1); f -= f2; return f; }
inline bofixed operator-(const bofixed& f1, quint32 f2) { bofixed f(f1); f -= f2; return f; }
inline bofixed operator-(const bofixed& f1, qint16 f2)  { bofixed f(f1); f -= f2; return f; }
inline bofixed operator-(const bofixed& f1, quint16 f2) { bofixed f(f1); f -= f2; return f; }
inline bofixed operator-(const bofixed& f1, qint8 f2)   { bofixed f(f1); f -= f2; return f; }
inline bofixed operator-(const bofixed& f1, quint8 f2)  { bofixed f(f1); f -= f2; return f; }
inline bofixed operator-(const bofixed& f1, long f2 )  { bofixed f(f1); f -= f2; return f; }
inline bofixed operator-(const bofixed& f1, unsigned long f2)  { bofixed f(f1); f -= f2; return f; }
inline bofixed operator-(const bofixed& f1, float f2)    { bofixed f(f1); f -= f2; return f; }
inline bofixed operator-(const bofixed& f1, double f2)   { bofixed f(f1); f -= f2; return f; }
inline bofixed operator-(qint32 f1, const bofixed& f2)  { bofixed f(f1); f -= f2; return f; }
inline bofixed operator-(quint32 f1, const bofixed& f2) { bofixed f(f1); f -= f2; return f; }
inline bofixed operator-(qint8 f1, const bofixed& f2)   { bofixed f(f1); f -= f2; return f; }
inline bofixed operator-(quint8 f1, const bofixed& f2)  { bofixed f(f1); f -= f2; return f; }
inline bofixed operator-(qint16 f1, const bofixed& f2)  { bofixed f(f1); f -= f2; return f; }
inline bofixed operator-(quint16 f1, const bofixed& f2) { bofixed f(f1); f -= f2; return f; }
inline bofixed operator-(long f1, const bofixed& f2)   { bofixed f(f1); f -= f2; return f; }
inline bofixed operator-(unsigned long f1, const bofixed& f2)  { bofixed f(f1); f -= f2; return f; }
inline bofixed operator-(float f1, const bofixed& f2)    { bofixed f(f1); f -= f2; return f; }
inline bofixed operator-(double f1, const bofixed& f2)   { bofixed f(f1); f -= f2; return f; }

inline bofixed operator*(const bofixed& f1, const bofixed& f2) { bofixed f(f1); f *= f2; return f; }
inline bofixed operator*(const bofixed& f1, qint32 f2)  { bofixed f(f1); f *= f2; return f; }
inline bofixed operator*(const bofixed& f1, quint32 f2) { bofixed f(f1); f *= f2; return f; }
inline bofixed operator*(const bofixed& f1, qint16 f2)  { bofixed f(f1); f *= f2; return f; }
inline bofixed operator*(const bofixed& f1, quint16 f2) { bofixed f(f1); f *= f2; return f; }
inline bofixed operator*(const bofixed& f1, qint8 f2)   { bofixed f(f1); f *= f2; return f; }
inline bofixed operator*(const bofixed& f1, quint8 f2)  { bofixed f(f1); f *= f2; return f; }
inline bofixed operator*(const bofixed& f1, long f2 )  { bofixed f(f1); f *= f2; return f; }
inline bofixed operator*(const bofixed& f1, unsigned long f2)  { bofixed f(f1); f *= f2; return f; }
inline bofixed operator*(const bofixed& f1, float f2)    { bofixed f(f1); f *= f2; return f; }
inline bofixed operator*(const bofixed& f1, double f2)   { bofixed f(f1); f *= f2; return f; }
inline bofixed operator*(qint32 f1, const bofixed& f2)  { bofixed f(f1); f *= f2; return f; }
inline bofixed operator*(quint32 f1, const bofixed& f2) { bofixed f(f1); f *= f2; return f; }
inline bofixed operator*(qint8 f1, const bofixed& f2)   { bofixed f(f1); f *= f2; return f; }
inline bofixed operator*(quint8 f1, const bofixed& f2)  { bofixed f(f1); f *= f2; return f; }
inline bofixed operator*(qint16 f1, const bofixed& f2)  { bofixed f(f1); f *= f2; return f; }
inline bofixed operator*(quint16 f1, const bofixed& f2) { bofixed f(f1); f *= f2; return f; }
inline bofixed operator*(long f1, const bofixed& f2)   { bofixed f(f1); f *= f2; return f; }
inline bofixed operator*(unsigned long f1, const bofixed& f2)  { bofixed f(f1); f *= f2; return f; }
inline bofixed operator*(float f1, const bofixed& f2)    { bofixed f(f1); f *= f2; return f; }
inline bofixed operator*(double f1, const bofixed& f2)   { bofixed f(f1); f *= f2; return f; }

inline bofixed operator/(const bofixed& f1, const bofixed& f2) { bofixed f(f1); f /= f2; return f; }
inline bofixed operator/(const bofixed& f1, qint32 f2)  { bofixed f(f1); f /= f2; return f; }
inline bofixed operator/(const bofixed& f1, quint32 f2) { bofixed f(f1); f /= f2; return f; }
inline bofixed operator/(const bofixed& f1, qint16 f2)  { bofixed f(f1); f /= f2; return f; }
inline bofixed operator/(const bofixed& f1, quint16 f2) { bofixed f(f1); f /= f2; return f; }
inline bofixed operator/(const bofixed& f1, qint8 f2)   { bofixed f(f1); f /= f2; return f; }
inline bofixed operator/(const bofixed& f1, quint8 f2)  { bofixed f(f1); f /= f2; return f; }
inline bofixed operator/(const bofixed& f1, long f2 )  { bofixed f(f1); f /= f2; return f; }
inline bofixed operator/(const bofixed& f1, unsigned long f2)  { bofixed f(f1); f /= f2; return f; }
inline bofixed operator/(const bofixed& f1, float f2)    { bofixed f(f1); f /= f2; return f; }
inline bofixed operator/(const bofixed& f1, double f2)   { bofixed f(f1); f /= f2; return f; }
inline bofixed operator/(qint32 f1, const bofixed& f2)  { bofixed f(f1); f /= f2; return f; }
inline bofixed operator/(quint32 f1, const bofixed& f2) { bofixed f(f1); f /= f2; return f; }
inline bofixed operator/(qint8 f1, const bofixed& f2)   { bofixed f(f1); f /= f2; return f; }
inline bofixed operator/(quint8 f1, const bofixed& f2)  { bofixed f(f1); f /= f2; return f; }
inline bofixed operator/(qint16 f1, const bofixed& f2)  { bofixed f(f1); f /= f2; return f; }
inline bofixed operator/(quint16 f1, const bofixed& f2) { bofixed f(f1); f /= f2; return f; }
inline bofixed operator/(long f1, const bofixed& f2)   { bofixed f(f1); f /= f2; return f; }
inline bofixed operator/(unsigned long f1, const bofixed& f2)  { bofixed f(f1); f /= f2; return f; }
inline bofixed operator/(float f1, const bofixed& f2)    { bofixed f(f1); f /= f2; return f; }
inline bofixed operator/(double f1, const bofixed& f2)   { bofixed f(f1); f /= f2; return f; }

inline bool operator==(const bofixed& f1, const bofixed& f2) { return f1.equals(f2); }
inline bool operator==(const bofixed& f1, qint32 f2)  { return f1.equals(bofixed(f2)); }
inline bool operator==(const bofixed& f1, quint32 f2) { return f1.equals(bofixed(f2)); }
inline bool operator==(const bofixed& f1, qint16 f2)  { return f1.equals(bofixed(f2)); }
inline bool operator==(const bofixed& f1, quint16 f2) { return f1.equals(bofixed(f2)); }
inline bool operator==(const bofixed& f1, qint8 f2)   { return f1.equals(bofixed(f2)); }
inline bool operator==(const bofixed& f1, quint8 f2)  { return f1.equals(bofixed(f2)); }
inline bool operator==(const bofixed& f1, long f2)   { return f1.equals(bofixed(f2)); }
inline bool operator==(const bofixed& f1, unsigned long f2)  { return f1.equals(bofixed(f2)); }
inline bool operator==(const bofixed& f1, float f2)    { return f1.equals(bofixed(f2)); }
inline bool operator==(const bofixed& f1, double f2)   { return f1.equals(bofixed(f2)); }
inline bool operator==(qint32 f1, const bofixed& f2)  { return bofixed(f1).equals(f2); }
inline bool operator==(quint32 f1, const bofixed& f2) { return bofixed(f1).equals(f2); }
inline bool operator==(qint16 f1, const bofixed& f2)  { return bofixed(f1).equals(f2); }
inline bool operator==(quint16 f1, const bofixed& f2) { return bofixed(f1).equals(f2); }
inline bool operator==(qint8 f1, const bofixed& f2)   { return bofixed(f1).equals(f2); }
inline bool operator==(quint8 f1, const bofixed& f2)  { return bofixed(f1).equals(f2); }
inline bool operator==(long f1, const bofixed& f2)   { return bofixed(f1).equals(f2); }
inline bool operator==(unsigned long f1, const bofixed& f2)  { return bofixed(f1).equals(f2); }
inline bool operator==(float f1, const bofixed& f2)    { return bofixed(f1).equals(f2); }
inline bool operator==(double f1, const bofixed& f2)   { return bofixed(f1).equals(f2); }

inline bool operator!=(const bofixed& f1, const bofixed& f2) { return !f1.equals(f2); }
inline bool operator!=(const bofixed& f1, qint32 f2)  { return !f1.equals(bofixed(f2)); }
inline bool operator!=(const bofixed& f1, quint32 f2) { return !f1.equals(bofixed(f2)); }
inline bool operator!=(const bofixed& f1, qint16 f2)  { return !f1.equals(bofixed(f2)); }
inline bool operator!=(const bofixed& f1, quint16 f2) { return !f1.equals(bofixed(f2)); }
inline bool operator!=(const bofixed& f1, qint8 f2)   { return !f1.equals(bofixed(f2)); }
inline bool operator!=(const bofixed& f1, quint8 f2)  { return !f1.equals(bofixed(f2)); }
inline bool operator!=(const bofixed& f1, long f2)   { return !f1.equals(bofixed(f2)); }
inline bool operator!=(const bofixed& f1, unsigned long f2)  { return !f1.equals(bofixed(f2)); }
inline bool operator!=(const bofixed& f1, float f2)    { return !f1.equals(bofixed(f2)); }
inline bool operator!=(const bofixed& f1, double f2)   { return !f1.equals(bofixed(f2)); }
inline bool operator!=(qint32 f1, const bofixed& f2)  { return !bofixed(f1).equals(f2); }
#if 0 // conflicts with QVariant!
inline bool operator!=(quint32 f1, const bofixed& f2) { return !bofixed(f1).equals(f2); }
#endif
inline bool operator!=(qint16 f1, const bofixed& f2)  { return !bofixed(f1).equals(f2); }
inline bool operator!=(quint16 f1, const bofixed& f2) { return !bofixed(f1).equals(f2); }
inline bool operator!=(qint8 f1, const bofixed& f2)   { return !bofixed(f1).equals(f2); }
inline bool operator!=(quint8 f1, const bofixed& f2)  { return !bofixed(f1).equals(f2); }
inline bool operator!=(long f1, const bofixed& f2)   { return !bofixed(f1).equals(f2); }
inline bool operator!=(unsigned long f1, const bofixed& f2)  { return !bofixed(f1).equals(f2); }
inline bool operator!=(float f1, const bofixed& f2)    { return !bofixed(f1).equals(f2); }
inline bool operator!=(double f1, const bofixed& f2)   { return !bofixed(f1).equals(f2); }

inline bool operator<=(const bofixed& f1, const bofixed& f2) { return f1.isLessEqual(f2); }
inline bool operator<=(const bofixed& f1, qint32 f2)  { return f1.isLessEqual(bofixed(f2)); }
inline bool operator<=(const bofixed& f1, quint32 f2) { return f1.isLessEqual(bofixed(f2)); }
inline bool operator<=(const bofixed& f1, qint16 f2)  { return f1.isLessEqual(bofixed(f2)); }
inline bool operator<=(const bofixed& f1, quint16 f2) { return f1.isLessEqual(bofixed(f2)); }
inline bool operator<=(const bofixed& f1, qint8 f2)   { return f1.isLessEqual(bofixed(f2)); }
inline bool operator<=(const bofixed& f1, quint8 f2)  { return f1.isLessEqual(bofixed(f2)); }
inline bool operator<=(const bofixed& f1, long f2)   { return f1.isLessEqual(bofixed(f2)); }
inline bool operator<=(const bofixed& f1, unsigned long f2)  { return f1.isLessEqual(bofixed(f2)); }
inline bool operator<=(const bofixed& f1, float f2)    { return f1.isLessEqual(bofixed(f2)); }
inline bool operator<=(const bofixed& f1, double f2)   { return f1.isLessEqual(bofixed(f2)); }
inline bool operator<=(qint32 f1, const bofixed& f2)  { return bofixed(f1).isLessEqual(f2); }
inline bool operator<=(quint32 f1, const bofixed& f2) { return bofixed(f1).isLessEqual(f2); }
inline bool operator<=(qint16 f1, const bofixed& f2)  { return bofixed(f1).isLessEqual(f2); }
inline bool operator<=(quint16 f1, const bofixed& f2) { return bofixed(f1).isLessEqual(f2); }
inline bool operator<=(qint8 f1, const bofixed& f2)   { return bofixed(f1).isLessEqual(f2); }
inline bool operator<=(quint8 f1, const bofixed& f2)  { return bofixed(f1).isLessEqual(f2); }
inline bool operator<=(long f1, const bofixed& f2)   { return bofixed(f1).isLessEqual(f2); }
inline bool operator<=(unsigned long f1, const bofixed& f2)  { return bofixed(f1).isLessEqual(f2); }
inline bool operator<=(float f1, const bofixed& f2)    { return bofixed(f1).isLessEqual(f2); }
inline bool operator<=(double f1, const bofixed& f2)   { return bofixed(f1).isLessEqual(f2); }

inline bool operator>=(const bofixed& f1, const bofixed& f2) { return f1.isGreaterEqual(f2); }
inline bool operator>=(const bofixed& f1, qint32 f2)  { return f1.isGreaterEqual(bofixed(f2)); }
inline bool operator>=(const bofixed& f1, quint32 f2) { return f1.isGreaterEqual(bofixed(f2)); }
inline bool operator>=(const bofixed& f1, qint16 f2)  { return f1.isGreaterEqual(bofixed(f2)); }
inline bool operator>=(const bofixed& f1, quint16 f2) { return f1.isGreaterEqual(bofixed(f2)); }
inline bool operator>=(const bofixed& f1, qint8 f2)   { return f1.isGreaterEqual(bofixed(f2)); }
inline bool operator>=(const bofixed& f1, quint8 f2)  { return f1.isGreaterEqual(bofixed(f2)); }
inline bool operator>=(const bofixed& f1, long f2)   { return f1.isGreaterEqual(bofixed(f2)); }
inline bool operator>=(const bofixed& f1, unsigned long f2)  { return f1.isGreaterEqual(bofixed(f2)); }
inline bool operator>=(const bofixed& f1, float f2)    { return f1.isGreaterEqual(bofixed(f2)); }
inline bool operator>=(const bofixed& f1, double f2)   { return f1.isGreaterEqual(bofixed(f2)); }
inline bool operator>=(qint32 f1, const bofixed& f2)  { return bofixed(f1).isGreaterEqual(f2); }
inline bool operator>=(quint32 f1, const bofixed& f2) { return bofixed(f1).isGreaterEqual(f2); }
inline bool operator>=(qint16 f1, const bofixed& f2)  { return bofixed(f1).isGreaterEqual(f2); }
inline bool operator>=(quint16 f1, const bofixed& f2) { return bofixed(f1).isGreaterEqual(f2); }
inline bool operator>=(qint8 f1, const bofixed& f2)   { return bofixed(f1).isGreaterEqual(f2); }
inline bool operator>=(quint8 f1, const bofixed& f2)  { return bofixed(f1).isGreaterEqual(f2); }
inline bool operator>=(long f1, const bofixed& f2)   { return bofixed(f1).isGreaterEqual(f2); }
inline bool operator>=(unsigned long f1, const bofixed& f2)  { return bofixed(f1).isGreaterEqual(f2); }
inline bool operator>=(float f1, const bofixed& f2)    { return bofixed(f1).isGreaterEqual(f2); }
inline bool operator>=(double f1, const bofixed& f2)   { return bofixed(f1).isGreaterEqual(f2); }

inline bool operator<(const bofixed& f1, const bofixed& f2) { return f1.isLess(f2); }
inline bool operator<(const bofixed& f1, qint32 f2)  { return f1.isLess(bofixed(f2)); }
inline bool operator<(const bofixed& f1, quint32 f2) { return f1.isLess(bofixed(f2)); }
inline bool operator<(const bofixed& f1, qint16 f2)  { return f1.isLess(bofixed(f2)); }
inline bool operator<(const bofixed& f1, quint16 f2) { return f1.isLess(bofixed(f2)); }
inline bool operator<(const bofixed& f1, qint8 f2)   { return f1.isLess(bofixed(f2)); }
inline bool operator<(const bofixed& f1, quint8 f2)  { return f1.isLess(bofixed(f2)); }
inline bool operator<(const bofixed& f1, long f2)   { return f1.isLess(bofixed(f2)); }
inline bool operator<(const bofixed& f1, unsigned long f2)  { return f1.isLess(bofixed(f2)); }
inline bool operator<(const bofixed& f1, float f2)    { return f1.isLess(bofixed(f2)); }
inline bool operator<(const bofixed& f1, double f2)   { return f1.isLess(bofixed(f2)); }
inline bool operator<(qint32 f1, const bofixed& f2)  { return bofixed(f1).isLess(f2); }
inline bool operator<(quint32 f1, const bofixed& f2) { return bofixed(f1).isLess(f2); }
inline bool operator<(qint16 f1, const bofixed& f2)  { return bofixed(f1).isLess(f2); }
inline bool operator<(quint16 f1, const bofixed& f2) { return bofixed(f1).isLess(f2); }
inline bool operator<(qint8 f1, const bofixed& f2)   { return bofixed(f1).isLess(f2); }
inline bool operator<(quint8 f1, const bofixed& f2)  { return bofixed(f1).isLess(f2); }
inline bool operator<(long f1, const bofixed& f2)   { return bofixed(f1).isLess(f2); }
inline bool operator<(unsigned long f1, const bofixed& f2)  { return bofixed(f1).isLess(f2); }
inline bool operator<(float f1, const bofixed& f2)    { return bofixed(f1).isLess(f2); }
inline bool operator<(double f1, const bofixed& f2)   { return bofixed(f1).isLess(f2); }

inline bool operator>(const bofixed& f1, const bofixed& f2) { return f1.isGreater(f2); }
inline bool operator>(const bofixed& f1, qint32 f2)  { return f1.isGreater(bofixed(f2)); }
inline bool operator>(const bofixed& f1, quint32 f2) { return f1.isGreater(bofixed(f2)); }
inline bool operator>(const bofixed& f1, qint16 f2)  { return f1.isGreater(bofixed(f2)); }
inline bool operator>(const bofixed& f1, quint16 f2) { return f1.isGreater(bofixed(f2)); }
inline bool operator>(const bofixed& f1, qint8 f2)   { return f1.isGreater(bofixed(f2)); }
inline bool operator>(const bofixed& f1, quint8 f2)  { return f1.isGreater(bofixed(f2)); }
inline bool operator>(const bofixed& f1, long f2)   { return f1.isGreater(bofixed(f2)); }
inline bool operator>(const bofixed& f1, unsigned long f2)  { return f1.isGreater(bofixed(f2)); }
inline bool operator>(const bofixed& f1, float f2)    { return f1.isGreater(bofixed(f2)); }
inline bool operator>(const bofixed& f1, double f2)   { return f1.isGreater(bofixed(f2)); }
inline bool operator>(qint32 f1, const bofixed& f2)  { return bofixed(f1).isGreater(f2); }
inline bool operator>(quint32 f1, const bofixed& f2) { return bofixed(f1).isGreater(f2); }
inline bool operator>(qint16 f1, const bofixed& f2)  { return bofixed(f1).isGreater(f2); }
inline bool operator>(quint16 f1, const bofixed& f2) { return bofixed(f1).isGreater(f2); }
inline bool operator>(qint8 f1, const bofixed& f2)   { return bofixed(f1).isGreater(f2); }
inline bool operator>(quint8 f1, const bofixed& f2)  { return bofixed(f1).isGreater(f2); }
inline bool operator>(long f1, const bofixed& f2)   { return bofixed(f1).isGreater(f2); }
inline bool operator>(unsigned long f1, const bofixed& f2)  { return bofixed(f1).isGreater(f2); }
inline bool operator>(float f1, const bofixed& f2)    { return bofixed(f1).isGreater(f2); }
inline bool operator>(double f1, const bofixed& f2)   { return bofixed(f1).isGreater(f2); }
#endif // BOFIXED_IS_FLOAT

#endif

