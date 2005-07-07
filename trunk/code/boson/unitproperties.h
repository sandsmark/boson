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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef UNITPROPERTIES_H
#define UNITPROPERTIES_H

#include "global.h"
#include "bomath.h"

#include "boupgradeableproperty.h"

class SpeciesTheme;
class PluginProperties;
class BosonWeaponProperties;
class BosonEffect;
class BosonEffectProperties;
class BoAction;
class QCString;
class QString;
template<class T> class QValueList;
template<class T> class QPtrList;
template<class T> class QIntDict;
template<class T1, class T2> class QMap;
template<class T> class BoVector3;
typedef BoVector3<bofixed> BoVector3Fixed;

class KSimpleConfig;

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
class UnitProperties
{
protected:
	enum TerrainType {
		Land = 0,
		Water = 1,
		Air = 2
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
		WarFactory = Land,
		Shipyard = Water,
		Airport = Air, // grrr - I have no good idea for a name... "airport" is definitely wrong as a producer name
		Barracks = 3,
		CommandBunker = 10
	};

	UnitProperties(SpeciesTheme*, bool fullMode = true);
	~UnitProperties();


	/**
	 * Load the file. This sets all values of UnitProperties. All values are
	 * readOnly, as UnitProperties is meant to change never.
	 *
	 * The file should contain units/your_unit_dir/index.unit at the end
	 * and should be an absolute path.
	 **/
	bool loadUnitType(const QString& fileName, bool fullmode = true);

	/**
	 * Save UnitProperties to the file. This sets all values of UnitProperties. All values are
	 * readOnly, as UnitProperties is meant to change never.
	 *
	 * The file should contain units/your_unit_dir/index.desktop at the end
	 * and should be an absolute path.
	 **/
	void saveUnitType(const QString& fileName);


	/**
	 * @return The @ref SpeciesTheme this property belongs to.
	 **/
	SpeciesTheme* theme() const { return mTheme; }

	/**
	 * @return The MD5 sum of the file this UnitProperties object was loaded
	 * from. Note that when @ref loadUnitType was not called, the md5 sum is
	 * empty (null).
	 **/
	const QCString& md5() const;

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
	 * @return Default health aka power aka hitpoints of this unit.
	 **/
	unsigned long int baseHealth() const { return mBaseHealth; }

	unsigned long int maxHeatlh() const { return mHealth.value(upgradesCollection()); }

	/**
	 * @return The default shields value of this unit.
	 **/
	unsigned long int baseShields() const { return mBaseShields; }

	unsigned long int maxShields() const { return mShields.value(upgradesCollection()); }

	/**
	 * @return The default armor value of this unit.
	 **/
	unsigned long int baseArmor() const { return mBaseArmor; }

	unsigned long int maxArmor() const { return mArmor.value(upgradesCollection()); }

	/**
	 * @return How much this unit costs (of your mineral account).
	 * Note that this is the "base" cost only, that can be modified by
	 * upgrades. To retrieve the actual cost, all matching current upgrades
	 * of the player need to be applied.
	 **/
	unsigned long int baseMineralCost() const { return mBaseMineralCost; }

	unsigned long int mineralCost() const { return mMineralCost.value(upgradesCollection()); }

	/**
	 * @return How much this unit costs (of your oil account)
	 * Note that this is the "base" cost only, that can be modified by
	 * upgrades. To retrieve the actual cost, all matching current upgrades
	 * of the player need to be applied.
	 **/
	unsigned long int baseOilCost() const { return mBaseOilCost; }

	unsigned long int oilCost() const { return mOilCost.value(upgradesCollection()); }

	/**
	 * return How far this unit can see. Is a number of cells
	 **/
	unsigned int baseSightRange() const { return mBaseSightRange; }

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
	 * @return Maximal speed of the mobile unit. 0 if this is a facility. See
	 * @ref isFacility
	 **/
	bofixed baseSpeed() const;

	bofixed maxSpeed() const { return mSpeed.value(upgradesCollection()); }

	/**
	 * @return How fast this mobile unit accelerates. 0 if this is a facility. See
	 * @ref isFacility
	 **/
	bofixed baseAccelerationSpeed() const;

