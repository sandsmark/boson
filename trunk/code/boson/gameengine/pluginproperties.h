/*
    This file is part of the Boson game
    Copyright (C) 2002 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef PLUGINPROPERTIES_H
#define PLUGINPROPERTIES_H

#include "boupgradeableproperty.h"

#include <qstring.h>
#include <qvaluelist.h>

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
		RadarJammer = 8
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

	QValueList<unsigned long int> producerList() const { return mProducerList; }

private:
	QValueList<unsigned long int> mProducerList;

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

	QValueList<QString> mCanStore;
	QValueList<QString> mMustBePickedUp;
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

