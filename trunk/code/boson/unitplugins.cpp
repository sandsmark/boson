/*
    This file is part of the Boson game
    Copyright (C) 2002-2005 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2002-2005 Rivo Laks (rivolaks@hot.ee)

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
#include "unitplugins.h"

#include "../bomemory/bodummymemory.h"
#include "defines.h"
#include "unit.h"
#include "unitproperties.h"
#include "pluginproperties.h"
#include "speciestheme.h"
#include "player.h"
#include "playerio.h"
#include "bosoncanvas.h"
#include "boson.h"
#include "boitemlist.h"
#include "bosonstatistics.h"
#include "cell.h"
#include "upgradeproperties.h"
#include "bodebug.h"
#include "bosonweapon.h"
#include "bosonpath.h"
#include "boevent.h"
#include "bo3dtools.h"

#include <klocale.h>

#include <qpair.h>
#include <qdom.h>
#include <qstringlist.h>

#include <math.h>

UnitPlugin::UnitPlugin(Unit* unit)
{
 mUnit = unit;
}

UnitPlugin::~UnitPlugin()
{
}

SpeciesTheme* UnitPlugin::speciesTheme() const
{
 return unit()->speciesTheme();
}

Player* UnitPlugin::player() const
{
 return unit()->owner();
}

const UnitProperties* UnitPlugin::unitProperties() const
{
 return unit()->unitProperties();
}

const PluginProperties* UnitPlugin::properties(int propertyType) const
{
 return unit()->properties(propertyType);
}

KGamePropertyHandler* UnitPlugin::dataHandler() const
{
 return unit()->dataHandler();
}

BosonCanvas* UnitPlugin::canvas() const
{
 return unit()->canvas();
}

Boson* UnitPlugin::game() const
{
 if (!player()) {
	return 0;
 }
 return (Boson*)player()->game();
}

const BoUpgradesCollection& UnitPlugin::upgradesCollection() const
{
 return unit()->upgradesCollection();
}


ProductionPlugin::ProductionPlugin(Unit* unit) : UnitPlugin(unit)
{
// unit->registerData(&mProductions, Unit::IdProductions);
 unit->registerData(&mProductionState, Unit::IdProductionState);
 mProductionState.setLocal(0);
// mProductions.setEmittingSignal(false); // just to prevent warning in Player::slotUnitPropertyChanged()
 mProductionState.setEmittingSignal(false); // called quite often - not emitting will increase speed a little bit
}

ProductionPlugin::~ProductionPlugin()
{
}

unsigned long int ProductionPlugin::completedProductionId() const
{
 if (!hasProduction()) {
	return 0;
 }
 unsigned long int id = currentProductionId();
 if (id == 0) {
	return 0;
 }
 if (completedProductionType() == ProduceUnit) {
	if (mProductionState < speciesTheme()->unitProperties(id)->productionTime()) {
//		boDebug() << k_funcinfo << "not yet completed: " << id << endl;
		return 0;
	}
 } else {
	if (mProductionState < speciesTheme()->technology(id)->productionTime()) {
//		boDebug() << k_funcinfo << "not yet completed: " << id << endl;
		return 0;
	}
 }
 return id;
}

ProductionType ProductionPlugin::completedProductionType() const
{
 return currentProductionType();
}

void ProductionPlugin::productionPlaced(Unit* produced)
{
 BO_CHECK_NULL_RET(produced);
 if (produced->isFacility()) {
	player()->statistics()->addProducedFacility((Facility*)produced, this);
 } else {
	player()->statistics()->addProducedMobileUnit((MobileUnit*)produced, this);
 }

 BoEvent* productionPlaced = new BoEvent("ProducedUnitWithTypePlaced", QString::number(produced->type()), QString::number(unit()->id()));
 productionPlaced->setUnitId(produced->id());
 productionPlaced->setPlayerId(produced->owner()->bosonId());
 productionPlaced->setLocation(BoVector3Fixed(produced->x(), produced->y(), produced->z()));
 boGame->queueEvent(productionPlaced);

 // the current production is done.
 removeProduction();
}

void ProductionPlugin::addProduction(ProductionType type, unsigned long int id)
{
 BO_CHECK_NULL_RET(unit());
 BO_CHECK_NULL_RET(unitProperties());
 BO_CHECK_NULL_RET(player());
 BO_CHECK_NULL_RET(speciesTheme());
 BO_CHECK_NULL_RET(game());
 ProductionProperties* p = (ProductionProperties*)unitProperties()->properties(PluginProperties::Production);
 if (!p) {
	boError() << k_funcinfo << "NULL production properties" << endl;
	return;
 }

 unsigned long int mineralCost = 0;
 unsigned long int oilCost = 0;

 if (type == ProduceUnit) {
	const UnitProperties* prop = speciesTheme()->unitProperties(id);
	if (!prop) {
		boError() << k_lineinfo << "NULL unit properties (EVIL BUG)" << endl;
		return;
	}
	if (!possibleUnitProductions().contains(id)) {
		boError() << k_funcinfo << " cannot produce unit witht type " << id << endl;
		game()->slotAddChatSystemMessage(i18n("Cannot produce unit %1").arg(prop->name()), player());
		return;
	}

	mineralCost = prop->mineralCost();
	oilCost = prop->oilCost();
 } else if (type == ProduceTech) {
	const UpgradeProperties* prop = speciesTheme()->technology(id);
	if (!prop) {
		boError() << k_lineinfo << "NULL technology properties (EVIL BUG)" << endl;
		return;
	}
	if (!possibleTechnologyProductions().contains(id)) {
		boError() << k_funcinfo << " cannot produce technology with id " << id << endl;
		game()->slotAddChatSystemMessage(i18n("Cannot produce technology %1").arg(prop->upgradeName()), player());
		return;
	}

	mineralCost = prop->mineralCost();
	oilCost = prop->oilCost();
 } else {
	boError() << k_funcinfo << "Invalid productionType: " << (int)type << endl;
	return;
 }

 if (player()->minerals() < mineralCost) {
	game()->slotAddChatSystemMessage(i18n("You have not enough minerals!"), player());
	return;
 }
 if (player()->oil() < oilCost) {
	game()->slotAddChatSystemMessage(i18n("You have not enough oil!"), player());
	return;
 }
 player()->setMinerals(player()->minerals() - mineralCost);
 player()->setOil(player()->oil() - oilCost);


 bool start = false;
 if (!hasProduction()) {
	start = true;
 }
 QPair<ProductionType, unsigned long int> pair;
 pair.first = type;
 pair.second = id;
 mProductions.append(pair);
 if (start) {
	unit()->setPluginWork(pluginType());
 }

 QCString eventName;
 if (type == ProduceUnit) {
	eventName = "StartProductionOfUnitWithType";
 } else if (type == ProduceTech) {
	eventName = "StartProductionOfTechnologyWithType";
 } else {
	boError() << k_funcinfo << "Invalid productionType: " << (int)type << endl;
 }
 if (!eventName.isNull()) {
	BoEvent* event = new BoEvent(eventName, QString::number(id), QString::number(unit()->id()));
	event->setPlayerId(player()->bosonId());
	event->setLocation(BoVector3Fixed(unit()->x(), unit()->y(), unit()->z()));
	game()->queueEvent(event);
 }
}

void ProductionPlugin::pauseProduction()
{
 if (unit()->currentPluginType() != UnitPlugin::Production) {
	// already paused
	return;
 }
 if (currentProductionId() == 0) {
	// no production
	boWarning() << k_funcinfo << "no current production" << endl;
	return;
 }

 unit()->setWork(Unit::WorkIdle);

 QCString eventName;
 if (currentProductionType() == ProduceUnit) {
	eventName = "PauseProductionOfUnitWithType";
 } else if (currentProductionType() == ProduceTech) {
	eventName = "PauseProductionOfTechnologyWithType";
 } else {
	boError() << k_funcinfo << "Invalid productionType: " << (int)currentProductionType() << endl;
 }

 if (!eventName.isNull()) {
	BoEvent* event = new BoEvent(eventName, QString::number(currentProductionId()), QString::number(unit()->id()));
	event->setPlayerId(player()->bosonId());
	event->setLocation(BoVector3Fixed(unit()->x(), unit()->y(), unit()->z()));
	game()->queueEvent(event);
 }
}

void ProductionPlugin::unpauseProduction()
{
 if (unit()->currentPluginType() == UnitPlugin::Production) {
	// not paused
	return;
 }
 if (currentProductionId() == 0) {
	// no production
	boWarning() << k_funcinfo << "no current production" << endl;
	return;
 }

 unit()->setPluginWork(UnitPlugin::Production);


 QCString eventName;
 if (currentProductionType() == ProduceUnit) {
	eventName = "ContinueProductionOfUnitWithType";
 } else if (currentProductionType() == ProduceTech) {
	eventName = "ContinueProductionOfTechnologyWithType";
 } else {
	boError() << k_funcinfo << "Invalid productionType: " << (int)currentProductionType() << endl;
 }
 if (!eventName.isNull()) {
	BoEvent* event = new BoEvent(eventName, QString::number(currentProductionId()), QString::number(unit()->id()));
	event->setPlayerId(player()->bosonId());
	event->setLocation(BoVector3Fixed(unit()->x(), unit()->y(), unit()->z()));
	game()->queueEvent(event);
 }
}

void ProductionPlugin::abortProduction(ProductionType type, unsigned long int id)
{
 if (!hasProduction()) {
	// no production
	boWarning() << k_funcinfo << "no productions" << endl;
	return;
 }

 unsigned long int mineralCost = 0, oilCost = 0;

 if (type == ProduceUnit) {
	const UnitProperties* prop = player()->unitProperties(id);
	if (!prop) {
		boError() << k_lineinfo << "NULL unit properties (EVIL BUG)" << endl;
		return;
	}
	mineralCost = prop->mineralCost();
	oilCost = prop->oilCost();
 } else if (type == ProduceTech) {
	const UpgradeProperties* prop = player()->speciesTheme()->technology(id);
	if (!prop) {
		boError() << k_lineinfo << "NULL technology properties (EVIL BUG)" << endl;
		return;
	}
	mineralCost = prop->mineralCost();
	oilCost = prop->oilCost();
 } else {
	boError() << k_funcinfo << "Invalid productionType: " << (int)type << endl;
	return;
 }


 QString eventTypeParameter = QString::number(id);
 QCString eventName;
 if (type == ProduceUnit) {
	eventName = "StopProductionOfUnitWithType";
 } else if (type == ProduceTech) {
	eventName = "StopProductionOfTechnologyWithType";
 } else {
	boError() << k_funcinfo << "Invalid productionType: " << (int)type << endl;
 }

 if (!removeProduction(type, id)) {
	boError() << k_funcinfo << "no such production available" << endl;
	return;
 }

 // FIXME: money should be paid when the production is
 // actually started! (currently it is paid as soon as an
 // item is added to the queue)
 player()->setMinerals(player()->minerals() + mineralCost);
 player()->setOil(player()->oil() + oilCost);

 if (!eventName.isNull()) {
	BoEvent* event = new BoEvent(eventName, QString::number(id), QString::number(unit()->id()));
	event->setPlayerId(player()->bosonId());
	event->setLocation(BoVector3Fixed(unit()->x(), unit()->y(), unit()->z()));
	game()->queueEvent(event);
 }
}

bool ProductionPlugin::removeProduction()
{
 if (!hasProduction()) {
	return false;
 }
 return removeProduction(currentProductionType(), currentProductionId());
}

bool ProductionPlugin::removeProduction(ProductionType type, unsigned long int id)
{
 for (unsigned int i = 0; i < productionList().count(); i++) {
	if ((mProductions[i].first == type) && (mProductions[i].second == id)) {
		boDebug() << k_funcinfo << "remove; type: " << type << ", id: " << id << endl;
		mProductions.remove(mProductions.at(i));
		if (i == 0) {
			mProductionState = 0; // start next production (if any)
		}
		return true;
	}
 }
 return false;
}

double ProductionPlugin::productionProgress() const
{
 unsigned int productionTime = 0;
 if (currentProductionType() == ProduceUnit) {
	productionTime = speciesTheme()->unitProperties(currentProductionId())->productionTime();
 } else if (currentProductionType() == ProduceTech) {
	productionTime = speciesTheme()->technology(currentProductionId())->productionTime();
 } else {
	boDebug() << k_funcinfo << "Unknown productiontype: " << currentProductionType() << endl;
 }
 double percentage = (double)(mProductionState * 100) / (double)productionTime;
 return percentage;
}

void ProductionPlugin::advance(unsigned int)
{
 unsigned long int id = currentProductionId();
 if (id <= 0) { // no production
	unit()->setWork(Unit::WorkIdle);
	mProductionState = 0;
	return;
 }



 // a unit is completed as soon as mProductionState == player()->unitProperties(type)->productionTime()
 unsigned int productionTime = 0;
 if (currentProductionType() == ProduceUnit) {
	productionTime = speciesTheme()->unitProperties(currentProductionId())->productionTime();
 } else if (currentProductionType() == ProduceTech) {
	productionTime = speciesTheme()->technology(currentProductionId())->productionTime();
 } else {
	boDebug() << k_funcinfo << "Unknown productiontype: " << currentProductionType() << endl;
 }
 if (mProductionState <= productionTime) {
	if (mProductionState == productionTime) {
		boDebug() << "Production with type " << currentProductionType() << " and id " << id << " completed :-)" << endl;
		mProductionState = mProductionState + 1;

		if (currentProductionType() != ProduceUnit) {
			// It's technology
			// these can be handled immediately.
			removeProduction();
			player()->technologyResearched(this, id);
			return;
		}

		// A unit must be placed on the map, before removeProduction()
		// may be called.

		// Auto-place unit
		// Unit positioning scheme: all tiles starting with tile that is below
		// facility's lower-left tile, are tested counter-clockwise. Unit is placed
		// to first free tile.
		// No auto-placing for facilities
		const UnitProperties* prop = speciesTheme()->unitProperties(id);
		if (!prop) {
			boError() << k_lineinfo << "Unknown id " << id << endl;
			return;
		}

		BoEvent* unitProduced = new BoEvent("UnitWithTypeProduced", QString::number(id),
				QString::number(unit()->id()));
		unitProduced->setPlayerId(player()->bosonId());
		game()->queueEvent(unitProduced);

		if (prop->isFacility()) {
			return;
		}

		boDebug() << k_funcinfo << "auto-placing unit "<< prop->typeId() << endl;

		int tilex, tiley; // Position of lower-left corner of facility in tiles
		int theight, twidth; // height and width of facility in tiles
		int currentx, currenty; // Position of tile currently tested

#warning converting to ints! -> we should use float here
		theight = unit()->height();
		twidth = unit()-> width();
		tilex = (int)(unit()->x());
		tiley = (int)(unit()->y() + theight);
		int tries; // Tiles to try for free space
		int ctry; // Current try
		currentx = tilex - 1;
		currenty = tiley - 1;
		for (int i = 1; i <= BUILD_RANGE; i++) {
			tries = 2 * i * twidth + 2 * i * theight + 4;
			currenty++;
			for (ctry = 1; ctry <= tries; ctry++) {
				if (ctry <= twidth + i) {
					currentx++;
				} else if (ctry <= twidth + i + theight + 2 * i - 1) {
					currenty--;
				} else if (ctry <= twidth + i + 2 * (theight + 2 * i - 1)) {
					currentx--;
				} else if (ctry <= twidth + i + 3 * (theight + 2 * i - 1)) {
					currenty++;
				} else {
					currentx++;
				}

				if (canvas()->canPlaceUnitAt(speciesTheme()->unitProperties(id), BoVector2Fixed(currentx, currenty), this)) {
					// Free cell - place unit at it
					mProductionState = mProductionState + 1;
					((Boson*)player()->game())->buildProducedUnit(this, id, BoVector2Fixed(currentx, currenty));
					return;
				}
			}
		}
		boDebug() << k_funcinfo << "Cannot find free cell around facility :-(" << endl;
		game()->slotAddChatSystemMessage(
				i18n("%1 could not be placed on the map - no free cell found. Place it manuall!").arg(prop->name()),
				player());
	} else {
		mProductionState = mProductionState + 1;
	}
 }
}

void ProductionPlugin::unitDestroyed(Unit* destroyedUnit)
{
 QValueList<unsigned long int> units = possibleUnitProductions();
 QValueList<unsigned long int> techs = possibleTechnologyProductions();
 QValueList< QPair<ProductionType, unsigned long int> > abort;
 QValueList<unsigned long int> abortTechs;
 for (unsigned int i = 0; i < mProductions.count(); i++) {
	unsigned long int id = mProductions[i].second;
	if (mProductions[i].first == ProduceUnit) {
		if (!units.contains(id)) {
			QPair<ProductionType, unsigned long int> pair;
			pair.first = ProduceUnit;
			pair.second = id;
			abort.append(pair);
		}
	} else if (mProductions[i].first == ProduceTech) {
		if (!techs.contains(id)) {
			QPair<ProductionType, unsigned long int> pair;
			pair.first = ProduceTech;
			pair.second = id;
			abort.append(pair);
		}
	} else {
		boError() << k_funcinfo << "production " << i << " has unexpected type" << endl;
		continue;
	}
 }

 for (unsigned int i = 0; i < abort.count(); i++) {
	abortProduction(abort[i].first, abort[i].second);
 }
 if (abort.count() > 0) {
	game()->slotAddChatSystemMessage(i18n("Production in %1 got aborted because %2 was required but destroyed").arg(unit()->name()).arg(destroyedUnit->name()), player());
 }
}

bool ProductionPlugin::saveAsXML(QDomElement& root) const
{
 if (mProductions.count() == 0) {
	return true;
 }

 // AB: we don't use a KGamePropertyList here, cause it is a bit more difficult
 // with the QPair I guess. simply saving this by hand.
 QStringList list;
 for (unsigned int i = 0; i < mProductions.count(); i++) {
	unsigned int type = (unsigned int)(mProductions[i].first);
	unsigned int id = mProductions[i].second;
	QString s = QString::fromLatin1("%1 %2").arg(type).arg(id);
	list.append(s);
 }
 QString productionString = list.join(QString::fromLatin1(","));
 QDomDocument doc = root.ownerDocument();
 QDomElement e = doc.createElement("ProductionList");
 root.appendChild(e);
 e.appendChild(doc.createTextNode(productionString));
 return true;
}

bool ProductionPlugin::loadFromXML(const QDomElement& root)
{
 mProductions.clear();
 QDomElement e = root.namedItem("ProductionList").toElement();
 if (e.isNull()) {
	// nothing to load here
	return true;
 }
 QString s = e.text();
 if (s.isEmpty()) {
	// list is here, but empty
	return true;
 }
 QStringList list = QStringList::split(QString::fromLatin1(","), s);
 for (unsigned int i = 0; i < list.count(); i++) {
	QStringList l = QStringList::split(" ", list[i]);
	if (l.count() != 2) {
		boWarning() << k_funcinfo << "type and id of a production item must be space separated" << endl;
		return false;
	}
	bool ok = false;
	unsigned int type = l[0].toUInt(&ok);
	if (!ok) {
		boError() << k_funcinfo << "type of production item " << i << " is not a valid number" << endl;
		return false;
	}
	unsigned int id = l[1].toUInt(&ok);
	if (!ok) {
		boError() << k_funcinfo << "id of production item " << i << " is not a valid number" << endl;
		return false;
	}
	QPair<ProductionType, unsigned long int> pair;
	pair.first = (ProductionType)type;
	pair.second = id;
	mProductions.append(pair);
 }
 return true;
}

bool ProductionPlugin::contains(ProductionType type, unsigned long int id)
{
 QPair<ProductionType, unsigned long int> pair;
 pair.first = type;
 pair.second = id;
 return productionList().contains(pair);
}

QValueList<unsigned long int> ProductionPlugin::possibleUnitProductions(QValueList<unsigned long int>* impossibleUnits) const
{
 QValueList<unsigned long int> ret;
 ProductionProperties* pp = (ProductionProperties*)properties(PluginProperties::Production);
 if (!pp) {
	// Must not happen when the unit has a ProductionPlugin
	boError() << k_funcinfo << "no ProductionProperties" << endl;
	return ret;
 }
 if (!speciesTheme()) {
	BO_NULL_ERROR(speciesTheme());
	return ret;
 }

 QValueList<unsigned long int> unitsList = speciesTheme()->productions(pp->producerList());

 if (impossibleUnits) {
	impossibleUnits->clear();
 }

 // Filter out things that player can't actually build (requirements aren't met yet)
 QValueList<unsigned long int>::Iterator it;
 it = unitsList.begin();
 for (; it != unitsList.end(); ++it) {
	if (player()->canBuild(*it)) {
		ret.append(*it);
	} else {
		if (impossibleUnits) {
			impossibleUnits->append(*it);
		}
	}
 }

 return ret;
}

QValueList<unsigned long int> ProductionPlugin::possibleTechnologyProductions(QValueList<unsigned long int>* impossibleTechnologies) const
{
 QValueList<unsigned long int> ret;
 ProductionProperties* pp = (ProductionProperties*)properties(PluginProperties::Production);
 if (!pp) {
	// Must not happen when the unit has a ProductionPlugin
	boError() << k_funcinfo << "no ProductionProperties" << endl;
	return ret;
 }
 if (!speciesTheme()) {
	BO_NULL_ERROR(speciesTheme());
	return ret;
 }

 QValueList<unsigned long int> techList = speciesTheme()->technologies(pp->producerList());

 if (impossibleTechnologies) {
	impossibleTechnologies->clear();
 }

 // Filter out things that player can't actually build (requirements aren't met yet)
 QValueList<unsigned long int>::Iterator it;
 for (it = techList.begin(); it != techList.end(); it++) {
	if ((!player()->hasTechnology(*it)) && (player()->canResearchTech(*it))) {
		ret.append(*it);
	} else {
		if (impossibleTechnologies) {
			impossibleTechnologies->append(*it);
		}
	}
 }

 return ret;
}


RepairPlugin::RepairPlugin(Unit* unit) : UnitPlugin(unit)
{
}

RepairPlugin::~RepairPlugin()
{
}

void RepairPlugin::repair(Unit* u)
{
 boDebug() << k_funcinfo << endl;
 if (unit()->isFacility()) {
	if (!u->moveTo(unit(), 0)) {
		boDebug() << k_funcinfo << u->id() << " cannot find a way to repairyard" << endl;
		u->setWork(Unit::WorkIdle);
	} else {
		u->setWork(Unit::WorkMove);
	}
 } else {
	if (!unit()->moveTo(u, 0)) {
		boDebug() << k_funcinfo << "Cannot find way to " << u->id() << endl;
		unit()->setWork(Unit::WorkIdle);
	} else {
		unit()->setAdvanceWork(Unit::WorkMove);
	}
 }
}

void RepairPlugin::repairInRange()
{
// boDebug() << k_funcinfo << endl;
 //TODO: support for friendly non-player (i.e. allied) units

 // TODO: once we started repairing a unit also repair it in the next call,
 // until it isn't in range anymore or is repaired
/* BoItemList list = unit()->unitsInRange();
 for (unsigned int i = 0; i < list.count(); i++) {
	Unit* u = (Unit*)list[i];
	if (u->health() >= u->unitProperties()->health()) {
		continue;
	}
	if (!player()->isEnemy(u->owner())) {
		boDebug() << k_funcinfo << "repair " << u->id() << endl;
		int diff = u->health() - u->unitProperties()->health();
		if (diff > 0) {
			boError() << k_funcinfo << "health > maxhealth" << endl;
			continue;
		}*/
