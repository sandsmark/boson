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

#include "bosongameviewinput.h"
#include "bosongameviewinput.moc"

#include "../bomemory/bodummymemory.h"
#include "../no_player.h"
#include "bosongameview.h"
#include "../boselection.h"
#include "../bosoncanvas.h"
#include "../bosonconfig.h"
#include "../boson.h"
#include "../bosoncursor.h"
#include "../playerio.h"
#include "../unitproperties.h"
#include "../pluginproperties.h"
#include "../unit.h"
#include "../unitplugins.h"
#include "bodebug.h"
#include "../boaction.h"
#include "bosonlocalplayerinput.h"
#include "../bosonweapon.h"
#include "../bo3dtools.h"
#include "../speciestheme.h"
#include "../speciesdata.h"

#include <klocale.h>

BosonGameViewInput::BosonGameViewInput()
	: BosonGameViewInputBase()
{
 weaponId = -1;
}

BosonGameViewInput::~BosonGameViewInput()
{
}

void BosonGameViewInput::actionClicked(const BoMouseEvent& event)
{
 boDebug() << k_funcinfo << endl;
// this method should not perform any tasks but rather send the input through
// the KGameIO. this way it is very easy (it should be at least) to write a
// computer player
 BO_CHECK_NULL_RET(selection());
 BO_CHECK_NULL_RET(canvas());
 BO_CHECK_NULL_RET(localPlayerIO());
 if (selection()->isEmpty()) {
	return;
 }
 if (!canvas()->onCanvas(event.canvasVector())) {
	boError() << k_funcinfo << event.canvasVector().x() << "," << event.canvasVector().y() << " is not on the canvas!" << endl;
	return;
 }

 if (actionLocked()) {
	bool unlock = false;
	switch (actionType()) {
		case ActionMove:
		{
			unlock = actionMoveWithoutAttack(event.canvasVector());
			break;
		}
		case ActionAttack:
		{
			Unit* unit = canvas()->findUnitAt(event.canvasVector());
			if (unit) {
				unlock = actionAttack(event.canvasVector());
			} else {
				unlock = actionMoveWithAttack(event.canvasVector());
			}
			break;
		}
		case ActionDropBomb:
		{
			unlock = actionDropBomb(event.canvasVector());
			break;
		}
		case ActionFollow:
		{
			Unit* unit = canvas()->findUnitAt(event.canvasVector());
			if (unit) {
				unlock = actionFollow(event.canvasVector());
			}
			break;
		}
		case ActionHarvest:
		{
			// TODO check if player clicked on oil/minerals
			unlock = actionHarvest(event.canvasVector());
			break;
		}
		case ActionRepair:
		{
			unlock = actionRepair(event.canvasVector());
			break;
		}
		case ActionPlacementPreview:
		{
			unlock = actionBuild(event.canvasVector());
			break;
		}
		default:
			boError() << k_funcinfo << "Unsupported actiontype for locked action: " << actionType() << endl;
			break;
	}
	if (unlock) {
		// TODO: also play "cannot do that" sound
		unlockAction();
	}
	return;
 }

 Unit* unit = 0;
 unit = localPlayerIO()->findUnitAt(canvas(), event.canvasVector()); // also checks for canSee()
 if (!unit) {
	//FIXME: first check if a the unit can produce! even mobile units can
	//have the production plugin!!
	if (selection()->hasMobileUnit()) { // move the selection to pos
		if (selection()->count() == 1) {
			// there are special things to do for a single selected unit
			// (e.g. mining if the unit is a harvester)
			/*
			 * AB: obsolete, as harvesting will take place on units
			 * (->mines) only!
			if (actionHarvest(event.canvasVector())) {
				return;
			}
			*/
		}

		if (boConfig->boolValue("RMBMovesWithAttack")) {
			if (!actionMoveWithAttack(event.canvasVector())) {
				return;
			}
		} else {
			if (!actionMoveWithoutAttack(event.canvasVector())) {
				return;
			}
		}
	} else {
		// TODO: another option: add the waypoint to the facility and
		// apply it to any unit that gets constructed by that facility.
		// For this we'd probably have to use LMB for unit placing
		// AB: the facility-placement gets done for actionLocked()==true
		// now ! (ActionBuild)
	}
 } else { // there is a unit - attack it?
	if ((localPlayerIO()->isEnemy(unit) || event.forceAttack()) &&
			selection()->canShootAt(unit)) {
		// attack the unit
		if (!actionAttack(event.canvasVector())) {
			return;
		}

	} else if (localPlayerIO()->isEnemy(unit)) {
		// a non-friendly unit, but the selection cannot shoot
		// we probably won't do anything here
		// IDEA: what about "I cannot shoot that!" sound?
	} else if (localPlayerIO()->isAllied(unit)) {
		// click on a friendly unit
		if (unit->isFacility() && unit->plugin(UnitPlugin::Repair)) {
			// some kind of repairyard - repair all units
			// (not yet implemented) //FIXME
			// note that currently the unit can go to every friendly
			// player, even non-local players
			if (!actionRepair(event.canvasVector())) {
				return;
			}
		} else if (unit->properties(PluginProperties::Refinery)) {
			const RefineryProperties* prop = (RefineryProperties*)unit->properties(PluginProperties::Refinery);
			if((prop->canRefineMinerals() && selection()->hasMineralHarvester()) ||
					(prop->canRefineOil() && selection()->hasOilHarvester())) {
				// go to the refinery
				if (!actionRefine(event.canvasVector())) {
					return;
				}
			}
		} else if (selection()->hasMobileUnit() && unit->isMobile()) {
			// Follow
			if (!actionFollow(event.canvasVector())) {
				return;
			}
		} else {
			// selection and clicked unit both are friendly
			// no repairyard and no refinery
			// (at least no valid)
			// add other possibilities here
		}
	} else if (localPlayerIO()->isNeutral(unit)) {
		// click on a neutral unit
		// note: this is NOT a friendly unit, so we won' repair it or
		// so. but we won't shoot at it either (by default).

		if (actionHarvest(event.canvasVector())) {
			return;
		}
	}
 }

}


