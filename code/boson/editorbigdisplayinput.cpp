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

#include "editorbigdisplayinput.h"
#include "editorbigdisplayinput.moc"

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
#include "cell.h"
#include "bodebug.h"

#include <klocale.h>
#include <kapplication.h>


// this just stores a *selection* for placement. This means e.g. if you click
// on a unit (in the command frame!) the unit type is placed here as well as the
// player the unit shall belong to.
class Placement
{
public:
	enum PlacementType {
		PlaceNothing = 0,
		PlaceUnit = 1,
		PlaceCell = 2
	};

	Placement()
	{
		reset();
	}
	void reset()
	{
		mType = PlaceNothing;
		mUnitType = 0;
		mGroundType = -1;
		mOwner = 0;
	}

	void placeUnit(unsigned long int t, Player* owner)
	{
		reset();
		mType = PlaceUnit;
		mUnitType = t;
		mOwner = owner;
	}

	void placeCell(int t)
	{
		reset();
		mType = PlaceCell;
		mGroundType = t;
	}

	/**
	 * @return The ID of the unittype to be placed, or 0 if none is to be
	 * placed.
	 **/
	unsigned long int unitType() const
	{
		if (isUnit()) {
			return mUnitType;
		}
		return 0;
	}

	/**
	 * @return The player that is currently selected (i.e. that the unit
	 * should be placed for), if @ref isUnit is TRUE. Otherwise 0.
	 **/
	Player* owner() const
	{
		if (isUnit()) {
			return mOwner;
		}
		return 0;
	}

	/**
	 * @return The tile number of the to-be-placed cell, or -1 if none is to
	 * be placed.
	 **/
	int cell() const
	{
		if (isCell()) {
			return mGroundType;
		}
		return -1;
	}

	PlacementType type() const { return mType; }
	bool isUnit() const { return type() == PlaceUnit; }
	bool isCell() const { return type() == PlaceCell; }

private:
	PlacementType mType;
	unsigned long int mUnitType;
	Player* mOwner;
	int mGroundType;
};

class EditorBigDisplayInputPrivate
{
public:
	EditorBigDisplayInputPrivate()
	{
	}
	Placement mPlacement;
};

EditorBigDisplayInput::EditorBigDisplayInput(BosonBigDisplayBase* parent) : BosonBigDisplayInputBase(parent)
{
 d = new EditorBigDisplayInputPrivate;
 setActionType(ActionBuild); // dummy initialization
}

EditorBigDisplayInput::~EditorBigDisplayInput()
{
 delete d;
}

void EditorBigDisplayInput::actionClicked(const BoAction& action, QDataStream& stream, bool* send)
{
 boDebug() << k_funcinfo << endl;
 if (!canvas()->onCanvas(action.canvasVector())) {
	return;
 }
 if (actionLocked()) {
	if (actionType() == ActionBuild) {
		if (actionPlace(stream, action.canvasVector())) {
			*send = true;
		}
		return;
	} else if (actionType() == ActionChangeHeight) {
		bool up = !action.controlButton();
		if (actionChangeHeight(stream, action.canvasVector(), up)) {
			*send = true;
		}
	}
 }
}

bool EditorBigDisplayInput::actionPlace(QDataStream& stream, const BoVector3& canvasVector)
{
 boDebug() << k_funcinfo << endl;
 if (!canvas()) {
	BO_NULL_ERROR(canvas())
	return false;
 }
 if (!localPlayer()) {
	BO_NULL_ERROR(localPlayer())
	return false;
 }
 bool ret = false;
 int x = (int)(canvasVector.x()) / BO_TILE_SIZE;
 int y = (int)(canvasVector.y()) / BO_TILE_SIZE;
 if (!canvas()->cell(x, y)) {
	return false;
 }
 if (d->mPlacement.isUnit()) {
	if (!d->mPlacement.owner()) { // TODO
		boError() << k_funcinfo << "NULL owner" << endl;
		return false;
	}
	const UnitProperties* prop = localPlayer()->unitProperties(d->mPlacement.unitType());
	if (!prop) {
		boError() << k_funcinfo << "invalid unittype " << d->mPlacement.unitType() << endl;
		return false;
	}
	if (!canvas()->canPlaceUnitAtCell(prop, QPoint(x, y), 0)) {
		boDebug() << k_funcinfo << "Can't place unit at " << x << " " << y << endl;
		boGame->slotAddChatSystemMessage(i18n("You can't place a %1 there!").arg(prop->name()));
		ret = false;
	} else {
	
		boDebug() << "place unit " << d->mPlacement.unitType() << endl;

		stream << (Q_UINT32)BosonMessage::MoveEditor;
		stream << (Q_UINT32)BosonMessage::MovePlaceUnit;
		stream << (Q_INT32)d->mPlacement.owner()->id();
		stream << (Q_INT32)d->mPlacement.unitType();
		stream << (Q_INT32)x;
		stream << (Q_INT32)y;
		ret = true;
	}
 } else if (d->mPlacement.isCell()) {
	if (canvas()->cellOccupied(x, y)) {
		// AB: this is not very user friendly.
		// we should check whether a unit can go on the new cell type
		// and if it can we should allow placing!!
		// only warn and abort if it can't go there // TODO
		boGame->slotAddChatSystemMessage(i18n("Remove the unit first"));
		ret = false;
	} else {
		unsigned char version = kapp->random() % 4;
		boDebug() << k_funcinfo << "place ground " << d->mPlacement.cell() << ",version=" << version << endl;

		stream << (Q_UINT32)BosonMessage::MoveEditor;
		stream << (Q_UINT32)BosonMessage::MovePlaceCell;
		stream << (Q_INT32)d->mPlacement.cell();
		stream << (Q_UINT8)version;
		stream << (Q_INT8)Cell::isBigTrans(d->mPlacement.cell());
		stream << (Q_INT32)x;
		stream << (Q_INT32)y;
		ret = true;
	}
 }
 if (ret) {
	// TODO: in BosonPlayField (call it when the message is received?
	// setModified(true);
 }
 return ret;
}

