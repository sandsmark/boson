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
#include "boson.h"

#include <kdebug.h>

#include <qdatastream.h>
#include <qdom.h>

#include "defines.h"

class ScenarioUnit
{
public:
	ScenarioUnit()
	{
	}

	bool save(QDomElement& node)
	{
		if (!mNode.hasAttribute("UnitType")) {
			kdError() << k_funcinfo << "missing UnitType" << endl;
			return false;
		}
		if (!mNode.hasAttribute("x")) {
			kdError() << k_funcinfo << "missing x" << endl;
			return false;
		}
		if (!mNode.hasAttribute("y")) {
			kdError() << k_funcinfo << "missing y" << endl;
			return false;
		}
		node = mNode.cloneNode(true).toElement();
		return true;
	}
	
	bool load(QDomElement& node)
	{
		if (!node.hasAttribute("UnitType")) {
			kdError() << "Missing UnitType" << endl;
			return false;
		}
		if (!node.hasAttribute("x")) {
			kdError() << "Missing x" << endl;
			return false;
		}
		if (!node.hasAttribute("y")) {
			kdError() << "Missing y" << endl;
			return false;
		}

		mNode = node.cloneNode(true).toElement(); 
		return true;
	}

private:
	QDomElement mNode;
};


class ScenarioPlayer 
{
public:

	ScenarioPlayer()
	{
		mMinerals = 0;
		mOil = 0;

		mUnits.setAutoDelete(true);
	}

	~ScenarioPlayer()
	{
		mUnits.clear();
	}
	bool savePlayer(QDomDocument& doc, QDomElement& node)
	{
		QDomElement m = doc.createElement("Minerals");
		m.appendChild(doc.createTextNode(QString::number(minerals())));
		node.appendChild(m);

		QDomElement o = doc.createElement("Oil");
		o.appendChild(doc.createTextNode(QString::number(oil())));
		node.appendChild(o);

		QPtrListIterator<ScenarioUnit> it(mUnits);
		while (it.current()) {
			QDomElement unit = doc.createElement("Unit");
			it.current()->save(unit);
			node.appendChild(unit);
			++it;
		}
		return true;
	}

	bool loadPlayer(QDomElement& node)
	{
		bool ret = true;
		unsigned long int minerals = 0;
		unsigned long int oil = 0;

		if (!readMinerals(node, minerals)) {
			ret = false;
		}
		if (!readOil(node, oil)) {
			ret = false;
		}
		setMinerals(minerals);
		setOil(oil);
 
		QDomNodeList list = node.elementsByTagName("Unit");
		for (unsigned int i = 0; i < list.count(); i++) {
			QDomElement unit = list.item(i).toElement();
			if (unit.isNull()) {
				kdError() << "Unit is not a QDomElement" << endl;
				return false;
			}
			ScenarioUnit* s = new ScenarioUnit();
			if (!s->load(unit)) {
				ret = false;
			}
		 	mUnits.append(s);
		}
		return ret;
	}

	unsigned int unitCount() const
	{
		return mUnits.count();
	}
	
	ScenarioUnit* unit(unsigned int i)
	{
		return mUnits.at(i);
	}

	void setMinerals(unsigned long int m) { mMinerals = m; }
	unsigned long int minerals() const { return mMinerals; }

	void setOil(unsigned long int o) { mOil = o; }
	unsigned long int oil() const { return mOil; }

	void addToGame(Boson* boson, Player* p)
	{
		p->setMinerals(minerals());
		p->setOil(oil());

		// now add the units
		QDomDocument doc;
		QDomElement root = doc.createElement("BosonUnits");
		doc.appendChild(root);

		for (unsigned int i = 0; i < unitCount(); i++) {
			QDomElement e = doc.createElement("Unit");
			if (unit(i)->save(e)) {
				root.appendChild(e);
			} else {
				kdWarning() << k_funcinfo << "failed saving unit " << i << endl;
			}
		 }

		boson->sendAddUnits(doc.toString(), p);
	}

protected:
	bool readMinerals(QDomElement& node, unsigned long int& minerals)
	{
		QDomNodeList list = node.elementsByTagName("Minerals");
		if (list.count() != 1) {
			kdWarning() << "Must have exactly one \"Minerals\" per player" << endl;
			return false;
		}
		bool ok = false;
		QDomElement e = list.item(0).toElement();
		minerals = e.text().toULong(&ok);
		if (!ok) {
			kdError() << "Invalid minerals" << endl;
			return false;
		}
		return true;
	}

	bool readOil(QDomElement& node, unsigned long int& oil)
	{
		QDomNodeList list = node.elementsByTagName("Oil");
		if (list.count() != 1) {
			kdWarning() << "Must have exactly one \"Oil\" per player" << endl;
			return false;
		}
		bool ok = false;
		QDomElement e = list.item(0).toElement();
		oil = e.text().toULong(&ok);
		if (!ok) {
			kdError() << "Invalid oil" << endl;
			return false;
		}
		return true;
	}
	
private:
	unsigned long int mMinerals;
	unsigned long int mOil;

	QPtrList<ScenarioUnit> mUnits;
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

