/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/util/ucolor.cpp
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

#include "ufo/util/ucolor.hpp"

#include <algorithm>

using namespace ufo;

//UFO_IMPLEMENT_DEFAULT_DYNAMIC_CLASS(UColor, "")

const UColor UColor::red(1.0f, 0.0f, 0.0f);
const UColor UColor::green(0.0f, 1.0f, 0.0f);
const UColor UColor::blue(0.0f, 0.0f, 1.0f);

const UColor UColor::darkRed(0.5f, 0.0f, 0.0f);
const UColor UColor::darkGreen(0.0f, 0.5f, 0.0f);
const UColor UColor::darkBlue(0.0f, 0.0f, 0.5f);

const UColor UColor::white(1.0f, 1.0f, 1.0f);
const UColor UColor::black(0.0f, 0.0f, 0.0f);

const UColor UColor::gray(0.5f, 0.5f, 0.5f);
const UColor UColor::darkGray(0.25f, 0.25f, 0.25f);
const UColor UColor::lightGray(0.75f, 0.75f, 0.75f);

const UColor UColor::cyan(0.0f, 1.0f, 1.0f);
const UColor UColor::magenta(1.0f, 0.0f, 1.0f);
const UColor UColor::yellow(1.0f, 1.0f, 0.0f);

const UColor UColor::darkCyan(0.0f, 0.5f, 0.5f);
const UColor UColor::darkMagenta(0.5f, 0.0f, 0.5f);
const UColor UColor::darkYellow(0.5f, 0.5f, 0.0f);

//const UColor * UColor::orange = new UColor(1.0f, 0.75f, 0.0f);
//const UColor * UColor::pink = new UColor(1.0f, 0.75f, 0.75f);

const float UColor::FACTOR = 0.7f;
const float UColor::MIN_VAL = 0.01f;


// assuming num values in array
inline void fillFloat(const uint8_t * src, float * dst, int num = 4) {
	for (int i = 0; i < num; ++i) {
		dst[i] = src[i] / 255.f;
	}
}
inline void fillFloat(const int * src, float * dst, int num = 4) {
	for (int i = 0; i < num; ++i) {
		dst[i] = (src[i] & 0xff) / 255.f;
	}
}
inline void fillFloat(const double * src, float * dst, int num = 4) {
	for (int i = 0; i < num; ++i) {
		dst[i] = float(src[i]);
	}
}
inline void fillFloat(const float * src, float * dst, int num = 4) {
	for (int i = 0; i < num; ++i) {
		dst[i] = src[i];
	}
}

void readHexString(std::istream & stream, float * dst) {
	// ignore '#'
	stream.ignore();
	int color = 0;
	stream >> std::hex >> color;

	// FIXME
	// RGB or BGR ?
	dst[2] = (color & 0xff) / 255.0f;
	dst[1] = ((color >> 8) & 0xff) / 255.0f;
	dst[0] = ((color >> 16) & 0xff) / 255.0f;
	dst[3] = 1.0f;
}

void readDecString(std::istream & stream, float * dst) {
	int temp = 0;
	for (int i = 0; i < 4 && stream.good(); ++i) {
		stream >> temp;
		dst[i] = temp / 255.0f;

		char ch = stream.peek();
		while (stream.good() && (ch < '0' || ch > '9')) {
			stream.ignore();
			ch = stream.peek();
		}
	}
}

UColor::UColor() {
	m_farr[0] = 0.0f;
	m_farr[1] = 0.0f;
	m_farr[2] = 0.0f;
	m_farr[3] = 1.0f;
}

UColor::UColor(const UColor & col) {
	m_farr[0] = col.m_farr[0];
	m_farr[1] = col.m_farr[1];
	m_farr[2] = col.m_farr[2];
	m_farr[3] = col.m_farr[3];
}

UColor::UColor(const std::string & colorString) {
	// init all with zeros
	m_farr[0] = 0.0f;
	m_farr[1] = 0.0f;
	m_farr[2] = 0.0f;
	m_farr[3] = 1.0f;
	if (colorString != "") {
		// hex string
		if (colorString[0] == '#') {
			UIStringStream stream(colorString);
			readHexString(stream, m_farr);
		} else {
		// suppose integer string
		UIStringStream stream(colorString);
		readDecString(stream, m_farr);
		}
	}
}

