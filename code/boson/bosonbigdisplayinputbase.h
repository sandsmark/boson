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
#ifndef BOSONBIGDISPLAYINPUTBASE_H
#define BOSONBIGDISPLAYINPUTBASE_H

#include "global.h"

#include <qobject.h>

class BosonBigDisplayBase;
class BoSelection;
class BosonCanvas;
class BosonCollisions;
class BoAction;
class Player;
class Unit;
class UnitProperties;
class BoItemList;
class BoVector3;

template<class T> class QPtrList;

class BosonBigDisplayInputBase : public QObject
{
	Q_OBJECT
public:
	BosonBigDisplayInputBase(BosonBigDisplayBase* parent);
	virtual ~BosonBigDisplayInputBase();

	BosonBigDisplayBase* bigDisplay() const { return mBigDisplay; }
	BoSelection* selection() const;
	BosonCanvas* canvas() const;
	BosonCollisions* collisions() const;
	Player* localPlayer() const;
	const QPoint& cursorCanvasPos() const;
	const BoVector3& cursorCanvasVector() const;

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
	}
	void lockAction()
	{
		mActionLocked = true;
		emit signalLockAction(mActionLocked);
	}

	void setCursorType(CursorType type) { mCursorType = type; }
	CursorType cursorType() const { return mCursorType; }

	/**
	 * Called when the user right-clicks on the big display.
	 *
	 * Should e.g. move a unit
	 * @param action Contains information about the mouse event (position,
	 * additional buttons, ...)
	 * @param stream The move should be placed here. A move should
	 * <em>not</em> be done in this method but rather sent to @ref KGame
	 * which performs the move on every client
	 * @param send Set to true if you actually want to send the stream
	 **/
	virtual void actionClicked(const BoAction& action, QDataStream& stream, bool* send) = 0;

	virtual void unitAction(int actionType) = 0;

	/**
	 * Called when the placement preview should get updated. Note that you
	 * need to check whether a facility is actually selected and that it can
	 * actually place a unit.
	 *
	 * Note that this gets called (at least) whenever the mouse is moved, so
	 * don't do expensive calculations here.
	 **/
	virtual void updatePlacementPreviewData() = 0;

	virtual void updateCursor() = 0;

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
	 * See @ref EditorBigDisplayInput::placeUnit
	 **/
	virtual void placeUnit(unsigned long int unitType, Player* owner) { Q_UNUSED(unitType); Q_UNUSED(owner); }

	/**
	 * See @ref EditorBigDisplayInput::placeGround
	 **/
	virtual void placeGround(unsigned int textureCount, unsigned char* alpha)
	{
		Q_UNUSED(textureCount);
		Q_UNUSED(alpha);
	}

	/**
	 * See @ref EditorBigDisplayInput::deleteSelectedUnits
	 **/
	virtual void deleteSelectedUnits() { }

signals:
	/**
	 * Emitted when the action gets locked or unlocked (see @ref lockAction
	 * and @ref unlockAction). You may want to display the fact that the
	 * action is locked now somewhere.
	 **/
	void signalLockAction(bool locked);

public slots:
	/**
	 * Called when the player clicks RMB on the minimap. in game mode the
	 * selected units should get moved there.
	 **/
	virtual void slotMoveSelection(int x, int y) { Q_UNUSED(x); Q_UNUSED(y); }

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
	BosonBigDisplayBase* mBigDisplay;
	bool mActionLocked;
	UnitAction mActionType;
	CursorType mCursorType;
};

#endif

