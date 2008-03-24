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
#include "boglobal.h"
#include "bosondata.h"

CanvasTest::CanvasTest(QObject* parent)
	: QObject(parent)
{
 mCanvas = 0;
 mPlayField = 0;
 mEventManager = 0;
}

CanvasTest::~CanvasTest()
{
 delete mCanvas;
 delete mPlayField;
 delete mEventManager;
}

bool CanvasTest::initTest()
{
 delete mCanvas;
 delete mPlayField;
 delete mEventManager;
 mPlayField = 0;
 mEventManager = 0;
 mCanvas = 0;


 mCanvas = new BosonCanvas(this);
 mEventManager = new BoEventManager(this);

 const unsigned int groundTypeCount = 3;
 BosonGroundTheme* theme = TestFrameWork::createNewGroundTheme("dummy_theme_ID", groundTypeCount);
 BosonData::bosonData()->insertGroundTheme(new BosonGenericDataObject("dummy_file", theme->identifier(), theme));

 mPlayField = TestFrameWork::createDummyPlayField("dummy_theme_ID");
 if (!mPlayField) {
	boError() << k_funcinfo << "NULL playfield created" << endl;
	return false;
 }

 if (!mCanvas->init(mEventManager)) {
	boError() << k_funcinfo << "initializing canvas failed" << endl;
	return false;
 }
 mCanvas->setMap(mPlayField->map());

 return true;
}

void CanvasTest::cleanupTest()
{
 delete mPlayField;
 delete mCanvas;
 delete mEventManager;
 mEventManager = 0;
 mPlayField = 0;
 mCanvas = 0;
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
 if (!checkIfCanvasIsValid(mCanvas)) {
	return false;
 }

 MY_VERIFY(mCanvas->allItemsCount() == 0);

 return true;
}

bool CanvasTest::testSaveLoadCanvas()
{
 QCString canvasXML = mCanvas->saveCanvas();
 if (canvasXML.isEmpty()) {
	boError() << k_funcinfo "saving failed" << endl;
	return false;
 }
 BoEventManager* eventManager2 = new BoEventManager(this);
 BosonCanvas* canvas2 = new BosonCanvas(this);
 if (!canvas2->init(eventManager2)) {
	return false;
 }
 if (!canvas2->loadCanvas(canvasXML)) {
	boError() << k_funcinfo "loading failed" << endl;
	return false;
 }
 if (!checkIfCanvasIsValid(canvas2)) {
	return false;
 }
 if (!checkIfCanvasAreEqual(mCanvas, canvas2)) {
	return false;
 }

 // check that a loaded canvas can still be saved correctly.
 // the resulting canvas should match exactly both, mCanvas and canvas2
 QCString canvasXML2 = canvas2->saveCanvas();
 if (canvasXML2.isEmpty()) {
	boError() << k_funcinfo << "saving of canvas2 failed" << endl;
	return false;
 }
 BoEventManager* eventManager3 = new BoEventManager(this);
 BosonCanvas* canvas3 = new BosonCanvas(this);
 if (!canvas3->init(eventManager3)) {
	return false;
 }
 if (!canvas3->loadCanvas(canvasXML2)) {
	return false;
 }
 if (!checkIfCanvasIsValid(canvas3)) {
	return false;
 }
 if (!checkIfCanvasAreEqual(mCanvas, canvas3)) {
	return false;
 }

 delete canvas2;
 delete canvas3;
 delete eventManager2;
 delete eventManager3;

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
// MY_VERIFY(canvas->pathFinder() != 0); // FIXME!
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

