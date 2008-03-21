/*
    This file is part of the Boson game
    Copyright (C) 1999-2000 Thomas Capricelli (capricel@email.enst.fr)
    Copyright (C) 2001-2005 Andreas Beckermann (b_mann@gmx.de)

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

#include "bosoncanvas.h"
#include "bosoncanvas.moc"

#include "../bomemory/bodummymemory.h"
#include "player.h"
#include "cell.h"
#include "unit.h"
#include "unitplugins/radarplugin.h"
#include "unitplugins/radarjammerplugin.h"
#include "unitplugins/productionplugin.h"
#include "unitplugins/resourcemineplugin.h"
#include "bosonmap.h"
#include "unitproperties.h"
#include "speciestheme.h"
#include "boitemlist.h"
#include "boitemlisthandler.h"
#include "defines.h"
#include "bosonshot.h"
#include "bosonweapon.h"
#include "bosonstatistics.h"
#include "bosonprofiling.h"
#include "bosoncanvasstatistics.h"
#include "bodebug.h"
#include "boson.h"
#include "boevent.h"
#include "boeventlistener.h"
#include "bosonpropertyxml.h"
#include "bosonpath.h"
#include "bowater.h"
#include "bocanvasquadtreenode.h"
#include "playerio.h"

#include <klocale.h>
#include <kgame/kgamepropertyhandler.h>

#include <qpointarray.h>
#include <qdatastream.h>
#include <qdom.h>
#include <qintdict.h>

#include <math.h>

ItemType ItemType::typeForUnit(unsigned long int unitType)
{
 return ItemType(unitType);
}
ItemType ItemType::typeForExplosion()
{
 return ItemType(BosonShot::Explosion, 0, 0);
}
ItemType ItemType::typeForFragment()
{
 return ItemType(BosonShot::Fragment, 0, 0);
}
ItemType ItemType::typeForShot(unsigned long int shotType, unsigned long int unitType, unsigned long int weaponPropertyId)
{
 return ItemType(shotType, unitType, weaponPropertyId);
}

class BoCanvasSightManager;

class BosonCanvas::BosonCanvasPrivate
{
public:
	BosonCanvasPrivate()
		: mAllItems(BoItemList(0, false))
	{
		mQuadTreeCollection = 0;
		mStatistics = 0;
		mMap = 0;

		mProperties = 0;

		mPathFinder = 0;

		mEventListener = 0;
		mSightManager = 0;
	}
	BoCanvasQuadTreeCollection* mQuadTreeCollection;
	BosonCanvasStatistics* mStatistics;

	QPtrList<Unit> mDestroyedUnits;

	BosonMap* mMap; // just a pointer - no memory allocated

	BoItemList mAllItems;

	// by default ALL items are in "work" == -1. if an item changes its work
	// (i.e. it is a unit and it called setAdvanceWork()) then it will go to
	// another list (once slotAdvance() reaches its end)
	QMap<int, QPtrList<BosonItem> > mWork2AdvanceList;
	QPtrList<BosonItem> mChangeAdvanceList; // work has been changed - request to change advance list

	KGamePropertyHandler* mProperties;

	// AB: we _really_ need ulong here. for _very_ long games (maybe more
	// than a few days playing without break and a lot of shots) we might
	// even exceed that.
	// at the moment we won't be able to have so long games and won't have
	// _that_ big battles. in the future (if we maybe add even more items)
	// we might use unsigned long long for this.
	// but for now we are safe.
	KGameProperty<unsigned long int> mNextItemId;

	BosonPath* mPathFinder;

	BoCanvasEventListener* mEventListener;

	QIntDict<BosonMoveData> mUnitProperties2MoveData;

	BoCanvasSightManager* mSightManager;
};


class BoCanvasSightManager
{
public:
	BoCanvasSightManager(BosonCanvas* canvas) { mMap = 0; mCanvas = canvas; }

	void setMap(BosonMap* map) { mMap = map; }
	void quitGame();

	void unitMoved(Unit* u, bofixed oldX, bofixed oldY);
	void updateSights();
	void updateRadars();

	void addSight(Unit* unit);
	void removeSight(Unit* unit);
	void updateSight(Unit* unit, bofixed oldX, bofixed oldY);

	void updateVisibleStatus(Unit* unit);

	void addRadar(Unit* unit);
	void removeRadar(Unit* unit);
	enum RadarChangeType { Move = 0, Add, Remove };
	void updateChangedRadar(Unit* unit, bofixed oldX, bofixed oldY, RadarChangeType type = Move);
	void updateRadarSignal(Unit* unit, bofixed oldX, bofixed oldY);
	bofixed radarSignalStrength(const RadarPlugin* radar, bofixed x, bofixed y, Unit* u);
	/**
	 * Recalculates signal strength of radar and radarjammer units
	 **/
	void recalculateSpecialSignalStrengths();
	const QValueList<const Unit*>* radarUnits() const { return &mRadarUnits; }

	void addRadarJammer(Unit* unit);
	void removeRadarJammer(Unit* unit);
	const QValueList<const Unit*>* radarJammerUnits() const { return &mRadarJammers; }
	void updateChangedJammer(Unit* unit, bofixed oldX, bofixed oldY, RadarChangeType type = Move);
	bofixed jammerSignalStrength(const RadarJammerPlugin* radar, bofixed x, bofixed y, Unit* u);

private:
	class ScheduledUnit
	{
	public:
		ScheduledUnit(Unit* u, bofixed x, bofixed y) { unit = u; lastX = x; lastY = y; }
		ScheduledUnit() {}

		Unit* unit;
		bofixed lastX;
		bofixed lastY;
	};

	// List of units that have moved, their sights need to be updated
	QValueList<ScheduledUnit> mScheduledSightUpdates;
	// List of all units that have moved, their radar signal strengths have
	//  to be updated
	QValueList<ScheduledUnit> mScheduledRadarUpdates;
	// List of _radar_ units that have moved, everything in their _range_ need to
	//  be updated
	QValueList<ScheduledUnit> mChangedRadars;
	// List of _radar jammer_ units that have moved, everything in their _range_
	//  need to be updated
	QValueList<ScheduledUnit> mChangedJammers;

	QValueList<const Unit*> mRadarUnits;
	QValueList<const Unit*> mRadarJammers;

	BosonMap* mMap;
	BosonCanvas* mCanvas;
};

void BoCanvasSightManager::quitGame()
{
 mScheduledSightUpdates.clear();
 mScheduledRadarUpdates.clear();
 mChangedRadars.clear();
 mChangedJammers.clear();
 mRadarUnits.clear();
 mRadarJammers.clear();
}

void BoCanvasSightManager::unitMoved(Unit* u, bofixed oldX, bofixed oldY)
{
 if (!u->isScheduledForSightUpdate()) {
	mScheduledSightUpdates.append(ScheduledUnit(u, oldX, oldY));
	u->setScheduledForSightUpdate(true);
 }

 if (!u->isScheduledForRadarUpdate()) {
	if (u->plugin(UnitPlugin::Radar)) {
		mChangedRadars.append(ScheduledUnit(u, oldX, oldY));
	}
	if (u->plugin(UnitPlugin::RadarJammer)) {
		mChangedJammers.append(ScheduledUnit(u, oldX, oldY));
	}
	mScheduledRadarUpdates.append(ScheduledUnit(u, oldX, oldY));
	u->setScheduledForRadarUpdate(true);
 }
}

void BoCanvasSightManager::updateSights()
{
 while (!mScheduledSightUpdates.isEmpty()) {
	const ScheduledUnit& s = mScheduledSightUpdates.first();
	if (!s.unit->isScheduledForSightUpdate()) {
		// Unit's sight has already been updated (e.g. when sight range increases)
		mScheduledSightUpdates.pop_front();
		continue;
	}
	updateSight(s.unit, s.lastX, s.lastY);

	mScheduledSightUpdates.pop_front();
 }
}

void BoCanvasSightManager::updateSight(Unit* unit, bofixed oldX, bofixed oldY)
{
 PROFILE_METHOD;
 unsigned int sight = (int)unit->sightRange();
 int sight2 = sight * sight;
 unsigned int x = (unsigned int)unit->centerX();
 unsigned int y = (unsigned int)unit->centerY();

 int left = ((x > sight) ? (x - sight) : 0) - x;
 int top = ((y > sight) ? (y - sight) : 0) - y;
 int right = ((x + sight > mMap->width()) ?  mMap->width() :
		x + sight) - x;
 int bottom = ((y + sight > mMap->height()) ?  mMap->height() :
		y + sight) - y;


 unsigned int oldCenterX = (unsigned int)(unit->centerX() + (oldX - unit->x()));
 unsigned int oldCenterY = (unsigned int)(unit->centerY() + (oldY - unit->y()));
 int deltaX = x - oldCenterX;
 int deltaY = y - oldCenterY;
 if (!deltaX && !deltaY) {
	unit->setScheduledForSightUpdate(false);
	return;
 }

 int oldleft = ((oldCenterX < sight) ? 0 : (oldCenterX - sight)) - x;
 int oldtop = ((oldCenterY < sight) ? 0 : (oldCenterY - sight)) - y;
 int oldright = ((oldCenterX + sight > mMap->width()) ? mMap->width() : (oldCenterX + sight)) - x;
 int oldbottom = ((oldCenterY + sight > mMap->height()) ? mMap->height() : (oldCenterY + sight)) - y;

 if (QMAX(deltaX, deltaY) <= 5) {
	// Speed up the common case by looking through every cell just once
	left = QMIN(left, oldleft);
	top = QMIN(top, oldtop);
	right = QMAX(right, oldright);
	bottom = QMAX(bottom, oldbottom);
	for (int i = left; i < right; i++) {
		for (int j = top; j < bottom; j++) {
			int olddist = (i+deltaX)*(i+deltaX) + (j+deltaY)*(j+deltaY);
			int newdist = i*i + j*j;
			if ((newdist >= sight2) && (olddist < sight2)) {
				// This cell is now out of sight but was previously in sight
				unit->owner()->removeFogRef(x + i, y + j);
			} else if ((newdist < sight2) && (olddist >= sight2)) {
				unit->owner()->addFogRef(x + i, y + j);
			}
		}
	}
 } else {
	// First remove cells which were visible but aren't anymore...
	for (int i = oldleft; i < oldright; i++) {
		for (int j = oldtop; j < oldbottom; j++) {
			if ((i*i + j*j >= sight2) && ((i+deltaX)*(i+deltaX) + (j+deltaY)*(j+deltaY) < sight2)) {
				// This cell is now out of sight but was previously in sight
				unit->owner()->removeFogRef(x + i, y + j);
			}
		}
	}

	// ... then add those which just became visible
	for (int i = left; i < right; i++) {
		for (int j = top; j < bottom; j++) {
			if ((i*i + j*j < sight2) && ((i+deltaX)*(i+deltaX) + (j+deltaY)*(j+deltaY) >= sight2)) {
				unit->owner()->addFogRef(x + i, y + j);
			}
		}
	}
 }

 // Update visible status of the unit for all players
 updateVisibleStatus(unit);

 unit->setScheduledForSightUpdate(false);
}

void BoCanvasSightManager::addSight(Unit* unit)
{
 unsigned int sight = (int)unit->sightRange();
 int sight2 = sight * sight;
 unsigned int x = (unsigned int)unit->centerX();
 unsigned int y = (unsigned int)unit->centerY();

 int left = ((x > sight) ? (x - sight) : 0) - x;
 int top = ((y > sight) ? (y - sight) : 0) - y;
 int right = ((x + sight > mMap->width()) ?  mMap->width() :
		x + sight) - x;
 int bottom = ((y + sight > mMap->height()) ?  mMap->height() :
		y + sight) - y;
 for (int i = left; i < right; i++) {
	for (int j = top; j < bottom; j++) {
		if (i*i + j*j < sight2) {
			unit->owner()->addFogRef(x + i, y + j);
		}
	}
 }
 unit->setScheduledForSightUpdate(false);
}

