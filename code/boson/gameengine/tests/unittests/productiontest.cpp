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

#include "productiontest.h"
#include "productiontest.moc"

#include "testframework.h"

#include "bodebug.h"
#include "bosonplayfield.h"
#include "bosoncanvas.h"
#include "bosongroundtheme.h"
#include "bosonplayerlistmanager.h"
#include "boglobal.h"
#include "bosondata.h"
#include "unit.h"
#include "bo3dtools.h"
#include "unitplugins/productionplugin.h"


 // TODO: check if events are sent
 // * when a production is started ("StartProductionOfUnitWithType")
 // * when a production is paused ("PauseProductionOfUnitWithType")
 // * when a production is resumed ("ContinueProductionOfUnitWithType")
 // * when a production is aborted ("StopProductionOfUnitWithType")
 // * when a production is completed ("UnitWithTypeProduced")
 // * when a (mobile unit) production is placed ("ProductionUnitWithTypePlaced")

 // TODO: test abortProduction() at various places in the queue.
 //       e.g. if unittypes "1,1,1,2,3,4,1,2" are in the queue and unittype "2"
 //       is aborted, the resultung queue should be "1,1,1,3,4,1,2", i.e. the
 //       same with the first "2" being removed.
 //       -> in particular the production must not pause the current production.


static void advanceCanvas(BosonCanvas* canvas, unsigned int* advanceCallsCount)
{
 canvas->setAdvanceFlag(!canvas->advanceFlag());
 canvas->slotAdvance(*advanceCallsCount);
 (*advanceCallsCount)++;
}

ProductionTest::ProductionTest(QObject* parent)
	: QObject(parent)
{
 mBosonContainer = 0;
}

ProductionTest::~ProductionTest()
{
 delete mBosonContainer;
}

bool ProductionTest::initTest()
{
 delete mBosonContainer;
 mBosonContainer = 0;

 const unsigned int groundTypeCount = 3;
 BosonGroundTheme* theme = TestFrameWork::createNewGroundTheme("dummy_theme_ID", groundTypeCount);
 BosonData::bosonData()->insertGroundTheme(new BosonGenericDataObject("dummy_file", theme->identifier(), theme));

 mBosonContainer = new BosonContainer();
 if (!mBosonContainer->createBoson("dummy_theme_ID")) {
	return false;
 }
 mBosonContainer->mCanvas->loadCanvas(BosonCanvas::emptyCanvasFile(0));

 unsigned int unitTypePowerPlant = 5;

 // power plant (otherwise production may be very slow or wont even start)
 mBosonContainer->createNewUnitAtTopLeftPos(unitTypePowerPlant, BoVector3Fixed(30.0, 10.0, 0.0));

 return true;
}

void ProductionTest::cleanupTest()
{
 delete mBosonContainer;
 mBosonContainer = 0;

 BosonData::bosonData()->clearData();
}

bool ProductionTest::test()
{
 BoGlobal::initStatic();

 cleanupTest();

 DO_TEST(testProductionMobileUnits());
 DO_TEST(testPauseResumeAbortProduction());
 DO_TEST(testProductionQueue());

 return true;
}

