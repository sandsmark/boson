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
#ifndef BOSONBIGDISPLAYINPUT_H
#define BOSONBIGDISPLAYINPUT_H

#include "bosonbigdisplayinputbase.h"

class BosonBigDisplayBase;
class BoSelection;
class BoVector3;

class BosonBigDisplayInput : public BosonBigDisplayInputBase
{
	Q_OBJECT
public:
	BosonBigDisplayInput(BosonBigDisplayBase* parent);
	~BosonBigDisplayInput();

	virtual void actionClicked(const BoAction& action, QDataStream& stream, bool* send);
	virtual void updatePlacementPreviewData();
	virtual void unitAction(int);
	virtual void updateCursor();

	virtual bool selectAll(const UnitProperties* prop, bool replace);

public slots:
	virtual void slotMoveSelection(int cellX, int cellY);

protected:
	//AB: use BoAction as 2nd parameter
	bool actionMine(QDataStream& stream, const BoVector3& pos);
	bool actionMove(QDataStream& stream, const BoVector3& pos);
	bool actionBuild(QDataStream& stream, const BoVector3& pos);
	bool actionAttack(QDataStream& stream, const BoVector3& pos);
	bool actionAttackPos(QDataStream& stream, const BoVector3& pos);
	bool actionRepair(QDataStream& stream, const BoVector3& pos);
	bool actionRefine(QDataStream& stream, const BoVector3& pos);
	bool actionFollow(QDataStream& stream, const BoVector3& pos);

	CanSelectUnit canSelect(Unit* unit) const;

private:
};

#endif

