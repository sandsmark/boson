/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/text/utextlayout.hpp
    begin             : Sun Mar 20 2005
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

#ifndef UTEXTLAYOUT_HPP
#define UTEXTLAYOUT_HPP

#include "../uobject.hpp"

#include "../font/ufont.hpp"
#include "../util/urectangle.hpp"

namespace ufo {

class UGraphics;

class UFO_EXPORT UTextLine {
public:
	UTextLine() : start(0), length(0), height(0), pos(), valid(false) {}
	UTextLine(unsigned int offset, unsigned int length, int h, const UPoint & p)
		: start(offset), length(length), height(h), pos(p), valid(true) {}
	unsigned int getOffset() const { return start; }
	unsigned int getLength() const { return length; }
	int getHeight() const { return height; }
	UPoint getPos() const { return pos; }
	bool isValid() const { return valid; }
public:
	unsigned int start;
	unsigned int length;
	int height;
	UPoint pos;
	bool valid;
};

/** @short Lays out one block of text which has one unique font and color.
  * @ingroup text
  *
  * @author Johannes Schmidt
  */
class UFO_EXPORT UTextLayout : public UObject {
	UFO_DECLARE_DYNAMIC_CLASS(UTextLayout)
public:
	UTextLayout();
	void setFont(const UFont & font);
	UFont getFont() const;

	/** Sets a pointer to the char array of the text. */
	void setText(const char * text, unsigned int length);
	/** @return A pointer to the text array. */
	const char * getText();
	unsigned int getLength();

	void setMaximumSize(const UDimension & dim);
	UDimension getPreferredSize(const UDimension & maxSize);

	void render(UGraphics * g, const URectangle & rect, const UPoint & offset = UPoint());
	/** @return The rectangle of the letter at the given offset in the coordinate
	  *  system of the rendered text.
	  */
	URectangle modelToView(int offset);
	/** @return The offset of the letter at the given position in the coordinate
	  *  system of the rendered text.
	  */
	int viewToModel(const UPoint & pos);

	void layout();

	int getLineCount();
	UTextLine getLine(int i);
	UTextLine getLineForTextPosition(int i);
protected:
	void invalidate();
private:
	const char * m_text;
	unsigned int m_length;
	UFont m_font;
	std::vector<UTextLine> m_lines;
	UDimension m_maxSize;
	bool m_validLayout;
};

} // namespace ufo

#endif // UTEXTLAYOUT_HPP