void BoCanvasSightManager::removeSight(Unit* unit)
{
 unsigned int sight = (int)unit->sightRange();
 int sight2 = sight * sight;
 unsigned int x = (unsigned int)unit->centerX();
 unsigned int y = (unsigned int)unit->centerY();

 if (unit->isScheduledForSightUpdate()) {
	// If unit has already moved since the last sight update, we need to use the
	//  old coordinate
	QValueList<ScheduledUnit>::iterator it;
	for (it = mScheduledSightUpdates.begin(); it != mScheduledSightUpdates.end(); ++it) {
		const ScheduledUnit& s = *it;
		if (s.unit == unit) {
			x = (unsigned int)(unit->centerX() + (s.lastX - unit->x()));
			y = (unsigned int)(unit->centerY() + (s.lastY - unit->y()));
			mScheduledSightUpdates.remove(it);
			break;
		}
	}
 }

 int left = ((x > sight) ? (x - sight) : 0) - x;
 int top = ((y > sight) ? (y - sight) : 0) - y;
 int right = ((x + sight > mMap->width()) ?  mMap->width() :
		x + sight) - x;
 int bottom = ((y + sight > mMap->height()) ?  mMap->height() :
		y + sight) - y;
 for (int i = left; i < right; i++) {
	for (int j = top; j < bottom; j++) {
		if (i*i + j*j < sight2) {
			unit->owner()->removeFogRef(x + i, y + j);
		}
	}
 }
 unit->setScheduledForSightUpdate(false);
}

void BoCanvasSightManager::updateVisibleStatus(Unit* unit)
{
 QPtrList<Player>* players = boGame->activeGamePlayerList();
 for (QPtrListIterator<Player> pit(*players); pit.current(); ++pit) {
	Player* player = pit.current();
	if (unit->owner() == player) {
		continue;
	}
	bool visible = player->playerIO()->canSee(unit);
	UnitBase::VisibleStatus status = unit->visibleStatus(player->bosonId());
	if ((status & UnitBase::VS_Visible) == visible) {
		continue;
	}
	if (visible) {
		if (unit->isFacility()) {
			unit->setVisibleStatus(player->bosonId(), (UnitBase::VisibleStatus)(status | UnitBase::VS_Visible | UnitBase::VS_Earlier));
		} else {
			unit->setVisibleStatus(player->bosonId(), (UnitBase::VisibleStatus)(status | UnitBase::VS_Visible));
		}
	} else {
		unit->setVisibleStatus(player->bosonId(), (UnitBase::VisibleStatus)(status & ~UnitBase::VS_Visible));
	}
 }
}

void BoCanvasSightManager::updateRadars()
{
 bool recalculateSpecialSignalStrengthsRequired = false;
 // First update changed radars
 while (!mChangedRadars.isEmpty()) {
	const ScheduledUnit& s = mChangedRadars.first();
	updateChangedRadar(s.unit, s.lastX, s.lastY);

	mChangedRadars.pop_front();
	recalculateSpecialSignalStrengthsRequired = true;
 }

 // Then changed jammers
 while (!mChangedJammers.isEmpty()) {
	const ScheduledUnit& s = mChangedJammers.first();
	updateChangedJammer(s.unit, s.lastX, s.lastY);

	mChangedJammers.pop_front();
	recalculateSpecialSignalStrengthsRequired = true;
 }

 // And finally all moved non-radar units
 while (!mScheduledRadarUpdates.isEmpty()) {
	const ScheduledUnit& s = mScheduledRadarUpdates.first();
	if (!s.unit->isScheduledForRadarUpdate()) {
		mScheduledRadarUpdates.pop_front();
		continue;
	}
	updateRadarSignal(s.unit, s.lastX, s.lastY);

	mScheduledRadarUpdates.pop_front();
 }

 if (recalculateSpecialSignalStrengthsRequired) {
	recalculateSpecialSignalStrengths();
 }
}

void BoCanvasSightManager::updateChangedRadar(Unit* unit, bofixed oldX, bofixed oldY, RadarChangeType type)
{
 PROFILE_METHOD;
 const RadarPlugin* prop = (const RadarPlugin*)unit->plugin(UnitPlugin::Radar);
 if (!prop) {
	boError() << k_funcinfo << "unit " << unit->id() << " is not radar!" << endl;
	return;
 }

 if (type != Move) {
	oldX = unit->x();
	oldY = unit->y();
 }

 // Maximum range of the radar
 // See below for the radar equation, here we calculate maximum distance of an
 //  object with size = 4.0 so that it's still detected by the radar
 bofixed maxrange = prop->range();

 // Calculate bbox of the radar-affected area
 bofixed minx = QMAX(QMIN(unit->x(), oldX) - maxrange, bofixed(0));
 bofixed maxx = QMIN(QMAX(unit->x(), oldX) + maxrange, bofixed(mMap->width()));
 bofixed miny = QMAX(QMIN(unit->y(), oldY) - maxrange, bofixed(0));
 bofixed maxy = QMIN(QMAX(unit->y(), oldY) + maxrange, bofixed(mMap->height()));

 BoRect2Fixed area(minx,  miny, maxx, maxy);
 BoItemList* items = mCanvas->collisions()->collisionsAtCells(area, 0, false);

 BoItemList::ConstIterator it;
 for (it = items->begin(); it != items->end(); ++it) {
	if (!RTTI::isUnit((*it)->rtti())) {
		continue;
	}
	Unit* u = (Unit*)*it;
	if (u->isDestroyed()) {
		continue;
	} else if (u->isScheduledForRadarUpdate()) {
		// This unit's signalstrengths will be recalculated for all players anyway.
		//  Just skip it for now
		continue;
	}
	// Make sure the radar can detect the unit
	if (u->isFlying() && !prop->detectsAirUnits()) {
		continue;
	} else if (!u->isFlying() && !prop->detectsLandUnits()) {
		continue;
	}

	updateRadarSignal(u, -1, -1);
 }
}

void BoCanvasSightManager::updateRadarSignal(Unit* unit, bofixed, bofixed)
{
 PROFILE_METHOD;
 // First calculate jammer signal strength
 bofixed jammersignalstrength = 0;
 for (QValueList<const Unit*>::iterator it = mRadarJammers.begin(); it != mRadarJammers.end(); ++it) {
	const Unit* u = *it;
	const RadarJammerPlugin* jammer = (const RadarJammerPlugin*)u->plugin(UnitPlugin::RadarJammer);
	jammersignalstrength += jammerSignalStrength(jammer, u->x(), u->y(), unit);
 }

 // Then radar signal strength for all players
 QPtrList<Player>* players = boGame->activeGamePlayerList();
 for (QPtrListIterator<Player> it(*players); it.current(); ++it) {
	Player* player = it.current();
	bofixed signalstrength = 0;

	const QValueList<const Unit*>* radars = player->radarUnits();
	for (QValueList<const Unit*>::const_iterator rit = radars->begin(); rit != radars->end(); ++rit) {
		const Unit* u = *rit;
		const RadarPlugin* radar = (const RadarPlugin*)u->plugin(UnitPlugin::Radar);
		if (unit->isFlying() && radar->detectsAirUnits()) {
			signalstrength += radarSignalStrength(radar, u->x(), u->y(), unit);
		} else if (!unit->isFlying() && radar->detectsLandUnits()) {
			signalstrength += radarSignalStrength(radar, u->x(), u->y(), unit);
		}
	}

	// Substract jammer's signal
	signalstrength = QMAX(bofixed(0), signalstrength - jammersignalstrength);
	unit->setRadarSignalStrength(player->bosonId(), signalstrength);
 }

 unit->setScheduledForRadarUpdate(false);
}

void BoCanvasSightManager::addRadar(Unit* unit)
{
 // Update radar's range and transmitted power
 ((RadarPlugin*)unit->plugin(UnitPlugin::Radar))->unitHealthChanged();
 mRadarUnits.append(unit);
 updateChangedRadar(unit, -1, -1, Add);
 recalculateSpecialSignalStrengths();
}

void BoCanvasSightManager::removeRadar(Unit* unit)
{
 // Note: do NOT call RadarPlugin::unitHealthChanged() here! Radar must be
 //  removed using _old_ health/range
 mRadarUnits.remove(unit);
 updateChangedRadar(unit, -1, -1, Remove);
}

bofixed BoCanvasSightManager::radarSignalStrength(const RadarPlugin* radar, bofixed x, bofixed y, Unit* u)
{
 float dx = x - u->centerX();
 float dy = y - u->centerY();
 float dist = sqrtf(dx*dx + dy*dy);
 if (dist < 1) {
	dist = 1;
 }
 if (dist*dist > radar->range()*radar->range()) {
	return 0;
 }

 // To calculate strength of radar signal, we use simplified radar equation:
 //      R = (T * S) / (D^3)
 //  where R - received power
 //        T - transmitter power
 //        S - size of the object (how well it reflects the signal)
 //        D - distance between radar and the object
 // See http://en.wikipedia.org/wiki/Radar for more info about radars and
 //  radar equation.
 float receivedpower = (radar->transmittedPower() * (float)u->width()) / (dist*dist*dist);
 float signalstrength = receivedpower / radar->minReceivedPower();
 //boDebug() << k_funcinfo << radar->unit()->id() << " -> " << u->id() << ": " << receivedpower << " / " << signalstrength << endl;
 if (signalstrength >= 1) {
	// Clamp to 200 to avoid overflows
	if (signalstrength > 200.0f) {
		return bofixed(200);
	} else {
		return bofixed(signalstrength);
	}
 } else {
	return 0;
 }
}

void BoCanvasSightManager::recalculateSpecialSignalStrengths()
{
 PROFILE_METHOD;
 QPtrList<Player>* players = boGame->activeGamePlayerList();

 // Calculate signal strengths for radars
 // Go through all radar units
 for (QValueList<const Unit*>::Iterator it = mRadarUnits.begin(); it != mRadarUnits.end(); ++it) {
	// This is the radar unit that we're trying to detect
	Unit* radarUnit = (Unit*)*it;
	float radarUnitTransmittedPower = ((const RadarPlugin*)radarUnit->plugin(UnitPlugin::Radar))->transmittedPower();
	// Go through all players
	for (QPtrListIterator<Player> pit(*players); pit.current(); ++pit) {
		Player* player = pit.current();
		if (radarUnit->owner() == player) {
			continue;
		}
		// This is strength of signal coming from radarUnit and detected by player
		bofixed signalStrength = 0;

		// Go through all player's radars and calculate signal strengths
		const QValueList<const Unit*>* playerRadars = player->radarUnits();
		for (QValueList<const Unit*>::const_iterator rit = playerRadars->begin(); rit != playerRadars->end(); ++rit) {
			const Unit* u = *rit;
			const RadarPlugin* radar = (const RadarPlugin*)u->plugin(UnitPlugin::Radar);
			// Make sure player's radar can detect radarUnit
			// TODO: is this a good idea? Maybe radars should always be able to detect each other?
			if (radarUnit->isFlying() && !radar->detectsAirUnits()) {
				continue;
			} else if (!radarUnit->isFlying() && !radar->detectsLandUnits()) {
				continue;
			}

			// Calculate how well u can detect radarUnit
			float dx = radarUnit->centerX() - u->centerX();
			float dy = radarUnit->centerY() - u->centerY();
			float distSqr = dx*dx + dy*dy;
			// Note that we use  4*distSqr  instead of just  distSqr  to make the
			//  detection range a bit smaller (2 times smaller in fact)
			float receivedpower = radarUnitTransmittedPower / (4 * distSqr);
			float currentSignalStrength = receivedpower / radar->minReceivedPower();
			if (currentSignalStrength > 100.0f) {
				signalStrength += bofixed(100);
			} else if (currentSignalStrength >= 1.0f) {
				signalStrength += currentSignalStrength;
			}
		}
		radarUnit->setRadarSignalStrength(player->bosonId(), signalStrength);
	}
 }

 // Calculate signal strengths for jammers in a similar fashion
 // Go through all jammer units
 for (QValueList<const Unit*>::Iterator it = mRadarJammers.begin(); it != mRadarJammers.end(); ++it) {
	// This is the jammer unit that we're trying to detect
	Unit* jammerUnit = (Unit*)*it;
	float jammerUnitTransmittedPower = ((const RadarJammerPlugin*)jammerUnit->plugin(UnitPlugin::RadarJammer))->transmittedPower();
	// Go through all players
	for (QPtrListIterator<Player> pit(*players); pit.current(); ++pit) {
		Player* player = pit.current();
		if (jammerUnit->owner() == player) {
			continue;
		}
		// This is strength of signal coming from jammerUnit and detected by player
		bofixed signalStrength = 0;

		// Go through all player's radars and calculate signal strengths
		const QValueList<const Unit*>* playerRadars = player->radarUnits();
		for (QValueList<const Unit*>::const_iterator rit = playerRadars->begin(); rit != playerRadars->end(); ++rit) {
			const Unit* u = *rit;
			const RadarPlugin* radar = (const RadarPlugin*)u->plugin(UnitPlugin::Radar);
			// Make sure player's radar can detect jammerUnit
			// TODO: is this a good idea? Maybe radars should always be able to detect each other?
			if (jammerUnit->isFlying() && !radar->detectsAirUnits()) {
				continue;
			} else if (!jammerUnit->isFlying() && !radar->detectsLandUnits()) {
				continue;
			}

			// Calculate how well u can detect jammerUnit
			float dx = jammerUnit->centerX() - u->centerX();
			float dy = jammerUnit->centerY() - u->centerY();
			float distSqr = dx*dx + dy*dy;
			// Note that we use  2*distSqr  instead of just  distSqr  to make the
			//  detection range a bit smaller (sqrt(2) times smaller in fact)
			float receivedpower = jammerUnitTransmittedPower / (2 * distSqr);
			float currentSignalStrength = receivedpower / radar->minReceivedPower();
			if (currentSignalStrength > 100.0f) {
				signalStrength += bofixed(100);
			} else if (currentSignalStrength >= 1.0f) {
				signalStrength += currentSignalStrength;
			}
		}
		jammerUnit->setRadarSignalStrength(player->bosonId(), signalStrength);
	}
 }
}