bool ProductionTest::testProductionMobileUnits()
{
 unsigned int unitTypeFactoryMobile = 3;
 unsigned int unitTypeMobile = 1;

 Unit* factory = mBosonContainer->createNewUnitAtTopLeftPos(unitTypeFactoryMobile, BoVector3Fixed(10.0, 10.0, 0.0));

 MY_VERIFY(factory->construction()->isConstructionComplete() == true);
 ProductionPlugin* production = static_cast<ProductionPlugin*>(factory->plugin(UnitPlugin::Production));
 MY_VERIFY(production != 0);
 MY_VERIFY(production->hasProduction() == false);
 MY_VERIFY(production->currentProductionId() == 0);
 MY_VERIFY(production->productionProgress() == 0.0);

 MY_VERIFY(production->canCurrentlyProduceUnit(unitTypeMobile) == true);

 production->addProduction(ProduceUnit, unitTypeMobile);

 MY_VERIFY(production->hasProduction() == true);
 MY_VERIFY(production->currentProductionId() == unitTypeMobile);
 MY_VERIFY(production->productionProgress() == 0.0);


 double previousProgress = 0.0;
 bool willBePlaced = false; // auto-placing should occur in the next call

 unsigned int allItemsCount = mBosonContainer->mCanvas->allItemsCount();

 unsigned int maximalAdvanceCalls = 20000;
 unsigned int advanceCallsCount = 0;
 while (advanceCallsCount < maximalAdvanceCalls && production->hasProduction()) {
	advanceCanvas(mBosonContainer->mCanvas, &advanceCallsCount);

	MY_VERIFY(production->completedProductionId() == unitTypeMobile || production->completedProductionId() == 0);
	if (willBePlaced == false) {
		MY_VERIFY(mBosonContainer->mCanvas->allItemsCount() == allItemsCount);
		MY_VERIFY(production->hasProduction() == true);
		MY_VERIFY(production->currentProductionId() == unitTypeMobile);
		MY_VERIFY(production->productionProgress() >= previousProgress);
		MY_VERIFY(production->productionProgress() <= 100.0);
		previousProgress = production->productionProgress();

		// we reached 100% production progress, the unit is completed.
		// mobile units will be auto-placed. auto-placement must occur
		// in the next advance call.
		if (production->completedProductionId() == unitTypeMobile) {
			MY_VERIFY(production->completedProductionType() == ProduceUnit);
			MY_VERIFY(production->productionProgress() == 100.0);
			willBePlaced = true;
		}
	} else {
		// unit has been placed in this advance call
		break;
	}
 }
 // the unit should have been placed in the last advance call.
 // production list should be empty now.
 MY_VERIFY(production->hasProduction() == false);
 MY_VERIFY(production->currentProductionId() == 0);
 MY_VERIFY(production->productionProgress() == 0.0);

 // we should now have another unit on the canvas
 // TODO: somehow check if the new unit has the correct unittype ID.
 //       probably we should do that by looking at the event that
 //       was sent ("ProductionUnitWithTypePlaced")
 MY_VERIFY(mBosonContainer->mCanvas->allItemsCount() == allItemsCount + 1);

 return true;
}

