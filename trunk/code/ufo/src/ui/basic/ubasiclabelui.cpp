/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/ui/basic/ubasiclabelui.cpp
    begin             : Mon Jul 22 2002
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

#include "ufo/ui/basic/ubasiclabelui.hpp"

#include "ufo/ui/uuimanager.hpp"
#include "ufo/ui/ustyle.hpp"
#include "ufo/uicon.hpp"

#include "ufo/widgets/ulabel.hpp"
#include "ufo/font/ufont.hpp"


using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UBasicLabelUI, ULabelUI)

UBasicLabelUI * UBasicLabelUI::m_labelUI = new UBasicLabelUI();

std::string UBasicLabelUI::m_lafId("ULabel");

UBasicLabelUI * UBasicLabelUI::createUI(UWidget * w) {
	return m_labelUI;
}

void
UBasicLabelUI::paint(UGraphics * g, UWidget * w) {
	UWidgetUI::paint(g, w);

	ULabel * label;
	const UInsets & insets = w->getInsets();

	label = dynamic_cast<ULabel *>(w);

	UStyle * style = w->getUIManager()->getStyle();

	URectangle viewRect(insets.left, insets.top,
		label->getWidth() - insets.getHorizontal(),
		label->getHeight() - insets.getVertical());

	style->paintCompoundTextAndIcon(g, label, viewRect,
		label->getText(), label->getIcon());
}

const std::string &
UBasicLabelUI::getLafId() {
	return m_lafId;
}



UDimension
UBasicLabelUI::getPreferredSize(const UWidget * w) {
	const ULabel * label = dynamic_cast<const ULabel *>(w);
	//return UUIUtilities::getCompoundPreferredSize(label);
	return w->getUIManager()->getStyle()->getCompoundPreferredSize(
		label,
		label->getFont(),
		label->getText(),
		label->getIcon()
	);
}
