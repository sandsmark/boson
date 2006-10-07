/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/uvalidator.cpp
    begin             : Tue Apr 5 2005
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

#include "ufo/uvalidator.hpp"

#include "ufo/util/ustring.hpp"
#include "ufo/util/uinteger.hpp"

// for int_max
#include <climits>

// for huge_val
#include <cmath>

using namespace ufo;

UFO_IMPLEMENT_ABSTRACT_CLASS(UValidator, UObject)
UFO_IMPLEMENT_CLASS(UIntValidator, UValidator)
UFO_IMPLEMENT_CLASS(UDoubleValidator, UValidator)

UIntValidator::UIntValidator()
	: m_min(INT_MIN)
	, m_max(INT_MAX)
{}

UIntValidator::UIntValidator(int min, int max)
	: m_min(min)
	, m_max(max)
{}

void
UIntValidator::fixup(std::string * text) const {
	if (!text) {
		return;
	}
	int value = UInteger::toInt(*text);
	*text = UString::toString(value);
}

UValidator::State
UIntValidator::validate(std::string * text, int * pos) const {
	if (!text) {
		return Invalid;
	}
	if (text->find(' ') != std::string::npos) {
		return Invalid;
	}
	bool ok = false;
	int value = UInteger::toInt(*text, &ok);
	if (!ok) {
		return Invalid;
	}
	if (value < m_min || value > m_max) {
		return Intermediate;
	}
	return Acceptable;
}

void
UIntValidator::setRange(int min, int max) {
	m_min = min;
	m_max = max;
}

int
UIntValidator::getMinimum() const {
	return m_min;
}

int
UIntValidator::getMaximum() const {
	return m_max;
}


//
// double validator
//

UDoubleValidator::UDoubleValidator()
	: m_min(-HUGE_VAL)
	, m_max(HUGE_VAL)
	, m_decimals(-1)
{}

UDoubleValidator::UDoubleValidator(double min, double max, int decimals)
	: m_min(min)
	, m_max(max)
	, m_decimals(decimals)
{}

void
UDoubleValidator::fixup(std::string * text) const {
	if (!text) {
		return;
	}
	UIStringStream istream(*text);
	//istream.precision(8);

	double value = 0;
	istream >> value;

	UOStringStream ostream;
	ostream.precision(8);
	if (m_decimals != -1) {
		// FIXME: this is simply wrong
		ostream.setf(std::ios_base::fixed, std::ios_base::floatfield);
		ostream.precision(m_decimals);
	}
	ostream << value;

	*text = ostream.str();
	//*text = UString::toString(value);
/*
	// remove digits after decimal point
	unsigned int i = text->find('.');
	if (i < std::string::npos) {
		if (i + 1 + m_decimals < text->length()) {
			*text = text->substr(0, i + 1 + m_decimals);
		}
	}
	*/
}

UValidator::State
UDoubleValidator::validate(std::string * text, int * pos) const {
	if (!text) {
		return Invalid;
	}
	if (text->find(' ') != std::string::npos) {
		return Invalid;
	}

	UIStringStream stream(*text);

	double value = 0;
	stream >> value;

	// FIXME: what about eof and not good?
	if (!stream.good() && !stream.eof()) {
		return Invalid;
	}
	// count digits after decimal point
	std::string::size_type i = text->find('.');
	if (i < std::string::npos) {
		i++;
		unsigned int index = i;
		while (index < text->length() && isdigit((*text)[index])) {
			index++;
		}

		if (int(index - i) > m_decimals) {
			return Intermediate;
		}
	}
	if (value < m_min || value > m_max) {
		return Intermediate;
	}
	return Acceptable;
}

void
UDoubleValidator::setRange(double min, double max) {
	m_min = min;
	m_max = max;
}

double
UDoubleValidator::getMinimum() const {
	return m_min;
}

double
UDoubleValidator::getMaximum() const {
	return m_max;
}

void
UDoubleValidator::setDecimals(int decimals) {
	m_decimals = decimals;
}

int
UDoubleValidator::getDecimals() const {
	return m_decimals;
}
