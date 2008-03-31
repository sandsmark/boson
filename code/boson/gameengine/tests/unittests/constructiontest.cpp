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

#include "constructiontest.h"
#include "constructiontest.moc"

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

ConstructionTest::ConstructionTest(QObject* parent)
	: QObject(parent)
{
 mBosonContainer = 0;
}

ConstructionTest::~ConstructionTest()
{
 delete mBosonContainer;
}

bool ConstructionTest::initTest()
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

 return true;
}

void ConstructionTest::cleanupTest()
{
 delete mBosonContainer;
 mBosonContainer = 0;

 BosonData::bosonData()->clearData();
}

bool ConstructionTest::test()
{
 BoGlobal::initStatic();

 cleanupTest();

 DO_TEST(testConstruction());

 return true;
}

bool ConstructionTest::testConstruction()
{
 boDebug() << k_funcinfo << endl;
 int unitType1 = 2; // UnitProperties ID

 Unit* facility = mBosonContainer->createNewUnitAtTopLeftPos(unitType1, BoVector3Fixed(10.0, 10.0, 0.0));

 MY_VERIFY(facility != 0);
 MY_VERIFY(facility->construction() != 0); // a facility must ALWAYS have such an object

 // according to the README in the species/human/units/ directory, 20 is the
 // default value. make sure the code matches the documentation.
 unsigned int totalConstructionSteps = 20;
 MY_VERIFY(facility->construction()->constructionSteps() == totalConstructionSteps);

 MY_VERIFY(facility->construction()->isConstructionComplete() == false);
 MY_VERIFY(facility->construction()->currentConstructionStep() == 0);
 MY_VERIFY(facility->advanceWork() == UnitBase::WorkConstructed);

 // AB: a construction step is made only every n advance calls. currently n==20
 const unsigned int constructionStepInterval = 20;
 unsigned int totalAdvanceCalls = (totalConstructionSteps + 1) * constructionStepInterval;

 unsigned int advanceCallsCount = 0;
 while (advanceCallsCount < totalAdvanceCalls) {
	mBosonContainer->mCanvas->setAdvanceFlag(!mBosonContainer->mCanvas->advanceFlag());
	mBosonContainer->mCanvas->slotAdvance(advanceCallsCount);
	advanceCallsCount++;

	unsigned int expectedConstructionStep = ((advanceCallsCount-1) / constructionStepInterval) + 1;
	MY_VERIFY(facility->construction()->currentConstructionStep() == expectedConstructionStep);
	if (expectedConstructionStep < totalConstructionSteps) {
		MY_VERIFY(facility->construction()->isConstructionComplete() == false);
		MY_VERIFY(facility->advanceWork() == UnitBase::WorkConstructed);
	}
	if (expectedConstructionStep == totalConstructionSteps) {
		MY_VERIFY(facility->construction()->isConstructionComplete() == true);
		break;
	}
 }

 MY_VERIFY(facility->construction()->isConstructionComplete() == true);
 MY_VERIFY(facility->currentOrder() == 0);
 MY_VERIFY(facility->advanceWork() != UnitBase::WorkConstructed);

 // TODO: when construction is completed, an event should be sent. check if
 // that even actually has been sent!


 int unitType2 = 3;
 Unit* facilityNoConstructionStep = mBosonContainer->createNewUnitAtTopLeftPos(unitType2, BoVector3Fixed(10.0, 30.0, 0.0));

 MY_VERIFY(facilityNoConstructionStep != 0);
 MY_VERIFY(facilityNoConstructionStep->construction() != 0); // a facility must ALWAYS have such an object

 // this unittype should be completely constructed once placed.
 MY_VERIFY(facilityNoConstructionStep->construction()->constructionSteps() == 0);
 MY_VERIFY(facilityNoConstructionStep->construction()->currentConstructionStep() == 0);

 MY_VERIFY(facilityNoConstructionStep->construction()->isConstructionComplete() == true);
 MY_VERIFY(facilityNoConstructionStep->advanceWork() != UnitBase::WorkConstructed);

 // an advance call should not change the construction state.
 mBosonContainer->mCanvas->setAdvanceFlag(!mBosonContainer->mCanvas->advanceFlag());
 mBosonContainer->mCanvas->slotAdvance(advanceCallsCount);
 advanceCallsCount++;

 MY_VERIFY(facilityNoConstructionStep->construction()->constructionSteps() == 0);
 MY_VERIFY(facilityNoConstructionStep->construction()->currentConstructionStep() == 0);

 MY_VERIFY(facilityNoConstructionStep->construction()->isConstructionComplete() == true);

 return true;
}

