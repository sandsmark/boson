/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001 The Boson Team (boson-devel@lists.sourceforge.net)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "top.h"
//#include "editortop.h"

#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>

static const char *description =
    I18N_NOOP("A realtime strategy game for KDE");

static const char *version = "v0.1";

static KCmdLineOptions options[] =
{
//    { "+[URL]", I18N_NOOP( "Document to open." ), 0 },
//    { "s", 0, 0 },
//    { "server", I18N_NOOP( "Only server - no GUI"), 0 },
//    { "e", 0, 0 },
//    { "editor", I18N_NOOP( "Map editor"), 0 },
    { 0, 0, 0 }
};

int main(int argc, char **argv)
{
//FIXME:
 KAboutData about("boson",
		I18N_NOOP("Boson game"),
		version,
		description,
		KAboutData::License_GPL,
		"(C) 1999-2000,2001 The Boson team",
		0,
		"http://boson.eu.org",
		"b_mann@gmx.de");
 about.addAuthor("Thomas Capricelli", I18N_NOOP("Game Design & Coding"), "orzel@yalbi.com", "http://aquila.rezel.enst.fr/thomas/");
 about.addAuthor("Benjamin Adler", I18N_NOOP("Graphics & Homepage Design"), "benadler@bigfoot.de");
 about.addAuthor( "Andreas Beckermann", I18N_NOOP("Coding"), "b_mann@gmx.de" );

 KCmdLineArgs::init(argc, argv, &about);
 KCmdLineArgs::addCmdLineOptions(options);
 KApplication app;
 KGlobal::locale()->insertCatalogue("libkdegames");

    // register ourselves as a dcop client
//    app.dcopClient()->registerAs(app.name(), false);

    // see if we are starting with session management
 if (app.isRestored()) {
 //FIXME: do we use this at all?? probably not...
	RESTORE(Top)
 } else {
// no session.. just start up normally
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
/*	if (args->isSet("editor")) {
		EditorTop *widget = new EditorTop;
		widget->show();
	} else {*/
		Top *widget = new Top;
		widget->show();
//	}
	args->clear();
 }
 return app.exec();
}

