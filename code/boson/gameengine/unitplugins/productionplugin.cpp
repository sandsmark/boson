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
#include "productionplugin.h"

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
#include "bosonstatistics.h"
#include "upgradeproperties.h"
#include "bodebug.h"
#include "boevent.h"
#include "../bo3dtools.h"

#include <klocale.h>

#include <qdom.h>
//Added by qt3to4:
#include <Q3CString>
#include <Q3ValueList>

#include <math.h>

ProductionPlugin::ProductionPlugin(Unit* unit) : UnitPlugin(unit)
{
// unit->registerData(&mProductions, Unit::IdProductions);
 unit->registerData(&mProductionState, Unit::IdProductionState);
 unit->registerData(&mMineralsPaid, Unit::IdMineralsPaid);
 unit->registerData(&mOilPaid, Unit::IdOilPaid);
 mProductionState.setLocal(0);
 mMineralsPaid.setLocal(0);
 mOilPaid.setLocal(0);
// mProductions.setEmittingSignal(false); // just to prevent warning in Player::slotUnitPropertyChanged()
 mProductionState.setEmittingSignal(false); // called quite often - not emitting will increase speed a little bit
 mMineralsPaid.setEmittingSignal(false);
 mOilPaid.setEmittingSignal(false);
}

ProductionPlugin::~ProductionPlugin()
{
}

