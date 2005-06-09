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

#include "boeventmanager.h"
#include "boeventmanager.moc"

#include "boevent.h"
#include "boeventlistener.h"
#include "boson.h"
#include "bosonpropertyxml.h"
#include "bosonprofiling.h"
#include "bodebug.h"
#include "bosonmessageids.h"

#include <kgame/kgameproperty.h>
#include <kgame/kgamepropertyhandler.h>
#include <kgame/kplayer.h> // KPlayer::id()

#include <qdom.h>
#include <qvaluevector.h>
#include <qmap.h>

#include <stdlib.h>


class BoEventManagerPrivate
{
public:
	BoEventManagerPrivate()
	{
	}
	KGamePropertyHandler* mProperties;
	KGameProperty<unsigned long int> mNextEventId;

	QPtrList<BoEvent> mEvents;
	QPtrList<BoEvent> mStatusEvents;
	QPtrList<BoEventListener> mEventListeners;

	QValueVector<QCString> mEventNames;

	QMap<QString, QByteArray> mAvailableScripts;
};

BoEventManager::BoEventManager(QObject* parent) : QObject(parent)
{
 d = new BoEventManagerPrivate;
 d->mEvents.setAutoDelete(true);
 d->mStatusEvents.setAutoDelete(true);

 d->mProperties = new KGamePropertyHandler(this);
 d->mNextEventId.registerData(IdNextEvent, d->mProperties,
		 KGamePropertyBase::PolicyLocal, "NextEventId");
 d->mNextEventId.setLocal(1); // 0 is an invalid event id!

 declareEvents();
}

BoEventManager::~BoEventManager()
{
 d->mAvailableScripts.clear();
 delete d;
}

void BoEventManager::declareEvents()
{
#define BO_DECLARE_EVENT(name) d->mEventNames.append(QCString(#name));
 BO_DECLARE_EVENT(UnitWithTypeProduced);
 BO_DECLARE_EVENT(FacilityWithTypeConstructed);
 BO_DECLARE_EVENT(UnitWithTypeDestroyed);
 BO_DECLARE_EVENT(AllUnitsDestroyed);
 BO_DECLARE_EVENT(LostMinimap);
 BO_DECLARE_EVENT(GainedMinimap);

 BO_DECLARE_EVENT(AllMobileUnitsDestroyed);
 BO_DECLARE_EVENT(AllFacilitiesDestroyed);
 BO_DECLARE_EVENT(AllUnitsWithTypeDestroyed);
 BO_DECLARE_EVENT(ProducedUnitWithTypePlaced);
 BO_DECLARE_EVENT(UnitsWithTypeDestroyedCount);
 BO_DECLARE_EVENT(EnemyUnitsWithTypeDestroyedCount);
 BO_DECLARE_EVENT(UnitsWithTypeLostCount);
 BO_DECLARE_EVENT(UnitWithIdUnfogged);
 BO_DECLARE_EVENT(UnitsWithTypeUnfogged);
 BO_DECLARE_EVENT(TechnologyWithTypeResearched);

 BO_DECLARE_EVENT(PlayerLost);

 BO_DECLARE_EVENT(Advance);

 BO_DECLARE_EVENT(CustomEvent);
 BO_DECLARE_EVENT(CustomStringEvent);
#undef BO_DECLARE_EVENT
 qHeapSort(d->mEventNames);
}

bool BoEventManager::saveAsXML(QDomElement& root) const
{
 QDomDocument doc = root.ownerDocument();
 QDomElement handler = doc.createElement(QString::fromLatin1("DataHandler"));
 root.appendChild(handler);
 BosonPropertyXML propertyXML;
 if (!propertyXML.saveAsXML(handler, d->mProperties)) {
	boError(360) << k_funcinfo << "Unable to save DataHandler" << endl;
	return false;
 }

 QDomElement events = doc.createElement(QString::fromLatin1("EventQueue"));
 root.appendChild(events);

 QMap<int, int> playerId2Index;
 QPtrListIterator<KPlayer> playerIt(*boGame->playerList());
 while (playerIt.current()) {
	int index = boGame->playerList()->findRef(playerIt.current());
	playerId2Index.insert(playerIt.current()->id(), index);
	++playerIt;
 }
 QPtrListIterator<BoEvent> it(d->mEvents);
 while (it.current()) {
	QDomElement e = doc.createElement(QString::fromLatin1("Event"));
	if (!it.current()->save(e, &playerId2Index)) {
		boError(360) << k_funcinfo << "error saving event" << endl;
		return false;
	}
	events.appendChild(e);
	++it;
 }
 return true;
}

