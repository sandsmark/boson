/*
    This file is part of the Boson game
    Copyright (C) 2004 Andreas Beckermann (b_mann@gmx.de)

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

#include "boeventlistener.h"
#include "boeventlistener.moc"

#include "../bomemory/bodummymemory.h"
#include "bodebug.h"
#include "boson.h"
#include "boeventmanager.h"
#include "boevent.h"
#include "bocondition.h"
#include "playerio.h"
#include "unitproperties.h"
#include "script/bosonscript.h"
#include "script/bosonscriptinterface.h"
#include "speciestheme.h"
#include "player.h"
#include "bosonprofiling.h"

#include <klocale.h>

#include <qptrlist.h>
#include <qdom.h>
#include <qintdict.h>


class BoEventHandlerInfo
{
public:
	QString function;
	QString args;
	QString eventName;
};

class BoEventListenerPrivate
{
public:
	BoEventListenerPrivate()
	{
		mScript = 0;
	}
	QPtrList<BoCondition> mConditions;

	QIntDict<BoEventHandlerInfo> mEventHandlers;
	int mNextEventHandlerId;

	BosonScript* mScript;
};

BoEventListener::BoEventListener(BoEventManager* manager, QObject* parent)
	: QObject(parent)
{
 d = new BoEventListenerPrivate;
 d->mConditions.setAutoDelete(true);
 d->mEventHandlers.setAutoDelete(true);
 d->mNextEventHandlerId = 1;
 mManager = manager;
 mManager->addEventListener(this);
}

BoEventListener::~BoEventListener()
{
 if (mManager) {
	mManager->removeEventListener(this);
 }
 d->mConditions.clear();
 d->mEventHandlers.setAutoDelete(true);
 d->mEventHandlers.clear();
 delete d->mScript;
 delete d;
}

bool BoEventListener::initScript()
{
 if (d->mScript) {
	// already done
	return true;
 }
 if (!mManager) {
	BO_NULL_ERROR(mManager);
	return false;
 }
 return mManager->loadEventListenerScript(this);
}

bool BoEventListener::addCondition(BoCondition* c)
{
 boDebug(360) << k_funcinfo << endl;
 d->mConditions.append(c);
 return true;
}

bool BoEventListener::saveAsXML(QDomElement& root) const
{
 QDomDocument doc = root.ownerDocument();
 QDomElement conditions = doc.createElement("Conditions");
 root.appendChild(conditions);
 if (!saveConditions(conditions)) {
	return false;
 }
 QDomElement eventhandlers = doc.createElement("EventHandlers");
 root.appendChild(eventhandlers);
 if (!saveEventHandlers(eventhandlers)) {
	return false;
 }

 return true;
}

bool BoEventListener::loadFromXML(const QDomElement& root)
{
 QDomElement conditions = root.namedItem("Conditions").toElement();
 if (conditions.isNull()) {
	boError(360) << k_funcinfo << "No Conditions tag" << endl;
	return false;
 }
 if (!loadConditions(conditions)) {
	return false;
 }
 QDomElement eventhandlers = root.namedItem("EventHandlers").toElement();
 if (eventhandlers.isNull()) {
	boDebug(360) << k_funcinfo << "No EventHandlers tag" << endl;
	return true;
 }
 if (!loadEventHandlers(eventhandlers)) {
	return false;
 }

 return true;
}

bool BoEventListener::saveScriptData(QByteArray* scriptData) const
{
 *scriptData = QByteArray();
 if (scriptFileName().isNull()) {
	boError() << k_funcinfo << "no scriptFileName()" << endl;
	return false;
 }
 if (!d->mScript) {
	// this event listener doesn't use scripts.
	return true;
 }
 QDataStream dataStream(*scriptData, IO_WriteOnly);
 if (!d->mScript->save(dataStream)) {
	boError() << k_funcinfo << "saving script failed" << endl;
	return false;
 }
 return true;
}

bool BoEventListener::loadScript(const QByteArray& script, const QByteArray& scriptData)
{
 PROFILE_METHOD
 if (d->mScript) {
	boError() << k_funcinfo << "script already loaded" << endl;
	return false;
 }

 d->mEventHandlers.setAutoDelete(true);
 d->mEventHandlers.clear();

 // AB: scriptData contains script + variable values. this is available when
 // loading saved games.
 // if scriptData is empty, we use script instead (e.g. a .bpf file)
 if (scriptData.size() != 0) {
	d->mScript = createScriptParser();

	QDataStream stream(scriptData, IO_ReadOnly);
	return d->mScript->load(stream);
 }

 if (script.size() == 0) {
	return true;
 }

 bool ret = true;
 d->mScript = createScriptParser();
 if (!d->mScript->loadScriptFromString(script)) {
	boError() << k_funcinfo << "unable to load script from string" << endl;
	ret = false;
	// we don't return, maybe only some parts of the script are buggy.
	// if we'd return without calling init(), we might cause even more
	// trouble when other script functions will be called.
 }

 connect(d->mScript->interface(), SIGNAL(signalAddEventHandler(const QString&, const QString&, const QString&, int*)),
		this, SLOT(addEventHandler(const QString&, const QString&, const QString&, int*)));
 connect(d->mScript->interface(), SIGNAL(signalRemoveEventHandler(int)),
		this, SLOT(removeEventHandler(int)));

 d->mScript->init();

 BoEventHandlerInfo* info = new BoEventHandlerInfo;
 info->function = "advance";
 info->eventName = "Advance";
 info->args = "";
 d->mEventHandlers.insert(1, info);
 d->mNextEventHandlerId = 2;


 return ret;
}

BosonScript* BoEventListener::script() const
{
 return d->mScript;
}

bool BoEventListener::saveConditions(QDomElement& root) const
{
 QDomDocument doc = root.ownerDocument();

 QPtrListIterator<BoCondition> it(d->mConditions);
 while (it.current()) {
	QDomElement e = doc.createElement("Condition");
	if (!it.current()->saveAsXML(e)) {
		boError(360) << k_funcinfo << "unable to save condition" << endl;
		return false;
	}
	root.appendChild(e);
	++it;
 }
 return true;
}

bool BoEventListener::loadConditions(const QDomElement& root)
{
 boDebug(360) << k_funcinfo << endl;
 d->mConditions.clear();
 QDomNodeList list = root.elementsByTagName("Condition");
 for (unsigned int i = 0; i < list.count(); i++) {
	QDomElement e = list.item(i).toElement();
	if (e.isNull()) {
		boError(360) << k_funcinfo << "not a valid element" << endl;
		return false;
	}
	BoCondition* c = new BoCondition();
	if (!c->loadFromXML(e)) {
		boError(360) << k_funcinfo << "unable to load condition" << endl;
		delete c;
		return false;
	}
	if (!addCondition(c)) {
		// already deleted
		boError(360) << k_funcinfo << "unable to add condition!" << endl;
		return false;
	}
 }
 boDebug(360) << k_funcinfo << "done" << endl;
 return true;
}

bool BoEventListener::saveEventHandlers(QDomElement& root) const
{
 QDomDocument doc = root.ownerDocument();

 QIntDictIterator<BoEventHandlerInfo> it(d->mEventHandlers);
 for( ; it.current(); ++it) {
	BoEventHandlerInfo* info = it.current();
	QDomElement e = doc.createElement("EventHandler");
	e.setAttribute("id", it.currentKey());
	e.setAttribute("EventName", info->eventName);
	e.setAttribute("FunctionName", info->function);
	e.setAttribute("FunctionArgs", info->args);
	root.appendChild(e);
 }

 root.setAttribute("NextId", d->mNextEventHandlerId);
 return true;
}

bool BoEventListener::loadEventHandlers(const QDomElement& root)
{
 boDebug(360) << k_funcinfo << endl;
 d->mNextEventHandlerId = 1;
 bool ok = false;
 QDomNodeList list = root.elementsByTagName("EventHandler");
 if (list.count() > 0) {
	if (!d->mEventHandlers.isEmpty()) {
		// AB:
		// * EventHandlers are added by the script. Either in init() or
		//   at a later point.
		//   When starting a new game, loadEventHandlers() should be a
		//   noop (i.e. no "EventHandler" tag should be around)
		// * When loading a game (.bsg file), d->mScript->init() is not
		//   called and therefore the original eventhandlers are not
		//   added by the script. Only those that were saved previously
		//   should be added. This is what this method does.
		boError() << k_funcinfo << "already loaded eventhandlers .. maybe init() was called? or maybe the xml file in a .bpf archive contains eventhandlers?" << endl;
		return false;
	}
 }
 for (unsigned int i = 0; i < list.count(); i++) {
	QDomElement e = list.item(i).toElement();
	if (e.isNull()) {
		boError(360) << k_funcinfo << "not a valid element" << endl;
		return false;
	}
	BoEventHandlerInfo* info = new BoEventHandlerInfo;
	info->eventName = e.attribute("EventName");
	info->function = e.attribute("FunctionName");
	info->args = e.attribute("FunctionArgs");
	bool ok = false;
	int id = e.attribute("id").toLong(&ok);
	if (!ok) {
		boError(360) << k_funcinfo << "Invalid id!" << endl;
		delete info;
		return false;
	}
	d->mEventHandlers.insert(id, info);
 }
 int nextid = root.attribute("NextId").toLong(&ok);
 if (!ok) {
	boError(360) << k_funcinfo << "Invalid NextId!" << endl;
	return false;
 }
 d->mNextEventHandlerId = nextid;
 boDebug(360) << k_funcinfo << "done" << endl;
 return true;
}

void BoEventListener::deliverToConditions(const BoEvent* event)
{
 PROFILE_METHOD
// boDebug(360) << k_funcinfo << "conditions: " << d->mConditions.count() << endl;
 QPtrList<BoCondition> remove;
 QPtrListIterator<BoCondition> it(d->mConditions);
 while (it.current()) {
	it.current()->processEvent(event);
	if (it.current()->conditionDone(d->mScript)) {
		it.current()->fireAction();
		if (!it.current()->reset()) {
			remove.append(it.current());
		}
	}
	++it;
 }
 while (!remove.isEmpty()) {
	BoCondition* c = remove.take(0);
	d->mConditions.removeRef(c);
 }
// boDebug(360) << k_funcinfo << "done" << endl;
}

void BoEventListener::deliverToScript(const BoEvent* event)
{
 PROFILE_METHOD
 if (!d->mScript) {
	if (d->mEventHandlers.count() > 0) {
		boError() << k_funcinfo << "have eventhandlers, but d->mScript is NULL" << endl;
	}
	return;
 }
 QIntDictIterator<BoEventHandlerInfo> it(d->mEventHandlers);
 for( ; it.current(); ++it) {
	BoEventHandlerInfo* info = it.current();

	if (info->eventName.latin1() == event->name()) {
		d->mScript->callEventHandler(event, info->function, info->args);
	}
 }
}

void BoEventListener::receiveEvent(const BoEvent* event)
{
 PROFILE_METHOD
 BO_CHECK_NULL_RET(event);
 deliverToConditions(event);

 deliverToScript(event);

 // AB: we might split the event handling up now, e.g. all player events go to
 // processPlayerEvent(), the rest to processEvent().
 processEvent(event);
}

void BoEventListener::addEventHandler(const QString& eventName, const QString& functionName, const QString& args, int* id)
{
 BoEventHandlerInfo* info = new BoEventHandlerInfo;
 info->function = functionName;
 info->eventName = eventName;
 info->args = args;
 d->mEventHandlers.insert(d->mNextEventHandlerId, info);
 *id = d->mNextEventHandlerId;
 d->mNextEventHandlerId++;
}

void BoEventListener::removeEventHandler(int id)
{
 d->mEventHandlers.setAutoDelete(true);
 d->mEventHandlers.remove(id);
}


// TODO: there may be only one instance of this class
BoCanvasEventListener::BoCanvasEventListener(BoEventManager* manager, QObject* parent)
	: BoEventListener(manager, parent)
{
}

BoCanvasEventListener::~BoCanvasEventListener()
{
}

BosonScript* BoCanvasEventListener::createScriptParser() const
{
 PROFILE_METHOD
 return BosonScript::newScriptParser(BosonScript::Python, -1);
}

void BoCanvasEventListener::processEvent(const BoEvent* event)
{
 PROFILE_METHOD
 if (event->name() == "AllUnitsDestroyed") {
	Player* p = (Player*)boGame->findPlayerByUserId(event->playerId());
	if (!p) {
		boError(360) << k_funcinfo << "could not find specified player " << event->playerId() << endl;
		return;
	}
	boGame->slotAddChatSystemMessage(i18n("%1 has lost all units").arg(p->name()));

	checkGameOverAndEndGame();
 } else if (event->name() == "PlayerLost") {
	Player* p = (Player*)boGame->findPlayerByUserId(event->playerId());
	if (!p) {
		boError(360) << k_funcinfo << "could not find specified player " << event->playerId() << endl;
		return;
	}
	boDebug() << k_funcinfo << "PlayerLost event received for player " << event->playerId() << endl;
	p->setHasLost(true);
	p->setHasWon(false);

	// AB: atm "PlayerLost" also implies that player is out of the game
	boGame->killPlayer(p);
	p->setOutOfGame();

	boGame->slotAddChatSystemMessage(i18n("%1 has lost and is out of the game").arg(p->name()));
 } else if (event->name() == "PlayerWon") {
	Player* p = (Player*)boGame->findPlayerByUserId(event->playerId());
	if (!p) {
		boError(360) << k_funcinfo << "could not find specified player " << event->playerId() << endl;
		return;
	}
	boDebug() << k_funcinfo << "PlayerWon event received for player " << event->playerId() << endl;
	p->setHasLost(false);
	p->setHasWon(true);
 } else if (event->name() == "GameOver") {
	boDebug() << k_funcinfo << endl;
	emit signalGameOver();
 } else if (event->name() == "CustomStringEvent") {
	boGame->slotAddChatSystemMessage(i18n("Received CustomStringEvent - parameter1: %1").arg(event->data1()));
 }
}

// AB: here we could do several nice things
// * provide an XML file (and a dialog in the editor) to define winning
//   conditions
// * provide a python script function to test for winning conditions
bool BoCanvasEventListener::checkGameOver(QPtrList<Player>* fullfilledWinningConditions) const
{
 // AB: we could use some condition object here: once all events from the
 // condition are received, the player has won (or lost) and once a won/lost
 // from all players is received
 //
 // -> however atm we just poll for number of units of all players

 boDebug() << k_funcinfo << endl;
 // AB: atm we use the winning condition "at most 1 player left in game"
 QPtrList<Player> fullfilled;

 QPtrList<Player> activeGamePlayerList = boGame->activeGamePlayerList();
 for (unsigned int i = 0; i < activeGamePlayerList.count(); i++) {
	Player* p = activeGamePlayerList.at(i);
	if (p->bosonId() <= 127 || p->bosonId() >= 256) {
		boError() << k_funcinfo << "not an active game player: " << p->bosonId() << endl;
		continue;
	}
	if (p->allUnits()->count() > 0) {
		fullfilled.append(p);
		boDebug() << "FULLFILLED: " << p->bosonId() << endl;
	}
 }
 if (fullfilled.count() <= 1) {
	// at most one player with units left - game is over, remaining player has won
	if (fullfilledWinningConditions) {
		fullfilledWinningConditions->clear();
		if (fullfilled.count() == 1) {
			fullfilledWinningConditions->append(fullfilled.at(0));
		}
	}
	return true;
 }
 return false;
}

void BoCanvasEventListener::checkGameOverAndEndGame()
{
 QPtrList<Player> fullfilledWinningConditions;
 if (checkGameOver(&fullfilledWinningConditions)) {
	QPtrList<Player> activeGamePlayerList = boGame->activeGamePlayerList();
	for (unsigned int i = 0; i < activeGamePlayerList.count(); i++) {
		Player* p = activeGamePlayerList.at(i);
		if (fullfilledWinningConditions.contains(p)) {
			BoEvent* won = new BoEvent("PlayerWon");
			won->setPlayerId(p->bosonId());
			boGame->queueEvent(won);
		} else {
			BoEvent* lost = new BoEvent("PlayerLost");
			lost->setPlayerId(p->bosonId());
			boGame->queueEvent(lost);
		}
	}
	BoEvent* gameOver = new BoEvent("GameOver");

	// We use a "fadeout" time of 100 advance calls.
	// This is meant to make sure that the final explosions are actually
	// displayed, and that the player can watch his units to leave the enemy
	// base again or so (this is "nice to look at").
	gameOver->setDelayedDelivery(100);

	boGame->queueEvent(gameOver);
 }
}



BoComputerPlayerEventListener::BoComputerPlayerEventListener(Player* p, BoEventManager* manager, QObject* parent)
	: BoEventListener(manager, parent)
{
 mPlayerIO = p->playerIO();
}

BoComputerPlayerEventListener::~BoComputerPlayerEventListener()
{
}

BosonScript* BoComputerPlayerEventListener::createScriptParser() const
{
 PROFILE_METHOD
 int playerId = -1;
 if (!playerIO()) {
	BO_NULL_ERROR(playerIO());
	// really bad error.
	// fortunately it should be impossible anyway :)
 } else {
	playerId = playerIO()->playerId();
 }

 return BosonScript::newScriptParser(BosonScript::Python, playerId);
}

void BoComputerPlayerEventListener::processEvent(const BoEvent* event)
{
 PROFILE_METHOD
 BO_CHECK_NULL_RET(playerIO());
 BO_CHECK_NULL_RET(playerIO()->speciesTheme());
 if (event->playerId() != 0) {
	if (event->playerId() != playerIO()->playerId()) {
		return;
	}
 }
}

bool BoComputerPlayerEventListener::canSee(const BoEvent* event) const
{
 return mPlayerIO->canSee(event->location());
}

QString BoComputerPlayerEventListener::scriptFileName() const
{
 if (!playerIO()) {
	BO_NULL_ERROR(playerIO());
	return QString();
 }
 if (!playerIO()->player()) {
	BO_NULL_ERROR(playerIO()->player());
	return QString();
 }
 return QString("ai-player_%1.py").arg(playerIO()->player()->bosonId());
}

QString BoComputerPlayerEventListener::xmlFileName() const
{
 if (!playerIO()) {
	BO_NULL_ERROR(playerIO());
	return QString();
 }
 if (!playerIO()->player()) {
	BO_NULL_ERROR(playerIO()->player());
	return QString();
 }
 return QString("ai-player_%1.xml").arg(playerIO()->player()->bosonId());
}

