/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2002 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bosonunitdialog.h"

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>

static const char *description =
    I18N_NOOP("Unit Editor for Boson");

static const char *version = "v0.6.1";

static KCmdLineOptions options[] =
{
    { 0, 0, 0 }
};

int main(int argc, char **argv)
{
 KAboutData about("bounit",
		I18N_NOOP("Boson Unit Editor"),
		version,
		description,
		KAboutData::License_GPL,
		"(C) 1999-2000,2001-2002 The Boson team",
		0,
		"http://boson.eu.org",
		"b_mann@gmx.de");
 about.addAuthor("Thomas Capricelli", I18N_NOOP("Initial Game Design & Coding"), "orzel@kde.org", "http://orzel.freehackers.org");
 about.addAuthor("Benjamin Adler", I18N_NOOP("Graphics & Homepage Design"), "benadler@bigfoot.de");
 about.addAuthor( "Andreas Beckermann", I18N_NOOP("Coding & Current Maintainer"), "b_mann@gmx.de" );

 KCmdLineArgs::init(argc, argv, &about);
 KCmdLineArgs::addCmdLineOptions(options);
 KApplication app;
 KGlobal::locale()->insertCatalogue("libkdegames");

    // register ourselves as a dcop client
//    app.dcopClient()->registerAs(app.name(), false);

 KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
 BosonUnitDialog* dlg = new BosonUnitDialog(0);
 app.setMainWidget(dlg);
 dlg->show();
 
 args->clear();
 return app.exec();
}


