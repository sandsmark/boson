/*
    This file is part of the Boson game
    Copyright (C) 2001 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include "kgameunitdebug.h"

#include "boson.h"
#include "unit.h"
#include "player.h"

#include <klistview.h>
#include <klistbox.h>
#include <klocale.h>
#include <kdebug.h>

#include <qlayout.h>
#include <qpushbutton.h>
#include <qintdict.h>

#include "kgameunitdebug.moc"

class KGameUnitDebug::KGameUnitDebugPrivate
{
public:
	KGameUnitDebugPrivate()
	{
		mBoson = 0;
		mUnitList = 0;
		mWaypoints = 0;
	}

	Boson* mBoson;

	KListView* mUnitList;
	KListBox* mWaypoints;
	KListView* mProduction;

	QIntDict<Unit> mUnits;
};

KGameUnitDebug::KGameUnitDebug(QWidget* parent) : QWidget(parent)
{
 d = new KGameUnitDebugPrivate;
 QVBoxLayout* topLayout = new QVBoxLayout(this);
 QHBoxLayout* layout = new QHBoxLayout(topLayout);
 
 d->mUnitList = new KListView(this);
 d->mUnitList->addColumn(i18n("Id"));
 d->mUnitList->addColumn(i18n("Owner"));
 d->mUnitList->addColumn(i18n("RTTI"));
 d->mUnitList->addColumn(i18n("X"));
 d->mUnitList->addColumn(i18n("Y"));
 d->mUnitList->addColumn(i18n("Z"));
 d->mUnitList->addColumn(i18n("Name"));
 d->mUnitList->addColumn(i18n("Health"));
// d->mUnitList->addColumn(i18n("Costs"));
 d->mUnitList->addColumn(i18n("Speed"));
// connect(d->mUnitList, SIGNAL(executed(QListBoxItem*)), 
//		this, SLOT(slotSelectUnit(QListBoxItem*)));
 layout->addWidget(d->mUnitList);

 QVBoxLayout* l = new QVBoxLayout(layout);
 d->mWaypoints = new KListBox(this);
 connect(d->mUnitList, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(updateWaypoints(QListViewItem*)));
 l->addWidget(d->mWaypoints);

 d->mProduction = new KListView(this);
 d->mProduction->addColumn(i18n("Number"));
 d->mProduction->addColumn(i18n("TypeId"));
 d->mProduction->addColumn(i18n("ETA"));
 connect(d->mUnitList, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(updateProduction(QListViewItem*)));
 l->addWidget(d->mProduction);
 

/*
 d->mUnitView = new KListView(this);
 d->mUnitView->addColumn(i18n("Id"));
 d->mUnitView->addColumn(i18n("Name"));
 layout->addWidget(d->mUnitView);*/

 QPushButton* update = new QPushButton(i18n("Update"), this);
 connect(update, SIGNAL(pressed()), this, SLOT(slotUpdate()));
 topLayout->addWidget(update);
}

KGameUnitDebug::~KGameUnitDebug()
{
 delete d;
}

void KGameUnitDebug::setBoson(Boson* b)
{
 d->mBoson = b;
 slotUpdate();
}

void KGameUnitDebug::slotUpdate()
{
 d->mUnitList->clear();
// d->mUnitView->clear();
 d->mUnits.clear();
 d->mWaypoints->clear();
 d->mProduction->clear();
 if (!d->mBoson) {
	return;
 }

 QPtrList<Unit> units;
 QPtrList<KPlayer>list = *d->mBoson->playerList();
 for (unsigned int i = 0; i < d->mBoson->playerCount(); i++) {
	QPtrList<Unit> playerUnits = ((Player*)list.at(i))->allUnits();
	for (unsigned int j = 0; j < playerUnits.count(); j++) {
		units.append(playerUnits.at(j));
	}
 }

 QPtrListIterator<Unit> it(units);
 while (it.current()) {
	if (d->mUnits.find(it.current()->id())) {
		kdError() << "Cannot double-add id " << it.current()->id() << endl;
	} else {
		Unit* unit = it.current();
		d->mUnits.insert(unit->id(), unit);
		addUnit(unit);
	}
	++it;
 }
}

void KGameUnitDebug::addUnit(Unit* unit)
{
 QListViewItem* item = new QListViewItem(d->mUnitList);
 item->setText(0, QString::number(unit->id()));
 item->setText(1, QString::number(unit->owner() ? unit->owner()->id() : 0));
 item->setText(2, QString::number(unit->rtti()));
 item->setText(3, QString::number(unit->x()));
 item->setText(4, QString::number(unit->y()));
 item->setText(5, QString::number(unit->z()));
 item->setText(6, unit->name());
 item->setText(7, QString::number(unit->health()));
 item->setText(8, QString::number(unit->speed()));
}

void KGameUnitDebug::updateWaypoints(QListViewItem* item)
{
 d->mWaypoints->clear();
 if (!item) {
	return;
 }
 int id = item->text(0).toInt();
 Unit* unit = d->mUnits[id];
 if (!unit) {
	kdWarning() << k_lineinfo << "id " << id << " not found" << endl;
	return;
 }
 QValueList<QPoint> points = unit->waypointList();
 for (unsigned int i = 0; i < points.count(); i++) {
	(void)new QListBoxText(d->mWaypoints, i18n("x=%1 y=%2").arg(points[i].x()).arg(points[i].y()));
 }
}

void KGameUnitDebug::updateProduction(QListViewItem* item)
{
 d->mProduction->clear();
 if (!item) {
	return;
 }
 int id = item->text(0).toInt();
 Unit* unit = d->mUnits[id];
 if (!unit) {
	kdWarning() << k_lineinfo << "id " << id << " not found" << endl;
	return;
 }
 if (!unit->isFacility()) {
	return;
 }
 QValueList<int> constructions = ((Facility*)unit)->productionList();
 for (unsigned int i = 0; i < constructions.count(); i++) {
	QListViewItem* item = new QListViewItem(d->mProduction);
	item->setText(0, QString::number(i+1));
	item->setText(1, QString::number(constructions[i]));
	item->setText(2, i18n("Ready")); // currently always ready
 }
 
}

