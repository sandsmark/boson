/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2004 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bosonwidgetbase.h"
#include "bosonwidgetbase.moc"

#include "defines.h"
#include "bosoncanvas.h"
#include "boson.h"
#include "bosonsaveload.h"
#include "player.h"
#include "unit.h"
#include "bosonmessage.h"
#include "bosonplayfield.h"
#include "bosonmap.h"
#include "bosonconfig.h"
#include "bosoncursor.h"
#include "bodisplaymanager.h"
#include "bosonbigdisplaybase.h"
#include "bosonbigdisplayinput.h"
#include "editorbigdisplayinput.h"
#include "boselection.h"
#include "global.h"
#include "bodebug.h"
#include "bosonprofiling.h"
#include "optionsdialog.h"
#include "boaction.h"
#include "bosonlocalplayerinput.h"
#include "bosoncomputerio.h"
#include "bosonmodeltextures.h"
#include "commandframe/bosoncommandframebase.h"
#include "sound/bosonaudiointerface.h"
#include "script/bosonscript.h"
#include "bosonwidgets/bogamechat.h"
#include "bosonpath.h"
#include "bomeshrenderermanager.h"
#include "bogroundrenderermanager.h"
#include "boglstatewidget.h"
#include "boconditionwidget.h"
#ifdef BOSON_USE_BOMEMORY
#include "bomemory/bomemorydialog.h"
#endif

#include <kapplication.h>
#include <klocale.h>
#include <kaction.h>
#include <kconfig.h>
#include <kpopupmenu.h>
#include <kgame/kgamedebugdialog.h>
#include <kgame/kgamepropertyhandler.h>
#include <klineedit.h>
#include <kdockwidget.h>
#include <kmessagebox.h>

#include <qlayout.h>
#include <qptrlist.h>
#include <qtimer.h>
#include <qptrdict.h>
#include <qsignalmapper.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdatastream.h>
#include <qdom.h>
#include <qimage.h>

#include <stdlib.h>

#define ID_DEBUG_KILLPLAYER 0
#define ID_DEBUG_ADD_10000_MINERALS 1
#define ID_DEBUG_ADD_1000_MINERALS 2
#define ID_DEBUG_SUB_1000_MINERALS 3
#define ID_DEBUG_ADD_10000_OIL 4
#define ID_DEBUG_ADD_1000_OIL 5
#define ID_DEBUG_SUB_1000_OIL 6

class BosonWidgetBase::BosonWidgetBasePrivate
{
public:
	BosonWidgetBasePrivate()
	{
		mCommandFrame = 0;
		mChat = 0;

		mActionDebugPlayers = 0;
		mActionZoom = 0;
		mActionChat = 0;
		mActionCmdFrame = 0;

		mScript = 0;

		mCanvas = 0;
	}

	BosonCommandFrameBase* mCommandFrame;
	BoGameChatWidget* mChat;

	KActionMenu* mActionDebugPlayers;
	KSelectAction* mActionZoom;
	KToggleAction* mActionChat;
	KToggleAction* mActionCmdFrame;

	QPtrDict<KPlayer> mPlayers; // needed for debug only

	bool mInitialized;

	BosonScript* mScript;

	BosonCanvas* mCanvas;
};

BosonWidgetBase::BosonWidgetBase(QWidget* parent)
    : QWidget( parent, "BosonWidgetBase" )
{
 d = new BosonWidgetBasePrivate;
 d->mInitialized = false;

 mDisplayManager = 0;
 mCursor = 0;
 mLocalPlayer = 0;
}

BosonWidgetBase::~BosonWidgetBase()
{
 boDebug() << k_funcinfo << endl;
 if (factory()) {
	// remove the bosonwidget-specific menus from the XML GUI (menubar,
	// toolbar, ...)
	factory()->removeClient(this);
 }
 if (mDisplayManager) {
	// we do NOT delete the display manager here
	setDisplayManager(0);
 }
 d->mPlayers.clear();

 delete mCursor;
 delete d->mCommandFrame;
 delete d->mChat;

 delete d;
 boDebug() << k_funcinfo << "done" << endl;
}

void BosonWidgetBase::setDisplayManager(BoDisplayManager* displayManager)
{
 if (mDisplayManager) {
	mDisplayManager->hide();
	mDisplayManager->reparent(0, QPoint(0, 0)); // we do NOT own the display manager!
 }
 mDisplayManager = displayManager;
 if (mDisplayManager) {
	if (mDisplayManager->parent()) {
		boError() << k_funcinfo << "the displaymanager already has a parent - reparenting..." << endl;
	}
	mDisplayManager->reparent(this, QPoint(0, 0));
	mDisplayManager->show();

	changeToConfigCursor();
 }
}

BosonCanvas* BosonWidgetBase::canvas() const
{
 return d->mCanvas;
}

#include <kstandarddirs.h> //locate()
void BosonWidgetBase::init(KDockWidget* chatDock, KDockWidget* commandFrameDock)
{
 // NOTE: order of init* methods is very important here, so don't change it,
 //  unless you know what you're doing!
 if (d->mInitialized) {
	return;
 }
 d->mInitialized = true;
 initChat(chatDock);
 initCommandFrame(commandFrameDock);
 initDisplayManager();

 initConnections();
 actionCollection()->setWidget(this); // needs to be called *before* initKActions()
 initKActions();
 // XMLClient stuff. needs to be called *after* initKActions().
 setBosonXMLFile();
// setXML(top()->domDocument().toString());
 setXMLGUIBuildDocument(QDomDocument());
 // XMLClient stuff ends. note that there is a factory()->addClient() in
 // TopWidget!

 setFocusPolicy(StrongFocus); // accept key event
// setFocus(); // nonsense, since its still hidden

 initPlayersMenu();

 BosonScript::setGame(boGame);
}

