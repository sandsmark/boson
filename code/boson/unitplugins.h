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

#include <kgame/kgameproperty.h>
#include <kgame/kgamepropertylist.h>

class Unit;

class ProductionPlugin
{
public:
	ProductionPlugin(Unit* unit);

	inline Unit* unit() const { return mUnit; }

	/**
	 * @return Whether there are any productions pending for this unit.
	 * Always FALSE if unitProperties()->canProduce() is FALSE.
	 **/
	inline bool hasProduction() const
	{
		return !mProductions.isEmpty();
	}

	/**
	 * @return The unit type ID (see @ref UnitProperties::typeId) of the
	 * completed production (if any).
	 **/
	unsigned long int completedProduction() const;

	/**
	 * @return The unit type ID of the current production. -1 if there is no
	 * production.
	 **/
	inline unsigned long int currentProduction() const
	{
		if (!hasProduction()) {
			return 0;
		}
		return mProductions.first();
	}

	/**
	 * Remove the first item from the production list.
	 **/
	void removeProduction(); // removes first item

	/**
	 * Remove first occurance of unitType in the production list. Does not
	 * remove anything if unitType is not in the list.
	 **/
	void removeProduction(unsigned long int unitType);

	/**
	 * Add unitType (see @ref UnitProprties::typeId) to the construction
	 * list.
	 **/
	void addProduction(unsigned long int unitType);

	QValueList<unsigned long int> productionList() const { return mProductions; }
	bool contains(unsigned long int unitType) { return productionList().contains(unitType); }

	/**
	 * @return The percentage of the production progress. 0 means the
	 * production just started, 100 means the production is completed.
	 **/
	double productionProgress() const;

	bool canPlaceProductionAt(const QPoint& pos);

	void advance();

private:
	Unit* mUnit;
	KGamePropertyList<unsigned long int> mProductions;
	KGameProperty<unsigned int> mProductionState;
};

/**
 * Experimental plugin. At the current state id doesn't make any sense, since I
 * don't use any member variables anymore...
 *
 * Nevertheless I don't entegrate the functionality into Unit since it should
 * get some more testing
 **/
class RepairPlugin
{
public:
	RepairPlugin(Unit* owner);

	Unit* unit() const { return mUnit; }

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

private:
	Unit* mUnit;
//	KGameProperty<unsigned int> mAdvanceCount;
//	KGamePropertyList<unsigned long int> mRepairList;
};

#endif
