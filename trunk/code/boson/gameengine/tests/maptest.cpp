/*
    This file is part of the Boson game
    Copyright (C) 2008 Andreas Beckermann (b_mann@gmx.de)

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#include "maptest.h"
#include "maptest.moc"

#include "bodebug.h"
#include "bosonmap.h"
#include "bosongroundtheme.h"
#include "cell.h"

#include "boglobal.h"
#include "bosondata.h"

#define MY_VERIFY(x) \
	if (!(x)) { \
		boDebug() << "failed: " #x << endl; \
		return false; \
	}

MapTest::MapTest(QObject* parent)
	: QObject(parent)
{
 mMap = 0;
}

MapTest::~MapTest()
{
 delete mMap;
}

void MapTest::initMap()
{
 delete mMap;
 mMap = new BosonMap(this);

 BosonGroundTheme* theme = new BosonGroundTheme();
 {
	QPtrVector<BosonGroundType> types(1);
	BosonGroundType* type1 = new BosonGroundType();
	type1->name = "dummy";
	type1->textureFile = "dummy_file.jpg"; // AB: does not actually exist. we won't use it anyway.

	types.insert(0, type1);

	theme->applyGroundThemeConfig("dummy_ID", types, "dummy_directory");
 }
 BosonData::bosonData()->insertGroundTheme(new BosonGenericDataObject("dummy_file", theme->identifier(), theme));
}

void MapTest::cleanupMap()
{
 delete mMap;
 mMap = 0;
 BosonData::bosonData()->clearData();
}

bool MapTest::test()
{
 // AB: I do NOT like this one here! we should not need this! very unclean!
 BoGlobal::initStatic();

 cleanupMap();

#define DO_TEST(x) \
	initMap(); \
	if (!x) { \
		boDebug() << "test failed: " #x << endl; \
		cleanupMap(); \
		return false; \
	}; \
	cleanupMap();


 DO_TEST(testCreateNewMaps());
 DO_TEST(testSaveLoadMaps());

 return true;
}

bool MapTest::testCreateNewMaps()
{
 const unsigned int width = 100;
 const unsigned int height = 200;

 BosonGroundTheme* theme = BosonData::bosonData()->groundTheme("dummy_ID");
 if (!theme) {
	BO_NULL_ERROR(theme);
	return false;
 }
 mMap->createNewMap(width, height, theme);
 if (!checkIfMapIsValid(mMap, width, height, theme)) {
	return false;
 }

 // TODO: check contents of
 // * heightMap (all corners should be 0.0)
 // * texMap
 // * normalMap
 // * waterDepthAtCorner()
 MY_VERIFY(mMap->lakes()->isEmpty() == true);

 MY_VERIFY(mMap->modified() == false);


 // it must be possible to save a newly created map and load it again
 QMap<QString, QByteArray> savedMap = mMap->saveMapToFiles();
 MY_VERIFY(savedMap.isEmpty() == false);
 BosonMap* copy = new BosonMap(this);
 bool success = copy->loadMapFromFiles(savedMap);
 MY_VERIFY(success == true);
 if (!checkIfMapsAreEqual(mMap, copy)) {
	return false;
 }
 delete copy;

 return true;
}

bool MapTest::testSaveLoadMaps()
{
 const unsigned int width = 100;
 const unsigned int height = 200;

 BosonGroundTheme* theme = BosonData::bosonData()->groundTheme("dummy_ID");
 if (!theme) {
	BO_NULL_ERROR(theme);
	return false;
 }
 mMap->createNewMap(width, height, theme);
 if (!checkIfMapIsValid(mMap, width, height, theme)) {
	return false;
 }

 QMap<QString, QByteArray> savedMap = mMap->saveMapToFiles();
 MY_VERIFY(savedMap.isEmpty() == false);

 BosonMap* copy = new BosonMap(this);
 bool success = copy->loadMapFromFiles(savedMap);
 MY_VERIFY(success == true);
 if (!checkIfMapIsValid(copy, width, height, theme)) {
	return false;
 }

 if (!checkIfMapsAreEqual(mMap, copy)) {
	return false;
 }

 // check that a loaded map can still be saved correctly.
 // the resulting map should match exctly both, mMap and copy
 QMap<QString, QByteArray> savedMap2 = copy->saveMapToFiles();
 BosonMap* copy2 = new BosonMap(this);
 success = copy2->loadMapFromFiles(savedMap2);
 MY_VERIFY(success == true);
 if (!checkIfMapIsValid(copy2, width, height, theme)) {
	return false;
 }
 if (!checkIfMapsAreEqual(mMap, copy2)) {
	return false;
 }


 delete copy;
 delete copy2;

 return true;
}

bool MapTest::checkIfMapIsValid(BosonMap* map, unsigned int width, unsigned int height, BosonGroundTheme* theme)
{
 MY_VERIFY(map->width() == width);
 MY_VERIFY(map->height() == height);
 MY_VERIFY(map->groundTheme() == theme);

 // ensure the cells were created correctly
 MY_VERIFY(map->cells() != 0);
 for (unsigned int x = 0; x < width; x++) {
	for (unsigned int y = 0; y < width; y++) {
		MY_VERIFY(map->cell(x, y) != 0);
		MY_VERIFY(map->cell(x, y)->x() == (int)x);
		MY_VERIFY(map->cell(x, y)->y() == (int)y);
		MY_VERIFY(map->cell(x, y) == map->cells() + map->cellArrayPos(x, y));
		MY_VERIFY(map->isValidCell(x, y) == true);
	}
 }
 MY_VERIFY(map->isValidCell(width, height) == false);
 MY_VERIFY(map->isValidCell(-1, 0) == false);
 MY_VERIFY(map->isValidCell(0, -1) == false);
 MY_VERIFY(map->isValidCell(-1, -1) == false);

 // ensure the data structures were created
 MY_VERIFY(map->heightMap() != 0);
 MY_VERIFY(map->normalMap() != 0);
 for (unsigned int i = 0; i < theme->groundTypeCount(); i++) {
	MY_VERIFY(map->texMap(i) != 0);
 }
 MY_VERIFY(map->lakes() != 0);

 return true;
}

bool MapTest::checkIfMapsAreEqual(BosonMap* map1, BosonMap* map2)
{
 MY_VERIFY(map1->width() == map2->width());
 MY_VERIFY(map1->height() == map2->height());
 MY_VERIFY(map1->groundTheme() == map2->groundTheme());

 MY_VERIFY(map1->modified() == map2->modified());
 MY_VERIFY(map1->lakes()->isEmpty() == map2->lakes()->isEmpty());

 // TODO: check contents of
 // * heightMap
 // * texMap
 // * normalMap
 // * waterDepthAtCorner()

 return true;
}

