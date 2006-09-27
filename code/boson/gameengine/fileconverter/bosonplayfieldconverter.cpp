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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "bosonplayfieldconverter.h"

#include "../../bomemory/bodummymemory.h"
#include "../boversion.h"
#include "bosonfileconverter.h"
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

 // AB: this is the point where you should insert your conversion code !
 bool handled = false;
 if (!convertFilesToCurrentFormat(destFiles, version, &handled)) {
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


bool BosonPlayFieldConverter::convertFilesToCurrentFormat(QMap<QString, QByteArray>& destFiles, unsigned int version, bool* handled)
{
 // AB: this is where the conversion should be done!
 // (of course - most actual conversion code will be in BosonFileConverter)

 *handled = true;
 bool ret = false;
 switch (version) {
	case BOSON_SAVEGAME_FORMAT_VERSION_0_9:
	{
		boDebug() << k_funcinfo << "converting from 0.9 to 0.9.1 format" << endl;
		BosonFileConverter converter;
		if (!converter.convertPlayField_From_0_9_To_0_9_1(destFiles)) {
			boError() << k_funcinfo << "could not convert from boson 0.9 to boson 0.9.1 file format" << endl;
			ret = false;
		} else {
			ret = true;
		}
		break;
	}
	case BOSON_SAVEGAME_FORMAT_VERSION_0_9_1:
	{
		boDebug() << k_funcinfo << "converting from 0.9.1 to 0.10 format" << endl;
		BosonFileConverter converter;
		if (!converter.convertPlayField_From_0_9_1_To_0_10(destFiles)) {
			boError() << k_funcinfo << "could not convert from boson 0.9 to boson 0.9.1 file format" << endl;
			ret = false;
		} else {
			ret = true;
		}
		break;
	}
	case BOSON_SAVEGAME_FORMAT_VERSION_0_10:
	{
		boDebug() << k_funcinfo << "converting from 0.10 to 0.10.80 format" << endl;
		BosonFileConverter converter;
		if (!converter.convertPlayField_From_0_10_To_0_10_80(destFiles)) {
			boError() << k_funcinfo << "could not convert from boson 0.10 to boson 0.10.80 file format" << endl;
			ret = false;
		} else {
			ret = true;
		}
		break;
	}
	case BOSON_MAKE_SAVEGAME_FORMAT_VERSION(0x00, 0x02, 0x04): // development version ("0.10.80")
	{
		boDebug() << k_funcinfo << "converting from 0.10.80 to 0.10.81 format" << endl;
		BosonFileConverter converter;
		if (!converter.convertPlayField_From_0_10_80_To_0_10_81(destFiles)) {
			boError() << k_funcinfo << "could not convert from boson 0.10.80 to boson 0.10.81 file format" << endl;
			ret = false;
		} else {
			ret = true;
		}
		break;
	}
	case BOSON_MAKE_SAVEGAME_FORMAT_VERSION(0x00, 0x02, 0x05): // development version ("0.10.81")
	{
		boDebug() << k_funcinfo << "converting from 0.10.81 to 0.10.82 format" << endl;
		BosonFileConverter converter;
		if (!converter.convertPlayField_From_0_10_81_To_0_10_82(destFiles)) {
			boError() << k_funcinfo << "could not convert from boson 0.10.81 to boson 0.10.82 file format" << endl;
			ret = false;
		} else {
			ret = true;
		}
		break;
	}
	case BOSON_MAKE_SAVEGAME_FORMAT_VERSION(0x00, 0x02, 0x06): // development version ("0.10.82")
	{
		boDebug() << k_funcinfo << "converting from 0.10.82 to 0.10.83 format" << endl;
		BosonFileConverter converter;
		if (!converter.convertPlayField_From_0_10_82_To_0_10_83(destFiles)) {
			boError() << k_funcinfo << "could not convert from boson 0.10.82 to boson 0.10.83 file format" << endl;
			ret = false;
		} else {
			ret = true;
		}
		break;
	}
	case BOSON_MAKE_SAVEGAME_FORMAT_VERSION(0x00, 0x02, 0x07): // development version ("0.10.83")
	{
		boDebug() << k_funcinfo << "converting from 0.10.83 to 0.10.84 format" << endl;
		BosonFileConverter converter;
		if (!converter.convertPlayField_From_0_10_83_To_0_10_84(destFiles)) {
			boError() << k_funcinfo << "could not convert from boson 0.10.83 to boson 0.10.84 file format" << endl;
			ret = false;
		} else {
			ret = true;
		}
		break;
	}
	case BOSON_MAKE_SAVEGAME_FORMAT_VERSION(0x00, 0x02, 0x08): // development version ("0.10.84")
	{
		boDebug() << k_funcinfo << "converting from 0.10.84 to 0.10.85 format" << endl;
		BosonFileConverter converter;
		if (!converter.convertPlayField_From_0_10_84_To_0_10_85(destFiles)) {
			boError() << k_funcinfo << "could not convert from boson 0.10.84 to boson 0.10.85 file format" << endl;
			ret = false;
		} else {
			ret = true;
		}
		break;
	}
	case BOSON_MAKE_SAVEGAME_FORMAT_VERSION(0x00, 0x02, 0x09): // development version ("0.10.85")
	{
		boDebug() << k_funcinfo << "converting from 0.10.85 to 0.11 format" << endl;
		BosonFileConverter converter;
		if (!converter.convertPlayField_From_0_10_85_To_0_11(destFiles)) {
			boError() << k_funcinfo << "could not convert from boson 0.10.85 to boson 0.11 file format" << endl;
			ret = false;
		} else {
			ret = true;
		}
		break;
	}
	case BOSON_SAVEGAME_FORMAT_VERSION_0_11:
	{
		boDebug() << k_funcinfo << "converting from 0.11 to 0.11.80 format" << endl;
		BosonFileConverter converter;
		if (!converter.convertPlayField_From_0_11_To_0_11_80(destFiles)) {
			boError() << k_funcinfo << "could not convert from boson 0.11 to boson 0.11.80 file format" << endl;
			ret = false;
		} else {
			ret = true;
		}
		break;
	}
	case BOSON_MAKE_SAVEGAME_FORMAT_VERSION(0x00, 0x03, 0x00): // development version ("0.11.80")
	{
		boDebug() << k_funcinfo << "converting from 0.11.80 to 0.11.81 format" << endl;
		BosonFileConverter converter;
		if (!converter.convertPlayField_From_0_11_80_To_0_11_81(destFiles)) {
			boError() << k_funcinfo << "could not convert from boson 0.11.80 to boson 0.11.81 file format" << endl;
			ret = false;
		} else {
			ret = true;
		}
		break;
	}
	case BOSON_MAKE_SAVEGAME_FORMAT_VERSION(0x00, 0x03, 0x01): // development version ("0.11.81")
	{
		const char* from = "0.11.81";
		const char* to = "0.12";
		boDebug() << k_funcinfo << "converting from " << from << " to " << to << " format" << endl;
		BosonFileConverter converter;
		if (!converter.convertPlayField_From_0_11_81_To_0_12(destFiles)) {
			boError() << k_funcinfo << "could not convert from boson " << from << " to boson " << to << " file format" << endl;
			ret = false;
		} else {
			ret = true;
		}
		break;
	}
	case BOSON_SAVEGAME_FORMAT_VERSION_0_12:
	{
		const char* from = "0.12";
		const char* to = "0.13";
		boDebug() << k_funcinfo << "converting from " << from << " to " << to << " format" << endl;
		BosonFileConverter converter;
		if (!converter.convertPlayField_From_0_12_To_0_13(destFiles)) {
			boError() << k_funcinfo << "could not convert from boson " << from << " to boson " << to << " file format" << endl;
			ret = false;
		} else {
			ret = true;
		}
		break;
	}
	default:
		*handled = false;
		ret = true;
		break;
 }
 return ret;
}


