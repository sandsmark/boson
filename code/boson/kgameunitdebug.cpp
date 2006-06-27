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
#include "kgameunitdebug.h"
#include "kgameunitdebug.moc"

#include "../bomemory/bodummymemory.h"
#include "gameengine/boson.h"
#include "gameengine/cell.h"
#include "gameengine/unit.h"
#include "gameengine/unitplugins.h"
#include "gameengine/player.h"
#include "gameengine/boitemlist.h"
#include "gameengine/bosoncanvas.h"
#include "bodebug.h"
#include "bo3dtools.h"
#include "gameengine/bosonpropertyxml.h"
#include "qlistviewitemnumber.h"

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
#include <qsplitter.h>
#include <qpopupmenu.h>
#include <qcursor.h>


class KGameUnitDebug::KGameUnitDebugPrivate
{
public:
	KGameUnitDebugPrivate()
	{
		mBoson = 0;
		mUnitList = 0;
		mProperties = 0;
		mWeaponProperties = 0;
		mProduction = 0;
		mUnitsInRange = 0;
		mUnitCollisions = 0;
		mCells = 0;
	}

	Boson* mBoson;

	KListView* mUnitList;
	KListView* mProperties;
	KListView* mWeaponProperties;
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
	int mAdvanceWork;
	int mWidth;
	int mHeight;
	int mBoundingRect;
};

KGameUnitDebug::KGameUnitDebug(QWidget* parent) : QWidget(parent)
{
 d = new KGameUnitDebugPrivate;
 QVBoxLayout* topLayout = new QVBoxLayout(this);
 QSplitter* splitter = new QSplitter(this);
 topLayout->addWidget(splitter);

 d->mUnitList = new KListView(splitter);
 d->mUnitList->setAllColumnsShowFocus(true);
 d->mId = d->mUnitList->addColumn(i18n("Id"));
 d->mOwner = d->mUnitList->addColumn(i18n("Owner"));
 d->mRTTI = d->mUnitList->addColumn(i18n("RTTI"));
 d->mX = d->mUnitList->addColumn(i18n("X"));
 d->mY = d->mUnitList->addColumn(i18n("Y"));
 d->mZ = d->mUnitList->addColumn(i18n("Z"));
 d->mAdvanceWork = d->mUnitList->addColumn(i18n("AdvanceWork"));
 d->mName = d->mUnitList->addColumn(i18n("Name"));
 d->mHealth = d->mUnitList->addColumn(i18n("Health"));
// d->mUnitList->addColumn(i18n("Costs"));
 d->mSpeed = d->mUnitList->addColumn(i18n("Speed"));
// connect(d->mUnitList, SIGNAL(executed(QListBoxItem*)),
//		this, SLOT(slotSelectUnit(QListBoxItem*)));
 d->mWidth = d->mUnitList->addColumn(i18n("Width"));
 d->mHeight = d->mUnitList->addColumn(i18n("Height"));
 d->mBoundingRect = d->mUnitList->addColumn(i18n("BoundingRect"));
 connect(d->mUnitList, SIGNAL(selectionChanged(QListViewItem*)),
		this, SLOT(slotUnitSelected(QListViewItem*)));
 connect(d->mUnitList, SIGNAL(contextMenuRequested(QListViewItem*, const QPoint&, int)),
		this, SLOT(slotUnitListMenu(QListViewItem*, const QPoint&, int)));

 QSplitter* propertiesSplitter = new QSplitter(Vertical, splitter);
 QWidget* propertiesBox = new QWidget(propertiesSplitter);
 QVBoxLayout* propertiesLayout = new QVBoxLayout(propertiesBox);
 QLabel* propertiesTitle = new QLabel(i18n("Properties"), propertiesBox);
 propertiesLayout->addWidget(propertiesTitle, 0);
 d->mProperties = new KListView(propertiesBox);
 d->mProperties->setAllColumnsShowFocus(true);
 d->mProperties->addColumn(i18n("Property"));
 d->mProperties->addColumn(i18n("Id"));
 d->mProperties->addColumn(i18n("Value"));
 propertiesLayout->addWidget(d->mProperties, 1);

 QWidget* weaponPropertiesBox = new QWidget(propertiesSplitter);
 QVBoxLayout* weaponPropertiesLayout = new QVBoxLayout(weaponPropertiesBox);
 QLabel* weaponPropertiesTitle = new QLabel(i18n("Weapon Properties"), weaponPropertiesBox);
 weaponPropertiesLayout->addWidget(weaponPropertiesTitle, 0);
 d->mWeaponProperties = new KListView(weaponPropertiesBox);
 d->mWeaponProperties->setAllColumnsShowFocus(true);
 d->mWeaponProperties->addColumn(i18n("Property"));
 d->mWeaponProperties->addColumn(i18n("Id"));
 d->mWeaponProperties->addColumn(i18n("Value"));
 weaponPropertiesLayout->addWidget(d->mWeaponProperties, 1);

 QVBox* vbox = new QVBox(splitter);
 QWidget* productionBox = new QWidget(vbox);
 QVBoxLayout* productionLayout = new QVBoxLayout(productionBox);
 QLabel* productionTitle = new QLabel(i18n("Productions"), productionBox);
 productionLayout->addWidget(productionTitle, 0);
 d->mProduction = new KListView(productionBox);
 d->mProduction->setAllColumnsShowFocus(true);
 d->mProduction->addColumn(i18n("Number"));
 d->mProduction->addColumn(i18n("TypeId"));
 d->mProduction->addColumn(i18n("ETA"));
 productionLayout->addWidget(d->mProduction, 1);

 QVGroupBox* collisionsBox = new QVGroupBox(i18n("Collisions"), splitter);
 QVBox* inRangeWidget = new QVBox(collisionsBox);
 (void)new QLabel(i18n("InRange:"), inRangeWidget);
 d->mUnitsInRange = new KListView(inRangeWidget);
 d->mUnitsInRange->setAllColumnsShowFocus(true);
 d->mUnitsInRange->addColumn(i18n("ID"));
 d->mUnitsInRange->addColumn(i18n("Enemy"));
 QVBox* unitCollisionsWidget = new QVBox(collisionsBox);
 (void)new QLabel(i18n("UnitCollisions:"), unitCollisionsWidget);
 d->mUnitCollisions = new KListView(unitCollisionsWidget);
 d->mUnitCollisions->setAllColumnsShowFocus(true);
 d->mUnitCollisions->addColumn(i18n("ID"));
 d->mUnitCollisions->addColumn(i18n("Exact"));

 QWidget* cellsBox = new QWidget(splitter);
 QVBoxLayout* cellsLayout = new QVBoxLayout(cellsBox);
 QLabel* cellsTitle = new QLabel(i18n("Cells"), cellsBox);
 cellsLayout->addWidget(cellsTitle, 0);
 d->mCells = new KListView(cellsBox);
 d->mCells->setAllColumnsShowFocus(true);
 d->mCells->addColumn(i18n("X"));
 d->mCells->addColumn(i18n("Y"));
 cellsLayout->addWidget(d->mCells, 1);

 QPushButton* update = new QPushButton(i18n("Update"), this);
 connect(update, SIGNAL(pressed()), this, SLOT(slotUpdate()));
 topLayout->addWidget(update);
}

