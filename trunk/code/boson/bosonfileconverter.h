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
#ifndef BOSONFILECONVERTER_H
#define BOSONFILECONVERTER_H

#include <qdatastream.h>

template<class T> class QValueList;
template<class T, class T2> class QMap;
class QDomDocument;

class BosonFileConverter
{
public:
	BosonFileConverter()
	{
	}
	~BosonFileConverter()
	{
	}

	/**
	 * Convert a map from file (as loaded by @ref BosonMap::loadMapFromFile)
	 * from boson 0.8 (and maybe even before?) into the file format as used
	 * by boson 0.9 (AB: not yet released, so this is CVS only...)
	 **/
	bool convertMapFile_From_0_8_To_0_9(const QByteArray& map, QByteArray* newMap, QByteArray* texMap);

	/**
	 * Convert a "map" file (part of the .bpf files) to a map.xml file, as
	 * used in boson 0.9.
	 *
	 * It is a trivial conversion, the values from the @p map array are
	 * streamed into variables and then placed into a DOM tree, which is
	 * stored into @p mapXML.
	 **/
	bool convertMapFile_From_0_8_128_To_0_9(const QByteArray& map, QByteArray* mapXML, int* mapWidth = 0, int* mapHeight = 0);

	/**
	 * Convert a set of files (kgame.xml, players.xml, canvas.xml,
	 * external.xml, map) from boson 0.8 to boson 0.9.
	 *
	 * @param list A reference to the list of files. This will be modified
	 * to the 0.9 file format or left untouched on error.
	 **/
	bool convertSaveGame_From_0_8_To_0_9(QMap<QString, QByteArray>& list);

	/**
	 * Convert a set of files from boson 0.8.128 (i.e. 0x00, 0x08, 0x80) to
	 * boson 0.9.
	 *
	 * The difference applies to the file "map" only, which get split up
	 * into different files.
	 **/
	bool convertSaveGame_From_0_8_128_To_0_9(QMap<QString, QByteArray>& list);

protected:
	bool loadXMLDoc(QDomDocument* doc, const QString& xml) const;
};

/**
 * Helper class for @ref BosonFileConverter.
 *
 * This class takes care about converting a map file from boson 0.8 to the
 * format used in boson 0.9.
 **/
class MapToTexMap_From_0_8_To_0_9
{
public:
	MapToTexMap_From_0_8_To_0_9(unsigned int width, unsigned int height)
	{
		mMapWidth = width;
		mMapHeight = height;
	}

	/**
	 * Convert an ald map (boson <= 0.8) to a the format used by boson 0.9.
	 *
	 * @param groundTypes An array of mapWidht*mapHeight elements,
	 * containing the groundTypes (see Cell::groundType() from boson 0.8).
	 * The version numbers which are stored in Cell, too can safely be
	 * ignored, as they don't influece the groundType in any way.
	 * @param map The output buffer for the map file.
	 * @param texMap The ouput buffer for the texmap.
	 *
	 * WARNING: the @p map will be a boson 0.8.128 map only. You need to
	 * call @ref BosonFileConverter::convertMapFile_From_0_8_128_To_0_9 on
	 * it!
	 **/
	bool convert(int* groundTypes, QByteArray* map, QByteArray* texMap);

protected:
	/**
	 * Convert the groundTypes from an old boson 0.8 file to the format used
	 * by 0.9
	 *
	 * The number of used textures is hardcoded to 3 (grass, desert, water).
	 **/
	bool convertToTexMap_From_0_8_To_0_9(int* groundTypes, unsigned char* texMap);

	/**
	 * See @ref BosonMap::texMapArrayPos. This is the implementation as used
	 * in boson 0.9.
	 *
	 * Before you can use it you must call @ref setMapSize, which should be
	 * done in the conver*() method.
	 **/
	unsigned int texMapArrayPos(unsigned int texture, unsigned int x, unsigned int y) const
	{
		return ((texture) * (mMapWidth + 1) * (mMapHeight + 1) + cornerArrayPos(x, y));
	}

	/**
	 * See @ref BosonMap::cornerArrayPos. This is the implementation as used
	 * in boson 0.9.
	 *
	 * Before you can use it you must call @ref setMapSize, which should be
	 * done in the conver*() method.
	 **/
	unsigned int cornerArrayPos(unsigned int x, unsigned int y) const
	{
		return x + y * (mMapWidth + 1);
	}

