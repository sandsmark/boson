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
#include "editorwidget.h"
#include "bosonconfig.h"
#include "boson.h"
#include "player.h"
#include "bosonplayfield.h"
#include "bosonmap.h"
#include "bosoncanvas.h"
#include "bosonmessage.h"
#include "speciestheme.h"
#include "bosonprofiling.h"
#include "bodisplaymanager.h"
#include "bosonbigdisplaybase.h"
#include "startupwidgets/bosonloadingwidget.h"

#include <kapplication.h>
#include <klocale.h>
#include <kdebug.h>
#include <kconfig.h>

#include <qcursor.h>
#include <qwidgetstack.h>
#include <qtimer.h>
#include <qhbox.h>
#include <qfile.h>

BosonStarting::BosonStarting(QObject* parent) : QObject(parent)
{
 mLoadingWidget = 0;
 mPlayField = 0;
 mLoading = false;
}

void BosonStarting::startNewGame()
{
 kdDebug() << k_funcinfo << endl;
 mLoading = false; // we are starting a new game
 mLoadingWidget->setProgress(0);
 mLoadingWidget->showProgressBar(true);
 if (boGame->isAdmin()) {
	mLoadingWidget->setLoading(BosonLoadingWidget::SendMap);
	QByteArray buffer;
	QDataStream stream(buffer, IO_WriteOnly);
	mPlayField->saveMap(stream);
	mLoadingWidget->setProgress(50);
	// send the loaded map via network
	boGame->sendMessage(stream, BosonMessage::InitMap);
	mLoadingWidget->setProgress(100);
 }
 // clients have no map, but ADMIN has. this call makes them equal
 mPlayField->deleteMap();


 // before actually starting the game we need to wait for the map (which is sent
 // by the ADMIN)
 mLoadingWidget->setLoading(BosonLoadingWidget::ReceiveMap);
}

bool BosonStarting::loadGame(const QString& loadingFileName)
{
 // If mLoading true, then we're loading saved game; if it's false, we're
 //  starting new game
 mLoading = true;
 if (loadingFileName == QString::null) {
	kdError() << k_funcinfo << "Cannot load game with NULL filename" << endl;
	//TODO: set Boson::loadingStatus()
	return false;
 }

 // Open file and QDataStream on it
 QFile f(loadingFileName);
 f.open(IO_ReadOnly);
 QDataStream s(&f);

 // Load game
 mLoadingWidget->setLoading(BosonLoadingWidget::LoadMap);
 mLoadingWidget->showProgressBar(false);
 boGame->lock();
 bool loaded = boGame->load(s, true);
 slotLoadTiles();
 boGame->unlock();

 // Close file
 f.close();
 mLoading = false;

 kdDebug() << k_funcinfo << "mit signalStartGame()" << endl;
 emit signalStartGame(); // FIXME - also in loadgamedata3
 emit signalStartGameLoadWorkaround();
 return loaded;
}

void BosonStarting::checkEvents()
{
 if (qApp->hasPendingEvents()) {
	qApp->processEvents(100);
 }
}

void BosonStarting::slotTilesLoading(int tiles)
{
 mLoadingWidget->setTileProgress(600, tiles);
}

