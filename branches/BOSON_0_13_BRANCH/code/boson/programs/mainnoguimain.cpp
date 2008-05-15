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
#include "mainnogui.h"
#include <config.h>
#include "../boversion.h"
#include "bodebug.h"
#include "bodebugdcopiface.h"
#include "../bosonconfig.h"
#include "../boglobal.h"
#include "../bocheckinstallation.h"
#include "../boapplication.h"
#include "../gameengine/boeventloop.h"

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
    { "playfield <identifier>", I18N_NOOP("Playfield identifier for newgame/start editor widget"), 0 },
    { "computerplayers <count>", I18N_NOOP("Add <count> computer players to the game. Default is 1."), "1" },
    { "computerspecies <species>", I18N_NOOP("Comma separated list of species identifiers - one species per computer player."), 0 },
    { "networkplayers <count>", I18N_NOOP("Wait for <count> players to enter the game from network. Default is 0."), "0" },
    { "port <number>", I18N_NOOP("Port to listen on for network players"), QString("%1").arg(BOSON_PORT) },
    { "aidelay <delay>", I18N_NOOP("Set AI delay (in seconds). The less it is, the faster AI will send it's units"), 0 },
    { "noai", I18N_NOOP("Disable AI"), 0 },
    { "connectto <host:port>" I18N_NOOP("Connect to a server"), 0 },
    { 0, 0, 0 }
};

static bool parseArgs(MainNoGUIStartOptions* options, KCmdLineArgs* args);
static bool parseAddComputerArgs(MainNoGUIStartOptions* options, KCmdLineArgs* args);
static bool parsePlayFieldArgs(MainNoGUIStartOptions* options, KCmdLineArgs* args);

static void postBosonConfigInit();

int main(int argc, char **argv)
{
 KAboutData about("boson",
		I18N_NOOP("BosonNoGUI"),
		version,
		description,
		KAboutData::License_GPL,
		"(C) 1999-2000,2001-2006 The Boson team",
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
	boError() << k_funcinfo << "init failed" << endl;
	KMessageBox::sorry(0, i18n("Unable to initialize game"));
	return 1;
 }

 KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

 MainNoGUIStartOptions options;

 if (!parseArgs(&options, args)) {
	return 1;
 }
 args->clear();

 if (!main->startGame(options)) {
	boError() << k_funcinfo << "unable to start the game" << endl;
	return 1;
 }

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


static bool parseArgs(MainNoGUIStartOptions* options, KCmdLineArgs* args)
{
 if (args->isSet("load")) {
	boError() << k_funcinfo << "loading games is not yet supported here" << endl;
	return false;
	options->load = true;
 }

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

	if (!parseAddComputerArgs(options, args)) {
		boError() << k_funcinfo << "adding player to game failed" << endl;
		return false;
	}


	options->host = host;
	options->port = port;
	options->isClient = true;
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

	int port = options->port;
	if (args->isSet("port")) {
		bool ok;
		port = args->getOption("port").toInt(&ok);
		if (!ok) {
			boError() << k_funcinfo << "invalid port parameter" << endl;
			return false;
		}
	}

	options->remotePlayers += players;
	options->port = port;
 }

 if (!parsePlayFieldArgs(options, args)) {
	return false;
 }

 if (!options->load) {
	if (!parseAddComputerArgs(options, args)) {
		return false;
	}
 }
 return true;
}

static bool parseAddComputerArgs(MainNoGUIStartOptions* options, KCmdLineArgs* args)
{
 int players = 1; // AB: add one AI player by default
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
 } else {
	for (int i = 0; i < players; i++) {
		species.append(QString::null);
	}
 }

 QValueList<MainNoGUIAIPlayerOptions> aiPlayers;
 for (QStringList::iterator it = species.begin(); it != species.end(); ++it) {
	MainNoGUIAIPlayerOptions p;
	p.species = *it;
	p.io = -1; // default IO
	aiPlayers.append(p);
 }
 options->computerPlayers = aiPlayers;

 return true;
}

bool parsePlayFieldArgs(MainNoGUIStartOptions* options, KCmdLineArgs* args)
{
 if (!args) {
	return QByteArray();
 }
 if (!options) {
	return QByteArray();
 }
 QString identifier;
 if (args->isSet("playfield")) {
	options->playField = args->getOption("playfield");
 }
 return true;
}
