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

#include <qdatastream.h>
#include <qcolor.h>

#define BOSONMAP_VERSION BosonMap::mapFileFormatVersion()

 // version number as used by boson 0.9 // AB: until this is released is must always be equal to BOSONMAP_VERSION!
#define BOSONMAP_VERSION_0_9 BOSONMAP_VERSION

#if BOSON_VERSION_MINOR >= 0x09
do not compile!
// You are about to release boson 0.9 - first you have to hardcode the
// BOSONMAP_VERSION into BOSONMAP_VERSION_0_9 above!
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
 writeTexMapStream << (Q_UINT32)BO_COMPAT_0_8_TEXTURE_COUNT;


 unsigned char* tex = new unsigned char[(mMapWidth + 1) * (mMapHeight + 1) * BO_COMPAT_0_8_TEXTURE_COUNT];
 convertToTexMap_From_0_8_To_0_9(groundTypes, tex);
 boDebug() << "writing texmap to stream" << endl;
 for (unsigned int i = 0; i < BO_COMPAT_0_8_TEXTURE_COUNT && ret; i++) {
	Q_INT32 groundType = 0;
	QRgb miniMapColor = Qt::black.rgb();
	Q_UINT8 amountOfLand = 0;
	Q_UINT8 amountOfWater = 0;
	if (i == 0) { // grass
		groundType = 0;
		miniMapColor = Qt::darkGreen.rgb();
		amountOfLand = 255;
		amountOfWater = 0;
	} else if (i == 1) { // desert
		groundType = 1;
		miniMapColor = Qt::darkYellow.rgb();
		amountOfLand = 255;
		amountOfWater = 0;
	} else if (i == 2) { // water
		groundType = 2;
		miniMapColor = Qt::blue.rgb();
		amountOfLand = 0;
		amountOfWater = 255;
	} else {
		boError() << k_funcinfo << "invalid texture " << i << " - only 3 textures supported here!" << endl;
		ret = false;
		continue;
	}
	writeTexMapStream << (Q_INT32)groundType;
	writeTexMapStream << (QRgb)miniMapColor;
	writeTexMapStream << (Q_UINT8)amountOfLand;
	writeTexMapStream << (Q_UINT8)amountOfWater;
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

