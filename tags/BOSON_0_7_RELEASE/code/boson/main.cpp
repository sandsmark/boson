/*
    This file is part of the Boson game
    Copyright (C) 2001-2002 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>

static const char *description =
    I18N_NOOP("A realtime strategy game for KDE");

static const char *version = "v0.7";

static KCmdLineOptions options[] =
{
    { "nosound", I18N_NOOP("Disable Sounds"), 0 },
    { "new", I18N_NOOP("Skip Welcome Widget and display the New Game screen"), 0 },
    { "editor", I18N_NOOP("Skip Welcome Widget and display the Start Editor screen"), 0 },
    { 0, 0, 0 }
};

int main(int argc, char **argv)
{
//FIXME:
 KAboutData about("boson",
		I18N_NOOP("Boson"),
		version,
		description,
		KAboutData::License_GPL,
		"(C) 1999-2000,2001-2002 The Boson team",
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
 
 TopWidget *widget = new TopWidget;
 bool showMaximized = true; // TODO: make this a config option
 app.setMainWidget(widget);

// if (showMaximized) {
//	widget->showMaximized();
// } else {
	widget->show();
// }

 if (args->isSet("new")) {
	widget->slotNewGame();
 } else if (args->isSet("editor")) {
	widget->slotStartEditor();
}
 args->clear();
 return app.exec();
}

