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

#include "bocondition.h"

#include "boevent.h"
#include "boeventmatching.h"
#include "boson.h" // boGame->queueEvent()
#include "bodebug.h"
#include "script/bosonscript.h"

#include <qvaluelist.h>
#include <qptrlist.h>
#include <qdom.h>

class BoStatusCondition
{
public:
	BoStatusCondition(const QString& function)
	{
		mFunction = function;
	}
	BoStatusCondition()
	{
	}

	~BoStatusCondition()
	{
	}

	bool save(QDomElement& root) const
	{
		root.setAttribute("Function", mFunction);
		return true;
	}
	bool load(const QDomElement& root)
	{
		if (!root.hasAttribute("Function")) {
			return false;
		}
		mFunction = root.attribute("Function");
		return true;
	}

	bool isFullfilled(BosonScript* script) const
	{
		int ret = script->callFunctionWithReturn(mFunction);
		if (ret == 1) {
			return true;
		}
		return false;
	}

private:
	QString mFunction;
};

class BoConditionPrivate
{
public:
	BoConditionPrivate()
	{
	}

	// only the "main" disjunction has pointers to the other ones. the
	// "main" disjunction is the one in the event manager.
	QPtrList<BoCondition> mAlternatives; // other disjunctions, i.e. ORs

	// TODO: negations.
	// if event X was received then condition is FALSE.
	// (alternatives might still be TRUE)
	// e.g. ((E0 AND !E1) OR (E0 AND E2)) means "either E0 was received and
	// E1 was NOT received, or E0 and E2 were received"
	QPtrList<BoEventMatching> mEvents;
	QPtrList<BoEventMatching> mEventsLeft;

	QPtrList<BoStatusCondition> mStatusConditions;
};

BoCondition::BoCondition()
{
 d = new BoConditionPrivate;
 d->mAlternatives.setAutoDelete(true);
 d->mEvents.setAutoDelete(true);
 d->mStatusConditions.setAutoDelete(true);
}

BoCondition::~BoCondition()
{
 d->mAlternatives.clear();
 d->mEventsLeft.clear();
 d->mEvents.clear();
 d->mStatusConditions.clear();
 delete d;
}


void BoCondition::processEvent(const BoEvent* event)
{
 QPtrListIterator<BoCondition> orIt(d->mAlternatives);
 while (orIt.current()) {
	orIt.current()->processEvent(event);
	++orIt;
 }

 QPtrListIterator<BoEventMatching> it(d->mEventsLeft);
 BoEventMatching* match = 0;
 while (it.current() && !match) {
	if (it.current()->matches(event)) {
		match = it.current();
	}
	++it;
 }
 if (!match) {
	return;
 }
 d->mEventsLeft.removeRef(match);

 // this is for the (rare) case that a single event matches more than
 // once. should not happen though.
 processEvent(event);
}

bool BoCondition::save(QDomElement& root, const QMap<int, int>* playerId2Index) const
{
 QDomDocument doc = root.ownerDocument();
 QDomElement eventsLeft = doc.createElement("Events");
 root.appendChild(eventsLeft);
 QPtrListIterator<BoEventMatching> it(d->mEvents);
 while (it.current()) {
	QDomElement m = doc.createElement("EventMatching");
	if (!it.current()->save(m, playerId2Index)) {
		boError() << k_funcinfo << "cannot save EventMatching" << endl;
		return false;
	}
	eventsLeft.appendChild(m);
	if (d->mEventsLeft.contains(it.current())) {
		m.setAttribute("IsLeft", QString::number(1));
	} else {
		m.setAttribute("IsLeft", QString::number(0));
	}
	++it;
 }

 QDomElement statusConditions = doc.createElement("StatusConditions");
 root.appendChild(statusConditions);
 QPtrListIterator<BoStatusCondition> statIt(d->mStatusConditions);
 while (statIt.current()) {
	QDomElement e = doc.createElement("StatusCondition");
	if (!statIt.current()->save(e)) {
		boError() << k_funcinfo << "unable to save StatusCondition" << endl;
		return false;
	}
	statusConditions.appendChild(e);
	++statIt;
 }

 root.setAttribute("EventCaused", mEventCaused);

 return true;
}