void BosonWidgetBase::initConnections()
{
 connect(boGame, SIGNAL(signalLoadExternalStuffFromXML(const QDomElement&)),
		this, SLOT(slotLoadExternalStuffFromXML(const QDomElement&)));
 connect(boGame, SIGNAL(signalSaveExternalStuffAsXML(QDomElement&)),
		this, SLOT(slotSaveExternalStuffAsXML(QDomElement&)));

 connect(boGame, SIGNAL(signalAddChatSystemMessage(const QString&, const QString&, const Player*)),
		this, SLOT(slotAddChatSystemMessage(const QString&, const QString&, const Player*)));
}

void BosonWidgetBase::initDisplayManager()
{
#warning do NOT do these connections here!
 // dont do the connect()s here, as some objects might not be deleted and
 // therefore we do the same connect twice if an endgame() occurs!
 connect(mDisplayManager, SIGNAL(signalSelectionChanged(BoSelection*)),
		cmdFrame(), SLOT(slotSelectionChanged(BoSelection*)));
 connect(cmdFrame(), SIGNAL(signalSelectUnit(Unit*)),
		mDisplayManager, SLOT(slotActiveSelectSingleUnit(Unit*)));
 connect(boGame, SIGNAL(signalAdvance(unsigned int, bool)),
		mDisplayManager, SLOT(slotAdvance(unsigned int, bool)));
 connect(boGame, SIGNAL(signalAdvance(unsigned int, bool)),
		this, SLOT(slotAdvance(unsigned int, bool)));

 displayManager()->setLocalPlayerIO(localPlayerIO()); // this does nothing.

 connect(localPlayer(), SIGNAL(signalUnitChanged(Unit*)),
		mDisplayManager, SLOT(slotUnitChanged(Unit*)));
}


void BosonWidgetBase::initChat(KDockWidget* chatDock)
{
 // note: we can use the chat widget even for editor mode, e.g. for status
 // messages!
 d->mChat = new BoGameChatWidget(chatDock, "chatwidget", boGame, BosonMessage::IdChat);
 connect(d->mChat->chatWidget(), SIGNAL(signalScriptCommand(const QString&)),
		this, SLOT(slotRunScriptLine(const QString&)));
 chatDock->setWidget(d->mChat);

 connect(chatDock, SIGNAL(iMBeingClosed()), this, SLOT(slotChatDockHidden()));
 connect(chatDock, SIGNAL(hasUndocked()), this, SLOT(slotChatDockHidden()));
}

void BosonWidgetBase::initPlayer()
{
 boDebug() << k_funcinfo << endl;
 if (!localPlayer()) {
	boError() << k_funcinfo << "NULL local player" << endl;
	return;
 }

 connect(localPlayer(), SIGNAL(signalPropertyChanged(KGamePropertyBase*, KPlayer*)),
		this, SLOT(slotPlayerPropertyChanged(KGamePropertyBase*, KPlayer*)));

 // Needed for loading game
 emit signalMineralsUpdated(localPlayer()->minerals());
 emit signalOilUpdated(localPlayer()->oil());
 slotUnitCountChanged(localPlayer());
}

void BosonWidgetBase::initGameMode()//FIXME: rename! we don't have a difference to initEditorMode anymore. maybe just initGame() or so??
{
 BO_CHECK_NULL_RET(displayManager());

 // Init all bigdisplays
 QPtrListIterator<BosonBigDisplayBase> it(*displayManager()->displayList());
 while (it.current()) {
	initBigDisplay(it.current());
	++it;
 }

 initLayout();
 startScenarioAndGame();

 initScripts();

#ifdef PATHFINDER_TNG
 // FIXME: this isn't correct I suppose. But atm it's only used for debugging
 //  anyway (and I don't intend to use it for anything else)
 boDebug() << k_funcinfo << "Trying searching sample path" << endl;
 BosonPathInfo i;
 i.start = QPoint(5 * BO_TILE_SIZE, 5 * BO_TILE_SIZE);
 i.dest = QPoint(45 * BO_TILE_SIZE, 35 * BO_TILE_SIZE);
 boDebug() << k_funcinfo << "Let's go!" << endl;
 canvas()->pathfinder()->findPath(&i);
 boDebug() << k_funcinfo << "sample path searching complete" << endl;
#endif
}

void BosonWidgetBase::initBigDisplay(BosonBigDisplayBase* b)
{
 if (!b) {
	boError() << k_funcinfo << "NULL display" << endl;
	return;
 }
 if (b->isInputInitialized()) {
	// Already initialized
	return;
 }
 BO_CHECK_NULL_RET(boGame);
 if (boGame->gameMode()) {
	BosonBigDisplayInput* i = new BosonBigDisplayInput(b);
	b->setDisplayInput(i);
 } else {
	EditorBigDisplayInput* i = new EditorBigDisplayInput(b);
	b->setDisplayInput(i);
 }
 connect(b->displayInput(), SIGNAL(signalLockAction(bool)),
		mDisplayManager, SIGNAL(signalLockAction(bool)));
 b->setCanvas(canvas());

 // FIXME: this should be done by this->setLocalPlayer(), NOT here!
 // (setLocalPlayer() is also called when changing player in editor mode)
 b->setLocalPlayerIO(localPlayer()->playerIO()); // AB: this will also add the mouseIO!

 b->setCursor(mCursor);
 b->setKGameChat(d->mChat->chatWidget());

 b->show();
 b->makeActive();

 b->setInputInitialized(true);
}

