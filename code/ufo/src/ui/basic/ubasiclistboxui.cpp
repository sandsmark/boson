/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/ui/basic/ubasiclistboxui.cpp
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

#include "ufo/ui/basic/ubasiclistboxui.hpp"

#include "ufo/widgets/ulistbox.hpp"
#include "ufo/widgets/uitem.hpp"

#include "ufo/events/umouseevent.hpp"

#include "ufo/ui/uuimanager.hpp"


using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UBasicListBoxUI, UListBoxUI)

UBasicListBoxUI * UBasicListBoxUI::m_defaultListBoxUI = new UBasicListBoxUI();

std::string UBasicListBoxUI::m_lafId("UListBox");

UBasicListBoxUI *
UBasicListBoxUI::createUI(UWidget * w) {
	return m_defaultListBoxUI;
}


void
UBasicListBoxUI::installUI(UWidget * w) {
	UListBoxUI::installUI(w);

	UListBox * list = dynamic_cast<UListBox*>(w);
	installSignals(list);
}

void
UBasicListBoxUI::uninstallUI(UWidget * w) {
	UListBoxUI::uninstallUI(w);

	UListBox * list = dynamic_cast<UListBox*>(w);
	uninstallSignals(list);
}

void
UBasicListBoxUI::paint(UGraphics * g, UWidget * w) {
	UWidgetUI::paint(g, w);

	UListBox * list = static_cast<UListBox*>(w);

	UInsets insets = w->getInsets();
	// painting cells
	int x = insets.left;
	int y = insets.top;
	const UColor & background = list->getBackgroundColor();
	const UColor & foreground = list->getForegroundColor();
	const std::vector<UItem*> & items = list->getItems();

	for (std::vector<UItem*>::const_iterator iter = items.begin();
			iter != items.end();
			++iter) {
		if (list->isSelectedIndex(iter - items.begin())) {
			(*iter)->paintItem(g, list, x, y,
				true, // is selected
				false, // has focus
				list->getColorGroup().highlightedText(), // foreground
				list->getColorGroup().highlight() // background
			);
		} else {
			(*iter)->paintItem(g, list, x, y,
				false, false,
				foreground, background
			);
		}
		y += (*iter)->getItemSize(list).getHeight();
	}
}


const std::string &
UBasicListBoxUI::getLafId() {
	return m_lafId;
}


UDimension
UBasicListBoxUI::getPreferredSize(const UWidget * w) {
	const UListBox * list = static_cast<const UListBox*>(w);

	UDimension ret;

	// iterate through all cells
	// FIXME: this is quite expensive

	const std::vector<UItem*> & items = list->getItems();
	for (std::vector<UItem*>::const_iterator iter = items.begin();
			iter != items.end();
			++iter) {
		const UDimension & wsize = (*iter)->getItemSize(list);

		ret.h += wsize.h;
		ret.w = std::max(ret.w, wsize.w);
	}

	const UInsets & insets = list->getInsets();
	ret.w += insets.left + insets.right;
	ret.h += insets.top + insets.bottom;

	return ret;
}


UPoint
UBasicListBoxUI::indexToLocation(const UListBox * listA, unsigned int indexA) {
	// FIXME:
	// this is only a rough approximation and should be fixed

	UItem * item = listA->getItemAt(0);
	int height = 0;
	if (item) {
		height = item->getItemSize(listA).h;
	}

	return UPoint(0, indexA * height);
}

int
UBasicListBoxUI::locationToIndex(const UListBox * listA, const UPoint & locationA) {
	UItem * item = listA->getItemAt(0);
	if (item) {
		int height = item->getItemSize(listA).h;
		return locationA.y / height;
	} else {
		// FIXME
		// what about -1 ?
		return 0;
	}
}

static void focus_listener_slotPressed(UMouseEvent * e) {
	UListBox * list = dynamic_cast<UListBox*>(e->getSource());
	if (list == NULL) {
		return;
	}
	e->consume();

	int index = list->locationToIndex(e->getLocation());
	list->setSelectedIndex(index);
}

void
UBasicListBoxUI::installSignals(UListBox * listBox) {
	listBox->sigMousePressed().connect(slot(&focus_listener_slotPressed));
}

void
UBasicListBoxUI::uninstallSignals(UListBox * listBox) {
	listBox->sigMousePressed().disconnect(slot(&focus_listener_slotPressed));
}
