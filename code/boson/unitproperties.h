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

	unsigned long int health() const { return mHealth; }
	unsigned long int shields() const;
	unsigned long int armor() const;
	unsigned long int prize() const;

	/**
	 * @return The weapon range of this unit.
	 **/
	unsigned long int range() const { return mRange; }

	/**
	 * @return The number of advance calls until the weapon is reloaded
	 **/
	unsigned int reload() const { return mReload; }

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

	void loadUnitType(const QString& fileName);

	/**
	 * @return If this is a mobile unit. Better use @ref Unit::isMobile()
	 **/
	bool isMobile() const;

	/**
	 * @return If this is a facility. Better use @ref Unit::isFacility()
	 **/
	bool isFacility() const;

// only if this unit is a mobile unit:
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
	long int mDamage;
	unsigned int mReload;
	TerrainType mTerrain;
};

#endif