/*		if (diff < unit()->weaponDamage()) {
			// usually the case
			u->setHealth(u->health() - unit()->weaponDamage());
		} else {
			u->setHealth(u->health() - diff);
		}*/
		/*return; // only one unit at once
	}
 }*/
}

bool RepairPlugin::saveAsXML(QDomElement& root) const
{
 Q_UNUSED(root);
 return true;
}

bool RepairPlugin::loadFromXML(const QDomElement& root)
{
 Q_UNUSED(root);
 return true;
}

void RepairPlugin::unitDestroyed(Unit*)
{
}

void RepairPlugin::itemRemoved(BosonItem*)
{
}


HarvesterPlugin::HarvesterPlugin(Unit* unit)
		: UnitPlugin(unit)
{
 // FIXME: we should clean the property IDs. They should be in UnitPlugin, not
 // in Unit.
 unit->registerData(&mResourcesMined, Unit::IdResourcesMined);
 unit->registerData(&mResourcesX, Unit::IdResourcesX);
 unit->registerData(&mResourcesY, Unit::IdResourcesY);
 unit->registerData(&mHarvestingType, Unit::IdHarvestingType);
 mResourcesMined.setLocal(0);
 mResourcesX.setLocal(0); // obsolete
 mResourcesY.setLocal(0); // obsolete
 mHarvestingType.setLocal(0);

 mRefinery = 0;
 mResourceMine = 0;
}

