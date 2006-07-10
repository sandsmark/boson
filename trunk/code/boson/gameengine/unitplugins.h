/*
    This file is part of the Boson game
    Copyright (C) 2002-2005 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2002-2005 Rivo Laks (rivolaks@hot.ee)

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

#include "../global.h"
#include "../bomath.h"

#include "bogameproperty.h"

#include <qvaluelist.h>
#include <qmap.h>
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
class BoUpgradesCollection;
template<class T> class BoVector2;
template<class T> class BoRect2;
typedef BoVector2<bofixed> BoVector2Fixed;
typedef BoRect2<bofixed> BoRect2Fixed;

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
		Refinery = 8,
		AmmunitionStorage = 9,
		Radar = 10,

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

	/**
	 * Convenience method for unit()->upgradesCollection().
	 *
	 * This object can be used to implement @ref BoUpgradeableProperty
	 * objects in UnitPlugin objects.
	 **/
	const BoUpgradesCollection& upgradesCollection() const;

	virtual int pluginType() const = 0;

	/**
	 * See @ref Unit::unitDestroyed
	 **/
	virtual void unitDestroyed(Unit* unit) = 0;

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
	 * @param advanceCallsCount See @ref BosonCanvas::slotAdvance. You can use
	 * this to do expensive calculations only as seldom as possible. Note
	 * that there is still some overhead, since this advance method still
	 * gets called!
	 **/
	virtual void advance(unsigned int advanceCallsCount) = 0;

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

protected:
	bool isNextTo(const Unit* unit) const;

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
	 * This is called when the current production has been placed onto the
	 * map. The current production is removed from the internal list and the
	 * next production is started.
	 *
	 * The result of calling this while the current production is not yet
	 * finished is undefined.
	 **/
	void productionPlaced(Unit* produced);

	/**
	 * Add production of type and with id (see @ref UnitProprties::typeId) to the
	 * construction list and start the production.
	 **/
	void addProduction(ProductionType type, unsigned long int id);

	void pauseProduction();
	void unpauseProduction();
	void abortProduction(ProductionType type, unsigned long int id);

	QValueList<QPair<ProductionType, unsigned long int> > productionList() const { return mProductions; }
	bool contains(ProductionType type, unsigned long int id); // { return productionList().contains(typeId);}

	/**
	 * @return A list with all unittypes that this plugin could prodcue if
	 * all production requirements were fullfilled.
	 *
	 * @param producible If non-NULL, this returns alls unittypes that this
	 * plugin can currently produce.
	 * @param impossible This list returns (if non-NULL) all
	 * unittypes that cannot be produced currently, but could be, if the
	 * necessary requirements were met.
	 **/
	QValueList<unsigned long int> allUnitProductions(QValueList<unsigned long int>* producible, QValueList<unsigned long int>* notYetProducible) const;

	/**
	 * This behaves like @ref allUnitProductions with one exception:
	 *
	 * All technologies that already have been researched and thus cannot be
	 * researched anymore, are not returned.
	 **/
	QValueList<unsigned long int> allTechnologyProductions(QValueList<unsigned long int>* producible, QValueList<unsigned long int>* notYetProducible) const;



	/**
	 * See @ref canCurrentlyProduceUnit and @ref canCurrentlyProduceTechnology
	 **/
	bool canCurrentlyProduce(ProductionType p, unsigned long int type) const;

	/**
	 * @return TRUE if this plugin can produce the unit @p type. This is the
	 * case if this plugin is a producer of @p type and all requirements are
	 * fullfilled.
	 **/
	bool canCurrentlyProduceUnit(unsigned long int type) const;

	/**
	 * @return TRUE if this plugin can produce (i.e. research) technology @p
	 * type. This is the case if this plugin is a producer of @p type and
	 * all requirements are fullfilled and if @p type has not yet been
	 * researched.
	 **/
	bool canCurrentlyProduceTechnology(unsigned long int type) const;

	/**
	 * @return The percentage of the production progress. 0 means the
	 * production just started, 100 means the production is completed.
	 **/
	double productionProgress() const;

	virtual void advance(unsigned int);

	virtual bool saveAsXML(QDomElement& root) const;
	virtual bool loadFromXML(const QDomElement& root);

	virtual void unitDestroyed(Unit*);
	virtual void itemRemoved(BosonItem*) { }

