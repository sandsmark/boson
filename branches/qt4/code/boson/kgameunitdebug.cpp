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
#include "kgameunitdebug.h"
#include "kgameunitdebug.moc"

#include "../bomemory/bodummymemory.h"
#include "gameengine/boson.h"
#include "gameengine/cell.h"
#include "gameengine/unit.h"
#include "gameengine/unitplugins/productionplugin.h"
#include "gameengine/player.h"
#include "gameengine/boitemlist.h"
#include "gameengine/bosoncanvas.h"
#include "gameengine/bosonitempropertyhandler.h"
#include "bodebug.h"
#include "bo3dtools.h"
#include "gameengine/bosonpropertyxml.h"
#include "gameengine/bosonpath.h"
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
#include <qtabwidget.h>

class KGameUnitDebugItemListPrivate
{
public:
	KGameUnitDebugItemListPrivate()
	{
		mItemList = 0;
	}
	KListView* mItemList;
	int mIndexId;
	int mIndexOwner;
	int mIndexRTTI;
	int mIndexName;
	int mIndexX;
	int mIndexY;
	int mIndexZ;

	QMap<BosonItem*, QListViewItem*> mGameItem2ListItem;
	QMap<QListViewItem*, BosonItem*> mListItem2GameItem;
};

KGameUnitDebugItemList::KGameUnitDebugItemList(QWidget* parent)
	: QWidget(parent)
{
 d = new KGameUnitDebugItemListPrivate;
 QVBoxLayout* layout = new QVBoxLayout(this);
 d->mItemList = new KListView(this);
 layout->addWidget(d->mItemList);

// connect(d->mItemList, SIGNAL(executed(QListBoxItem*)),
//		this, SLOT(slotSelectUnit(QListBoxItem*)));
 connect(d->mItemList, SIGNAL(selectionChanged(QListViewItem*)),
		this, SLOT(slotSelected(QListViewItem*)));
 connect(d->mItemList, SIGNAL(contextMenuRequested(QListViewItem*, const QPoint&, int)),
		this, SLOT(slotItemListMenu(QListViewItem*, const QPoint&, int)));


 d->mItemList->setAllColumnsShowFocus(true);

 d->mIndexId = d->mItemList->addColumn(i18n("Id"));
 d->mIndexOwner = d->mItemList->addColumn(i18n("Owner"));
 d->mIndexRTTI = d->mItemList->addColumn(i18n("RTTI"));
 d->mIndexName = d->mItemList->addColumn(i18n("Name"));
 d->mIndexX = d->mItemList->addColumn(i18n("CenterX"));
 d->mIndexY = d->mItemList->addColumn(i18n("CenterY"));
 d->mIndexZ = d->mItemList->addColumn(i18n("Z"));

 // hide some columns by defaul
 QValueList<int> hidden;
 hidden.append(d->mIndexX);
 hidden.append(d->mIndexY);
 hidden.append(d->mIndexZ);
 for (QValueList<int>::iterator it = hidden.begin(); it != hidden.end(); ++it) {
	d->mItemList->setColumnWidthMode(*it, QListView::Manual);
	d->mItemList->hideColumn(*it);
 }
}

KGameUnitDebugItemList::~KGameUnitDebugItemList()
{
 clear();
 delete d;
}

void KGameUnitDebugItemList::slotItemListMenu(QListViewItem*, const QPoint&, int)
{
 QPopupMenu menu(this);
 QPopupMenu columns(this);
 columns.setCheckable(true);

 for (int i = 0; i < d->mItemList->columns(); i++) {
	QString text = d->mItemList->columnText(i);
	bool hidden = (d->mItemList->columnWidth(i) == 0);
	int id = columns.insertItem(text, this, SLOT(slotItemListToggleShowColumn(int)));
	columns.setItemParameter(id, i);
	columns.setItemChecked(id, !hidden);
 }
 menu.insertItem(i18n("Show Column"), &columns);
 menu.exec(QCursor::pos());
}

void KGameUnitDebugItemList::slotItemListToggleShowColumn(int index)
{
 if (index < 0) {
	boError() << k_funcinfo << "index < 0: index=" << index << endl;
	return;
 }
 if (index >= d->mItemList->columns()) {
	boError() << k_funcinfo << "index >= number of columns: index=" << index << endl;
	return;
 }
 bool hidden = (d->mItemList->columnWidth(index) == 0);
 if (hidden) {
	d->mItemList->setColumnWidthMode(index, QListView::Maximum);
	d->mItemList->adjustColumn(index);
 } else {
	d->mItemList->setColumnWidthMode(index, QListView::Manual);
	d->mItemList->hideColumn(index);
 }
}

void KGameUnitDebugItemList::clear()
{
 d->mItemList->clear();
 d->mGameItem2ListItem.clear();
 d->mListItem2GameItem.clear();
}

