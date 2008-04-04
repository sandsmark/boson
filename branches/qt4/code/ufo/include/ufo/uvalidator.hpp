/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/uvalidator.hpp
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA *
 ***************************************************************************/

#ifndef UVALIDATOR_HPP
#define UVALIDATOR_HPP

#include "uobject.hpp"

namespace ufo {

/** An abstract class for text validation.
  *
  * @author Johannes Schmidt
  */
class UFO_EXPORT UValidator : public UObject {
	UFO_DECLARE_ABSTRACT_CLASS(UValidator)
public: // Public types
	enum State {
		Invalid = 0,
		Intermediate,
		Acceptable
	};
public:
	virtual void fixup(std::string * text) const = 0;
	virtual State validate(std::string * text, int * pos) const = 0;
};

/** A validator for integer text.
  *
  * @author Johannes Schmidt
  */
class UFO_EXPORT UIntValidator : public UValidator {
	UFO_DECLARE_DYNAMIC_CLASS(UIntValidator)
public:
	/** Creates an int validator which accepts all integers. */
	UIntValidator();
	/** Creates an int validator with min max bounds. */
	UIntValidator(int min, int max);
public: // Implements UValidator
	virtual void fixup(std::string * text) const;
	virtual State validate(std::string * text, int * pos) const;
public:
	void setRange(int min, int max);
	int getMinimum() const;
	int getMaximum() const;
private:
	int m_min;
	int m_max;
};

/** A validator for floating point text.
  *
  * @author Johannes Schmidt
  */
class UFO_EXPORT UDoubleValidator : public UValidator {
	UFO_DECLARE_DYNAMIC_CLASS(UDoubleValidator)
public:
	/** Creates an double validator which accepts all integers. */
	UDoubleValidator();
	/** Creates an double validator with min max bounds. */
	UDoubleValidator(double min, double max, int decimals);
public: // Implements UValidator
	virtual void fixup(std::string * text) const;
	virtual State validate(std::string * text, int * pos) const;
public:
	void setRange(double min, double max);
	double getMinimum() const;
	double getMaximum() const;
	/** Sets the number of digits behind the decimal point. */
	void setDecimals(int decimals);
	int getDecimals() const;
private:
	double m_min;
	double m_max;
	int m_decimals;
};

} // namespace ufo

#endif // UVALIDATOR_HPP
