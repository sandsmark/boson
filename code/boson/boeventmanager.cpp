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
#include "bodebug.h"

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
	QPtrList<BoEventListener> mEventListeners;

	QValueVector<QCString> mEventNames;
};

BoEventManager::BoEventManager(QObject* parent) : QObject(parent)
{
 d = new BoEventManagerPrivate;
 d->mEvents.setAutoDelete(true);

 d->mProperties = new KGamePropertyHandler(this);
 d->mNextEventId.registerData(IdNextEvent, d->mProperties,
		 KGamePropertyBase::PolicyLocal, "NextEventId");
 d->mNextEventId.setLocal(1); // 0 is an invalid event id!

 declareEvents();
}

BoEventManager::~BoEventManager()
{
 delete d;
}

void BoEventManager::declareEvents()
{
#define BO_DECLARE_EVENT(name) d->mEventNames.append(QCString(#name));
 BO_DECLARE_EVENT(UnitWithTypeProduced);
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

 BO_DECLARE_EVENT(PlayerLost);

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
 if (!scripts->isEmpty()) {
	boError() << k_funcinfo << "scripts map must be empty" << endl;
	return false;
 }
 QPtrListIterator<BoEventListener> it(d->mEventListeners);
 for (; it.current(); ++it) {
	QByteArray script;
	QByteArray scriptData;
	if (!it.current()->saveScript(&script)) {
		boError() << k_funcinfo << "unable to save listener script" << endl;
		return false;
	}
	if (!it.current()->saveScriptData(&scriptData)) {
		boError() << k_funcinfo << "unable to save listener script data" << endl;
		return false;
	}

	QString name = it.current()->scriptFileName();
	if (name.isEmpty()) {
		boError() << k_funcinfo << "listener did not return a name" << endl;
		return false;
	}

	QString scriptName = QString::fromLatin1("scripts/") + name;
	QString scriptDataName = QString::fromLatin1("scripts/data/") + name;
	if (scripts->contains(scriptName)) {
		boError() << k_funcinfo << "script with name " << scriptName << " already present" << endl;
		return false;
	}
	if (scripts->contains(scriptDataName)) {
		boError() << k_funcinfo << "script with name " << scriptDataName << " already present" << endl;
		return false;
	}
	scripts->insert(scriptName, script);
	scripts->insert(scriptDataName, scriptData);
 }
 return true;
}

bool BoEventManager::loadListenerScripts(const QMap<QString, QByteArray>& scripts)
{
 QPtrListIterator<BoEventListener> it(d->mEventListeners);
 for (; it.current(); ++it) {
	QString name = it.current()->scriptFileName();
	if (name.isEmpty()) {
		boError() << k_funcinfo << "script filename is empty" << endl;
		return false;
	}
	QByteArray script = scripts[QString::fromLatin1("scripts/") + name];
	QByteArray scriptData = scripts[QString::fromLatin1("scripts/data/") + name];
	if (scriptData.size() == 0) {
		if (!it.current()->loadScript(script)) {
			boError() << k_funcinfo << "unable to load listener script" << endl;
			return false;
		}
	} else {
		if (!it.current()->loadScriptData(scriptData)) {
			boError() << k_funcinfo << "unable to load listener script data" << endl;
			return false;
		}
	}
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
 delete event;
}

void BoEventManager::advance()
{
 QPtrList<BoEvent> remainingEvents;
 while (!d->mEvents.isEmpty()) {
	BoEvent* e = d->mEvents.take(0);
	if (e->delayedDelivery() == 0) {
		deliverEvent(e); // deletes the event
	} else {
		e->setDelayedDelivery(e->delayedDelivery() - 1);
		remainingEvents.append(e);
	}
 }
 d->mEvents = remainingEvents;
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

