/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2005 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bosonprofilingdialog.h"

#include "bodebugdcopiface.h"
#include "boversion.h"
#include "boapplication.h"

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
//Added by qt3to4:
#include <Q3CString>

static const char *description =
    I18N_NOOP("Profiling Data reader for Boson");

static const char *version = BOSON_VERSION_STRING;

static KCmdLineOptions options[] =
{
    { "+[FILE]", I18N_NOOP("Profiling file to open."), 0},
    { 0, 0, 0 }
};

int main(int argc, char **argv)
{
 KAboutData about("boprofiling",
		I18N_NOOP("Boson Profiling Reader"),
		version,
		description,
		KAboutData::License_GPL,
		"(C) 2002 The Boson team",
		0,
		"http://boson.eu.org");
 about.addAuthor( "Andreas Beckermann", I18N_NOOP("Coding & Current Maintainer"), "b_mann@gmx.de" );

 Q3CString argv0(argv[0]);
 KCmdLineArgs::init(argc, argv, &about);
 KCmdLineArgs::addCmdLineOptions(options);
 BoApplication app(argv0);

    // register ourselves as a dcop client
//    app.dcopClient()->registerAs(app.name(), false);

 KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
 QObject::connect(kapp, SIGNAL(lastWindowClosed()), kapp, SLOT(quit()));

 if (args->count() == 0) {
	BosonProfilingDialog* dlg = new BosonProfilingDialog(0);
	QObject::connect(dlg, SIGNAL(finished()), dlg, SLOT(close()));
	dlg->show();
 } else {
	for (int i = 0; i < args->count(); i++) {
		BosonProfilingDialog* dlg = new BosonProfilingDialog(0);
		QObject::connect(dlg, SIGNAL(finished()), dlg, SLOT(close()));
		KUrl url = args->url(i);
		dlg->loadFromFile(url.directory(false) + url.fileName());
		dlg->show();
	}
 }

 BoDebugDCOPIface* iface = new BoDebugDCOPIface();
 args->clear();
 int r = app.exec();
 delete iface;
 return r;
}