HarvesterPlugin::~HarvesterPlugin()
{
}

void HarvesterPlugin::advance(unsigned int)
{
 if (mHarvestingType == 0) {
	unit()->setWork(Unit::WorkIdle);
	return;
 } else if (mHarvestingType == 1) {
	advanceMine();
 } else if (mHarvestingType == 2) {
	advanceRefine();
 }
}

bool HarvesterPlugin::saveAsXML(QDomElement& root) const
{
 unsigned int refineryId = 0;
 unsigned int mine = 0;
 if (mRefinery) {
	refineryId = mRefinery->unit()->id();
 }
 if (mResourceMine) {
	mine = mResourceMine->unit()->id();
 }
 root.setAttribute(QString::fromLatin1("Refinery"), refineryId);
 root.setAttribute(QString::fromLatin1("ResourceMine"), mine);
 return true;
}

bool HarvesterPlugin::loadFromXML(const QDomElement& root)
{
 unsigned int refineryId = 0;
 unsigned int mineId = 0;
 bool ok = false;

 mRefinery = 0;
 mResourceMine = 0;

 refineryId = root.attribute(QString::fromLatin1("Refinery")).toUInt(&ok);
 if (!ok) {
	boError() << k_funcinfo << "Invalid number for Refinery attribute" << endl;
	return false;
 }
 mineId = root.attribute(QString::fromLatin1("ResourceMine")).toUInt(&ok);
 if (!ok) {
	boError() << k_funcinfo << "Invalid number for ResourceMine attribute" << endl;
	return false;
 }
 if (refineryId != 0) {
	// AB: retrieving from Boson is not 100% nice, but definitely necessary
	// and valid at this point. we need to get the pointer, even if the
	// refinery is an enemy or if it is invisible to us
	// --> it was saved this way, so we must load it this way.
	Unit* u = game()->findUnit(refineryId, 0);
	if (!u) {
		boError() << k_funcinfo << "cannot find refinery mine " << refineryId << endl;
	} else {
		mRefinery = (RefineryPlugin*)u->plugin(UnitPlugin::Refinery);
		if (!mRefinery) {
			boError() << k_funcinfo << "unit " << refineryId << " is not a refinery" << endl;
			mRefinery = 0;
		}
	}
 }
 if (mineId != 0) {
	Unit* u = game()->findUnit(mineId, 0);
	if (!u) {
		boError() << k_funcinfo << "cannot find resource mine " << mineId << endl;
	} else {
		mResourceMine = (ResourceMinePlugin*)u->plugin(UnitPlugin::ResourceMine);
		if (!mResourceMine) {
			boError() << k_funcinfo << "unit " << mineId << " is not a resource mine" << endl;
			mResourceMine = 0;
		}
	}
 }
 return true;
}

