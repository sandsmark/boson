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

#include "canvastest.h"
#include "canvastest.moc"

#include "testframework.h"

#include "bodebug.h"
#include "bosonplayfield.h"
#include "bosoncanvas.h"
#include "bosonmap.h"
#include "bosongroundtheme.h"
#include "bpfdescription.h"
#include "boeventmanager.h"
#include "bosonplayerlistmanager.h"
#include "boglobal.h"
#include "bosondata.h"
#include "speciestheme.h"
#include "unitproperties.h"
#include "player.h"

#include <ktempfile.h>

#include <qtextstream.h>

// AB: note: we _need_ a new map/playfield for every new canvas, as the canvas
//     is allowed to (and will) modify the map
//     -> at the very leas: the pathfinder will add colormaps
class CanvasContainer
{
public:
	CanvasContainer()
	{
		mPlayField = 0;
		mCanvas = 0;
		mPlayerListManager = 0;
		mEventManager = 0;
	}
	~CanvasContainer()
	{
		for (QPtrListIterator<Player> it(mPlayerListManager->allPlayerList()); it.current(); ++it) {
			delete it.current();
		}
		delete mCanvas;
		delete mPlayField;
		delete mEventManager;
		delete mPlayerListManager;
	}

	// also creates players for the canvas!
	bool createCanvas(const QString& groundThemeId);

	BoEventManager* mEventManager;
	BosonPlayerListManager* mPlayerListManager;
	BosonPlayField* mPlayField;
	BosonCanvas* mCanvas;

protected:
	bool createPlayers(unsigned int count);
};

bool CanvasContainer::createCanvas(const QString& groundThemeId)
{
 mPlayField = TestFrameWork::createDummyPlayField(groundThemeId);
 if (!mPlayField) {
	boError() << k_funcinfo << "NULL playfield created" << endl;
	return false;
 }

 mEventManager = new BoEventManager(0);
 mPlayerListManager = new BosonPlayerListManager(0);
 mCanvas = new BosonCanvas(true, 0);

 if (!mCanvas->init(mPlayField->map(), mPlayerListManager, mEventManager)) {
	boError() << k_funcinfo << "initializing canvas failed" << endl;
	return false;
 }

 if (!createPlayers(2)) {
	boError() << k_funcinfo << "creating players failed" << endl;
	return false;
 }

 return true;
}

// AB: creates count+1 players (count players + 1 neutral player)
bool CanvasContainer::createPlayers(unsigned int count)
{
 QPtrList<KPlayer> players;
 for (unsigned int i = 0; i < count; i++) {
	SpeciesTheme* theme = TestFrameWork::createAndLoadDummySpeciesTheme(QColor(i * 10, 0, 0));
	if (!theme) {
		boError() << k_funcinfo << "creating a speciestheme failed" << endl;
		return false;
	}

	Player* p = new Player();
	p->setUserId(128 + i);
	p->setSpeciesTheme(theme);
	p->initMap(mPlayField->map());
	// AB: do we need a p->loadFromXML()?
	players.append(p);
 }
 SpeciesTheme* neutralTheme = TestFrameWork::createAndLoadDummySpeciesTheme(QColor(0, 100, 0), true);
 if (!neutralTheme) {
	boError() << k_funcinfo << "creating a neutral speciestheme failed" << endl;
	return false;
 }
 Player* neutralPlayer = new Player(true);
 neutralPlayer->setUserId(256);
 neutralPlayer->setSpeciesTheme(neutralTheme);
 neutralPlayer->initMap(mPlayField->map());
 // AB: do we need a neutralPlayer->loadFromXML()?

 mPlayerListManager->recalculatePlayerLists(players);

 return true;
}

CanvasTest::CanvasTest(QObject* parent)
	: QObject(parent)
{
 mCanvasContainer = 0;
}

CanvasTest::~CanvasTest()
{
 delete mCanvasContainer;
}

bool CanvasTest::initTest()
{
 delete mCanvasContainer;
 mCanvasContainer = 0;

 const unsigned int groundTypeCount = 3;
 BosonGroundTheme* theme = TestFrameWork::createNewGroundTheme("dummy_theme_ID", groundTypeCount);
 BosonData::bosonData()->insertGroundTheme(new BosonGenericDataObject("dummy_file", theme->identifier(), theme));

 mCanvasContainer = new CanvasContainer();
 if (!mCanvasContainer->createCanvas("dummy_theme_ID")) {
	return false;
 }
 mCanvasContainer->mCanvas->loadCanvas(BosonCanvas::emptyCanvasFile(0));

 return true;
}