protected:
	/**
	 * Remove first occurance of type ID id in the production list. Does not
	 * remove anything if id is not in the list.
	 *
	 * This will re-fund any minerals/oil paid for this production so far.
	 **/
	bool removeProduction(ProductionType type, unsigned long int id);

	/**
	 * @overload
	 * Remove the first item from the production list.
	 **/
	bool removeProduction();

	/**
	 * Like @ref removeProduction but won't re-fund the minerals/oil used
	 * for this production.
	 **/
	void removeCompletedProduction();

private:
	/**
	 * Helper method for @ref advance. This is called when the production is
	 * completed and will e.g. place the production (or whatever is
	 * applicable)
	 **/
	void productionCompleted();

private:
	QValueList<QPair<ProductionType, unsigned long int> > mProductions;
	KGameProperty<unsigned int> mProductionState;
	KGameProperty<unsigned long int> mMineralsPaid;
	KGameProperty<unsigned long int> mOilPaid;
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
	 * Called from @ref Unit::advanceIdle. Repair the next unit that is in
	 * range. An alternative name might be "advance", just like in @ref
	 * ProducePlugin but since we don't have a WorkRepair in @ref Unit there
	 * is no advance call for it from @ref BosonCanvas::slotAdvance either.
	 *
	 * @ref Unit::advanceIdle is used for it instead.
	 **/
	void repairInRange();


	// does nothing, yet. plugin is experimental anyway.
	virtual void advance(unsigned int) {}

	virtual bool saveAsXML(QDomElement& root) const;
	virtual bool loadFromXML(const QDomElement& root);

	virtual void unitDestroyed(Unit*);
	virtual void itemRemoved(BosonItem*);

private:
};

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

	void bomb(int weaponId, BoVector2Fixed pos);

	virtual void advance(unsigned int);

	virtual bool saveAsXML(QDomElement& root) const;
	virtual bool loadFromXML(const QDomElement& root);

	virtual void unitDestroyed(Unit*) {}
	virtual void itemRemoved(BosonItem*) {}

private:
	BosonWeapon* mWeapon; // FIXME: must be saved on Unit::save()
	KGameProperty<bofixed> mTargetX;
	KGameProperty<bofixed> mTargetY;
	KGameProperty<bofixed> mDropDist;
	KGameProperty<bofixed> mLastDistFromDropPoint;
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

	virtual void unitDestroyed(Unit*) {}
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
	virtual void advance(unsigned int advanceCallsCount);

	bool isUsableTo(const HarvesterPlugin* harvester) const;

	/**
	 * @return How much minerals are left here. -1 means unlimited.
	 **/
	int minerals() const;

	/**
	 * @return How much oil is left here. -1 means unlimited.
	 **/
	int oil() const;

	/**
	 * Mine minerals. The amount of @ref minerals is (if limited) reduced
	 * by the returned value.
	 * @return The amount of mined minerals
	 **/
	unsigned int mineMinerals(const HarvesterPlugin* harvester);

	/**
	 * Mine oil. The amount of @ref oil is (if limited) reduced
	 * by the returned value.
	 * @return The amount of mined oil
	 **/
	unsigned int mineOil(const HarvesterPlugin* harvester);

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

	virtual void unitDestroyed(Unit*);
	virtual void itemRemoved(BosonItem*);

protected:
	/**
	 * @return The amount of resources that @ref mineMinerals or @ref
	 * mineOil will mine.
	 **/
	unsigned int mineStep(const HarvesterPlugin* harvester, int resourcesAvailable) const;

private:
	KGameProperty<int> mOil;
	KGameProperty<int> mMinerals;
};

class RefineryPlugin : public UnitPlugin
{
public:
	RefineryPlugin(Unit* owner);
	~RefineryPlugin();

	virtual int pluginType() const { return Refinery; }

	virtual bool loadFromXML(const QDomElement& root);
	virtual bool saveAsXML(QDomElement& root) const;
	virtual void advance(unsigned int advanceCallsCount);