bool BosonGameViewInput::actionHarvest(const BoVector3Fixed& canvasVector)
{
 if (!canvas()) {
	BO_NULL_ERROR(canvas());
	return false;
 }
 if (!selection()) {
	BO_NULL_ERROR(selection());
	return false;
 }
 if (!localPlayerInput()) {
	BO_NULL_ERROR(localPlayerInput());
	return false;
 }
 Unit* resourceUnit = canvas()->findUnitAt(canvasVector);
 if (!resourceUnit) {
	return false;
 }
 ResourceMinePlugin* resource = (ResourceMinePlugin*)resourceUnit->plugin(UnitPlugin::ResourceMine);
 if (!resource) {
	// there is no mine at destination
	return false;
 }
 boDebug() << k_funcinfo << "clicked on a resource mine" << endl;

 bool taken = false;
 QPtrList<Unit> units = selection()->allUnits();
 QPtrListIterator<Unit> it(units);
 while (it.current()) {
	Unit* u = it.current();
	++it;

	HarvesterPlugin* h = (HarvesterPlugin*)u->plugin(UnitPlugin::Harvester);
	if (!h) {
		continue;
	}
	boDebug() << k_funcinfo << u->id() << " is a harvester" << endl;
	if (resource->isUsableTo(h)) {
		boDebug() << k_funcinfo << u->id() << " will harvest at " << resource->unit()->id() << "," << canvasVector.y() << " which is " << h->unit()->id() << endl;
		localPlayerInput()->harvest(h, resource);
		taken = true;
	} else {
		boDebug() << k_funcinfo << u->id() << " cannot mine at " << resourceUnit->id() << endl;
	}
 }
 return taken;
}

bool BosonGameViewInput::actionMoveWithoutAttack(const BoVector3Fixed& canvasVector)
{
 if (!selection()) {
	BO_NULL_ERROR(selection());
	return false;
 }
 if (!localPlayerIO()) {
	BO_NULL_ERROR(localPlayerIO());
	return false;
 }
 if (!localPlayerInput()) {
	BO_NULL_ERROR(localPlayerInput());
	return false;
 }
 // AB: note that only x and y are relevant from canvasVector !
 // z is ignored
 localPlayerInput()->moveWithoutAttack(selection()->allUnits(), canvasVector.x(), canvasVector.y());
 if (localPlayerIO()->ownsUnit(selection()->leader())) {
	Unit* leader = selection()->leader();
	if (leader->speciesTheme()) {
		leader->speciesTheme()->data()->playSound(leader, SoundOrderMove);
	}
 }
 return true;
}

