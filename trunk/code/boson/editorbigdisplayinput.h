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
#ifndef EDITORBIGDISPLAYINPUT_H
#define EDITORBIGDISPLAYINPUT_H

#include "bosonbigdisplayinputbase.h"

class BosonBigDisplayBase;
class BoSelection;

class EditorBigDisplayInputPrivate;

class EditorBigDisplayInput : public BosonBigDisplayInputBase
{
	Q_OBJECT
public:
	EditorBigDisplayInput(BosonBigDisplayBase* parent);
	~EditorBigDisplayInput();


	//AB: this should generate an error when owner != localPlayer()
	/**
	 * The specified unitType is marked to be placed whenever the user
	 * clicks on the map.
	 **/
	virtual void placeUnit(unsigned long int unitType, Player* owner);

	/**
	 * The specified cell is marked to be placed whenever the user
	 * clicks on the map.
	 **/
	virtual void placeCell(int);

	/**
	 * Delete the currently selected units
	 **/
	virtual void deleteSelectedUnits();

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
	virtual void actionClicked(const BoAction& action, QDataStream& stream, bool* send);

	virtual void updatePlacementPreviewData();
	virtual void unitAction(int);

	/**
	 * In editor mode this does just nothing.
	 **/
	virtual void updateCursor() { }

	virtual bool selectAll(const UnitProperties* prop, bool replace);

public slots:
	virtual void slotMoveSelection(int cellX, int cellY);

protected:
	//AB: use BoAction as 2nd parameter
	bool actionPlace(QDataStream& stream, const BoVector3& pos);
	bool actionChangeHeight(QDataStream& stream, const BoVector3& pos, bool up);

	/**
	 * Editor mode can select just everything, even destroyed units
	 * (otherwise we can't delete them anymore).
	 * @return CanSelectMultipleOk
	 **/
	virtual CanSelectUnit canSelect(Unit* unit) const { Q_UNUSED(unit); return CanSelectMultipleOk; }

private:
	EditorBigDisplayInputPrivate* d;
};

#endif

