/*
    This file is part of the Boson game
    Copyright (C) 2004-2006 Andreas Beckermann (b_mann@gmx.de)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef BOWIDGETLIST_H
#define BOWIDGETLIST_H

#include <qwidget.h>

class Q3ListBox;
class Q3ListBoxItem;

// displays a list of widgets that can be placed
class BoWidgetList : public QWidget
{
	Q_OBJECT
public:
	BoWidgetList(QWidget* parent, const char* name = 0);
	~BoWidgetList();

	QString widget() const;
	void clearSelection();

signals:
	void signalWidgetSelected(const QString&);

private slots:
	void slotWidgetHighlighted(Q3ListBoxItem*);
	void slotWidgetSelectionChanged();

private:
	Q3ListBox* mListBox;
};

#endif