	/**
	 * Set the alpha value in the texmap @p texMap at the coordinates @p x / @p y
	 * for the texture @p texture to @p alpha.
	 **/
	void setTexMapAlpha(unsigned int texture, unsigned int x, unsigned int y, unsigned char alpha, unsigned char* texMap);

	/**
	 * Call @ref addGroundType for the top-left corner of cell @p
	 * groundType.
	 *
	 * @p groundType can be any valid groundType (including transitions),
	 * assuming @ref fixGroundTypes has already been called.
	 *
	 * For transitions this will analyze the kind of transition and call
	 * @ref addGroundType with the correct value.
	 **/
	void addCornerTopLeft(int groundType, int* grass, int* desert, int* water);

	/**
	 * See @ref addCornerTopLeft. This function operates on the top-right
	 * corner of the cell in @p groundType
	 **/
	void addCornerTopRight(int groundType, int* grass, int* desert, int* water);

	/**
	 * See @ref addCornerTopLeft. This function operates on the bottom-left
	 * corner of the cell in @p groundType
	 **/
	void addCornerBottomLeft(int groundType, int* grass, int* desert, int* water);

	/**
	 * See @ref addCornerTopLeft. This function operates on the bottom-right
	 * corner of the cell in @p groundType
	 **/
	void addCornerBottomRight(int groundType, int* grass, int* desert, int* water);


	/**
	 * This function should get called by addCorner*() <em>only!</em>
	 *
	 * This adds 1 to either grass,desert or water, depending on @p
	 * groundType. @p groundType must be a plain tile (see @ref isPlain),
	 * not a transition.
	 **/
	void addGroundType(int groundType, int* grass, int* desert, int* water);


	/**
	 * Implementation of Cell::isValidGround() from boson 0.8
	 **/
	bool isValidGroundType(int g) const
	{
		if (g < 0 || g >= 7 + 4 * (tilesPerTransition())) {
			return false;
		}
		return true;
	}

	/**
	 * @return The number of tiles per transition in boson <= 0.8
	 **/
	int tilesPerTransition() const
	{
		return 12 + 4 * 16;
	}

	/**
	 * Implementation of Cell::getTransRef() from boson 0.8.
	 *
	 * This will return the type of the transition (use @ref isTrans()
	 * before you call this!), i.e. GroundGrass, GroundDesert or
	 * GroundWater/GroundDeepWater.
	 **/
	int getTransRef(int groundType) const
	{
		return (groundType - 7) / tilesPerTransition();
	}

	/**
	 * Implementation of Cell::isPlain() from boson 0.8.
	 **/
	bool isPlain(int groundType) const
	{
		if (groundType < 0 || groundType >= 7) {
			return false;
		}
		return true;
	}

	/**
	 * Implementation of Cell::getTransTile() from boson 0.8.
	 *
	 * This will return the direction of the transition (use @ref isTrans()
	 * before you call this!), i.e. TransUpLeft, TransUp, ...
	 **/
	int getTransTile(int groundType) const
	{
		return (groundType - 7) % tilesPerTransition();
	}

	/**
	 * @return The from part of a transition. See Cell::from() from boson
	 * 0.8
	 **/
	int from(int groundType) const;

	/**
	 * @return The to part of a transition. See Cell::to() from boson
	 * 0.8
	 **/
	int to(int groundType) const;

	/**
	 * Fix the @p groundTypes concerning obsolete values. This will filter
	 * out things that are not used anymore - for example all
	 * GroundDeepWater occurances will be replaced by GroundWater.
	 *
	 * It will also replace the "bigtiles" by plain tiles and small
	 * transitions. (writing conversion code is hard enough already without
	 * them...)
	 *
	 * Furthermore it checks for invalid values and replaces them by
	 * GroundWater. GroundWater can be a few units only and therefore is the
	 * closest to "not-valid". (of course this should never happen anyway!)
	 **/
	void fixGroundTypes(int* groundTypes);

private:
	unsigned int mMapWidth;
	unsigned int mMapHeight;
};


#endif
