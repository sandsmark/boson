/*
    This file is part of the Boson game
    Copyright (C) 2004 Andreas Beckermann (b_mann@gmx.de)

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "bosonmenuinput.h"
#include "bosonmenuinput.moc"

#include "bodebug.h"
#include "boufo/boufoaction.h"
#include "defines.h"
#include "bosonconfig.h"
#include "boson.h"
#include "bosondata.h"

#include <klocale.h>
#include <kshortcut.h>
#include <kgame/kplayer.h>

#include <qguardedptr.h>
#include <qptrdict.h>
#include <qsignalmapper.h>
#include <qtimer.h>


#define ID_DEBUG_KILLPLAYER 0
#define ID_DEBUG_ADD_10000_MINERALS 1
#define ID_DEBUG_ADD_1000_MINERALS 2
#define ID_DEBUG_SUB_1000_MINERALS 3
#define ID_DEBUG_ADD_10000_OIL 4
#define ID_DEBUG_ADD_1000_OIL 5
#define ID_DEBUG_SUB_1000_OIL 6



class BosonMenuInputDataPrivate
{
public:
	BosonMenuInputDataPrivate()
	{
		mActionCollection = 0;

#if 0
		mActionMenubar = 0;
#endif
		mActionDebugPlayers = 0;
		mActionFullScreen = 0;
		mActionEditorPlayer = 0;
		mActionEditorPlace = 0;

		mSelectMapper = 0;
		mCreateMapper = 0;
	}

	BoUfoActionCollection* mActionCollection;

#if 0
	QGuardedPtr<BoUfoToggleAction> mActionMenubar;
#endif
	QGuardedPtr<BoUfoActionMenu> mActionDebugPlayers;
	QGuardedPtr<BoUfoToggleAction> mActionFullScreen;
	QGuardedPtr<BoUfoSelectAction> mActionEditorPlayer;
	QGuardedPtr<BoUfoSelectAction> mActionEditorPlace;
	QGuardedPtr<BoUfoToggleAction> mActionEditorChangeHeight;
	QPtrList<KPlayer> mEditorPlayers;
	QPtrDict<KPlayer> mActionDebugPlayer2Player;

	QSignalMapper* mSelectMapper;
	QSignalMapper* mCreateMapper;
};

BosonMenuInputData::BosonMenuInputData(BoUfoActionCollection* collection)
	: QObject(collection)
{
 d = new BosonMenuInputDataPrivate;
 d->mActionCollection = collection;
}

BosonMenuInputData::~BosonMenuInputData()
{
 delete d;
}

BoUfoActionCollection* BosonMenuInputData::actionCollection() const
{
 return d->mActionCollection;
}

void BosonMenuInputData::initUfoActions(bool gameMode)
{
 BO_CHECK_NULL_RET(actionCollection());

 // TODO: help menu

 QSignalMapper* scrollMapper = new QSignalMapper(this);
 connect(scrollMapper, SIGNAL(mapped(int)), this, SIGNAL(signalScroll(int)));
 BoUfoAction* a;
 KShortcut scrollUp(Qt::Key_Up);
 scrollUp.append(KKeySequence(KKey(Qt::Key_W)));
 a = new BoUfoAction(i18n("Scroll Up"), scrollUp, scrollMapper,
		SLOT(map()), actionCollection(),
		"scroll_up");
 scrollMapper->setMapping(a, ScrollUp);
 KShortcut scrollDown(Qt::Key_Down);
 scrollDown.append(KKeySequence(KKey(Qt::Key_S)));
 a = new BoUfoAction(i18n("Scroll Down"), scrollDown, scrollMapper,
		SLOT(map()), actionCollection(),
		"scroll_down");
 scrollMapper->setMapping(a, ScrollDown);
 KShortcut scrollLeft(Qt::Key_Left);
 scrollLeft.append(KKeySequence(KKey(Qt::Key_A)));
 a = new BoUfoAction(i18n("Scroll Left"), scrollLeft, scrollMapper,
		SLOT(map()), actionCollection(),
		"scroll_left");
 scrollMapper->setMapping(a, ScrollLeft);
 KShortcut scrollRight(Qt::Key_Right);
 scrollRight.append(KKeySequence(KKey(Qt::Key_D)));
 a = new BoUfoAction(i18n("Scroll Right"), scrollRight, scrollMapper,
		SLOT(map()), actionCollection(),
		"scroll_right");
 scrollMapper->setMapping(a, ScrollRight);
 KShortcut rotateLeft(Qt::Key_Q);
 a = new BoUfoAction(i18n("Rotate Left"), rotateLeft, this,
		SIGNAL(signalRotateLeft()), actionCollection(),
		"rotate_left");
 KShortcut rotateRight(Qt::Key_E);
 a = new BoUfoAction(i18n("Rotate Right"), rotateRight, this,
		SIGNAL(signalRotateRight()), actionCollection(),
		"rotate_right");
 KShortcut zoomIn(Qt::Key_F);
 a = new BoUfoAction(i18n("Zoom In"), zoomIn, this,
		SIGNAL(signalZoomIn()), actionCollection(),
		"zoom_in");
 KShortcut zoomOut(Qt::Key_V);
 a = new BoUfoAction(i18n("Zoom out"), zoomOut, this,
		SIGNAL(signalZoomOut()), actionCollection(),
		"zoom_out");



 // Settings
// (void)BoUfoStdAction::keyBindings(this, SLOT(slotConfigureKeys()), actionCollection());
// d->mActionMenubar = BoUfoStdAction::showMenubar(this, SLOT(slotToggleMenubar()), actionCollection());
 BoUfoToggleAction* statusbar = (BoUfoToggleAction*)BoUfoStdAction::showStatusbar(this, SIGNAL(signalToggleStatusbar()), actionCollection());
 connect(statusbar, SIGNAL(signalToggled(bool)),
		this, SIGNAL(signalToggleStatusbar(bool)));
 BoUfoToggleAction* sound = new BoUfoToggleAction(i18n("Soun&d"),
		KShortcut(), this, SIGNAL(signalToggleSound()),
		actionCollection(), "options_sound");
 sound->setChecked(boConfig->sound());
 BoUfoToggleAction* music = new BoUfoToggleAction(i18n("M&usic"), KShortcut(),
		this, SIGNAL(signalToggleMusic()),
		actionCollection(), "options_music");
 music->setChecked(boConfig->music());
 (void)new BoUfoAction(i18n("Maximal entries per event..."), KShortcut(), this,
		SIGNAL(signalChangeMaxProfilingEventEntries()), actionCollection(), "options_profiling_max_event_entries");
 (void)new BoUfoAction(i18n("Maximal advance call entries..."), KShortcut(), this,
		SIGNAL(signalChangeMaxProfilingAdvanceEntries()), actionCollection(), "options_profiling_max_advance_entries");
 (void)new BoUfoAction(i18n("Maximal rendering entries..."), KShortcut(), this,
		SIGNAL(signalChangeMaxProfilingRenderingEntries()), actionCollection(), "options_profiling_max_rendering_entries");


 // Display
 d->mActionFullScreen = BoUfoStdAction::fullScreen(0,
		0, actionCollection());
 connect(d->mActionFullScreen, SIGNAL(signalToggled(bool)),
		this, SIGNAL(signalToggleFullScreen(bool)));
 d->mActionFullScreen->setChecked(false);

 // Debug
 (void)new BoUfoAction(i18n("&Profiling..."), KShortcut(), this,
		SIGNAL(signalProfiling()), actionCollection(), "debug_profiling");
 (void)new BoUfoAction(i18n("&Debug KGame..."), KShortcut(), this,
		SIGNAL(signalDebugKGame()), actionCollection(), "debug_kgame");
 (void)new BoUfoAction(i18n("Debug &BoDebug log..."), KShortcut(), this,
		SIGNAL(signalBoDebugLogDialog()), actionCollection(), "debug_bodebuglog");
 (void)new BoUfoAction(i18n("sleep() 1s"), KShortcut(), this,
		SIGNAL(signalSleep1s()), actionCollection(), "debug_sleep_1s");

 (void)new BoUfoAction(i18n("&Reset View Properties"), KShortcut(Qt::Key_R), this,
		SIGNAL(signalResetViewProperties()), actionCollection(), "game_reset_view_properties");


 (void)new BoUfoToggleAction(i18n("Show Cha&t"),
		KShortcut(Qt::CTRL+Qt::Key_C), this, SIGNAL(signalToggleChatVisible()),
		actionCollection(), "options_show_chat");

 (void)new BoUfoAction(i18n("&Grab Screenshot"), KShortcut(Qt::CTRL + Qt::Key_G),
		this, SIGNAL(signalGrabScreenshot()), actionCollection(), "game_grab_screenshot");
 (void)new BoUfoAction(i18n("Grab &Profiling data"), KShortcut(Qt::CTRL + Qt::Key_P),
		this, SIGNAL(signalGrabProfiling()), actionCollection(), "game_grab_profiling");
 BoUfoToggleAction* movie = new BoUfoToggleAction(i18n("Grab &Movie"),
		KShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_M), 0, 0, actionCollection(), "game_grab_movie");
 movie->setChecked(false);
 connect(movie, SIGNAL(signalToggled(bool)),
		this, SIGNAL(signalSetGrabMovie(bool)));

 BoUfoToggleAction* resources = new BoUfoToggleAction(i18n("Show resources"),
		KShortcut(), 0, 0, actionCollection(), "show_resources");
 resources->setChecked(true);
 connect(resources, SIGNAL(signalToggled(bool)),
		this, SIGNAL(signalSetShowResources(bool)));
 BoUfoToggleAction* mapCoordinates = new BoUfoToggleAction(i18n("Debug &map coordinates"),
		KShortcut(), 0, 0, actionCollection(), "debug_map_coordinates");
 mapCoordinates->setChecked(false);
 connect(mapCoordinates, SIGNAL(signalToggled(bool)),
		this, SIGNAL(signalSetDebugMapCoordinates(bool)));
 BoUfoToggleAction* PFData = new BoUfoToggleAction(i18n("Debug pathfinder data"),
		KShortcut(), 0, 0, actionCollection(), "debug_pf_data");
 PFData->setChecked(false);
 connect(PFData, SIGNAL(signalToggled(bool)),
		this, SIGNAL(signalSetDebugPFData(bool)));
 BoUfoToggleAction* cellGrid = new BoUfoToggleAction(i18n("Show Cell &Grid"),
		KShortcut(), 0, 0, actionCollection(), "debug_cell_grid");
 cellGrid->setChecked(false);
 connect(cellGrid, SIGNAL(signalToggled(bool)),
		this, SIGNAL(signalSetDebugShowCellGrid(bool)));
 BoUfoToggleAction* matrices = new BoUfoToggleAction(i18n("Debug Ma&trices"),
		KShortcut(), 0, 0, actionCollection(), "debug_matrices");
 matrices->setChecked(false);
 connect(matrices, SIGNAL(signalToggled(bool)),
		this, SIGNAL(signalSetDebugMatrices(bool)));
 BoUfoToggleAction* works = new BoUfoToggleAction(i18n("Debug Item works"),
		KShortcut(), 0, 0, actionCollection(), "debug_works");
 works->setChecked(false);
 connect(works, SIGNAL(signalToggled(bool)),
		this, SIGNAL(signalSetDebugItemWorks(bool)));
 BoUfoToggleAction* camera = new BoUfoToggleAction(i18n("Debug camera"),
		KShortcut(), 0, 0, actionCollection(), "debug_camera");
 camera->setChecked(false);
 connect(camera, SIGNAL(signalToggled(bool)),
		this, SIGNAL(signalSetDebugCamera(bool)));
 BoUfoToggleAction* renderCounts = new BoUfoToggleAction(i18n("Debug Rendering counts"),
		KShortcut(), 0, 0, actionCollection(), "debug_rendercounts");
 renderCounts->setChecked(false);
 connect(renderCounts, SIGNAL(signalToggled(bool)),
		this, SIGNAL(signalSetDebugRenderCounts(bool)));
 BoUfoToggleAction* cheating = new BoUfoToggleAction(i18n("Enable &Cheating"),
		KShortcut(), 0, 0, actionCollection(), "debug_enable_cheating");
 connect(cheating, SIGNAL(signalToggled(bool)), this,
		SIGNAL(signalToggleCheating(bool)));
 BoUfoToggleAction* wireFrames = new BoUfoToggleAction(i18n("Render &Wireframes"),
		KShortcut(), 0, 0, actionCollection(), "debug_wireframes");
 connect(wireFrames, SIGNAL(signalToggled(bool)), this,
		SIGNAL(signalSetDebugWireFrames(bool)));
 BoUfoToggleAction* boundingboxes = new BoUfoToggleAction(i18n("Render item's bounding boxes"),
		KShortcut(), 0, 0, actionCollection(), "debug_boundingboxes");
 connect(boundingboxes, SIGNAL(signalToggled(bool)),
		this, SIGNAL(signalSetDebugBoundingBoxes(bool)));
 BoUfoToggleAction* fps = new BoUfoToggleAction(i18n("Debug FPS"),
		KShortcut(), 0, 0, actionCollection(), "debug_fps");
 connect(fps, SIGNAL(signalToggled(bool)),
		this, SIGNAL(signalSetDebugFPS(bool)));
 BoUfoToggleAction* debugAdvanceCalls = new BoUfoToggleAction(i18n("Debug &Advance calls"),
		KShortcut(), 0, 0, actionCollection(), "debug_advance_calls");
 connect(debugAdvanceCalls, SIGNAL(signalToggled(bool)), this,
		SIGNAL(signalSetDebugAdvanceCalls(bool)));
 BoUfoToggleAction* debugTextureMemory = new BoUfoToggleAction(i18n("Debug &Texture Memory"),
		KShortcut(), 0, 0, actionCollection(), "debug_texture_memory");
 connect(debugTextureMemory, SIGNAL(signalToggled(bool)), this,
		SIGNAL(signalSetDebugTextureMemory(bool)));
 (void)new BoUfoAction(i18n("&Unfog"), KShortcut(), this,
		SIGNAL(signalUnfogAll()), actionCollection(), "debug_unfog");
 (void)new BoUfoAction(i18n("Dump game &log"), KShortcut(), this,
		SIGNAL(signalDumpGameLog()), actionCollection(), "debug_gamelog");
 BoUfoToggleAction* enableColorMap = new BoUfoToggleAction(i18n("Enable colormap"),
		KShortcut(), 0, 0, actionCollection(), "debug_colormap_enable");
 connect(enableColorMap, SIGNAL(signalToggled(bool)),
		this, SIGNAL(signalSetEnableColorMap(bool)));
 (void)new BoUfoAction(i18n("Edit global conditions..."), KShortcut(), this,
		SIGNAL(signalEditConditions()), actionCollection(),
		"debug_edit_conditions");


 BoUfoSelectAction* debugMode = new BoUfoSelectAction(i18n("Mode"), 0, 0,
		actionCollection(), "debug_mode");
 connect(debugMode, SIGNAL(signalActivated(int)), this,
		SIGNAL(signalSetDebugMode(int)));
 QStringList l;
 l.append(i18n("Normal"));
 l.append(i18n("Debug Selection"));
 debugMode->setItems(l);
 debugMode->setCurrentItem(0);

 (void)new BoUfoAction(i18n("Show OpenGL states"), KShortcut(), this,
		SIGNAL(signalShowGLStates()), actionCollection(),
		"debug_show_opengl_states");
 (void)new BoUfoAction(i18n("Reload &meshrenderer plugin"), KShortcut(), this,
		SIGNAL(signalReloadMeshRenderer()), actionCollection(),
		"debug_lazy_reload_meshrenderer");
 (void)new BoUfoAction(i18n("Reload &groundrenderer plugin"), KShortcut(), this,
		SIGNAL(signalReloadGroundRenderer()), actionCollection(),
		"debug_lazy_reload_groundrenderer");
 (void)new BoUfoAction(i18n("Light0..."), KShortcut(), this,
		SIGNAL(signalShowLight0Widget()), actionCollection(),
		"debug_light0");
 (void)new BoUfoAction(i18n("Crash boson"), KShortcut(), this,
		SIGNAL(signalCrashBoson()), actionCollection(),
		"debug_crash_boson");
#ifdef BOSON_USE_BOMEMORY
 (void)new BoUfoAction(i18n("Debug M&emory"), KShortcut(), this,
		SIGNAL(signalDebugMemory()), actionCollection(),
		"debug_memory");
#endif
 createDebugPlayersMenu();


 QStringList files;
 files.append(boData->locateDataFile("boson/topui.rc"));
 files.append(boData->locateDataFile("boson/bosonbaseui.rc"));
 if (gameMode) {
	initUfoGameActions();
	files.append(boData->locateDataFile("boson/bosonui.rc"));
 } else {
	initUfoEditorActions();
	files.append(boData->locateDataFile("boson/editorui.rc"));
 }

 actionCollection()->createGUI(files);

 resetDefaults();
}

void BosonMenuInputData::initUfoGameActions()
{
 BO_CHECK_NULL_RET(actionCollection());

 (void)BoUfoStdAction::gameQuit(this, SIGNAL(signalQuit()), actionCollection());
 (void)BoUfoStdAction::gameEnd(this, SIGNAL(signalEndGame()), actionCollection());
 (void)BoUfoStdAction::gameSave(this, SIGNAL(signalSaveGame()), actionCollection());
 (void)BoUfoStdAction::gamePause(boGame, SLOT(slotTogglePause()), actionCollection());
 (void)BoUfoStdAction::preferences(this, SIGNAL(signalPreferences()), actionCollection());
 (void)new BoUfoAction(i18n("Center &Home Base"),
		KShortcut(Qt::Key_H),
		this, SIGNAL(signalCenterHomeBase()),
		actionCollection(), "game_center_base");
 (void)new BoUfoAction(i18n("Sync Network"),
		KShortcut(),
		this, SIGNAL(signalSyncNetwork()),
		actionCollection(), "debug_sync_network");


 delete d->mSelectMapper;
 d->mSelectMapper = new QSignalMapper(this);
 delete d->mCreateMapper;
 d->mCreateMapper = new QSignalMapper(this);
 connect(d->mSelectMapper, SIGNAL(mapped(int)),
		this, SIGNAL(signalSelectSelectionGroup(int)));
 connect(d->mCreateMapper, SIGNAL(mapped(int)),
		this, SIGNAL(signalCreateSelectionGroup(int)));

 for (int i = 0; i < 10; i++) {
	BoUfoAction* a = new BoUfoAction(i18n("Select Group %1").arg(i == 0 ? 10 : i),
			Qt::Key_0 + i, d->mSelectMapper,
			SLOT(map()), actionCollection(),
			QString("select_group_%1").arg(i));
	d->mSelectMapper->setMapping(a, i);
	a = new BoUfoAction(i18n("Create Group %1").arg(i == 0 ? 10 : i),
			Qt::CTRL + Qt::Key_0 + i, d->mCreateMapper,
			SLOT(map()), actionCollection(),
			QString("create_group_%1").arg(i));
	d->mCreateMapper->setMapping(a, i);
 }

}

void BosonMenuInputData::initUfoEditorActions()
{
 BO_CHECK_NULL_RET(actionCollection());


 BoUfoStdAction::fileSaveAs(this, SIGNAL(signalEditorSavePlayFieldAs()), actionCollection(), "file_save_playfield_as");
 BoUfoAction* close = BoUfoStdAction::fileClose(this, SIGNAL(signalEndGame()), actionCollection());

 // TODO
// close->setText(i18n("&End Editor"));
 BoUfoStdAction::fileQuit(this, SIGNAL(signalQuit()), actionCollection());
 (void)BoUfoStdAction::preferences(this, SIGNAL(signalPreferences()), actionCollection());

 d->mActionEditorPlayer = new BoUfoSelectAction(i18n("&Player"), 0, 0, actionCollection(), "editor_player");
 connect(d->mActionEditorPlayer, SIGNAL(signalActivated(int)),
		this, SLOT(slotEditorChangeLocalPlayer(int)));

 QStringList list;
 list.append(i18n("&Facilities"));
 list.append(i18n("&Mobiles"));
 list.append(i18n("&Ground"));
 d->mActionEditorPlace = new BoUfoSelectAction(i18n("Place"), 0, 0, actionCollection(), "editor_place");
 connect(d->mActionEditorPlace, SIGNAL(signalActivated(int)),
		this, SLOT(slotEditorPlace(int)));
 d->mActionEditorPlace->setItems(list);

 KShortcut s;
 s.append(KKeySequence(QKeySequence(Qt::Key_Delete)));
 s.append(KKeySequence(QKeySequence(Qt::Key_D)));
 (void)new BoUfoAction(i18n("Delete selected unit"), KShortcut(s), this,
		SIGNAL(signalEditorDeleteSelectedUnits()), actionCollection(),
		"editor_delete_selected_unit");

 (void)new BoUfoAction(i18n("Map &description"), KShortcut(), this,
		SIGNAL(signalEditorEditMapDescription()), actionCollection(),
		"editor_map_description");
 (void)new BoUfoAction(i18n("Edit &Minerals"), KShortcut(), this,
		SIGNAL(slotEditorEditPlayerMinerals()), actionCollection(),
		"editor_player_minerals");
 (void)new BoUfoAction(i18n("Edit &Oil"), KShortcut(), this,
		SIGNAL(slotEditorEditPlayerOil()), actionCollection(),
		"editor_player_oil");
 d->mActionEditorChangeHeight = new BoUfoToggleAction(i18n("Edit &Height"),
		KShortcut(), this, 0, actionCollection(), "editor_height");
 connect(d->mActionEditorChangeHeight, SIGNAL(signalToggled(bool)),
		this, SIGNAL(signalEditorEditHeight(bool)));
 (void)new BoUfoAction(i18n("&Import height map"), KShortcut(), this,
		SIGNAL(signalEditorImportHeightMap()), actionCollection(),
		"editor_import_heightmap");
 (void)new BoUfoAction(i18n("&Export height map"), KShortcut(), this,
		SIGNAL(signalEditorExportHeightMap()), actionCollection(),
		"editor_export_heightmap");
 (void)new BoUfoAction(i18n("I&mport texmap"), KShortcut(), this,
		SIGNAL(signalEditorImportTexMap()), actionCollection(),
		"editor_import_texmap");
 (void)new BoUfoAction(i18n("E&xport texmap"), KShortcut(), this,
		SIGNAL(signalEditorExportTexMap()), actionCollection(),
		"editor_export_texmap");
 (void)new BoUfoAction(i18n("Edit global conditions"), KShortcut(), this,
		SIGNAL(signalEditConditions()), actionCollection(),
		"editor_edit_conditions");

// KStdAction::preferences(bosonWidget(), SLOT(slotGamePreferences()), actionCollection()); // FIXME: slotEditorPreferences()

 createEditorPlayerMenu();
 d->mActionEditorPlace->setCurrentItem(0);
 slotEditorPlace(0);
}

void BosonMenuInputData::resetDefaults()
{
 BO_CHECK_NULL_RET(actionCollection());

#warning fixme
// resetToggleAction(d->mUfoGameWidget->isChatVisible(), "options_show_chat");
 resetToggleAction(false, "debug_wireframes");
 resetToggleAction(false, "debug_boundingboxes");
 resetToggleAction(false, "debug_fps");
 resetToggleAction(false, "debug_colormap_enable");
 resetToggleAction(true, "options_show_statusbar");
 resetToggleAction(DEFAULT_CHEAT_MODE, "debug_enable_cheating");

}

void BosonMenuInputData::resetToggleAction(bool defaultValue, const QString& name)
{
 BO_CHECK_NULL_RET(actionCollection());
 BoUfoAction* a = actionCollection()->action(name);
 BO_CHECK_NULL_RET(a);
 if (!a->inherits("BoUfoToggleAction")) {
	boError() << k_funcinfo << "action " << name << " is not a BoUfoToggleAction" << endl;
	return;
 }

 BoUfoToggleAction* t = (BoUfoToggleAction*)a;
 t->setChecked(!defaultValue);
 t->slotActivated();
}

void BosonMenuInputData::createEditorPlayerMenu()
{
 BO_CHECK_NULL_RET(boGame);
 BO_CHECK_NULL_RET(d->mActionEditorPlayer);
 QPtrList<KPlayer> players = *boGame->playerList();
 QPtrListIterator<KPlayer> it(players);
 QStringList items;

 int current = -1;
 while (it.current()) {
	d->mEditorPlayers.append(it.current());
	items.append(it.current()->name());
#warning FIXME
#if 0
	if (localPlayerIO()) {
		if (it.current() == (KPlayer*)localPlayerIO()->player()) {
			current = items.count() - 1;
		}
	}
#endif
	++it;
 }
 d->mActionEditorPlayer->setItems(items);

 if (current >= 0) {
	d->mActionEditorPlayer->setCurrentItem(current);

	// AB: this causes a recursion
//	slotEditorChangeLocalPlayer(current);
 }
}

void BosonMenuInputData::createDebugPlayersMenu()
{
 // note: NOT listed in the *ui.rc files! we create it dynamically when the player enters ; not using the xml framework
 if (d->mActionDebugPlayers) {
	boError() << k_funcinfo << "menu already created" << endl;
	return;
 }
 BO_CHECK_NULL_RET(boGame);
 d->mActionDebugPlayers = new BoUfoActionMenu(i18n("Players"),
		actionCollection(), "debug_players");

 QPtrList<KPlayer> players = *boGame->playerList();
 QPtrListIterator<KPlayer> it(players);
 for (; it.current(); ++it) {
	KPlayer* player = it.current();
	BoUfoActionMenu* menu = new BoUfoActionMenu(player->name(),
			actionCollection(),
			QString("debug_players_%1").arg(player->name()));

	connect(menu, SIGNAL(signalActivated(int)),
			this, SLOT(slotDebugPlayer(int)));
	menu->insertItem(i18n("Kill Player"), ID_DEBUG_KILLPLAYER);
	menu->insertItem(i18n("Minerals += 10000"), ID_DEBUG_ADD_10000_MINERALS);
	menu->insertItem(i18n("Minerals += 1000"), ID_DEBUG_ADD_1000_MINERALS);
	menu->insertItem(i18n("Minerals -= 1000"), ID_DEBUG_SUB_1000_MINERALS);
	menu->insertItem(i18n("Oil += 10000"), ID_DEBUG_ADD_10000_OIL);
	menu->insertItem(i18n("Oil += 1000"), ID_DEBUG_ADD_1000_OIL);
	menu->insertItem(i18n("Oil -= 1000"), ID_DEBUG_SUB_1000_OIL);

	d->mActionDebugPlayers->insert(menu);
	d->mActionDebugPlayer2Player.insert(menu, player);
 }
}

void BosonMenuInputData::slotDebugPlayer(int index)
{
 BO_CHECK_NULL_RET(sender());
 if (!sender()->inherits("BoUfoActionMenu")) {
	boError() << k_funcinfo << "sender() is not a BoUfoActionMenu" << endl;
	return;
 }
 QPtrDictIterator<KPlayer> it(d->mActionDebugPlayer2Player);
 BoUfoActionMenu* menu = (BoUfoActionMenu*)sender();
 KPlayer* p = 0;
 while (it.current() && !p) {
	BoUfoActionMenu* m = (BoUfoActionMenu*)it.currentKey();
	if (m == menu) {
		p = it.current();
	}
	++it;
 }

 if (!p) {
	boError() << k_funcinfo << "player not found" << endl;
	return;
 }

 Q_UINT32 playerId = p->id();
 switch (index) {
	case ID_DEBUG_KILLPLAYER:
		emit signalDebugKillPlayer(playerId);
		break;
	case ID_DEBUG_ADD_10000_MINERALS:
		emit signalDebugModifyMinerals(playerId, 10000);
		break;
	case ID_DEBUG_ADD_1000_MINERALS:
		emit signalDebugModifyMinerals(playerId, 1000);
		break;
	case ID_DEBUG_SUB_1000_MINERALS:
		emit signalDebugModifyMinerals(playerId, -1000);
		break;
	case ID_DEBUG_ADD_1000_OIL:
		emit signalDebugModifyOil(playerId, 1000);
		break;
	case ID_DEBUG_ADD_10000_OIL:
		emit signalDebugModifyOil(playerId, 10000);
		break;
	case ID_DEBUG_SUB_1000_OIL:
		emit signalDebugModifyOil(playerId, -1000);
		break;
	default:
		boError() << k_funcinfo << "unknown index " << index << endl;
		break;
 }
}

void BosonMenuInputData::slotEditorChangeLocalPlayerHack()
{
 slotEditorChangeLocalPlayer(0);
}

void BosonMenuInputData::slotEditorChangeLocalPlayer(int index)
{
 Player* p = 0;
 p = (Player*)d->mEditorPlayers.at(index);

 static Player* senderPlayer = 0;
 if (sender() && sender()->inherits("BoUfoAction")) {

	// AB: when changing the local player we delete the whole menu and
	// recreate it.
	// however we must not do that when called while dispatching ufo events
	// (calling this slot usually results from a menu item click!).
	// therefore we delay changing the local player using a QTimer.

	boDebug() << k_funcinfo << "calling again indirectly using a QTimer..." << endl;
	senderPlayer = p;
	QTimer::singleShot(0, this, SLOT(slotEditorChangeLocalPlayerHack()));
	return;
 }
 boDebug() << k_funcinfo << "sender() is NULL - we were probably called indirectly using a QTimer - continue..." << endl;
 p = senderPlayer;
 if (p) {
	emit signalEditorChangeLocalPlayer((Player*)p);
	BO_CHECK_NULL_RET(d->mActionEditorPlace);
	if (d->mActionEditorPlace->currentItem() >= 0) {
		slotEditorPlace(d->mActionEditorPlace->currentItem());
	}
 } else {
	boWarning() << k_funcinfo << "NULL player for index " << index << endl;
 }

}

void BosonMenuInputData::slotEditorPlace(int index)
{
 boDebug() << k_funcinfo << "index: " << index << endl;
 switch (index) {
	case 0:
		emit signalEditorShowPlaceFacilities();
		break;
	case 1:
		emit signalEditorShowPlaceMobiles();
		break;
	case 2:
		emit signalEditorShowPlaceGround();
		break;
	default:
		boError() << k_funcinfo << "unknown index" << endl;
		break;
 }
}






BosonMenuInput::BosonMenuInput()
{
}

BosonMenuInput::~BosonMenuInput()
{
}

void BosonMenuInput::initIO(KPlayer*)
{
}

