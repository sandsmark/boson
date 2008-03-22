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

#ifndef MAPTEST_H
#define MAPTEST_H

#include <qobject.h>

class BosonMap;
class BosonGroundTheme;

class MapTest : public QObject
{
	Q_OBJECT
public:
	MapTest(QObject* parent = 0);
	~MapTest();

	bool test();

protected:
	void initMap();
	void cleanupMap();
	bool testCreateNewMaps();
	bool testSaveLoadMaps();

	bool checkIfMapIsValid(BosonMap* map, unsigned int width, unsigned int height, BosonGroundTheme* theme);
	bool checkIfMapsAreEqual(BosonMap* map1, BosonMap* map2);

private:
	BosonMap* mMap;
};

#endif