bool BosonGameViewInput::actionMoveWithAttack(const BoVector3Fixed& canvasVector)
{
 if (!localPlayerIO()) {
	BO_NULL_ERROR(localPlayerIO());
	return false;
 }
 if (!selection()) {
	BO_NULL_ERROR(selection());
	return false;
 }
 if (!localPlayerInput()) {
	BO_NULL_ERROR(localPlayerInput());
	return false;
 }
 // AB: note that only x and y are relevant from canvasVector !
 // z is ignored
 localPlayerInput()->moveWithAttack(selection()->allUnits(), canvasVector.x(), canvasVector.y());
 if (localPlayerIO()->ownsUnit(selection()->leader())) {
	Unit* leader = selection()->leader();
	if (leader->speciesTheme()) {
		leader->speciesTheme()->data()->playSound(leader, SoundOrderMove);
	}
 }
 return true;
}

bool BosonGameViewInput::actionBuild(const BoVector3Fixed& canvasVector)
{
 if (!localPlayerIO()) {
	BO_NULL_ERROR(localPlayerIO());
	return false;
 }
 if (!localPlayerInput()) {
	BO_NULL_ERROR(localPlayerInput());
	return false;
 }
 if (!selection()) {
	BO_NULL_ERROR(selection());
	return false;
 }
 if (!canvas()) {
	BO_NULL_ERROR(canvas());
	return false;
 }
 Unit* factory = selection()->leader();
 if (!factory) {
	return false;
 }

 bofixed x = rintf(canvasVector.x());
 bofixed y = rintf(canvasVector.y());

 // FIXME: lot of this code should probably be moved to BosonLocalPlayerInput
 ProductionPlugin* production = (ProductionPlugin*)(factory->plugin(UnitPlugin::Production));
 if (!production || !production->hasProduction() || production->completedProductionId() <= 0) {
	return false;
 }

 const UnitProperties* prop = localPlayerIO()->unitProperties(production->currentProductionId());
 if (!prop) {
	boError() << k_funcinfo << "NULL unit properties" << endl;
	return false;
 }
 if (!canvas()->canPlaceUnitAt(prop, BoVector2Fixed(x, y), production)) {
	boDebug() << k_funcinfo << "Cannot place production here" << endl;
	boGame->slotAddChatSystemMessage(i18n("You can't place a %1 there").arg(prop->name()));
	return false;
 }

 // create the new unit
 localPlayerInput()->build(production->completedProductionType(), factory, x, y);
 return true;
}

bool BosonGameViewInput::actionAttack(const BoVector3Fixed& canvasVector)
{
 if (!localPlayerIO()) {
	BO_NULL_ERROR(localPlayerIO());
	return false;
 }
 if (!localPlayerInput()) {
	BO_NULL_ERROR(localPlayerInput());
	return false;
 }
 if (!selection()) {
	BO_NULL_ERROR(selection());
	return false;
 }
 if (!canvas()) {
	BO_NULL_ERROR(canvas());
	return false;
 }
 Unit* unit = canvas()->findUnitAt(canvasVector);
 localPlayerInput()->attack(selection()->allUnits(), unit);
 Unit* u = selection()->leader();
 if (localPlayerIO()->ownsUnit(u)) {
	if (u->speciesTheme()) {
		u->speciesTheme()->data()->playSound(u, SoundOrderAttack);
	}
 }
 return true;
}

bool BosonGameViewInput::actionDropBomb(const BoVector3Fixed& canvasVector)
{
 if (!localPlayerIO()) {
	BO_NULL_ERROR(localPlayerIO());
	return false;
 }
 if (!localPlayerInput()) {
	BO_NULL_ERROR(localPlayerInput());
	return false;
 }
 if (!selection()) {
	BO_NULL_ERROR(selection());
	return false;
 }
 // AB: note that only x and y are relevant from canvasVector !
 // z is ignored
 localPlayerInput()->dropBomb(selection()->leader(), weaponId, canvasVector.x(), canvasVector.y());
 weaponId = -1;
 if (localPlayerIO()->ownsUnit(selection()->leader())) {
	Unit* leader = selection()->leader();
	if (leader->speciesTheme()) {
		leader->speciesTheme()->data()->playSound(leader, SoundOrderAttack);
	}
 }
 return true;
}

