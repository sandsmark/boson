/*
    This file is part of the Boson game
    Copyright (C) 1999-2000 Thomas Capricelli (capricel@email.enst.fr)
    Copyright (C) 2001-2005 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef PLAYER_H
#define PLAYER_H

#include <kplayer.h>
#include <kgameproperty.h>
#include "../math/bofixed.h"

#ifdef NO_PLAYER_H_HERE
#error No player.h include is allowed here
#endif

class QColor;
class QDomElement;
class QTextStream;

class Unit;
class SpeciesTheme;
class UnitProperties;
class BosonMap;
class BosonStatistics;
class ProductionPlugin;
class PlayerIO;
class UpgradeProperties;

class PlayerPrivate;
/**
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class Player : public KPlayer
{
	Q_OBJECT
public:
	enum PropertyIds {
		IdFogged = KGamePropertyBase::IdUser + 1,
		IdMinerals = KGamePropertyBase::IdUser + 2,
		IdOil = KGamePropertyBase::IdUser + 3,
		IdIsNeutralPlayer = KGamePropertyBase::IdUser + 4,
		IdOutOfGame = KGamePropertyBase::IdUser + 5,
		IdHasLost = KGamePropertyBase::IdUser + 6,
		IdHasWon = KGamePropertyBase::IdUser + 7
	};

	/**
	 * @param neutral TRUE if this is meant to be the neutral player. There
	 * is only exactly one neutral player per game!
	 **/
	Player(bool neutral = false);
	virtual ~Player();

	int bosonId() const;

	/**
	 * A "game player" is a player that may own units and move units.
	 *
	 * This can be both, a human controllable unit and also neutral players.
	 *
	 * See also http://boson.freehackers.org/wiki/Main/PlayerIds
	 **/
	bool isGamePlayer() const;

	/**
	 * A "active game player" is a player that is relevant for winning
	 * conditions. A neutral player is never a "active" game player.
	 *
	 * A "active game player" is always a @ref isGamePlayer
	 *
	 * See also http://boson.freehackers.org/wiki/Main/PlayerIds
	 **/
	bool isActiveGamePlayer() const;

	/**
	 * @return TRUE if this is the neutral player. Note that the neutral
	 * player is always at (Boson::playerList()->count() - 1), i.e. it is
	 * always the last player in the list of players. You should prefer the
	 * player index if possible. This method is used to ensure that if there
	 * is a last player in the list it actually is the neutral player.
	 **/
	bool isNeutralPlayer() const;

	PlayerIO* playerIO() const;

	BosonMap* map() const;

	/**
	 * Quit the current game for this player. This will reset some variables
	 * so that the player can play on a new map.
	 *
	 * Note that this does <em>not</em> delete e.g. the species theme of the
	 * player!
	 *
	 * Currently players are deleted for a new map anyway, but maybe this
	 * behavior will change one day.
	 * @param destruct TRUE is relevant for the destructor only. Then
	 * deleted objects won't be new'ed again.
	 **/
	void quitGame(bool destruct = false);

	void loadTheme(const QString& species, const QColor& teamColor);

	/**
	 * Add @p unit to this player.
	 * @param dataHandlerId Used for loading only. The datahandler ID is
	 * already known, so use that value. If -1 we will use the next
	 * integer value.
	 **/
	void addUnit(Unit* unit, int datHandlerId = -1);
	void unitDestroyed(Unit* unit);
	SpeciesTheme* speciesTheme() const { return mSpecies; }

	/**
	 * Convenience method for ((Boson*)game()->advanceFlag
	 **/
	bool advanceFlag() const;

	/**
	 * @return @ref SpeciesTheme::teamColor
	 **/
	const QColor& teamColor() const;

	Unit* findUnit(unsigned long int unitId) const;

	bool saveAsXML(QDomElement& element);
	bool loadFromXML(const QDomElement& element);

	virtual bool load(QDataStream& stream);
	virtual bool save(QDataStream& stream);

	/**
	 * @return <em>All</em> units of this player. Please don't use this as
	 * it is very unclean. This is meant for KGameUnitDebug only.
	 **/
	QPtrList<Unit>* allUnits() const;

	/**
	 * Convenience method for theme()->unitProperties()
	 **/
	const UnitProperties* unitProperties(unsigned long int unitType) const;

	void explore(int x, int y);
	void unexplore(int x, int y);

	void addFogRef(int x, int y);
	void removeFogRef(int x, int y);

	/**
	 * @return Whether the coordinates @p cellX, @p cellY are explored for
	 * this player.
	 * Explored means that player has seen this cell, so he can see the terrain,
	 * but this doesn't mean he can see enemy units (@ref isFogged determines
	 * that)
	 **/
	bool isExplored(int cellX, int cellY) const;

	/**
	 * @param x Position of the cell in <em>Cell</em>-coordinates
	 * @param y Position of the cell in <em>Cell</em>-coordinates
	 * @return Whether the map is fogged for this player at the
	 * (cell)-coordinates x,y.
	 **/
	bool isFogged(int x, int y) const;

	/**
	 * @return How many cells are currently fogged for this player
	 **/
	unsigned int unfoggedCells() const;
  unsigned int exploredCells() const;

	unsigned long int minerals() const;
	unsigned long int oil() const;
	void setMinerals(unsigned long int m);
	void setOil(unsigned long int o);

	/**
	 * Use up given amount of player's minerals.
	 * If player currently has less minerals than @p amount, FALSE is returned
	 * immediately. Otherwise, player's amount of minerals is decreased by the
	 * @p amount and TRUE is returned.
	 **/
	bool useMinerals(unsigned long int amount);
	/**
	 * Use up given amount of player's oil.
	 * If player currently has less oil than @p amount, FALSE is returned
	 * immediately. Otherwise, player's amount of oil is decreased by the
	 * @p amount and TRUE is returned.
	 **/
	bool useOil(unsigned long int amount);
	/**
	 * Use up given amount of player's minerals and oil.
	 * This is same as calling both @ref useMinerals and @ref useOil except that
	 * either both or none of the resources is decreased.
	 **/
	bool useResources(unsigned long int mineralamount, unsigned long int oilamount);

	/**
	 * @return The amount of ammunition of type @p type in the "global
	 * pool", i.e. the amount of ammunition that ist "just there" and can be
	 * used by units from everywhere (they don't need to go to a certain
	 * pouint and pick it up).
	 **/
	unsigned long int ammunition(const QString& type) const;
	void setAmmunition(const QString& type, unsigned long int a);

	/**
	 * @return A number between 0 and @p requested that represents the
	 * amount of ammo that is delivered to the caller. The ammunition of the
	 * player is reduced by this amount.
	 **/
	unsigned long int requestAmmunition(const QString& type, unsigned long int requested);

	void clearUpgrades();
	void addUpgrade(const UpgradeProperties* upgrade);
	void removeUpgrade(const UpgradeProperties* upgrade);
	void removeUpgrade(unsigned long int id);
	const QValueList<const UpgradeProperties*>* upgrades() const;

	const UpgradeProperties* technologyProperties(unsigned long int type) const;

	const QValueList<const Unit*>* radarUnits() const;
	void addRadar(Unit* u);
	void removeRadar(Unit* u);

	/**
	 * Initialize the map for this player - this is mostly the fog of war,
	 * currently.
	 *
	 * Note that this function depends on calling @ref quitGame correctly,
	 * i.e. whenever the map changes.
	 * @param unexplored Whether the map is initially unexplored or not. You can
	 * specify false here for the editor. Note that the fog is <em>not</em>
	 * changed if it was initialized before (i.e. size != 0)! This is
	 * usually the case for loading games.
	 * @param fogged Whether the map is initially fogged. You can specify false
	 * for the editor.
	 **/
	void initMap(BosonMap* map, bool unexplored = true, bool fogged = true);

	/**
	 * Called by @ref Facility when the construction has been completed.
	 * When this facility has some special functions they should be
	 * activated now (e.g. the mini map for the radar station)
	 **/
	void facilityCompleted(Unit* fac);

	/**
	 * @return TRUE if the player can display a minimap (i.e. he has a radar
	 * station), otherwise FALSE.
	 **/
	bool hasMiniMap() const;

	/**
	 * Calculates the amount of power (i.e. electricity) the units of this
	 * player generate and consume.
	 *
	 * If the player consumes more than he generates, then
	 * certain tasks should be slower than usual (or not be functional at
	 * all).
	 *
	 * @param includeUnconstructedFacilities If FALSE (the default) only the
	 * power that is actually generated and consumed is taken into account.
	 * If TRUE, the power generated by facilities that are not yet fully
	 * constructed is included, i.e. the power they will soon generate.
	 **/
	void calculatePower(unsigned long int* powerGenerated = 0, unsigned long int* powerConsumed = 0, bool includeUnconstructedFacilities = false) const;

	/**
	 * Removes the consumed power from the units.
	 *
	 * Should be called once per advance call, @em after the units (items)
	 * were advanced. @em MUST be called exactly once per @ref chargeUnits
	 * call in an advance call.
	 **/
	void unchargeUnitsForAdvance();

	void updatePowerChargeForCurrentAdvanceCall();
	inline bofixed powerChargeForCurrentAdvanceCall() const
	{
		return mPowerChargeForCurrentAdvanceCall;
	}

	/**
	 * @return If the player is already "destroyed", i.e. doesn't have the
	 * right to do anything. Note that this doesn't check if the player has
	 * units left or something, but simply returns what was checked before.
	 * See also @ref setOutOfGame
	 **/
	bool isOutOfGame() const { return mOutOfGame; }

	/**
	 * The @ref hasWon and @ref hasLost flags are usually set at the ond of the
	 * game, as information to the gameover dialog. They may however be set
	 * earlier in the game to indicate that a player has already won (lost)
	 * and cannot lose (win) anymore, no matter what he does. Note that this
	 * means for example, that when the @ref hasWon flag is set, not even
	 * losing all units could cause the player to lose the game. WARNING:
	 * this interpretation of these flags (that they are permanent and
	 * cannot change anymore once set in a game) might change in the future!
	 *
	 * Note that even at the end of the game it is possible that neither
	 * @ref hasWon nor @ref hasLost are set. In that case the player is "in
	 * between", he hasn't accomplished his goals, but hasn't lost the game
	 * either. Winning and losing at once is not possible.
	 *
	 * @return TRUE if the player lost the game. During the game this is
	 * usually not used directly, use @ref isOutOfGame instead.
	 **/
	bool hasLost() const { return mHasLost; }

	/**
	 * See @ref hasLost for a more detailed explanation of @ref hasLost and
	 * @ref hasWon flags.
	 *
	 * @return TRUE if the player has won the game, otherwise FALSE. Note
	 * that this is not necessarily the opposite of @ref hasLost, a player
	 * may have neither won nor lost (however winning and losing is not
	 * possible).
	 **/
	bool hasWon() const { return mHasWon; }

	/**
	 * See @ref hasLost
	 **/
	void setHasLost(bool l)
	{
		mHasLost = l;
		if (hasLost() && hasWon()) {
			// hasLost() has precedence
			mHasWon = false;
		}
	}

	/**
	 * See @ref hasWon and @ref hasLost.
	 **/

	void setHasWon(bool w)
	{
		mHasWon = w;
		if (hasLost()) {
			// hasLost() has precedence
			mHasWon = false;
		}
	}

	/**
	 * This is called by the global @ref BoCanvasEventListener to indicate
	 * that the player has lost the game.
	 *
	 * The concrete moment of when this is called depends on the winning
	 * conditions of the current map.
	 **/
	void setOutOfGame();

	BosonStatistics* statistics() const;

	/**
	 * @return TRUE if player is an enemy or FALSE if it is e.g. allied with
	 * us. See also @ref isNeutral and @ref isAllied
	 **/
	bool isEnemy(const Player* player) const;
	bool isPlayerEnemy(int id) const;

	/**
	 * A player is neutral to another player if it is neither an enemy nor
	 * an allied. When two players are neutral to each other they are meant
	 * not to shoot at each other, but they won't share their resources
	 * (sight range, repair units, refineries, ...) either.
	 * @return TRUE if @p p is neutral to this player, i.e. neither an
	 * allied nor an enemy player, otherwise FALSE. See also @ref isAllied
	 * and @ref isEnemy
	 **/
	bool isNeutral(const Player* p) const;
	bool isPlayerNeutral(int id) const;

	/**
	 * An allied player will share some of their resources, such as sight
	 * ranges of units (they will see the map of each other), maybe
	 * refineries and so on. Also they won't shoot at each other.
	 * @return TRUE if the player @p p is allied with this this player. See
	 * also @ref isEnemy and @ref isNeutral
	 **/
	bool isAllied(const Player* p) const;
	bool isPlayerAllied(int id) const;

	int mobilesCount();
	int facilitiesCount();

	/**
	 * @return TRUE if this player can build units with type unitType, FALSE
	 * otherwise
	 **/
	bool canBuild(unsigned long int unitType) const;

	/**
	 * @return TRUE if this player can research technology with id id, FALSE
	 * otherwise
	 **/
	bool canResearchTech(unsigned long int id) const;

	/**
	 * @return TRUE if player has <em>constructed</em> (if it is a facility) unit
	 * with type type, FALSE otherwise
	 **/
	bool hasUnitWithType(unsigned long int type) const;

	/**
	 * @return TRUE if player has <em>researched</em> technology
	 * with id id, FALSE otherwise
	 **/
	bool hasTechnology(unsigned long int id) const;

	void technologyResearched(ProductionPlugin* factory, unsigned long int id);

	void writeGameLog(QTextStream& log);

	virtual void networkTransmission(QDataStream& stream, int msgid, Q_UINT32 sender);

