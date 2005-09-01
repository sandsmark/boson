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

#include "bosonstarting.h"
#include "bosonstarting.moc"

#include "../bomemory/bodummymemory.h"
#include "defines.h"
#include "boson.h"
#include "player.h"
#include "bosonplayfield.h"
#include "bosonmap.h"
#include "bosongroundtheme.h"
#include "bosongroundthemedata.h"
#include "bosonmessageids.h"
#include "speciestheme.h"
#include "speciesdata.h"
#include "bosonprofiling.h"
#include "bosondata.h"
#include "bosoncanvas.h"
#include "bodebug.h"
#include "bosonsaveload.h"
#include "boevent.h"
#include "bosoneffectproperties.h"
#include "bowaterrenderer.h"
#include "script/bosonscript.h"
#include "unit.h"
#include "bosonviewdata.h"
#include "gameview/bosongameview.h"

#include <klocale.h>
#include <kgame/kmessageclient.h>

#include <qtimer.h>
#include <qdom.h>
#include <qmap.h>
#include <qregexp.h>

/**
 * Calls @ref Boson::lock on construction and @ref Boson::unlock on destruction,
 * or on @ref unlock.
 **/
class BosonStartingBosonLocker
{
public:
	BosonStartingBosonLocker(Boson* b)
	{
		mBoson = b;
		mBoson->lock();
		mIsLocked = true;
	}
	~BosonStartingBosonLocker()
	{
		unlock();
	}

	void unlock()
	{
		if (mIsLocked) {
			mBoson->unlock();
			mIsLocked = false;
		}
	}

private:
	Boson* mBoson;
	bool mIsLocked;
};

class BosonStartingPrivate
{
public:
	BosonStartingPrivate()
	{
		mGameView = 0;
	}
	QValueList<Q_UINT32> mStartingCompleted; // clients that completed starting
	QMap<unsigned int, QByteArray> mStartingCompletedMessage;

	QString mLoadFromLogFile;

	BosonGameView* mGameView;
};

BosonStarting::BosonStarting(QObject* parent) : QObject(parent, "bosonstarting")
{
 d = new BosonStartingPrivate;
 mDestPlayField = 0;
}

BosonStarting::~BosonStarting()
{
 delete mDestPlayField;
 delete d;
}

void BosonStarting::setGameView(BosonGameView* gameView)
{
 d->mGameView = gameView;
}

void BosonStarting::slotSetNewGameData(const QByteArray& data, bool* taken)
{
 if (taken) {
	if (*taken) {
		boError() << k_funcinfo << "data has been taken before already! only the starting object should take it!" << endl;
		// don't return
	}
 }
 mNewGameData = data;
 if (taken) {
	*taken = true;
 }
}

void BosonStarting::setEditorMap(const QByteArray& buffer)
{
}

void BosonStarting::setLoadFromLogFile(const QString& file)
{
 // TODO check if thats a valid file
 d->mLoadFromLogFile = file;
}

QString BosonStarting::logFile() const
{
 return d->mLoadFromLogFile;
}

void BosonStarting::startNewGame()
{
 boDebug(270) << k_funcinfo << endl;

 // we need to do this with timer because we can't add checkEvents() here. there is a
 // (more or less) KGame bug in KDE < 3.1 that causes KGame to go into an
 // infinite loop when calling checkEvents() from a slot that gets called from a
 // network message (exact: from KMessageDirect::received())
 QTimer::singleShot(0, this, SLOT(slotStart()));
}

bool BosonStarting::executeTasks(const QPtrList<BosonStartingTask>& tasks)
{
 unsigned long int duration = 0;
 for (QPtrListIterator<BosonStartingTask> it(tasks); it.current(); ++it) {
	disconnect(it.current(), SIGNAL(signalStartSubTask(const QString&)), this, 0);
	connect(it.current(), SIGNAL(signalStartSubTask(const QString&)),
			this, SIGNAL(signalLoadingStartSubTask(const QString&)));
	disconnect(it.current(), SIGNAL(signalCompleteSubTask(unsigned int)), this, 0);
	connect(it.current(), SIGNAL(signalCompleteSubTask(unsigned int)),
			this, SIGNAL(signalLoadingTaskCompleted(unsigned int)));

	duration += it.current()->taskDuration();
 }
 emit signalLoadingMaxDuration(duration);

 duration = 0;
 emit signalLoadingTaskCompleted(duration);
 for (QPtrListIterator<BosonStartingTask> it(tasks); it.current(); ++it) {
	boDebug(270) << k_funcinfo << "starting task: " << it.current()->text() << endl;
	emit signalLoadingStartTask(it.current()->text());
	emit signalLoadingStartSubTask("");
	if (!it.current()->start(duration)) {
		boError(270) << k_funcinfo << "could not complete task " << it.current()->text() << endl;

		return false;
	}
	boDebug(270) << k_funcinfo << "completed task: " << it.current()->text() << endl;

	duration += it.current()->taskDuration();
	emit signalLoadingTaskCompleted(duration);
 }
 return true;
}

