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
#include "playerio.h"
#include "unit.h"
#include "bosonlocalplayerinput.h"
#include "items/bosonitem.h"

#warning TODO: the input classes should touch PlayerIO only, not Player directly!

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

#include "boson.h"
BosonCanvas* BosonBigDisplayInputBase::canvas() const
{
// return bigDisplay()->canvas();
 return boGame->canvasNonConst(); // FIXME: we need a non-const version here but dont have it in the big display
}

BosonCollisions* BosonBigDisplayInputBase::collisions() const
{
 BO_CHECK_NULL_RET0(canvas());
 return canvas()->collisions();
}

PlayerIO* BosonBigDisplayInputBase::localPlayerIO() const
{
 return bigDisplay()->localPlayerIO();
}

BosonLocalPlayerInput* BosonBigDisplayInputBase::localPlayerInput() const
{
 if (!localPlayerIO()) {
	return 0;
 }
 return (BosonLocalPlayerInput*)localPlayerIO()->findRttiIO(BosonLocalPlayerInput::LocalPlayerInputRTTI);
}

const BoVector3Fixed& BosonBigDisplayInputBase::cursorCanvasVector() const
{
 return bigDisplay()->cursorCanvasVector();
}

void BosonBigDisplayInputBase::selectSingle(Unit* unit, bool replace)
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

void BosonBigDisplayInputBase::selectArea(BoItemList* itemsInArea, bool replace)
{
 BO_CHECK_NULL_RET(localPlayerIO());
 BO_CHECK_NULL_RET(canvas());
 BO_CHECK_NULL_RET(selection());
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
	if (!canvas()->onCanvas((*it)->x(), (*it)->y())) {
		boError() << k_funcinfo << "item is not on the canvas" << endl;
		continue;
	}
	if (!localPlayerIO()->canSee(*it)) {
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

void BosonBigDisplayInputBase::selectUnits(QPtrList<Unit> unitList, bool replace)
{
 boDebug() << k_funcinfo << endl;
 BO_CHECK_NULL_RET(selection());
 selection()->selectUnits(unitList, replace);
}

