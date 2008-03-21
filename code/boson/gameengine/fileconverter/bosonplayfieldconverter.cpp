/*
    This file is part of the Boson game
    Copyright (C) 2002-2006 Andreas Beckermann (b_mann@gmx.de)

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

#include "bosonplayfieldconverter.h"

#include "../../bomemory/bodummymemory.h"
#include "../boversion.h"
#include "bosonfileconverter.h"
#include "bosavegameconverter.h"
#include "bodebug.h"

#include <qdom.h>
#include <qmap.h>
#include <qstringlist.h>


// AB: must be the LAST include
#include "no_game_code.h"


BosonPlayFieldConverter::BosonPlayFieldConverter()
{
}

BosonPlayFieldConverter::~BosonPlayFieldConverter()
{
}

bool BosonPlayFieldConverter::convertFilesToCurrentFormat(QMap<QString, QByteArray>& destFiles)
{
 if (destFiles.isEmpty()) {
	boError() << k_funcinfo << "no files availble" << endl;
	return false;
 }
 // AB: this was added after 0.9, so at this point the files in destFiles are
 // _at least_ from boson 0.9.

 QStringList requireFiles; // required by Boson 0.9 _and_ current all following formats
 requireFiles.append("kgame.xml"); // contains the version number and thus must be required!
 requireFiles.append("map/texmap");
 requireFiles.append("map/heightmap.png");
 requireFiles.append("map/map.xml");
 requireFiles.append("players.xml");
 requireFiles.append("canvas.xml");
 requireFiles.append("C/description.xml");
 // AB: all other files are optional for boson 0.9

 for (QStringList::iterator it = requireFiles.begin(); it != requireFiles.end(); ++it) {
	if (!destFiles.contains(*it)) {
		boError() << k_funcinfo << "no file \"" << *it << "\" found." << endl;
		return false;
	}
 }

 QDomDocument kgameDoc(QString::fromLatin1("Boson"));
 if (!kgameDoc.setContent(QString(destFiles["kgame.xml"]))) {
	boError() << k_funcinfo << "unable to load kgame.xml" << endl;
	return false;
 }
 QDomElement kgameRoot = kgameDoc.documentElement();
 bool ok = false;
 unsigned int version = kgameRoot.attribute("Version").toUInt(&ok);
 if (!ok) {
	boError() << k_funcinfo << "Version attribute in kgame.xml is not a valid number" << endl;
	return false;
 }
 if (version < BOSON_SAVEGAME_FORMAT_VERSION_0_9) {
	boError() << k_funcinfo << "this method expects at least file format from 0.9 (" << BOSON_SAVEGAME_FORMAT_VERSION_0_9 << ") - found: " << version << endl;
	return false;
 }

 bool conversionSucceeded = true;
 bool handled = false;
 QMap<int, BoSaveGameConverter*> converters = BoSaveGameConverter::createConverters();
 if (!converters.contains(version)) {
	handled = false;
	conversionSucceeded = true;
 } else {
	conversionSucceeded = converters[version]->convertFiles(destFiles);
	handled = true;
 }
 for (QMap<int, BoSaveGameConverter*>::iterator it = converters.begin(); it != converters.end(); ++it) {
	delete it.data();
 }
 converters.clear();

 if (!conversionSucceeded) {
	boError() << k_funcinfo << "conversion failed" << endl;
	return false;
 }

 if (handled) {
	// the files got converted to a different format.
	// -> call this function again and check whether we can convert any
	// further (e.g. 0.9 -> 0.9.1 in the first call, 0.9.1->0.10 in the 2nd
	// or so)

	 // first check whether the version got changed (prevent infinite loop)
	if (!kgameDoc.setContent(QString(destFiles["kgame.xml"]))) {
		boError() << k_funcinfo << "unable to load kgame.xml after conversion" << endl;
		return false;
	}
	QDomElement kgameRoot = kgameDoc.documentElement();
	ok = false;
	unsigned int newVersion = kgameRoot.attribute("Version").toUInt(&ok);
	if (!ok) {
		boError() << k_funcinfo << "Version attribute in kgame.xml is not a valid number" << endl;
		return false;
	}
	if (newVersion == version) {
		boError() << k_funcinfo << "format " << version << " got converted, but version has not been changed" << endl;
		return false;
	}

	// check whether we can convert any further
	return convertFilesToCurrentFormat(destFiles);
 }

 return true;
}



