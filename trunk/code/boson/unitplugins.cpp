/*
    This file is part of the Boson game
    Copyright (C) 2002-2004 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "defines.h"
#include "unit.h"
#include "unitproperties.h"
#include "pluginproperties.h"
#include "speciestheme.h"
#include "player.h"
#include "bosoncanvas.h"
#include "boson.h"
#include "boitemlist.h"
#include "bosonstatistics.h"
#include "cell.h"
#include "upgradeproperties.h"
#include "bodebug.h"
#include "bosonweapon.h"
#include "bosonpath.h"

#include <klocale.h>

#include <qpair.h>
#include <qpoint.h>
#include <qdom.h>
#include <qstringlist.h>

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

void ProductionPlugin::addProduction(ProductionType type, unsigned long int id)
{
 ProductionProperties* p = (ProductionProperties*)unit()->unitProperties()->properties(PluginProperties::Production);
 if (!p) {
	boError() << k_funcinfo << "NULL production properties" << endl;
	return;
 }
 if(type == ProduceUnit) {
	if (!speciesTheme()->productions(p->producerList()).contains(id)) {
		boError() << k_funcinfo << " cannot produce unit with id " << id << endl;
		return;
	}
 } else if(type == ProduceTech) {
	if (!speciesTheme()->technologies(p->producerList()).contains(id)) {
		boError() << k_funcinfo << " cannot produce technology with id " << id << endl;
		return;
	}
 }
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
}

void ProductionPlugin::removeProduction()
{
 mProductions.pop_front();
 mProductionState = 0; // start next production (if any)
}

void ProductionPlugin::removeProduction(ProductionType type, unsigned long int id)
{
 for (unsigned int i = 0; i < productionList().count(); i++) {
	if ((mProductions[i].first == type) && (mProductions[i].second == id)) {
		boDebug() << k_funcinfo << "remove; type: " << type << ", id: " << id << endl;
		mProductions.remove(mProductions.at(i));
		return;
	}
 }
}

double ProductionPlugin::productionProgress() const
{
 unsigned int productionTime = 0;
 if(currentProductionType() == ProduceUnit) {
	productionTime = speciesTheme()->unitProperties(currentProductionId())->productionTime();
 } else if(currentProductionType() == ProduceTech) {
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
	unit()->setWork(Unit::WorkNone);
	mProductionState = 0;
	return;
 }



 // FIXME: this code is broken!
 // it gets executed on *every* client but sendInput() should be used on *one*
 // client only!
 // a unit is completed as soon as mProductionState == player()->unitProperties(type)->productionTime()
 unsigned int productionTime = 0;
 if(currentProductionType() == ProduceUnit) {
	productionTime = speciesTheme()->unitProperties(currentProductionId())->productionTime();
 } else if(currentProductionType() == ProduceTech) {
	productionTime = speciesTheme()->technology(currentProductionId())->productionTime();
 } else {
	boDebug() << k_funcinfo << "Unknown productiontype: " << currentProductionType() << endl;
 }
 if (mProductionState <= productionTime) {
	if (mProductionState == productionTime) {
		boDebug() << "Production with type " << currentProductionType() << " and id " << id << " completed :-)" << endl;
		mProductionState = mProductionState + 1;

		if(currentProductionType() != ProduceUnit) {
			// It's technology
			removeProduction();
			player()->technologyResearched(this, id);
			return;
		}

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
		if (speciesTheme()->unitProperties(id)->isFacility()) {
			game()->slotAddChatSystemMessage(
					i18n("A %1 has been produced - place it on the map to start construction!").arg(prop->name()),
					player());
			return;
		} else {
			game()->slotAddChatSystemMessage(
					i18n("A %1 has been produced and will be placed on the map now").arg(prop->name()),
					player());
		}
		int tilex, tiley; // Position of lower-left corner of facility in tiles
		int theight, twidth; // height and width of facility in tiles
		int currentx, currenty; // Position of tile currently tested
		theight = unit()->height() / BO_TILE_SIZE;
		twidth = unit()-> width() / BO_TILE_SIZE;
		tilex = (int)(unit()->x() / BO_TILE_SIZE);
		tiley = (int)(unit()->y() / BO_TILE_SIZE + theight);
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

				if (canvas()->canPlaceUnitAtCell(speciesTheme()->unitProperties(id), QPoint(currentx, currenty), this)) {
					// Free cell - place unit at it
					mProductionState = mProductionState + 1;
					//FIXME: buildProduction should not
					//depend on Facility! should be Unit
					((Boson*)player()->game())->buildProducedUnit(this, id, currentx, currenty);
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
	if (!u->moveTo(unit()->x(), unit()->y(), 1)) {
		boDebug() << k_funcinfo << u->id() << " cannot find a way to repairyard" << endl;
		u->setWork(Unit::WorkNone);
	} else {
		u->setWork(Unit::WorkMove);
	}
 } else {
	if (!unit()->moveTo(u->x(), u->y(), 1)) {
		boDebug() << k_funcinfo << "Cannot find way to " << u->id() << endl;
		unit()->setWork(Unit::WorkNone);
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
 mResourcesX.setLocal(0);
 mResourcesY.setLocal(0);
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
	unit()->setWork(Unit::WorkNone);
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
	refineryId = mRefinery->id();
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
 if (refineryId != 0) {
	// AB: retrieving from Boson is not 100% nice, but definitely necessary
	// and valid at this point. we need to get the pointer, even if the
	// refinery is an enemy or if it is invisible to us
	// --> it was saved this way, so we must load it this way.
	mRefinery = game()->findUnit(refineryId, 0);
 }
 if (mineId != 0) {
	Unit* u = game()->findUnit(mineId, 0);
	if (!u) {
		boError() << k_funcinfo << "cannot find resource mine " << mineId << endl;
	} else {
		mResourceMine = (ResourceMinePlugin*)u->plugin(UnitPlugin::ResourceMine);
		if (!mResourceMine) {
			boError() << k_funcinfo << "unit " << mineId << " is not a resource mine" << endl;
		}
	}
 }
 return true;
}

bool HarvesterPlugin::canMine(Unit* unit) const
{
 if (!unit) {
	return false;
 }
 return canMine((ResourceMinePlugin*)unit->plugin(UnitPlugin::ResourceMine));
}
bool HarvesterPlugin::canMine(ResourceMinePlugin* p) const
{
 if (!p) {
	return false;
 }
 const HarvesterProperties* prop = (HarvesterProperties*)properties(PluginProperties::Harvester);
 if (!prop) {
	boError() << k_funcinfo << "NULL harvester properties" << endl;
	return false;
 }
 if (prop->canMineMinerals() && p->canProvideMinerals() && p->minerals() != 0) {
	return true;
 }
 if (prop->canMineOil() && p->canProvideOil() && p->oil() != 0) {
	return true;
 }
 return false;
}

bool HarvesterPlugin::canMine(Cell* cell) const
{
 return false;
}


void HarvesterPlugin::advanceMine()
{
 const HarvesterProperties* prop = (HarvesterProperties*)properties(PluginProperties::Harvester);
 if (!prop) {
	boError() << k_funcinfo << "NULL harvester properties" << endl;
	unit()->setWork(Unit::WorkNone);
	return;
 }
 if (!canMine(mResourceMine)) {
	// TODO: search a new resource mine
	boDebug() << k_funcinfo << "cannot mine there" << endl;
	unit()->setWork(Unit::WorkNone);
	return;
 }

 // Check if unit is at mining location. If not, go there
 if ((mResourcesX / BO_TILE_SIZE != unit()->x() / BO_TILE_SIZE) || (mResourcesY / BO_TILE_SIZE != unit()->y() / BO_TILE_SIZE)) {
	unit()->moveTo(mResourcesX, mResourcesY);
	unit()->setAdvanceWork(Unit::WorkMove);
	return;
 }
 // Check if we should harvest more
 if (resourcesMined() < prop->maxResources()) {
	if (canMine(canvas()->cellAt(unit()))) {
		// How much more to harvest
		const int step = (resourcesMined() + miningSpeed() <= prop->maxResources()) ? miningSpeed() : prop->maxResources() - resourcesMined();
		mResourcesMined = resourcesMined() + step;
		if (prop->canMineMinerals()) {
			player()->statistics()->increaseMinedMinerals(step);
		} else if (prop->canMineOil()) {
			player()->statistics()->increaseMinedOil(step);
		}
		boDebug() << k_funcinfo << "resources mined: " << resourcesMined() << endl;
	} else {
		// This unit cannot mine at this cell
		boDebug() << k_funcinfo << "cannot mine here" << endl;
		unit()->setWork(Unit::WorkNone);
		unit()->setAdvanceWork(Unit::WorkNone);
		return;
	}
 } else {
	// Back to refinery
	boDebug() << k_funcinfo << "Maximal amount of resources mined." << endl;
	mHarvestingType = 2; // refining
 }
}

void HarvesterPlugin::advanceRefine()
{
 // This is the second step of harvesting: returning to refinery and unloading
 boDebug() << k_funcinfo << endl;
 if (resourcesMined() == 0) {
	boDebug() << k_funcinfo << "refining done" << endl;
	if (mResourceMine) {
		mineAt(mResourceMine);
	} else {
		unit()->setWork(Unit::WorkNone);
		unit()->setAdvanceWork(Unit::WorkNone);
	}
	return;
 }
 if (!refinery()) {
	// Refinery is not yet set. Find the closest one.
	QPtrListIterator<Unit> it(*(player()->allUnits()));
	const HarvesterProperties* prop = (HarvesterProperties*)unit()->properties(PluginProperties::Harvester);
	if (!prop) {
		boError() << k_funcinfo << "NULL harvester plugin" << endl;
		unit()->setWork(Unit::WorkNone);
		unit()->setAdvanceWork(Unit::WorkNone);
		return;
	}
	Unit* ref = 0;
	float refdist = 0;
	RefineryProperties* rprop;
	boDebug() << k_funcinfo << "going throught units list. count: " << it.count() << endl;
	while (it.current()) {
		boDebug() << k_funcinfo << "        testing unit " << it.current()->id() << endl;
		rprop = (RefineryProperties*)it.current()->properties(PluginProperties::Refinery);
		if (!rprop) {
			++it;
			continue;
		}
		boDebug() << k_funcinfo << "    unit " << it.current()->id() << " has refinery properties" << endl;
		if ((prop->canMineMinerals() && rprop->canRefineMinerals()) || (prop->canMineOil() && rprop->canRefineOil())) {
			boDebug() << k_funcinfo << "    unit " << it.current()->id() << " would be suitable..." << endl;
			float dist = QMAX(QABS(unit()->x() - it.current()->x()), QABS(unit()->y() - it.current()->y()));
			if((dist < refdist) || (refdist == 0)) {
				refdist = dist;
				ref = it.current();
			}
		}
		++it;
	}
	if (!ref) {
		boDebug() << k_funcinfo << "no suitable refinery found" << endl;
		unit()->setWork(Unit::WorkNone);
		unit()->setAdvanceWork(Unit::WorkNone);
	} else {
		boDebug() << k_funcinfo << "refinery: " << ref->id() << endl;
		refineAt(ref);
	}
	return;
 } else {
	// Refinery is set
	if (unit()->isNextTo(refinery())) {
		const int step = (resourcesMined() >= unloadingSpeed()) ? unloadingSpeed() : resourcesMined();
		mResourcesMined = resourcesMined() - step;
		const HarvesterProperties* prop = (HarvesterProperties*)unit()->properties(PluginProperties::Harvester);
		if (!prop) {
			boError() << k_funcinfo << "NULL harvester plugin" << endl;
			unit()->setWork(Unit::WorkNone);
			return;
		}
		if (prop->canMineMinerals()) {
			player()->setMinerals(player()->minerals() + step);
			player()->statistics()->increaseRefinedMinerals(step);
		} else if (prop->canMineOil()) {
			player()->setOil(player()->oil() + step);
			player()->statistics()->increaseRefinedOil(step);
		}
	} else {
		// Move next to refinery
		if (!unit()->moveTo(refinery()->x(), refinery()->y(), 1)) {
			boDebug() << k_funcinfo << "Cannot find way to refinery" << endl;
			unit()->setWork(Unit::WorkNone);
			unit()->setAdvanceWork(Unit::WorkNone);
		} else {
			unit()->setAdvanceWork(Unit::WorkMove);
		}
	}
 }
}

void HarvesterPlugin::mineAt(const ResourceMinePlugin* resource)
{
 //TODO: don't move if unit cannot mine more minerals/oil or no minerals/oil at all
 boDebug() << k_funcinfo << endl;
 QPoint pos((int)resource->unit()->x(), (int)resource->unit()->y());
 unit()->moveTo(pos);
 unit()->setPluginWork(UnitPlugin::Harvester);
 unit()->setAdvanceWork(Unit::WorkMove);
 mResourcesX = pos.x();
 mResourcesY = pos.y();

 mHarvestingType = 1;
 mRefinery = 0;  // we'll search the closest refinery after mining
}


void HarvesterPlugin::refineAt(Unit* refinery)
{
 if (!refinery) {
	boError() << k_funcinfo << "NULL refinery" << endl;
	return;
 }
 RefineryProperties* prop = (RefineryProperties*)refinery->properties(PluginProperties::Refinery);
 if (!prop) {
	boError() << k_funcinfo << refinery->id() << " doesn't have refinery properties" << endl;
	return;
 }
 if (!(canMineMinerals() && prop->canRefineMinerals()) && !(canMineOil() && prop->canRefineOil())) {
	boError() << k_funcinfo << refinery->id() << " not a suitable refinery" << endl;
 }
 boDebug() << k_funcinfo << endl;
 setRefinery(refinery);
 unit()->setPluginWork(pluginType());
 mHarvestingType = 2; // refining
 // move...
 boDebug() << k_funcinfo << "move to refinery " << refinery->id() << endl;
 if (!unit()->moveTo(refinery->x(), refinery->y(), 1)) {
	boDebug() << k_funcinfo << "Cannot find way to refinery" << endl;
	unit()->setWork(Unit::WorkNone);
	unit()->setAdvanceWork(Unit::WorkNone);
 } else {
	unit()->setAdvanceWork(Unit::WorkMove);
 }
}


void HarvesterPlugin::setRefinery(Unit* refinery)
{
 if (!refinery || refinery->isDestroyed()) {
	return;
 }
 mRefinery = refinery;
}

bool HarvesterPlugin::canMineMinerals() const
{
 const HarvesterProperties* prop = (HarvesterProperties*)unit()->properties(PluginProperties::Harvester);
 if (!prop) {
	return false;
 }
 return prop->canMineMinerals();
}

bool HarvesterPlugin::canMineOil() const
{
 const HarvesterProperties* prop = (HarvesterProperties*)unit()->properties(PluginProperties::Harvester);
 if (!prop) {
	return false;
 }
 return prop->canMineOil();
}

unsigned int HarvesterPlugin::maxResources() const
{
 const HarvesterProperties* prop = (HarvesterProperties*)unit()->properties(PluginProperties::Harvester);
 if (!prop) {
	return 0;
 }
 return prop->maxResources();
}

unsigned int HarvesterPlugin::miningSpeed() const
{
 const HarvesterProperties* prop = (HarvesterProperties*)unit()->properties(PluginProperties::Harvester);
 if (!prop) {
	return 0;
 }
 return prop->miningSpeed();
}

unsigned int HarvesterPlugin::unloadingSpeed() const
{
 const HarvesterProperties* prop = (HarvesterProperties*)unit()->properties(PluginProperties::Harvester);
 if (!prop) {
	return 0;
 }
 return prop->unloadingSpeed();
}

void HarvesterPlugin::itemRemoved(BosonItem* item)
{
 if (item == (BosonItem*)mRefinery) {
	setRefinery(0);
 }
}

BombingPlugin::BombingPlugin(Unit* owner) : UnitPlugin(owner)
{
 owner->registerData(&mPosX, Unit::IdBombingPosX);
 owner->registerData(&mPosY, Unit::IdBombingPosY);
 mWeapon = 0;
 mPosX = 0;
 mPosY = 0;
}

BombingPlugin::~BombingPlugin()
{
}

void BombingPlugin::bomb(int weaponId, float x, float y)
{
 boDebug() << k_funcinfo << "wep: " << weaponId << "; pos: (" << x << "; " << y << ")" << endl;
 BosonWeapon* w = unit()->weapon(weaponId);
 if (!w) {
	boError() << k_funcinfo << "No weapon with id " << weaponId << endl;
	return;
 }
 if (w->properties()->shotType() != BosonShot::Bomb) {
	boError() << k_funcinfo << "Weapon with id " << weaponId << " is not a bomb" << endl;
	return;
 }

 unit()->stopMoving();

 int cellX = (int)(x / BO_TILE_SIZE);
 int cellY = (int)(y / BO_TILE_SIZE);
 if (!canvas()->cell(cellX, cellY)) {
	boError() << k_funcinfo << x << "," << y << " is no valid cell!" << endl;
	return;
 }

 // This is where unit has to be when making the drop
 // FIXME: REWRITE MOVING CODE OF Unit!!!
 // This is probably one of the worst examples of hacks used to work around
 //  problems with moving code. I want to be able to just write
 //  unit()->move(x, y) and unit would go exactly to (x, y) without any trouble.
 //  But ATM this isn't possible.
 mPosX = (int)(cellX * BO_TILE_SIZE + BO_TILE_SIZE / 2 - unit()->width() / 2/* + w->offset().x()*/);
 mPosY = (int)(cellY * BO_TILE_SIZE + BO_TILE_SIZE / 2 - unit()->height() / 2/* + w->offset().y()*/);
 boDebug() << k_funcinfo << "Drop-point: (" << mPosX << "; " << mPosY << ")" << endl;
 mWeapon = w;

 unit()->setPluginWork(UnitPlugin::Bombing);
}

