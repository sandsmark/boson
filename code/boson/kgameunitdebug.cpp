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
#include "cell.h"
#include "unit.h"
#include "unitplugins.h"
#include "player.h"
#include "boitemlist.h"
#include "bosoncanvas.h"
#include "bodebug.h"
#include "bo3dtools.h"

#include <klistview.h>
#include <klistbox.h>
#include <klocale.h>
#include <kgame/kgamepropertyhandler.h>

#include <qlayout.h>
#include <qpushbutton.h>
#include <qintdict.h>
#include <qlabel.h>
#include <qvgroupbox.h>
#include <qvbox.h>
#include <qpointarray.h>
#include <qptrdict.h>
#include <qptrvector.h>

#include "kgameunitdebug.moc"

class KGameUnitDebug::KGameUnitDebugPrivate
{
public:
	KGameUnitDebugPrivate()
	{
		mBoson = 0;
		mUnitList = 0;
		mWaypoints = 0;
		mProduction = 0;
		mUnitsInRange = 0;
		mUnitCollisions = 0;
		mCells = 0;
	}

	Boson* mBoson;

	KListView* mUnitList;
	KListBox* mWaypoints;
	KListView* mProduction;
	KListView* mUnitsInRange;
	KListView* mUnitCollisions;
	KListView* mCells;

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
 QVBoxLayout* waypointLayout = new QVBoxLayout(l);
 QLabel* waypointTitle = new QLabel(i18n("Waypoints"), this);
 waypointLayout->addWidget(waypointTitle, 0);
 d->mWaypoints = new KListBox(this);
 connect(d->mUnitList, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(updateWaypoints(QListViewItem*)));
 waypointLayout->addWidget(d->mWaypoints, 1);

 QVBoxLayout* productionLayout = new QVBoxLayout(l);
 QLabel* productionTitle = new QLabel(i18n("Productions"), this);
 productionLayout->addWidget(productionTitle, 0);
 d->mProduction = new KListView(this);
 d->mProduction->addColumn(i18n("Number"));
 d->mProduction->addColumn(i18n("TypeId"));
 d->mProduction->addColumn(i18n("ETA"));
 connect(d->mUnitList, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(updateProduction(QListViewItem*)));
 productionLayout->addWidget(d->mProduction, 1);

 QVGroupBox* collisionsBox = new QVGroupBox(i18n("Collisions"), this);
 layout->addWidget(collisionsBox);
 QVBox* inRangeWidget = new QVBox(collisionsBox);
 (void)new QLabel(i18n("InRange:"), inRangeWidget);
 d->mUnitsInRange = new KListView(inRangeWidget);
 d->mUnitsInRange->addColumn(i18n("ID"));
 d->mUnitsInRange->addColumn(i18n("Enemy"));
 connect(d->mUnitList, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(updateUnitsInRange(QListViewItem*)));
 QVBox* unitCollisionsWidget = new QVBox(collisionsBox);
 (void)new QLabel(i18n("UnitCollisions:"), unitCollisionsWidget);
 d->mUnitCollisions = new KListView(unitCollisionsWidget);
 d->mUnitCollisions->addColumn(i18n("ID"));
 d->mUnitCollisions->addColumn(i18n("Exact"));
 connect(d->mUnitList, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(updateUnitCollisions(QListViewItem*)));

 d->mCells = new KListView(this);
 layout->addWidget(d->mCells);
 d->mCells->addColumn(i18n("X"));
 d->mCells->addColumn(i18n("Y"));
 connect(d->mUnitList, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(updateCells(QListViewItem*)));


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
 d->mUnits.clear();
 d->mItems.clear();
 d->mWaypoints->clear();
 d->mProduction->clear();
 d->mUnitsInRange->clear();
 d->mUnitCollisions->clear();
 d->mCells->clear();
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
 d->mUnitsInRange->clear();
 d->mUnitCollisions->clear();
 d->mCells->clear();
 if (!d->mBoson) {
	return;
 }
 if (!d->mBoson->canvas()) {
	return;
 }

 QPtrList<Unit> units;
 BoItemList::ConstIterator itemIt = boGame->canvas()->allItems()->begin();
 for (; itemIt != boGame->canvas()->allItems()->end(); ++itemIt) {
	if (RTTI::isUnit((*itemIt)->rtti())) {
		units.append((Unit*)*itemIt);
	}
 }

