/*
    This file is part of the Boson game
    Copyright (C) 2002-2006 Andreas Beckermann (b_mann@gmx.de)

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

#ifndef BOSONGAMEVIEWSTARTING_H
#define BOSONGAMEVIEWSTARTING_H

#include "bosonstarting.h"

class BosonPlayField;
class Player;
class Boson;
class BosonCanvas;
class BosonGameView;
template<class T> class QPtrList;
template<class T1, class T2> class QMap;

class BosonStartingPrivate;

class BosonGameViewStarting : public BosonStartingTaskCreator
{
public:
	BosonGameViewStarting(BosonStarting* starting, QObject* parent);
	~BosonGameViewStarting();

	void setGameView(BosonGameView* gameView);

	virtual void setFiles(QMap<QString, QByteArray>* files);
	virtual QString creatorName() const;

	virtual bool createTasks(QPtrList<BosonStartingTask>* tasks);

private:
	BosonStarting* mStarting;
	BosonGameView* mGameView;
	QMap<QString, QByteArray>* mFiles;
};


class BosonStartingStartGameView : public BosonStartingTask
{
	Q_OBJECT
public:
	BosonStartingStartGameView(const QString& text)
		: BosonStartingTask(text)
	{
		mGameView = 0;
		mFiles = 0;

		mCanvas = 0;
	}

	virtual unsigned int taskDuration() const;

	void setGameView(BosonGameView* gameView);

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

private:
	BosonGameView* mGameView;
	QMap<QString, QByteArray>* mFiles;

	BosonCanvas* mCanvas;
};

#endif