bool BoEventManager::loadFromXML(const QDomElement& root)
{
 d->mEvents.clear();
 QDomElement handler = root.namedItem(QString::fromLatin1("DataHandler")).toElement();
 if (handler.isNull()) {
	boError(360) << k_funcinfo << "DataHandler not found" << endl;
	return false;
 }
 BosonPropertyXML propertyXML;
 if (!propertyXML.loadFromXML(handler, d->mProperties)) {
	boError(360) << k_funcinfo << "Unable to load from DataHandler" << endl;
	return false;
 }
 QDomElement events = root.namedItem(QString::fromLatin1("EventQueue")).toElement();
 if (events.isNull()) {
	boError(360) << k_funcinfo << "no EvenetQueue found" << endl;
	return false;
 }
 QDomNodeList list = events.elementsByTagName("Event");
 for (unsigned int i = 0; i < list.count(); i++) {
	QDomElement e = list.item(i).toElement();
	if (e.isNull()) {
		boError(360) << k_funcinfo << "not an element" << endl;
		return false;
	}
	BoEvent* event = new BoEvent();
	if (!event->load(e)) {
		boError(360) << k_funcinfo << "could not load event" << endl;
		delete event;
		return false;
	}
	if (!queueEvent(event)) {
		boError(360) << k_funcinfo << "could not queue event" << endl;
		// already deleted!
		return false;
	}
 }
 return true;
}

bool BoEventManager::saveListenerScripts(QMap<QString, QByteArray>* scripts) const
{
 // save in 2 steps.
 // 1. save the actual scripts (d->mAvailableScripts)
 // 2. save the current script data (i.e. the variable values).
 //
 // note that in 1. we may save more scripts than we actually use - e.g. if
 // player n is a human player, we still save the ai script of that player
 // (later we may replace the human player by a computer player).

 // save all available scripts
 for (QMap<QString, QByteArray>::iterator it = d->mAvailableScripts.begin(); it != d->mAvailableScripts.end(); ++it) {
	if (it.key().contains("/data/")) {
		continue;
	}
	scripts->insert(it.key(), it.data());
 }

 // save the current data of all currently used scripts
 QPtrListIterator<BoEventListener> it(d->mEventListeners);
 for (; it.current(); ++it) {
	QString name = it.current()->scriptFileName();
	if (name.isEmpty()) {
		boError() << k_funcinfo << "listener did not return a name" << endl;
		return false;
	}

	QString scriptDataName = QString::fromLatin1("scripts/eventlistener/data/") + name;

	QByteArray scriptData;
	if (!it.current()->saveScriptData(&scriptData)) {
		boError() << k_funcinfo << "unable to save listener script" << endl;
		return false;
	}

	if (scripts->contains(scriptDataName)) {
		boError() << k_funcinfo << "script with name " << scriptDataName << " already present" << endl;
		return false;
	}
	scripts->insert(scriptDataName, scriptData);
 }
 return true;
}

bool BoEventManager::loadListenerScripts(const QMap<QString, QByteArray>& files)
{
 boDebug() << k_funcinfo << endl;
 d->mAvailableScripts.clear();
 for (QMap<QString, QByteArray>::const_iterator it = files.begin(); it != files.end(); ++it) {
	if (it.key().startsWith("scripts/eventlistener/")) {
		d->mAvailableScripts.insert(it.key(), it.data());
	}
 }

 // AB: note that even if we don't need a script from d->mAvailableScripts here
 // now, then we still need to leave it the in memory. we might need it later,
 // e.g. when a human player is replaced by a computer io.

 QPtrListIterator<BoEventListener> it(d->mEventListeners);
 for (; it.current(); ++it) {
	QString name = it.current()->scriptFileName();
	QString fullScriptName = QString::fromLatin1("scripts/eventlistener/") + name;
	if (!d->mAvailableScripts.contains(fullScriptName)) {
		boError() << k_funcinfo << "no script with name " << fullScriptName << " available" << endl;
		return false;
	}
	if (!it.current()->initScript()) {
		boError() << k_funcinfo << "unable to initialize script of event listener" << endl;
		return false;
	}
 }
 return true;
}

