/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2002 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include "bosonbigdisplay.h"
#include "bosonbigdisplay.moc"

#include "unit.h"
#include "unitplugins.h"
#include "bosoncanvas.h"
#include "player.h"
#include "unitproperties.h"
#include "pluginproperties.h"
#include "cell.h"
#include "bosonmessage.h"
#include "bosoncursor.h"
#include "bosonconfig.h"
#include "global.h"
#include "kspritetooltip.h"
#include "boselection.h"
#include "bodebug.h"
#include "defines.h"

#include <kgame/kgameio.h>
#include <klocale.h>

#include <qptrlist.h>
#include <qpoint.h>
#include <qcursor.h>

class BosonBigDisplay::BosonBigDisplayPrivate
{
public:
	BosonBigDisplayPrivate()
	{
		mMouseIO = 0;
		mLockAction = false;
		mCursorType = CursorDefault;
	}

	KGameMouseIO* mMouseIO;
	bool mLockAction;
	CursorType mCursorType;
	UnitAction mActionType;
};

BosonBigDisplay::BosonBigDisplay(BosonCanvas* c, QWidget* parent) 
		: BosonBigDisplayBase(c, parent)
{
 init();
}

void BosonBigDisplay::init()
{
 d = new BosonBigDisplayPrivate;

// setSizePolicy(QSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding ));
// setResizePolicy(QScrollView::AutoOne);
}

BosonBigDisplay::~BosonBigDisplay()
{
 delete d;
}