bool BoCondition::load(const QDomElement& root)
{
 QDomElement events = root.namedItem("Events").toElement();
 if (events.isNull()) {
	boError() << k_funcinfo << "No Events tag" << endl;
	return false;
 }
 d->mEvents.clear();
 d->mEventsLeft.clear();
 QDomNodeList matchings = events.elementsByTagName("EventMatching");
 for (unsigned int i = 0; i < matchings.count(); i++) {
	QDomElement m = matchings.item(i).toElement();
	if (m.isNull()) {
		boError() << k_funcinfo << "not an element" << endl;
		return false;
	}
	BoEventMatching* matching = new BoEventMatching();
	if (!matching->load(m)) {
		boError() << k_funcinfo << "cannot load EventMatching " << i << endl;
		delete matching;
		return false;
	}
	d->mEvents.append(matching);
	bool ok;
	if (m.attribute("IsLeft").toInt(&ok) == 1) {
		d->mEventsLeft.append(matching);
	}
	if (!ok) {
		boError() << k_funcinfo << "IsLeft is not a valid number" << endl;
		return false;
	}
 }

 QDomElement statusConditions = root.namedItem("StatusConditions").toElement();
 if (statusConditions.isNull()) {
	boError() << k_funcinfo << "no StatusConditions tag" << endl;
	return false;
 }
 QDomNodeList statuses = statusConditions.elementsByTagName("StatusCondition");
 for (unsigned int i = 0; i < statuses.count(); i++) {
	QDomElement e = statuses.item(i).toElement();
	BoStatusCondition* stat = new BoStatusCondition();
	if (!stat->load(e)) {
		boError() << k_funcinfo << "unable to load StatusCondition" << endl;
		delete stat;
		return false;
	}
	d->mStatusConditions.append(stat);
 }

 if (!root.hasAttribute("EventCaused")) {
	boError() << k_funcinfo << "no EventCaused attribute" << endl;
	return false;
 }
 mEventCaused = root.attribute("EventCaused");

 return true;
}

bool BoCondition::conditionDone(BosonScript* script) const
{
 if (thisConditionDone(script)) {
	return true;
 }
 if (!d->mAlternatives.isEmpty()) {
	QPtrListIterator<BoCondition> it(d->mAlternatives);
	while (it.current()) {
		if (it.current()->thisConditionDone(script)) {
			return true;
		}
		++it;
	}
 }
 return false;
}

bool BoCondition::thisConditionDone(BosonScript* script) const
{
 if (!d->mEventsLeft.isEmpty()) {
	return false;
 }
 if (d->mStatusConditions.isEmpty()) {
	return true;
 }
 if (!script) {
	BO_NULL_ERROR(script);
	return false;
 }
 QPtrListIterator<BoStatusCondition> it(d->mStatusConditions);
 while (it.current()) {
	if (!it.current()->isFullfilled(script)) {
		return false;
	}
	++it;
 }
 return true;
}

void BoCondition::fireAction()
{
 boDebug() << k_funcinfo << endl;
 // the "eventCaused()" is actually the parameter of the event.
 // TODO: use an action class instead, probably BoConditionalAction.
 // -> this could be an even (including parameters), a chat message, a script
 // function, ...
 BoEvent* e = new BoEvent("CustomStringEvent", eventCaused());
 boGame->queueEvent(e);
}

bool BoCondition::requireScript() const
{
 if (d->mStatusConditions.isEmpty()) {
	return false;
 }
 return true;
}

bool BoCondition::reset()
{
 // some conditions might match only once, i.e. as soon as they matched, they
 // should be removed. in that case we don't reset the condition, but remove it
 // instead (by returning false here).
 bool ret = true;
 if (!ret) {
	return ret;
 }

 d->mEventsLeft.clear();
 QPtrListIterator<BoEventMatching> it(d->mEvents);
 while (it.current()) {
	d->mEventsLeft.append(it.current());
	++it;
 }
 return ret;
}
