/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/gl/ugl_style.hpp
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

#ifndef UGL_STYLE_HPP
#define UGL_STYLE_HPP

#include "../ui/ustyle.hpp"

namespace ufo {

class UFO_EXPORT UGL_Style : public UStyle {
	UFO_DECLARE_ABSTRACT_CLASS(UGL_Style)

public: // Implements UStyle
	virtual void paintBevel(UGraphics * g, UWidget * w,
		const URectangle & rect,
		Relief type);

	virtual void paintArrow(UGraphics * g, UWidget * w,
		const URectangle & rect,
		bool highlighted, bool activated,
		Direction direction);

	virtual void paintFocus(UGraphics * g, UWidget * w,
		const URectangle & rect);

	virtual void paintScrollTrack(UGraphics * g, UWidget * w,
		const URectangle & rect,
		bool highlighted, bool activated);

	virtual void paintTextSelection(UGraphics * g, UWidget * w,
		int left, int right,
		const URectangle & position, const URectangle & mark);

	virtual void paintCaret(UGraphics * g, UWidget * w,
		const URectangle & rect, UCaret * caret);

	virtual void paintTab(UGraphics * g, UWidget * w,
		const URectangle & rect, const std::string & text,
		bool highlighted, bool activated);

	virtual void paintControlCaption(UGraphics * g, UWidget * w,
		const URectangle & rect, const std::string & text);

	virtual void paintControlBackground(UGraphics * g, UWidget * w,
		const URectangle & rect,
		bool highlighted, bool activated);

	virtual void paintCompoundTextAndIcon(UGraphics * g, UCompound * w,
		const URectangle & rect, const std::string & text,
		UIcon * icon);

	virtual void paintBorder(UGraphics * g, UWidget * w,
		const URectangle & rect, BorderType borderType);

	virtual UInsets getBorderInsets(UWidget * w, BorderType borderType);

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
		int textIconGap);

	virtual UDimension getCompoundPreferredSize(
		const UCompound * w,
		const UFont * f,
		const std::string & text,
		const UIcon * icon);

protected: // Helper functions
	virtual void paintLineBorder(UGraphics * g, UWidget * w,
		const URectangle & rect);
	virtual void paintRaisedBevelBorder(UGraphics * g, UWidget * w,
		const URectangle & rect);
	virtual void paintLoweredBevelBorder(UGraphics * g, UWidget * w,
		const URectangle & rect);
	virtual void paintTitledBorder(UGraphics * g, UWidget * w,
		const URectangle & rect);
	virtual void paintUIBorder(UGraphics * g, UWidget * w,
		const URectangle & rect);
	virtual UInsets getUIBorderInsets(UWidget * w, BorderType borderType);
	virtual void paintControlBorder(UGraphics * g, UWidget * w,
		const URectangle & rect);
	virtual void paintMenuBorder(UGraphics * g, UWidget * w,
		const URectangle & rect);
	virtual void paintMenuBarBorder(UGraphics * g, UWidget * w,
		const URectangle & rect);
	virtual void paintInternalFrameBorder(UGraphics * g, UWidget * w,
		const URectangle & rect);
};

} // namespace ufo

#endif // UGL_STYLE_HPP
