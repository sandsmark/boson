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
#include "boufoactionext.h"
#include "defines.h"
#include "bosonconfig.h"
#include "boson.h"
#include "bosondata.h"
#include "bofullscreen.h"
#include "bosonprofiling.h"
#include "bosonprofilingdialog.h"
#include "bosonconfig.h"
#include "bodebuglogdialog.h"
#include "kgameunitdebug.h"
#include "kgameplayerdebug.h"
#include "kgameadvancemessagesdebug.h"
#include "bpfdescriptiondialog.h"
#include "boconditionwidget.h"
#include "bosonmessage.h"
#include "bocamera.h"
#include "sound/bosonaudiointerface.h"
#include "boaction.h"
#include "bosonplayfield.h"
#include "bosonmap.h"
#include "bosongroundtheme.h"
#include "optionsdialog.h"
#include "boglstatewidget.h"
#include "bogroundrenderermanager.h"
#include "bomeshrenderermanager.h"
#include "playerio.h"
#include "botexmapimportdialog.h"
#ifdef BOSON_USE_BOMEMORY
#include "bomemory/bomemorydialog.h"
#endif

#include <klocale.h>
#include <kshortcut.h>
#include <kgame/kplayer.h>
#include <kgame/kgamedebugdialog.h>
#include <kmessagebox.h>
#include <kfiledialog.h>

#include <qguardedptr.h>
#include <qptrdict.h>
#include <qsignalmapper.h>
#include <qtimer.h>
#include <qinputdialog.h>
#include <qvbox.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qvaluestack.h>
#include <qlayout.h>
#include <qdom.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlabel.h>

