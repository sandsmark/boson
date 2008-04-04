/*
    This file is part of the Boson game
    Copyright (C) 2001-2008 Andreas Beckermann (b_mann@gmx.de)

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

#include "mainnogui.h"
#include "mainnogui.moc"

#include "../../bomemory/bodummymemory.h"
#include "../defines.h"
#include "../bocheckinstallation.h"
#include "../bosonconfig.h"
#include "../boglobal.h"
#include "bodebug.h"
#include "../bo3dtools.h"
#include "../gameengine/boeventloop.h"
#include "../gameengine/bosongameengine.h"
#include "../gameengine/player.h"
#include "../gameengine/boson.h"
#include "../gameengine/bosonstarting.h"
#include "../gameengine/bosongameenginestarting.h"
#include "../gameengine/bosonmessageids.h"
#include "../gameengine/bosonplayfield.h"
#include "../bosondata.h"
#include "../gameengine/speciestheme.h"
#include "../gameengine/bosoncomputerio.h"
#include "../gameengine/bpfloader.h"
#include <config.h>

#include <kaboutdata.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <qtimer.h>

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
		if (boGame->allPlayerCount() >= mRequiredPlayers) {
			return true;
		} else {
			boDebug() << k_funcinfo << "not enough players yet. have: " << boGame->allPlayerCount() << " need: " << mRequiredPlayers << endl;
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
	bool mStartingExecuted;

	StartGame* mStartGame;
};

MainNoGUI::MainNoGUI()
	: QObject(0)
{
 d = new MainNoGUIPrivate();
 d->mGameEngine = new BosonGameEngine(0);
 d->mStarting = new BosonStarting(0);
 BosonGameEngineStarting* game = new BosonGameEngineStarting(d->mStarting, d->mStarting);
 d->mStarting->addTaskCreator(game);
 d->mStartingExecuted = false;

 connect(this, SIGNAL(signalAddIOs(Player*, int*, bool*)),
		this, SLOT(slotAddIOs(Player*, int*, bool*)));
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

BosonStarting* MainNoGUI::startingObject() const
{
 return d->mStarting;
}

bool MainNoGUI::startGame(const MainNoGUIStartOptions& options)
{
 if (d->mStartingExecuted) {
	boError() << k_funcinfo << "already started before" << endl;
	return false;
 }
 d->mStartingExecuted = true;
 QObject::connect(boGame, SIGNAL(signalStartingCompletedReceived(const QByteArray&, Q_UINT32)),
		d->mStarting, SLOT(slotStartingCompletedReceived(const QByteArray&, Q_UINT32)));
 QObject::connect(boGame, SIGNAL(signalSetNewGameData(const QByteArray&, bool*)),
		d->mStarting, SLOT(slotSetNewGameData(const QByteArray&, bool*)));
 QObject::connect(boGame, SIGNAL(signalStartNewGame()),
		d->mStarting, SLOT(slotStartNewGameWithTimer()));
 connect(boGame, SIGNAL(signalPlayerJoinedGame(KPlayer*)),
		this, SLOT(slotPlayerJoinedGame(KPlayer*)));



 const bool loadGame = options.load;
 if (options.load) {
	boError() << k_funcinfo << "loading games is not yet supported here" << endl;
	return false;
 }

 delete d->mStartGame;
 d->mStartGame = new StartGame();

 if (options.isClient) {
	if (!addComputerPlayersToGame(options, 1)) {
		boError() << k_funcinfo << "adding player to game failed" << endl;
		return false;
	}

	d->mStartGame->mHost = options.host;
	d->mStartGame->mPort = options.port;
	d->mStartGame->mClient = true;
	return true;
 }


 if (options.remotePlayers > 0) {
	d->mStartGame->mRequiredPlayers += options.remotePlayers;
	if (!boGame->offerConnections(options.port)) {
		boError() << k_funcinfo << "unable to offer connections on port " << options.port << endl;
		return false;
	}
 }


 QByteArray gameData = loadPlayFieldFromDisk(options);
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

	if (!addComputerPlayersToGame(options)) {
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

bool MainNoGUI::addComputerPlayersToGame(const MainNoGUIStartOptions& options, unsigned int needPlayers)
{
 if (options.computerPlayers.count() > BOSON_MAX_PLAYERS) {
	boError() << k_funcinfo << "requested " << options.computerPlayers.count() << " computer players, but can have at most " << BOSON_MAX_PLAYERS << endl;
	return false;
 }
 QStringList species;
 QValueList<int> ios;
 for (QValueList<MainNoGUIAIPlayerOptions>::const_iterator it = options.computerPlayers.begin(); it != options.computerPlayers.end(); ++it) {
	if ((*it).species.isNull()) {
		species.append(SpeciesTheme::defaultSpecies());
	} else {
		species.append((*it).species);
	}
	ios.append((*it).io);
 }

 for (QStringList::iterator it = species.begin(); it != species.end(); ++it) {
	if (SpeciesTheme::speciesDirectory(*it).isEmpty()) {
		boError() << k_funcinfo << *it << " is not a valid species identifier" << endl;
		return false;
	}
 }

 if (needPlayers > species.count()) {
	boError() << k_funcinfo << "about to add " << species.count() << " players, but need " << needPlayers << " players" << endl;
	return false;
 }

 if (d->mStartGame) {
	d->mStartGame->mRequiredPlayers += species.count();
 }
 if (ios.count() != species.count()) {
	return false;
 }
 for (unsigned int i = 0; i < species.count(); i++) {
	Player* p = new Player();
	p->loadTheme(SpeciesTheme::speciesDirectory(species[i]), QColor(255,( 255 / BOSON_MAX_PLAYERS) * i, 0));

	int ioMask = 0;
	if (ios[i] == MainNoGUIAIPlayerOptions::NoIO) {
		ioMask = 0;
	} else if (ios[i] == MainNoGUIAIPlayerOptions::DefaultIO) {
		ioMask = MainNoGUIAIPlayerOptions::ComputerIO;
	} else {
		ioMask = ios[i];
	}

	bool failure = false;
	emit signalAddIOs(p, &ioMask, &failure);
	if (failure) {
		return false;
	}

	if (p->ioList()->count() == 0) {
		ioMask = MainNoGUIAIPlayerOptions::NoIO;
	}
	if (ioMask == MainNoGUIAIPlayerOptions::NoIO) {
		KGameIO* io = new KGameComputerIO();
		p->addGameIO(io);
	}

	boGame->bosonAddPlayer(p);
 }
 return true;
}

QByteArray MainNoGUI::loadPlayFieldFromDisk(const MainNoGUIStartOptions& options)
{
 QString identifier = options.playField;
 if (identifier.isEmpty()) {
	identifier = BosonPlayField::defaultPlayField();
 }
 BPFPreview* preview = boData->playFieldPreview(identifier);
 if (!preview) {
	boError() << k_funcinfo << "no playfield " << identifier << endl;
	return QByteArray();
 }
 if (!preview->isLoaded()) {
	boError() << k_funcinfo << "playfieldpreview " << identifier << " has no yet been loaded?!" << endl;
	return QByteArray();
 }

 QString fileName = preview->fileName();

 boDebug() << k_funcinfo << "loading " << identifier << endl;
 QByteArray data = BPFLoader::loadFromDiskToStream(fileName);
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

void MainNoGUI::slotAddIOs(Player* p, int* ioMask, bool* failure)
{
 if ((*ioMask) & MainNoGUIAIPlayerOptions::ComputerIO) {
	BosonComputerIO* io = new BosonComputerIO();
	p->addGameIO(io);
	if (!io->initializeIO()) {
		boError() << k_funcinfo << "computer IO could not be initialized" << endl;
		*failure = true;
		return;
	}
	(*ioMask) &= ~MainNoGUIAIPlayerOptions::ComputerIO;
 }
}
