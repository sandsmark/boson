/*
    This file is part of the Boson game
    Copyright (C) 1999-2000 Thomas Capricelli (capricel@email.enst.fr)
    Copyright (C) 2001-2006 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2001-2006 Rivo Laks (rivolaks@hot.ee)

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

#include "unit.h"

#include "../bomemory/bodummymemory.h"
#include "player.h"
#include "playerio.h"
#include "bosoncanvas.h"
#include "cell.h"
#include "speciestheme.h"
#include "unitproperties.h"
#include "bosonpath.h"
#include "bosonstatistics.h"
#include "unitplugins/unitplugins.h"
#include "boitemlist.h"
#include "pluginproperties.h"
#include "bosonweapon.h"
#include "bopointeriterator.h"
#include "bodebug.h"
#include "bo3dtools.h"
#include "bosonprofiling.h"
#include "unitmover.h"
#include "unitorder.h"

#include <kgame/kgamepropertylist.h>
#include <kgame/kgame.h>
#include <krandomsequence.h>

#include <qptrlist.h>
#include <qdom.h>

#include <math.h>

#include "defines.h"


class UnitOrderQueuePrivate;
/**
 * A queue of UnitOrders.
 **/
class UnitOrderQueue
{
public:
	UnitOrderQueue(UnitOrdersInterface* parent, Unit* parentUnit);
	~UnitOrderQueue();

	UnitOrder* currentOrder() const;
	UnitOrderData* currentOrderData() const;

	UnitOrder* toplevelOrder() const;
	UnitOrderData* toplevelOrderData() const;

	void clearOrders();

	bool addToplevelOrder(UnitOrder* order);
	bool addCurrentSuborder(UnitOrder* order);

	bool replaceToplevelOrders(UnitOrder* order);

	bool lastOrderStatus() const;

	void currentSuborderDone(bool success);


	bool saveAsXML(QDomElement& root);
	bool loadFromXML(const QDomElement& root, BosonCanvas* canvas);

protected:
	bool currentOrderChanged();
	bool currentOrderAdded();
	void currentSuborderRemoved();
	bool canAddOrder() const;

	void toplevelOrderRemoved();

private:
	UnitOrdersInterface* mParent;
	Unit* mParentUnit;
	UnitOrderQueuePrivate* d;
};

class UnitOrderQueuePrivate
{
public:
	UnitOrderQueuePrivate()
	{
		mCurrentOrder = 0;
	}
	QValueList<UnitOrderData*> mToplevelOrders;
	UnitOrderData* mCurrentOrder;
	UnitOrder::FinishStatus mLastOrderStatus;
};

UnitOrderQueue::UnitOrderQueue(UnitOrdersInterface* parent, Unit* parentUnit)
{
 d = new UnitOrderQueuePrivate;
 d->mLastOrderStatus = UnitOrder::Success;
 mParent = parent;

 // AB: this pointer is not nice. this class should not need it.
 //     however it is used in clearOrders() once, atm.
 mParentUnit = parentUnit;
}

UnitOrderQueue::~UnitOrderQueue()
{
 delete d;
}

UnitOrder* UnitOrderQueue::currentOrder() const
{
 if (d->mCurrentOrder) {
	return d->mCurrentOrder->order();
 } else {
	return 0;
 }
}

UnitOrderData* UnitOrderQueue::currentOrderData() const
{
 return d->mCurrentOrder;
}

UnitOrder* UnitOrderQueue::toplevelOrder() const
{
 if (d->mToplevelOrders.count() > 0) {
	return d->mToplevelOrders.first()->order();
 } else {
	return 0;
 }
}

UnitOrderData* UnitOrderQueue::toplevelOrderData() const
{
 if (d->mToplevelOrders.count() > 0) {
	return d->mToplevelOrders.first();
 } else {
	return 0;
 }
}

void UnitOrderQueue::clearOrders()
{
 if (d->mToplevelOrders.isEmpty()) {
	// AB: this call is ugly: it accesses the unit directly, instead of the
	//     UnitOrdersInterface. However I don't want to include something
	//     like clearOrdersFailsafe() in the interface either.
	mParentUnit->setAdvanceWork(UnitBase::WorkIdle);  // Failsafe
	return;
 }

 // note: this is UGLY!
 //       we hardcode the check for MoveInsideUnit, as currently only that order
 //       must be uninterruptible.
 //       also we have duplicated code: mostly the same code is in
 //       replaceToplevelOrders(), too.
 if (currentOrder() && (currentOrder()->type() == UnitOrder::MoveInsideUnit ||
		currentOrderData()->parent() && currentOrderData()->parent()->order()->type() == UnitOrder::MoveInsideUnit)) {
	// AB: MoveInsideUnit orders can't be interrupted.
	UnitOrderData* current = d->mToplevelOrders.front();
	d->mToplevelOrders.pop_front();
	while (!d->mToplevelOrders.isEmpty()) {
		delete d->mToplevelOrders.first();
		d->mToplevelOrders.pop_front();
	}
	d->mToplevelOrders.append(current);
 } else {
#if 1
	toplevelOrderRemoved();
#else
	currentOrderRemoved();
#endif
	while (!d->mToplevelOrders.isEmpty()) {
		delete d->mToplevelOrders.first();
		d->mToplevelOrders.pop_front();
	}
	currentOrderChanged();
 }
}

bool UnitOrderQueue::replaceToplevelOrders(UnitOrder* order)
{
// note: see also comment in clearOrders()
//       the difference to clearOrders() is in the else branch only (no
//       currentOrderChanged() call)
 if (currentOrder() && (currentOrder()->type() == UnitOrder::MoveInsideUnit ||
		currentOrderData()->parent() && currentOrderData()->parent()->order()->type() == UnitOrder::MoveInsideUnit)) {
	// AB: MoveInsideUnit orders can't be interrupted.
	UnitOrderData* current = d->mToplevelOrders.front();
	d->mToplevelOrders.pop_front();
	while (!d->mToplevelOrders.isEmpty()) {
		delete d->mToplevelOrders.first();
		d->mToplevelOrders.pop_front();
	}
	d->mToplevelOrders.append(current);
 } else {
#if 1
	toplevelOrderRemoved();
#else
	currentOrderRemoved();
#endif
	while (!d->mToplevelOrders.isEmpty()) {
		delete d->mToplevelOrders.first();
		d->mToplevelOrders.pop_front();
	}
 }

 return addToplevelOrder(order);
}

void UnitOrderQueue::toplevelOrderRemoved()
{
 boDebug() << k_funcinfo << endl;
 if (!currentOrder()) {
	boDebug() << k_funcinfo << "nothing to remove" << endl;
	return;
 }
 while (currentOrder() != toplevelOrder()) {
	currentSuborderDone(false);
 }

 // remove toplevel order
 currentSuborderDone(false);
 boDebug() << k_funcinfo << "toplevel order removed" << endl;
}

bool UnitOrderQueue::addToplevelOrder(UnitOrder* order)
{
 UnitOrderData* data = UnitOrderData::createData(order);
 if (!data) {
	boError() << k_funcinfo << "no UnitOrderData object for UnitOrder object created" << endl;
	delete order;
	return false;
 }
 d->mToplevelOrders.append(data);
 if (d->mToplevelOrders.count() == 1) {
	if (!currentOrderAdded()) {
		currentSuborderDone(false);
		return false;
	}
 }
 return true;
}

bool UnitOrderQueue::currentOrderChanged()
{
 d->mCurrentOrder = 0;
 if (!d->mToplevelOrders.isEmpty()) {
	d->mCurrentOrder = d->mToplevelOrders.first()->currentOrder();
 }

 return mParent->currentOrderChanged();
}

bool UnitOrderQueue::currentOrderAdded()
{
 if (!canAddOrder()) {
	return false;
 }
 if (!currentOrderChanged()) {
	return false;
 }
 if (!d->mCurrentOrder) {
	return true;
 }
 return mParent->currentOrderAdded();
}

bool UnitOrderQueue::addCurrentSuborder(UnitOrder* order)
{
 if (currentOrderData()) {
	// order will be child of currentOrder()
	UnitOrderData* orderdata = UnitOrderData::createData(order);
	if (!orderdata) {
		boError() << k_funcinfo << "no UnitOrderData object for UnitOrder object created" << endl;
		delete order;
		return false;
	}
	currentOrderData()->setSuborder(orderdata);
	d->mCurrentOrder = orderdata;
	if (!currentOrderAdded()) {
		// order couldn't be added, remove it
		currentSuborderDone(false);
		return false;
	} else {
		return true;
	}
 } else {
	// order will be toplevel order
	return addToplevelOrder(order);
 }
}

void UnitOrderQueue::currentSuborderDone(bool success)
{
 if (!d->mCurrentOrder) {
	boError() << k_funcinfo << "No current order!" << endl;
	return;
 }
 d->mLastOrderStatus = (success ? UnitOrder::Success : UnitOrder::Failure);

 currentSuborderRemoved();

 if (d->mCurrentOrder->parent()) {
	d->mCurrentOrder->parent()->suborderDone();  // Deletes the child order
	currentOrderChanged();
 } else {
	delete d->mToplevelOrders.first();
	d->mToplevelOrders.pop_front();
	if (!currentOrderAdded()) {
		currentSuborderDone(false);
	}
 }
}

void UnitOrderQueue::currentSuborderRemoved()
{
 mParent->currentSuborderRemoved();
}

bool UnitOrderQueue::canAddOrder() const
{
 return mParent->canAddOrder();
}

bool UnitOrderQueue::lastOrderStatus() const
{
 return (d->mLastOrderStatus == UnitOrder::Success);
}

bool UnitOrderQueue::saveAsXML(QDomElement& root)
{
 QDomDocument doc = root.ownerDocument();
 QValueList<UnitOrderData*>::Iterator it;
 for (it = d->mToplevelOrders.begin(); it != d->mToplevelOrders.end(); ++it) {
	QDomElement orderdataxml = doc.createElement("OrderData");
	root.appendChild(orderdataxml);
	(*it)->saveAsXML(orderdataxml);
 }

 root.setAttribute("LastOrderStatus", (int)d->mLastOrderStatus);
 return true;
}

