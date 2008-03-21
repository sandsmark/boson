/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/widgets/ucombobox.cpp
    begin             : Sat May 24 2003
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA *
 ***************************************************************************/


#include "ufo/widgets/ucombobox.hpp"
#include "ufo/widgets/uitem.hpp"
#include "ufo/widgets/ulistbox.hpp"
#include "ufo/widgets/ulineedit.hpp"

#include "ufo/layouts/uboxlayout.hpp"
#include "ufo/widgets/uscrollpane.hpp"
#include "ufo/upopupmanager.hpp"
#include "ufo/upopup.hpp"

#include "ufo/events/umouseevent.hpp"
#include "ufo/events/ukeyevent.hpp"

using namespace ufo;

UFO_IMPLEMENT_DEFAULT_DYNAMIC_CLASS(UComboBox, UWidget)

UComboBox::UComboBox()
	: m_listBox(new UListBox())
	, m_textEdit(new ULineEdit())
	, m_activated(NULL)
	, m_currentText()
	, m_visRowCount(4)
{
	trackPointer(m_listBox);
	trackPointer(m_textEdit);
	setLayout(new UBoxLayout(Horizontal));
	add(getTextEdit());/*, UBorderLayout::Center);
	UWidget * spacer = new UWidget();
	spacer->setPreferredSize(UDimension(20, 0));
	add(spacer, UBorderLayout::East);*/
}

UComboBox::UComboBox(const std::vector<UString> & listDataA)
	: m_listBox(new UListBox(listDataA))
	, m_textEdit(new ULineEdit())
	, m_activated(NULL)
	, m_currentText()
	, m_visRowCount(4)
{
	trackPointer(m_listBox);
	trackPointer(m_textEdit);
	setLayout(new UBoxLayout(Horizontal));
	add(getTextEdit());/*, UBorderLayout::Center);
	UWidget * spacer = new UWidget();
	spacer->setPreferredSize(UDimension(20, 0));
	add(spacer, UBorderLayout::East);*/
}


//
// public methods
//

void
UComboBox::addList(const std::vector<UString> & listData) {
	m_listBox->addList(listData);
	repaint();
}

void
UComboBox::addItem(UItem * itemA, int index) {
	m_listBox->addItem(itemA, index);
	repaint();
}

void
UComboBox::addItem(const UString & itemA, int index) {
	m_listBox->addItem(itemA, index);
	repaint();
}

void
UComboBox::addItem(UIcon * itemA, int index) {
	m_listBox->addItem(itemA, index);
	repaint();
}


void
UComboBox::removeItem(unsigned int index) {
	m_listBox->removeItem(index);
	repaint();
}

void
UComboBox::removeAllItems() {
	m_listBox->removeAllItems();
	repaint();
}


UItem *
UComboBox::getItemAt(unsigned int n) const {
	return m_listBox->getItemAt(n);
}

const std::vector<UItem *> &
UComboBox::getItems() const {
	return m_listBox->getItems();
}

unsigned int
UComboBox::getItemCount() const {
	return m_listBox->getItemCount();
}

int
UComboBox::getVisibleRowCount() const {
	//m_visRowCount
	return m_listBox->getVisibleRowCount();
}

void
UComboBox::setVisibleRowCount(int visibleRowCountA) {
	//m_visRowCount = visibleRowCountA;
	m_listBox->setVisibleRowCount(visibleRowCountA);
	repaint();
}

int
UComboBox::getCurrentItem() const {
	return m_listBox->getSelectedIndex();
}

void
UComboBox::setCurrentItem(int index) {
	m_listBox->setSelectedIndex(index);
	m_activated = m_listBox->getSelectedItem();
	m_textEdit->setText(m_activated->itemToString());
	m_sigActivated(this, index);
}

std::string
UComboBox::getCurrentText() const {
	return m_textEdit->getText();
}

void
UComboBox::setCurrentText(const std::string & text) {
	m_textEdit->setText(text);
}


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

void
UComboBox::popup() {
	if (s_pane == NULL) {
		s_pane = new UScrollPane();
		s_pane->setBorder(LineBorder);
		s_pane->reference();
	}
	if (!s_popup) {
		s_comboBox = this;
		getListBox()->sigMouseMoved().connect(slot(&combobox_highlight));
		getListBox()->sigMouseClicked().connect(slot(&combobox_activated));

		s_pane->setScrollable(getListBox());
		s_popup = UPopupManager::getPopupManager()->createPopup(
			this,
			s_pane,
			0, getHeight(), getWidth(), 0);
		s_popup->sigPopupAboutToClose().connect(slot(&popup_about_to_close));
	} else {
		s_popup->hide();
	}
}

void
UComboBox::setListBox(UListBox * listBox) {
	// FIXME !
	// change data
	swapPointers(m_listBox, listBox);
	m_listBox = listBox;
}

UListBox *
UComboBox::getListBox() const {
	return m_listBox;
}

void
UComboBox::setTextEdit(UTextEdit * textEdit) {
	std::string oldText = m_textEdit->getText();
	swapPointers(m_textEdit, textEdit);
	removeAll();
	m_textEdit = textEdit;
	m_textEdit->setText(oldText);
	add(m_textEdit);
}

UTextEdit *
UComboBox::getTextEdit() const {
	return m_textEdit;
}

UDimension
UComboBox::getContentsSize(const UDimension & maxSize) const {
	return UWidget::getContentsSize(maxSize);
}

void
UComboBox::processMouseEvent(UMouseEvent * e) {
	switch (e->getType()) {
		case UEvent::MousePressed:
			popup();
		break;
		default:
		break;
	}
	UWidget::processMouseEvent(e);
}

void
UComboBox::processKeyEvent(UKeyEvent * e) {
	if (e->isConsumed()) {
		UWidget::processKeyEvent(e);
		return;
	}
	bool isOpen = false;
	if (m_listBox->isVisible()) {
		isOpen = true;
	}

	int cur = getCurrentItem();
	if (e->getType() == UEvent::KeyPressed) {
		switch (e->getKeyCode()) {
			case UKey::UK_KP_DOWN:
			case UKey::UK_DOWN:
				setCurrentItem(cur + 1);
				e->consume();
			break;
			case UKey::UK_KP_UP:
			case UKey::UK_UP: {
				setCurrentItem((cur) ? cur - 1 : 0);
				e->consume();
			}
			break;
			/* FIXME:
			case UKey::UK_SPACE:
				if (!isOpen) {
					popup();
				}
				e->consume();
			break;*/
			case UKey::UK_ESCAPE:
				if (isOpen) {
					popup();
				}
				e->consume();
			break;
			/*
			case UKey::UK_KP_ENTER:
			case UKey::UK_RETURN:
				if (!isOpen) {
					popup();
				}
				e->consume();
			break;
			*/
			default:
			break;
		}
	}
	UWidget::processKeyEvent(e);
}
