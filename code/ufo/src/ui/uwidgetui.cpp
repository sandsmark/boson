/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/ui/uwidgetui.cpp
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

#include "ufo/ui/uwidgetui.hpp"

#include "ufo/ugraphics.hpp"
#include "ufo/udrawable.hpp"

#include "ufo/ui/uuimanager.hpp"
#include "ufo/ui/ustyle.hpp"

#include "ufo/widgets/uwidget.hpp"

#include "ufo/util/ucolor.hpp"
#include "ufo/util/urectangle.hpp"

using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UWidgetUI, UObject)

UWidgetUI * UWidgetUI::m_widgetUI = new UWidgetUI();
std::string UWidgetUI::m_lafId("UWidget");


UWidgetUI *
UWidgetUI::createUI(UWidget * w) {
	return m_widgetUI;
}

void
UWidgetUI::installUI(UWidget * w) {
	UUIManager * manager = w->getUIManager();

	uint32_t attribState = w->getUIAttributesState();
	if (attribState & UWidget::AttribPalette) {
		w->setPalette(manager->getPalette(getLafId() + ".palette"));
		w->markUIAttribute(UWidget::AttribPalette);
	}
	if (attribState & UWidget::AttribBorder) {
		w->setBorder(manager->getBorder(getLafId() + ".border"));
		w->markUIAttribute(UWidget::AttribBorder);
	}
	if (w->getFont() == NULL || (attribState & UWidget::AttribFont)) {
		w->setFont(manager->getFont(getLafId() + ".font"));
		w->markUIAttribute(UWidget::AttribFont);
	}
}

void
UWidgetUI::uninstallUI(UWidget * w) {
	uint32_t attribState = w->getUIAttributesState();
	if (attribState & UWidget::AttribBorder) {
		w->setBorder(NoBorder);
		w->markUIAttribute(UWidget::AttribBorder);
	}
	if (attribState & UWidget::AttribPalette) {
		w->setPalette(UPalette::nullPalette);
		w->markUIAttribute(UWidget::AttribPalette);
	}
	if (attribState & UWidget::AttribFont) {
		w->setFont(NULL);
		w->markUIAttribute(UWidget::AttribFont);
	}
}

void
UWidgetUI::paint(UGraphics * g, UWidget * w) {
	//if (w->isOpaque()) {
	//float oldAlpha = g->getAlpha();
	//g->setAlpha(w->getOpacity());
	if (w->getOpacity()) {
		const UDimension & size = w->getSize(); // getInnerSize();
		//const UInsets & insets = w->getInsets();
		if (w->hasBackground()) {
			w->getBackground()->paintDrawable(g, 0, 0, size.w, size.h);
		} else {
			UColor col(w->getBackgroundColor());
			col.getFloat()[3] = w->getOpacity();
			g->setColor(col);
			g->fillRect(0, 0, size.w, size.h);
		}
	}
	//g->setAlpha(oldAlpha);
	//}
}

void
UWidgetUI::paintBorder(UGraphics * g, UWidget * w) {
	w->getUIManager()->getStyle()->paintBorder(
		g,
		w,
		URectangle(UPoint(), w->getSize()),
		w->getBorder()
	);
}

UInsets
UWidgetUI::getBorderInsets(UWidget * w) {
	return w->getUIManager()->getStyle()->getBorderInsets(w, w->getBorder());
}

const std::string &
UWidgetUI::getLafId() {
	return m_lafId;
}

UDimension
UWidgetUI::getPreferredSize(const UWidget * w) {
	return UDimension::invalid;
}

UDimension
UWidgetUI::getPreferredSize(const UWidget * w, const UDimension & maxSize) {
	return getPreferredSize(w);
}

UDimension
UWidgetUI::getMinimumSize(const UWidget * w) {
	return UDimension();
}

UDimension
UWidgetUI::getMaximumSize(const UWidget * w) {
	return UDimension::maxDimension;
}

//
// FIXME !
// this should be in other files

#include "ufo/ui/ulookandfeel.hpp"
#include "ufo/ui/ubuttonui.hpp"
#include "ufo/ui/ucomboboxui.hpp"
#include "ufo/ui/uinternalframeui.hpp"
#include "ufo/ui/ulabelui.hpp"
#include "ufo/ui/umenubarui.hpp"
#include "ufo/ui/umenuitemui.hpp"
#include "ufo/ui/upopupmenuui.hpp"
#include "ufo/ui/uscrollbarui.hpp"
#include "ufo/ui/useparatorui.hpp"
#include "ufo/ui/usliderui.hpp"
#include "ufo/ui/ustyle.hpp"

UFO_IMPLEMENT_ABSTRACT_CLASS(ULookAndFeel, UObject)
UFO_IMPLEMENT_DYNAMIC_CLASS(UButtonUI, UWidgetUI)
UFO_IMPLEMENT_ABSTRACT_CLASS(UComboBoxUI, UWidgetUI)
UFO_IMPLEMENT_DYNAMIC_CLASS(UInternalFrameUI, UWidgetUI)
UFO_IMPLEMENT_DYNAMIC_CLASS(ULabelUI, UWidgetUI)
UFO_IMPLEMENT_DYNAMIC_CLASS(UMenuBarUI, UWidgetUI)
UFO_IMPLEMENT_DYNAMIC_CLASS(UMenuItemUI, UWidgetUI)
UFO_IMPLEMENT_DYNAMIC_CLASS(UPopupMenuUI, UWidgetUI)
UFO_IMPLEMENT_DYNAMIC_CLASS(UScrollBarUI, UWidgetUI)
UFO_IMPLEMENT_DYNAMIC_CLASS(USeparatorUI, UWidgetUI)
UFO_IMPLEMENT_DYNAMIC_CLASS(USliderUI, UWidgetUI)
UFO_IMPLEMENT_DYNAMIC_CLASS(UStyle, UObject)