bool HarvesterPlugin::isAtResourceMine() const
{
 if (!mResourceMine) {
	return false;
 }
 if (!mResourceMine->unit()) {
	BO_NULL_ERROR(mResourceMine->unit());
	return false;
 }
 return isNextTo(mResourceMine->unit());
}

bool HarvesterPlugin::isAtRefinery() const
{
 if (!mRefinery) {
	return false;
 }
 if (!mRefinery->unit()) {
	BO_NULL_ERROR(mRefinery->unit());
	return false;
 }
 return isNextTo(mRefinery->unit());
}

bool HarvesterPlugin::isNextTo(const Unit* u) const
{
 // warning: we measure center of _this_ unit to center of the other unit only.
 // this will cause problems for units that occupy multiple cells (e.g.
 // refineries)
 if (!u) {
	return false;
 }
 if (!unit()) {
	BO_NULL_ERROR(unit());
	return false;
 }

 bofixed distx, disty;
 distx = (int)(u->centerX() - unit()->centerX());
 disty = (int)(u->centerY() - unit()->centerY());
 distx = QABS(distx);
 disty = QABS(disty);
 // We might get some precision trouble with floats, so we do this:
 distx = QMAX(distx - 0.1, bofixed(0));
 disty = QMAX(disty - 0.1, bofixed(0));

 bofixed allowedx, allowedy;
 allowedx = ceilf(unit()->width()) / 2 + ceilf(u->width()) / 2;
 allowedy = ceilf(unit()->height()) / 2 + ceilf(u->height()) / 2;

 if (distx <= allowedx && disty <= allowedy) {
	return true;
 }
 return false;
}

