/*
    This file is part of the Boson game
    Copyright (C) 2002-2005 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "editorviewinput.h"
#include "editorviewinput.moc"

#include "../bomemory/bodummymemory.h"
#include "bosongameview.h"
#include "../boselection.h"
#include "../bosoncanvas.h"
#include "../bosonconfig.h"
#include "../bosonmessage.h"
#include "../bosonmessageids.h"
#include "../bosongroundtheme.h"
#include "../boson.h"
#include "../bosonmap.h"
#include "../bosoncursor.h"
#include "../playerio.h"
#include "../unitproperties.h"
#include "../pluginproperties.h"
#include "../unit.h"
#include "../unitplugins.h"
#include "../cell.h"
#include "bodebug.h"
#include "../boaction.h"
#include "bosonlocalplayerinput.h"

#warning TODO: the input classes should touch PlayerIO only, not Player directly!
#include "../player.h" // FIXME: should not be here!
//#include "../no_player.h"

#include <klocale.h>
#include <kapplication.h>

#include <qptrstack.h>

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

class EditorViewInputPrivate
{
public:
	EditorViewInputPrivate()
	{
	}
	Placement mPlacement;

	QPtrStack<BosonMessageEditorMove> mUndoStack;
	QPtrStack<BosonMessageEditorMove> mRedoStack;
};

EditorViewInput::EditorViewInput()
	: BosonGameViewInputBase()
{
 d = new EditorViewInputPrivate;
 setActionType(ActionPlacementPreview); // dummy initialization

 connect(boGame, SIGNAL(signalEditorClearUndoStack()),
		this, SLOT(slotClearUndoStack()));
 connect(boGame, SIGNAL(signalEditorClearRedoStack()),
		this, SLOT(slotClearRedoStack()));
 connect(boGame, SIGNAL(signalEditorNewUndoMessage(const BosonMessageEditorMove&, bool)),
		this, SLOT(slotNewUndoMessage(const BosonMessageEditorMove&, bool)));
 connect(boGame, SIGNAL(signalEditorNewRedoMessage(const BosonMessageEditorMove&)),
		this, SLOT(slotNewRedoMessage(const BosonMessageEditorMove&)));
}

EditorViewInput::~EditorViewInput()
{
 slotClearRedoStack();
 slotClearUndoStack();
 delete d;
}

void EditorViewInput::actionClicked(const BoMouseEvent& event)
{
 boDebug() << k_funcinfo << endl;
 BO_CHECK_NULL_RET(canvas());
 if (!canvas()->onCanvas(event.canvasVector())) {
	return;
 }
 if (actionLocked()) {
	if (actionType() == ActionPlacementPreview) {
		actionPlace(event.canvasVector(), event.controlButton(), event.shiftButton());
		return;
	} else if (actionType() == ActionChangeHeight) {
		bool up = !event.controlButton();
		actionChangeHeight(event.canvasVector(), up);
	}
 }
}

bool EditorViewInput::actionPlace(const BoVector3Fixed& canvasVector, bool exact, bool force)
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
 bofixed x = canvasVector.x();
 bofixed y = canvasVector.y();
 if(!exact || d->mPlacement.isGround())
 {
	x = rintf(x);
	y = rintf(y);
 }
 if (!canvas()->cell((int)x, (int)y)) {
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
	// Don't allow forcing mobile unit placement. If mobile units are inside
	//  another unit, they won't be able to move
	if (prop->isMobile()) {
		force = false;
	}
	if (!force && !canvas()->canPlaceUnitAt(prop, BoVector2Fixed(x, y), 0)) {
		boDebug() << k_funcinfo << "Can't place unit at " << x << " " << y << endl;
		boGame->slotAddChatSystemMessage(i18n("You can't place a %1 there!").arg(prop->name()));
		ret = false;
	} else {
		boDebug() << k_funcinfo << "place unit " << d->mPlacement.unitType() << endl;

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

#if 0
		stream << (Q_UINT32)BosonMessageIds::MoveEditor;
		stream << (Q_UINT32)BosonMessageIds::MovePlaceCell;
		stream << (Q_INT32)d->mPlacement.cell();
		stream << (Q_UINT8)version;
//		stream << (Q_INT8)Cell::isBigTrans(d->mPlacement.cell());
		stream << (Q_INT8)false; // never a big trans
		stream << (Q_INT32)x;
		stream << (Q_INT32)y;
#else
		BosonMessageEditorMovePlaceCell message;
		...
#endif
		ret = true;
	}
#endif
	// AB: we should convert this to be corner based.
	// also the canvas()->cellOccupied() check must check adjacent cells,
	// too as all surrounding cells are affected when the corners are
	// changed.

	const BosonMap* map = canvas()->map();
	if (!map) {
		BO_NULL_ERROR(map);
		return false;
	}
	const BosonGroundTheme* groundTheme = map->groundTheme();
	if (!groundTheme) {
		BO_NULL_ERROR(groundTheme);
		return false;
	}
	if (groundTheme->groundTypeCount() != d->mPlacement.textureCount()) {
		boError() << k_funcinfo << "groundTypeCount=" << groundTheme->groundTypeCount()
				<< " , placement textureCount="
				<< d->mPlacement.textureCount() << endl;
		return false;
	}

	// I find it more comfortable to create the message here and then use
	//  localPlayerInput()->sendInput() rather than to add method with many many
	//  parameters to localPlayerInput


	// we modify 4 corners (hardcoded at the moment)
	QValueVector<Q_UINT32> cornersX(4);
	QValueVector<Q_UINT32> cornersY(4);
	QValueVector<Q_UINT32> cornersTextureCount(4);
	QValueVector< QValueVector<Q_UINT32> > cornerTextures(4);
	QValueVector< QValueVector<Q_UINT8> > cornerAlpha(4);

	cornersX[0] = x;
	cornersX[1] = x + 1;
	cornersX[2] = x + 1;
	cornersX[3] = x;
	cornersY[0] = y;
	cornersY[1] = y;
	cornersY[2] = y + 1;
	cornersY[3] = y + 1;
	for (int i = 0; i < 4; i++) {
		cornersTextureCount[i] = groundTheme->groundTypeCount();
		if (d->mPlacement.textureCount() != groundTheme->groundTypeCount()) {
			boError() << k_funcinfo << "invalid texture count" << endl;
			return false;
		}
		cornerTextures[i].resize(d->mPlacement.textureCount());
		cornerAlpha[i].resize(d->mPlacement.textureCount());
		for (unsigned int j = 0; j < d->mPlacement.textureCount(); j++) {
			unsigned char alpha = 0;
			alpha = d->mPlacement.textureAlpha(j);
			(cornerTextures[i])[j] = j;
			(cornerAlpha[i])[j] = alpha;
		}
	}

	QByteArray b;
	QDataStream stream(b, IO_WriteOnly);

	BosonMessageEditorMoveChangeTexMap message(cornersX, cornersY, cornersTextureCount, cornerTextures, cornerAlpha);
	if (!message.save(stream)) {
		boError() << k_funcinfo << "unable to save message (" << message.messageId() << ")" << endl;
		return false;
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

bool EditorViewInput::actionChangeHeight(const BoVector3Fixed& canvasVector, bool up)
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
void EditorViewInput::placeUnit(unsigned long int unitType, Player* owner)
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

void EditorViewInput::placeGround(unsigned int textureCount, unsigned char* alpha)
{
 boDebug() << k_funcinfo << endl;
 QString s;
 for (unsigned int i = 0; i < textureCount; i++) {
	s += QString::number(alpha[i]) + QString(" ");
 }
 boDebug() << k_funcinfo << "now placing ground: " << s << endl;
 d->mPlacement.placeGround(textureCount, alpha);
}

void EditorViewInput::deleteSelectedUnits()
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

 QValueList<Q_ULONG> items;
 QPtrListIterator<Unit> it(units);
 for (; it.current(); ++it) {
	items.append(it.current()->id());
 }

 QByteArray b;
 QDataStream stream(b, IO_WriteOnly);
 BosonMessageEditorMoveDeleteItems message(items);
 if (!message.save(stream)) {
	boError() << k_funcinfo << "unable to save message (" << message.messageId() << ")" << endl;
	return;
 }

 QDataStream msg(b, IO_ReadOnly);
 localPlayerInput()->sendInput(msg);
}

void EditorViewInput::updatePlacementPreviewData()
{
 BO_CHECK_NULL_RET(canvas());
 if (!d->mPlacement.isUnit() && !d->mPlacement.isGround()) {
	emit signalSetPlacementPreviewData(0, false, placementFreePlacement(), !placementDisableCollisions());
	return;
 }
 if (d->mPlacement.isUnit()) {
	if (!d->mPlacement.owner()) {
		boError() << k_funcinfo << "NULL owner" << endl;
		emit signalSetPlacementPreviewData(0, false, placementFreePlacement(), !placementDisableCollisions());
		return;
	}
#warning do NOT use Player here! use PlayerIO
	const UnitProperties* prop = d->mPlacement.owner()->unitProperties(d->mPlacement.unitType());

	bool canPlace = canvas()->canPlaceUnitAt(prop, BoVector2Fixed(cursorCanvasVector().x(), cursorCanvasVector().y()), 0);
	emit signalSetPlacementPreviewData(prop, canPlace, placementFreePlacement(), !placementDisableCollisions());
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
	emit signalSetPlacementCellPreviewData(d->mPlacement.textureCount(),
			d->mPlacement.textureAlpha(), canPlace);
 }
}

void EditorViewInput::action(const BoSpecificAction& action)
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

bool EditorViewInput::selectAll(const UnitProperties* prop, bool replace)
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

void EditorViewInput::slotMoveSelection(int cellX, int cellY)
{
 BO_CHECK_NULL_RET(localPlayerIO());
 BO_CHECK_NULL_RET(selection());
 if (selection()->isEmpty()) {
	return;
 }
 BoMouseEvent event;
 event.setCanvasVector(BoVector3Fixed((float)(cellX + 1.0f / 2),
		(float)(cellY + 1.0f / 2),
		0.0f));
 actionClicked(event);
}

void EditorViewInput::updateCursor()
{
 BosonCursor* c = cursor();
 BO_CHECK_NULL_RET(c);

 // AB: this especially calls setWidgetCursor(), this is necessary in case the
 // cursor was hidden before
 c->setCursor(CursorDefault);
}

void EditorViewInput::slotClearUndoStack()
{
 while (!d->mUndoStack.isEmpty()) {
	BosonMessageEditorMove* m = d->mUndoStack.pop();
	delete m;
 }
 emit signalEditorHasUndo(QString::null);
}

void EditorViewInput::slotClearRedoStack()
{
 while (!d->mRedoStack.isEmpty()) {
	BosonMessageEditorMove* m = d->mRedoStack.pop();
	delete m;
 }
 emit signalEditorHasRedo(QString::null);
}

void EditorViewInput::slotNewUndoMessage(const BosonMessageEditorMove& undo, bool fromRedo)
{
 if (!fromRedo) {
	// message resulted from a normal (not-undo) message. no redo possible
	// anymore
	slotClearRedoStack();
 }
 BosonMessageEditorMove* m = 0;
 m = BosonMessageEditorMove::newCopy(undo);
 if (!m) {
	boError() << k_funcinfo << "unable to create a copy of message " << undo.messageId() << endl;
	return;
 }
 d->mUndoStack.push(m);

 QString name = messageName(d->mUndoStack.top());
 emit signalEditorHasUndo(name);
}

void EditorViewInput::slotNewRedoMessage(const BosonMessageEditorMove& redo)
{
 BosonMessageEditorMove* m = 0;
 m = BosonMessageEditorMove::newCopy(redo);
 if (!m) {
	boError() << k_funcinfo << "unable to create a copy of message " << redo.messageId() << endl;
	return;
 }
 d->mRedoStack.push(m);

 emit signalEditorHasRedo(messageName(d->mRedoStack.top()));
}

void EditorViewInput::undo()
{
 BO_CHECK_NULL_RET(localPlayerInput());
 BosonMessageEditorMove* message = d->mUndoStack.pop();
 if (!message) {
	boWarning() << k_funcinfo << "no message on stack" << endl;
	return;
 }
 message->setUndo();

 QByteArray b;
 QDataStream stream(b, IO_WriteOnly);
 if (!message->save(stream)) {
	boError() << k_funcinfo << "unable to save message (" << message->messageId() << ")" << endl;
	delete message;
	return;
 }

 delete message;

 QDataStream msg(b, IO_ReadOnly);
 localPlayerInput()->sendInput(msg);

 emit signalEditorHasUndo(messageName(d->mUndoStack.top()));
}

void EditorViewInput::redo()
{
 BO_CHECK_NULL_RET(localPlayerInput());
 BosonMessageEditorMove* message = d->mRedoStack.pop();
 if (!message) {
	boWarning() << k_funcinfo << "no message on stack" << endl;
	return;
 }
 message->setRedo();

 QByteArray b;
 QDataStream stream(b, IO_WriteOnly);
 if (!message->save(stream)) {
	boError() << k_funcinfo << "unable to save message (" << message->messageId() << ")" << endl;
	delete message;
	return;
 }

 delete message;

 QDataStream msg(b, IO_ReadOnly);
 localPlayerInput()->sendInput(msg);

 emit signalEditorHasRedo(messageName(d->mRedoStack.top()));
}

QString EditorViewInput::messageName(const BosonMessageEditorMove* message) const
{
 if (!message) {
	return QString::null;
 }
 QString name;
 switch (message->messageId()) {
	case BosonMessageIds::MovePlaceUnit: // redo
	{
		BosonMessageEditorMovePlaceUnit* m = (BosonMessageEditorMovePlaceUnit*)message;
		QString type = i18n("Unknown");
		PlayerIO* p = boGame->findPlayerIO(m->mOwner);
		if (p) {
			const UnitProperties* prop = p->unitProperties(m->mUnitType);
			if (prop) {
				type = prop->name();
			}
		}
		name = i18n("Place unit (%1)").arg(type);
		break;
	}
	case BosonMessageIds::MoveUndoPlaceUnit:
	{
		BosonMessageEditorMoveUndoPlaceUnit* m = (BosonMessageEditorMoveUndoPlaceUnit*)message;
		QString type = i18n("Unknown");
		PlayerIO* p = boGame->findPlayerIO(m->mMessage.mOwner);
		if (p) {
			const UnitProperties* prop = p->unitProperties(m->mMessage.mUnitType);
			if (prop) {
				type = prop->name();
			}
		}
		QString id = i18n("Unknown");
		if (m->mDeleteUnit.mItems.count() > 0) {
			id = QString::number(m->mDeleteUnit.mItems[0]);
		}
		name = i18n("Place unit (%1 , ID %2)").arg(type).arg(id);
		break;
	}
	case BosonMessageIds::MoveDeleteItems: // redo
	{
		BosonMessageEditorMoveDeleteItems* m = (BosonMessageEditorMoveDeleteItems*)message;
		QString count = QString::number(m->mItems.count());
		name = i18n("Delete %1 items").arg(count);
		break;
	}
	case BosonMessageIds::MoveUndoDeleteItems:
	{
		BosonMessageEditorMoveUndoDeleteItems* m = (BosonMessageEditorMoveUndoDeleteItems*)message;
		QString count = QString::number(m->mMessage.mItems.count());
		name = i18n("Delete %1 items").arg(count);
		break;
	}
	case BosonMessageIds::MoveChangeHeight: // redo
	{
		BosonMessageEditorMoveChangeHeight* m = (BosonMessageEditorMoveChangeHeight*)message;
		QString count = QString::number(m->mCellCornersX.count());
		name = i18n("Change height of %1 corners").arg(count);
		break;
	}
	case BosonMessageIds::MoveUndoChangeHeight:
	{
		BosonMessageEditorMoveUndoChangeHeight* m = (BosonMessageEditorMoveUndoChangeHeight*)message;
		QString count = QString::number(m->mOriginalHeights.mCellCornersX.count());
		name = i18n("Change height of %1 corners").arg(count);
		break;
	}
	default:
		name = i18n("(Unknown - ID %1)").arg(message->messageId());
		break;
 }
 return name;
}

