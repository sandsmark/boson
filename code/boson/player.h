/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001 The Boson Team (boson-devel@lists.sourceforge.net)

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

#ifdef NO_PLAYER_H_HERE
#error No player.h include is allowed here
#endif

class QColor;
class QDomElement;
class QTextStream;

class Unit;
class Facility;
class SpeciesTheme;
class UnitProperties;
class BosonMap;
class BosonStatistics;
class ProductionPlugin;
class PlayerIO;

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
		IdIsNeutralPlayer = KGamePropertyBase::IdUser + 4
	};

	/**
	 * @param neutral TRUE if this is meant to be the neutral player. There
	 * is only exactly one neutral player per game!
	 **/
	Player(bool neutral = false);
	virtual ~Player();

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

	void fog(int x, int y);
	void unfog(int x, int y);

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
	unsigned int foggedCells() const;

	unsigned long int minerals() const;
	unsigned long int oil() const;
	void setMinerals(unsigned long int m);
	void setOil(unsigned long int o);

	/**
	 * Initialize the map for this player - this is mostly the fog of war,
	 * currently.
	 *
	 * Note that this function depends on calling @ref quitGame correctly,
	 * i.e. whenever the map changes.
	 * @param fogged Whether the map is fogged initially or not. You can
	 * specify false here for the editor. Note that the fog is <em>not</em>
	 * changed if it was initialized before (i.e. size != 0)! This is
	 * usually the case for loading games.
	 **/
	void initMap(BosonMap* map, bool fogged = true);

	/**
	 * Called by @ref Facility when the construction has been completed.
	 * When this facility has some special functions they should be
	 * activated now (e.g. the mini map for the radar station)
	 **/
	void facilityCompleted(Facility* fac);

	/**
	 * @return TRUE if the player can display a minimap (i.e. he has a radar
	 * station), otherwise FALSE.
	 **/
	bool hasMiniMap() const;

	/**
	 * @return If the player is already "destroyed", i.e. doesn't have the
	 * right to do anything. Note that this doesn't check if the player has
	 * units left or something, but simply returns what was checked before.
	 * See also @ref setOutOfGame
	 **/
	bool isOutOfGame() const { return mOutOfGame; }

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

	/**
	 * An allied player will share some of their resources, such as sight
	 * ranges of units (they will see the map of each other), maybe
	 * refineries and so on. Also they won't shoot at each other.
	 * @return TRUE if the player @p p is allied with this this player. See
	 * also @ref isEnemy and @ref isNeutral
	 **/
	bool isAllied(const Player* p) const;

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

	void emitSignalShowMiniMap(bool show);

signals:
	void signalLoadUnit(unsigned long int unitType, unsigned long int id, Player* owner);

	void signalUnitChanged(Unit* unit);

	void signalFog(int x, int y);
	void signalUnfog(int x, int y);

	void signalShowMiniMap(bool show);

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
	bool loadFogOfWar(const QDomElement& root);

private:
	class PlayerPrivate;
	PlayerPrivate* d;

	SpeciesTheme* mSpecies;
	bool mOutOfGame;
};

#endif
