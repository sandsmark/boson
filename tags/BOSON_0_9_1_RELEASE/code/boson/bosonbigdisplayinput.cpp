/*
    This file is part of the Boson game
    Copyright (C) 2002-2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bosonbigdisplayinput.h"
#include "bosonbigdisplayinput.moc"

#include "bosonbigdisplaybase.h"

#include "boselection.h"
#include "bosoncanvas.h"
#include "bosonconfig.h"
#include "bosonmessage.h"
#include "boson.h"
#include "bosoncursor.h"
#include "player.h"
#include "unitproperties.h"
#include "pluginproperties.h"
#include "unit.h"
#include "unitplugins.h"
#include "bodebug.h"
#include "boaction.h"
#include "bosonlocalplayerinput.h"
#include "bosonweapon.h"

#include <klocale.h>

BosonBigDisplayInput::BosonBigDisplayInput(BosonBigDisplayBase* parent) : BosonBigDisplayInputBase(parent)
{
 weaponId = -1;
}

BosonBigDisplayInput::~BosonBigDisplayInput()
{
}

void BosonBigDisplayInput::actionClicked(const BoMouseEvent& event)
{
 boDebug() << k_funcinfo << endl;
// this method should not perform any tasks but rather send the input through
// the KGameIO. this way it is very easy (it should be at least) to write a
// computer player
 BO_CHECK_NULL_RET(selection());
 BO_CHECK_NULL_RET(canvas());
 BO_CHECK_NULL_RET(localPlayer());
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
 if (!localPlayer()->isFogged((int)(event.canvasVector().x() / BO_TILE_SIZE), (int)(event.canvasVector().y() / BO_TILE_SIZE))) {
	unit = canvas()->findUnitAt(event.canvasVector());
 }
 if (!unit) {
	//FIXME: first check if a the unit can produce! even mobile units can
	//have the production plugin!!
	if (selection()->hasMobileUnit()) { // move the selection to pos
		if (selection()->count() == 1) {
			// there are special things to do for a single selected unit
			// (e.g. mining if the unit is a harvester)
			if (actionHarvest(event.canvasVector())) {
				return;
			}
		}

		if (boConfig->RMBMovesWithAttack()) {
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
	if ((localPlayer()->isEnemy(unit->owner()) || event.forceAttack()) &&
			selection()->canShootAt(unit)) {
		// attack the unit
		if (!actionAttack(event.canvasVector())) {
			return;
		}

	} else if (localPlayer()->isEnemy(unit->owner())) {
		// a non-friendly unit, but the selection cannot shoot
		// we probably won't do anything here
		// IDEA: what about "I cannot shoot that!" sound?
	} else {
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
	}
 }

}


bool BosonBigDisplayInput::actionHarvest(const BoVector3& canvasVector)
{
 if (!canvas()) {
	BO_NULL_ERROR(canvas());
	return false;
 }
 if (!selection()) {
	BO_NULL_ERROR(selection());
	return false;
 }
 MobileUnit* u = (MobileUnit*)selection()->leader();
 HarvesterPlugin* h = (HarvesterPlugin*)u->plugin(UnitPlugin::Harvester);
 if (!h) {
	return false;
 }
 if (h->canMine((canvas())->cellAt(canvasVector.x(), canvasVector.y()))) {
	localPlayerInput()->harvest(u, (int)canvasVector.x(), (int)canvasVector.y());
	return true;
 }
 return false;
}

bool BosonBigDisplayInput::actionMoveWithoutAttack(const BoVector3& canvasVector)
{
 if (!selection()) {
	BO_NULL_ERROR(selection());
	return false;
 }
 if (!localPlayer()) {
	BO_NULL_ERROR(localPlayer());
	return false;
 }
 // AB: note that only x and y are relevant from canvasVector !
 // z is ignored
 localPlayerInput()->moveWithoutAttack(selection()->allUnits(), (int)canvasVector.x(), (int)canvasVector.y());
 if (selection()->leader()->owner() == localPlayer()) {
	selection()->leader()->playSound(SoundOrderMove);
 }
 return true;
}

bool BosonBigDisplayInput::actionMoveWithAttack(const BoVector3& canvasVector)
{
 if (!localPlayer()) {
	BO_NULL_ERROR(localPlayer());
	return false;
 }
 if (!selection()) {
	BO_NULL_ERROR(selection());
	return false;
 }
 // AB: note that only x and y are relevant from canvasVector !
 // z is ignored
 localPlayerInput()->moveWithAttack(selection()->allUnits(), (int)canvasVector.x(), (int)canvasVector.y());
 if (selection()->leader()->owner() == localPlayer()) {
	selection()->leader()->playSound(SoundOrderAttack);
 }
 return true;
}

bool BosonBigDisplayInput::actionBuild(const BoVector3& canvasVector)
{
 if (!localPlayer()) {
	BO_NULL_ERROR(localPlayer());
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
 // FIXME: lot of this code should probably be moved to BosonLocalPlayerInput
 ProductionPlugin* production = (ProductionPlugin*)(factory->plugin(UnitPlugin::Production));
 if (!production || !production->hasProduction() || production->completedProductionId() <= 0) {
	return false;
 }

 const UnitProperties* prop = localPlayer()->unitProperties(production->currentProductionId());
 if (!prop) {
	boError() << k_funcinfo << "NULL unit properties" << endl;
	return false;
 }
 if (!canvas()->canPlaceUnitAt(prop, QPoint((int)canvasVector.x(), (int)canvasVector.y()), production)) {
	boDebug() << k_funcinfo << "Cannot place production here" << endl;
	boGame->slotAddChatSystemMessage(i18n("You can't place a %1 there").arg(prop->name()));
	return false;
 }

 // create the new unit
 localPlayerInput()->build(production->completedProductionType(), factory,
		(int)(canvasVector.x() / BO_TILE_SIZE), (int)(canvasVector.y() / BO_TILE_SIZE));
 return true;
}

bool BosonBigDisplayInput::actionAttack(const BoVector3& canvasVector)
{
 if (!localPlayer()) {
	BO_NULL_ERROR(localPlayer());
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
 if (u->owner() == localPlayer()) {
	u->playSound(SoundOrderAttack);
 }
 return true;
}

bool BosonBigDisplayInput::actionDropBomb(const BoVector3& canvasVector)
{
 if (!localPlayer()) {
	BO_NULL_ERROR(localPlayer());
	return false;
 }
 if (!selection()) {
	BO_NULL_ERROR(selection());
	return false;
 }
 // AB: note that only x and y are relevant from canvasVector !
 // z is ignored
 localPlayerInput()->dropBomb(selection()->leader(), weaponId, (int)canvasVector.x(), (int)canvasVector.y());
 weaponId = -1;
 if (selection()->leader()->owner() == localPlayer()) {
	selection()->leader()->playSound(SoundOrderAttack);
 }
 return true;
}

bool BosonBigDisplayInput::actionRepair(const BoVector3& canvasVector)
{
 if (!selection()) {
	BO_NULL_ERROR(selection());
	return false;
 }
 if (!canvas()) {
	BO_NULL_ERROR(canvas());
	return false;
 }
 Unit* unit = canvas()->findUnitAt(canvasVector);
 QPtrList<Unit> allUnits = selection()->allUnits();
 QPtrList<Unit> list;
 QPtrListIterator<Unit> it(allUnits);
 while (it.current()) {
	if (it.current()->health() < it.current()->unitProperties()->health()) {
		boDebug() << "repair " << it.current()->id() << endl;
		list.append(it.current());
	}
	++it;
 }
 localPlayerInput()->repair(list, unit);
// TODO:
// Unit* u = selection()->leader();
// u->playSound(SoundOrderRepair);
 return true;
}

bool BosonBigDisplayInput::actionRefine(const BoVector3& canvasVector)
{
 if (!selection()) {
	BO_NULL_ERROR(selection());
	return false;
 }
 if (!canvas()) {
	BO_NULL_ERROR(canvas());
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
// TODO:
// Unit* u = selection()->leader();
// u->playSound(SoundOrderRefine);
 return true;
}

bool BosonBigDisplayInput::actionFollow(const BoVector3& canvasVector)
{
 if (!localPlayer()) {
	BO_NULL_ERROR(localPlayer());
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
 localPlayerInput()->follow(selection()->allUnits(), unit);
 Unit* u = selection()->leader();
 if (u->owner() == localPlayer()) {
	u->playSound(SoundOrderMove);
 }
 return true;
}

void BosonBigDisplayInput::updatePlacementPreviewData()
{
 BO_CHECK_NULL_RET(localPlayer());
 BO_CHECK_NULL_RET(selection());
 BO_CHECK_NULL_RET(canvas());
 bigDisplay()->setPlacementPreviewData(0, false);
 if (!actionLocked() || actionType() != ActionPlacementPreview) {
	return;
 }
 if (!selection() || selection()->isEmpty() || !selection()->leader()) {
	return;
 }
 if (!localPlayer()) {
	boError() << k_funcinfo << "NULL local player" << endl;
	return;
 }
 Unit* leader = selection()->leader();
 if (leader->owner() != localPlayer()) {
	return;
 }
 ProductionPlugin* pp = (ProductionPlugin*)leader->plugin(UnitPlugin::Production);
 if (!pp || pp->completedProductionId() == 0 || pp->completedProductionType() != ProduceUnit) {
	return;
 }
 const UnitProperties* prop = localPlayer()->unitProperties(pp->completedProductionId());
 if (!prop) {
	return;
 }
 // note: this applies to mobiles as well as for facilities!
 // (mobiles are usually auto placed, but manual placement might get used if
 // auto-placement failed)
 bigDisplay()->setPlacementPreviewData(prop, canvas()->canPlaceUnitAt(prop, cursorCanvasPos(), pp));

}

void BosonBigDisplayInput::action(const BoSpecificAction& action)
{
 boDebug() << k_funcinfo << endl;
 BO_CHECK_NULL_RET(localPlayer());
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

void BosonBigDisplayInput::updateCursor()
{
 BosonCursor* c = bigDisplay()->cursor();
 if (!c) {
	boError() << k_funcinfo << "NULL cursor!!" << endl;
	return;
 }
 BO_CHECK_NULL_RET(localPlayer());
 BO_CHECK_NULL_RET(selection());
 BO_CHECK_NULL_RET(canvas());

 if (!canvas()->onCanvas(cursorCanvasVector())) {
	if (actionLocked()) {  // TODO: show "can't do that" cursor if action is locked
	} else {
		setCursorType(CursorDefault);
		c->setCursor(cursorType());
		c->setWidgetCursor(bigDisplay());
		return;
	}
 }
 if (actionLocked()) {
	c->setCursor(cursorType());
	c->setWidgetCursor(bigDisplay());
	return;
 }

 if (!selection()->isEmpty() && selection()->leader()->owner() == localPlayer()) {
	Unit* leader = selection()->leader();
	if (localPlayer()->isFogged(cursorCanvasPos().x() / BO_TILE_SIZE, cursorCanvasPos().y() / BO_TILE_SIZE)) {
		if (leader->isMobile()) {
			setCursorType(CursorMove);
		} else {
			setCursorType(CursorDefault);
		}
	} else {
		Unit* unit = canvas()->findUnitAt(cursorCanvasVector());
		if (unit) {
			if (unit->owner() == localPlayer()) {
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
 c->setWidgetCursor(bigDisplay());
}

BosonBigDisplayInputBase::CanSelectUnit BosonBigDisplayInput::canSelect(Unit* unit) const
{
 if (!unit) {
	return CanSelectError;
 }
 if (unit->isDestroyed()) {
	return CanSelectDestroyed;
 }
 if (unit->owner() != localPlayer()) {
	// we can select this unit, but only as a single unit.
	return CanSelectSingleOk;
 }
 if (unit->isFacility()) {
	return CanSelectSingleOk;
 }
 return CanSelectMultipleOk;
}

bool BosonBigDisplayInput::selectAll(const UnitProperties* prop, bool replace)
{
 if (!localPlayer()) {
	BO_NULL_ERROR(localPlayer());
	return false;
 }
 if (prop->isFacility()) {
	// we don't select all facilities, but only the one that was
	// double-clicked. it makes no sense for facilities
	return false;
 }
 QPtrList<Unit> allUnits = *(localPlayer()->allUnits());
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

void BosonBigDisplayInput::slotMoveSelection(int cellX, int cellY)
{
 BO_CHECK_NULL_RET(localPlayer());
 BO_CHECK_NULL_RET(selection());
 if (selection()->isEmpty()) {
	return;
 }
 BoMouseEvent event;
 event.setCanvasVector(BoVector3((float)(cellX * BO_TILE_SIZE + BO_TILE_SIZE / 2),
		(float)(cellY * BO_TILE_SIZE + BO_TILE_SIZE / 2),
		0.0f));
 actionClicked(event);
}

