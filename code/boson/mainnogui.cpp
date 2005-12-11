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

#include "../bomemory/bodummymemory.h"
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
#include <config.h>

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kmessagebox.h>

static const char *description =
    I18N_NOOP("A realtime strategy game for KDE");

static const char *version = BOSON_VERSION_STRING;

static KCmdLineOptions options[] =
{
    { "load", I18N_NOOP("Skip Welcome Widget and display the Load Game screen"), 0 },
    { "load-from-log <file>", I18N_NOOP("Load from emergency log, for debugging"), 0 },
    { "playfield <identifier>", I18N_NOOP("Playfield identifier for newgame/start editor widget"), 0 },
    { "computer <count>", I18N_NOOP("Add (currently dummy) computer player"), 0 },
    { "start", I18N_NOOP("Start the game"), 0},
    { "aidelay <delay>", I18N_NOOP("Set AI delay (in seconds). The less it is, the faster AI will send it's units"), 0 },
    { "noai", I18N_NOOP("Disable AI"), 0 },
    { 0, 0, 0 }
};


static void postBosonConfigInit();
static QByteArray loadPlayFieldFromDisk(KCmdLineArgs* args);


int main(int argc, char **argv)
{
 KAboutData about("boson",
		I18N_NOOP("Boson"),
		version,
		description,
		KAboutData::License_GPL,
		"(C) 1999-2000,2001-2005 The Boson team",
		0,
		"http://boson.eu.org");
 about.addAuthor("Thomas Capricelli", I18N_NOOP("Initial Game Design & Coding"),
		"orzel@freehackers.org", "http://orzel.freehackers.org");
 about.addAuthor("Benjamin Adler", I18N_NOOP("Graphics & Homepage Design"),
		"benadler@bigfoot.de");
 about.addAuthor("Andreas Beckermann", I18N_NOOP("Coding & Current Maintainer"), "b_mann@gmx.de");
 about.addAuthor("Rivo Laks", I18N_NOOP("Coding & Homepage Redesign"), "rivolaks@hot.ee");
 about.addAuthor("Felix Seeger", I18N_NOOP("Documentation"), "felix.seeger@gmx.de");
 about.addAuthor("Christopher J. Jepson (Xenthorious)", I18N_NOOP("Modeling & texturing"), "xenthorious@yahoo.com");

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
 BoApplication app(argv0);
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

 BosonGameEngine* gameEngine = new BosonGameEngine(0);
 BosonStarting* starting = new BosonStarting(0);

 if (!gameEngine->preloadData()) {
	boError() << k_funcinfo << "unable to preload some data" << endl;
	KMessageBox::sorry(0, i18n("Unable to preload data. Check your installation!"), i18n("Check your installation"));
	return 1;
 }

 gameEngine->slotResetGame();

 KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
 bool loadGame = false;
 if (args->isSet("load")) {
	boError() << k_funcinfo << "loading games is not yet supported here" << endl;
	return 1;
	loadGame = true;
 }
 args->clear();

 if (!loadGame) {
	boDebug() << k_funcinfo << "starting new game" << endl;

	Player* localPlayer = new Player;
	localPlayer->loadTheme(SpeciesTheme::speciesDirectory(SpeciesTheme::defaultSpecies()), Qt::red);
#if 0
	BosonLocalPlayerInput* input = new BosonLocalPlayerInput();
	p->addGameIO(input);
	if (!input->initializeIO()) {
		boError() << k_funcinfo << "localplayer IO could not be initialized" << endl;
		return 1;
	}
#endif
	boGame->bosonAddPlayer(localPlayer);

	BosonGameEngineStarting* game = new BosonGameEngineStarting(starting, starting);
	starting->addTaskCreator(game);

	boGame->addNeutralPlayer();
 }

 QByteArray gameData = loadPlayFieldFromDisk(args);
 if (gameData.size() == 0) {
	boError() << k_funcinfo << "unable to load playfield from disk" << endl;
	return 1;
 }

 QObject::connect(boGame, SIGNAL(signalSetNewGameData(const QByteArray&, bool*)),
		starting, SLOT(slotSetNewGameData(const QByteArray&, bool*)));
 QObject::connect(boGame, SIGNAL(signalStartNewGame()),
		starting, SLOT(slotStartNewGameWithTimer()));
 QByteArray buffer;
 QDataStream stream(buffer, IO_WriteOnly);
 stream << (Q_INT8)1; // game mode (not editor)
 stream << qCompress(gameData);
 boGame->sendMessage(buffer, BosonMessageIds::IdNewGame);

 boDebug() << "starting main loop" << endl;
 int ret = app.exec();

 delete iface;
 delete starting;
 delete gameEngine;

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

QByteArray loadPlayFieldFromDisk(KCmdLineArgs* args)
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

