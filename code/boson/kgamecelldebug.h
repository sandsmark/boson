/*
    This file is part of the Boson game
    Copyright (C) 2002 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef KGAMECELLDEBUG_H
#define KGAMECELLDEBUG_H

#include <qwidget.h>

class BosonMap;
class QListViewItem;

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class KGameCellDebug : public QWidget
{
	Q_OBJECT
public:
	KGameCellDebug(QWidget* parent);
	~KGameCellDebug();

	void setMap(BosonMap* map);

protected slots:
	void slotUpdate();
	void slotUpdateCell(QListViewItem*);
	
private:
	class KGameCellDebugPrivate;
	KGameCellDebugPrivate* d;
};

#endif