void HarvesterPlugin::advanceMine()
{
 boDebug(430) << k_funcinfo << endl;
 const HarvesterProperties* prop = (HarvesterProperties*)properties(PluginProperties::Harvester);
 if (!prop) {
	boError(430) << k_funcinfo << "NULL harvester properties" << endl;
	unit()->setWork(Unit::WorkIdle);
	return;
 }
 if (!mResourceMine || !mResourceMine->isUsableTo(this)) {
	ResourceMinePlugin* mine = findClosestResourceMine();
	if (!mine || !mine->unit()) {
		boDebug(430) << k_funcinfo << "no resource mine found" << endl;
		mHarvestingType = 0; // stop
	} else {
		boDebug(430) << k_funcinfo << "resource mine: " << mine->unit()->id() << endl;
		mineAt(mine);
	}
	return;
 }
 if (!mResourceMine || !mResourceMine->isUsableTo(this)) {
	QString mineId = "no id";
	if (mResourceMine && mResourceMine->unit()) {
		mineId = QString::number(mResourceMine->unit()->id());
	}
	boDebug(430) << k_funcinfo << "cannot mine at " << mResourceMine << " (" << mineId << ")" << endl;
	unit()->setWork(Unit::WorkIdle);

	// TODO: handle special case when mine has become empty!
	// -> we should go to a refinery now
	return;
 }

 BO_CHECK_NULL_RET(mResourceMine);
 BO_CHECK_NULL_RET(mResourceMine->unit());
 if (mResourceMine->unit()->isDestroyed()) {
	boDebug(430) << k_funcinfo << "resource mine has been destroyed!" << endl;
	mResourceMine = 0; // search a new one
	return;
 }

 // Check if unit is at mining location. If not, go there
 if (!isAtResourceMine()) {
	Unit* u = mResourceMine->unit();
	if (!unit()->moveTo(u, 0)) {
		boDebug(430) << k_funcinfo << "Cannot move to refinery (id=" << u->id() <<
				") at (" << u->x() << "; " << u->y() << ")" << endl;
		unit()->setWork(Unit::WorkIdle);
		return;
	}
	unit()->setAdvanceWork(Unit::WorkMove);
	return;
 }
 if (resourcesMined() >= prop->maxResources()) {
	// Back to refinery
	boDebug(430) << k_funcinfo << "Maximal amount of resources mined." << endl;
	mHarvestingType = 2; // refining
	return;
 }
 unsigned int mined = 0;
 if (canMineMinerals() && mResourceMine->canProvideMinerals()) {
	mined = mResourceMine->mineMinerals(this);
	player()->statistics()->increaseMinedMinerals(mined);
 } else if (canMineOil() && mResourceMine->canProvideOil()) {
	mined = mResourceMine->mineOil(this);
	player()->statistics()->increaseMinedOil(mined);
 } else {
	boError(430) << k_funcinfo << "oops - cannot mine here?!" << endl;
	mined = 0;
 }
 if (resourcesMined() + mined > prop->maxResources()) {
	// any additional resources are lost now, not even another harvester can
	// get them (intended).
	// cannot happen anyway.
	mined = prop->maxResources() - resourcesMined();
 }
 mResourcesMined = resourcesMined() + mined;
 boDebug(430) << k_funcinfo << "resources mined: " << resourcesMined() << endl;
}

void HarvesterPlugin::advanceRefine()
{
 // This is the second step of harvesting: returning to refinery and unloading
 boDebug(430) << k_funcinfo << endl;
 if (resourcesMined() == 0) {
	boDebug(430) << k_funcinfo << "refining done" << endl;
	mHarvestingType = 1; // mining
	return;
 }
 if (!mRefinery) {
	RefineryPlugin* refinery = findClosestRefinery();
	if (!refinery || !refinery->unit()) {
		boDebug(430) << k_funcinfo << "no refinery found" << endl;
		mHarvestingType = 0; // stop
	} else {
		boDebug(430) << k_funcinfo << "refinery: " << refinery->unit()->id() << endl;
		refineAt(refinery);
	}
	return;
 }

 BO_CHECK_NULL_RET(mRefinery);
 BO_CHECK_NULL_RET(mRefinery->unit());
 if (mRefinery->unit()->isDestroyed()) {
	boDebug(430) << k_funcinfo << "refinery has been destroyed!" << endl;
	mRefinery = 0; // search a new one
	return;
 }

 if (!isAtRefinery()) {
	Unit* u = mRefinery->unit();
	if (!unit()->moveTo(u, 0)) {
		boDebug(430) << k_funcinfo << "Cannot move to refinery (id=" << u->id() <<
				") at (" << u->x() << "; " << u->y() << ")" << endl;
		unit()->setWork(Unit::WorkIdle);
		return;
	}
	unit()->setAdvanceWork(Unit::WorkMove);
	return;
 }
 const HarvesterProperties* prop = (HarvesterProperties*)unit()->properties(PluginProperties::Harvester);
 if (!prop) {
	boError(430) << k_funcinfo << "NULL harvester plugin" << endl;
	mHarvestingType = 0; // stop
	return;
 }

 int amount = unloadingSpeed();
 if (amount > (int)resourcesMined()) {
	amount = resourcesMined();
 }
 if (amount < 0) {
	boError(430) << k_funcinfo << "a negative amount of resources to be refined??" << endl;
	amount = 0;
 }
 unsigned int refined = 0;
 if (canMineMinerals()) {
	refined = mRefinery->refineMinerals(amount);
	player()->statistics()->increaseRefinedMinerals(refined);
 } else if (canMineOil()) {
	refined = mRefinery->refineOil(amount);
	player()->statistics()->increaseRefinedOil(refined);
 }
 if (mResourcesMined < refined) {
	boError(430) << k_funcinfo << "oops - more processed than available!" << endl;
	refined = mResourcesMined;
 }
 mResourcesMined = mResourcesMined - refined;
 boDebug(430) << k_funcinfo << "resources left: " << resourcesMined() << endl;
}

ResourceMinePlugin* HarvesterPlugin::findClosestResourceMine() const
{
 BO_CHECK_NULL_RET0(game());
 BO_CHECK_NULL_RET0(game()->playerList());
 BO_CHECK_NULL_RET0(player());
 BO_CHECK_NULL_RET0(player()->playerIO());

 // FIXME: we should use player()->playerIO()->getUnitsOf(playerCount() - 1)
 // instead
 // --> that should give us all units that are visible to us only
 Player* neutral = (Player*)game()->playerList()->getLast();

 BO_CHECK_NULL_RET0(neutral);
 QPtrListIterator<Unit> it(*(neutral->allUnits()));
 ResourceMinePlugin* mine = 0;
 bofixed mineDist = 0.0f;
 while (it.current()) {
	Unit* u = it.current();
	if (!player()->playerIO()->canSee(u)) {
		++it;
		continue;
	}
	ResourceMinePlugin* m = (ResourceMinePlugin*)u->plugin(UnitPlugin::ResourceMine);
	if (!m) {
		++it;
		continue;
	}
	if (m->isUsableTo(this)) {
		bofixed dist = QMAX(QABS(unit()->x() - u->x()), QABS(unit()->y() - u->y()));
		if ((dist < mineDist) || (mineDist == 0.0f)) {
			mineDist = dist;
			mine = m;
		}
	}
	++it;
 }
 return mine;
}

RefineryPlugin* HarvesterPlugin::findClosestRefinery() const
{
 BO_CHECK_NULL_RET0(player());
 QPtrListIterator<Unit> it(*(player()->allUnits()));
 const HarvesterProperties* prop = (HarvesterProperties*)properties(PluginProperties::Harvester);
 if (!prop) {
	boError() << k_funcinfo << "NULL harvester plugin" << endl;
	return 0;
 }
 RefineryPlugin* ref = 0;
 bofixed refdist = 0.0f;
 while (it.current()) {
	RefineryPlugin* r = (RefineryPlugin*)it.current()->plugin(UnitPlugin::Refinery);
	if (!r) {
		++it;
		continue;
	}
	if (r->isUsableTo(this)) {
		bofixed dist = QMAX(QABS(unit()->x() - it.current()->x()), QABS(unit()->y() - it.current()->y()));
		if ((dist < refdist) || (refdist == 0.0f)) {
			refdist = dist;
			ref = r;
		}
	}
	++it;
 }
 return ref;
}

