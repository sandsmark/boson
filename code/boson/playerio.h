/*
    This file is part of the Boson game
    Copyright (C) 2003 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef PLAYERIO_H
#define PLAYERIO_H

class QColor;
class QPoint;
class Player;
class Cell;
class BosonItem;
class Unit;
class BoVector3;
class BoItemList;
template<class T> class QPtrVector;

class PlayerIOPrivate;

/**
 * This class provides "high-level" access to the data of @ref Player. You are
 * meant to use this class as often as possible instead of @ref Player. Access
 * to @ref Player should be allowed to the very core of boson only, the UI and
 * scripts should access the PlayerIO <em>only</em>.
 *
 * This class provides functions from the perspective of the player, such as
 * e.g. @ref canSee. These functions also keep @ref isFogged in mind and
 * therefore can be used safely inside scripting, without the risk of letting
 * the computer player know more than a human player. You don't have to check
 * for @ref isFogged yourself then.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class PlayerIO
{
public:
	PlayerIO(Player*);
	~PlayerIO();

	Player* player() const { return mPlayer; }

	bool isFogged(int cellX, int cellY) const;
	bool isFogged(Cell* c) const;
	bool canSee(int cellX, int cellY) const
	{
		return !isFogged(cellX, cellY);
	}
	bool canSee(Cell* c) const { return !isFogged(c); }
	bool canSee(BosonItem* item) const;
	bool canSee(const BoVector3& canvasVector) const;

	bool ownsUnit(Unit* unit) const;

	const QColor& teamColor() const;
	unsigned long int minerals() const;
	unsigned long int oil() const;

	QPoint homeBase() const;

	bool canBuild(unsigned long int unitType) const;
	bool canResearchTech(unsigned long int id) const;

	/**
	 * @return The unit at @p canvasVector, if any. This returns any unit
	 * that is visible to this player, also a unit that is not owned by this
	 * player.
	 **/
//	Unit* findUnitAt(const BoVector3& canvasVector) const; // FIXME TODO

	BoItemList* unitsAtCells(const QPtrVector<Cell>* cells) const;

private:
	PlayerIOPrivate* d;
	Player* mPlayer;
};

#endif

