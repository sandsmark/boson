/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bosonscenario.h"

#include "player.h"
#include "unit.h"
#include "unitproperties.h"
#include "boson.h"

#include <kdebug.h>

#include <qdom.h>

#include "defines.h"

class BosonScenario::BosonScenarioPrivate
{
public:
	BosonScenarioPrivate()
	{
		mMaxPlayers = 0;
		mMinPlayers = 0;
	}
	
	int mMaxPlayers; // -1 == unlimited
	unsigned int mMinPlayers;

	QDomDocument mInternalDoc;
};

BosonScenario::BosonScenario()
{
 init();
}

BosonScenario::~BosonScenario()
{
 kdDebug() << k_funcinfo << endl;
 delete d;
}

void BosonScenario::init()
{
 d = new BosonScenarioPrivate;
 setModified(false);
}

bool BosonScenario::loadScenario(QDomElement& root)
{
 // TODO: check for syntax errors
 d->mInternalDoc.appendChild(root.cloneNode(true));

 QDomNodeList list = root.elementsByTagName("ScenarioSettings");
 if (list.count() != 1) {
	kdError() << "Cannot have ScenarioSettings " << list.count() 
			<< " times"<< endl;
	return false;
 }
 QDomElement settings = list.item(0).toElement();
 if (settings.isNull()) {
	kdError() << "settings is not a QDomElement" << endl;
	return false;
 }

 if (!loadScenarioSettings(settings)) {
	kdError() << "Could not load scenario settings" << endl;
	return false;
 }

 return true;
 
 
 /*
 QDomNodeList list;
 list = root.elementsByTagName("ScenarioSettings");
 if (list.count() != 1) {
	kdError() << "Cannot have ScenarioSettings " << list.count() 
			<< " times"<< endl;
	return false;
 }
 QDomElement settings = list.item(0).toElement();
 if (settings.isNull()) {
	kdError() << "settings is not a QDomElement" << endl;
	return false;
 }
 if (!loadScenarioSettings(settings)) {
	kdError() << "Could not load scenario settings" << endl;
	return false;
 }

 list = root.elementsByTagName("ScenarioPlayers");
 if (list.count() != 1) {
	kdError() << "Cannot have ScenarioPlayers " << list.count() 
			<< " times"<< endl;
	return false;
 }
 QDomElement players = list.item(0).toElement();
 if (players.isNull()) {
	kdError() << "players is not a QDomElement" << endl;
	return false;
 }
 if (!loadPlayers(players)) {
	kdError() << "Could not load scenario players" << endl;
	return false;
 }*/

 return false;
}

bool BosonScenario::saveScenario(QDomElement& root)
{
 // we manage our own xml document of the scenario. we simply need to append the
 // root of our own document to the other root.
 root.appendChild(d->mInternalDoc.documentElement().cloneNode(true));
 return true;
}

bool BosonScenario::saveScenarioSettings(QDomElement& node)
{
 QDomDocument doc = node.ownerDocument();
 node.setAttribute("MinPlayers", minPlayers());
 node.setAttribute("MaxPlayers", maxPlayers());
 return true;
}

bool BosonScenario::loadScenarioSettings(QDomElement& node)
{
 if (!node.hasAttribute("MaxPlayers")) {
	kdError() << "Missing MaxPlayers" << endl;
	return false;
 }

 bool ok;
 unsigned int min = 1;
 if (node.hasAttribute("MinPlayers")) {
	min = node.attribute("MinPlayers").toUInt(&ok);
	if (!ok) {
		kdWarning() << "invalid MinPlayers" << endl;
		min = 1;
	}
 }
 if (min < 1) {
	kdError() << k_funcinfo << "broken scenario file!" << endl;
	kdError() << "min < 1" << endl;
	return false;
 }
 int max = node.attribute("MaxPlayers").toInt(&ok);
 if (!ok) {
	kdWarning() << "invalid MaxPlayers" << endl;
	max = min;
 }
 if (max > BOSON_MAX_PLAYERS) {
	kdError() << k_funcinfo << "broken scenario file!" << endl;
	kdError() << k_funcinfo << "max > " << BOSON_MAX_PLAYERS << endl;
	return false;
 }
 if ((int)min > max) { 
	kdError() << k_funcinfo << "broken scenario file!" << endl;
	kdError() << k_funcinfo << "min > max" << endl;
	return false;
 }

 d->mMinPlayers = min;
 d->mMaxPlayers = max;

 return true;
}


bool BosonScenario::isValid() const
{
 if (d->mMinPlayers != (uint)d->mMaxPlayers) { // FIXME
	kdError() << k_funcinfo << ": internal error" << endl;
	return false;
 }
 if (d->mMinPlayers < 1) {
	kdError() << "minplayers < " << 1 << endl;
	return false;
 }
 if (d->mMinPlayers > BOSON_MAX_PLAYERS) {
	kdError() << "minplayers > " << BOSON_MAX_PLAYERS << endl;
	return false;
 }
 return true;
}

