/*
    This file is part of the Boson game
    Copyright (C) 2002-2005 Andreas Beckermann (b_mann@gmx.de)

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
template<class T> class QPtrList;
template<class T1, class T2> class QMap;
class BosonStartingTask;

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

public slots:
	/**
	 * Called by @ref Boson once a message indicating that a client
	 * completed game starting has been received.
	 *
	 * That message is sent by this class.
	 * @param buffer The message that was sent. At the moment this is empty.
	 * It might be used for additional data one day, e.g. to check whether
	 * loading was successfull - we also might use this to find out about
	 * starting failure.
	 *
	 * See also @ref Boson::signalStartingCompletedReceived
	 *
	 * @param The sender of the message, i.e. the client that completed
	 * loading.
	 **/
	void slotStartingCompletedReceived(const QByteArray& message, Q_UINT32 sender);

	/**
	 * Set all data that are needed to start a new game. This stream should
	 * have been sent by the ADMIN to all clients. It should contain at
	 * least the map and the scenario
	 **/
	void slotSetNewGameData(const QByteArray& data, bool* taken);

signals:
	void signalStartingFailed();

	void signalLoadingMaxDuration(unsigned int duration);
	
	/**
	 * Emitted when a task has been completed. The progress bar should be
	 * updated.
	 * @param duration The amount of "time" (as an abstract starting
	 * progress, not with a real unit) that has passed. See @ref
	 * signalMaxLoadingDuration for the maximal value.
	 **/
	void signalLoadingTaskCompleted(unsigned int duration);

	/**
	 * Emitted when a starting task begins. @p text describes this task
	 * (i18n'ed)
	 **/
	void signalLoadingStartTask(const QString& text);

	void signalLoadingStartSubTask(const QString& text);

	void signalLoadingType(int type);
	void signalLoadingUnitsCount(int count);
	void signalLoadingUnit(int current);

protected slots:
	void slotStart();

	void slotPlayFieldCreated(BosonPlayField* playField, bool* ownerChanged);


protected:
	/**
	 * @return The playfield. protected, as you should get this from
	 * elsewhere (e.g. @ref TopWidget) if you are outside this class
	 **/
	BosonPlayField* playField() const { return mDestPlayField; }

	bool start();



	/**
	 * Add the players for a loaded game.
	 **/
	bool addLoadGamePlayers(const QString& playersXML);

	void sendStartingCompleted(bool success);

	bool checkStartingCompletedMessages() const;

	bool executeTasks(const QPtrList<BosonStartingTask>& tasks);

signals:
	void signalDestPlayField(BosonPlayField*);

private:
	BosonStartingPrivate* d;

	QByteArray mNewGameData;
	BosonPlayField* mDestPlayField;
};


class BosonStartingTask : public QObject
{
	Q_OBJECT
public:
	BosonStartingTask(const QString& text, QObject* parent = 0);
	~BosonStartingTask();

	virtual QString text() const
	{
		return mText;
	}

	void checkEvents();

	/**
	 * Start this task. This calls @ref startTask, which does the actual
	 * work.
	 * @param duration How much time has already passed before this task was
	 * started.
	 **/
	bool start(unsigned int duration);

	/**
	 * @return The estimated amount of time this task will take. You can
	 * return any non-negative number that you like, there is no special
	 * unit (ms,s,hour,...) assigned to it. Have a look at the
	 * implementations in derived classes to get a feeling what number may
	 * be sensible.
	 **/
	virtual unsigned int taskDuration() const = 0;

signals:
	void signalStartSubTask(const QString& text);
	void signalCompleteSubTask(unsigned int duration);

protected:
	virtual bool startTask() = 0;

protected:
	/**
	 * Call this when a sub-task has been completed.
	 * @param duration How much time has passed since this task was started
	 * until the sub-task has been completed. 0 is the minimum, @ref
	 * taskDuration the maximum.
	 **/
	void completeSubTask(unsigned int duration);

	void startSubTask(const QString& text);


private:
	QString mText;
	unsigned int mTimePassed;
};

class BosonStartingLoadPlayField : public BosonStartingTask
{
	Q_OBJECT
public:
	BosonStartingLoadPlayField(const QString& text)
		: BosonStartingTask(text)
	{
	}

	virtual unsigned int taskDuration() const;

	void setFiles(QMap<QString, QByteArray>* files)
	{
		mFiles = files;
	}

signals:
	void signalPlayFieldCreated(BosonPlayField* playField, bool* ownerChanged);

protected:
	virtual bool startTask();

private:
	QMap<QString, QByteArray>* mFiles;
};