bool UnitOrderQueue::loadFromXML(const QDomElement& root, BosonCanvas* canvas)
{
 QDomNodeList list = root.elementsByTagName("OrderData");
 for (unsigned int i = 0; i < list.count(); i++) {
	QDomElement orderdataelement = list.item(i).toElement();
	if (orderdataelement.isNull()) {
		boError(260) << k_funcinfo << "OrderData tag is not an element" << endl;
		return false;
	}

	UnitOrderData* data = UnitOrderData::createAndLoadFromXML(orderdataelement, canvas);
	if (!data) {
		return false;
	}
	d->mToplevelOrders.append(data);
 }

 // TODO: is this enough?
 currentOrderChanged();
 return true;
}



bool Unit::mInitialized = false;

class UnitPrivate
{
public:
	UnitPrivate()
	{
		mWeapons = 0;

		mMoveData = 0;

		mUnitMover = 0;
		mUnitInsideUnitMover = 0;

		mOrderQueue = 0;
	}
	KGamePropertyList<BoVector2Fixed> mPathPoints;
	KGameProperty<Q_INT8> mIsInsideUnit;
	KGameProperty<Q_INT8> mIsFlying;

	// be *very* careful with those - NewGameDialog uses Unit::save() which
	// saves all KGameProperty objects. If non-KGameProperty properties are
	// changed before all players entered the game we'll have a broken
	// network game.

	// these must NOT be touched (items added or removed) after the c'tor.
	// loading code will depend in this list to be at the c'tor state!
	QPtrList<UnitPlugin> mPlugins;
	BosonWeapon** mWeapons;

	BosonMoveData* mMoveData;

	unsigned long int mMaxWeaponRange;
	unsigned long int mMaxLandWeaponRange;
	unsigned long int mMaxAirWeaponRange;

	UnitMover* mUnitMover;
	UnitMover* mUnitInsideUnitMover;

	UnitOrderQueue* mOrderQueue;

	bool mHaveUnitStorage;
};

Unit::Unit(const UnitProperties* prop, Player* owner, BosonCanvas* canvas)
		: UnitBase(prop, owner, canvas)
{
 if (!mInitialized) {
	initStatic();
 }
 d = new UnitPrivate;
 d->mMaxWeaponRange = 0;
 d->mMaxAirWeaponRange = 0;
 d->mMaxLandWeaponRange = 0;
 mCurrentPlugin = 0;
 mAdvanceFunction = &Unit::advanceIdle;
 mAdvanceFunction2 = &Unit::advanceIdle;
 d->mPlugins.setAutoDelete(true);
 d->mPlugins.clear();

 // note: these width and height can be used for e.g. pathfinding. It does not
 // depend in any way on the .3ds file or another OpenGL thing.
 setSize(prop->unitWidth(), prop->unitHeight(), prop->unitDepth());

 registerData(&d->mPathPoints, IdPathPoints);
 registerData(&d->mIsInsideUnit, IdIsInsideUnit);
 registerData(&d->mIsFlying, IdIsFlying);

 d->mIsInsideUnit = false;
 d->mIsFlying = false;
 d->mHaveUnitStorage = false;

 d->mOrderQueue = new UnitOrderQueue(this, this);

 mUnitConstruction = 0;
 if (prop->isFacility()) {
	mUnitConstruction = new UnitConstruction(this);
 } else {
	if (unitProperties()->isAircraft()) {
		d->mUnitMover = new UnitMoverFlying(this);
		d->mIsFlying = true;
	} else {
		d->mUnitMover = new UnitMoverLand(this);
		d->mIsFlying = false;
	}

	d->mUnitInsideUnitMover = new UnitMoverInsideUnit(this);
 }
}

Unit::~Unit()
{
 delete d->mUnitMover;
 delete d->mUnitInsideUnitMover;
 delete mUnitConstruction;
 unselect();
 d->mPlugins.clear();
 BoPointerIterator<BosonWeapon> it = d->mWeapons;
 for (; *it; ++it) {
	delete *it;
 }
 delete[] d->mWeapons;
 if (canvas()->pathFinder()) {
	// Release highlevel path here once we cache them
 }
 delete d->mOrderQueue;
 delete d;
}

bool Unit::init()
{
 bool ret = BosonItem::init();
 if (!ret) {
	return ret;
 }
 if (!speciesTheme()) {
	BO_NULL_ERROR(speciesTheme());
	return false;
 }
 const UnitProperties* prop = unitProperties();
 if (!prop) {
	BO_NULL_ERROR(prop);
	return false;
 }
 if (!owner()) {
	BO_NULL_ERROR(owner());
	return false;
 }
// create the plugins
// note: we use fixed KGame-property IDs, so we can't add any plugin twice. if
// we ever want to support this, we need to use dynamically assigned (see
// KGameProperty docs) - i use fixed ids to make debugging easier.
 if (prop->properties(PluginProperties::Production)) {
	d->mPlugins.append(new ProductionPlugin(this));
 }
 if (prop->properties(PluginProperties::Repair)) {
	d->mPlugins.append(new RepairPlugin(this));
 }
 if (prop->properties(PluginProperties::Harvester)) {
	d->mPlugins.append(new HarvesterPlugin(this));
 }
 if (prop->properties(PluginProperties::Refinery)) {
	d->mPlugins.append(new RefineryPlugin(this));
 }
 if (prop->properties(PluginProperties::ResourceMine)) {
	d->mPlugins.append(new ResourceMinePlugin(this));
 }
 if (prop->properties(PluginProperties::AmmunitionStorage)) {
	d->mPlugins.append(new AmmunitionStoragePlugin(this));
 }
 if (prop->properties(PluginProperties::Radar)) {
	d->mPlugins.append(new RadarPlugin(this));
 }
 if (prop->properties(PluginProperties::RadarJammer)) {
	d->mPlugins.append(new RadarJammerPlugin(this));
 }
 d->mHaveUnitStorage = false;
 if (prop->properties(PluginProperties::UnitStorage)) {
	d->mPlugins.append(new UnitStoragePlugin(this));

	// AB: this variable is simply to avoid a slow plugin() call in
	// advanceIdle() if the unit has no storage.
	d->mHaveUnitStorage = true;
 }

 // we always have this plugin (and atm there is no corresponding
 // PluginProperties)
 d->mPlugins.append(new EnterUnitPlugin(this));


 loadWeapons();

 speciesTheme()->loadNewUnit(this);

 if (unitProperties()->isFacility()) {
	if (unitProperties()->constructionSteps() > 0) {
		setAdvanceWork(WorkConstructed);
	}
	updateAnimationMode();
 }

 return true;
}

void Unit::initStatic()
{
 // we initialize the properties for Unit, MobileUnit, Facility and the plugins
 // here
 // Unit
 addPropertyId(IdWantedRotation, QString::fromLatin1("WantedRotation"));
 addPropertyId(IdPathPoints, QString::fromLatin1("PathPoints"));
 addPropertyId(IdIsInsideUnit, QString::fromLatin1("IsInsideUnit"));
 addPropertyId(IdIsFlying, QString::fromLatin1("IsFlying"));

 // Facility
 addPropertyId(IdConstructionStep, QString::fromLatin1("ConstructionStep"));

 // UnitPlugin and derived classes
 addPropertyId(IdProductionState, QString::fromLatin1("ProductionState"));
 addPropertyId(IdResourcesMined, QString::fromLatin1("ResourcesMined"));
 addPropertyId(IdResourcesX, QString::fromLatin1("ResourcesX"));
 addPropertyId(IdResourcesY, QString::fromLatin1("ResourcesY"));
 addPropertyId(IdHarvestingType, QString::fromLatin1("HarvestingType"));
 addPropertyId(IdBombingTargetX, QString::fromLatin1("IdBombingTargetX"));
 addPropertyId(IdBombingTargetY, QString::fromLatin1("IdBombingTargetY"));
 addPropertyId(IdMinePlacingCounter, QString::fromLatin1("MinePlacingCounter"));
 addPropertyId(IdResourceMineMinerals, QString::fromLatin1("ResourceMineMinerals"));
 addPropertyId(IdResourceMineOil, QString::fromLatin1("ResourceMineOil"));
 addPropertyId(IdBombingDropDist, QString::fromLatin1("IdBombingDropDist"));
 addPropertyId(IdBombingLastDistFromDropPoint, QString::fromLatin1("IdBombingLastDistFromDropPoint"));
 addPropertyId(IdMineralsPaid, QString::fromLatin1("IdMineralsPaid"));
 addPropertyId(IdOilPaid, QString::fromLatin1("IdOilPaid"));
 addPropertyId(IdEnterPointOutside1, QString::fromLatin1("IdEnterPointOutside1"));
 addPropertyId(IdEnterPointOutside2, QString::fromLatin1("IdEnterPointOutside2"));
 addPropertyId(IdEnterUnitRemainingInsidePath, QString::fromLatin1("IdEnterUnitRemainingInsidePath"));
 addPropertyId(IdEnterUnitPathIndex, QString::fromLatin1("IdEnterUnitPathIndex"));
 addPropertyId(IdMovingInStatus, QString::fromLatin1("IdMovingInStatus"));
 addPropertyId(IdLandingStatus, QString::fromLatin1("IdLandingStatus"));
 addPropertyId(IdEnterUnitTriedMovingCounter, QString::fromLatin1("IdEnterUnitTriedMovingCounter"));
 addPropertyId(IdStoringStatus, QString::fromLatin1("IdStoringStatus"));
 addPropertyId(IdPendingEnterRequests, QString::fromLatin1("IdPendingEnterRequests"));
 addPropertyId(IdApprovedEnterRequests, QString::fromLatin1("IdApprovedEnterRequests"));
 addPropertyId(IdPendingLeaveRequests, QString::fromLatin1("IdPendingLeaveRequests"));
 addPropertyId(IdApprovedLeaveRequests, QString::fromLatin1("IdApprovedLeaveRequests"));
 addPropertyId(IdEnteringUnitOnPath, QString::fromLatin1("IdEnteringUnitOnPath"));

 UnitMover::initStatic();

 mInitialized = true;
}

void Unit::setMoveData(BosonMoveData* data)
{
 d->mMoveData = data;
}

