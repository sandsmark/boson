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

#include "boitemlist.h"
#include "boitemlisthandler.h"

#include "bodebug.h"
#include "rtti.h"
#include "unit.h"
#include "bosonpath.h"

BoItemList::BoItemList()
{
 registerList();
 mLandOccupied = false;
 mAirOccupied = false;
}

BoItemList::BoItemList(const BoItemList& list, bool _registerList)
{
 if (_registerList) {
	registerList();
 }
 mList = list.mList;
 mLandOccupied = list.mLandOccupied;
 mAirOccupied = list.mAirOccupied;
}

BoItemList::~BoItemList()
{
 BoItemListHandler* handler = BoItemListHandler::itemListHandler();
 if (handler) {
	handler->unregisterList(this);
 } else {
	boWarning() << k_funcinfo << "NULL item list handler" << endl;
 }
}

void BoItemList::registerList()
{
 BoItemListHandler* handler = BoItemListHandler::itemListHandler();
 if (handler) {
	handler->registerList(this);
 } else {
	boWarning() << k_funcinfo << "NULL item list handler" << endl;
 }
}

BosonItem* BoItemList::findItem(unsigned long int id) const
{
 ConstIterator it = begin();
 for (; it != end(); ++it) {
	if ((*it)->id() == id) {
		return *it;
	}
 }
 return 0;
}

QValueList<BosonItem*> BoItemList::items(bool collidingOnly, bool includeMoving, Unit* forUnit) const
{
 QValueList<BosonItem*> list;
 QValueList<Unit*> unitList = units(collidingOnly, includeMoving, forUnit, &list);

 //TODO: once we have non-unit items we need to test if they are actually
 //interesting for collision detection!

 QValueList<Unit*>::Iterator it = unitList.begin();
 for (; it != unitList.end(); ++it) {
	list.append((BosonItem*)*it);
 }
 return list;
}

QValueList<Unit*> BoItemList::units(bool collidingOnly, bool includeMoving, Unit* forUnit, QValueList<BosonItem*>* nonUnits) const
{
 QValueList<Unit*> list;
 ConstIterator it = begin();
 for (; it != end(); ++it) {
	if ((Unit*)forUnit == (Unit*)*it) {
		continue;
	}
	if (RTTI::isUnit((*it)->rtti())) {
		Unit* u = (Unit*)*it;
		if (u->isDestroyed()) {
			continue;
		}
		if (collidingOnly) {
			if (!includeMoving && u->movingStatus() != UnitBase::Standing) {
				continue;
			}
			if (!forUnit || (!forUnit->isFlying() && !u->isFlying())) {
				list.append((Unit*)u);
			}
		} else {
			list.append((Unit*)*it);
		}
	} else if (nonUnits) {
		nonUnits->append(*it);
	}
 }
 return list;
}

bool BoItemList::isOccupied(Unit* forUnit, bool includeMoving) const
{
 // Note that some code here is taken from units() (code duplication), but it
 // makes this method much faster

 bool flying = forUnit->isFlying();

 for (ConstIterator it = begin(); it != end(); ++it) {
	if (RTTI::isUnit((*it)->rtti())) {
		Unit* u = (Unit*)*it;
		if (forUnit == u) {
			continue;
		}
		if (u->isFlying() != flying) {
			continue;
		}
		if (!includeMoving) {
			if (u->movingStatus() != UnitBase::Standing) {
				continue;
			}
		}
		return true;
	}
 }

 return false;
}

bool BoItemList::isOccupied(bool includeMoving) const
{
 // AB: this is basically the same as the above isOccupied, but it doesn't take
 // a forUnit param. we have even more code duplication here, but this is
 // necessary, since isOccupied is used by pathfinding and must be fast.
 // rivol: this method is never used by pathfinder.

 for (ConstIterator it = begin(); it != end(); ++it) {
	if (RTTI::isUnit((*it)->rtti())) {
		Unit* u = (Unit*)*it;
		if (!includeMoving) {
			if (u->movingStatus() != UnitBase::Standing) {
				continue;
			}
		}
		return true;
	}
 }

 return false;
}

void BoItemList::isOccupied(Unit* forUnit, bool& hasmoving, bool& hasany) const
{
 // TODO: those values could be cached

 bool flying = forUnit->isFlying();
 hasmoving = false;
 hasany = false;

 for (ConstIterator it = begin(); it != end(); ++it) {
	if (RTTI::isUnit((*it)->rtti())) {
		Unit* u = (Unit*)*it;
		if (forUnit == u) {
			continue;
		}
		if (u->isFlying() != flying) {
			continue;
		}
		if (u->movingStatus() != UnitBase::Standing) {
			hasmoving = true;
			hasany = true;
			return;
		}
		hasany = true;
	}
 }
}

