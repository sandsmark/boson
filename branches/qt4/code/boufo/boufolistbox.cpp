/*
    This file is part of the Boson game
    Copyright (C) 2004-2005 Andreas Beckermann (b_mann@gmx.de)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <bogl.h>

// AB: first include the ufo headers, otherwise we conflict with Qt
#include <ufo/ufo.hpp>

// AB: make sure that we are compatible to system that have QT_NO_STL defined
#ifndef QT_NO_STL
#define QT_NO_STL
#endif

#include "boufolistbox.h"
#include "boufolistbox.moc"

#include <bodebug.h>
#include <QStringList>
//Added by qt3to4:
#include <Q3ValueList>

BoUfoListBox::BoUfoListBox() : BoUfoWidget()
{
 init();
}

void BoUfoListBox::init()
{
 setLayoutClass(UHBoxLayout);
 mListBox = new ufo::UListBox();

 ufo::UScrollPane* pane = new ufo::UScrollPane(mListBox);
 pane->setOpaque(false);
 ufoWidget()->add(pane);

 CONNECT_UFO_TO_QT(BoUfoListBox, mListBox, SelectionChanged);
}

void BoUfoListBox::setSelectionMode(BoUfoListBox::SelectionMode mode)
{
 ufo::UListBox::SelectionMode ufoMode;
 switch (mode) {
	default:
	case NoSelection:
		ufoMode = ufo::UListBox::NoSelection;
		break;
	case SingleSelection:
		ufoMode = ufo::UListBox::SingleSelection;
		break;
	case MultiSelection:
		ufoMode = ufo::UListBox::MultipleSelection;
		break;
 }
 mListBox->setSelectionMode(ufoMode);
}

BoUfoListBox::SelectionMode BoUfoListBox::selectionMode() const
{
 switch (mListBox->getSelectionMode()) {
	case ufo::UListBox::NoSelection:
		 return NoSelection;
	case ufo::UListBox::SingleSelection:
		 return SingleSelection;
	case ufo::UListBox::MultipleSelection:
		return MultiSelection;
 }
 return NoSelection;
}

void BoUfoListBox::clear()
{
 mListBox->removeAllItems();
}

int BoUfoListBox::selectedItem() const
{
 return mListBox->getSelectedIndex();
}

void BoUfoListBox::setSelectedItem(int i)
{
 mListBox->setSelectedIndex(i);
}

bool BoUfoListBox::isSelected(int index) const
{
 return mListBox->isSelectedIndex(index);
}

bool BoUfoListBox::isTextSelected(const QString& text) const
{
 QStringList items = selectedItemsText();
 if (items.contains(text)) {
	return true;
 }
 return false;
}

void BoUfoListBox::unselectAll()
{
 mListBox->setSelectedIndices(std::vector<unsigned int>());
}

void BoUfoListBox::setItemSelected(int index, bool select)
{
 switch (selectionMode()) {
	default:
	case NoSelection:
		break;
	case SingleSelection:
		if (select) {
			if (selectedItem() == index) {
				return;
			}
			unselectAll();
			mListBox->setSelectedIndex(index);
			return;
		} else {
			// FIXME: use mListBox->setSelectedIndex(index, false);
			// once such a thing exists!
			unselectAll();
		}
		break;
	case MultiSelection:
		// TODO: implement. ufo doesn't support multi selections yet.
		if (select) {
			mListBox->setSelectedIndex(index);
		} else {
			unselectAll();
		}
		break;
 }
}

QString BoUfoListBox::selectedText() const
{
 int i = selectedItem();
 if (i < 0) {
	return QString::null;
 }
 return items()[i];
}

Q3ValueList<unsigned int> BoUfoListBox::selectedItems() const
{
 Q3ValueList<unsigned int> ret;
 std::vector<unsigned int> sel = mListBox->getSelectedIndices();
 for (unsigned int i = 0; i < sel.size(); i++) {
	ret.append(sel[i]);
 }
 return ret;
}

QStringList BoUfoListBox::selectedItemsText() const
{
 // note: this may be pretty slow for large lists, it is just a trivial
 // implementation!
 // this doesnt matter since we don't have large lists in boson currently
 QStringList ret;
 QStringList allItems = items();
 Q3ValueList<unsigned int> sel = selectedItems();
 for (Q3ValueList<unsigned int>::iterator it = sel.begin(); it != sel.end(); ++it) {
	ret.append(allItems[*it]);
 }
 return ret;
}

QStringList BoUfoListBox::items() const
{
 QStringList list;
 std::vector<ufo::UItem*> items = mListBox->getItems();
 for (unsigned int i = 0; i < items.size(); i++) {
	list.append(items[i]->itemToString().c_str());
 }
 return list;
}

void BoUfoListBox::setItems(const QStringList& items)
{
 clear();
 for (QStringList::const_iterator it = items.begin(); it != items.end(); ++it) {
	QByteArray tmp = (*it).toAscii();
	mListBox->addItem(std::string(tmp.constData(), tmp.length()));
 }
}

unsigned int BoUfoListBox::count() const
{
 return mListBox->getItemCount();
}

QString BoUfoListBox::itemText(int i) const
{
 // TODO: implement properly. this is only a trivial and slow implementation.
 if (i < 0 || (unsigned int)i >= count()) {
	return QString::null;
 }
 return items()[i];
}

void BoUfoListBox::setItemText(int i, const QString& text)
{
 // TODO: implement properly. this is only a trivial and slow implementation.
 if (i < 0 || (unsigned int)i >= count()) {
	return;
 }
 QStringList list = items();
 list[i] = text;
 setItems(list);
}

void BoUfoListBox::insertItem(const QString& text, int index)
{
 // TODO: implement properly. this is only a trivial and slow implementation.
 QStringList list = items();
 if (index < 0) {
	index = list.count();
 }
 list.insert(index, text);
 setItems(list);
}

void BoUfoListBox::removeItem(int index)
{
 if (index < 0) {
	boError() << k_funcinfo << "index must be >= 0" << endl;
	return;
 }
 // TODO: implement properly. this is only a trivial and slow implementation.
 QStringList list = items();
 if (index >= (int)list.count()) {
	return;
 }
 list.removeAll(list.at(index));
 setItems(list);
}

void BoUfoListBox::setOpaque(bool o)
{
 BoUfoWidget::setOpaque(o);
 mListBox->setOpaque(o);
}

void BoUfoListBox::uslotSelectionChanged(ufo::UListBox*, int first, int last)
{
 emit signalSelectionChanged(first, last);
}

void BoUfoListBox::setMinimumSize(const ufo::UDimension& s)
{
 BoUfoWidget::setMinimumSize(s);
// mListBox->setMinimumSize(s);
}

void BoUfoListBox::setPreferredSize(const ufo::UDimension& s)
{
 BoUfoWidget::setPreferredSize(s);
// mListBox->setPreferredSize(s);
}

