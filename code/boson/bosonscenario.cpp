/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2003 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include "bodebug.h"

#include <qdom.h>

#include "defines.h"

/**
 * Class that is used to emphasize in the code where game-logic relevant actions
 * are executed in @ref BosonScenario code. Whenever @ref Boson or similar is
 * touched (in a non-readonly way) this class is used.
 *
 * This mainly means that this builder is used to start a scenario.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonScenarioBuilder
{
public:
	BosonScenarioBuilder(BosonScenario* s)
	{
		mScenario = s;
	}

	bool startScenario(Boson* boson);

	static bool loadBasicUnit(const QDomElement& node, unsigned long int& unitType, unsigned int& x, unsigned int& y);
	static bool loadUnit(const QDomElement& node, Unit* unit);

protected:
	static bool loadMobile(const QDomElement& node, MobileUnit* mob);
	static bool loadFacility(const QDomElement& node, Facility* fac);

	bool loadPlayer(const QDomElement& node, Player* p);

private:
	const BosonScenario* mScenario;
};


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
 boDebug(250) << k_funcinfo << endl;
 delete d;
}

void BosonScenario::init()
{
 d = new BosonScenarioPrivate;
 setModified(false);
}

void BosonScenario::setPlayers(unsigned int min, int max)
{
 d->mMinPlayers = min;
 d->mMaxPlayers = max;
}

bool BosonScenario::loadScenarioFromDocument(const QString& xml)
{
 if (xml.isEmpty()) {
	boError(250) << k_funcinfo << "empty xml document" << endl;
	return false;
 }
 QDomDocument doc(QString::fromLatin1("BosonScenario"));
 QString errorMsg;
 int lineNo, columnNo;
 if (!doc.setContent(xml, &errorMsg, &lineNo, &columnNo)) {
	boError(250) << k_funcinfo << "parse error in line " << lineNo
			<< " column " << columnNo << " error message: "
			<< errorMsg << endl;
	return false;
 }
 QDomElement root = doc.documentElement();
 if (root.childNodes().count() < 2) {
	// there must be at least scenario settings and one player
	boError(250) << k_funcinfo << "no scenario found in file" << endl;
	return false;
 }
 return loadScenario(root);
}

bool BosonScenario::loadScenario(const QDomElement& root)
{
 // TODO: check for syntax errors
 d->mInternalDoc.appendChild(root.cloneNode(true));

 QDomNodeList list = root.elementsByTagName("ScenarioSettings");
 if (list.count() != 1) {
	boError(250) << k_funcinfo << "Cannot have ScenarioSettings "
			<< list.count() << " times"<< endl;
	return false;
 }
 QDomElement settings = list.item(0).toElement();
 if (settings.isNull()) {
	boError(250) << k_funcinfo << "settings is not a QDomElement" << endl;
	return false;
 }

 if (!loadScenarioSettings(settings)) {
	boError(250) << k_funcinfo << "Could not load scenario settings" << endl;
	return false;
 }

 return true;


 /*
 QDomNodeList list;
 list = root.elementsByTagName("ScenarioSettings");
 if (list.count() != 1) {
	boError(250) << k_funcinfo << "Cannot have ScenarioSettings " << list.count() 
			<< " times"<< endl;
	return false;
 }
 QDomElement settings = list.item(0).toElement();
 if (settings.isNull()) {
	boError(250) << k_funcinfo << "settings is not a QDomElement" << endl;
	return false;
 }
 if (!loadScenarioSettings(settings)) {
	boError(250) << k_funcinfo << "Could not load scenario settings" << endl;
	return false;
 }

 list = root.elementsByTagName("ScenarioPlayers");
 if (list.count() != 1) {
	boError(250) << k_funcinfo << "Cannot have ScenarioPlayers " << list.count() 
			<< " times"<< endl;
	return false;
 }
 QDomElement players = list.item(0).toElement();
 if (players.isNull()) {
	boError(250) << k_funcinfo << "players is not a QDomElement" << endl;
	return false;
 }
 if (!loadPlayers(players)) {
	boError(250) << k_funcinfo << "Could not load scenario players" << endl;
	return false;
 }*/

 return false;
}

