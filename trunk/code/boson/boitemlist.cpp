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

#include "rtti.h"
#include "unit.h"

#include <qcanvas.h>

QValueList<QCanvasItem*> BoItemList::items(bool collidingOnly, bool includeMoving, Unit* forUnit) const 
{
 QValueList<QCanvasItem*> list;
 QValueList<Unit*> unitList = units(collidingOnly, includeMoving, forUnit, &list);

 //TODO: once we have non-unit items we need to test if they are actually
 //interesting for collision detection!

 QValueList<Unit*>::Iterator it = unitList.begin();
 for (; it != unitList.end(); ++it) {
	list.append((QCanvasItem*)*it);
 }
 return list;
}

QValueList<Unit*> BoItemList::units(bool collidingOnly, bool includeMoving, Unit* forUnit, QValueList<QCanvasItem*>* nonUnits) const 
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
			if (includeMoving && u->isMoving()) {
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
 // Note that we can make performance improvements here!
 // We currently first create the list (in items()) and then test if it is
 // empty. If we would place the code from items() here we could return as soon
 // as the first item appears. In case of lots of items this would be faster.
 //
 // But since that means code duplication and since we currently don't have many
 // units on a cell usually we do it the "slow" way.
 QValueList<QCanvasItem*> list;
 list = items(true, includeMoving, forUnit);
 if (forUnit) {
	list.remove((QCanvasItem*)forUnit);
 }
 return (list.count() != 0);
}

