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

#include "editorbigdisplayinput.h"
#include "editorbigdisplayinput.moc"

#include "bosonbigdisplaybase.h"

#include "boselection.h"
#include "bosoncanvas.h"
#include "bosonconfig.h"
#include "bosonmessage.h"
#include "bosongroundtheme.h"
#include "boson.h"
#include "bosonmap.h"
#include "bosoncursor.h"
#include "playerio.h"
#include "unitproperties.h"
#include "pluginproperties.h"
#include "unit.h"
#include "unitplugins.h"
#include "cell.h"
#include "bodebug.h"
#include "boaction.h"
#include "bosonlocalplayerinput.h"
#include "player.h" // FIXME: should not be here!
//#include "no_player.h"

#include <klocale.h>
#include <kapplication.h>

#include <math.h>


// this just stores a *selection* for placement. This means e.g. if you click
// on a unit (in the command frame!) the unit type is placed here as well as the
// player the unit shall belong to.
class Placement
{
public:
	enum PlacementType {
		PlaceNothing = 0,
		PlaceUnit = 1,
		PlaceGround = 2
	};

	Placement()
	{
		mTextureAlpha = 0;
		reset();
	}
	~Placement()
	{
	}
	void reset()
	{
		mType = PlaceNothing;
		mUnitType = 0;
		mOwner = 0;
		mTextureCount = 0;
		delete mTextureAlpha;
		mTextureAlpha = 0;
	}

	void placeUnit(unsigned long int t, Player* owner)
	{
		reset();
		mType = PlaceUnit;
		mUnitType = t;
		mOwner = owner;
	}