BosonMoveData* Unit::moveData() const
{
 return d->mMoveData;
}

void Unit::select(bool markAsLeader)
{
 if (isDestroyed()) {
	boDebug() << k_funcinfo << id() << " is destroyed" << endl;
	return; // shall we really return?
 }
 BosonItem::select(markAsLeader);
}

void Unit::setHealth(unsigned long int h)
{
 if (h > maxHealth()) {
	h = maxHealth();
 }
 if (maxHealth() == 0) {
	boError() << "Ooop - maxHealth == 0" << endl;
	return;
 }
 // Radar's range depends on the unit's health, so we need to remove the radar,
 //  then change unit's health and then readd the radar
 if (plugin(UnitPlugin::Radar)) {
	canvas()->removeRadar(this);
 }
 if (plugin(UnitPlugin::RadarJammer)) {
	canvas()->removeRadarJammer(this);
 }
 UnitBase::setHealth(h);
 if (isDestroyed()) {
	unselect();
	updateAnimationMode();
 } else {
	if (plugin(UnitPlugin::Radar)) {
		canvas()->addRadar(this);
	}
	if (plugin(UnitPlugin::RadarJammer)) {
		canvas()->addRadarJammer(this);
	}
 }
}

void Unit::setSightRange(unsigned long int r)
{
 if(canvas()) {
	canvas()->removeSight(this);
 }
 UnitBase::setSightRange(r);
 if (isDestroyed()) {
	return;
 }
 if (canvas()) {
	canvas()->addSight(this);
 }
}

void Unit::setPluginWork(int pluginType)
{
 UnitPlugin* p = plugin(pluginType);
 if (!p) {
	boError() << k_funcinfo << id() << " does not have plugin " << pluginType << endl;
	return;
 }
 setAdvanceWork(WorkPlugin);
 mCurrentPlugin = p;
}

UnitPlugin* Unit::plugin(int pluginType) const
{
 if (mUnitConstruction && !mUnitConstruction->isConstructionComplete()) {
	return 0;
 }
 QPtrListIterator<UnitPlugin> it(d->mPlugins);
 for (; it.current(); ++it) {
	if (it.current()->pluginType() == pluginType) {
		return it.current();
	}
 }
 return 0;
}

int Unit::currentPluginType() const
{
 if (!currentPlugin()) {
	return 0;
 }
 return currentPlugin()->pluginType();
}


void Unit::moveBy(bofixed moveX, bofixed moveY, bofixed moveZ)
{
// time critical function
 if (!moveX && !moveY && !moveZ) {
	return;
 }

 if (isDestroyed()) {
	// Just move the unit and return. No need to update z or minimap
	BosonItem::moveBy(moveX, moveY, moveZ);
	return;
 }

 // QCanvas::advance() uses a different approach than we do. They call moveBy()
 // from phase 1 and do interesting stuff like collision detection in phase 0.
 // we do collision detection of 1st unit and then move 1st unit and *then* do
 // collision detection of 2nd unit and then move 2nd unit, i.e. we do both
 // parts in a single phase.
 //
 // this will most probably cause trouble in the future, but it is necessary for
 // things like addToCells().

 bofixed oldX = centerX();
 bofixed oldY = centerY();

 if (!isFlying()) {
	bofixed rotateX = 0.0f;
	bofixed rotateY = 0.0f;
	updateZ(moveX, moveY, &moveZ, &rotateX, &rotateY);
	setXRotation(rotateX);
	setYRotation(rotateY);
 }

 BosonItem::moveBy(moveX, moveY, moveZ);
 canvas()->unitMoved(this, oldX, oldY);
}

void Unit::updateZ(bofixed moveByX, bofixed moveByY, bofixed* moveByZ, bofixed* rotateX, bofixed* rotateY)
{
 // Center point of the unit.
 bofixed centerx = centerX() + moveByX;
 bofixed centery = centerY() + moveByY;

 // Calculate unit's rotation (depends on ground height).
 // These are offsets (from center point) to front and right side of the unit.
 bofixed frontx = 0.0f, fronty = 0.0f, sidex = 0.0f, sidey = 0.0f;
 // Calculate front offset
 Bo3dTools::pointByRotation(&frontx, &fronty, rotation(), height() / 2);
 // Calculate right side offset
 bofixed myrot = rotation() + 90;
 if (myrot > 360) {
	myrot -= 360;
 }
 Bo3dTools::pointByRotation(&sidex, &sidey, myrot, width() / 2);

 // Find necessary height values.
 bofixed rearz, frontz, rightz, leftz, newZ;
 // For flying units and ships, we take water level into surface; for land
 //  units, we don't.
 if (unitProperties()->isAircraft() || unitProperties()->canGoOnWater()) {
	newZ = canvas()->heightAtPoint(centerx, centery);
	frontz = canvas()->heightAtPoint(centerx + frontx, centery + fronty);
	rearz = canvas()->heightAtPoint(centerx - frontx, centery - fronty);
	rightz = canvas()->heightAtPoint(centerx + sidex, centery + sidey);
	leftz = canvas()->heightAtPoint(centerx - sidex, centery - sidey);
	if (!unitProperties()->isAircraft()) {
		// We want ships to be a bit inside the water.
		newZ -= 0.05;
		// rearz, frontz, rightz and leftz are only used for rotation calculation,
		//  so there's no need to change them.
	}
 } else {
   // Land unit
	newZ = canvas()->terrainHeightAtPoint(centerx, centery);
	frontz = canvas()->terrainHeightAtPoint(centerx + frontx, centery + fronty);
	rearz = canvas()->terrainHeightAtPoint(centerx - frontx, centery - fronty);
	rightz = canvas()->terrainHeightAtPoint(centerx + sidex, centery + sidey);
	leftz = canvas()->terrainHeightAtPoint(centerx - sidex, centery - sidey);
 }

 // Calculate rotations
 // Calculate angle from frontz to rearz
 bofixed xrot = Bo3dTools::rad2deg(atan(QABS(frontz - rearz) / height()));
 *rotateX = (frontz >= rearz) ? xrot : -xrot;

 // Calculate y rotation
 // Calculate angle from leftz to rightz
 bofixed yrot = Bo3dTools::rad2deg(atan(QABS(rightz - leftz) / width()));
 *rotateY = (leftz >= rightz) ? yrot : -yrot;


 if (isFlying()) {
	newZ += 2.0f;  // Flying units are always 2 units above the ground
 }
 *moveByZ = newZ - z();
}

void Unit::reload(unsigned int count)
{
 if (isDestroyed()) {
	return;
 }
 chargePowerForReload(owner()->powerChargeForCurrentAdvanceCall());
 bool unitConsumesPower = false;
 if (powerConsumedByUnit() > 0) {
	unitConsumesPower = true;
 }
 bool isCharged = isChargedForReload();
 if (d->mWeapons[0]) {
	BosonWeapon** w = &d->mWeapons[0];
	for (; *w; w++) {
		// TODO
#if 0
		bool weaponRequiresPower = true;
		if ((*w)->powerConsumedByWeapon == 0) {
			weaponRequiresPower = false;
		}
#else
		bool weaponRequiresPower = unitConsumesPower;
#endif
		if (isCharged || !weaponRequiresPower) {
			(*w)->reload(this, count);
		}
	}
 }

 // shield reloading always requires power (unless the unit doesn't consume
 // power at all)
 bool shieldsRequirePower = true;
 if (!unitConsumesPower) {
	shieldsRequirePower = false;
 }
 if (shields() < maxShields()) {
	if (isCharged || !shieldsRequirePower) {
		reloadShields(count);
	}
 }

 unchargePowerForReload();
}

unsigned long int Unit::requestAmmunition(const QString& type, unsigned long int requested)
{
 if (!owner()) {
	BO_NULL_ERROR(owner);
	return 0;
 }

 // AB: there are 3 places where ammunition could be takes from
 // 1. the global "pool" of the player. this ammunition "just exists" and is
 //    bound to a player only (not to e.g. a facility)
 // 2. the globally accessible ammunition that is stored in some storage unit
 //    (usually a facility that produces the ammo). it is destroyed when that
 //    storage unit is destroyed, but can be accessed globally, without the need
 //    to pick it up.
 // 3. ammunition that needs to be picked up at a unit.

 // this searches for ammo of type 1 and 2
 unsigned long int ammo = owner()->requestAmmunition(type, requested);
 if (ammo > requested) {
	boError(610) << k_funcinfo << "received more ammo than requested" << endl;
	ammo = requested;
 }
 if (ammo == requested) {
	return ammo;
 }

 boDebug(610) << k_funcinfo << "searching for unit to pick ammo up from" << endl;
 for (QPtrListIterator<Unit> it(*owner()->allUnits()); it.current(); ++it) {
	if (it.current()->isDestroyed()) {
		continue;
	}
	Unit* unit = it.current();
	AmmunitionStoragePlugin* storage = (AmmunitionStoragePlugin*)unit->plugin(UnitPlugin::AmmunitionStorage);
	if (!storage) {
		continue;
	}
	if (storage->ammunitionStored(type) == 0) {
		continue;
	}

	bool denied = false;
	ammo += storage->pickupAmmunition(this, type, requested - ammo, &denied);
	if (ammo > requested) {
		boError(610) << k_funcinfo << "received more ammo than requested" << endl;
		ammo = requested;
	}
	if (ammo == requested) {
		return ammo;
	}
	if (denied) {
		boDebug(610) << k_funcinfo << id() << ": picking up from unit " << storage->unit()->id() << " denied" << endl;
	}
 }


 return ammo;
}

void Unit::advanceNone(unsigned int)
{
 // do NOT do anything here!
 // usually it won't be called anyway.
}

void Unit::advanceIdle(unsigned int advanceCallsCount)
{
 advanceIdleBasic(advanceCallsCount);
 moveIdle();

 if (d->mHaveUnitStorage) {
	advanceIdleUnitStorage(advanceCallsCount);
 }
}

