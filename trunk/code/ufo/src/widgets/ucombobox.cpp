/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/


#include "ufo/widgets/ucombobox.hpp"
#include "ufo/widgets/uitem.hpp"
#include "ufo/widgets/ulistbox.hpp"
#include "ufo/widgets/utextedit.hpp"

namespace ufo {

UFO_IMPLEMENT_DEFAULT_DYNAMIC_CLASS(UComboBox, UWidget)

UComboBox::UComboBox()
	: m_listBox(new UListBox())
	, m_textEdit(new UTextEdit())
	, m_activated(NULL)
	, m_currentText()
	, m_visRowCount(4)
{
	trackPointer(m_listBox);
	trackPointer(m_textEdit);
}

UComboBox::UComboBox(const std::vector<UString> & listDataA)
	: m_listBox(new UListBox(listDataA))
	, m_textEdit(new UTextEdit())
	, m_activated(NULL)
	, m_currentText()
	, m_visRowCount(4)
{
	trackPointer(m_listBox);
	trackPointer(m_textEdit);
}


//*
//* hides | overrides UWidget
//*
/*
void
UComboBox::setUI(UComboBoxUI * ui) {
	UWidget::setUI(ui);
}

UWidgetUI *
UComboBox::getUI() const {
	return static_cast<UComboBoxUI*>(UWidget::getUI());
}

void
UComboBox::updateUI() {
	setUI(static_cast<UComboBoxUI*>(getUIManager()->getUI(this)));
}
*/

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
	return m_visRowCount;
}

void
UComboBox::setVisibleRowCount(int visibleRowCountA) {
	m_visRowCount = visibleRowCountA;
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


void
UComboBox::popup() {

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
	m_textEdit = textEdit;
	m_textEdit->setText(oldText);
}

UTextEdit *
UComboBox::getTextEdit() const {
	return m_textEdit;
}

} // namespace ufo
