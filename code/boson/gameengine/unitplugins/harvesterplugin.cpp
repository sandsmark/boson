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
#include "harvesterplugin.h"

#include "../../bomemory/bodummymemory.h"
#include "../defines.h"
#include "refineryplugin.h"
#include "resourcemineplugin.h"
#include "unit.h"
#include "unitproperties.h"
#include "pluginproperties.h"
#include "speciestheme.h"
#include "player.h"
#include "playerio.h"
#include "bosoncanvas.h"
#include "boson.h"
#include "bosonstatistics.h"
#include "bodebug.h"
#include "unitorder.h"
#include "../bo3dtools.h"

#include <qdom.h>

#include <math.h>

HarvesterPlugin::HarvesterPlugin(Unit* unit)
		: UnitPlugin(unit)
{
 // FIXME: we should clean the property IDs. They should be in UnitPlugin, not
 // in Unit.
 unit->registerData(&mResourcesMined, Unit::IdResourcesMined);
 unit->registerData(&mResourcesX, Unit::IdResourcesX);
 unit->registerData(&mResourcesY, Unit::IdResourcesY);
 unit->registerData(&mHarvestingType, Unit::IdHarvestingType);
 mResourcesMined.setLocal(0);
 mResourcesX.setLocal(0); // obsolete
 mResourcesY.setLocal(0); // obsolete
 mHarvestingType.setLocal(0);

 mRefinery = 0;
 mResourceMine = 0;
}

HarvesterPlugin::~HarvesterPlugin()
{
}

void HarvesterPlugin::advance(unsigned int)
{
 if (mHarvestingType == 0) {
	unit()->currentSuborderDone(true);
	return;
 } else if (mHarvestingType == 1) {
	advanceMine();
 } else if (mHarvestingType == 2) {
	advanceRefine();
 }
}

bool HarvesterPlugin::saveAsXML(QDomElement& root) const
{
 unsigned int refineryId = 0;
 unsigned int mine = 0;
 if (mRefinery) {
	refineryId = mRefinery->unit()->id();
 }
 if (mResourceMine) {
	mine = mResourceMine->unit()->id();
 }
 root.setAttribute(QString::fromLatin1("Refinery"), refineryId);
 root.setAttribute(QString::fromLatin1("ResourceMine"), mine);
 return true;
}

bool HarvesterPlugin::loadFromXML(const QDomElement& root)
{
 unsigned int refineryId = 0;
 unsigned int mineId = 0;
 bool ok = false;

 mRefinery = 0;
 mResourceMine = 0;

 refineryId = root.attribute(QString::fromLatin1("Refinery")).toUInt(&ok);
 if (!ok) {
	boError() << k_funcinfo << "Invalid number for Refinery attribute" << endl;
	return false;
 }
 mineId = root.attribute(QString::fromLatin1("ResourceMine")).toUInt(&ok);
 if (!ok) {
	boError() << k_funcinfo << "Invalid number for ResourceMine attribute" << endl;
	return false;
 }
 if (refineryId != 0) {
	// AB: retrieving from Boson is not 100% nice, but definitely necessary
	// and valid at this point. we need to get the pointer, even if the
	// refinery is an enemy or if it is invisible to us
	// --> it was saved this way, so we must load it this way.
	Unit* u = game()->findUnit(refineryId, 0);
	if (!u) {
		boError() << k_funcinfo << "cannot find refinery mine " << refineryId << endl;
		return false;
	} else {
		mRefinery = (RefineryPlugin*)u->plugin(UnitPlugin::Refinery);
		if (!mRefinery) {
			boError() << k_funcinfo << "unit " << refineryId << " is not a refinery" << endl;
			mRefinery = 0;
			return false;
		}
	}
 }
 if (mineId != 0) {
	Unit* u = game()->findUnit(mineId, 0);
	if (!u) {
		boError() << k_funcinfo << "cannot find resource mine " << mineId << endl;
	} else {
		mResourceMine = (ResourceMinePlugin*)u->plugin(UnitPlugin::ResourceMine);
		if (!mResourceMine) {
			boError() << k_funcinfo << "unit " << mineId << " is not a resource mine" << endl;
			mResourceMine = 0;
		}
	}
 }
 return true;
}

bool HarvesterPlugin::isAtResourceMine() const
{
 if (!mResourceMine) {
	return false;
 }
 if (!mResourceMine->unit()) {
	BO_NULL_ERROR(mResourceMine->unit());
	return false;
 }
 return isNextTo(mResourceMine->unit());
}