void Unit::advanceIdleBasic(unsigned int advanceCallsCount)
{
 if (advanceCallsCount % 40 != (id() % 40)) {
	return;
 }
 BosonProfiler profiler("advanceIdle");

 if (unitProperties()->canShoot() && d->mWeapons[0]) {
	// Attack enemy units in range
	Unit* target = bestEnemyUnitInRange();
	if (target) {
		addCurrentSuborder(new UnitAttackOrder(target, false));
	}
 } else if (!unitProperties()->isAircraft() && !d->mHaveUnitStorage) {
	// this unit does not have any weapons, so it will never shoot anyway.
	// no need to call advanceIdle() again
	setAdvanceWork(WorkNone);
 }
}

void Unit::advanceIdleUnitStorage(unsigned int advanceCallsCount)
{
 if (advanceCallsCount % 40 != (id() % 40)) {
	return;
 }

 UnitStoragePlugin* storage = (UnitStoragePlugin*)plugin(UnitPlugin::UnitStorage);
 if (!storage) { // e.g. because construction is incomplete
	return;
 }
 if (storage->storageStatus() != UnitStoragePlugin::StatusDoorsClosed) {
	boDebug() << k_funcinfo << "storage has not yet closed doors. giving control to storage." << endl;
	setPluginWork(UnitPlugin::UnitStorage);
	return;
 }

 if (storage->requestsCount() > 0) {
	boDebug() << k_funcinfo << "storage has requests. giving control to storage." << endl;
	setPluginWork(UnitPlugin::UnitStorage);
	return;
 }
}

void Unit::advanceConstruction(unsigned int advanceCallsCount)
{
 if (mUnitConstruction) {
	mUnitConstruction->advanceConstruction(advanceCallsCount);
 }
}

Unit* Unit::attackEnemyUnitsInRange(Unit* target)
{
 PROFILE_METHOD
 if (!unitProperties()->canShoot()) {
	return 0;
 }
 if (!d->mWeapons[0]) {
	return 0;
 }

 // TODO: Note that this is not completely realistic nor good: it may be good to
 //  e.g. not waste some weapon with very big damage and reload values for very
 //  weak unit. So there room left for improving :-)
 bool targetfound = false;
 BoPointerIterator<BosonWeapon> wit(d->mWeapons);
 for (; *wit; ++wit) {
	BosonWeapon* w = *wit;

	if (!w->properties()->autoUse()) {
		continue;
	}

	if (!w->reloaded()) {
		continue;
	}

	// We use target to store best enemy in range so we don't have to look for it every time.
	// If there's no target or target isn't in range anymore, find new best enemy unit in range
	// FIXME: check for max(Land|Air)WeaponRange
	if (!target || target->isDestroyed() ||
			!inRange(maxWeaponRange(), target)) {
		target = bestEnemyUnitInRange();
		if (!target) {
			return 0;
		}
	}
	targetfound = true;

	// If unit is mobile, rotate to face the target if it isn't facing it yet
	if (!w->turret() && isMobile() && !isFlying()) {
		if (!turnToUnit(target)) {
			// Suborder was constructed to turn the unit. Return for now
			return target;
		}
	}

	// And finally... let it have everything we've got
	if (w->canShootAt(target) && inRange(w->range(), target)) {
		shootAt(w, target);
		if (target->isDestroyed()) {
			target = 0;
		}
	}
 }

/* // It might be that none of the unit's weapons is reloaded, but target has been
 //  found. So we check for this now.
 targetfound = targetfound || (target);*/

 return target;
}

Unit* Unit::bestEnemyUnitInRange()
{
 PROFILE_METHOD
 // Return if unit can't shoot
 if (!unitProperties()->canShoot()) {
	return 0;
 }
 // Return if no enemies in range
 BoItemList* list = enemyUnitsInRange(maxWeaponRange());
 if (!list->count() > 0) {
	return 0;
 }

 // Initialize some variables
 Unit* best = 0;
 BoItemList::Iterator it = list->begin();
 Unit* u = 0;
 bofixed dist = 0;
 // Candidates to best unit, see below
 Unit* c1 = 0;
 Unit* c2 = 0;
 Unit* c3 = 0;

 // Iterate through the list of enemies and pick the best ones
 for (; it != list->end(); ++it) {
	u = ((Unit*)*it);
	dist = QMAX(QABS((int)(u->centerX() - centerX())), QABS((int)(u->centerY() - centerY())));
	// Quick check if we can shoot at u
	if (u->isFlying()) {
		if (!unitProperties()->canShootAtAirUnits()) {
			continue;
		}
		if (dist > maxAirWeaponRange()) {
			continue;
		}
	} else {
		if (!unitProperties()->canShootAtLandUnits()) {
			continue;
		}
		if (dist > maxLandWeaponRange()) {
			continue;
		}
	}

	// Check if it's the best unit so far.
	// This is presedence of enemies:
	//  1. enemies that can shoot at us
	//  2. enemies that can shoot, but not at us
	//  3. others
	// Shoot at units that can shoot first, then at units that cannot shoot
	if (u->unitProperties()->canShoot()) {
		if ((isFlying() && u->unitProperties()->canShootAtAirUnits()) ||
				(!isFlying() && u->unitProperties()->canShootAtLandUnits())) {
			// u is type 1 - it can shoot at us
			// TODO: check also for health here - first kill weaker units
			c1 = u;
		} else {
			// u is type 2 - it can shoot but not at us
			c2 = u;
		}
	} else {
		// u is type 3 - it can't shoot
		c3 = u;
	}
 }

 // Pick the best unit from the candidates
 if (c1) {
	best = c1;
 } else if (c2) {
	best = c2;
 } else if (c3) {
	best = c3;
 }
 return best;
}

void Unit::advanceAttack(unsigned int advanceCallsCount)
{
 if (advanceCallsCount % 5 != 0) {
	moveIdle();
	return;
 }
 BosonProfiler profiler("advanceAttack");
 boDebug(300) << k_funcinfo << endl;

 UnitAttackOrder* attackorder = (UnitAttackOrder*)currentOrder();
 Unit* target = attackorder->target();

 if (!target) {
	boWarning() << k_funcinfo << id() << " cannot attack NULL target" << endl;
	currentSuborderDone(false);
	return;
 }
 if (target->isDestroyed()) {
	boDebug(300) << "Target (" << target->id() << ") is destroyed!" << endl;
	currentSuborderDone(true);
	return;
 }

 boDebug(300) << "    " << k_funcinfo << "checking if unit " << target->id() << ") is in range" << endl;
 int range;
 if (target->isFlying()) {
	range = maxAirWeaponRange();
 } else {
	range = maxLandWeaponRange();
 }

 if (!inRange(range, target)) {
	if (!attackorder->canMove()) {
		currentSuborderDone(true);
		return;
	}
	// AB: warning - this does a lookup on all items and therefore is slow!
	// --> but we need it as a simple test on the pointer causes trouble if
	// that pointer is already deleted. any nice solutions?
	if (!canvas()->allItems()->contains(target)) {
		boDebug(300) << "Target seems to be destroyed!" << endl;
		currentSuborderDone(true);
		return;
	}
	boDebug(300) << "unit (" << target->id() << ") not in range - moving..." << endl;
	if (range >= 1) {
		range--;
	}
	if (!addCurrentSuborder(new UnitMoveToUnitOrder(target, range, false))) {
		currentSuborderDone(false);
	}
	return;
 }

 // Shoot at target with as many weapons as possible
 boDebug(300) << "    " << k_funcinfo << "shooting at target" << endl;
 BoPointerIterator<BosonWeapon> wit(d->mWeapons);
 BosonWeapon* w;
 for (; *wit; ++wit) {
	w = *wit;
	boDebug(300) << "      " << k_funcinfo << "trying weapon " << w->properties()->weaponName() << endl;
	if (w->properties()->autoUse() && w->reloaded() && w->canShootAt(target) && inRange(w->range(), target)) {
		// If unit is mobile, rotate to face the target if it isn't facing it yet
		if (!w->turret() && isMobile() && !isFlying()) {
			if (!turnToUnit(target)) {
				boDebug(300) << "        " << k_funcinfo << "turning to target..." << endl;
				// Suborder was constructed to turn the unit. Return for now
				return;
			}
		}

		boDebug(300) << "        " << k_funcinfo << "fire!" << endl;
		shootAt(w, target);
		if (!target || target->isDestroyed()) {
			boDebug(300) << "    " << k_funcinfo << "target gone, returning" << endl;
			currentSuborderDone(true);
			moveIdle();
			return;
		}
	}
 }
 boDebug(300) << "    " << k_funcinfo << "done shooting" << endl;
 // TODO: fly on straight, pass the target, fly a bit more, then turn and fly back toward the target
 moveIdle();
}

void Unit::advanceFollow(unsigned int advanceCallsCount)
{
 if (unitMover()) {
	unitMover()->advanceFollow(advanceCallsCount);
 }
}

void Unit::advanceMove(unsigned int advanceCallsCount)
{
 if (unitMover()) {
	unitMover()->advanceMove(advanceCallsCount);
 }
}

void Unit::advanceDestroyed(unsigned int advanceCallsCount)
{
 // note: the unit/wreckage will get deleted pretty soon
 if (advanceCallsCount % 10 != 0) {
	return;
 }
 BosonProfiler profiler("advanceDestroyed");
 if (isVisible()) {
	// Make unit slowly sink into ground
#define MAXIMAL_ADVANCE_COUNT 19
	setVelocity(0, 0, -(depth() / (REMOVE_WRECKAGES_TIME * MAXIMAL_ADVANCE_COUNT)) * 1.2);
#undef MAXIMAL_ADVANCE_COUNT
 }
}

void Unit::advancePlugin(unsigned int advanceCallsCount)
{
 BosonProfiler profiler("advancePlugin");
 if (!currentPlugin()) {
	boWarning() << k_funcinfo << "NULL plugin!" << endl;
	currentSuborderDone(false);
 } else {
	currentPlugin()->advance(advanceCallsCount);
 }
}