#include <unistd.h>


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
 sound->setChecked(boConfig->boolValue("Sound"));
 BoUfoToggleAction* music = new BoUfoToggleAction(i18n("M&usic"), KShortcut(),
		this, SIGNAL(signalToggleMusic()),
		actionCollection(), "options_music");
 music->setChecked(boConfig->boolValue("Music"));
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

 (void)new BoUfoConfigToggleAction(i18n("Show resources"),
		KShortcut(), 0, 0, actionCollection(), "show_resources");
 (void)new BoUfoConfigToggleAction(i18n("Debug &map coordinates"),
		KShortcut(), 0, 0, actionCollection(), "debug_map_coordinates");
 (void)new BoUfoConfigToggleAction(i18n("Debug pathfinder data"),
		KShortcut(), 0, 0, actionCollection(), "debug_pf_data");
 (void)new BoUfoConfigToggleAction(i18n("Show Cell &Grid"),
		KShortcut(), 0, 0, actionCollection(), "debug_cell_grid");
 (void)new BoUfoConfigToggleAction(i18n("Debug Ma&trices"),
		KShortcut(), 0, 0, actionCollection(), "debug_matrices");
 (void)new BoUfoConfigToggleAction(i18n("Debug Item works"),
		KShortcut(), 0, 0, actionCollection(), "debug_works");
 (void)new BoUfoConfigToggleAction(i18n("Debug camera"),
		KShortcut(), 0, 0, actionCollection(), "debug_camera");
 (void)new BoUfoConfigToggleAction(i18n("Debug Rendering counts"),
		KShortcut(), 0, 0, actionCollection(), "debug_rendercounts");
 BoUfoToggleAction* cheating = new BoUfoToggleAction(i18n("Enable &Cheating"),
		KShortcut(), 0, 0, actionCollection(), "debug_enable_cheating");
 connect(cheating, SIGNAL(signalToggled(bool)), this,
		SIGNAL(signalToggleCheating(bool)));
 (void)new BoUfoConfigToggleAction(i18n("Render &Wireframes"),
		KShortcut(), 0, 0, actionCollection(), "debug_wireframes");
 (void)new BoUfoConfigToggleAction(i18n("Render item's bounding boxes"),
		KShortcut(), 0, 0, actionCollection(), "debug_boundingboxes");
 (void)new BoUfoConfigToggleAction(i18n("Debug FPS"),
		KShortcut(), 0, 0, actionCollection(), "debug_fps");
 (void)new BoUfoConfigToggleAction(i18n("Debug &Advance calls"),
		KShortcut(), 0, 0, actionCollection(), "debug_advance_calls");
 (void)new BoUfoConfigToggleAction(i18n("Debug &Texture Memory"),
		KShortcut(), 0, 0, actionCollection(), "debug_texture_memory");
 (void)new BoUfoAction(i18n("&Unfog"), KShortcut(), this,
		SIGNAL(signalUnfogAll()), actionCollection(), "debug_unfog");
 (void)new BoUfoAction(i18n("Dump game &log"), KShortcut(), this,
		SIGNAL(signalDumpGameLog()), actionCollection(), "debug_gamelog");
 (void)new BoUfoConfigToggleAction(i18n("Enable colormap"),
		KShortcut(), 0, 0, actionCollection(), "debug_colormap_enable");
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
		SIGNAL(signalEditorEditPlayerMinerals()), actionCollection(),
		"editor_player_minerals");
 (void)new BoUfoAction(i18n("Edit &Oil"), KShortcut(), this,
		SIGNAL(signalEditorEditPlayerOil()), actionCollection(),
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






static QString findSaveFileName(const QString& prefix, const QString& suffix)
{
 QString file;
 for (int i = 0; i < 1000; i++) {
	file.sprintf("%s-%03d.%s", prefix.latin1(), i, suffix.latin1());
	if (!QFile::exists(file)) {
		return QFileInfo(file).absFilePath();
		return file;
	}
 }
 return QString::null;
}


BosonMenuInput::BosonMenuInput(bool gameMode)
{
 mGameMode = gameMode;
 mCamera = 0;
 mPlayerIO = 0;
 mActionCollection = 0;
 mData = 0;
}

BosonMenuInput::~BosonMenuInput()
{
 delete mData;
}

void BosonMenuInput::initIO(KPlayer* player)
{
 KGameIO::initIO(player);

 // AB: we should retrieve mPlayerIO from the player here, but we don't allow
 // including player.h in this file currently
 // (in order to avoid that someone accidentally calls methods in there that
 // should not be called here)
 // -> as a workaround we set mPlayerIO manually using setPlayerIO() after
 // construction of this object

 BO_CHECK_NULL_RET(actionCollection());

 mData = new BosonMenuInputData(actionCollection());
 mData->initUfoActions(mGameMode);

 connect(mData, SIGNAL(signalRotateLeft()),
		this, SLOT(slotRotateLeft()));
 connect(mData, SIGNAL(signalRotateRight()),
		this, SLOT(slotRotateRight()));
 connect(mData, SIGNAL(signalZoomIn()),
		this, SLOT(slotZoomOut()));
 connect(mData, SIGNAL(signalToggleStatusbar(bool)),
		this, SIGNAL(signalToggleStatusbar(bool)));
 connect(mData, SIGNAL(signalToggleSound()),
		this, SLOT(slotToggleSound()));
 connect(mData, SIGNAL(signalToggleMusic()),
		this, SLOT(slotToggleMusic()));
 connect(mData, SIGNAL(signalToggleFullScreen(bool)),
		this, SLOT(slotToggleFullScreen(bool)));
 connect(mData, SIGNAL(signalToggleChatVisible()),
		this, SIGNAL(signalToggleChatVisible()));
 connect(mData, SIGNAL(signalChangeMaxProfilingEventEntries()),
		this, SLOT(slotChangeMaxProfilingEventEntries()));
 connect(mData, SIGNAL(signalChangeMaxProfilingAdvanceEntries()),
		this, SLOT(slotChangeMaxProfilingAdvanceEntries()));
 connect(mData, SIGNAL(signalChangeMaxProfilingRenderingEntries()),
		this, SLOT(slotChangeMaxProfilingRenderingEntries()));
 connect(mData, SIGNAL(signalProfiling()),
		this, SLOT(slotProfiling()));
 connect(mData, SIGNAL(signalDebugKGame()),
		this, SLOT(slotDebugKGame()));
 connect(mData, SIGNAL(signalBoDebugLogDialog()),
		this, SLOT(slotBoDebugLogDialog()));
 connect(mData, SIGNAL(signalSleep1s()),
		this, SLOT(slotSleep1s()));
 connect(mData, SIGNAL(signalResetViewProperties()),
		this, SIGNAL(signalResetViewProperties()));
 connect(mData, SIGNAL(signalGrabScreenshot()),
		this, SLOT(slotGrabScreenshot()));
 connect(mData, SIGNAL(signalGrabProfiling()),
		this, SLOT(slotGrabProfiling()));
 connect(mData, SIGNAL(signalSetGrabMovie(bool)),
		this, SLOT(slotSetGrabMovie(bool)));
 connect(mData, SIGNAL(signalSetDebugMode(int)),
		this, SLOT(slotSetDebugMode(int)));
 connect(mData, SIGNAL(signalDebugKillPlayer(Q_UINT32)),
		this, SLOT(slotDebugKillPlayer(Q_UINT32)));
 connect(mData, SIGNAL(signalDebugModifyMinerals(Q_UINT32, int)),
		this, SLOT(slotDebugModifyMinerals(Q_UINT32, int)));
 connect(mData, SIGNAL(signalDebugModifyOil(Q_UINT32, int)),
		this, SLOT(slotDebugModifyOil(Q_UINT32, int)));
 connect(mData, SIGNAL(signalToggleCheating(bool)),
		this, SLOT(slotToggleCheating(bool)));
 connect(mData, SIGNAL(signalUnfogAll()),
		this, SLOT(slotUnfogAll()));
 connect(mData, SIGNAL(signalDumpGameLog()),
		this, SLOT(slotDumpGameLog()));
 connect(mData, SIGNAL(signalEditConditions()),
		this, SLOT(slotEditConditions()));
 connect(mData, SIGNAL(signalShowGLStates()),
		this, SLOT(slotShowGLStates()));
 connect(mData, SIGNAL(signalReloadMeshRenderer()),
		this, SLOT(slotReloadMeshRenderer()));
 connect(mData, SIGNAL(signalReloadGroundRenderer()),
		this, SLOT(slotReloadGroundRenderer()));
 connect(mData, SIGNAL(signalShowLight0Widget()),
		this, SIGNAL(signalShowLight0Widget()));
 connect(mData, SIGNAL(signalCrashBoson()),
		this, SLOT(slotCrashBoson()));
 connect(mData, SIGNAL(signalDebugMemory()),
		this, SLOT(slotDebugMemory()));
 connect(mData, SIGNAL(signalEndGame()),
		this, SIGNAL(signalEndGame()));
 connect(mData, SIGNAL(signalQuit()),
		this, SIGNAL(signalQuit()));
 connect(mData, SIGNAL(signalPreferences()),
		this, SLOT(slotPreferences()));

 // game signals
 connect(mData, SIGNAL(signalSaveGame()),
		this, SIGNAL(signalSaveGame()));
 connect(mData, SIGNAL(signalCenterHomeBase()),
		this, SLOT(slotCenterHomeBase()));
 connect(mData, SIGNAL(signalSyncNetwork()),
		this, SLOT(slotSyncNetwork()));
 connect(mData, SIGNAL(signalSelectSelectionGroup(int)),
		this, SIGNAL(signalSelectSelectionGroup(int)));
 connect(mData, SIGNAL(signalCreateSelectionGroup(int)),
		this, SIGNAL(signalCreateSelectionGroup(int)));

 // editor signals
 connect(mData, SIGNAL(signalEditorSavePlayFieldAs()),
		this, SLOT(slotEditorSavePlayFieldAs()));
 connect(mData, SIGNAL(signalEditorChangeLocalPlayer(Player*)),
		this, SIGNAL(signalEditorChangeLocalPlayer(Player*)));
 connect(mData, SIGNAL(signalEditorShowPlaceFacilities()),
		this, SIGNAL(signalEditorShowPlaceFacilities()));
 connect(mData, SIGNAL(signalEditorShowPlaceMobiles()),
		this, SIGNAL(signalEditorShowPlaceMobiles()));
 connect(mData, SIGNAL(signalEditorShowPlaceGround()),
		this, SIGNAL(signalEditorShowPlaceGround()));
 connect(mData, SIGNAL(signalEditorDeleteSelectedUnits()),
		this, SIGNAL(signalEditorDeleteSelectedUnits()));
 connect(mData, SIGNAL(signalEditorEditMapDescription()),
		this, SLOT(slotEditorEditMapDescription()));
 connect(mData, SIGNAL(signalEditorEditPlayerMinerals()),
		this, SLOT(slotEditorEditPlayerMinerals()));
 connect(mData, SIGNAL(signalEditorEditPlayerOil()),
		this, SLOT(slotEditorEditPlayerOil()));
 connect(mData, SIGNAL(signalEditorEditHeight(bool)),
		this, SIGNAL(signalEditorEditHeight(bool)));
 connect(mData, SIGNAL(signalEditorImportHeightMap()),
		this, SLOT(slotEditorImportHeightMap()));
 connect(mData, SIGNAL(signalEditorExportHeightMap()),
		this, SLOT(slotEditorExportHeightMap()));
 connect(mData, SIGNAL(signalEditorImportTexMap()),
		this, SLOT(slotEditorImportTexMap()));
 connect(mData, SIGNAL(signalEditorExportTexMap()),
		this, SLOT(slotEditorExportTexMap()));

}


void BosonMenuInput::setCamera(BoGameCamera* camera)
{
 mCamera = camera;
}

BoGameCamera* BosonMenuInput::camera() const
{
 return mCamera;
}

void BosonMenuInput::setPlayerIO(PlayerIO* io)
{
 mPlayerIO = io;
}

PlayerIO* BosonMenuInput::playerIO() const
{
 // TODO: actually return ((Player*)player())->playerIO(); would be nicer!
 // -> but we disallow including player.h here currently
 return mPlayerIO;
}

void BosonMenuInput::setActionCollection(BoUfoActionCollection* a)
{
 mActionCollection = a;
}

BoUfoActionCollection* BosonMenuInput::actionCollection() const
{
 return mActionCollection;
}

void BosonMenuInput::slotRotateLeft()
{
 BO_CHECK_NULL_RET(camera());
 camera()->changeRotation(5);
}

void BosonMenuInput::slotRotateRight()
{
 BO_CHECK_NULL_RET(camera());
 camera()->changeRotation(-5);
}

void BosonMenuInput::slotZoomIn()
{
 BO_CHECK_NULL_RET(camera());

 float delta = 5;

 camera()->changeZ(-delta);
}

void BosonMenuInput::slotZoomOut()
{
 BO_CHECK_NULL_RET(camera());

 float delta = 5;

 camera()->changeZ(delta);
}

void BosonMenuInput::slotToggleSound()
{
 boAudio->setSound(!boAudio->sound());
 boConfig->setBoolValue("Sound", boAudio->sound());
}

void BosonMenuInput::slotToggleMusic()
{
 boAudio->setMusic(!boAudio->music());
 boConfig->setBoolValue("Music", boAudio->music());
}

void BosonMenuInput::slotToggleFullScreen(bool fullScreen)
{
 if (fullScreen) {
	BoFullScreen::enterMode(-1);
 } else {
	BoFullScreen::leaveFullScreen();
 }
}

void BosonMenuInput::slotChangeMaxProfilingEventEntries()
{
 bool ok = true;
 unsigned int max = boConfig->uintValue("MaxProfilingEventEntries");
 max = (unsigned int)QInputDialog::getInteger(i18n("Profiling event entries"),
		i18n("Maximal number of profiling entries per event"),
		(int)max, 0, 100000, 1, &ok, 0);
 if (ok) {
	boConfig->setUIntValue("MaxProfilingEventEntries", max);
	boProfiling->setMaxEventEntries(boConfig->uintValue("MaxProfilingEventEntries"));
 }
}

void BosonMenuInput::slotChangeMaxProfilingAdvanceEntries()
{
 bool ok = true;
 unsigned int max = boConfig->uintValue("MaxProfilingAdvanceEntries");
 max = (unsigned int)QInputDialog::getInteger(i18n("Profiling advance entries"),
		i18n("Maximal number of profiled advance calls"),
		(int)max, 0, 100000, 1, &ok, 0);
 if (ok) {
	boConfig->setUIntValue("MaxProfilingAdvanceEntries", max);
	boProfiling->setMaxAdvanceEntries(boConfig->uintValue("MaxProfilingAdvanceEntries"));
 }
}

void BosonMenuInput::slotChangeMaxProfilingRenderingEntries()
{
 bool ok = true;
 unsigned int max = boConfig->uintValue("MaxProfilingRenderingEntries");
 max = (unsigned int)QInputDialog::getInteger(i18n("Profiling rendering entries"),
		i18n("Maximal number of profiled frames"),
		(int)max, 0, 100000, 1, &ok, 0);
 if (ok) {
	boConfig->setUIntValue("MaxProfilingRenderingEntries", max);
	boProfiling->setMaxRenderingEntries(boConfig->uintValue("MaxProfilingRenderingEntries"));
 }
}

void BosonMenuInput::slotProfiling()
{
 BosonProfilingDialog* dialog = new BosonProfilingDialog(0, false);
 connect(dialog, SIGNAL(finished()), dialog, SLOT(deleteLater()));
 dialog->show();
}

void BosonMenuInput::slotDebugKGame()
{
 if (!boGame) {
	boError() << k_funcinfo << "NULL game" << endl;
	return;
 }
 KGameDebugDialog* dlg = new KGameDebugDialog(boGame, 0, false);

 QVBox* b = dlg->addVBoxPage(i18n("Debug &Units"));
 KGameUnitDebug* units = new KGameUnitDebug(b);
 units->setBoson(boGame);

 b = dlg->addVBoxPage(i18n("Debug &Boson Players"));
 KGamePlayerDebug* player = new KGamePlayerDebug(b);
 player->setBoson(boGame);

 b = dlg->addVBoxPage(i18n("Debug &Advance messages"));
 KGameAdvanceMessagesDebug* messages = new KGameAdvanceMessagesDebug(b);
 messages->setBoson(boGame);

#if 0
 if (boGame->playField()) {
	BosonMap* map = boGame->playField()->map();
	if (!map) {
		boError() << k_funcinfo << "NULL map" << endl;
		return;
	}
	b = dlg->addVBoxPage(i18n("Debug &Cells"));

	// AB: this hardly does anything atm (04/04/23), but it takes a lot of
	// time and memory to be initialized on big maps (on list item per cell,
	// on a 500x500 map thats a lot)
	KGameCellDebug* cells = new KGameCellDebug(b);
	cells->setMap(map);
 }
#endif

 connect(dlg, SIGNAL(signalRequestIdName(int,bool,QString&)),
		this, SLOT(slotDebugRequestIdName(int,bool,QString&)));

 connect(dlg, SIGNAL(finished()), dlg, SLOT(deleteLater()));
 dlg->show();
}

void BosonMenuInput::slotDebugRequestIdName(int msgid, bool , QString& name)
{
 // we don't use i18n() for debug messages... not worth the work
 switch (msgid) {
	case BosonMessage::ChangeSpecies:
		name = "Change Species";
		break;
	case BosonMessage::ChangePlayField:
		name = "Change PlayField";
		break;
	case BosonMessage::ChangeTeamColor:
		name = "Change TeamColor";
		break;
	case BosonMessage::AdvanceN:
		name = "Advance";
		break;
	case BosonMessage::IdChat:
		name = "Chat Message";
		break;
	case BosonMessage::IdGameIsStarted:
		name = "Game is started";
		break;
	case BosonMessage::MoveMove:
		name = "PlayerInput: Move";
		break;
	case BosonMessage::MoveAttack:
		name = "PlayerInput: Attack";
		break;
	case BosonMessage::MoveBuild:
		name = "PlayerInput: Build";
		break;
	case BosonMessage::MoveProduce:
		name = "PlayerInput: Produce";
		break;
	case BosonMessage::MoveProduceStop:
		name = "PlayerInput: Produce Stop";
		break;
	case BosonMessage::MoveMine:
		name = "PlayerInput: Mine";
		break;
	case BosonMessage::UnitPropertyHandler:
	default:
		// a unit property was changed
		// all ids > UnitPropertyHandler will be a unit property. we
		// don't check further...
		break;
 }
// boDebug() << name << endl;
}

void BosonMenuInput::slotBoDebugLogDialog()
{
 BoDebugLogDialog* dialog = new BoDebugLogDialog(0);
 connect(dialog, SIGNAL(finished()), dialog, SLOT(deleteLater()));
 dialog->slotUpdate();
 dialog->show();
#if 0
 BoUfoDebugLogDialog* dialog = new BoUfoDebugLogDialog(ufoManager());
 dialog->slotUpdate();
 dialog->show();
#endif
}

void BosonMenuInput::slotSleep1s()
{
 sleep(1);
}

void BosonMenuInput::slotGrabScreenshot()
{
 boDebug() << k_funcinfo << "Taking screenshot!" << endl;

 QPixmap shot = QPixmap::grabWindow(qApp->mainWidget()->winId());
 if (shot.isNull()) {
	boError() << k_funcinfo << "NULL image returned" << endl;
	return;
 }
 QString file = findSaveFileName("boson", "jpg");
 if (file.isNull()) {
	boWarning() << k_funcinfo << "Can't find free filename???" << endl;
	return;
 }
 boDebug() << k_funcinfo << "Saving screenshot to " << file << endl;
 bool ok = shot.save(file, "JPEG", 90);
 if (!ok) {
	boError() << k_funcinfo << "Error saving screenshot to " << file << endl;
	boGame->slotAddChatSystemMessage(i18n("An error occured while saving screenshot to %1").arg(file));
 } else {
	boGame->slotAddChatSystemMessage(i18n("Screenshot saved to %1").arg(file));
 }
}

void BosonMenuInput::slotGrabProfiling()
{
 QString file = findSaveFileName("boprofiling", "boprof");
 if (file.isNull()) {
	boWarning() << k_funcinfo << "Can't find free filename???" << endl;
	return;
 }
 // TODO: chat message about file location!
 boDebug() << k_funcinfo << "Saving profiling to " << file << endl;
 bool ok = boProfiling->saveToFile(file);
 if (!ok) {
	boError() << k_funcinfo << "Error saving profiling to " << file << endl;
	boGame->slotAddChatSystemMessage(i18n("An error occured while saving profiling log to %1").arg(file));
 } else {
	boGame->slotAddChatSystemMessage(i18n("Profiling log saved to %1").arg(file));
 }
}

void BosonMenuInput::slotSetGrabMovie(bool grab)
{
#warning fixme
#if 0
 d->mGrabMovie = grab;
#endif
}

void BosonMenuInput::slotSetDebugMode(int index)
{
 boConfig->setIntValue("DebugMode", (int)((BosonConfig::DebugMode)index));
}

void BosonMenuInput::slotDebugKillPlayer(Q_UINT32 playerId)
{
 QByteArray b;
 QDataStream stream(b, IO_WriteOnly);
 stream << (Q_UINT32)playerId;
 boGame->sendMessage(b, BosonMessage::IdKillPlayer);
}

void BosonMenuInput::slotDebugModifyMinerals(Q_UINT32 playerId, int amount)
{
 QByteArray b;
 QDataStream stream(b, IO_WriteOnly);
 stream << (Q_UINT32)playerId;
 stream << (Q_INT32)amount;
 boGame->sendMessage(b, BosonMessage::IdModifyMinerals);
}

void BosonMenuInput::slotDebugModifyOil(Q_UINT32 playerId, int amount)
{
 QByteArray b;
 QDataStream stream(b, IO_WriteOnly);
 stream << (Q_UINT32)playerId;
 stream << (Q_INT32)amount;
 boGame->sendMessage(b, BosonMessage::IdModifyOil);
}

void BosonMenuInput::slotToggleCheating(bool on)
{
 if (!actionCollection()) {
	return;
 }
 actionCollection()->setActionEnabled("debug_unfog", on);
 actionCollection()->setActionEnabled("debug_players", on);
}

void BosonMenuInput::slotUnfogAll(Player* pl)
{
 // AB: disabled, because we cannot use player.h here.
 // see also comment in slotEditorEditPlayerMinerals()
#if 0
 if (!boGame) {
	boError() << k_funcinfo << "NULL game" << endl;
	return;
 }
 if (!boGame->playField()) {
	boError() << k_funcinfo << "NULL playField" << endl;
	return;
 }
 BosonMap* map = boGame->playField()->map();
 if (!map) {
	boError() << k_funcinfo << "NULL map" << endl;
	return;
 }
 QPtrList<KPlayer> list;
 if (!pl) {
	list = *boGame->playerList();
 } else {
	list.append(pl);
 }
 for (unsigned int i = 0; i < list.count(); i++) {
	Player* p = (Player*)list.at(i);
	for (unsigned int x = 0; x < map->width(); x++) {
		for (unsigned int y = 0; y < map->height(); y++) {
			p->unfog(x, y);
		}
	}
	boGame->slotAddChatSystemMessage(i18n("Debug"), i18n("Unfogged player %1 - %2").arg(p->id()).arg(p->name()));
 }
#endif
}

void BosonMenuInput::slotDumpGameLog()
{
 boGame->saveGameLogs("boson");
}

void BosonMenuInput::slotEditConditions()
{
 KDialogBase* dialog = new KDialogBase(KDialogBase::Plain, i18n("Conditions"),
		KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok, 0,
		"editconditions", true, true);
 QVBoxLayout* layout = new QVBoxLayout(dialog->plainPage());
 BoConditionWidget* widget = new BoConditionWidget(dialog->plainPage());
 layout->addWidget(widget);

 {
	QDomDocument doc;
	QDomElement root = doc.createElement("Conditions");
	doc.appendChild(root);

	if (!boGame->saveCanvasConditions(root)) {
		boError() << k_funcinfo << "unable to save canvas conditions from game" << endl;
		KMessageBox::information(0, i18n("Canvas conditions could not be imported to the widget"));
	} else {
		QValueStack<QDomElement> stack;
		stack.push(root);
		while (!stack.isEmpty()) {
			QDomElement e = stack.pop();
			for (QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling()) {
				QDomElement e2 = n.toElement();
				if (e2.isNull()) {
					continue;
				}
				stack.push(e2);
			}
			if (e.hasAttribute("PlayerId")) {
				bool ok = false;
				int index = e.attribute("PlayerId").toInt(&ok);
				if (!ok) {
					boError() << k_funcinfo << "PlayerId attribute not a valid number" << endl;
					continue;
				}
				KPlayer* p = boGame->playerList()->at(index);
				e.setAttribute("PlayerId", p->id());
			}
		}

		widget->loadConditions(root);
	}
;
 }

 int ret = dialog->exec();
 QString xml = widget->toString();
 delete widget;
 widget = 0;
 delete dialog;
 dialog = 0;
 if (ret == KDialogBase::Accepted) {
	QDomDocument doc;
	bool ret = doc.setContent(xml);
	QDomElement root = doc.documentElement();
	if (!ret || root.isNull()) {
		boError() << k_funcinfo << "invalid XML document created" << endl;
		KMessageBox::sorry(0, i18n("Oops - an invalid XML document was created. Internal error."));
		return;
	}
	boDebug() << k_funcinfo << "applying canvas conditions" << endl;
	boGame->loadCanvasConditions(root);
 }
}

void BosonMenuInput::slotShowGLStates()
{
 boDebug() << k_funcinfo << endl;
 BoGLStateWidget* w = new BoGLStateWidget(0, 0, WDestructiveClose);
 w->show();
}

void BosonMenuInput::slotReloadMeshRenderer()
{
 bool unusable = false;
 bool r = BoMeshRendererManager::manager()->reloadPlugin(&unusable);
 if (r) {
	return;
 }
 boError() << "meshrenderer reloading failed" << endl;
 if (unusable) {
	KMessageBox::sorry(0, i18n("Reloading meshrenderer failed, library is now unusable. quitting."));
	exit(1);
 } else {
	KMessageBox::sorry(0, i18n("Reloading meshrenderer failed but library should still be usable"));
 }
}

void BosonMenuInput::slotReloadGroundRenderer()
{
 bool unusable = false;
 bool r = BoGroundRendererManager::manager()->reloadPlugin(&unusable);
 if (r) {
	return;
 }
 boError() << "groundrenderer reloading failed" << endl;
 if (unusable) {
	KMessageBox::sorry(0, i18n("Reloading groundrenderer failed, library is now unusable. quitting."));
	exit(1);
 } else {
	KMessageBox::sorry(0, i18n("Reloading groundrenderer failed but library should still be usable"));
 }
}

void BosonMenuInput::slotCrashBoson()
{
 ((QObject*)0)->name();
}

void BosonMenuInput::slotDebugMemory()
{
#ifdef BOSON_USE_BOMEMORY
 boDebug() << k_funcinfo << endl;
 BoMemoryDialog* dialog = new BoMemoryDialog(this);
 connect(dialog, SIGNAL(finished()), dialog, SLOT(deleteLater()));
 boDebug() << k_funcinfo << "update data" << endl;
 dialog->slotUpdate();
 dialog->show();
 boDebug() << k_funcinfo << "done" << endl;
#endif
}

void BosonMenuInput::slotPreferences()
{
 if (!boGame) {
	boWarning() << k_funcinfo << "NULL boGame object" << endl;
	return;
 }
 OptionsDialog* dlg = new OptionsDialog(!boGame->gameMode(), 0);
 dlg->setGame(boGame);
 dlg->setPlayer(playerIO()->player());
 dlg->slotLoad();

 connect(dlg, SIGNAL(finished()), dlg, SLOT(deleteLater())); // seems not to be called if you quit with "cancel"!

 connect(dlg, SIGNAL(signalCursorChanged(int, const QString&)),
		this, SIGNAL(signalChangeCursor(int, const QString&)));
// connect(dlg, SIGNAL(signalCmdBackgroundChanged(const QString&)),
//		this, SLOT(slotCmdBackgroundChanged(const QString&)));
 connect(dlg, SIGNAL(signalOpenGLSettingsUpdated()),
		this, SIGNAL(signalUpdateOpenGLSettings()));
 connect(dlg, SIGNAL(signalApply()),
		this, SIGNAL(signalPreferencesApply()));
// connect(dlg, SIGNAL(signalFontChanged(const BoFontInfo&)),
//		displayManager(), SLOT(slotChangeFont(const BoFontInfo&)));

 dlg->show();
}

void BosonMenuInput::slotCenterHomeBase()
{
 boDebug() << k_funcinfo << endl;
 BO_CHECK_NULL_RET(camera());
 BO_CHECK_NULL_RET(playerIO());

 QPoint pos = playerIO()->homeBase();
 camera()->setLookAt(BoVector3Float((float)pos.x(), -(float)pos.y(), 0.0));
}

void BosonMenuInput::slotSyncNetwork()
{
 BO_CHECK_NULL_RET(boGame);
 boGame->syncNetwork();
}

void BosonMenuInput::slotEditorSavePlayFieldAs()
{
 boDebug() << k_funcinfo << endl;
 QString startIn; // shall we provide this??
 QString fileName = KFileDialog::getSaveFileName(startIn, "*.bpf", 0);
 if (fileName.isNull()) {
	return;
 }
 QFileInfo info(fileName);
 if (info.extension().isEmpty()) {
	fileName += ".bpf";
 }
 if (info.exists()) {
	int r = KMessageBox::warningYesNoCancel(0, i18n("The file \"%1\" already exists. Are you sure you want to overwrite it?").arg(info.fileName()), i18n("Overwrite File?"));
	if (r != KMessageBox::Yes) {
		return;
	}
 }
 bool ret = boGame->savePlayFieldToFile(fileName);
 if (!ret) {
	KMessageBox::sorry(0, i18n("An error occurred while saving the playfield. Unable to save."));
 }
}

void BosonMenuInput::slotEditorEditMapDescription()
{
 BO_CHECK_NULL_RET(boGame);
 BO_CHECK_NULL_RET(boGame->playField());
 BO_CHECK_NULL_RET(boGame->playField()->description());


// TODO: non-modal might be fine. one could use that for translations (one
// dialog the original language, one the translated language)
 BPFDescriptionDialog* dialog = new BPFDescriptionDialog(0, true);
 dialog->setDescription(boGame->playField()->description());
 dialog->exec();

 delete dialog;
}

void BosonMenuInput::slotEditorEditPlayerMinerals()
{
 // atm disabled, as we must not include player.h in this file
 // these methods (e.g. all editor dependent methods) should get moved to a
 // different file anyway.
 // maybe all game/editor menu items will be handled by a dedicated KGameIO
 // class.
#if 0
 BO_CHECK_NULL_RET(localPlayer());
 bool ok = false;
 QString value = QString::number(localPlayer()->minerals());
 QIntValidator val(this);
 val.setBottom(0);
 val.setTop(1000000); // we need to set a top, because int is limited. this should be enough, i hope (otherwise feel free to increase)
 value = KLineEditDlg::getText(i18n("Minerals for player %1").arg(localPlayer()->name()), value, &ok, this, &val);
 if (!ok) {
	// cancel pressed
	return;
 }
 boDebug() << k_funcinfo << value << endl;
 unsigned long int v = value.toULong(&ok);
 if (!ok) {
	boWarning() << k_funcinfo << "value " << value << " not valid" << endl;
	return;
 }
 localPlayer()->setMinerals(v);
#endif
}

void BosonMenuInput::slotEditorEditPlayerOil()
{
 // atm disabled, as we must not include player.h in this file
#if 0
 BO_CHECK_NULL_RET(localPlayer());
 bool ok = false;
 QString value = QString::number(localPlayer()->oil());
 QIntValidator val(this);
 val.setBottom(0);
 val.setTop(1000000); // we need to set a top, because int is limited. this should be enough, i hope (otherwise feel free to increase)
 value = KLineEditDlg::getText(i18n("Oil for player %1").arg(localPlayer()->name()), value, &ok, this, &val);
 if (!ok) {
	return;
 }
 boDebug() << k_funcinfo << value << endl;
 unsigned long int v = value.toULong(&ok);
 if (!ok) {
	boWarning() << k_funcinfo << "value " << value << " not valid" << endl;
	return;
 }
 localPlayer()->setOil(v);
#endif
}

void BosonMenuInput::slotEditorImportHeightMap()
{
 BO_CHECK_NULL_RET(boGame);
 BO_CHECK_NULL_RET(boGame->playField());
 BO_CHECK_NULL_RET(boGame->playField()->map());
 boDebug() << k_funcinfo << endl;
 QString fileName = KFileDialog::getOpenFileName(QString::null, "*.png", 0);
 if (fileName.isNull()) {
	return;
 }
 // first load the file as an image. We need it to be in greyscale png in boson,
 // and we can easily convert that image.
 QImage image(fileName);
 if (image.isNull()) {
	boError() << k_funcinfo << "unbable to load file " << fileName << endl;
	KMessageBox::sorry(0, i18n("Unable to load %1\nSeems not to be a valid image.").arg(fileName));
	return;
 }
 BosonMap* map = boGame->playField()->map();
 if ((unsigned int)image.width() != map->width() + 1 ||
		(unsigned int)image.height() != map->height() + 1) {
	KMessageBox::sorry(0, i18n("This image can't be used as height map for this map. The map is a %1x%2 map, meaning you need a %3x%4 image.\nThe image selected %5 was %6x%7").
			arg(map->width()).arg(map->height()).
			arg(map->width() + 1).arg(map->height() + 1).
			arg(fileName).
			arg(image.width()).arg(image.height()));
	return;
 }
 if (!image.isGrayscale()) {
	KMessageBox::sorry(0, i18n("%1 is not a greyscale image").arg(fileName));
	return;
 }
 boGame->playField()->importHeightMapImage(image);
 // TODO: update unit positions!
}

void BosonMenuInput::slotEditorExportHeightMap()
{
 BO_CHECK_NULL_RET(boGame);
 BO_CHECK_NULL_RET(boGame->playField());
 BO_CHECK_NULL_RET(boGame->playField()->map());
 boDebug() << k_funcinfo << endl;
 QString fileName = KFileDialog::getSaveFileName(QString::null, "*.png", 0);
 if (fileName.isNull()) {
	return;
 }
 QByteArray buffer = boGame->playField()->exportHeightMap();
 if (buffer.size() == 0) {
	boError() << k_funcinfo << "Could not export heightMap" << endl;
	KMessageBox::sorry(0, i18n("Unable to export heightMap"));
	return;
 }
 QImage image(buffer);
 if (image.isNull()) {
	boError() << k_funcinfo << "an invalid image has been generated" << endl;
	KMessageBox::sorry(0, i18n("An invalid heightmop image has been generated."));
 }
 if (!image.save(fileName, "PNG")) {
	boError() << k_funcinfo << "unable to save image to " << fileName << endl;
	KMessageBox::sorry(0, i18n("Unable to save image to %1.").arg(fileName));
	return;
 }
}

void BosonMenuInput::slotEditorImportTexMap()
{
 boDebug() << k_funcinfo << endl;
 BoTexMapImportDialog* dialog = new BoTexMapImportDialog(0);
 connect(dialog, SIGNAL(finished()),
		dialog, SLOT(deleteLater()));

 BosonMap* map = boGame->playField()->map();
 dialog->setMap(map);

 dialog->show();
 dialog->slotSelectTexMapImage();
}

void BosonMenuInput::slotEditorExportTexMap()
{
 boDebug() << k_funcinfo << endl;
 BO_CHECK_NULL_RET(boGame);
 BO_CHECK_NULL_RET(boGame->playField());
 BO_CHECK_NULL_RET(boGame->playField()->map());
 BO_CHECK_NULL_RET(boGame->playField()->map()->groundTheme());

 BosonMap* map = boGame->playField()->map();
 QStringList textures;
 for (unsigned int i = 0; i < map->groundTheme()->textureCount(); i++) {
	textures.append(map->groundTheme()->textureFileName(i));
 }

 QDialog* d = new QDialog(0, 0, true);
 QVBoxLayout* layout = new QVBoxLayout(d);
 QLabel* label = new QLabel(i18n("Select texture to export:"), d);
 QComboBox* combo = new QComboBox(d);
 QPushButton* button = new QPushButton(i18n("Ok"), d);
 connect(button, SIGNAL(clicked()), d, SLOT(accept()));
 combo->insertStringList(textures);
 layout->addWidget(label);
 layout->addWidget(combo);
 layout->addWidget(button);
 d->exec();
 unsigned int tex = (unsigned int)combo->currentItem();
 boDebug() << k_funcinfo << "tex: " << tex << endl;
 delete d;

 QString fileName = KFileDialog::getSaveFileName(QString::null, "*.png", 0);
 if (fileName.isNull()) {
	return;
 }
 QByteArray buffer = boGame->playField()->exportTexMap(tex);
 if (buffer.size() == 0) {
	boError() << k_funcinfo << "Could not export texmap" << endl;
	KMessageBox::sorry(0, i18n("Unable to export texmap"));
	return;
 }
 QImage image(buffer);
 if (image.isNull()) {
	boError() << k_funcinfo << "an invalid image has been generated" << endl;
	KMessageBox::sorry(0, i18n("An invalid texmap image has been generated."));
 }
 if (!image.save(fileName, "PNG")) {
	boError() << k_funcinfo << "unable to save image to " << fileName << endl;
	KMessageBox::sorry(0, i18n("Unable to save image to %1.").arg(fileName));
	return;
 }
}