bool BoEventManager::loadEventListenerScript(BoEventListener* listener)
{
 if (!listener) {
	BO_NULL_ERROR(listener);
	return false;
 }
 QString name = listener->scriptFileName();
 if (name.isEmpty()) {
	boError() << k_funcinfo << "script filename is empty" << endl;
	return false;
 }
 QString fullScriptName = QString::fromLatin1("scripts/eventlistener/") + name;
 QString fullScriptDataName = QString::fromLatin1("scripts/eventlistener/data/") + name;
 if (!d->mAvailableScripts.contains(fullScriptName)) {
	// event manager hasn't loaded the scripts yet.
	// note this is perfectly valid!
	return true;
 }
 boDebug() << k_funcinfo << "loading listener script " << name << endl;
 QByteArray script = d->mAvailableScripts[fullScriptName];
 QByteArray scriptData = d->mAvailableScripts[fullScriptDataName];
 if (!listener->loadScript(script, scriptData)) {
	boError() << k_funcinfo << "unable to load listener script" << endl;
	return false;
 }
 return true;
}

bool BoEventManager::queueEvent(BoEvent* event)
{
 // this is just to avoid typos!
 // -> it might be a problem if we want to support custom events, and maybe we
 // will remove it.
 if (!knowEventName(event->name())) {
	boError(360) << k_funcinfo << "The event " << event->name() << " has not been declared in the event manager. cannot use this event." << endl;
	delete event;
	return false;
 }
 if (!boGame->gameMode()) {
	// in editor mode we just ignore the event.
	delete event;
	return true;
 }
 boDebug(360) << k_funcinfo << "queue event " << event->name() << endl;
 unsigned long int id = d->mNextEventId;
 d->mNextEventId = d->mNextEventId + 1;
 event->setId(id);

 // the event is valid now.
 d->mEvents.append(event);

 return true;
}

void BoEventManager::deliverEvent(BoEvent* event)
{
 PROFILE_METHOD
 const BoEvent* e = event;
 boDebug(360) << k_funcinfo << e->name() << endl;
 QPtrListIterator<BoEventListener> it(d->mEventListeners);
 while (it.current()) {
	if (e->hasLocation()) {
		if (!it.current()->canSee(e)) {
			++it;
			continue;
		}
	}
	it.current()->receiveEvent(e);
	++it;
 }
 cacheStatusEvent(event);
}

void BoEventManager::cacheStatusEvent(BoEvent* event)
{
 d->mStatusEvents.append(event);
}

void BoEventManager::sendStatusEvents()
{
 // Only ADMIN sends status messages (and only in game mode)
 if (!boGame->gameMode() || !boGame->isAdmin()) {
	return;
 }

 QByteArray b;
 QDataStream stream(b, IO_WriteOnly);
 stream << (Q_UINT32)d->mStatusEvents.count();

 while (!d->mStatusEvents.isEmpty()) {
	BoEvent* e = d->mStatusEvents.take(0);

	stream << e->name();
	stream << (Q_UINT32)e->id();
	stream << (Q_UINT32)e->playerId();
	stream << (Q_UINT32)e->unitId();
	stream << (Q_UINT8)(e->hasLocation() ? 1 : 0);
	if (e->hasLocation()) {
		stream << e->location();
	}
	stream << e->data1().latin1();
	stream << e->data2().latin1();

	delete e;
 }

 boGame->sendMessage(b, BosonMessageIds::IdStatus);
}

void BoEventManager::advance(unsigned int advanceCallsCount)
{
 PROFILE_METHOD
 QPtrList<BoEvent> remainingEvents;
 while (!d->mEvents.isEmpty()) {
	BoEvent* e = d->mEvents.take(0);
	if (e->delayedDelivery() == 0) {
		deliverEvent(e);
	} else {
		e->setDelayedDelivery(e->delayedDelivery() - 1);
		remainingEvents.append(e);
	}
 }
 d->mEvents = remainingEvents;

 if ((advanceCallsCount % 20) == 0) {
	sendStatusEvents();
 }
}

void BoEventManager::addEventListener(BoEventListener* l)
{
 d->mEventListeners.append(l);
}

void BoEventManager::removeEventListener(BoEventListener* l)
{
 d->mEventListeners.removeRef(l);
}

static int compare_cstrings(const void* s1, const void* s2)
{
 const QCString* string1 = (const QCString*)s1;
 const QCString* string2 = (const QCString*)s2;
 return qstrcmp(*string1, *string2);
}

bool BoEventManager::knowEventName(const QCString& name) const
{
 if (d->mEventNames.isEmpty()) {
	return false;
 }
 // d->mEventNames is a sorted array. we make a binary search on it.
 void* e = ::bsearch(&name, d->mEventNames.begin(), d->mEventNames.count(),
		sizeof(QCString), compare_cstrings);
 if (!e) {
	return false;
 }
 return true;
}