bool HarvesterPlugin::isAtRefinery() const
{
 if (!mRefinery) {
	return false;
 }
 if (!mRefinery->unit()) {
	BO_NULL_ERROR(mRefinery->unit());
	return false;
 }
 return isNextTo(mRefinery->unit());
}

void HarvesterPlugin::advanceMine()
{
 boDebug(430) << k_funcinfo << endl;
 const HarvesterProperties* prop = (HarvesterProperties*)properties(PluginProperties::Harvester);
 if (!prop) {
	boError(430) << k_funcinfo << "NULL harvester properties" << endl;
	mHarvestingType = 0; // stop
	return;
 }
 if (!mResourceMine || !mResourceMine->isUsableTo(this)) {
	ResourceMinePlugin* mine = findClosestResourceMine();
	if (!mine || !mine->unit()) {
		boDebug(430) << k_funcinfo << "no resource mine found" << endl;
		mHarvestingType = 0; // stop
	} else {
		boDebug(430) << k_funcinfo << "resource mine: " << mine->unit()->id() << endl;
		mineAt(mine);
	}
	return;
 }
 if (!mResourceMine || !mResourceMine->isUsableTo(this)) {
	QString mineId = "no id";
	if (mResourceMine && mResourceMine->unit()) {
		mineId = QString::number(mResourceMine->unit()->id());
	}
	boDebug(430) << k_funcinfo << "cannot mine at " << mResourceMine << " (" << mineId << ")" << endl;
	mHarvestingType = 0; // stop

	// TODO: handle special case when mine has become empty!
	// -> we should go to a refinery now
	return;
 }

 BO_CHECK_NULL_RET(mResourceMine);
 BO_CHECK_NULL_RET(mResourceMine->unit());
 if (mResourceMine->unit()->isDestroyed()) {
	boDebug(430) << k_funcinfo << "resource mine has been destroyed!" << endl;
	mResourceMine = 0; // search a new one
	return;
 }

 // Check if unit is at mining location. If not, go there
 if (!isAtResourceMine()) {
	Unit* u = mResourceMine->unit();
	if (!unit()->addCurrentSuborder(new UnitMoveToUnitOrder(u))) {
		boDebug(430) << k_funcinfo << "Cannot move to refinery (id=" << u->id() <<
				") at (" << u->x() << "; " << u->y() << ")" << endl;
		unit()->currentSuborderDone(false);
	}
	return;
 }
 if (resourcesMined() >= prop->maxResources()) {
	// Back to refinery
	boDebug(430) << k_funcinfo << "Maximal amount of resources mined." << endl;
	mHarvestingType = 2; // refining
	return;
 }
 unsigned int mined = 0;
 if (canMineMinerals() && mResourceMine->canProvideMinerals()) {
	mined = mResourceMine->mineMinerals(this);
	player()->statistics()->increaseMinedMinerals(mined);
 } else if (canMineOil() && mResourceMine->canProvideOil()) {
	mined = mResourceMine->mineOil(this);
	player()->statistics()->increaseMinedOil(mined);
 } else {
	boError(430) << k_funcinfo << "oops - cannot mine here?!" << endl;
	mined = 0;
 }
 if (resourcesMined() + mined > prop->maxResources()) {
	// any additional resources are lost now, not even another harvester can
	// get them (intended).
	// cannot happen anyway.
	mined = prop->maxResources() - resourcesMined();
 }
 mResourcesMined = resourcesMined() + mined;
 boDebug(430) << k_funcinfo << "resources mined: " << resourcesMined() << endl;
}

