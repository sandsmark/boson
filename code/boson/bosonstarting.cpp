/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bosonstarting.h"
#include "bosonstarting.moc"

#include "defines.h"
#include "bosonwidget.h"
#include "boson.h"
#include "player.h"
#include "bosonplayfield.h"
#include "bosonmap.h"
#include "bosonscenario.h" // should not be here!
#include "bosonmessage.h"
#include "speciestheme.h"
#include "bosonprofiling.h"
#include "startupwidgets/bosonloadingwidget.h"
#include "bodebug.h"

#include <klocale.h>

#include <qtimer.h>
#include <qfile.h>

BosonStarting::BosonStarting(QObject* parent) : QObject(parent)
{
 mPlayField = 0;
 mNewPlayField = 0; // in case we are starting a new map
 mLoading = false;
}

BosonStarting::~BosonStarting()
{
 delete mNewPlayField;
}

void BosonStarting::setEditorMap(const QByteArray& buffer)
{
 boDebug() << k_funcinfo << endl;
 QDataStream stream(buffer, IO_ReadOnly);
 delete mNewPlayField;
 mNewPlayField = new BosonPlayField(this);
 BosonMap* map = new BosonMap(mNewPlayField);
 map->loadMap(stream);
 mNewPlayField->changeMap(map);

 // WARNING: this is a hack. See BosonStartEditorWidget class!
 // this message should contain the map only, *not* the scenario!
 Q_INT32 maxPlayers;
 Q_INT32 minPlayers;
 stream >> maxPlayers;
 stream >> minPlayers;
 BosonScenario* scenario = new BosonScenario();
 scenario->setPlayers(minPlayers, maxPlayers);
 scenario->initializeScenario();
 mNewPlayField->changeScenario(scenario);

 mNewPlayField->finalizeLoading(); // do not preload anything or so
}

void BosonStarting::startNewGame()
{
 boDebug() << k_funcinfo << endl;
 mLoading = false; // we are starting a new game
 // Reset progressbar
 emit signalLoadingShowProgressBar(true);
 emit signalLoadingSetLoading(false);
 emit signalLoadingReset();
 emit signalLoadingSetAdmin(boGame->isAdmin());
 emit signalLoadingPlayersCount(boGame->playerList()->count());
 // Transmit playfield over the net if we're admin
 if (boGame->isAdmin()) {
	emit signalLoadingType(BosonLoadingWidget::AdminLoadMap);
	QByteArray buffer;
	QDataStream stream(buffer, IO_WriteOnly);

	// only ADMIN should access mPlayFieldId !! the playfield gets
	// transmitted through network. first map here - scenario later
	BosonPlayField* loadField = 0;
	if (!mPlayFieldId.isNull()) {
		boDebug() << k_funcinfo << "use " << mPlayFieldId << endl;
		loadField = BosonPlayField::playField(mPlayFieldId);
	} else {
		// here we should be in editor mode creating a new map
		boDebug() << k_funcinfo << "editor mode: create new map" << endl;
		loadField = mNewPlayField;
		if (!mNewPlayField) {
			boError() << k_funcinfo << "NULL new playfield!" << endl;
			return;
		}
	}
	if (!loadField) {
		boError() << k_funcinfo << "NULL playField" << endl;
		return;
	}
	loadField->loadPlayField(mPlayFieldId);
	loadField->saveMap(stream);

	// I'm not too happy about tranmsmitting the complete description
	// (including translations...) but i'm afraid we need them. in case of
	// conflicting maps the network version will be used, then we should be
	// able to provide the correct description, too
	loadField->saveDescription(stream);

	// in case we are starting a new map
	delete mNewPlayField;
	mNewPlayField = 0;

	emit signalLoadingType(BosonLoadingWidget::SendMap);
	// send the loaded map via network
	boGame->sendMessage(stream, BosonMessage::InitMap);
 }
 boDebug() << k_funcinfo << endl;
 if (!mPlayField) {
	boError() << k_funcinfo << "NULL playfield" << endl;
	return;
 }
 // mPlayField should be empty - ensure this by deleting the map
 mPlayField->deleteMap();


 // before actually starting the game we need to wait for the map (which is sent
 // by the ADMIN)
 emit signalLoadingType(BosonLoadingWidget::ReceiveMap);
 boDebug() << k_funcinfo << "done" << endl;
}