void BosonStarting::slotStart()
{
 boDebug(270) << k_funcinfo << "STARTING" << endl;
 if (!start()) {
	boError(270) << k_funcinfo << "STARTING: game starting failed" << endl;
	emit signalStartingFailed();
	return;
 }
 boDebug(270) << k_funcinfo << "STARTING: game starting succeeded" << endl;
}

void BosonStarting::slotPlayFieldCreated(BosonPlayField* playField, bool* ownerChanged)
{
 if (!ownerChanged) {
	BO_NULL_ERROR(ownerChanged);
	return;
 }
 mDestPlayField = playField;
 *ownerChanged = true;

 emit signalDestPlayField(mDestPlayField);
}

bool BosonStarting::start()
{
 BosonProfiler profiler("BosonStarting::start()", "Starting");
 d->mStartingCompleted.clear();

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
 if (boGame->playerCount() < 2) {
	boError(270) << k_funcinfo << "not enough players in game. there must be at least a real player and a (internal) neutral player" << endl;
	return false;
 }

 QMap<QString, QByteArray> files;
 if (!BosonPlayField::unstreamFiles(files, mNewGameData)) {
	boError(270) << k_funcinfo << "invalid newgame stream" << endl;
	return false;
 }

 BosonStartingBosonLocker lock(boGame); // calls Boson::lock() and unlocks it again on destruction

 QPtrList<BosonStartingTask> tasks;
 tasks.setAutoDelete(true);

 BosonStartingLoadPlayField* loadPlayField = new BosonStartingLoadPlayField(i18n("Load PlayField"));
 connect(loadPlayField, SIGNAL(signalPlayFieldCreated(BosonPlayField*, bool*)),
		this, SLOT(slotPlayFieldCreated(BosonPlayField*, bool*)));
 loadPlayField->setFiles(&files);
 tasks.append(loadPlayField);

 BosonStartingCreateCanvas* createCanvas = new BosonStartingCreateCanvas(i18n("Create Canvas"));
 connect(this, SIGNAL(signalDestPlayField(BosonPlayField*)),
		createCanvas, SLOT(slotSetDestPlayField(BosonPlayField*)));
 connect(createCanvas, SIGNAL(signalCanvasCreated(BosonCanvas*)),
		this, SIGNAL(signalCanvas(BosonCanvas*)));
 tasks.append(createCanvas);


 // AB: We can't do this
 // when players are initialized, as the map must be known to them once we start
 // loading the units (for *loading* games)
 BosonStartingInitPlayerMap* playerMap = new BosonStartingInitPlayerMap(i18n("Init Player Map"));
 connect(this, SIGNAL(signalDestPlayField(BosonPlayField*)),
		playerMap, SLOT(slotSetDestPlayField(BosonPlayField*)));
 tasks.append(playerMap);

 BosonStartingInitScript* script = new BosonStartingInitScript(i18n("Init Script"));
 connect(this, SIGNAL(signalCanvas(BosonCanvas*)),
		script, SLOT(slotSetCanvas(BosonCanvas*)));
 tasks.append(script);

 BosonStartingLoadTiles* tiles = new BosonStartingLoadTiles(i18n("Load Tiles"));
 connect(this, SIGNAL(signalDestPlayField(BosonPlayField*)),
		tiles, SLOT(slotSetDestPlayField(BosonPlayField*)));
 tasks.append(tiles);

 BosonStartingLoadEffects* effects = new BosonStartingLoadEffects(i18n("Load Effects"));
 tasks.append(effects);

 for (QPtrListIterator<KPlayer> it(*(boGame->playerList())); it.current(); ++it) {
	Player* p = (Player*)it.current();
	QString text;
	unsigned int index = boGame->playerList()->find(it.current());
	if (index < boGame->playerCount() - 1) {
		text = i18n("Load player data of player %1 (of %2)").arg(index + 1).arg(boGame->playerCount() - 1);
	} else {
		text = i18n("Load player data of neutral player");
	}
	BosonStartingLoadPlayerData* playerData = new BosonStartingLoadPlayerData(text);
	playerData->setPlayer(p);
	tasks.append(playerData);
 }

 BosonStartingLoadWater* water = new BosonStartingLoadWater(i18n("Load Water"));
 connect(this, SIGNAL(signalDestPlayField(BosonPlayField*)),
		water, SLOT(slotSetDestPlayField(BosonPlayField*)));
 tasks.append(water);

 BosonStartingStartScenario* scenario = new BosonStartingStartScenario(i18n("Start Scenario"));
 connect(this, SIGNAL(signalDestPlayField(BosonPlayField*)),
		scenario, SLOT(slotSetDestPlayField(BosonPlayField*)));
 connect(this, SIGNAL(signalCanvas(BosonCanvas*)),
		scenario, SLOT(slotSetCanvas(BosonCanvas*)));
 scenario->setGameView(d->mGameView);
 scenario->setFiles(&files);
 tasks.append(scenario);

 bool ret = executeTasks(tasks);
 tasks.setAutoDelete(true);
 tasks.clear();
 if (!ret) {
	return false;
 }


 sendStartingCompleted(true);
 return true;
}