unsigned int BosonScenario::minPlayers() const
{
 return d->mMinPlayers;
}

int BosonScenario::maxPlayers() const
{
 return d->mMaxPlayers;
}

void BosonScenario::startScenario(Boson* boson)
{
 if (!isValid()) {
	return;
 }
 if (!boson) {
	kdError() << k_funcinfo << "NULL game" << endl;
	return;
 }

 // no error must happen here anymore!! everything should have been checked in
 // loadScenario()

 QDomNodeList list = d->mInternalDoc.documentElement().elementsByTagName("ScenarioPlayers").item(0).toElement().elementsByTagName("Player");
 QValueList<int> playerOrder;
 for (int unsigned i = 0; i < list.count(); i++) {
	QDomElement player = list.item(i).toElement();
	if (!player.hasAttribute("PlayerNumber")) {
		kdError() << "Missing PlayerNumber" << endl;
		return;
	}
	bool ok = true;
	unsigned int playerNumber = player.attribute("PlayerNumber").toUInt(&ok);
	if (!ok) {
		kdError() << k_funcinfo << "PlayerNumber was no number" << endl;
		playerNumber = 0;
	}
	if ((int)playerNumber >= maxPlayers()) {
		kdError() << k_funcinfo << "don't know player " << playerNumber 
				<< endl;
		return;
	}
	playerOrder.append(playerNumber);
 }

 
 for (unsigned int i = 0; i < list.count(); i++) {
	bool ok = false;
	QDomElement node = list.item(i).toElement();
	Player* p = (Player*)boson->playerList()->at(playerOrder[i]);

	p->setOil(node.attribute("Oil").toULong());
	QDomElement m = node.elementsByTagName("Minerals").item(0).toElement();
	unsigned long int minerals = m.text().toULong(&ok);
	if (!ok) {
		kdError() << "Invalid minerals" << endl;
		minerals = 0;
	}
	p->setMinerals(minerals);

	QDomElement o = node.elementsByTagName("Oil").item(0).toElement();
	unsigned long int oil = o.text().toULong(&ok);
	if (!ok) {
		kdError() << "Invalid oil" << endl;
		oil= 0;
	}
	p->setOil(oil);

	loadPlayer(node, p);
 }
}

void BosonScenario::applyScenario(Boson* boson)
{
 if (!boson) {
	kdError() << k_funcinfo << "NULL game" << endl;
	return;
 }
 if (d->mInternalDoc.hasChildNodes()) {
	kdWarning() << k_funcinfo << "oops - should be empty" << endl;
	return;
 }
 QDomElement root = d->mInternalDoc.createElement("BosonScenario");
 d->mInternalDoc.appendChild(root);

 kdDebug() << k_funcinfo << endl;
 d->mMaxPlayers = boson->maxPlayers();
 d->mMinPlayers = boson->minPlayers();

 QDomElement scenarioSettings = d->mInternalDoc.createElement("ScenarioSettings");
 root.appendChild(scenarioSettings);
 if (!saveScenarioSettings(scenarioSettings)) { //FIXME: "save" is not correct. maybe apply.. use a separate function?
	kdError() << "Could not apply scenario settings" << endl;
	return;
 }

 QDomElement players = d->mInternalDoc.createElement("ScenarioPlayers");
 root.appendChild(players);

 for (int i = 0; i < d->mMaxPlayers; i++) {
	Player* p = (Player*)boson->playerList()->at(i);
	QDomElement playerNode = d->mInternalDoc.createElement("Player");
	playerNode.setAttribute("PlayerNumber", (unsigned int)i);
	players.appendChild(playerNode);
	if (!savePlayer(playerNode, p)) {
		kdError() << "Error saving player " << i << endl;
		return;
	}
 }
}



bool BosonScenario::savePlayer(QDomElement& node, Player* p)
{
 QDomDocument doc = node.ownerDocument();
 QDomElement m = doc.createElement("Minerals");
 m.appendChild(doc.createTextNode(QString::number(p->minerals())));
 node.appendChild(m);

 QDomElement o = doc.createElement("Oil");
 o.appendChild(doc.createTextNode(QString::number(p->oil())));
 node.appendChild(o);

 // now save all units of the player into node
 QPtrList<Unit> list = p->allUnits();
 QPtrListIterator<Unit> it(list);
 while (it.current()) {
	Unit* u = it.current();
	QDomElement unit = doc.createElement("Unit");
	saveBasicUnit(unit, u->type(), (unsigned int)(u->x() / BO_TILE_SIZE), 
			(unsigned int)(u->y() / BO_TILE_SIZE));
	saveUnit(unit, u);
	node.appendChild(unit);
	++it;
 }
 return true;
}

