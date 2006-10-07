/*
    This file is part of the Boson game
    Copyright (C) 2003-2005 Andreas Beckermann (b_mann@gmx.de)

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

#include "info/boinfo.h"
#include "info/boinfodialog.h"

#include "../bomemory/bodummymemory.h"
#include "bodebugdcopiface.h"
#include "bodebug.h"
#include "boversion.h"
#include "boapplication.h"
#include "bogl.h"

// we need this to initialize the GLX context.
#include "bosonglwidget.h"

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>

static const char *description =
    I18N_NOOP("BoInfo reader for Boson");

static const char *version = BOSON_VERSION_STRING;

static KCmdLineOptions options[] =
{
    { "+[FILE]", I18N_NOOP("BoInfo file to open."), 0},
    { 0, 0, 0 }
};

int main(int argc, char **argv)
{
 KAboutData about("boinfo",
		I18N_NOOP("Boson Info file Reader"),
		version,
		description,
		KAboutData::License_GPL,
		"(C) 2003-2005 The Boson team",
		0,
		"http://boson.eu.org");
 about.addAuthor( "Andreas Beckermann", I18N_NOOP("Coding & Current Maintainer"), "b_mann@gmx.de" );

 QCString argv0(argv[0]);
 KCmdLineArgs::init(argc, argv, &about);
 KCmdLineArgs::addCmdLineOptions(options);
 BoApplication app(argv0);

    // register ourselves as a dcop client
//    app.dcopClient()->registerAs(app.name(), false);

 KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
 QObject::connect(kapp, SIGNAL(lastWindowClosed()), kapp, SLOT(quit()));

 BosonGLWidget* glWidget = new BosonGLWidget(0);
 glWidget->hide();
 glWidget->makeCurrent();
 BoGL::bogl()->initialize();
 BoInfo::boInfo()->update(glWidget);

// if (args->count() == 0) {
	BoInfoDialog* dlg = new BoInfoDialog(0);
	QObject::connect(dlg, SIGNAL(finished()), dlg, SLOT(close()));
	dlg->show();
	dlg->reset();
	/*
 } else {
	for (int i = 0; i < args->count(); i++) {
		BoInfoDialog* dlg = new BoInfoDialog(0);
		QObject::connect(dlg, SIGNAL(finished()), dlg, SLOT(close()));
		KURL url = args->url(i);
		dlg->loadFromFile(url.directory(false) + url.fileName());
		dlg->show();
	}
 }
 */

 BoDebugDCOPIface* iface = new BoDebugDCOPIface();
 args->clear();
 int r = app.exec();
 delete iface;
 delete glWidget;
 return r;
}