void BosonWidgetBase::initCommandFrame(KDockWidget* commandFrameDock)
{
 d->mCommandFrame = createCommandFrame(commandFrameDock);
 commandFrameDock->setWidget(d->mCommandFrame);

 connect(commandFrameDock, SIGNAL(iMBeingClosed()), this, SLOT(slotCmdFrameDockHidden()));
 connect(commandFrameDock, SIGNAL(hasUndocked()), this, SLOT(slotCmdFrameDockHidden()));
}

void BosonWidgetBase::initLayout()
{
 boDebug() << k_funcinfo << endl;

 QVBoxLayout* topLayout = new QVBoxLayout(this);
 topLayout->addWidget(displayManager());

 emit signalLoadBosonGameDock();
}

void BosonWidgetBase::changeCursor(BosonCursor* cursor)
{
 if (!cursor) {
	boError() << k_funcinfo << "NULL cursor" << endl;
	return;
 }
 delete mCursor;
 mCursor = cursor;
 displayManager()->setCursor(mCursor);
}

void BosonWidgetBase::slotHack1()
{
 QSize size = displayManager()->activeDisplay()->size();
 displayManager()->activeDisplay()->resize(size.width() - 1, size.height() - 1);
 displayManager()->activeDisplay()->resize(size);
}

void BosonWidgetBase::slotItemAdded(BosonItem* item)
{
 if (!item) {
	boError() << k_funcinfo << "NULL item" << endl;
	return;
 }
 if (!RTTI::isUnit(item->rtti())) {
	return;
 }
 Unit* unit = (Unit*)item;
 Player* p = unit->owner();
 if (!p) {
	boError() << k_funcinfo << "NULL owner" << endl;
	return;
 }
 if (p != localPlayer()) {
	return;
 }

 slotUnitCountChanged(p);
}

void BosonWidgetBase::slotUnitRemoved(Unit* unit)
{
 if (unit->owner() != localPlayer()) {
	return;
 }

 slotUnitCountChanged(unit->owner());
}

void BosonWidgetBase::slotPlayerPropertyChanged(KGamePropertyBase* prop, KPlayer* p)
{
 if (p != localPlayer()) {
	// not yet used
	return;
 }
 switch (prop->id()) {
	case Player::IdMinerals:
		emit signalMineralsUpdated(localPlayer()->minerals());
		break;
	case Player::IdOil:
		emit signalOilUpdated(localPlayer()->oil());
		break;
	default:
		break;
 }
}

bool BosonWidgetBase::sound() const
{
 return boAudio->sound();
}

bool BosonWidgetBase::music() const
{
 return boAudio->music();
}

void BosonWidgetBase::slotToggleSound()
{
 boAudio->setSound(!boAudio->sound());
 boConfig->setSound(boAudio->sound());
}

void BosonWidgetBase::slotToggleMusic()
{
 boAudio->setMusic(!boAudio->music());
 boConfig->setMusic(boAudio->music());
}

void BosonWidgetBase::slotAddChatSystemMessage(const QString& fromName, const QString& text, const Player* forPlayer)
{
 if (forPlayer && forPlayer != localPlayer()) {
	return;
 }
 // add a chat system-message *without* sending it over network (makes no sense
 // for system messages)
 d->mChat->chatWidget()->addSystemMessage(fromName, text);

 displayManager()->addChatMessage(i18n("--- %1: %2").arg(fromName).arg(text));
}

void BosonWidgetBase::slotUnfogAll(Player* pl)
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
}

void BosonWidgetBase::slotSplitDisplayHorizontal()
{
 initBigDisplay(displayManager()->splitActiveDisplayHorizontal());
}

void BosonWidgetBase::slotSplitDisplayVertical()
{
 initBigDisplay(displayManager()->splitActiveDisplayVertical());
}

void BosonWidgetBase::slotRemoveActiveDisplay()
{
 displayManager()->removeActiveDisplay();
}

void BosonWidgetBase::slotCmdBackgroundChanged(const QString& file)
{
 if (file.isNull()) {
	cmdFrame()->unsetPalette();
	return;
 }
 QPixmap p(file);
 if (p.isNull()) {
	boError() << k_funcinfo << "Could not load " << file << endl;
	cmdFrame()->unsetPalette();
	return;
 }
 cmdFrame()->setPaletteBackgroundPixmap(p);
}