void BosonStarting::slotReceiveMap(const QByteArray& buffer)
{
 kdDebug() << k_funcinfo << endl;
 if (!boGame) {
	kdError() << k_funcinfo << "NULL boson object" << endl;
	return;
 }

 // usually we must be in Init state to receive this. But loading code also
 // loads the gameStatus, so we won't be in Init state in that case.
 if (boGame->gameStatus() != KGame::Init) {
	kdError() << k_funcinfo
		<< "Boson must be in init status to receive map!" << endl
		<< "Current status: " << boGame->gameStatus() << endl;
	return;
 }
 if (!mPlayField) {
	kdError() << k_funcinfo << "NULL playfield" << endl;
	emit signalStartingFailed(); // TODO: display welcome widget or so
	return;
 }


 QDataStream stream(buffer, IO_ReadOnly);
 if (!mPlayField->loadMap(stream)) {
	kdError() << k_funcinfo << "Broken map file at this point?!" << endl;
	return;
 }
 boGame->setPlayField(mPlayField);
 emit signalAssignMap(); // for the BosonWidgetBase

 mLoadingWidget->setProgress(300);

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

 // If we're loading game, almost everything except map (players, units...) are
 //  loaded after this method returns. So we set correct loading label
 if (mLoading) {
	mLoadingWidget->setLoading(BosonLoadingWidget::LoadGame);
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
	kdError() << k_funcinfo << "NULL boson object" << endl;
	return;
 }
 if (!playField()) {
	kdError() << k_funcinfo << "NULL playField" << endl;
	return;
 }
 if (!playField()->map()) {
	kdError() << k_funcinfo << "NULL map" << endl;
	return;
 }
 boGame->lock();
 boProfiling->start(BosonProfiling::LoadTiles);

 mLoadingWidget->setProgress(600);
 mLoadingWidget->setLoading(BosonLoadingWidget::LoadTiles);
 // just in case.. disconnect before connecting. the map should be newly
 // created, bu i don't know if this will stay this way.
 disconnect(playField()->map(), 0, this, 0);
 connect(playField()->map(), SIGNAL(signalTilesLoading(int)), this, SLOT(slotTilesLoading(int)));
 checkEvents();

 // Note that next call doesn't return before tiles are fully loaded (because
 //  second argument is false; if it would be true, then it would return
 //  immediately). This is needed for loading saved game. GUI is
 //  still non-blocking though, because qApp->processEvents() is called while
 //  loading tiles
 playField()->map()->loadTiles(QString("earth"), false);

 mLoadingWidget->setProgress(3000);
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

 int progress = 3000 + boGame->playerList()->count() * BosonLoadingWidget::unitDataLoadingFactor();

 mLoadingWidget->setLoading(BosonLoadingWidget::InitGame);

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

 progress += 100;
 mLoadingWidget->setProgress(progress);
 mLoadingWidget->setLoading(BosonLoadingWidget::StartingGame);

 kdDebug() << k_funcinfo << "done" << endl;
 if (!mLoading) {
	emit signalStartGame();
 }
// boGame->unlock();
}

void BosonStarting::loadPlayerData()
{
 // If we're loading saved game, local player isn't set and inited, because it
 //  was not known (not loaded) when BosonWidgetBase was constructed. Set and init
 //  it now
 int progress = 0;
 kdDebug() << k_funcinfo << endl;
 if (mLoading) {
	kdDebug() << k_funcinfo << "set local player for loaded games now" << endl;
	if (!boGame->localPlayer()) {
		kdWarning() << k_funcinfo << "NULL player" << endl;
		return;
	}
#warning FIXME for LOADING code
//	slotChangeLocalPlayer(boGame->localPlayer()); //AB: can we place this after signalStartGame?
	mPlayer = boGame->localPlayer();
 }

 // Load unit datas (pixmaps and sounds), but only if we're starting new game,
 //  because if we're loading saved game, then units are already constructed
 //  and unit datas loaded
 if (!mLoading) {
	QPtrListIterator<KPlayer> it(*(boGame->playerList()));
	progress = 3000;
	while (it.current()) {
		// Order of calls below is very important!!! Don't change this unless you're sure you know what you're doing!!!
		((Player*)it.current())->speciesTheme()->loadParticleSystems();
		((Player*)it.current())->speciesTheme()->readUnitConfigs();
		loadUnitDatas(((Player*)it.current()), progress);
		((Player*)it.current())->speciesTheme()->loadTechnologies();
//		((Player*)it.current())->speciesTheme()->loadObjectModels();
		++it;
		progress += BosonLoadingWidget::unitDataLoadingFactor();
	}
 }

 // these are sounds like minimap activated.
 // FIXME: are there sounds of other player (i.e. non-localplayers) we need,
 // too?
 // FIXME: do we need to support player-independant sounds?
 mPlayer->speciesTheme()->loadGeneralSounds();
 mLoadingWidget->setProgress(progress);
 kdDebug() << k_funcinfo << "done" << endl;
}

void BosonStarting::loadUnitDatas(Player* p, int progress)
{
 // This loads all unit datas for player p
 mLoadingWidget->setProgress(progress);
 mLoadingWidget->setLoading(BosonLoadingWidget::LoadUnits);
 checkEvents();
 // First get all id's of units
 QValueList<unsigned long int> unitIds = p->speciesTheme()->allFacilities();
 unitIds += p->speciesTheme()->allMobiles();
 QValueList<unsigned long int>::iterator it;
 int current = 0;
 int total = unitIds.count();
 for (it = unitIds.begin(); it != unitIds.end(); ++it) {
	current++;
	p->speciesTheme()->loadUnit(*it);
	mLoadingWidget->setUnitProgress(progress, current, total);
 }
}