	bofixed maxAccelerationSpeed() const { return mAccelerationSpeed.value(upgradesCollection()); }

	/**
	 * @return How fast this mobile unit decelerates. 0 if this is a facility. See
	 * @ref isFacility
	 **/
	bofixed baseDecelerationSpeed() const;

	bofixed maxDecelerationSpeed() const { return mDecelerationSpeed.value(upgradesCollection()); }

	/**
	 * @return Turning speed of this mobile unit (degrees per advance call). 0 if
	 * this is a facility. See @ref isFacility
	 **/
	int rotationSpeed() const;

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
	 * Note that this is the "base" time only, that can be modified by
	 * upgrades. To retrieve the actual time, all matching current upgrades
	 * of the player need to be applied.
	 **/
	unsigned int baseProductionTime() const { return mBaseProductionTime; }

	unsigned int productionTime() const { return mProductionTime.value(upgradesCollection()); }

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
	QValueList<unsigned long int> requirements() const;

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

	QPtrList<BosonEffect> newDestroyedEffects(float x, float y, float z) const;
	QValueList<unsigned long int> destroyedEffectIds() const;

	QPtrList<BosonEffect> newConstructedEffects(float x, float y, float z) const;
	QValueList<unsigned long int> constructedEffectIds() const;

	const QPtrList<PluginProperties>* plugins() const;

	/**
	 * @return maximum range of weapons of this unit e.g. range of weapon with the longest range
	 **/
	unsigned long int maxWeaponRange() const  { return mMaxWeaponRange; }
	unsigned long int maxAirWeaponRange() const  { return mMaxAirWeaponRange; }
	unsigned long int maxLandWeaponRange() const  { return mMaxLandWeaponRange; }

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

	BoAction* action(UnitAction type) const;

	const QIntDict<BoAction>* allActions() const;

	BoAction* produceAction() const { return mProduceAction; };

	unsigned int explodingFragmentCount() const { return mExplodingFragmentCount; }
	long int explodingFragmentDamage() const { return mExplodingFragmentDamage; }
	const bofixed& explodingFragmentDamageRange() const { return mExplodingFragmentDamageRange; }
	QPtrList<BosonEffect> newExplodingFragmentFlyEffects(BoVector3Fixed pos) const;
	QPtrList<BosonEffect> newExplodingFragmentHitEffects(BoVector3Fixed pos) const;

	bool removeWreckageImmediately() const { return mRemoveWreckageImmediately; }

	bool getUpgradeableBaseValue(unsigned long int* ret, const QString& name, const QString& type) const;
	bool getUpgradeableBaseValue(long int* ret, const QString& name, const QString& type) const;
	bool getUpgradeableBaseValue(bofixed* ret, const QString& name, const QString& type) const;

	const UpgradesCollection& upgradesCollection() const;

	bool saveUpgradesAsXML(QDomElement& root) const;
	bool loadUpgradesFromXML(const QDomElement& root);


	/**
	 * Load actions for this unit. Must be called after overview pixmaps are
	 * loaded.
	 * Should be used by only SpeciesTheme.
	 **/
	void loadActions();

	void clearUpgrades();
	void addUpgrade(const UpgradeProperties* prop);
	void removeUpgrade(const UpgradeProperties* prop);
	void removeUpgrade(unsigned long int id);


protected:
	void loadMobileProperties(KSimpleConfig* conf);
	void loadFacilityProperties(KSimpleConfig* conf);
	void loadAllPluginProperties(KSimpleConfig* conf);
	void loadPluginProperties(PluginProperties* prop, KSimpleConfig* conf);

	void loadTextureNames(KSimpleConfig* conf);
	void loadSoundNames(KSimpleConfig* conf);
	void loadWeapons(KSimpleConfig* conf);

	void saveMobileProperties(KSimpleConfig* conf);
	void saveFacilityProperties(KSimpleConfig* conf);
	void saveAllPluginProperties(KSimpleConfig* conf);
	void saveTextureNames(KSimpleConfig* conf);
	void saveSoundNames(KSimpleConfig* conf);

