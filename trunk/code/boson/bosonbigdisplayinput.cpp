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

#include <klocale.h>

BosonBigDisplayInput::BosonBigDisplayInput(BosonBigDisplayBase* parent) : BosonBigDisplayInputBase(parent)
{
}

BosonBigDisplayInput::~BosonBigDisplayInput()
{
}

void BosonBigDisplayInput::actionClicked(const BoAction& action, QDataStream& stream, bool* send)
{
// this method should not perform any tasks but rather send the input through
// the KGameIO. this way it is very easy (it should be at least) to write a
// computer player
 if (!selection()) {
	boError() << k_funcinfo << "NULL selection" << endl;
	return;
 }
 if (selection()->isEmpty()) {
	return;
 }
 if (!canvas()->onCanvas(action.canvasVector())) {
	boError() << k_funcinfo << action.canvasVector().x() << "," << action.canvasVector().y() << " is not on the canvas!" << endl;
	return;
 }

 if (actionLocked()) {
	switch (actionType()) {
		case ActionMove:
		{
			if (actionMove(stream, action.canvasVector())) {
				*send = true;
			}
			break;
		}
		case ActionAttack:
		{
			Unit* unit = canvas()->findUnitAt(action.canvasVector());
			if (unit) {
				if (actionAttack(stream, action.canvasVector())) {
					*send = true;
				}
			} else {
				actionAttackPos(stream, action.canvasVector());
				*send = true;
			}
			break;
		}
		case ActionFollow:
		{
			Unit* unit = canvas()->findUnitAt(action.canvasVector());
			if (unit) {
				if (actionFollow(stream, action.canvasVector())) {
					*send = true;
				}
			}
			break;
		}
		case ActionMine:
		{
			// TODO check if player clicked on oil/minerals
			if (actionMine(stream, action.canvasVector())) {
				*send = true;
			}
			break;
		}
		case ActionRepair:
		{
			if (actionRepair(stream, action.canvasVector())) {
				*send = true;
			}
			break;
		}
		case ActionBuild:
		{
			if (actionBuild(stream, action.canvasVector())) {
				*send = true;
			}
			break;
		}
		default:
			boError() << k_funcinfo << "Unknown actiontype for locked action: " << actionType() << endl;
			break;
	}
	if (*send) {
		// TODO: also play "cannot do that" sound
		unlockAction();
	}
	return;
 }

 Unit* unit = 0l;
 if (!localPlayer()->isFogged((int)(action.canvasVector().x() / BO_TILE_SIZE), (int)(action.canvasVector().y() / BO_TILE_SIZE))) {
	unit = canvas()->findUnitAt(action.canvasVector());
 }
 if (!unit) {
	//FIXME: first check if a the unit can produce! even mobile units can
	//have the production plugin!!
	if (selection()->hasMobileUnit()) { // move the selection to pos
		if (selection()->count() == 1) {
			// there are special things to do for a single selected unit
			// (e.g. mining if the unit is a harvester)
			if (actionMine(stream, action.canvasVector())) {
				*send = true;
				return;
			}
		}

		if (boConfig->RMBAction() == ActionAttack) {
			if (!actionAttackPos(stream, action.canvasVector())) {
				return;
			}
		} else {
			if (!actionMove(stream, action.canvasVector())) {
				return;
			}
		}
		*send = true;
	} else {
		// TODO: another option: add the waypoint to the facility and
		// apply it to any unit that gets constructed by that facility.
		// For this we'd probably have to use LMB for unit placing
		// AB: the facility-placement gets done for actionLocked()==true
		// now ! (ActionBuild)
	}
 } else { // there is a unit - attack it?
	if ((localPlayer()->isEnemy(unit->owner()) || action.forceAttack()) &&
			selection()->canShootAt(unit)) {
		// attack the unit
		if (!actionAttack(stream, action.canvasVector())) {
			return;
		}
		*send = true;

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
			if (!actionRepair(stream, action.canvasVector())) {
				return;
			}
			*send = true;
		} else if (unit->properties(PluginProperties::Refinery)) {
			const RefineryProperties* prop = (RefineryProperties*)unit->properties(PluginProperties::Refinery);
			if((prop->canRefineMinerals() && selection()->hasMineralHarvester()) ||
					(prop->canRefineOil() && selection()->hasOilHarvester())) {
				// go to the refinery
				if (!actionRefine(stream, action.canvasVector())) {
					return;
				}
				*send = true;
			}
		} else if (selection()->hasMobileUnit() && unit->isMobile()) {
			// Follow
			if (!actionFollow(stream, action.canvasVector())) {
				return;
			}
			*send = true;
		} else {
			// selection and clicked unit both are friendly
			// no repairyard and no refinery
			// (at least no valid)
			// add other possibilities here
		}
	}
 }

}