void HarvesterPlugin::mineAt(ResourceMinePlugin* resource)
{
 //TODO: don't move if unit cannot mine more minerals/oil or no minerals/oil at all
 BO_CHECK_NULL_RET(resource);
 BO_CHECK_NULL_RET(resource->unit());
 BO_CHECK_NULL_RET(unit());
 boDebug() << k_funcinfo << resource->unit()->id() << endl;
 if (resource->unit()->isDestroyed()) {
	boDebug() << k_funcinfo << "sorry, resource mine is already destroyed. cannot use it" << endl;
	return;
 }
 if (!resource->isUsableTo(this)) {
	boError() << k_funcinfo << resource->unit()->id() << " not a suitable resource mine" << endl;
	return;
 }
 if (!unit()->moveTo(resource->unit(), 0)) {
	boDebug() << k_funcinfo << "cannot find a way to resource mine" << endl;
	boDebug() << k_funcinfo << "TODO: search another resource mine" << endl;
	return;
 }
 unit()->setPluginWork(UnitPlugin::Harvester);
 unit()->setAdvanceWork(Unit::WorkMove);
 mResourceMine = resource;

 mHarvestingType = 1;
}


void HarvesterPlugin::refineAt(RefineryPlugin* refinery)
{
 BO_CHECK_NULL_RET(refinery);
 BO_CHECK_NULL_RET(refinery->unit());
 BO_CHECK_NULL_RET(unit());
 boDebug() << k_funcinfo << refinery->unit()->id() << endl;
 if (!refinery->isUsableTo(this)) {
	boError() << k_funcinfo << refinery->unit()->id() << " not a suitable refinery" << endl;
	return;
 }
 if (refinery->unit()->isDestroyed()) {
	boDebug() << k_funcinfo << "sorry, refinery is already destroyed. cannot use it" << endl;
	return;
 }
 if (!unit()->moveTo(refinery->unit(), 0)) {
	boDebug() << k_funcinfo << "cannot find a way to refinery" << endl;
	boDebug() << k_funcinfo << "TODO: search another refinery" << endl;
	return;
 }
 unit()->setPluginWork(UnitPlugin::Harvester);
 unit()->setAdvanceWork(Unit::WorkMove);
 mRefinery = refinery;

 mHarvestingType = 2; // refining
}


bool HarvesterPlugin::canMineMinerals() const
{
 const HarvesterProperties* prop = (HarvesterProperties*)unit()->properties(PluginProperties::Harvester);
 if (!prop) {
	BO_NULL_ERROR(prop);
	return false;
 }
 return prop->canMineMinerals();
}

bool HarvesterPlugin::canMineOil() const
{
 const HarvesterProperties* prop = (HarvesterProperties*)unit()->properties(PluginProperties::Harvester);
 if (!prop) {
	BO_NULL_ERROR(prop);
	return false;
 }
 return prop->canMineOil();
}

unsigned int HarvesterPlugin::maxResources() const
{
 const HarvesterProperties* prop = (HarvesterProperties*)unit()->properties(PluginProperties::Harvester);
 if (!prop) {
	BO_NULL_ERROR(prop);
	return 0;
 }
 return prop->maxResources();
}

unsigned int HarvesterPlugin::miningSpeed() const
{
 const HarvesterProperties* prop = (HarvesterProperties*)unit()->properties(PluginProperties::Harvester);
 if (!prop) {
	BO_NULL_ERROR(prop);
	return 0;
 }
 return prop->miningSpeed();
}

unsigned int HarvesterPlugin::unloadingSpeed() const
{
 const HarvesterProperties* prop = (HarvesterProperties*)unit()->properties(PluginProperties::Harvester);
 if (!prop) {
	BO_NULL_ERROR(prop);
	return 0;
 }
 return prop->unloadingSpeed();
}

void HarvesterPlugin::unitDestroyed(Unit* u)
{
 RefineryPlugin* r = (RefineryPlugin*)u->plugin(UnitPlugin::Refinery);
 if (mRefinery == r) {
	mRefinery = 0;
 }
 ResourceMinePlugin* m = (ResourceMinePlugin*)u->plugin(UnitPlugin::ResourceMine);
 if (mResourceMine == m) {
	mResourceMine = 0;
 }
}

void HarvesterPlugin::itemRemoved(BosonItem* item)
{
 if (!item) {
	return;
 }
 if (!RTTI::isUnit(item->rtti())) {
	return;
 }
 Unit* u = (Unit*)item;
 unitDestroyed(u);
}

BombingPlugin::BombingPlugin(Unit* owner) : UnitPlugin(owner)
{
 owner->registerData(&mTargetX, Unit::IdBombingTargetX);
 owner->registerData(&mTargetY, Unit::IdBombingTargetY);
 owner->registerData(&mDropDist, Unit::IdBombingDropDist);
 owner->registerData(&mLastDistFromDropPoint, Unit::IdBombingLastDistFromDropPoint);
 mWeapon = 0;
 mTargetX = 0;
 mTargetY = 0;
 mDropDist = 0;
 mLastDistFromDropPoint = 0;
}

BombingPlugin::~BombingPlugin()
{
}

void BombingPlugin::bomb(int weaponId, BoVector2Fixed pos)
{
 boDebug() << k_funcinfo << "wep: " << weaponId << "; pos: (" << pos.x() << "; " << pos.y() << ")" << endl;
 BosonWeapon* w = unit()->weapon(weaponId);
 if (!w) {
	boError() << k_funcinfo << "No weapon with id " << weaponId << endl;
	return;
 }
 if (w->properties()->shotType() != BosonShot::Bomb) {
	boError() << k_funcinfo << "Weapon with id " << weaponId << " is not a bomb" << endl;
	return;
 }

 if (!unit()->isFlying()) {
	boWarning() << k_funcinfo << "Only flying units can drop bombs" << endl;
	return;
 }

 int cellX = (int)pos.x();
 int cellY = (int)pos.y();
 if (!canvas()->cell(cellX, cellY)) {
	boError() << k_funcinfo << cellX << "," << cellY << " is no valid cell!" << endl;
	return;
 }

 // This is where unit has to be when making the drop
 mTargetX = pos.x();
 mTargetY = pos.y();

 if (!unit()->moveTo(mTargetX, mTargetY, 0)) {
	boError() << k_funcinfo << "Moving failed. Now what?" << endl;
	unit()->setWork(Unit::WorkIdle);
	} else {
	unit()->pathInfo()->slowDownAtDest = false;
	unit()->pathInfo()->moveAttacking = false;
	// Instead of setting unit's advanceWork to WorkMove, we call
	//  unit()->advanceMove() ourselves to retain finer control
 }

 // Calculate drop distance (how far from the target we drop the bomb)
 bofixed height = unit()->unitProperties()->preferredAltitude();  // How high from the ground we release the bomb
 // t = sqrt(2s / a)
 bofixed droptime = sqrt(2 * height / w->properties()->accelerationSpeed());
 mDropDist = droptime * unit()->maxSpeed();  // In advance calls
 boDebug() << k_funcinfo << "Target point: (" << mTargetX << "; " << mTargetY << "); dropdist: " << mDropDist << endl;
 mWeapon = w;

 unit()->setPluginWork(UnitPlugin::Bombing);
}

