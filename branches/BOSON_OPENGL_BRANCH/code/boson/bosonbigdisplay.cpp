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
#include "cell.h"
#include "bosonmessage.h"
#include "kgamecanvaschat.h"
#include "bosoncursor.h"
#include "bosonmusic.h"
#include "bosonconfig.h"
#include "global.h"
#include "kspritetooltip.h"
#include "boselection.h"
#include "defines.h"

#include <kgame/kgameio.h>
#include <kdebug.h>
#include <klocale.h>

#include <qptrlist.h>
#include <qpoint.h>
#include <qpainter.h>
#include <qbitmap.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qcursor.h>

class BosonBigDisplay::BosonBigDisplayPrivate
{
public:
	BosonBigDisplayPrivate()
	{
		mMouseIO = 0;
		
//		mChat = 0;
	}

	KGameMouseIO* mMouseIO;

//	KGameCanvasChat* mChat;
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
	kdError() << k_funcinfo << "NULL selection" << endl;
	return;
 }
 if (selection()->isEmpty()) {
	return;
 }
 Unit* unit = ((BosonCanvas*)canvas())->findUnitAt(action.canvasPos());
 if (!unit) {
	if (selection()->hasMobileUnit()) { // move the selection to pos
		if (selection()->count() == 1) {
			// there are special things to do for a single selected unit
			// (e.g. mining if the unit is a harvester)
			MobileUnit* u = (MobileUnit*)selection()->leader();
			if (u->canMine(((BosonCanvas*)canvas())->cellAt(action.canvasPos().x(), action.canvasPos().y()))) {
				stream << (Q_UINT32)BosonMessage::MoveMine;
				stream << (Q_ULONG)u->id();
				stream << action.canvasPos();
				*send = true;
				return;
			}
		}

		QPtrList<Unit> list = selection()->allUnits();
		QPtrListIterator<Unit> it(list);
		// tell the clients we want to move units:
		stream << (Q_UINT32)BosonMessage::MoveMove;
		// tell them where to move to:
		stream << action.canvasPos();
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
			boMusic->playSound(unit, Unit::SoundOrderMove);
		}
		*send = true;
	} else { // place constructions
		// FIXME: another option: add the waypoint to the facility and
		// apply it to any unit that gets constructed by that facility.
		// For this we'd probably have to use LMB for unit placing
		Facility* fac = (Facility*)selection()->leader();
		ProductionPlugin* production = fac->productionPlugin();
		if (!production || !production->hasProduction() || production->completedProduction() < 0) {
			return;
		}
		
//		if (!fac->canPlaceProductionAt(action.pos())) { // obsolete
		if (!((BosonCanvas*)canvas())->canPlaceUnitAt(localPlayer()->unitProperties(production->currentProduction()), action.canvasPos(), fac)) {
			kdDebug() << k_funcinfo << "Cannot place production here" << endl;
			return;
		}

		// create the new unit
		stream << (Q_UINT32)BosonMessage::MoveBuild;
		stream << (Q_ULONG)fac->id();
		stream << (Q_UINT32)fac->owner()->id();
		stream << (Q_INT32)action.canvasPos().x() / BO_TILE_SIZE;
		stream << (Q_INT32)action.canvasPos().y() / BO_TILE_SIZE;
		*send = true;
	}
 } else { // there is a unit - attack it?
	if ((localPlayer()->isEnemy(unit->owner()) || action.forceAttack()) &&
			selection()->canShootAt(unit)) {
		// attack the unit
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
		*send = true;
		Unit* u = selection()->leader();
		if (u->owner() == localPlayer()) {
			boMusic->playSound(u, Unit::SoundOrderAttack);
		}

	} else if (localPlayer()->isEnemy(unit->owner())) {
		// a non-friendly unit, but the selection cannot shoot
		// we probably won't do anything here
	} else {
		// click on a friendly unit
		if (unit->weaponDamage() < 0 && unit->isFacility() && unit->repairPlugin()) {
			// some kind of repairyard - repair all units
			// (not yet implemented) //FIXME
			// note that currently the unit can go to every friendly
			// player, even non-local players
			QPtrList<Unit> allUnits = selection()->allUnits();
			QPtrList<Unit> list;
			QPtrListIterator<Unit> it(allUnits);
			while (it.current()) {
				if (it.current()->health() < it.current()->unitProperties()->health()) {
					kdDebug() << "repair " << it.current()->id() << endl;
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
			*send = true;
			// TODO:
//			Unit* u = selection()->leader();
//			boMusic->playSound(u, Unit::SoundOrderRepair);
		} else if ((unit->unitProperties()->canRefineMinerals() &&
				selection()->hasMineralHarvester()) ||
				(unit->unitProperties()->canRefineOil() &&
				selection()->hasOilHarvester())) {
			// go to the refinery
			bool minerals = unit->unitProperties()->canRefineMinerals();
			QPtrList<Unit> allUnits = selection()->allUnits();
			QPtrList<Unit> list;
			QPtrListIterator<Unit> unitsIt(allUnits);
			while (unitsIt.current()) {
				if (unitsIt.current()->unitProperties()->canMineMinerals() && minerals) {
					list.append(unitsIt.current());
				} else if (unitsIt.current()->unitProperties()->canMineOil() && !minerals) {
					list.append(unitsIt.current());
				}
				++unitsIt;
			}
			if (!list.count()) {
				kdError() << k_lineinfo << "MoveRefine: empty list!!" << endl;
				return;
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
			*send = true;
			// TODO:
//			Unit* u = selection()->leader();
//			boMusic->playSound(u, Unit::SoundOrderRefine);
		} else {
			// selection and clicked unit both are friendly
			// no repairyard and no refinery
			// (at least no valid)
			// add other possibilities here
		}
	}
 }
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
	kdError() << "NULL local player" << endl;
	return;
 }
 if (!selection()) {
	kdError() << k_funcinfo << "NULL selection" << endl;
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
	kdError() << k_funcinfo << "NULL cursor!!" << endl;
	return;
 }

 QPoint widgetPos = mapFromGlobal(QCursor::pos());
#ifndef NO_OPENGL
 QPoint canvasPos;
 GLdouble x, y, z;
 mapCoordinates(widgetPos, &x, &y, &z);
 worldToCanvas(x, y, z, &canvasPos);
#else
 QPoint canvasPos = viewportToContents(widgetPos);
#endif
 
 if (!selection()->isEmpty()) {
	if (selection()->leader()->owner() == localPlayer()) {
		Unit* unit = ((BosonCanvas*)canvas())->findUnitAt(canvasPos);
		if (unit) {
			if (unit->owner() == localPlayer()) {
				c->setCursor(CursorDefault);
//				c->setWidgetCursor(this);
			} else if(selection()->leader()->unitProperties()->canShoot()) {
				if((unit->isFlying() && selection()->leader()->unitProperties()->canShootAtAirUnits()) ||
						(!unit->isFlying() && selection()->leader()->unitProperties()->canShootAtLandUnits())) {
					c->setCursor(CursorAttack);
//					c->setWidgetCursor(this);
				}
			}
		} else if (selection()->leader()->isMobile()) {
			c->setCursor(CursorMove);
//			c->setWidgetCursor(this);
			c->showCursor();
		} else {
			c->setCursor(CursorDefault);
//			c->setWidgetCursor(this);
			c->showCursor();
		}
	} else {
		c->setCursor(CursorDefault);
//		c->setWidgetCursor(this);
	}
 } else {
	c->setCursor(CursorDefault);
//	c->setWidgetCursor(this);
 }

 c->move(canvasPos.x(), canvasPos.y());
 c->setWidgetCursor(this);
}

/*
void BosonBigDisplay::addMouseIO(Player* p)
{
//AB: make sure that both editor and game mode can share the same IO !
 if (!localPlayer()) {
	kdError() << k_funcinfo << "NULL player" << endl;
	return;
 }
 if (d->mMouseIO) {
	kdError() << "This view already has a mouse io!!" << endl;
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
