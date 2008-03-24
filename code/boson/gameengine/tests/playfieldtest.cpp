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


#include "playfieldtest.h"
#include "playfieldtest.moc"

#include "testframework.h"

#include "bodebug.h"
#include "bosonplayfield.h"
#include "bosonmap.h"
#include "bosongroundtheme.h"
#include "bpfdescription.h"
#include "boglobal.h"
#include "bosondata.h"

PlayFieldTest::PlayFieldTest(QObject* parent)
	: QObject(parent)
{
 mPlayField = 0;
 mMap = 0;
}

PlayFieldTest::~PlayFieldTest()
{
 // mMap is deleted by mPlayField
 delete mPlayField;
}

void PlayFieldTest::initTest()
{
 delete mPlayField;
 mPlayField = new BosonPlayField(this);
 mMap = new BosonMap(this);

 const unsigned int groundTypeCount = 3;
 BosonGroundTheme* theme = TestFrameWork::createNewGroundTheme("dummy_theme_ID", groundTypeCount);
 BosonData::bosonData()->insertGroundTheme(new BosonGenericDataObject("dummy_file", theme->identifier(), theme));

 const unsigned int width = 100;
 const unsigned int height = 200;
 mMap->createNewMap(width, height, theme);
 mPlayField->changeMap(mMap);

 BPFDescription* description = new BPFDescription;
 description->setName("name");
 description->setComment("comment");
 mPlayField->setModifiedDescription(description);
}

void PlayFieldTest::cleanupTest()
{
 delete mPlayField;
 mPlayField = 0;
 BosonData::bosonData()->clearData();
}

bool PlayFieldTest::test()
{
 BoGlobal::initStatic();

 cleanupTest();

 DO_TEST(testCreateNewPlayField());
 DO_TEST(testSaveLoadPlayField());

 return true;
}

bool PlayFieldTest::testCreateNewPlayField()
{
 // initTest() already created a new playfield
 if (!checkIfPlayFieldIsValid(mPlayField)) {
	return false;
 }

 return true;
}

bool PlayFieldTest::testSaveLoadPlayField()
{
 QMap<QString, QByteArray> files;
 if (!mPlayField->savePlayFieldToFiles(files)) {
	boError() << k_funcinfo "saving failed" << endl;
	return false;
 }
 BosonPlayField* field2 = new BosonPlayField(this);
 if (!field2->loadPlayFieldFromFiles(files)) {
	boError() << k_funcinfo << "loading failed" << endl;
	return false;
 }
 if (!checkIfPlayFieldIsValid(field2)) {
	return false;
 }
 if (!checkIfPlayFieldsAreEqual(mPlayField, field2)) {
	return false;
 }

 // check that a loaded field can still be saved correctly.
 // the resulting map should match exactly both, mPlayField and field2
 QMap<QString, QByteArray> files2;
 if (!field2->savePlayFieldToFiles(files2)) {
	boError() << k_funcinfo << "saving of field2 failed" << endl;
	return false;
 }
 BosonPlayField* field3 = new BosonPlayField(this);
 if (!field3->loadPlayFieldFromFiles(files)) {
	boError() << k_funcinfo << "loading of field3 failed" << endl;
	return false;
 }
 if (!checkIfPlayFieldIsValid(field3)) {
	return false;
 }
 if (!checkIfPlayFieldsAreEqual(mPlayField, field3)) {
	return false;
 }
 delete field2;
 delete field3;

 return true;
}

bool PlayFieldTest::checkIfPlayFieldIsValid(BosonPlayField* field)
{
 MY_VERIFY(field->map() != 0);
 MY_VERIFY(field->map()->width() > 0);
 MY_VERIFY(field->map()->height() > 0);
 MY_VERIFY(field->description() != 0);

 return true;
}

bool PlayFieldTest::checkIfPlayFieldsAreEqual(BosonPlayField* field1, BosonPlayField* field2)
{
 MY_VERIFY(field1->map()->width() == field2->map()->width());
 MY_VERIFY(field1->map()->height() == field2->map()->height());
 MY_VERIFY(field1->map()->groundTheme() == field2->map()->groundTheme());
 MY_VERIFY(field1->description()->name() == field2->description()->name());
 MY_VERIFY(field1->description()->comment() == field2->description()->comment());

 return true;
}