void Unit::advanceTurn(unsigned int)
{
 BosonProfiler profiler("advanceTurn");
 if (currentOrder()->type() != UnitOrder::Turn && currentOrder()->type() != UnitOrder::TurnToUnit) {
	boDebug() << k_funcinfo << "Invalid order type " << currentOrder()->type() << endl;
	currentSuborderDone(false);
	return;
 }
 // Unit is still while turning
 setVelocity(0, 0, 0);

 bofixed wanteddir;
 if (currentOrder()->type() == UnitOrder::TurnToUnit) {
	Unit* target = ((UnitTurnToUnitOrder*)currentOrder())->target();
	wanteddir = Bo3dTools::rotationToPoint(target->centerX() - centerX(), target->centerY() - centerY());
 } else {
	wanteddir = ((UnitTurnOrder*)currentOrder())->direction();
 }
 bofixed dir = rotation();
 bofixed a = dir - wanteddir;  // How many degrees to turn
 bool turncw = false;  // Direction of turning, CW or CCW

 // First find out direction of turning and huw much is left to turn
 if (a < 0) {
	a = QABS(a);
	turncw = true;
 }
 if (a > 180) {
	a = 180 - (a - 180);
	turncw = !turncw;
 }

 if (a <= unitProperties()->rotationSpeed()) {
	dir = wanteddir;
 } else {
	if (turncw) {
		dir += (int)unitProperties()->rotationSpeed();
	} else {
		dir -= (int)unitProperties()->rotationSpeed();
	}
 }
 // Check for overflows
 if (dir < 0) {
	dir += 360;
 } else if (dir > 360) {
	dir -= 360;
 }

 setRotation(dir);
 updateRotation();

 if (dir == wanteddir) {
	currentSuborderDone(true);
 }
}

void Unit::resetPathInfo()
{
 BO_CHECK_NULL_RET(pathInfo());

 // Release highlevel path here once we cache them
 pathInfo()->reset();
 pathInfo()->unit = this;
 pathInfo()->flying = unitProperties()->isAircraft();
}

bool Unit::moveTo(bofixed x, bofixed y, int range)
{
 if (mUnitConstruction && !mUnitConstruction->isConstructionComplete()) {
	boWarning(380) << k_funcinfo << "not yet constructed completely" << endl;
	return false;
 }
 if (maxSpeed() == 0) {
	boDebug(380) << k_funcinfo << "maxSpeed() == 0" << endl;
	// If unit's max speed is 0, it cannot move
	return false;
 }

 if (isFlying()) {
	// Aircrafts cannot go near the border of the map to make sure they have
	//  enough room for turning around
	x = QMIN(QMAX(x, bofixed(6)), (bofixed)canvas()->mapWidth() - 6);
	y = QMIN(QMAX(y, bofixed(6)), (bofixed)canvas()->mapHeight() - 6);
 }

 // TODO: move the destination point a bit in case the unit partially goes off
 // the map otherwise
 // Find destination cell
 int cellX = (int)x;
 int cellY = (int)y;
 Cell* cell = canvas()->cell(cellX, cellY);
 if (!cell) {
	boError(380) << k_funcinfo << "unit " << id() << ": Cell (" << cellX << "; " << cellY << ") at (" <<
			x << "; " << y << ") is not valid!" << endl;
	return false;
 }

 // Update path info
 resetPathInfo();
 pathInfo()->dest.setX(x);
 pathInfo()->dest.setY(y);
 pathInfo()->range = range;
 boDebug(380) << k_funcinfo << "unit " << id() << ": dest: (" << x << "; " << y << "); range: " << range << endl;

 // Remove old pathpoints
 // TODO: maybe call stopMoving() instead and remove setMovingStatus(Standing)
 //  from there...
 clearPathPoints();

 // Path is not searched here (it would break pathfinding for groups). Instead,
 //  moving status is set to MustSearchPath and in MobileUnit::advanceMove(), path
 //  is searched for.
 setMovingStatus(MustSearchPath);

 return true;
}

void Unit::addPathPoint(const BoVector2Fixed& pos)
{
 d->mPathPoints.append(pos);
}

unsigned int Unit::pathPointCount() const
{
 return d->mPathPoints.count();
}

const BoVector2Fixed& Unit::currentPathPoint()
{
 return d->mPathPoints.first();
}

void Unit::clearPathPoints()
{
 d->mPathPoints.clear();
}

void Unit::pathPointDone()
{
 if (d->mPathPoints.count() == 0) {
	boError() << k_funcinfo << "no pathpoints" << endl;
	return;
 }
 d->mPathPoints.pop_front();
}

const QValueList<BoVector2Fixed>& Unit::pathPointList() const
{
 return d->mPathPoints;
}

bool Unit::saveAsXML(QDomElement& root)
{
 // we should probably add pure virtual methods save() and load() to the plugins,
 // in order to store non-KGameProperty data there, too
 // note that UnitBase::save() also saves KGameProperty data of plugins and
 // weapons
 if (!UnitBase::saveAsXML(root)) {
	boError() << "Unit not saved properly" << endl;
	return false;
 }

 QDomDocument doc = root.ownerDocument();

 root.setAttribute(QString::fromLatin1("Rotation"), rotation());
 // No need to store x and y rotations

 // store the current plugin:
 if (currentPlugin()) {
	int pluginIndex = d->mPlugins.findRef(currentPlugin());
	root.setAttribute(QString::fromLatin1("CurrentPlugin"), pluginIndex);
 } else {
	// the unit won't have this attribute.
 }

 // also store the target:
 unsigned long int targetId = 0;
 /*if (target()) {
	targetId = target()->id();
 }
 root.setAttribute(QString::fromLatin1("Target"), (unsigned int)targetId);*/

 if (d->mPlugins.count() != 0) {
	QDomElement pluginElement = doc.createElement(QString::fromLatin1("UnitPlugin"));
	QPtrListIterator<UnitPlugin> it(d->mPlugins);
	for (; it.current(); ++it) {
		it.current()->saveAsXML(pluginElement);

		// we won't add the element at all, if it wasn't used.
		if (pluginElement.hasAttributes() || pluginElement.hasChildNodes()) {
			pluginElement.setAttribute(QString::fromLatin1("Type"), it.current()->pluginType());

			// AB: atm we support a single instance only anyway.
			pluginElement.setAttribute(QString::fromLatin1("Instance"), 0);
			root.appendChild(pluginElement);

			// create an element for the next plugin
			pluginElement = doc.createElement(QString::fromLatin1("UnitPlugin"));
		}
	}
 }

 // Save orders queue
 QDomElement orderqueuexml = doc.createElement("OrderQueue");
 root.appendChild(orderqueuexml);
 if (!d->mOrderQueue->saveAsXML(orderqueuexml)) {
	return false;
 }

 if (mUnitConstruction) {
	if (!mUnitConstruction->saveAsXML(root)) {
		return false;
	}
 }

 if (d->mUnitMover) {
	if (!d->mUnitMover->saveAsXML(root)) {
		return false;
	}
 }

 // AB: atm we don't need to save d->mUnitInsideUnitMover. hopefully that won't
 //     change...

 return true;
}

bool Unit::loadFromXML(const QDomElement& root)
{
 if (!UnitBase::loadFromXML(root)) {
	boError(260) << k_funcinfo << "Unit not loaded properly" << endl;
	return false;
 }
 if (health() > maxHealth()) {
	boError(260) << k_funcinfo << "Unit with Id " << id() << " (Type=" << type() << ") wants health=" << health() << " but only " << maxHealth() << " is possible for that type according to index.unit file. decreasing health to maximum." << endl;
	setHealth(maxHealth());
 }
 bool ok = false;
 bofixed rotation = root.attribute(QString::fromLatin1("Rotation")).toFloat(&ok);
 if (!ok) {
	boError(260) << k_funcinfo << "Invalid value for Rotation tag" << endl;
	rotation = 0.0f;
 }

 int pluginIndex = 0;
 if (root.hasAttribute(QString::fromLatin1("CurrentPlugin"))) {
	pluginIndex = root.attribute(QString::fromLatin1("CurrentPlugin")).toInt(&ok);
	if (!ok) {
		boError(260) << k_funcinfo << "Invalid value for CurrentPlugin tag" << endl;
		pluginIndex = 0;
	}
	if ((unsigned int)pluginIndex >= d->mPlugins.count()) {
		boWarning(260) << k_funcinfo << "Invalid current plugin index: " << pluginIndex << endl;
		pluginIndex = 0;
	}
	mCurrentPlugin = d->mPlugins.at(pluginIndex);
 } else {
	mCurrentPlugin = 0;
 }

 if (d->mPlugins.count() != 0) {
	QDomNodeList list = root.elementsByTagName(QString::fromLatin1("UnitPlugin"));
	for (unsigned int i = 0; i < list.count(); i++) {
		QDomElement e = list.item(i).toElement();
		if (e.isNull()) {
			continue;
		}
		unsigned int type;
		unsigned int instance;
		bool ok = false;
		type = e.attribute(QString::fromLatin1("Type")).toUInt(&ok);
		if (!ok) {
			boError() << k_funcinfo << "Type of UnitPlugin " << i << " is not a valid number" << endl;
			return false;
		}
		instance = e.attribute(QString::fromLatin1("Instance")).toUInt(&ok);
		if (!ok) {
			boError() << k_funcinfo << "Instance of UnitPlugin " << i << " is not a valid number" << endl;
			return false;
		}
		if (instance != 0) {
			boError() << k_funcinfo << "instance != 0 for UnitPlugin " << i << " is not yet supported." << endl;
			return false;
		}
		UnitPlugin* p = plugin(type);
		Q_UNUSED(instance);
		if (!p) {
			boWarning() << k_funcinfo << "UnitPlugin " << type << " not found for unit " << id() << endl;
			continue;
		}
		p->loadFromXML(e);
	}
 }
 setRotation(rotation);
 updateRotation();
 setAdvanceWork(advanceWork());



 // Load orders queue
 QDomElement orderqueuexml = root.namedItem("OrderQueue").toElement();
 if (!orderqueuexml.isNull()) {
	if (!d->mOrderQueue->loadFromXML(orderqueuexml, canvas())) {
		return false;
	}
 }
 if (pathInfo()) {
	pathInfo()->unit = this;
	pathInfo()->flying = unitProperties()->isAircraft();
 }

 recalculateMaxWeaponRange();

 if (mUnitConstruction) {
	if (!mUnitConstruction->loadFromXML(root)) {
		return false;
	}
 }

 if (d->mUnitMover) {
	if (!d->mUnitMover->loadFromXML(root)) {
		return false;
	}
 }

 updateAnimationMode();

 return true;
}

