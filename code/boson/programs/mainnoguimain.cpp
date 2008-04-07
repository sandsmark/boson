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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "../bomemory/bodummymemory.h"
#include "mainnogui.h"
#include <config.h>
#include "../boversion.h"
#include "bodebug.h"
#include "../bosonconfig.h"
#include "../boglobal.h"
#include "../bocheckinstallation.h"
#include "../boapplication.h"
//#include "../gameengine/boeventloop.h"

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <qtimer.h>
//Added by qt3to4:
#include <Q3CString>
#include <Q3ValueList>

static KLocalizedString description =
    ki18n("Boson without GUI");

static const char *version = BOSON_VERSION_STRING;

static bool parseArgs(MainNoGUIStartOptions* options, KCmdLineArgs* args);
static bool parseAddComputerArgs(MainNoGUIStartOptions* options, KCmdLineArgs* args);
static bool parsePlayFieldArgs(MainNoGUIStartOptions* args, KCmdLineArgs* args);

static void postBosonConfigInit();

int main(int argc, char **argv)
{
 KAboutData about("boson",
		QByteArray(),
		ki18n("BosonNoGUI"),
		version,
		description,
		KAboutData::License_GPL,
		ki18n("(C) 1999-2000,2001-2006 The Boson team"),
		KLocalizedString(),
		"http://boson.eu.org");
 about.addAuthor(ki18n("Thomas Capricelli"),
		ki18n("Initial Game Design & Coding"),
		"orzel@freehackers.org",
		"http://orzel.freehackers.org");
 about.addAuthor(ki18n("Andreas Beckermann"),
		ki18n("Coding & Current Maintainer"),
		"b_mann@gmx.de");
 about.addAuthor(ki18n("Rivo Laks"),
		ki18n("Coding & Homepage Redesign"),
		"rivolaks@hot.ee");
 about.addAuthor(ki18n("Felix Seeger"),
		ki18n("Documentation"),
		"felix.seeger@gmx.de");

 KCmdLineOptions options;
 options.add("load", ki18n("Skip Welcome Widget and display the Load Game screen"));
 options.add("playfield <identifier>", ki18n("Playfield identifier for newgame/start editor widget"), "0");
 options.add("computerplayers <count>", ki18n("Add <count> computer players to the game. Default is 1."), "1");
 options.add("computerspecies <species>", ki18n("Comma separated list of species identifiers - one species per computer player."));
 options.add("networkplayers <count>", ki18n("Wait for <count> players to enter the game from network. Default is 0."), "0");
 options.add("port <number>", ki18n("Port to listen on for network players"), QString("%1").arg(BOSON_PORT).toLatin1());
 options.add("aidelay <delay>", ki18n("Set AI delay (in seconds). The less it is, the faster AI will send it's units"));
 options.add("noai", ki18n("Disable AI"));
 options.add("connectto <host:port>", ki18n("Connect to a server"));

 // first tell BoGlobal that we need to do extra stuff after BosonConfig's
 // initialization
 BosonConfig::setPostInitFunction(&postBosonConfigInit);

 Q3CString argv0(argv[0]);
 KCmdLineArgs::init(argc, argv, &about);
 KCmdLineArgs::addCmdLineOptions(options);
#if BOSON_LINK_STATIC
 KApplication::disableAutoDcopRegistration();
#endif

#if 0
 BoEventLoop eventLoop(0, "main event loop");
#endif
 BoApplication app(argv0, false);
 KGlobal::locale()->insertCatalog("libkdegames");

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

 MainNoGUI* main = new MainNoGUI();
 if (!main->init()) {
	boError() << k_funcinfo << "init failed" << endl;
	KMessageBox::sorry(0, i18n("Unable to initialize game"));
	return 1;
 }

 KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

 MainNoGUIStartOptions mainOptions;

 if (!parseArgs(&mainOptions, args)) {
	return 1;
 }
 args->clear();

 if (!main->startGame(mainOptions)) {
	boError() << k_funcinfo << "unable to start the game" << endl;
	return 1;
 }

 boDebug() << "starting main loop" << endl;
 int ret = app.exec();

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

 Q3ValueList<MainNoGUIAIPlayerOptions> aiPlayers;
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
	return false;
 }
 if (!options) {
	return false;
 }
 QString identifier;
 if (args->isSet("playfield")) {
	options->playField = args->getOption("playfield");
 }
 return true;
}
