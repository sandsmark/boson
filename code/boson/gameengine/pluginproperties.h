/*
    This file is part of the Boson game
    Copyright (C) 2002-2006 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef PLUGINPROPERTIES_H
#define PLUGINPROPERTIES_H

#include "boupgradeableproperty.h"
#include "../../math/bovector.h"

#include <qstring.h>
#include <q3valuelist.h>
#include <q3valuevector.h>

class UnitProperties;
class SpeciesTheme;
class PluginPropertiesEditor;

class KSimpleConfig;

// note that we can have PluginProperties that do <em>not</em> have any UnitPlugin, and
// we can also have UnitPlugins that <em>don't</em> have any
/**
 * The PluginProperties class extends the properties of @ref UnitProperties.
 * Every derived class must provide a static propertyGroup function returning
 * the @ref KConfig group in the index.unit file this plugin does belong to.
 * That group is used to load the plugin.
 *
 * Note that this has nothing to do with the @ref UnitPlugins class. The @ref
 * UnitPlugins class provides plugins for actual @ref Unit objects, whereas
 * this class allows extending the @ref UnitProperties objects. But although
 * they are not (technically) related, usually there is exactly one
 * class derived from PluginProperties for every class derived from @ref
 * UnitPlugins. I encourage you to follow this, even
 * though it is not necessary, but it will make the code easier.
 *
 * @short Base class for plugin properties for @ref UnitProperties
 * @authorAndreas Beckermann <b_mann@gmx.de>
 **/
class PluginProperties : public BoBaseValueCollection
{
public:
	enum PluginPropertiesTypes {
		Production = 0,
		Repair = 1,
		Harvester = 2,
		Refinery = 3,
		Weapon = 4,
		ResourceMine = 5,
		AmmunitionStorage = 6,
		Radar = 7,
		RadarJammer = 8,
		UnitStorage = 9
	};
	PluginProperties(const UnitProperties* parent);
	virtual ~PluginProperties();

	/**
	 * Use this (in the unit editor ! NEVER use this in the game!) to allow
	 * editing the properties of this class.
	 * See also @ref PluginPropertiesEditor
	 **/
	void setEditorObject(PluginPropertiesEditor* editor);

	/**
	 * @return NULL in the game. The object set by @ref setEditorObject in
	 * the unit editor.
	 * See also @ref PluginPropertiesEditor
	 **/
	PluginPropertiesEditor* editorObject() const
	{
		return mEditorObject;
	}


	SpeciesTheme* speciesTheme() const;
	const UnitProperties* unitProperties() const { return mUnitProperties; }

	/**
	 * @return The i18n'ed name of this plugin. Does not need to be unique
	 * or so - just to display it in a list one day
	 **/
	virtual QString name() const = 0;
	virtual bool loadPlugin(KSimpleConfig* config) = 0;
	virtual bool savePlugin(KSimpleConfig* config) = 0;

	/**
	 * @return A unique ID for the plugin. See @ref PluginPropertiesTypes
	 **/
	virtual int pluginType() const = 0;


	/**
	 * @return UnitProperties::upgradesCollection.
	 **/
	const BoUpgradesCollection& upgradesCollection() const;

private:
	const UnitProperties* mUnitProperties;
	PluginPropertiesEditor* mEditorObject;
};

class ProductionProperties : public PluginProperties
{
public:
	ProductionProperties(const UnitProperties* parent);
	~ProductionProperties();

	static QString propertyGroup();

	virtual QString name() const;
	virtual bool loadPlugin(KSimpleConfig* config);
	virtual bool savePlugin(KSimpleConfig* config);
	virtual int pluginType() const { return Production; }

	Q3ValueList<unsigned long int> producerList() const { return mProducerList; }

private:
	Q3ValueList<unsigned long int> mProducerList;

	friend class ProductionPropertiesEditor;
};

class RepairProperties : public PluginProperties
{
public:
	RepairProperties(const UnitProperties* parent);
	~RepairProperties();

	static QString propertyGroup();

	virtual QString name() const;
	virtual bool loadPlugin(KSimpleConfig* config);
	virtual bool savePlugin(KSimpleConfig* config);
	virtual int pluginType() const { return Repair; }

private:
	friend class RepairPropertiesEditor;
};

class HarvesterProperties : public PluginProperties
{
public:
	HarvesterProperties(const UnitProperties* parent);
	~HarvesterProperties();