void BoItemList::recalculateLandOccupiedStatus()
{
#ifdef PATHFINDER_TNG
 mLandOccupied = false;

 for (ConstIterator it = begin(); it != end(); ++it) {
	if (RTTI::isUnit((*it)->rtti())) {
		Unit* u = (Unit*)*it;
		if (u->isFlying()) {
			continue;
		}
		if (u->movingStatus() != UnitBase::Standing) {
			continue;
		}
		mLandOccupied = true;
		return;
	}
 }
#endif // PATHFINDER_TNG
}

void BoItemList::recalculateAirOccupiedStatus()
{
#ifdef PATHFINDER_TNG
 mAirOccupied = false;

 for (ConstIterator it = begin(); it != end(); ++it) {
	if (RTTI::isUnit((*it)->rtti())) {
		Unit* u = (Unit*)*it;
		if (!u->isFlying()) {
			continue;
		}
		if (u->movingStatus() != UnitBase::Standing) {
			continue;
		}
		mAirOccupied = true;
		return;
	}
 }
#endif // PATHFINDER_TNG
}

bofixed BoItemList::passageCostLand() const
{
 // FIXME: this rather belongs to pathfinder, but it's here because pathfinder
 //  doesn't have direct access to list of items on a cell and so it is faster
 //  like this
 // TODO: we shouldn't only check which units are on this cell, but also how
 //  much of the cell they occupy and change cost accordingly. E.g. if waiting
 //  unit occupies 25% of the cell, we should use  WAITING_COST * 0.25  instead
 //  of just  WAITING_COST.
 //  Maybe use maximum of costs of all units on this cell, not sum of them?
 bofixed cost = 0;
#ifdef PATHFINDER_TNG
 for (ConstIterator it = begin(); it != end(); ++it) {
	if (RTTI::isUnit((*it)->rtti())) {
		Unit* u = (Unit*)*it;
		if (u->isFlying()) {
			continue;
		}
		if (u->movingStatus() == UnitBase::Standing) {
			cost += PF_TNG_COST_STANDING_UNIT;
		} else if(u->movingStatus() == UnitBase::Moving) {
			cost += PF_TNG_COST_MOVING_UNIT;
		} else if(u->movingStatus() == UnitBase::Waiting) {
			cost += PF_TNG_COST_WAITING_UNIT;
		} else if(u->movingStatus() == UnitBase::Engaging) {
			cost += PF_TNG_COST_ENGAGING_UNIT;
		} else if(u->movingStatus() == UnitBase::MustSearch) {
			cost += PF_TNG_COST_MUSTSEARCH_UNIT;
		} else if(u->movingStatus() == UnitBase::Removing) {
			// Unit is being deleted, but it hasn't been removed from cells yet.
			//  Do nothing
		} else {
			// Internal moving status. This shouldn't be reached
			boError() << k_funcinfo << "Internal moving status " << u->movingStatus() <<
					" for unit with id " << u->id() << endl;
			cost += PF_TNG_COST_INTERNAL_UNIT;
		}
	}
 }
#endif // PATHFINDER_TNG
 return cost;
}

bofixed BoItemList::passageCostAir() const
{
 // FIXME: this rather belongs to pathfinder, but it's here because pathfinder
 //  doesn't have direct access to list of items on a cell and so it is faster
 //  like this
 // TODO: we shouldn't only check which units are on this cell, but also how
 //  much of the cell they occupy and change cost accordingly. E.g. if waiting
 //  unit occupies 25% of the cell, we should use  WAITING_COST * 0.25  instead
 //  of just  WAITING_COST.
 //  Maybe use maximum of costs of all units on this cell, not sum of them?
 bofixed cost = 0;
#ifdef PATHFINDER_TNG
 for (ConstIterator it = begin(); it != end(); ++it) {
	if (RTTI::isUnit((*it)->rtti())) {
		Unit* u = (Unit*)*it;
		if (!u->isFlying()) {
			continue;
		}
		if (u->movingStatus() == UnitBase::Standing) {
			cost += PF_TNG_COST_STANDING_UNIT;
		} else if(u->movingStatus() == UnitBase::Moving) {
			cost += PF_TNG_COST_MOVING_UNIT;
		} else if(u->movingStatus() == UnitBase::Waiting) {
			cost += PF_TNG_COST_WAITING_UNIT;
		} else if(u->movingStatus() == UnitBase::Engaging) {
			cost += PF_TNG_COST_ENGAGING_UNIT;
		} else if(u->movingStatus() == UnitBase::MustSearch) {
			cost += PF_TNG_COST_MUSTSEARCH_UNIT;
		} else if(u->movingStatus() == UnitBase::Removing) {
			// Unit is being deleted, but it hasn't been removed from cells yet.
			//  Do nothing
		} else {
			// Internal moving status. This shouldn't be reached
			boError() << k_funcinfo << "Internal moving status " << u->movingStatus() <<
					" for unit with id " << u->id() << endl;
			cost += PF_TNG_COST_INTERNAL_UNIT;
		}
	}
 }
#endif // PATHFINDER_TNG
 return cost;
}

