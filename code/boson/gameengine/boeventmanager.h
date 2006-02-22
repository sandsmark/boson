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
	bool saveAllEventListenerScripts(QMap<QString, QByteArray>* scripts) const;

	/**
	 * This calls @ref BoEventListener::saveAsXML for all listeners.
	 **/
	bool saveAllEventListenersXML(QMap<QString, QByteArray>* scripts) const;

	bool copyEventListenerScripts(const QMap<QString, QByteArray>& scripts);
	bool copyEventListenerXML(const QMap<QString, QByteArray>& scripts);

	/**
	 * This calls @ref BoEventListener::loadScript for all listeners.
	 **/
	bool loadAllEventListenerScripts();

	/**
	 * This calls @rev BoEventListener::loadFromXML for all listeners. Note
	 * that before this is called, @ref loadAllEventListenerScripts should
	 * be called first, i.e. first load the scripts, then the XML.
	 **/
	bool loadAllEventListenersXML();

	/**
	 * Used by @ref BoEventListener.
	 **/
	bool loadEventListenerScript(BoEventListener* listener);


	/**
	 * The "currently available scripts" are all scripts that belong to the
	 * current map - AI scripts, localplayer script, canvas/game script, ...
	 *
	 * This includes scripts that are not in use (e.g. the AI script for
	 * players played by humans)
	 *
	 * By default data files, that include the script and the saved values
	 * of the variables in the script are not returned. Use dataFiles = TRUE
	 * to include them, but note that they are not human readable.
	 *
	 * @return A list of filenames of the currently available scripts.
	 **/
	QStringList availableScriptFiles(bool dataFiles = false) const;

	/**
	 * @return TRUE if a script for @p fileName is available, otherwise
	 * FALSE. See also @ref availableScriptFiles.
	 **/
	bool haveScriptForFile(const QString& fileName) const;

	/**
	 * @return A @ref QByteArray containing the script that belongs to @p
	 * fileName, or an empty object if @ref haveScriptForFile returns FALSE.
	 **/
	QByteArray scriptForFile(const QString& fileName) const;


	static QByteArray createEmptyEventListenerXML();

protected:
	void deliverEvent(BoEvent* event);
	void declareEvents();

private:
	BoEventManagerPrivate* d;
};

#endif

