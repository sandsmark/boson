/*
    This file is part of the Boson game
    Copyright (C) 2006 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef QLISTVIEWITEMNUMBER_H
#define QLISTVIEWITEMNUMBER_H

#include <q3listview.h>

/**
 * @short @ref QListViewItem that tries to identify number columns and sort them
 * by their value
 **/
class QListViewItemNumber : public Q3ListViewItem
{
public:
	QListViewItemNumber(Q3ListView* p) : Q3ListViewItem(p)
	{
	}
	QListViewItemNumber(Q3ListViewItem* p) : Q3ListViewItem(p)
	{
	}

	virtual int compare(Q3ListViewItem* i, int col, bool ascending) const;
};

/**
 * @short Like @ref QListViewItemNumber, but uses only the part of the string
 * that starts with a number
 **/
class QListViewItemNumberPrefix : public QListViewItemNumber
{
public:
	QListViewItemNumberPrefix(Q3ListView* p) : QListViewItemNumber(p)
	{
	}
	QListViewItemNumberPrefix(Q3ListViewItem* p) : QListViewItemNumber(p)
	{
	}

	virtual QString key(int column, bool ascending) const;
};

#endif