bool BosonBigDisplayInput::actionMine(QDataStream& stream, const BoVector3& canvasVector)
{
 MobileUnit* u = (MobileUnit*)selection()->leader();
 HarvesterPlugin* h = (HarvesterPlugin*)u->plugin(UnitPlugin::Harvester);
 if (!h) {
	return false;
 }
 if (h->canMine((canvas())->cellAt(canvasVector.x(), canvasVector.y()))) {
	stream << (Q_UINT32)BosonMessage::MoveMine;
	stream << (Q_ULONG)u->id();
	stream << QPoint((int)canvasVector.x(), (int)canvasVector.y());
	return true;
 }
 return false;
}

bool BosonBigDisplayInput::actionMove(QDataStream& stream, const BoVector3& canvasVector)
{
 // AB: note that only x and y are relevant from canvasVector !
 // z is ignored
 QPtrList<Unit> list = selection()->allUnits();
 QPtrListIterator<Unit> it(list);
 // tell the clients we want to move units:
 stream << (Q_UINT32)BosonMessage::MoveMove;
 // We want to move without attacking
 stream << (Q_UINT8)0;
 // tell them where to move to:
 stream << QPoint((int)canvasVector.x(), (int)canvasVector.y());
 // tell them how many units:
 stream << (Q_UINT32)list.count();
 Unit* unit = 0;
 while (it.current()) {
	if (!unit) {
		unit = it.current();
	}
	// tell them which unit to move:
	stream << (Q_ULONG)it.current()->id(); // MUST BE UNIQUE!
	++it;
 }
 if (unit->owner() == localPlayer()) {
	unit->playSound(SoundOrderMove);
 }
 return true;
}

bool BosonBigDisplayInput::actionBuild(QDataStream& stream, const BoVector3& canvasVector)
{
 Unit* factory = selection()->leader();
 if (!factory) {
	return false;
 }
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
	boGame->slotAddChatSystemMessage(i18n("You can't place a %1 there").arg(prop->name()), localPlayer());
	return false;
 }

 // create the new unit
 stream << (Q_UINT32)BosonMessage::MoveBuild;
 stream << (Q_UINT32)production->completedProductionType();
 stream << (Q_ULONG)factory->id();
 stream << (Q_UINT32)factory->owner()->id();
 stream << (Q_INT32)(canvasVector.x() / BO_TILE_SIZE);
 stream << (Q_INT32)(canvasVector.y() / BO_TILE_SIZE);
 return true;
}

bool BosonBigDisplayInput::actionAttack(QDataStream& stream, const BoVector3& canvasVector)
{
 Unit* unit = canvas()->findUnitAt(canvasVector);
 QPtrList<Unit> list = selection()->allUnits();
 QPtrListIterator<Unit> it(list);
 // tell the clients we want to attack:
 stream << (Q_UINT32)BosonMessage::MoveAttack;
 // tell them which unit to attack:
 stream << (Q_ULONG)unit->id();
 // tell them how many units attack:
 stream << (Q_UINT32)list.count();
 while (it.current()) {
	// tell them which unit is going to attack:
	stream << (Q_ULONG)it.current()->id(); // MUST BE UNIQUE!
	++it;
 }
 Unit* u = selection()->leader();
 if (u->owner() == localPlayer()) {
	u->playSound(SoundOrderAttack);
 }
 return true;
}

bool BosonBigDisplayInput::actionAttackPos(QDataStream& stream, const BoVector3& canvasVector)
{
 // AB: note that only x and y are relevant from canvasVector !
 // z is ignored
 QPtrList<Unit> list = selection()->allUnits();
 QPtrListIterator<Unit> it(list);
 // tell the clients we want to move units:
 stream << (Q_UINT32)BosonMessage::MoveMove;
 // We want to move with attacking
 stream << (Q_UINT8)1;
 // tell them where to move to:
 stream << QPoint((int)canvasVector.x(), (int)canvasVector.y());
 // tell them how many units:
 stream << (Q_UINT32)list.count();
 Unit* unit = 0;
 while (it.current()) {
	if (!unit) {
		unit = it.current();
	}
	// tell them which unit to move:
	stream << (Q_ULONG)it.current()->id(); // MUST BE UNIQUE!
	++it;
 }
 if (unit->owner() == localPlayer()) {
	unit->playSound(SoundOrderAttack);
 }
 return true;
}

bool BosonBigDisplayInput::actionRepair(QDataStream& stream, const BoVector3& canvasVector)
{
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
 it = QPtrListIterator<Unit>(list);
 // tell the clients we want to repair:
 stream << (Q_UINT32)BosonMessage::MoveRepair;
 // the owner of the repairyard (can also be an allied
 // player - not localplayer only)
 stream << (Q_UINT32)unit->owner()->id();
 // tell them where to repair the units:
 stream << (Q_ULONG)unit->id();
 // tell them how many units to be repaired:
 stream << (Q_UINT32)list.count();
 while (it.current()) {
	// tell them which unit is going be repaired:
	stream << (Q_ULONG)it.current()->id();
	++it;
 }
// TODO:
// Unit* u = selection()->leader();
// u->playSound(SoundOrderRepair);
 return true;
}

