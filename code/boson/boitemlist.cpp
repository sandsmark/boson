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
#include <kdebug.h>

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
			if (!includeMoving && u->isMoving()) {
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
 //  makes this method much faster

 // Flying units never collide - different altitudes
 if(forUnit && forUnit->isFlying()) {
	return false;
 }

 for (ConstIterator it = begin(); it != end(); ++it) {
	if (RTTI::isUnit((*it)->rtti())) {
		Unit* u = (Unit*)*it;
		if (forUnit == u) {
			continue;
		}
		if (u->isDestroyed()) {
			continue;
		}
		if (u->isFlying()) {
			continue;
		}
		if (u->isMoving()) {
			if (includeMoving) {
				return true;
			}
			continue;
		}
		return true;
	}
 }

 return false;
}