KGameUnitDebug::~KGameUnitDebug()
{
 d->mUnits.clear();
 d->mItems.clear();
 d->mProperties->clear();
 d->mWeaponProperties->clear();
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
 d->mUnits.clear();
 d->mItems.clear();
 d->mProperties->clear();
 d->mWeaponProperties->clear();
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
 QListViewItem* item = new QListViewItemNumber(d->mUnitList);
 d->mItems.insert(unit, item);
 connect(unit->dataHandler(), SIGNAL(signalPropertyChanged(KGamePropertyBase*)),
		this, SLOT(slotUnitPropertyChanged(KGamePropertyBase*)));
 update(item, unit);
}

void KGameUnitDebug::update(QListViewItem* item, Unit* unit)
{
 item->setText(d->mId, QString::number(unit->id()));
 item->setText(d->mOwner, QString::number(unit->owner() ? unit->owner()->bosonId() : 0));
 item->setText(d->mRTTI, QString::number(unit->rtti()));
 item->setText(d->mX, QString::number(unit->x()));
 item->setText(d->mY, QString::number(unit->y()));
 item->setText(d->mZ, QString::number(unit->z()));
 item->setText(d->mAdvanceWork, QString::number((int)unit->advanceWork()));
 item->setText(d->mName, unit->name());
 item->setText(d->mHealth, QString::number(unit->health()));
 item->setText(d->mSpeed, QString::number(unit->speed()));
 item->setText(d->mWidth, QString::number(unit->width()));
 item->setText(d->mHeight, QString::number(unit->height()));

 BoRect2Fixed r = unit->boundingRect();
 item->setText(d->mBoundingRect, QString("%1,%2,%3,%4").
		arg(r.x()).
		arg(r.y()).
		arg(r.width()).
		arg(r.height()));
}