void BosonWidgetBase::initKActions()
{
 QSignalMapper* scrollMapper = new QSignalMapper(this);
 connect(scrollMapper, SIGNAL(mapped(int)), displayManager(), SLOT(slotScroll(int)));
 KAction* a;
 KShortcut scrollUp(Qt::Key_Up);
 scrollUp.append(KKeySequence(KKey(Qt::Key_W)));
 a = new KAction(i18n("Scroll Up"), scrollUp, scrollMapper,
		SLOT(map()), actionCollection(),
		"scroll_up");
 scrollMapper->setMapping(a, BoDisplayManager::ScrollUp);
 KShortcut scrollDown(Qt::Key_Down);
 scrollDown.append(KKeySequence(KKey(Qt::Key_S)));
 a = new KAction(i18n("Scroll Down"), scrollDown, scrollMapper,
		SLOT(map()), actionCollection(),
		"scroll_down");
 scrollMapper->setMapping(a, BoDisplayManager::ScrollDown);
 KShortcut scrollLeft(Qt::Key_Left);
 scrollLeft.append(KKeySequence(KKey(Qt::Key_A)));
 a = new KAction(i18n("Scroll Left"), scrollLeft, scrollMapper,
		SLOT(map()), actionCollection(),
		"scroll_left");
 scrollMapper->setMapping(a, BoDisplayManager::ScrollLeft);
 KShortcut scrollRight(Qt::Key_Right);
 scrollRight.append(KKeySequence(KKey(Qt::Key_D)));
 a = new KAction(i18n("Scroll Right"), scrollRight, scrollMapper,
		SLOT(map()), actionCollection(),
		"scroll_right");
 scrollMapper->setMapping(a, BoDisplayManager::ScrollRight);
 KShortcut rotateLeft(Qt::Key_Q);
 a = new KAction(i18n("Rotate Left"), rotateLeft, displayManager(),
		SLOT(slotRotateLeft()), actionCollection(),
		"rotate_left");
 KShortcut rotateRight(Qt::Key_E);
 a = new KAction(i18n("Rotate Right"), rotateRight, displayManager(),
		SLOT(slotRotateRight()), actionCollection(),
		"rotate_right");
 KShortcut zoomIn(Qt::Key_F);
 a = new KAction(i18n("Zoom In"), zoomIn, displayManager(),
		SLOT(slotZoomIn()), actionCollection(),
		"zoom_in");
 KShortcut zoomOut(Qt::Key_V);
 a = new KAction(i18n("Zoom out"), zoomOut, displayManager(),
		SLOT(slotZoomOut()), actionCollection(),
		"zoom_out");


 // FIXME: the editor should not have a "game" menu, so what to do with this?
 (void)new KAction(i18n("&Reset View Properties"), KShortcut(Qt::Key_R),
		displayManager(), SLOT(slotResetViewProperties()), actionCollection(), "game_reset_view_properties");

 // Dockwidgets show/hide
 d->mActionChat = new KToggleAction(i18n("Show Cha&t"),
		KShortcut(Qt::CTRL+Qt::Key_C), this, SIGNAL(signalToggleChatVisible()),
		actionCollection(), "options_show_chat");
 d->mActionCmdFrame = new KToggleAction(i18n("Show C&ommandframe"),
		KShortcut(Qt::CTRL+Qt::Key_F), this, SIGNAL(signalToggleCmdFrameVisible()),
		actionCollection(), "options_show_cmdframe");

 (void)new KAction(i18n("&Grab Screenshot"), KShortcut(Qt::CTRL + Qt::Key_G),
		this, SLOT(slotGrabScreenshot()), actionCollection(), "game_grab_screenshot");
 (void)new KAction(i18n("Grab &Profiling data"), KShortcut(Qt::CTRL + Qt::Key_P),
		this, SLOT(slotGrabProfiling()), actionCollection(), "game_grab_profiling");
 KToggleAction* movie = new KToggleAction(i18n("Grab &Movie"),
		KShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_M), 0, 0, actionCollection(), "game_grab_movie");
 movie->setChecked(false);
 connect(movie, SIGNAL(toggled(bool)),
		displayManager(), SLOT(slotSetGrabMovie(bool)));

 // Debug
 KToggleAction* resources = new KToggleAction(i18n("Show resources"),
		KShortcut(), 0, 0, actionCollection(), "show_resources");
 resources->setChecked(true);
 connect(resources, SIGNAL(toggled(bool)),
		this, SLOT(slotSetShowResources(bool)));
 (void)new KAction(i18n("&Unfog"), KShortcut(), this,
		SLOT(slotUnfogAll()), actionCollection(), "debug_unfog");
 KToggleAction* mapCoordinates = new KToggleAction(i18n("Debug &map coordinates"),
		KShortcut(), 0, 0, actionCollection(), "debug_map_coordinates");
 mapCoordinates->setChecked(false);
 connect(mapCoordinates, SIGNAL(toggled(bool)),
		this, SLOT(slotSetDebugMapCoordinates(bool)));
 KToggleAction* PFData = new KToggleAction(i18n("Debug pathfinder data"),
		KShortcut(), 0, 0, actionCollection(), "debug_pf_data");
 PFData->setChecked(false);
 connect(PFData, SIGNAL(toggled(bool)),
		this, SLOT(slotSetDebugPFData(bool)));
 KToggleAction* cellGrid = new KToggleAction(i18n("Show Cell &Grid"),
		KShortcut(), 0, 0, actionCollection(), "debug_cell_grid");
 cellGrid->setChecked(false);
 connect(cellGrid, SIGNAL(toggled(bool)),
		this, SLOT(slotSetDebugShowCellGrid(bool)));
 KToggleAction* matrices = new KToggleAction(i18n("Debug Ma&trices"),
		KShortcut(), 0, 0, actionCollection(), "debug_matrices");
 matrices->setChecked(false);
 connect(matrices, SIGNAL(toggled(bool)),
		this, SLOT(slotSetDebugMatrices(bool)));
 KToggleAction* works = new KToggleAction(i18n("Debug Item works"),
		KShortcut(), 0, 0, actionCollection(), "debug_works");
 works->setChecked(false);
 connect(works, SIGNAL(toggled(bool)),
		this, SLOT(slotSetDebugItemWorks(bool)));
 KToggleAction* camera = new KToggleAction(i18n("Debug camera"),
		KShortcut(), 0, 0, actionCollection(), "debug_camera");
 camera->setChecked(false);
 connect(camera, SIGNAL(toggled(bool)),
		this, SLOT(slotSetDebugCamera(bool)));
 KToggleAction* rendercounts = new KToggleAction(i18n("Debug Rendering counts"),
		KShortcut(), 0, 0, actionCollection(), "debug_rendercounts");
 rendercounts->setChecked(false);
 connect(rendercounts, SIGNAL(toggled(bool)),
		this, SLOT(slotSetDebugRenderCounts(bool)));
 KToggleAction* cheating = new KToggleAction(i18n("Enable &Cheating"),
		KShortcut(), 0, 0, actionCollection(), "debug_enable_cheating");
 connect(cheating, SIGNAL(toggled(bool)), this, SLOT(slotToggleCheating(bool)));
 KToggleAction* wireFrames = new KToggleAction(i18n("Render &Wireframes"),
		KShortcut(), 0, 0, actionCollection(), "debug_wireframes");
 connect(wireFrames, SIGNAL(toggled(bool)), this, SLOT(slotDebugToggleWireFrames(bool)));
 wireFrames->setChecked(false);
 slotDebugToggleWireFrames(false);
 KToggleAction* boundingboxes = new KToggleAction(i18n("Render item's bounding boxes"),
		KShortcut(), 0, 0, actionCollection(), "debug_boundingboxes");
 boundingboxes->setChecked(false);
 connect(boundingboxes, SIGNAL(toggled(bool)),
		this, SLOT(slotSetDebugBoundingBoxes(bool)));
 KToggleAction* fps = new KToggleAction(i18n("Debug FPS"),
		KShortcut(), 0, 0, actionCollection(), "debug_fps");
 fps->setChecked(false);
 connect(fps, SIGNAL(toggled(bool)),
		this, SLOT(slotSetDebugFPS(bool)));
 KToggleAction* debugAdvanceCalls = new KToggleAction(i18n("Debug &Advance calls"),
		KShortcut(), 0, 0, actionCollection(), "debug_advance_calls");
 connect(debugAdvanceCalls, SIGNAL(toggled(bool)), this, SLOT(slotSetDebugAdvanceCalls(bool)));
 (void)new KAction(i18n("&Unfog"), KShortcut(), this,
		SLOT(slotUnfogAll()), actionCollection(), "debug_unfog");
 (void)new KAction(i18n("Dump game &log"), KShortcut(), this,
		SLOT(slotDumpGameLog()), actionCollection(), "debug_gamelog");
 KToggleAction* enablecolormap = new KToggleAction(i18n("Enable colormap"),
		KShortcut(), 0, 0, actionCollection(), "debug_colormap_enable");
 enablecolormap->setChecked(false);
 connect(enablecolormap, SIGNAL(toggled(bool)),
		this, SLOT(slotSetEnableColormap(bool)));
 (void)new KAction(i18n("Edit global conditions"), KShortcut(), this,
		SLOT(slotEditConditions()), actionCollection(),
		"debug_edit_conditions");


 KSelectAction* debugMode = new KSelectAction(i18n("Mode"), KShortcut(),
		actionCollection(), "debug_mode");
 connect(debugMode, SIGNAL(activated(int)), this, SLOT(slotDebugMode(int)));
 QStringList l;
 l.append(i18n("Normal"));
 l.append(i18n("Debug Selection"));
 debugMode->setItems(l);
 debugMode->setCurrentItem(0);
 d->mActionDebugPlayers = new KActionMenu(i18n("Players"),
		actionCollection(), "debug_players");

 (void)new KAction(i18n("Show OpenGL states"), KShortcut(), this,
		SLOT(slotShowGLStates()), actionCollection(),
		"debug_show_opengl_states");
 (void)new KAction(i18n("&Reload model textures"), KShortcut(), this,
		SLOT(slotReloadModelTextures()), actionCollection(), "debug_lazy_reload_model_textures");
 (void)new KAction(i18n("Light0..."), KShortcut(), displayManager(),
		SLOT(slotShowLight0Widget()), actionCollection(),
		"debug_light0");
 (void)new KAction(i18n("Reload &meshrenderer plugin"), KShortcut(), this,
		SLOT(slotReloadMeshRenderer()), actionCollection(),
		"debug_lazy_reload_meshrenderer");
 (void)new KAction(i18n("Reload &groundrenderer plugin"), KShortcut(), this,
		SLOT(slotReloadGroundRenderer()), actionCollection(),
		"debug_lazy_reload_groundrenderer");
 (void)new KAction(i18n("Crash boson"), KShortcut(), this,
		SLOT(slotCrashBoson()), actionCollection(),
		"debug_crash_boson");