bool BosonStarting::loadGame(const QString& loadingFileName)
{
 // If mLoading true, then we're loading saved game; if it's false, we're
 //  starting new game
 mLoading = true;
 if (loadingFileName.isNull()) {
	boError() << k_funcinfo << "Cannot load game with NULL filename" << endl;
	//TODO: set Boson::loadingStatus()
	return false;
 }

 // Open file and QDataStream on it
 QFile f(loadingFileName);
 f.open(IO_ReadOnly);
 QDataStream s(&f);

 // Load game
 emit signalLoadingShowProgressBar(false);
 emit signalLoadingSetLoading(true);
 emit signalLoadingReset();
 connect(boGame, SIGNAL(signalLoadPlayerData(Player*)), this, SLOT(slotLoadPlayerData(Player*)));
 connect(boGame, SIGNAL(signalLoadingPlayersCount(int)), this, SIGNAL(signalLoadingPlayersCount(int)));
 connect(boGame, SIGNAL(signalLoadingPlayer(int)), this, SIGNAL(signalLoadingPlayer(int)));
 boGame->lock();
 bool loaded = boGame->load(s, true);
 slotLoadTiles();
 boGame->unlock();

 // Close file
 f.close();
 mLoading = false;

 startGame();
 return loaded;
}

void BosonStarting::checkEvents()
{
 if (qApp->hasPendingEvents()) {
	qApp->processEvents(100);
 }
}

void BosonStarting::slotReceiveMap(const QByteArray& buffer)
{
 boDebug() << k_funcinfo << endl;
 if (!boGame) {
	boError() << k_funcinfo << "NULL boson object" << endl;
	return;
 }

 // usually we must be in Init state to receive this. But loading code also
 // loads the gameStatus, so we won't be in Init state in that case.
 if (boGame->gameStatus() != KGame::Init) {
	boError() << k_funcinfo
		<< "Boson must be in init status to receive map!" << endl
		<< "Current status: " << boGame->gameStatus() << endl;
	return;
 }
 if (!mPlayField) {
	boError() << k_funcinfo << "NULL playfield" << endl;
	emit signalStartingFailed(); // TODO: display welcome widget or so
	return;
 }

 emit signalLoadingType(BosonLoadingWidget::LoadMap);
 QDataStream stream(buffer, IO_ReadOnly);
 if (!mPlayField->loadMap(stream)) {
	boError() << k_funcinfo << "Broken map file at this point?!" << endl;
	return;
 }
 if (!mPlayField->loadDescription(stream)) {
	boError() << k_funcinfo << "broken description" << endl;
	return;
 }
 boGame->setPlayField(mPlayField);
 emit signalAssignMap(); // for the BosonWidgetBase

 // If we're loading saved game, local player isn't set and inited, because it
 //  was not known (not loaded) when BosonWidgetBase was constructed. Set and init
 //  it now
 if (mLoading) {
	boDebug() << k_funcinfo << "set local player for loaded games now" << endl;
	if (!boGame->localPlayer()) {
		boWarning() << k_funcinfo << "NULL player" << endl;
		return;
	}
#warning FIXME for LOADING code
//	slotChangeLocalPlayer(boGame->localPlayer());
	mPlayer = boGame->localPlayer();
 }

 // we need to do this with timer because we can't add checkEvents() here. there is a
 // (more or less) KGame bug in KDE < 3.1 that causes KGame to go into an
 // infinite loop when calling checkEvents() from a slot that gets called from a
 // network message (exact: from KMessageDirect::received())
 if (!mLoading) {
	QTimer::singleShot(0, this, SLOT(slotLoadTiles()));
 } else {
	// loading code mustn't contain any singleShot()s
//	slotLoadTiles();
 }
}