class BosonStartingCreateCanvas : public BosonStartingTask
{
	Q_OBJECT
public:
	BosonStartingCreateCanvas(const QString& text)
		: BosonStartingTask(text)
	{
		mDestPlayField = 0;
	}

	virtual unsigned int taskDuration() const;

	BosonPlayField* playField() const
	{
		return mDestPlayField;
	}

public slots:
	void slotSetDestPlayField(BosonPlayField* dest)
	{
		mDestPlayField = dest;
	}

protected:
	virtual bool startTask();

private:
	BosonPlayField* mDestPlayField;
};

class BosonStartingInitPlayerMap : public BosonStartingTask
{
	Q_OBJECT
public:
	BosonStartingInitPlayerMap(const QString& text)
		: BosonStartingTask(text)
	{
		mDestPlayField = 0;
	}

	virtual unsigned int taskDuration() const;

	BosonPlayField* playField() const
	{
		return mDestPlayField;
	}

public slots:
	void slotSetDestPlayField(BosonPlayField* dest)
	{
		mDestPlayField = dest;
	}

protected:
	virtual bool startTask();

private:
	BosonPlayField* mDestPlayField;
};

class BosonStartingInitScript : public BosonStartingTask
{
	Q_OBJECT
public:
	BosonStartingInitScript(const QString& text)
		: BosonStartingTask(text)
	{
	}

	virtual unsigned int taskDuration() const;

protected:
	virtual bool startTask();
};

class BosonStartingLoadTiles : public BosonStartingTask
{
	Q_OBJECT
public:
	BosonStartingLoadTiles(const QString& text)
		: BosonStartingTask(text)
	{
		mDestPlayField = 0;
	}

	virtual unsigned int taskDuration() const;

	BosonPlayField* playField() const
	{
		return mDestPlayField;
	}

public slots:
	void slotSetDestPlayField(BosonPlayField* dest)
	{
		mDestPlayField = dest;
	}

protected:
	virtual bool startTask();

private:
	BosonPlayField* mDestPlayField;
};

class BosonStartingLoadEffects : public BosonStartingTask
{
	Q_OBJECT
public:
	BosonStartingLoadEffects(const QString& text)
		: BosonStartingTask(text)
	{
	}

	virtual unsigned int taskDuration() const;

protected:
	virtual bool startTask();
};

class BosonStartingLoadPlayerData : public BosonStartingTask
{
	Q_OBJECT
public:
	BosonStartingLoadPlayerData(const QString& text)
		: BosonStartingTask(text)
	{
		mPlayer = 0;
		mDuration = 0;
	}

	virtual unsigned int taskDuration() const;

	void setPlayer(Player* p);
	Player* player() const
	{
		return mPlayer;
	}

protected:
	virtual bool startTask();

	bool loadUnitDatas();

	unsigned int durationBeforeUnitLoading() const;
	unsigned int loadUnitDuration() const;

private:
	Player* mPlayer;

	unsigned int mDuration;
};

class BosonStartingLoadWater : public BosonStartingTask
{
	Q_OBJECT
public:
	BosonStartingLoadWater(const QString& text)
		: BosonStartingTask(text)
	{
		mDestPlayField = 0;
	}

	virtual unsigned int taskDuration() const;

	BosonPlayField* playField() const
	{
		return mDestPlayField;
	}

public slots:
	void slotSetDestPlayField(BosonPlayField* dest)
	{
		mDestPlayField = dest;
	}

protected:
	virtual bool startTask();

private:
	BosonPlayField* mDestPlayField;
};

class BosonStartingStartScenario : public BosonStartingTask
{
	Q_OBJECT
public:
	BosonStartingStartScenario(const QString& text)
		: BosonStartingTask(text)
	{
		mDestPlayField = 0;
		mFiles = 0;
	}

	virtual unsigned int taskDuration() const;

	BosonPlayField* playField() const
	{
		return mDestPlayField;
	}

	void setFiles(QMap<QString, QByteArray>* files)
	{
		mFiles = files;
	}

public slots:
	void slotSetDestPlayField(BosonPlayField* dest)
	{
		mDestPlayField = dest;
	}

protected:
	virtual bool startTask();

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
	bool fixPlayerIdsInFileNames(int* actualIds, unsigned int players, QMap<QString, QByteArray>& files) const;

private:
	BosonPlayField* mDestPlayField;
	QMap<QString, QByteArray>* mFiles;
};

#endif

