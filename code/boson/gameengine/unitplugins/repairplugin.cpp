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
#include "repairplugin.h"

#include "../../bomemory/bodummymemory.h"
#include "unit.h"
#include "unitorder.h"
#include "bodebug.h"

RepairPlugin::RepairPlugin(Unit* unit) : UnitPlugin(unit)
{
}

RepairPlugin::~RepairPlugin()
{
}

void RepairPlugin::repair(Unit* u)
{
 boDebug() << k_funcinfo << endl;
 if (unit()->isFacility()) {
	// TODO: do we also need to clear u's current orders?
	if (!u->addToplevelOrder(new UnitMoveToUnitOrder(unit()))) {
		boDebug() << k_funcinfo << u->id() << " cannot find a way to repairyard" << endl;
		unit()->currentSuborderDone(false);
	}
 } else {
	if (!unit()->addCurrentSuborder(new UnitMoveToUnitOrder(u))) {
		boDebug() << k_funcinfo << "Cannot find way to " << u->id() << endl;
		unit()->currentSuborderDone(false);
	}
 }
}

void RepairPlugin::repairInRange()
{
// boDebug() << k_funcinfo << endl;
 //TODO: support for friendly non-player (i.e. allied) units

 // TODO: once we started repairing a unit also repair it in the next call,
 // until it isn't in range anymore or is repaired
/* BoItemList list = unit()->unitsInRange();
 for (unsigned int i = 0; i < list.count(); i++) {
	Unit* u = (Unit*)list[i];
	if (u->health() >= u->unitProperties()->health()) {
		continue;
	}
	if (!player()->isEnemy(u->owner())) {
		boDebug() << k_funcinfo << "repair " << u->id() << endl;
		int diff = u->health() - u->unitProperties()->health();
		if (diff > 0) {
			boError() << k_funcinfo << "health > maxhealth" << endl;
			continue;
		}*/
/*		if (diff < unit()->weaponDamage()) {
			// usually the case
			u->setHealth(u->health() - unit()->weaponDamage());
		} else {
			u->setHealth(u->health() - diff);
		}*/
		/*return; // only one unit at once
	}
 }*/
}

bool RepairPlugin::saveAsXML(QDomElement& root) const
{
 Q_UNUSED(root);
 return true;
}

bool RepairPlugin::loadFromXML(const QDomElement& root)
{
 Q_UNUSED(root);
 return true;
}

void RepairPlugin::unitDestroyed(Unit*)
{
}

void RepairPlugin::itemRemoved(BosonItem*)
{
}