void BosonStarting::slotLoadTiles()
{
 // Load map tiles. This takes most startup time

 // This slot method is called from slotReceiveMap(), which in turn is called when map
 // is received in Boson class.
 //
 // Note that slotReceiveMap() calls this using a QTimer::singleShot(), so you
 // can safely use checkEvents() here

 if (!boGame) {
	boError() << k_funcinfo << "NULL boson object" << endl;
	return;
 }
 if (!playField()) {
	boError() << k_funcinfo << "NULL playField" << endl;
	return;
 }
 if (!playField()->map()) {
	boError() << k_funcinfo << "NULL map" << endl;
	return;
 }
 boGame->lock();
 boProfiling->start(BosonProfiling::LoadTiles);

 emit signalLoadingType(BosonLoadingWidget::LoadTiles);
 // just in case.. disconnect before connecting. the map should be newly
 // created, but i don't know if this will stay this way.
 disconnect(playField()->map(), 0, this, 0);
 connect(playField()->map(), SIGNAL(signalTilesLoading(int)), this, SIGNAL(signalLoadingTile(int)));
 checkEvents();

 // Note that next call doesn't return before tiles are fully loaded (because
 //  second argument is false; if it would be true, then it would return
 //  immediately). This is needed for loading saved game. GUI is
 //  still non-blocking though, because qApp->processEvents() is called while
 //  loading tiles
 playField()->map()->loadTiles(QString("earth"), false);

 boProfiling->stop(BosonProfiling::LoadTiles);

 slotLoadGameData3(); // FIXME: not a slot

 boGame->unlock();
}

void BosonStarting::slotLoadGameData3() // FIXME rename!
{
 // this call mostly cares about network messages. It may happen that the ADMIN
 // already sends IdInitFogOfWar (e.g.) while we are still loading. This call
 // ensures that they are *not* delivered when we call checkEvents(). otherwise
 // every checkEvents() call would deliver any messages from the current event
 // loop.
 // TODO: we might still have this problem if the client takes e.g. 10 minutes
 // to load, but the ADMIN only 40 seconds or so. then the client would be in a
 // totally different loading phase. we need to ensure that thos functions are
 // locked/unlocked correctly, too! or at least don't end up in the event loop.
 //
 // the same problem appears with QTimer::singleShot(). We need to ensure that
 // the singleShot message is received first, not the network message
 // UPDATE: it was moved to another method calling this method.
// boGame->lock();

 boProfiling->start(BosonProfiling::LoadGameData3);

 loadPlayerData(); // FIXME: most of the stuff below should be in this method, too!

 emit signalLoadingType(BosonLoadingWidget::InitGame);

#warning FIXME
 checkEvents();
// FIXME this checkEvents() (and those in e.g. loadPlayerData()) are a problem
// for network games. it may happen (and probably will) that IdInitFogOfWar was
// already sent!!!
// we cannot remove them, cause we'd have blocking UI then

 if (boGame->isAdmin() && !mLoading) {
	// Send InitFogOfWar and StartScenario messages if we're starting new game
	if (boGame->gameMode()) {
		boGame->sendMessage(0, BosonMessage::IdInitFogOfWar);
	}
	boGame->sendMessage(0, BosonMessage::IdStartScenario);
 } else if (mLoading) {
	// If we're loading saved game, init fog of war for local player
#warning LOADING code: FIXME
	// why is this called directly and not using sendMessage() ??
//	d->mBosonWidget->slotInitFogOfWar(); // FIXME
 }

 boProfiling->stop(BosonProfiling::LoadGameData3);

 emit signalLoadingType(BosonLoadingWidget::StartingGame);

 boDebug() << k_funcinfo << "done" << endl;
 if (!mLoading) {
	if (boGame->isAdmin()) {
		startScenario();
	}

	// load games will do that themselves.
	startGame();
 }
// boGame->unlock();
}