void BombingPlugin::advance(unsigned int advanceCalls)
{
 // Check if we're at the drop point
 // Unit's center point
 BoVector2Fixed totarget(mTargetX - unit()->centerX(), mTargetY - unit()->centerY());
 bofixed dist = totarget.length();
 bofixed distfromdroppoint = QABS(dist - mDropDist);

 if ((distfromdroppoint < 3) && (distfromdroppoint > mLastDistFromDropPoint)) {
	// We're at drop point. Drop the bomb
	if (!mWeapon->reloaded()) {
		//boError() << k_funcinfo << "Weapon not reloaded!" << endl;
		unit()->flyInCircle();
		return;
	}

	boDebug() << k_funcinfo << "Bomb ready. Dropping..." << endl;
	bofixed hvelox, hveloy;
	Bo3dTools::pointByRotation(&hvelox, &hveloy, unit()->rotation(), unit()->speed());
	mWeapon->dropBomb(BoVector2Fixed(hvelox, hveloy));

	// And get the hell out of there
	// Go away from bomb's explosion radius
	bofixed dist = mWeapon->damageRange() + unit()->width() / 2;
	boDebug() << k_funcinfo << "Getaway dist: " << dist << "; rot: " << unit()->rotation() << endl;
	bofixed newx, newy;
	Bo3dTools::pointByRotation(&newx, &newy, unit()->rotation(), dist);
	newx = unit()->centerX() - newx;
	newy = unit()->centerY() - newy;

	// Make sure coords are valid
	// TODO: if current getaway point is off the canvas (or too close to the
	//  edge), rotate a bit and select new getaway point.
	newx = QMAX(unit()->width() / 2, QMIN(newx, (canvas()->mapWidth() - 1) - unit()->width() / 2));
	newy = QMAX(unit()->height() / 2, QMIN(newy, (canvas()->mapHeight() - 1) - unit()->height() / 2));

  boDebug() << k_funcinfo << "Getaway point is at (" << newx << "; " << newy << ")" << endl;
	if (!unit()->moveTo(newx, newy)) {
		boWarning() << k_funcinfo << "Aargh! Can't move away from drop-point!" << endl;
		unit()->setWork(Unit::WorkIdle);
	} else {
		unit()->pathInfo()->moveAttacking = false;
		unit()->setWork(Unit::WorkMove);  // We don't want to return here anymore
	}

	mWeapon = 0;
	boDebug() << k_funcinfo << "returning" << endl;

 } else {
	// Continue moving towards the target
	mLastDistFromDropPoint = distfromdroppoint;
	unit()->advanceMove(advanceCalls);
	return;
 }
}

bool BombingPlugin::saveAsXML(QDomElement& root) const
{
 unsigned int weaponId = 0;
 // TODO
 root.setAttribute(QString::fromLatin1("Weapon"), weaponId);
 return true;
}

bool BombingPlugin::loadFromXML(const QDomElement& root)
{
 unsigned int weaponId = 0;
 bool ok = false;
 weaponId = root.attribute(QString::fromLatin1("Weapon")).toUInt(&ok);
 if (!ok) {
	boError() << k_funcinfo << "Invalid number for Weapon attribute" << endl;
	return false;
 }
 // TODO
 // mWeapon = ...;
 return true;
}



MiningPlugin::MiningPlugin(Unit* owner) : UnitPlugin(owner)
{
 mWeapon = 0;
 owner->registerData(&mPlacingCounter, Unit::IdMinePlacingCounter);
 mPlacingCounter = 0;
}

MiningPlugin::~MiningPlugin()
{
}

// How many game ticks does it take to place mine
// FIXME: make configurable in BosonWeapon
#define MINE_PLACE_TIME 10

void MiningPlugin::mine(int weaponId)
{
 boDebug() << k_funcinfo << "wep: " << weaponId << endl;
 BosonWeapon* w = unit()->weapon(weaponId);
 if (!w) {
	boError() << k_funcinfo << "No weapon with id " << weaponId << endl;
	return;
 }
 if (w->properties()->shotType() != BosonShot::Mine) {
	boError() << k_funcinfo << "Weapon with id " << weaponId << " is not a mine" << endl;
	return;
 }

 unit()->stopMoving();

 mWeapon = w;
 mPlacingCounter = MINE_PLACE_TIME;

 // Mine will be layed in advance()

 unit()->setPluginWork(UnitPlugin::Mining);
}

void MiningPlugin::advance(unsigned int)
{
 // Lay the mine
 if (mWeapon->reloaded()) {
	// Don't place the mine immediately
	if (mPlacingCounter > 0) {
//		boDebug() << k_funcinfo << "mPlacingCounter: " << mPlacingCounter << endl;
		mPlacingCounter = mPlacingCounter - 1;
		return;
	}

	boDebug() << k_funcinfo << "Mine ready. Placing..." << endl;
	mWeapon->layMine();

	// Go one cell away from the mine. Maybe go away from explosion radius?
	// FIXME: code taken from BombingPlugin. This could probably be written better
	bofixed dist = 1.0f + unit()->width() / 2.0f;
	boDebug() << k_funcinfo << "Getaway dist: " << dist << "; rot: " << unit()->rotation() << endl;
	bofixed oldx = unit()->x();
	bofixed oldy = unit()->y();
	bofixed newx, newy;
	bool couldmove = false;

	// TODO: quite messy code, maybe it can be cleaned up somehow
	for (int i = 0; i <= 7; i++) {
		newx = oldx;
		newy = oldy;

		// First try to go straight ahead, then try go to 45 degrees right, then
		//  45 degrees left, then 2 * 45 degrees right, then 2 * 45 degrees left, etc
		int rotadd = ((i + 1) / 2) * 45;
		if (i % 2 == 0) {
			rotadd = -rotadd;
		}
		int rot = ((int)unit()->rotation() + rotadd) % 360;

		boDebug() << k_funcinfo << "i: " << i << "; rotadd: " << rotadd << "; rot: " << rot << endl;

		if (rot >= 45 && rot <= 135) {
			newx += dist;
		} else if (rot >= 225 && rot <= 315) {
			newx -= dist;
		}
		if (rot <= 45 || rot >= 315) {
			newy -= dist;
		} else if (rot >= 135 && rot <= 225) {
			newy += dist;
		}

		// Make sure coords are valid
		newx = QMAX(bofixed(0), QMIN(newx, bofixed((canvas()->mapWidth() - 1))));
		newy = QMAX(bofixed(0), QMIN(newy, bofixed((canvas()->mapHeight() - 1))));

		boDebug() << k_funcinfo << "i: " << i << "; Getaway point is at (" << newx << "; " << newy << ")" << endl;
		if (unit()->moveTo(newx, newy)) {
			unit()->addWaypoint(BoVector2Fixed(newx, newy));
			unit()->setWork(Unit::WorkMove);  // We don't want to return here anymore
			couldmove = true;
			break;
		}
	}

	if (!couldmove) {
		boDebug() << k_funcinfo << "Can't move away!" << endl;
		unit()->setWork(Unit::WorkIdle);  // We don't want to return here anymore
	}

	mWeapon = 0;
	boDebug() << k_funcinfo << "returning" << endl;
	return;
 } else {
//	boDebug() << k_funcinfo << "Weapon not yet reloaded" << endl;
 }
}

