/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/font/ufontmetrics.hpp
    begin             : Tue Oct 2 2001
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

#ifndef UFONTMETRICS_HPP
#define UFONTMETRICS_HPP

#include "../uobject.hpp"

namespace ufo {

class UFontRenderer;

/** @short Describes metrics of a font.
  * @ingroup text
  * @ingroup drawing
  *
  * This class describes the metrices of single characters as well as
  * character strings.
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UFontMetrics : public UObject {
	UFO_DECLARE_ABSTRACT_CLASS(UFontMetrics)
public:
	virtual UFontRenderer * getFontRenderer() const = 0;
	/** returns the space between the base line and the top of
	  * the highest character
	  */
	virtual int getAscent() const = 0;
	/** returns the space between the base line and the bottom of
	  * the highest character
	  */
	virtual int getDescent() const = 0;

	/** returns the space in pixels between the descent of
	  * one line and the ascent of the next line
	  */
	virtual int getLineskip() const = 0;
	/** Returns the total height of text.
	  * This means ascent + descent.
	  */
	virtual int getHeight() const = 0;

	virtual int getMaxAscent() const = 0;
	virtual int getMaxDescent() const = 0;
	virtual int getMaxCharWidth() const = 0;

	/** returns the position of the underline relative to the base line. */
	virtual int getUnderlinePosition() const = 0;
	/** returns the thickness of the underline. */
	virtual int getUnderlineThickness() const = 0;

	/** returns the width for the given text.
	  * Calls internally getStringWidth(const char*, unsigned int);
	  * @return getStringWidth
	  */
	virtual int getStringWidth(const std::string & text) const {
		return getStringWidth(text.data(), text.length());
	}
	/** returns the width for the given text and the specified length.
	  * @return the minimal width needed to paint this text
	  */
	virtual int getStringWidth(const char * text, unsigned int nChar) const = 0;

	/** Returns the width of the given character. */
	virtual int getCharWidth(const wchar_t chA) const = 0;

	/** @see viewToModel(const char * text, unsigned int nChar, unsigned int w)
	  */
	virtual unsigned int
	viewToModel(const std::string & text, unsigned int w) const {
		return viewToModel(text.data(), text.length(), w);
	}
	/** This method converts the view matrix to the model index,
	  * i.e. returns index of that character which will be drawn at
	  * the specified x position.
	  *
	  * @param text The text which should be drawn.
	  * @param nChar The maximal amount of characters
	  * @param w The x position (in pixels) of the searched character
	  * @return The index of the character
	  */
	virtual unsigned int
	viewToModel(const char * text, unsigned int nChar, unsigned int w) const = 0;
};

} // namespace ufo

#endif // UFONTMETRICS_HPP
