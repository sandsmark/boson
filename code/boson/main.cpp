/*
    This file is part of the Boson game
    Copyright (C) 2001-2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "top.h"

#include "bosonconfig.h"
#include "boversion.h"
#include "bodebug.h"

#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>

static const char *description =
    I18N_NOOP("A realtime strategy game for KDE");

static const char *version = BOSON_VERSION_STRING;

static KCmdLineOptions options[] =
{
    { "nosound", I18N_NOOP("Disable Sounds"), 0 },
    { "new", I18N_NOOP("Skip Welcome Widget and display the New Game screen"), 0 },
    { "editor", I18N_NOOP("Skip Welcome Widget and display the Start Editor screen"), 0 },
    { "load", I18N_NOOP("Skip Welcome Widget and display the Load Game screen"), 0 },
    { "playfield <identifier>", I18N_NOOP("Playfield identifier for newgame/start editor widget"), 0 },
    { "computer <count>", I18N_NOOP("Add (currently dummy) computer player"), 0 },
    { "start", I18N_NOOP("Start the game"), 0},
    { "noloadtiles", I18N_NOOP("Do not load tiles (debugging only)"), 0},
    { "aidelay <delay>", I18N_NOOP("Set AI delay (in seconds). The less it is, the faster AI will send it's units"), 0 },
    { "noai", I18N_NOOP("Disable AI"), 0 },
    { 0, 0, 0 }
};

// FIXME: I don't like static vars
extern float aidelay;

int main(int argc, char **argv)
{
//FIXME:
 KAboutData about("boson",
		I18N_NOOP("Boson"),
		version,
		description,
		KAboutData::License_GPL,
		"(C) 1999-2000,2001-2003 The Boson team",
		0,
		"http://boson.eu.org");
 about.addAuthor("Thomas Capricelli", I18N_NOOP("Initial Game Design & Coding"),
		"orzel@freehackers.org", "http://orzel.freehackers.org");
 about.addAuthor("Benjamin Adler", I18N_NOOP("Graphics & Homepage Design"), 
		"benadler@bigfoot.de");
 about.addAuthor("Andreas Beckermann", I18N_NOOP("Coding & Current Maintainer"), "b_mann@gmx.de");
 about.addAuthor("Rivo Laks", I18N_NOOP("Coding"), "rivolaks@hot.ee");

 KCmdLineArgs::init(argc, argv, &about);
 KCmdLineArgs::addCmdLineOptions(options);
 KApplication app;
 KGlobal::locale()->insertCatalogue("libkdegames");

    // register ourselves as a dcop client
//    app.dcopClient()->registerAs(app.name(), false);

 BosonConfig::initBosonConfig();
 KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
 if (!args->isSet("sound")) {
	boConfig->setDisableSound(true);
 }
 if (!args->isSet("loadtiles")) {
	boConfig->setLoadTiles(false);
 }
 aidelay = 2.0;  // default
 if (!args->isSet("ai")) {
	boDebug() << k_funcinfo << "ai arg is not set" << endl;
	aidelay = 0.0;
 } else if (args->isSet("aidelay")) {
	QString delay = args->getOption("aidelay");
	bool ok;
	aidelay = delay.toFloat(&ok);
	boDebug() << k_funcinfo << "aidelay set to " << aidelay << endl;
	if (!ok) {
		boError() << k_funcinfo << "aidelay is not a valid float!" << endl;
		// Fall back to default
		aidelay = 2.0;
	}
 }

 TopWidget *top = new TopWidget;
 app.setMainWidget(top);

 top->show();

 if (args->isSet("new")) {
	top->slotNewGame(args);
 } else if (args->isSet("editor")) {
	top->slotStartEditor(args);
 } else if (args->isSet("load")) {
	top->slotLoadGame(args);
 }
 args->clear();
 return app.exec();
}

