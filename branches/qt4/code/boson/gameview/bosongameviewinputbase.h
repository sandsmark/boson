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
#ifndef BOSONGAMEVIEWINPUTBASE_H
#define BOSONGAMEVIEWINPUTBASE_H

#include "../global.h"

#include <qobject.h>

class BoSelection;
class BosonCanvas;
class BosonCollisions;
class BoMouseEvent;
class Player;
class PlayerIO;
class Unit;
class UnitProperties;
class BoItemList;
class BoSpecificAction;
class BosonLocalPlayerInput;
class BosonCursor;
class bofixed;
template<class T> class BoVector2;
template<class T> class BoVector3;
template<class T> class BoRect2;
typedef BoVector2<bofixed> BoVector2Fixed;
typedef BoVector3<bofixed> BoVector3Fixed;
typedef BoRect2<bofixed> BoRect2Fixed;

template<class T> class QPtrList;

class BosonGameViewInputBase : public QObject
{
	Q_OBJECT
public:
	BosonGameViewInputBase();
	virtual ~BosonGameViewInputBase();

	BoSelection* selection() const;
	const BosonCanvas* canvas() const;
	const BosonCollisions* collisions() const;
	PlayerIO* localPlayerIO() const;
	BosonLocalPlayerInput* localPlayerInput() const;
	BosonCursor* cursor() const;

	void setSelection(BoSelection*);
	void setCursor(BosonCursor*);
	void setCanvas(const BosonCanvas*);
	void setLocalPlayerIO(PlayerIO*);
	void setCursorCanvasVector(const BoVector3Fixed*);

	const BoVector3Fixed& cursorCanvasVector() const;

	void setActionType(UnitAction type) { mActionType = type; }

	/**
	 * @return The currently active @ref UnitAction if @ref actionLocked is
	 * TRUE. Otherwise undefined.
	 **/
	UnitAction actionType() const { return mActionType; }

	bool actionLocked() const { return mActionLocked; }
	void unlockAction()
	{
		mActionLocked = false;
		emit signalLockAction(mActionLocked);
		emit signalLockAction(mActionLocked, ActionInvalid);
	}
	void lockAction()
	{
		mActionLocked = true;
		emit signalLockAction(mActionLocked);
		emit signalLockAction(mActionLocked, actionType());
	}

	void setCursorType(CursorType type) { mCursorType = type; }
	CursorType cursorType() const { return mCursorType; }

	/**
	 * Called when the user right-clicks on the big display.
	 *
	 * Should e.g. move a unit
	 * @param event Contains information about the mouse event (position,
	 * additional buttons, ...)
	 **/
	virtual void actionClicked(const BoMouseEvent& event) = 0;

	virtual void action(const BoSpecificAction& action) = 0;

	/**
	 * Called when the placement preview should get updated. Note that you
	 * need to check whether a facility is actually selected and that it can
	 * actually place a unit.
	 *
	 * Note that this gets called (at least) whenever the mouse is moved, so
	 * don't do expensive calculations here.
	 **/
	virtual void updatePlacementPreviewData() = 0;

	/**
	 * Call this to notify the input, that placements should happen in free
	 * mode (i.e. the position is not aligned to any cell).
	 *
	 * This has an effect only, if @ref updatePlacementPreviewData is called
	 * afterwards and if there is actually a placement in effect (i.e. @ref
	 * actionType is @ref ActionPlacementPreview)
	 **/
	void setPlacementFreePlacement(bool free);
	bool placementFreePlacement() const
	{
		return mPlacementFreePlacement;
	}

	/**
	 * Call this to notify the input, that placements should happen with
	 * collision detection disabled.
	 *
	 * This has an effect only, if @ref updatePlacementPreviewData is called
	 * afterwards and if there is actually a placement in effect (i.e. @ref
	 * actionType is @ref ActionPlacementPreview)
	 **/
	void setPlacementDisableCollisions(bool disable);
	bool placementDisableCollisions() const
	{
		return mPlacementDisableCollisionDetection;
	}

	virtual void updateCursor() = 0;
	void makeCursorInvalid();

