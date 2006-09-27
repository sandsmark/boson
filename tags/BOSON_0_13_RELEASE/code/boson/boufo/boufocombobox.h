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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

// note the copyright above: this is LGPL!
#ifndef BOUFOCOMBOBOX_H
#define BOUFOCOMBOBOX_H

#include "boufowidget.h"

class BoUfoComboBox : public BoUfoWidget
{
	Q_OBJECT
public:
	BoUfoComboBox();

	ufo::UComboBox* comboBox() const
	{
		return mComboBox;
	}

	virtual void setMinimumSize(const ufo::UDimension& size);
	virtual void setPreferredSize(const ufo::UDimension& size);

	virtual void setOpaque(bool o);

	int currentItem() const;
	void setCurrentItem(int i);
	QString currentText() const;

	QStringList items() const;
	void setItems(const QStringList& items);
	void clear();
	unsigned int count() const;
	QString itemText(int i) const;
	void setItemText(int i, const QString& text);
	void insertItem(const QString& text, int index = -1);
	void removeItem(int index);


signals:
	void signalActivated(int);
	void signalHighlighted(int);
	void signalSelectionChanged(int, int);

private:
	void init();
	void uslotActivated(ufo::UComboBox*, int);
	void uslotHighlighted(ufo::UComboBox*, int);
	void uslotSelectionChanged(ufo::UComboBox*, int, int);

signals:

private:
	ufo::UComboBox* mComboBox;
};

#endif
