/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
    copyright         : (C) 2004 by Andreas Beckermann
    email             : b_mann at gmx.de
                             -------------------

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

// AB: this is a modified file from libufo. the license remains LGPL.
// the original file was include/ufo/ui/basic/ubasiclabelui.hpp

#ifndef UBOLABELUI_H
#define UBOLABELUI_H

#include <ufo/ui/ulabelui.hpp>

#include <ufo/ui/ulabelui.hpp>
#include <ufo/util/urectangle.hpp>

namespace ufo {

class UCompound;
class UIcon;
class UFont;

// AB: implementation of a ULabelUI with support for line breaks
class UFO_EXPORT UBoLabelUI : public ULabelUI {
	UFO_DECLARE_DYNAMIC_CLASS(UBoLabelUI)
public:
	static UBoLabelUI * createUI(UWidget * w);

	void paint(UGraphics * g, UWidget * w);

	const std::string & getLafId();

	UDimension getPreferredSize(const UWidget * w);

protected:
	// AB: these is our modification of this class.
	// we use the style* methods instead of corresponding methods in
	// UGL_Style
	void stylePaintCompoundTextAndIcon(UGraphics * g, UCompound * w,
		const URectangle & rect, const std::string & text,
		UIcon * icon);
	std::string UBoLabelUI::styleLayoutCompoundWidget(
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
	void stylePaintControlCaption(UGraphics * g, UWidget * w, const URectangle & rect, const std::string & text);
	UDimension getStyleCompoundPreferredSize(const UCompound * w, const UFont * f, const std::string & text, const UIcon * icon);


private:  // Private attributes
	/**  */
	static UBoLabelUI * m_labelUI;
	/** The shared look and feel id */
	static std::string m_lafId;
};

} // namespace ufo

#endif // UBOLABELUI_H