QByteArray BosonStarting::loadGame(const QString& loadingFileName)
{
 if (loadingFileName.isNull()) {
	boError(260) << k_funcinfo << "Cannot load game with NULL filename" << endl;
	//TODO: set Boson::loadingStatus()
	return QByteArray();
 }
 BosonPlayField loadField;
 if (!loadField.preLoadPlayField(loadingFileName)) {
	boError(260) << k_funcinfo << "could not preload " << loadingFileName << endl;
	return QByteArray();
 }

 QMap<QString, QByteArray> files;
 if (!loadField.loadFromDiskToFiles(files)) {
	boError(260) << k_funcinfo << "could not load " << loadingFileName << endl;
	return QByteArray();
 }
 QByteArray playField = BosonPlayField::streamFiles(files);
 if (playField.size() == 0) {
	boError(260) << k_funcinfo << "empty playfield loaded from " << loadingFileName << endl;
	return QByteArray();
 }

 if (!files.contains("players.xml")) {
	boError(260) << k_funcinfo << "did not find players.xml" << endl;
	return QByteArray();
 }
 if (!addLoadGamePlayers(files["players.xml"])) {
	boError(260) << k_funcinfo << "adding players failed" << endl;
	return QByteArray();
 }

 boDebug(270) << k_funcinfo << "done" << endl;

 return playField;
}

void BosonStarting::checkEvents()
{
 if (qApp->hasPendingEvents()) {
	qApp->processEvents(100);
 }
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
 boDebug(260) << k_funcinfo << "adding " << list.count() << " players" << endl;
 for (unsigned int i = 0; i < list.count(); i++) {
	QDomElement p = list.item(i).toElement();
	bool ok = false;
	unsigned int id = p.attribute("PlayerId").toUInt(&ok);
	if (!ok) {
		boError(260) << k_funcinfo << "invalid PlayerId" << endl;
		return false;
	}
	QDomElement speciesTheme = p.namedItem("SpeciesTheme").toElement();
	if (speciesTheme.isNull()) {
		boError(260) << k_funcinfo << "NULL SpeciesTheme tag for player " << i<< endl;
		return false;
	}
	QString species = speciesTheme.attribute(QString::fromLatin1("Identifier"));
	QColor color;
	color.setRgb(speciesTheme.attribute(QString::fromLatin1("TeamColor")).toUInt(&ok));
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
	if (boGame->findPlayerByUserId(id)) {
		boError(260) << k_funcinfo << "id " << id << " already in the game" << endl;
		return false;
	}
	Player* player = new Player();
	boWarning() << k_funcinfo << "probably call player->setUserId(id); here" << endl;
	player->loadTheme(SpeciesTheme::speciesDirectory(species), color);

	boGame->bosonAddPlayer(player);
 }

 return true;
}

