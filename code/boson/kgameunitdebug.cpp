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
#include <kgame/kgamepropertyhandler.h>

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
	QPtrDict<QListViewItem> mItems;

	// column ids:
	int mId;
	int mOwner;
	int mRTTI;
	int mX;
	int mY;
	int mZ;
	int mName;
	int mHealth;
	int mSpeed;
	int mWork;
	int mWidth;
	int mHeight;
	int mBoundingRect;
};

KGameUnitDebug::KGameUnitDebug(QWidget* parent) : QWidget(parent)
{
 d = new KGameUnitDebugPrivate;
 QVBoxLayout* topLayout = new QVBoxLayout(this);
 QHBoxLayout* layout = new QHBoxLayout(topLayout);
 
 d->mUnitList = new KListView(this);
 d->mId = d->mUnitList->addColumn(i18n("Id"));
 d->mOwner = d->mUnitList->addColumn(i18n("Owner"));
 d->mRTTI = d->mUnitList->addColumn(i18n("RTTI"));
 d->mX = d->mUnitList->addColumn(i18n("X"));
 d->mY = d->mUnitList->addColumn(i18n("Y"));
 d->mZ = d->mUnitList->addColumn(i18n("Z"));
 d->mWork = d->mUnitList->addColumn(i18n("Work"));
 d->mName = d->mUnitList->addColumn(i18n("Name"));
 d->mHealth = d->mUnitList->addColumn(i18n("Health"));
// d->mUnitList->addColumn(i18n("Costs"));
 d->mSpeed = d->mUnitList->addColumn(i18n("Speed"));
// connect(d->mUnitList, SIGNAL(executed(QListBoxItem*)), 
//		this, SLOT(slotSelectUnit(QListBoxItem*)));
 d->mWidth = d->mUnitList->addColumn(i18n("Width"));
 d->mHeight = d->mUnitList->addColumn(i18n("Height"));
 d->mBoundingRect = d->mUnitList->addColumn(i18n("BoundingRect"));
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
 d->mItems.clear();
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
 d->mItems.insert(unit, item);
 connect(unit->dataHandler(), SIGNAL(signalPropertyChanged(KGamePropertyBase*)),
		this, SLOT(slotUnitPropertyChanged(KGamePropertyBase*)));
 update(item, unit);
}

void KGameUnitDebug::update(QListViewItem* item, Unit* unit)
{
 item->setText(d->mId, QString::number(unit->id()));
 item->setText(d->mOwner, QString::number(unit->owner() ? unit->owner()->id() : 0));
 item->setText(d->mRTTI, QString::number(unit->rtti()));
 item->setText(d->mX, QString::number(unit->x()));
 item->setText(d->mY, QString::number(unit->y()));
 item->setText(d->mZ, QString::number(unit->z()));
 item->setText(d->mWork, QString::number((int)unit->work()));
 item->setText(d->mName, unit->name());
 item->setText(d->mHealth, QString::number(unit->health()));
 item->setText(d->mSpeed, QString::number(unit->speed()));
 item->setText(d->mWidth, QString::number(unit->width()));
 item->setText(d->mHeight, QString::number(unit->height()));

 QRect r = unit->boundingRect();
 item->setText(d->mBoundingRect, QString("%1,%2,%3,%4").arg(r.x()).arg(r.y()).arg(r.width()).arg(r.height()));
}

void KGameUnitDebug::slotUnitPropertyChanged(KGamePropertyBase* prop)
{
// again, our evil hack. See Player::slotUnitPropertyChanged()
 Unit* unit = 0;
 QPtrList<Unit> units;
 QPtrList<KPlayer>list = *d->mBoson->playerList();
 for (unsigned int i = 0; i < d->mBoson->playerCount() && !unit; i++) {
	QPtrList<Unit> playerUnits = ((Player*)list.at(i))->allUnits();
	for (unit = playerUnits.first(); unit; unit = playerUnits.next()) {
		if (unit->dataHandler() == (KGamePropertyHandler*)sender()) {
			break;
		}
		
	}
 }
 if (!unit) {
	kdWarning() << k_funcinfo << "unit not found" << endl;
	return;
 }

 switch (prop->id()) {
	case Unit::IdReloadState:
		break;
	default:
		update(d->mItems[unit], unit);
		break;
 }
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

