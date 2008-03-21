/*
    This file is part of the Boson game
    Copyright (C) 2002-2006 Andreas Beckermann (b_mann@gmx.de)

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "bosonguistarting.h"
#include "bosonguistarting.moc"

#include "../bomemory/bodummymemory.h"
#include "defines.h"
#include "bosongroundthemedata.h"
#include "speciesdata.h"
#include "bosonprofiling.h"
#include "bosondata.h"
#include "bodebug.h"
#include "bowaterrenderer.h"
#include "bosonviewdata.h"
#include "gameengine/bosoncanvas.h"
#include "gameengine/boson.h"
#include "gameengine/player.h"
#include "gameengine/bosonplayfield.h"
#include "gameengine/bosonmap.h"
#include "gameengine/bosongroundtheme.h"
#include "gameengine/bosonmessageids.h"
#include "gameengine/speciestheme.h"
#include "gameengine/unit.h"
#include "gameengine/boitemlist.h"
#include "gameengine/bosoncomputerio.h"
#include "gameview/bosonlocalplayerinput.h"
#include "gameview/bosoneffectproperties.h"
#include "modelrendering/bomeshrenderermanager.h"
#include "botexture.h"

#include <klocale.h>

#include <qmap.h>

BosonGUIStarting::BosonGUIStarting(BosonStarting* starting, QObject* parent)
	: BosonStartingTaskCreator(parent)
{
 mStarting = starting;
 mFiles = 0;
}

BosonGUIStarting::~BosonGUIStarting()
{
}

QString BosonGUIStarting::creatorName() const
{
 return i18n("GUI");
}

void BosonGUIStarting::setFiles(QMap<QString, QByteArray>* files)
{
 mFiles = files;
}

bool BosonGUIStarting::createTasks(QPtrList<BosonStartingTask>* tasks)
{
 if (!mStarting) {
	BO_NULL_ERROR(mStarting);
	return false;
 }
 if (!mFiles) {
	BO_NULL_ERROR(mFiles);
	return false;
 }


 BosonStartingLoadTiles* tiles = new BosonStartingLoadTiles(i18n("Load Tiles"));
 connect(mStarting, SIGNAL(signalDestPlayField(BosonPlayField*)),
		tiles, SLOT(slotSetDestPlayField(BosonPlayField*)));
 tasks->append(tiles);

 BosonStartingLoadWater* water = new BosonStartingLoadWater(i18n("Load Water"));
 connect(mStarting, SIGNAL(signalDestPlayField(BosonPlayField*)),
		water, SLOT(slotSetDestPlayField(BosonPlayField*)));
 tasks->append(water);


 BosonStartingLoadEffects* effects = new BosonStartingLoadEffects(i18n("Load Effects"));
 tasks->append(effects);

 for (QPtrListIterator<Player> it(*boGame->gamePlayerList()); it.current(); ++it) {
	Player* p = (Player*)it.current();
	QString text;
	unsigned int index = boGame->gamePlayerList()->find(it.current());
	if (p->isActiveGamePlayer()) {
		text = i18n("Load player data of player %1 (of %2)").arg(index + 1).arg(boGame->activeGamePlayerCount());
	} else {
		text = i18n("Load player data of neutral player");
	}

	BosonStartingLoadPlayerGUIData* playerData = new BosonStartingLoadPlayerGUIData(text);
	playerData->setPlayer(p);
	tasks->append(playerData);
 }


 BosonStartingStartScenarioGUI* scenarioGUI = new BosonStartingStartScenarioGUI(i18n("Start Scenario (GUI)"));
 connect(mStarting, SIGNAL(signalCanvas(BosonCanvas*)),
		scenarioGUI, SLOT(slotSetCanvas(BosonCanvas*)));
 tasks->append(scenarioGUI);



 BosonStartingCheckIOs* ios = new BosonStartingCheckIOs(i18n("Check IOs"));
 tasks->append(ios);

 return true;
}



bool BosonStartingLoadTiles::startTask()
{
 if (!boGame) {
	boError(270) << k_funcinfo << "NULL boson object" << endl;
	return false;
 }
 if (!playField()) {
	boError(270) << k_funcinfo << "NULL playField" << endl;
	return false;
 }
 if (!playField()->map()) {
	boError(270) << k_funcinfo << "NULL map" << endl;
	return false;
 }
 if (!playField()->map()->groundTheme()) {
	boError(270) << k_funcinfo << "NULL groundtheme" << endl;
	return false;
 }
 if (!boViewData) {
	boError(270) << k_funcinfo << "NULL boViewData" << endl;
	return false;
 }
 boProfiling->push("LoadTiles");

 BoTextureManager::initStatic(); // noop if already done

 checkEvents();

 // actually load the theme, including textures.
 // AB: note: this is a noop if we started a game before (the groundtheme datas
 //           are not deleted when the game ends)
 boViewData->addGroundTheme(playField()->map()->groundTheme());

 if (!boViewData->groundThemeData(playField()->map()->groundTheme())) {
	boError() << k_funcinfo << "loading groundtheme data object failed" << endl;
	return false;
 }

 boProfiling->pop(); // LoadTiles
 return true;
}

unsigned int BosonStartingLoadTiles::taskDuration() const
{
 return 1000;
}

bool BosonStartingLoadEffects::startTask()
{
 BosonEffectPropertiesManager::initStatic();
 boEffectPropertiesManager->loadEffectProperties();
 return true;
}

unsigned int BosonStartingLoadEffects::taskDuration() const
{
 return 100;
}



bool BosonStartingLoadPlayerGUIData::startTask()
{
 boDebug(270) << k_funcinfo << endl;
 if (!boGame) {
	BO_NULL_ERROR(boGame);
	return false;
 }
 if (!player()) {
	BO_NULL_ERROR(player());
	return false;
 }
 if (!player()->speciesTheme()) {
	BO_NULL_ERROR(player()->speciesTheme());
	return false;
 }
 if (!boViewData) {
	boError(270) << k_funcinfo << "NULL boViewData" << endl;
	return false;
 }
 BosonProfiler profiler("LoadPlayerGUIData");

 BoMeshRendererManager::initStatic();

 boDebug(270) << k_funcinfo << player()->bosonId() << endl;
 // Order of calls below is very important!!! Don't change this unless you're sure you know what you're doing!!!

 boViewData->addSpeciesTheme(player()->speciesTheme());
 SpeciesData* speciesData = boViewData->speciesData(player()->speciesTheme());
 if (!speciesData) {
	BO_NULL_ERROR(speciesData);
	return false;
 }

 mDuration = 0;

 startSubTask(i18n("Actions..."));
 if (!speciesData->loadActions()) {
	boError(270) << k_funcinfo << "loading actions failed" << endl;
	return false;
 }
 mDuration = 25;
 completeSubTask(mDuration);

 startSubTask(i18n("Objects..."));
 if (!speciesData->loadObjects(player()->speciesTheme()->teamColor())) {
	boError(270) << k_funcinfo << "loading objects failed" << endl;
	return false;
 }
 mDuration = 50;
 completeSubTask(mDuration);

 if (!loadUnitDatas()) {
	return false;
 }

 // AB: atm only the sounds of the local player are required, but I believe this
 // can easily change.
 startSubTask(i18n("Sounds..."));
 if (!speciesData->loadGeneralSounds()) {
	boError(270) << k_funcinfo << "loading general sounds failed" << endl;
	return false;
 }
 mDuration = loadUnitDuration() + 50;
 completeSubTask(mDuration);


 boDebug(270) << k_funcinfo << "done" << endl;
 return true;
}

unsigned int BosonStartingLoadPlayerGUIData::taskDuration() const
{
 if (!player()) {
	return 0;
 }
 return loadUnitDuration() + 50;
}

void BosonStartingLoadPlayerGUIData::setPlayer(Player* p)
{
 mPlayer = p;
}

unsigned int BosonStartingLoadPlayerGUIData::durationBeforeUnitLoading() const
{
 return 50;
}

unsigned int BosonStartingLoadPlayerGUIData::loadUnitDuration() const
{
 // AB: it would be nicer to use unitCount * 100 or so, but we don't have
 // unitCount yet, as speciestheme isn't fully loaded yet.
 return 1500;
}

bool BosonStartingLoadPlayerGUIData::loadUnitDatas()
{
 if (!boViewData) {
	boError(270) << k_funcinfo << "NULL boViewData" << endl;
	return false;
 }
 startSubTask(i18n("Units..."));

 // AB: this is to ensure that we really are where we expect to be
 mDuration = durationBeforeUnitLoading();
 completeSubTask(mDuration);

 checkEvents();

 SpeciesData* speciesData = boViewData->speciesData(player()->speciesTheme());

 // First get all id's of units
 QValueList<unsigned long int> unitIds;
 unitIds += player()->speciesTheme()->allFacilities();
 unitIds += player()->speciesTheme()->allMobiles();
 QValueList<unsigned long int>::iterator it;
 int currentUnit = 0;
 float factor = 0.0f;
 for (it = unitIds.begin(); it != unitIds.end(); ++it, currentUnit++) {
	startSubTask(i18n("Unit %1 of %2...").arg(currentUnit).arg(unitIds.count()));

	const UnitProperties* prop = player()->speciesTheme()->unitProperties(*it);
	if (!speciesData->loadUnit(prop, player()->speciesTheme()->teamColor())) {
		boError(270) << k_funcinfo << "loading unit with ID " << *it << " failed" << endl;
		return false;
	}

	factor = ((float)currentUnit + 1) / ((float)unitIds.count());
	completeSubTask(durationBeforeUnitLoading() + (int)(loadUnitDuration() * factor));
	checkEvents();
 }

 // make sure that we are where we expect to be
 mDuration = durationBeforeUnitLoading() + loadUnitDuration();
 return true;
}

bool BosonStartingLoadWater::startTask()
{
 if (!playField()) {
	BO_NULL_ERROR(playField());
	return false;
 }
 if (!playField()->map()) {
	BO_NULL_ERROR(playField()->map());
	return false;
 }
 if (!playField()->map()->lakes()) {
	BO_NULL_ERROR(playField()->lakes());
	return false;
 }
 BoWaterRenderer::initStatic();
 boWaterRenderer->setMap(playField()->map());
 boWaterRenderer->loadNecessaryTextures();
 return true;
}

unsigned int BosonStartingLoadWater::taskDuration() const
{
 return 100;
}

bool BosonStartingStartScenarioGUI::startTask()
{
 PROFILE_METHOD
 if (!boGame) {
	BO_NULL_ERROR(boGame);
	return false;
 }
 if (!boViewData) {
	boError(270) << k_funcinfo << "NULL boViewData" << endl;
	return false;
 }

 disconnect(mCanvas, 0, boViewData, 0);
 if (boViewData->allItemContainers().count() > 0) {
	boError() << k_funcinfo << "there are already item containers left in boViewData (probably from a previous game?). this might be a significant memory leak!" << endl;
	while (boViewData->allItemContainers().count() > 0) {
		BosonItemContainer* c = (boViewData->allItemContainers()).getFirst();
		if (!c) {
			BO_NULL_ERROR(c);
			return false;
		}
		if (!c->item()) {
			BO_NULL_ERROR(c->item());
			return false;
		}
		boViewData->slotRemoveItemContainerFor(c->item());
	}
 }
 connect(mCanvas, SIGNAL(signalItemAdded(BosonItem*)),
		boViewData, SLOT(slotAddItemContainerFor(BosonItem*)));
 connect(mCanvas, SIGNAL(signalRemovedItem(BosonItem*)),
		boViewData, SLOT(slotRemoveItemContainerFor(BosonItem*)));
 for (BoItemList::iterator it = mCanvas->allItems()->begin(); it != mCanvas->allItems()->end(); ++it) {
	boViewData->slotAddItemContainerFor(*it);
 }

 return true;
}

unsigned int BosonStartingStartScenarioGUI::taskDuration() const
{
 return 500;
}


bool BosonStartingCheckIOs::startTask()
{
 PROFILE_METHOD
 if (!boGame) {
	BO_NULL_ERROR(boGame);
	return false;
 }

 if (!boGame) {
	BO_NULL_ERROR(boGame);
	return false;
 }
 for (unsigned int i = 0; i < boGame->allPlayerCount(); i++) {
	boDebug(270) << "init IO for player " << i << endl;
	Player* p = boGame->allPlayerList()->at(i);
	if (!p) {
		BO_NULL_ERROR(p);
		return false;
	}
	bool expectIO = true;
	if (p->bosonId() < 128 || p->isVirtual()) {
		expectIO = false;
	} else if (p->bosonId() >= 256) {
		// neutral players must have an IO in editor mode, but must not
		// have any IO in game mode
		if (boGame->gameMode()) {
			expectIO = false;
		} else if (p->bosonId() >= 512) {
			expectIO = false;
		}
	}
	if (!expectIO) {
		if (p->ioList()->count() > 0) {
			boError() << k_funcinfo << "non-game player has a player IO?!" << endl;
			// AB: might be useful sometimes (e.g. players that
			// watch only could have a menuinput IO or so)
			// but atm we don't allow this.
			return false;
		}
	} else {
		if (p->ioList()->count() == 0) {
			boError() << k_funcinfo << "player " << p->bosonId() << " has no IO!" << endl;
			return false;
		}
	}
 }

 return true;
}

unsigned int BosonStartingCheckIOs::taskDuration() const
{
 return 5;
}


