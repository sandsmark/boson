/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/ui/basic/ubasiccomboboxui.cpp
    begin             : Mon Jun 9 2003
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

#include "ufo/ui/basic/ubasiccomboboxui.hpp"

#include "ufo/widgets/ucombobox.hpp"
#include "ufo/widgets/ubutton.hpp"
#include "ufo/widgets/utextedit.hpp"
#include "ufo/widgets/uitem.hpp"

#include "ufo/image/uxbmicon.hpp"

#include "ufo/layouts/uborderlayout.hpp"

#include "ufo/ui/uuimanager.hpp"


#include "ufo/widgets/ulistbox.hpp"
#include "ufo/widgets/uscrollpane.hpp"
#include "ufo/events/uactionevent.hpp"
#include "ufo/events/umouseevent.hpp"
#include "ufo/upopupmanager.hpp"
#include "ufo/upopup.hpp"

#include "arrows.xbm"


using namespace ufo;

static UXBMIcon s_arrowDownIcon(arrow_down_bits, arrow_down_width, arrow_down_height);

static UComboBox * s_comboBox = NULL;
static UPopup * s_popup = NULL;
static UScrollPane  * s_pane = NULL;

static void
combobox_highlight(UMouseEvent * e) {
	e->consume();
	s_comboBox->getListBox()->setSelectedIndex(s_comboBox->getListBox()->
		locationToIndex(e->getLocation()));
}

static void
combobox_activated(UMouseEvent * e) {
	e->consume();
	s_comboBox->setCurrentItem(s_comboBox->getListBox()->
		locationToIndex(e->getLocation()));
	s_popup->hide();
}

static void
popup_about_to_close(UPopup * popup) {
	s_popup->sigPopupAboutToClose().disconnect(slot(&popup_about_to_close));
	s_comboBox->getListBox()->sigMouseMoved().disconnect(slot(&combobox_highlight));
	s_comboBox->getListBox()->sigMouseClicked().disconnect(slot(&combobox_activated));
	s_pane->setScrollable(NULL);
	s_comboBox = NULL;
	s_popup = NULL;
}


static void
combobox_popup(UActionEvent * e) {
	e->consume();
	if (s_pane == NULL) {
		s_pane = new UScrollPane();
		s_pane->reference();
	}
	UWidget * button = dynamic_cast<UWidget*>(e->getSource());
	if (button) {
		if (UComboBox * box = dynamic_cast<UComboBox*>(button->getParent())) {
			if (!s_popup) {
				s_comboBox = box;
				box->getListBox()->sigMouseMoved().connect(slot(&combobox_highlight));
				box->getListBox()->sigMouseClicked().connect(slot(&combobox_activated));

				s_pane->setScrollable(box->getListBox());
				s_popup = UPopupManager::getPopupManager()->createPopup(
					box,
					s_pane,
					0, box->getHeight(), box->getWidth(), 0);
				s_popup->sigPopupAboutToClose().connect(slot(&popup_about_to_close));
			} else {
				s_popup->hide();
			}
		}
	}
}

UFO_IMPLEMENT_DYNAMIC_CLASS(UBasicComboBoxUI, UComboBoxUI)

UBasicComboBoxUI * UBasicComboBoxUI::m_defaultComboBoxUI = new UBasicComboBoxUI();

std::string UBasicComboBoxUI::m_lafId("UComboBox");

UBasicComboBoxUI *
UBasicComboBoxUI::createUI(UWidget * w) {
	return m_defaultComboBoxUI;
}


void
UBasicComboBoxUI::installUI(UWidget * w) {
	UComboBoxUI::installUI(w);

	UComboBox * combo = dynamic_cast<UComboBox*>(w);

	combo->setLayout(new UBorderLayout());
	combo->add(combo->getTextEdit(), UBorderLayout::Center);

	UButton * downArrow = new UButton(&s_arrowDownIcon);
	downArrow->setBorderPainted(false);
	downArrow->setFocusable(false);
	downArrow->sigButtonClicked().connect(slot(&combobox_popup));
	combo->add(downArrow, UBorderLayout::East);
}

void
UBasicComboBoxUI::uninstallUI(UWidget * w) {
	UComboBoxUI::uninstallUI(w);

	UComboBox * combo = dynamic_cast<UComboBox*>(w);

	combo->removeAll();
	combo->setLayout(NULL);
}

const std::string &
UBasicComboBoxUI::getLafId() {
	return m_lafId;
}