void HarvesterPlugin::advanceRefine()
{
 // This is the second step of harvesting: returning to refinery and unloading
 boDebug(430) << k_funcinfo << endl;
 if (resourcesMined() == 0) {
	boDebug(430) << k_funcinfo << "refining done" << endl;
	mHarvestingType = 1; // mining
	return;
 }
 if (!mRefinery) {
	RefineryPlugin* refinery = findClosestRefinery();
	if (!refinery || !refinery->unit()) {
		boDebug(430) << k_funcinfo << "no refinery found" << endl;
		mHarvestingType = 0; // stop
	} else {
		boDebug(430) << k_funcinfo << "refinery: " << refinery->unit()->id() << endl;
		refineAt(refinery);
	}
	return;
 }

 BO_CHECK_NULL_RET(mRefinery);
 BO_CHECK_NULL_RET(mRefinery->unit());
 if (mRefinery->unit()->isDestroyed()) {
	boDebug(430) << k_funcinfo << "refinery has been destroyed!" << endl;
	mRefinery = 0; // search a new one
	return;
 }

 if (!isAtRefinery()) {
	Unit* u = mRefinery->unit();
	if (!unit()->addCurrentSuborder(new UnitMoveToUnitOrder(u))) {
		boDebug(430) << k_funcinfo << "Cannot move to refinery (id=" << u->id() <<
				") at (" << u->x() << "; " << u->y() << ")" << endl;
		unit()->currentSuborderDone(false);
	}
	return;
 }
 const HarvesterProperties* prop = (HarvesterProperties*)unit()->properties(PluginProperties::Harvester);
 if (!prop) {
	boError(430) << k_funcinfo << "NULL harvester plugin" << endl;
	mHarvestingType = 0; // stop
	return;
 }

 int amount = unloadingSpeed();
 if (amount > (int)resourcesMined()) {
	amount = resourcesMined();
 }
 if (amount < 0) {
	boError(430) << k_funcinfo << "a negative amount of resources to be refined??" << endl;
	amount = 0;
 }
 unsigned int refined = 0;
 if (canMineMinerals()) {
	refined = mRefinery->refineMinerals(amount);
	player()->statistics()->increaseRefinedMinerals(refined);
 } else if (canMineOil()) {
	refined = mRefinery->refineOil(amount);
	player()->statistics()->increaseRefinedOil(refined);
 }
 if (mResourcesMined < refined) {
	boError(430) << k_funcinfo << "oops - more processed than available!" << endl;
	refined = mResourcesMined;
 }
 mResourcesMined = mResourcesMined - refined;
 boDebug(430) << k_funcinfo << "resources left: " << resourcesMined() << endl;
}

ResourceMinePlugin* HarvesterPlugin::findClosestResourceMine() const
{
 BO_CHECK_NULL_RET0(game());
 BO_CHECK_NULL_RET0(player());
 BO_CHECK_NULL_RET0(player()->playerIO());

 // FIXME: we should use player()->playerIO()->getUnitsOfPlayer(128)
 // instead
 // --> that should give us all units that are visible to us only
 Player* neutral = (Player*)game()->findPlayerByUserId(256);

 BO_CHECK_NULL_RET0(neutral);
 QPtrListIterator<Unit> it(*(neutral->allUnits()));
 ResourceMinePlugin* mine = 0;
 bofixed mineDist = 0.0f;
 while (it.current()) {
	Unit* u = it.current();
	if (!(u->visibleStatus(player()->bosonId()) & (UnitBase::VS_Visible | UnitBase::VS_Earlier))) {
		++it;
		continue;
	}
	ResourceMinePlugin* m = (ResourceMinePlugin*)u->plugin(UnitPlugin::ResourceMine);
	if (!m) {
		++it;
		continue;
	}
	if (m->isUsableTo(this)) {
		bofixed dist = QMAX(QABS(unit()->x() - u->x()), QABS(unit()->y() - u->y()));
		if ((dist < mineDist) || (mineDist == 0.0f)) {
			mineDist = dist;
			mine = m;
		}
	}
	++it;
 }
 return mine;
}

RefineryPlugin* HarvesterPlugin::findClosestRefinery() const
{
 BO_CHECK_NULL_RET0(player());
 QPtrListIterator<Unit> it(*(player()->allUnits()));
 const HarvesterProperties* prop = (HarvesterProperties*)properties(PluginProperties::Harvester);
 if (!prop) {
	boError() << k_funcinfo << "NULL harvester plugin" << endl;
	return 0;
 }
 RefineryPlugin* ref = 0;
 bofixed refdist = 0.0f;
 while (it.current()) {
	RefineryPlugin* r = (RefineryPlugin*)it.current()->plugin(UnitPlugin::Refinery);
	if (!r) {
		++it;
		continue;
	}
	if (r->isUsableTo(this)) {
		bofixed dist = QMAX(QABS(unit()->x() - it.current()->x()), QABS(unit()->y() - it.current()->y()));
		if ((dist < refdist) || (refdist == 0.0f)) {
			refdist = dist;
			ref = r;
		}
	}
	++it;
 }
 return ref;
}

