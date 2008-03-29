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
#include "player.h"
#include "bosoncanvas.h"
#include "boeventmanager.h"
#include "bosonplayerlistmanager.h"

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
	if (i == 0) {
		stream << "\n";
		stream << "[Boson Mobile Unit]\n";
		stream << "CanGoOnLand=true\n";
		stream << "Speed=2\n";
	}
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


CanvasContainer::CanvasContainer()
{
 mPlayField = 0;
 mCanvas = 0;
 mPlayerListManager = 0;
 mEventManager = 0;
}

CanvasContainer::~CanvasContainer()
{
 for (QPtrListIterator<Player> it(mPlayerListManager->allPlayerList()); it.current(); ++it) {
	delete it.current();
 }
 delete mCanvas;
 delete mPlayField;
 delete mEventManager;
 delete mPlayerListManager;
}

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

