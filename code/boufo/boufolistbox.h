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

// note the copyright above: this is LGPL!
#ifndef BOUFOLISTBOX_H
#define BOUFOLISTBOX_H

#include "boufowidget.h"

class BoUfoListBox : public BoUfoWidget
{
	Q_OBJECT
	Q_PROPERTY(SelectionMode selection READ selectionMode WRITE setSelectionMode);
	Q_ENUMS(SelectionMode);
public:
	enum SelectionMode {
		NoSelection,
		SingleSelection,
		MultiSelection
	};
public:
	BoUfoListBox();

	ufo::UListBox* listBox() const
	{
		return mListBox;
	}

	virtual void setMinimumSize(const ufo::UDimension& size);
	virtual void setPreferredSize(const ufo::UDimension& size);

	virtual void setOpaque(bool o);

	int selectedItem() const;
	void setItemSelected(int index, bool select);
	QString selectedText() const;
	bool isSelected(int index) const;
	void unselectAll();

	// this exists because of libufo only. use setItemSelected() instead!
	void setSelectedItem(int index);

	/**
	 * @return A list with the indices of all selected items
	 **/
	QValueList<unsigned int> selectedItems() const;

	/**
	 * @return A list with the text of all selected items
	 **/
	QStringList selectedItemsText() const;

	/**
	 * @return TRUE if there is currently one item with @p text selected
	 **/
	bool isTextSelected(const QString& text) const;

	QStringList items() const;
	void setItems(const QStringList& items);
	void clear();
	unsigned int count() const;
	QString itemText(int i) const;
	void setItemText(int i, const QString& text);
	void insertItem(const QString& text, int index = -1);
	void removeItem(int index);


	// note: libufo does not yet implement MultiSelection
	void setSelectionMode(SelectionMode mode);
	SelectionMode selectionMode() const;


signals:
	void signalSelectionChanged(int firstIndex, int lastIndex);

private:
	void init();
	void uslotSelectionChanged(ufo::UListBox*, int, int);

signals:

private:
	ufo::UListBox* mListBox;
};

#endif