QString BosonScenario::saveScenarioToDocument() const
{
 QDomDocument doc(QString::fromLatin1("BosonScenario"));
 QDomElement root = doc.createElement(QString::fromLatin1("BosonScenario"));
 doc.appendChild(root);
 bool ret = saveScenario(root);
 if (!ret) {
	return QString::null;
 }
 return doc.toString();
}

bool BosonScenario::saveScenario(QDomElement& root) const
{
 // we manage our own xml document of the scenario. we simply need to append all
 // childs of our own document to the provided root
 boDebug(250) << k_funcinfo << endl;

 QDomNodeList list = d->mInternalDoc.documentElement().childNodes();
 for (unsigned int i = 0; i < list.count(); i++) {
	root.appendChild(list.item(i).cloneNode(true));
 }
 return true;
}

bool BosonScenario::saveScenarioSettings(QDomElement& node) const
{
 boDebug(250) << k_funcinfo << "MinPlayers=" << minPlayers() << " MaxPlayers=" << maxPlayers() << endl;
 if (maxPlayers() > 0) {
	if (minPlayers() > (unsigned int)maxPlayers()) {
		boError(250) << k_funcinfo << "minPlayers() > maxPlayers()" << endl;
		return false;
	}
 } else if (maxPlayers() == 0) {
	boError() << k_funcinfo << "maxPlayers()==0 !" << endl;
	return false;
 }
 if (maxPlayers() > BOSON_MAX_PLAYERS) {
	boError() << k_funcinfo << "maxPlayers can't be greater than " << BOSON_MAX_PLAYERS << endl;
	return false;
 }
 if (minPlayers() > BOSON_MAX_PLAYERS) {
	boError() << k_funcinfo << "minPlayers can't be greazer than " << BOSON_MAX_PLAYERS << endl;
	return false;
 }
 QDomDocument doc = node.ownerDocument();
 node.setAttribute("MinPlayers", minPlayers());
 node.setAttribute("MaxPlayers", maxPlayers());
 return true;
}

bool BosonScenario::loadScenarioSettings(const QDomElement& node)
{
 if (!node.hasAttribute("MaxPlayers")) {
	boError(250) << k_funcinfo << "Missing MaxPlayers" << endl;
	return false;
 }

 bool ok;
 unsigned int min = 1;
 if (node.hasAttribute("MinPlayers")) {
	min = node.attribute("MinPlayers").toUInt(&ok);
	if (!ok) {
		boWarning(250) << k_funcinfo << "invalid MinPlayers" << endl;
		min = 1;
	}
 }
 if (min < 1) {
	boError(250) << k_funcinfo << "broken scenario file!" << endl;
	boError(250) << k_funcinfo << "min < 1" << endl;
	return false;
 }
 int max = node.attribute("MaxPlayers").toInt(&ok);
 if (!ok) {
	boWarning(250) << k_funcinfo << "invalid MaxPlayers" << endl;
	max = min;
 }
 if (max > BOSON_MAX_PLAYERS) {
	boError(250) << k_funcinfo << "broken scenario file!" << endl;
	boError(250) << k_funcinfo << "max > " << BOSON_MAX_PLAYERS << endl;
	return false;
 }
 if ((int)min > max) {
	boError(250) << k_funcinfo << "broken scenario file!" << endl;
	boError(250) << k_funcinfo << "min > max" << endl;
	return false;
 }

 setPlayers(min, max);

 return true;
}