bool Unit::inRange(unsigned long int r, Unit* target) const
{
 return (QMAX(QABS((target->centerX() - centerX())), QABS((target->centerY() - centerY()))) <= bofixed(r));
}

void Unit::shootAt(BosonWeapon* w, Unit* target)
{
 if (!w->reloaded()) {
//	boDebug() << k_funcinfo << "gotta reload first" << endl;
	return;
 }
 if (!w->canShootAt(target)) {
	boDebug() << k_funcinfo << "can't shoot at target!" << endl;
	return;
 }
 if (target->isDestroyed()) {
	boWarning() << k_funcinfo << target->id() << " is already destroyed" << endl;
	return;
 }
// boDebug() << id() << " shoots at unit " << target->id() << endl;
 w->shoot(target);
 ownerIO()->statistics()->increaseShots();
}

BoItemList* Unit::unitsInRange(unsigned long int range) const
{
 PROFILE_METHOD
 // TODO: we use a *rect* for the range this is extremely bad.
 // ever heard about pythagoras ;-) ?

 // AB: note that we don't need to do error checking like left < 0, since
 // collisions() does this anyway.
 BoRect2Fixed rect(leftEdge() - range, topEdge() - range, rightEdge() + range, bottomEdge() + range);

 // TODO: we should do this using PlayerIO. It should return items that are
 // actually visible to us only!
 boProfiling->push("collisionsAtCells()");
 BoItemList* items = collisions()->collisionsAtCells(rect, (BosonItem*)this, false);
 boProfiling->pop();
 items->remove((BosonItem*)this);

 BoItemList* units = new BoItemList();
 BoItemList::Iterator it = items->begin();
 Unit* u;
 for (; it != items->end(); ++it) {
	if (!RTTI::isUnit((*it)->rtti())) {
		continue;
	}
	u = (Unit*)(*it);
	if (u->isDestroyed()) {
		continue;
	}
	if (!(u->visibleStatus(owner()->bosonId()) & (UnitBase::VS_Visible | UnitBase::VS_Earlier))) {
		continue;
	}
	if (!inRange(range, u)) {
		continue;
	}
	// TODO: remove the items from inRange which are not actually in range (hint:
	// pythagoras)
	units->append(*it);
 }
 return units;
}

BoItemList* Unit::enemyUnitsInRange(unsigned long int range) const
{
 PROFILE_METHOD
 boProfiling->push("unitsInRange()");
 BoItemList* units = unitsInRange(range);
 boProfiling->pop();
 BoItemList* enemy = new BoItemList();
 Unit* u;
 BoItemList::Iterator it = units->begin();
 boProfiling->push("find enemies");
 for (; it != units->end(); ++it) {
	u = (Unit*)*it;
	if (ownerIO()->isEnemy(u)) {
		enemy->append(u);
	}
 }
 boProfiling->pop();
 return enemy;
}

QValueList<Unit*> Unit::unitCollisions(bool exact)
{
 PROFILE_METHOD
 QValueList<Unit*> units;
 boDebug(310) << k_funcinfo << endl;
 BoItemList* collisionList = collisions()->collisionsAtCells(cells(), (BosonItem*)this, exact);
 if (collisionList->isEmpty()) {
	return units;
 }

 BoItemList::Iterator it;
 Unit* unit;
 for (it = collisionList->begin(); it != collisionList->end(); ++it) {
	if (!RTTI::isUnit((*it)->rtti())) {
		continue;
	}
	unit = ((Unit*)*it);
	if (unit->isDestroyed()) {
		continue;
	}
	if (unit->isFlying() != isFlying()) {
		continue;
	}
	units.append(unit);
 }
 return units;
}

void Unit::setAdvanceWork(WorkType w)
{
 // velicities should be 0 anyway - this is the final fallback in case it was
 // missing by any reason
 // AB: this could break loading games! so we do NOT do it anymore. remove this,
 // if it doesn't cause problems.
// setVelocity(0.0, 0.0, 0.0);

 if (advanceWork() != w) {
	// we change the list only if work() actually changed (in favor of
	// performance). but do not return here!
	canvas()->changeAdvanceList((BosonItem*)this);
 }
 if (currentPlugin() && w != WorkPlugin) {
	mCurrentPlugin = 0;
 }
 UnitBase::setAdvanceWork(w);

 // we even do this if nothing changed - just in case...
 switch (w) {
	case WorkIdle:
		setAdvanceFunction(&Unit::advanceIdle, canvas()->advanceFlag());
		break;
	case WorkNone:
		setAdvanceFunction(&Unit::advanceNone, canvas()->advanceFlag());
		break;
	case WorkMove:
		setAdvanceFunction(&Unit::advanceMove, canvas()->advanceFlag());
		break;
	case WorkAttack:
		setAdvanceFunction(&Unit::advanceAttack, canvas()->advanceFlag());
		break;
	case WorkConstructed:
		setAdvanceFunction(&Unit::advanceConstruction, canvas()->advanceFlag());
		break;
	case WorkDestroyed:
		setAdvanceFunction(&Unit::advanceDestroyed, canvas()->advanceFlag());
		break;
	case WorkFollow:
		setAdvanceFunction(&Unit::advanceFollow, canvas()->advanceFlag());
		break;
	case WorkPlugin:
		setAdvanceFunction(&Unit::advancePlugin, canvas()->advanceFlag());
		break;
	case WorkTurn:
		setAdvanceFunction(&Unit::advanceTurn, canvas()->advanceFlag());
		break;
 }
}

void Unit::setAdvanceFunction(MemberFunction func, bool advanceFlag)
{
 if (canvas()->advanceFunctionLocked()) {
	if (advanceFlag) {
		mAdvanceFunction = func;
	} else {
		mAdvanceFunction2 = func;
	}
 } else {
	mAdvanceFunction = func;
	mAdvanceFunction2 = func;
 }
}

bool Unit::isNextTo(Unit* target) const
{
 //const int r = BO_TILE_SIZE;
 const int r = 10;
 // in theory r = 1 should be enough... both of the above make problems under
 // certain circumstances
 if (QABS(rightEdge() - target->leftEdge()) <= r ||
		QABS(leftEdge() - target->rightEdge()) <= r ||
		rightEdge() <= target->rightEdge() && leftEdge() <= target->leftEdge()// will never happen with current pixmaps
		) {
	if (QABS(topEdge() - target->bottomEdge() <= r) ||
			QABS(bottomEdge() - target->topEdge()) <= r||
			topEdge() <= target->topEdge() && bottomEdge() <= target->bottomEdge()// will never happen with current pixmaps
			) {
		boDebug() << k_funcinfo << "ok - inrange" << endl;
		return true;
	}
 }
 return false;
}

bool Unit::turnTo(bofixed dir)
{
 if (isDestroyed()) {
	boError(380) << k_funcinfo << "unit is already destroyed!" << endl;
	return true;
 }

 if (rotation() != dir) {
	// Find out how much we have to turn
	bofixed delta = QABS(rotation() - dir);
	if (delta > 180) {
		delta = 360 - delta;
	}

	if (delta < unitProperties()->rotationSpeed()) {
		// Turn immediately (and hope this method won't be called more than once per
		//  advance call)
		setRotation(dir);
		updateRotation();
		return true;
	}
	boDebug(380) << k_funcinfo << id() << ": will slowly rotate from " << rotation() << " to " << dir << endl;
	addCurrentSuborder(new UnitTurnOrder(dir));
	return false;
	// If we're moving, we want to take one more step with current velocity, but
	//  setAdvanceWork() resets it to 0, so we have this workaround here
	/*bofixed _xVelocity = 0, _yVelocity = 0;
	if (advanceWork() == WorkMove) {
		_xVelocity = xVelocity();
		_yVelocity = yVelocity();
	}
	d->mWantedRotation = dir;
	setAdvanceWork(WorkTurn);
	setVelocity(_xVelocity, _yVelocity, 0);*/
 } else {
	// Unit already has correct rotation
	return true;
 }
}

bool Unit::turnToUnit(Unit* target)
{
 if (isDestroyed()) {
	boError(380) << k_funcinfo << "unit is already destroyed!" << endl;
	return true;
 }

 bofixed wanteddir = Bo3dTools::rotationToPoint(target->centerX() - centerX(), target->centerY() - centerY());
 boDebug() << k_funcinfo << "rotation: " << rotation() << "; wanted: " << wanteddir << endl;
 if (rotation() != wanteddir) {
	// Find out how much we have to turn
	bofixed delta = QABS(rotation() - wanteddir);
	if (delta > 180) {
		delta = 360 - delta;
	}

	if (delta < unitProperties()->rotationSpeed()) {
		// Turn immediately (and hope this method won't be called more than once per
		//  advance call)
		setRotation(wanteddir);
		updateRotation();
		return true;
	}
	boDebug(380) << k_funcinfo << id() << ": will slowly rotate from " << rotation() << " to " << wanteddir << endl;
	addCurrentSuborder(new UnitTurnToUnitOrder(target));
	return false;
 } else {
	// Unit already has correct rotation
	return true;
 }
}

void Unit::loadWeapons()
{
 if (d->mWeapons) {
	boWarning() << k_funcinfo << "Weapons already loaded! doing nothing." << endl;
	return;
 }

 // since we use an array and not a list we need to count the weapons first.
 int count = 0;
 bool hasbomb = false;
 bool hasmine = false;
 QPtrListIterator<PluginProperties> it(*(unitProperties()->plugins()));
 for (; it.current(); ++it) {
	if (it.current()->pluginType() == PluginProperties::Weapon) {
		count++;
		// Check for bombing weapons and create bombing plugin if necessary
		if (!hasbomb && ((BosonWeaponProperties*)it.current())->shotType() == BosonShot::Bomb) {
#warning FIXME: this does _not_ belog to plugins, but to weapons!
			d->mPlugins.append(new BombingPlugin(this));
			hasbomb = true;
		}
		// Check for mine weapons and create mining plugin if necessary
		if (!hasmine && ((BosonWeaponProperties*)it.current())->shotType() == BosonShot::Mine) {
#warning FIXME: this does _not_ belog to plugins, but to weapons!
			d->mPlugins.append(new MiningPlugin(this));
			hasmine = true;
		}
	}
 }
 if (count > MAX_WEAPONS_PER_UNIT) {
	boError() << k_funcinfo << "Too many weapons in this unit! type=" << type() << endl;
	count = MAX_WEAPONS_PER_UNIT;
 }
 d->mWeapons = new BosonWeapon*[count + 1]; // last element MUST be 0
 for (int i = 0; i < count + 1; i++) {
	d->mWeapons[i] = 0;
 }
 int weaponPos = 0;
 it.toFirst();
 for (; it.current() && weaponPos < count; ++it) {
	if (it.current()->pluginType() == PluginProperties::Weapon) {
		BosonWeaponProperties* prop = (BosonWeaponProperties*)it.current();
		d->mWeapons[weaponPos] = new BosonWeapon(weaponPos, prop, this);
		weaponPos++;
	}
 }
 recalculateMaxWeaponRange();
}

