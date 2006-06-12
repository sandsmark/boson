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

#include "bosongameenginestarting.h"
#include "bosongameenginestarting.moc"

#include "../bomemory/bodummymemory.h"
#include "defines.h"
#include "boson.h"
#include "player.h"
#include "bosonplayfield.h"
#include "bosonmap.h"
#include "bosonmessageids.h"
#include "speciestheme.h"
#include "bosonprofiling.h"
#include "bosondata.h"
#include "bosoncanvas.h"
#include "bodebug.h"
#include "bosonsaveload.h"
#include "boevent.h"
#include "script/bosonscript.h"
#include "unit.h"
#include "boitemlist.h"
#include "bosonconfig.h"

#include <klocale.h>

#include <qmap.h>

BosonGameEngineStarting::BosonGameEngineStarting(BosonStarting* starting, QObject* parent)
	: BosonStartingTaskCreator(parent)
{
 mStarting = starting;
}

BosonGameEngineStarting::~BosonGameEngineStarting()
{
}

QString BosonGameEngineStarting::creatorName() const
{
 return i18n("Game Engine");
}

void BosonGameEngineStarting::setFiles(QMap<QString, QByteArray>* files)
{
 mFiles = files;
}

bool BosonGameEngineStarting::createTasks(QPtrList<BosonStartingTask>* tasks)
{
 if (!mStarting) {
	BO_NULL_ERROR(mStarting);
	return false;
 }
 if (!mFiles) {
	BO_NULL_ERROR(mFiles);
	return false;
 }

 boDebug(270) << k_funcinfo << endl;
 if (!boGame) {
	boError(270) << k_funcinfo << "NULL boson object" << endl;
	return false;
 }
 if (boGame->gameStatus() != KGame::Init) {
	boError(270) << k_funcinfo
		<< "Boson must be in init status to receive map!" << endl
		<< "Current status: " << boGame->gameStatus() << endl;
	return false;
 }
 if (boGame->gamePlayerCount() < 2) {
	boError(270) << k_funcinfo << "not enough players in game. there must be at least a real player and a (internal) neutral player" << endl;
	return false;
 }

 // check if player IDs are valid
 {
	QValueList<int> players;
	QValueList<int> watchPlayers;
	for (unsigned i = 0; i < boGame->allPlayerCount(); i++) {
		Player* p = (Player*)boGame->allPlayerList()->at(i);
		int id = p->bosonId();
		if (id <= 0) {
			// IDs <= 0 are invalid.
			boError() << k_funcinfo << "Invalid player ID " << id << " for player with KGame ID " << p->kgameId() << endl;
			return false;
		}
		if (id >= 512) {
			// IDs >= 512 are reserved for future use. they must not
			// be used currently.
			boError() << k_funcinfo << "Invalid player ID " << id << " for player with KGame ID " << p->kgameId() << endl;
			return false;
		}
		if (id < 127) {
			boDebug() << k_funcinfo << "player with KGame ID " << p->kgameId() << " is just watching the game. no active player" << endl;
			if (watchPlayers.contains(id)) {
				// AB: note that duplicated IDs are (currently)
				// allowed here, but should not appear anyway.
				boWarning() << k_funcinfo << "already have a player with ID " << id << endl;
			}
			watchPlayers.append(id);
		}
		if (id >= 128 && id <= 511) {
			// player ID 128..511 are game players (256-511 are
			// non-playable players)
			if (players.contains(id)) {
				boError() << k_funcinfo << "have more than one player with ID " << id << " - this is not allowed. aborting." << endl;
				return false;
			}
		}
	}
 }



 BosonStartingLoadPlayField* loadPlayField = new BosonStartingLoadPlayField(i18n("Load PlayField"));
 connect(loadPlayField, SIGNAL(signalPlayFieldCreated(BosonPlayField*, bool*)),
		mStarting, SLOT(slotPlayFieldCreated(BosonPlayField*, bool*)));
 loadPlayField->setFiles(mFiles);
 tasks->append(loadPlayField);

 BosonStartingCreateCanvas* createCanvas = new BosonStartingCreateCanvas(i18n("Create Canvas"));
 connect(mStarting, SIGNAL(signalDestPlayField(BosonPlayField*)),
		createCanvas, SLOT(slotSetDestPlayField(BosonPlayField*)));
 connect(createCanvas, SIGNAL(signalCanvasCreated(BosonCanvas*)),
		mStarting, SIGNAL(signalCanvas(BosonCanvas*)));
 tasks->append(createCanvas);


 // AB: We can't do this
 // when players are initialized, as the map must be known to them once we start
 // loading the units (for *loading* games)
 BosonStartingInitPlayerMap* playerMap = new BosonStartingInitPlayerMap(i18n("Init Player Map"));
 connect(mStarting, SIGNAL(signalDestPlayField(BosonPlayField*)),
		playerMap, SLOT(slotSetDestPlayField(BosonPlayField*)));
 tasks->append(playerMap);

 BosonStartingInitScript* script = new BosonStartingInitScript(i18n("Init Script"));
 connect(mStarting, SIGNAL(signalCanvas(BosonCanvas*)),
		script, SLOT(slotSetCanvas(BosonCanvas*)));
 tasks->append(script);


 int index = 0;
 for (QPtrListIterator<Player> it(*boGame->gamePlayerList()); it.current(); ++it) {
	Player* p = (Player*)it.current();
	QString text;
	if (p->isActiveGamePlayer()) {
		text = i18n("Load player game data of player %1 (of %2)").arg(index + 1).arg(boGame->activeGamePlayerCount());
	} else {
		text = i18n("Load player game data of neutral player");
	}
	BosonStartingLoadPlayerGameData* playerData = new BosonStartingLoadPlayerGameData(text);
	playerData->setPlayer(p);
	tasks->append(playerData);
	index++;
 }


 BosonStartingStartScenario* scenario = new BosonStartingStartScenario(i18n("Start Scenario"));
 connect(mStarting, SIGNAL(signalCanvas(BosonCanvas*)),
		scenario, SLOT(slotSetCanvas(BosonCanvas*)));
 scenario->setFiles(mFiles);
 tasks->append(scenario);


 return true;
}



