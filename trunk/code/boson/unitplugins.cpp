/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include <kdebug.h>

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


ProductionPlugin::ProductionPlugin(Unit* unit) : UnitPlugin(unit)
{
 mProductions.registerData(Unit::IdPlugin_Productions, dataHandler(),
		KGamePropertyBase::PolicyLocal, "Productions");
 mProductionState.registerData(Unit::IdPlugin_ProductionState, dataHandler(),
		KGamePropertyBase::PolicyLocal, "ProductionState");
 mProductionState.setLocal(0);
 mProductions.setEmittingSignal(false); // just to prevent warning in Player::slotUnitPropertyChanged()
 mProductionState.setEmittingSignal(false); // called quite often - not emitting will increase speed a little bit
}

ProductionPlugin::~ProductionPlugin()
{
}

unsigned long int ProductionPlugin::completedProduction() const
{
 if (!hasProduction()) {
	return 0;
 }
 unsigned long int type = currentProduction();
 if (type == 0) {
	return 0;
 }
 if (mProductionState < speciesTheme()->unitProperties(type)->productionTime()) {
	kdDebug() << "not yet completed: " << type << endl;
	return 0;
 }
 return type;
}

void ProductionPlugin::addProduction(unsigned long int unitType)
{
 ProductionProperties* p = (ProductionProperties*)unit()->unitProperties()->properties(PluginProperties::Production);
 if (!p) {
	kdError() << k_funcinfo << "NULL production properties" << endl;
	return;
 }
 if (!speciesTheme()->productions(p->producerList()).contains(unitType)) {
	kdError() << k_funcinfo << " cannot produce " << unitType << endl;
	return;
 }
 bool start = false;
 if (!hasProduction()) {
	start = true;
 }
 mProductions.append(unitType);
 if (start) {
	unit()->setPluginWork(pluginType());
 }
}

void ProductionPlugin::removeProduction()
{
 mProductions.pop_front();
 mProductionState = 0; // start next production (if any)
}

void ProductionPlugin::removeProduction(unsigned long int unitType)
{
 for (unsigned int i = 0; i < productionList().count(); i++) {
	if (mProductions[i] == unitType) {
		kdDebug() << k_funcinfo << "remove " << unitType << endl;
		mProductions.remove(mProductions.at(i));
		return;
	}
 }
}

double ProductionPlugin::productionProgress() const
{
 unsigned int productionTime = speciesTheme()->unitProperties(currentProduction())->productionTime();
 double percentage = (double)(mProductionState * 100) / (double)productionTime;
 return percentage;
}

bool ProductionPlugin::canPlaceProductionAt(const QPoint& pos)
{
 if (!hasProduction() || completedProduction() <= 0) {
	kdDebug() << k_lineinfo << "no completed construction" << endl;
	return false;
 }
 QValueList<Unit*> list = canvas()->unitCollisionsInRange(pos, BUILD_RANGE);

 const UnitProperties* prop = speciesTheme()->unitProperties(currentProduction());
 if (!prop) {
	kdError() << k_lineinfo << "NULL unit properties - EVIL BUG!" << endl;
	return false;
 }
 if (prop->isFacility()) {
	// a facility can be placed within BUILD_RANGE of *any* friendly
	// facility on map
	for (unsigned int i = 0; i < list.count(); i++) {
		if (list[i]->isFacility() && list[i]->owner() == player()) {
			kdDebug() << "Facility in BUILD_RANGE" << endl;
			// TODO: also check whether a unit is already at that position!!
			return true;
		}
	}
 } else {
	// a mobile unit can be placed within BUILD_RANGE of its factory *only*
	for (unsigned int i = 0; i < list.count(); i++) {
		if (list[i] == (Unit*)this) {
			// TODO: also check whether a unit is already at that position!!
			return true;
		}
	}
 }
 return false;
}


