/*
    This file is part of the Boson game
    Copyright (C) 1999-2000 Thomas Capricelli (capricel@email.enst.fr)
    Copyright (C) 2001-2005 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2001-2005 Rivo Laks (rivolaks@hot.ee)

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef UNITPROPERTIES_H
#define UNITPROPERTIES_H

#include "../global.h"
#include "../bomath.h"

#include "boupgradeableproperty.h"
//Added by qt3to4:
#include <Q3CString>
#include <Q3ValueList>
#include <Q3PtrList>

class SpeciesTheme;
class PluginProperties;
class BosonWeaponProperties;
template<class T> class Q3ValueList;
template<class T> class Q3PtrList;
template<class T> class Q3IntDict;
template<class T1, class T2> class QMap;
template<class T> class BoVector3;
typedef BoVector3<bofixed> BoVector3Fixed;

class KSimpleConfig;


/**
 *
 **/
class BosonMoveData
{
public:
	BosonMoveData()
	{
		id = 0;
		type = Land;
		maxSlope = 30;
		waterDepth = 0.25;
		size = 1;
		crushDamage = 0;
		edgedist1 = 0;
		edgedist2 = 0;
		cellPassable = 0;
	}
	~BosonMoveData()
	{
		delete[] cellPassable;
	}

	enum Type { Land = 1, Water = 2 };

	// Type of the unit
	Type type;
	// Unit can move on cells with slope <= maxSlope
	bofixed maxSlope;
	// Max water depth for land units, min water depth for water units
	bofixed waterDepth;
	// Size of the unit (in cells)
	int size;
	// Unit can crush units with maxHealth <= crushDamage
	unsigned int crushDamage;

	int edgedist1;
	int edgedist2;

	// Internal id of the movedata
	unsigned int id;
	// Whether or not a cell is passable for units which use this movedata
	// Note that this does NOT take unit size into account!
	// TODO: maybe it should take unit size into account?
	bool* cellPassable;
};


