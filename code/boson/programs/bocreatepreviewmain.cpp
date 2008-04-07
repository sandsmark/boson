/*
    This file is part of the Boson game
    Copyright (C) 2006-2008 Andreas Beckermann (b_mann@gmx.de)

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
#include "../boversion.h"
#include "bodebug.h"
#include "../boglobal.h"
#include "../boapplication.h"
#include "../gameengine/bpfloader.h"
#include "../gameengine/bosonplayfield.h"
#include "../gameengine/bosonsaveload.h"
#include "../gameengine/bosonmap.h"
#include "../gameengine/bosongroundtheme.h"
#include "../bosondata.h"

#include <kaboutdata.h>
#include <kcmdlineargs.h>

#include <qmap.h>
#include <qimage.h>
//Added by qt3to4:
#include <Q3CString>

static KLocalizedString description =
    ki18n("Boson file converter");

static const char *version = BOSON_VERSION_STRING;

int main(int argc, char **argv)
{
 KAboutData about("bofileconverter",
		QByteArray(),
		ki18n("BoFileConverter"),
		version,
		description,
		KAboutData::License_GPL,
		ki18n("(C) 2006 Andreas Beckermann"),
		KLocalizedString(),
		"http://boson.eu.org");
 about.addAuthor(ki18n("Andreas Beckermann"),
		ki18n("Coding & Current Maintainer"),
		"b_mann@gmx.de");

 KCmdLineOptions options;
 options.add("+input", ki18n("Input .bpf file"));
 options.add("+output", ki18n("Output .bpf file"));


 Q3CString argv0(argv[0]);
 KCmdLineArgs::init(argc, argv, &about);
 KCmdLineArgs::addCmdLineOptions(options);

 BoApplication app(argv0, false);

 KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

 if (args->count() < 2) {
	boError() << k_funcinfo << "not enough arguments" << endl;
	return 1;
 }

 QString inFile = args->arg(0);
 QString outFile = args->arg(1);

 boDebug() << k_funcinfo << "loading " << inFile << endl;

 QByteArray buffer = BPFLoader::loadFromDiskToStream(inFile);
 if (buffer.size() == 0) {
	boError() << k_funcinfo << "unable to load " << inFile << endl;
	return 1;
 }
 QMap<QString, QByteArray> files;
 if (!BPFLoader::unstreamFiles(files, buffer)) {
	boError() << k_funcinfo << "invalid file format for playfield " << inFile << endl;
	return 1;
 }

 if (!BosonGroundTheme::createGroundThemeList()) {
	boError() << k_funcinfo << "unable to load groundthemes" << endl;
	return 1;
 }

 BosonPlayField* field = new BosonPlayField();
 if (!field->loadPlayFieldFromFiles(files)) {
	boError() << k_funcinfo << "could load playfield from disk into memory, but failed at loading data into our data structures" << endl;
	return 1;
 }
 if (!field->map()) {
	BO_NULL_ERROR(field->map());
	return 1;
 }
 QByteArray mapPNG = field->map()->saveMapPreviewPNGToFile();

 files.insert("mappreview/map.png", mapPNG);

 if (!BosonSaveLoad::saveToFile(files, outFile)) {
	boError() << k_funcinfo << "unable to save to " << outFile << endl;
	return 1;
 }
 return 0;
}

