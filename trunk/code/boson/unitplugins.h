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
#ifndef UNITPLUGINS_H
#define UNITPLUGINS_H

#include "global.h"

#include <kgame/kgameproperty.h>

#include <qvaluelist.h>
#include <qpair.h>

class Unit;
class SpeciesTheme;
class UnitProperties;
class BosonCanvas;
class Cell;
class Player;
class PluginProperties;
class Boson;
class BosonItem;
class BosonWeapon;
class QPoint;
class QDomElement;

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class UnitPlugin
{
public:
	enum UnitPlugins {
		PluginStart = 0, // MUST be the first!
		Production = 1,
		Repair = 2,
		Harvester = 3,
		Weapon = 4, // note: this won't end up in Unit::plugin()! weapons are stored separately. also note that rtti==Weapon is *not* unique! they have their own class and rttis - see BosonWeapon
		Bombing = 5,
		Mining = 6, // placing mine (the exploding ones)
		ResourceMine = 7,

		PluginEnd // MUST be the last entry!
	};

	UnitPlugin(Unit* unit);
	virtual ~UnitPlugin();

	inline Unit* unit() const { return mUnit; }

	/**
	 * Convenience method for unit()->speciesTheme()
	 **/
	SpeciesTheme* speciesTheme() const;

	/**
	 * Convenience method for unit()->owner()
	 **/
	Player* player() const;

	/**
	 * Convenience method for unit()->unitProperties()
	 **/
	const UnitProperties* unitProperties() const;

	/**
	 * Convenience method for unit()->properties()
	 **/
	const PluginProperties* properties(int propertyType) const;

	/**
	 * Convenience method for unit()->canvas()
	 **/
	BosonCanvas* canvas() const;

	/**
	 * Convenience method for unit()->dataHandler()
	 **/
	KGamePropertyHandler* dataHandler() const;

	/**
	 * Convenience method for player()->game()
	 **/
	Boson* game() const;

	virtual int pluginType() const = 0;

	/**
	 * Called when @p item is about to be removed from the game. When your
	 * plugin stores a pointer to an item (e.g. a unit, such as a pointer to
	 * a refinery), you should set it at least to NULL now.
	 *
	 * Note that at this point @p item has not yet been deleted, but it will
	 * be soon!
	 **/
	virtual void itemRemoved(BosonItem* item) = 0;

	/**
	 * @param advanceCount See @ref BosonCanvas::slotAdvance. You can use
	 * this to do expensive calculations only as seldom as possible. Note
	 * that there is still some overhead, since this advance method still
	 * gets called!
	 **/
	virtual void advance(unsigned int advanceCount) = 0;

	/**
	 * Save the plugin into @p root. You must implement this in derived
	 * classes, but usually you will simply return true without touching @p
	 * root.
	 *
	 * Note that you are meant to use @ref KGameProperty for most of the
	 * properties. Don't use saveAsXML() for integer values or so.
	 *
	 * You should save e.g. IDs that help you to identify a pointer (e.g.
	 * the harvester plugin will save the ID of the refinery it is going
	 * to).
	 *
	 * See also @ref loadFromXML.
	 **/
	virtual bool saveAsXML(QDomElement& root) const = 0;

	/**
	 * See also @ref saveAsXML. You should use @ref KGameProperty for most
	 * properties.
	 *
	 * Here you will usually load pointers - e.g. you could save the ID of a
	 * target in @ref saveAsXML and here you could set the pointer of the
	 * target.
	 **/
	virtual bool loadFromXML(const QDomElement& root) = 0;

private:
	Unit* mUnit;
};

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class ProductionPlugin : public UnitPlugin
{
public:
	ProductionPlugin(Unit* unit);
	~ProductionPlugin();

	virtual int pluginType() const { return Production; }

	/**
	 * @return Whether there are any productions pending for this unit.
	 * Always FALSE if unitProperties()->canProduce() is FALSE.
	 **/
	inline bool hasProduction() const
	{
		return !mProductions.isEmpty();
	}

	/**
	 * @return The type ID (see @ref UnitProperties::typeId) of the
	 * completed production (if any).
	 **/
	unsigned long int completedProductionId() const;
	ProductionType completedProductionType() const;

	/**
	 * @return The type ID of the current production. -1 if there is no
	 * production.
	 **/
	inline unsigned long int currentProductionId() const
	{
		if (!hasProduction()) {
			return 0;
		}
		return mProductions.first().second;
	}

	inline ProductionType currentProductionType() const
	{
		if (!hasProduction()) {
			return ProduceNothing;
		}
		return mProductions.first().first;
	}

	/**
	 * Remove the first item from the production list.
	 **/
	void removeProduction(); // removes first item

	/**
	 * Remove first occurance of type ID id in the production list. Does not
	 * remove anything if id is not in the list.
	 **/
	void removeProduction(ProductionType type, unsigned long int id);

	/**
	 * Add production of type and with id (see @ref UnitProprties::typeId) to the
	 * construction list.
	 **/
	void addProduction(ProductionType type, unsigned long int id);

	QValueList<QPair<ProductionType, unsigned long int> > productionList() const { return mProductions; }
	bool contains(ProductionType type, unsigned long int id); // { return productionList().contains(typeId); }

	/**
	 * @return The percentage of the production progress. 0 means the
	 * production just started, 100 means the production is completed.
	 **/
	double productionProgress() const;

	virtual void advance(unsigned int);

	virtual bool saveAsXML(QDomElement& root) const;
	virtual bool loadFromXML(const QDomElement& root);

	virtual void itemRemoved(BosonItem*) {}

private:
	QValueList<QPair<ProductionType, unsigned long int> > mProductions;
	KGameProperty<unsigned int> mProductionState;
};