void ProductionPlugin::advance(unsigned int)
{
 unsigned long int type = currentProduction();
 if (type <= 0) { // no production
	unit()->setWork(Unit::WorkNone);
	mProductionState = 0;
	return;
 }



 // FIXME: this code is broken!
 // it gets executed on *every* client but sendInput() should be used on *one*
 // client only!
 // a unit is completed as soon as mProductionState == player()->unitProperties(type)->productionTime()
 unsigned int productionTime = speciesTheme()->unitProperties(type)->productionTime();
 if (mProductionState <= productionTime) {
	if (mProductionState == productionTime) {
		kdDebug() << "unit " << type << " completed :-)" << endl;
		mProductionState = mProductionState + 1;
		// Auto-place unit
		// Unit positioning scheme: all tiles starting with tile that is below
		// facility's lower-left tile, are tested counter-clockwise. Unit is placed
		// to first free tile.
		// No auto-placing for facilities
		if(!speciesTheme()->unitProperties(type)) {
			kdError() << k_lineinfo << "Unknown type " << type << endl;
			return;
		}
		if(speciesTheme()->unitProperties(type)->isFacility()) {
			return;
		}
		int tilex, tiley; // Position of lower-left corner of facility in tiles
		int theight, twidth; // height and width of facility in tiles
		int currentx, currenty; // Position of tile currently tested
		theight = unit()->height() / BO_TILE_SIZE;
		twidth =unit()-> width() / BO_TILE_SIZE;
		tilex = (int)(unit()->x() / BO_TILE_SIZE);
		tiley = (int)(unit()->y() / BO_TILE_SIZE + theight);
		int tries; // Tiles to try for free space
		int ctry; // Current try
		currentx = tilex - 1;
		currenty = tiley - 1;
		for(int i=1; i <= 3; i++) {
			tries = 2 * i * twidth + 2 * i * theight + 4;
			currenty++;
			for(ctry = 1; ctry <= tries; ctry++) {
				kdDebug() << "    Try " << ctry << " of " << tries << endl;
				if(ctry <= twidth + i) {
					currentx++;
				} else if(ctry <= twidth + i + theight + 2 * i - 1) {
					currenty--;
				} else if(ctry <= twidth + i + 2 * (theight + 2 * i - 1)) {
					currentx--;
				} else if(ctry <= twidth + i + 3 * (theight + 2 * i - 1)) {
					currenty++;
				} else {
					currentx++;
				}

				//TODO: use BosonCanvas::canPlaceUnitAt()
//				if(canvas()->cellOccupied(currentx, currenty)) {
//				FIXME: should not depend on Facility*
				if(canvas()->canPlaceUnitAt(speciesTheme()->unitProperties(type), QPoint(currentx * BO_TILE_SIZE, currenty * BO_TILE_SIZE), this)) {
					// Free cell - place unit at it
					mProductionState = mProductionState + 1;
					//FIXME: buildProduction should not
					//depend on Facility! should be Unit
					((Boson*)player()->game())->buildProducedUnit((Facility*)unit(), type, currentx, currenty);
					return;
				}
			}
		}
		kdDebug() << "Cannot find free cell around facility :-(" << endl;
	} else {
		mProductionState = mProductionState + 1;
	}
 }
}

RepairPlugin::RepairPlugin(Unit* unit) : UnitPlugin(unit)
{
}

RepairPlugin::~RepairPlugin()
{
}

void RepairPlugin::repair(Unit* u)
{
 kdDebug() << k_funcinfo << endl;
 if (unit()->isFacility()) {
	if (!u->moveTo(unit()->x(), unit()->y(), 1)) {
		kdDebug() << u->id() << " cannot find a way to repairyard" << endl;
		u->setWork(Unit::WorkNone);
	} else {
		u->setWork(Unit::WorkMove);
	}
 } else {
	if (!unit()->moveTo(u->x(), u->y(), 1)) {
		kdDebug() << "Cannot find way to " << u->id() << endl;
		unit()->setWork(Unit::WorkNone);
	} else {
		unit()->setAdvanceWork(Unit::WorkMove);
	}
 }
}