#ifdef BOSON_USE_BOMEMORY
 (void)new KAction(i18n("Debug M&emory"), KShortcut(), this,
		SLOT(slotDebugMemory()), actionCollection(),
		"debug_memory");
#endif

 cheating->setChecked(DEFAULT_CHEAT_MODE);
 slotToggleCheating(DEFAULT_CHEAT_MODE);
 emit signalCheckDockStatus();
}

void BosonWidgetBase::quitGame()
{
// this needs to be done first, before the players are removed
 boDebug() << k_funcinfo << endl;
 boGame->quitGame();
 d->mCanvas = 0;
 boDebug() << k_funcinfo << "done" << endl;
}

void BosonWidgetBase::setActionChat(bool chatVisible)
{
 d->mActionChat->setChecked(chatVisible);
}

void BosonWidgetBase::setActionCmdFrame(bool cmdFrameVisible)
{
 d->mActionCmdFrame->setChecked(cmdFrameVisible);
}


void BosonWidgetBase::saveConfig()
{
 // note: the game is *not* saved here! just general settings like game speed,
 // player name, ...
 boDebug() << k_funcinfo << endl;
 if (!boGame) {
	boError() << k_funcinfo << "NULL game" << endl;
	return;
 }
 if (!localPlayer()) {
	boError() << k_funcinfo << "NULL local player" << endl;
	return;
 }

// boConfig->save(editor); //FIXME - what is this for?
 boDebug() << k_funcinfo << "done" << endl;
}

