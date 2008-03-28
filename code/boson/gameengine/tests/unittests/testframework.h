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

#ifndef TESTFRAMEWORK_H
#define TESTFRAMEWORK_H

#include <qobject.h>

class BosonMap;
class BosonPlayField;
class BosonGroundTheme;
class SpeciesTheme;
class BoEventManager;
class BosonPlayerListManager;
class BosonCanvas;

#define MY_VERIFY(x) \
	if (!(x)) { \
		boDebug() << "failed: " #x << endl; \
		return false; \
	}

#define DO_TEST(x) \
	if (!initTest()) { \
		boError() << "initTest() failed for test: " #x << endl; \
		return false; \
	}\
	if (!x) { \
		boDebug() << "test failed: " #x << endl; \
		cleanupTest(); \
		return false; \
	}; \
	cleanupTest();

class TestFrameWork : public QObject
{
	Q_OBJECT
public:
	TestFrameWork(QObject* parent = 0);
	~TestFrameWork();

	/**
	 * Create a ground theme in "some" way. This method may be implemented
	 * whichever way seems most logical, including by calling @ref
	 * createNewGroundTheme or by loading a theme from a file.
	 *
	 * By using this method, the caller explicitly states that he does not
	 * really care how the theme looks like.
	 **/
	static BosonGroundTheme* createDummyGroundTheme(const QString& identifier);

	/**
	 * Create a @em new ground theme. Contrary to @ref
	 * createDummyGroundTheme this method always creates a new theme by
	 * code, it is never loaded from a file.
	 **/
	static BosonGroundTheme* createNewGroundTheme(const QString& identifier, unsigned int groundTypeCount);

	/**
	 * Create a map based on @p groundThemeId. The theme referenced by @p
	 * groundThemeId must have been inserted to @ref BosonData already.
	 **/
	static BosonMap* createDummyMap(const QString& groundThemeId);
	static BosonPlayField* createDummyPlayField(const QString& groundThemeId);

	static SpeciesTheme* createAndLoadDummySpeciesTheme(const QColor& teamColor, bool neutralSpecies = false);
};

// AB: note: we _need_ a new map/playfield for every new canvas, as the canvas
//     is allowed to (and will) modify the map
//     -> at the very least: the pathfinder will add colormaps
class CanvasContainer
{
public:
	CanvasContainer();
	~CanvasContainer();

	// also creates players for the canvas!
	bool createCanvas(const QString& groundThemeId);

public:
	BoEventManager* mEventManager;
	BosonPlayerListManager* mPlayerListManager;
	BosonPlayField* mPlayField;
	BosonCanvas* mCanvas;

protected:
	bool createPlayers(unsigned int count);
};

#endif