void BoCanvasSightManager::addRadarJammer(Unit* unit)
{
 // Update radar's range and transmitted power
 ((RadarJammerPlugin*)unit->plugin(UnitPlugin::RadarJammer))->unitHealthChanged();
 mRadarJammers.append(unit);
 updateChangedJammer(unit, -1, -1, Add);
 recalculateSpecialSignalStrengths();
}

void BoCanvasSightManager::removeRadarJammer(Unit* unit)
{
 // Note: do NOT call RadarJammerPlugin::unitHealthChanged() here! Jammer must
 //  be removed using _old_ health/range
 mRadarJammers.remove(unit);
 updateChangedJammer(unit, -1, -1, Remove);
}

void BoCanvasSightManager::updateChangedJammer(Unit* unit, bofixed oldX, bofixed oldY, RadarChangeType type)
{
 PROFILE_METHOD;
 const RadarJammerPlugin* prop = (const RadarJammerPlugin*)unit->plugin(UnitPlugin::RadarJammer);
 if (!prop) {
	boError() << k_funcinfo << "unit " << unit->id() << " is not radar jammer!" << endl;
	return;
 }

 if (type != Move) {
	oldX = unit->x();
	oldY = unit->y();
 }

 // Maximum range of the jammer
 bofixed maxrange = prop->range();

 // Calculate bbox of the radar-affected area
 bofixed minx = QMAX(QMIN(unit->x(), oldX) - maxrange, bofixed(0));
 bofixed maxx = QMIN(QMAX(unit->x(), oldX) + maxrange, bofixed(mMap->width()));
 bofixed miny = QMAX(QMIN(unit->y(), oldY) - maxrange, bofixed(0));
 bofixed maxy = QMIN(QMAX(unit->y(), oldY) + maxrange, bofixed(mMap->height()));

 BoRect2Fixed area(minx,  miny, maxx, maxy);
 BoItemList* items = mCanvas->collisions()->collisionsAtCells(area, 0, false);

 BoItemList::ConstIterator it;
 for (it = items->begin(); it != items->end(); ++it) {
	if (!RTTI::isUnit((*it)->rtti())) {
		continue;
	}
	Unit* u = (Unit*)*it;
	if (u->isDestroyed()) {
		continue;
	} else if (u->isScheduledForRadarUpdate()) {
		// This unit's signal strength will be recalculated anyway. Just skip it for now
		continue;
	}
	// Recalculate signal strength (for all players)
	updateRadarSignal(u, u->x(), u->y());
 }
}

bofixed BoCanvasSightManager::jammerSignalStrength(const RadarJammerPlugin* jammer, bofixed x, bofixed y, Unit* u)
{
 float dx = x - u->centerX();
 float dy = y - u->centerY();
 float distsqr = dx*dx + dy*dy;
 if (distsqr < 1) {
	distsqr = 1;
 }
 if (distsqr > jammer->range() * jammer->range()) {
	return 0;
 }

 float signalstrength = jammer->transmittedPower() / distsqr;
 //boDebug() << k_funcinfo << radar->unit()->id() << " -> " << u->id() << ": " << receivedpower << " / " << signalstrength << endl;
 if (signalstrength >= 0.5f) {
	// Clamp to 500 to avoid overflows
	if (signalstrength > 500.0f) {
		return bofixed(500);
	} else {
		return bofixed(signalstrength);
	}
 } else {
	return 0;
 }
}




/**
 * Actual implementation of BosonCanvas::slotAdvance(), i.e. the items are
 * advanced here.
 *
 * This deserves a dedicated class, because it is a very (the most?) central
 * aspect of boson. Profiling of what is done here is very important, which is
 * easier with a dedicated class.
 * BoCanvasAdvance is a friend of @ref BosonCanvas, so this is not a limitation.
 **/
class BoCanvasAdvance
{
public:
	BoCanvasAdvance(BosonCanvas* c)
	{
		mCanvas = c;
	}
	void advance(const BoItemList& allItems, unsigned int advanceCallsCount, bool advanceFlag);

protected:
	void chargeUnits(unsigned int advanceCallsCount, bool advanceFlag);
	void unchargeUnits(unsigned int advanceCallsCount, bool advanceFlag);
	void advanceItems(const BoItemList& allItems, unsigned int advanceCallsCount, bool advanceFlag);
	void itemReload(const BoItemList& allItems, unsigned int advanceCallsCount); // calls BosonItem::advance()

	// AB: note that allItems is not used here currently. we use
	// mCanvas->d->mWork2AdvanceList.
	void advanceFunctionAndMove(unsigned int advanceCallsCount, bool advanceFlag);
	void syncAdvanceFunctions(const BoItemList& allItems, bool advanceFlag); // MUST be called after advanceFunction() stuff
	void updateWork2AdvanceList();
	void maximalAdvanceCountTasks(unsigned int advanceCallsCount); // "maximalAdvanceCount" is nonsense here, but the name has historic reasons
	void notifyAboutDestroyedUnits(const QPtrList<Unit>& destroyedUnits, unsigned int first, const BoItemList& allItems);

private:
	BosonCanvas* mCanvas;
};

void BoCanvasAdvance::advance(const BoItemList& allItems, unsigned int advanceCallsCount, bool advanceFlag)
{
 boProfiling->push(prof_funcinfo + " - Whole method");

 QMap<Player*, bool> player2HasMiniMap;
 for (QPtrListIterator<Player> it(*boGame->gamePlayerList()); it.current(); ++it) {
	Player* p = it.current();
	player2HasMiniMap.insert(p, p->hasMiniMap());
 }

 boProfiling->push("Charge units");
 chargeUnits(advanceCallsCount, advanceFlag);
 boProfiling->pop(); // Advance Items

 unsigned int initialDestroyedUnitsCount = mCanvas->d->mDestroyedUnits.count();

 /*
  * Main part of this method. This is the most important, but unfortunately also
  * the most time consuming part.
  */
 boProfiling->push("Advance Items");
 advanceItems(allItems, advanceCallsCount, advanceFlag);
 boProfiling->pop(); // Advance Items

 // TODO: check when is it best to do this
 boProfiling->push("Advance Pathfinder");
 mCanvas->pathFinder()->advance();
 boProfiling->pop(); // Advance Pathfinder

 boProfiling->push("Notify About Destroyed Units");
 notifyAboutDestroyedUnits(mCanvas->d->mDestroyedUnits, initialDestroyedUnitsCount, allItems);
 boProfiling->pop(); // Advance Maximal Advance Count

 boProfiling->push("Unharge units");
 unchargeUnits(advanceCallsCount, advanceFlag);
 boProfiling->pop(); // Advance Items

 if (advanceCallsCount % 20 == 7) {
	boProfiling->push("SightManager::updateSights()");
	mCanvas->d->mSightManager->updateSights();
	boProfiling->pop();
 }
 if (advanceCallsCount % 40 == 18) {
	boProfiling->push("SightManager::updateRadars()");
	mCanvas->d->mSightManager->updateRadars();
	boProfiling->pop();
 }


 // TODO: use a condition for this code: every n advance call the condition
 // should send an event "GainNewAmmo" for the players
#if 1
 // refill "Generic" ammo of all players by a small amount, up to a certain value.
 // TODO: use the actual _small_ amount once a "ammunition center" exists, where
 // the player can build new ammo (to increase the refill rate)
#if 0
 const unsigned long int amount = 5;
#else
 const unsigned long int amount = 100;
#endif
 // if this value is reached, "free" refill stops. only using ammunition center
 // (i.e. by producing new ammo), new ammo can be gained.
 const unsigned long int maxAmmo = 1000;
 for (QPtrListIterator<Player> it(*boGame->activeGamePlayerList()); it.current(); ++it) {
	Player* p = it.current();
	QString type = "Generic";
	if (p->ammunition(type) < maxAmmo) {
		p->setAmmunition(type, p->ammunition(type) + amount);
	}
 }
#endif

 /*
  * This contains some things that need to be done "sometimes" only - currently
  * that is deletion of destroyed units and unused shots.
  *
  * These things are often very time consuming, but that hardly matters since it
  * is rarely done.
  */
 boProfiling->push("Advance Maximal Advance Count");
 maximalAdvanceCountTasks(advanceCallsCount);
 boProfiling->pop(); // Advance Maximal Advance Count

 for (QMap<Player*, bool>::const_iterator it = player2HasMiniMap.begin(); it != player2HasMiniMap.end(); ++it) {
	Player* p = it.key();
	bool had = it.data();
	bool has = p->hasMiniMap();
	if (has == had) {
		continue;
	}
	if (has) {
		BoEvent* miniMapEvent = new BoEvent("GainedMinimap");
		miniMapEvent->setPlayerId(p->bosonId());
		boGame->queueEvent(miniMapEvent);
	} else {
		BoEvent* event = new BoEvent("LostMinimap");
		event->setPlayerId(p->bosonId());
		boGame->queueEvent(event);
	}
 }

 boProfiling->pop(); // Whole method
}

void BoCanvasAdvance::chargeUnits(unsigned int advanceCallsCount, bool advanceFlag)
{
 Q_UNUSED(advanceCallsCount);
 Q_UNUSED(advanceFlag);
 for (QPtrListIterator<Player> it(*boGame->gamePlayerList()); it.current(); ++it) {
	Player* p = it.current();
	p->updatePowerChargeForCurrentAdvanceCall();
 }
}

void BoCanvasAdvance::unchargeUnits(unsigned int advanceCallsCount, bool advanceFlag)
{
 Q_UNUSED(advanceCallsCount);
 Q_UNUSED(advanceFlag);
 for (QPtrListIterator<Player> it(*boGame->gamePlayerList()); it.current(); ++it) {
	Player* p = it.current();
	p->unchargeUnitsForAdvance();
 }
}

void BoCanvasAdvance::advanceItems(const BoItemList& allItems, unsigned int advanceCallsCount, bool advanceFlag)
{
 mCanvas->lockAdvanceFunction();
 mCanvas->d->mStatistics->resetWorkCounts();

 // first we need to call *all* BosonItem::advance() functions.
 // AB: profiling information will be inaccurate because of this... we are
 // collecting for every item advance() here, and below for some items
 // advanceFunction(). those will be listed as *different* items...
 boProfiling->push("Advance: BosonItem::reload()");
 itemReload(allItems, advanceCallsCount);
 boProfiling->pop();

 // now the rest - mainly call BosonItem::advanceFunction().
 // this depends on in which list an item resides (changed when Unit::work()
 // changes). normal items are usually in -1.
 boProfiling->push("Advance: BosonItem::advanceFunction() and move()");
 advanceFunctionAndMove(advanceCallsCount, advanceFlag);
 boProfiling->pop();

 // now we need to make sure that the correct advance function will be called in
 // the next advance call.
 syncAdvanceFunctions(allItems, advanceFlag);

 // we completed iterating through the advance lists. now we might have to
 // change the list for some items.
 updateWork2AdvanceList();

 mCanvas->unlockAdvanceFunction();
}

