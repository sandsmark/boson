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

#include "unit.h"
#include "unitproperties.h"
#include "pluginproperties.h"
#include "speciestheme.h"
#include "player.h"
#include "bosoncanvas.h"
#include "boson.h"
#include "boitemlist.h"

#include "defines.h"

#include <kdebug.h>

ProductionPlugin::ProductionPlugin(Unit* unit)
{
 mUnit = unit;
 KGamePropertyHandler* dataHandler = mUnit->dataHandler();

 mProductions.registerData(Unit::IdPlugin_Productions, dataHandler, 
		KGamePropertyBase::PolicyLocal, "Productions");
 mProductionState.registerData(Unit::IdPlugin_ProductionState, dataHandler, 
		KGamePropertyBase::PolicyLocal, "ProductionState");
 mProductionState.setLocal(0);
 mProductions.setEmittingSignal(false); // just to prevent warning in Player::slotUnitPropertyChanged()
 mProductionState.setEmittingSignal(false); // called quite often - not emitting will increase speed a little bit
}

ProductionPlugin::~ProductionPlugin()
{
}

SpeciesTheme* ProductionPlugin::speciesTheme() const
{
 return unit()->speciesTheme();
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
 if (mProductionState < unit()->owner()->unitProperties(type)->productionTime()) {
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
	unit()->setWork(Unit::WorkProduce);
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
 unsigned int productionTime = unit()->owner()->unitProperties(currentProduction())->productionTime();
 double percentage = (double)(mProductionState * 100) / (double)productionTime;
 return percentage;
}

bool ProductionPlugin::canPlaceProductionAt(const QPoint& pos)
{
 if (!hasProduction() || completedProduction() <= 0) {
	kdDebug() << k_lineinfo << "no completed construction" << endl;
	return false;
 }
 QValueList<Unit*> list = unit()->canvas()->unitCollisionsInRange(pos, BUILD_RANGE);

 const UnitProperties* prop = unit()->owner()->unitProperties(currentProduction());
 if (!prop) {
	kdError() << k_lineinfo << "NULL unit properties - EVIL BUG!" << endl;
	return false;
 }
 if (prop->isFacility()) {
	// a facility can be placed within BUILD_RANGE of *any* friendly
	// facility on map
	for (unsigned int i = 0; i < list.count(); i++) {
		if (list[i]->isFacility() && list[i]->owner() == unit()->owner()) {
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


void ProductionPlugin::advance()
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
 // a unit is completed as soon as mProductionState == owner()->unitProperties(type)->productionTime()
 unsigned int productionTime = unit()->owner()->unitProperties(type)->productionTime();
 if (mProductionState <= productionTime) {
	if (mProductionState == productionTime) {
		kdDebug() << "unit " << type << " completed :-)" << endl;
		mProductionState = mProductionState + 1;
		// Auto-place unit
		// Unit positioning scheme: all tiles starting with tile that is below
		// facility's lower-left tile, are tested counter-clockwise. Unit is placed
		// to first free tile.
		// No auto-placing for facilities
		if(!unit()->owner()->unitProperties(type)) {
			kdError() << k_lineinfo << "Unknown type " << type << endl;
			return;
		}
		if(unit()->owner()->unitProperties(type)->isFacility()) {
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
//				if(unit()->canvas()->cellOccupied(currentx, currenty)) {
//				FIXME: should not depend on Facility*
				if(unit()->canvas()->canPlaceUnitAt(unit()->speciesTheme()->unitProperties(type), QPoint(currentx * BO_TILE_SIZE, currenty * BO_TILE_SIZE), (Facility*)unit())) {
					// Free cell - place unit at it
					mProductionState = mProductionState + 1;
					//FIXME: buildProduction should not
					//depend on Facility! should be Unit
					((Boson*)unit()->owner()->game())->buildProducedUnit((Facility*)unit(), type, currentx, currenty);
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

RepairPlugin::RepairPlugin(Unit* unit)
{
 mUnit = unit;

 /*
 KGamePropertyHandler* dataHandler = mUnit->dataHandler();
 mRepairList.registerData(Unit::IdPlugin_RepairList, dataHandler, 
		KGamePropertyBase::PolicyLocal, "RepairList");
 mRepairList.setEmittingSignal(false); // just to prevent warning in Player::slotUnitPropertyChanged()
 */

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
	if (!unit()->owner()->isEnemy(u->owner())) {
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