class UnitPropertiesPrivate;
/**
 * Represents the config file of a unit. This holds all information about unit
 * type, such as unit's maximum health, damage it causes or armor it has.
 *
 * See themes/species/human/units/README in the data directory for infos.
 *
 * Note that the entire class is <em>not</em> memory critical in any way. You
 * can add as many member variables as necessary - there is always only a single
 * instance of this class per unit type.
 *
 * There are also only very few methods which are speed critical. @ref
 * health might be such a method, since it is used as max health in @ref Unit.
 * But most other methods are used only once to initialize a method.
 *
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class UnitProperties : public BoBaseValueCollection
{
public:
	enum TerrainType {
		TerrainLand = 1,
		TerrainWater = 2,
		TerrainAirPlane = 4,
		TerrainAirHelicopter = 8,


		TerrainMaskAir = (TerrainAirPlane | TerrainAirHelicopter)
	};

public:
	/**
	 * Which type of factory can produce a unit.
	 *
	 * Note that the names (e.g. WarFactory) have <em>nothin</em> to do with
	 * the actual factory names! I.e. other facilities can also be a
	 * producer called "WarFactory", too!
	 **/
	enum Producer {
		ProducerWarFactory = 0,
		ProducerShipyard = 1,
		ProducerAirport = 2, // grrr - I have no good idea for a name... "airport" is definitely wrong as a producer name
		ProducerBarracks = 3,
		ProducerCommandBunker = 10
	};

	UnitProperties(SpeciesTheme*);
	~UnitProperties();


	/**
	 * Load the file. This sets all values of UnitProperties. All values are
	 * readOnly, as UnitProperties is meant to change never.
	 *
	 * The file should contain units/your_unit_dir/index.unit at the end
	 * and should be an absolute path.
	 **/
	bool loadUnitType(const QString& fileName);

	/**
	 * @return The @ref SpeciesTheme this property belongs to.
	 **/
	SpeciesTheme* theme() const { return mTheme; }

	/**
	 * @return The MD5 sum of the file this UnitProperties object was loaded
	 * from. Note that when @ref loadUnitType was not called, the md5 sum is
	 * empty (null).
	 **/
	const Q3CString& md5() const;

	/**
	 * @return The unrotated width of the unit. The value is number of cells
	 * this unit occupies
	 **/
	const bofixed& unitWidth() const { return mUnitWidth; }

	/**
	 * @return The unrotated height of the unit. The value is number of cells
	 * this unit occupies
	 **/
	const bofixed& unitHeight() const { return mUnitHeight; }

	/**
	 * @return The unrotated height in z-direction of the unit.
	 **/
	const bofixed& unitDepth() const { return mUnitDepth; }

	/**
	 * @return Maximum health of units of this type. Note that for a
	 * particular @ref Unit object of this type, this value might be larger,
	 * depending on the upgrades applied to that unit.
	 *
	 * This value applies to newly produced units only.
	 **/
	unsigned long int maxHealth() const { return mHealth.value(upgradesCollection()); }

	/**
	 * @return Maximum shields of units of this type. Note that for a
	 * particular @ref Unit object of this type, this value might be larger,
	 * depending on the upgrades applied to that unit.
	 *
	 * This value applies to newly produced units only.
	 **/
	unsigned long int maxShields() const { return mShields.value(upgradesCollection()); }

	/**
	 * @return Maximum armor of units of this type. Note that for a
	 * particular @ref Unit object of this type, this value might be larger,
	 * depending on the upgrades applied to that unit.
	 *
	 * This value applies to newly produced units only.
	 **/
	unsigned long int maxArmor() const { return mArmor.value(upgradesCollection()); }

	/**
	 * @return How much this unit costs (of your mineral account).
	 **/
	unsigned long int mineralCost() const { return mMineralCost.value(upgradesCollection()); }

	/**
	 * @return How much this unit costs (of your oil account)
	 **/
	unsigned long int oilCost() const { return mOilCost.value(upgradesCollection()); }

	unsigned long int powerConsumed() const { return mPowerConsumed.value(upgradesCollection()); }
	unsigned long int powerGenerated() const { return mPowerGenerated.value(upgradesCollection()); }

	/**
	 * @return Maximum sight range of units of this type. Note that for a
	 * particular @ref Unit object of this type, this value might be larger,
	 * depending on the upgrades applied to that unit.
	 *
	 * This value applies to newly produced units only.
	 **/
	unsigned int maxSightRange() const { return mSightRange.value(upgradesCollection()); }


	/**
	 * @return The Type ID of the unit. This ID is unique for this
	 * UnitProperties. There is no other unit with the same type ID. Note
	 * that you can construct several units of the same type ID in a game -
	 * they will all be of the same type (e.g. they are all ships).
	 * 0 means invalid typeId.
	 **/
	unsigned long int typeId() const { return mTypeId; }

	/**
	 * @return The name of this unit type. Examples are "Aircraft", "Quad",
	 * "Ship"
	 **/
	const QString& name() const;

	/**
	 * @return Longer description of this unit type. This might e.g. say what
	 * the unit is good for.
	 **/
	const QString& description() const;

	/**
	 * @return If this is a mobile unit. Better use @ref Unit::isMobile()
	 **/
	bool isMobile() const { return !mIsFacility; }

	/**
	 * @return If this is a facility. Better use @ref Unit::isFacility()
	 **/
	bool isFacility() const { return mIsFacility; }

	/**
	 * @return Maximum speed of units of this type. Note that for a
	 * particular @ref Unit object of this type, this value might be larger,
	 * depending on the upgrades applied to that unit.
	 *
	 * This value applies to newly produced units only.
	 **/
	bofixed maxSpeed() const;

	/**
	 * @return How fast this mobile unit accelerates. 0 if this is a facility. See
	 * @ref isFacility
	 **/
	bofixed maxAccelerationSpeed() const;

	/**
	 * @return How fast this mobile unit decelerates. 0 if this is a facility. See
	 * @ref isFacility
	 **/
	bofixed maxDecelerationSpeed() const;

	/**
	 * @return Turning speed of this mobile unit (degrees per advance call). 0 if
	 * this is a facility. See @ref isFacility
	 **/
	bofixed rotationSpeed() const;

	/**
	 * @return Whether this unit can move like a helicopter.
	 * Work only for aircrafts
	 **/
	bool isHelicopter() const;
	/**
	 * @return Preferred flying altitude for flying units
	 **/
	bofixed preferredAltitude() const;

	/**
	 * @return Mobile unit's crushDamage.
	 * If @ref maxHealth of some other unit is less than that, then this unit can
	 *  crush it (read: drive over it and destory it)
	 **/
	unsigned int crushDamage() const { return mCrushDamage; }

	/**
	 * @return Max slope (in degrees) that this mobile unit can climb
	 **/
	bofixed maxSlope() const { return mMaxSlope; }

	/**
	 * @return Max water depth in which the unit can move for mobile land units
	 * and min water depth for ships.
	 **/
	bofixed waterDepth() const { return mWaterDepth; }

	/**
	 * @return Whether this unit can go over land
	 **/
	bool canGoOnLand() const; // FIXME is there a shorter and better name?

	/**
	 * @return Whether this unit can go on water - currently only ships.
	 **/
	bool canGoOnWater() const;

	/**
	 * @return Whether this is an aircraft unit.
	 **/
	bool isAircraft() const { return (mTerrain & TerrainMaskAir); }

	/**
	 * @return Whether this is a ship
	 **/
	bool isShip() const { return (mTerrain & TerrainWater); }

	/**
	 * @return Whether this is a land unit.
	 **/
	bool isLand() const { return (mTerrain & TerrainLand); }

	/**
	 * @return Whether this unit can shoot at aircrafts.
	 **/
	bool canShootAtAirUnits() const { return mCanShootAtAirUnits; }

	/**
	 * @return Whether this unit can shoot at land units
	 **/
	bool canShootAtLandUnits() const { return mCanShootAtLandUnits; }

	/**
	 * @return Whether the unit can shoot at all. A unit that can shoot is a
	 * military unit and is meant to be destroyed first.
	 **/
	bool canShoot() const { return (mCanShootAtAirUnits || mCanShootAtLandUnits); }

	/**
	 * @return Which type of factory can produce this unit. See
	 * data/themes/species/human/units/README for more in this.
	 **/
	unsigned int producer() const { return mProducer; }

	/**
	 * @return The path to the unit files. That is the directory where the
	 * index.unit file and the pixmap files are stored.
	 **/
	const QString& unitPath() const;

	/**
	 * The time that a unit needs to be produced
	 *
	 * Note that productionTime is the time that is needed to
	 * <em>build</em> the unit, * so <em>before</em> it is being placed on
	 * the map.
	 *
	 * The production time may be influenced by the facility which produces
	 * the unit and maybe th number of facilities (to name 2 examples).
	 * @return The number of @ref Unit::advance calls this unit needs
	 * (usually) to be produced.
	 **/
	unsigned long int productionTime() const { return mProductionTime.value(upgradesCollection()); }

	/**
	 * @return TRUE if this unittype gives you the ability to show a
	 * minimap, otherwise FALSE.
	 **/
	bool supportMiniMap() const { return mSupportMiniMap; }

	/**
	 * @return the number of frames for the construction animation
	 **/
	unsigned int constructionSteps() const;

	/**
	 * @return list with types of units player must have before he can build unit
	 *  of this type
	 * @see Player::canBuild
	 **/
	Q3ValueList<unsigned long int> requirements() const;

	/**
	 * .3ds files seem to support only filenames of 8+3 length. We work
	 * around this by mapping the "short name" to a longer name.
	 * @return All map of all texture names that have a longer name assigned
	 * to.
	 **/
	QMap<QString, QString> longTextureNames() const;

	const PluginProperties* properties(int pluginType) const;

	/**
	 * @return First part of the sound filename - e.g. "move" if the file
	 * name should be "move_00.ogg". the _00 is added dynamically (randomly)
	 * by @ref BosonSound
	 **/
	QString sound(int soundEvent) const;

	QMap<int, QString> sounds() const;

	const Q3ValueList<unsigned long int>& destroyedEffectIds() const;
	const Q3ValueList<unsigned long int>& constructedEffectIds() const;
	const Q3ValueList<unsigned long int>& explodingFragmentFlyEffectIds() const;
	const Q3ValueList<unsigned long int>& explodingFragmentHitEffectIds() const;

	const Q3PtrList<PluginProperties>* plugins() const;


	/**
	 * @return Damage done by explosion when this unit is destroyed
	 **/
	long int explodingDamage() const { return mExplodingDamage; }
	/**
	 * @return Radius of explosion when this unit is destroyed
	 **/
	const bofixed& explodingDamageRange() const { return mExplodingDamageRange; }

	/**
	 * @return So called hitpoint of this unit
	 * Hitpoint is used to calculate target position for missiles and position for
	 * some effects. Hitpoint is relative to the center of the unit.
	 * It should be a point on the surface of the unit.
	 **/
	 const BoVector3Fixed& hitPoint() const;

	/**
	 * @return BosonWeaponProperties with id id or NULL if they doesn't exist
	 **/
	const BosonWeaponProperties* weaponProperties(unsigned long int id) const;
	BosonWeaponProperties* nonConstWeaponProperties(unsigned long int id) const;

	QString actionString(UnitAction type) const;

	const QMap<int, QString>* allActionStrings() const;

	unsigned int explodingFragmentCount() const { return mExplodingFragmentCount; }
	long int explodingFragmentDamage() const { return mExplodingFragmentDamage; }
	const bofixed& explodingFragmentDamageRange() const { return mExplodingFragmentDamageRange; }

	bool removeWreckageImmediately() const { return mRemoveWreckageImmediately; }

	const BoUpgradesCollection& upgradesCollection() const;

	bool saveUpgradesAsXML(QDomElement& root) const;
	bool loadUpgradesFromXML(const QDomElement& root);

	void clearUpgrades();
	void addUpgrade(const UpgradeProperties* prop);
	void removeUpgrade(const UpgradeProperties* prop);
	void removeUpgrade(unsigned long int id);

	/**
	 * @return Whether unit of this type can go onto cell with given coordinates.
	 * Note that this method takes only terrain into account, it doesn't care
	 *  about units or fog of war.
	 **/
	bool canGo(int x, int y);

