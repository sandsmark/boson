/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001 The Boson Team (boson-devel@lists.sourceforge.net)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef __UNITPROPERTIES_H__
#define __UNITPROPERTIES_H__

#include <qstring.h>

#include <qvaluelist.h>

class KSimpleConfig;

/**
 * Represents the config file of a unit. See README of the config file for
 * infos.
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class UnitProperties
{
public:
	/**
	 * Only public cause of the private d-pointer! Will become protected as
	 * soon as the d pointer is removed. Don't use outside this class!
	 **/
	enum TerrainType {
		Land=0,
		Water=1,
		Air=2
	};
	
	UnitProperties();

	/**
	 * @param fileName The filename of the config file for this unit type
	 **/
	UnitProperties(const QString& fileName);
	~UnitProperties();

	/**
	 * @return Default health aka power aka whatever of this unit.
	 **/
	unsigned long int health() const { return mHealth; }

	/**
	 * @return The default shields value of this unit. Not yet used.
	 **/
	unsigned long int shields() const;

	/**
	 * @return The default armor value of this unit. Not yet used.
	 **/
	unsigned long int armor() const;

	/**
	 * @return How much this unit costs (of your mineral account)
	 **/
	unsigned long int mineralCost() const;

	/**
	 * @return How much this unit costs (of your oil account)
	 **/
	unsigned long int oilCost() const;

	/**
	 * @return The weapon range of this unit.
	 **/
	unsigned long int range() const { return mRange; }

	/**
	 * @return The number of advance calls until the weapon is reloaded
	 **/
	unsigned int reload() const { return mReload; }

	/**
	 * return How far this unit can see. Is a number of cells, so multiply
	 * with BO_TILE_SIZE to use it on the canvas.
	 **/
	unsigned int sightRange() const { return mSightRange; }

	/**
	 * The damage this unit makes to other units. Negative values means
	 * repairing
	 **/
	long int damage() const { return mDamage; }
	
	/**
	 * @return The Type ID of the unit. This ID is unique for this
	 * UnitProperties. There is no other unit with the same type ID. Note
	 * that you can construct several units of the same type ID in a game -
	 * they will all be of the same type (e.g. they are all ships).
	 **/
	int typeId() const { return mTypeId; };  // we MUST use int (not unsigned int) as -1 is used for invalid

	/**
	 * @return The name of this unit type. Examples are "Aircraft", "Quad",
	 * "Ship"
	 **/
	const QString& name() const { return mName; };

	/**
	 * Load the file. This sets all values of UnitProperties. All values are
	 * readOnly, as UnitProperties is meant to change never.
	 *
	 * The file should contain units/your_unit_dir/index.desktop at the end
	 * and should be an absolute path.
	 **/
	void loadUnitType(const QString& fileName);

	/**
	 * @return If this is a mobile unit. Better use @ref Unit::isMobile()
	 **/
	bool isMobile() const;

	/**
	 * @return If this is a facility. Better use @ref Unit::isFacility()
	 **/
	bool isFacility() const;

	/**
	 * @return The speed of the mobiel unit. 0 if this is a facility. See
	 * @ref isFacility
	 **/
	double speed() const;

	/**
	 * @return Whether this unit can go over land
	 **/
	bool canGoOnLand() const; // FIXME is there a shorter and better name?

	/**
	 * @return Whether this unit can go on water - currently only ships.
	 **/
	bool canGoOnWater() const;

	/**
	 * @return Whether this is an aircraft unit. Currently there is only
	 * one.
	 **/
	bool isAircraft() const { return mTerrain == Air; }

	/**
	 * @return Whether this is a ship
	 **/
	bool isShip() const { return mTerrain == Water; }

	/**
	 * @return Whether this is a land unit.
	 **/
	bool isLand() const { return mTerrain == Land; }
	
	/**
	 * @return Whether this facility (if it is one) can produce anything.
	 **/
	bool canProduce() const;

	/**
	 * @return A list of units which can be produced by this facility (if
	 * any).
	 **/
	QValueList<int> produceList() const;

	/**
	 * @return The path to the unit files. That is the directory where the
	 * index.desktop file and the pixmap files are stored.
	 **/
	const QString& unitPath() const { return mUnitPath; };

	/**
	 * @return The number of @ref Unit::advance calls to achieve another
	 * construction step. See @ref Facility::constructionSteps
	 **/
	int constructionDelay() const;

	/**
	 * The time that a unit needs to be produced
	 * 
	 * Note that in contrary to @ref constructionDelay which influences the
	 * construction of a building after if was placed on the map the
	 * productionTime is the time that is needed to <em>build</em> the unit,
	 * so <em>before</em> it is being placed on the map.
	 *
	 * The production time may be influenced by the facility which produces
	 * the unit and maybe th number of facilities (to name 2 examples).
	 * @return The number of @ref Unit::advance calls this unit needs 
	 * (usually) to be produced.
	 **/
	unsigned int productionTime() const;

protected:
	void loadMobileProperties(KSimpleConfig* conf);
	void loadFacilityProperties(KSimpleConfig* conf);
	
private:
	class UnitPropertiesPrivate;
	UnitPropertiesPrivate* d;

	QString mName;
	QString mUnitPath; // the path to the unit files
	int mTypeId; // note: -1 is invalid!
	unsigned long int mHealth;
	unsigned long int mRange;
	unsigned int mSightRange;
	long int mDamage;
	unsigned int mReload;
	unsigned int mProductionTime;
	unsigned long int mMineralCost;
	unsigned long int mOilCost;
	TerrainType mTerrain;
};

#endif
