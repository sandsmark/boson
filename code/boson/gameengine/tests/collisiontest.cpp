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

#include <config.h>
#include "unittests/testframework.h" // FIXME: maybe move out of the unittests dir?
#include "boversion.h"
#include "bodebug.h"
#include "boglobal.h"
#include "bosondata.h"
#include "bosoncanvas.h"
#include "bosongroundtheme.h"
#include "bosonplayfield.h"
#include "bosonmap.h"
#include "bosonplayerlistmanager.h"
#include "boeventmanager.h"
#include "player.h"
#include "rtti.h"
#include "boitemlist.h"
#include "unit.h"
#include "bosoncollisions.h"

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kcomponentdata.h>
//Added by qt3to4:
#include <Q3CString>
#include <Q3PtrList>

static const char *version = BOSON_VERSION_STRING;

static bool start();
static bool createPlayers(unsigned int count, BosonPlayField*, BosonPlayerListManager*);
static bool createAndAddUnit(const BoVector3Fixed& pos, BosonCanvas* canvas, BosonPlayerListManager* playerListManager);

int main(int argc, char **argv)
{
// BoDebug::disableAreas(); // dont load bodebug.areas
 KAboutData about("bosontest",
		QByteArray(),
		ki18n("BosonTest"),
		version);
 about.addAuthor(ki18n("Andreas Beckermann"),
		ki18n("Coding & Current Maintainer"),
		"b_mann@gmx.de");

 Q3CString argv0(argv[0]);
 KCmdLineArgs::init(argc, argv, &about);

 BoGlobal::initStatic();
 BoGlobal::boGlobal()->initGlobalObjects();

 KComponentData instance(&about);

 if (!start()) {
	return 1;
 }
 return 0;

}

bool start()
{
 const unsigned int groundTypeCount = 3;
 BosonGroundTheme* theme = TestFrameWork::createNewGroundTheme("dummy_theme_ID", groundTypeCount);
 BosonData::bosonData()->insertGroundTheme(new BosonGenericDataObject("dummy_file", theme->identifier(), theme));
 BosonPlayField* playField = TestFrameWork::createDummyPlayField("dummy_theme_ID");
 if (!playField) {
	boError() << k_funcinfo << "NULL playfield created" << endl;
	return false;
 }

 BoEventManager* eventManager = new BoEventManager(0);
 BosonPlayerListManager* playerListManager = new BosonPlayerListManager(0);
 BosonCanvas* canvas = new BosonCanvas(true, 0);
 BosonCollisions* collisions = canvas->collisions();

 if (!canvas->init(playField->map(), playerListManager, eventManager)) {
	boError() << k_funcinfo << "initializing canvas failed" << endl;
	return false;
 }

 if (!createPlayers(2, playField, playerListManager)) {
	boError() << k_funcinfo << "creating players failed" << endl;
	return false;
 }


 boDebug() << "canvas initialized. playfield size: " << playField->map()->width() << "x" << playField->map()->height() << endl;

 for (int x = 0; x < 20; x++) {
	for (int y = 0; y < 20; y++) {
		bofixed xPos = 10.0 + x * 2.0;
		bofixed yPos = 10.0 + y * 2.0;
		if (xPos + 2.0 > playField->map()->width()) {
			boError() << "unit too far on the edge of the map!" << endl;
			return false;
		}
		if (yPos + 2.0 > playField->map()->height()) {
			boError() << "unit too far on the edge of the map!" << endl;
			return false;
		}
		BoVector3Fixed pos(xPos, yPos, 0.0);
		if (!createAndAddUnit(pos, canvas, playerListManager)) {
			boError() << "failed adding unit" << endl;
			return false;
		}
	}
 }

 Q3PtrList<Unit> units;
 for (BoItemList::Iterator it = canvas->allItems()->begin(); it != canvas->allItems()->end(); ++it) {
	if (RTTI::isUnit((*it)->rtti())) {
		units.append((Unit*)*it);
	}
 }

 boDebug() << "added units. unitcount=" << units.count() << endl;

 const unsigned int maxIterations = 40;
 unsigned int iteration = 1;
 for (iteration = 1; iteration < maxIterations; iteration++) {

	// move the units, so that we don't have some kind of collision cache or
	// so around!
	// (e.g. in the form of a "cells" cache)
	for (Q3PtrListIterator<Unit> it(units); it.current(); ++it) {
		Unit* u = it.current();
		if ((iteration % 2) == 0) {
			u->moveBy(1.0, 0.0, 0.0);
		} else {
			u->moveBy(-1.0, 0.0, 0.0);
		}
	}

	// we assume one collision check _for_every_unit_ in every step!
	for (Q3PtrListIterator<Unit> it(units); it.current(); ++it) {
		Unit* u = it.current();

		// AB: Unit::unitsInRange() is called once per advanceIdle()
		//     call.
		//
		//     unitsInRange() uses collisionsAtCells(), which sucks
		//     (very imprecise), but should be pretty fast.
		//     so that is what we are testing here, collisionsAtCells()
		//     should be really fast.
		quint32 range = 3; // FIXME: what would be a realistic value? (check unit configs)
		BoRect2Fixed rect(u->leftEdge() - range, u->topEdge() - range, u->rightEdge() + range, u->bottomEdge() + range);
		const bool exact = false;
		collisions->collisionsAtCells(rect, u, exact);
	}

 }

 boDebug() << "completed iterations: " << iteration << endl;

 return true;
}


bool createPlayers(unsigned int count, BosonPlayField* playField, BosonPlayerListManager* playerListManager)
{
 QList<KPlayer*> players;
 for (unsigned int i = 0; i < count; i++) {
	SpeciesTheme* theme = TestFrameWork::createAndLoadDummySpeciesTheme(QColor(i * 10, 0, 0));
	if (!theme) {
		boError() << k_funcinfo << "creating a speciestheme failed" << endl;
		return false;
	}

	Player* p = new Player();
	p->setUserId(128 + i);
	p->setSpeciesTheme(theme);
	p->initMap(playField->map());
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
 neutralPlayer->initMap(playField->map());
 // AB: do we need a neutralPlayer->loadFromXML()?

 playerListManager->recalculatePlayerLists(players);

 return true;
}

bool createAndAddUnit(const BoVector3Fixed& pos, BosonCanvas* canvas, BosonPlayerListManager* playerListManager)
{
 const int unitType = 1; // UnitProperties ID
 canvas->createNewItemAtTopLeftPos(RTTI::UnitStart + unitType,
		playerListManager->gamePlayerList().first(),
		ItemType(unitType),
		pos);

 return true;
}

