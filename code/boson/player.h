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
#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <kgame/kplayer.h>

class QCanvasPixmapArray;
class QColor;
class Unit;
class SpeciesTheme;
class UnitProperties;
class BosonMap;

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

	/**
	 * @return A @ref QCanvasPixmapArray for the specified unit (see @ref
	 * UnitProperties::typeId) of the species of this player
	 **/
	QCanvasPixmapArray* pixmapArray(int unitType) const;

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
	 * Called by @ref Unit. This informs a player (and only this player, so
	 * the owner of the factory) that the production of a unit has advanced.
	 *
	 * This just emits @ref signalProductionAdvanced
	 **/
	void productionAdvanced(Unit* factory, double percentage);

signals:
	void signalLoadUnit(int unitType, unsigned long int id, Player* owner);

	void signalUnitChanged(Unit* unit);

	void signalUnitPropertyChanged(KGamePropertyBase*);

	void signalFog(int x, int y);
	void signalUnfog(int x, int y);

	/**
	 * A production has advanced by one step. You might want to use this to
	 * give some visual feedback to the player (see @ref
	 * BosonCommandWidget).
	 *
	 * Note that this is probably called several times per second, so it is
	 * surely a bad idea (tm) to give visual feedback on every call of this
	 * signal.
	 **/
	void signalProductionAdvanced(Unit* factory, double percentage);

public slots:
	void slotUnitPropertyChanged(KGamePropertyBase* prop);
	void slotNetworkData(int msgid, const QByteArray& msg, Q_UINT32 sender, KPlayer*);

private:
	class PlayerPrivate;
	PlayerPrivate* d;

	SpeciesTheme* mSpecies;
};

#endif