void KGameUnitDebugItemList::addItem(BosonItem* gameItem)
{
 QListViewItem* listItem = new QListViewItemNumber(d->mItemList);
 d->mGameItem2ListItem.insert(gameItem, listItem);
 d->mListItem2GameItem.insert(listItem, gameItem);

 disconnect(gameItem->dataHandler(), SIGNAL(signalPropertyChanged(KGamePropertyBase*)),
		this, SLOT(slotItemPropertyChanged(KGamePropertyBase*)));
 connect(gameItem->dataHandler(), SIGNAL(signalPropertyChanged(KGamePropertyBase*)),
		this, SLOT(slotItemPropertyChanged(KGamePropertyBase*)));

 update(gameItem);
}

void KGameUnitDebugItemList::update(BosonItem* gameItem)
{
 QListViewItem* listItem = d->mGameItem2ListItem[gameItem];
 BO_CHECK_NULL_RET(listItem);

 listItem->setText(d->mIndexId, QString::number(gameItem->id()));
 listItem->setText(d->mIndexOwner, QString::number(gameItem->owner() ? gameItem->owner()->bosonId() : 0));
 listItem->setText(d->mIndexRTTI, QString::number(gameItem->rtti()));
 QString name;
 if (RTTI::isUnit(gameItem->rtti())) {
	Unit* u = (Unit*)gameItem;
	name = u->name();
 } else {
	name = i18n("(Unknown)");
 }
 listItem->setText(d->mIndexName, name);
 listItem->setText(d->mIndexX, QString::number(gameItem->centerX()));
 listItem->setText(d->mIndexY, QString::number(gameItem->centerY()));
 listItem->setText(d->mIndexZ, QString::number(gameItem->z()));
}

void KGameUnitDebugItemList::update(const BosonCanvas* canvas)
{
 clear();

 if (!canvas) {
	return;
 }

 for (BoItemList::ConstIterator it = canvas->allItems()->begin(); it != canvas->allItems()->end(); ++it) {
	addItem(*it);
 }
}

void KGameUnitDebugItemList::updateProperty(BosonItem* item, KGamePropertyBase* prop)
{
 BO_CHECK_NULL_RET(item);
 BO_CHECK_NULL_RET(prop);

 Q_UNUSED(prop);

 update(item);
}

void KGameUnitDebugItemList::slotSelected(QListViewItem* item)
{
 if (!item) {
	return;
 }
 BosonItem* gameItem = d->mListItem2GameItem[item];
 if (gameItem) {
	emit signalItemSelected(gameItem);
 }
}

void KGameUnitDebugItemList::slotItemPropertyChanged(KGamePropertyBase* prop)
{
 BO_CHECK_NULL_RET(sender());
 if (!sender()->isA("BosonItemPropertyHandler")) {
	boError() << k_funcinfo << "sender() is not a BosonItemPropertyHandler" << endl;
	return;
 }
 BosonItemPropertyHandler* p = (BosonItemPropertyHandler*)sender();
 if (!p->item()) {
	boError() << k_funcinfo << "NULL parent item for property handler" << endl;
	return;
 }
 BosonItem* item = (BosonItem*)p->item();

 updateProperty(item, prop);
}


KGameUnitDebugDataHandlerDisplay::KGameUnitDebugDataHandlerDisplay(QWidget* parent)
	: QWidget(parent)
{
 mProperties = new KListView(this);
 mProperties->setAllColumnsShowFocus(true);
 mProperties->addColumn(i18n("Property"));
 mProperties->addColumn(i18n("Id"));
 mProperties->addColumn(i18n("Value"));

 QVBoxLayout* layout = new QVBoxLayout(this);
 layout->addWidget(mProperties);
}

KGameUnitDebugDataHandlerDisplay::~KGameUnitDebugDataHandlerDisplay()
{
 mProperties->clear();
}

void KGameUnitDebugDataHandlerDisplay::clear()
{
 displayDataHandler(0);
}

void KGameUnitDebugDataHandlerDisplay::displayDataHandler(KGamePropertyHandler* dataHandler)
{
 mProperties->clear();
 if (!dataHandler) {
	return;
 }
 BosonCustomPropertyXML propertyXML;
 QIntDict<KGamePropertyBase>& dict = dataHandler->dict();
 for (QIntDictIterator<KGamePropertyBase> it(dict); it.current(); ++it) {
	QString name = dataHandler->propertyName(it.current()->id());
	QString id = QString::number(it.current()->id());
	QString value = propertyXML.propertyValue(it.current());
	QListViewItemNumber* item = new QListViewItemNumber(mProperties);
	if (name.isEmpty()) {
		name = i18n("Unknown");
	}
	item->setText(0, name);
	item->setText(1, id);
	item->setText(2, value);
 }
}