 QPtrListIterator<Unit> it(units);
 while (it.current()) {
	if (d->mUnits.find(it.current()->id())) {
		boError() << "Cannot double-add id " << it.current()->id() << endl;
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

 BoRectFixed r = unit->boundingRect();
 item->setText(d->mBoundingRect, QString("%1,%2,%3,%4").arg(r.x()).arg(r.y()).arg(r.width()).arg(r.height()));
}

void KGameUnitDebug::slotUnitPropertyChanged(KGamePropertyBase* prop)
{
// again, our evil hack. See Player::slotUnitPropertyChanged()
 Unit* unit = 0;
 QPtrList<Unit> units;
 QPtrList<KPlayer>list = *d->mBoson->playerList();
 for (unsigned int i = 0; i < d->mBoson->playerCount() && !unit; i++) {
	QPtrList<Unit> playerUnits = *((Player*)list.at(i))->allUnits();
	for (unit = playerUnits.first(); unit; unit = playerUnits.next()) {
		if (unit->dataHandler() == (KGamePropertyHandler*)sender()) {
			break;
		}

	}
 }
 if (!unit) {
	boWarning() << k_funcinfo << "unit not found" << endl;
	return;
 }

 switch (prop->id()) {
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
	boWarning() << k_lineinfo << "id " << id << " not found" << endl;
	return;
 }
 QValueList<BoVector2Fixed> points = unit->waypointList();
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
	boWarning() << k_lineinfo << "id " << id << " not found" << endl;
	return;
 }
 ProductionPlugin* production = (ProductionPlugin*)unit->plugin(UnitPlugin::Production);
 if (production) {
	QValueList<QPair<ProductionType, unsigned long int> > productions = production->productionList();
	for (unsigned int i = 0; i < productions.count(); i++) {
		QListViewItem* item = new QListViewItem(d->mProduction);
		item->setText(0, QString::number(i+1));
		item->setText(1, QString::number(productions[i].second));
		item->setText(2, i18n("Ready")); // currently always ready
		// TODO: also show ProductionType
	}

 }
}

void KGameUnitDebug::updateUnitsInRange(QListViewItem* item)
{
 d->mUnitsInRange->clear();
 if (!item) {
	return;
 }
 int id = item->text(0).toInt();
 Unit* unit = d->mUnits[id];
 if (!unit) {
	boWarning() << k_lineinfo << "id " << id << " not found" << endl;
	return;
 }
 /*BoItemList inRange = unit->unitsInRange();
 BoItemList enemyInRange = unit->enemyUnitsInRange();
 if (inRange.count() == 0) */{
	QListViewItem* item = new QListViewItem(d->mUnitsInRange);
	item->setText(0, i18n("No units in range for unit %1").arg(unit->id()));
 }/*
 BoItemList::Iterator it = inRange.begin();
 for (; it != inRange.end(); ++it) {
	if (!RTTI::isUnit((*it)->rtti())) {
		BosonItem* i = (BosonItem*)*it;
		QListViewItem* item = new QListViewItem(d->mUnitsInRange);
		QString text = i18n("Item is not a unit rtti=%1 ; x=%2 ; y=%3 ; z=%4").arg(i->rtti()).arg(i->x()).arg(i->y()).arg(i->z());
		item->setText(0, text);
	}
	Unit* u = (Unit*)*it;
	QListViewItem* item = new QListViewItem(d->mUnitsInRange);
	item->setText(0, QString::number(u->id()));
	if (enemyInRange.contains(*it)) {
		item->setText(1, i18n("Yes"));
	} else {
		item->setText(1, i18n("No"));
	}
 }*/
}

void KGameUnitDebug::updateUnitCollisions(QListViewItem* item)
{
 d->mUnitCollisions->clear();
 if (!item) {
	return;
 }
 int id = item->text(0).toInt();
 Unit* unit = d->mUnits[id];
 if (!unit) {
	boWarning() << k_lineinfo << "id " << id << " not found" << endl;
	return;
 }
 QValueList<Unit*> collisionsFalse = unit->unitCollisions(false);
 QValueList<Unit*> collisionsTrue = unit->unitCollisions(false);
 if (collisionsFalse.count() == 0) {
	QListViewItem* item = new QListViewItem(d->mUnitCollisions);
	item->setText(0, i18n("No unit collisions for %1").arg(unit->id()));
 }

 QValueList<Unit*>::Iterator it = collisionsFalse.begin();
 for (; it != collisionsFalse.end(); ++it) {
	Unit* u = (Unit*)*it;
	QListViewItem* item = new QListViewItem(d->mUnitCollisions);
	item->setText(0, QString::number(u->id()));
	if (collisionsTrue.contains(*it)) {
		item->setText(1, i18n("True"));
	} else {
		item->setText(1, i18n("False"));
	}
 }
}

void KGameUnitDebug::updateCells(QListViewItem* item)
{
 d->mCells->clear();
 if (!item) {
	return;
 }
 int id = item->text(0).toInt();
 Unit* unit = d->mUnits[id];
 if (!unit) {
	boWarning() << k_lineinfo << "id " << id << " not found" << endl;
	return;
 }
 const QPtrVector<Cell>* cells = unit->cells();
 for (unsigned int i = 0; i < cells->count(); i++) {
	Cell* c = cells->at(i);
	if (!c) {
		boError() << k_funcinfo << "invalid cell at " << i << endl;
	}
	QListViewItem* item = new QListViewItem(d->mCells);
	item->setText(0, QString::number(c->x()));
	item->setText(1, QString::number(c->y()));
 }
}


