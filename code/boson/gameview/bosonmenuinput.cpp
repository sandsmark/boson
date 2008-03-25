/*
    This file is part of the Boson game
    Copyright (C) 2004-2005 Andreas Beckermann (b_mann@gmx.de)

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

#include "bosonmenuinput.h"
#include "bosonmenuinput.moc"

#include "../bomemory/bodummymemory.h"
#include "bodebug.h"
#include "../boufo/boufoaction.h"
#include "../boufoactionext.h"
#include "../defines.h"
#include "../bosonconfig.h"
#include "../gameengine/boson.h"
#include "../bosondata.h"
#include "../bosonconfig.h"
#include "../bpfdescriptiondialog.h"
#include "../boconditionwidget.h"
#include "../gameengine/bosonmessageids.h"
#include "../bocamera.h"
#include "../boaction.h"
#include "../gameengine/bpfdescription.h"
#include "../gameengine/bosonplayfield.h"
#include "../gameengine/bosonmap.h"
#include "../gameengine/bosongroundtheme.h"
#include "../bogroundrenderermanager.h"
#include "../modelrendering/bomeshrenderermanager.h"
#include "../gameengine/playerio.h"
#include "../gameengine/player.h"
#include "../botexmapimportdialog.h"
#include "../bofiledialog.h"
#include "boeditplayerinputswidget.h"
#ifdef BOSON_USE_BOMEMORY
#include "../bomemory/bomemorydialog.h"
#endif

#include <klocale.h>
#include <kshortcut.h>
#include <kgame/kplayer.h>
#include <kgame/kgamedebugdialog.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <klineeditdlg.h>

#include <qguardedptr.h>
#include <qptrdict.h>
#include <qsignalmapper.h>
#include <qtimer.h>
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
#include <qvalidator.h>
#include <qdict.h>

#include <unistd.h>

#define ID_DEBUG_KILLPLAYER 0
#define ID_DEBUG_ADD_10000_MINERALS 1
#define ID_DEBUG_ADD_1000_MINERALS 2
#define ID_DEBUG_SUB_1000_MINERALS 3
#define ID_DEBUG_ADD_10000_OIL 4
#define ID_DEBUG_ADD_1000_OIL 5
#define ID_DEBUG_SUB_1000_OIL 6
#define ID_DEBUG_PLAYER_INPUT 7



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
		mActionEditorPlayer = 0;
		mActionEditorPlace = 0;

		mSelectMapper = 0;
		mCreateMapper = 0;
		mShowSelectionMapper = 0;
	}

	BoUfoActionCollection* mActionCollection;

#if 0
	QGuardedPtr<BoUfoToggleAction> mActionMenubar;
#endif
	QGuardedPtr<BoUfoActionMenu> mActionDebugPlayers;
	QGuardedPtr<BoUfoActionMenu> mActionDebugColormap;
	QGuardedPtr<BoUfoSelectAction> mActionEditorPlayer;
	QGuardedPtr<BoUfoSelectAction> mActionEditorPlace;
	QGuardedPtr<BoUfoToggleAction> mActionEditorChangeHeight;
	QPtrList<KPlayer> mEditorPlayers;
	QPtrDict<KPlayer> mActionDebugPlayer2Player;

	QSignalMapper* mSelectMapper;
	QSignalMapper* mCreateMapper;
	QSignalMapper* mShowSelectionMapper;
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

void BosonMenuInputData::setLocalPlayerIO(PlayerIO* io)
{
 BO_CHECK_NULL_RET(io);
 Player* p = io->player();
 BO_CHECK_NULL_RET(p);
 if (d->mActionEditorPlayer) { // editor mode
	int current = d->mEditorPlayers.findRef((KPlayer*)p);
	if (current < 0) {
		boError() << k_funcinfo << "cannot find player " << p << " (" << io->playerId() << ") for IO " << io << endl;
		return;
	}
	d->mActionEditorPlayer->blockSignals(true);
	d->mActionEditorPlayer->setCurrentItem(current);
	d->mActionEditorPlayer->blockSignals(false);

	d->mActionEditorPlace->setCurrentItem(0);
 }
}

BoUfoActionCollection* BosonMenuInputData::actionCollection() const
{
 return d->mActionCollection;
}

void BosonMenuInputData::initUfoActions(bool gameMode)
{
 BO_CHECK_NULL_RET(actionCollection());
 BO_CHECK_NULL_RET(boGame);
 BO_CHECK_NULL_RET(boGame->playField());
 BO_CHECK_NULL_RET(boGame->playField()->map());

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



 (void)new BoUfoAction(i18n("&Reset View Properties"), KShortcut(Qt::Key_R), this,
		SIGNAL(signalResetViewProperties()), actionCollection(), "game_reset_view_properties");


 (void)new BoUfoToggleAction(i18n("Show Cha&t"),
		KShortcut(Qt::CTRL+Qt::Key_C), this, SIGNAL(signalToggleChatVisible()),
		actionCollection(), "options_show_chat");

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
 (void)new BoUfoConfigToggleAction(i18n("Debug Memory Usage"),
		KShortcut(), 0, 0, actionCollection(), "debug_memory_usage");
 (void)new BoUfoConfigToggleAction(i18n("Debug Memory Usage (VmData only)"),
		KShortcut(), 0, 0, actionCollection(), "debug_memory_vmdata_only");
 (void)new BoUfoConfigToggleAction(i18n("Debug Profiling Graph"),
		KShortcut(), 0, 0, actionCollection(), "debug_profiling_graph");
 (void)new BoUfoConfigToggleAction(i18n("Debug Rendering Config Switches"),
		KShortcut(), 0, 0, actionCollection(), "debug_rendering_config");
 (void)new BoUfoConfigToggleAction(i18n("Debug Network Traffic"),
		KShortcut(), 0, 0, actionCollection(), "debug_network_traffic");
 (void)new BoUfoConfigToggleAction(i18n("glFinish() before profiling GL methods"),
		KShortcut(), 0, 0, actionCollection(), "debug_glfinish_before_profiling");
 (void)new BoUfoConfigToggleAction(i18n("Debug Groundrenderer"),
		KShortcut(), 0, 0, actionCollection(), "debug_groundrenderer_debug");
 (void)new BoUfoConfigToggleAction(i18n("Debug CPU Usage"),
		KShortcut(), 0, 0, actionCollection(), "debug_cpu_usage");
 (void)new BoUfoAction(i18n("&Explore"), KShortcut(), this,
		SIGNAL(signalExploreAll()), actionCollection(), "debug_explore");
 (void)new BoUfoAction(i18n("&Unfog"), KShortcut(), this,
		SIGNAL(signalUnfogAll()), actionCollection(), "debug_unfog");
 (void)new BoUfoAction(i18n("&Fog"), KShortcut(), this,
		SIGNAL(signalFogAll()), actionCollection(), "debug_fog");
 (void)new BoUfoAction(i18n("Dump game &log"), KShortcut(), this,
		SIGNAL(signalDumpGameLog()), actionCollection(), "debug_gamelog");
 (void)new BoUfoAction(i18n("Edit global conditions..."), KShortcut(), this,
		SIGNAL(signalEditConditions()), actionCollection(),
		"debug_edit_conditions");
 (void)new BoUfoConfigToggleAction(i18n("Show Unit Debug Widget"),
		KShortcut(), 0, 0, actionCollection(),
		"debug_show_unit_debug_widget",
		"ShowUnitDebugWidget");


 BoUfoSelectAction* debugMode = new BoUfoSelectAction(i18n("Mode"), 0, 0,
		actionCollection(), "debug_mode");
 connect(debugMode, SIGNAL(signalActivated(int)), this,
		SIGNAL(signalSetDebugMode(int)));
 QStringList l;
 l.append(i18n("Normal"));
 l.append(i18n("Debug Selection"));
 debugMode->setItems(l);
 debugMode->setCurrentItem(0);

 (void)new BoUfoAction(i18n("Reload &meshrenderer plugin"), KShortcut(), this,
		SIGNAL(signalReloadMeshRenderer()), actionCollection(),
		"debug_lazy_reload_meshrenderer");
 (void)new BoUfoAction(i18n("Reload &groundrenderer plugin"), KShortcut(), this,
		SIGNAL(signalReloadGroundRenderer()), actionCollection(),
		"debug_lazy_reload_groundrenderer");
 (void)new BoUfoAction(i18n("Reload g&ameview plugin"), KShortcut(), this,
		SIGNAL(signalReloadGameViewPlugin()), actionCollection(),
		"debug_lazy_reload_gameviewplugin");
 (void)new BoUfoAction(i18n("Light0..."), KShortcut(), this,
		SIGNAL(signalShowLight0Widget()), actionCollection(),
		"debug_light0");
#ifdef BOSON_USE_BOMEMORY
 (void)new BoUfoAction(i18n("Debug M&emory"), KShortcut(), this,
		SIGNAL(signalDebugMemory()), actionCollection(),
		"debug_memory");
#endif
 createDebugPlayersMenu();

 d->mActionDebugColormap = new BoUfoActionMenu(i18n("Colormap"),
		actionCollection(), "debug_colormap");
 connect(d->mActionDebugColormap, SIGNAL(signalActivated(int)),
		this, SLOT(slotChangeColorMap(int)));
 slotUpdateColorMapsMenu();
 connect(boGame->playField()->map(), SIGNAL(signalColorMapsChanged()),
		this, SLOT(slotUpdateColorMapsMenu()));

 QStringList files;
 files.append(boData->locateDataFile("boson/bosonbaseui.rc"));
 if (gameMode) {
	initUfoGameActions();
	files.append(boData->locateDataFile("boson/bosonui.rc"));
 } else {
	initUfoEditorActions();
	files.append(boData->locateDataFile("boson/editorui.rc"));
 }

 actionCollection()->setGUIFiles(files);
 actionCollection()->createGUI();

 resetDefaults();
}

void BosonMenuInputData::initUfoGameActions()
{
 BO_CHECK_NULL_RET(actionCollection());

 (void)BoUfoStdAction::gameQuit(this, SIGNAL(signalQuit()), actionCollection());
 (void)BoUfoStdAction::gameEnd(this, SIGNAL(signalEndGame()), actionCollection());
 (void)BoUfoStdAction::gameSave(this, SIGNAL(signalSaveGame()), actionCollection());
 (void)BoUfoStdAction::gameLoad(this, SIGNAL(signalLoadGame()), actionCollection());
 (void)BoUfoStdAction::gamePause(boGame, SLOT(slotTogglePause()), actionCollection());
 (void)new BoUfoAction(i18n("Quicksave"),
		KShortcut(Qt::Key_F5),
		this, SIGNAL(signalQuicksaveGame()),
		actionCollection(), "game_quicksave");
 (void)new BoUfoAction(i18n("Quickload"),
		KShortcut(Qt::Key_F7),
		this, SIGNAL(signalQuickloadGame()),
		actionCollection(), "game_quickload");
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
 delete d->mShowSelectionMapper;
 d->mShowSelectionMapper = new QSignalMapper(this);
 connect(d->mSelectMapper, SIGNAL(mapped(int)),
		this, SIGNAL(signalSelectSelectionGroup(int)));
 connect(d->mCreateMapper, SIGNAL(mapped(int)),
		this, SIGNAL(signalCreateSelectionGroup(int)));
 connect(d->mShowSelectionMapper, SIGNAL(mapped(int)),
		this, SIGNAL(signalShowSelectionGroup(int)));

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
	a = new BoUfoAction(i18n("Show Group %1").arg(i == 0 ? 10 : i),
			Qt::ALT + Qt::Key_0 + i, d->mShowSelectionMapper,
			SLOT(map()), actionCollection(),
			QString("show_group_%1").arg(i));
	d->mShowSelectionMapper->setMapping(a, i);
 }

}

void BosonMenuInputData::initUfoEditorActions()
{
 BO_CHECK_NULL_RET(actionCollection());


 BoUfoStdAction::fileSaveAs(this, SIGNAL(signalEditorSavePlayFieldAs()), actionCollection(), "file_save_playfield_as");
 BoUfoStdAction::fileClose(this, SIGNAL(signalEndGame()), actionCollection());

 // TODO
// close->setText(i18n("&End Editor"));
 BoUfoStdAction::fileQuit(this, SIGNAL(signalQuit()), actionCollection());

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
 (void)new BoUfoConfigToggleAction(i18n("Show Random Map Generation Widget"),
		KShortcut(), 0, 0, actionCollection(),
		"editor_show_random_map_generation",
		"EditorShowRandomMapGenerationWidget");

 BoUfoStdAction::editUndo(this, SIGNAL(signalEditorUndo()), actionCollection());
 BoUfoStdAction::editRedo(this, SIGNAL(signalEditorRedo()), actionCollection());
 actionCollection()->action("edit_undo")->setEnabled(false);
 actionCollection()->action("edit_redo")->setEnabled(false);

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
 QPtrList<Player> players = boGame->gamePlayerList();
 QStringList items;

 d->mEditorPlayers.clear();
 d->mActionEditorPlayer->clear();
 for (QPtrListIterator<Player> it(players); it.current(); ++it) {
	d->mEditorPlayers.append(it.current());
	items.append(it.current()->name());
 }
 d->mActionEditorPlayer->setItems(items);
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

 QPtrList<Player> players = boGame->gamePlayerList();
 QPtrListIterator<Player> it(players);
 for (; it.current(); ++it) {
	KPlayer* player = it.current();
	BoUfoActionMenu* menu = new BoUfoActionMenu(player->name(),
			actionCollection(),
			QString("debug_players_%1").arg(player->name()));

	connect(menu, SIGNAL(signalActivated(int)),
			this, SLOT(slotDebugPlayer(int)));

	menu->insertItem(i18n("Edit Inputs"), ID_DEBUG_PLAYER_INPUT);
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

void BosonMenuInputData::slotDebugPlayer(int id)
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

 Q_UINT32 playerId = ((Player*)p)->bosonId();
 switch (id) {
	case ID_DEBUG_PLAYER_INPUT:
		emit signalDebugEditPlayerInputs((Player*)p);
		break;
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
		boError() << k_funcinfo << "unknown ID " << id << endl;
		break;
 }
}

void BosonMenuInputData::slotUpdateColorMapsMenu()
{
 BO_CHECK_NULL_RET(boGame);
 BO_CHECK_NULL_RET(boGame->playField());
 // Clear current entries
 d->mActionDebugColormap->clear();

 int i = 0;
 // First add the "disabled" entry
 d->mActionDebugColormap->insertItem(i18n("Disabled"), i++);

 // Then add colormap entries
 BosonMap* map = boGame->playField()->map();
 BO_CHECK_NULL_RET(map);
 BO_CHECK_NULL_RET(map->colorMaps());
 QDictIterator<BoColorMap> it(*map->colorMaps());
 while (it.current()) {
	d->mActionDebugColormap->insertItem(it.currentKey(), i++);
	++it;
 }
}

void BosonMenuInputData::slotChangeColorMap(int index)
{
 BosonMap* map = boGame->playField()->map();
 // Check if colormap was disabled
 if (index == 0) {
	map->setActiveColorMap(0);
	return;
 }

 // Find the wanted colormap
 int i = 1;
 QDictIterator<BoColorMap> it(*map->colorMaps());
 while (it.current()) {
	if (index == i) {
		// Set it to be the active one
		map->setActiveColorMap(it.current());
		return;
	}

	i++;
	++it;
 }
}

void BosonMenuInputData::slotEditorChangeLocalPlayerHack()
{
 slotEditorChangeLocalPlayer(0);
}

void BosonMenuInputData::slotEditorChangeLocalPlayer(int index)
{
 boDebug() << k_funcinfo << index << endl;
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
 boDebug() << k_funcinfo << "sender() not a BoUfoAction - we were probably called indirectly using a QTimer - continue..." << endl;
 p = senderPlayer;
 if (p) {
	emit signalEditorChangeLocalPlayer((Player*)p);

	// AB: returning here is important: the signal causes the menu input to
	// be deleted!
	return;
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
 //    construction of this object
 //
 // AB: UPDATE: player.h is allowed in this file

 BO_CHECK_NULL_RET(actionCollection());

 mData = new BosonMenuInputData(actionCollection());

 connect(mData, SIGNAL(signalScroll(int)),
		this, SIGNAL(signalScroll(int)));
 connect(mData, SIGNAL(signalRotateLeft()),
		this, SLOT(slotRotateLeft()));
 connect(mData, SIGNAL(signalRotateRight()),
		this, SLOT(slotRotateRight()));
 connect(mData, SIGNAL(signalZoomIn()),
		this, SLOT(slotZoomIn()));
 connect(mData, SIGNAL(signalZoomOut()),
		this, SLOT(slotZoomOut()));
 connect(mData, SIGNAL(signalToggleStatusbar(bool)),
		this, SIGNAL(signalToggleStatusbar(bool)));
 connect(mData, SIGNAL(signalToggleChatVisible()),
		this, SIGNAL(signalToggleChatVisible()));
 connect(mData, SIGNAL(signalResetViewProperties()),
		this, SIGNAL(signalResetViewProperties()));
 connect(mData, SIGNAL(signalSetGrabMovie(bool)),
		this, SLOT(slotSetGrabMovie(bool)));
 connect(mData, SIGNAL(signalSetDebugMode(int)),
		this, SLOT(slotSetDebugMode(int)));
 connect(mData, SIGNAL(signalDebugEditPlayerInputs(Player*)),
		this, SLOT(slotDebugEditPlayerInputs(Player*)));
 connect(mData, SIGNAL(signalDebugKillPlayer(Q_UINT32)),
		this, SLOT(slotDebugKillPlayer(Q_UINT32)));
 connect(mData, SIGNAL(signalDebugModifyMinerals(Q_UINT32, int)),
		this, SLOT(slotDebugModifyMinerals(Q_UINT32, int)));
 connect(mData, SIGNAL(signalDebugModifyOil(Q_UINT32, int)),
		this, SLOT(slotDebugModifyOil(Q_UINT32, int)));
 connect(mData, SIGNAL(signalToggleCheating(bool)),
		this, SLOT(slotToggleCheating(bool)));
 connect(mData, SIGNAL(signalExploreAll()),
		this, SLOT(slotExploreAll()));
 connect(mData, SIGNAL(signalUnfogAll()),
		this, SLOT(slotUnfogAll()));
 connect(mData, SIGNAL(signalFogAll()),
		this, SLOT(slotFogAll()));
 connect(mData, SIGNAL(signalDumpGameLog()),
		this, SLOT(slotDumpGameLog()));
 connect(mData, SIGNAL(signalEditConditions()),
		this, SLOT(slotEditConditions()));
 connect(mData, SIGNAL(signalReloadMeshRenderer()),
		this, SLOT(slotReloadMeshRenderer()));
 connect(mData, SIGNAL(signalReloadGroundRenderer()),
		this, SLOT(slotReloadGroundRenderer()));
 connect(mData, SIGNAL(signalReloadGameViewPlugin()),
		this, SIGNAL(signalReloadGameViewPlugin()));
 connect(mData, SIGNAL(signalShowLight0Widget()),
		this, SIGNAL(signalShowLight0Widget()));
 connect(mData, SIGNAL(signalDebugMemory()),
		this, SLOT(slotDebugMemory()));
 connect(mData, SIGNAL(signalEndGame()),
		this, SLOT(slotEndGame()));
 connect(mData, SIGNAL(signalQuit()),
		this, SIGNAL(signalQuit()));

 // game signals
 connect(mData, SIGNAL(signalSaveGame()),
		this, SIGNAL(signalSaveGame()));
 connect(mData, SIGNAL(signalLoadGame()),
		this, SIGNAL(signalLoadGame()));
 connect(mData, SIGNAL(signalQuicksaveGame()),
		this, SLOT(slotQuicksaveGame()));
 connect(mData, SIGNAL(signalQuickloadGame()),
		this, SLOT(slotQuickloadGame()));
 connect(mData, SIGNAL(signalCenterHomeBase()),
		this, SLOT(slotCenterHomeBase()));
 connect(mData, SIGNAL(signalSyncNetwork()),
		this, SLOT(slotSyncNetwork()));
 connect(mData, SIGNAL(signalSelectSelectionGroup(int)),
		this, SIGNAL(signalSelectSelectionGroup(int)));
 connect(mData, SIGNAL(signalCreateSelectionGroup(int)),
		this, SIGNAL(signalCreateSelectionGroup(int)));
 connect(mData, SIGNAL(signalShowSelectionGroup(int)),
		this, SIGNAL(signalShowSelectionGroup(int)));

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
 connect(mData, SIGNAL(signalEditorUndo()),
		this, SIGNAL(signalEditorUndo()));
 connect(mData, SIGNAL(signalEditorRedo()),
		this, SIGNAL(signalEditorRedo()));

 mData->initUfoActions(mGameMode);
 mData->setLocalPlayerIO(mPlayerIO);
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
 // AB: UPDATE: player.h is allowed here
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

 camera()->changeDistance(-delta);
}

void BosonMenuInput::slotZoomOut()
{
 BO_CHECK_NULL_RET(camera());

 float delta = 5;

 camera()->changeDistance(delta);
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

void BosonMenuInput::slotDebugEditPlayerInputs(Player* p)
{
 BO_CHECK_NULL_RET(p);
 KDialogBase* dialog = new KDialogBase(KDialogBase::Plain, i18n("Player Inputs"),
		KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok, 0,
		"playerinputs", true, true);
 connect(dialog, SIGNAL(finished()), dialog, SLOT(deleteLater()));
 QVBoxLayout* layout = new QVBoxLayout(dialog->plainPage());

 BoEditPlayerInputsWidget* widget = new BoEditPlayerInputsWidget(dialog->plainPage());
 layout->addWidget(widget);

 widget->setPlayer(p);

 connect(widget, SIGNAL(signalAddMenuInput()),
		this, SIGNAL(signalDebugAddMenuInput()));
 connect(widget, SIGNAL(signalAddedLocalPlayerInput()),
		this, SIGNAL(signalDebugAddedLocalPlayerInput()));

 dialog->show();
}

void BosonMenuInput::slotDebugKillPlayer(Q_UINT32 playerId)
{
 QByteArray b;
 QDataStream stream(b, IO_WriteOnly);
 stream << (Q_UINT32)playerId;
 boGame->sendMessage(b, BosonMessageIds::IdKillPlayer);
}

void BosonMenuInput::slotDebugModifyMinerals(Q_UINT32 playerId, int amount)
{
 QByteArray b;
 QDataStream stream(b, IO_WriteOnly);
 stream << (Q_UINT32)playerId;
 stream << (Q_INT32)amount;
 boGame->sendMessage(b, BosonMessageIds::IdModifyMinerals);
}

void BosonMenuInput::slotDebugModifyOil(Q_UINT32 playerId, int amount)
{
 QByteArray b;
 QDataStream stream(b, IO_WriteOnly);
 stream << (Q_UINT32)playerId;
 stream << (Q_INT32)amount;
 boGame->sendMessage(b, BosonMessageIds::IdModifyOil);
}

void BosonMenuInput::slotToggleCheating(bool on)
{
 if (!actionCollection()) {
	return;
 }
 actionCollection()->setActionEnabled("debug_explore", on);
 actionCollection()->setActionEnabled("debug_unfog", on);
 actionCollection()->setActionEnabled("debug_fog", on);
 actionCollection()->setActionEnabled("debug_players", on);
}

void BosonMenuInput::slotExploreAll(Player* pl)
{
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
 QPtrList<Player> list;
 if (!pl) {
	list = boGame->allPlayerList();
 } else {
	list.append(pl);
 }
 for (unsigned int i = 0; i < list.count(); i++) {
	Player* p = list.at(i);
	for (unsigned int x = 0; x < map->width(); x++) {
		for (unsigned int y = 0; y < map->height(); y++) {
			p->explore(x, y);
		}
	}
	boGame->slotAddChatSystemMessage(i18n("Debug"), i18n("Explored player %1 - %2").arg(p->bosonId()).arg(p->name()));
 }
}

void BosonMenuInput::slotUnfogAll(Player* pl)
{
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
 QPtrList<Player> list;
 if (!pl) {
	list = boGame->allPlayerList();
 } else {
	list.append(pl);
 }
 for (unsigned int i = 0; i < list.count(); i++) {
	Player* p = list.at(i);
	for (unsigned int x = 0; x < map->width(); x++) {
		for (unsigned int y = 0; y < map->height(); y++) {
			p->addFogRef(x, y);
		}
	}
	boGame->slotAddChatSystemMessage(i18n("Debug"), i18n("Unfogged player %1 - %2").arg(p->bosonId()).arg(p->name()));
 }
}

void BosonMenuInput::slotFogAll(Player* pl)
{
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
 QPtrList<Player> list;
 if (!pl) {
	list = boGame->allPlayerList();
 } else {
	list.append(pl);
 }
 for (unsigned int i = 0; i < list.count(); i++) {
	Player* p = list.at(i);
	for (unsigned int x = 0; x < map->width(); x++) {
		for (unsigned int y = 0; y < map->height(); y++) {
			p->removeFogRef(x, y);
		}
	}
	boGame->slotAddChatSystemMessage(i18n("Debug"), i18n("Fogged player %1 - %2").arg(p->bosonId()).arg(p->name()));
 }
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
				QPtrList<Player> gamePlayerList = boGame->gamePlayerList();
				Player* p = gamePlayerList.at(index);
				e.setAttribute("PlayerId", p->bosonId());
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

void BosonMenuInput::slotReloadMeshRenderer()
{
 bool unusable = false;
 bool r = BoMeshRendererManager::manager()->reloadPlugin(&unusable);
 if (r) {
	return;
 }
 boError() << k_funcinfo << "meshrenderer reloading failed" << endl;
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
 boError() << k_funcinfo << "groundrenderer reloading failed" << endl;
 if (unusable) {
	KMessageBox::sorry(0, i18n("Reloading groundrenderer failed, library is now unusable. quitting."));
	exit(1);
 } else {
	KMessageBox::sorry(0, i18n("Reloading groundrenderer failed but library should still be usable"));
 }
}

void BosonMenuInput::slotDebugMemory()
{
#ifdef BOSON_USE_BOMEMORY
 boDebug() << k_funcinfo << endl;
 BoMemoryDialog* dialog = new BoMemoryDialog(0);
 connect(dialog, SIGNAL(finished()), dialog, SLOT(deleteLater()));
 boDebug() << k_funcinfo << "update data" << endl;
 dialog->slotUpdate();
 dialog->show();
 boDebug() << k_funcinfo << "done" << endl;
#endif
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

void BosonMenuInput::slotEndGame()
{
 QTimer::singleShot(0, this, SIGNAL(signalEndGame()));
}

void BosonMenuInput::slotQuicksaveGame()
{
 QTimer::singleShot(0, this, SIGNAL(signalQuicksaveGame()));
}

void BosonMenuInput::slotQuickloadGame()
{
 QTimer::singleShot(0, this, SIGNAL(signalQuickloadGame()));
}

void BosonMenuInput::slotEditorSavePlayFieldAs()
{
 boDebug() << k_funcinfo << endl;
 QString startIn; // shall we provide this??
 QString fileName = BoFileDialog::getSaveFileName(startIn, "*.bpf", 0);
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

 BPFDescription* modifiedDescription = new BPFDescription(*boGame->playField()->description());
 boGame->playField()->setModifiedDescription(modifiedDescription);

// TODO: non-modal might be fine. one could use that for translations (one
// dialog the original language, one the translated language)
 BPFDescriptionDialog* dialog = new BPFDescriptionDialog(0, true);
 dialog->setDescription(modifiedDescription);
 dialog->exec();

 delete dialog;
}

void BosonMenuInput::slotEditorEditPlayerMinerals()
{
 BO_CHECK_NULL_RET(playerIO());
 BO_CHECK_NULL_RET(playerIO()->player());
 Player* localPlayer = playerIO()->player();
 bool ok = false;
 QString value = QString::number(playerIO()->minerals());
 QIntValidator val(this);
 val.setBottom(0);
 val.setTop(1000000); // we need to set a top, because int is limited. this should be enough, i hope (otherwise feel free to increase)
 value = KLineEditDlg::getText(i18n("Minerals for player %1").arg(playerIO()->name()), value, &ok, 0, &val);
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
 localPlayer->setMinerals(v);
}

void BosonMenuInput::slotEditorEditPlayerOil()
{
 BO_CHECK_NULL_RET(playerIO());
 BO_CHECK_NULL_RET(playerIO()->player());
 Player* localPlayer = playerIO()->player();
 bool ok = false;
 QString value = QString::number(playerIO()->oil());
 QIntValidator val(this);
 val.setBottom(0);
 val.setTop(1000000); // we need to set a top, because int is limited. this should be enough, i hope (otherwise feel free to increase)
 value = KLineEditDlg::getText(i18n("Oil for player %1").arg(playerIO()->name()), value, &ok, 0, &val);
 if (!ok) {
	return;
 }
 boDebug() << k_funcinfo << value << endl;
 unsigned long int v = value.toULong(&ok);
 if (!ok) {
	boWarning() << k_funcinfo << "value " << value << " not valid" << endl;
	return;
 }
 localPlayer->setOil(v);
}

void BosonMenuInput::slotEditorImportHeightMap()
{
 BO_CHECK_NULL_RET(boGame);
 BO_CHECK_NULL_RET(boGame->playField());
 BO_CHECK_NULL_RET(boGame->playField()->map());
 boDebug() << k_funcinfo << endl;
 QString fileName = BoFileDialog::getOpenFileName(QString::null, "*.png", 0);
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
 QString fileName = BoFileDialog::getSaveFileName(QString::null, "*.png", 0);
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
 for (unsigned int i = 0; i < map->groundTheme()->groundTypeCount(); i++) {
	textures.append(map->groundTheme()->groundType(i)->name);
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

 QString fileName = BoFileDialog::getSaveFileName(QString::null, "*.png", 0);
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

void BosonMenuInput::slotEditorHasUndo(const QString& name)
{
 BoUfoAction* undo = actionCollection()->action("edit_undo");
 if (!undo) {
	boDebug() << "no undo action" << endl;
	return;
 }
 if (name.isEmpty()) {
	undo->setEnabled(false);
	undo->setText(BoUfoStdAction::label(BoUfoStdAction::EditUndo));
 } else {
	undo->setEnabled(true);
	undo->setText(i18n("%1: %2").arg(BoUfoStdAction::label(BoUfoStdAction::EditUndo)).arg(name));
 }
}

void BosonMenuInput::slotEditorHasRedo(const QString& name)
{
 BoUfoAction* redo = actionCollection()->action("edit_redo");
 if (!redo) {
	return;
 }
 if (name.isEmpty()) {
	redo->setEnabled(false);
	redo->setText(BoUfoStdAction::label(BoUfoStdAction::EditRedo));
 } else {
	redo->setEnabled(true);
	redo->setText(i18n("%1: %2").arg(BoUfoStdAction::label(BoUfoStdAction::EditRedo)).arg(name));
 }
}