bool BosonStartingLoadPlayField::startTask()
{
 if (!mFiles) {
	BO_NULL_ERROR(mFiles);
	return false;
 }
 BosonPlayField* playField = new BosonPlayField(0);

 bool ownerChanged = false;
 emit signalPlayFieldCreated(playField, &ownerChanged);
 if (!ownerChanged) {
	boError(270) << k_funcinfo << "playfield owner not changed, connect to signal!" << endl;
	delete playField;
	return false;
 }


 if (!playField->loadPlayField(*mFiles)) {
	boError(270) << k_funcinfo << "error loading the playfield" << endl;
	return false;
 }

 if (!playField->map()) {
	boError(270) << k_funcinfo << "NULL map loaded" << endl;
	return false;
 }

 boGame->setPlayField(playField);
 return true;
}

unsigned int BosonStartingLoadPlayField::taskDuration() const
{
 return 100;
}


bool BosonStartingCreateCanvas::startTask()
{
 if (!boGame) {
	BO_NULL_ERROR(boGame);
	return false;
 }
 if (!playField()) {
	BO_NULL_ERROR(playField());
	return false;
 }
 if (!playField()->map()) {
	BO_NULL_ERROR(playField()->map());
	return false;
 }
 boGame->createCanvas();

 BosonCanvas* canvas = boGame->canvasNonConst();
 if (!canvas) {
	BO_NULL_ERROR(canvas);
	return false;
 }
 canvas->setMap(playField()->map());

 emit signalCanvasCreated(canvas);

 return true;
}

unsigned int BosonStartingCreateCanvas::taskDuration() const
{
 // this takes essentiall no time at all
 return 5;
}

bool BosonStartingInitPlayerMap::startTask()
{
 if (!playField()) {
	BO_NULL_ERROR(playField());
	return false;
 }
 if (!playField()->map()) {
	BO_NULL_ERROR(playField()->map());
	return false;
 }
 if (!boGame) {
	BO_NULL_ERROR(boGame);
	return false;
 }
 bool mapExplored = boConfig->boolValue("ExploreMapOnStartup");
 for (unsigned int i = 0; i < boGame->allPlayerCount(); i++) {
	boDebug(270) << "init map for player " << i << endl;
	Player* p = boGame->allPlayerList()->at(i);
	if (p) {
    p->initMap(playField()->map(), boGame->gameMode() && !mapExplored, boGame->gameMode());
	}
 }
 return true;
}

unsigned int BosonStartingInitPlayerMap::taskDuration() const
{
 return 100;
}

bool BosonStartingInitScript::startTask()
{
 if (!mCanvas) {
	BO_NULL_ERROR(mCanvas);
	return false;
 }
 BosonScript::setGame(boGame);
 BosonScript::setCanvas(mCanvas);
 return true;
}

unsigned int BosonStartingInitScript::taskDuration() const
{
 // this takes essentiall no time at all
 return 1;
}

bool BosonStartingLoadPlayerGameData::startTask()
{
 boDebug(270) << k_funcinfo << endl;
 if (!boGame) {
	return false;
 }
 if (!player()) {
	BO_NULL_ERROR(player());
	return false;
 }
 if (!player()->speciesTheme()) {
	BO_NULL_ERROR(player()->speciesTheme());
	return false;
 }
 BosonProfiler profiler("LoadPlayerGameData");

 boDebug(270) << k_funcinfo << player()->bosonId() << endl;

 if (!player()->speciesTheme()->readUnitConfigs()) {
	boError(270) << k_funcinfo << "reading unit configs failed" << endl;
	return false;
 }

 if (!player()->speciesTheme()->loadTechnologies()) {
	boError(270) << k_funcinfo << "loading technologies failed" << endl;
	return false;
 }

 boDebug(270) << k_funcinfo << "done" << endl;
 return true;
}

unsigned int BosonStartingLoadPlayerGameData::taskDuration() const
{
 if (!player()) {
	return 0;
 }
 return 200;
}

void BosonStartingLoadPlayerGameData::setPlayer(Player* p)
{
 mPlayer = p;
}



bool BosonStartingStartScenario::startTask()
{
 PROFILE_METHOD
 if (!boGame) {
	BO_NULL_ERROR(boGame);
	return false;
 }

 if (!createMoveDatas()) {
	boError(270) << k_funcinfo << "creation of MoveDatas failed" << endl;
	return false;
 }

 BosonSaveLoad load(boGame);
 if (!load.loadKGameFromXML(*mFiles)) {
	boError(270) << k_funcinfo << "unable to load KGame from XML" << endl;
	return false;
 }
 if (!load.loadPlayersFromXML(*mFiles)) {
	boError(270) << k_funcinfo << "unable to load players from XML" << endl;
	return false;
 }
 boDebug(270) << k_funcinfo << boGame->gamePlayerCount() << " players loaded" << endl;

 if (!load.loadCanvasFromXML(*mFiles)) {
	boError(270) << k_funcinfo << "unable to load canvas from XML" << endl;
	return false;
 }

 return true;
}

unsigned int BosonStartingStartScenario::taskDuration() const
{
 return 500;
}

bool BosonStartingStartScenario::createMoveDatas()
{
 if (!mCanvas) {
	BO_NULL_ERROR(mCanvas);
	return false;
 }

 // TODO

 return true;
}

