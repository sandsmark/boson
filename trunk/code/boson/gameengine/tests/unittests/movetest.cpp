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

#include "movetest.h"
#include "movetest.moc"

#include "testframework.h"

#include "bodebug.h"
#include "bosonplayfield.h"
#include "bosoncanvas.h"
#include "bosongroundtheme.h"
#include "bpfdescription.h"
#include "bosonplayerlistmanager.h"
#include "boglobal.h"
#include "bosondata.h"
#include "unit.h"
#include "unitorder.h"
#include "cell.h"
#include "bo3dtools.h"

// FIXME: is "MoveTest" a good name? it suggests BosonItem::move() is being
//        tested, but we test advanceMove() primarily here

MoveTest::MoveTest(QObject* parent)
	: QObject(parent)
{
 mCanvasContainer = 0;
}

MoveTest::~MoveTest()
{
 delete mCanvasContainer;
}

bool MoveTest::initTest()
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

void MoveTest::cleanupTest()
{
 delete mCanvasContainer;
 mCanvasContainer = 0;

 BosonData::bosonData()->clearData();
}

bool MoveTest::test()
{
 BoGlobal::initStatic();

 cleanupTest();

 DO_TEST(testMove());

 return true;
}

bool MoveTest::testMove()
{
 boDebug() << k_funcinfo << endl;
 int unitType1 = 1; // UnitProperties ID

 // unit moves from A to B without an obstacle of any kind (other unit or
 // terrain) in the way.
 BoVector3Fixed unitMoveNoObstaclesPos(10.0, 10.0, 0.0);
 BoVector2Fixed unitMoveNoObstaclesDest = BoVector2Fixed(unitMoveNoObstaclesPos.x(), unitMoveNoObstaclesPos.y())
		+ BoVector2Fixed(10.0, 10.0);
 Unit* unitMoveNoObstacles = static_cast<Unit*>(mCanvasContainer->mCanvas->createNewItemAtTopLeftPos(RTTI::UnitStart + unitType1,
		mCanvasContainer->mPlayerListManager->gamePlayerList().getFirst(),
		ItemType(unitType1),
		unitMoveNoObstaclesPos));

 // unit moves from A to B with a unit in the way, which can easily be evaded.
 BoVector3Fixed unitMoveWithObstaclePos(50.0, 50.0, 0.0);
 BoVector2Fixed unitMoveWithObstacleDest = BoVector2Fixed(unitMoveWithObstaclePos.x(), unitMoveWithObstaclePos.y())
		+ BoVector2Fixed(10.0, 0.0);
 BoVector3Fixed unitMoveIsObstaclePos(55.0, 50.0, 0.0);
 Unit* unitMoveWithObstacle = static_cast<Unit*>(mCanvasContainer->mCanvas->createNewItemAtTopLeftPos(RTTI::UnitStart + unitType1,
		mCanvasContainer->mPlayerListManager->gamePlayerList().getFirst(),
		ItemType(unitType1),
		unitMoveWithObstaclePos));
 Unit* unitMoveIsObstacle = static_cast<Unit*>(mCanvasContainer->mCanvas->createNewItemAtTopLeftPos(RTTI::UnitStart + unitType1,
		mCanvasContainer->mPlayerListManager->gamePlayerList().getFirst(),
		ItemType(unitType1),
		unitMoveIsObstaclePos));
 Q_UNUSED(unitMoveIsObstacle);

 bool ok;
boDebug() << debugStringVector(unitMoveNoObstaclesDest) << endl;
 ok = unitMoveNoObstacles->replaceToplevelOrders(new UnitMoveOrder(unitMoveNoObstaclesDest));
 MY_VERIFY(ok == true);
boDebug() << debugStringVector(unitMoveWithObstaclePos) << endl;
 ok = unitMoveWithObstacle->replaceToplevelOrders(new UnitMoveOrder(unitMoveWithObstacleDest));
 MY_VERIFY(ok == true);

 unsigned int estimatedMovingTime = 200;

 unsigned int advanceCallsCount = 0;
 while (advanceCallsCount < (estimatedMovingTime * 1.2)) {
	mCanvasContainer->mCanvas->setAdvanceFlag(!mCanvasContainer->mCanvas->advanceFlag());
	mCanvasContainer->mCanvas->slotAdvance(advanceCallsCount);
	advanceCallsCount++;

	// TODO: check if "unitMoveIsObstacle" is actually evaded, i.e. that
	// unitMoveIsObstacle and unitMoveWithObstacle do not collide with each
	// other
 }

 // FIXME: _much_ more precise checking!
 MY_VERIFY(fabsf(unitMoveNoObstacles->centerX() - unitMoveNoObstaclesDest.x()) <= 1.1);
 MY_VERIFY(fabsf(unitMoveNoObstacles->centerY() - unitMoveNoObstaclesDest.y()) <= 1.1);
 MY_VERIFY(fabsf(unitMoveWithObstacle->centerX() - unitMoveWithObstacleDest.x()) <= 1.1);
 MY_VERIFY(fabsf(unitMoveWithObstacle->centerY() - unitMoveWithObstacleDest.y()) <= 1.1);


 // TODO: the UnitMoveOrder should cause a UnitTurnOrder as suborder. check for
 //       that!
 //       -> check for successful completion, i.e. unit is faced towards
 //          destination point
 // TODO: check that the UnitMoveOrder has been removed once completed (i.e. no
 //       currentorder set)

 return true;
}