bool ProductionTest::testPauseResumeAbortProduction()
{
 unsigned int unitTypeFactoryMobile = 3;
 unsigned int unitTypeMobile = 1;

 Unit* factory = mBosonContainer->createNewUnitAtTopLeftPos(unitTypeFactoryMobile, BoVector3Fixed(10.0, 10.0, 0.0));

 MY_VERIFY(factory->construction()->isConstructionComplete() == true);
 ProductionPlugin* production = static_cast<ProductionPlugin*>(factory->plugin(UnitPlugin::Production));
 MY_VERIFY(production != 0);
 MY_VERIFY(production->hasProduction() == false);
 MY_VERIFY(production->currentProductionId() == 0);
 MY_VERIFY(production->productionProgress() == 0.0);

 MY_VERIFY(production->canCurrentlyProduceUnit(unitTypeMobile) == true);

 production->addProduction(ProduceUnit, unitTypeMobile);

 MY_VERIFY(production->hasProduction() == true);
 MY_VERIFY(production->currentProductionId() == unitTypeMobile);
 MY_VERIFY(production->productionProgress() == 0.0);

 unsigned int advanceCallsCount = 0;

 double previousProgress = 0.0;
 unsigned int allItemsCount = mBosonContainer->mCanvas->allItemsCount();

 advanceCanvas(mBosonContainer->mCanvas, &advanceCallsCount);

 MY_VERIFY(production->completedProductionId() == unitTypeMobile || production->completedProductionId() == 0);
 MY_VERIFY(mBosonContainer->mCanvas->allItemsCount() == allItemsCount);
 MY_VERIFY(production->hasProduction() == true);
 MY_VERIFY(production->currentProductionId() == unitTypeMobile);
 MY_VERIFY(production->productionProgress() >= previousProgress);
 MY_VERIFY(production->productionProgress() <= 100.0);

 previousProgress = production->productionProgress();

 // TODO: should send an event. check if it this actually was sent.
 production->pauseProduction();

 // a call to pauseProduction() should not change the production (only advance
 // calls do!)
 // -> this is ensured below: advanceCalls must not change the production either
 //    when production is paused.

 // additional advance calls should not influence the production in any way.
 advanceCanvas(mBosonContainer->mCanvas, &advanceCallsCount);
 advanceCanvas(mBosonContainer->mCanvas, &advanceCallsCount);
 advanceCanvas(mBosonContainer->mCanvas, &advanceCallsCount);

 MY_VERIFY(production->completedProductionId() == unitTypeMobile || production->completedProductionId() == 0);
 MY_VERIFY(mBosonContainer->mCanvas->allItemsCount() == allItemsCount);
 MY_VERIFY(production->hasProduction() == true);
 MY_VERIFY(production->currentProductionId() == unitTypeMobile);
 MY_VERIFY(production->productionProgress() == previousProgress);

 // TODO: should send an event. check if it this actually was sent.
 production->unpauseProduction();

 // a call to unpauseProduction() should not change the production (only advance
 // calls do!)

 MY_VERIFY(production->completedProductionId() == unitTypeMobile || production->completedProductionId() == 0);
 MY_VERIFY(mBosonContainer->mCanvas->allItemsCount() == allItemsCount);
 MY_VERIFY(production->hasProduction() == true);
 MY_VERIFY(production->currentProductionId() == unitTypeMobile);
 MY_VERIFY(production->productionProgress() == previousProgress);

 // make sure that after unpausing, we can complete production successfully.

 unsigned int maximalAdvanceCalls = 20000;
 bool willBePlaced = false;
 while (advanceCallsCount < maximalAdvanceCalls && production->hasProduction()) {
	advanceCanvas(mBosonContainer->mCanvas, &advanceCallsCount);

	MY_VERIFY(production->completedProductionId() == unitTypeMobile || production->completedProductionId() == 0);
	if (willBePlaced == false) {
		MY_VERIFY(mBosonContainer->mCanvas->allItemsCount() == allItemsCount);
		MY_VERIFY(production->hasProduction() == true);
		MY_VERIFY(production->currentProductionId() == unitTypeMobile);
		MY_VERIFY(production->productionProgress() >= previousProgress);
		MY_VERIFY(production->productionProgress() <= 100.0);
		previousProgress = production->productionProgress();

		// we reached 100% production progress, the unit is completed.
		// mobile units will be auto-placed. auto-placement must occur
		// in the next advance call.
		if (production->completedProductionId() == unitTypeMobile) {
			MY_VERIFY(production->completedProductionType() == ProduceUnit);
			MY_VERIFY(production->productionProgress() == 100.0);
			willBePlaced = true;
		}
	} else {
		// unit has been placed in this advance call
		break;
	}
 }
 // the unit should have been placed in the last advance call.
 // production list should be empty now.
 MY_VERIFY(production->hasProduction() == false);
 MY_VERIFY(production->currentProductionId() == 0);
 MY_VERIFY(production->productionProgress() == 0.0);

 // we should now have another unit on the canvas
 // TODO: somehow check if the new unit has the correct unittype ID.
 //       probably we should do that by looking at the event that
 //       was sent ("ProductionUnitWithTypePlaced")
 MY_VERIFY(mBosonContainer->mCanvas->allItemsCount() == allItemsCount + 1);

 /////////////////////////////////////////////////////////////////////////
 // add another production and abort production shortly after
 // -> abortProduction() test.
 /////////////////////////////////////////////////////////////////////////

 previousProgress = production->productionProgress();

 MY_VERIFY(production != 0);
 MY_VERIFY(production->hasProduction() == false);
 MY_VERIFY(production->currentProductionId() == 0);
 MY_VERIFY(production->productionProgress() == 0.0);
 MY_VERIFY(production->canCurrentlyProduceUnit(unitTypeMobile) == true);

 production->addProduction(ProduceUnit, unitTypeMobile);

 MY_VERIFY(production->hasProduction() == true);
 MY_VERIFY(production->currentProductionId() == unitTypeMobile);
 MY_VERIFY(production->productionProgress() == 0.0);

 advanceCanvas(mBosonContainer->mCanvas, &advanceCallsCount);

 MY_VERIFY(production->completedProductionId() == unitTypeMobile || production->completedProductionId() == 0);
 MY_VERIFY(mBosonContainer->mCanvas->allItemsCount() == allItemsCount + 1);
 MY_VERIFY(production->hasProduction() == true);
 MY_VERIFY(production->currentProductionId() == unitTypeMobile);
 MY_VERIFY(production->productionProgress() >= previousProgress);
 MY_VERIFY(production->productionProgress() <= 100.0);

 production->abortProduction(production->currentProductionType(), production->currentProductionId());

 MY_VERIFY(production->completedProductionId() == 0);
 MY_VERIFY(mBosonContainer->mCanvas->allItemsCount() == allItemsCount + 1);
 MY_VERIFY(production->hasProduction() == false);
 MY_VERIFY(production->currentProductionId() == 0);
 MY_VERIFY(production->productionProgress() == 0.0);


 return true;
}