void BoCanvasAdvance::itemReload(const BoItemList& allItems, unsigned int advanceCallsCount)
{
 const unsigned int interval = 5;
 if (advanceCallsCount % interval == 0) {
	BoItemList::ConstIterator allIt;
	BoItemList::ConstIterator allItemsEnd = allItems.end();
	for (allIt = allItems.begin(); allIt != allItemsEnd; ++allIt) {
		(*allIt)->reload(interval);
	}
 }
}

void BoCanvasAdvance::advanceFunctionAndMove(unsigned int advanceCallsCount, bool advanceFlag)
{
 PROFILE_METHOD;
 // FIXME: the mWork2AdvanceList map should be a parameter
 QMap<int, QPtrList<BosonItem> >::Iterator it;
 for (it = mCanvas->d->mWork2AdvanceList.begin(); it != mCanvas->d->mWork2AdvanceList.end(); ++it) {
	int work = it.key();
	bool skip = false;
	switch (work) {
		// TODO: instead of a big switch we should maintain a
		// d->mWork2AdvancePeriod map
		case -1:
			// *always* execute this!
			skip = false;
			break;
		case (int)UnitBase::WorkIdle:
			// TODO: maybe use some other work for circling aircrafts?
			skip = false;
			break;
		case (int)UnitBase::WorkNone:
			skip = true;
			break;
		case (int)UnitBase::WorkMove:
			skip = false;
			break;
		case (int)UnitBase::WorkAttack:
			/*if (advanceCallsCount % 5 != 0) {
				skip = true;
			}*/
			break;
		case (int)UnitBase::WorkConstructed:
			if (advanceCallsCount % 20 != 0) {
				skip = true;
			}
			break;
		case (int)UnitBase::WorkDestroyed:
			skip = false;
			break;
		case (int)UnitBase::WorkFollow:
			if (advanceCallsCount % 5 != 0) {
				skip = true;
			}
			break;
		case (int)UnitBase::WorkPlugin:
			skip = false;
			break;
		case (int)UnitBase::WorkTurn:
			skip = false;
			break;
		default:
			// shouldn't happen! (TODO: add a warning! havent done
			// so in favor of testing)
			skip = false;
			break;
	}

	mCanvas->d->mStatistics->increaseWorkCountBy(work, (*it).count());

	if (skip) {
		continue;
	}
	BosonProfiler profiler(QString("advanceFunction() and moveBy() for work==%1").arg(work));
//	boDebug() << "advancing " << (*it).count() << " items with advanceWork=" << work << endl;
	QPtrListIterator<BosonItem> itemIt(*it);
	for (; itemIt.current(); ++itemIt) {
		BosonItem* s = itemIt.current();
		if (work != UnitBase::WorkDestroyed && RTTI::isUnit(s->rtti())) {
			Unit* unit = (Unit*)s;
			if (unit->isDestroyed()) {
				// Don't call advanceFunction for destroyed unit, except for advanceDestroyed
				continue;
			}
		}
		if (advanceFlag) { // bah - inside the loop..
			s->advanceFunction(advanceCallsCount); // once this was called this object is allowed to change its advanceFunction()
		} else {
			s->advanceFunction2(advanceCallsCount); // once this was called this object is allowed to change its advanceFunction()
		}

		if (s->xVelocity() || s->yVelocity() || s->zVelocity()) {
			s->moveBy(s->xVelocity(), s->yVelocity(), s->zVelocity());
		}
	}
 }
}

void BoCanvasAdvance::syncAdvanceFunctions(const BoItemList& allItems, bool advanceFlag)
{
 BoItemList::ConstIterator it;
 BoItemList::ConstIterator allItemsEnd = allItems.end();
 if (advanceFlag) {
	for (it = allItems.begin(); it != allItemsEnd; ++it) {
		BosonItem* i = *it;
		i->syncAdvanceFunction();
	}
 } else {
	for (it = allItems.begin(); it != allItemsEnd; ++it) {
		BosonItem* i = *it;
		i->syncAdvanceFunction2();
	}
 }
}

void BoCanvasAdvance::updateWork2AdvanceList()
{
 QPtrListIterator<BosonItem> changeIt(mCanvas->d->mChangeAdvanceList);
 for (; changeIt.current(); ++changeIt) {
	BosonItem* item = changeIt.current();
	mCanvas->removeFromAdvanceLists(item); // AB: this will probably take too much time :(
	if (!RTTI::isUnit(item->rtti())) {
		// oops - this should not (yet?) happen!
		// --> append to default list
		mCanvas->d->mWork2AdvanceList[-1].append(item);
		continue;
	}
	Unit* unit = (Unit*)item;
	mCanvas->d->mWork2AdvanceList[unit->advanceWork()].append(item);
 }
 mCanvas->d->mChangeAdvanceList.clear();
}

void BoCanvasAdvance::maximalAdvanceCountTasks(unsigned int advanceCallsCount)
{
 BosonProfiler profiler("Advance: special MAXIMAL_ADVANCE_COUNT tasks"); // measure _all_ advanceCallsCounts

 const unsigned int MAXIMAL_ADVANCE_COUNT = 39;
 if (advanceCallsCount % MAXIMAL_ADVANCE_COUNT != 0) {
	return;
 }
 BosonProfiler profiler2("Advance MAXIMAL_ADVANCE_COUNT: all tasks");
 boDebug(300) << "MAXIMAL_ADVANCE_COUNT" << endl;
 QPtrListIterator<Unit> deletionIt(mCanvas->d->mDestroyedUnits);
 QPtrList<BosonItem> deleteList;
 boProfiling->push("Advance MAXIMAL_ADVANCE_COUNT: construction of item deletion list");
 while (deletionIt.current()) {
	deletionIt.current()->increaseDeletionTimer();
	if (deletionIt.current()->deletionTimer() >= REMOVE_WRECKAGES_TIME) {
		deleteList.append(deletionIt.current());
	}
	++deletionIt;
 }
 boProfiling->pop();

 boProfiling->push("Advance MAXIMAL_ADVANCE_COUNT: update destroyed list");
 QPtrListIterator<BosonItem> destroyedIt(deleteList);
 while (destroyedIt.current()) {
	mCanvas->d->mDestroyedUnits.removeRef((Unit*)destroyedIt.current());
	++destroyedIt;
 }
 boProfiling->pop();


 boProfiling->push("Advance MAXIMAL_ADVANCE_COUNT: deleting items");
 mCanvas->deleteItems(deleteList);
 boProfiling->pop();

 boProfiling->push("Advance MAXIMAL_ADVANCE_COUNT: deleteUnusedShots()");
 mCanvas->deleteUnusedShots();
 boProfiling->pop();
}

void BoCanvasAdvance::notifyAboutDestroyedUnits(const QPtrList<Unit>& destroyedUnits, unsigned int first, const BoItemList& allItems)
{
 unsigned int i = 0;
 QPtrListIterator<Unit> destIt(destroyedUnits);
 while (destIt.current() && i < first) {
	++destIt;
 }

 // notify all units about the destroyed units in this advance call (i.e. from first
 // to end of list)
 while (destIt.current()) {
	Unit* destroyedUnit = destIt.current();
	for (BoItemList::const_iterator it = allItems.begin(); it != allItems.end(); ++it) {
		if (!RTTI::isUnit((*it)->rtti())) {
			continue;
		}
		Unit* u = (Unit*)*it;
		if (u == destroyedUnit) {
			continue;
		}
		u->unitDestroyed(destroyedUnit);
	}
	++destIt;
 }
}


BosonCanvas::BosonCanvas(QObject* parent)
		: QObject(parent, "BosonCanvas")
{
 init();
}

void BosonCanvas::init()
{
 d = new BosonCanvasPrivate;
 d->mDestroyedUnits.setAutoDelete(false);
 mAdvanceFunctionLocked = false;
 mCollisions = new BosonCollisions();
 d->mSightManager = new BoCanvasSightManager(this);
 d->mQuadTreeCollection = new BoCanvasQuadTreeCollection(this);
 d->mStatistics = new BosonCanvasStatistics(this);
 d->mProperties = new KGamePropertyHandler(this);
 d->mNextItemId.registerData(IdNextItemId, d->mProperties,
		KGamePropertyBase::PolicyLocal, "NextItemId");
 d->mNextItemId.setLocal(0);

 if (!boGame) {
	boError() << k_funcinfo << "NULL boGame object: cannot install event listener" << endl;
 } else {
	d->mEventListener = new BoCanvasEventListener(boGame->eventManager(), this);
	connect(d->mEventListener, SIGNAL(signalGameOver()),
			this, SIGNAL(signalGameOver()));
	if (!d->mEventListener->initScript()) {
		boError() << k_funcinfo << "cannot init eventlistener script" << endl;
		return; // TODO: return false
	}
 }
}

BosonCanvas::~BosonCanvas()
{
 boDebug()<< k_funcinfo << endl;
 quitGame();
 delete d->mStatistics;
 delete mCollisions;
 clearMoveDatas();
 delete d->mQuadTreeCollection;
 delete d->mSightManager;
 delete d;
 boDebug()<< k_funcinfo <<"done"<< endl;
}

BosonCanvasStatistics* BosonCanvas::canvasStatistics() const
{
 return d->mStatistics;
}

void BosonCanvas::quitGame()
{
 // Delete pathfinder first. Otherwise lot of time would be spent recalculating
 //  regions (when units are removed), which is totally unnecessary
 delete d->mPathFinder;
 d->mPathFinder = 0;
 deleteDestroyed();
 QMap<int, QPtrList<BosonItem> >::Iterator it;
 for (it = d->mWork2AdvanceList.begin(); it != d->mWork2AdvanceList.end(); ++it) {
	(*it).clear();
 }
 deleteItems(d->mAllItems);
 if (!d->mAllItems.isEmpty()) {
	boError() << k_funcinfo << "mAllItems is not empty!" << endl;
 }
 d->mChangeAdvanceList.clear();
 d->mNextItemId = 0;
 d->mSightManager->quitGame();

 BoItemListHandler::itemListHandler()->slotDeleteLists();
}

void BosonCanvas::deleteDestroyed()
{
 QPtrList<BosonItem> items;
 QPtrListIterator<Unit> it(d->mDestroyedUnits);
 while (it.current()) {
	items.append(it.current());
	++it;
 }
 deleteItems(items);
 d->mDestroyedUnits.clear();
}

Cell* BosonCanvas::cell(int x, int y) const
{
 if (!d->mMap) {
	boError() << k_funcinfo << "NULL map" << endl;
	return 0;
 }
 return d->mMap->cell(x, y);
}

Cell* BosonCanvas::cells() const
{
 BO_CHECK_NULL_RET0(d->mMap);
 return d->mMap->cells();
}

void BosonCanvas::slotAdvance(unsigned int advanceCallsCount, bool advanceFlag)
{
 boProfiling->pushStorage("Advance");
 boProfiling->push("slotAdvance()");

 BoCanvasAdvance a(this);
 a.advance(d->mAllItems, advanceCallsCount, advanceFlag);

 boProfiling->pop();
 boProfiling->popStorage();
}

bool BosonCanvas::canGo(const UnitProperties* prop, const BoRect2Fixed& rect, bool _default) const
{
// boDebug() << k_funcinfo << endl;
 if (rect.left() < 0 || rect.top() < 0 ||
		rect.right() > mapWidth() ||
		rect.bottom() > mapHeight()) {
	return false;
 }
 int right = (int)ceil(rect.right());
 int bottom = (int)ceil(rect.bottom());
 int y = (int)rect.y(); // what about modulu? do we care ?
 do {
	int x = (int)rect.x();
	do {
		if (!canGo(prop, x, y, _default)) {
			return false;
		}
		x++;
	} while (x < right);
	y++;
 } while (y < bottom);

 return true;
}

