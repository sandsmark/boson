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

#include "bosonbigdisplayinputbase.h"
#include "bosonbigdisplayinputbase.moc"

#include "bosonbigdisplaybase.h"

#include "bosonconfig.h"
#include "boselection.h"
#include "bosoncanvas.h"
#include "boitemlist.h"
#include "bodebug.h"
#include "rtti.h"
#include "player.h"
#include "unit.h"
#include "items/bosonitem.h"

BosonBigDisplayInputBase::BosonBigDisplayInputBase(BosonBigDisplayBase* parent) : QObject(parent)
{
 BO_CHECK_NULL_RET(parent);
 mBigDisplay = parent;
 mCursorType = CursorDefault;
 mActionLocked = false;
 mActionType = ActionAttack; // dummy initialization
}

BosonBigDisplayInputBase::~BosonBigDisplayInputBase()
{
}

BoSelection* BosonBigDisplayInputBase::selection() const
{
 return bigDisplay()->selection();
}

BosonCanvas* BosonBigDisplayInputBase::canvas() const
{
 return bigDisplay()->canvas();
}

BosonCollisions* BosonBigDisplayInputBase::collisions() const
{
 return canvas()->collisions();
}

Player* BosonBigDisplayInputBase::localPlayer() const
{
 return bigDisplay()->localPlayer();
}

const QPoint& BosonBigDisplayInputBase::cursorCanvasPos() const
{
 return bigDisplay()->cursorCanvasPos();
}

const BoVector3& BosonBigDisplayInputBase::cursorCanvasVector() const
{
 return bigDisplay()->cursorCanvasVector();
}

void BosonBigDisplayInputBase::selectSingle(Unit* unit, bool replace)
{
 boDebug() << k_funcinfo << endl;
 switch (canSelect(unit)) {
	case CanSelectSingleOk:
		// this should not happen, as it should have been checked before
		// already.
		replace = true;
		break;
	case CanSelectMultipleOk:
		break;
	default:
		return;
 }
 selection()->selectUnit(unit, replace);
}

void BosonBigDisplayInputBase::selectArea(BoItemList* itemsInArea, bool replace)
{
 BO_CHECK_NULL_RET(itemsInArea);
 if (boConfig->debugMode() == BosonConfig::DebugSelection) {
	const BoItemList* list = itemsInArea;
	BoItemList::ConstIterator it;
	boDebug() << "Selection count: " << list->count() << endl;
	for (it = list->begin(); it != list->end(); ++it) {
		QString s = QString("Selected: RTTI=%1").arg((*it)->rtti());
		if (RTTI::isUnit((*it)->rtti())) {
			Unit* u = (Unit*)*it;
			s += QString(" Unit ID=%1").arg(u->id());
			if (u->isDestroyed()) {
				s += QString("(destroyed)");
			}
		}
		boDebug() << s << endl;
	}
 }

 QPtrList<Unit> unitList;
 Unit* fallBackUnit= 0; // in case no localplayer mobile unit can be found we'll select this instead
 BoItemList::Iterator it;
 for (it = itemsInArea->begin(); it != itemsInArea->end(); ++it) {
	if (!RTTI::isUnit((*it)->rtti())) {
		continue;
	}
	if (!canvas()->onCanvas((int)(*it)->x(), (int)(*it)->y())) {
		boError() << k_funcinfo << "item is not on the canvas" << endl;
		continue;
	}
	if (localPlayer()->isFogged((int)(*it)->x() / BO_TILE_SIZE, (int)(*it)->y() / BO_TILE_SIZE)) {
		continue;
	}
	Unit* unit = (Unit*)*it;
	CanSelectUnit s = canSelect(unit);
	switch (s) {
		case CanSelectSingleOk:
			fallBackUnit = unit;
			break;
		case CanSelectMultipleOk:
			unitList.append(unit);
			break;
		case CanSelectDestroyed:
		case CanSelectError:
			break;
	}
 }

 if (unitList.count() > 0) {
	boDebug() << "select " << unitList.count() << " units" << endl;
	selectUnits(unitList, replace);
 } else if (fallBackUnit && selection()->count() == 0) {
	selectSingle(fallBackUnit, replace);
 } else {
	boDebug() << k_funcinfo << "select nothing" << endl;
	if (replace) {
		selection()->clear();
	}
 }
}

void BosonBigDisplayInputBase::unselectArea(BoItemList* itemsInArea)
{
 BO_CHECK_NULL_RET(itemsInArea);
 BoItemList::Iterator it;
 for (it = itemsInArea->begin(); it != itemsInArea->end(); ++it) {
	if (!RTTI::isUnit((*it)->rtti())) {
		continue;
	}
	Unit* u = (Unit*)*it;
	selection()->removeUnit(u);
 }
}

void BosonBigDisplayInputBase::selectUnits(QPtrList<Unit> unitList, bool replace)
{
 boDebug() << k_funcinfo << endl;
 selection()->selectUnits(unitList, replace);
}

