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
#ifndef BOEVENTLISTENER_H
#define BOEVENTLISTENER_H

#include <qobject.h>

class Boson;
class KGamePropertyHandler;
class QDomElement;
class BoEvent;
class BoEventManager;
class BoCondition;
class Player;
class PlayerIO;


class BoEventListenerPrivate;
class BoEventListener : public QObject
{
	Q_OBJECT
public:
	BoEventListener(BoEventManager* manager, QObject* parent);
	virtual ~BoEventListener();

	virtual bool save(QDomElement& root) const;
	virtual bool load(const QDomElement& root);

	bool addCondition(BoCondition* c);


	void receiveEvent(const BoEvent* event);


	/**
	 * @param event An event with a location, i.e. @ref BoEvent::hasLocation
	 * is TRUE.
	 * @return TRUE if the listener can "see" the event (e.g. the player
	 * this listener is for can see the location). Otherwise FALSE.
	 **/
	virtual bool canSee(const BoEvent* event) const = 0;

protected:
	virtual void processEvent(const BoEvent* event) = 0;

	void deliverToConditions(const BoEvent* event);

	bool saveConditions(QDomElement& root) const;
	bool loadConditions(const QDomElement& root);

	void ensureScriptInterpreter();

private:
	BoEventListenerPrivate* d;
	BoEventManager* mManager;
};

/**
 * This is the global event listener, which receives all events
 **/
class BoCanvasEventListener : public BoEventListener
{
	Q_OBJECT
public:
	BoCanvasEventListener(BoEventManager* manager, QObject* parent);
	~BoCanvasEventListener();

	virtual void processEvent(const BoEvent* event);
	virtual bool canSee(const BoEvent*) const
	{
		return true;
	}

private:
};

class BoLocalPlayerEventListener : public BoEventListener
{
	Q_OBJECT
public:
	BoLocalPlayerEventListener(PlayerIO* io, BoEventManager* manager, QObject* parent);
	~BoLocalPlayerEventListener();

	PlayerIO* playerIO() const
	{
		return mPlayerIO;
	}

	virtual void processEvent(const BoEvent* event);
	virtual bool canSee(const BoEvent* event) const;

private:
	PlayerIO* mPlayerIO;
};

#endif