void RepairPlugin::repairInRange()
{
// kdDebug() << k_funcinfo << endl;
 //TODO: support for friendly non-player (i.e. allied) units

 // TODO: once we started repairing a unit also repair it in the next call,
 // until it isn't in range anymore or is repaired
 BoItemList list = unit()->unitsInRange();
 for (unsigned int i = 0; i < list.count(); i++) {
	Unit* u = (Unit*)list[i];
	if (u->health() >= u->unitProperties()->health()) {
		continue;
	}
	if (!player()->isEnemy(u->owner())) {
		kdDebug() << "repair " << u->id() << endl;
		int diff = u->health() - u->unitProperties()->health();
		if (diff > 0) {
			kdError() << k_funcinfo << "health > maxhealth" << endl;
			continue;
		}
		if (diff < unit()->weaponDamage()) {
			// usually the case
			u->setHealth(u->health() - unit()->weaponDamage());
		} else {
			u->setHealth(u->health() - diff);
		}
		return; // only one unit at once
	}
 }
}

HarvesterPlugin::HarvesterPlugin(Unit* unit)
		: UnitPlugin(unit)
{
 // FIXME: we should clean the property IDs. They should be in UnitPlugin, not
 // in Unit.
 mResourcesMined.registerData(Unit::IdMob_ResourcesMined, dataHandler(),
		KGamePropertyBase::PolicyLocal, "ResourcesMined");
 mResourcesMined.setLocal(0);
 mResourcesX.registerData(Unit::IdMob_ResourcesX, dataHandler(),
		KGamePropertyBase::PolicyLocal, "ResourcesX");
 mResourcesX.setLocal(0);
 mResourcesY.registerData(Unit::IdMob_ResourcesY, dataHandler(),
		KGamePropertyBase::PolicyLocal, "ResourcesY");
 mResourcesY.setLocal(0);
 mHarvestingType.registerData(Unit::IdMob_HarvestingType, dataHandler(),
		KGamePropertyBase::PolicyLocal, "HarvestingType");
 mHarvestingType.setLocal(0);
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

void HarvesterPlugin::advanceMine()
{
 const HarvesterProperties* prop = (HarvesterProperties*)unit()->properties(PluginProperties::Harvester);
 if (!prop) {
	kdError() << k_funcinfo << "NULL harvester properties" << endl;
	unit()->setWork(Unit::WorkNone);
	return;
 }
 if (resourcesMined() < prop->maxResources()) {
	if (canMine(canvas()->cellAt(unit()))) {
		const int step = (resourcesMined() + 10 <= prop->maxResources()) ? 10 : prop->maxResources() - resourcesMined();
		mResourcesMined = resourcesMined() + step;
		if (prop->canMineMinerals()) {
			player()->statistics()->increaseMinedMinerals(step);
		} else if (prop->canMineOil()) {
			player()->statistics()->increaseMinedOil(step);
		}
		kdDebug() << "resources mined: " << resourcesMined() << endl;
	} else {
		kdDebug() << k_funcinfo << "cannot mine here" << endl;
		unit()->setWork(Unit::WorkNone);
		return;
	}
 } else {
	kdDebug() << k_funcinfo << "Maximal amount of resources mined." << endl;
	mHarvestingType = 2; // refining
 }
}

void HarvesterPlugin::advanceRefine()
{
 kdDebug() << k_funcinfo << endl;
 if (resourcesMined() == 0) {
	kdDebug() << k_funcinfo << "refining done" << endl;
	if (resourcesX() != -1 && resourcesY() != -1) {
		mineAt(QPoint(resourcesX(), resourcesY()));
	} else {
		unit()->setWork(Unit::WorkNone);
	}
	return;
 }
 if (!refinery()) {
	// TODO: pick closest refinery
	QPtrList<Unit> list = player()->allUnits();
	QPtrListIterator<Unit> it(list);
	const HarvesterProperties* prop = (HarvesterProperties*)unit()->properties(PluginProperties::Harvester);
	if (!prop) {
		kdError() << k_funcinfo << "NULL harvester plugin" << endl;
		unit()->setWork(Unit::WorkNone);
		return;
	}
	Facility* ref = 0;
	while (it.current() && !ref) {
		const UnitProperties* unitProp = it.current()->unitProperties();
		if (!it.current()->isFacility()) {
			++it;
			continue;
		}
		if (prop->canMineMinerals() && unitProp->canRefineMinerals()) {
			ref = (Facility*)it.current();
		} else if (prop->canMineOil() && unitProp->canRefineOil()) {
			ref = (Facility*)it.current();
		}
		++it;
	}
	if (!ref) {
		kdDebug() << k_funcinfo << "no suitable refinery found" << endl;
		unit()->setWork(Unit::WorkNone);
	} else {
		kdDebug() << k_funcinfo << "refinery: " << ref->id() << endl;
		refineAt(ref);
	}
	return;
 } else {
	if (unit()->isNextTo(refinery())) {
		const int step = (resourcesMined() >= 10) ? 10 : resourcesMined();
		mResourcesMined = resourcesMined() - step;
		const HarvesterProperties* prop = (HarvesterProperties*)unit()->properties(PluginProperties::Harvester);
		if (!prop) {
			kdError() << k_funcinfo << "NULL harvester plugin" << endl;
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
	}
 }
}

void HarvesterPlugin::mineAt(const QPoint& pos)
{
 //TODO: don't move if unit cannot mine more minerals/oil or no minerals/oil at all
 kdDebug() << k_funcinfo << endl;
 unit()->moveTo(pos);
 unit()->setPluginWork(UnitPlugin::Harvester);
 unit()->setAdvanceWork(Unit::WorkMove);
 mResourcesX = pos.x();
 mResourcesY = pos.y();
 mHarvestingType = 1;
}


void HarvesterPlugin::refineAt(Unit* refinery)
{
 if (!refinery) {
	kdError() << k_funcinfo << "NULL refinery" << endl;
	return;
 }
 if (!refinery->unitProperties()->canRefineMinerals() &&
		!refinery->unitProperties()->canRefineOil()) {
	kdError() << k_funcinfo << refinery->id() << " not a refinery" << endl;
 }
 kdDebug() << k_funcinfo << endl;
 setRefinery(refinery);
 unit()->setPluginWork(pluginType());
 mHarvestingType = 2; // refining
 // move...
 kdDebug() << k_funcinfo << "move to refinery " << refinery->id() << endl;
 if (!unit()->moveTo(refinery->x(), refinery->y(), 1)) {
	kdDebug() << k_funcinfo << "Cannot find way to refinery" << endl;
	unit()->setWork(Unit::WorkNone);
 } else {
	unit()->setAdvanceWork(Unit::WorkMove);
 }
}


void HarvesterPlugin::setRefinery(Unit* refinery)
{
 if (!refinery || refinery->isDestroyed()) {
	return;
 }
 const HarvesterProperties* prop = (HarvesterProperties*)unit()->properties(PluginProperties::Harvester);
 if (!prop) {
	return;
 }
 const UnitProperties* refProp = refinery->unitProperties(); // TODO: replace by pluginProperties
 if (prop->canMineMinerals() && refProp->canRefineMinerals()) {
	mRefinery = refinery;
 } else if (prop->canMineOil() && refProp->canRefineOil()) {
	mRefinery = refinery;
 }
}

bool HarvesterPlugin::canMine(Cell* cell) const
{
 if (canMineMinerals() && cell->groundType() == Cell::GroundGrassMineral) {
	return true;
 }
 if (canMineOil() && cell->groundType() == Cell::GroundGrassOil) {
	return true;
 }
 return false;
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
	return false;
 }
 return prop->maxResources();
}