void BosonWidgetBase::startScenarioAndGame()
{
 boDebug() << k_funcinfo << endl;
 // Center home base if new game was started. If game is loaded, camera was
 //  already loaded as well
 // FIXME: this is hackish but I don't know any other way of checking if game
 //  is loaded or new one here. Feel free to improve
 if (boGame->loadingStatus() != BosonSaveLoad::LoadingCompleted) {
	displayManager()->slotCenterHomeBase();
 }
}

void BosonWidgetBase::slotDebugMode(int index)
{
 boConfig->setDebugMode((BosonConfig::DebugMode)index);
}

void BosonWidgetBase::slotDebugMemory()
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

void BosonWidgetBase::initPlayersMenu()
{
 QPtrListIterator<KPlayer> it(*(boGame->playerList()));
 while (it.current()) {
	slotPlayerJoinedGame(it.current());
	++it;
 }
}

void BosonWidgetBase::slotDebugPlayer(int index)
{
 QPtrDictIterator<KPlayer> it(d->mPlayers);
 KPopupMenu* menu = (KPopupMenu*)sender();
 KPlayer* p = 0;
 while (it.current() && !p) {
	KActionMenu* m = (KActionMenu*)it.currentKey();
	if (m->popupMenu() == menu) {
		p = it.current();
	}
	++it;
 }

 if (!p) {
	boError() << k_funcinfo << "player not found" << endl;
	return;
 }

 QByteArray b;
 QDataStream stream(b, IO_WriteOnly);
 stream << (Q_UINT32)p->id();
 switch (index) {
	case ID_DEBUG_KILLPLAYER:
		boGame->sendMessage(b, BosonMessage::IdKillPlayer);
		break;
	case ID_DEBUG_ADD_10000_MINERALS:
		stream << (Q_INT32)10000;
		boGame->sendMessage(b, BosonMessage::IdModifyMinerals);
		break;
	case ID_DEBUG_ADD_1000_MINERALS:
		stream << (Q_INT32)1000;
		boGame->sendMessage(b, BosonMessage::IdModifyMinerals);
		break;
	case ID_DEBUG_SUB_1000_MINERALS:
		stream << (Q_INT32)-1000;
		boGame->sendMessage(b, BosonMessage::IdModifyMinerals);
		break;
	case ID_DEBUG_ADD_1000_OIL:
		stream << (Q_INT32)1000;
		boGame->sendMessage(b, BosonMessage::IdModifyOil);
		break;
	case ID_DEBUG_ADD_10000_OIL:
		stream << (Q_INT32)10000;
		boGame->sendMessage(b, BosonMessage::IdModifyOil);
		break;
	case ID_DEBUG_SUB_1000_OIL:
		stream << (Q_INT32)-1000;
		boGame->sendMessage(b, BosonMessage::IdModifyOil);
		break;
	default:
		boError() << k_funcinfo << "unknown index " << index << endl;
		break;
 }
}

void BosonWidgetBase::slotChatDockHidden()
{
 d->mActionChat->setChecked(false);
 setFocus();
}

void BosonWidgetBase::slotCmdFrameDockHidden()
{
 d->mActionCmdFrame->setChecked(false);
}

void BosonWidgetBase::setBosonXMLFile()
{
 QString file = locate("config", "ui/ui_standards.rc", instance());
 setXMLFile(file);
 setXMLFile("bosonbaseui.rc", true);
}

void BosonWidgetBase::slotPlayerJoinedGame(KPlayer* player)
{
 if (!player) {
	return;
 }
 // note: NOT listed in the *ui.rc files! we create it dynamically when the player enters ; not using the xml framework
 KActionMenu* menu = new KActionMenu(player->name(), this, QString("debug_players_%1").arg(player->name()));

 connect(menu->popupMenu(), SIGNAL(activated(int)),
		this, SLOT(slotDebugPlayer(int)));
 menu->popupMenu()->insertItem(i18n("Kill Player"), ID_DEBUG_KILLPLAYER);
 menu->popupMenu()->insertItem(i18n("Minerals += 10000"), ID_DEBUG_ADD_10000_MINERALS);
 menu->popupMenu()->insertItem(i18n("Minerals += 1000"), ID_DEBUG_ADD_1000_MINERALS);
 menu->popupMenu()->insertItem(i18n("Minerals -= 1000"), ID_DEBUG_SUB_1000_MINERALS);
 menu->popupMenu()->insertItem(i18n("Oil += 10000"), ID_DEBUG_ADD_10000_OIL);
 menu->popupMenu()->insertItem(i18n("Oil += 1000"), ID_DEBUG_ADD_1000_OIL);
 menu->popupMenu()->insertItem(i18n("Oil -= 1000"), ID_DEBUG_SUB_1000_OIL);

 d->mActionDebugPlayers->insert(menu);
 d->mPlayers.insert(menu, player);
}

void BosonWidgetBase::slotPlayerLeftGame(KPlayer* player)
{
 if (!player) {
	return;
 }
 boDebug() << k_funcinfo << endl;
 KActionMenu* menu = 0;
 QPtrDictIterator<KPlayer> it(d->mPlayers);
 for (; it.current() && !menu; ++it) {
	if (it.current() == player) {
		menu = (KActionMenu*)it.currentKey();
	}
 }
 if (!menu) {
	boWarning() << k_funcinfo << "NULL player debug menu" << endl;
	return;
 }
 d->mActionDebugPlayers->remove(menu);
 d->mPlayers.remove(player);
 delete menu;
}