	void placeGround(unsigned int texCount, unsigned char* alpha)
	{
		reset();
		mType = PlaceGround;
		mTextureCount = texCount;
		mTextureAlpha = new unsigned char[texCount];
		for (unsigned int i = 0; i < texCount; i++) {
			mTextureAlpha[i] = alpha[i];
		}
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

	unsigned int textureCount() const
	{
		return mTextureCount;
	}
	unsigned char textureAlpha(unsigned int texture) const
	{
		if (texture >= textureCount()) {
			return 0;
		}
		if (!mTextureAlpha) {
			BO_NULL_ERROR(mTextureAlpha);
			return 0;
		}
		return mTextureAlpha[texture];
	}
	unsigned char* textureAlpha() const
	{
		return mTextureAlpha;
	}

	PlacementType type() const { return mType; }
	bool isUnit() const { return type() == PlaceUnit; }
	bool isGround() const { return type() == PlaceGround; }

private:
	PlacementType mType;
	unsigned long int mUnitType;
	Player* mOwner;
	unsigned int mTextureCount;
	unsigned char* mTextureAlpha;
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
 setActionType(ActionPlacementPreview); // dummy initialization
}

EditorBigDisplayInput::~EditorBigDisplayInput()
{
 delete d;
}

void EditorBigDisplayInput::actionClicked(const BoMouseEvent& event)
{
 boDebug() << k_funcinfo << endl;
 BO_CHECK_NULL_RET(canvas());
 if (!canvas()->onCanvas(event.canvasVector())) {
	return;
 }
 if (actionLocked()) {
	if (actionType() == ActionPlacementPreview) {
		actionPlace(event.canvasVector());
		return;
	} else if (actionType() == ActionChangeHeight) {
		bool up = !event.controlButton();
		actionChangeHeight(event.canvasVector(), up);
	}
 }
}

bool EditorBigDisplayInput::actionPlace(const BoVector3& canvasVector)
{
 boDebug() << k_funcinfo << endl;
 if (!canvas()) {
	BO_NULL_ERROR(canvas())
	return false;
 }
 if (!localPlayerIO()) {
	BO_NULL_ERROR(localPlayerIO())
	return false;
 }
 if (!localPlayerInput()) {
	BO_NULL_ERROR(localPlayerInput());
	return false;
 }
 bool ret = false;
 int x = (int)(canvasVector.x());
 int y = (int)(canvasVector.y());
 if (!canvas()->cell(x, y)) {
	return false;
 }
 if (d->mPlacement.isUnit()) {
	if (!d->mPlacement.owner()) { // TODO
		boError() << k_funcinfo << "NULL owner" << endl;
		return false;
	}
	const UnitProperties* prop = localPlayerIO()->unitProperties(d->mPlacement.unitType());
	if (!prop) {
		boError() << k_funcinfo << "invalid unittype " << d->mPlacement.unitType() << endl;
		return false;
	}
	if (!canvas()->canPlaceUnitAt(prop, BoVector2(x, y), 0)) {
		boDebug() << k_funcinfo << "Can't place unit at " << x << " " << y << endl;
		boGame->slotAddChatSystemMessage(i18n("You can't place a %1 there!").arg(prop->name()));
		ret = false;
	} else {
		boDebug() << "place unit " << d->mPlacement.unitType() << endl;

		localPlayerInput()->placeUnit(d->mPlacement.owner(), d->mPlacement.unitType(), x, y);
		ret = true;
	}
 } else if (d->mPlacement.isGround()) {
#if 0
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
//		stream << (Q_INT8)Cell::isBigTrans(d->mPlacement.cell());
		stream << (Q_INT8)false; // never a big trans
		stream << (Q_INT32)x;
		stream << (Q_INT32)y;
		ret = true;
	}
#endif
	// AB: we should convert this to be corner based.
	// also the canvas()->cellOccupied() check must check adjacent cells,
	// too as all surrounding cells are affected when the corners are
	// changed.

	BosonMap* map = canvas()->map();
	if (!map) {
		BO_NULL_ERROR(map);
		return false;
	}
	BosonGroundTheme* groundTheme = map->groundTheme();
	if (!groundTheme) {
		BO_NULL_ERROR(groundTheme);
		return false;
	}
	if (groundTheme->textureCount() != d->mPlacement.textureCount()) {
		boError() << k_funcinfo << "textureCount=" << groundTheme->textureCount()
				<< " , placement textureCount="
				<< d->mPlacement.textureCount() << endl;
		return false;
	}

	// I find it more comfortable to put everything to stream here and use
	//  localPlayerInput()->sendInput() rather than to add method with many many
	//  parameters to localPlayerInput
	QByteArray b;
	QDataStream stream(b, IO_WriteOnly);


	stream << (Q_UINT32)BosonMessage::MoveEditor;
	stream << (Q_UINT32)BosonMessage::MoveChangeTexMap;

	// we modify 4 corners (hardcoded at the moment)
	stream << (Q_UINT32)4;

	unsigned int cornersX[4] = { x, x + 1, x + 1,     x };
	unsigned int cornersY[4] = { y,     y, y + 1, y + 1 };
	for (unsigned int i = 0; i < 4; i++) {
		stream << (Q_UINT32)cornersX[i];
		stream << (Q_UINT32)cornersY[i];
		stream << (Q_UINT32)groundTheme->textureCount();
		for (unsigned int j = 0; j < d->mPlacement.textureCount(); j++) {
			unsigned char alpha = 0;
			alpha = d->mPlacement.textureAlpha(j);
			stream << (Q_UINT32)j;
			stream << (Q_UINT8)alpha;
		}
	}
	QDataStream msg(b, IO_ReadOnly);
	localPlayerInput()->sendInput(msg);
	ret = true;
 }
 if (ret) {
	// TODO: in BosonPlayField (call it when the message is received?
	// setModified(true);
 }
 return ret;
}

bool EditorBigDisplayInput::actionChangeHeight(const BoVector3& canvasVector, bool up)
{
 boDebug() << k_funcinfo << endl;
 if (!localPlayerInput()) {
	BO_NULL_ERROR(localPlayerInput());
	return false;
 }
 if (!canvas()) {
	BO_NULL_ERROR(canvas());
	return false;
 }
 if (!canvas()->onCanvas(canvasVector)) {
	return false;
 }
 int cellX = (int)(canvasVector.x());
 int cellY = (int)(canvasVector.y());
 if (!canvas()->cell(cellX, cellY)) {
	return false;
 }
 // we need the corner that was clicked, not the cell!
 int cornerX = lrint(canvasVector.x());
 int cornerY = lrint(canvasVector.y());

 float height = canvas()->heightAtCorner(cornerX, cornerY);
 if (up) {
	height += 0.5f;
 } else {
	height -= 0.5f;
 }
 localPlayerInput()->changeHeight(cornerX, cornerY, height);
 return true;
}

