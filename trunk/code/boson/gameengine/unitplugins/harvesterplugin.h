/*
    This file is part of the Boson game
    Copyright (C) 2002-2006 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2002-2006 Rivo Laks (rivolaks@hot.ee)

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
#ifndef HARVESTERPLUGIN_H
#define HARVESTERPLUGIN_H

#include "unitplugin.h"

template<class T> class BoVector2;
template<class T> class BoRect2;
typedef BoVector2<bofixed> BoVector2Fixed;
typedef BoRect2<bofixed> BoRect2Fixed;
class RefineryPlugin;
class ResourceMinePlugin;

class QDomElement;

class ResourceMinePlugin;
class RefineryPlugin;
/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class HarvesterPlugin : public UnitPlugin
{
public:
	HarvesterPlugin(Unit* owner);
	~HarvesterPlugin();

	virtual int pluginType() const { return Harvester; }

	virtual void advance(unsigned int);
	void advanceMine();
	void advanceRefine();

	int resourcesX() const { return mResourcesX; }
	int resourcesY() const { return mResourcesY; }
	unsigned int resourcesMined() const { return mResourcesMined; }

	void mineAt(ResourceMinePlugin* resource);
	void refineAt(RefineryPlugin* refinery);

	/**
	 * @return PluginProperties::canMineMinerals
	 **/
	bool canMineMinerals() const;

	/**
	 * @return PluginProperties::canMineOil
	 **/
	bool canMineOil() const;

	/**
	 * @return PluginProperties::maxResources
	 **/
	unsigned int maxResources() const;

	/**
	 * @return PluginProperties::miningSpeed
	 **/
	unsigned int miningSpeed() const;

	/**
	 * @return PluginProperties::unloadingSpeed
	 **/
	unsigned int unloadingSpeed() const;

	virtual bool saveAsXML(QDomElement& root) const;
	virtual bool loadFromXML(const QDomElement& root);

	virtual void unitDestroyed(Unit*);
	virtual void itemRemoved(BosonItem*);

protected:
	bool isAtResourceMine() const;
	bool isAtRefinery() const;

	ResourceMinePlugin* findClosestResourceMine() const;
	RefineryPlugin* findClosestRefinery() const;

private:
	KGameProperty<int> mResourcesX;
	KGameProperty<int> mResourcesY;
	KGameProperty<unsigned int> mResourcesMined;

	KGameProperty<int> mHarvestingType; // either mining or refining

	RefineryPlugin* mRefinery;
	ResourceMinePlugin* mResourceMine;
};

#endif
