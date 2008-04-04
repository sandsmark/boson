/*
    This file is part of the Boson game
    Copyright (C) 2002-2006 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2002-2006 Rivo Laks (rivolaks@hot.ee)

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
#ifndef BOSONGAMEVIEWINPUT_H
#define BOSONGAMEVIEWINPUT_H

#include "bosongameviewinputbase.h"

class BoSelection;
class BoSpecificAction;

class BosonGameViewInput : public BosonGameViewInputBase
{
	Q_OBJECT
public:
	BosonGameViewInput();
	~BosonGameViewInput();

	virtual void actionClicked(const BoMouseEvent& event);
	virtual void updatePlacementPreviewData();
	virtual void action(const BoSpecificAction& action);
	virtual void updateCursor();

	virtual bool selectAll(const UnitProperties* prop, bool replace);

public slots:
	virtual void slotMoveSelection(int cellX, int cellY);

protected:
	//AB: use BoMouseEvent as 1st parameter
	bool actionMoveWithAttack(const BoVector3Fixed& pos);
	bool actionMoveWithoutAttack(const BoVector3Fixed& pos);
	bool actionAttack(Unit* target);
	bool actionDropBomb(const BoVector3Fixed& pos);
	bool actionBuild(const BoVector3Fixed& pos);
	bool actionFollow(Unit* unit);
	bool actionRepair(Unit* repairYard);
	bool actionRefine(Unit* refinery);
	bool actionHarvest(Unit* resourceMine);
	bool actionEnterUnit(Unit* target);

	CanSelectUnit canSelect(Unit* unit) const;

private:
	int weaponId;
};

#endif