BosonCommandFrameBase* BosonWidgetBase::cmdFrame() const
{
 return d->mCommandFrame;
}

void BosonWidgetBase::setLocalPlayer(Player* p)
{
 if (mLocalPlayer) {
	KGameIO* oldIO = mLocalPlayer->findRttiIO(BosonLocalPlayerInput::LocalPlayerInputRTTI);
	if (oldIO) {
		mLocalPlayer->removeGameIO(oldIO);
	}
 }
 mLocalPlayer = p;
 if (mLocalPlayer) {
	BosonLocalPlayerInput* input = new BosonLocalPlayerInput();
	connect(input, SIGNAL(signalAction(const BoSpecificAction&)),
			mDisplayManager, SLOT(slotAction(const BoSpecificAction&)));
	mLocalPlayer->addGameIO(input);
	if (!cmdFrame()) {
		boError() << k_funcinfo << "cmdFrame() is NULL - this should not be possible here!" << endl;
	} else {
		input->setCommandFrame(cmdFrame());
	}
 }
 if (displayManager()) {
	displayManager()->setLocalPlayerIO(localPlayer()->playerIO());
 }
 if (!boGame) {
	boError() << k_funcinfo << "NULL game object" << endl;
	return;
 }
 if (cmdFrame()) {
	cmdFrame()->setLocalPlayer(localPlayer());
 }
 if (d->mChat) {
	d->mChat->chatWidget()->setFromPlayer(localPlayer());
 }
}

