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
#include "bodebugdcopiface.h"
#include "boversion.h"
#include "boapplication.h"
#include "bosonconfig.h"
#include "boglobal.h"
#include "bodebug.h"

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>

static void postBosonConfigInit();

static const char *description =
    I18N_NOOP("Unit Editor for Boson");

static const char *version = BOSON_VERSION_STRING;

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
		"(C) 1999-2000,2001-2005 The Boson team",
		0,
		"http://boson.eu.org");
 about.addAuthor("Thomas Capricelli", I18N_NOOP("Initial Game Design & Coding"), "orzel@freehackers.org", "http://orzel.freehackers.org");
 about.addAuthor("Benjamin Adler", I18N_NOOP("Graphics & Homepage Design"), "benadler@bigfoot.de");
 about.addAuthor( "Andreas Beckermann", I18N_NOOP("Coding & Current Maintainer"), "b_mann@gmx.de" );
 about.addAuthor( "Rivo Laks", I18N_NOOP("Design & Coding"), "rivolaks@hot.ee" );

 BosonConfig::setPostInitFunction(&postBosonConfigInit);

 QCString argv0(argv[0]);
 KCmdLineArgs::init(argc, argv, &about);
 KCmdLineArgs::addCmdLineOptions(options);
 BoApplication app(argv0);

 KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
 BoUnitEditor* dlg = new BoUnitEditor(0);
 app.setMainWidget(dlg);
 dlg->show();

 BoDebugDCOPIface* iface = new BoDebugDCOPIface();
 args->clear();
 int r = app.exec();
 delete iface;
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