quint32 ProductionPlugin::completedProductionId() const
{
 if (!hasProduction()) {
	return 0;
 }
 quint32 id = currentProductionId();
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

void ProductionPlugin::productionPlaced(Unit* produced)
{
 BO_CHECK_NULL_RET(produced);
 if (produced->isFacility()) {
	player()->statistics()->addProducedFacility(produced, this);
 } else {
	player()->statistics()->addProducedMobileUnit(produced, this);
 }

 BoEvent* productionPlaced = new BoEvent("ProducedUnitWithTypePlaced", QString::number(produced->type()), QString::number(unit()->id()));
 productionPlaced->setUnitId(produced->id());
 productionPlaced->setPlayerId(produced->owner()->bosonId());
 productionPlaced->setLocation(BoVector3Fixed(produced->centerX(), produced->centerY(), produced->z()));
 boGame->queueEvent(productionPlaced);

 // the current production is done.
 mMineralsPaid = 0;
 mOilPaid = 0;
 removeCompletedProduction();
}

void ProductionPlugin::addProduction(ProductionType type, quint32 id)
{
 BO_CHECK_NULL_RET(unit());
 BO_CHECK_NULL_RET(unitProperties());
 BO_CHECK_NULL_RET(player());
 BO_CHECK_NULL_RET(speciesTheme());
 BO_CHECK_NULL_RET(game());
 ProductionProperties* p = (ProductionProperties*)unitProperties()->properties(PluginProperties::Production);
 if (!p) {
	boError() << k_funcinfo << "NULL production properties" << endl;
	return;
 }

 if (type == ProduceUnit) {
	const UnitProperties* prop = speciesTheme()->unitProperties(id);
	if (!prop) {
		boError() << k_lineinfo << "NULL unit properties (EVIL BUG)" << endl;
		return;
	}
	if (!canCurrentlyProduceUnit(id)) {
		boError() << k_funcinfo << " cannot produce unit witht type " << id << endl;
		game()->slotAddChatSystemMessage(i18n("Cannot produce unit %1", prop->name()), player());
		return;
	}
 } else if (type == ProduceTech) {
	const UpgradeProperties* prop = speciesTheme()->technology(id);
	if (!prop) {
		boError() << k_lineinfo << "NULL technology properties (EVIL BUG)" << endl;
		return;
	}
	if (!canCurrentlyProduceTechnology(id)) {
		boError() << k_funcinfo << " cannot produce technology with id " << id << endl;
		game()->slotAddChatSystemMessage(i18n("Cannot produce technology %1", prop->upgradeName()), player());
		return;
	}
 } else {
	boError() << k_funcinfo << "Invalid productionType: " << (int)type << endl;
	return;
 }


 bool start = false;
 if (!hasProduction()) {
	start = true;
 }
 QPair<ProductionType, quint32> pair;
 pair.first = type;
 pair.second = id;
 mProductions.append(pair);
 if (start) {
	unit()->setPluginWork(pluginType());
 }

 Q3CString eventName;
 if (type == ProduceUnit) {
	eventName = "StartProductionOfUnitWithType";
 } else if (type == ProduceTech) {
	eventName = "StartProductionOfTechnologyWithType";
 } else {
	boError() << k_funcinfo << "Invalid productionType: " << (int)type << endl;
 }
 if (!eventName.isNull()) {
	BoEvent* event = new BoEvent(eventName, QString::number(id), QString::number(unit()->id()));
	event->setPlayerId(player()->bosonId());
	event->setLocation(BoVector3Fixed(unit()->centerX(), unit()->centerY(), unit()->z()));
	game()->queueEvent(event);
 }
}

void ProductionPlugin::pauseProduction()
{
 if (unit()->currentPluginType() != UnitPlugin::Production) {
	// already paused
	return;
 }
 if (currentProductionId() == 0) {
	// no production
	boWarning() << k_funcinfo << "no current production" << endl;
	return;
 }

 unit()->setAdvanceWork(Unit::WorkIdle);

 Q3CString eventName;
 if (currentProductionType() == ProduceUnit) {
	eventName = "PauseProductionOfUnitWithType";
 } else if (currentProductionType() == ProduceTech) {
	eventName = "PauseProductionOfTechnologyWithType";
 } else {
	boError() << k_funcinfo << "Invalid productionType: " << (int)currentProductionType() << endl;
 }

 if (!eventName.isNull()) {
	BoEvent* event = new BoEvent(eventName, QString::number(currentProductionId()), QString::number(unit()->id()));
	event->setPlayerId(player()->bosonId());
	event->setLocation(BoVector3Fixed(unit()->centerX(), unit()->centerY(), unit()->z()));
	game()->queueEvent(event);
 }
}

void ProductionPlugin::unpauseProduction()
{
 if (unit()->currentPluginType() == UnitPlugin::Production) {
	// not paused
	return;
 }
 if (currentProductionId() == 0) {
	// no production
	boWarning() << k_funcinfo << "no current production" << endl;
	return;
 }

 unit()->setPluginWork(UnitPlugin::Production);


 Q3CString eventName;
 if (currentProductionType() == ProduceUnit) {
	eventName = "ContinueProductionOfUnitWithType";
 } else if (currentProductionType() == ProduceTech) {
	eventName = "ContinueProductionOfTechnologyWithType";
 } else {
	boError() << k_funcinfo << "Invalid productionType: " << (int)currentProductionType() << endl;
 }
 if (!eventName.isNull()) {
	BoEvent* event = new BoEvent(eventName, QString::number(currentProductionId()), QString::number(unit()->id()));
	event->setPlayerId(player()->bosonId());
	event->setLocation(BoVector3Fixed(unit()->centerX(), unit()->centerY(), unit()->z()));
	game()->queueEvent(event);
 }
}

void ProductionPlugin::abortProduction(ProductionType type, quint32 id)
{
 if (!hasProduction()) {
	// no production
	boWarning() << k_funcinfo << "no productions" << endl;
	return;
 }

 QString eventTypeParameter = QString::number(id);
 Q3CString eventName;
 if (type == ProduceUnit) {
	eventName = "StopProductionOfUnitWithType";
 } else if (type == ProduceTech) {
	eventName = "StopProductionOfTechnologyWithType";
 } else {
	boError() << k_funcinfo << "Invalid productionType: " << (int)type << endl;
 }

 if (!removeProduction(type, id)) {
	boError() << k_funcinfo << "no such production available" << endl;
	return;
 }

 if (!eventName.isNull()) {
	BoEvent* event = new BoEvent(eventName, QString::number(id), QString::number(unit()->id()));
	event->setPlayerId(player()->bosonId());
	event->setLocation(BoVector3Fixed(unit()->centerX(), unit()->centerY(), unit()->z()));
	game()->queueEvent(event);
 }
}

void ProductionPlugin::removeCompletedProduction()
{
 mMineralsPaid = 0;
 mOilPaid = 0;
 removeProduction();
}

bool ProductionPlugin::removeProduction()
{
 if (!hasProduction()) {
	return false;
 }
 return removeProduction(currentProductionType(), currentProductionId());
}

bool ProductionPlugin::removeProduction(ProductionType type, quint32 id)
{
 for (int i = 0; i < productionList().count(); i++) {
	if ((mProductions[i].first == type) && (mProductions[i].second == id)) {
		boDebug() << k_funcinfo << "remove; type: " << type << ", id: " << id << endl;
		mProductions.remove(mProductions.at(i));
		// Return resources to the player
		player()->setMinerals(player()->minerals() + mMineralsPaid);
		player()->setOil(player()->oil() + mOilPaid);
		mMineralsPaid = 0;
		mOilPaid = 0;
		if (i == 0) {
			mProductionState = 0; // start next production (if any)
		}
		return true;
	}
 }
 return false;
}

double ProductionPlugin::productionProgress() const
{
 unsigned int productionTime = 0;
 if (currentProductionType() == ProduceUnit) {
	productionTime = speciesTheme()->unitProperties(currentProductionId())->productionTime();
 } else if (currentProductionType() == ProduceTech) {
	productionTime = speciesTheme()->technology(currentProductionId())->productionTime();
 } else if (currentProductionType() == ProduceNothing) {
	return 0.0;
 } else {
	boDebug() << k_funcinfo << "Unknown productiontype: " << currentProductionType() << endl;
 }
 if (productionTime == 0) {
	boError() << k_funcinfo << "productionTime == 0 is invalid" << endl;
	return 0.0;
 }
 double percentage = (double)(mProductionState * 100) / (double)productionTime;
 return percentage;
}

void ProductionPlugin::advance(unsigned int)
{
 quint32 id = currentProductionId();
 if (id <= 0) { // no production
	unit()->setAdvanceWork(Unit::WorkIdle);
	mProductionState = 0;
	mMineralsPaid = 0;
	mOilPaid = 0;
	return;
 }

 if (!unit()->requestPowerChargeForAdvance()) {
	return;
 }



 // a unit is completed as soon as mProductionState == player()->unitProperties(type)->productionTime()
 unsigned int productionTime = 0;
 if (currentProductionType() == ProduceUnit) {
	productionTime = speciesTheme()->unitProperties(currentProductionId())->productionTime();
 } else if (currentProductionType() == ProduceTech) {
	productionTime = speciesTheme()->technology(currentProductionId())->productionTime();
 } else {
	boDebug() << k_funcinfo << "Unknown productiontype: " << currentProductionType() << endl;
 }
 if (mProductionState <= productionTime) {
	if (mProductionState == productionTime) {
		productionCompleted();
		return;
	} else {
		// Make sure the player has enough resources
		quint32 mineralCost = 0, oilCost = 0;
		if (currentProductionType() == ProduceUnit) {
			mineralCost = speciesTheme()->unitProperties(currentProductionId())->mineralCost();
			oilCost = speciesTheme()->unitProperties(currentProductionId())->oilCost();
		} else if (currentProductionType() == ProduceTech) {
			mineralCost = speciesTheme()->technology(currentProductionId())->mineralCost();
			oilCost = speciesTheme()->technology(currentProductionId())->oilCost();
		}
		float factor = (mProductionState + 1) / (float)productionTime;
		if (mProductionState + 1 == productionTime) {
			// Work around possible precision issues
			factor = 1.0f;
		}
		// How much we have to pay this advance call?
		quint32 payMinerals = (quint32)(factor * mineralCost) - mMineralsPaid;
		quint32 payOil = (quint32)(factor * oilCost) - mOilPaid;
		if (player()->useResources(payMinerals, payOil)) {
			mProductionState = mProductionState + 1;
			mMineralsPaid = mMineralsPaid + payMinerals;
			mOilPaid = mOilPaid + payOil;
		} else {
			// TODO: maybe show a warning in the chat?
		}
	}
 }
}

void ProductionPlugin::productionCompleted()
{
 quint32 id = currentProductionId();
 if (id <= 0) {
	boError() << k_funcinfo << "invalid id " << id << endl;
	return;
 }
 boDebug() << "Production with type " << currentProductionType() << " and id " << id << " completed :-)" << endl;
 mProductionState = mProductionState + 1;

 if (currentProductionType() == ProduceTech) {
	// these can be handled immediately.
	removeCompletedProduction();
	player()->technologyResearched(this, id);
	return;
 } else if (currentProductionType() != ProduceUnit) {
	boError() << k_funcinfo << "productiontype is neither technology nor unit .. cannot handle this" << endl;
	removeProduction(); // will re-fund minerals/oil
	return;
 }

 // A unit must be placed on the map, before removeProduction()
 // may be called.

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

 BoEvent* unitProduced = new BoEvent("UnitWithTypeProduced", QString::number(id),
		QString::number(unit()->id()));
 unitProduced->setPlayerId(player()->bosonId());
 game()->queueEvent(unitProduced);

 if (prop->isFacility()) {
	return;
 }

 boDebug() << k_funcinfo << "auto-placing unit "<< prop->typeId() << endl;

 int tilex, tiley; // Position of lower-left corner of facility in tiles
 int theight, twidth; // height and width of facility in tiles
 int currentx, currenty; // Position of tile currently tested

#warning converting to ints! -> we should use float here
 theight = unit()->height();
 twidth = unit()-> width();
 tilex = (int)(unit()->leftEdge());
 tiley = (int)(unit()->topEdge() + theight);
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

		if (canvas()->canPlaceUnitAtTopLeftPos(speciesTheme()->unitProperties(id), BoVector2Fixed(currentx, currenty), this)) {
			// Free cell - place unit at it
			mProductionState = mProductionState + 1;
			if (!((Boson*)player()->game())->buildProducedUnitAtTopLeftPos(this, id, BoVector2Fixed(currentx, currenty))) {
				boError() << k_funcinfo << "OOPS - produced unit has NOT been placed! production is lost!" << endl;
				// FIXME: try again later? at a different
				// position?
				// -> should never happen!
			}
			return;
		}
	}
 }
 boDebug() << k_funcinfo << "Cannot find free cell around facility :-(" << endl;
 game()->slotAddChatSystemMessage(
		i18n("%1 could not be placed on the map - no free cell found. Place it manuall!", prop->name()),
		player());

}

