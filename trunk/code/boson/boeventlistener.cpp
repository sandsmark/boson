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
#include "speciestheme.h"
#include "player.h"

#include <klocale.h>

#include <qptrlist.h>
#include <qdom.h>

class BoEventListenerPrivate
{
public:
	BoEventListenerPrivate()
	{
		mScript = 0;
	}
	QPtrList<BoCondition> mConditions;

	BosonScript* mScript;
};

BoEventListener::BoEventListener(const QString& scriptFile, BoEventManager* manager, QObject* parent)
	: QObject(parent)
{
 d = new BoEventListenerPrivate;
 d->mConditions.setAutoDelete(true);
 mScriptFileName = scriptFile;
 mManager = manager;
 mManager->addEventListener(this);
}

BoEventListener::~BoEventListener()
{
 if (mManager) {
	mManager->removeEventListener(this);
 }
 d->mConditions.clear();
 delete d;
}

bool BoEventListener::addCondition(BoCondition* c)
{
 boDebug(360) << k_funcinfo << endl;
 d->mConditions.append(c);
 return true;
}

bool BoEventListener::save(QDomElement& root) const
{
 QDomDocument doc = root.ownerDocument();
 QDomElement conditions = doc.createElement("Conditions");
 root.appendChild(conditions);
 return saveConditions(conditions);
}

bool BoEventListener::load(const QDomElement& root)
{
 QDomElement conditions = root.namedItem("Conditions").toElement();
 if (conditions.isNull()) {
	boError(360) << k_funcinfo << "No Conditions tag" << endl;
	return false;
 }
 return loadConditions(conditions);
}

#warning TODO
// TODO: BosonScript::save() saves the script _and_ the data of the script. we
// should save the script _only_ here
#define BOSONSCRIPT_LACKS_SCRIPT_SAVE 1

bool BoEventListener::saveScript(QByteArray* script) const
{
 if (!d->mScript) {
	// this event listener doesn't use scripts.
	*script = QByteArray();
	return true;
 }
#if BOSONSCRIPT_LACKS_SCRIPT_SAVE
 *script = QByteArray();
 QDataStream stream(*script, IO_WriteOnly);
 if (!d->mScript->save(stream)) {
	boError() << k_funcinfo << "saving script failed" << endl;
	return false;
 }
#else
 // this should just save the .py file into a string and return it.
 *script = d->mScript->saveScriptFile();
#endif
 return true;
}

bool BoEventListener::loadScript(const QByteArray& script)
{
 if (script.size() == 0) {
	return true;
 }
#if BOSONSCRIPT_LACKS_SCRIPT_SAVE
 ensureScriptInterpreter();
 QDataStream stream(script, IO_ReadOnly);
 return d->mScript->load(stream);
#else
 mScriptString = script;
 ensureScriptInterpreter(); // loads the script

 // FIXME: maybe we should return the result of loadScriptFromString(), which is
 // in ensureScriptInterpreter()
 return true;
#endif
}

bool BoEventListener::saveScriptData(QByteArray* script) const
{
 if (!d->mScript) {
	// this event listener doesn't use scripts.
	*script = QByteArray();
	return true;
 }
 *script = QByteArray();
 QDataStream stream(*script, IO_WriteOnly);
 if (!d->mScript->save(stream)) {
	boError() << k_funcinfo << "saving script failed" << endl;
	return false;
 }
 return true;
}

bool BoEventListener::loadScriptData(const QByteArray& script)
{
 if (script.size() == 0) {
	return true;
 }
 ensureScriptInterpreter();
 QDataStream stream(script, IO_ReadOnly);
 return d->mScript->load(stream);
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
	if (c->requireScript()) {
		ensureScriptInterpreter();
	}
 }
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

void BoEventListener::receiveEvent(const BoEvent* event)
{
 BO_CHECK_NULL_RET(event);
 deliverToConditions(event);

 // AB: we might split the event handling up now, e.g. all player events go to
 // processPlayerEvent(), the rest to processEvent().
 processEvent(event);
}

void BoEventListener::ensureScriptInterpreter()
{
 if (d->mScript) {
	return;
 }
 if (mScriptString.isEmpty()) {
	boWarning() << k_funcinfo << "empty script string" << endl;
	// do NOT return. we expect d->mScript being non-NULL when this method
	// returns
 }

 d->mScript = createScriptParser();

 d->mScript->loadScriptFromString(mScriptString);
 d->mScript->init();
}


// TODO: there may be only one instance of this class
BoCanvasEventListener::BoCanvasEventListener(BoEventManager* manager, QObject* parent)
	: BoEventListener(QString::fromLatin1("game.py"), manager, parent)
{
}

BoCanvasEventListener::~BoCanvasEventListener()
{
}

BosonScript* BoCanvasEventListener::createScriptParser() const
{
 return BosonScript::newScriptParser(BosonScript::Python, 0);
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
	: BoEventListener(QString::fromLatin1("localplayer.py"), manager, parent)
{
 mPlayerIO = io;
}

BoLocalPlayerEventListener::~BoLocalPlayerEventListener()
{
}

BosonScript* BoLocalPlayerEventListener::createScriptParser() const
{
 Player* p = 0;
 if (!playerIO()) {
	BO_NULL_ERROR(playerIO());
	// really bad error.
	// fortunately it should be impossible anyway :)
 } else {
	p = playerIO()->player();
 }
 return BosonScript::newScriptParser(BosonScript::Python, p);
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