void HarvesterPlugin::mineAt(ResourceMinePlugin* resource)
{
 //TODO: don't move if unit cannot mine more minerals/oil or no minerals/oil at all
 BO_CHECK_NULL_RET(resource);
 BO_CHECK_NULL_RET(resource->unit());
 BO_CHECK_NULL_RET(unit());
 boDebug() << k_funcinfo << resource->unit()->id() << endl;
 if (resource->unit()->isDestroyed()) {
	boDebug() << k_funcinfo << "sorry, resource mine is already destroyed. cannot use it" << endl;
	return;
 }
 if (!resource->isUsableTo(this)) {
	boError() << k_funcinfo << resource->unit()->id() << " not a suitable resource mine" << endl;
	return;
 }
 if (!unit()->addCurrentSuborder(new UnitMoveToUnitOrder(resource->unit()))) {
	boDebug() << k_funcinfo << "cannot find a way to resource mine" << endl;
	boDebug() << k_funcinfo << "TODO: search another resource mine" << endl;
	unit()->currentSuborderDone(false);
	return;
 }
 mResourceMine = resource;

 mHarvestingType = 1;
}


void HarvesterPlugin::refineAt(RefineryPlugin* refinery)
{
 BO_CHECK_NULL_RET(refinery);
 BO_CHECK_NULL_RET(refinery->unit());
 BO_CHECK_NULL_RET(unit());
 boDebug() << k_funcinfo << refinery->unit()->id() << endl;
 if (!refinery->isUsableTo(this)) {
	boError() << k_funcinfo << refinery->unit()->id() << " not a suitable refinery" << endl;
	return;
 }
 if (refinery->unit()->isDestroyed()) {
	boDebug() << k_funcinfo << "sorry, refinery is already destroyed. cannot use it" << endl;
	return;
 }
 if (!unit()->addCurrentSuborder(new UnitMoveToUnitOrder(refinery->unit()))) {
	boDebug() << k_funcinfo << "cannot find a way to refinery" << endl;
	boDebug() << k_funcinfo << "TODO: search another refinery" << endl;
	unit()->currentSuborderDone(false);
	return;
 }
 mRefinery = refinery;

 mHarvestingType = 2; // refining
}


bool HarvesterPlugin::canMineMinerals() const
{
 const HarvesterProperties* prop = (HarvesterProperties*)unit()->properties(PluginProperties::Harvester);
 if (!prop) {
	BO_NULL_ERROR(prop);
	return false;
 }
 return prop->canMineMinerals();
}

bool HarvesterPlugin::canMineOil() const
{
 const HarvesterProperties* prop = (HarvesterProperties*)unit()->properties(PluginProperties::Harvester);
 if (!prop) {
	BO_NULL_ERROR(prop);
	return false;
 }
 return prop->canMineOil();
}

unsigned int HarvesterPlugin::maxResources() const
{
 const HarvesterProperties* prop = (HarvesterProperties*)unit()->properties(PluginProperties::Harvester);
 if (!prop) {
	BO_NULL_ERROR(prop);
	return 0;
 }
 return prop->maxResources();
}

unsigned int HarvesterPlugin::miningSpeed() const
{
 const HarvesterProperties* prop = (HarvesterProperties*)unit()->properties(PluginProperties::Harvester);
 if (!prop) {
	BO_NULL_ERROR(prop);
	return 0;
 }
 return prop->miningSpeed();
}

unsigned int HarvesterPlugin::unloadingSpeed() const
{
 const HarvesterProperties* prop = (HarvesterProperties*)unit()->properties(PluginProperties::Harvester);
 if (!prop) {
	BO_NULL_ERROR(prop);
	return 0;
 }
 return prop->unloadingSpeed();
}

void HarvesterPlugin::unitDestroyed(Unit* u)
{
 RefineryPlugin* r = (RefineryPlugin*)u->plugin(UnitPlugin::Refinery);
 if (mRefinery == r) {
	mRefinery = 0;
 }
 ResourceMinePlugin* m = (ResourceMinePlugin*)u->plugin(UnitPlugin::ResourceMine);
 if (mResourceMine == m) {
	mResourceMine = 0;
 }
}

void HarvesterPlugin::itemRemoved(BosonItem* item)
{
 if (!item) {
	return;
 }
 if (!RTTI::isUnit(item->rtti())) {
	return;
 }
 Unit* u = (Unit*)item;
 unitDestroyed(u);
}


