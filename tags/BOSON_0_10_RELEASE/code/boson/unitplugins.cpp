/*
    This file is part of the Boson game
    Copyright (C) 2002-2004 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include "unitplugins.h"

#include "defines.h"
#include "unit.h"
#include "unitproperties.h"
#include "pluginproperties.h"
#include "speciestheme.h"
#include "player.h"
#include "playerio.h"
#include "bosoncanvas.h"
#include "boson.h"
#include "boitemlist.h"
#include "bosonstatistics.h"
#include "cell.h"
#include "upgradeproperties.h"
#include "bodebug.h"
#include "bosonweapon.h"
#include "bosonpath.h"

#include <klocale.h>

#include <qpair.h>
#include <qpoint.h>
#include <qdom.h>
#include <qstringlist.h>

#include <math.h>

UnitPlugin::UnitPlugin(Unit* unit)
{
 mUnit = unit;
}

UnitPlugin::~UnitPlugin()
{
}

SpeciesTheme* UnitPlugin::speciesTheme() const
{
 return unit()->speciesTheme();
}

Player* UnitPlugin::player() const
{
 return unit()->owner();
}

const UnitProperties* UnitPlugin::unitProperties() const
{
 return unit()->unitProperties();
}

const PluginProperties* UnitPlugin::properties(int propertyType) const
{
 return unit()->properties(propertyType);
}

KGamePropertyHandler* UnitPlugin::dataHandler() const
{
 return unit()->dataHandler();
}

BosonCanvas* UnitPlugin::canvas() const
{
 return unit()->canvas();
}

Boson* UnitPlugin::game() const
{
 if (!player()) {
	return 0;
 }
 return (Boson*)player()->game();
}


ProductionPlugin::ProductionPlugin(Unit* unit) : UnitPlugin(unit)
{
// unit->registerData(&mProductions, Unit::IdProductions);
 unit->registerData(&mProductionState, Unit::IdProductionState);
 mProductionState.setLocal(0);
// mProductions.setEmittingSignal(false); // just to prevent warning in Player::slotUnitPropertyChanged()
 mProductionState.setEmittingSignal(false); // called quite often - not emitting will increase speed a little bit
}

ProductionPlugin::~ProductionPlugin()
{
}

unsigned long int ProductionPlugin::completedProductionId() const
{
 if (!hasProduction()) {
	return 0;
 }
 unsigned long int id = currentProductionId();
 if (id == 0) {
	return 0;
 }
 if (completedProductionType() == ProduceUnit) {
	if (mProductionState < speciesTheme()->unitProperties(id)->productionTime()) {
//		boDebug() << k_funcinfo << "not yet completed: " << id << endl;
		return 0;
	}
 } else {
	if (mProductionState < speciesTheme()->technology(id)->productionTime()) {
//		boDebug() << k_funcinfo << "not yet completed: " << id << endl;
		return 0;
	}
 }
 return id;
}

ProductionType ProductionPlugin::completedProductionType() const
{
 return currentProductionType();
}

void ProductionPlugin::addProduction(ProductionType type, unsigned long int id)
{
 ProductionProperties* p = (ProductionProperties*)unit()->unitProperties()->properties(PluginProperties::Production);
 if (!p) {
	boError() << k_funcinfo << "NULL production properties" << endl;
	return;
 }
 if(type == ProduceUnit) {
	if (!speciesTheme()->productions(p->producerList()).contains(id)) {
		boError() << k_funcinfo << " cannot produce unit with id " << id << endl;
		return;
	}
 } else if(type == ProduceTech) {
	if (!speciesTheme()->technologies(p->producerList()).contains(id)) {
		boError() << k_funcinfo << " cannot produce technology with id " << id << endl;
		return;
	}
 }
 bool start = false;
 if (!hasProduction()) {
	start = true;
 }
 QPair<ProductionType, unsigned long int> pair;
 pair.first = type;
 pair.second = id;
 mProductions.append(pair);
 if (start) {
	unit()->setPluginWork(pluginType());
 }
}

void ProductionPlugin::removeProduction()
{
 mProductions.pop_front();
 mProductionState = 0; // start next production (if any)
}

void ProductionPlugin::removeProduction(ProductionType type, unsigned long int id)
{
 for (unsigned int i = 0; i < productionList().count(); i++) {
	if ((mProductions[i].first == type) && (mProductions[i].second == id)) {
		boDebug() << k_funcinfo << "remove; type: " << type << ", id: " << id << endl;
		mProductions.remove(mProductions.at(i));
		return;
	}
 }
}

double ProductionPlugin::productionProgress() const
{
 unsigned int productionTime = 0;
 if(currentProductionType() == ProduceUnit) {
	productionTime = speciesTheme()->unitProperties(currentProductionId())->productionTime();
 } else if(currentProductionType() == ProduceTech) {
	productionTime = speciesTheme()->technology(currentProductionId())->productionTime();
 } else {
	boDebug() << k_funcinfo << "Unknown productiontype: " << currentProductionType() << endl;
 }
 double percentage = (double)(mProductionState * 100) / (double)productionTime;
 return percentage;
}

void ProductionPlugin::advance(unsigned int)
{
 unsigned long int id = currentProductionId();
 if (id <= 0) { // no production
	unit()->setWork(Unit::WorkNone);
	mProductionState = 0;
	return;
 }



 // FIXME: this code is broken!
 // it gets executed on *every* client but sendInput() should be used on *one*
 // client only!
 // a unit is completed as soon as mProductionState == player()->unitProperties(type)->productionTime()
 unsigned int productionTime = 0;
 if(currentProductionType() == ProduceUnit) {
	productionTime = speciesTheme()->unitProperties(currentProductionId())->productionTime();
 } else if(currentProductionType() == ProduceTech) {
	productionTime = speciesTheme()->technology(currentProductionId())->productionTime();
 } else {
	boDebug() << k_funcinfo << "Unknown productiontype: " << currentProductionType() << endl;
 }
 if (mProductionState <= productionTime) {
	if (mProductionState == productionTime) {
		boDebug() << "Production with type " << currentProductionType() << " and id " << id << " completed :-)" << endl;
		mProductionState = mProductionState + 1;

		if(currentProductionType() != ProduceUnit) {
			// It's technology
			removeProduction();
			player()->technologyResearched(this, id);
			return;
		}

		// Auto-place unit
		// Unit positioning scheme: all tiles starting with tile that is below
		// facility's lower-left tile, are tested counter-clockwise. Unit is placed
		// to first free tile.
		// No auto-placing for facilities
		const UnitProperties* prop = speciesTheme()->unitProperties(id);
		if (!prop) {
			boError() << k_lineinfo << "Unknown id " << id << endl;
			return;
		}
		if (speciesTheme()->unitProperties(id)->isFacility()) {
			game()->slotAddChatSystemMessage(
					i18n("A %1 has been produced - place it on the map to start construction!").arg(prop->name()),
					player());
			return;
		} else {
			game()->slotAddChatSystemMessage(
					i18n("A %1 has been produced and will be placed on the map now").arg(prop->name()),
					player());
		}
		int tilex, tiley; // Position of lower-left corner of facility in tiles
		int theight, twidth; // height and width of facility in tiles
		int currentx, currenty; // Position of tile currently tested
		theight = unit()->height() / BO_TILE_SIZE;
		twidth = unit()-> width() / BO_TILE_SIZE;
		tilex = (int)(unit()->x() / BO_TILE_SIZE);
		tiley = (int)(unit()->y() / BO_TILE_SIZE + theight);
		int tries; // Tiles to try for free space
		int ctry; // Current try
		currentx = tilex - 1;
		currenty = tiley - 1;
		for (int i = 1; i <= BUILD_RANGE; i++) {
			tries = 2 * i * twidth + 2 * i * theight + 4;
			currenty++;
			for (ctry = 1; ctry <= tries; ctry++) {
				if (ctry <= twidth + i) {
					currentx++;
				} else if (ctry <= twidth + i + theight + 2 * i - 1) {
					currenty--;
				} else if (ctry <= twidth + i + 2 * (theight + 2 * i - 1)) {
					currentx--;
				} else if (ctry <= twidth + i + 3 * (theight + 2 * i - 1)) {
					currenty++;
				} else {
					currentx++;
				}

				if (canvas()->canPlaceUnitAtCell(speciesTheme()->unitProperties(id), QPoint(currentx, currenty), this)) {
					// Free cell - place unit at it
					mProductionState = mProductionState + 1;
					//FIXME: buildProduction should not
					//depend on Facility! should be Unit
					((Boson*)player()->game())->buildProducedUnit(this, id, currentx, currenty);
					return;
				}
			}
		}
		boDebug() << k_funcinfo << "Cannot find free cell around facility :-(" << endl;
		game()->slotAddChatSystemMessage(
				i18n("%1 could not be placed on the map - no free cell found. Place it manuall!").arg(prop->name()),
				player());
	} else {
		mProductionState = mProductionState + 1;
	}
 }
}

bool ProductionPlugin::saveAsXML(QDomElement& root) const
{
 if (mProductions.count() == 0) {
	return true;
 }

 // AB: we don't use a KGamePropertyList here, cause it is a bit more difficult
 // with the QPair I guess. simply saving this by hand.
 QStringList list;
 for (unsigned int i = 0; i < mProductions.count(); i++) {
	unsigned int type = (unsigned int)(mProductions[i].first);
	unsigned int id = mProductions[i].second;
	QString s = QString::fromLatin1("%1 %2").arg(type).arg(id);
	list.append(s);
 }
 QString productionString = list.join(QString::fromLatin1(","));
 QDomDocument doc = root.ownerDocument();
 QDomElement e = doc.createElement("ProductionList");
 root.appendChild(e);
 e.appendChild(doc.createTextNode(productionString));
 return true;
}

bool ProductionPlugin::loadFromXML(const QDomElement& root)
{
 mProductions.clear();
 QDomElement e = root.namedItem("ProductionList").toElement();
 if (e.isNull()) {
	// nothing to load here
	return true;
 }
 QString s = e.text();
 if (s.isEmpty()) {
	// list is here, but empty
	return true;
 }
 QStringList list = QStringList::split(QString::fromLatin1(","), s);
 for (unsigned int i = 0; i < list.count(); i++) {
	QStringList l = QStringList::split(" ", list[i]);
	if (l.count() != 2) {
		boWarning() << k_funcinfo << "type and id of a production item must be space separated" << endl;
		return false;
	}
	bool ok = false;
	unsigned int type = l[0].toUInt(&ok);
	if (!ok) {
		boError() << k_funcinfo << "type of production item " << i << " is not a valid number" << endl;
		return false;
	}
	unsigned int id = l[1].toUInt(&ok);
	if (!ok) {
		boError() << k_funcinfo << "id of production item " << i << " is not a valid number" << endl;
		return false;
	}
	QPair<ProductionType, unsigned long int> pair;
	pair.first = (ProductionType)type;
	pair.second = id;
	mProductions.append(pair);
 }
 return true;
}

bool ProductionPlugin::contains(ProductionType type, unsigned long int id)
{
 QPair<ProductionType, unsigned long int> pair;
 pair.first = type;
 pair.second = id;
 return productionList().contains(pair);
}

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
	if (!u->moveTo(unit()->x(), unit()->y(), 1)) {
		boDebug() << k_funcinfo << u->id() << " cannot find a way to repairyard" << endl;
		u->setWork(Unit::WorkNone);
	} else {
		u->setWork(Unit::WorkMove);
	}
 } else {
	if (!unit()->moveTo(u->x(), u->y(), 1)) {
		boDebug() << k_funcinfo << "Cannot find way to " << u->id() << endl;
		unit()->setWork(Unit::WorkNone);
	} else {
		unit()->setAdvanceWork(Unit::WorkMove);
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

void RepairPlugin::itemRemoved(BosonItem*)
{
}


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
	unit()->setWork(Unit::WorkNone);
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
 if (refineryId != 0) {
	// AB: retrieving from Boson is not 100% nice, but definitely necessary
	// and valid at this point. we need to get the pointer, even if the
	// refinery is an enemy or if it is invisible to us
	// --> it was saved this way, so we must load it this way.
	Unit* u = game()->findUnit(refineryId, 0);
	if (!u) {
		boError() << k_funcinfo << "cannot find refinery mine " << refineryId << endl;
	} else {
		mRefinery = (RefineryPlugin*)u->plugin(UnitPlugin::Refinery);
		if (!mRefinery) {
			boError() << k_funcinfo << "unit " << mineId << " is not a refinery" << endl;
			mRefinery = 0;
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

bool HarvesterPlugin::isNextTo(const Unit* u) const
{
 // warning: we measure center of _this_ unit to center of the other unit only.
 // this will cause problems for units that occupy multiple cells (e.g.
 // refineries)
 if (!u) {
	return false;
 }
 if (!unit()) {
	BO_NULL_ERROR(unit());
	return false;
 }

 float distx, disty;
 distx = fabsf((u->x() + u->width() / 2) - (unit()->x() + unit()->width() / 2));
 disty = fabsf((u->y() + u->height() / 2) - (unit()->y() + unit()->height() / 2));
 // We might get some precision trouble with floats, so we do this:
 distx = fmaxf(distx - 1, 0);
 disty = fmaxf(disty - 1, 0);

 float allowedx, allowedy;
 allowedx = unit()->width() / 2 + u->width() / 2;
 allowedy = unit()->height() / 2 + u->height() / 2;

 if (distx <= allowedx && disty <= allowedy) {
	return true;
 }
 return false;
}

void HarvesterPlugin::advanceMine()
{
 boDebug() << k_funcinfo << endl;
 const HarvesterProperties* prop = (HarvesterProperties*)properties(PluginProperties::Harvester);
 if (!prop) {
	boError() << k_funcinfo << "NULL harvester properties" << endl;
	unit()->setWork(Unit::WorkNone);
	return;
 }
 if (!mResourceMine) {
	ResourceMinePlugin* mine = findClosestResourceMine();
	if (!mine || !mine->unit()) {
		boDebug() << k_funcinfo << "no resource mine found" << endl;
		mHarvestingType = 0; // stop
	} else {
		boDebug() << k_funcinfo << "resource mine: " << mine->unit()->id() << endl;
		mineAt(mine);
	}
	return;
 }
 if (!mResourceMine || !mResourceMine->isUsableTo(this)) {
	// TODO: search a new resource mine
	QString mineId = "no id";
	if (mResourceMine && mResourceMine->unit()) {
		mineId = QString::number(mResourceMine->unit()->id());
	}
	boDebug() << k_funcinfo << "cannot mine at " << mResourceMine << " (" << mineId << ")" << endl;
	unit()->setWork(Unit::WorkNone);

	// TODO: handle special case when mine has become empty!
	// -> we should go to a refinery now
	return;
 }

 BO_CHECK_NULL_RET(mResourceMine);
 BO_CHECK_NULL_RET(mResourceMine->unit());
 if (mResourceMine->unit()->isDestroyed()) {
	boDebug() << k_funcinfo << "resource mine has been destroyed!" << endl;
	mResourceMine = 0; // search a new one
	return;
 }

 // Check if unit is at mining location. If not, go there
 if (!isAtResourceMine()) {
	Unit* u = mResourceMine->unit();
	unit()->moveTo(u->x(), u->y(), 1);
	unit()->setAdvanceWork(Unit::WorkMove);
	return;
 }
 if (resourcesMined() >= prop->maxResources()) {
	// Back to refinery
	boDebug() << k_funcinfo << "Maximal amount of resources mined." << endl;
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
	boError() << k_funcinfo << "oops - cannot mine here?!" << endl;
	mined = 0;
 }
 if (resourcesMined() + mined > prop->maxResources()) {
	// any additional resources are lost now, not even another harvester can
	// get them (intended).
	// cannot happen anyway.
	mined = prop->maxResources() - resourcesMined();
 }
 mResourcesMined = resourcesMined() + mined;
 boDebug() << k_funcinfo << "resources mined: " << resourcesMined() << endl;
}

void HarvesterPlugin::advanceRefine()
{
 // This is the second step of harvesting: returning to refinery and unloading
 boDebug() << k_funcinfo << endl;
 if (resourcesMined() == 0) {
	boDebug() << k_funcinfo << "refining done" << endl;
	mHarvestingType = 1; // mining
	return;
 }
 if (!mRefinery) {
	RefineryPlugin* refinery = findClosestRefinery();
	if (!refinery || !refinery->unit()) {
		boDebug() << k_funcinfo << "no refinery found" << endl;
		mHarvestingType = 0; // stop
	} else {
		boDebug() << k_funcinfo << "refinery: " << refinery->unit()->id() << endl;
		refineAt(refinery);
	}
	return;
 }

 BO_CHECK_NULL_RET(mRefinery);
 BO_CHECK_NULL_RET(mRefinery->unit());
 if (mRefinery->unit()->isDestroyed()) {
	boDebug() << k_funcinfo << "refinery has been destroyed!" << endl;
	mRefinery = 0; // search a new one
	return;
 }

 if (!isAtRefinery()) {
	Unit* u = mRefinery->unit();
	unit()->moveTo(u->x(), u->y(), 1);
	unit()->setAdvanceWork(Unit::WorkMove);
	return;
 }
 const HarvesterProperties* prop = (HarvesterProperties*)unit()->properties(PluginProperties::Harvester);
 if (!prop) {
	boError() << k_funcinfo << "NULL harvester plugin" << endl;
	mHarvestingType = 0; // stop
	return;
 }

 int amount = unloadingSpeed();
 if (amount > (int)resourcesMined()) {
	amount = resourcesMined();
 }
 if (amount < 0) {
	boError() << k_funcinfo << "a negative amount of resources to be refined??" << endl;
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
	boError() << k_funcinfo << "oops - more processed than available!" << endl;
	refined = mResourcesMined;
 }
 mResourcesMined = mResourcesMined - refined;
 boDebug() << k_funcinfo << "resources left: " << resourcesMined() << endl;
}

ResourceMinePlugin* HarvesterPlugin::findClosestResourceMine() const
{
 BO_CHECK_NULL_RET0(game());
 BO_CHECK_NULL_RET0(game()->playerList());
 BO_CHECK_NULL_RET0(player());
 BO_CHECK_NULL_RET0(player()->playerIO());

 // FIXME: we should use player()->playerIO()->getUnitsOf(playerCount() - 1)
 // instead
 // --> that should give us all units that are visible to us only
 Player* neutral = (Player*)game()->playerList()->getLast();

 BO_CHECK_NULL_RET0(neutral);
 QPtrListIterator<Unit> it(*(neutral->allUnits()));
 ResourceMinePlugin* mine = 0;
 float mineDist = 0.0f;
 while (it.current()) {
	Unit* u = it.current();
	if (!player()->playerIO()->canSee(u)) {
		++it;
		continue;
	}
	ResourceMinePlugin* m = (ResourceMinePlugin*)u->plugin(UnitPlugin::ResourceMine);
	if (!m) {
		++it;
		continue;
	}
	if (m->isUsableTo(this)) {
		float dist = QMAX(QABS(unit()->x() - u->x()), QABS(unit()->y() - u->y()));
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
 float refdist = 0.0f;
 while (it.current()) {
	RefineryPlugin* r = (RefineryPlugin*)it.current()->plugin(UnitPlugin::Refinery);
	if (!r) {
		++it;
		continue;
	}
	if (r->isUsableTo(this)) {
		float dist = QMAX(QABS(unit()->x() - it.current()->x()), QABS(unit()->y() - it.current()->y()));
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
 if (!unit()->moveTo(resource->unit()->x(), resource->unit()->y(), 1)) {
	boDebug() << k_funcinfo << "cannot find a way to resource mine" << endl;
	boDebug() << k_funcinfo << "TODO: search another resource mine" << endl;
	return;
 }
 unit()->setPluginWork(UnitPlugin::Harvester);
 unit()->setAdvanceWork(Unit::WorkMove);
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
 if (!unit()->moveTo(refinery->unit()->x(), refinery->unit()->y(), 1)) {
	boDebug() << k_funcinfo << "cannot find a way to refinery" << endl;
	boDebug() << k_funcinfo << "TODO: search another refinery" << endl;
	return;
 }
 unit()->setPluginWork(UnitPlugin::Harvester);
 unit()->setAdvanceWork(Unit::WorkMove);
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

void HarvesterPlugin::itemRemoved(BosonItem* item)
{
 if (!item) {
	return;
 }
 if (!RTTI::isUnit(item->rtti())) {
	return;
 }
 Unit* u = (Unit*)item;
 RefineryPlugin* r = (RefineryPlugin*)u->plugin(UnitPlugin::Refinery);
 if (mRefinery == r) {
	mRefinery = 0;
 }
 ResourceMinePlugin* m = (ResourceMinePlugin*)u->plugin(UnitPlugin::ResourceMine);
 if (mResourceMine == m) {
	mResourceMine = 0;
 }
}

BombingPlugin::BombingPlugin(Unit* owner) : UnitPlugin(owner)
{
 owner->registerData(&mPosX, Unit::IdBombingPosX);
 owner->registerData(&mPosY, Unit::IdBombingPosY);
 mWeapon = 0;
 mPosX = 0;
 mPosY = 0;
}

BombingPlugin::~BombingPlugin()
{
}

void BombingPlugin::bomb(int weaponId, float x, float y)
{
 boDebug() << k_funcinfo << "wep: " << weaponId << "; pos: (" << x << "; " << y << ")" << endl;
 BosonWeapon* w = unit()->weapon(weaponId);
 if (!w) {
	boError() << k_funcinfo << "No weapon with id " << weaponId << endl;
	return;
 }
 if (w->properties()->shotType() != BosonShot::Bomb) {
	boError() << k_funcinfo << "Weapon with id " << weaponId << " is not a bomb" << endl;
	return;
 }

 unit()->stopMoving();

 int cellX = (int)(x / BO_TILE_SIZE);
 int cellY = (int)(y / BO_TILE_SIZE);
 if (!canvas()->cell(cellX, cellY)) {
	boError() << k_funcinfo << x << "," << y << " is no valid cell!" << endl;
	return;
 }

 // This is where unit has to be when making the drop
 mPosX = cellX * BO_TILE_SIZE + BO_TILE_SIZE / 2;
 mPosY = cellY * BO_TILE_SIZE + BO_TILE_SIZE / 2;
 boDebug() << k_funcinfo << "Drop-point: (" << mPosX << "; " << mPosY << ")" << endl;
 mWeapon = w;

 unit()->setPluginWork(UnitPlugin::Bombing);
}

void BombingPlugin::advance(unsigned int)
{
 boDebug() << k_funcinfo << endl;

 // Check if we're at the drop point
 // Unit's center point
 float unitx = unit()->x() + unit()->width() / 2;
 float unity = unit()->y() + unit()->height() / 2;
 float dist = QMAX(QABS(unitx - mPosX), QABS(unity - mPosY));
 boDebug() << k_funcinfo << "dist: " << dist << endl;
 boDebug() << k_funcinfo << "my pos is: (" << unitx << "; " << unity << ");  drop-point is: (" << mPosX << "; " << mPosY << ")" << endl;
 if ((unitx != mPosX) || (unity != mPosY)) {
	boDebug() << k_funcinfo << "not at drop point - moving..." << endl;
	if (!unit()->moveTo(mPosX, mPosY, 0)) {
		boWarning() << k_funcinfo << "Moving failed. Now what?" << endl;
		unit()->setWork(Unit::WorkNone);
	} else {
		unit()->pathInfo()->slowDownAtDest = false;
		unit()->pathInfo()->moveAttacking = false;
		unit()->addWaypoint(QPoint((int)mPosX, (int)mPosY));
		unit()->setAdvanceWork(Unit::WorkMove);
	}
	return;
 }

 // We're at drop point. Drop the bomb
 boDebug() << k_funcinfo << "At drop-point!" << endl;
 if (mWeapon->reloaded()) {
	boDebug() << k_funcinfo << "Bomb ready. Dropping..." << endl;
	mWeapon->dropBomb();
	// And get the hell out of there
	// Go away from bomb's explosion radius
	float dist = mWeapon->properties()->damageRange() * BO_TILE_SIZE + unit()->width() / 2;
  boDebug() << k_funcinfo << "Getaway dist: " << dist << "; rot: " << unit()->rotation() << endl;
	float newx = unitx;
	float newy = unity;
	// TODO: quite messy code, maybe it can be cleaned up somehow
	int rot = (int)unit()->rotation() % 360;
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
	newx = QMAX(unit()->width() / 2, QMIN(newx, (canvas()->mapWidth() - 1) * BO_TILE_SIZE - unit()->width() / 2));
	newy = QMAX(unit()->height() / 2, QMIN(newy, (canvas()->mapHeight() - 1) * BO_TILE_SIZE - unit()->height() / 2));

  boDebug() << k_funcinfo << "Getaway point is at (" << newx << "; " << newy << ")" << endl;
	if (!unit()->moveTo(newx, newy)) {
		boWarning() << k_funcinfo << "Aargh! Can't move away from drop-point!" << endl;
		unit()->setWork(Unit::WorkNone);
	} else {
		unit()->pathInfo()->moveAttacking = false;
		unit()->addWaypoint(QPoint((int)newx, (int)newy));
		unit()->setWork(Unit::WorkMove);  // We don't want to return here anymore
	}
	mWeapon = 0;
	boDebug() << k_funcinfo << "returning" << endl;
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

 unit()->stopMoving();

 mWeapon = w;
 mPlacingCounter = MINE_PLACE_TIME;

 // Mine will be layed in advance()

 unit()->setPluginWork(UnitPlugin::Mining);
}

void MiningPlugin::advance(unsigned int)
{
 boDebug() << k_funcinfo << endl;

 // Lay the mine
 if (mWeapon->reloaded()) {
	// Don't place the mine immediately
	if (mPlacingCounter > 0) {
		boDebug() << k_funcinfo << "mPlacingCounter: " << mPlacingCounter << endl;
		mPlacingCounter = mPlacingCounter - 1;
		return;
	}

	boDebug() << k_funcinfo << "Mine ready. Placing..." << endl;
	mWeapon->layMine();

	// Go one cell away from the mine. Maybe go away from explosion radius?
	// FIXME: code taken from BombingPlugin. This could probably be written better
	float dist = 1 * BO_TILE_SIZE + unit()->width() / 2;
	boDebug() << k_funcinfo << "Getaway dist: " << dist << "; rot: " << unit()->rotation() << endl;
	float oldx = unit()->x();
	float oldy = unit()->y();
	float newx, newy;
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
		newx = QMAX(0, QMIN(newx, (canvas()->mapWidth() - 1) * BO_TILE_SIZE));
		newy = QMAX(0, QMIN(newy, (canvas()->mapHeight() - 1) * BO_TILE_SIZE));

		boDebug() << k_funcinfo << "i: " << i << "; Getaway point is at (" << newx << "; " << newy << ")" << endl;
		if (unit()->moveTo(newx, newy)) {
			unit()->setWork(Unit::WorkMove);  // We don't want to return here anymore
			couldmove = true;
			break;
		}
	}

	if (!couldmove) {
		boDebug() << k_funcinfo << "Can't move away!" << endl;
		unit()->setWork(Unit::WorkNone);  // We don't want to return here anymore
	}

	mWeapon = 0;
	boDebug() << k_funcinfo << "returning" << endl;
	return;
 } else {
	boDebug() << k_funcinfo << "Weapon not yet reloaded" << endl;
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

ResourceMinePlugin::ResourceMinePlugin(Unit* unit)
		: UnitPlugin(unit)
{
 unit->registerData(&mMinerals, Unit::IdResourceMineMinerals);
 unit->registerData(&mOil, Unit::IdResourceMineOil);
 mMinerals.setLocal(0);
 mOil.setLocal(0);
}

ResourceMinePlugin::~ResourceMinePlugin()
{
}

bool ResourceMinePlugin::saveAsXML(QDomElement& root) const
{
 // everything is saved using KGameProperty
 Q_UNUSED(root);
 return true;
}

bool ResourceMinePlugin::loadFromXML(const QDomElement& root)
{
 // everything is already loaded using KGameProperty - we do safety checks here
 ResourceMineProperties* prop = (ResourceMineProperties*)properties(PluginProperties::ResourceMine);
 if (!prop) {
	BO_NULL_ERROR(prop);
	return false;
 }
 if (!prop->canProvideMinerals()) {
	if (mMinerals != 0) {
		boWarning() << k_funcinfo << "unit can't have minerals, but minerals not 0" << endl;
	}
	mMinerals = 0;
 }
 if (!prop->canProvideOil()) {
	if (mOil != 0) {
		boWarning() << k_funcinfo << "unit can't have oil, but oil not 0" << endl;
	}
	mOil = 0;
 }
 return true;
}

void ResourceMinePlugin::advance(unsigned int)
{
}

bool ResourceMinePlugin::canProvideMinerals() const
{
 const ResourceMineProperties* prop = (ResourceMineProperties*)unit()->properties(PluginProperties::ResourceMine);
 if (!prop) {
	return false;
 }
 return prop->canProvideMinerals();
}

bool ResourceMinePlugin::canProvideOil() const
{
 const ResourceMineProperties* prop = (ResourceMineProperties*)unit()->properties(PluginProperties::ResourceMine);
 if (!prop) {
	return false;
 }
 return prop->canProvideOil();
}

bool ResourceMinePlugin::isUsableTo(const HarvesterPlugin* harvester) const
{
 if (!harvester) {
	return false;
 }
 if (harvester->canMineMinerals() && canProvideMinerals() && minerals() != 0) {
	return true;
 }
 if (harvester->canMineOil() && canProvideOil() && oil() != 0) {
	return true;
 }
 return false;
}

unsigned int ResourceMinePlugin::mineMinerals(const HarvesterPlugin* harvester)
{
 if (!harvester) {
	BO_NULL_ERROR(harvester);
	return 0;
 }
 if (!canProvideMinerals()) {
	boDebug() << k_funcinfo << "no minerals available. sorry." << endl;
	return 0;
 }
 if (!isUsableTo(harvester)) {
	boDebug() << k_funcinfo << "harvester cannot mine here" << endl;
	return 0;
 }

 int amount = mineStep(harvester, minerals());
 if (minerals() >= 0) {
	setMinerals(minerals() - amount);
 }
 return amount;
}

unsigned int ResourceMinePlugin::mineOil(const HarvesterPlugin* harvester)
{
 if (!harvester) {
	BO_NULL_ERROR(harvester);
	return 0;
 }
 if (!canProvideOil()) {
	boDebug() << k_funcinfo << "no oil available. sorry." << endl;
	return 0;
 }
 if (!isUsableTo(harvester)) {
	boDebug() << k_funcinfo << "harvester cannot mine here" << endl;
	return 0;
 }

 int amount = mineStep(harvester, oil());
 if (oil() >= 0) {
	setOil(oil() - amount);
 }
 return amount;
}

unsigned int ResourceMinePlugin::mineStep(const HarvesterPlugin* harvester, int resourcesAvailable) const
{
 if (!harvester) {
	BO_NULL_ERROR(harvester);
	return 0;
 }
 int maxCapacity = QMAX((int)harvester->maxResources() - (int)harvester->resourcesMined(), 0);
 int maxAvailable = resourcesAvailable;
 if (resourcesAvailable < 0) {
	// infinite resources
	maxAvailable = maxCapacity;
 }
 int step = harvester->miningSpeed();
 if (step > maxAvailable) {
	step = maxAvailable;
 }
 if (step > maxCapacity) {
	step = maxCapacity;
 }
 return step;
}

void ResourceMinePlugin::setMinerals(int m)
{
 if (!canProvideMinerals()) {
	mMinerals = 0;
	return;
 }
 mMinerals = m;
}

void ResourceMinePlugin::setOil(int o)
{
 if (!canProvideOil()) {
	mOil = 0;
	return;
 }
 mOil = o;
}

int ResourceMinePlugin::minerals() const
{
 return mMinerals;
}

int ResourceMinePlugin::oil() const
{
 return mOil;
}

void ResourceMinePlugin::itemRemoved(BosonItem*)
{
}

RefineryPlugin::RefineryPlugin(Unit* owner)
		: UnitPlugin(owner)
{
}
RefineryPlugin::~RefineryPlugin()
{
}

bool RefineryPlugin::saveAsXML(QDomElement& root) const
{
 Q_UNUSED(root);
 return true;
}

bool RefineryPlugin::loadFromXML(const QDomElement& root)
{
 Q_UNUSED(root);
 return true;
}

void RefineryPlugin::advance(unsigned int)
{
}

bool RefineryPlugin::canRefineMinerals() const
{
 const RefineryProperties * prop = (RefineryProperties*)unit()->properties(PluginProperties::Refinery);
 if (!prop) {
	return false;
 }
 return prop->canRefineMinerals();
}

bool RefineryPlugin::canRefineOil() const
{
 const RefineryProperties * prop = (RefineryProperties*)unit()->properties(PluginProperties::Refinery);
 if (!prop) {
	return false;
 }
 return prop->canRefineOil();
}

void RefineryPlugin::itemRemoved(BosonItem*)
{
}

bool RefineryPlugin::isUsableTo(const HarvesterPlugin* harvester) const
{
 if (!harvester) {
	return false;
 }
 if (canRefineMinerals() && harvester->canMineMinerals()) {
	return true;
 }
 if (canRefineOil() && harvester->canMineOil()) {
	return true;
 }
 return false;
}

unsigned int RefineryPlugin::refineMinerals(unsigned int minerals)
{
 if (!player()) {
	BO_NULL_ERROR(player());
	return 0;
 }
 player()->setMinerals(player()->minerals() + minerals);
 return minerals;
}

unsigned int RefineryPlugin::refineOil(unsigned int oil)
{
 if (!player()) {
	BO_NULL_ERROR(player());
	return 0;
 }
 player()->setOil(player()->oil() + oil);
 return oil;
}
