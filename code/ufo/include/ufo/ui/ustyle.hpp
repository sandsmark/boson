/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/ui/ustyle.hpp
    begin             : Sat Nov 29 2003
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


#ifndef USTYLE_HPP
#define USTYLE_HPP

#include "../uobject.hpp"

#include "../util/upoint.hpp"
#include "../util/udimension.hpp"
#include "../util/urectangle.hpp"
#include "../util/uinsets.hpp"

namespace ufo {

class UColor;
class UGraphics;
class UFont;

class UIcon;

class UWidget;
class UCompound;

class UCaret;

/** A style object performs all custom drawing done and associated
  * layouting by a look and feel.
  * It was introduced as drawing backend for the basic look and feel and
  * replaces the former UUIUtilities class.
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UStyle : public UObject {
	UFO_DECLARE_ABSTRACT_CLASS(UStyle)
public: // basic drawing
	/** Draws a bevel using ..*/
	virtual void paintBevel(UGraphics * g, UWidget * w,
		const URectangle & rect,
		Relief type) = 0;

	virtual void paintArrow(UGraphics * g, UWidget * w,
		const URectangle & rect,
		bool highlighted, bool activated,
		Direction direction) = 0;

	virtual void paintFocus(UGraphics * g, UWidget * w,
		const URectangle & rect) = 0;

	virtual void paintScrollTrack(UGraphics * g, UWidget * w,
		const URectangle & rect,
		bool highlighted, bool activated) = 0;

	/**
	  *@param left the left end of the widget
	  *@param right the right end of the widget
	  */
	virtual void paintTextSelection(UGraphics * g, UWidget * w,
		int left, int right,
		const URectangle & position, const URectangle & mark) = 0;
	/** Paints a text caret.
	  * @param rect The bounding rectangle of the current caret position
	  */
	virtual void paintCaret(UGraphics * g, UWidget * w,
		const URectangle & rect, UCaret * caret) = 0;

public: // basic widget templates

	/** Paints a tab */
	virtual void paintTab(UGraphics * g, UWidget * w,
		const URectangle & rect, const std::string & text,
		bool highlighted, bool activated) = 0;

	/** Paints the caption into the given rectangle.
	  * Shortens the string if necessary.
	  */
	virtual void paintControlCaption(UGraphics * g, UWidget * w,
		const URectangle & rect, const std::string & text) = 0;

	/** Paints the background from controls like buttons, menu items. */
	virtual void paintControlBackground(UGraphics * g, UWidget * w,
		const URectangle & rect,
		bool highlighted, bool activated) = 0;

	/** A generic paint method for compound icon and text.
	  * Does not paint background or similar.
	  * @return The bounding rectangle which was used for painting
	  */
	virtual void paintCompoundTextAndIcon(UGraphics * g, UCompound * w,
		const URectangle & rect, const std::string & text,
		UIcon * icon) = 0;

public: // widget drawing
	/** Paints icon and text of a compound widget. */

	virtual void paintBorder(UGraphics * g, UWidget * w,
		const URectangle & rect, BorderType borderType) = 0;

public: //
	virtual UInsets getBorderInsets(UWidget * w, BorderType borderType) = 0;

	/** layouts components with icon and text, e.g. labels, buttons
	  * the iconRect and textRect rectangles will be filled with the
	  *  correct sizes to paint the widget.
	  *@return
	  	the clipped text if the size of the widget is too small for the text
	  */
	virtual std::string layoutCompoundWidget(
		const UCompound * w,
		const UFont * f,
		const std::string & text,
		const UIcon * icon,
		URectangle * viewRect,
		URectangle * iconRect,
		URectangle * textRect,
		Alignment hAlignment,
		Alignment vAlignment,
		int textIconGap) = 0;

	/** Returns the preferred size of this compound widget with all of its
	  * own attributes.
	  */
	//virtual UDimension getCompoundPreferredSize(const UCompound * w) = 0;

	/** Icon may be NULL, Text may be ""
	  * @return
	  * 	the preferred size for a compound widget
	  */
	virtual UDimension getCompoundPreferredSize(
		const UCompound * w,
		const UFont * f,
		const std::string & text,
		const UIcon * icon) = 0;
};

} // namespace ufo

#endif // USTYLE_HPP