bool MiningPlugin::saveAsXML(QDomElement& root) const
{
 unsigned int weaponId = 0;
 // TODO
 root.setAttribute(QString::fromLatin1("Weapon"), weaponId);
 return true;
}

bool MiningPlugin::loadFromXML(const QDomElement& root)
{
 unsigned int weaponId = 0;
 bool ok = false;
 weaponId = root.attribute(QString::fromLatin1("Weapon")).toUInt(&ok);
 if (!ok) {
	boError() << k_funcinfo << "Invalid number for Weapon attribute" << endl;
	return false;
 }
 // TODO
 // mWeapon = ...;
 return true;
}

ResourceMinePlugin::ResourceMinePlugin(Unit* unit)
		: UnitPlugin(unit)
{
 unit->registerData(&mMinerals, Unit::IdResourceMineMinerals);
 unit->registerData(&mOil, Unit::IdResourceMineOil);
 mMinerals.setLocal(0);
 mOil.setLocal(0);
}

ResourceMinePlugin::~ResourceMinePlugin()
{
}

bool ResourceMinePlugin::saveAsXML(QDomElement& root) const
{
 // everything is saved using KGameProperty
 Q_UNUSED(root);
 return true;
}

bool ResourceMinePlugin::loadFromXML(const QDomElement& root)
{
 // everything is already loaded using KGameProperty - we do safety checks here
 ResourceMineProperties* prop = (ResourceMineProperties*)properties(PluginProperties::ResourceMine);
 if (!prop) {
	BO_NULL_ERROR(prop);
	return false;
 }
 if (!prop->canProvideMinerals()) {
	if (mMinerals != 0) {
		boWarning() << k_funcinfo << "unit can't have minerals, but minerals not 0" << endl;
	}
	mMinerals = 0;
 }
 if (!prop->canProvideOil()) {
	if (mOil != 0) {
		boWarning() << k_funcinfo << "unit can't have oil, but oil not 0" << endl;
	}
	mOil = 0;
 }
 return true;
}

void ResourceMinePlugin::advance(unsigned int)
{
}

bool ResourceMinePlugin::canProvideMinerals() const
{
 const ResourceMineProperties* prop = (ResourceMineProperties*)unit()->properties(PluginProperties::ResourceMine);
 if (!prop) {
	return false;
 }
 return prop->canProvideMinerals();
}

bool ResourceMinePlugin::canProvideOil() const
{
 const ResourceMineProperties* prop = (ResourceMineProperties*)unit()->properties(PluginProperties::ResourceMine);
 if (!prop) {
	return false;
 }
 return prop->canProvideOil();
}

bool ResourceMinePlugin::isUsableTo(const HarvesterPlugin* harvester) const
{
 if (!harvester) {
	return false;
 }
 if (harvester->canMineMinerals() && canProvideMinerals() && minerals() != 0) {
	return true;
 }
 if (harvester->canMineOil() && canProvideOil() && oil() != 0) {
	return true;
 }
 return false;
}

unsigned int ResourceMinePlugin::mineMinerals(const HarvesterPlugin* harvester)
{
 if (!harvester) {
	BO_NULL_ERROR(harvester);
	return 0;
 }
 if (!canProvideMinerals()) {
	boDebug() << k_funcinfo << "no minerals available. sorry." << endl;
	return 0;
 }
 if (!isUsableTo(harvester)) {
	boDebug() << k_funcinfo << "harvester cannot mine here" << endl;
	return 0;
 }

 int amount = mineStep(harvester, minerals());
 // When minerals() == -1, then mine's mineral supply is infinite, so we won't change it.
 if (minerals() >= 0) {
	setMinerals(minerals() - amount);
 }
 return amount;
}

unsigned int ResourceMinePlugin::mineOil(const HarvesterPlugin* harvester)
{
 if (!harvester) {
	BO_NULL_ERROR(harvester);
	return 0;
 }
 if (!canProvideOil()) {
	boDebug() << k_funcinfo << "no oil available. sorry." << endl;
	return 0;
 }
 if (!isUsableTo(harvester)) {
	boDebug() << k_funcinfo << "harvester cannot mine here" << endl;
	return 0;
 }

 int amount = mineStep(harvester, oil());
 // When oil() == -1, then mine's oil supply is infinite, so we won't change it.
 if (oil() >= 0) {
	setOil(oil() - amount);
 }
 return amount;
}

unsigned int ResourceMinePlugin::mineStep(const HarvesterPlugin* harvester, int resourcesAvailable) const
{
 if (!harvester) {
	BO_NULL_ERROR(harvester);
	return 0;
 }
 int maxCapacity = QMAX((int)harvester->maxResources() - (int)harvester->resourcesMined(), 0);
 int maxAvailable = resourcesAvailable;
 if (resourcesAvailable < 0) {
	// infinite resources
	maxAvailable = maxCapacity;
 }
 int step = harvester->miningSpeed();
 if (step > maxAvailable) {
	step = maxAvailable;
 }
 if (step > maxCapacity) {
	step = maxCapacity;
 }
 return step;
}

void ResourceMinePlugin::setMinerals(int m)
{
 if (!canProvideMinerals()) {
	mMinerals = 0;
	return;
 }
 mMinerals = m;
}

void ResourceMinePlugin::setOil(int o)
{
 if (!canProvideOil()) {
	mOil = 0;
	return;
 }
 mOil = o;
}

int ResourceMinePlugin::minerals() const
{
 return mMinerals;
}

int ResourceMinePlugin::oil() const
{
 return mOil;
}

void ResourceMinePlugin::unitDestroyed(Unit*)
{
}

void ResourceMinePlugin::itemRemoved(BosonItem*)
{
}

RefineryPlugin::RefineryPlugin(Unit* owner)
		: UnitPlugin(owner)
{
}
RefineryPlugin::~RefineryPlugin()
{
}

bool RefineryPlugin::saveAsXML(QDomElement& root) const
{
 Q_UNUSED(root);
 return true;
}

bool RefineryPlugin::loadFromXML(const QDomElement& root)
{
 Q_UNUSED(root);
 return true;
}

void RefineryPlugin::advance(unsigned int)
{
}

bool RefineryPlugin::canRefineMinerals() const
{
 const RefineryProperties * prop = (RefineryProperties*)unit()->properties(PluginProperties::Refinery);
 if (!prop) {
	return false;
 }
 return prop->canRefineMinerals();
}

bool RefineryPlugin::canRefineOil() const
{
 const RefineryProperties * prop = (RefineryProperties*)unit()->properties(PluginProperties::Refinery);
 if (!prop) {
	return false;
 }
 return prop->canRefineOil();
}

void RefineryPlugin::unitDestroyed(Unit*)
{
}

void RefineryPlugin::itemRemoved(BosonItem*)
{
}

bool RefineryPlugin::isUsableTo(const HarvesterPlugin* harvester) const
{
 if (!harvester) {
	return false;
 }
 if (canRefineMinerals() && harvester->canMineMinerals()) {
	return true;
 }
 if (canRefineOil() && harvester->canMineOil()) {
	return true;
 }
 return false;
}

unsigned int RefineryPlugin::refineMinerals(unsigned int minerals)
{
 if (!player()) {
	BO_NULL_ERROR(player());
	return 0;
 }
 player()->setMinerals(player()->minerals() + minerals);
 return minerals;
}

unsigned int RefineryPlugin::refineOil(unsigned int oil)
{
 if (!player()) {
	BO_NULL_ERROR(player());
	return 0;
 }
 player()->setOil(player()->oil() + oil);
 return oil;
}