void BosonWidgetBase::slotGrabScreenshot()
{
 BO_CHECK_NULL_RET(displayManager());
 BO_CHECK_NULL_RET(displayManager()->activeDisplay());
 boDebug() << k_funcinfo << "Taking screenshot!" << endl;

// QImage image = displayManager()->activeDisplay()->screenShot();
 QPixmap shot = QPixmap::grabWindow(parentWidget()->winId());
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

void BosonWidgetBase::slotGrabProfiling()
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

QString BosonWidgetBase::findSaveFileName(const QString& prefix, const QString& suffix)
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

void BosonWidgetBase::slotUnitCountChanged(Player* p)
{
 emit signalMobilesCount(p->mobilesCount());
 emit signalFacilitiesCount(p->facilitiesCount());
}

void BosonWidgetBase::slotToggleCheating(bool on)
{
 setActionEnabled("debug_unfog", on);
 setActionEnabled("debug_players", on);
}

void BosonWidgetBase::slotDebugToggleWireFrames(bool on)
{
 boConfig->setWireFrames(on);
}

void BosonWidgetBase::setActionEnabled(const char* name, bool on)
{
 KAction* a = actionCollection()->action(name);
 if (!a) {
	boError() << k_funcinfo << "NULL " << name << " action" << endl;
 } else {
	a->setEnabled(on);
 }
}

void BosonWidgetBase::slotLoadExternalStuffFromXML(const QDomElement& root)
{
 boDebug() << k_funcinfo << endl;
 // TODO: load camera
 // TODO: load unitgroups
 displayManager()->loadFromXML(root);
}

void BosonWidgetBase::slotSaveExternalStuffAsXML(QDomElement& root)
{
 boDebug() << k_funcinfo << endl;
 // TODO: save camera  (BosonBigDisplayBase?)
 // TODO: save unitgroups  (BoDisplayManager?)
 displayManager()->saveAsXML(root);
}

OptionsDialog* BosonWidgetBase::gamePreferences(bool editor)
{
 OptionsDialog* dlg = new OptionsDialog(editor, this);
 dlg->setGame(boGame);
 dlg->setPlayer(localPlayer());
 dlg->slotLoad();

 connect(dlg, SIGNAL(finished()), dlg, SLOT(deleteLater())); // seems not to be called if you quit with "cancel"!

 connect(dlg, SIGNAL(signalCursorChanged(int, const QString&)),
		this, SLOT(slotChangeCursor(int, const QString&)));
 connect(dlg, SIGNAL(signalCmdBackgroundChanged(const QString&)),
		this, SLOT(slotCmdBackgroundChanged(const QString&)));
 connect(dlg, SIGNAL(signalOpenGLSettingsUpdated()),
		displayManager(), SLOT(slotUpdateOpenGLSettings()));
 connect(dlg, SIGNAL(signalApply()),
		this, SLOT(slotApplyOptions()));
 connect(dlg, SIGNAL(signalFontChanged(const BoFontInfo&)),
		displayManager(), SLOT(slotChangeFont(const BoFontInfo&)));

 return dlg;
}

void BosonWidgetBase::slotApplyOptions()
{
 // apply all options from boConfig to boson, that need to be applied. all
 // options that are stored in boConfig only don't need to be touched.
 // AB: cursor is still a special case and not handled here.
 // AB: FIXME: cmdbackground is not yet stored in boConfig! that option should
 // be managed here!
 boDebug() << k_funcinfo << endl;
 displayManager()->slotUpdateIntervalChanged(boConfig->updateInterval()); // FIXME: no slot anymore
 displayManager()->setToolTipCreator(boConfig->toolTipCreator());
 displayManager()->setToolTipUpdatePeriod(boConfig->toolTipUpdatePeriod());
}

void BosonWidgetBase::slotSetDebugMapCoordinates(bool debug)
{
 boConfig->setDebugMapCoordinates(debug);
}

void BosonWidgetBase::slotSetDebugPFData(bool debug)
{
 boConfig->setDebugPFData(debug);
}

void BosonWidgetBase::slotSetDebugShowCellGrid(bool debug)
{
 boConfig->setDebugShowCellGrid(debug);
}

void BosonWidgetBase::slotSetDebugMatrices(bool debug)
{
 boConfig->setDebugOpenGLMatrices(debug);
}

void BosonWidgetBase::slotSetDebugItemWorks(bool debug)
{
 boConfig->setDebugItemWorkStatistics(debug);
}

void BosonWidgetBase::slotSetDebugCamera(bool debug)
{
 boConfig->setDebugOpenGLCamera(debug);
}

void BosonWidgetBase::slotSetDebugRenderCounts(bool debug)
{
 boConfig->setDebugRenderCounts(debug);
}

void BosonWidgetBase::slotSetDebugBoundingBoxes(bool debug)
{
 boConfig->setDebugBoundingBoxes(debug);
}

void BosonWidgetBase::slotSetDebugFPS(bool debug)
{
 boConfig->setDebugFPS(debug);
}

void BosonWidgetBase::slotSetDebugAdvanceCalls(bool debug)
{
 boConfig->setDebugAdvanceCalls(debug);
}

void BosonWidgetBase::slotSetShowResources(bool show)
{
 boConfig->setShowResources(show);
}

void BosonWidgetBase::slotSetEnableColormap(bool enable)
{
 boConfig->setEnableColormap(enable);
}

void BosonWidgetBase::slotRunScriptLine(const QString& line)
{
 d->mScript->execLine(line);
}

void BosonWidgetBase::slotAdvance(unsigned int, bool)
{
 d->mScript->advance();
}

void BosonWidgetBase::initScripts()
{
 // Init computer player scripts
 QPtrListIterator<KPlayer> it(*boGame->playerList());
 for (; it.current(); ++it) {
	QPtrListIterator<KGameIO> ioit(*it.current()->ioList());
	for (; ioit.current(); ++ioit) {
		if (ioit.current()->rtti() == KGameIO::ComputerIO) {
			BosonComputerIO* io = (BosonComputerIO*)ioit.current();
			io->initScript();
		}
	}
 }

 // Init script for local player
 d->mScript = BosonScript::newScriptParser(BosonScript::Python, localPlayer());
 // No script will be loaded
 d->mScript->loadScript(locate("data", "boson/scripts/localplayer-script.py"));
 d->mScript->init();
}

void BosonWidgetBase::slotDumpGameLog()
{
 boGame->saveGameLogs("boson");
}

void BosonWidgetBase::slotReloadModelTextures()
{
 BO_CHECK_NULL_RET(BosonModelTextures::modelTextures());
 BosonModelTextures::modelTextures()->reloadTextures();
}

void BosonWidgetBase::slotReloadMeshRenderer()
{
 bool unusable = false;
 bool r = BoMeshRendererManager::manager()->reloadPlugin(&unusable);
 if (r) {
	return;
 }
 boError() << "meshrenderer reloading failed" << endl;
 if (unusable) {
	KMessageBox::sorry(this, i18n("Reloading meshrenderer failed, library is now unusable. quitting."));
	exit(1);
 } else {
	KMessageBox::sorry(this, i18n("Reloading meshrenderer failed but library should still be usable"));
 }
}

void BosonWidgetBase::slotReloadGroundRenderer()
{
 bool unusable = false;
 bool r = BoGroundRendererManager::manager()->reloadPlugin(&unusable);
 if (r) {
	return;
 }
 boError() << "groundrenderer reloading failed" << endl;
 if (unusable) {
	KMessageBox::sorry(this, i18n("Reloading groundrenderer failed, library is now unusable. quitting."));
	exit(1);
 } else {
	KMessageBox::sorry(this, i18n("Reloading groundrenderer failed but library should still be usable"));
 }
}

void BosonWidgetBase::slotShowGLStates()
{
 boDebug() << k_funcinfo << endl;
 BoGLStateWidget* w = new BoGLStateWidget(0, 0, WDestructiveClose);
 w->show();
}

void BosonWidgetBase::changeToConfigCursor()
{
 slotChangeCursor(boConfig->cursorMode(), boConfig->cursorDir());
}

void BosonWidgetBase::setCanvas(BosonCanvas* canvas)
{
 d->mCanvas = canvas;
 connect(d->mCanvas, SIGNAL(signalUnitRemoved(Unit*)),
		this, SLOT(slotUnitRemoved(Unit*)));
 connect(d->mCanvas, SIGNAL(signalItemAdded(BosonItem*)),
		this, SLOT(slotItemAdded(BosonItem*)));

 BosonScript::setCanvas(d->mCanvas);
}

void BosonWidgetBase::initMap()
{
 // implemented by EditorWidget
}

void BosonWidgetBase::slotCrashBoson()
{
 ((QObject*)0)->name();
}

PlayerIO* BosonWidgetBase::localPlayerIO() const
{
 if (localPlayer()) {
	return localPlayer()->playerIO();
 }
 return 0;
}

void BosonWidgetBase::slotEditConditions()
{
 KDialogBase* dialog = new KDialogBase(KDialogBase::Plain, i18n("Conditions"),
		KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Cancel, 0,
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
		KMessageBox::information(this, i18n("Canvas conditions could not be imported to the widget"));
	} else {
		widget->loadConditions(root);
	}
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
		KMessageBox::sorry(this, i18n("Oops - an invalid XML document was created. Internal error."));
		return;
	}
	boDebug() << k_funcinfo << "applying canvas conditions" << endl;
	boGame->loadCanvasConditions(root);
 }
}