bool BosonGameViewInput::actionRepair(const BoVector3Fixed& canvasVector)
{
 if (!selection()) {
	BO_NULL_ERROR(selection());
	return false;
 }
 if (!canvas()) {
	BO_NULL_ERROR(canvas());
	return false;
 }
 if (!localPlayerInput()) {
	BO_NULL_ERROR(localPlayerInput());
	return false;
 }
 Unit* unit = canvas()->findUnitAt(canvasVector);
 QPtrList<Unit> allUnits = selection()->allUnits();
 QPtrList<Unit> list;
 QPtrListIterator<Unit> it(allUnits);
 while (it.current()) {
	if (it.current()->health() < it.current()->maxHealth()) {
		boDebug() << k_funcinfo << "repair " << it.current()->id() << endl;
		list.append(it.current());
	}
	++it;
 }
 localPlayerInput()->repair(list, unit);
 return true;
}

bool BosonGameViewInput::actionRefine(const BoVector3Fixed& canvasVector)
{
 if (!selection()) {
	BO_NULL_ERROR(selection());
	return false;
 }
 if (!canvas()) {
	BO_NULL_ERROR(canvas());
	return false;
 }
 if (!localPlayerInput()) {
	BO_NULL_ERROR(localPlayerInput());
	return false;
 }
 Unit* unit = canvas()->findUnitAt(canvasVector);
 const RefineryProperties* prop = (RefineryProperties*)unit->properties(PluginProperties::Refinery);
 if (!prop) {
	boError() << k_funcinfo << unit->id() << "cannot refine" << endl;
 }
 QPtrList<Unit> allUnits = selection()->allUnits();
 QPtrList<Unit> list;
 QPtrListIterator<Unit> unitsIt(allUnits);
 while (unitsIt.current()) {
	HarvesterProperties* hprop = (HarvesterProperties*)unitsIt.current()->properties(PluginProperties::Harvester);
	if (prop) {
		if ((prop->canRefineMinerals() && hprop->canMineMinerals()) ||
				(prop->canRefineOil() && hprop->canMineOil())) {
			list.append(unitsIt.current());
		}
	}
	++unitsIt;
 }
 if (!list.count()) {
	boError() << k_lineinfo << "MoveRefine: empty list!!" << endl;
	return false;
 }
 localPlayerInput()->refine(list, unit);
 return true;
}

bool BosonGameViewInput::actionFollow(const BoVector3Fixed& canvasVector)
{
 if (!localPlayerIO()) {
	BO_NULL_ERROR(localPlayerIO());
	return false;
 }
 if (!selection()) {
	BO_NULL_ERROR(selection());
	return false;
 }
 if (!canvas()) {
	BO_NULL_ERROR(canvas());
	return false;
 }
 if (!localPlayerInput()) {
	BO_NULL_ERROR(localPlayerInput());
	return false;
 }
 Unit* unit = canvas()->findUnitAt(canvasVector);
 localPlayerInput()->follow(selection()->allUnits(), unit);
 Unit* u = selection()->leader();
 if (localPlayerIO()->ownsUnit(u)) {
	if (u->speciesTheme()) {
		u->speciesTheme()->data()->playSound(u, SoundOrderMove);
	}
 }
 return true;
}

void BosonGameViewInput::updatePlacementPreviewData()
{
 BO_CHECK_NULL_RET(localPlayerIO());
 BO_CHECK_NULL_RET(selection());
 BO_CHECK_NULL_RET(canvas());
 emit signalSetPlacementPreviewData(0, false, placementFreePlacement(), !placementDisableCollisions());
 if (!actionLocked() || actionType() != ActionPlacementPreview) {
	return;
 }
 if (!selection() || selection()->isEmpty() || !selection()->leader()) {
	return;
 }
 Unit* leader = selection()->leader();
 if (!localPlayerIO()->ownsUnit(leader)) {
	return;
 }
 ProductionPlugin* pp = (ProductionPlugin*)leader->plugin(UnitPlugin::Production);
 if (!pp || pp->completedProductionId() == 0 || pp->completedProductionType() != ProduceUnit) {
	return;
 }
 const UnitProperties* prop = localPlayerIO()->unitProperties(pp->completedProductionId());
 if (!prop) {
	return;
 }
 // note: this applies to mobiles as well as for facilities!
 // (mobiles are usually auto placed, but manual placement might get used if
 // auto-placement failed)
 bool canPlace = canvas()->canPlaceUnitAt(prop, BoVector2Fixed(cursorCanvasVector().x(), cursorCanvasVector().y()), pp);
 emit signalSetPlacementPreviewData(prop, canPlace, placementFreePlacement(), !placementDisableCollisions());

}

