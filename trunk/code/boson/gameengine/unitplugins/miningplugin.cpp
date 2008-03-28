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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#include "miningplugin.h"

#include "../../bomemory/bodummymemory.h"
#include "../defines.h"
#include "unit.h"
#include "unitproperties.h"
#include "pluginproperties.h"
#include "speciestheme.h"
#include "player.h"
#include "playerio.h"
#include "bosoncanvas.h"
#include "boson.h"
#include "cell.h"
#include "bodebug.h"
#include "bosonweapon.h"
#include "unitorder.h"
#include "../bo3dtools.h"

#include <qdom.h>

#include <math.h>


MiningPlugin::MiningPlugin(Unit* owner) : UnitPlugin(owner)
{
 mWeapon = 0;
 owner->registerData(&mPlacingCounter, Unit::IdMinePlacingCounter);
 mPlacingCounter = 0;
}

MiningPlugin::~MiningPlugin()
{
}

// How many game ticks does it take to place mine
// FIXME: make configurable in BosonWeapon
#define MINE_PLACE_TIME 10

void MiningPlugin::mine(int weaponId)
{
 boDebug() << k_funcinfo << "wep: " << weaponId << endl;
 BosonWeapon* w = unit()->weapon(weaponId);
 if (!w) {
	boError() << k_funcinfo << "No weapon with id " << weaponId << endl;
	return;
 }
 if (w->properties()->shotType() != BosonShot::Mine) {
	boError() << k_funcinfo << "Weapon with id " << weaponId << " is not a mine" << endl;
	return;
 }

// unit()->stopMoving();

 mWeapon = w;
 mPlacingCounter = MINE_PLACE_TIME;

 // Mine will be layed in advance()

 unit()->setPluginWork(UnitPlugin::Mining);
}

void MiningPlugin::advance(unsigned int)
{
 // Lay the mine
 if (mWeapon->reloaded()) {
	// Don't place the mine immediately
	if (mPlacingCounter > 0) {
//		boDebug() << k_funcinfo << "mPlacingCounter: " << mPlacingCounter << endl;
		mPlacingCounter = mPlacingCounter - 1;
		return;
	}

	boDebug() << k_funcinfo << "Mine ready. Placing..." << endl;
	mWeapon->layMine();

	// Go one cell away from the mine. Maybe go away from explosion radius?
	// FIXME: code taken from BombingPlugin. This could probably be written better
	bofixed dist = 1.0f + unit()->width() / 2.0f;
	boDebug() << k_funcinfo << "Getaway dist: " << dist << "; rot: " << unit()->rotation() << endl;
	bofixed oldx = unit()->centerX();
	bofixed oldy = unit()->centerY();
	bofixed newx, newy;
	bool couldmove = false;

	// TODO: quite messy code, maybe it can be cleaned up somehow
	for (int i = 0; i <= 7; i++) {
		newx = oldx;
		newy = oldy;

		// First try to go straight ahead, then try go to 45 degrees right, then
		//  45 degrees left, then 2 * 45 degrees right, then 2 * 45 degrees left, etc
		int rotadd = ((i + 1) / 2) * 45;
		if (i % 2 == 0) {
			rotadd = -rotadd;
		}
		int rot = ((int)unit()->rotation() + rotadd) % 360;

		boDebug() << k_funcinfo << "i: " << i << "; rotadd: " << rotadd << "; rot: " << rot << endl;

		if (rot >= 45 && rot <= 135) {
			newx += dist;
		} else if (rot >= 225 && rot <= 315) {
			newx -= dist;
		}
		if (rot <= 45 || rot >= 315) {
			newy -= dist;
		} else if (rot >= 135 && rot <= 225) {
			newy += dist;
		}

		// Make sure coords are valid
		newx = QMAX(bofixed(0), QMIN(newx, bofixed((canvas()->mapWidth() - 1))));
		newy = QMAX(bofixed(0), QMIN(newy, bofixed((canvas()->mapHeight() - 1))));

		boDebug() << k_funcinfo << "i: " << i << "; Getaway point is at (" << newx << "; " << newy << ")" << endl;
		// FIXME: hackish
		unit()->currentSuborderDone(true);
		boDebug() << k_funcinfo << "Getaway point is at (" << newx << "; " << newy << ")" << endl;
		if (!unit()->addCurrentSuborder(new UnitMoveOrder(BoVector2Fixed(newx, newy), 0, false))) {
			couldmove = true;
			break;
		}
	}

	if (!couldmove) {
		boDebug() << k_funcinfo << "Can't move away!" << endl;
		unit()->setAdvanceWork(Unit::WorkIdle);  // We don't want to return here anymore
	}

	mWeapon = 0;
	boDebug() << k_funcinfo << "returning" << endl;
	return;
 } else {
//	boDebug() << k_funcinfo << "Weapon not yet reloaded" << endl;
 }
}

bool MiningPlugin::saveAsXML(QDomElement& root) const
{
 unsigned int weaponId = 0;
 // TODO
 root.setAttribute(QString::fromLatin1("Weapon"), weaponId);
 return true;
}

bool MiningPlugin::loadFromXML(const QDomElement& root)
{
 unsigned int weaponId = 0;
 bool ok = false;
 weaponId = root.attribute(QString::fromLatin1("Weapon")).toUInt(&ok);
 if (!ok) {
	boError() << k_funcinfo << "Invalid number for Weapon attribute" << endl;
	return false;
 }
 // TODO
 // mWeapon = ...;
 return true;
}