void BosonStarting::slotStartingCompletedReceived(const QByteArray& buffer, Q_UINT32 sender)
{
 if (!d->mStartingCompleted.contains(sender)) {
	d->mStartingCompleted.append(sender);
	d->mStartingCompletedMessage.insert(sender, buffer);
 }

 if (!boGame->isAdmin()) {
	return;
 }
 QValueList<Q_UINT32> clients = boGame->messageClient()->clientList();
 if (clients.count() > d->mStartingCompleted.count()) {
	return;
 }
 QValueList<Q_UINT32>::Iterator it;
 for (it = clients.begin(); it != clients.end(); ++it) {
	if (!d->mStartingCompleted.contains(*it)) {
		return;
	}
 }

 boDebug(270) << k_funcinfo << "received IdGameStartingCompleted from all clients." << endl;
 if (!checkStartingCompletedMessages()) {
	#warning TODO
	// TODO: abort game starting.
	// AB: we cannot use return, as then boson would be in a unusable state
	// (cannot leave loading widget)
	boError(270) << k_funcinfo << "starting messages broken." << endl;
	boError(270) << k_funcinfo << "TODO: abort game starting" << endl;
 } else {
	boDebug(270) << k_funcinfo << "all IdGameStartingCompleted valid. starting the game." << endl;
 }

 // AB: _first_ set the new game status.
 // note: Boson is in PolicyClean, so the game status does *not* change
 // immediately. but it will change before IdGameIsStarted is received.
 boGame->setGameStatus(KGame::Run);

 // AB: d->mLoadFromLogFile is null usually. non-null makes sense only for
 // non-network games, we will not start a normal game, but reproduce from a log
 // file then.
 boGame->sendMessage(d->mLoadFromLogFile, BosonMessageIds::IdGameIsStarted);
}

void BosonStarting::sendStartingCompleted(bool success)
{
 QByteArray b;
 QDataStream stream(b, IO_WriteOnly);
 stream << (Q_INT8)success;
 QCString themeMD5;
 for (unsigned int i = 0; i < boGame->playerCount(); i++) {
	Player* p = (Player*)boGame->playerList()->at(i);
	SpeciesTheme* theme = p->speciesTheme();
	if (!theme) {
		// make an invalid string.
		themeMD5 = QCString();
		break;
	}
	QCString num;
	num.setNum(p->bosonId());
	themeMD5 += QCString("Player ") + num + ":\n";
	themeMD5 += "UnitProperties:\n" + theme->unitPropertiesMD5();
	themeMD5 += "\n";
 }
 stream << themeMD5;

 boGame->sendMessage(b, BosonMessageIds::IdGameStartingCompleted);
}

bool BosonStarting::checkStartingCompletedMessages() const
{
 if (!boGame->isAdmin()) {
	boError(270) << k_funcinfo << "only ADMIN can do this" << endl;
	return false;
 }
 QByteArray admin = d->mStartingCompletedMessage[boGame->gameId()];
 if (admin.size() == 0) {
	boError(270) << k_funcinfo << "have not StartingCompleted message from ADMIN" << endl;
	return false;
 }
 QDataStream adminStream(admin, IO_ReadOnly);
 Q_INT8 adminSuccess;
 adminStream >> adminSuccess;
 if (!adminSuccess) {
	boError(270) << k_funcinfo << "ADMIN failed in game starting" << endl;
	return false;
 }
 QCString adminThemeMD5;
 adminStream >> adminThemeMD5;
 if (adminThemeMD5.isNull()) {
	boError(270) << k_funcinfo << "no MD5 string for themes by ADMIN" << endl;
	return false;
 }
 QMap<unsigned int, QByteArray>::Iterator it = d->mStartingCompletedMessage.begin();
 for (; it != d->mStartingCompletedMessage.end(); ++it) {
	if (it.key() == boGame->gameId()) {
		continue;
	}
	QDataStream stream(it.data(), IO_ReadOnly);
	Q_INT8 success;
	stream >> success;
	if (!success) {
		boError(270) << k_funcinfo << "client " << it.key() << " failed on game starting" << endl;
		return false;
	}
	QCString themeMD5;
	stream >> themeMD5;
	if (themeMD5 != adminThemeMD5) {
		boError(270) << k_funcinfo << "theme MD5 sums of client "
				<< it.key()
				<< " and ADMIN differ." << endl
				<< "ADMIN has: " << adminThemeMD5 << endl
				<< "client " << it.key() << " has: " << themeMD5 << endl;
		return false;
	}
 }
 return true;
}

BosonStartingTask::BosonStartingTask(const QString& text)
	: QObject(0)
{
 mText = text;
 mTimePassed = 0;
}

