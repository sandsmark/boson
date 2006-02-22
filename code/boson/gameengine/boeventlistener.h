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
template<class T> class QPtrList;


class BoEventListenerPrivate;
class BoEventListener : public QObject
{
	Q_OBJECT
public:
	BoEventListener(BoEventManager* manager, QObject* parent);
	virtual ~BoEventListener();

	/**
	 * Initializes and loads a script for the event listener (if any).
	 *
	 * You should call this immediately after constructing the event
	 * listener (it can't be done in the constructor as it depends on
	 * virtual methods).
	 *
	 * Note that if this event listener does not use scripts, this does
	 * nothing. So calling it won't hurt.
	 *
	 * @return FALSE on error, otherwise TRUE. Note that if scripts could
	 * not yet be initialized this will return TRUE, as it will be done
	 * later and therefore is not an error.
	 **/
	bool initScript();

	/**
	 * @return The filename of the script of this event listener. This is
	 * only the filename, without the path. Example: "localplayer.py". The
	 * file is automatically in a scripts/eventlistener/ subdir.
	 *
	 * Note that <em>every</em> event listener <em>must</em> provide a
	 * unique script filename, event if it does not make use of scripts. It will
	 * just save empty script data files then.
	 **/
	virtual QString scriptFileName() const = 0;

	/**
	 * @return The filename that the XML data of this eventlistener is saved
	 * to.
	 **/
	virtual QString xmlFileName() const = 0;

	/**
	 * Do NOT call this manually! @ref
	 * BoEventManager::saveAllEventListenersXML will do so!
	 **/
	virtual bool saveAsXML(QDomElement& root) const;

	/**
	 * Do NOT call this manually! @ref
	 * BoEventManager::loadAllEventListenersXML will do so!
	 **/
	virtual bool loadFromXML(const QDomElement& root);

	bool saveScriptData(QByteArray* scriptData) const;
	bool loadScript(const QByteArray& script, const QByteArray& scriptData);

	/**
	 * @return Script object of this event listener.
	 * Note that it can also be 0
	 **/
	BosonScript* script() const;

	bool saveConditions(QDomElement& root) const;
	bool loadConditions(const QDomElement& root);
	bool saveEventHandlers(QDomElement& root) const;
	bool loadEventHandlers(const QDomElement& root);

	bool addCondition(BoCondition* c);

	void receiveEvent(const BoEvent* event);

	/**
	 * @param event An event with a location, i.e. @ref BoEvent::hasLocation
	 * is TRUE.
	 * @return TRUE if the listener can "see" the event (e.g. the player
	 * this listener is for can see the location). Otherwise FALSE.
	 **/
	virtual bool canSee(const BoEvent* event) const = 0;

public slots:
	void addEventHandler(const QString& eventname, const QString& functionname, const QString& args, int* id);
	void removeEventHandler(int id);

protected:
	virtual void processEvent(const BoEvent* event) = 0;

	void deliverToConditions(const BoEvent* event);
	void deliverToScript(const BoEvent* event);

	/**
	 * This should return the @ref BosonScript::newScriptParser with the
	 * correct player parameter.
	 **/
	virtual BosonScript* createScriptParser() const = 0;

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

	virtual QString scriptFileName() const
	{
		return QString("game.py");
	}

	virtual QString xmlFileName() const
	{
		return QString("canvas.xml");
	}

signals:
	void signalGameOver();

protected:
	virtual BosonScript* createScriptParser() const;

	/**
	 * Check whether the game is over (see @ref checkGameOver) and send a
	 * "GameOver" event if it is.
	 **/
	void checkGameOverAndEndGame();

	/**
	 * @return TRUE if the game is over, otherwise FALSE. In either case,
	 * the players that fullfilled the winning conditions up to now are returned
	 * in @p fullfilledWinningConditions.
	 **/
	bool checkGameOver(QPtrList<Player>* fullfilledWinningConditions = 0) const;

private:
};

class BoComputerPlayerEventListener : public BoEventListener
{
	Q_OBJECT
public:
	BoComputerPlayerEventListener(Player* p, BoEventManager* manager, QObject* parent);
	~BoComputerPlayerEventListener();

	PlayerIO* playerIO() const
	{
		return mPlayerIO;
	}

	virtual QString scriptFileName() const;
	virtual QString xmlFileName() const;

	virtual void processEvent(const BoEvent* event);
	virtual bool canSee(const BoEvent* event) const;

protected:
	virtual BosonScript* createScriptParser() const;

private:
	PlayerIO* mPlayerIO;
};

#endif

