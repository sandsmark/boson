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
class UnitProperties;
class BoVector3;
class BoItemList;
class BosonStatistics;
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

	/**
	 * @return Whether the coordinates @p cellX, @p cellY are fogged for
	 * this player.
	 **/
	bool isFogged(int cellX, int cellY) const;
	/**
	 * @overload
	 **/
	bool isFogged(const Cell* c) const;

	/**
	 * @return The cell at @p x, @p y (see @ref BosonMap::cell) if that cell
	 * is visible, or 0 if the cell is either not visible to this player, or
	 * if @p x, @p y are not a valid cell.
	 * @param If 0 is returned because the @p x, @p y parameters were
	 * incorrect, this is set to FALSE, if non-null. Otherwise (i.e. the
	 * cell is not visible to this player) it is set to TRUE. If it is NULL,
	 * it is simply ignored.
	 **/
	Cell* cell(int x, int y, bool* isValid = 0) const;

	bool isValidCell(int x, int y) const;

	/**
	 * @return Whether the player can see these coordinates. This is equal
	 * to !isFogged(cellX, cellY).
	 **/
	bool canSee(int cellX, int cellY) const
	{
		return !isFogged(cellX, cellY);
	}
	/**
	 * @overload
	 **/
	bool canSee(const Cell* c) const { return !isFogged(c); }
	/**
	 * @overload
	 **/
	bool canSee(const BoVector3& canvasVector) const;
	/**
	 * @overload
	 * This version checks whehter the specified @ref BosonItem can be seen.
	 * An item can be seen when one of the cells it occupies is visible.
	 **/
	bool canSee(BosonItem* item) const;

	/**
	 * @return Whether this player is the owner of @p unit.
	 **/
	bool ownsUnit(const Unit* unit) const;

	bool isEnemy(Player* player) const;
	bool isEnemyUnit(const Unit* unit) const;

	/**
	 * @return @ref Player::teamColor
	 **/
	const QColor& teamColor() const;
	/**
	 * @return @ref Player::minerals
	 **/
	unsigned long int minerals() const;
	/**
	 * @return @ref Player::oil
	 **/
	unsigned long int oil() const;
	/**
	 * @return @ref Player::statistics
	 **/
	BosonStatistics* statistics() const;

	/**
	 * @return The position of the home base (cell coordinates).
	 *
	 * atmn the position of the homebase is equal to the position of the
	 * first unit found.
	 **/
	QPoint homeBase() const;

	bool canBuild(unsigned long int unitType) const;
	bool canResearchTech(unsigned long int id) const;

	/**
	 * @return Whether a unit with @p prop can gto on @p cell. When the
	 * player can not see this cell, @p _default is returned.
	 **/
	bool canGo(const UnitProperties* prop, const Cell* cell, bool _default = false) const;

	/**
	 * @overload
	 **/
	bool canGo(const Unit* unit, const Cell* cell, bool _default = false) const;

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