bool BosonBigDisplayInput::actionRefine(QDataStream& stream, const BoVector3& canvasVector)
{
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
 QPtrListIterator<Unit> it(list);
 stream << (Q_UINT32)BosonMessage::MoveRefine;
 // the owner of the refinery (can also be an allied
 // player - not localplayer only)
 stream << (Q_UINT32)unit->owner()->id();
 // destination:
 stream << (Q_ULONG)unit->id();
 // how many units go to the refinery
 stream << (Q_UINT32)list.count();
 while (it.current()) {
	// tell them which unit goes there
	stream << (Q_ULONG)it.current()->id();
	++it;
 }
// TODO:
// Unit* u = selection()->leader();
// u->playSound(SoundOrderRefine);
 return true;
}

bool BosonBigDisplayInput::actionFollow(QDataStream& stream, const BoVector3& canvasVector)
{
 Unit* unit = canvas()->findUnitAt(canvasVector);
 QPtrList<Unit> list = selection()->allUnits();
 QPtrListIterator<Unit> it(list);
 // tell the clients we want to follow:
 stream << (Q_UINT32)BosonMessage::MoveFollow;
 // tell them which unit to follow:
 stream << (Q_ULONG)unit->id();
 // tell them how many units follow:
 stream << (Q_UINT32)list.count();
 while (it.current()) {
	// tell them which unit is going to follow:
	stream << (Q_ULONG)it.current()->id(); // MUST BE UNIQUE!
	++it;
 }
 Unit* u = selection()->leader();
 if (u->owner() == localPlayer()) {
	u->playSound(SoundOrderMove);
 }
 return true;
}

void BosonBigDisplayInput::updatePlacementPreviewData()
{
 bigDisplay()->setPlacementPreviewData(0, false);
 if (!actionLocked() || actionType() != ActionBuild) {
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

void BosonBigDisplayInput::unitAction(int actionType)
{
 switch ((UnitAction)actionType) {
	case ActionFollow:
	case ActionMine:
	case ActionRepair:
	case ActionMove:
		setCursorType(CursorMove);
		break;
	case ActionBuild:
		break;
	case ActionAttack:
		setCursorType(CursorAttack);
		break;
	case ActionStop:
	{
		if (selection()->isEmpty()) {
			boError() << k_funcinfo << "Selection is empty!" << endl;
			return;
		}
		// Stop all selected units
		// I REALLY hope I'm doing this correctly
		// TODO: should be in actionStop()
		QPtrList<Unit> list = selection()->allUnits();
		QPtrListIterator<Unit> it(list);
		QByteArray b;
		QDataStream stream(b, IO_WriteOnly);

		// tell the clients we want to move units:
		stream << (Q_UINT32)BosonMessage::MoveStop;
		// tell them how many units:
		stream << (Q_UINT32)list.count();
		while (it.current()) {
			// tell them which unit to move:
			stream << (Q_ULONG)it.current()->id(); // MUST BE UNIQUE!
			++it;
		}

		QDataStream msg(b, IO_WriteOnly);
		localPlayer()->forwardInput(msg);
		unlockAction();
		return;
	}
	default:
		boError() << k_funcinfo << "Unknown actionType: " << actionType << endl;
		return;
 }
 setActionType((UnitAction)actionType);
 lockAction();
}

void BosonBigDisplayInput::updateCursor()
{
 BosonCursor* c = bigDisplay()->cursor();
 if (!c) {
	boError() << k_funcinfo << "NULL cursor!!" << endl;
	return;
 }
 if (!localPlayer()) {
	boError() << k_funcinfo << "NULL local player" << endl;
	return;
 }

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
	boError() << k_funcinfo << "NULL localplayer" << endl;
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
 if (!localPlayer()) {
	boError() << "NULL local player" << endl;
	return;
 }
 if (!selection()) {
	boError() << k_funcinfo << "NULL selection" << endl;
	return;
 }
 if (selection()->isEmpty()) {
	return;
 }
 QByteArray buffer;
 QDataStream stream(buffer, IO_WriteOnly);
 bool send = false;
 BoAction action;
 action.setCanvasVector(BoVector3((float)(cellX * BO_TILE_SIZE + BO_TILE_SIZE / 2),
		(float)(cellY * BO_TILE_SIZE + BO_TILE_SIZE / 2),
		0.0f));
 actionClicked(action, stream, &send);
 if (send) {
	QDataStream msg(buffer, IO_ReadOnly);
	localPlayer()->forwardInput(msg, true);
 }
}