protected:
	bool loadActions(KSimpleConfig* conf);
	bool loadMobileProperties(KSimpleConfig* conf);
	bool loadFacilityProperties(KSimpleConfig* conf);
	bool loadAllPluginProperties(KSimpleConfig* conf);
	bool loadPluginProperties(PluginProperties* prop, KSimpleConfig* conf);

	bool loadTextureNames(KSimpleConfig* conf);
	bool loadSoundNames(KSimpleConfig* conf);
	bool loadWeapons(KSimpleConfig* conf);

	friend class BoUnitEditor;

private:
	void init();

protected:
	class MobileProperties;
	class FacilityProperties;

protected:
	UnitPropertiesPrivate* d;
	SpeciesTheme* mTheme;

	unsigned long int mTypeId; // note: 0 is invalid!
	bofixed mUnitWidth;
	bofixed mUnitHeight;
	bofixed mUnitDepth;
	unsigned int mProducer;
	TerrainType mTerrain;
	bool mSupportMiniMap;
	bool mCanShootAtAirUnits;
	bool mCanShootAtLandUnits;
	BoUpgradeableProperty<unsigned long int> mHealth;
	BoUpgradeableProperty<unsigned long int> mSightRange;
	BoUpgradeableProperty<unsigned long int> mProductionTime;
	BoUpgradeableProperty<unsigned long int> mMineralCost;
	BoUpgradeableProperty<unsigned long int> mOilCost;
	BoUpgradeableProperty<unsigned long int> mArmor;
	BoUpgradeableProperty<unsigned long int> mShields;
	BoUpgradeableProperty<unsigned long int> mPowerConsumed;
	BoUpgradeableProperty<unsigned long int> mPowerGenerated;
	long int mExplodingDamage;
	bofixed mExplodingDamageRange;
	unsigned int mExplodingFragmentCount;
	long int mExplodingFragmentDamage;
	bofixed mExplodingFragmentDamageRange;
	bool mRemoveWreckageImmediately;

	// for mobile units only
	BoUpgradeableProperty<bofixed> mSpeed;

	BoUpgradeableProperty<bofixed> mAccelerationSpeed;
	BoUpgradeableProperty<bofixed> mDecelerationSpeed;

	bofixed mRotationSpeed;
	bofixed mPreferredAltitude;
	bool mCanGoOnLand;
	bool mCanGoOnWater;
	unsigned int mCrushDamage;
	bofixed mMaxSlope;
	bofixed mWaterDepth;

	// for facilities only
	unsigned int mConstructionFrames;

	bool mIsFacility;
	MobileProperties* mMobileProperties;
	FacilityProperties* mFacilityProperties;
};

#endif
