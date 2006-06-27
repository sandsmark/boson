/*
    This file is part of the Boson game
    Copyright (C) 2001 Andreas Beckermann (b_mann@gmx.de)

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef KGAMEUNITDEBUG_H
#define KGAMEUNITDEBUG_H

#include <qwidget.h>

class QListViewItem;

class KGamePropertyBase;

class Boson;
class Unit;

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class KGameUnitDebug : public QWidget
{
	Q_OBJECT
public:
	KGameUnitDebug(QWidget* parent);
	~KGameUnitDebug();

	void setBoson(Boson*);

protected:
	void addUnit(Unit* unit);
	void update(QListViewItem*, Unit*);

protected slots:
	void slotUpdate();
	void slotUnitSelected(QListViewItem*);
	void slotUnitListMenu(QListViewItem*, const QPoint&, int);
	void slotUnitListToggleShowColumn(int);

	void slotUnitPropertyChanged(KGamePropertyBase*);

protected:
	void updateProduction(Unit*);
	void updateUnitsInRange(Unit*);
	void updateUnitCollisions(Unit*);
	void updateCells(Unit*);
	void updateProperties(Unit*);

private:
	class KGameUnitDebugPrivate;
	KGameUnitDebugPrivate* d;
};

#endif