	bool isUsableTo(const HarvesterPlugin* harvester) const;

	bool canRefineMinerals() const;
	bool canRefineOil() const;

	/**
	 * Try to refine @p minerals.
	 * @return The minerals that got actually refined. The harvester should
	 * reduce it's resources by exactly that amount only
	 **/
	unsigned int refineMinerals(unsigned int minerals);

	/**
	 * Try to refine @p oil.
	 * @return The oil that got actually refined. The harvester should
	 * reduce it's resources by exactly that amount only
	 **/
	unsigned int refineOil(unsigned int oil);

	virtual void unitDestroyed(Unit*);
	virtual void itemRemoved(BosonItem*);
};

class AmmunitionStoragePlugin : public UnitPlugin
{
public:
	AmmunitionStoragePlugin (Unit* owner);
	~AmmunitionStoragePlugin();

	virtual int pluginType() const { return AmmunitionStorage; }

	virtual bool loadFromXML(const QDomElement& root);
	virtual bool saveAsXML(QDomElement& root) const;
	virtual void advance(unsigned int advanceCallsCount);

	virtual void unitDestroyed(Unit*);
	virtual void itemRemoved(BosonItem*);

	/**
	 * @return TRUE if the ammo @p type must be picked up from the unit that has
	 * this plugin. If FALSE, the ammo can be used by a unit even if it is
	 * on the other side of the map.
	 **/
	bool mustBePickedUp(const QString& type) const;

	bool canStore(const QString& type) const;

	unsigned long int requestAmmunitionGlobally(const QString& type, unsigned long int requested);

	/**
	 * Like @ref requestAmmunitionGlobally, but this implies that the unit
	 * requesting the ammunition is close enough to actually pick the ammo
	 * up itself.
	 *
	 * @param picksUp The unit that picks up the ammunition. Note that the
	 * plugin may decide not to give any ammunition to that unit. See @p
	 * denies
	 * @param denied If non-NULL, then this is set to TRUE if the plugin
	 * decided not to give ammunition to the unit @p picksUp, most likely it
	 * is not close enough.
	 **/
	unsigned long int pickupAmmunition(Unit* picksUp, const QString& type, unsigned long int requested, bool* denied = 0);

	unsigned long int ammunitionStored(const QString& type) const;

	/**
	 * Try to put an amount of @p ammo of type @p type into the storage.
	 * This is usually successful, but if there is limited capacity, this
	 * might fail
	 * @return The amount of ammo that was actually stored. Always <= @p
	 * ammo.
	 **/
	unsigned long int tryToFillStorage(const QString& type, unsigned long int ammo);

protected:
	int changeAmmunition(const QString& type, int change);

	/**
	 * @internal
	 **/
	unsigned long int giveAmmunition(const QString& type, unsigned long int requested);

private:
	QMap<QString, unsigned long int> mAmmunitionStorage;
};

class RadarPlugin : public UnitPlugin
{
public:
	RadarPlugin (Unit* owner);
	~RadarPlugin();

	virtual int pluginType() const { return Radar; }

	virtual bool loadFromXML(const QDomElement& root);
	virtual bool saveAsXML(QDomElement& root) const;
	virtual void advance(unsigned int advanceCallsCount);

	virtual void unitDestroyed(Unit*);
	virtual void itemRemoved(BosonItem*);

	/**
	 * Call this whenever unit's health changes. It recalculates radar's
	 *  transmitted power and range (which are dependant on health)
	 **/
	void unitHealthChanged();

	/**
	 * @return Power transmitted by the transmitter antenna.
	 * Note that this has _no_ correspondance to the power resource.
	 **/
	float transmittedPower() const { return mTransmittedPower; }

	/**
	 * @return Minimum received power to notice the target
	 **/
	float minReceivedPower() const;

	/**
	 * @return Range of the radar
	 * It's unlikely that any objects outside this range would be detected
	 **/
	bofixed range() const { return mRange; }

	bool detectsLandUnits() const;
	bool detectsAirUnits() const;

private:
	bofixed mRange;
	float mTransmittedPower;
};

#endif
