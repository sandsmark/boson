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

#include "bosongameviewstarting.h"
#include "bosongameviewstarting.moc"

#include "../bomemory/bodummymemory.h"
#include "defines.h"
#include "boson.h"
#include "gameview/bosongameview.h"
#include "bosonsaveload.h"
#include "bodebug.h"
#include "bosonprofiling.h"

#include <klocale.h>

#include <qmap.h>
//Added by qt3to4:
#include <Q3PtrList>

BosonGameViewStarting::BosonGameViewStarting(BosonStarting* starting, QObject* parent)
	: BosonStartingTaskCreator(parent)
{
 mStarting = starting;
 mGameView = 0;
 mFiles = 0;
}

BosonGameViewStarting::~BosonGameViewStarting()
{
}

QString BosonGameViewStarting::creatorName() const
{
 return i18n("GameView");
}

void BosonGameViewStarting::setGameView(BosonQtGameView* gameView)
{
 mGameView = gameView;
}

void BosonGameViewStarting::setFiles(QMap<QString, QByteArray>* files)
{
 mFiles = files;
}

bool BosonGameViewStarting::createTasks(Q3PtrList<BosonStartingTask>* tasks)
{
 if (!mStarting) {
	BO_NULL_ERROR(mStarting);
	return false;
 }
 if (!mFiles) {
	BO_NULL_ERROR(mFiles);
	return false;
 }
 if (!mGameView) {
	BO_NULL_ERROR(mGameView);
	return false;
 }

 BosonStartingStartGameView* gameView = new BosonStartingStartGameView(i18n("Start gameview"));
 connect(mStarting, SIGNAL(signalCanvas(BosonCanvas*)),
		gameView, SLOT(slotSetCanvas(BosonCanvas*)));
 gameView->setGameView(mGameView);
 gameView->setFiles(mFiles);
 tasks->append(gameView);

 return true;
}



void BosonStartingStartGameView::setGameView(BosonQtGameView* gameView)
{
 mGameView = gameView;
}

bool BosonStartingStartGameView::startTask()
{
 PROFILE_METHOD
 if (!boGame) {
	BO_NULL_ERROR(boGame);
	return false;
 }
 if (!mGameView) {
	BO_NULL_ERROR(mGameView);
	return false;
 }

 mGameView->setCanvas(mCanvas);
 if (!mGameView->initializeItems()) {
	boError() << k_funcinfo << "initializing items failed" << endl;
	return false;
 }

 BosonSaveLoad load(boGame);

 // TODO: rename to "loadGameViewFromXML()"
 if (!load.loadExternalFromXML(*mFiles)) {
	boError(270) << k_funcinfo << "unable to load external data from XML" << endl;
	return false;
 }

 return true;
}

unsigned int BosonStartingStartGameView::taskDuration() const
{
 return 100;
}



