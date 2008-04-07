/*
    This file is part of the Boson game
    Copyright (C) 2001-2005 Rivo Laks (rivolaks@hot.ee)

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

#include "bouniteditor.h"

#include "../bomemory/bodummymemory.h"
#include "boversion.h"
#include "boapplication.h"
#include "bosonconfig.h"
#include "boglobal.h"
#include "bodebug.h"

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
//Added by qt3to4:
#include <Q3CString>

static void postBosonConfigInit();

static KLocalizedString description =
    ki18n("Unit Editor for Boson");

static const char *version = BOSON_VERSION_STRING;

int main(int argc, char **argv)
{
 KAboutData about("bounit",
		QByteArray(),
		ki18n("Boson Unit Editor"),
		version,
		description,
		KAboutData::License_GPL,
		ki18n("(C) 1999-2000,2001-2005 The Boson team"),
		KLocalizedString(),
		"http://boson.eu.org");
 about.addAuthor(ki18n("Thomas Capricelli"), ki18n("Initial Game Design & Coding"), "orzel@freehackers.org", "http://orzel.freehackers.org");
 about.addAuthor(ki18n("Benjamin Adler"), ki18n("Graphics & Homepage Design"), "benadler@bigfoot.de");
 about.addAuthor(ki18n("Andreas Beckermann"), ki18n("Coding & Current Maintainer"), "b_mann@gmx.de");
 about.addAuthor(ki18n("Rivo Laks"), ki18n("Design & Coding"), "rivolaks@hot.ee");

 BosonConfig::setPostInitFunction(&postBosonConfigInit);

 Q3CString argv0(argv[0]);
 KCmdLineArgs::init(argc, argv, &about);
 BoApplication app(argv0);

 KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
 BoUnitEditor* dlg = new BoUnitEditor(0);
 app.setMainWidget(dlg);
 dlg->show();

 args->clear();
 int r = app.exec();
 delete dlg;
 return r;
}


static void postBosonConfigInit()
{
 BosonConfig* conf = BoGlobal::boGlobal()->bosonConfig();
 if (!conf) {
	boError() << k_funcinfo << "NULL BosonConfig object" << endl;
	return;
 }
 conf->setBoolValue("ForceDisableSound", true);
}