	static QString propertyGroup();

	virtual QString name() const;
	virtual bool loadPlugin(KSimpleConfig* config);
	virtual bool savePlugin(KSimpleConfig* config);
	virtual int pluginType() const { return Harvester; }

	/**
	 * @return TRUE if the unit can mine minerals, otherwise FALSE.
	 **/
	inline bool canMineMinerals() const { return mCanMineMinerals; }

	/**
	 * @return TRUE if the unit can mine oil, otherwise FALSE.
	 **/
	inline bool canMineOil() const { return mCanMineOil; }

	/**
	 * @return The maximal amount of resources (oil or minerals) that can be
	 * mined until the unit must return to a refinery. The type of resources
	 * depends on @ref canMineMinerals and @ref canMineOil (only one of them
	 * can be true)
	 **/
	unsigned int maxResources() const { return mMaxResources; }

	/**
	 * @return Maximal amount of resources that can be mined during one advance call
	 **/
	unsigned int miningSpeed() const { return mMiningSpeed; }

	/**
	 * @return Maximal amount of resources that can be unloaded during one advance call
	 **/
	unsigned int unloadingSpeed() const { return mUnloadingSpeed; }

protected:

private:
	bool mCanMineMinerals;
	bool mCanMineOil;
	unsigned int mMaxResources;
	unsigned int mMiningSpeed;
	unsigned int mUnloadingSpeed;

	friend class HarvesterPropertiesEditor;
};


class RefineryProperties : public PluginProperties
{
public:
	RefineryProperties(const UnitProperties* parent);
	~RefineryProperties();

	static QString propertyGroup();

	virtual QString name() const;
	virtual bool loadPlugin(KSimpleConfig* config);
	virtual bool savePlugin(KSimpleConfig* config);
	virtual int pluginType() const { return PluginProperties::Refinery; }

	/**
	 * @return TRUE if the unit can mine minerals, otherwise FALSE.
	 **/
	inline bool canRefineMinerals() const { return mCanRefineMinerals; }

	/**
	 * @return TRUE if the unit can mine oil, otherwise FALSE.
	 **/
	inline bool canRefineOil() const { return mCanRefineOil; }

private:
	bool mCanRefineMinerals;
	bool mCanRefineOil;

	friend class RefineryPropertiesEditor;
};

class ResourceMineProperties : public PluginProperties
{
public:
	ResourceMineProperties(const UnitProperties* parent);
	~ResourceMineProperties();

	static QString propertyGroup();

	virtual QString name() const;
	virtual bool loadPlugin(KSimpleConfig* config);
	virtual bool savePlugin(KSimpleConfig* config);
	virtual int pluginType() const { return ResourceMine; }

	bool canProvideMinerals() const { return mMinerals; }
	bool canProvideOil() const { return mOil; }

private:
	bool mMinerals;
	bool mOil;

	friend class ResourceMinePropertiesEditor;
};

class AmmunitionStorageProperties : public PluginProperties
{
public:
	AmmunitionStorageProperties(const UnitProperties* parent);
	~AmmunitionStorageProperties();

	static QString propertyGroup();

	virtual QString name() const;
	virtual bool loadPlugin(KSimpleConfig* config);
	virtual bool savePlugin(KSimpleConfig* config);
	virtual int pluginType() const { return AmmunitionStorage; }

	/**
	 * @return See @ref AmmunitionStoragePlugin::mustBePickedUp
	 **/
	bool mustBePickedUp(const QString& type) const;

	/**
	 * @return TRUE if this plugin can store @p type, otherwise FALSE.
	 **/
	bool canStore(const QString& type) const;

private:
	friend class AmmunitionStoragePropertiesEditor;

	Q3ValueList<QString> mCanStore;
	Q3ValueList<QString> mMustBePickedUp;
};

class RadarProperties : public PluginProperties
{
public:
	RadarProperties(const UnitProperties* parent);
	~RadarProperties();

	static QString propertyGroup();

	virtual QString name() const;
	virtual bool loadPlugin(KSimpleConfig* config);
	virtual bool savePlugin(KSimpleConfig* config);
	virtual int pluginType() const { return Radar; }

	/**
	 * @return Power transmitted by the transmitter antenna.
	 * Note that this has _no_ correspondance to the power resource.
	 **/
	float transmittedPower() const { return mTransmittedPower; }