// the place*() methods get called when an item in (e.g.) the commandframe is
// selected.
void EditorBigDisplayInput::placeUnit(unsigned long int unitType, Player* owner)
{
 boDebug() << k_funcinfo << endl;
 if (!owner) {
	boError() << k_funcinfo << "NULL owner" << endl;
	return;
 }
 if (owner != localPlayerIO()->player()) {
	boError() << k_funcinfo << "owner != localplayer" << endl;
	return;
 }
 boDebug() << k_funcinfo << "now placing unit: " << unitType << " for " << localPlayerIO()->playerId() << "==" << localPlayerIO()->name() << endl;
 d->mPlacement.placeUnit(unitType, owner);
}

void EditorBigDisplayInput::placeGround(unsigned int textureCount, unsigned char* alpha)
{
 boDebug() << k_funcinfo << endl;
 QString s;
 for (unsigned int i = 0; i < textureCount; i++) {
	s += QString::number(alpha[i]) + QString(" ");
 }
 boDebug() << k_funcinfo << "now placing ground: " << s << endl;
 d->mPlacement.placeGround(textureCount, alpha);
}

void EditorBigDisplayInput::deleteSelectedUnits()
{
 BO_CHECK_NULL_RET(selection());
 BO_CHECK_NULL_RET(canvas());
 boDebug() << k_funcinfo << endl;
 if (selection()->isEmpty()) {
	boDebug() << k_funcinfo << "no unit selected" << endl;
	return;
 }
 QPtrList<Unit> units = selection()->allUnits();
 selection()->clear();
 QByteArray b;
 QDataStream s(b, IO_WriteOnly);
 s << (Q_UINT32)BosonMessage::MoveEditor;
 s << (Q_UINT32)BosonMessage::MoveDeleteItems;
 s << (Q_UINT32)units.count();
 QPtrListIterator<Unit> it(units);
 QPtrList<BosonItem> items;
 for (; it.current(); ++it) {
	s << (Q_ULONG)it.current()->id();
 }

 QDataStream msg(b, IO_ReadOnly);
 localPlayerInput()->sendInput(msg);
}

void EditorBigDisplayInput::updatePlacementPreviewData()
{
 BO_CHECK_NULL_RET(canvas());
 if (!d->mPlacement.isUnit() && !d->mPlacement.isGround()) {
	bigDisplay()->setPlacementPreviewData(0, false);
	return;
 }
 if (d->mPlacement.isUnit()) {
	if (!d->mPlacement.owner()) {
		boError() << k_funcinfo << "NULL owner" << endl;
		bigDisplay()->setPlacementPreviewData(0, false);
		return;
	}
#warning do NOT use Player here! use PlayerIO
	const UnitProperties* prop = d->mPlacement.owner()->unitProperties(d->mPlacement.unitType());

	bool canPlace = canvas()->canPlaceUnitAt(prop, cursorCanvasPos(), 0);
	bigDisplay()->setPlacementPreviewData(prop, canPlace);
 } else if (d->mPlacement.isGround()) {
	if (d->mPlacement.textureCount() == 0) {
		boError() << k_funcinfo << "no textures" << endl;
		return;
	}
	if (!d->mPlacement.textureAlpha()) {
		boError() << k_funcinfo << "NULL textureAlpha" << endl;
		return;
	}
	bool canPlace = true; // we could use false if there is a unit or so?
	bigDisplay()->setPlacementCellPreviewData(d->mPlacement.textureCount(),
			d->mPlacement.textureAlpha(), canPlace);
 }
}

void EditorBigDisplayInput::action(const BoSpecificAction& action)
{
 boDebug() << k_funcinfo << action.type() << endl;
 switch (action.type()) {
	case ActionPlacementPreview:
	case ActionChangeHeight:
		setActionType(action.type());
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
 BO_CHECK_NULL_RET(localPlayerIO());
 BO_CHECK_NULL_RET(selection());
 if (selection()->isEmpty()) {
	return;
 }
 BoMouseEvent event;
 event.setCanvasVector(BoVector3((float)(cellX + 1.0f / 2),
		(float)(cellY + 1.0f / 2),
		0.0f));
 actionClicked(event);
}

void EditorBigDisplayInput::updateCursor()
{
 BO_CHECK_NULL_RET(bigDisplay());
 BosonCursor* c = bigDisplay()->cursor();
 BO_CHECK_NULL_RET(c);

 // AB: in editor mode we always use the default KDE cursor, so calling this
 // method does basically nothing at all.
 // but under certain rare circumstances it might happen that there is no cursor
 // set for a widget (i.e. a blank cursor). then we must revert to the default -
 // that is what we do here. for simplicity we simply revert to default whenever
 // this is called.

 c->setWidgetCursor(bigDisplay());
}

