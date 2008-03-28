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


#include "testframework.h"
#include "testframework.moc"

#include "bodebug.h"
#include "bosonmap.h"
#include "bosongroundtheme.h"
#include "bosonplayfield.h"
#include "bpfdescription.h"
#include "speciestheme.h"

#include "boglobal.h"
#include "bosondata.h"

#include <ktempfile.h>
#include <ktempdir.h>
#include <qdir.h>

#include <memory> // std::auto_ptr

TestFrameWork::TestFrameWork(QObject* parent)
	: QObject(parent)
{
}

TestFrameWork::~TestFrameWork()
{
}

BosonGroundTheme* TestFrameWork::createDummyGroundTheme(const QString& identifier)
{
 // here we should create and return a ground theme.
 // it does not matter how that theme looks like (-> "dummy") and since we
 // cannot easily load one from a file in our unit tests, we simply create a new
 // one.
 return createNewGroundTheme(identifier, 3);
}

BosonGroundTheme* TestFrameWork::createNewGroundTheme(const QString& identifier, unsigned int groundTypeCount)
{
 BosonGroundTheme* theme = new BosonGroundTheme();
 {
	QPtrVector<BosonGroundType> types(groundTypeCount);
	for (unsigned int i = 0; i < groundTypeCount; i++) {
		BosonGroundType* type = new BosonGroundType();
		type->name = "dummy";
		type->textureFile = "dummy_file.jpg"; // AB: does not actually exist. we won't use it anyway.

		types.insert(i, type);
	}
	theme->applyGroundThemeConfig(identifier, types, "dummy_directory");
 }
 return theme;
}

BosonMap* TestFrameWork::createDummyMap(const QString& groundThemeId)
{
 BosonGroundTheme* theme = BosonData::bosonData()->groundTheme(groundThemeId);
 if (!theme) {
	boError() << k_funcinfo << "BosonData does not know groundThemeId " << groundThemeId << endl;
	return 0;
 }
 BosonMap* map = new BosonMap();

 const unsigned int width = 100;
 const unsigned int height = 200;
 map->createNewMap(width, height, theme);

 return map;
}

BosonPlayField* TestFrameWork::createDummyPlayField(const QString& groundThemeId)
{
 BosonMap* map = createDummyMap(groundThemeId);
 if (!map) {
	boError() << k_funcinfo << "could not create a map" << endl;
	return 0;
 }

 BPFDescription* description = new BPFDescription;
 description->setName("DummyPlayField");
 description->setComment("Dummy PlayField for testing");

 BosonPlayField* playField = new BosonPlayField();
 playField->changeMap(map);
 playField->setModifiedDescription(description);

 return playField;
}

SpeciesTheme* TestFrameWork::createAndLoadDummySpeciesTheme(const QColor& teamColor, bool neutralSpecies)
{
 const int unitCount = 3;

 KTempDir speciesDir_("/tmp/");
 speciesDir_.setAutoDelete(true); // AB: deletes the dir recursively (implemented using ::system("/bin/rm -rf"))

 QDir* speciesDir = speciesDir_.qDir();
 if (!speciesDir) {
	return 0;
 }
 std::auto_ptr<QDir> speciesDirDeleter(speciesDir);

 if (!speciesDir->mkdir("units")) {
	return false;
 }
 QDir unitsDir(speciesDir->absFilePath("units"));

 SpeciesTheme* theme = new SpeciesTheme();
 theme->setThemePath(speciesDir_.name());
 theme->setTeamColor(teamColor);

 QFile technologies(speciesDir->absFilePath("index.technologies"));
 // open && close once, to write an empty file
 if (!technologies.open(IO_WriteOnly)) {
	boError() << k_funcinfo << "could not write technologies file " << technologies.name() << endl;
	return false;
 }
 technologies.close();

 for (int i = 0; i < unitCount; i++) {
	if (!unitsDir.mkdir(QString("unit_%1").arg(i))) {
		return false;
	}
	QFile file(speciesDir->absFilePath(QString("units/unit_%1/index.unit").arg(i)));
	if (!file.open(IO_WriteOnly)) {
		return false;
	}
	QTextStream stream(&file);
	stream << "[Boson Unit]\n";
	stream << "Id=" << i+1 << "\n";
	stream << "Name=Unit " << i+1 << "\n";
	file.close();
 }

 if (!theme->loadTechnologies()) {
	return false;
 }
 if (!theme->readUnitConfigs()) {
	return false;
 }

 return theme;
}