bool BosonScenario::loadPlayer(QDomElement& node, Player* p)
{
 QDomNodeList list = node.elementsByTagName("Unit");

 QDomDocument doc;
 QDomElement root = doc.createElement("BosonUnits");
 doc.appendChild(root);
 for (unsigned int i = 0; i < list.count(); i++) {
	QDomElement unit = list.item(i).toElement();
	root.appendChild(unit.cloneNode(true));
 }

 ((Boson*)p->game())->sendAddUnits(doc.toString(), p);
 return true;
}


bool BosonScenario::saveBasicUnit(QDomElement& node, int unitType, unsigned int x, unsigned int y)
{
 node.setAttribute("UnitType", unitType);
 node.setAttribute("x", x);
 node.setAttribute("y", y);
 return true;
}

bool BosonScenario::loadBasicUnit(QDomElement& node, int& unitType, unsigned int& x, unsigned int& y)
{
 if (!node.hasAttribute("UnitType")) {
	kdError() << k_funcinfo << "missing UnitType" << endl;
	return false;
 }
 if (!node.hasAttribute("x")) {
	kdError() << k_funcinfo << "missing x" << endl;
	return false;
 }
 if (!node.hasAttribute("y")) {
	kdError() << k_funcinfo << "missing y" << endl;
	return false;
 }
 bool ok = false;
 unitType = node.attribute("UnitType").toInt(&ok);
 if (!ok) {
	kdError() << k_funcinfo << "UnitType is no number" << endl;
	return false;
 }
 x = node.attribute("x").toUInt(&ok);
 if (!ok) {
	kdError() << k_funcinfo << "x is no number" << endl;
	return false;
 }
 y = node.attribute("y").toUInt(&ok);
 if (!ok) {
	kdError() << k_funcinfo << "y is no number" << endl;
	return false;
 }
 return true;
}


bool BosonScenario::saveUnit(QDomElement& node, Unit* unit)
{
 if (!unit) {
	kdError() << k_funcinfo << "NULL unit" << endl;
	return false;
 }

 // this stuff is only meant to get into the xml file if they
 // differ from the defaults
 const UnitProperties* prop = unit->unitProperties();
 if (unit->health() != prop->health()) {
	node.setAttribute("Health", (unsigned int)unit->health());
 }
 if (unit->armor() != prop->armor()) {
	node.setAttribute("Armor", (unsigned int)unit->armor()); // currently unused
 }
 if (unit->shields() != prop->shields()) {
	node.setAttribute("Shields", (unsigned int)unit->shields()); // currently unused
 }
 if (unit->work() != UnitBase::WorkNone) {
	node.setAttribute("Work", (unsigned int)unit->work());
 }
 if (unit->reloadState() != 0) {
	node.setAttribute("ReloadState", (unsigned int)unit->reloadState());
 }

 // these entries are *meant* to be changeable during the game.
 // but currently they are not. they will never appear in the xml
 // file, as they don't differ from the default (howver if that
 // ever gets implemented it can be used immediately)
 if (unit->weaponDamage() != prop->weaponDamage()) {
	node.setAttribute("WeaponDamage", (int)unit->weaponDamage());
 }
 if (unit->weaponRange() != prop->weaponRange()) {
	node.setAttribute("WeaponRange", (unsigned int)unit->weaponRange());
 }
 if (unit->sightRange() != prop->sightRange()) {
	node.setAttribute("SightRange", (unsigned int)unit->sightRange());
 }


 // finally save the mobile unit / facility specific properties.
 if (unit->isFacility()) {
	if (!saveFacility(node, (Facility*)unit)) {
		return false;
	}
 } else {
	if (!saveMobile(node, (MobileUnit*)unit)) {
		return false;
	}
 }
 return true;
}