void ProductionPlugin::unitDestroyed(Unit* destroyedUnit)
{
 Q3ValueList< QPair<ProductionType, quint32> > abort;
 Q3ValueList<quint32> abortTechs;
 for (int i = 0; i < mProductions.count(); i++) {
	quint32 id = mProductions[i].second;
	if (mProductions[i].first == ProduceUnit) {
		if (!canCurrentlyProduceUnit(id)) {
			QPair<ProductionType, quint32> pair;
			pair.first = ProduceUnit;
			pair.second = id;
			abort.append(pair);
		}
	} else if (mProductions[i].first == ProduceTech) {
		if (!canCurrentlyProduceTechnology(id)) {
			QPair<ProductionType, quint32> pair;
			pair.first = ProduceTech;
			pair.second = id;
			abort.append(pair);
		}
	} else {
		boError() << k_funcinfo << "production " << i << " has unexpected type" << endl;
		continue;
	}
 }

 for (int i = 0; i < abort.count(); i++) {
	abortProduction(abort[i].first, abort[i].second);
 }
 if (abort.count() > 0) {
	game()->slotAddChatSystemMessage(i18n("Production in %1 got aborted because %2 was required but destroyed", unit()->name(), destroyedUnit->name()), player());
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
 for (int i = 0; i < mProductions.count(); i++) {
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
 QStringList list = s.split(QString::fromLatin1(","));
 for (int i = 0; i < list.count(); i++) {
	QStringList l = list[i].split(" ");
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
	QPair<ProductionType, quint32> pair;
	pair.first = (ProductionType)type;
	pair.second = id;
	mProductions.append(pair);
 }
 return true;
}

bool ProductionPlugin::contains(ProductionType type, quint32 id)
{
 QPair<ProductionType, quint32> pair;
 pair.first = type;
 pair.second = id;
 return productionList().contains(pair);
}

Q3ValueList<quint32> ProductionPlugin::allUnitProductions(Q3ValueList<quint32>* producible, Q3ValueList<quint32>* notYetProducible) const
{
 Q3ValueList<quint32> ret;
 ProductionProperties* pp = (ProductionProperties*)properties(PluginProperties::Production);
 if (!pp) {
	// Must not happen when the unit has a ProductionPlugin
	boError() << k_funcinfo << "no ProductionProperties" << endl;
	return ret;
 }
 if (!speciesTheme()) {
	BO_NULL_ERROR(speciesTheme());
	return ret;
 }

 Q3ValueList<quint32> allProductions = speciesTheme()->productions(pp->producerList());

 if (producible) {
	producible->clear();
 }
 if (notYetProducible) {
	notYetProducible->clear();
 }

 for (Q3ValueList<quint32>::Iterator it = allProductions.begin(); it != allProductions.end(); ++it) {
	if (player()->canBuild(*it)) {
		if (producible) {
			producible->append(*it);
		}
	} else {
		if (notYetProducible) {
			notYetProducible->append(*it);
		}
	}
 }

 return allProductions;
}

Q3ValueList<quint32> ProductionPlugin::allTechnologyProductions(Q3ValueList<quint32>* producible, Q3ValueList<quint32>* notYetProducible) const
{
 Q3ValueList<quint32> ret;
 ProductionProperties* pp = (ProductionProperties*)properties(PluginProperties::Production);
 if (!pp) {
	// Must not happen when the unit has a ProductionPlugin
	boError() << k_funcinfo << "no ProductionProperties" << endl;
	return ret;
 }
 if (!speciesTheme()) {
	BO_NULL_ERROR(speciesTheme());
	return ret;
 }

 Q3ValueList<quint32> allTechs = speciesTheme()->technologies(pp->producerList());

 if (producible) {
	producible->clear();
 }
 if (notYetProducible) {
	notYetProducible->clear();
 }

 Q3ValueList<quint32> allUnresearchedTechs;
 for (Q3ValueList<quint32>::Iterator it = allTechs.begin(); it != allTechs.end(); it++) {
	if (!player()->hasTechnology(*it)) {
		allUnresearchedTechs.append(*it);
	}
 }
 for (Q3ValueList<quint32>::Iterator it = allUnresearchedTechs.begin(); it != allUnresearchedTechs.end(); it++) {
	if (player()->canResearchTech(*it)) {
		if (producible) {
			producible->append(*it);
		}
	} else {
		if (notYetProducible) {
			notYetProducible->append(*it);
		}
	}
 }

 return allUnresearchedTechs;
}

bool ProductionPlugin::canCurrentlyProduce(ProductionType p, quint32 type) const
{
 switch (p) {
	case ProduceUnit:
		return canCurrentlyProduceUnit(type);
	case ProduceTech:
		return canCurrentlyProduceTechnology(type);
	default:
		boWarning() << k_funcinfo << "unknown ProductionType " << (int)p << endl;
		// fall-through intended
	case ProduceNothing:
		return false;
 }
 return false;
}

bool ProductionPlugin::canCurrentlyProduceUnit(quint32 type) const
{
 Q3ValueList<quint32> producible;
 allUnitProductions(&producible, 0);
 if (producible.contains(type)) {
	return true;
 }
 return false;
}

bool ProductionPlugin::canCurrentlyProduceTechnology(quint32 type) const
{
 Q3ValueList<quint32> producible;
 allTechnologyProductions(&producible, 0);
 if (producible.contains(type)) {
	return true;
 }
 return false;
}