signals:
	void signalLoadUnit(unsigned long int unitType, unsigned long int id, Player* owner);

	void signalUnitChanged(Unit* unit);

	void signalFog(int x, int y);
	void signalUnfog(int x, int y);
	void signalExplored(int x, int y);
	void signalUnexplored(int x, int y);

public slots:
	/**
	 * Called when a @ref KGameProperty object of a @ref Unit changes.
	 *
	 * WARNING: we assume that this actually <em>is</em> a unit property and
	 * its property handler must be a @ref UnitPropertyHandler!
	 **/
	void slotUnitPropertyChanged(KGamePropertyBase* prop);
	void slotNetworkData(int msgid, const QByteArray& msg, Q_UINT32 sender, KPlayer*);

protected:
	bool saveFogOfWar(QDomElement& root) const;
	bool saveAmmunition(QDomElement& root) const;
	bool loadFogOfWar(const QDomElement& root);
	bool loadAmmunition(const QDomElement& root);

private:
	PlayerPrivate* d;

	SpeciesTheme* mSpecies;
	KGameProperty<Q_UINT8> mOutOfGame;
	KGameProperty<Q_UINT8> mHasLost;
	KGameProperty<Q_UINT8> mHasWon;

	bofixed mPowerChargeForCurrentAdvanceCall; // no need for KGameProperty. this is recalculated every advance call
};

#endif
