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

#include <kgame/kplayer.h>

class QColor;
class Unit;
class Facility;
class SpeciesTheme;
class UnitProperties;
class BosonMap;
class BosonStatistics;

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
		IdOil = KGamePropertyBase::IdUser + 3
	};
	Player();
	virtual ~Player();

	void loadTheme(const QString& species, const QColor& teamColor);

	void addUnit(Unit* unit);
	void unitDestroyed(Unit* unit);
	SpeciesTheme* speciesTheme() const { return mSpecies; }

	/**
	 * @return @ref SpeciesTheme::teamColor
	 **/
	const QColor& teamColor() const;

	Unit* findUnit(unsigned long int unitId) const;

	virtual bool load(QDataStream& stream);
	virtual bool save(QDataStream& stream);

	/**
	 * @return <em>All</em> units of this player. Please don't use this as
	 * it is very unclean. This is meant for KGameUnitDebug only.
	 **/
	QPtrList<Unit> allUnits() const;

	/**
	 * Convenience method for theme()->unitProperties()
	 **/
	const UnitProperties* unitProperties(int unitType) const;

	void fog(int x, int y);
	void unfog(int x, int y);
	bool isFogged(int x, int y) const;

	unsigned long int minerals() const;
	unsigned long int oil() const;
	void setMinerals(unsigned long int m);
	void setOil(unsigned long int o);

	void initMap(BosonMap* map);

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
	 * Use @ref checkOutOfGame if you need an up-to-date information!
	 **/
	bool isOutOfGame() const { return mOutOfGame; }

	/**
	 * Check if the player still fullfills the scenario conditions. If not
	 * the player will be out of the game - i.e. from this point on @ref
	 * isOutOfGame will always return true.
	 * @return @ref isOutOfGame after it was updated.
	 **/
	bool checkOutOfGame();

	BosonStatistics* statistics() const;

	/**
	 * @return TRUE if player is an enemy or FALSE if it is e.g. allied with
	 * us.
	 **/
	bool isEnemy(Player* player) const;

	int mobilesCount();
	int facilitiesCount();

	/**
	 * @return TRUE if this player can build units with type unitType.
	 * otherwise FALSE
	 **/
	bool canBuild(int unitType) const;

	/**
	 * @return TRUE if player has unit with type type, FALSE otherwise
	 **/
	bool hasUnitWithType(int type) const;

signals:
	void signalLoadUnit(int unitType, unsigned long int id, Player* owner);

	void signalUnitChanged(Unit* unit);

	void signalUnitPropertyChanged(KGamePropertyBase*);

	void signalFog(int x, int y);
	void signalUnfog(int x, int y);

	void signalShowMiniMap(bool show);

	void signalUnitLoaded(Unit* unit, int x, int y);

public slots:
	void slotUnitPropertyChanged(KGamePropertyBase* prop);
	void slotNetworkData(int msgid, const QByteArray& msg, Q_UINT32 sender, KPlayer*);

private:
	class PlayerPrivate;
	PlayerPrivate* d;

	SpeciesTheme* mSpecies;
	bool mOutOfGame;
};

#endif
