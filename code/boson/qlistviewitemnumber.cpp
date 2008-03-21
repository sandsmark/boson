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

#include "qlistviewitemnumber.h"

#include <qregexp.h>

int QListViewItemNumber::compare(QListViewItem* i, int col, bool ascending) const
{
 bool ok = true;
 bool ok2 = true;
 double n = key(col, ascending).toDouble(&ok);
 double n2 = i->key(col, ascending).toDouble(&ok2);
 // numbers first - then letters
 if (ok && ok2) {
	if (n == n2) {
		return 0;
	} else if (n > n2) {
		return 1;
	} else {
		return -1;
	}
 } else if (ok) {
	// this is a number, i is not. this comes first.
	return -1;
 } else if (ok2) {
	// this is not a number, i is. i comes first.
	return 1;
 } else {
	return QListViewItem::compare(i, col, ascending);
 }
}


QString QListViewItemNumberPrefix::key(int column, bool ascending) const
{
 QString k = QListViewItemNumber::key(column, ascending);
 QRegExp r("^[0-9]+(\\.[0-9]+)?");
 if (r.search(k) >= 0) {
	return r.cap(0);
 }
 return k;
}

