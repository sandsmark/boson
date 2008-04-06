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

#ifndef BOSONGUISTARTING_H
#define BOSONGUISTARTING_H

#include "bosonstarting.h"
//Added by qt3to4:
#include <Q3PtrList>

class BosonPlayField;
class Player;
class Boson;
class BosonCanvas;
class QDomElement;
template<class T> class Q3PtrList;
template<class T1, class T2> class QMap;
class BosonStartingTask;

class BosonGUIStarting;
class BosonStartingPrivate;

class BosonGUIStarting : public BosonStartingTaskCreator
{
public:
	BosonGUIStarting(BosonStarting* starting, QObject* parent);
	~BosonGUIStarting();

	virtual void setFiles(QMap<QString, QByteArray>* files);
	virtual QString creatorName() const;

	virtual bool createTasks(Q3PtrList<BosonStartingTask>* tasks);

private:
	BosonStarting* mStarting;
	QMap<QString, QByteArray>* mFiles;
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

class BosonStartingLoadPlayerGUIData : public BosonStartingTask
{
	Q_OBJECT
public:
	BosonStartingLoadPlayerGUIData(const QString& text)
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

class BosonStartingStartScenarioGUI : public BosonStartingTask
{
	Q_OBJECT
public:
	BosonStartingStartScenarioGUI(const QString& text)
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

class BosonStartingCheckIOs : public BosonStartingTask
{
	Q_OBJECT
public:
	BosonStartingCheckIOs(const QString& text)
		: BosonStartingTask(text)
	{
	}

	virtual unsigned int taskDuration() const;

protected:
	virtual bool startTask();
};

#endif

