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

#include "../bomemory/bodummymemory.h"
#include "boevent.h"
#include "boeventmatching.h"
#include "boson.h" // boGame->queueEvent()
#include "bodebug.h"
#include "script/bosonscript.h"

#include <qvaluelist.h>
#include <qptrlist.h>
#include <qdom.h>


// stores the action that is fired once a condition is fullfilled
class BoConditionAction
{
public:
	BoConditionAction()
	{
		mEventCaused = 0;
	}
	~BoConditionAction()
	{
		delete mEventCaused;
	}

	void fire();

	bool saveActionAsXML(QDomElement& root) const;
	bool loadActionFromXML(const QDomElement& root);

private:
	BoEvent* mEventCaused;
	QString mScriptFile;
	QString mScriptCall;
};

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
 mAction = 0;
}

BoCondition::~BoCondition()
{
 d->mAlternatives.clear();
 d->mEventsLeft.clear();
 d->mEvents.clear();
 d->mStatusConditions.clear();
 delete mAction;
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

bool BoCondition::saveAsXML(QDomElement& root) const
{
 QDomDocument doc = root.ownerDocument();
 QDomElement eventsLeft = doc.createElement("Events");
 root.appendChild(eventsLeft);
 QPtrListIterator<BoEventMatching> it(d->mEvents);
 while (it.current()) {
	QDomElement m = doc.createElement("EventMatching");
	if (!it.current()->saveAsXML(m)) {
		boError(360) << k_funcinfo << "cannot save EventMatching" << endl;
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
		boError(360) << k_funcinfo << "unable to save StatusCondition" << endl;
		return false;
	}
	statusConditions.appendChild(e);
	++statIt;
 }

 if (!mAction) {
	BO_NULL_ERROR(mAction);
	return false;
 }

 QDomElement action = doc.createElement("Action");
 root.appendChild(action);
 if (!mAction->saveActionAsXML(action)) {
	boError(360) << k_funcinfo << "could not save action" << endl;
	return false;
 }

 return true;
}

bool BoCondition::loadFromXML(const QDomElement& root)
{
 QDomElement events = root.namedItem("Events").toElement();
 if (events.isNull()) {
	boError(360) << k_funcinfo << "No Events tag" << endl;
	return false;
 }
 d->mEvents.clear();
 d->mEventsLeft.clear();
 QDomNodeList matchings = events.elementsByTagName("EventMatching");
 for (unsigned int i = 0; i < matchings.count(); i++) {
	QDomElement m = matchings.item(i).toElement();
	if (m.isNull()) {
		boError(360) << k_funcinfo << "not an element" << endl;
		return false;
	}
	BoEventMatching* matching = new BoEventMatching();
	if (!matching->loadFromXML(m)) {
		boError(360) << k_funcinfo << "cannot load EventMatching " << i << endl;
		delete matching;
		return false;
	}
	d->mEvents.append(matching);
	bool ok;
	if (m.attribute("IsLeft").toInt(&ok) == 1) {
		d->mEventsLeft.append(matching);
	}
	if (!ok) {
		boError(360) << k_funcinfo << "IsLeft is not a valid number" << endl;
		return false;
	}
 }

 QDomElement statusConditions = root.namedItem("StatusConditions").toElement();
 if (statusConditions.isNull()) {
	boError(360) << k_funcinfo << "no StatusConditions tag" << endl;
	return false;
 }
 QDomNodeList statuses = statusConditions.elementsByTagName("StatusCondition");
 for (unsigned int i = 0; i < statuses.count(); i++) {
	QDomElement e = statuses.item(i).toElement();
	BoStatusCondition* stat = new BoStatusCondition();
	if (!stat->load(e)) {
		boError(360) << k_funcinfo << "unable to load StatusCondition" << endl;
		delete stat;
		return false;
	}
	d->mStatusConditions.append(stat);
 }

 QDomElement action = root.namedItem("Action").toElement();
 if (action.isNull()) {
	boError(360) << k_funcinfo << "no Action tag" << endl;
	return false;
 }
 delete mAction;
 mAction = new BoConditionAction();
 if (!mAction->loadActionFromXML(action)) {
	boError(360) << k_funcinfo << "unable to load Action" << endl;
	return false;
 }

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
 BO_CHECK_NULL_RET(mAction);
 mAction->fire();
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

bool BoConditionAction::saveActionAsXML(QDomElement& root) const
{
 QDomDocument doc = root.ownerDocument();
 QString type;
 if (mEventCaused) {
	type = "Event";
	QDomElement actionEvent = doc.createElement("Event");
	root.appendChild(actionEvent);
	if (!mEventCaused->saveAsXML(actionEvent)) {
		boError(360) << k_funcinfo << "cannot save event that is caused by condition" << endl;
		return false;
	}
 } else if (!mScriptFile.isEmpty()) {
	type = "Script";
	QDomElement file = doc.createElement("ScriptFile");
	root.appendChild(file);
	QDomText fileText = doc.createTextNode(mScriptFile);
	file.appendChild(fileText);

	QDomElement call = doc.createElement("ScriptCall");
	root.appendChild(call);
	QDomText callText = doc.createTextNode(mScriptCall);
	call.appendChild(callText);

	// TODO: parameters
 }

 if (type.isEmpty()) {
	boError(360) << k_funcinfo << "internal error: don't know type of Action" << endl;
	return false;
 }
 root.setAttribute("Type", type);
 return true;
}

bool BoConditionAction::loadActionFromXML(const QDomElement& root)
{
 if (root.isNull()) {
	boError(360) << k_funcinfo << "no Action tag" << endl;
	return false;
 }
 QString type = root.attribute("Type");
 if (type.isEmpty()) {
	boError(360) << k_funcinfo << "Action tag has no type" << endl;
	return false;
 }
 delete mEventCaused;
 mEventCaused = 0;
 mScriptFile = QString::null;
 mScriptCall = QString::null;

 if (type == "Event") {
	QDomElement actionEvent = root.namedItem("Event").toElement();
	if (!actionEvent.isNull()) {
		mEventCaused = new BoEvent();
		if (!mEventCaused->loadFromXML(actionEvent)) {
			boError(360) << k_funcinfo << "cannot load event that is caused by condition" << endl;
			delete mEventCaused;
			mEventCaused = 0;
			return false;
		}
	}
 } else if (type == "Script") {
	QDomElement file = root.namedItem("ScriptFile").toElement();
	if (file.isNull()) {
		boError(360) << k_funcinfo << "Type is Script, but Action has no ScriptFile element" << endl;
		return false;
	}

	// the filename of the script must start with
	// - scripts/ (the $KDEDIR/share/apps/boson/scripts dir is used)
	// - map_scripts/ (the map_scripts dir in the .bpf/.bsg file is used)
	// no other prefixes are allowed.
	mScriptFile = file.text();
	if (mScriptFile.isEmpty()) {
		boError(360) << k_funcinfo << "ScriptFile element has empty text. You must specify the script that contains the function you want to call" << endl;
		return false;
	}
	if (!mScriptFile.startsWith("scripts/") && !mScriptFile.startsWith("map_scripts/")) {
		boError(360) << k_funcinfo << "ScriptFile must start with either scripts/ or map_scripts/ - filename: " << mScriptFile << endl;
		return false;
	}

	// TODO: check if fileName actually exists!
	// (might be difficult in the map_scripts case, since we don't have
	// direct access to that file system)
	bool file_exists = true;
	boWarning(360) << k_funcinfo << "TODO: check if script file actually exists" << endl;

	if (!file_exists) {
		boError(360) << k_funcinfo << "ScriptFile " << mScriptFile << " was not found" << endl;
		return false;
	}

	QDomElement call = root.namedItem("ScriptCall").toElement();
	if (call.isNull()) {
		boError(360) << k_funcinfo << "Script Action has no ScriptCall element." << endl;
		return false;
	}
	if (call.text().isEmpty()) {
		boError(360) << k_funcinfo << "ScriptCall element has empty text. you must specify a call (function)" << endl;
		return false;
	}
	mScriptCall = call.text();

	// TODO: parameters.
	// AB: they may be implemented using multiple ScriptParameter elements.
	//  -> attributes may not be sufficient, as a parameter may be something
	//     else but a constant (i.e. depend on game states)
	//  -> probably we need to save a QStringList or so containing the
	//     parameters. maybe we even have to write a BoScriptCall class

 } else {
	boError(360) << k_funcinfo << "Action type " << type << " is not supported." << endl;
	return false;
 }

 return true;
}

void BoConditionAction::fire()
{
 // atm only events are supported. this will change in the future (support
 // calling functions in scripts)
 boDebug(360) << k_funcinfo << endl;
 if (mEventCaused) {
	QDomDocument doc;
	QDomElement root = doc.createElement("Event");
	if (!mEventCaused->saveAsXML(root)) {
		boError(360) << k_funcinfo << "could not save event that is caused by condition" << endl;
		return;
	}
	BoEvent* e = new BoEvent();
	if (!e->loadFromXML(root)) {
		boError(360) << k_funcinfo << "could not load event that is caused by condition" << endl;
		delete e;
		return;
	}
	boGame->queueEvent(e);
 } else if (!mScriptFile.isEmpty()) {
	boWarning(360) << k_funcinfo << "calling script function in " << mScriptFile << endl;
	boWarning(360) << k_funcinfo << "calling: " << mScriptCall << endl;

	boError(360) << k_funcinfo << "not yet implemented" << endl;
 } else {
	boError(360) << k_funcinfo << "not a supported action - loding this object should have failed!" << endl;
 }
}

