/*
    This file is part of the Boson game
    Copyright (C) 2001-2005 Andreas Beckermann (b_mann@gmx.de)

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

#include "mainnogui.h"
#include "mainnogui.moc"

#include "../bomemory/bodummymemory.h"
#include "defines.h"
#include "bocheckinstallation.h"
#include "bosonconfig.h"
#include "boglobal.h"
#include "boapplication.h"
#include "boversion.h"
#include "bodebug.h"
#include "bodebugdcopiface.h"
#include "bo3dtools.h"
#include "boeventloop.h"
#include "bosongameengine.h"
#include "player.h"
#include "boson.h"
#include "bosonstarting.h"
#include "bosongameenginestarting.h"
#include "bosonmessageids.h"
#include "bosonplayfield.h"
#include "bosondata.h"
#include "speciestheme.h"
#include "bosoncomputerio.h"
#include <config.h>

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <qtimer.h>

static const char *description =
    I18N_NOOP("Boson without GUI");

static const char *version = BOSON_VERSION_STRING;

static KCmdLineOptions options[] =
{
    { "load", I18N_NOOP("Skip Welcome Widget and display the Load Game screen"), 0 },
    { "load-from-log <file>", I18N_NOOP("Load from emergency log, for debugging"), 0 },
    { "playfield <identifier>", I18N_NOOP("Playfield identifier for newgame/start editor widget"), 0 },
    { "computerplayers <count>", I18N_NOOP("Add <count> computer players to the game. Default is 1."), "1" },
    { "computerspecies <species>", I18N_NOOP("Comma separated list of species identifiers - one species per computer player."), 0 },
    { "networkplayers <count>", I18N_NOOP("Wait for <count> players to enter the game from network. Default is 0."), "0" },
    { "port <number>", I18N_NOOP("Port to listen on for network players"), QString("%1").arg(BOSON_PORT) },
    { "start", I18N_NOOP("Start the game"), 0},
    { "aidelay <delay>", I18N_NOOP("Set AI delay (in seconds). The less it is, the faster AI will send it's units"), 0 },
    { "noai", I18N_NOOP("Disable AI"), 0 },
    { "connectto <host:port>" I18N_NOOP("Connect to a server"), 0 },
    { 0, 0, 0 }
};


static void postBosonConfigInit();

class StartGame
{
public:
	StartGame()
	{
		mRequiredPlayers = 0;
		mPort = BOSON_PORT;
		mHost = "localhost";
		mClient = false;
	}

	bool start()
	{
		if (!mClient && mPlayField.size() == 0) {
			return false;
		}
		if (!boGame) {
			return false;
		}
		if (!mClient) {
			boGame->sendMessage(mPlayField, BosonMessageIds::IdNewGame);
		} else {
			boDebug() << k_funcinfo << "connecting to host " << mHost << " on port " << mPort << endl;
			boGame->connectToServer(mHost, mPort);
		}
		return true;
	}

	bool checkStart() const
	{
		if (!boGame) {
			return false;
		}
		if (boGame->playerCount() >= mRequiredPlayers) {
			return true;
		} else {
			boDebug() << k_funcinfo << "not enough players yet. have: " << boGame->playerCount() << " need: " << mRequiredPlayers << endl;
			return false;
		}
		return false;
	}

public:
	unsigned int mRequiredPlayers;
	QByteArray mPlayField;
	QString mHost;
	int mPort;
	bool mClient;
};

class MainNoGUIPrivate
{
public:
	MainNoGUIPrivate()
	{
		mGameEngine = 0;
		mStarting = 0;

		mStartGame = 0;
	}
	BosonGameEngine* mGameEngine;
	BosonStarting* mStarting;

	StartGame* mStartGame;
};

MainNoGUI::MainNoGUI()
	: QObject(0)
{
 d = new MainNoGUIPrivate();
 d->mGameEngine = new BosonGameEngine(0);
 d->mStarting = new BosonStarting(0);
}

MainNoGUI::~MainNoGUI()
{
 delete d->mStarting;
 delete d->mGameEngine;
 delete d;
}

bool MainNoGUI::init()
{
 if (!d->mGameEngine->preloadData()) {
	boError() << k_funcinfo << "unable to preload some data" << endl;
	KMessageBox::sorry(0, i18n("Unable to preload data. Check your installation!"), i18n("Check your installation"));
	return false;
 }
 d->mGameEngine->slotResetGame();

 if (!boGame) {
	BO_NULL_ERROR(boGame);
	return false;
 }

 QObject::connect(boGame, SIGNAL(signalGameStarted()),
		this, SLOT(slotGameStarted()));

 return true;
}

bool MainNoGUI::startGame(KCmdLineArgs* args)
{
 delete d->mStarting;
 d->mStarting = new BosonStarting(0);
 QObject::connect(boGame, SIGNAL(signalStartingCompletedReceived(const QByteArray&, Q_UINT32)),
		d->mStarting, SLOT(slotStartingCompletedReceived(const QByteArray&, Q_UINT32)));
 QObject::connect(boGame, SIGNAL(signalSetNewGameData(const QByteArray&, bool*)),
		d->mStarting, SLOT(slotSetNewGameData(const QByteArray&, bool*)));
 QObject::connect(boGame, SIGNAL(signalStartNewGame()),
		d->mStarting, SLOT(slotStartNewGameWithTimer()));
 connect(boGame, SIGNAL(signalPlayerJoinedGame(KPlayer*)),
		this, SLOT(slotPlayerJoinedGame(KPlayer*)));
 BosonGameEngineStarting* game = new BosonGameEngineStarting(d->mStarting, d->mStarting);
 d->mStarting->addTaskCreator(game);



 bool loadGame = false;
 if (args->isSet("load")) {
	boError() << k_funcinfo << "loading games is not yet supported here" << endl;
	return false;
	loadGame = true;
 }

 delete d->mStartGame;
 d->mStartGame = new StartGame();

 if (args->isSet("connectto")) {
	QString option = args->getOption("connectto");
	QString host;
	int port = BOSON_PORT;
	if (option.find(':') >= 0) {
		bool ok;
		host = option.left(option.find(':'));
		port = option.right(option.length() - (option.find(':') + 1)).toInt(&ok);
		if (!ok) {
			boError() << k_funcinfo << "invalid port. use host:port with port being a number" << endl;
			return false;
		}
	} else {
		host = option;
	}

	if (!addComputerPlayersToGame(args, 1)) {
		boError() << k_funcinfo << "adding player to game failed" << endl;
		return false;
	}

	d->mStartGame->mHost = host;
	d->mStartGame->mPort = port;
	d->mStartGame->mClient = true;
	return true;
 }


 if (args->isSet("networkplayers")) {
	QString n = args->getOption("networkplayers");
	bool ok;
	int players = n.toInt(&ok);
	if (!ok || players < 0) {
		boError() << k_funcinfo << "\"networkplayers\" argument is not a valid number" << endl;
		return false;
	}

	d->mStartGame->mRequiredPlayers += players;
	if (players > 0) {
		int port = BOSON_PORT;
		if (args->isSet("port")) {
			bool ok;
			port = args->getOption("port").toInt(&ok);
			if (!ok) {
				boError() << k_funcinfo << "invalid port parameter" << endl;
				return false;
			}
		}
		if (!boGame->offerConnections(port)) {
			boError() << k_funcinfo << "unable to offer connections on port " << port << endl;
			return false;
		}
	}
 }


 QByteArray gameData = loadPlayFieldFromDisk(args);
 if (gameData.size() == 0) {
	boError() << k_funcinfo << "unable to load playfield from disk" << endl;
	return 1;
 }
 QByteArray buffer;
 QDataStream stream(buffer, IO_WriteOnly);
 stream << (Q_INT8)1; // game mode (not editor)
 stream << qCompress(gameData);
 d->mStartGame->mPlayField = buffer;


 if (!loadGame) {
	boDebug() << k_funcinfo << "starting new game" << endl;

	if (!addComputerPlayersToGame(args)) {
		boError() << k_funcinfo << "adding player to game failed" << endl;
		return false;
	}


	d->mStartGame->mRequiredPlayers += 1;
	boGame->addNeutralPlayer();
 }

 if (d->mStartGame->mRequiredPlayers > BOSON_MAX_PLAYERS + 1) { // +1 because of neutral player
	boError() << k_funcinfo << "too many players. only " << BOSON_MAX_PLAYERS << " allowed" << endl;
	return false;
 }

 return true;
}

bool MainNoGUI::addComputerPlayersToGame(KCmdLineArgs* args, unsigned int needPlayers)
{
 int players = 1;
 QStringList species;
 if (args->isSet("computerplayers")) {
	QString n = args->getOption("computerplayers");
	bool ok;
	players = n.toInt(&ok);
	if (!ok || players < 0) {
		boError() << k_funcinfo << "\"computerplayers\" argument is not a valid number" << endl;
		return false;
	}
 }
 if (players > BOSON_MAX_PLAYERS) {
	boWarning() << k_funcinfo << "requested " << players << " computer players, but can have at most " << BOSON_MAX_PLAYERS << endl;
	players = BOSON_MAX_PLAYERS;
 }
 if (args->isSet("computerspecies")) {
	QString l = args->getOption("computerspecies");
	species = QStringList::split(",", l);
	if ((int)species.count() != players) {
		boError() << k_funcinfo << "must have exactly " << players << " species for argument \"computerspecies\" (one species per player). have " << species.count() << endl;
		return false;
	}
	for (QStringList::iterator it = species.begin(); it != species.end(); ++it) {
		if (SpeciesTheme::speciesDirectory(*it).isEmpty()) {
			boError() << k_funcinfo << *it << " is not a valid species identifier" << endl;
			return false;
		}
	}
 } else {
	for (int i = 0; i < players; i++) {
		species.append(SpeciesTheme::defaultSpecies());
	}
 }

 if (needPlayers > species.count()) {
	boError() << k_funcinfo << "about to add " << species.count() << " players, but need " << needPlayers << " players" << endl;
	return false;
 }

 if (d->mStartGame) {
	d->mStartGame->mRequiredPlayers += species.count();
 }
 for (unsigned int i = 0; i < species.count(); i++) {
	Player* p = new Player();
	p->loadTheme(SpeciesTheme::speciesDirectory(species[i]), QColor(255,( 255 / BOSON_MAX_PLAYERS) * i, 0));
	BosonComputerIO* input = new BosonComputerIO();
	p->addGameIO(input);
	if (!input->initializeIO()) {
		boError() << k_funcinfo << "computer IO could not be initialized" << endl;
		return 1;
	}
	boGame->bosonAddPlayer(p);
 }
 return true;
}

QByteArray MainNoGUI::loadPlayFieldFromDisk(KCmdLineArgs* args)
{
 if (!args) {
	return QByteArray();
 }
 QString identifier;
 if (args->isSet("playfield")) {
	identifier = args->getOption("playfield");
 } else {
	identifier = BosonPlayField::defaultPlayField();
 }
 BosonPlayField* field = boData->playField(identifier);
 if (!field) {
	boError() << k_funcinfo << "no playfield " << identifier << endl;
	return QByteArray();
 }
 if (!field->isPreLoaded()) {
	boError() << k_funcinfo << "playfield " << identifier << " has no yet been preloaded?!" << endl;
	return QByteArray();
 }

 boDebug() << k_funcinfo << "loading " << identifier << endl;
 QByteArray data = field->loadFromDiskToStream();
 return data;
}

void MainNoGUI::slotGameStarted()
{
 if (!boGame->gameMode()) {
	boWarning() << k_funcinfo << "game in editor mode .. not expected! (does this make any sense without GUI?" << endl;
	return;
 }
 boDebug() << k_funcinfo << endl;
 if (boGame->isAdmin()) {
	if (boGame->gameSpeed() == 0) {
		boDebug() << k_funcinfo << "unpause game" << endl;
		boGame->slotSetGameSpeed(boConfig->intValue("GameSpeed"));
	}
 }
 boDebug() << k_funcinfo << "done" << endl;
}

void MainNoGUI::slotPlayerJoinedGame(KPlayer*)
{
 QTimer::singleShot(0, this, SLOT(slotCheckStart()));
}

void MainNoGUI::slotCheckStart()
{
 if (d->mStartGame) {
	if (d->mStartGame->checkStart()) {
		if (!d->mStartGame->start()) {
			boError() << k_funcinfo << "start failed" << endl;
			return;
		} else {
			delete d->mStartGame;
			d->mStartGame = 0;
		}
	}
 }
}


int main(int argc, char **argv)
{
 KAboutData about("boson",
		I18N_NOOP("BosonNoGUI"),
		version,
		description,
		KAboutData::License_GPL,
		"(C) 1999-2000,2001-2005 The Boson team",
		0,
		"http://boson.eu.org");
 about.addAuthor("Thomas Capricelli",
		I18N_NOOP("Initial Game Design & Coding"),
		"orzel@freehackers.org",
		"http://orzel.freehackers.org");
 about.addAuthor("Andreas Beckermann",
		I18N_NOOP("Coding & Current Maintainer"),
		"b_mann@gmx.de");
 about.addAuthor("Rivo Laks",
		I18N_NOOP("Coding & Homepage Redesign"),
		"rivolaks@hot.ee");
 about.addAuthor("Felix Seeger",
		I18N_NOOP("Documentation"),
		"felix.seeger@gmx.de");

 // first tell BoGlobal that we need to do extra stuff after BosonConfig's
 // initialization
 BosonConfig::setPostInitFunction(&postBosonConfigInit);

 QCString argv0(argv[0]);
 KCmdLineArgs::init(argc, argv, &about);
 KCmdLineArgs::addCmdLineOptions(options);
#if BOSON_LINK_STATIC
 KApplication::disableAutoDcopRegistration();
#endif

 BoEventLoop eventLoop(0, "main event loop");
 BoApplication app(argv0, false, false);
 KGlobal::locale()->insertCatalogue("libkdegames");

 // register ourselves as a dcop client
// app.dcopClient()->registerAs(app.name(), false);

 // make sure the data files are installed at the correct location
 BoCheckInstallation checkInstallation;
 QString errorMessage = checkInstallation.checkInstallation();
 if (!errorMessage.isNull()) {
	boError() << k_funcinfo << errorMessage << endl;
	boError() << k_funcinfo << "check your installation!" << endl;
	KMessageBox::sorry(0, errorMessage, i18n("Check your installation"));
	return 1;
 }

 BoDebugDCOPIface* iface = 0;
#if !BOSON_LINK_STATIC
 // AB: if we build a static binary, we do not allow DCOP connections, so no
 // need to construct this.
 iface = new BoDebugDCOPIface;
#endif

 MainNoGUI* main = new MainNoGUI();
 if (!main->init()) {
	return 1;
 }

 KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

 if (!main->startGame(args)) {
	boError() << k_funcinfo << "unable to start the game" << endl;
	return 1;
 }

 args->clear();
 boDebug() << "starting main loop" << endl;
 int ret = app.exec();

 delete iface;

 return ret;
}

void postBosonConfigInit()
{
 KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
 if (!args) {
	boError() << k_funcinfo << "NULL cmdline args" << endl;
	return;
 }
 if (!BoGlobal::boGlobal()) {
	boError() << k_funcinfo << "NULL BoGlobal object" << endl;
	return;
 }
 BosonConfig* conf = BoGlobal::boGlobal()->bosonConfig();
 if (!conf) {
	boError() << k_funcinfo << "NULL BosonConfig object" << endl;
	return;
 }
 if (!args->isSet("ai")) {
	boDebug() << k_funcinfo << "ai arg is not set" << endl;
	boConfig->setDoubleValue("AIDelay", -1.0);
 } else if (args->isSet("aidelay")) {
	QString delay = args->getOption("aidelay");
	bool ok;
	float aidelay = delay.toFloat(&ok);
	if (ok) {
		boConfig->setDoubleValue("AIDelay", aidelay);
		boDebug() << k_funcinfo << "aidelay set to " << boConfig->doubleValue("AIDelay") << endl;
	} else {
		boError() << k_funcinfo << "aidelay is not a valid float!" << endl;
	}
 }
}



