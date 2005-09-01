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
#ifndef BOEVENTMANAGER_H
#define BOEVENTMANAGER_H

#include <qobject.h>

class Boson;
class KGamePropertyHandler;
class QDomElement;
class BoEvent;
class BoEventListener;
class Player;
template<class T1, class T2> class QMap;

class BoEventManagerPrivate;
class BoEventManager : public QObject
{
	Q_OBJECT
public:
	enum PropertyIds {
		IdNextEvent = 500
	};
public:
	BoEventManager(QObject* parent);
	~BoEventManager();

	bool knowEventName(const QCString& name) const;

	/**
	 * This does NOT take ownership of the listener! You still have to
	 * delete it on your own.
	 **/
	void addEventListener(BoEventListener* listener);

	void removeEventListener(BoEventListener* listener);

	/**
	 * Queue an event for delivery. Delivery takes place in @ref advance.
	 *
	 * this method takes ownership of the event.
	 * @return TRUE on success, or FALSE on error (e.g. name not known).
	 * When FALSE is returned, the event is deleted when this method
	 * returns.
	 **/
	bool queueEvent(BoEvent*);

	/**
	 * Advance any queued events.
	 **/
	void advance(unsigned int advanceCallsCount);

	bool saveAsXML(QDomElement& root) const;
	bool loadFromXML(const QDomElement& root);

	/**
	 * This calls @ref BoEventListener::saveScript for all listeners.
	 **/
	bool saveListenerScripts(QMap<QString, QByteArray>* scripts) const;

	bool copyEventListenerScripts(const QMap<QString, QByteArray>& scripts);

	/**
	 * This calls @ref BoEventListener::loadScript for all listeners.
	 **/
	bool loadAllEventListenerScripts();

	/**
	 * Used by @ref BoEventListener.
	 **/
	bool loadEventListenerScript(BoEventListener* listener);

protected:
	void deliverEvent(BoEvent* event);
	void cacheStatusEvent(BoEvent* event);
	void sendStatusEvents();
	void declareEvents();

private:
	BoEventManagerPrivate* d;
};

#endif

