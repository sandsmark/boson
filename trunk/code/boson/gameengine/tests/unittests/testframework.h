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

class Boson;
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

/**
 * This class provides functions used by various tests.
 *
 * It is intended to be used by both, the unit tests and "misc" test
 * applications.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
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

	/**
	 * Create and load a @ref SpeciesTheme suitable for testing.
	 *
	 * The theme has several different unittypes, at least the following are
	 * guaranteed to be previded:
	 * @li Id=1: mobile ground unit
	 * @li Id=2: facility with the default number of construction steps
	 **/
	static SpeciesTheme* createAndLoadDummySpeciesTheme(const QColor& teamColor, bool neutralSpecies = false);
};

// AB: note: we _need_ a new map/playfield for every new canvas, as the canvas
//     is allowed to (and will) modify the map
//     -> at the very least: the pathfinder will add colormaps
/**
 * This class provides a @ref BosonCanvas object and all objects required for
 * it, such as a @ref BosonPlayField. @ref TestFrameWork is used to create/load
 * these objects, unless straight-forward initializations are possible.
 *
 * A few @ref Player objects are also provided by this class.
 *
 * Note that this class does @em not provide all objects required to use all
 * @ref BosonCanvas features - in particular no @ref Boson object and no Qt
 * event loop and no @ref QApplication object is required by this class. However
 * some @ref BosonCanvas features may depend on them and they may crash if you
 * try to use them.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
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

public:
	/**
	 * @internal
	 **/
	static bool createPlayers(unsigned int count, BosonPlayerListManager* playerListManager, BosonPlayField* playField);
};

class BosonContainer
{
public:
	BosonContainer();
	~BosonContainer();

	// also creates players for the canvas!
	bool createBoson(const QString& groundThemeId);

public:
	Boson* mBoson;
	BosonPlayField* mPlayField;
	BosonCanvas* mCanvas;
	BosonPlayerListManager* mPlayerListManager;

protected:
	bool createPlayers(unsigned int count);
};

#endif