void KGameUnitDebug::slotUnitPropertyChanged(KGamePropertyBase* prop)
{
// again, our evil hack. See Player::slotUnitPropertyChanged()
 Unit* unit = 0;
 QPtrList<Unit> units;
 QPtrList<Player> list = *d->mBoson->gamePlayerList();
 for (unsigned int i = 0; i < d->mBoson->gamePlayerCount() && !unit; i++) {
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

void KGameUnitDebug::slotUnitListMenu(QListViewItem*, const QPoint&, int)
{
 QPopupMenu menu(this);
 QPopupMenu columns(this);
 columns.setCheckable(true);

 for (int i = 0; i < d->mUnitList->columns(); i++) {
	QString text = d->mUnitList->columnText(i);
	bool hidden = (d->mUnitList->columnWidth(i) == 0);
	int id = columns.insertItem(text, this, SLOT(slotUnitListToggleShowColumn(int)));
	columns.setItemParameter(id, i);
	columns.setItemChecked(id, !hidden);
 }
 menu.insertItem(i18n("Show Column"), &columns);
 menu.exec(QCursor::pos());
}

void KGameUnitDebug::slotUnitListToggleShowColumn(int index)
{
 if (index < 0) {
	boError() << k_funcinfo << "index < 0: index=" << index << endl;
	return;
 }
 if (index >= d->mUnitList->columns()) {
	boError() << k_funcinfo << "index >= number of columns: index=" << index << endl;
	return;
 }
 bool hidden = (d->mUnitList->columnWidth(index) == 0);
 if (hidden) {
	d->mUnitList->setColumnWidthMode(index, QListView::Maximum);
	d->mUnitList->adjustColumn(index);
 } else {
	d->mUnitList->setColumnWidthMode(index, QListView::Manual);
	d->mUnitList->hideColumn(index);
 }
}

void KGameUnitDebug::slotUnitSelected(QListViewItem* item)
{
 Unit* unit = 0;
 if (item) {
	int id = item->text(0).toInt();
	unit = d->mUnits[id];
	if (!unit) {
		boWarning() << k_funcinfo << "id " << id << " not found" << endl;
	}
 }
 updateProduction(unit);
 updateUnitsInRange(unit);
 updateUnitCollisions(unit);
 updateCells(unit);
 updateProperties(unit);
}

void KGameUnitDebug::updateProperties(Unit* unit)
{
 d->mProperties->clear();
 d->mWeaponProperties->clear();
 if (!unit) {
	return;
 }

 BosonCustomPropertyXML propertyXML;
 {
	KGamePropertyHandler* dataHandler = unit->dataHandler();
	QIntDict<KGamePropertyBase>& dict = dataHandler->dict();
	QIntDictIterator<KGamePropertyBase> it(dict);
	while (it.current()) {
		QString name = dataHandler->propertyName(it.current()->id());
		QString id = QString::number(it.current()->id());
		QString value = propertyXML.propertyValue(it.current());
		QListViewItemNumber* item = new QListViewItemNumber(d->mProperties);
		if (name.isEmpty()) {
			name = i18n("Unknown");
		}
		item->setText(0, name);
		item->setText(1, id);
		item->setText(2, value);
		++it;
	}
 }

 {
	KGamePropertyHandler* weaponDataHandler = unit->weaponDataHandler();
	QIntDict<KGamePropertyBase>& dict = weaponDataHandler->dict();
	QIntDictIterator<KGamePropertyBase> it(dict);
	while (it.current()) {
		QString name = weaponDataHandler->propertyName(it.current()->id());
		QString id = QString::number(it.current()->id());
		QString value = propertyXML.propertyValue(it.current());
		QListViewItemNumber* item = new QListViewItemNumber(d->mWeaponProperties);
		if (name.isEmpty()) {
			name = i18n("Unknown");
		}
		item->setText(0, name);
		item->setText(1, id);
		item->setText(2, value);
		++it;
	}
 }
}

void KGameUnitDebug::updateProduction(Unit* unit)
{
 d->mProduction->clear();
 if (!unit) {
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

void KGameUnitDebug::updateUnitsInRange(Unit* unit)
{
 d->mUnitsInRange->clear();
 if (!unit) {
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

void KGameUnitDebug::updateUnitCollisions(Unit* unit)
{
 d->mUnitCollisions->clear();
 if (!unit) {
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

void KGameUnitDebug::updateCells(Unit* unit)
{
 d->mCells->clear();
 if (!unit) {
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


