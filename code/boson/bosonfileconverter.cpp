/*
    This file is part of the Boson game
    Copyright (C) 2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bosonfileconverter.h"

#include "bosonmap.h" // for BosonMap::mapFileFormatVersion()
#include "bodebug.h"
#include "defines.h"
#include "boversion.h"
#include "bosonsaveload.h"

#include <qdatastream.h>
#include <qcolor.h>
#include <qvaluelist.h>
#include <qdom.h>
#include <qimage.h>

#define BOSONMAP_VERSION BosonMap::mapFileFormatVersion()

// version number as used by boson 0.8.128 (aka 0x00,0x08,0x80 - development
// version. got never released)
#define BOSONMAP_VERSION_0_8_128 0x01

 // version number as used by boson 0.9 // AB: until this is released is must always be equal to BOSONMAP_VERSION!
#define BOSONMAP_VERSION_0_9 BOSONMAP_VERSION
// AB: hardcode once boson 0.9 is released! (see bosonsaveload.cpp, where it gets
// defined!)
#define BOSON_SAVEGAME_FORMAT_VERSION_0_9 BosonSaveLoad::latestSavegameVersion()
//#define BOSON_SAVEGAME_FORMAT_VERSION_0_9 ( ((0x00) << 16) | ((0x02) << 8) | (0x01) )



#if BOSON_VERSION_MINOR >= 0x09
do not compile!
// You are about to release boson 0.9 - first you have to hardcode the
// BOSONMAP_VERSION into BOSONMAP_VERSION_0_9 above!
// same for the
// BOSON_SAVEGAME_FORMAT_VERSION_0_9
#endif

// compatibility for boson 0.8
#define BO_COMPAT_0_8_TEXTURE_COUNT 3 // we use 3 textured by default for old maps (grass, desert, water).

bool BosonFileConverter::convertMapFile_From_0_8_To_0_9(const QByteArray& map, QByteArray* newMap, QByteArray* texMap)
{
 QDataStream readStream(map, IO_ReadOnly);

 // read in the boson 0.8 map first.
 // read map geo
 Q_INT32 mapWidth;
 Q_INT32 mapHeight;
 readStream >> mapWidth;
 readStream >> mapHeight;

 if (mapWidth < 10 || mapWidth > 500) {
	boError() << k_funcinfo << "broken map file - invalid width: " << mapWidth << endl;
	return false;
 }
 if (mapHeight < 10 || mapHeight > 500) {
	boError() << k_funcinfo << "broken map file - invalid height: " << mapWidth << endl;
	return false;
 }

 // read cells
 int* groundTypes = new int[mapWidth * mapHeight];
 unsigned char* versions = new unsigned char[mapWidth * mapHeight];
 for (int i = 0; i < mapWidth; i++) {
	for (int j = 0; j < mapHeight; j++) {
		Q_INT32 g;
		Q_INT8 version;
		readStream >> g;
		if (g < 0 || g > (7 + (4 * (12 + 4 * 16)))) { // Cell::isValidGround() from boson 0.8
			boError() << k_funcinfo << "not a valid ground" << endl;
			return false;
		}
		readStream >> version;
		if (version > 4) {
			boError() << k_funcinfo << "invalid version for ground" << endl;
			return false;
		}
		groundTypes[i + j * mapWidth] = g;
		versions[i + j * mapWidth] = version;
	}
 }

  // the versions don't influence textures or terrain type or anything else. they
 // are just "looks-better" tiles, so that we didn't have to use the same tile
 // for all grass cells. but we don't need it for converting.
 delete[] versions;
 versions = 0;

 MapToTexMap_From_0_8_To_0_9 converter((unsigned int)mapWidth, (unsigned int)mapHeight);
 bool ret = converter.convert(groundTypes, newMap, texMap);
 if (!ret) {
	boError() << k_funcinfo << "unable to convert file" << endl;
 }
 // delete temporary variables. also versions, which is NULL here, in case we
 // remove the delete above one day.
 delete[] groundTypes;
 delete[] versions;


 return ret;
}

bool BosonFileConverter::convertSaveGame_From_0_8_To_0_9(QMap<QString, QByteArray>& fileList)
{
 // converting a set of files from a .bsg archive from boson 0.8 to 0.9
 // "map" remains unmodified, same for "external.xml" (at the
 // moment at least - development process not yet completed). in kgame.xml we
 // need to update the savegame version.
 // the important part is "players.xml" and "canvas.xml".
 //
 // The structure of canvas.xml has changed and all Unit tags from players.xml
 // are moved to canvas.xml.
 // Thanks to XML this should be pretty simple to update.

 QDomDocument kgameDoc(QString::fromLatin1("Boson"));
 QDomDocument playersDoc(QString::fromLatin1("Players"));
 QDomDocument canvasDoc(QString::fromLatin1("Canvas"));
 if (!kgameDoc.setContent(QString(fileList["kgameXML"]))) {
	boError() << k_funcinfo << "unable to load kgame.xml" << endl;
	return false;
 }
 if (!playersDoc.setContent(QString(fileList["playersXML"]))) {
	boError() << k_funcinfo << "unable to load players.xml" << endl;
	return false;
 }
 if (!canvasDoc.setContent(QString(fileList["canvasXML"]))) {
	boError() << k_funcinfo << "unable to load canvas.xml" << endl;
	return false;
 }

 // this does two things now
 // 1. parse the players.xml file and add "Items" tag to canvas.xml for every
 // player found.
 // 2. while we are on it add all "Unit" nodes to canvas.xml and remove them
 // from players.xml
 QDomElement playersRoot = playersDoc.documentElement();
 QDomElement canvasRoot = canvasDoc.documentElement();
 QDomNodeList list = playersDoc.elementsByTagName(QString::fromLatin1("Player"));
 QMap<unsigned int, QDomElement> canvasOwner2Items;
 for (unsigned int i = 0; i < list.count(); i++) {
	QDomElement player = list.item(i).toElement();
	if (player.isNull()) {
		continue;
	}
	if (!player.hasAttribute(QString::fromLatin1("Id"))) {
		boError() << k_funcinfo << "Player tag " << i << " has not Id attribute" << endl;
		continue;
	}
	QDomElement items = canvasDoc.createElement(QString::fromLatin1("Items"));
	items.setAttribute(QString::fromLatin1("OwnerId"), player.attribute(QString::fromLatin1("Id")));
	canvasRoot.appendChild(items);
	canvasOwner2Items.insert(items.attribute(QString::fromLatin1("OwnerId")).toUInt(), items);

	QDomElement unitsTag = player.namedItem(QString::fromLatin1("Units")).toElement();
	if (unitsTag.isNull()) {
		boError() << k_funcinfo << "not valid Units tag found for player " << i << endl;
		continue;
	}
	QDomNodeList units = unitsTag.elementsByTagName(QString::fromLatin1("Unit"));
	for (unsigned int j = 0; j < units.count(); j++) {
		QDomElement unit = units.item(j).toElement();
		if (unit.isNull()) {
			continue;
		}
		unitsTag.removeChild(unit);
//		QDomElement unitCopy = unit.cloneNode(true);
		unit.setTagName(QString::fromLatin1("Item"));
		bool ok = false;
		unsigned int unitType = unit.attribute(QString::fromLatin1("UnitType")).toUInt(&ok);
		if (!ok) {
			boError() << k_funcinfo << "UnitType attribute is not a valid number for Item " << j << endl;
			continue;
		}
		unit.setAttribute(QString::fromLatin1("Type"), QString::number(unitType));

		// 200 is RTTI::UnitStart. Hardcoded in case this value is
		// changed one day (it was 200 in 0.8)
		unit.setAttribute(QString::fromLatin1("Rtti"), 200 + unitType);
		items.appendChild(unit);
	}
	player.removeChild(unitsTag);
 }


 // now we parse the canvas.xml and move the "Shot" tags into the "Items" tags.
 // players.xml is not touched.
 QDomElement shotsTag = canvasRoot.namedItem(QString::fromLatin1("Shots")).toElement();
 if (shotsTag.isNull()) {
	boError() << k_funcinfo << "not Shots tag in canvas.xml" << endl;
	return false;
 }
 list = shotsTag.elementsByTagName(QString::fromLatin1("Shot"));
 for (unsigned int i = 0; i < list.count(); i++) {
	QDomElement shot = list.item(i).toElement();
	if (shot.isNull()) {
		continue;
	}
	bool ok = false;
	if (!shot.hasAttribute(QString::fromLatin1("Owner"))) {
		boError() << k_funcinfo << "Shot tag has no Owner attribute" << endl;
		continue;
	}
	unsigned int id = shot.attribute(QString::fromLatin1("Owner")).toULong(&ok);
	if (!ok) {
		boError() << k_funcinfo << "Shot tag has no valid Owner attribute (" << shot.attribute(QString::fromLatin1("Owner")) << ")" << endl;
		continue;
	}
	QDomElement items = canvasOwner2Items[id];
	if (items.isNull()) {
		boError() << k_funcinfo << "could not find owner " << id << endl;
		continue;
	}
	shotsTag.removeChild(shot);
	items.appendChild(shot);
	shot.removeAttribute(QString::fromLatin1("Owner"));
	shot.setTagName(QString::fromLatin1("Item"));
	shot.setAttribute(QString::fromLatin1("Rtti"), 150);
 }
 canvasRoot.removeChild(shotsTag);

 QCString playersXML = playersDoc.toCString();
 QCString canvasXML = canvasDoc.toCString();

 // finally we need to update the savegame version.
 QDomElement kgameRoot = kgameDoc.documentElement();

 // AB: we set to 0.8.128 first - will get fixed to 0.9 later.
 kgameRoot.setAttribute(QString::fromLatin1("SaveGameVersion"), BOSON_SAVEGAME_FORMAT_VERSION_0_8_128);

 QCString kgameXML = kgameDoc.toCString();
 fileList.insert("kgameXML", kgameXML);
 fileList.insert("playersXML", playersXML);
 fileList.insert("canvasXML", canvasXML);

 return convertSaveGame_From_0_8_128_To_0_9(fileList);;
}

bool BosonFileConverter::convertSaveGame_From_0_8_128_To_0_9(QMap<QString, QByteArray>& fileList)
{
 // convert from boson 0.8.128 (aka 0x00, 0x08, 0x80) to 0.9.
 // note that 0.8.128 got never released and is a development only version. no
 // such files should exist. this function is also used to convert from 0.8 to
 // 0.9.
 if (!fileList.contains("kgameXML")) {
	boError() << k_funcinfo << "not a boson 0.8.128 file. missing kgameXML file" << endl;
	return false;
 }
 if (!fileList.contains("playersXML")) {
	boError() << k_funcinfo << "not a boson 0.8.128 file. missing playersXML file" << endl;
	return false;
 }
 if (!fileList.contains("canvasXML")) {
	boError() << k_funcinfo << "not a boson 0.8.128 file. missing canvasXML file" << endl;
	return false;
 }
 if (!fileList.contains("externalXML")) {
	boError() << k_funcinfo << "not a boson 0.8.128 file. missing map file" << endl;
	return false;
 }
 if (!fileList.contains("map")) {
	boError() << k_funcinfo << "not a boson 0.8.128 file. missing map file" << endl;
	return false;
 }

 // we need to split "map" up.
 // in boson 0.8.128 (and previous) the map was stored using
 // BosonPlayField::savePlayFieldForRemote(). in the stream are the following
 // files (in this order):
 // - map (width/height, groundtheme)
 // - heightmap
 // - texmap
 // - cells
 // - description (only C/description.xml, i.e. only default language)
 // the cells are redundant and don't remain in 0.9. This requires us to
 // recalculate them when games are loaded - and it could cause some bugs (in
 // case we ever change the way the cells are calculated), so we beed to be
 // careful.
 QByteArray map = fileList["map"];
 if (map.size() == 0) {
	boError() << k_funcinfo << "empty map file" << endl;
	return false;
 }
 QDataStream stream(map, IO_ReadOnly);

 QByteArray map_mapXML;
 QByteArray map_heightMap;
 QByteArray map_texMap;
 QByteArray map_description;

 QString magicCookie;
 Q_UINT32 mapVersion;
 Q_INT32 width;
 Q_INT32 height;
 QString groundTheme;
 {
	QByteArray buffer;
	stream >> buffer;

	QDataStream mapStream(buffer, IO_ReadOnly);
	mapStream >> magicCookie;
	mapStream >> mapVersion;
	if (magicCookie != QString::fromLatin1("BosonMap")) {
		boError() << k_funcinfo << "invalid magic cookie for map file: " << magicCookie << endl;
		return false;
	}
	if (mapVersion != BOSONMAP_VERSION_0_8_128) {
		boError() << k_funcinfo << "version must be " << BOSONMAP_VERSION_0_8_128 <<  "- is: " << mapVersion << endl;
		return false;
	}
	mapStream >> width;
	mapStream >> height;
	if (width < 10 || height < 10 || width > 500 || height > 500) {
		boError() << k_funcinfo << "invalid map geometry: " << width << "x" << height << endl;
		return false;
	}
	mapStream >> groundTheme;
 }
 {
	QDomDocument doc(QString::fromLatin1("BosonMap"));
	QDomElement root = doc.createElement(QString::fromLatin1("BosonMap"));
	root.setAttribute(QString::fromLatin1("GroundTheme"), groundTheme);
	doc.appendChild(root);
	QDomElement geometry = doc.createElement(QString::fromLatin1("Geometry"));
	geometry.setAttribute(QString::fromLatin1("Width"), width);
	geometry.setAttribute(QString::fromLatin1("Height"), height);
	root.appendChild(geometry);

	map_mapXML = doc.toCString();
 }

 // height map.
 float* heightMap = new float[width + height * (width + 1) + 1];
 for (int x = 0; x < width + 1; x++) {
	for (int y = 0; y < height + 1; y++) {
		stream >> heightMap[x + y * (height + 1)];
	}
 }
 {
	QImage image;
	image.create(width + 1, height + 1, 32, 0);
	for (int y = 0; y < image.height(); y++) {
		uint* p = (uint*)image.scanLine(y);
		for (int x = 0; x < image.width(); x++) {
			float value = heightMap[x + y * (height + 1)];
			// height to pixel:
			int v = (int)(value* 10 + 105);
			*p = qRgb(v, v, v);
			p++;
		}
	}
	QDataStream heightMapStream(map_heightMap, IO_WriteOnly);
	QImageIO io;
	io.setIODevice(heightMapStream.device());
	io.setFormat("PNG");
	io.setImage(image);
	io.write();
 }
 delete[] heightMap;
 heightMap = 0;

 // texmap.
 // this is used as-is, just copied to a new file (i.e. stream).
 QDataStream texMapStream(map_texMap, IO_WriteOnly);
 QString texMapCookie;
 Q_UINT32 texMapVersion;
 Q_UINT32 textureCount;
 stream >> texMapCookie;
 if (texMapCookie != QString::fromLatin1("BoTexMap")) {
	boError() << k_funcinfo << "invalid magic cookie for texmap: " << texMapCookie << endl;
	return false;
 }
 stream >> texMapVersion;
 if (texMapVersion != BOSONMAP_VERSION_0_8_128) {
	boError() << k_funcinfo << "invalid version for texmap (expected " << BOSONMAP_VERSION_0_8_128 << "): " << texMapVersion << endl;
	return false;
 }
 stream >> textureCount;
 if (textureCount == 0) {
	boError() << k_funcinfo << "no texture in texmap?!" << endl;
	return false;
 }
 if (textureCount > 100) {
	boError() << k_funcinfo << "more than 100 textures?!?!" << endl;
	return false;
 }
 texMapStream << QString::fromLatin1("BoTexMap"); // the texmap magic cookie
 texMapStream << (Q_UINT32)BOSONMAP_VERSION_0_9; // the texmap version
 texMapStream << (Q_UINT32)textureCount;
 for (unsigned int i = 0; i < textureCount; i++) {
	for (int x = 0; x < width + 1; x++) {
		for (int y = 0; y < height + 1; y++) {
			Q_UINT8 alpha;
			stream >> alpha;
			texMapStream << (Q_UINT8)alpha;
		}
	}
 }

 // the cells.
 for (int x = 0; x < width; x++) {
	for (int y = 0; y < height; y++) {
		Q_UINT8 amountOfLand;
		Q_UINT8 amountOfWater;
		stream >> amountOfLand;
		stream >> amountOfWater;
		// AB: we do not use these values. they will get re-calculated
		// on game startup from the texmap.
	}
 }

 // the description. only one description (default language) can exist in boson
 // < 0.9.
 QString descriptionXML;
 stream >> descriptionXML;
 map_description = descriptionXML.local8Bit(); // AB: i have _no_ idea whether this is correct. but I need a QCString, not a QString and this one seems (!) to be the better choice than utf8().

 QDomDocument kgameDoc(QString::fromLatin1("Boson"));
 if (!kgameDoc.setContent(QString(fileList["kgameXML"]))) {
	boError() << k_funcinfo << "unable to load kgame.xml" << endl;
	return false;
 }
 QDomElement kgameRoot = kgameDoc.documentElement();
 if (kgameRoot.isNull()) {
	boError() << k_funcinfo << "no root element found in kgame.xml" << endl;
	return false;
 }
 kgameRoot.setAttribute(QString::fromLatin1("SaveGameVersion"), BOSON_SAVEGAME_FORMAT_VERSION_0_9);
 QCString kgameXML = kgameDoc.toCString();

 fileList.remove("map");
 fileList.insert("map/mapXML", map_mapXML);
 fileList.insert("map/heightmap.png", map_heightMap);
 fileList.insert("map/texmap", map_texMap);
 fileList.insert("map/C/descriptionXML", map_description);
 fileList.insert("kgameXML", kgameXML);

 delete[] heightMap;
 heightMap = 0;

 return true;
}


bool MapToTexMap_From_0_8_To_0_9::convert(int* groundTypes, QByteArray* newMap, QByteArray* texMap)
{
 bool ret = true;
 QDataStream writeMapStream(*newMap, IO_WriteOnly);
 QDataStream writeTexMapStream(*texMap, IO_WriteOnly);

 writeMapStream << BOSONMAP_MAP_MAGIC_COOKIE;
 writeMapStream << (Q_UINT32)BOSONMAP_VERSION_0_9;
 writeTexMapStream << BOSONMAP_TEXMAP_MAGIC_COOKIE;
 writeTexMapStream << (Q_UINT32)BOSONMAP_VERSION_0_9;

 // now we can start to convert.
 writeMapStream << (Q_UINT32)mMapWidth;
 writeMapStream << (Q_UINT32)mMapHeight;
 writeMapStream << QString::fromLatin1("earth");
 writeTexMapStream << (Q_UINT32)BO_COMPAT_0_8_TEXTURE_COUNT;


 unsigned char* tex = new unsigned char[(mMapWidth + 1) * (mMapHeight + 1) * BO_COMPAT_0_8_TEXTURE_COUNT];
 convertToTexMap_From_0_8_To_0_9(groundTypes, tex);
 boDebug() << "writing texmap to stream" << endl;
 for (unsigned int i = 0; i < BO_COMPAT_0_8_TEXTURE_COUNT && ret; i++) {
	for (unsigned int x = 0; x < (unsigned int)mMapWidth + 1; x++) {
		for (unsigned int y = 0; y < (unsigned int)mMapHeight + 1; y++) {
			writeTexMapStream << (Q_UINT8)tex[texMapArrayPos(i, x, y)];
		}
	}
 }
 boDebug() << "wrote texmap to stream" << endl;
 delete[] tex;
 tex = 0;

 boDebug() << k_funcinfo << "done" << endl;
 return ret;
}


void MapToTexMap_From_0_8_To_0_9::addGroundType(int groundType, int* grass, int* desert, int* water)
{
 if (!isPlain(groundType)) {
	boError() << k_funcinfo << "not a plain groundtype: " << groundType << endl;
	return;
 }
 switch (groundType) {
	case 2: // GroundWater
		(*water) += 1;
		break;
	case 3: // GroundGrass
		(*grass) += 1;
		break;
	case 4: // GroundDesert
		(*desert) += 1;
		break;
	default:
		boWarning() << k_funcinfo << "unrecognized plain tile: " << groundType << endl;
		return;
 }
}

int MapToTexMap_From_0_8_To_0_9::from(int groundType) const
{
 if (!isValidGroundType(groundType) || isPlain(groundType)) {
	return 0;
 }
 int transRef = getTransRef(groundType);
 int from = 0;
 switch (transRef) {
	case 0: // TransGrassWater
		from = 3; // GroundGrass
		break;
	case 1: // TransGrassDesert
		from = 3; // GroundGrass
		break;
	case 2: // TransDesertWater
		from = 4; // GroundDesert
		break;
	case 3: // TransDeepWater
		boError() << k_funcinfo << "TransDeepWater should have been filtered out already! - groundType: " << groundType << endl;
		return 0;
		break;
	default:
		boError() << k_funcinfo << "invalid transref " << transRef << " - groundType: " << groundType << endl;
		return 0;
 }
 return from;
}

int MapToTexMap_From_0_8_To_0_9::to(int groundType) const
{
 if (!isValidGroundType(groundType) || isPlain(groundType)) {
	return 0;
 }
 int transRef = getTransRef(groundType);
 int to = 0;
 switch (transRef) {
	case 0: // TransGrassWater
		to = 2; // GroundWater
		break;
	case 1: // TransGrassDesert
		to = 4; // GroundDesert
		break;
	case 2: // TransDesertWater
		to = 2; // GroundWater
		break;
	case 3: // TransDeepWater
		boError() << k_funcinfo << "TransDeepWater should have been filtered out already! - groundType: " << groundType << endl;
		return 0;
		break;
	default:
		boError() << k_funcinfo << "invalid transref " << transRef << " - groundType: " << groundType << endl;
		return 0;
 }
 return to;
}

void MapToTexMap_From_0_8_To_0_9::addCornerTopLeft(int groundType, int* grass, int* desert, int* water)
{
 if (!isValidGroundType(groundType)) {
	boError() << k_funcinfo << "not a valid groundType: " << groundType << endl;
	return;
 }
 if (groundType >= 0 && groundType < 7) { // Cell::isPlain()
	addGroundType(groundType, grass, desert, water);
	return;
 }
 int from = this->from(groundType);
 int to = this->to(groundType);

 // the Cell::Transition names refer to the "to" part of the transition. e.g.
 // for desert_water and "TransUpLeft", we have desert in the lower right corner
 // of the cell only, the "UpLeft" part is water.
 int trans = getTransTile(groundType);
 switch (trans) {
	case 0: // TransUpLeft
		addGroundType(to, grass, desert, water);
		break;
	case 1: // TransUpRight
		addGroundType(to, grass, desert, water);
		break;
	case 2: // TransDownLeft
		addGroundType(to, grass, desert, water);
		break;
	case 3: // TransDownRight
		addGroundType(from, grass, desert, water);
		break;
	case 4: // TransUp
		addGroundType(to, grass, desert, water);
		break;
	case 5: // TransDown
		addGroundType(from, grass, desert, water);
		break;
	case 6: // TransLeft
		addGroundType(to, grass, desert, water);
		break;
	case 7: // TransRight
		addGroundType(from, grass, desert, water);
		break;
	case 8: // TransUpLeftInverted
		addGroundType(from, grass, desert, water);
		break;
	case 9: // TransUpRightInverted
		addGroundType(from, grass, desert, water);
		break;
	case 10: // TransDownLeftInverted
		addGroundType(from, grass, desert, water);
		break;
	case 11: // TransDownRightInverted
		addGroundType(to, grass, desert, water);
		break;
	default:
		// this is probably a bigtile. big tiles are not supported here
		// - they should have been split into plain tiles and small
		// transitions already!
		boError() << k_funcinfo << "invalid transition - groundType: " << groundType << endl;
		return;
 }
}

void MapToTexMap_From_0_8_To_0_9::addCornerTopRight(int groundType, int* grass, int* desert, int* water)
{
 if (!isValidGroundType(groundType)) {
	boError() << k_funcinfo << "not a valid groundType: " << groundType << endl;
	return;
 }
 if (groundType >= 0 && groundType < 7) { // Cell::isPlain()
	addGroundType(groundType, grass, desert, water);
	return;
 }
 int from = this->from(groundType);
 int to = this->to(groundType);

 // the Cell::Transition names refer to the "to" part of the transition. e.g.
 // for desert_water and "TransUpLeft", we have desert in the lower right corner
 // of the cell only, the "UpLeft" part is water.
 int trans = getTransTile(groundType);
 switch (trans) {
	case 0: // TransUpLeft
		addGroundType(to, grass, desert, water);
		break;
	case 1: // TransUpRight
		addGroundType(to, grass, desert, water);
		break;
	case 2: // TransDownLeft
		addGroundType(from, grass, desert, water);
		break;
	case 3: // TransDownRight
		addGroundType(to, grass, desert, water);
		break;
	case 4: // TransUp
		addGroundType(to, grass, desert, water);
		break;
	case 5: // TransDown
		addGroundType(from, grass, desert, water);
		break;
	case 6: // TransLeft
		addGroundType(from, grass, desert, water);
		break;
	case 7: // TransRight
		addGroundType(to, grass, desert, water);
		break;
	case 8: // TransUpLeftInverted
		addGroundType(from, grass, desert, water);
		break;
	case 9: // TransUpRightInverted
		addGroundType(from, grass, desert, water);
		break;
	case 10: // TransDownLeftInverted
		addGroundType(to, grass, desert, water);
		break;
	case 11: // TransDownRightInverted
		addGroundType(from, grass, desert, water);
		break;
	default:
		// this is probably a bigtile. big tiles are not supported here
		// - they should have been split into plain tiles and small
		// transitions already!
		boError() << k_funcinfo << "invalid transition - groundType: " << groundType << endl;
		return;
 }
}

void MapToTexMap_From_0_8_To_0_9::addCornerBottomLeft(int groundType, int* grass, int* desert, int* water)
{
 if (!isValidGroundType(groundType)) {
	boError() << k_funcinfo << "not a valid groundType: " << groundType << endl;
	return;
 }
 if (groundType >= 0 && groundType < 7) { // Cell::isPlain()
	addGroundType(groundType, grass, desert, water);
	return;
 }
 int from = this->from(groundType);
 int to = this->to(groundType);

 // the Cell::Transition names refer to the "to" part of the transition. e.g.
 // for desert_water and "TransUpLeft", we have desert in the lower right corner
 // of the cell only, the "UpLeft" part is water.
 int trans = getTransTile(groundType);
 switch (trans) {
	case 0: // TransUpLeft
		addGroundType(to, grass, desert, water);
		break;
	case 1: // TransUpRight
		addGroundType(from, grass, desert, water);
		break;
	case 2: // TransDownLeft
		addGroundType(to, grass, desert, water);
		break;
	case 3: // TransDownRight
		addGroundType(to, grass, desert, water);
		break;
	case 4: // TransUp
		addGroundType(from, grass, desert, water);
		break;
	case 5: // TransDown
		addGroundType(to, grass, desert, water);
		break;
	case 6: // TransLeft
		addGroundType(to, grass, desert, water);
		break;
	case 7: // TransRight
		addGroundType(from, grass, desert, water);
		break;
	case 8: // TransUpLeftInverted
		addGroundType(from, grass, desert, water);
		break;
	case 9: // TransUpRightInverted
		addGroundType(to, grass, desert, water);
		break;
	case 10: // TransDownLeftInverted
		addGroundType(from, grass, desert, water);
		break;
	case 11: // TransDownRightInverted
		addGroundType(from, grass, desert, water);
		break;
	default:
		// this is probably a bigtile. big tiles are not supported here
		// - they should have been split into plain tiles and small
		// transitions already!
		boError() << k_funcinfo << "invalid transition - groundType: " << groundType << endl;
		return;
 }
}

void MapToTexMap_From_0_8_To_0_9::addCornerBottomRight(int groundType, int* grass, int* desert, int* water)
{
 if (!isValidGroundType(groundType)) {
	boError() << k_funcinfo << "not a valid groundType: " << groundType << endl;
	return;
 }
 if (groundType >= 0 && groundType < 7) { // Cell::isPlain()
	addGroundType(groundType, grass, desert, water);
	return;
 }
 int from = this->from(groundType);
 int to = this->to(groundType);

 // the Cell::Transition names refer to the "to" part of the transition. e.g.
 // for desert_water and "TransUpLeft", we have desert in the lower right corner
 // of the cell only, the "UpLeft" part is water.
 int trans = getTransTile(groundType);
 switch (trans) {
	case 0: // TransUpLeft
		addGroundType(from, grass, desert, water);
		break;
	case 1: // TransUpRight
		addGroundType(to, grass, desert, water);
		break;
	case 2: // TransDownLeft
		addGroundType(to, grass, desert, water);
		break;
	case 3: // TransDownRight
		addGroundType(to, grass, desert, water);
		break;
	case 4: // TransUp
		addGroundType(from, grass, desert, water);
		break;
	case 5: // TransDown
		addGroundType(to, grass, desert, water);
		break;
	case 6: // TransLeft
		addGroundType(from, grass, desert, water);
		break;
	case 7: // TransRight
		addGroundType(to, grass, desert, water);
		break;
	case 8: // TransUpLeftInverted
		addGroundType(to, grass, desert, water);
		break;
	case 9: // TransUpRightInverted
		addGroundType(from, grass, desert, water);
		break;
	case 10: // TransDownLeftInverted
		addGroundType(from, grass, desert, water);
		break;
	case 11: // TransDownRightInverted
		addGroundType(from, grass, desert, water);
		break;
	default:
		// this is probably a bigtile. big tiles are not supported here
		// - they should have been split into plain tiles and small
		// transitions already!
		boError() << k_funcinfo << "invalid transition - groundType: " << groundType << endl;
		return;
 }
}

bool MapToTexMap_From_0_8_To_0_9::convertToTexMap_From_0_8_To_0_9(int* groundTypes, unsigned char* texMap)
{
 boDebug() << k_funcinfo << endl;
 if (mMapWidth == 0 || mMapHeight == 0) {
	return false;
 }
 if (!groundTypes) {
	return false;
 }
 if (!texMap) {
	return false;
 }

 // initialize all values to 0 first
 for (unsigned int i = 0; i < BO_COMPAT_0_8_TEXTURE_COUNT; i++) {
	for (unsigned int x = 0; x < mMapWidth + 1; x++) {
		for (unsigned int y = 0; y < mMapHeight + 1; y++) {
			setTexMapAlpha(i, x, y, 0, texMap);
		}
	}
 }

 // fixing groundTypes a bit...
 // we have to simplify a lot before i am able to write conversion code...
 fixGroundTypes(groundTypes);

 const unsigned int texGrass = 0;
 const unsigned int texDesert = 1;
 const unsigned int texWater = 2;

 for (unsigned int x = 0; x < mMapWidth + 1; x++) {
	for (unsigned int y = 0; y < mMapHeight + 1; y++) {
		// if a cell doesn't exist for a corner it'll have an invalid
		// groundType
		int topLeft = -1;
		int topRight = -1;
		int bottomLeft = -1;
		int bottomRight = -1;
		if (x > 0 && y > 0) {
			topLeft = groundTypes[(x - 1) + (y - 1) * mMapWidth];
		}
		if (x < mMapWidth && y < mMapHeight) {
			bottomRight = groundTypes[x + y * mMapWidth];
		}
		if (x < mMapWidth && y > 0) {
			topRight = groundTypes[x + (y - 1) * mMapWidth];
		}
		if (x > 0 && y < mMapWidth) {
			bottomLeft = groundTypes[(x - 1) + y * mMapWidth];
		}

		int grass = 0;
		int desert = 0;
		int water = 0;

		// collect informations about the (x,y) in the texMap.
		// note: the cell named "topLeft" for the (x,y) corner,
		// considers the corner as "bottomRight"!

		// the top-left cell of this corner
		if (isValidGroundType(topLeft)) {
			addCornerBottomRight(topLeft, &grass, &desert, &water);
		}

		// the top-right cell of this corner
		if (isValidGroundType(topRight)) {
			addCornerBottomLeft(topRight, &grass, &desert, &water);
		}

		// the bottom-left cell of this corner
		if (isValidGroundType(bottomLeft)) {
			addCornerTopRight(bottomLeft, &grass, &desert, &water);
		}

		// the bottom-right cell of this corner
		if (isValidGroundType(bottomRight)) {
			addCornerTopLeft(bottomRight, &grass, &desert, &water);
		}

		// now we know how many grass/desert/water cells are adjacent to
		// this cell. note, that in a corner the groundType of a cell is
		// unique - even for transitions!
		// with this information we can compute alpha values for the
		// texmap.
		int cells = grass + desert + water; // will always be for, except at the borders!
		if (cells > 4 || cells <= 0) {
			boError() << k_funcinfo << "more than 4 or less than 1 adjacent cells is impossible for a corner!" << endl;
			grass = 0;
			desert = 0;
			water = 4;
			cells = 4;
		}
		unsigned char grassAlpha = 0;
		unsigned char desertAlpha = 0;
		unsigned char waterAlpha = 0;

		grassAlpha = (unsigned char)(255 * grass / cells);
		desertAlpha = (unsigned char)(255 * desert / cells);
		waterAlpha = (unsigned char)(255 * water / cells);

		setTexMapAlpha(texGrass, x, y, grassAlpha, texMap);
		setTexMapAlpha(texDesert, x, y, desertAlpha, texMap);
		setTexMapAlpha(texWater, x, y, waterAlpha, texMap);
	}
 }

 return true;
}

void MapToTexMap_From_0_8_To_0_9::fixGroundTypes(int* groundTypes)
{
 const int tilesPerTransition = 12 + 4 * 16;
 const int groundTilesNumber = 7 + 4 * tilesPerTransition;
 for (unsigned int x = 0; x < mMapWidth; x++) {
	for (unsigned int y = 0; y < mMapHeight; y++) {
		int g = groundTypes[x + y * mMapWidth];
		if (g < 7) { // Cell::isPlain()
			if (g == 0) { // GroundUnknown.
				// we cannot use this. converting to
				// GroundWater.
				// most units can't cross water, so this is the
				// closest type.
				g = 2; // GroundWater
			} else if (g == 1) { // GroundDeepWater
				// we make no difference between deep water and
				// water
				g = 2; // GroundWater
			} else if (g == 5 || g == 6) { // GroundGrass[Mineral|Oil]
				// mineral / oil won't be in the groundType.
				// we lose this information at this point, but
				// that won't hurt too much as mining is mostly
				// experimental anyway.
				g = 3; // GroundGrass
			}
		} else if (g >= 7 && g < groundTilesNumber) { // Cell::isTrans()
			int transType = (g - 7) / tilesPerTransition; // Cell::getTransRef()
			if (transType < 0 || transType >= 4) {
				boError() << k_funcinfo << "invalid transType " << transType << " for groundType " << g << " at " << x << "," << y << endl;
				g = 2; // same as for GroundUnknown.
			} else if (transType == 3) { // TransDeepWater
				// deep water is equal to water for us.
				// so we have an easy job here: the transition
				// becomes a plain (water) tile.
				g = 2;
			}

			int transTile = (g - 7) % tilesPerTransition; // Cell::getTransTile()
			if (transTile >= 12) { // Cell::isBigTrans()
				// a big transition consists of 4 tiles. for our
				// conversion algorithm we'll be able to convert
				// these into small transitions and plain tiles.
				int from = this->from(g);
				int to = this->to(g);
				int trans = transTile - 12; // bigtile number (12==smallTilesPerTransition)
				trans /= 4; // which one
				trans += 12;

				// want docs for this?
				// look into ground/earth/*_*/ and paint some
				// pictures for these transitions. it's close to
				// impossible to explain this, or keep these
				// information in text.
				// the big tiles are the earth/*_*/*_*_nn-*.png
				// with nn >= 13. internally we work with nn - 1
				//
				// note that grass_desert and grass_water use
				// the same format (i.e. their "to" part are
				// treated equally), whereas for desert_water
				// the "to" and "from" part is flipped.
				if (transType == 2) { // TransDesertWater
					// flip from/to for desert_water
					int tmp = from;
					from = to;
					to = tmp;
				}
				int big[4]; // topleft,topright,bottomleft,bottomright
				big[0] = big[1] = big[2] = big[3] = to;
				switch (trans) {
					case 12:
						big[3] = from;
						break;
					case 13:
						big[2] = from;
						break;
					case 14:
						big[1] = from;
						break;
					case 15:
						big[0] = from;
						break;
					case 16:
						big[0] = big[1] = big[2] = from;
						break;
					case 17:
						big[0] = big[1] = big[3] = from;
						break;
					case 18:
						big[0] = big[2] = big[3] = from;
						break;
					case 19:
						big[1] = big[2] = big[3] = from;
						break;
					case 20:
					case 21:
						big[2] = big[3] = from;
						break;
					case 22:
					case 23:
						big[0] = big[1] = from;
						break;
					case 24:
					case 26: // 24 and 26, not 25! not a typo!
						big[0] = big[2] = from;
						break;
					case 25:
					case 27:
						big[1] = big[3] = from;
						break;
					default:
						boError() << k_funcinfo << "invalid trans number for bigtile: " << trans << " groundType: " << g << endl;
						g = 2;
						continue;
				}

				int pos = transTile - ((trans - 12) * 4 + 12);
				if (pos < 0 || pos >= 4) {
					boError() << k_funcinfo << "invalid position: " << pos << " groundType = " << g << endl;
					g = 2;
					continue;
				}
				g = big[pos];
			}

		} else {
			boWarning() << k_funcinfo << "invalid groundtype at " << x << " " << y << endl;
			g = 2; // AB: same as for GroundUnknown above.
		}

		groundTypes[x + y * mMapWidth] = g;
	}
 }
}

void MapToTexMap_From_0_8_To_0_9::setTexMapAlpha(unsigned int texture, unsigned int x, unsigned int y, unsigned char alpha, unsigned char* texMap)
{
 unsigned char* tex = &texMap[texMapArrayPos(texture, x, y)];
 *tex = alpha;
}