void CanvasTest::cleanupTest()
{
 delete mCanvasContainer;
 mCanvasContainer = 0;

 BosonData::bosonData()->clearData();
}

bool CanvasTest::test()
{
 BoGlobal::initStatic();

 cleanupTest();

 DO_TEST(testCreateNewCanvas());
 DO_TEST(testSaveLoadCanvas());

 return true;
}

bool CanvasTest::testCreateNewCanvas()
{
 // initTest() already created a new canvas
 BosonCanvas* canvas = mCanvasContainer->mCanvas;
 if (!checkIfCanvasIsValid(canvas)) {
	return false;
 }

 MY_VERIFY(canvas->allItemsCount() == 0);

 return true;
}

bool CanvasTest::testSaveLoadCanvas()
{
 QCString canvasXML = mCanvasContainer->mCanvas->saveCanvas();
 if (canvasXML.isEmpty()) {
	boError() << k_funcinfo "saving failed" << endl;
	return false;
 }

 CanvasContainer* canvasContainer2 = new CanvasContainer();
 if (!canvasContainer2->createCanvas("dummy_theme_ID")) {
	return false;
 }
 if (!canvasContainer2->mCanvas->loadCanvas(canvasXML)) {
	boError() << k_funcinfo "loading failed" << endl;
	return false;
 }
 if (!checkIfCanvasIsValid(canvasContainer2->mCanvas)) {
	return false;
 }
 if (!checkIfCanvasAreEqual(mCanvasContainer->mCanvas, canvasContainer2->mCanvas)) {
	return false;
 }

 // check that a loaded canvas can still be saved correctly.
 // the resulting canvas should match exactly both, mCanvas and canvas2
 QCString canvasXML2 = canvasContainer2->mCanvas->saveCanvas();
 if (canvasXML2.isEmpty()) {
	boError() << k_funcinfo << "saving of canvas2 failed" << endl;
	return false;
 }

 CanvasContainer* canvasContainer3 = new CanvasContainer();
 if (!canvasContainer3->createCanvas("dummy_theme_ID")) {
	return false;
 }
 if (!canvasContainer3->mCanvas->loadCanvas(canvasXML2)) {
	return false;
 }
 if (!checkIfCanvasIsValid(canvasContainer3->mCanvas)) {
	return false;
 }
 if (!checkIfCanvasAreEqual(mCanvasContainer->mCanvas, canvasContainer3->mCanvas)) {
	return false;
 }

 delete canvasContainer2;
 delete canvasContainer3;

 return true;
}

bool CanvasTest::checkIfCanvasIsValid(BosonCanvas* canvas)
{
 MY_VERIFY(canvas->map() != 0);
 MY_VERIFY(canvas->map()->width() > 0);
 MY_VERIFY(canvas->map()->height() > 0);
 MY_VERIFY(canvas->map()->heightMap() != 0);
 MY_VERIFY(canvas->eventListener() != 0);
 MY_VERIFY(canvas->canvasStatistics() != 0);
 MY_VERIFY(canvas->pathFinder() != 0);
 MY_VERIFY(canvas->collisions() != 0);
 MY_VERIFY(canvas->map()->width() == canvas->mapWidth());
 MY_VERIFY(canvas->map()->height() == canvas->mapHeight());
 MY_VERIFY(canvas->map()->heightMap() == canvas->heightMap());
 MY_VERIFY(canvas->allItems() != 0);

 return true;
}

bool CanvasTest::checkIfCanvasAreEqual(BosonCanvas* canvas1, BosonCanvas* canvas2)
{
 MY_VERIFY(canvas1->map()->width() == canvas2->map()->width());
 MY_VERIFY(canvas1->map()->height() == canvas2->map()->height());
 MY_VERIFY(canvas1->map()->groundTheme() == canvas2->map()->groundTheme());
 MY_VERIFY(canvas1->allItemsCount() == canvas2->allItemsCount());

 // TODO: check if canvasStatistics() contents match
 // TODO: check if all units are at the same position and have the same IDs
 //       (possibly more? health?)

 return true;
}