bool EditorBigDisplayInput::actionChangeHeight(QDataStream& stream, const BoVector3& canvasVector, bool up)
{
 boDebug() << k_funcinfo << endl;
 if (!canvas()) {
	BO_NULL_ERROR(canvas());
	return false;
 }
 if (!canvas()->onCanvas(canvasVector)) {
	return false;
 }
 int cellX = (int)(canvasVector.x()) / BO_TILE_SIZE;
 int cellY = (int)(canvasVector.y()) / BO_TILE_SIZE;
 if (!canvas()->cell(cellX, cellY)) {
	return false;
 }
 int cornerX = 0;
 int cornerY = 0;
 // we need the corner that was clicked, not the cell!
 if (((int)canvasVector.x()) % BO_TILE_SIZE >= BO_TILE_SIZE) {
	// a right corner
	cornerX = cellX + 1;
 } else {
	cornerX = cellX;
 }
 if (((int)canvasVector.x()) % BO_TILE_SIZE >= BO_TILE_SIZE) {
	cornerY = cellY + 1;
 } else {
	cornerY = cellY;
 }

 float height = canvas()->heightAtCorner(cornerX, cornerY);
 if (up) {
	height += 0.5f;
 } else {
	height -= 0.5f;
 }
 stream << (Q_UINT32)BosonMessage::MoveEditor;
 stream << (Q_UINT32)BosonMessage::MoveChangeHeight;
 stream << (Q_UINT32)1; // we change one corner only
 stream << (Q_INT32)cornerX;
 stream << (Q_INT32)cornerY;
 stream << (float)height;
 return true;
}

// the place*() methods get called when an item in (e.g.) the commandframe is
// selected.
void EditorBigDisplayInput::placeUnit(unsigned long int unitType, Player* owner)
{
 if (!owner) {
	boError() << k_funcinfo << "NULL owner" << endl;
	return;
 }
 if (owner != localPlayer()) {
	boError() << k_funcinfo << "owner != localplayer" << endl;
	return;
 }
 boDebug() << k_funcinfo << "now placing unit: " << unitType << " for " << owner->id() << "==" << owner->name() << endl;
 d->mPlacement.placeUnit(unitType, owner);
}

void EditorBigDisplayInput::placeCell(int tile)
{
 boDebug() << k_funcinfo << "now placing cell: " << tile << endl;
 if (tile < 0) {
	boError() << k_funcinfo << "invalid tile " << tile << endl;
	return;
 }
 d->mPlacement.placeCell(tile);
}

void EditorBigDisplayInput::deleteSelectedUnits()
{
 if (!selection()) {
	boError() << k_funcinfo << "NULL selection" << endl;
	return;
 }
 if (selection()->isEmpty()) {
	boDebug() << k_funcinfo << "no unit selected" << endl;
	return;
 }
 QPtrList<Unit> units = selection()->allUnits();
 selection()->clear();
 QPtrListIterator<Unit> it(units);
 for (; it.current(); ++it) {
	canvas()->removeUnit(it.current());
 }
 units.setAutoDelete(true);
 units.clear();
}

void EditorBigDisplayInput::updatePlacementPreviewData()
{
 if (!d->mPlacement.isUnit() && !d->mPlacement.isCell()) {
	bigDisplay()->setPlacementPreviewData(0, false);
	return;
 }
 if (d->mPlacement.isUnit()) {
	if (!d->mPlacement.owner()) {
		boError() << k_funcinfo << "NULL owner" << endl;
		bigDisplay()->setPlacementPreviewData(0, false);
		return;
	}
	const UnitProperties* prop = d->mPlacement.owner()->unitProperties(d->mPlacement.unitType());

	bool canPlace = canvas()->canPlaceUnitAt(prop, cursorCanvasPos(), 0);
	bigDisplay()->setPlacementPreviewData(prop, canPlace);

	QPoint p = cursorCanvasPos() / BO_TILE_SIZE;
 } else if (d->mPlacement.isCell()) {
	if (d->mPlacement.cell() < 0) {
		boError() << k_funcinfo << "invalid cell" << endl;
		return;
	}
	bigDisplay()->setPlacementCellPreviewData(d->mPlacement.cell(), true); // we could use false if there is a unit or so?
 }
}

void EditorBigDisplayInput::unitAction(int actionType)
{
 boDebug() << k_funcinfo << actionType << endl;
 switch (actionType) {
	case ActionBuild:
	case ActionChangeHeight:
		setActionType((UnitAction)actionType);
		lockAction();
		break;
	default:
		unlockAction();
		break;
 }
}

bool EditorBigDisplayInput::selectAll(const UnitProperties* prop, bool replace)
{
 QPtrList<Unit> list;
 for (unsigned int i = 0; i < boGame->playerCount(); i++) {
	QPtrListIterator<Unit> it(*(((Player*)boGame->playerList()->at(i))->allUnits()));
	while (it.current()) {
		if (it.current()->unitProperties()->typeId() == prop->typeId()) {
			if (canSelect(it.current()) == CanSelectMultipleOk) {
				list.append(it.current());
			}
		}
		++it;
	}
 }
 if (list.count() > 0) {
	selectUnits(list, replace);
	return true;
 }
 return false;
}

void EditorBigDisplayInput::slotMoveSelection(int cellX, int cellY)
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

