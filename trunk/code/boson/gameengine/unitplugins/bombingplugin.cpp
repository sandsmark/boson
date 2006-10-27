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
#include "bombingplugin.h"

#include "../../bomemory/bodummymemory.h"
#include "../defines.h"
#include "unit.h"
#include "unitproperties.h"
#include "pluginproperties.h"
#include "bosoncanvas.h"
#include "bodebug.h"
#include "bosonweapon.h"
#include "bosonpath.h"
#include "unitorder.h"
#include "../bo3dtools.h"

#include <qdom.h>

#include <math.h>

BombingPlugin::BombingPlugin(Unit* owner) : UnitPlugin(owner)
{
 owner->registerData(&mTargetX, Unit::IdBombingTargetX);
 owner->registerData(&mTargetY, Unit::IdBombingTargetY);
 owner->registerData(&mDropDist, Unit::IdBombingDropDist);
 owner->registerData(&mLastDistFromDropPoint, Unit::IdBombingLastDistFromDropPoint);
 mWeapon = 0;
 mTargetX = 0;
 mTargetY = 0;
 mDropDist = 0;
 mLastDistFromDropPoint = 0;
}

BombingPlugin::~BombingPlugin()
{
}

void BombingPlugin::bomb(int weaponId, BoVector2Fixed pos)
{
 boDebug() << k_funcinfo << "wep: " << weaponId << "; pos: (" << pos.x() << "; " << pos.y() << ")" << endl;
 BosonWeapon* w = unit()->weapon(weaponId);
 if (!w) {
	boError() << k_funcinfo << "No weapon with id " << weaponId << endl;
	return;
 }
 if (w->properties()->shotType() != BosonShot::Bomb) {
	boError() << k_funcinfo << "Weapon with id " << weaponId << " is not a bomb" << endl;
	return;
 }

 if (!unit()->isFlying()) {
	boWarning() << k_funcinfo << "Only flying units can drop bombs" << endl;
	return;
 }

 int cellX = (int)pos.x();
 int cellY = (int)pos.y();
 if (!canvas()->cell(cellX, cellY)) {
	boError() << k_funcinfo << cellX << "," << cellY << " is no valid cell!" << endl;
	return;
 }

 // This is where unit has to be when making the drop
 mTargetX = pos.x();
 mTargetY = pos.y();

 if (!unit()->addCurrentSuborder(new UnitMoveOrder(pos, 0, false))) {
	boError() << k_funcinfo << "Moving failed. Now what?" << endl;
	unit()->currentSuborderDone(false);
	return;
 } else {
	unit()->pathInfo()->slowDownAtDest = false;
	unit()->pathInfo()->moveAttacking = false;
	// Instead of setting unit's advanceWork to WorkMove, we call
	//  unit()->advanceMove() ourselves to retain finer control
 }

 // Calculate drop distance (how far from the target we drop the bomb)
 bofixed height = unit()->unitProperties()->preferredAltitude();  // How high from the ground we release the bomb
 // t = sqrt(2s / a)
 bofixed droptime = sqrt(2 * height / w->properties()->accelerationSpeed());
 mDropDist = droptime * unit()->maxSpeed();  // In advance calls
 boDebug() << k_funcinfo << "Target point: (" << mTargetX << "; " << mTargetY << "); dropdist: " << mDropDist << endl;
 mWeapon = w;

 unit()->setPluginWork(UnitPlugin::Bombing);
}

void BombingPlugin::advance(unsigned int advanceCalls)
{
 // Check if we're at the drop point
 // Unit's center point
 BoVector2Fixed totarget(mTargetX - unit()->centerX(), mTargetY - unit()->centerY());
 bofixed dist = totarget.length();
 bofixed distfromdroppoint = QABS(dist - mDropDist);

 if ((distfromdroppoint < 3) && (distfromdroppoint > mLastDistFromDropPoint)) {
	// We're at drop point. Drop the bomb
	if (!mWeapon->reloaded()) {
		//boError() << k_funcinfo << "Weapon not reloaded!" << endl;
		unit()->moveIdle();
		return;
	}

	boDebug() << k_funcinfo << "Bomb ready. Dropping..." << endl;
	bofixed hvelox, hveloy;
	Bo3dTools::pointByRotation(&hvelox, &hveloy, unit()->rotation(), unit()->speed());
	mWeapon->dropBomb(BoVector2Fixed(hvelox, hveloy));

	// And get the hell out of there
	// Go away from bomb's explosion radius
	bofixed dist = mWeapon->damageRange() + unit()->width() / 2;
	boDebug() << k_funcinfo << "Getaway dist: " << dist << "; rot: " << unit()->rotation() << endl;
	bofixed newx, newy;
	Bo3dTools::pointByRotation(&newx, &newy, unit()->rotation(), dist);
	newx = unit()->centerX() - newx;
	newy = unit()->centerY() - newy;

	// Make sure coords are valid
	// TODO: if current getaway point is off the canvas (or too close to the
	//  edge), rotate a bit and select new getaway point.
	newx = QMAX(unit()->width() / 2, QMIN(newx, (canvas()->mapWidth() - 1) - unit()->width() / 2));
	newy = QMAX(unit()->height() / 2, QMIN(newy, (canvas()->mapHeight() - 1) - unit()->height() / 2));

	// FIXME: hackish
	unit()->currentSuborderDone(true);
	boDebug() << k_funcinfo << "Getaway point is at (" << newx << "; " << newy << ")" << endl;
	if (!unit()->addCurrentSuborder(new UnitMoveOrder(BoVector2Fixed(newx, newy), 0, false))) {
		boWarning() << k_funcinfo << "Aargh! Can't move away from drop-point!" << endl;
	} else {
		unit()->pathInfo()->moveAttacking = false;
		//unit()->setWork(Unit::WorkMove);  // We don't want to return here anymore
	}

	mWeapon = 0;
	boDebug() << k_funcinfo << "returning" << endl;

 } else {
	// Continue moving towards the target
	mLastDistFromDropPoint = distfromdroppoint;
	unit()->advanceMove(advanceCalls);
	return;
 }
}

bool BombingPlugin::saveAsXML(QDomElement& root) const
{
 unsigned int weaponId = 0;
 // TODO
 root.setAttribute(QString::fromLatin1("Weapon"), weaponId);
 return true;
}

bool BombingPlugin::loadFromXML(const QDomElement& root)
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


