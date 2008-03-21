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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "bosongameviewinputbase.h"
#include "bosongameviewinputbase.moc"

#include "../bomemory/bodummymemory.h"
#include "../no_player.h"
#include "../bosonconfig.h"
#include "boselection.h"
#include "../gameengine/bosoncanvas.h"
#include "../gameengine/boitemlist.h"
#include "bodebug.h"
#include "../gameengine/rtti.h"
#include "../gameengine/playerio.h"
#include "../gameengine/unit.h"
#include "../bosoncursor.h"
#include "bosonlocalplayerinput.h"
#include "../gameengine/bosonitem.h"

BosonGameViewInputBase::BosonGameViewInputBase()
	: QObject()
{
 mSelection = 0;
 mCursor = 0;
 mCanvas = 0;
 mLocalPlayerIO = 0;
 mCursorCanvasVector = 0;
 mActionLocked = false;
 mActionType = ActionAttack; // dummy initialization
 mCursorType = CursorDefault;
 mPlacementFreePlacement = false;
 mPlacementDisableCollisionDetection = false;
}

BosonGameViewInputBase::~BosonGameViewInputBase()
{
}

void BosonGameViewInputBase::setSelection(BoSelection* selection)
{
 mSelection = selection;
}

BoSelection* BosonGameViewInputBase::selection() const
{
 return mSelection;
}

void BosonGameViewInputBase::setCursor(BosonCursor* c)
{
 mCursor = c;
}

BosonCursor* BosonGameViewInputBase::cursor() const
{
 return mCursor;
}

void BosonGameViewInputBase::setCanvas(const BosonCanvas* c)
{
 mCanvas = c;
}

const BosonCanvas* BosonGameViewInputBase::canvas() const
{
 return mCanvas;
}

const BosonCollisions* BosonGameViewInputBase::collisions() const
{
 BO_CHECK_NULL_RET0(canvas());
 return canvas()->collisions();
}

void BosonGameViewInputBase::setLocalPlayerIO(PlayerIO* io)
{
 mLocalPlayerIO = io;
}

PlayerIO* BosonGameViewInputBase::localPlayerIO() const
{
 return mLocalPlayerIO;
}

BosonLocalPlayerInput* BosonGameViewInputBase::localPlayerInput() const
{
 if (!localPlayerIO()) {
	return 0;
 }
 return (BosonLocalPlayerInput*)localPlayerIO()->findRttiIO(BosonLocalPlayerInput::LocalPlayerInputRTTI);
}

void BosonGameViewInputBase::setCursorCanvasVector(const BoVector3Fixed* v)
{
 mCursorCanvasVector = v;
}

const BoVector3Fixed& BosonGameViewInputBase::cursorCanvasVector() const
{
 return *mCursorCanvasVector;
}

void BosonGameViewInputBase::selectSingle(Unit* unit, bool replace)
{
 boDebug() << k_funcinfo << endl;
 BO_CHECK_NULL_RET(selection());
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

void BosonGameViewInputBase::selectArea(BoItemList* itemsInArea, bool replace)
{
 BO_CHECK_NULL_RET(localPlayerIO());
 BO_CHECK_NULL_RET(canvas());
 BO_CHECK_NULL_RET(selection());
 BO_CHECK_NULL_RET(itemsInArea);
 if (boConfig->intValue("DebugMode") == (int)BosonConfig::DebugSelection) {
	const BoItemList* list = itemsInArea;
	BoItemList::ConstIterator it;
	boDebug() << k_funcinfo << "Selection count: " << list->count() << endl;
	for (it = list->begin(); it != list->end(); ++it) {
		QString s = QString("Selected: RTTI=%1").arg((*it)->rtti());
		if (RTTI::isUnit((*it)->rtti())) {
			Unit* u = (Unit*)*it;
			s += QString(" Unit ID=%1").arg(u->id());
			if (u->isDestroyed()) {
				s += QString("(destroyed)");
			}
		}
		boDebug() << k_funcinfo << s << endl;
	}
 }

 QPtrList<Unit> unitList;
 Unit* fallBackUnit= 0; // in case no localplayer mobile unit can be found we'll select this instead
 BoItemList::Iterator it;
 for (it = itemsInArea->begin(); it != itemsInArea->end(); ++it) {
	if (!RTTI::isUnit((*it)->rtti())) {
		continue;
	}
	if (!canvas()->onCanvas((*it)->x(), (*it)->y())) {
		boError() << k_funcinfo << "item is not on the canvas" << endl;
		continue;
	}
	Unit* unit = (Unit*)*it;
	if (!(unit->visibleStatus(localPlayerIO()->playerId()) & (UnitBase::VS_Visible | UnitBase::VS_Earlier))) {
		//continue;
	}
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
	boDebug() << k_funcinfo << "select " << unitList.count() << " units" << endl;
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

void BosonGameViewInputBase::unselectArea(BoItemList* itemsInArea)
{
 BO_CHECK_NULL_RET(selection());
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

void BosonGameViewInputBase::selectUnits(QPtrList<Unit> unitList, bool replace)
{
 boDebug() << k_funcinfo << endl;
 BO_CHECK_NULL_RET(selection());
 selection()->selectUnits(unitList, replace);
}

void BosonGameViewInputBase::setPlacementFreePlacement(bool free)
{
 mPlacementFreePlacement = free;
}


void BosonGameViewInputBase::setPlacementDisableCollisions(bool disable)
{
 mPlacementDisableCollisionDetection = disable;
}

void BosonGameViewInputBase::makeCursorInvalid()
{
 if (mCursor) {
	mCursor->setCursor(-1);
 }
}

