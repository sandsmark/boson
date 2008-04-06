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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef BOSONGAMEENGINESTARTING_H
#define BOSONGAMEENGINESTARTING_H

#include "bosonstarting.h"
//Added by qt3to4:
#include <Q3PtrList>

class BosonPlayField;
class Player;
class Boson;
class BosonCanvas;
class BosonGameView;
class QDomElement;
template<class T> class Q3PtrList;
template<class T1, class T2> class QMap;
class BosonStartingTask;

class BosonGameEngineStarting : public BosonStartingTaskCreator
{
public:
	BosonGameEngineStarting(BosonStarting* starting, QObject* parent);
	~BosonGameEngineStarting();

	virtual void setFiles(QMap<QString, QByteArray>* files);
	virtual QString creatorName() const;

	virtual bool createTasks(Q3PtrList<BosonStartingTask>* tasks);

private:
	BosonStarting* mStarting;
	QMap<QString, QByteArray>* mFiles;
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

signals:
	void signalCanvasCreated(BosonCanvas* canvas);

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
		mCanvas = 0;
	}

	virtual unsigned int taskDuration() const;

public slots:
	void slotSetCanvas(BosonCanvas* canvas)
	{
		mCanvas = canvas;
	}

protected:
	virtual bool startTask();

private:
	BosonCanvas* mCanvas;
};

class BosonStartingLoadPlayerGameData : public BosonStartingTask
{
	Q_OBJECT
public:
	BosonStartingLoadPlayerGameData(const QString& text)
		: BosonStartingTask(text)
	{
		mPlayer = 0;
	}

	virtual unsigned int taskDuration() const;

	void setPlayer(Player* p);
	Player* player() const
	{
		return mPlayer;
	}

protected:
	virtual bool startTask();

private:
	Player* mPlayer;
};

class BosonStartingStartScenario : public BosonStartingTask
{
	Q_OBJECT
public:
	BosonStartingStartScenario(const QString& text)
		: BosonStartingTask(text)
	{
		mFiles = 0;

		mCanvas = 0;
	}

	virtual unsigned int taskDuration() const;

	void setFiles(QMap<QString, QByteArray>* files)
	{
		mFiles = files;
	}

public slots:
	void slotSetCanvas(BosonCanvas* canvas)
	{
		mCanvas = canvas;
	}

protected:
	virtual bool startTask();

	/**
	 * Creates @ref BosonMoveData objects for all unitproperties
	 **/
	bool createMoveDatas();

private:
	QMap<QString, QByteArray>* mFiles;

	BosonCanvas* mCanvas;
};

#endif