void BosonGameViewInput::action(const BoSpecificAction& action)
{
 boDebug() << k_funcinfo << endl;
 BO_CHECK_NULL_RET(localPlayerIO());
 BO_CHECK_NULL_RET(selection());

 switch (action.type()) {
	case ActionFollow:
	case ActionHarvest:
	case ActionRepair:
	case ActionMove:
		setCursorType(CursorMove);
		break;
	case ActionPlacementPreview:
		break;
	case ActionAttack:
		setCursorType(CursorAttack);
		break;
	case ActionDropBomb:
		weaponId = action.weapon()->id();
		setCursorType(CursorAttack);
		break;
	default:
		boError() << k_funcinfo << "Invalid or unsupported action! type: " << action.type() << endl;
		return;
 }
 setActionType(action.type());
 lockAction();
}

void BosonGameViewInput::updateCursor()
{
 BosonCursor* c = cursor();
 if (!c) {
	boError() << k_funcinfo << "NULL cursor!!" << endl;
	return;
 }
 BO_CHECK_NULL_RET(localPlayerIO());
 BO_CHECK_NULL_RET(selection());
 BO_CHECK_NULL_RET(canvas());

 if (!canvas()->onCanvas(cursorCanvasVector())) {
	if (actionLocked()) {  // TODO: show "can't do that" cursor if action is locked
	} else {
		setCursorType(CursorDefault);
		c->setCursor(cursorType());
		return;
	}
 }
 if (actionLocked()) {
	c->setCursor(cursorType());
	return;
 }

 if (!selection()->isEmpty() && localPlayerIO()->ownsUnit(selection()->leader())) {
	Unit* leader = selection()->leader();
	if (localPlayerIO()->isFogged(cursorCanvasVector())) {
		if (leader->isMobile()) {
			setCursorType(CursorMove);
		} else {
			setCursorType(CursorDefault);
		}
	} else {
		Unit* unit = canvas()->findUnitAt(cursorCanvasVector());
		if (unit) {
			if (localPlayerIO()->ownsUnit(unit)) {
				setCursorType(CursorDefault);
				// we might add something like
				// if (leader->isDamaged() && unit->canRepair())
				// here
			} else if (leader->unitProperties()->canShoot()) {
				if ((unit->isFlying() && leader->unitProperties()->canShootAtAirUnits()) ||
						(!unit->isFlying() && leader->unitProperties()->canShootAtLandUnits())) {
					setCursorType(CursorAttack);
				}
			}
		} else if (leader->isMobile()) {
			setCursorType(CursorMove);
		} else {
			setCursorType(CursorDefault);
		}
	}
 } else {
	setCursorType(CursorDefault);
 }

 c->setCursor(cursorType());
}

BosonGameViewInputBase::CanSelectUnit BosonGameViewInput::canSelect(Unit* unit) const
{
 if (!unit) {
	return CanSelectError;
 }
 if (unit->isDestroyed()) {
	return CanSelectDestroyed;
 }
 if (localPlayerIO() && !localPlayerIO()->ownsUnit(unit)) {
	// we can select this unit, but only as a single unit.
	return CanSelectSingleOk;
 }
 if (unit->isFacility()) {
	return CanSelectSingleOk;
 }
 return CanSelectMultipleOk;
}

bool BosonGameViewInput::selectAll(const UnitProperties* prop, bool replace)
{
 if (!localPlayerIO()) {
	BO_NULL_ERROR(localPlayerIO());
	return false;
 }
 if (prop->isFacility()) {
	// we don't select all facilities, but only the one that was
	// double-clicked. it makes no sense for facilities
	return false;
 }
 QPtrList<Unit> allUnits = *(localPlayerIO()->allMyUnits());
 QPtrList<Unit> list;
 QPtrListIterator<Unit> it(allUnits);
 while (it.current()) {
	if (it.current()->unitProperties() == prop) {
		if (canSelect(it.current()) == CanSelectMultipleOk) {
			list.append(it.current());
		}
	}
	++it;
 }
 if (list.count() > 0) {
	selectUnits(list, replace);
	return true;
 }
 return false;
}

void BosonGameViewInput::slotMoveSelection(int cellX, int cellY)
{
 BO_CHECK_NULL_RET(localPlayerIO());
 BO_CHECK_NULL_RET(selection());
 if (selection()->isEmpty()) {
	return;
 }
 BoMouseEvent event;
 event.setCanvasVector(BoVector3Fixed((float)(cellX + 1.0f / 2),
		(float)(cellY + 1.0f / 2),
		0.0f));
 actionClicked(event);
}

