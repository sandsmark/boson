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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "boeventlistener.h"
#include "boeventlistener.moc"

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

#include <klocale.h>

#include <qptrlist.h>
#include <qdom.h>
#include <qintdict.h>


class BoEventHandlerInfo
{
public:
	QString function;
	QString args;
	QString eventname;
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
 mManager = manager;
 mManager->addEventListener(this);
}

BoEventListener::~BoEventListener()
{
 if (mManager) {
	mManager->removeEventListener(this);
 }
 d->mConditions.clear();
 d->mEventHandlers.clear();
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

// TODO: rename to saveAsXML()
bool BoEventListener::save(QDomElement& root) const
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

// TODO: rename to loadFromXML()
bool BoEventListener::load(const QDomElement& root)
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
 if (d->mScript) {
	boError() << k_funcinfo << "script already loaded" << endl;
	return false;
 }

 // AB: scriptData contains script + variable values. this is available when
 // loading saved games.
 // if scriptData is empty, we use script instead (e.g. a .bpf file)
 if (scriptData.size() != 0) {
	d->mScript = createScriptParser();

#warning FIXME: playerId
	// the script may have saved the PlayerId into the script. we somehow
	// must tell it the new Id.
	// (we are loading a saved game here)
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
 info->eventname = "Advance";
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

 QMap<int, int> playerId2Index;
 QPtrListIterator<KPlayer> playerIt(*boGame->playerList());
 while (playerIt.current()) {
	int index = boGame->playerList()->findRef(playerIt.current());
	playerId2Index.insert(playerIt.current()->id(), index);
	++playerIt;
 }
 QPtrListIterator<BoCondition> it(d->mConditions);
 while (it.current()) {
	QDomElement e = doc.createElement("Condition");
	if (!it.current()->save(e, &playerId2Index)) {
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
	if (!c->load(e)) {
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
	e.setAttribute("EventName", info->eventname);
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
 d->mEventHandlers.clear();
 d->mNextEventHandlerId = 1;
 bool ok = false;
 QDomNodeList list = root.elementsByTagName("EventHandler");
 for (unsigned int i = 0; i < list.count(); i++) {
	QDomElement e = list.item(i).toElement();
	if (e.isNull()) {
		boError(360) << k_funcinfo << "not a valid element" << endl;
		return false;
	}
	BoEventHandlerInfo* info = new BoEventHandlerInfo;
	info->eventname = e.attribute("EventName");
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
 boDebug(360) << k_funcinfo << "conditions: " << d->mConditions.count() << endl;
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
 boDebug(360) << k_funcinfo << "done" << endl;
}

void BoEventListener::deliverToScript(const BoEvent* event)
{
 QIntDictIterator<BoEventHandlerInfo> it(d->mEventHandlers);
 for( ; it.current(); ++it) {
	BoEventHandlerInfo* info = it.current();
	if (info->eventname.latin1() == event->name()) {
		d->mScript->callEventHandler(event, info->function, info->args);
	}
 }
}

void BoEventListener::receiveEvent(const BoEvent* event)
{
 BO_CHECK_NULL_RET(event);
 deliverToConditions(event);

 deliverToScript(event);

 // AB: we might split the event handling up now, e.g. all player events go to
 // processPlayerEvent(), the rest to processEvent().
 processEvent(event);
}

void BoEventListener::addEventHandler(const QString& eventname, const QString& functionname, const QString& args, int* id)
{
 BoEventHandlerInfo* info = new BoEventHandlerInfo;
 info->function = functionname;
 info->eventname = eventname;
 info->args = args;
 d->mEventHandlers.insert(d->mNextEventHandlerId, info);
 *id = d->mNextEventHandlerId;
 d->mNextEventHandlerId++;
}

void BoEventListener::removeEventHandler(int id)
{
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
 return BosonScript::newScriptParser(BosonScript::Python, -1);
}

void BoCanvasEventListener::processEvent(const BoEvent* event)
{
 if (event->name() == "AllUnitsDestroyed") {
	// AB: atm the player loses when all of his units are destroyed.
	//     will change when we support winning conditions
	BoEvent* lost = new BoEvent("PlayerLost");
	lost->setPlayerId(event->playerId());
	boGame->queueEvent(lost);
 } else if (event->name() == "PlayerLost") {
	Player* p = (Player*)boGame->findPlayer(event->playerId());
	if (!p) {
		boError(360) << k_funcinfo << "could not find specified player " << event->playerId() << endl;
		return;
	}
	p->setOutOfGame();
	boGame->killPlayer(p);
 } else if (event->name() == "CustomStringEvent") {
	boGame->slotAddChatSystemMessage(i18n("Received CustomStringEvent - parameter1: %1").arg(event->data1()));
 }
}


BoLocalPlayerEventListener::BoLocalPlayerEventListener(PlayerIO* io, BoEventManager* manager, QObject* parent)
	: BoEventListener(manager, parent)
{
 mPlayerIO = io;
}

BoLocalPlayerEventListener::~BoLocalPlayerEventListener()
{
}

BosonScript* BoLocalPlayerEventListener::createScriptParser() const
{
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

void BoLocalPlayerEventListener::processEvent(const BoEvent* event)
{
 BO_CHECK_NULL_RET(playerIO());
 BO_CHECK_NULL_RET(playerIO()->speciesTheme());
 if (event->playerId() != 0) {
	if (event->playerId() != playerIO()->playerId()) {
		return;
	}
 }
 if (event->name() == "UnitWithTypeProduced") {
	bool ok;
	unsigned long int unitType = event->data1().toULong(&ok);
	if (!ok) {
		boError(360) << k_funcinfo << "data1 parameter for UnitWithTypeProduced event is not a valid number: " << event->data1() << endl;
		return;
	}
	const UnitProperties* prop = playerIO()->unitProperties(unitType);
	if (!prop) {
		boError(360) << k_funcinfo << "cannot find unittype " << unitType << " specified in UnitWithTypeProduced" << endl;
		return;
	}
	if (prop->isFacility()) {
		boGame->slotAddChatSystemMessage(
				i18n("A %1 has been produced - place it on the map to start the construction!").arg(prop->name()),
				playerIO()->player());
	} else {
		// actually it has been placed already, but who cares
		// for so much preciseness ;)
		// --> this way we dont say something wrong when placing
		// the unit failed
		boGame->slotAddChatSystemMessage(
				i18n("A %1 has been produced and will be placed on the map now").arg(prop->name()),
				playerIO()->player());
	}
 } else if (event->name() == "LostMinimap") {
	playerIO()->speciesTheme()->playSound(SoundReportMinimapDeactivated);
	playerIO()->emitSignalShowMiniMap(false);
 } else if (event->name() == "GainedMinimap") {
	playerIO()->speciesTheme()->playSound(SoundReportMinimapActivated);
	playerIO()->emitSignalShowMiniMap(true);
 } else if (event->name() == "AllUnitsDestroyed") {
 }
}

bool BoLocalPlayerEventListener::canSee(const BoEvent* event) const
{
 return mPlayerIO->canSee(event->location());
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
 return QString("ai-player_%1.py").arg(playerIO()->player()->id());
}

