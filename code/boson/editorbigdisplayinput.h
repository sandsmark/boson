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
#ifndef EDITORBIGDISPLAYINPUT_H
#define EDITORBIGDISPLAYINPUT_H

#include "bosonbigdisplayinputbase.h"

class BosonBigDisplayBase;
class BoSelection;
class BoSpecificAction;

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

	virtual void placeGround(unsigned int textureCount, unsigned char* alpha);

	/**
	 * Delete the currently selected units
	 **/
	virtual void deleteSelectedUnits();

	/**
	 * Called when the user right-clicks on the big display.
	 * 
	 * Should e.g. move a unit
	 * @param event Contains information about the mouse event (position,
	 * additional buttons, ...)
	 **/
	virtual void actionClicked(const BoMouseEvent& event);

	virtual void updatePlacementPreviewData();
	virtual void action(BoSpecificAction action);

	/**
	 * In editor mode this does just nothing.
	 **/
	virtual void updateCursor() { }

	virtual bool selectAll(const UnitProperties* prop, bool replace);

public slots:
	virtual void slotMoveSelection(int cellX, int cellY);

protected:
	//AB: use BoMouseEvent as 1st parameter
	bool actionPlace(const BoVector3& pos);
	bool actionChangeHeight(const BoVector3& pos, bool up);

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

