/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include <qstring.h>

#include <qvaluelist.h>

class UnitProperties;
class SpeciesTheme;

class KSimpleConfig;

// AB: we might implement weapons as PluginProperties, in order to support
// several weapons!
// disadvantage: canShootAt*() would get slower, since we need to iterate. but I
// guess it is not speed critical

// note that we can have PluginProperties that do <em>not</em> have any UnitPlugin, and
// we can also have UnitPlugins that <em>don't</em> have any 
/**
 * @short Base class for plugin properties for @ref UnitProperties
 * @authorAndreas Beckermann <b_mann@gmx.de>
 **/
class PluginProperties
{
public:
	enum PluginPropertiesTypes {
		Production = 0,
		Repair = 1,
		Harvester = 2,
		Refine = 3,
		Upgrade = 4,
		Weapon = 5
	};
	PluginProperties(const UnitProperties* parent);
	virtual ~PluginProperties();

	SpeciesTheme* speciesTheme() const;
	const UnitProperties* unitProperties() const { return mUnitProperties; }

	/**
	 * @return The i18n'ed name of this plugin. Does not need to be unique
	 * or so - just to display it in a list one day
	 **/
	virtual QString name() const = 0;
	virtual void loadPlugin(KSimpleConfig* config) = 0;

	/**
	 * @return A unique ID for the plugin. See @ref PluginPropertiesTypes
	 **/
	virtual int pluginType() const = 0;

private:
	const UnitProperties* mUnitProperties;
};

class ProductionProperties : public PluginProperties
{
public:
	ProductionProperties(const UnitProperties* parent);
	~ProductionProperties();

	static QString propertyGroup();

	virtual QString name() const;
	virtual void loadPlugin(KSimpleConfig* config);
	virtual int pluginType() const { return Production; }

	QValueList<int> producerList() const { return mProducerList; }

private:
	QValueList<int> mProducerList;
};

class RepairProperties : public PluginProperties
{
public:
	RepairProperties(const UnitProperties* parent);
	~RepairProperties();

	static QString propertyGroup();

	virtual QString name() const;
	virtual void loadPlugin(KSimpleConfig* config);
	virtual int pluginType() const { return Repair; }

private:
};

class HarvesterProperties : public PluginProperties
{
public:
	HarvesterProperties(const UnitProperties* parent);
	~HarvesterProperties();

	static QString propertyGroup();

	virtual QString name() const;
	virtual void loadPlugin(KSimpleConfig* config);
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

private:
	bool mCanMineMinerals;
	bool mCanMineOil;
	unsigned int mMaxResources;
};

#endif

