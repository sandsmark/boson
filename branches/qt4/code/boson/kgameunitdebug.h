/*
    This file is part of the Boson game
    Copyright (C) 2001-2006 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef KGAMEUNITDEBUG_H
#define KGAMEUNITDEBUG_H

#include <qwidget.h>

class Q3ListViewItem;
class K3ListView;

class KGamePropertyBase;
class KGamePropertyHandler;

class Boson;
class BosonItem;
class BosonCanvas;
class Unit;

class KGameUnitDebugItemListPrivate;
class KGameUnitDebugItemList : public QWidget
{
	Q_OBJECT
public:
	KGameUnitDebugItemList(QWidget* parent);
	~KGameUnitDebugItemList();

	void clear();
	void update(const BosonCanvas* canvas);
	void updateProperty(BosonItem* item, KGamePropertyBase* prop);

signals:
	void signalItemSelected(BosonItem* item);

protected:
	void addItem(BosonItem* item);
	void update(BosonItem*);

protected slots:
	void slotSelected(Q3ListViewItem*);
	void slotItemListMenu(Q3ListViewItem*, const QPoint&, int);
	void slotItemListToggleShowColumn(int);
	void slotItemPropertyChanged(KGamePropertyBase*);

private:
	KGameUnitDebugItemListPrivate* d;
};


class KGameUnitDebugDataHandlerDisplay : public QWidget
{
	Q_OBJECT
public:
	KGameUnitDebugDataHandlerDisplay(QWidget* parent);
	~KGameUnitDebugDataHandlerDisplay();

	void clear();
	void displayDataHandler(KGamePropertyHandler* dataHandler);

private:
	K3ListView* mProperties;
};


class KGameUnitDebugPrivate;
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

protected slots:
	void slotUpdate();

	void slotItemSelected(BosonItem*);

protected:
	void updateCells(BosonItem*);
	void updateProperties(BosonItem*);
	void updateProduction(BosonItem*);
	void updateUnitCollisions(BosonItem*);
	void updatePathInfo(BosonItem* item);

	void addPropertiesPage();
	void addProductionsPage();
	void addCollisionsPage();
	void addCellsPage();
	void addPathInfoPage();

private:
	KGameUnitDebugPrivate* d;
};

#endif
