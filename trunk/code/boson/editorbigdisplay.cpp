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
#include "editorbigdisplay.h"
#include "editorbigdisplay.moc"

#include "unit.h"
#include "unitproperties.h"
#include "bosoncanvas.h"
#include "player.h"
#include "boson.h"
#include "cell.h"
#include "bosonmessage.h"
#include "global.h"
#include "boselection.h"
#include "defines.h"
#include "bodebug.h"
//#include "kspritetooltip.h"//TODO

#include <kapplication.h>
#include <kgame/kgameio.h>
#include <klocale.h>

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

class EditorBigDisplay::EditorBigDisplayPrivate
{
public:
	EditorBigDisplayPrivate()
	{
		mMouseIO = 0;
	}

	KGameMouseIO* mMouseIO;

	Placement mPlacement;

	bool mActionLocked;
	UnitAction mActionType;
};

EditorBigDisplay::EditorBigDisplay(BosonCanvas* c, QWidget* parent) 
		: BosonBigDisplayBase(c, parent)
{
 init();
}

void EditorBigDisplay::init()
{
 d = new EditorBigDisplayPrivate;
}

EditorBigDisplay::~EditorBigDisplay()
{
 delete d;
}

void EditorBigDisplay::setLocalPlayer(Player* p)
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
 if (localPlayer()) {
	addMouseIO(localPlayer());
 }
}

void EditorBigDisplay::actionClicked(const BoAction& action, QDataStream& stream, bool* send)
{
// boDebug() << k_funcinfo << endl;
 if (actionLocked()) {
	if (actionType() == ActionBuild) {
		if (actionPlace(stream, action.canvasPos())) {
			*send = true;
		}
		return;
	}
 }
 if (actionPlace(stream, action.canvasPos())) {
	*send = true;
 }
}

bool EditorBigDisplay::actionPlace(QDataStream& stream, const QPoint& canvasPos)
{
 bool ret = false;
 int x = canvasPos.x() / BO_TILE_SIZE;
 int y = canvasPos.y() / BO_TILE_SIZE;
 if (d->mPlacement.isUnit()) {
	if (!d->mPlacement.owner()) { // TODO
		boError() << k_funcinfo << "NULL owner" << endl;
		return false;
	}
	boDebug() << "place unit " << d->mPlacement.unitType() << endl;

	stream << (Q_UINT32)BosonMessage::MoveEditor;
	stream << (Q_UINT32)BosonMessage::MovePlaceUnit;
	stream << (Q_INT32)d->mPlacement.owner()->id();
	stream << (Q_INT32)d->mPlacement.unitType();
	stream << (Q_INT32)x;
	stream << (Q_INT32)y;
	ret = true;
//	setModified(true); // TODO: in BosonPlayField
 } else if (d->mPlacement.isCell()) {
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
 if (ret) {
	// TODO: in BosonPlayField (call it when the message is received?
	// setModified(true);
 }
 return ret;
}

/*
void EditorBigDisplay::addMouseIO(Player* p)
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

// the place*() methods get called when an item in (e.g.) the commandframe is
// selected.
void EditorBigDisplay::placeUnit(unsigned long int unitType, Player* owner)
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

void EditorBigDisplay::placeCell(int tile)
{
 boDebug() << k_funcinfo << "now placing cell: " << tile << endl;
 if (tile < 0) {
	boError() << k_funcinfo << "invalid tile " << tile << endl;
	return;
 }
 d->mPlacement.placeCell(tile);
}

void EditorBigDisplay::deleteSelectedUnits()
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

bool EditorBigDisplay::selectAll(const UnitProperties* prop, bool replace)
{
 QPtrList<Unit> list;
 for (unsigned int i = 0; i < boGame->playerCount(); i++) {
	QPtrList<Unit> allUnits = ((Player*)boGame->playerList()->at(i))->allUnits();
	QPtrListIterator<Unit> it(allUnits);
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

void EditorBigDisplay::updatePlacementPreviewData()
{
 if (!d->mPlacement.isUnit()) {
	setPlacementPreviewData(0, false);
	return;
 }
 if (!d->mPlacement.owner()) {
	boError() << k_funcinfo << "NULL owner" << endl;
	setPlacementPreviewData(0, false);
	return;
 }
 const UnitProperties* prop = d->mPlacement.owner()->unitProperties(d->mPlacement.unitType());
 QPoint pos(cursorCanvasPos() / BO_TILE_SIZE);
 setPlacementPreviewData(prop, canvas()->canPlaceUnitAt(prop, pos, 0));
}


void EditorBigDisplay::unitAction(int actionType)
{
 boDebug() << k_funcinfo << actionType << endl;
 switch (actionType) {
	case ActionBuild:
		d->mActionType = ActionBuild;
		d->mActionLocked = true;
		break;
	default:
		unlockAction();
		break;
 }
}

bool EditorBigDisplay::actionLocked() const
{
 return d->mActionLocked;
}

void EditorBigDisplay::unlockAction()
{
 d->mActionLocked = false;
}

UnitAction EditorBigDisplay::actionType() const
{
 return d->mActionType;
}

