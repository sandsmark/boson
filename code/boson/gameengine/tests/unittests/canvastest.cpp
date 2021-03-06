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
#include "rtti.h"
#include "boitemlist.h"
#include "unit.h"
#include "cell.h"

#include <ktempfile.h>

#include <qtextstream.h>

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
 DO_TEST(testMoveUnits());

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
 int unitType1 = 1; // UnitProperties ID
 BoVector3Fixed unit1Pos(10.0, 10.0, 0.0);
 mCanvasContainer->createNewUnitAtTopLeftPos(unitType1, unit1Pos);

 int unitType2 = unitType1;
 BoVector3Fixed unit2Pos(20.0, 10.0, 0.0);
 mCanvasContainer->createNewUnitAtTopLeftPos(unitType2, unit2Pos);

 int unitType3 = unitType1;
 BoVector3Fixed unit3Pos(20.0, 10.0, 0.0);
 mCanvasContainer->createNewUnitAtTopLeftPos(unitType3, unit3Pos);

 MY_VERIFY(mCanvasContainer->mCanvas->allItemsCount() == 3);

 QCString canvasXML = mCanvasContainer->mCanvas->saveCanvas();
 if (canvasXML.isEmpty()) {
	boError() << k_funcinfo "saving failed" << endl;
	return false;
 }

 // muse be unchanged, save must not add/remove items
 MY_VERIFY(mCanvasContainer->mCanvas->allItemsCount() == 3);

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

 BoItemList::iterator it1 = canvas1->allItems()->begin();
 BoItemList::iterator it2 = canvas2->allItems()->begin();
 for (unsigned int i = 0; i < canvas1->allItemsCount(); i++) {
	BosonItem* item1 = *it1;
	BosonItem* item2 = *it2;
	++it1;
	++it2;

	MY_VERIFY(item1->rtti() == item2->rtti());

	// AB: note: we explicitly do _NOT_ check every property, only a subset
	// of the most important ones.

	MY_VERIFY(item1->centerX() == item2->centerX());
	MY_VERIFY(item1->centerY() == item2->centerY());
	MY_VERIFY(item1->z() == item2->z());
	MY_VERIFY(item1->width() == item2->width());
	MY_VERIFY(item1->height() == item2->height());
	MY_VERIFY(item1->depth() == item2->depth());
	MY_VERIFY(item1->xVelocity() == item2->xVelocity());
	MY_VERIFY(item1->yVelocity() == item2->yVelocity());
	MY_VERIFY(item1->zVelocity() == item2->zVelocity());
	MY_VERIFY(item1->speed() == item2->speed());
	MY_VERIFY(item1->maxSpeed() == item2->maxSpeed());
	MY_VERIFY(item1->accelerationSpeed() == item2->accelerationSpeed());
	MY_VERIFY(item1->decelerationSpeed() == item2->decelerationSpeed());
	MY_VERIFY(item1->decelerationDistance() == item2->decelerationDistance());
	MY_VERIFY(item1->isVisible() == item2->isVisible());
	MY_VERIFY(item1->rotation() == item2->rotation());
	MY_VERIFY(item1->xRotation() == item2->xRotation());
	MY_VERIFY(item1->yRotation() == item2->yRotation());

	QPtrVector<Cell>* cells1 = item1->cells();
	QPtrVector<Cell>* cells2 = item2->cells();
	MY_VERIFY(cells1->size() == cells2->size());
	for (unsigned int i = 0; i < cells1->size(); i++) {
		MY_VERIFY((*cells1)[i]->x() == (*cells2)[i]->x());
		MY_VERIFY((*cells1)[i]->y() == (*cells2)[i]->y());
	}

	MY_VERIFY(RTTI::isUnit(item1->rtti()));
	Unit* unit1 = static_cast<Unit*>(item1);
	Unit* unit2 = static_cast<Unit*>(item2);

	MY_VERIFY(unit1->id() == unit2->id());
	MY_VERIFY(unit1->type() == unit2->type());
	MY_VERIFY(unit1->advanceWork() == unit2->advanceWork());
	MY_VERIFY(unit1->health() == unit2->health());
	MY_VERIFY(unit1->maxHealth() == unit2->maxHealth());
	MY_VERIFY(unit1->isDestroyed() == unit2->isDestroyed());
	MY_VERIFY(unit1->isFacility() == unit2->isFacility());
	MY_VERIFY(unit1->isMobile() == unit2->isMobile());

 }

 return true;
}

bool CanvasTest::testMoveUnits()
{
 int unitType1 = 1; // UnitProperties ID
 BoVector3Fixed unit1Pos(10.0, 10.0, 0.0);
 Unit* unit1 = mCanvasContainer->createNewUnitAtTopLeftPos(unitType1, unit1Pos);

 int unitType2 = unitType1;
 BoVector3Fixed unit2Pos(20.0, 10.0, 0.0);
 Unit* unit2 = mCanvasContainer->createNewUnitAtTopLeftPos(unitType2, unit2Pos);

 int unitType3 = unitType1;
 BoVector3Fixed unit3Pos(20.0, 10.0, 0.0);
 Unit* unit3 = mCanvasContainer->createNewUnitAtTopLeftPos(unitType3, unit3Pos);

 // TODO: leftEdge->centerX, topEdge->centerY
 MY_VERIFY(BoVector3Fixed(unit1->leftEdge(), unit1->topEdge(), unit1->z()) == unit1Pos);
 MY_VERIFY(BoVector3Fixed(unit2->leftEdge(), unit2->topEdge(), unit2->z()) == unit2Pos);
 MY_VERIFY(BoVector3Fixed(unit3->leftEdge(), unit3->topEdge(), unit3->z()) == unit3Pos);

 // TODO: cells?

 unit1->moveBy(10.0, 0.0, 0.0);
 unit2->moveBy(0.0, 10.0, 0.0);
 unit3->moveBy(10.0, 10.0, 0.0);
 unit1Pos += BoVector3Fixed(10.0, 0.0, 0.0);
 unit2Pos += BoVector3Fixed(0.0, 10.0, 0.0);
 unit3Pos += BoVector3Fixed(10.0, 10.0, 0.0);

 // TODO: leftEdge->centerX, topEdge->centerY
 MY_VERIFY(BoVector3Fixed(unit1->leftEdge(), unit1->topEdge(), unit1->z()) == unit1Pos);
 MY_VERIFY(BoVector3Fixed(unit2->leftEdge(), unit2->topEdge(), unit2->z()) == unit2Pos);
 MY_VERIFY(BoVector3Fixed(unit3->leftEdge(), unit3->topEdge(), unit3->z()) == unit3Pos);

 // TODO: cells?

 return true;
}