//
// uint8_t constructors
//

UColor::UColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	m_farr[0] = r / 255.f;
	m_farr[1] = g / 255.f;
	m_farr[2] = b / 255.f;
	m_farr[3] = a / 255.f;
}

UColor::UColor(const uint8_t rgba[], bool hasAlpha)
{
	if (hasAlpha) {
		fillFloat(rgba, m_farr, 4);
	} else {
		fillFloat(rgba, m_farr, 3);
		m_farr[3] = 1.0f;
	}
}

//
// int constructors
//

UColor::UColor(int r, int g, int b, int a)
{
	m_farr[0] = (r & 0xff) / 255.f;
	m_farr[1] = (g & 0xff) / 255.f;
	m_farr[2] = (b & 0xff) / 255.f;
	m_farr[3] = (a & 0xff) / 255.f;
}

UColor::UColor(const int rgba[], bool hasAlpha)
{
	if (hasAlpha) {
		fillFloat(rgba, m_farr, 4);
	} else {
		fillFloat(rgba, m_farr, 3);
		m_farr[3] = 1.0f;
	}
}

//
// float constructors
//

UColor::UColor(float r, float g, float b, float a)
{
	m_farr[0] = r;
	m_farr[1] = g;
	m_farr[2] = b;
	m_farr[3] = a;
}
UColor::UColor(const float rgba[], bool hasAlpha)
{
	if (hasAlpha) {
		fillFloat(rgba, m_farr, 4);
	} else {
		fillFloat(rgba, m_farr, 3);
		m_farr[3] = 1.0f;
	}
}

//
// double constructors
//

UColor::UColor(double r, double g, double b, double a)
{
	m_farr[0] = float(r);
	m_farr[1] = float(g);
	m_farr[2] = float(b);
	m_farr[3] = float(a);
}


UColor::UColor(const double rgba[], bool hasAlpha)
{
	if (hasAlpha) {
		fillFloat(rgba, m_farr, 4);
	} else {
		fillFloat(rgba, m_farr, 3);
		m_farr[3] = 1.0f;
	}
}

UColor
UColor::darker() const {
	float temp[4];
	temp[0] = m_farr[0] * FACTOR;
	temp[1] = m_farr[1] * FACTOR;
	temp[2] = m_farr[2] * FACTOR;
	temp[3] = m_farr[3];

	// clamp
	for (int i = 0;i < 3; i++) {
		if (temp[i] < MIN_VAL) {
			temp[i] = 0.f;
		}
	}
	return UColor(temp, true);
}

UColor
UColor::brighter() const {
	// if is entirely black don't calculate other minima
	if (isBlack()) {
		return UColor(MIN_VAL, MIN_VAL, MIN_VAL, getAlpha());
	} else {
		// create temporary values, we do not want to modify this color
		float temp[4];
		temp[0] = m_farr[0] / FACTOR;
		temp[1] = m_farr[1] / FACTOR;
		temp[2] = m_farr[2] / FACTOR;
		temp[3] = m_farr[3];

		// clamp
		for (int i = 0;i < 3; i++) {
			temp[i] = std::max(MIN_VAL, temp[i]);
			temp[i] = std::min(temp[i], 1.0f);
		}
		return UColor(temp, true);
	}
}

//
// class UColorObject
//

UFO_IMPLEMENT_DEFAULT_DYNAMIC_CLASS(UColorObject, UColor)

unsigned int
UColorObject::hashCode() const {
	return uint8_t(getRed() * 255) << 24 || uint8_t(getGreen() * 255) << 16 ||
			uint8_t(getBlue() * 255) << 8 || uint8_t(getAlpha() * 255);
}


bool
UColorObject::equals(const UObject * obj) {
	if (const UColorObject * c = dynamic_cast<const UColorObject*>(obj)) {
		return *c == *obj;
	}
	return false;
}

bool
UColorObject::equals(const UColorObject * obj) {
	return *this == static_cast<const UColor>(*obj);
}

UObject *
UColorObject::clone() const {
	return new UColorObject(getFloat());
}

std::ostream &
UColorObject::paramString(std::ostream & os) const {
	const float * farr = getFloat();
	return os << farr[0]
		<< "," << farr[1]
		<< "," << farr[2]
		<< "," << farr[3];
}