bool BosonCanvas::canGo(const UnitProperties* prop, int x, int y, bool _default) const
{
 // TODO: check for fog (we'll probably need Player/PlayerIO for this)
 // TODO: move this to UnitProperties. The reason it's here ATM is that we need
 //  map's width
 if (prop->isAircraft()) {
	return true;
 } else if(prop->isFacility()) {
	// TODO: add MaxSlope and WaterDepth properties to facilities and take those into account
	return !cell(x, y)->isWater();
 }
 BosonMoveData* md = moveData(prop);
 if (!md) {
	BO_NULL_ERROR(md);
	return false;
 }

 return md->cellPassable[y * mapWidth() + x];
}

void BosonCanvas::setMap(BosonMap* map)
{
 d->mMap = map;
 d->mSightManager->setMap(map);
 collisions()->setMap(map);
}

void BosonCanvas::unitMoved(Unit* unit, bofixed oldX, bofixed oldY)
{
 d->mSightManager->unitMoved(unit, oldX, oldY);

// test if any unit has this unit as target. If sou then adjust the destination.
//TODO

// used to adjust the mini map
 emit signalUnitMoved(unit, oldX, oldY);
}

void BosonCanvas::updateSight(Unit* unit, bofixed oldX, bofixed oldY)
{
 d->mSightManager->updateSight(unit, oldX, oldY);
}

void BosonCanvas::addSight(Unit* unit)
{
 d->mSightManager->addSight(unit);
}

void BosonCanvas::removeSight(Unit* unit)
{
 d->mSightManager->removeSight(unit);
}

void BosonCanvas::addRadar(Unit* unit)
{
 if (!unit->plugin(UnitPlugin::Radar)) {
	return;
 }

 d->mSightManager->addRadar(unit);
 unit->owner()->addRadar(unit);
}

void BosonCanvas::removeRadar(Unit* unit)
{
 if (!unit->plugin(UnitPlugin::Radar)) {
	return;
 }

 d->mSightManager->removeRadar(unit);
 unit->owner()->removeRadar(unit);
}

void BosonCanvas::addRadarJammer(Unit* unit)
{
 if (!unit->plugin(UnitPlugin::RadarJammer)) {
	return;
 }

 d->mSightManager->addRadarJammer(unit);
}

void BosonCanvas::removeRadarJammer(Unit* unit)
{
 if (!unit->plugin(UnitPlugin::RadarJammer)) {
	return;
 }

 d->mSightManager->removeRadarJammer(unit);
}

const QValueList<const Unit*>* BosonCanvas::radarJammerUnits() const
{
 return d->mSightManager->radarJammerUnits();
}

const QValueList<const Unit*>* BosonCanvas::radarUnits() const
{
 return d->mSightManager->radarUnits();
}

void BosonCanvas::newShot(BosonShot*)
{
 boDebug(350) << k_funcinfo << endl;
}

void BosonCanvas::shotHit(BosonShot* s)
{
 if (!s) {
	boError() << k_funcinfo << "NULL shot" << endl;
	return;
 }

 emit signalShotHit(s);

 explosion(BoVector3Fixed(s->x(), s->y(), s->z()), s->damage(), s->damageRange(),
		s->fullDamageRange(), s->owner());
}

void BosonCanvas::explosion(const BoVector3Fixed& pos, long int damage, bofixed range, bofixed fullrange, Player* owner)
{
 // Decrease health of all units within damaging range of explosion
 long int d;
 bofixed dist;
 QValueList<Unit*> l = collisions()->unitCollisionsInSphere(pos, range);
 for (unsigned int i = 0; i < l.count(); i++) {
	Unit* u = l[i];
	// Calculate actual distance of unit from explosion's center (this takes
	//  unit's size into account)
	dist = sqrt(u->distanceSquared(pos));
	if (dist <= fullrange || range == fullrange) {
		d = damage;
	} else {
		d = (long int)((1 - (dist - fullrange) / (range - fullrange)) * damage);
	}
	unitDamaged(u, d);
	if (u->isDestroyed() && owner) {
		if (u->isFacility()) {
			owner->statistics()->addDestroyedFacility(u, owner);
		} else {
			owner->statistics()->addDestroyedMobileUnit(u, owner);
		}
	}
 }
}

void BosonCanvas::unitDamaged(Unit* unit, long int damage)
{
 // Shield
 if (unit->shields() > 0) {
	if (unit->shields() >= (unsigned long int)damage) {
		// Unit will not be damaged (it has enough shields)
		unit->setShields(unit->shields() - damage);
		// TODO: show some shield animation
		return;
	} else {
		damage -= unit->shields();
		unit->setShields(0);
		// Also show shield animation?
	}
 }

 if (damage < 0) {
	unit->setHealth(unit->health() + ((unsigned long)-damage));
 } else {
	// Usually, unit's armor is substracted from attacker's weaponDamage, but
	//  if target has only little health left, then armor doesn't have full effect
	int health = (int)unit->health();
	if (health <= (int)(unit->maxHealth() / 10.0)) {
		// If unit has only 10% or less of it's hitpoint left, armor has no effect (it's probably destroyed)
	} else if (health <= (int)(unit->maxHealth() / 2.5)) {
		// Unit has 40% or less of hitpoints left. Only half of armor is "working"
		damage -= (int)(unit->armor() / 2.0);
	} else {
		damage -= unit->armor();
	}
	if (damage < 0) {
		damage = 0;
	}
	health -= damage;
	unit->setHealth((health >= 0) ? health : 0);
 }

 if (unit->isDestroyed()) {
	destroyUnit(unit); // display the explosion ; not the shoot
 } else {
/*	bofixed factor = 2.0 - unit->health() / (unit->unitProperties()->health() / 2.0);
//	if (unit->health() <= (unit->unitProperties()->health() / 2.0)) {
	if (factor >= 1.0) {
		// If unit has less than 50% hitpoints, it's smoking
		BoVector3Fixed pos((unit->x() + unit->width() / 2),
				-((unit->y() + unit->height() / 2)),
				unit->z());
		BosonParticleSystem* s;
		if (!unit->smokeParticleSystem()) {
			s = BosonParticleManager::newSmallSmoke(pos);
			unit->setSmokeParticleSystem(s);
			d->mParticles.append(s);
		}
		s = unit->smokeParticleSystem();
		// FIXME: maybe move this to BosonParticleManager?
		s->setCreateRate(factor * 25);
//		s->setVelocity(BoVector3Fixed(0, 0, factor * 0.5));  // This is only hint for BosonParticleManager
		bofixed c = 0.8 - factor * 0.4;
		s->setColor(BoVector4(c, c, c, 0.25));

		// Facilities are burning too
		if (unit->isFacility()) {
			if (!((Facility*)unit)->flamesParticleSystem()) {
				s = BosonParticleManager::newFire(pos);
				((Facility*)unit)->setFlamesParticleSystem(s);
				d->mParticles.append(s);
			}
			s = ((Facility*)unit)->flamesParticleSystem();
			// FIXME: maybe move this to BosonParticleManager?
			s->setCreateRate(factor * 30);
			s->setVelocity(BoVector3Fixed(0, 0, factor * 0.5));  // This is only hint for BosonParticleManager
		}
	} else {
		// If it has more hitpoints, it's not burning ;-)
		if (unit->isFacility()) {
			if (((Facility*)unit)->flamesParticleSystem()) {
				((Facility*)unit)->flamesParticleSystem()->setAge(0);
			}
		}
		if (unit->smokeParticleSystem()) {
			unit->smokeParticleSystem()->setAge(0);
		}
	}*/
 }
}

void BosonCanvas::destroyUnit(Unit* unit)
{
 // please note: you MUST NOT delete the unit here!!
 // we call it from advance() and items must not be deleted from there!
 if (!unit) {
	return;
 }
 if (!d->mDestroyedUnits.contains(unit)) {
//	boDebug() << k_funcinfo << "destroy unit " << unit->id() << endl;
	Player* owner = unit->owner();
	d->mDestroyedUnits.append(unit);

	// This stops everything
	// (except moving of flying units)
	unit->clearOrders();

	// TODO: flying units: fall down the air
	// -> we MUST stop the unit, otherwise it'd go off the map very fast.
	unit->setVelocity(0.0, 0.0, 0.0);
	unit->setSpeed(0);

	// the unit is added to a list - now displayed as a wreckage only.
	removeUnit(unit);
	// Pos is center of unit
	BoVector3Fixed pos(unit->x() + unit->width() / 2, unit->y() + unit->height() / 2, unit->z());
	//pos += unit->unitProperties()->hitPoint();
	// Make explosion if needed
	const UnitProperties* prop = unit->unitProperties();
	if (prop->explodingDamage() > 0) {
		BosonShotExplosion* e = (BosonShotExplosion*)createNewItem(RTTI::Shot, unit->owner(), ItemType(BosonShot::Explosion, 0, 0), pos);
		// Do we want ability to set fullDamageRange here?
		if (e) {
			// AB: pos parameter is redundant due to createNewItem()
			// change
			e->activate(pos, prop->explodingDamage(), prop->explodingDamageRange(), 0.0f, 10);
		}
	}
	// Add explosion fragments
	for (unsigned int i = 0; i < unit->unitProperties()->explodingFragmentCount(); i++) {
		BosonShotFragment* f = (BosonShotFragment*)createNewItem(RTTI::Shot, unit->owner(), ItemType(BosonShot::Fragment, 0, 0), pos);
		if (f) {
			// AB: pos parameter is redundant due to createNewItem()
			// change
			f->activate(pos, unit->unitProperties());

			// AB: ugly.
			// when fragments are created, they don't have the
			// unitproperties pointer yet, so we can not yet create
			// the corresponding effect object.
			// so we must notify the effects somehow that activate()
			// got called.
			// -> still this exception for fragments is very ugly.
			emit signalFragmentCreated(f);
		}
	}

	// Decrease fogref of all cells that the unit can see
	removeSight(unit);

	emit signalUnitDestroyed(unit);

	// Hide unit if wreckage should be removed immediately
	if (unit->unitProperties()->removeWreckageImmediately()) {
		unit->setVisible(false);
	}

	BoEvent* unitDestroyed = new BoEvent("UnitWithTypeDestroyed", QString::number(unit->type()));
	unitDestroyed->setUnitId(unit->id());
	unitDestroyed->setPlayerId(unit->owner()->bosonId());
	boGame->queueEvent(unitDestroyed);

	// the following events are not emitted for the neutral player
	if (owner->isActiveGamePlayer()) {
		if (owner->mobilesCount() == 0) {
			BoEvent* event = new BoEvent("AllMobileUnitsDestroyed");
			event->setPlayerId(unit->owner()->bosonId());
			boGame->queueEvent(event);
		}
		if (owner->facilitiesCount() == 0) {
			BoEvent* allFacilitiesDestroyed = new BoEvent("AllFacilitiesDestroyed");
			allFacilitiesDestroyed->setPlayerId(unit->owner()->bosonId());
			boGame->queueEvent(allFacilitiesDestroyed);
		}
		if (owner->allUnits()->count() == 0) {
			BoEvent* event = new BoEvent("AllUnitsDestroyed");
			event->setPlayerId(unit->owner()->bosonId());
			boGame->queueEvent(event);
		}
	}
 }
}

void BosonCanvas::removeUnit(Unit* unit)
{
 // please note: you MUST NOT delete the unit here!!
 // we call it from advance() and items must not be deleted from there!
 if (!unit) {
	return;
 }
 Player* owner = unit->owner();
 //unit->setAnimated(false);
 unit->setHealth(0); // in case of an accidental change before
 unit->setAdvanceWork(UnitBase::WorkDestroyed);
 owner->unitDestroyed(unit); // remove from player without deleting
 emit signalUnitRemoved(unit);

 // note: we don't add unit to any list and we don't delete it here.
 // editor will now delete it, while game mustn't delete it (displays wreckage)
}

void BosonCanvas::shotFired(BosonShot* shot, BosonWeapon* weapon)
{
 emit signalShotFired(shot, weapon);
}

Cell* BosonCanvas::cellAt(Unit* unit) const
{
 if (!unit) {
	return 0;
 }
 return cellAt(unit->x() + unit->width() / 2, unit->y() + unit->width() / 2);
}

