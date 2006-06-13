/*
    This file is part of the Boson game
    Copyright (C) 2003-2005 Andreas Beckermann (b_mann@gmx.de)

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

class Boson;
class Player;
class Cell;
class BosonItem;
class Unit;
class UnitProperties;
class BoItemList;
class BosonCanvas;
class BosonStatistics;
class SpeciesTheme;
class KGameIO;
class KGameIOList;
class bofixed;
class UpgradeProperties;
template<class T> class BoVector2;
template<class T> class BoVector3;
typedef BoVector2<bofixed> BoVector2Fixed;
typedef BoVector3<bofixed> BoVector3Fixed;

template<class T> class QPtrVector;
template<class T> class QPtrList;
template<class T> class QValueList;
class QObject;
class QString;
class QColor;
class QPoint;

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
	const QString& name() const;
	unsigned long int playerId() const;

	/**
	 * @return Player::game()
	 **/
	const Boson* game() const;

	/**
	 * @return Boson::canvas() or NULL if @ref game is NULL
	 **/
	const BosonCanvas* canvas() const;

	/**
	 * @return Player::isOutOfGame
	 **/
	bool isOutOfGame() const;

	/**
	 * @return Player::hasWon
	 **/
	bool hasWon() const;

	/**
	 * @return Player::hasLost
	 **/
	bool hasLost() const;

	/**
	 * Connect to the specified signal of the player. Works like any usual
	 * @ref QObject::connect, but the first parameter (the emitter of the
	 * signal) is always the @ref player.
	 **/
	bool connect(const char* signal, const QObject* receiver, const char* member);
	bool disconnect(const char* signal, const QObject* receiver, const char* member);

	/**
	 * @return Player::hasRtti
	 **/
	bool hasRtti(int rtti) const;
	KGameIO* findRttiIO(int rtti) const;
	QPtrList<KGameIO>* ioList();
	bool removeGameIO(KGameIO* io = 0, bool deleteit = true);

	/**
	 * @return Player::addGameIO
	 **/
	bool addGameIO(KGameIO* io);

	/**
	 * @return Player::speciesTheme
	 **/
	SpeciesTheme* speciesTheme() const;

	/**
	 * @return Player::unitProperties
	 **/
	const UnitProperties* unitProperties(unsigned long int type) const;

	/**
	 * @return Player::technologyProperties
	 **/
	const UpgradeProperties* technologyProperties(unsigned long int type) const;


	/**
	 * @return All currently visible units.
	 *
	 * Note that if you need the units of this player only, you should the
	 * much faster @ref allMyUnits instead!
	 **/
	QPtrList<Unit> allUnits() const;
	QPtrList<Unit> allEnemyUnits() const;

	/**
	 * @return Player::allUnits
	 **/
	QPtrList<Unit>* allMyUnits() const;
	QPtrList<Unit> allMyLivingUnits() const;

	/**
	 * @return Whether the coordinates @p cellX, @p cellY are explored for
	 * this player.
	 * Explored means that player has seen this cell, so he can see the terrain,
	 * but this doesn't mean he can see enemy units (@ref isFogged determines
	 * that)
	 **/
	bool isExplored(int cellX, int cellY) const;

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
	 * Like @ref isFogged, but takes a vector of canvas coordinates
	 **/
	bool isFogged(const BoVector3Fixed& canvasVector) const;

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
	bool canSee(const BoVector3Fixed& canvasVector) const
	{
		return !isFogged(canvasVector);
	}

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

	bool hasMiniMap() const;

	bool isEnemy(const Player* player) const;
	bool isPlayerEnemy(int id) const;
	bool isEnemy(const Unit* unit) const;
	bool isNeutral(const Player*) const;
	bool isPlayerNeutral(int id) const;
	bool isNeutral(const Unit*) const;
	bool isAllied(const Player*) const;
	bool isPlayerAllied(int id) const;
	bool isAllied(const Unit*) const;

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
	bool useMinerals(unsigned long int amount);
	bool useOil(unsigned long int amount);
	bool useResources(unsigned long int mineralamount, unsigned long int oilamount);
	/**
	 * @return @ref Player::ammunition
	 **/
	unsigned long int ammunition(const QString& type) const;
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
	bool hasTechnology(unsigned long int id) const;
	bool hasUnitWithType(unsigned long int type) const;

	/**
	 * @return The item at @p canvasVector, if any.
	 **/
	BosonItem* findItemAt(const BoVector3Fixed& canvasVector) const;

	/**
	 * @return The unit at @p canvasVector, if any. This returns any unit
	 * that is visible to this player, also a unit that is not owned by this
	 * player.
	 **/
	Unit* findUnitAt(const BoVector3Fixed& canvasVector) const;
	Unit* findUnit(unsigned long int unitId) const;

	BoItemList* unitsAtCells(const QPtrVector<const Cell>* cells) const;

	/**
	 * @return Player::calculatePower
	 **/
	void calculatePower(unsigned long int* powerGenerated = 0, unsigned long int* powerConsumed = 0, bool includeUnconstructedFacilities = false) const;

	/**
	 * Finds n nearest mineral locations to point (x, y), that are visible
	 * to the player.
	 * At most radius tiles are searched.
	 * If n is 0, all visible mineral mines in given are returned.
	 **/
	QValueList<BoVector2Fixed> nearestMineralLocations(int x, int y, unsigned int n, unsigned int radius) const;

	/**
	 * Finds n nearest oil locations to point (x, y), that are visible to
	 * the player.
	 * At most radius tiles are searched.
	 * If n is 0, all visible oil mines in given are returned.
	 **/
	QValueList<BoVector2Fixed> nearestOilLocations(int x, int y, unsigned int n, unsigned int radius) const;

private:
	PlayerIOPrivate* d;
	Player* mPlayer;
};

#endif