	/**
	 * @return Minimum received power to notice the target
	 **/
	float minReceivedPower() const { return mMinReceivedPower; }

	bool detectsLandUnits() const { return mDetectsLandUnits; }
	bool detectsAirUnits() const { return mDetectsAirUnits; }

private:
	float mTransmittedPower;
	float mMinReceivedPower;
	bool mDetectsLandUnits;
	bool mDetectsAirUnits;
};

class RadarJammerProperties : public PluginProperties
{
public:
	RadarJammerProperties(const UnitProperties* parent);
	~RadarJammerProperties();

	static QString propertyGroup();

	virtual QString name() const;
	virtual bool loadPlugin(KSimpleConfig* config);
	virtual bool savePlugin(KSimpleConfig* config);
	virtual int pluginType() const { return RadarJammer; }

	/**
	 * @return Power transmitted by the jammer.
	 * Note that this has _no_ correspondance to the power resource.
	 **/
	float transmittedPower() const { return mTransmittedPower; }

private:
	float mTransmittedPower;
};

class UnitStorageProperties : public PluginProperties
{
public:
	/**
	 * The type of units that can use a certain path
	 **/
	enum PathUnitType {
		/**
		 * Default. The enter and exit points of the path must be on the
		 * border of the unit (i.e. bottom, top, left or right side of
		 * the unit).
		 **/
		PathTypeLand = 0,

		/**
		 * Path for planes, i.e. flying units that can NOT "stand still"
		 * in the air. The enter/exit points can be at any position
		 * inside the unit, but the plane can land/start in a certain
		 * specified direction only.
		 **/
		PathTypePlane = 1,

		/**
		 * Like a plane, but a helicopter can simply move over the enter
		 * point and reduce its z position until it has landed. No
		 * requirements on x and y direction of the landing unit.
		 **/
		PathTypeHelicopter = 2,

		/**
		 * Like Land units, but for ships.
		 **/
		PathTypeShip = 3
	};

public:
	UnitStorageProperties(const UnitProperties* parent);
	~UnitStorageProperties();

	static QString propertyGroup();

	virtual QString name() const;
	virtual bool loadPlugin(KSimpleConfig* config);
	virtual bool savePlugin(KSimpleConfig* config);
	virtual int pluginType() const { return UnitStorage; }

	/**
	 * @return The number of enter paths that are supported by this unit.
	 **/
	unsigned int enterPathCount() const;

	/**
	 * @return The enter path number @p i. The returned vectors are meant to be
	 * pathpoints for the unit.
	 **/
	Q3ValueList<BoVector2Float> enterPath(unsigned int i) const;

	/**
	 * @return The "leavePath" that corresponds to the @ref enterPath with
	 * number @p i.
	 **/
	Q3ValueList<BoVector2Float> leavePathForEnterPath(unsigned int i) const;

	/**
	 * @return Which kind of unit can use the path number @p i.
	 **/
	PathUnitType enterPathUnitType(unsigned int i) const;

	BoVector2Float enterDirection(unsigned int i) const;

protected:
	bool loadEnterPath(int i, KSimpleConfig* config);
	bool saveEnterPath(int i, KSimpleConfig* config);

protected:
	class Path {
	public:
		Path()
		{
			clear();
		}
		Path(const Path& p)
		{
			*this = p;
		}
		Path& operator=(const Path& p)
		{
			mPathPoints = p.mPathPoints;
			mType = p.mType;
			mLeaveMethod = p.mLeaveMethod;
			return *this;
		}
		void clear()
		{
			mPathPoints.clear();
			mType = PathTypeLand;
			mLeaveMethod = 0;
		}
		bool loadPath(KSimpleConfig* config);
		bool savePath(KSimpleConfig* config);

		// AB: note: float, not fixed! -> values in [0;1]
		Q3ValueList<BoVector2Float> mPathPoints;
		PathUnitType mType;
		int mLeaveMethod;
	};

private:
	Q3ValueVector<Path> mEnterPaths;
};


/**
 * This class may be used from within the unit editor only. It can be used to
 * modify the properties of a @ref PluginProperties object directly.
 * @authorAndreas Beckermann <b_mann@gmx.de>
 **/
class PluginPropertiesEditor
{
public:
	PluginPropertiesEditor(PluginProperties* properties)
	{
		mProperties = properties;
	}

private:
	PluginProperties* mProperties;
};

#endif