Cell* BosonCanvas::cellAt(bofixed x, bofixed y) const
{
 return cell((int)(x), (int)(y));
}

BosonMap* BosonCanvas::map() const
{
 return d->mMap;
}

unsigned int BosonCanvas::mapWidth() const
{
 return map() ? map()->width() : 0;
}

unsigned int BosonCanvas::mapHeight() const
{
 return map() ? map()->height() : 0;
}

const float* BosonCanvas::heightMap() const
{
 return map() ? map()->heightMap() : 0;
}

void BosonCanvas::setHeightAtCorner(int x, int y, float height)
{
 BO_CHECK_NULL_RET(map());
 map()->setHeightAtCorner(x, y, height);
}

void BosonCanvas::setHeightsAtCorners(const QValueList< QPair<QPoint, float> >& heights)
{
 BO_CHECK_NULL_RET(map());
 map()->setHeightsAtCorners(heights);
}

float BosonCanvas::heightAtCorner(int x, int y) const
{
 if (!map()) {
	BO_NULL_ERROR(map());
	return 1.0f;
 }
 return map()->heightAtCorner(x, y);
}

float BosonCanvas::waterDepthAtCorner(int x, int y) const
{
 if (!map()) {
	BO_NULL_ERROR(map());
	return 0.0f;
 }
 return map()->waterDepthAtCorner(x, y);
}

float BosonCanvas::heightAtPoint(bofixed x, bofixed y) const
{
 // Coordinates of the cell (x; y) is on
 int cellX = (int)(x);
 int cellY = (int)(y);

 if ((x == cellX) && (y == cellY)) {
	return heightAtCorner(cellX, cellY) + waterDepthAtCorner(cellX, cellY);
 } else if(x == cellX) {
	bofixed y2 = (y) - cellY;
	float h1, h2;
	h1 = heightAtCorner(cellX, cellY) + waterDepthAtCorner(cellX, cellY);
	h2 = heightAtCorner(cellX, cellY + 1) + waterDepthAtCorner(cellX, cellY + 1);
	return h1 * (1 - y2) + (h2 * y2);
 } else if(y == cellY) {
	bofixed x2 = (x) - cellX;
	float h1, h2;
	h1 = heightAtCorner(cellX, cellY) + waterDepthAtCorner(cellX, cellY);
	h2 = heightAtCorner(cellX + 1, cellY) + waterDepthAtCorner(cellX + 1, cellY);
	return h1 * (1 - x2) + (h2 * x2);
 }

 // Will be used as factors for blending
 bofixed x2 = (x) - cellX;
 bofixed y2 = (y) - cellY;

 // These are heights of the corners of the cell (x; y) is on
 float h1, h2, h3, h4;

 h1 = heightAtCorner(cellX, cellY) + waterDepthAtCorner(cellX, cellY);
 h2 = heightAtCorner(cellX + 1, cellY) + waterDepthAtCorner(cellX + 1, cellY);
 h3 = heightAtCorner(cellX, cellY + 1) + waterDepthAtCorner(cellX, cellY + 1);
 h4 = heightAtCorner(cellX + 1, cellY + 1) + waterDepthAtCorner(cellX + 1, cellY + 1);

 // Blend all corners together and return the result
 // FIXME: this can probably be written _much_ more understandably and maybe faster
 return ((h1 * (1 - x2) + (h2 * x2)) * (1 - y2)) + ((h3 * (1 - x2) + (h4 * x2)) * y2);
}

float BosonCanvas::terrainHeightAtPoint(bofixed x, bofixed y) const
{
 // Coordinates of the cell (x; y) is on
 int cellX = (int)(x);
 int cellY = (int)(y);

 if ((x == cellX) && (y == cellY)) {
	return heightAtCorner(cellX, cellY);
 } else if(x == cellX) {
	bofixed y2 = (y) - cellY;
	float h1, h2;
	h1 = heightAtCorner(cellX, cellY);
	h2 = heightAtCorner(cellX, cellY + 1);
	return h1 * (1 - y2) + (h2 * y2);
 } else if(y == cellY) {
	bofixed x2 = (x) - cellX;
	float h1, h2;
	h1 = heightAtCorner(cellX, cellY);
	h2 = heightAtCorner(cellX + 1, cellY);
	return h1 * (1 - x2) + (h2 * x2);
 }

 // Will be used as factors for blending
 bofixed x2 = (x) - cellX;
 bofixed y2 = (y) - cellY;

 // These are heights of the corners of the cell (x; y) is on
 float h1, h2, h3, h4;

 h1 = heightAtCorner(cellX, cellY);
 h2 = heightAtCorner(cellX + 1, cellY);
 h3 = heightAtCorner(cellX, cellY + 1);
 h4 = heightAtCorner(cellX + 1, cellY + 1);

 // Blend all corners together and return the result
 // FIXME: this can probably be written _much_ more understandably and maybe faster
 return ((h1 * (1 - x2) + (h2 * x2)) * (1 - y2)) + ((h3 * (1 - x2) + (h4 * x2)) * y2);
}

void BosonCanvas::removeFromCells(BosonItem* item)
{
 const QPtrVector<Cell>* cells = item->cells();
 for (unsigned int i = 0; i < cells->count(); i++) {
	Cell* c = cells->at(i);
	if (!c) {
		boError() << k_funcinfo << "NULL cell at " << i << endl;
		continue;
	}
	c->removeItem(item);
 }

 if (cells->count() > 0) {
	int x1 = cells->at(0)->x();
	int y1 = cells->at(0)->y();
	int x2 = x1;
	int y2 = y1;
	for (unsigned int i = 1; i < cells->count(); i++) {
		x1 = QMIN(x1, cells->at(i)->x());
		y1 = QMIN(y1, cells->at(i)->y());
		x2 = QMAX(x2, cells->at(i)->x());
		y2 = QMAX(y2, cells->at(i)->y());
	}
	d->mQuadTreeCollection->cellUnitsChanged(this, x1, y1, x2, y2);
 }
}

void BosonCanvas::addToCells(BosonItem* item)
{
 const QPtrVector<Cell>* cells = item->cells();
 for (unsigned int i = 0; i < cells->count(); i++) {
	Cell* c = cells->at(i);
	if (!c) {
		boError() << k_funcinfo << "NULL cell at " << i << endl;
		continue;
	}
	c->addItem(item);
 }

 if (cells->count() > 0) {
	int x1 = cells->at(0)->x();
	int y1 = cells->at(0)->y();
	int x2 = x1;
	int y2 = y1;
	for (unsigned int i = 1; i < cells->count(); i++) {
		x1 = QMIN(x1, cells->at(i)->x());
		y1 = QMIN(y1, cells->at(i)->y());
		x2 = QMAX(x2, cells->at(i)->x());
		y2 = QMAX(y2, cells->at(i)->y());
	}
	d->mQuadTreeCollection->cellUnitsChanged(this, x1, y1, x2, y2);
 }
}

bool BosonCanvas::canPlaceUnitAt(const UnitProperties* prop, const BoVector2Fixed& pos, ProductionPlugin* factory) const
{
 if (!prop) {
	BO_NULL_ERROR(prop);
	return false;
 }
 bofixed width = prop->unitWidth();
 bofixed height = prop->unitHeight();
 if (width <= 0) {
	boError() << k_funcinfo << "invalid width for " << prop->typeId() << endl;
	return false;
 }
 if (height <= 0) {
	boError() << k_funcinfo << "invalid height for " << prop->typeId() << endl;
	return false;
 }
 if (!onCanvas(pos)) {
	return false;
 }
 BoRect2Fixed r(pos, BoVector2Fixed(pos.x() + width, pos.y() + height));
 if (!canGo(prop, r)) {
	return false;
 }
 if (collisions()->cellsOccupied(r)) {
	return false;
 }
 if (!factory) {
	return true;
 }
 if (prop->isMobile()) {
	// must be in BUILD_RANGE of factory
	// not perfect - there is alays a distance from center() to the edge of
	// both units which should also added to this. but this is not a maths
	// contest, so its ok this way
	Unit* factoryUnit = factory->unit();
	if (!factoryUnit) {
		boError() << k_funcinfo << "production plugin has NULL owner" << endl;
		return false;
	}
	if ((r.center() - factoryUnit->center()).dotProduct() <= BUILD_RANGE * BUILD_RANGE) {
		return true;
	}
 } else {
	const RefineryProperties* refinery = (RefineryProperties*)prop->properties(PluginProperties::Refinery);
	if(refinery) {
		// Refineries can't be built close to resource mines
		QValueList<Unit*> list = collisions()->unitCollisionsInRange(r.center(), REFINERY_FORBID_RANGE);
		QValueList<Unit*>::Iterator it;
		for (it = list.begin(); it != list.end(); it++) {
			ResourceMinePlugin* resource = (ResourceMinePlugin*)(*it)->plugin(UnitPlugin::ResourceMine);
			if(resource) {
				if((refinery->canRefineMinerals() && resource->canProvideMinerals()) ||
						(refinery->canRefineOil() && resource->canProvideOil())) {
					return false;
				}
			}
		}
	}
	// must be in BUILD_RANGE of any facility of the player
	QValueList<Unit*> list = collisions()->unitCollisionsInRange(r.center(), BUILD_RANGE);
	QValueList<Unit*>::Iterator it;
	for (it = list.begin(); it != list.end(); it++) {
		if ((*it)->isFacility() && (*it)->owner() == factory->player()) {
			return true;
		}
	}
 }
 return false;
}

BoItemList* BosonCanvas::allItems() const
{
 return &d->mAllItems;
}

unsigned int BosonCanvas::allItemsCount() const
{
 return d->mAllItems.count();
}

void BosonCanvas::addItem(BosonItem* item)
{
 d->mAllItems.append(item);

 // by default it goes to "work" == -1. units will change this.
 d->mWork2AdvanceList[-1].append(item);
}

void BosonCanvas::deleteItem(BosonItem* item)
{
 // remove the item from the canvas BEFORE deleting it. we might need to do some
 // cleanups and might need rtti() for them (which doesnt exist anymore in the
 // BosonItem d'tor)
 if (RTTI::isUnit(item->rtti())) {
	Unit* u = (Unit*)item;
	// Update occupied status of cells that unit occupied
	u->setMovingStatus(UnitBase::RemovingThis);
	// In editor mode, we need to do couple of things before deleting the unit,
	//  to prevent crashes later (e.g. when selecting units)
	if (!boGame->gameMode()) {
		u->owner()->unitDestroyed(u);
		emit signalUnitRemoved(u);
	}
 }

 removeItem(item);

 // actually delete it
 delete item;
}

void BosonCanvas::removeItem(BosonItem* item)
{
 d->mAllItems.remove(item);
 for (BoItemList::Iterator it = d->mAllItems.begin(); it != d->mAllItems.end(); ++it) {
	(*it)->itemRemoved(item);
 }
 if (RTTI::isUnit(item->rtti())) {
	Unit* u = (Unit*)item;
	if (d->mDestroyedUnits.contains(u)) {
		d->mDestroyedUnits.remove(u);
		//boError() << k_funcinfo << item << " still in destroyed units list" << endl;
	}
 }

 // remove from all advance lists
 for (QMap<int, QPtrList<BosonItem> >::Iterator it = d->mWork2AdvanceList.begin(); it != d->mWork2AdvanceList.end(); ++it) {
	(*it).removeRef(item);
 }
 d->mChangeAdvanceList.removeRef(item);

 emit signalRemovedItem(item);
}

void BosonCanvas::deleteUnusedShots()
{
 QPtrList<BosonItem> unusedShots;
 BoItemList::Iterator it;
 for (it = d->mAllItems.begin(); it != d->mAllItems.end(); ++it) {
	if (RTTI::isShot((*it)->rtti())) {
		BosonShot* shot = (BosonShot*)*it;
		if (!shot->isActive()) {
			unusedShots.append(*it);
		}
	}
 }
 while (!unusedShots.isEmpty()) {
	BosonItem* i = unusedShots.take(0);
	deleteItem(i);
 }
}

