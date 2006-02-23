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


#include "../bomemory/bodummymemory.h"
#include "kgame3dsmodeldebug.h"
#include "boapplication.h"
#include "bodebug.h"
#include "boversion.h"

#include <qlayout.h>
#include <qstringlist.h>

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kdialogbase.h>
#include <kglobal.h>
#include <kstandarddirs.h>

static const char *description =
    I18N_NOOP("A realtime strategy game for KDE");

static const char *version = BOSON_VERSION_STRING;

static KCmdLineOptions options[] =
{
  { 0, 0, 0 }
};

//void postBosonConfigInit();

int main(int argc, char ** argv)
{
 KAboutData data("bo3dsdebug",
		I18N_NOOP("Boson .3ds file debug dialog"),
		version,
		description,
		KAboutData::License_GPL,
		"(c) 2003-2005, Andreas Beckermann <b_mann@gmx.de>",
		0,
		"http://boson.eu.org");
 data.addAuthor("Andreas Beckermann", I18N_NOOP("Maintainer"), "b_mann@gmx.de");

 QCString argv0(argv[0]);
 KCmdLineArgs::init( argc, argv, &data );
 KCmdLineArgs::addCmdLineOptions( options );

 BoApplication app(argv0);
 KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
 QObject::connect(kapp, SIGNAL(lastWindowClosed()), kapp, SLOT(quit()));

 KDialogBase* dialog = new KDialogBase(KDialogBase::Plain, i18n("Debug 3ds files"),
		KDialogBase::Cancel, KDialogBase::Cancel, 0,
		"debug3dsdialog", false, true);

 QWidget* w = dialog->plainPage();
 QVBoxLayout* layout = new QVBoxLayout(w);
 KGame3DSModelDebug* models = new KGame3DSModelDebug(w);
 layout->addWidget(models);

 QStringList list = KGlobal::dirs()->resourceDirs("data");
 for (unsigned int i = 0; i < list.count(); i++) {
	models->addFiles(list[i] + QString::fromLatin1("/boson/themes"));
 }

 dialog->exec();

 return 0;
}