class KGameUnitDebugPrivate
{
public:
	KGameUnitDebugPrivate()
	{
		mBoson = 0;
		mItemList = 0;
		mItemProperties = 0;
		mUnitWeaponProperties = 0;
		mProduction = 0;
		mUnitCollisions = 0;
		mCells = 0;
		mPathInfo = 0;
	}

	Boson* mBoson;

	QTabWidget* mTabWidget;

	KGameUnitDebugItemList* mItemList;
	KGameUnitDebugDataHandlerDisplay* mItemProperties;
	KGameUnitDebugDataHandlerDisplay* mUnitWeaponProperties;
	KListView* mProduction;
	KListView* mUnitCollisions;
	KListView* mCells;
	KListView* mPathInfo;
};

KGameUnitDebug::KGameUnitDebug(QWidget* parent) : QWidget(parent)
{
 d = new KGameUnitDebugPrivate;
 QVBoxLayout* topLayout = new QVBoxLayout(this);
 QSplitter* splitter = new QSplitter(this);
 topLayout->addWidget(splitter);

 d->mItemList = new KGameUnitDebugItemList(splitter);
 connect(d->mItemList, SIGNAL(signalItemSelected(BosonItem*)),
		this, SLOT(slotItemSelected(BosonItem*)));

 d->mTabWidget = new QTabWidget(splitter);

 addPropertiesPage();
 addCellsPage();
 addCollisionsPage();
 addProductionsPage();
 addPathInfoPage();

 QPushButton* update = new QPushButton(i18n("Update"), this);
 connect(update, SIGNAL(pressed()), this, SLOT(slotUpdate()));
 topLayout->addWidget(update);
}

KGameUnitDebug::~KGameUnitDebug()
{
 d->mItemProperties->clear();
 d->mUnitWeaponProperties->clear();
 d->mProduction->clear();
 d->mUnitCollisions->clear();
 d->mCells->clear();
 d->mPathInfo->clear();
 delete d;
}

void KGameUnitDebug::addPropertiesPage()
{
 QSplitter* propertiesSplitter = new QSplitter(Vertical, d->mTabWidget);
 QWidget* propertiesBox = new QWidget(propertiesSplitter);
 QVBoxLayout* propertiesLayout = new QVBoxLayout(propertiesBox);
 QLabel* propertiesTitle = new QLabel(i18n("Properties"), propertiesBox);
 propertiesLayout->addWidget(propertiesTitle, 0);
 d->mItemProperties = new KGameUnitDebugDataHandlerDisplay(propertiesBox);
 propertiesLayout->addWidget(d->mItemProperties, 1);

 QWidget* weaponPropertiesBox = new QWidget(propertiesSplitter);
 QVBoxLayout* weaponPropertiesLayout = new QVBoxLayout(weaponPropertiesBox);
 QLabel* weaponPropertiesTitle = new QLabel(i18n("Weapon Properties"), weaponPropertiesBox);
 weaponPropertiesLayout->addWidget(weaponPropertiesTitle, 0);
 d->mUnitWeaponProperties = new KGameUnitDebugDataHandlerDisplay(weaponPropertiesBox);
 weaponPropertiesLayout->addWidget(d->mUnitWeaponProperties, 1);
 d->mTabWidget->addTab(propertiesSplitter, i18n("Properties"));
}

void KGameUnitDebug::addProductionsPage()
{
 d->mProduction = new KListView(d->mTabWidget);
 d->mProduction->setAllColumnsShowFocus(true);
 d->mProduction->addColumn(i18n("Number"));
 d->mProduction->addColumn(i18n("TypeId"));
 d->mProduction->addColumn(i18n("ETA"));
 d->mTabWidget->addTab(d->mProduction, i18n("Productions"));
 d->mTabWidget->setTabEnabled(d->mProduction, false);
}

void KGameUnitDebug::addCollisionsPage()
{
 QVBox* collisionsBox = new QVBox(d->mTabWidget);
 QVBox* unitCollisionsWidget = new QVBox(collisionsBox);
 (void)new QLabel(i18n("UnitCollisions:"), unitCollisionsWidget);
 d->mUnitCollisions = new KListView(unitCollisionsWidget);
 d->mUnitCollisions->setAllColumnsShowFocus(true);
 d->mUnitCollisions->addColumn(i18n("ID"));
 d->mUnitCollisions->addColumn(i18n("Exact"));
 d->mTabWidget->addTab(collisionsBox, i18n("Collisions"));
}