bool BosonCanvas::loadFromXML(const QDomElement& root)
{
 PROFILE_METHOD
 if (root.isNull()) {
	boError(260) << k_funcinfo << "NULL root node" << endl;
	return false;
 }


 if (!loadItemsFromXML(root)) {
	boError(260) << k_funcinfo << "unable to load items from XML" << endl;
	return false;
 }

 QDomElement handler = root.namedItem("DataHandler").toElement();
 if (handler.isNull()) {
	boError(260) << k_funcinfo << "DataHandler not found" << endl;
	return false;
 }
 BosonPropertyXML propertyXML;
 if (!propertyXML.loadFromXML(handler, d->mProperties)) {
	boError(260) << k_funcinfo << "unable to load the datahandler" << endl;
	return false;
 }

 initPathFinder();
 boDebug(260) << k_funcinfo << "done" << endl;
 return true;
}

bool BosonCanvas::loadItemsFromXML(const QDomElement& root)
{
 PROFILE_METHOD

 // TODO: FIXME
 // elementsByTagName() searches recursively, but we know that all Items tags
 // are direct child of root!!
 // -> this is speed relevant for large maps
 QDomNodeList list = root.elementsByTagName(QString::fromLatin1("Items"));
 QValueList<QDomElement> allItemElements;
 QValueList<BosonItem*> allItems;
 for (unsigned int i = 0; i < list.count(); i++) {
	QDomElement items = list.item(i).toElement();
	if (items.isNull()) {
		boError(260) << k_funcinfo << "Items tag is not an element" << endl;
		continue;
	}
	bool ok = false;

	unsigned int id = items.attribute(QString::fromLatin1("PlayerId")).toUInt(&ok);
	if (!ok) {
		boError(260) << k_funcinfo << "PlayerId of Items Tag " << i << " is not a valid number" << endl;
		continue;
	}
	Player* owner = (Player*)boGame->findPlayerByUserId(id);
	if (!owner) {
		// AB: this is totally valid. less players in game, than in the
		// file.
		continue;
	}

	QDomNodeList itemList = items.elementsByTagName(QString::fromLatin1("Item"));
	for (unsigned int j = 0; j < itemList.count(); j++) {
		QDomElement item = itemList.item(j).toElement();
		if (item.isNull()) {
			continue;
		}
		BosonItem* i = createItemFromXML(item, owner);
		if (!i) {
			boError(260) << k_funcinfo << "failed creating item " << j << endl;
			continue;
		}
		allItemElements.append(item);
		allItems.append(i);
	}
 }
 if (allItemElements.count() != allItems.count()) {
	boError(260) << k_funcinfo << "item count != element count" << endl;
	return false;
 }
 boDebug(260) << k_funcinfo << "created " << allItems.count() << " items" << endl;

 unsigned int itemCount = 0;
 for (unsigned int i = 0; i < allItems.count(); i++) {
	QDomElement e = allItemElements[i];
	BosonItem* item = allItems[i];
	if (!loadItemFromXML(e, item)) {
		boError(260) << k_funcinfo << "failed loading item" << endl;
		return false;
	}
	itemCount++;
 }
 boDebug(260) << k_funcinfo << "loaded " << itemCount << " items" << endl;

 return true;
}

BosonItem* BosonCanvas::createItemFromXML(const QDomElement& item, Player* owner)
{
 PROFILE_METHOD
 if (item.isNull()) {
	return 0;
 }
 if (!owner) {
	return 0;
 }
 bool ok = false;
 int rtti = item.attribute(QString::fromLatin1("Rtti")).toInt(&ok);
 if (!ok) {
	boError(260) << k_funcinfo << "Rtti attribute of Item is not a valid number" << endl;
	return 0;
 }

 unsigned long int type = 0;
 unsigned long int group = 0;
 unsigned long int groupType = 0;

 if (!item.hasAttribute(QString::fromLatin1("Type"))) {
	// check for deprecated attributes
	if (!item.hasAttribute(QString::fromLatin1("UnitType"))) {
		boError(260) << k_funcinfo << "missing attribute: Type for Item tag" << endl;
		return 0;
	} else {
		type = item.attribute(QString::fromLatin1("UnitType")).toULong(&ok);
	}
 } else {
	type = item.attribute(QString::fromLatin1("Type")).toULong(&ok);
 }
 if (!ok) {
	boError(260) << k_funcinfo << "Invalid Type number for Item tag" << endl;
	return 0;
 }

 if (item.hasAttribute(QString::fromLatin1("Group"))) {
	group = item.attribute(QString::fromLatin1("Group")).toULong(&ok);
 } else {
	// check for deprecated attributes.
	if (item.hasAttribute(QString::fromLatin1("UnitType")) && item.hasAttribute(QString::fromLatin1("Type"))) {
		// old Shot tags used "Type" for the type and "UnitType" for the
		// "Group" attribute.
		group = item.attribute(QString::fromLatin1("UnitType")).toULong(&ok);
	} else {
		// "Group" attribute is not present.
		// this is totally valid! items don't have to provide a Group
		// attribute.
		ok = true;
	}
 }
 if (!ok) {
	boError(260) << k_funcinfo << "Invalid Group number for Item tag" << endl;
	return 0;
 }

 if (item.hasAttribute(QString::fromLatin1("GroupType"))) {
	groupType = item.attribute(QString::fromLatin1("GroupType")).toULong(&ok);
 } else {
	// check for deprecated attributes.
	if (item.hasAttribute(QString::fromLatin1("WeaponType"))) {
		groupType = item.attribute(QString::fromLatin1("WeaponType")).toULong(&ok);
	}
 }
 if (!ok) {
	boError(260) << k_funcinfo << "Invalid GroupType number for Item tag" << endl;
	return 0;
 }

 BoVector3Fixed pos;
 pos.setX(item.attribute("x").toFloat(&ok));
 if (!ok) {
	boError() << k_funcinfo << "x attribute for Item tag missing or invalid" << endl;
	return 0;
 }
 pos.setY(item.attribute("y").toFloat(&ok));
 if (!ok) {
	boError() << k_funcinfo << "y attribute for Item tag missing or invalid" << endl;
	return 0;
 }
 pos.setZ(item.attribute("z").toFloat(&ok));
 if (!ok) {
	// missing z is ok, but not recommended.
	pos.setZ(0.0f);
 }


 unsigned long int id = 0;
 if (!item.hasAttribute(QString::fromLatin1("Id"))) {
	boError(260) << k_funcinfo << "missing attribute: Id for Item tag" << endl;
	return 0;
 }
 // AB: "0" indicates that we want boson to assign an Id. the tag must be prsent.
 id = item.attribute(QString::fromLatin1("Id")).toULong(&ok);
 if (!ok) {
	boError(260) << k_funcinfo << "Invalid Id number for Item tag" << endl;
	return 0;
 }

 if (id == 0) {
	id = nextItemId();
 }

 if (RTTI::isUnit(rtti)) {
	if (!item.hasAttribute(QString::fromLatin1("DataHandlerId"))) {
		boError(260) << k_funcinfo << "missing attribute: DataHandlerId for Item tag" << endl;
		return 0;
	}
	int dataHandlerId = -1;

	if (item.hasAttribute(QString::fromLatin1("DataHandlerId"))) {
		dataHandlerId = item.attribute(QString::fromLatin1("DataHandlerId")).toInt(&ok);
		if (!ok) {
			boError(260) << k_funcinfo << "Invalid DataHandlerId number for Item tag" << endl;
			return 0;
		}
	}

	// FIXME: I think we should move addUnit() to bosoncanvas.
	//
	// AB: TODO: createNewUnit() - which includes owner->addUnit()
	// AB: maybe drop create_New_Item completely and move all it does to
	// createItem().
	// (i.e. the owner->addUnit() and the theme->loadNewUnit() call. also a
	// few additional exceptions (editor, flying unit)).
	Unit* u = (Unit*)createItem(RTTI::UnitStart + type, owner, ItemType(type), pos, id);

	if (!u) {
		boError(260) << k_funcinfo << "could not create unit type=" << type << " for owner=" << owner->bosonId() << endl;
		return 0;
	}


	// Set additional properties
	owner->addUnit(u, dataHandlerId);

	// AB: some units may depend on properties of other units - e.g. on
	// whether a unit is constructed completely.
	// we must make sure that these properties are already loaded, even if
	// the unit that a unit depends on is loaded later.
	// I hope loading the DataHandler in advance will solve this problem
	// (it comes up for harvesters currently, as they require the
	// refineries/mines to be completely constructed, as Unit::plugin()
	// returns 0 otherwise)
	BosonCustomPropertyXML propertyXML;
	QDomElement handler = item.namedItem(QString::fromLatin1("DataHandler")).toElement();
	if (handler.isNull()) {
		boError() << k_funcinfo << "NULL DataHandler tag for item" << endl;
		delete u;
		return 0;
	}
	if (!propertyXML.loadFromXML(handler, u->dataHandler())) {
		boError(260) << k_funcinfo << "unable to load item data handler" << endl;
		return false;
	}

	return (BosonItem*)u;
 } else if (RTTI::isShot(rtti)) {
	BosonShot* s = (BosonShot*)createItem(RTTI::Shot, owner, ItemType(type, group, groupType), pos, id);
	if (!s) {
		boError() << k_funcinfo << "Invalid shot - type=" << type << " group=" << group << " groupType=" << groupType << endl;
		return 0;
	}
	return (BosonItem*)s;
 } else {
	boError(260) << k_funcinfo << "unknown Rtti " << rtti << endl;
	return 0;
 }
 return 0;
}

bool BosonCanvas::loadItemFromXML(const QDomElement& element, BosonItem* item)
{
 PROFILE_METHOD
 if (!item) {
	return false;
 }
 if (!item->loadFromXML(element)) {
	boError(260) << k_funcinfo << "Could not load item correctly" << endl;
	if (RTTI::isUnit(item->rtti())) {
		// need to remove from player again
		Player* owner = ((Unit*)item)->owner();
		if (owner) {
			owner->unitDestroyed((Unit*)item);
		}
	}
	return false;
 }
 return true;
}

bool BosonCanvas::saveAsXML(QDomElement& root) const
{
 boDebug() << k_funcinfo << endl;

 if (!saveItemsAsXML(root)) {
	boError() << k_funcinfo << "cannot save items as xml" << endl;
	return false;
 }

 QDomDocument doc = root.ownerDocument();
 // Save pathfinder
 QDomElement pathFinderXML = doc.createElement(QString::fromLatin1("Pathfinder"));
 root.appendChild(pathFinderXML);
 if (d->mPathFinder) {
	d->mPathFinder->saveAsXML(pathFinderXML);
 }

 // Save datahandler
 BosonPropertyXML propertyXML;
 QDomElement handler = doc.createElement(QString::fromLatin1("DataHandler"));
 root.appendChild(handler);
 if (!propertyXML.saveAsXML(handler, d->mProperties)) {
	boError() << k_funcinfo << "unable to save the datahandler" << endl;
	return false;
 }
 return true;
}

bool BosonCanvas::saveItemsAsXML(QDomElement& root) const
{
 QDomDocument doc = root.ownerDocument();
 QMap<unsigned int, QDomElement> owner2Items;
 QPtrList<Player> gamePlayerList = *boGame->gamePlayerList();
 for (KPlayer* p = gamePlayerList.first(); p; p = gamePlayerList.next()) {
	QDomElement items = doc.createElement(QString::fromLatin1("Items"));

	// note: we need to store the index in the list here, not the p->kgameId() !
	items.setAttribute(QString::fromLatin1("PlayerId"), ((Player*)p)->bosonId());
	root.appendChild(items);
	owner2Items.insert(((Player*)p)->bosonId(), items);
 }

 BoItemList::Iterator it;
 for (it = d->mAllItems.begin(); it != d->mAllItems.end(); ++it) {
	BosonItem* i = *it;
	QDomElement items;
	if (!i->owner()) {
		BO_NULL_ERROR(i->owner());
		return false;
	}
	unsigned int id = i->owner()->bosonId();
	items = owner2Items[id];
	if (items.isNull()) {
		boError() << k_funcinfo << "no Items element found" << endl;
		return false;
	}
	QDomElement item = doc.createElement(QString::fromLatin1("Item"));
	if (RTTI::isShot(i->rtti())) {
		if (!((BosonShot*)i)->isActive()) {
			continue;
		}
	}
	if (!i->saveAsXML(item)) {
		boError() << k_funcinfo << "Could not save item " << i << endl;
		return false;
	}
	items.appendChild(item);
 }
 return true;
}

