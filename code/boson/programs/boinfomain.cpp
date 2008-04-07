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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "info/boinfo.h"
#include "info/boinfodialog.h"

#include "../bomemory/bodummymemory.h"
#include "bodebug.h"
#include "boversion.h"
#include "boapplication.h"
#include "bogl.h"

// we need this to initialize the GLX context.
#include <QGLWidget>

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
//Added by qt3to4:
#include <Q3CString>

static KLocalizedString description =
    ki18n("BoInfo reader for Boson");

static const char *version = BOSON_VERSION_STRING;

int main(int argc, char **argv)
{
 KAboutData about("boinfo",
		QByteArray(),
		ki18n("Boson Info file Reader"),
		version,
		description,
		KAboutData::License_GPL,
		ki18n("(C) 2003-2005 The Boson team"),
		KLocalizedString(),
		"http://boson.eu.org");
 about.addAuthor( ki18n("Andreas Beckermann"), ki18n("Coding & Current Maintainer"), "b_mann@gmx.de" );

 KCmdLineOptions options;
 options.add("+[FILE]", ki18n("BoInfo file to open."));

 Q3CString argv0(argv[0]);
 KCmdLineArgs::init(argc, argv, &about);
 KCmdLineArgs::addCmdLineOptions(options);
 BoApplication app(argv0);

    // register ourselves as a dcop client
//    app.dcopClient()->registerAs(app.name(), false);

 KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
 QObject::connect(kapp, SIGNAL(lastWindowClosed()), kapp, SLOT(quit()));

 QGLWidget* glWidget = new QGLWidget();
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
		KUrl url = args->url(i);
		dlg->loadFromFile(url.directory(false) + url.fileName());
		dlg->show();
	}
 }
 */

 args->clear();
 int r = app.exec();
 delete glWidget;
 return r;
}