void BosonBigDisplay::actionClicked(const BoAction& action, QDataStream& stream, bool* send)
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

 if (d->mLockAction) {
	switch (d->mActionType) {
		case ActionMove:
		{
			if (actionMove(stream, action.canvasPos())) {
				*send = true;
			}
			break;
		}
		case ActionAttack:
		{
			Unit* unit = canvas()->findUnitAt(action.canvasPos());
			if (unit) {
				if (actionAttack(stream, action.canvasPos())) {
					*send = true;
				}
			}
			break;
		}
		case ActionFollow:
		{
			Unit* unit = canvas()->findUnitAt(action.canvasPos());
			if (unit) {
				if (actionFollow(stream, action.canvasPos())) {
					*send = true;
				}
			}
			break;
		}
		case ActionMine:
		{
			// TODO check if player clicked on oil/minerals
			if (actionMine(stream, action.canvasPos())) {
				*send = true;
			}
			break;
		}
		case ActionRepair:
		{
			if (actionRepair(stream, action.canvasPos())) {
				*send = true;
			}
			break;
		}
		default:
			boError() << k_funcinfo << "Unknown actiontype for locked action: " << d->mActionType << endl;
			break;
	}
	d->mLockAction = false;
	return;
 }

 Unit* unit = 0l;
 if(!localPlayer()->isFogged(action.canvasPos().x() / BO_TILE_SIZE, action.canvasPos().x() / BO_TILE_SIZE)) {
	unit = canvas()->findUnitAt(action.canvasPos());
 }
 if (!unit) {
	//FIXME: first check if a the unit can produce! even mobile units can
	//have the production plugin!!
	if (selection()->hasMobileUnit()) { // move the selection to pos
		if (selection()->count() == 1) {
			// there are special things to do for a single selected unit
			// (e.g. mining if the unit is a harvester)
			if (actionMine(stream, action.canvasPos())) {
				*send = true;
				return;
			}
		}

		if (!actionMove(stream, action.canvasPos())) {
			return;
		}
		*send = true;
	} else { // place constructions
		// FIXME: another option: add the waypoint to the facility and
		// apply it to any unit that gets constructed by that facility.
		// For this we'd probably have to use LMB for unit placing
		if (!actionBuild(stream, action.canvasPos())) {
			return;
		}
		*send = true;
	}
 } else { // there is a unit - attack it?
	if ((localPlayer()->isEnemy(unit->owner()) || action.forceAttack()) &&
			selection()->canShootAt(unit)) {
		// attack the unit
		if (!actionAttack(stream, action.canvasPos())) {
			return;
		}
		*send = true;

	} else if (localPlayer()->isEnemy(unit->owner())) {
		// a non-friendly unit, but the selection cannot shoot
		// we probably won't do anything here
		// IDEA: what about "I cannot shoot that!" sound?
	} else {
		// click on a friendly unit
		if (unit->isFacility() && unit->repairPlugin()) {
			// some kind of repairyard - repair all units
			// (not yet implemented) //FIXME
			// note that currently the unit can go to every friendly
			// player, even non-local players
			if (!actionRepair(stream, action.canvasPos())) {
				return;
			}
			*send = true;
		} else if ((unit->unitProperties()->canRefineMinerals() &&
				selection()->hasMineralHarvester()) ||
				(unit->unitProperties()->canRefineOil() &&
				selection()->hasOilHarvester())) {
			// go to the refinery
			if (!actionRefine(stream, action.canvasPos())) {
				return;
			}
			*send = true;
		} else if(selection()->hasMobileUnit() && unit->isMobile()) {
			// Follow
			if (!actionFollow(stream, action.canvasPos())) {
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

bool BosonBigDisplay::actionMine(QDataStream& stream, const QPoint& canvasPos)
{
 MobileUnit* u = (MobileUnit*)selection()->leader();
 HarvesterPlugin* h = (HarvesterPlugin*)u->plugin(UnitPlugin::Harvester);
 if (!h) {
	return false;
 }
 if (h->canMine(((BosonCanvas*)canvas())->cellAt(canvasPos.x(), canvasPos.y()))) {
	stream << (Q_UINT32)BosonMessage::MoveMine;
	stream << (Q_ULONG)u->id();
	stream << canvasPos;
	return true;
 }
 return false;
}

bool BosonBigDisplay::actionMove(QDataStream& stream, const QPoint& canvasPos)
{
 QPtrList<Unit> list = selection()->allUnits();
 QPtrListIterator<Unit> it(list);
 // tell the clients we want to move units:
 stream << (Q_UINT32)BosonMessage::MoveMove;
 // tell them where to move to:
 stream << canvasPos;
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

bool BosonBigDisplay::actionBuild(QDataStream& stream, const QPoint& canvasPos)
{
 Unit* factory = selection()->leader();
 if (!factory) {
	return false;
 }
 ProductionPlugin* production = (ProductionPlugin*)(factory->plugin(UnitPlugin::Production));
 if (!production || !production->hasProduction() || production->completedProductionId() <= 0) {
	return false;
 }

 if (!(canvas())->canPlaceUnitAt(localPlayer()->unitProperties(production->currentProductionId()), canvasPos, production)) {
	boDebug() << k_funcinfo << "Cannot place production here" << endl;
	return false;
 }

 // create the new unit
 stream << (Q_UINT32)BosonMessage::MoveBuild;
 stream << (Q_UINT32)production->completedProductionType();
 stream << (Q_ULONG)factory->id();
 stream << (Q_UINT32)factory->owner()->id();
 stream << (Q_INT32)canvasPos.x() / BO_TILE_SIZE;
 stream << (Q_INT32)canvasPos.y() / BO_TILE_SIZE;
 return true;
}

bool BosonBigDisplay::actionAttack(QDataStream& stream, const QPoint& canvasPos)
{
 Unit* unit = canvas()->findUnitAt(canvasPos);
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

bool BosonBigDisplay::actionRepair(QDataStream& stream, const QPoint& canvasPos)
{
 Unit* unit = canvas()->findUnitAt(canvasPos);
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

bool BosonBigDisplay::actionRefine(QDataStream& stream, const QPoint& canvasPos)
{
 Unit* unit = canvas()->findUnitAt(canvasPos);
// if (!unit->properites(PluginProperties::Refine)) {
//	boError() << k_funcinfo << unit->id() << "cannot refine" << endl;
//	return;
// }
// bool minerals = (RefineProperties*)unit)->properties(PluginProperties::Refine)->canRefineMinerals();
 bool minerals = unit->unitProperties()->canRefineMinerals();
 QPtrList<Unit> allUnits = selection()->allUnits();
 QPtrList<Unit> list;
 QPtrListIterator<Unit> unitsIt(allUnits);
 while (unitsIt.current()) {
	HarvesterProperties* prop = (HarvesterProperties*)unitsIt.current()->properties(PluginProperties::Harvester);
	if (prop) {
		if (prop->canMineMinerals() && minerals) {
			list.append(unitsIt.current());
		} else if (prop->canMineOil() && !minerals) {
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

bool BosonBigDisplay::actionFollow(QDataStream& stream, const QPoint& canvasPos)
{
 Unit* unit = canvas()->findUnitAt(canvasPos);
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

/*
void BosonBigDisplay::setLocalPlayer(Player* p)
{
 if (localPlayer() == p) {
	return;
 }

 if (localPlayer()) {
	//AB: in theory the IO gets removed from the players' IO list. if we
	//ever use this, then test it!
	delete d->mMouseIO;
	d->mMouseIO = 0;
 }

 BosonBigDisplayBase::setLocalPlayer(p);

 if (p) {
	addMouseIO(localPlayer());
//	d->mChat->setFromPlayer(localPlayer());
 }
}
 */

void BosonBigDisplay::slotMoveSelection(int cellX, int cellY)
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
 action.setCanvasPos(QPoint(cellX * BO_TILE_SIZE + BO_TILE_SIZE / 2, cellY * BO_TILE_SIZE + BO_TILE_SIZE / 2));
 actionClicked(action, stream, &send);
 if (send) {
	QDataStream msg(buffer, IO_ReadOnly);
	localPlayer()->forwardInput(msg, true);
 }
}

void BosonBigDisplay::updateCursor()
{
 BosonCursor* c = cursor();
 if (!c) {
	boError() << k_funcinfo << "NULL cursor!!" << endl;
	return;
 }

 QPoint widgetPos = mapFromGlobal(QCursor::pos());
 QPoint canvasPos;
 GLdouble x, y, z;
 mapCoordinates(widgetPos, &x, &y, &z);
 worldToCanvas(x, y, z, &canvasPos);

 if (!canvas()->onCanvas(canvasPos)) {
	d->mCursorType = CursorDefault;
	c->setCursor(d->mCursorType);
	c->setWidgetCursor(this);
	return;
 }

 if (!d->mLockAction) {
	if (!selection()->isEmpty()) {
		if (selection()->leader()->owner() == localPlayer()) {
			Unit* unit = ((BosonCanvas*)canvas())->findUnitAt(canvasPos);
			if (unit && (!selection()->leader()->owner()->isFogged(canvasPos.x() / BO_TILE_SIZE, canvasPos.y() / BO_TILE_SIZE))) {
				if (unit->owner() == localPlayer()) {
					d->mCursorType = CursorDefault;
				} else if(selection()->leader()->unitProperties()->canShoot()) {
					if((unit->isFlying() && selection()->leader()->unitProperties()->canShootAtAirUnits()) ||
							(!unit->isFlying() && selection()->leader()->unitProperties()->canShootAtLandUnits())) {
						d->mCursorType = CursorAttack;
					}
				}
			} else if (selection()->leader()->isMobile()) {
				d->mCursorType = CursorMove;
			} else {
				d->mCursorType = CursorDefault;
			}
		} else {
			d->mCursorType = CursorDefault;
		}
	} else {
		d->mCursorType = CursorDefault;
	}
 }

 c->setCursor(d->mCursorType);
 c->setWidgetCursor(this);
}

/*
void BosonBigDisplay::addMouseIO(Player* p)
{
//AB: make sure that both editor and game mode can share the same IO !
 if (!localPlayer()) {
	boError() << k_funcinfo << "NULL player" << endl;
	return;
 }
 if (d->mMouseIO) {
	boError() << "This view already has a mouse io!!" << endl;
	return;
 }
 d->mMouseIO = new KGameMouseIO(viewport(), true);
 connect(d->mMouseIO, SIGNAL(signalMouseEvent(KGameIO*, QDataStream&, 
		QMouseEvent*, bool*)),
		this, SLOT(slotMouseEvent(KGameIO*, QDataStream&, QMouseEvent*,
		bool*)));
 localPlayer()->addGameIO(d->mMouseIO);
}
*/

void BosonBigDisplay::unitAction(int actionType)
{
 switch ((UnitAction)actionType) {
	case ActionFollow:
	case ActionMine:
	case ActionRepair:
	case ActionMove:
		d->mCursorType = CursorMove;
		break;
	case ActionAttack:
		d->mCursorType = CursorAttack;
		break;
	case ActionStop:
	{
		if(selection()->isEmpty()) {
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
		d->mLockAction = false;
		return;
	}
	default:
		boError() << k_funcinfo << "Unknown actionType: " << actionType << endl;
		return;
 }
 d->mActionType = (UnitAction)actionType;
 d->mLockAction = true;
}

bool BosonBigDisplay::actionLocked() const
{
 return d->mLockAction;
}