bool BosonScenario::isValid() const
{
 if (d->mMinPlayers < 1) {
	boError(250) << k_funcinfo << "minplayers < " << 1 << endl;
	return false;
 }
 if (d->mMinPlayers > BOSON_MAX_PLAYERS) {
	boError(250) << k_funcinfo << "minplayers > " << BOSON_MAX_PLAYERS << endl;
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
 BosonScenarioBuilder builder(this);
 builder.startScenario(boson);
}

bool BosonScenario::initializeScenario()
{
 if (maxPlayers() < 0) {
	boError(250) << k_funcinfo << "Oops - infinite players not yet supported :(" << endl;
	return false;
 }
 if (d->mInternalDoc.hasChildNodes()) {
	boWarning(250) << k_funcinfo << "oops - should be empty" << endl;
	return false;
 }
 QDomElement root = d->mInternalDoc.createElement("BosonScenario");
 d->mInternalDoc.appendChild(root);

 boDebug(250) << k_funcinfo << endl;
 if ((int)d->mMinPlayers > d->mMaxPlayers) {
	boWarning(250) << k_funcinfo << "minPlayers > playerCount" << endl;
	d->mMinPlayers = d->mMaxPlayers;
 }

 QDomElement scenarioSettings = d->mInternalDoc.createElement("ScenarioSettings");
 root.appendChild(scenarioSettings);
 boDebug(250) << k_funcinfo << "minplayers=" << minPlayers() << " maxplayers=" << maxPlayers() << endl;
 if (!saveScenarioSettings(scenarioSettings)) { //FIXME: "save" is not correct. maybe apply.. use a separate function?
	boError(250) << k_funcinfo << "Could not apply scenario settings" << endl;
	return false;
 }

 QDomElement players = d->mInternalDoc.createElement("ScenarioPlayers");
 root.appendChild(players);
 for (unsigned int i = 0; i < (unsigned int)maxPlayers(); i++) {
	QDomElement playerNode = d->mInternalDoc.createElement("Player");
	players.appendChild(playerNode);
	initPlayerNode(playerNode, i);
 }
 return true;
}

void BosonScenario::applyScenario(const Boson* boson)
{
 if (!boson) {
	boError(250) << k_funcinfo << "NULL game" << endl;
	return;
 }
 if (d->mInternalDoc.hasChildNodes()) {
	boWarning(250) << k_funcinfo << "oops - should be empty" << endl;
	return;
 }
 QDomElement root = d->mInternalDoc.createElement("BosonScenario");
 d->mInternalDoc.appendChild(root);

 boDebug(250) << k_funcinfo << endl;
 // AB: do not use boson->maxPlayers()/minPlayers() here!
 // we use these as failback only, i.e. boson->maxPlayers() is always
 // BOSON_MAX_PLAYERS, no matter which scenario is running!
 setPlayers(1, boson->playerCount());
 if ((int)d->mMinPlayers > d->mMaxPlayers) {
	boWarning(250) << k_funcinfo << "minPlayers > playerCount" << endl;
	if (d->mMaxPlayers < 1) {
		if (d->mMaxPlayers < 0) {
			boWarning(250) << k_funcinfo << "infinite players are not yet supported" << endl;
		}
		boError(250) << k_funcinfo << "Can't have less than 1 player as maximum!" << endl;
		return;
	}
	d->mMinPlayers = d->mMaxPlayers;
 }

 QDomElement scenarioSettings = d->mInternalDoc.createElement("ScenarioSettings");
 root.appendChild(scenarioSettings);
 boDebug(250) << k_funcinfo << "minplayers=" << minPlayers() << " maxplayers=" << maxPlayers() << endl;
 if (!saveScenarioSettings(scenarioSettings)) { //FIXME: "save" is not correct. maybe apply.. use a separate function?
	boError(250) << k_funcinfo << "Could not apply scenario settings" << endl;
	return;
 }

 QDomElement players = d->mInternalDoc.createElement("ScenarioPlayers");
 root.appendChild(players);

 boDebug(250) << k_funcinfo << "saving " << d->mMaxPlayers << " players" << endl;
 QPtrListIterator<KPlayer> it(*boson->playerList());
 unsigned int i = 0;
 for (; it.current(); ++it, i++) {
	Player* p = (Player*)it.current();
	QDomElement playerNode = d->mInternalDoc.createElement("Player");
	players.appendChild(playerNode);
	initPlayerNode(playerNode, i);
	if (!savePlayer(playerNode, p)) {
		boError(250) << k_funcinfo << "Error saving player " << i << endl;
		return;
	}
 }
}

void BosonScenario::initPlayerNode(QDomElement& player, unsigned int playerNumber)
{
 QDomDocument doc = player.ownerDocument();
 player.setAttribute("PlayerNumber", (unsigned int)playerNumber);

 QDomElement m = doc.createElement("Minerals");
 m.appendChild(doc.createTextNode(QString::number(0)));
 player.appendChild(m);

 QDomElement o = doc.createElement("Oil");
 o.appendChild(doc.createTextNode(QString::number(0)));
 player.appendChild(o);
}

bool BosonScenario::savePlayer(QDomElement& node, const Player* p)
{
 boDebug(250) << k_funcinfo << endl;
 QDomDocument doc = node.ownerDocument();
 QDomNodeList nodeList = node.elementsByTagName("Minerals");
 if (nodeList.count() != 1) {
	boError(250) << k_funcinfo << "Invalid element count for Minerals: " << nodeList.count() << endl;
	return false;
 }
 if (nodeList.item(0).firstChild().isText()) {
	nodeList.item(0).replaceChild(doc.createTextNode(QString::number(p->minerals())), nodeList.item(0).firstChild());
 }

 nodeList = node.elementsByTagName("Oil");
 if (nodeList.count() != 1) {
	boError(250) << k_funcinfo << "Invalid element count for Oil: " << nodeList.count() << endl;
	return false;
 }
 if (nodeList.item(0).firstChild().isText()) {
	nodeList.item(0).replaceChild(doc.createTextNode(QString::number(p->oil())), nodeList.item(0).firstChild());
 }

 // now save all units of the player into node
 QPtrListIterator<Unit> it(*(p->allUnits()));
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


bool BosonScenario::saveBasicUnit(QDomElement& node, unsigned long int unitType, unsigned int x, unsigned int y)
{
 node.setAttribute("Type", (unsigned int)unitType);
 node.setAttribute("x", x);
 node.setAttribute("y", y);
 return true;
}

bool BosonScenario::loadBasicUnit(const QDomElement& node, unsigned long int& unitType, unsigned int& x, unsigned int& y)
{
 return BosonScenarioBuilder::loadBasicUnit(node, unitType, x, y);
}


bool BosonScenario::saveUnit(QDomElement& node, const Unit* unit)
{
 if (!unit) {
	boError(250) << k_funcinfo << "NULL unit" << endl;
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
/* if (unit->reloadState() != 0) {
	node.setAttribute("ReloadState", (unsigned int)unit->reloadState());
 }*/

 // these entries are *meant* to be changeable during the game.
 // but currently they are not. they will never appear in the xml
 // file, as they don't differ from the default (howver if that
 // ever gets implemented it can be used immediately)
/* if (unit->weaponDamage() != prop->weaponDamage()) {
	node.setAttribute("WeaponDamage", (int)unit->weaponDamage());
 }
 if (unit->weaponRange() != prop->weaponRange()) {
	node.setAttribute("WeaponRange", (unsigned int)unit->weaponRange());
 }*/
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
bool BosonScenario::loadUnit(const QDomElement& node, Unit* unit)
{
 return BosonScenarioBuilder::loadUnit(node, unit);
}

bool BosonScenario::saveFacility(QDomElement& node, const Facility* fac)
{
 if (fac->isConstructionComplete()) {
	node.setAttribute("ConstructionCompleted", 1);
 } else if (fac->currentConstructionStep() != 0) {
	node.setAttribute("ConstructionStep", fac->currentConstructionStep());
 }
 return true;
}

bool BosonScenario::saveMobile(QDomElement& node, const MobileUnit* mob)
{
 bool ret = true;
 const UnitProperties* prop = mob->unitProperties();

// meant to change during the game one day - but it is unused currenlty.
 if (mob->maxSpeed() != prop->speed()) {
	node.setAttribute("MaxSpeed", (double)mob->maxSpeed());
 }
 return ret;
}


bool BosonScenarioBuilder::startScenario(Boson* boson)
{
 if (!mScenario) {
	BO_NULL_ERROR(mScenario);
	return false;
 }
 if (!boson) {
	BO_NULL_ERROR(boson);
	return false;
 }
 if (!mScenario->isValid()) {
	return false;
 }

 // no error must happen here anymore!! everything should have been checked in
 // loadScenario()
 // UPDATE: we also have to support *new* scenario, i.e. where loadScenario()
 // wasn't used. but errors arent so important then, as it is used by editor
 // only and not in network.

 QDomNodeList l = mScenario->d->mInternalDoc.documentElement().elementsByTagName("ScenarioPlayers");
 if (l.count() < 1) {
	boError(250) << k_funcinfo << "oops - broken file? no players!" << endl;
	return false;
 }
 QDomNodeList list = l.item(0).toElement().elementsByTagName("Player");
 if (boson->playerList()->count() > list.count()) {
	boError(250) << k_funcinfo << "too many players for this scenario" << endl;
	return false;
 }
 QValueList<int> playerOrder;
 for (int unsigned i = 0; i < list.count(); i++) {
	QDomElement player = list.item(i).toElement();
	if (!player.hasAttribute("PlayerNumber")) {
		boError(250) << k_funcinfo << "Missing PlayerNumber" << endl;
		return false;
	}
	bool ok = true;
	unsigned int playerNumber = player.attribute("PlayerNumber").toUInt(&ok);
	if (!ok) {
		boError(250) << k_funcinfo << "PlayerNumber was no number" << endl;
		playerNumber = 0;
	}
	if ((int)playerNumber >= mScenario->maxPlayers()) {
		boError(250) << k_funcinfo << "don't know player " << playerNumber 
				<< endl;
		return false;
	}
	playerOrder.append(playerNumber);
 }


 boDebug(250) << k_funcinfo << "players done" << endl;
 for (unsigned int i = 0; i < boson->playerList()->count(); i++) {
	bool ok = false;
	QDomElement node = list.item(i).toElement();
	Player* p = (Player*)boson->playerList()->at(playerOrder[i]);

	p->setOil(node.attribute("Oil").toULong());
	QDomElement m = node.elementsByTagName("Minerals").item(0).toElement();
	unsigned long int minerals = m.text().toULong(&ok);
	if (!ok) {
		boError(250) << k_funcinfo << "Invalid minerals" << endl;
		minerals = 0;
	}
	p->setMinerals(minerals);

	QDomElement o = node.elementsByTagName("Oil").item(0).toElement();
	unsigned long int oil = o.text().toULong(&ok);
	if (!ok) {
		boError(250) << k_funcinfo << "Invalid oil" << endl;
		oil= 0;
	}
	p->setOil(oil);

	bool ret = loadPlayer(node, p);
	if (!ret) {
		boError(250) << k_funcinfo << "error loading player " << i << endl;
		return ret;
	}
 }
 boDebug(250) << k_funcinfo << "done" << endl;
 return true;
}

bool BosonScenarioBuilder::loadPlayer(const QDomElement& node, Player* p)
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

bool BosonScenarioBuilder::loadBasicUnit(const QDomElement& node, unsigned long int& unitType, unsigned int& x, unsigned int& y)
{
 if (!node.hasAttribute("Type") && !node.hasAttribute("UnitType")) {
	boError(250) << k_funcinfo << "missing Type" << endl;
	return false;
 }
 if (!node.hasAttribute("x")) {
	boError(250) << k_funcinfo << "missing x" << endl;
	return false;
 }
 if (!node.hasAttribute("y")) {
	boError(250) << k_funcinfo << "missing y" << endl;
	return false;
 }
 bool ok = false;
 unitType = node.attribute("Type").toInt(&ok);
 if (!ok) {
	unitType = node.attribute("UnitType").toInt(&ok);
	if (!ok) {
		boError(250) << k_funcinfo << "UnitType is no number" << endl;
		return false;
	}
 }
 x = node.attribute("x").toUInt(&ok);
 if (!ok) {
	boError(250) << k_funcinfo << "x is no number" << endl;
	return false;
 }
 y = node.attribute("y").toUInt(&ok);
 if (!ok) {
	boError(250) << k_funcinfo << "y is no number" << endl;
	return false;
 }
 return true;
}

bool BosonScenarioBuilder::loadUnit(const QDomElement& node, Unit* unit)
{
 // note that unit can be 0 !!!
 // this will be the case when the xml file is parsed initially. We simply load
 // all values but apply nowhere.
 // Check if unit is 0 before actually using it!
 bool ret = true;
 bool ok = false; // for QString::toInt() like functions
 if (!unit) {
	boDebug(250) << k_funcinfo << "NULL unit - starting test run" << endl;
 }

 if (node.hasAttribute("Health")) {
	unsigned long int v = node.attribute("Health").toULong(&ok);
	if (!ok) {
		boError(250) << k_funcinfo << "Invalid value for Health" << endl;
		ret = false;
	} else if (unit) {
		unit->setHealth(v);
	}
 }
  if (node.hasAttribute("Armor")) {
	unsigned long int v = node.attribute("Armor").toULong(&ok);
	if (!ok) {
		boError(250) << k_funcinfo << "Invalid value for Armor" << endl;
		ret = false;
	} else if (unit) {
		unit->setArmor(v);
	}
 }
 if (node.hasAttribute("Shield")) {
	unsigned long int v = node.attribute("Shield").toULong(&ok);
	if (!ok) {
		boError(250) << k_funcinfo << "Invalid valu for Shield" << endl;
		ret = false;
	} else if (unit) {
		unit->setShields(v);
	}
 }
 if (node.hasAttribute("Work")) {
	unsigned int v = node.attribute("Work").toUInt(&ok);
	if (!ok) {
		boError(250) << k_funcinfo << "Invalid value for Work" << endl;
		ret = false;
	} else if (unit) {
		unit->setWork((UnitBase::WorkType)v);
	}
 }

 if (node.hasAttribute("SightRange")) {
	unsigned int v = node.attribute("SightRange").toUInt(&ok);
	if (!ok) {
		boError(250) << k_funcinfo << "Invalid value for SightRange" << endl;
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

bool BosonScenarioBuilder::loadMobile(const QDomElement& node, MobileUnit* mob)
{
 // note that mob can be 0 !!!
 // this will be the case when the xml file is parsed initially. We simply load
 // all values but apply nowhere.
 // Check if mob is 0 before actually using it!
 bool ret = true;
 bool ok = false; // for QString::toInt() like functions
 if (node.hasAttribute("MaxSpeed")) {
	double speed = node.attribute("MaxSpeed").toDouble(&ok);
	if (!ok) {
		boError(250) << k_funcinfo << "Invalid value for MaxSpeed!" << endl;
	} else if (mob) {
		mob->setMaxSpeed(speed);
	}
 }
 return ret;
}

bool BosonScenarioBuilder::loadFacility(const QDomElement& node, Facility* fac)
{
 // note that fac can be 0 !!!
 // this will be the case when the xml file is parsed initially. We simply load
 // all values but apply nowhere.
 // Check if fac is 0 before actually using it!
 bool ret = true;
 bool ok = false; // for QString::toInt() like functions
 if (node.hasAttribute("ConstructionCompleted")) { // its value does not matter
	if (fac) {
		fac->setConstructionStep(fac->constructionSteps());
	}
 } else if (node.hasAttribute("ConstructionStep")) {
	unsigned int step = node.attribute("ConstructionStep").toUInt(&ok);
	if (!ok) {
		ret = false;
		boError(250) << k_funcinfo << "Invalid value for ConstructionStep" << endl;
	} else if (fac) {
		fac->setConstructionStep(step);
	}
 }
 return ret;
}