void BosonStarting::loadPlayerData()
{
 boDebug() << k_funcinfo << endl;

 // Load unit datas (pixmaps and sounds), but only if we're starting new game,
 //  because if we're loading saved game, then units are already constructed
 //  and unit datas loaded
 if (!mLoading) {
	QPtrListIterator<KPlayer> it(*(boGame->playerList()));
	int currentplayer = 0;
	while (it.current()) {
		emit signalLoadingPlayer(currentplayer);
		slotLoadPlayerData((Player*)it.current());
		currentplayer++;
		++it;
	}
 }

 if (!mPlayer) {
	// TODO: loading failed. we have to return to welcome widget now and
	// display a message box!
	boError() << k_funcinfo << "NULL player!" << endl;
	return;
 }

 // these are sounds like minimap activated.
 // FIXME: are there sounds of other player (i.e. non-localplayers) we need,
 // too?
 // FIXME: do we need to support player-independant sounds?
 emit signalLoadingType(BosonLoadingWidget::LoadGeneralData);
 mPlayer->speciesTheme()->loadGeneralSounds();
 boDebug() << k_funcinfo << "done" << endl;
}

void BosonStarting::slotLoadPlayerData(Player* p)
{
 // Order of calls below is very important!!! Don't change this unless you're sure you know what you're doing!!!
 emit signalLoadingType(BosonLoadingWidget::LoadParticleSystems);
 p->speciesTheme()->loadParticleSystems();
 emit signalLoadingType(BosonLoadingWidget::LoadUnitConfigs);
 p->speciesTheme()->readUnitConfigs();
 loadUnitDatas(p);
 emit signalLoadingType(BosonLoadingWidget::LoadTechnologies);
 p->speciesTheme()->loadTechnologies();
// ((Player*)it.current())->speciesTheme()->loadObjectModels();
}

void BosonStarting::loadUnitDatas(Player* p)
{
 // This loads all unit datas for player p
 emit signalLoadingType(BosonLoadingWidget::LoadUnits);
 checkEvents();
 // First get all id's of units
 QValueList<unsigned long int> unitIds = p->speciesTheme()->allFacilities();
 unitIds += p->speciesTheme()->allMobiles();
 emit signalLoadingUnitsCount(unitIds.count());
 QValueList<unsigned long int>::iterator it;
 int currentunit = 0;
 for (it = unitIds.begin(); it != unitIds.end(); ++it, currentunit++) {
	emit signalLoadingUnit(currentunit);
	p->speciesTheme()->loadUnit(*it);
 }
}

void BosonStarting::startScenario()
{
 if (!boGame->isAdmin()) {
	boError() << "not admin" << endl;
	return;
 }
 if (mLoading) {
	boError() << k_funcinfo << "scenario doesn't need to be started on loading" << endl;
	return;
 }
 BosonPlayField* field = BosonPlayField::playField(mPlayFieldId);
 // TODO: on error we should abort starting the game, i.e. 
 // return to the welcome widget (and displaying a msg box) or so
 if (!field) {
	boError() << k_funcinfo << "NULL playfield" << endl;
 } else if (!field->scenario()) {
	boError() << k_funcinfo << "NULL scenario" << endl;
 } else {
	field->scenario()->startScenario(boGame);
 }
}

void BosonStarting::startGame()
{
 boDebug() << k_funcinfo << endl;
 // we do some final initializations here (mostly status changed) and then send
 // out IdGameIsStarted.
 // At that point all units should be on the map and it should be ready to
 // start.
 // Once the message is received the clients need to do visual initializations
 // (i.e. hide the loading progressbar, show the big display, ...)
 boGame->startGame();
 if (!boGame->isAdmin()) {
	return;
 }
 boGame->sendMessage(0, BosonMessage::IdGameIsStarted);
}

