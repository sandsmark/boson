/*
    This file is part of the Boson game
    Copyright (C) 2002-2004 The Boson Team (boson-devel@lists.sourceforge.net)

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

#ifndef BOSONSTARTING_H
#define BOSONSTARTING_H

#include <qobject.h>

class BosonPlayField;
class Player;
class Boson;
class BosonCanvas;
class QDomElement;

class BosonStartingPrivate;
/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonStarting : public QObject
{
	Q_OBJECT
public:
	BosonStarting(QObject* parent);
	~BosonStarting();

	/**
	 * Set all data that are needed to start a new game. This stream should
	 * have been sent by the ADMIN to all clients. It should contain at
	 * least the map and the scenario
	 **/
	void setNewGameData(const QByteArray& data);

	void setEditorMap(const QByteArray& buffer);

	void setLoadFromLogFile(const QString& file);
	QString logFile() const;

	void startNewGame();

	/**
	 * Prepare for loading a game. This loads the playfield from @ref
	 * fileName and adds the players necessary for loading the game.
	 * @return An empty @ref QByteArray if an error occurred or the data
	 * necessary for @ref setNewGameData (such as the playField) if it
	 * succeeded.
	 **/
	QByteArray loadGame(const QString& fileName);

	/**
	 * Called by @ref Boson once a message indicating that a client
	 * completed game starting has been received.
	 *
	 * That message is sent by this class.
	 * @param buffer The message that was sent. At the moment this is empty.
	 * It might be used for additional data one day, e.g. to check whether
	 * loading was successfull - we also might use this to find out about
	 * starting failure.
	 * @param The sender of the message, i.e. the client that completed
	 * loading.
	 **/
	void startingCompletedReceived(const QByteArray& buffer, Q_UINT32 client);

	/**
	 * Check whether there are events and process them. See @ref
	 * QApplication::processEvents.
	 *
	 * Do <em>not</em> (this is very important!!) call this when you have
	 * just done something like @ref QTimer::singleShot() or @ref
	 * Boson::sendMessage!! These calls end up in the even queue and if they
	 * are executed fast then they are executed in checkEvents
	 * <em>before</em> your function returns (which is not inended). But
	 * often they are not executed (when they have not yet reacehed the
	 * queue for example) and so you have a nice race condition that is
	 * <em>really</em> hard to debug.
	 *
	 * So think twice before adding a call to this function. It is a good
	 * idea for long tasks (pixmap loading usually) but for everything else
	 * it is a very bad idea.
	 **/
	void checkEvents();

signals:
	void signalStartingFailed();

	/**
	 * Change the type of data thats currently being loaded. See @ref
	 * BosonLoadingWidget::LoadingType
	 **/
	void signalLoadingType(int type);

	void signalLoadingShowProgressBar(bool show);
	void signalLoadingReset();
	void signalLoadingSetAdmin(bool isAdmin);
	void signalLoadingSetLoading(bool isLoading);
	void signalLoadingPlayersCount(int count);
	void signalLoadingPlayer(int current);
	void signalLoadingUnitsCount(int count);
	void signalLoadingUnit(int current);

protected slots:
	void slotStart();


	void slotLoadPlayerData(Player* p);

protected:
	/**
	 * @return The playfield. protected, as you should get this from
	 * elsewhere (e.g. @ref TopWidget) if you are outside this class
	 **/
	BosonPlayField* playField() const { return mDestPlayField; }

	bool loadPlayerData();
	void loadUnitDatas(Player* p);
	bool startScenario(QMap<QString, QByteArray>& files);
	bool start();
	bool loadTiles();

	/**
	 * We cannot store the actual player ID in our files (.bsg/.bpf),
	 * because the ID can be totally different when the game is loaded again
	 * (remember: starting a new game is just a special case of loading a
	 * game).
	 *
	 * Therefore we store the _index_ of the players in our files ("player
	 * number"). But all load() methods (e.g. in BosonCanvas) expect the
	 * _actual ID_, as that's what they need to load the files correctly.
	 *
	 * So we need to map the "player number" from the file, to the actual
	 * player ID. This is done here.
	 **/
	bool fixPlayerIds(QMap<QString, QByteArray>& files) const;

	/**
	 * Fixes (recursively) all PlayerId tags in @p root and its children.
	 *
	 * @param actualIds An array containing the actual ID for every player
	 * index
	 * @param players The number of players. This is equal to the number of
	 * elements in @p actualIds.
	 **/
	bool fixPlayerIds(int* actualIds, unsigned int players, QDomElement& root) const;

	/**
	 * Add the players for a loaded game.
	 **/
	bool addLoadGamePlayers(const QString& playersXML);

	void sendStartingCompleted(bool success);

	bool checkStartingCompletedMessages() const;

private:
	BosonStartingPrivate* d;

	QByteArray mNewGameData;
	BosonPlayField* mDestPlayField;
};

#endif