void BombingPlugin::advance(unsigned int)
{
 boDebug() << k_funcinfo << endl;

 // Check if we're at the drop point
 float dist = QMAX(QABS(unit()->x() - mPosX), QABS(unit()->y() - mPosY));
 boDebug() << k_funcinfo << "dist: " << dist << endl;
 boDebug() << k_funcinfo << "my pos is: (" << unit()->x() << "; " << unit()->y() << ");  drop-point is: (" << mPosX << "; " << mPosX << ")" << endl; // ";  movedest is: (" << d->mMoveDestX << "; " << d->mMoveDestY << ")" << endl;
// int x = (int)(BosonItem::x() + width() / 2);
// int y = (int)(BosonItem::y() + height() / 2);
 if ((unit()->x() != mPosX) || (unit()->y() != mPosY)) {
// if ((x() != d->mMoveDestX) || (y() != d->mMoveDestY)) {
// if (dist > 2) {
	boDebug() << k_funcinfo << "not at drop point - moving..." << endl;
	if (!unit()->moveTo(mPosX, mPosY, 0)) {
		boWarning() << k_funcinfo << "Moving failed. Now what?" << endl;
		unit()->setWork(Unit::WorkNone);
	} else {
		unit()->pathInfo()->slowDownAtDest = false;
		unit()->addWaypoint(QPoint(mPosX, mPosY));
		unit()->setAdvanceWork(Unit::WorkMove);
	}
	return;
 }

 // We're at drop point. Drop the bomb
 boDebug() << k_funcinfo << "At drop-point!" << endl;
 if (mWeapon->reloaded()) {
	boDebug() << k_funcinfo << "Bomb ready. Dropping..." << endl;
	mWeapon->dropBomb();
	// And get the hell out of there
	// Go away from bomb's explosion radius
	float dist = mWeapon->properties()->damageRange() * BO_TILE_SIZE + unit()->width() / 2;
  boDebug() << k_funcinfo << "Getaway dist: " << dist << "; rot: " << unit()->rotation() << endl;
	float newx = unit()->x();
	float newy = unit()->y();
	// TODO: quite messy code, maybe it can be cleaned up somehow
	int rot = (int)unit()->rotation() % 360;
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
	newx = QMAX(0, QMIN(newx, (canvas()->mapWidth() - 1) * BO_TILE_SIZE));
	newy = QMAX(0, QMIN(newy, (canvas()->mapHeight() - 1) * BO_TILE_SIZE));

  boDebug() << k_funcinfo << "Getaway point is at (" << newx << "; " << newy << ")" << endl;
	if (!unit()->moveTo(newx, newy)) {
		boWarning() << k_funcinfo << "Aargh! Can't move away from drop-point!" << endl;
		unit()->setWork(Unit::WorkNone);
	} else {
		unit()->setWork(Unit::WorkMove);  // We don't want to return here anymore
	}
	mWeapon = 0;
	boDebug() << k_funcinfo << "returning" << endl;
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
 boDebug() << k_funcinfo << endl;

 // Lay the mine
 if (mWeapon->reloaded()) {
	// Don't place the mine immediately
	if (mPlacingCounter > 0) {
		boDebug() << k_funcinfo << "mPlacingCounter: " << mPlacingCounter << endl;
		mPlacingCounter = mPlacingCounter - 1;
		return;
	}

	boDebug() << k_funcinfo << "Mine ready. Placing..." << endl;
	mWeapon->layMine();

	// Go one cell away from the mine. Maybe go away from explosion radius?
	// FIXME: code taken from BombingPlugin. This could probably be written better
	float dist = 1 * BO_TILE_SIZE + unit()->width() / 2;
	boDebug() << k_funcinfo << "Getaway dist: " << dist << "; rot: " << unit()->rotation() << endl;
	float oldx = unit()->x();
	float oldy = unit()->y();
	float newx, newy;
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
		newx = QMAX(0, QMIN(newx, (canvas()->mapWidth() - 1) * BO_TILE_SIZE));
		newy = QMAX(0, QMIN(newy, (canvas()->mapHeight() - 1) * BO_TILE_SIZE));

		boDebug() << k_funcinfo << "i: " << i << "; Getaway point is at (" << newx << "; " << newy << ")" << endl;
		if (unit()->moveTo(newx, newy)) {
			unit()->setWork(Unit::WorkMove);  // We don't want to return here anymore
			couldmove = true;
			break;
		}
	}

	if (!couldmove) {
		boDebug() << k_funcinfo << "Can't move away!" << endl;
		unit()->setWork(Unit::WorkNone);  // We don't want to return here anymore
	}

	mWeapon = 0;
	boDebug() << k_funcinfo << "returning" << endl;
	return;
 } else {
	boDebug() << k_funcinfo << "Weapon not yet reloaded" << endl;
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

void ResourceMinePlugin::setMinerals(int m)
{
 if (!canProvideMinerals()) {
	mMinerals = m;
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

void ResourceMinePlugin::itemRemoved(BosonItem*)
{
}