BosonStartingTask::~BosonStartingTask()
{
}

bool BosonStartingTask::start(unsigned int timePassed)
{
 mTimePassed = timePassed;
 return startTask();
}

void BosonStartingTask::startSubTask(const QString& text)
{
 emit signalStartSubTask(text);
}

void BosonStartingTask::completeSubTask(unsigned int duration)
{
 if (duration > taskDuration()) {
	boError(270) << k_funcinfo << "a sub task must not take more time than the whole task! (" << duration << " > " << taskDuration() << endl;
	duration = taskDuration();
 }
 emit signalCompleteSubTask(mTimePassed + duration);
}

void BosonStartingTask::checkEvents()
{
 if (qApp->hasPendingEvents()) {
	qApp->processEvents(100);
 }
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
	boError(270) << k_funcinfo << "owner not changed, connect to signal!" << endl;
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
 connect(canvas, SIGNAL(signalItemAdded(BosonItem*)),
		boViewData, SLOT(slotAddItemContainerFor(BosonItem*)));
 connect(canvas, SIGNAL(signalRemovedItem(BosonItem*)),
		boViewData, SLOT(slotRemoveItemContainerFor(BosonItem*)));

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
 for (unsigned int i = 0; i < boGame->playerCount(); i++) {
	boDebug(270) << "init map for player " << i << endl;
	Player* p = (Player*)boGame->playerList()->at(i);
	if (p) {
		p->initMap(playField()->map(), boGame->gameMode());
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

bool BosonStartingLoadTiles::startTask()
{
 if (!boGame) {
	boError(270) << k_funcinfo << "NULL boson object" << endl;
	return false;
 }
 if (!playField()) {
	boError(270) << k_funcinfo << "NULL playField" << endl;
	return false;
 }
 if (!playField()->map()) {
	boError(270) << k_funcinfo << "NULL map" << endl;
	return false;
 }
 if (!playField()->map()->groundTheme()) {
	boError(270) << k_funcinfo << "NULL groundtheme" << endl;
	return false;
 }
 boProfiling->push("LoadTiles");

 checkEvents();

 // actually load the theme, including textures.
 // AB: note: this is a noop if we started a game before (the groundtheme datas
 //           are not deleted when the game ends)
 boViewData->addGroundTheme(playField()->map()->groundTheme());

 if (!boViewData->groundThemeData(playField()->map()->groundTheme())) {
	boError() << k_funcinfo << "loading groundtheme data object failed" << endl;
	return false;
 }

 boProfiling->pop(); // LoadTiles
 return true;
}

unsigned int BosonStartingLoadTiles::taskDuration() const
{
 return 1000;
}

bool BosonStartingLoadEffects::startTask()
{
 boEffectPropertiesManager->loadEffectProperties();
 return true;
}

unsigned int BosonStartingLoadEffects::taskDuration() const
{
 return 100;
}

bool BosonStartingLoadPlayerData::startTask()
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
 BosonProfiler profiler("LoadPlayerData");

 boDebug(270) << k_funcinfo << player()->bosonId() << endl;
 // Order of calls below is very important!!! Don't change this unless you're sure you know what you're doing!!!

 boViewData->addSpeciesTheme(player()->speciesTheme());
 SpeciesData* speciesData = boViewData->speciesData(player()->speciesTheme());
 if (!speciesData) {
	BO_NULL_ERROR(speciesData);
	return false;
 }

 mDuration = 0;

 startSubTask(i18n("Actions..."));
 if (!speciesData->loadActions()) {
	boError(270) << k_funcinfo << "loading actions failed" << endl;
	return false;
 }
 mDuration = 25;
 completeSubTask(mDuration);

 startSubTask(i18n("Objects..."));
 if (!speciesData->loadObjects(player()->speciesTheme()->teamColor())) {
	boError(270) << k_funcinfo << "loading objects failed" << endl;
	return false;
 }
 mDuration = 50;
 completeSubTask(mDuration);

 startSubTask(i18n("Unit config files..."));
 if (!player()->speciesTheme()->readUnitConfigs()) {
	boError(270) << k_funcinfo << "reading unit configs failed" << endl;
	return false;
 }
 mDuration = 150;
 completeSubTask(mDuration);

 if (!loadUnitDatas()) {
	return false;
 }

 startSubTask(i18n("Technologies..."));
 if (!player()->speciesTheme()->loadTechnologies()) {
	boError(270) << k_funcinfo << "loading technologies failed" << endl;
	return false;
 }
 mDuration = durationBeforeUnitLoading() + loadUnitDuration() + 50;
 completeSubTask(mDuration);

 // AB: atm only the sounds of the local player are required, but I believe this
 // can easily change.
 startSubTask(i18n("Sounds..."));
 if (!speciesData->loadGeneralSounds()) {
	boError(270) << k_funcinfo << "loading general sounds failed" << endl;
	return false;
 }
 mDuration = durationBeforeUnitLoading() + loadUnitDuration() + 100;
 completeSubTask(mDuration);


 boDebug(270) << k_funcinfo << "done" << endl;
 return true;
}

unsigned int BosonStartingLoadPlayerData::taskDuration() const
{
 if (!player()) {
	return 0;
 }
 return durationBeforeUnitLoading() + loadUnitDuration() + 100;
}

void BosonStartingLoadPlayerData::setPlayer(Player* p)
{
 mPlayer = p;
}

unsigned int BosonStartingLoadPlayerData::durationBeforeUnitLoading() const
{
 return 150;
}

unsigned int BosonStartingLoadPlayerData::loadUnitDuration() const
{
 // AB: it would be nicer to use unitCount * 100 or so, but we don't have
 // unitCount yet, as speciestheme isn't fully loaded yet.
 return 1500;
}

bool BosonStartingLoadPlayerData::loadUnitDatas()
{
 startSubTask(i18n("Units..."));

 // AB: this is to ensure that we really are where we expect to be
 mDuration = durationBeforeUnitLoading();
 completeSubTask(mDuration);

 checkEvents();

 SpeciesData* speciesData = boViewData->speciesData(player()->speciesTheme());

 // First get all id's of units
 QValueList<unsigned long int> unitIds;
 unitIds += player()->speciesTheme()->allFacilities();
 unitIds += player()->speciesTheme()->allMobiles();
 QValueList<unsigned long int>::iterator it;
 int currentUnit = 0;
 float factor = 0.0f;
 for (it = unitIds.begin(); it != unitIds.end(); ++it, currentUnit++) {
	startSubTask(i18n("Unit %1 of %2...").arg(currentUnit).arg(unitIds.count()));

	const UnitProperties* prop = player()->speciesTheme()->unitProperties(*it);
	if (!speciesData->loadUnit(prop, player()->speciesTheme()->teamColor())) {
		boError(270) << k_funcinfo << "loading unit with ID " << *it << " failed" << endl;
		return false;
	}

	factor = ((float)currentUnit + 1) / ((float)unitIds.count());
	completeSubTask(durationBeforeUnitLoading() + (int)(loadUnitDuration() * factor));
	checkEvents();
 }

 // make sure that we are where we expect to be
 mDuration = durationBeforeUnitLoading() + loadUnitDuration();
 return true;
}

bool BosonStartingLoadWater::startTask()
{
 if (!playField()) {
	BO_NULL_ERROR(playField());
	return false;
 }
 if (!playField()->map()) {
	BO_NULL_ERROR(playField()->map());
	return false;
 }
 if (!playField()->map()->lakes()) {
	BO_NULL_ERROR(playField()->lakes());
	return false;
 }
 boWaterRenderer->setMap(playField()->map());
 boWaterRenderer->loadNecessaryTextures();
 return true;
}

unsigned int BosonStartingLoadWater::taskDuration() const
{
 return 100;
}

void BosonStartingStartScenario::setGameView(BosonGameView* gameView)
{
 mGameView = gameView;
}

bool BosonStartingStartScenario::startTask()
{
 if (!boGame) {
	BO_NULL_ERROR(boGame);
	return false;
 }
 if (!mDestPlayField) {
	BO_NULL_ERROR(mDestPlayField);
	return false;
 }
 if (!mGameView) {
	BO_NULL_ERROR(mGameView);
	return false;
 }

 if (!createMoveDatas()) {
	boError(270) << k_funcinfo << "creation of MoveDatas failed" << endl;
	return false;
 }

 MobileUnit::initCellIntersectionTable();

 mGameView->setCanvas(mCanvas);

 BosonSaveLoad* load = new BosonSaveLoad(boGame);
 if (!load->startFromFiles(*mFiles)) {
	boError(270) << k_funcinfo << "failed starting game" << endl;
	delete load;
	return false;
 }
 delete load;
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

