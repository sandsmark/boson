/*
    This file is part of the Boson game
    Copyright (C) 2002-2003 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include "boson.h"
#include "player.h"
#include "bosonplayfield.h"
#include "bosonmap.h"
#include "bosonmessage.h"
#include "speciestheme.h"
#include "bosonprofiling.h"
#include "startupwidgets/bosonloadingwidget.h"
#include "bosondata.h"
#include "bodebug.h"
#include "bpfdescription.h"
#include "bosonfileconverter.h"
#include "bosonsaveload.h"

#include <klocale.h>

#include <qtimer.h>
#include <qdom.h>
#include <qmap.h>

BosonStarting::BosonStarting(QObject* parent) : QObject(parent, "bosonstarting")
{
 mDestPlayField = 0;
 mNewPlayField = 0; // in case we are starting a new map
 mLoading = false;
}

BosonStarting::~BosonStarting()
{
 delete mNewPlayField;
}

void BosonStarting::setNewGameData(const QByteArray& data)
{
 mNewGameData = data;
}

void BosonStarting::setEditorMap(const QByteArray& buffer)
{
}

void BosonStarting::startNewGame()
{
 boDebug() << k_funcinfo << endl;
 mLoading = false; // we are starting a new game

 emit signalLoadingShowProgressBar(true);
 emit signalLoadingSetLoading(false);

 // we need to do this with timer because we can't add checkEvents() here. there is a
 // (more or less) KGame bug in KDE < 3.1 that causes KGame to go into an
 // infinite loop when calling checkEvents() from a slot that gets called from a
 // network message (exact: from KMessageDirect::received())
 QTimer::singleShot(0, this, SLOT(slotStart()));
}

void BosonStarting::slotStart()
{
 if (!start()) {
	// in case we forgot this somewhere
	boGame->unlock();

	boError() << k_funcinfo << "game starting failed" << endl;
	emit signalStartingFailed();
	return;
 }
 boDebug() << k_funcinfo << "game starting succeeded" << endl;
}

bool BosonStarting::start()
{
 // Reset progressbar
 emit signalLoadingReset();
 emit signalLoadingSetAdmin(boGame->isAdmin());
 emit signalLoadingPlayersCount(boGame->playerList()->count());

 boDebug() << k_funcinfo << endl;
 if (!boGame) {
	boError() << k_funcinfo << "NULL boson object" << endl;
	return false;
 }
 if (boGame->gameStatus() != KGame::Init) {
	boError() << k_funcinfo
		<< "Boson must be in init status to receive map!" << endl
		<< "Current status: " << boGame->gameStatus() << endl;
	return false;
 }
 if (!mDestPlayField) {
	boError() << k_funcinfo << "NULL playfield" << endl;
	return false;
 }
 // mDestPlayField should be empty - ensure this by deleting the map
 mDestPlayField->deleteMap();


 // AB: Receiving the map is obsolete.
 emit signalLoadingType(BosonLoadingWidget::ReceiveMap);

 QMap<QString, QByteArray> files;
 if (!BosonPlayField::unstreamFiles(files, mNewGameData)) {
	boError() << k_funcinfo << "invalid newgame stream" << endl;
	return false;
 }
 if (!mDestPlayField->loadPlayField(files)) {
	boError() << k_funcinfo << "error loading the playfield" << endl;
	return false;
 }
 boDebug() << k_funcinfo << "playfield loaded" << endl;

 // - the scenario (see BosonScenario::loadScenarioFromDocument())



 boGame->setPlayField(mDestPlayField);
 emit signalAssignMap(); // for the BosonWidgetBase

 boGame->lock();
 if (!loadTiles()) {
	boError() << k_funcinfo << "Could not load tiles" << endl;
	boGame->unlock();
	return false;
 }
 if (!loadGameData3()) {
	boError() << k_funcinfo << "loading game data failed" << endl;
	boGame->unlock();
	return false;
 }
 emit signalLoadingType(BosonLoadingWidget::StartingGame);
 if (!startScenario()) {
	boError() << k_funcinfo << "starting scenario failed" << endl;
	boGame->unlock();
	return false;
 }
 if (!startGame()) {
	boError() << k_funcinfo << "starting game failed" << endl;
	boGame->unlock();
	return false;
 }
 boGame->unlock();
 return true;
}

bool BosonStarting::loadGame(const QString& loadingFileName)
{
 // If mLoading true, then we're loading saved game; if it's false, we're
 //  starting new game
 mLoading = true;
 if (loadingFileName.isNull()) {
	boError(260) << k_funcinfo << "Cannot load game with NULL filename" << endl;
	//TODO: set Boson::loadingStatus()
	return false;
 }
 BosonPlayField loadField;
 if (!loadField.preLoadPlayField(loadingFileName)) {
	boError(260) << k_funcinfo << "could not preload " << loadingFileName << endl;
	return false;
 }

 QMap<QString, QByteArray> files;
 if (!loadField.loadFromDiskToFiles(files)) {
	boError(260) << k_funcinfo << "could not load " << loadingFileName << endl;
	return false;
 }
 mNewGameData = BosonPlayField::streamFiles(files);
 if (mNewGameData.size() == 0) {
	boError(260) << k_funcinfo << "empty playfield loaded from " << loadingFileName << endl;
	return false;
 }

 if (!files.contains("players.xml")) {
	boError(260) << k_funcinfo << "did not find players.xml" << endl;
	return false;
}
 if (!addLoadGamePlayers(files["players.xml"])) {
	boError(260) << k_funcinfo << "adding players failed" << endl;
	return false;
 }

 return true;
}

void BosonStarting::checkEvents()
{
 if (qApp->hasPendingEvents()) {
	qApp->processEvents(100);
 }
}

bool BosonStarting::loadTiles()
{
 // Load map tiles.

 // Note that this is called using a QTimer::singleShot(), so you
 // can safely use checkEvents() here

 if (!boGame) {
	boError() << k_funcinfo << "NULL boson object" << endl;
	return false;
 }
 if (!playField()) {
	boError() << k_funcinfo << "NULL playField" << endl;
	return false;
 }
 if (!playField()->map()) {
	boError() << k_funcinfo << "NULL map" << endl;
	return false;
 }
 boProfiling->start(BosonProfiling::LoadTiles);

 emit signalLoadingType(BosonLoadingWidget::LoadTiles);
 // just in case.. disconnect before connecting. the map should be newly
 // created, but i don't know if this will stay this way.
 disconnect(playField()->map(), 0, this, 0);
 checkEvents();

 // actually load the theme, including textures.
 BosonData::bosonData()->loadGroundTheme(QString::fromLatin1("earth"));

 boProfiling->stop(BosonProfiling::LoadTiles);
 return true;
}

bool BosonStarting::loadGameData3() // FIXME rename!
{
 boProfiling->start(BosonProfiling::LoadGameData3);

 if (!loadPlayerData()) { // FIXME: most of the stuff below should be in this method, too!
	boError() << k_funcinfo << "player loading failed" << endl;
	return false;
 }

 emit signalLoadingType(BosonLoadingWidget::InitGame);

 checkEvents();

 if (!mLoading) {
	// emits a signal using a QTimer only - it returns immediately.
	boGame->initFogOfWar(this);
 } else if (mLoading) {
	// If we're loading saved game, init fog of war for local player
#warning LOADING code: FIXME
	// why is this called directly and not using sendMessage() ??
//	d->mBosonWidget->slotInitFogOfWar(); // FIXME
 }

 boProfiling->stop(BosonProfiling::LoadGameData3);
 boDebug() << k_funcinfo << "done" << endl;
 return true;
}

bool BosonStarting::loadPlayerData()
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
	boError() << k_funcinfo << "NULL player!" << endl;
	return false;
 }

 // these are sounds like minimap activated.
 // FIXME: are there sounds of other player (i.e. non-localplayers) we need,
 // too?
 // FIXME: do we need to support player-independant sounds?
 emit signalLoadingType(BosonLoadingWidget::LoadGeneralData);
 mPlayer->speciesTheme()->loadGeneralSounds();
 boDebug() << k_funcinfo << "done" << endl;
 return true;
}

void BosonStarting::slotLoadPlayerData(Player* p)
{
 BO_CHECK_NULL_RET(p);
 BO_CHECK_NULL_RET(p->speciesTheme());
 boDebug() << k_funcinfo << p->id() << endl;
 // Order of calls below is very important!!! Don't change this unless you're sure you know what you're doing!!!
 emit signalLoadingType(BosonLoadingWidget::LoadActions);
 p->speciesTheme()->loadActions();
 emit signalLoadingType(BosonLoadingWidget::LoadObjects);
 p->speciesTheme()->loadObjects();
 emit signalLoadingType(BosonLoadingWidget::LoadParticleSystems);
 p->speciesTheme()->loadParticleSystems();
 emit signalLoadingType(BosonLoadingWidget::LoadUnitConfigs);
 p->speciesTheme()->readUnitConfigs();
 loadUnitDatas(p);
 emit signalLoadingType(BosonLoadingWidget::LoadTechnologies);
 p->speciesTheme()->loadTechnologies();
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

// FIXME: should return bool !
// (return to startup page if starting fails)
// AB: startScenario should be moved to above, so that we don't have to unstream
// the files again. but we need a qtimer::singleshot on the way to here..
bool BosonStarting::startScenario()
{
 if (!mPlayer) {
	BO_NULL_ERROR(mPlayer);
	return false;
 }
 if (mLoading) {
	boError() << k_funcinfo << "scenario doesn't need to be started on loading" << endl;
	return false;
 }
 if (!boGame) {
	BO_NULL_ERROR(boGame);
	return false;
 }
 if (!mDestPlayField) {
	BO_NULL_ERROR(mDestPlayField);
	return false;
 }
 QMap<QString, QByteArray> files;
 if (!BosonPlayField::unstreamFiles(files, mNewGameData)) {
	// oops - it was unstreamed successfully before!
	boError() << k_funcinfo << "invalid stream??" << endl;
	return false;
 }
 if (!files.contains("players.xml")) {
	boError() << k_funcinfo << "no players.xml found" << endl;
	return false;
 }
 if (!files.contains("canvas.xml")) {
	boError() << k_funcinfo << "no canvas.xml found" << endl;
	return false;
 }
 QByteArray playersXML = files["players.xml"];
 QByteArray canvasXML = files["canvas.xml"];

 // FIXME: savegames store the _id_ of the players, but the scenario (and
 // thefore this playersXML) only the player _number_
 QString errorMsg;
 int line = 0, column = 0;
 QDomDocument doc;
 if (!doc.setContent(QString(playersXML), &errorMsg, &line, &column)) {
	boError() << k_funcinfo << "unable to load playersXML - parse error at line=" << line << ",column=" << column << " errorMsg=" << errorMsg << endl;
	return false;
 }
 QDomElement root = doc.documentElement();
 QDomNodeList list = root.elementsByTagName("Player");
 if (list.count() < 1) {
	boError() << k_funcinfo << "no Player tag found" << endl;
	return false;
 }
 QMap<int, int> player2Number;
 for (unsigned int i = 0; i < list.count(); i++) {
	QDomElement e = list.item(i).toElement();
	player2Number.insert(i, e.attribute("Id").toInt());
 }
 QValueList<QDomElement> players;
 players.append(list.item(0).toElement());
 for (unsigned int i = 1; i < list.count(); i++) {
	QDomElement e = list.item(i).toElement();
	int number = player2Number[i];
	bool inserted = false;
	for (unsigned int j = 0; j < players.count() && !inserted; j++) {
		if (player2Number[j] > number) {
			players.insert(players.at(j), e);
			inserted = true;
		} else if (j + 1 == players.count()) {
			players.append(e);
			inserted = true;
		}
	}
 }

 QDomDocument canvasDoc;
 if (!canvasDoc.setContent(QString(canvasXML))) {
	boError() << k_funcinfo << "unable to load canvasXML" << endl;
	return false;
 }
 QDomElement canvasRoot = canvasDoc.documentElement();
 QDomNodeList itemsList = canvasRoot.elementsByTagName("Items");

 // players is now a list of Player nodes, sorted by their Id (acutally at this
 // point it is the playernumber only)
 for (unsigned int i = 0; i < boGame->playerCount(); i++) {
	int id = boGame->playerList()->at(i)->id();
	QDomElement e = players[i];
	int number = e.attribute("Id").toInt();
	e.setAttribute("Id", id);

	// also change the id in the OwnerId attribute of the Items tag
	for (unsigned int j = 0; j < itemsList.count(); j++) {
		QDomElement items = itemsList.item(j).toElement();
		bool ok = false;
		int ownerNumber = items.attribute("OwnerId").toInt(&ok);
		if (!ok) {
			boError() << k_funcinfo << "invalid OnwerOd" << endl;
			continue;
		}
		if (ownerNumber == number) {
			items.setAttribute("OwnerId", id);
		}
	}
 }
 root.setAttribute("LocalPlayerId", mPlayer->id());
 playersXML = doc.toCString();
 canvasXML = canvasDoc.toCString();

 // AB: note that the player list can (and very often will) contain more players
 // then the actual boGame->playerList(). the code must allow that.

 // TODO: save the in-game players to a xml string, then replace all tags in it
 // that appear in playersXML too with those in playersXML.
// boGame->reset();

 BosonSaveLoad* load = new BosonSaveLoad(boGame);
 if (!load->loadNewGame(playersXML, canvasXML)) {
	boError() << k_funcinfo << "failed starting game" << endl;
	return false;
 }
 return true;
}

bool BosonStarting::startGame()
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
	return true;
 }
 boGame->sendMessage(0, BosonMessage::IdGameIsStarted);
 return true;
}


// note: this method is _incompatible_ with network!!
// if we want loading games to be network compatible, we need to add the players
// _before_ loading the game.
bool BosonStarting::addLoadGamePlayers(const QString& playersXML)
{
 QDomDocument playersDoc;
 if (!playersDoc.setContent(playersXML)) {
	boError(260) << k_funcinfo << "error loading players.xml" << endl;
	return false;
 }
 QDomElement playersRoot = playersDoc.documentElement();
 if (boGame->playerCount() != 0) {
	boError(260) << k_funcinfo << "no player are allowed at this point" << endl;
	return false;
 }
 QDomNodeList list = playersRoot.elementsByTagName("Player");
 if (list.count() == 0) {
	boError(260) << k_funcinfo << "no players in savegame" << endl;
	return false;
 }
 bool ok = false;
 unsigned int localId = playersRoot.attribute("LocalPlayerId").toUInt(&ok);
 if (!ok) {
	boError(260) << k_funcinfo << "no LocalPlayerId" << endl;
	return false;
 }
 boDebug(260) << k_funcinfo << "adding " << list.count() << " players" << endl;
 BosonSaveLoad load(boGame);
 for (unsigned int i = 0; i < list.count(); i++) {
	// AB: TODO: store "PlayerNumber" in the xml file only (use the same
	// number for the canvas.xml as ownerid / ownernumber).
	// We should assign new IDs.
	// -­> makes the code easier to be used in newgames, too and we will
	// support network then.
	QDomElement p = list.item(i).toElement();
	unsigned int id = p.attribute("Id").toUInt(&ok);
	if (!ok) {
		boError(260) << k_funcinfo << "invalid Id" << endl;
		return false;
	}
	QString species = p.attribute(QString::fromLatin1("SpeciesTheme"));
	QColor color;
	color.setRgb(p.attribute(QString::fromLatin1("TeamColor")).toUInt(&ok));
	if (!ok) {
		boError(260) << k_funcinfo << "invalid teamcolor" << endl;
		return false;
	}
	if (species.isEmpty()) {
		boError(260) << k_funcinfo << "invalid SpeciesTheme" << endl;
		// TODO: check whether species actually exists and can get
		// loaded
		return false;
	}
	if (boGame->findPlayer(id)) {
		boError(260) << k_funcinfo << "id " << id << " already in the game" << endl;
		return false;
	}
	Player* player = (Player*)boGame->createPlayer(0, 0, false);
	if (id == localId) {
		
	}
	player->setId(id);
	player->loadTheme(SpeciesTheme::speciesDirectory(species), color);

	// AB: we _must_ add the players _before_ we load the game, in order to
	// keep network working.
//
	load.systemAddPlayer(player);
 }

 return true;
}


