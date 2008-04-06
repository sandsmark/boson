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

#include "boufocombobox.h"
#include "boufocombobox.moc"

#include <QStringList>

#include <bodebug.h>

BoUfoComboBox::BoUfoComboBox() : BoUfoWidget()
{
 init();
}

void BoUfoComboBox::init()
{
 setLayoutClass(UHBoxLayout);
 mComboBox = new ufo::UComboBox();
 ufoWidget()->add(mComboBox);

 CONNECT_UFO_TO_QT(BoUfoComboBox, mComboBox, Activated);
 CONNECT_UFO_TO_QT(BoUfoComboBox, mComboBox, Highlighted);
 CONNECT_UFO_TO_QT(BoUfoComboBox, mComboBox, SelectionChanged);
}

void BoUfoComboBox::setOpaque(bool o)
{
 BoUfoWidget::setOpaque(o);
 mComboBox->setOpaque(o);
}

void BoUfoComboBox::uslotActivated(ufo::UComboBox*, int i)
{
 emit signalActivated(i);
}

void BoUfoComboBox::uslotHighlighted(ufo::UComboBox*, int i)
{
 emit signalHighlighted(i);
}

void BoUfoComboBox::uslotSelectionChanged(ufo::UComboBox*, int i1, int i2)
{
 emit signalSelectionChanged(i1, i2);
}

void BoUfoComboBox::setMinimumSize(const ufo::UDimension& s)
{
 BoUfoWidget::setMinimumSize(s);
 mComboBox->setMinimumSize(s);
}

void BoUfoComboBox::setPreferredSize(const ufo::UDimension& s)
{
 BoUfoWidget::setPreferredSize(s);
 mComboBox->setPreferredSize(s);
}

void BoUfoComboBox::clear()
{
 mComboBox->removeAllItems();
}

int BoUfoComboBox::currentItem() const
{
 return mComboBox->getCurrentItem();
}

void BoUfoComboBox::setCurrentItem(int i)
{
 if (i < 0) {
	return;
 }
 if ((unsigned int)i >= count()) {
	return;
 }
 mComboBox->setCurrentItem(i);
}

QString BoUfoComboBox::currentText() const
{
 return QString::fromLatin1(mComboBox->getCurrentText().c_str());
}

QStringList BoUfoComboBox::items() const
{
 QStringList list;
 std::vector<ufo::UItem*> items = mComboBox->getItems();
 for (unsigned int i = 0; i < items.size(); i++) {
	list.append(items[i]->itemToString().c_str());
 }
 return list;
}

void BoUfoComboBox::setItems(const QStringList& items)
{
 clear();
 for (QStringList::const_iterator it = items.begin(); it != items.end(); ++it) {
	QByteArray tmp = (*it).toAscii();
	mComboBox->addItem(std::string(tmp.constData(), tmp.length()));
 }
}

unsigned int BoUfoComboBox::count() const
{
 return mComboBox->getItemCount();
}

QString BoUfoComboBox::itemText(int i) const
{
 // TODO: implement properly. this is only a trivail and slow implementation.
 if (i < 0 || (unsigned int)i >= count()) {
	return QString::null;
 }
 return items()[i];
}

void BoUfoComboBox::setItemText(int i, const QString& text)
{
 // TODO: implement properly. this is only a trivail and slow implementation.
 if (i < 0 || (unsigned int)i >= count()) {
	return;
 }
 QStringList list = items();
 list[i] = text;
 setItems(list);
}

void BoUfoComboBox::insertItem(const QString& text, int index)
{
 // TODO: implement properly. this is only a trivail and slow implementation.
 QStringList list = items();
 if (index < 0) {
	index = list.count();
 }
 list.insert(index, text);
 setItems(list);
}

void BoUfoComboBox::removeItem(int index)
{
 // TODO: implement properly. this is only a trivail and slow implementation.
 QStringList list = items();
 list.removeAt(index);
 setItems(list);
}

