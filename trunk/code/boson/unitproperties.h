/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2002 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include <qstring.h>

#include <qvaluelist.h>
#include <qptrlist.h>
#include <qmap.h>

class SpeciesTheme;
class PluginProperties;
class UpgradeProperties;
class BosonWeaponProperties;
class BosonParticleSystem;
class BosonParticleSystemProperties;

class KSimpleConfig;

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

	UnitProperties();
	UnitProperties(SpeciesTheme*);

	/**
	 * @param fileName The filename of the config file for this unit type
	 **/
	UnitProperties(SpeciesTheme*, const QString& fileName);
	~UnitProperties();


	/**
	 * Load the file. This sets all values of UnitProperties. All values are
	 * readOnly, as UnitProperties is meant to change never.
	 *
	 * The file should contain units/your_unit_dir/index.unit at the end
	 * and should be an absolute path.
	 **/
	void loadUnitType(const QString& fileName, bool full = true);

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
	 * @return The unrotated width of the unit. The value is number of cells
	 * this unit occupies * BO_TILE_SIZE
	 **/
	unsigned int unitWidth() const { return mUnitWidth; }

	/**
	 * @return The unrotated height of the unit. The value is number of cells
	 * this unit occupies * BO_TILE_SIZE
	 **/
	unsigned int unitHeight() const { return mUnitHeight; }

	/**
	 * @return The unrotated height in z-direction of the unit. The value
	 * is number of cells this unit occupies * BO_TILE_SIZE
	 **/
	unsigned int unitDepth() const { return mUnitDepth; }

	/**
	 * @return Default health aka power aka hitpoints of this unit.
	 **/
	unsigned long int health() const { return mHealth; }

	/**
	 * @return The default shields value of this unit.
	 **/
	unsigned long int shields() const { return mShields; }

	/**
	 * @return The default armor value of this unit.
	 **/
	unsigned long int armor() const { return mArmor; }

	/**
	 * @return How much this unit costs (of your mineral account)
	 **/
	unsigned long int mineralCost() const { return mMineralCost; }

	/**
	 * @return How much this unit costs (of your oil account)
	 **/
	unsigned long int oilCost() const { return mOilCost; }

	/**
	 * return How far this unit can see. Is a number of cells, so multiply
	 * with BO_TILE_SIZE to use it on the canvas.
	 **/
	unsigned int sightRange() const { return mSightRange; }

	/**
	 * @return The Type ID of the unit. This ID is unique for this
	 * UnitProperties. There is no other unit with the same type ID. Note
	 * that you can construct several units of the same type ID in a game -
	 * they will all be of the same type (e.g. they are all ships).
	 * 0 means invalid typeId.
	 **/
	unsigned long int typeId() const { return mTypeId; };

	/**
	 * @return The name of this unit type. Examples are "Aircraft", "Quad",
	 * "Ship"
	 **/
	const QString& name() const { return mName; };

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
	float speed() const;

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

	bool canRefineMinerals() const;
	bool canRefineOil() const;

	/**
	 * @return Which type of factory can produce this unit. See
	 * data/themes/species/human/units/README for more in this.
	 **/
	unsigned int producer() const { return mProducer; }

	/**
	 * @return The path to the unit files. That is the directory where the
	 * index.unit file and the pixmap files are stored.
	 **/
	const QString& unitPath() const { return mUnitPath; };

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
	unsigned int productionTime() const { return mProductionTime; }

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
	QValueList<unsigned long int> requirements() const { return mRequirements; };

	/**
	 * .3ds files seem to support only filenames of 8+3 length. We work
	 * around this by mapping the "short name" to a longer name.
	 * @return All map of all texture names that have a longer name assigned
	 * to.
	 **/
	QMap<QString, QString> longTextureNames() const { return mTextureNames; }

	const PluginProperties* properties(int pluginType) const;

	/**
	 * @return First part of the sound filename - e.g. "move" if the file
	 * name should be "move_00.ogg". the _00 is added dynamically (randomly)
	 * by @ref BosonSound
	 **/
	QString sound(int soundEvent) const { return mSounds[soundEvent]; }

	QMap<int, QString> sounds() const { return mSounds; }

	/**
	 * @return List of all possible upgrades to this unit. Note that this
	 * <em>does not</em> include technologies
	 **/
	QPtrList<UpgradeProperties> possibleUpgrades() const { return mUpgrades; }

	/**
	 * @return List of all not researched upgrades to this unit. Note that this
	 * <em>does not</em> include technologies
	 **/
	QPtrList<UpgradeProperties> unresearchedUpgrades() const { return mNotResearchedUpgrades; }

	void upgradeResearched(UpgradeProperties* upgrade) { mNotResearchedUpgrades.removeRef(upgrade); };

	QPtrList<BosonParticleSystem> newDestroyedParticleSystems(float x, float y, float z) const;

	const QPtrList<PluginProperties>* plugins() const  { return &mPlugins; };