	QValueList<ScenarioPlayer> mPlayers;
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
}

bool BosonScenario::loadScenario(QDomElement& root)
{
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
 }

 return false;
}

bool BosonScenario::saveScenario(QDomElement& root)
{
 QDomDocument doc = root.ownerDocument();
 QDomNode node = doc.createElement("BosonScenario");
 root.appendChild(node);
 
 QDomElement settings = doc.createElement("ScenarioSettings");
 node.appendChild(settings);
 
 if (!saveScenarioSettings(settings)) {
	kdError() << "Could not save scenario settings to XML" << endl;
	return false;
 }

 QDomElement players = doc.createElement("ScenarioPlayers");
 node.appendChild(players);
 if (!savePlayers(players)) {
	kdError() << "Could not save players to XML" << endl;
	return false;
 }

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
 if (d->mPlayers.count() != 0) {
	kdError() << k_funcinfo << "called before!!" << endl;
	d->mPlayers.clear();
 }

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
	max = 0;
 }
 if (max > BOSON_MAX_PLAYERS) {
	kdError() << k_funcinfo << "broken scenario file!" << endl;
	kdError() << "max > " << BOSON_MAX_PLAYERS << endl;
	return false;
 }
 if ((int)min > max) {
	kdError() << k_funcinfo << "broken scenario file!" << endl;
	kdError() << "min > max" << endl;
	return false;
 }

 d->mMinPlayers = min;
 d->mMaxPlayers = max;
 
 for (int i = 0; i < d->mMaxPlayers; i++) {
	d->mPlayers.append(ScenarioPlayer());
 }
 return true;
}

bool BosonScenario::loadPlayers(QDomElement& node)
{
 QDomNodeList list = node.elementsByTagName("Player");
 if (list.count() < minPlayers()) {
	kdError() << "Not enough players in file - expected " << minPlayers() 
			<< ", found " << list.count() << endl;
	return false;
 }
 if ((int)list.count() > maxPlayers()) {
	kdWarning() << "Too many players in file. Read only " << maxPlayers() 
			<< endl;
 }

 for (int unsigned i = 0; i < list.count(); i++) {
	QDomElement player = list.item(i).toElement();
	if (player.isNull()) {
		kdError() << i << " is not a QDomElement" << endl;
		return false;
	}

	if (!loadPlayer(player)) {
		kdError() << "Could not load player " << i << endl;
		return false;
	}
 }
 return true;
}

bool BosonScenario::loadPlayer(QDomElement& node)
{
 if (!node.hasAttribute("PlayerNumber")) {
	kdError() << "Missing PlayerNumber" << endl;
	return false;
 }

 bool ok = false;
 unsigned int playerNumber = node.attribute("PlayerNumber").toUInt(&ok);
 if (!ok) {
	kdError() << k_funcinfo << "PlayerNumber was no number" << endl;
	playerNumber = 0;
 }

 if ((int)playerNumber >= maxPlayers()) {
	kdError() << k_funcinfo << ": don't know player " << playerNumber 
			<< endl;
	return false;
 }
 if (maxPlayers() != (int)d->mPlayers.count()) {
	kdError() << k_funcinfo << "maxPlayers() != d->mPlayers.count()" << endl;
	return false;
 }

 return d->mPlayers[playerNumber].loadPlayer(node);
}

bool BosonScenario::savePlayers(QDomElement& node)
{
 if (maxPlayers() != (int)d->mPlayers.count()) {
	kdError() << k_funcinfo << "maxPlayers() != d->mPlayers.count()" << endl;
	return false;
 }
 QDomDocument doc = node.ownerDocument();
 for (int i = 0; i < maxPlayers(); i++) {
	QDomElement player = doc.createElement("Player");
	node.appendChild(player);
	player.setAttribute("PlayerNumber", i);
	if (!d->mPlayers[i].savePlayer(doc, player)) {
		kdError() << "Error saving units of player " << i << endl;
		return false;
	}
 }
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
 if (maxPlayers() != (int)d->mPlayers.count()) {
	kdError() << k_funcinfo << "maxPlayers() != d->mPlayers.count()" << endl;
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

void BosonScenario::initPlayer(Boson* boson, int playerNumber)
{
 if (!boson) {
	kdError() << k_funcinfo << ": NULL game" << endl;
	return;
 }
 Player* p = (Player*)boson->playerList()->at(playerNumber);
 if (!p) {
	kdError() << k_funcinfo << ": NULL player" << endl;
	return;
 }
 kdDebug() << k_funcinfo << " player " << playerNumber << "==" << p->id() 
		<< endl;
 if (playerNumber >= (int)d->mPlayers.count()) {
	kdError() << k_funcinfo << ": don't have player " << playerNumber 
			<< endl;
	return;
 }
 d->mPlayers[playerNumber].addToGame(boson, p);
}

void BosonScenario::startScenario(Boson* boson)
{
 if (!isValid()) {
	return;
 }
 if (!boson) {
	kdError() << k_funcinfo << ": NULL game" << endl;
	return;
 }
 for (int i = 0; i < maxPlayers(); i++) {
	initPlayer(boson, i);
 }
}

