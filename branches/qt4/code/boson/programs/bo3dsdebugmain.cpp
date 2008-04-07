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


#include "../bomemory/bodummymemory.h"
#include "kgame3dsmodeldebug.h"
#include "boapplication.h"
#include "bodebug.h"
#include "boversion.h"

#include <qlayout.h>
#include <qstringlist.h>
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <Q3CString>

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <KDialog>
#include <kglobal.h>
#include <kstandarddirs.h>

static KLocalizedString description =
    ki18n("A realtime strategy game for KDE");

static const char *version = BOSON_VERSION_STRING;

//void postBosonConfigInit();

int main(int argc, char ** argv)
{
 KAboutData data("bo3dsdebug",
		QByteArray(),
		ki18n("Boson .3ds file debug dialog"),
		version,
		description,
		KAboutData::License_GPL,
		ki18n("(c) 2003-2005, Andreas Beckermann <b_mann@gmx.de>"),
		KLocalizedString(),
		"http://boson.eu.org");
 data.addAuthor(ki18n("Andreas Beckermann"), ki18n("Maintainer"), "b_mann@gmx.de");

 Q3CString argv0(argv[0]);
 KCmdLineArgs::init( argc, argv, &data );

 BoApplication app(argv0);
 KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
 QObject::connect(kapp, SIGNAL(lastWindowClosed()), kapp, SLOT(quit()));

 KDialog* dialog = new KDialog();
 dialog->setWindowTitle(KDialog::makeStandardCaption(i18n("Debug 3ds files")));
 dialog->setButtons(KDialog::Cancel);

 QWidget* w = dialog->mainWidget();
 Q3VBoxLayout* layout = new Q3VBoxLayout(w);
 KGame3DSModelDebug* models = new KGame3DSModelDebug(w);
 layout->addWidget(models);

 QStringList list = KGlobal::dirs()->resourceDirs("data");
 for (int i = 0; i < list.count(); i++) {
	models->addFiles(list[i] + QString::fromLatin1("/boson/themes"));
 }

 dialog->exec();

 return 0;
}