bool ProductionTest::testProductionQueue()
{
 unsigned int unitTypeFactoryMobile = 3;
 unsigned int unitTypeMobile = 1;

 Unit* factory = mBosonContainer->createNewUnitAtTopLeftPos(unitTypeFactoryMobile, BoVector3Fixed(10.0, 10.0, 0.0));

 MY_VERIFY(factory->construction()->isConstructionComplete() == true);
 ProductionPlugin* production = static_cast<ProductionPlugin*>(factory->plugin(UnitPlugin::Production));
 MY_VERIFY(production != 0);
 MY_VERIFY(production->hasProduction() == false);
 MY_VERIFY(production->currentProductionId() == 0);
 MY_VERIFY(production->productionProgress() == 0.0);

 MY_VERIFY(production->canCurrentlyProduceUnit(unitTypeMobile) == true);

 production->addProduction(ProduceUnit, unitTypeMobile);
 production->addProduction(ProduceUnit, unitTypeMobile);

 MY_VERIFY(production->hasProduction() == true);
 MY_VERIFY(production->currentProductionId() == unitTypeMobile);
 MY_VERIFY(production->productionProgress() == 0.0);


 double previousProgress = 0.0;
 bool willBePlaced = false; // auto-placing should occur in the next call

 unsigned int allItemsCount = mBosonContainer->mCanvas->allItemsCount();

 unsigned int maximalAdvanceCalls = 20000;
 unsigned int advanceCallsCount = 0;
 while (advanceCallsCount < maximalAdvanceCalls && production->hasProduction()) {
	advanceCanvas(mBosonContainer->mCanvas, &advanceCallsCount);

	MY_VERIFY(production->completedProductionId() == unitTypeMobile || production->completedProductionId() == 0);
	if (willBePlaced == false) {
		MY_VERIFY(mBosonContainer->mCanvas->allItemsCount() == allItemsCount);
		MY_VERIFY(production->hasProduction() == true);
		MY_VERIFY(production->currentProductionId() == unitTypeMobile);
		MY_VERIFY(production->productionProgress() >= previousProgress);
		MY_VERIFY(production->productionProgress() <= 100.0);
		previousProgress = production->productionProgress();

		// we reached 100% production progress, the unit is completed.
		// mobile units will be auto-placed. auto-placement must occur
		// in the next advance call.
		if (production->completedProductionId() == unitTypeMobile) {
			MY_VERIFY(production->completedProductionType() == ProduceUnit);
			MY_VERIFY(production->productionProgress() == 100.0);
			willBePlaced = true;
		}
	} else {
		// unit has been placed in this advance call
		break;
	}
 }

 // we should now have another unit on the canvas
 // TODO: somehow check if the new unit has the correct unittype ID.
 //       probably we should do that by looking at the event that
 //       was sent ("ProductionUnitWithTypePlaced")
 MY_VERIFY(mBosonContainer->mCanvas->allItemsCount() == allItemsCount + 1);

 MY_VERIFY(production->hasProduction() == true);
 MY_VERIFY(production->currentProductionId() == unitTypeMobile);
 MY_VERIFY(production->productionProgress() == 0.0);

 willBePlaced = false;
 previousProgress = 0.0;
 while (advanceCallsCount < maximalAdvanceCalls && production->hasProduction()) {
	advanceCanvas(mBosonContainer->mCanvas, &advanceCallsCount);

	MY_VERIFY(production->completedProductionId() == unitTypeMobile || production->completedProductionId() == 0);
	if (willBePlaced == false) {
		MY_VERIFY(mBosonContainer->mCanvas->allItemsCount() == allItemsCount + 1);
		MY_VERIFY(production->hasProduction() == true);
		MY_VERIFY(production->currentProductionId() == unitTypeMobile);
		MY_VERIFY(production->productionProgress() >= previousProgress);
		MY_VERIFY(production->productionProgress() <= 100.0);
		previousProgress = production->productionProgress();

		// we reached 100% production progress, the unit is completed.
		// mobile units will be auto-placed. auto-placement must occur
		// in the next advance call.
		if (production->completedProductionId() == unitTypeMobile) {
			MY_VERIFY(production->completedProductionType() == ProduceUnit);
			MY_VERIFY(production->productionProgress() == 100.0);
			willBePlaced = true;
		}
	} else {
		// unit has been placed in this advance call
		break;
	}
 }

 // the unit should have been placed in the last advance call.
 // production list should be empty now.
 MY_VERIFY(production->hasProduction() == false);
 MY_VERIFY(production->currentProductionId() == 0);
 MY_VERIFY(production->productionProgress() == 0.0);

 // we should now have another unit on the canvas
 // TODO: somehow check if the new unit has the correct unittype ID.
 //       probably we should do that by looking at the event that
 //       was sent ("ProductionUnitWithTypePlaced")
 MY_VERIFY(mBosonContainer->mCanvas->allItemsCount() == allItemsCount + 2);


 return true;
}