bool Unit::canShootAt(Unit *u) const
{
 BoPointerIterator<BosonWeapon> it(d->mWeapons);
 for (; *it; ++it) {
	if ((*it)->canShootAt(u)) {
		return true;
	}
 }
 return false;
}

bool Unit::canCrush(Unit *u) const
{
 if (!u || u == this) {
	return false;
 }
 if (u->isFlying() && !isFlying()) {
	return false;
 }
 if (isFlying()) {
	// AB: atm flying units won't crush each other
	return false;
 }
 if (u->maxHealth() <= unitProperties()->crushDamage()) {
	return true;
 }
 return false;
}

bofixed Unit::distanceSquared(const Unit* u) const
{
 bofixed dx = QMAX(bofixed(0), QABS(centerX() - u->centerX()) - width()/2 - u->width()/2);
 bofixed dy = QMAX(bofixed(0), QABS(centerY() - u->centerY()) - height()/2 - u->height()/2);
 bofixed dz = QMAX(bofixed(0), QABS(centerZ() - u->centerZ()) - depth()/2 - u->depth()/2);
 return dx*dx + dy*dy + dz*dz;
}

bofixed Unit::distanceSquared(const BoVector3Fixed& pos) const
{
 bofixed dx = QMAX(bofixed(0), QABS(pos.x() - centerX()) - width()/2);
 bofixed dy = QMAX(bofixed(0), QABS(pos.y() - centerY()) - height()/2);
 bofixed dz = QMAX(bofixed(0), QABS(pos.z() - centerZ()) - depth()/2);
 return dx*dx + dy*dy + dz*dz;
}

const QColor* Unit::teamColor() const
{
 return &ownerIO()->teamColor();
}

BosonWeapon* Unit::weapon(unsigned long int id) const
{
 BoPointerIterator<BosonWeapon> it(d->mWeapons);
 for (; *it; ++it) {
	if ((*it)->properties()->id() == id) {
		return *it;
	}
 }
 return 0;
}

void Unit::updateRotation()
{
 bofixed rotateX = 0.0f;
 bofixed rotateY = 0.0f;
 bofixed moveZ = 0.0f;
 updateZ(0.0f, 0.0f, &moveZ, &rotateX, &rotateY);
 setXRotation(rotateX);
 setYRotation(rotateY);
}

void Unit::setMovingStatus(MovingStatus m)
{
 if(movingStatus() == m) {
	return;
 }
 MovingStatus old = movingStatus();
 UnitBase::setMovingStatus(m);
 canvas()->unitMovingStatusChanges(this, old, m);
}

BosonPathInfo* Unit::pathInfo() const
{
 if (currentOrder() && currentOrder()->isMoveOrder()) {
	return ((UnitMoveOrderData*)currentOrderData())->pathinfo;
 } else {
	return 0;
 }
}

void Unit::unitDestroyed(Unit* unit)
{
 if (unit == this) {
	return;
 }
 // TODO: it might be that e.g. we currently have a turn order which is child
 //  of the attack order with destoryed unit as a target. So we should check
 //  all orders we have (including all toplevel ones), not just the current one
 if (currentOrder() && currentOrder()->type() == UnitOrder::AttackUnit) {
	if (((UnitAttackOrder*)currentOrder())->target() == unit) {
		currentSuborderDone(true);
	}
 }
 QPtrListIterator<UnitPlugin> it(d->mPlugins);
 while (it.current()) {
	it.current()->unitDestroyed(unit);
	++it;
 }
}

void Unit::itemRemoved(BosonItem* item)
{
 UnitBase::itemRemoved(item);
 if (item == (BosonItem*)this) {
	return;
 }
 // TODO: see comment in unitDestoryed() above
 if (currentOrder() && currentOrder()->type() == UnitOrder::AttackUnit) {
	if ((BosonItem*)((UnitAttackOrder*)currentOrder())->target() == item) {
		currentSuborderDone(true);
	}
 }
 QPtrListIterator<UnitPlugin> it(d->mPlugins);
 while (it.current()) {
	it.current()->itemRemoved(item);
	++it;
 }
}

void Unit::addUpgrade(const UpgradeProperties* upgrade)
{
 UnitBase::addUpgrade(upgrade);
 recalculateMaxWeaponRange();
 if (d->mUnitMover) {
	d->mUnitMover->addUpgrade(upgrade);
 }
}

void Unit::removeUpgrade(const UpgradeProperties* upgrade)
{
 UnitBase::removeUpgrade(upgrade);
 recalculateMaxWeaponRange();
 if (d->mUnitMover) {
	d->mUnitMover->removeUpgrade(upgrade);
 }
}

unsigned long int Unit::maxWeaponRange() const
{
 return d->mMaxWeaponRange;
}

unsigned long int Unit::maxAirWeaponRange() const
{
 return d->mMaxAirWeaponRange;
}

unsigned long int Unit::maxLandWeaponRange() const
{
 return d->mMaxLandWeaponRange;
}

void Unit::recalculateMaxWeaponRange()
{
 d->mMaxAirWeaponRange = 0;
 d->mMaxLandWeaponRange = 0;
 for (BoPointerIterator<BosonWeapon> it = d->mWeapons; *it; ++it) {
	const BosonWeapon* w = *it;
	if (!w->properties()) {
		boError() << k_funcinfo << "NULL properties for weapon " << w << endl;
		continue;
	}
	if (w->properties()->canShootAtAirUnits()) {
		if (w->range() > d->mMaxAirWeaponRange) {
			d->mMaxAirWeaponRange = w->range();
		}
	}
	if (w->properties()->canShootAtLandUnits()) {
		if (w->range() > d->mMaxLandWeaponRange) {
			d->mMaxLandWeaponRange = w->range();
		}
	}
 }
 d->mMaxWeaponRange = QMAX(d->mMaxAirWeaponRange, d->mMaxLandWeaponRange);
}

bool Unit::cellOccupied(int x, int y, bool ignoremoving) const
{
 // TODO: move this method away from here and merge it with the one in the pathfinder
 if (unitProperties()->isAircraft()) {
	return false;
 }
 if (!moveData()) {
	BO_NULL_ERROR(moveData());
	return false;
 }

 if (x < moveData()->edgedist1 || y < moveData()->edgedist1 ||
	x > (int)canvas()->mapWidth() - 1 - moveData()->edgedist2 ||
	y > (int)canvas()->mapHeight() - 1 - moveData()->edgedist2) {
	return true;
 }

 for (int x2 = x - moveData()->edgedist1; x2 <= x + moveData()->edgedist2; x2++) {
	for (int y2 = y - moveData()->edgedist1; y2 <= y + moveData()->edgedist2; y2++) {
		if (!moveData()->cellPassable[y2 * canvas()->mapWidth() + x2]) {
			return true;
		}

		const BoItemList* items = canvas()->cell(x2, y2)->items();
		for (BoItemList::ConstIterator it = items->begin(); it != items->end(); ++it) {
			if (RTTI::isUnit((*it)->rtti())) {
				Unit* u = (Unit*)*it;
				if(u->isFlying()) {
					// We don't care about air units
					continue;
				} else if(u == this) {
					continue;
				}

				// Maybe we can just crush the obstacle
				if (u->maxHealth() <= moveData()->crushDamage) {
					// Check player's relationship with the u's owner
					if (ownerIO()->isEnemy(u)) {
						// Crush the damn enemy :-)
						continue;
					} else if (ownerIO()->isNeutral(u)) {
						// Also crush it
						// TODO: maybe have additional cost for crushing neutral stuff???
						continue;
					}
				}

				if (ignoremoving && u->movingStatus() == UnitBase::Moving) {
					continue;
				}

				// This one's occupying the cell
				return true;
			}
			// TODO: check for e.g. mines
		}

	}
 }

 return false;
}

int Unit::getAnimationMode() const
{
 if (isDestroyed()) {
	return UnitAnimationWreckage;
 }
 if (mUnitConstruction && !mUnitConstruction->isConstructionComplete()) {
	return UnitAnimationConstruction;
 }
 return BosonItem::getAnimationMode();
}

void Unit::moveIdle()
{
 if (unitMover()) {
	unitMover()->advanceIdle();
 }
}

UnitOrderData* Unit::currentOrderData() const
{
 return d->mOrderQueue->currentOrderData();
}

UnitOrder* Unit::currentOrder() const
{
 return d->mOrderQueue->currentOrder();
}

UnitOrderData* Unit::toplevelOrderData() const
{
 return d->mOrderQueue->toplevelOrderData();
}

UnitOrder* Unit::toplevelOrder() const
{
 return d->mOrderQueue->toplevelOrder();
}

void Unit::clearOrders()
{
 d->mOrderQueue->clearOrders();
}

bool Unit::replaceToplevelOrders(UnitOrder* order)
{
 return d->mOrderQueue->replaceToplevelOrders(order);
}

bool Unit::addToplevelOrder(UnitOrder* order)
{
 return d->mOrderQueue->addToplevelOrder(order);
}

