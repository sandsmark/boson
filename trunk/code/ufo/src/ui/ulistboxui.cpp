/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/ui/ulistboxui.cpp
    begin             : Wed Jun 19 2002
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

#include "ufo/ui/ulistboxui.hpp"

#include "ufo/widgets/ulistbox.hpp"
#include "ufo/util/upoint.hpp"

#include "ufo/events/umouseevent.hpp"

using namespace ufo;

UFO_IMPLEMENT_ABSTRACT_CLASS(UListBoxUI, UWidgetUI)

//UListBoxUI::MouseSelectionListener * UListBoxUI::m_mouseSelectionListener =
//	new UListBoxUI::MouseSelectionListener();
/*static void focus_listener_slotPressed(UMouseEvent * e) {
	UListBox * list = dynamic_cast<UListBox*>(e->getSource());
	if (list == NULL) {
		return;
	}

	int index = list->locationToIndex(e->getLocation());
	list->setSelectedIndex(index);
}

void
UListBoxUI::installUI(UWidget * w) {
	UWidgetUI::installUI(w);
	//w->addMouseListener(m_mouseSelectionListener);
	w->sigMousePressed().connect(slot(&focus_listener_slotPressed));
}

void
UListBoxUI::uninstallUI(UWidget * w) {
	UWidgetUI::uninstallUI(w);
	w->sigMousePressed().disconnect(slot(&focus_listener_slotPressed));
	//w->addMouseListener(m_mouseSelectionListener);
}
*/
/*
void
UListBoxUI::MouseSelectionListener::mousePressed(UMouseEvent * e) {
	UListBox * list = dynamic_cast<UListBox*>(e->getSource());
	if (list == NULL) {
		return;
	}

	int index = list->locationToIndex(e->getPoint());
	list->setSelectedIndex(index);
}

void
UListBoxUI::MouseSelectionListener::mouseReleased(UMouseEvent * e) {}

void
UListBoxUI::MouseSelectionListener::mouseClicked(UMouseEvent * e) {}
*/