	/**
	 * Select a single unit. You should prefer this to a direct @ref
	 * BoSelection::selectUnit
	 **/
	void selectSingle(Unit* unit, bool replace);

	/**
	 * Select all units of the specified type.
	 *
	 * The editor implementation will select <em>all</em> units of this
	 * type, the game implementation only the units of the local player.
	 * @return TRUE if the selection was successful, otherwise FALSE (e.g.
	 * if the unit type is a facility in game mode)
	 **/
	virtual bool selectAll(const UnitProperties* prop, bool replace) = 0;

	/**
	 * Select units in the given rect
	 **/
	void selectArea(BoItemList* itemsInArea, bool replace);

	/**
	 * Deselect units in the given rect
	 **/
	void unselectArea(BoItemList* itemsInArea);

	/**
	 * Select a list of units. You should prefer this to a direct @ref
	 * BoSelection::selectUnits
	 **/
	void selectUnits(QPtrList<Unit>, bool replace);

	/**
	 * See @ref EditorViewInput::placeUnit
	 **/
	virtual void placeUnit(unsigned long int unitType, Player* owner) { Q_UNUSED(unitType); Q_UNUSED(owner); }

	/**
	 * See @ref EditorViewInput::placeGround
	 **/
	virtual void placeGround(unsigned int textureCount, unsigned char* alpha)
	{
		Q_UNUSED(textureCount);
		Q_UNUSED(alpha);
	}

	/**
	 * See @ref EditorViewInput::deleteSelectedUnits
	 **/
	virtual void deleteSelectedUnits() { }

	virtual void undo() { }
	virtual void redo() { }

signals:
	/**
	 * Emitted when the action gets locked or unlocked (see @ref lockAction
	 * and @ref unlockAction). You may want to display the fact that the
	 * action is locked now somewhere.
	 **/
	void signalLockAction(bool locked);

	/**
	 * @overload
	 * This signal is just like the one above, but additionally includes the
	 * @p actionType. The @ref actionType is valid only if @p locked
	 * is TRUE, therefore the @p actionType parameter is always @ref
	 * ActionInvalid if the action is not locked.
	 **/
	void signalLockAction(bool locked, int actionType);

	void signalSetPlacementPreviewData(const UnitProperties* prop, bool canPlace, bool freePlacement, bool collisionDetection);
	void signalSetPlacementCellPreviewData(unsigned int textureData, unsigned char* alpha, bool canPlace);

	void signalEditorHasUndo(const QString&);
	void signalEditorHasRedo(const QString&);


public slots:
	/**
	 * Called when the player clicks RMB on the minimap. in game mode the
	 * selected units should get moved there.
	 **/
	virtual void slotMoveSelection(int x, int y) { Q_UNUSED(x); Q_UNUSED(y); }

	void slotPlaceGround(unsigned int textureCount, unsigned char* alpha)
	{
		placeGround(textureCount, alpha);
	}
	void slotPlaceUnit(unsigned int unitType, Player* owner)
	{
		placeUnit(unitType, owner);
	}

	void slotSetCursor(BosonCursor* c)
	{
		setCursor(c);
	}

protected:
	enum CanSelectUnit {
		CanSelectMultipleOk = 0, // the unit can be selected - multiple selections allowed
		CanSelectSingleOk = 1, // the unit can be selected - only single selection allowed (e.g. for facilities)
		CanSelectDestroyed = 2, // can't be selected - is destroyed
		CanSelectError = 3 // can't be selected - unknown reason
	};

	// AB: this is not 100% clean here...
	// AB: it is! but it should be protected!
	virtual CanSelectUnit canSelect(Unit* unit) const = 0;

private:
	BoSelection* mSelection;
	BosonCursor* mCursor;
	const BosonCanvas* mCanvas;
	PlayerIO* mLocalPlayerIO;
	const BoVector3Fixed* mCursorCanvasVector;
	bool mActionLocked;
	UnitAction mActionType;
	CursorType mCursorType;
	bool mPlacementFreePlacement;
	bool mPlacementDisableCollisionDetection;
};

#endif

