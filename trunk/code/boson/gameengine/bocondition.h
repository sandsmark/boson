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
#ifndef BOCONDITION_H
#define BOCONDITION_H

#include <qstring.h>

class QDomElement;
class QDomNodeList;

class BoEvent;
class BoConditionAction;
class BosonScript;
template<class T1, class T2> class QMap;

class BoConditionPrivate;

/**
 * Objects of BoCondition are always part of a @ref BoEventListener (or part of
 * another BoCondition object).
 *
 * A BoCondition object is a collection of events and status conditions that
 * <em>all</em> must have been received or fullfilled in order to make the
 * condition be fullfilled (see @ref thisConditionDone).
 *
 * Events that are received are checked using @ref BoEventMatching for whether
 * they match an event in the condition.
 * StatusConditions are calls to script functions.
 *
 * Another way of fullfilling a BoCondition object, is by fullfilling an
 * "alternative" object. While all Events/StatusConditions in a BoCondition
 * object are connected using AND, several BoCondition objects can be connected
 * using OR (i.e. every single BoCondition is a conjunction and together with
 * its alternatives, they build up a disjunctive normal form).
 *
 * When a condition is fullfilled, you (i.e. the event listener) should call
 * @ref fireAction, that emits an action (such as emitting an event, or
 * displaying a system chat message).
 *
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoCondition
{
public:
	BoCondition();
	virtual ~BoCondition();

	bool requireScript() const;

	/**
	 * @return TRUE if this condition is done, or one of the other
	 * disjunctions (ORs) is done. See @ref thisConditionDone
	 **/
	bool conditionDone(BosonScript* script) const;

	/**
	 * Do what should be done once the condition is fullfilled (see @ref
	 * conditionDone).
	 **/
	void fireAction();

	void processEvent(const BoEvent* event);

	virtual bool saveAsXML(QDomElement& root) const;
	virtual bool loadFromXML(const QDomElement& root);

	/**
	 * Reset the condition - all events need to be received again.
	 *
	 * @return TRUE if resetting succeeded (i.e. if it was desired) or FALSE
	 * if the condition does not want to be resetted, i.e. it matches only
	 * once. The condition is to be removed in that case.
	 **/
	bool reset();

protected:
	/**
	 * @return TRUE if this condition is fullfilled, i.e. all status
	 * conditions are fullfilled and all events have been received.
	 **/
	bool thisConditionDone(BosonScript* script) const;

private:
	BoConditionPrivate* d;
	BoConditionAction* mAction;
};

#endif