void KGameUnitDebug::addCellsPage()
{
 QWidget* cellsBox = new QWidget(d->mTabWidget);
 QVBoxLayout* cellsLayout = new QVBoxLayout(cellsBox);
 QLabel* cellsTitle = new QLabel(i18n("Cells"), cellsBox);
 cellsLayout->addWidget(cellsTitle, 0);
 d->mCells = new KListView(cellsBox);
 d->mCells->setAllColumnsShowFocus(true);
 d->mCells->addColumn(i18n("X"));
 d->mCells->addColumn(i18n("Y"));
 cellsLayout->addWidget(d->mCells, 1);
 d->mTabWidget->addTab(cellsBox, i18n("Cells"));
}

void KGameUnitDebug::addPathInfoPage()
{
 d->mPathInfo = new KListView(d->mTabWidget);
 d->mPathInfo->setAllColumnsShowFocus(true);
 d->mPathInfo->addColumn(i18n("Name"));
 d->mPathInfo->addColumn(i18n("Value"));
// d->mPathInfo->addColumn(i18n("Number"));
// d->mPathInfo->addColumn(i18n("TypeId"));
// d->mPathInfo->addColumn(i18n("ETA"));
 d->mTabWidget->addTab(d->mPathInfo, i18n("PathInfo"));
 d->mTabWidget->setTabEnabled(d->mPathInfo, false);
}

void KGameUnitDebug::setBoson(Boson* b)
{
 d->mBoson = b;
 slotUpdate();
}

void KGameUnitDebug::slotUpdate()
{
 d->mItemList->clear();
 d->mItemProperties->clear();
 d->mUnitWeaponProperties->clear();
 d->mProduction->clear();
 d->mUnitCollisions->clear();
 d->mCells->clear();
 d->mPathInfo->clear();

 if (!d->mBoson) {
	return;
 }
 if (!d->mBoson->canvas()) {
	return;
 }
 d->mItemList->update(d->mBoson->canvas());
}

void KGameUnitDebug::slotItemSelected(BosonItem* item)
{
 BO_CHECK_NULL_RET(item);

 updateCells(item);
 updateProperties(item);
 updateProduction(item);
 updateUnitCollisions(item);
 updatePathInfo(item);

 bool isUnit = false;
 if (RTTI::isUnit(item->rtti())) {
	isUnit = true;
 }
 d->mTabWidget->setTabEnabled(d->mProduction, isUnit);
 d->mTabWidget->setTabEnabled(d->mPathInfo, isUnit);
}

void KGameUnitDebug::updateProperties(BosonItem* item)
{
 d->mItemProperties->clear();
 d->mUnitWeaponProperties->clear();
 if (!item) {
	return;
 }
 Unit* unit = 0;
 if (RTTI::isUnit(item->rtti())) {
	unit = (Unit*)item;
 }

 KGamePropertyHandler* weaponDataHandler = 0;
 d->mItemProperties->displayDataHandler(item->dataHandler());
 if (unit) {
	weaponDataHandler = unit->weaponDataHandler();
 }
 d->mUnitWeaponProperties->displayDataHandler(weaponDataHandler);
}

void KGameUnitDebug::updateProduction(BosonItem* item)
{
 d->mProduction->clear();
 if (!item || !RTTI::isUnit(item->rtti())) {
	return;
 }
 Unit* unit = (Unit*)item;
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

void KGameUnitDebug::updateUnitCollisions(BosonItem* item)
{
 d->mUnitCollisions->clear();
 if (!item || !RTTI::isUnit(item->rtti())) {
	return;
 }
 Unit* unit = (Unit*)item;
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

void KGameUnitDebug::updateCells(BosonItem* item)
{
 d->mCells->clear();
 if (!item) {
	return;
 }
 const QPtrVector<Cell>* cells = item->cells();
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

void KGameUnitDebug::updatePathInfo(BosonItem* item)
{
 d->mPathInfo->clear();
 if (!item || !RTTI::isUnit(item->rtti())) {
	return;
 }
 Unit* unit = (Unit*)item;

 BosonPathInfo* info = unit->pathInfo();
 if (!info) {
	return;
 }

 QListViewItem* start = new QListViewItem(d->mPathInfo, i18n("Start"));
 start->setText(1, i18n("(%1;%2)").arg(info->start.x()).arg(info->start.y()));
 QListViewItem* dest = new QListViewItem(d->mPathInfo, i18n("Dest"));
 dest->setText(1, i18n("(%1;%2)").arg(info->dest.x()).arg(info->dest.y()));
 new QListViewItem(d->mPathInfo, i18n("Range"), QString::number(info->range));
 QListViewItem* target = new QListViewItem(d->mPathInfo, i18n("Target"));
 if (info->target) {
	target->setText(1, QString::number(info->target->id()));
 }

 new QListViewItem(d->mPathInfo, i18n("Waiting"), QString::number(info->waiting));
 new QListViewItem(d->mPathInfo, i18n("PathRecalced"), QString::number(info->pathrecalced));

}