// called mainly by Boson::addUnit()
bool BosonScenario::loadUnit(QDomElement& node, Unit* unit)
{
 // note that unit can be 0 !!!
 // this will be the case when the xml file is parsed initially. We simply load
 // all values but apply nowhere.
 // Check if unit is 0 before actually using it!
 bool ret = true;
 bool ok = false; // for QString::toInt() like functions
 if (!unit) {
	kdDebug() << k_funcinfo << "NULL unit - starting test run" << endl;
 }

 if (node.hasAttribute("Health")) {
	unsigned long int v = node.attribute("Health").toULong(&ok);
	if (!ok) {
		kdError() << k_funcinfo << "Invalid value for Health" << endl;
		ret = false;
	} else if (unit) {
		unit->setHealth(v);
	}
 }
  if (node.hasAttribute("Armor")) {
	unsigned long int v = node.attribute("Armor").toULong(&ok);
	if (!ok) {
		kdError() << k_funcinfo << "Invalid value for Armor" << endl;
		ret = false;
	} else if (unit) {
		unit->setArmor(v);
	}
 }
 if (node.hasAttribute("Shield")) {
	unsigned long int v = node.attribute("Shield").toULong(&ok);
	if (!ok) {
		kdError() << k_funcinfo << "Invalid valu for Shield" << endl;
		ret = false;
	} else if (unit) {
		unit->setShields(v);
	}
 }
 if (node.hasAttribute("Work")) {
	unsigned int v = node.attribute("Work").toUInt(&ok);
	if (!ok) {
		kdError() << k_funcinfo << "Invalid value for Work" << endl;
		ret = false;
	} else if (unit) {
		unit->setWork((UnitBase::WorkType)v);
	}
 }
 if (node.hasAttribute("ReloadState")) {
	unsigned int v = node.attribute("ReloadState").toUInt(&ok);
	if (!ok) {
		kdError() << k_funcinfo << "Invalid value for ReloadState" << endl;
		ret = false;
	} else if (unit) {
		kdWarning() << k_funcinfo << "Value for ReloadStat is valid, but not yet supported." << endl;
//		unit->setReloadState(v);
	}
 }

 if (node.hasAttribute("WeaponDamage")) {
	int v = node.attribute("WeaponDamage").toInt(&ok);
	if (!ok) {
		kdError() << k_funcinfo << "Invalid value for WeaponDamage" << endl;
		ret = false;
	} else if (unit) {
		unit->setWeaponDamage(v);
	}
 }

 if (node.hasAttribute("WeaponRange")) {
	unsigned int v = node.attribute("WeaponRange").toUInt(&ok);
	if (!ok) {
		kdError() << k_funcinfo << "Invalid value for WeaponRange" << endl;
		ret = false;
	} else if (unit) {
		unit->setWeaponRange(v);
	}
 }

 if (node.hasAttribute("SightRange")) {
	unsigned int v = node.attribute("SightRange").toUInt(&ok);
	if (!ok) {
		kdError() << k_funcinfo << "Invalid value for SightRange" << endl;
		ret = false;
	} else if (unit) {
		unit->setSightRange(v);
	}
 }

 // finally load the mobile unit / facility specific properties.
 if (unit) {
	if (unit->isFacility()) {
		if (!loadFacility(node, (Facility*)unit)) {
			ret = false;
		}
	} else {
		if (!loadMobile(node, (MobileUnit*)unit)) {
			ret = false;
		}
	}
 }

 return ret;
}

bool BosonScenario::saveFacility(QDomElement& node, Facility* fac)
{
 if (fac->isConstructionComplete()) {
	node.setAttribute("ConstructionCompleted", 1);
 } else if (fac->currentConstructionStep() != 0) {
	node.setAttribute("ConstructionStep", fac->currentConstructionStep());
 }
 return true;
}

bool BosonScenario::loadFacility(QDomElement& node, Facility* fac)
{
 // note that fac can be 0 !!!
 // this will be the case when the xml file is parsed initially. We simply load
 // all values but apply nowhere.
 // Check if fac is 0 before actually using it!
 bool ret = true;
 bool ok = false; // for QString::toInt() like functions
 if (node.hasAttribute("ConstructionCompleted")) { // its value does not matter
	if (fac) {
		fac->setConstructionStep(fac->constructionSteps() - 1);
	}
 } else if (node.hasAttribute("ConstructionStep")) {
	unsigned int step = node.attribute("ConstructionStep").toUInt(&ok);
	if (!ok) {
		ret = false;
		kdError() << k_funcinfo << "Invalid value for ConstructionStep" << endl;
	} else if (fac) {
		fac->setConstructionStep(step);
	}
 }
 return ret;
}

bool BosonScenario::saveMobile(QDomElement& node, MobileUnit* mob)
{
 bool ret = true;
 const UnitProperties* prop = mob->unitProperties();

// meant to change during the game one day - but it is unused currenlty.
 if (mob->speed() != prop->speed()) {
	node.setAttribute("Speed", (double)mob->speed());
 }
 return ret;
}

bool BosonScenario::loadMobile(QDomElement& node, MobileUnit* mob)
{
 // note that mob can be 0 !!!
 // this will be the case when the xml file is parsed initially. We simply load
 // all values but apply nowhere.
 // Check if mob is 0 before actually using it!
 bool ret = true;
 bool ok = false; // for QString::toInt() like functions
 if (node.hasAttribute("Speed")) {
	double speed = node.attribute("Speed").toDouble(&ok);
	if (!ok) {
		kdError() << k_funcinfo << "Invalid value for speed!" << endl;
	} else if (mob) {
		mob->setSpeed(speed);
	}
 }
 return ret;
}