	// Methods to set values. They are only meant to be used by unit
	//  editor. Don't use them unless you know what you are doing
	void setName(const QString& name);
	void setTypeId(unsigned long int id)  { mTypeId = id; }
	void setIsFacility(bool f) { mIsFacility = f; }
	void setUnitWidth(bofixed unitWidth)  { mUnitWidth = unitWidth; }
	void setUnitHeight(bofixed unitHeight)  { mUnitHeight = unitHeight; }
	void setUnitDepth(bofixed unitDepth)  { mUnitDepth = unitDepth; }

	void setProducer(unsigned int producer)  { mProducer = producer; }
	void setTerrainType(TerrainType terrain)  { mTerrain = terrain; }
	void setSupportMiniMap(bool supportMiniMap)  { mSupportMiniMap = supportMiniMap; }
	void setRequirements(QValueList<unsigned long int> requirements);
	void setDestroyedEffectIds(QValueList<unsigned long int> ids);
	void setConstructedEffectIds(QValueList<unsigned long int> ids);
	void setExplodingDamageRange(bofixed range)  { mExplodingDamageRange = range; }
	void setExplodingDamage(long int damage)  { mExplodingDamage = damage; }
	void setHitPoint(const BoVector3Fixed& hitpoint);
	void setRemoveWreckageImmediately(bool remove)  { mRemoveWreckageImmediately = remove; }

	// These only have effect if there is mobile or facility properties
	void setConstructionSteps(unsigned int steps);
	void setRotationSpeed(int speed);
	void setCanGoOnLand(bool c);
	void setCanGoOnWater(bool c);

	void clearPlugins(bool deleteweapons = true);

	void reset();

	void addPlugin(PluginProperties* prop);
	void addTextureMapping(QString shortname, QString longname);
	void addSound(int event, QString filename);

	TerrainType terrainType() const  { return mTerrain; }

	friend class BoUnitEditor;

private:
	void init();

private:
	class MobileProperties;
	class FacilityProperties;

private:
	UnitPropertiesPrivate* d;
	SpeciesTheme* mTheme;
	bool mFullMode;

#define DECLAREUPGRADEABLE(type, name) \
		void setBase##name(type v) { mBase##name = v; } \
		BoUpgradeableProperty<type> m##name; \
		type mBase##name;

	unsigned long int mTypeId; // note: 0 is invalid!
	bofixed mUnitWidth;
	bofixed mUnitHeight;
	bofixed mUnitDepth;
	unsigned int mProducer;
	TerrainType mTerrain;
	bool mSupportMiniMap;
	bool mCanShootAtAirUnits;
	bool mCanShootAtLandUnits;
	DECLAREUPGRADEABLE(unsigned long int, Health);
	DECLAREUPGRADEABLE(unsigned long int, SightRange);
	DECLAREUPGRADEABLE(unsigned long int, ProductionTime);
	DECLAREUPGRADEABLE(unsigned long int, MineralCost);
	DECLAREUPGRADEABLE(unsigned long int, OilCost);
	DECLAREUPGRADEABLE(unsigned long int, Armor);
	DECLAREUPGRADEABLE(unsigned long int, Shields);
	unsigned long int mMaxWeaponRange;
	unsigned long int mMaxLandWeaponRange;
	unsigned long int mMaxAirWeaponRange;
	long int mExplodingDamage;
	bofixed mExplodingDamageRange;
	BoAction* mProduceAction;
	unsigned int mExplodingFragmentCount;
	long int mExplodingFragmentDamage;
	bofixed mExplodingFragmentDamageRange;
	bool mRemoveWreckageImmediately;

	// for mobile units only
	DECLAREUPGRADEABLE(bofixed, Speed);

	// AB: TODO: these two are now upgradeable, too!
	DECLAREUPGRADEABLE(bofixed, AccelerationSpeed);
	DECLAREUPGRADEABLE(bofixed, DecelerationSpeed);
	int mRotationSpeed;
	bool mCanGoOnLand;
	bool mCanGoOnWater;

	// for facilities only
	unsigned int mConstructionFrames;

#undef DECLAREUPGRADEABLE

	bool mIsFacility;
	MobileProperties* mMobileProperties;
	FacilityProperties* mFacilityProperties;
};

#endif