void BosonCanvas::changeAdvanceList(BosonItem* item)
{
 if (!d->mChangeAdvanceList.contains(item)) { // AB: this requires a complete search (I guess at least)! might be slow
	d->mChangeAdvanceList.append(item);
 }
}

void BosonCanvas::removeFromAdvanceLists(BosonItem* item)
{
 // this is slow :(
 // we need to iterator through all lists and all lists need to search for the
 // item. since all (except one) will fail finding the item they need to search
 // very long (remember: a search for a not-existing item usually takes long).
 // we cannot use an "oldWork" variable or so, as we can never depend 100% on
 // it. there may be several situations where it is unreliable (loading games,
 // changing advancWork() very often in one advance call, ...)
 //
 // possible solutions may be to add a QPtrDict which maps item->list. so
 // whenever we add an item to a list, we also add this item to that dict (and
 // of course remove when it when it gets removed from the list). then we could
 // get the correct list in constant time.
 QMap<int, QPtrList<BosonItem> >::Iterator it;
 for (it = d->mWork2AdvanceList.begin(); it != d->mWork2AdvanceList.end(); ++it) {
	(*it).removeRef(item);
 }
}

bool BosonCanvas::onCanvas(const BoVector2Fixed& pos) const
{
 return onCanvas(pos.x(), pos.y());
}

bool BosonCanvas::onCanvas(const BoVector3Fixed& pos) const
{
 return onCanvas(pos.x(), pos.y());
}

void BosonCanvas::deleteItems(const QValueList<unsigned long int>& _ids)
{
 if (!boGame || boGame->gameMode()) {
	boError() << k_funcinfo << "not in editor mode" << endl;
	return;
 }
 QValueList<unsigned long int> ids = _ids;
 BoItemList::Iterator it;
 while (!ids.isEmpty()) {
	unsigned long int id = ids.first();
	ids.pop_front();
	BosonItem* item = 0;
	for (it = d->mAllItems.begin(); !item && it != d->mAllItems.end(); ++it) {
		if (id == (*it)->id()) {
			item = (*it);
		}
	}
	deleteItem(item);
 }
}

void BosonCanvas::deleteItems(BoItemList& items)
{
 QPtrList<BosonItem> list;
 BoItemList::Iterator it;
 for (it = items.begin(); it != items.end(); ++it) {
	list.append(*it);
 }
 deleteItems(list);
 if (list.count() != 0) {
	boError() << k_funcinfo << "error on deleting items!" << endl;
 }
 items.clear();
}

void BosonCanvas::deleteItems(QPtrList<BosonItem>& items)
{
 while (items.count() > 0) {
	BosonItem* i = items.first();
	items.removeRef(i);
	deleteItem(i);
 }
}

BosonItem* BosonCanvas::createNewItem(int rtti, Player* owner, const ItemType& type, const BoVector3Fixed& pos)
{
 PROFILE_METHOD
 BosonItem* item = createItem(rtti, owner, type, pos, nextItemId());
 if (!item) {
	return 0;
 }
 if (RTTI::isUnit(item->rtti())) {
	Unit* unit = (Unit*)item;
	if (unit->owner() != owner) {
		boError() << k_funcinfo << "unexpected owner for new unit" << endl;
		return item;
	}
	owner->addUnit(unit);
	SpeciesTheme* theme = owner->speciesTheme();
	if (!theme) {
		boError() << k_funcinfo << "NULL speciesTheme" << endl;
		return item;
	}
	theme->loadNewUnit(unit);
 }

 return item;
}

BosonItem* BosonCanvas::createItem(int rtti, Player* owner, const ItemType& type, const BoVector3Fixed& pos, unsigned long int id)
{
 PROFILE_METHOD
 BosonItem* item = 0;
 if (!onCanvas(pos)) {
	boError() << k_funcinfo << "(" << pos[0] << "," << pos[1] << "," << pos[2] << ") is not on the canvas" << endl;
	return 0;
 }
 if (id == 0) {
	boError() << k_funcinfo << "id==0 is invalid." << endl;
	return 0;
 }
 if (RTTI::isUnit(rtti)) {
	item = (BosonItem*)createUnit(owner, type.mType);
 } else if (RTTI::isShot(rtti)) {
	item = (BosonItem*)createShot(owner, type.mType, type.mGroup, type.mGroupType);
 }
 if (item) {
	addItem(item);
	item->setId(id);
	item->move(pos.x(), pos.y(), pos.z());
	if (item && !item->init()) {
		boError() << k_funcinfo << "item initialization failed. cannot create item." << endl;
		deleteItem(item);
		item = 0;
	}
 }
 if (item) {
	if (RTTI::isUnit(rtti)) {
		Unit* unit = (Unit*)item;
		// We also need to recalc occupied status for cells that unit is on.
		// FIXME: this is hackish
		unitMovingStatusChanges(unit, UnitBase::Moving, UnitBase::Standing);
		// This unit might be a radar and/or radar jammer
		d->mSightManager->updateVisibleStatus(unit);
		addRadar(unit);
		addRadarJammer(unit);
	}
	emit signalItemAdded(item);
 }
 return item;
}

Unit* BosonCanvas::createUnit(Player* owner, unsigned long int unitType)
{
 PROFILE_METHOD
 BO_CHECK_NULL_RET0(owner);
 SpeciesTheme* theme = owner->speciesTheme();
 BO_CHECK_NULL_RET0(theme); // BAAAAD - will crash

 const UnitProperties* prop = theme->unitProperties(unitType);
 if (!prop) {
	boError() << k_funcinfo << "Unknown unitType " << unitType << endl;
	return 0;
 }

 Unit* unit = new Unit(prop, owner, this);
 unit->setMoveData(moveData(prop));
 return unit;
}

BosonShot* BosonCanvas::createShot(Player* owner, unsigned long int shotType, unsigned long int unitType, unsigned long int weaponPropertyId)
{
 PROFILE_METHOD
 BO_CHECK_NULL_RET0(owner);
 BosonShot* s = 0;
 switch (shotType) {
	case BosonShot::Bullet:
	{
		SpeciesTheme* t = owner->speciesTheme();
		BO_CHECK_NULL_RET0(t);
		const UnitProperties* unitProperties = t->unitProperties(unitType);
		BO_CHECK_NULL_RET0(unitProperties);
		const BosonWeaponProperties* prop = unitProperties->weaponProperties(weaponPropertyId);
		BO_CHECK_NULL_RET0(prop);
		s = (BosonShot*)new BosonShotBullet(owner, this, prop);
		break;
	}
	case BosonShot::Rocket:
	{
		SpeciesTheme* t = owner->speciesTheme();
		BO_CHECK_NULL_RET0(t);
		const UnitProperties* unitProperties = t->unitProperties(unitType);
		BO_CHECK_NULL_RET0(unitProperties);
		const BosonWeaponProperties* prop = unitProperties->weaponProperties(weaponPropertyId);
		BO_CHECK_NULL_RET0(prop);
		s = (BosonShot*)new BosonShotRocket(owner, this, prop);
		break;
	}
	case BosonShot::Missile:
	{
		SpeciesTheme* t = owner->speciesTheme();
		BO_CHECK_NULL_RET0(t);
		const UnitProperties* unitProperties = t->unitProperties(unitType);
		BO_CHECK_NULL_RET0(unitProperties);
		const BosonWeaponProperties* prop = unitProperties->weaponProperties(weaponPropertyId);
		BO_CHECK_NULL_RET0(prop);
		s = (BosonShot*)new BosonShotMissile(owner, this, prop);
		break;
	}
	case BosonShot::Explosion:
		s = (BosonShot*)new BosonShotExplosion(owner, this);
		break;
	case BosonShot::Mine:
	{
		SpeciesTheme* t = owner->speciesTheme();
		BO_CHECK_NULL_RET0(t);
		const UnitProperties* unitProperties = t->unitProperties(unitType);
		BO_CHECK_NULL_RET0(unitProperties);
		const BosonWeaponProperties* prop = unitProperties->weaponProperties(weaponPropertyId);
		BO_CHECK_NULL_RET0(prop);
		s = (BosonShot*)new BosonShotMine(owner, this, prop);
		break;
	}
	case BosonShot::Bomb:
	{
		SpeciesTheme* t = owner->speciesTheme();
		BO_CHECK_NULL_RET0(t);
		const UnitProperties* unitProperties = t->unitProperties(unitType);
		BO_CHECK_NULL_RET0(unitProperties);
		const BosonWeaponProperties* prop = unitProperties->weaponProperties(weaponPropertyId);
		BO_CHECK_NULL_RET0(prop);
		s = (BosonShot*)new BosonShotBomb(owner, this, prop);
		break;
	}
	case BosonShot::Fragment:
	{
		s = (BosonShot*)new BosonShotFragment(owner, this);
		break;
	}
	default:
		boError() << k_funcinfo << "Invalid type: " << shotType << endl;
		s = 0;
		break;
 }
 return s;
}

unsigned long int BosonCanvas::nextItemId()
{
 // note that per definition 0 is an invalid item ID!
 d->mNextItemId = d->mNextItemId + 1;
 return d->mNextItemId;
}

void BosonCanvas::initPathFinder()
{
 PROFILE_METHOD

 if (d->mPathFinder) {
	boError() << k_funcinfo << "PathFinder already created!" << endl;
	return;
 }

 d->mPathFinder = new BosonPath(map());
 d->mPathFinder->init(this);

 for (BoItemList::ConstIterator it = allItems()->begin(); it != allItems()->end(); ++it) {
	if (RTTI::isUnit((*it)->rtti())) {
		Unit* u = (Unit*)*it;
		u->setMoveData(moveData(u->unitProperties()));
	}
 }
}

BosonPath* BosonCanvas::pathFinder() const
{
 return d->mPathFinder;
}

void BosonCanvas::unitMovingStatusChanges(Unit* u, int oldstatus, int newstatus)
{
 if (pathFinder()) {
	pathFinder()->unitMovingStatusChanges(u, oldstatus, newstatus);
 }
}

BoEventListener* BosonCanvas::eventListener() const
{
 return d->mEventListener;
}

bool BosonCanvas::saveConditions(QDomElement& root) const
{
 if (!eventListener()) {
	BO_NULL_ERROR(eventListener());
	return false;
 }
 return eventListener()->saveConditions(root);
}

bool BosonCanvas::loadConditions(const QDomElement& root)
{
 if (!eventListener()) {
	BO_NULL_ERROR(eventListener());
	return false;
 }
 return eventListener()->loadConditions(root);
}

BosonItem* BosonCanvas::findItem(unsigned long int id) const
{
 return d->mAllItems.findItem(id);
}

Unit* BosonCanvas::findUnit(unsigned long int id) const
{
 BosonItem* item = findItem(id);
 if (!item || !RTTI::isUnit(item->rtti())) {
	return 0;
 } else {
	return (Unit*)item;
 }
}

void BosonCanvas::clearMoveDatas()
{
 d->mUnitProperties2MoveData.setAutoDelete(false);
 QPtrList<BosonMoveData> datas;
 for (QIntDictIterator<BosonMoveData> it(d->mUnitProperties2MoveData); it.current(); ++it) {
	if (!datas.contains(it.current())) {
		datas.append(it.current());
	}
 }
 d->mUnitProperties2MoveData.clear();
 while (!datas.isEmpty()) {
	BosonMoveData* data = datas.take(0);
	delete data;
 }
}

void BosonCanvas::insertMoveData(const UnitProperties* prop, BosonMoveData* data)
{
 BO_CHECK_NULL_RET(prop);
 BO_CHECK_NULL_RET(data);
 d->mUnitProperties2MoveData.insert(prop->typeId(), data);
}

BosonMoveData* BosonCanvas::moveData(const UnitProperties* prop) const
{
 BO_CHECK_NULL_RET0(prop);
 return d->mUnitProperties2MoveData[prop->typeId()];
}

void BosonCanvas::registerQuadTree(BoCanvasQuadTreeNode* tree)
{
 d->mQuadTreeCollection->registerTree(tree);
}

void BosonCanvas::unregisterQuadTree(BoCanvasQuadTreeNode* tree)
{
 d->mQuadTreeCollection->unregisterTree(tree);
}