/**
 * Experimental plugin. At the current state id doesn't make any sense, since I
 * don't use any member variables anymore...
 *
 * Nevertheless I don't entegrate the functionality into Unit since it should
 * get some more testing
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class RepairPlugin : public UnitPlugin
{
public:
	RepairPlugin(Unit* owner);
	~RepairPlugin();

	virtual int pluginType() const { return Repair; }

	/**
	 * Order to repair unit. For a repairyard this means the unit will move
	 * to the repairyard and once it is in range it'll be repaired. 
	 *
	 * For mobile repair-units this means that the <em>repairing</em> (i.e.
	 * the one that has this plugin) moves to unit and repairs it.
	 **/
	void repair(Unit* unit);

	/**
	 * Called from @ref Unit::advanceNone. Repair the next unit that is in
	 * range. An alternative name might be "advance", just like in @ref
	 * ProducePlugin but since we don't have a WorkRepair in @ref Unit there
	 * is no advance call for it from @ref BosonCanvas::slotAdvance either.
	 *
	 * @ref Unit::advanceNone is used for it instead.
	 **/
	void repairInRange();


	// does nothing, yet. plugin is experimental anyway.
	virtual void advance(unsigned int) {}

	virtual bool saveAsXML(QDomElement& root) const;
	virtual bool loadFromXML(const QDomElement& root);

	virtual void itemRemoved(BosonItem*);

private:
};

/*
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class ResourceMinePlugin;
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

	void mineAt(const QPoint& pos);
	void refineAt(Unit* refinery);

	void setRefinery(Unit* refinery);

	bool canMine(Cell* cell) const; // obsolete
	bool canMine(Unit*) const;
	bool canMine(ResourceMinePlugin*) const;

	inline Unit* refinery() const { return mRefinery; }

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

	virtual void itemRemoved(BosonItem*);

private:
	KGameProperty<int> mResourcesX;
	KGameProperty<int> mResourcesY;
	KGameProperty<unsigned int> mResourcesMined;

	KGameProperty<int> mHarvestingType; // either mining or refining

	Unit* mRefinery;
	ResourceMinePlugin* mResourceMine;
};

/**
 * @short Helper plugin for bombing (dropping bomb)
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BombingPlugin : public UnitPlugin
{
public:
	BombingPlugin(Unit* owner);
	~BombingPlugin();

	virtual int pluginType() const { return Bombing; }

	void bomb(int weaponId, float x, float y);

	virtual void advance(unsigned int);

	virtual bool saveAsXML(QDomElement& root) const;
	virtual bool loadFromXML(const QDomElement& root);

	virtual void itemRemoved(BosonItem*) {}

private:
	BosonWeapon* mWeapon; // FIXME: must be saved on Unit::save()
	KGameProperty<int> mPosX;
	KGameProperty<int> mPosY;
};

/**
 * @short Helper plugin for mining (mining = placing mines (mine = explosive
 * device ;-)))
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class MiningPlugin : public UnitPlugin
{
public:
	MiningPlugin(Unit* owner);
	~MiningPlugin();

	virtual int pluginType() const { return Mining; }

	void mine(int weaponId);

	virtual void advance(unsigned int);

	virtual bool saveAsXML(QDomElement& root) const;
	virtual bool loadFromXML(const QDomElement& root);

	virtual void itemRemoved(BosonItem*) {}

private:
	BosonWeapon* mWeapon; // FIXME: must be saved in Unit::save()
	KGameProperty<int> mPlacingCounter;
};

/**
 * @short Plugin for mineral/oil mines
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class ResourceMinePlugin : public UnitPlugin
{
public:
	ResourceMinePlugin(Unit* owner);
	~ResourceMinePlugin();

	virtual int pluginType() const { return ResourceMine; }

	virtual bool loadFromXML(const QDomElement& root);
	virtual bool saveAsXML(QDomElement& root) const;
	virtual void advance(unsigned int advanceCount);

	/**
	 * @return How much minerals are left here. -1 means unlimited.
	 **/
	int minerals() const;

	/**
	 * @return How much oil is left here. -1 means unlimited.
	 **/
	int oil() const;

	void setMinerals(int m);
	void setOil(int m);

	/**
	 * @return See @ref ResourceMinePropeties::canProvideMinerals
	 **/
	bool canProvideMinerals() const;

	/**
	 * @return See @ref ResourceMinePropeties::canProvideOil
	 **/
	bool canProvideOil() const;

	virtual void itemRemoved(BosonItem*);

private:
	KGameProperty<int> mOil;
	KGameProperty<int> mMinerals;
};

#endif