protected:
	void loadMobileProperties(KSimpleConfig* conf);
	void loadFacilityProperties(KSimpleConfig* conf);
	void loadAllPluginProperties(KSimpleConfig* conf);
	void loadPluginProperties(PluginProperties* prop, KSimpleConfig* conf);

	void loadUpgrades(KSimpleConfig* conf);

	void loadTextureNames(KSimpleConfig* conf);
	void loadSoundNames(KSimpleConfig* conf);
	void loadWeapons(KSimpleConfig* conf);

	void saveMobileProperties(KSimpleConfig* conf);
	void saveFacilityProperties(KSimpleConfig* conf);
	void saveAllPluginProperties(KSimpleConfig* conf);
	void saveTextureNames(KSimpleConfig* conf);
	void saveSoundNames(KSimpleConfig* conf);

	// Methods to set values. They are only meant to be used by upgrades and unit
	//  editor. Don't use them unless you know what you are doing
	void setName(QString name)  { mName = name; };
	void setUnitPath(QString unitPath)  { mUnitPath = unitPath; };
	void setUnitWidth(unsigned int unitWidth)  { mUnitWidth = unitWidth; };
	void setUnitHeight(unsigned int unitHeight)  { mUnitHeight = unitHeight; };
	void setUnitDepth(unsigned int unitDepth)  { mUnitDepth = unitDepth; };
	void setHealth(unsigned long int health)  { mHealth = health; };
	void setSightRange(unsigned int sightRange)  { mSightRange = sightRange; };
	void setProducer(unsigned int producer)  { mProducer = producer; };
	void setProductionTime(unsigned int productionTime)  { mProductionTime = productionTime; };
	void setMineralCost(unsigned long int mineralCost)  { mMineralCost = mineralCost; };
	void setOilCost(unsigned long int oilCost)  { mOilCost = oilCost; };
	void setTerrainType(TerrainType terrain)  { mTerrain = terrain; };
	void setSupportMiniMap(bool supportMiniMap)  { mSupportMiniMap = supportMiniMap; };
	void setRequirements(QValueList<unsigned long int> requirements)  { mRequirements = requirements; };
	void setArmor(unsigned long int armor)  { mArmor = armor; };
	void setShields(unsigned long int shields)  { mShields = shields; };
	void setLongTextureNames(QMap<QString, QString> textureNames)  { mTextureNames = textureNames; };
	void setSounds(QMap<int, QString> sounds)  { mSounds = sounds; };

	friend class BoUnitEditor;

private:
	SpeciesTheme* mTheme;

	friend class UpgradePropertiesBase; // UpgradePropertiesBase::apply() modifies our private vars
	void setSpeed(float speed); // Because speed is in MobileProperties

	QString mName;
	QString mUnitPath; // the path to the unit files
	unsigned long int mTypeId; // note: 0 is invalid!
	unsigned int mUnitWidth;
	unsigned int mUnitHeight;
	unsigned int mUnitDepth;
	unsigned long int mHealth;
	unsigned int mSightRange;
	unsigned int mProducer;
	unsigned int mProductionTime;
	unsigned long int mMineralCost;
	unsigned long int mOilCost;
	TerrainType mTerrain;
	bool mSupportMiniMap;
	QValueList<unsigned long int> mRequirements;
	unsigned long int mArmor;
	unsigned long int mShields;
	bool mCanShootAtAirUnits;
	bool mCanShootAtLandUnits;

	class MobileProperties;
	class FacilityProperties;
	MobileProperties* mMobileProperties;
	FacilityProperties* mFacilityProperties;
	QPtrList<PluginProperties> mPlugins;

	QMap<QString, QString> mTextureNames;
	QMap<int, QString> mSounds;

	QPtrList<UpgradeProperties> mUpgrades;
	QPtrList<UpgradeProperties> mNotResearchedUpgrades;

	QPtrList<BosonParticleSystemProperties> mDestroyedParticleSystems;
};

#endif