bool Unit::currentOrderChanged()
{
 if (!currentOrderData() || !currentOrder()) {
	setAdvanceWork(WorkIdle);
	return true;
 }

 setAdvanceWork(currentOrder()->work());

 // For WorkPlugin, we also need to set correct mCurrentPlugin
 if (currentOrder()->work() == WorkPlugin) {
	mCurrentPlugin = plugin(currentOrder()->workPluginType());
	if (!mCurrentPlugin) {
		boError() << k_funcinfo << "cannot find valid plugin for WorkPlugin" << endl;
		return false;
	}
 }

 return true;
}

bool Unit::currentOrderAdded()
{
 UnitOrder* order = currentOrder();
 if (!order) {
	BO_NULL_ERROR(order);
	return false;
 }
// switch (order->type()) {
	if (order->type() == UnitOrder::Move || order->type() == UnitOrder::MoveInsideUnit) {
		bool moveToCenterOfCell = true;
		UnitMoveOrder* moveo = (UnitMoveOrder*)order;
		bofixed x = moveo->position().x();
		bofixed y = moveo->position().y();

		if (moveToCenterOfCell) {
			// We want land unit's center point to be in the middle of the cell after
			//  moving.
			if (!unitProperties()->isAircraft()) {
				bofixed add = 0;
				if (!moveData()) {
					boDebug() << k_funcinfo << "no moveData. not adding order." << endl;
					return false;
				}
				add = (((moveData()->size % 2) == 1) ? 0.5 : 0);
				x = (int)moveo->position().x() + add;
				y = (int)moveo->position().y() + add;
			} else {
				// AB: flying units barely use the cells for
				// moving, so there is no point at cell-exact
				// moving.
			}
		}

		if (moveTo(x, y, moveo->range())) {
			boDebug(380) << k_funcinfo << "unit " << id() << ": Will move to (" << x << "; " << y << ")" << endl;
			pathInfo()->moveAttacking = moveo->withAttacking();
			pathInfo()->slowDownAtDest = true;
		} else {
			boDebug(380) << k_funcinfo << "unit " << id() << ": CANNOT move to (" << x << "; " << y << ")" << endl;
			return false;
		}

	} else if (order->type() == UnitOrder::MoveToUnit) {
		UnitMoveToUnitOrder* movetounito = (UnitMoveToUnitOrder*)order;
		const BoVector2Fixed& pos = movetounito->target()->center();
		if (moveTo(pos.x(), pos.y(), movetounito->range())) {
			boDebug(380) << k_funcinfo << "unit " << id() << ": Will move to unit " << movetounito->target()->id() << endl;
			pathInfo()->moveAttacking = movetounito->withAttacking();
			pathInfo()->slowDownAtDest = true;
		} else {
			boDebug(380) << k_funcinfo << "unit " << id() << ": CANNOT move to unit " << movetounito->target()->id() << endl;
			return false;
		}

	} else if (order->type() == UnitOrder::AttackUnit) {
		UnitAttackOrder* attacko = (UnitAttackOrder*)order;
		if (!canShootAt(attacko->target())) {
			return false;
		}

	} else if (order->type() == UnitOrder::Follow) {

	} else if (order->type() == UnitOrder::Turn) {
	} else if (order->type() == UnitOrder::TurnToUnit) {

	} else if (order->type() == UnitOrder::Harvest) {
		UnitHarvestOrder* harvestorder = (UnitHarvestOrder*)order;
		HarvesterPlugin* h = (HarvesterPlugin*)plugin(UnitPlugin::Harvester);
		if (!h) {
			boError() << k_lineinfo << "only harvester can mine" << endl;
			return false;
		}
		ResourceMinePlugin* r = (ResourceMinePlugin*)harvestorder->target()->plugin(UnitPlugin::ResourceMine);
		if (!r) {
			boError() << k_lineinfo << "can mine at resource mine only" << endl;
			return false;
		}
		h->mineAt(r);

	} else if (order->type() == UnitOrder::Refine) {
		UnitRefineOrder* refineorder = (UnitRefineOrder*)order;
		HarvesterPlugin* h = (HarvesterPlugin*)plugin(UnitPlugin::Harvester);
		if (!h) {
			boError() << k_lineinfo << "only harvester can execute refine orders" << endl;
			return false;
		}
		RefineryPlugin* refinery = (RefineryPlugin*)refineorder->target()->plugin(UnitPlugin::Refinery);
		if (!refinery) {
			boWarning() << k_lineinfo << "invalid refinery given" << endl;
			return false;
		}
		h->refineAt(refinery);
	} else if (order->type() == UnitOrder::EnterUnit) {
		UnitEnterUnitOrder* enterOrder = (UnitEnterUnitOrder*)order;
		if (!isMobile()) {
			boError() << k_lineinfo << "only mobile units can enter other units" << endl;
			return false;
		}
		EnterUnitPlugin* enterUnit = (EnterUnitPlugin*)plugin(UnitPlugin::EnterUnit);
		if (!enterUnit) {
			boError() << k_lineinfo << "cannot enter a unit: no EnterUnit plugin" << endl;
			return false;
		}

		if (enterOrder->isLeaveOrder()) {
			if (!enterUnit->leave()) {
				return false;
			}
		} else {
			if (!enterUnit->enter(enterOrder->target())) {
				return false;
			}
		}
	} else {
		// Invalid order
		boError() << k_funcinfo << "Invalid current order type " << order->type() << endl;
		return false;
 }

 return true;
}

bool Unit::addCurrentSuborder(UnitOrder* order)
{
 return d->mOrderQueue->addCurrentSuborder(order);
}

void Unit::currentSuborderDone(bool success)
{
 d->mOrderQueue->currentSuborderDone(success);
}

void Unit::currentSuborderRemoved()
{
 UnitOrder* order = currentOrder();
 if (!order) {
	return;
 }

 if (order->isMoveOrder()) {
	clearPathPoints();
	if (!isFlying()) {
		setMovingStatus(Standing);
		setVelocity(0.0, 0.0, 0.0);

		if (pathInfo() && pathInfo()->slowDownAtDest) {
			setSpeed(0);
		}
	}
 }

 if (order->type() == UnitOrder::EnterUnit) {
	EnterUnitPlugin* e = (EnterUnitPlugin*)plugin(UnitPlugin::EnterUnit);
	if (e) {
		e->abort();
	}
 }
}

bool Unit::canAddOrder() const
{
 if (isDestroyed()) {
	return false;
 }
 if (mUnitConstruction && !mUnitConstruction->isConstructionComplete()) {
	return false;
 }

 UnitStoragePlugin* storage = (UnitStoragePlugin*)plugin(UnitPlugin::UnitStorage);
 if (storage) {
	if (storage->storageStatus() != UnitStoragePlugin::StatusDoorsClosed) {
		return false;
	}
 }

 return true;
}

bool Unit::lastOrderStatus() const
{
 return d->mOrderQueue->lastOrderStatus();
}

void Unit::setIsInsideUnit(bool inside)
{
 d->mIsInsideUnit = inside;
}

bool Unit::isInsideUnit() const
{
 return d->mIsInsideUnit.value();
}

UnitMover* Unit::unitMover() const
{
 if (isInsideUnit()) {
	return d->mUnitInsideUnitMover;
 }
 return d->mUnitMover;
}

void Unit::setIsFlying(bool f)
{
 if (!unitProperties()) {
	BO_NULL_ERROR(f);
	return;
 }
 if (!unitProperties()->isAircraft()) {
	if (f) {
		boError() << k_funcinfo << "is not an aircraft" << endl;
	}
	return;
 }
 d->mIsFlying = f;
}

bool Unit::isFlying() const
{
 if (!unitProperties()) {
	return false;
 }
 if (!unitProperties()->isAircraft()) {
	return false;
 }
 return d->mIsFlying;
}



/////////////////////////////////////////////////
// Facility
/////////////////////////////////////////////////

UnitConstruction::UnitConstruction(Unit* f)
{
 mFacility = f;

 unit()->registerData(&mConstructionStep, Unit::IdConstructionStep);
 mConstructionStep.setLocal(0);
}

UnitConstruction::~UnitConstruction()
{
}

void UnitConstruction::advanceConstruction(unsigned int advanceCallsCount)
{
 if (advanceCallsCount % 20 != 0) {
	return;
 }
 BosonProfiler profiler("advanceConstruction");
 if (unit()->isDestroyed()) {
	boError() << k_funcinfo << "unit is already destroyed" << endl;
	return;
 }
 setConstructionStep(currentConstructionStep() + 1);
}

bool UnitConstruction::isConstructionComplete() const
{
 if (unit()->advanceWork() == UnitBase::WorkConstructed) {
	return false;
 }
 if (currentConstructionStep() < constructionSteps()) {
	return false;
 }
 return true;
}

double UnitConstruction::constructionProgress() const
{
 unsigned int constructionTime = constructionSteps();
 double percentage = (double)(currentConstructionStep() * 100) / (double)constructionTime;
 return percentage;
}

unsigned int UnitConstruction::constructionSteps() const
{
 return unit()->unitProperties()->constructionSteps();
}

void UnitConstruction::setConstructionStep(unsigned int step)
{
 if (step > constructionSteps()) {
	step = constructionSteps();
 }

 mConstructionStep = step;
 if (step == constructionSteps()) {
	unit()->setAdvanceWork(UnitBase::WorkIdle);
	unit()->owner()->facilityCompleted(unit());
	unit()->updateAnimationMode();
	// TODO: should this be moved somewhere else?
	if (unit()->plugin(UnitPlugin::Radar)) {
		unit()->canvas()->addRadar(unit());
	}
	if (unit()->plugin(UnitPlugin::RadarJammer)) {
		unit()->canvas()->addRadarJammer(unit());
	}
 }
}

unsigned int UnitConstruction::currentConstructionStep() const
{
 return mConstructionStep;
}

bool UnitConstruction::loadFromXML(const QDomElement& root)
{
 if (mConstructionStep > constructionSteps()) {
	mConstructionStep = constructionSteps();
 }

 unit()->updateAnimationMode();
 if (isConstructionComplete()) {
	if (unit()->plugin(UnitPlugin::Radar)) {
		unit()->canvas()->addRadar(unit());
	}
	if (unit()->plugin(UnitPlugin::RadarJammer)) {
		unit()->canvas()->addRadarJammer(unit());
	}
 }

 return true;
}

bool UnitConstruction::saveAsXML(QDomElement&)
{
 return true;
}


