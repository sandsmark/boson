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
class BosonScript;
template<class T1, class T2> class QMap;


class BoEventListenerPrivate;
class BoEventListener : public QObject
{
	Q_OBJECT
public:
	BoEventListener(const QString& scriptName, BoEventManager* manager, QObject* parent);
	virtual ~BoEventListener();

	virtual bool save(QDomElement& root) const;
	virtual bool load(const QDomElement& root);

	/**
	 * Saves the script of this event listener to @p script. Note that only
	 * the script itself is saved, not the current values of the variables.
	 * Loading @p script again is equal to loading the script from a normal
	 * .py file.
	 **/
	bool saveScript(QByteArray* script) const;
	bool loadScript(const QByteArray& script);

	/**
	 * This also saves the script, just like @ref saveScript does. However
	 * additionally it saves the current values of the variables, so that
	 * you can use the @p script bytearray for loading a savegame.
	 **/
	bool saveScriptData(QByteArray* script) const;
	bool loadScriptData(const QByteArray& script);

	bool saveConditions(QDomElement& root) const;
	bool loadConditions(const QDomElement& root);

	bool addCondition(BoCondition* c);

	void receiveEvent(const BoEvent* event);

	/**
	 * @return The filename of the script of this event listener. This is
	 * only the filename, without the path.
	 **/
	const QString& scriptFileName() const
	{
		return mScriptFileName;
	}

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

	void ensureScriptInterpreter();

	/**
	 * This should return the @ref BosonScript::newScriptParser with the
	 * correct player parameter.
	 **/
	virtual BosonScript* createScriptParser() const = 0;

private:
	BoEventListenerPrivate* d;
	BoEventManager* mManager;
	QString mScriptFileName;
	QString mScriptString;
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

protected:
	virtual BosonScript* createScriptParser() const;

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

protected:
	virtual BosonScript* createScriptParser() const;

private:
	PlayerIO* mPlayerIO;
};

#endif

