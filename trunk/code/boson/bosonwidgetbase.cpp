/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "defines.h"
#include "bosonminimap.h"
#include "bosoncanvas.h"
#include "boson.h"
#include "player.h"
#include "unit.h"
#include "bosonmessage.h"
#include "bosonplayfield.h"
#include "bosonmap.h"
#include "bosonscenario.h"
#include "bosonconfig.h"
#include "bosoncursor.h"
#include "bodisplaymanager.h"
#include "bosonbigdisplaybase.h"
#include "bosonbigdisplayinput.h"
#include "editorbigdisplayinput.h"
#include "boselection.h"
#include "global.h"
#include "top.h"
#include "bodebug.h"
#include "bosonprofiling.h"
#include "optionsdialog.h"
#include "boaction.h"
#include "bosonlocalplayerinput.h"
#include "commandframe/bosoncommandframebase.h"
#include "sound/bosonaudiointerface.h"

#include <kapplication.h>
#include <klocale.h>
#include <kaction.h>
#include <kconfig.h>
#include <kpopupmenu.h>
#include <kgame/kgamedebugdialog.h>
#include <kgame/kgamepropertyhandler.h>
#include <kgame/kgamechat.h>

#include <qlayout.h>
#include <qptrlist.h>
#include <qtimer.h>
#include <qptrdict.h>
#include <qsignalmapper.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdatastream.h>
#include <qdom.h>

#include "bosonwidgetbase.moc"

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
		mCommandFrameDock = 0;

		mChat = 0;
		mChatDock = 0;
	}

	BosonCommandFrameBase* mCommandFrame;
	KDockWidget* mCommandFrameDock;

	KGameChat* mChat;
	KDockWidget* mChatDock;

	KActionMenu* mActionDebugPlayers;
	KSelectAction* mActionZoom;
	KToggleAction* mActionChat;
	KToggleAction* mActionCmdFrame;

	QPtrDict<KPlayer> mPlayers; // needed for debug only

	bool mInitialized;
};

BosonWidgetBase::BosonWidgetBase(TopWidget* top, QWidget* parent)
    : QWidget( parent, "BosonWidgetBase" ), KXMLGUIClient(/*FIXME: clientParent!*/)
{
 d = new BosonWidgetBasePrivate;
 d->mInitialized = false;
 mTop = top;

 mMiniMap = 0;
 mDisplayManager = 0;
 mCursor = 0;
 mLocalPlayer = 0;
 mLocalPlayerInput = 0;
}

BosonWidgetBase::~BosonWidgetBase()
{
 boDebug() << k_funcinfo << endl;
 if (mDisplayManager) {
	// we do NOT delete the display manager here
	setDisplayManager(0);
 }
 d->mPlayers.clear();
 // LocalPlayerInput has been deleted already
 mLocalPlayerInput = 0;
 if (mTop && mTop->factory()) {
	mTop->factory()->removeClient(this);
 }
 delete d->mCommandFrameDock;
 delete d->mChatDock;

 delete mCursor;

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
 }
}

BosonCanvas* BosonWidgetBase::canvas() const
{
 return mTop->canvas();
}

#include <kstandarddirs.h> //locate()
void BosonWidgetBase::init()
{
 // NOTE: order of init* methods is very important here, so don't change it,
 //  unless you know what you're doing!
 if (d->mInitialized) {
	return;
 }
 d->mInitialized = true;
 initMiniMap();
 initChat();
 initCommandFrame();
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
}

void BosonWidgetBase::initMap()
{
 boDebug() << k_funcinfo << endl;

 if (!boGame->playField()) {
	boError() << k_funcinfo << "NULL playfield" << endl;
	return;
 }
 BosonMap* map = boGame->playField()->map();
 canvas()->setMap(map);
 minimap()->setMap(map);
 minimap()->initMap();
 displayManager()->mapChanged();
// boGame->setPlayField(playField()); // already done on startup in BosonStarting

 // AB: note that this meets the name "initMap" only slightly. We can't do this
 // when players are initialized, as the map must be known to them once we start
 // loading the units (for *loading* games)
 for (unsigned int i = 0; i < boGame->playerCount(); i++) {
	boDebug() << "init map for player " << i << endl;
	Player* p = (Player*)boGame->playerList()->at(i);
	if (p) {
		p->initMap(map, boGame->gameMode());
	}
 }

}

void BosonWidgetBase::initMiniMap()
{
 mMiniMap = new BosonMiniMap(0);
 minimap()->hide();
 minimap()->setBackgroundOrigin(WindowOrigin);

 connect(canvas(), SIGNAL(signalUnitMoved(Unit*, float, float)),
		minimap(), SLOT(slotMoveUnit(Unit*, float, float)));
 connect(canvas(), SIGNAL(signalUnitRemoved(Unit*)),
		minimap(), SLOT(slotUnitDestroyed(Unit*)));
}

void BosonWidgetBase::initConnections()
{
 connect(canvas(), SIGNAL(signalUnitRemoved(Unit*)),
		this, SLOT(slotRemoveUnit(Unit*)));

 connect(boGame, SIGNAL(signalAddUnit(Unit*, int, int)),
		canvas(), SLOT(slotAddUnit(Unit*, int, int)));
 connect(boGame, SIGNAL(signalAddUnit(Unit*, int, int)),
		this, SLOT(slotAddUnit(Unit*, int, int)));

 connect(boGame, SIGNAL(signalLoadExternalStuff(QDataStream&)),
		this, SLOT(slotLoadExternalStuff(QDataStream&)));
 connect(boGame, SIGNAL(signalSaveExternalStuff(QDataStream&)),
		this, SLOT(slotSaveExternalStuff(QDataStream&)));
 connect(boGame, SIGNAL(signalLoadExternalStuffFromXML(const QDomElement&)),
		this, SLOT(slotLoadExternalStuffFromXML(const QDomElement&)));
 connect(boGame, SIGNAL(signalSaveExternalStuffAsXML(QDomElement&)),
		this, SLOT(slotSaveExternalStuffAsXML(QDomElement&)));

 connect(boGame, SIGNAL(signalAddChatSystemMessage(const QString&, const QString&)),
		this, SLOT(slotAddChatSystemMessage(const QString&, const QString&)));

 connect(boGame, SIGNAL(signalInitFogOfWar()),
		this, SLOT(slotInitFogOfWar()));
}

void BosonWidgetBase::initDisplayManager()
{
#warning do NOT do these connections here!
 // dont do the connect()s here, as some objects might not be deleted and
 // therefore we do the same connect twice if an endgame() occurs!
 connect(mDisplayManager, SIGNAL(signalSelectionChanged(BoSelection*)),
		cmdFrame(), SLOT(slotSelectionChanged(BoSelection*)));
 connect(mDisplayManager, SIGNAL(signalChangeActiveViewport(
		const QPoint&, const QPoint&, const QPoint&, const QPoint&)),
		minimap(), SLOT(slotMoveRect(
		const QPoint&, const QPoint&, const QPoint&, const QPoint&)));
 connect(minimap(), SIGNAL(signalReCenterView(const QPoint&)),
		mDisplayManager, SLOT(slotReCenterActiveDisplay(const QPoint&)));
 connect(cmdFrame(), SIGNAL(signalSelectUnit(Unit*)),
		mDisplayManager, SLOT(slotActiveSelectSingleUnit(Unit*)));
 connect(boGame, SIGNAL(signalAdvance(unsigned int, bool)),
		mDisplayManager, SLOT(slotAdvance(unsigned int, bool)));

 displayManager()->setLocalPlayer(localPlayer()); // this does nothing.

 connect(localPlayer(), SIGNAL(signalUnitChanged(Unit*)),
		mDisplayManager, SLOT(slotUnitChanged(Unit*)));
}

void BosonWidgetBase::addInitialDisplay()
{
 displayManager()->addInitialDisplay();

 // we need to add the display first (in order to create a valid GL context) and
 // then load the cursor
 slotChangeCursor(boConfig->cursorMode(), boConfig->cursorDir());
}

void BosonWidgetBase::initChat()
{
 // note: we can use the chat widget even for editor mode, e.g. for status
 // messages!
 d->mChatDock = mTop->createDockWidget("chat_dock", 0, 0, i18n("Chat"));
 d->mChatDock->setEnableDocking(KDockWidget::DockTop | KDockWidget::DockBottom);
 d->mChatDock->setDockSite(KDockWidget::DockNone);
 d->mChat = new KGameChat(boGame, BosonMessage::IdChat, d->mChatDock);
 d->mChatDock->setWidget(d->mChat);

 connect(d->mChatDock, SIGNAL(iMBeingClosed()), this, SLOT(slotChatDockHidden()));
 connect(d->mChatDock, SIGNAL(hasUndocked()), this, SLOT(slotChatDockHidden()));
}

void BosonWidgetBase::initPlayer()
{
 boDebug() << k_funcinfo << endl;
 if (!localPlayer()) {
	boError() << k_funcinfo << "NULL local player" << endl;
	return;
 }

 setLocalPlayerRecursively(localPlayer());

 connect(localPlayer(), SIGNAL(signalPropertyChanged(KGamePropertyBase*, KPlayer*)),
		this, SLOT(slotPlayerPropertyChanged(KGamePropertyBase*, KPlayer*)));

 // Needed for loading game
 emit signalMineralsUpdated(localPlayer()->minerals());
 emit signalOilUpdated(localPlayer()->oil());
 slotUnitCountChanged(localPlayer());
}

void BosonWidgetBase::initLocalPlayerInput()
{
 mLocalPlayerInput = new BosonLocalPlayerInput();
 localPlayerInput()->setCommandFrame(cmdFrame());

 localPlayer()->removeGameIO(localPlayerInput(), false); // in case it was added before
 localPlayer()->addGameIO(localPlayerInput());

 connect(localPlayerInput(), SIGNAL(signalAction(const BoSpecificAction&)),
		mDisplayManager, SLOT(slotAction(const BoSpecificAction&)));
}

void BosonWidgetBase::initGameMode()//FIXME: rename! we don't have a difference to initEditorMode anymore. maybe just initGame() or so??
{
 BO_CHECK_NULL_RET(displayManager());

 initLocalPlayerInput();

 // Init all bigdisplays
 QPtrListIterator<BosonBigDisplayBase> it(*displayManager()->displayList());
 while (it.current()) {
	initBigDisplay(it.current());
	++it;
 }

 initLayout();
 startScenarioAndGame();
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
	i->setLocalPlayerInput(localPlayerInput());
	b->setDisplayInput(i);
 } else {
	EditorBigDisplayInput* i = new EditorBigDisplayInput(b);
	i->setLocalPlayerInput(localPlayerInput());
	b->setDisplayInput(i);
 }
 connect(b->displayInput(), SIGNAL(signalLockAction(bool)),
		mDisplayManager, SIGNAL(signalLockAction(bool)));
 b->setCanvas(canvas());
 b->setLocalPlayer(localPlayer()); // AB: this will also add the mouseIO!
 b->setCursor(mCursor);
 b->setKGameChat(d->mChat);

 b->show();
 b->makeActive();

 b->setInputInitialized(true);
}

void BosonWidgetBase::initCommandFrame()
{
 d->mCommandFrameDock = mTop->createDockWidget("cmdframe_dock", 0, 0, i18n("Command Frame"));
 d->mCommandFrameDock->setEnableDocking(KDockWidget::DockLeft | KDockWidget::DockRight);
 d->mCommandFrameDock->setDockSite(KDockWidget::DockNone);
 d->mCommandFrame = createCommandFrame(d->mCommandFrameDock);
 d->mCommandFrameDock->setWidget(d->mCommandFrame);
 d->mCommandFrame->reparentMiniMap(minimap());

 connect(d->mCommandFrameDock, SIGNAL(iMBeingClosed()), this, SLOT(slotCmdFrameDockHidden()));
 connect(d->mCommandFrameDock, SIGNAL(hasUndocked()), this, SLOT(slotCmdFrameDockHidden()));
}

void BosonWidgetBase::initLayout()
{
 boDebug() << k_funcinfo << endl;

 QVBoxLayout* topLayout = new QVBoxLayout(this);
 topLayout->addWidget(displayManager());

 d->mCommandFrameDock->manualDock(mTop->getMainDockWidget(), KDockWidget::DockLeft, 30);
 d->mChatDock->manualDock(mTop->getMainDockWidget(), KDockWidget::DockBottom, 80);

 if (!kapp->config()->hasGroup("BosonGameDock")) {
	boDebug() << k_funcinfo << "dock config does not exist" << endl;
	// Dock config isn't saved (probably first start). Hide chat dock (we only
	//  show commandframe by default)
	d->mChatDock->changeHideShowState();
	displayManager()->updateGeometry();  // Hack? Bug in BoDisplayManager?
 }
 else {
	boDebug() << k_funcinfo << "dock config exists, loading" << endl;
	mTop->loadGameDockConfig();
 }
 checkDockStatus();
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

void BosonWidgetBase::slotAddUnit(Unit* unit, int, int)
{
 if (!unit) {
	boError() << k_funcinfo << "NULL unit" << endl;
	return;
 }
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

void BosonWidgetBase::slotRemoveUnit(Unit* unit)
{
 if (unit->owner() != localPlayer()) {
	return;
 }

 slotUnitCountChanged(unit->owner());
}

void BosonWidgetBase::slotFog(int x, int y)
{
 // very time critical function!!

 // slotFog() and slotUnfog() exist here so that we need only a single slot
 // instead of two (on ein minimap and one to actually create/remove the fog)
 // should save some performance (at least I hope)
 minimap()->slotFog(x, y); // FIXME: no need for slot
}

void BosonWidgetBase::slotUnfog(int x, int y)
{
 minimap()->slotUnfog(x, y); // FIXME: no need for slot
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

void BosonWidgetBase::slotInitFogOfWar()
{
 if (boGame->gameMode()) {
	minimap()->initFogOfWar(localPlayer());
 } else {
	minimap()->initFogOfWar(0);
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

void BosonWidgetBase::slotAddChatSystemMessage(const QString& fromName, const QString& text)
{
 // add a chat system-message *without* sending it over network (makes no sense
 // for system messages)
 d->mChat->addSystemMessage(fromName, text);

 displayManager()->addChatMessage(i18n("--- %1: %2").arg(fromName).arg(text));
}

void BosonWidgetBase::slotSetCommandButtonsPerRow(int b)
{
 cmdFrame()->slotSetButtonsPerRow(b);
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
 bool minimapUpdatesEnabled = minimap()->isUpdatesEnabled();
 minimap()->setUpdatesEnabled(false);
 for (unsigned int i = 0; i < list.count(); i++) {
	Player* p = (Player*)list.at(i);
	for (unsigned int x = 0; x < map->width(); x++) {
		for (unsigned int y = 0; y < map->height(); y++) {
			p->unfog(x, y);
		}
	}
	boGame->slotAddChatSystemMessage(i18n("Debug"), i18n("Unfogged player %1 - %2").arg(p->id()).arg(p->name()));
 }
 minimap()->setUpdatesEnabled(minimapUpdatesEnabled);
 minimap()->update();
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
 a = new KAction(i18n("Scroll Up"), Qt::Key_Up, scrollMapper,
		SLOT(map()), actionCollection(),
		"scroll_up");
 scrollMapper->setMapping(a, BoDisplayManager::ScrollUp);
 a = new KAction(i18n("Scroll Down"), Qt::Key_Down, scrollMapper,
		SLOT(map()), actionCollection(),
		"scroll_down");
 scrollMapper->setMapping(a, BoDisplayManager::ScrollDown);
 a = new KAction(i18n("Scroll Left"), Qt::Key_Left, scrollMapper,
		SLOT(map()), actionCollection(),
		"scroll_left");
 scrollMapper->setMapping(a, BoDisplayManager::ScrollLeft);
 a = new KAction(i18n("Scroll Right"), Qt::Key_Right, scrollMapper,
		SLOT(map()), actionCollection(),
		"scroll_right");
 scrollMapper->setMapping(a, BoDisplayManager::ScrollRight);

 // FIXME: the editor should not have a "game" menu, so what to do with this?
 (void)new KAction(i18n("&Reset View Properties"), KShortcut(Qt::Key_R),
		displayManager(), SLOT(slotResetViewProperties()), actionCollection(), "game_reset_view_properties");

 // Dockwidgets show/hide
 d->mActionChat = new KToggleAction(i18n("Show &Chat"),
		KShortcut(Qt::CTRL+Qt::Key_C), this, SLOT(slotToggleChatVisible()),
		actionCollection(), "options_show_chat");
 d->mActionCmdFrame = new KToggleAction(i18n("Show C&ommandframe"),
		KShortcut(Qt::CTRL+Qt::Key_F), this, SLOT(slotToggleCmdFrameVisible()),
		actionCollection(), "options_show_cmdframe");

 (void)new KAction(i18n("&Grab Screenshot"), KShortcut(Qt::CTRL + Qt::Key_G),
		this, SLOT(slotGrabScreenshot()), actionCollection(), "game_grab_screenshot");
 (void)new KAction(i18n("Grab &Profiling data"), KShortcut(Qt::CTRL + Qt::Key_P),
		this, SLOT(slotGrabProfiling()), actionCollection(), "game_grab_profiling");

 // Debug
 (void)new KAction(i18n("&Unfog"), KShortcut(), this,
		SLOT(slotUnfogAll()), actionCollection(), "debug_unfog");
 KToggleAction* mapCoordinates = new KToggleAction(i18n("Debug &map coordinates"),
		KShortcut(), 0, 0, actionCollection(), "debug_map_coordinates");
 mapCoordinates->setChecked(false);
 connect(mapCoordinates, SIGNAL(toggled(bool)),
		this, SLOT(slotSetDebugMapCoordinates(bool)));
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

 cheating->setChecked(DEFAULT_CHEAT_MODE);
 slotToggleCheating(DEFAULT_CHEAT_MODE);
 checkDockStatus();
}

void BosonWidgetBase::quitGame()
{
// this needs to be done first, before the players are removed
 boDebug() << k_funcinfo << endl;
 canvas()->deleteDestroyed();
 boGame->quitGame();
 boDebug() << k_funcinfo << "done" << endl;
}

bool BosonWidgetBase::isCmdFrameVisible() const
{
 return d->mCommandFrameDock->isVisible();
}

bool BosonWidgetBase::isChatVisible() const
{
 return d->mChatDock->isVisible();
}

void BosonWidgetBase::setCmdFrameVisible(bool visible)
{
 if (visible && d->mCommandFrameDock->mayBeShow()) {
	d->mCommandFrameDock->show();
 } else if (! visible && d->mCommandFrameDock->mayBeHide()) {
	d->mCommandFrameDock->hide();
 }
}

void BosonWidgetBase::setChatVisible(bool visible)
{
 if (visible && d->mChatDock->mayBeShow()) {
	d->mChatDock->show();
 } else if (! visible && d->mChatDock->mayBeHide()) {
	d->mChatDock->hide();
 }
}

void BosonWidgetBase::slotToggleCmdFrameVisible()
{
 d->mCommandFrameDock->changeHideShowState();
 checkDockStatus();
}

void BosonWidgetBase::slotToggleChatVisible()
{
 d->mChatDock->changeHideShowState();
 checkDockStatus();
}

void BosonWidgetBase::checkDockStatus()
{
 d->mActionChat->setChecked(isChatVisible());
 d->mActionCmdFrame->setChecked(isCmdFrameVisible());
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
 if (boGame->loadingStatus() != Boson::LoadingCompleted) {
	displayManager()->slotCenterHomeBase();
 }

 #warning FIXME
 // this is a strange bug: we need to resize the widget once it is shown - otherwise we'll have a VERY slot frame rate.
 // I can't find out where the problem resides :-(
 //QTimer::singleShot(500, this, SLOT(slotHack1()));
}

void BosonWidgetBase::slotDebugMode(int index)
{
 boConfig->setDebugMode((BosonConfig::DebugMode)index);
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
}

void BosonWidgetBase::slotCmdFrameDockHidden()
{
 d->mActionCmdFrame->setChecked(false);
}

void BosonWidgetBase::setBosonXMLFile()
{
 setXMLFile(locate("config", "ui/ui_standards.rc", instance()));
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

void BosonWidgetBase::setLocalPlayer(Player* p, bool init) //AB: probably init = false is obsolete!
{
 // FIXME: ensure that d->mCmdInput gets removed in BosonWidget before calling
 // this! or maybe while calling this!
 setLocalPlayerRecursively(p);

 if (init) {
	if (!p) {
		boDebug() << k_funcinfo << "NULL player" << endl;
		
		return;
	}
	initPlayer();
 }
}

void BosonWidgetBase::setLocalPlayerRecursively(Player* p)
{
 mLocalPlayer = p;
 if (minimap()) {
	minimap()->setLocalPlayer(localPlayer());
 }
 if (displayManager()) {
	displayManager()->setLocalPlayer(localPlayer());
 }
 if (!boGame) {
	boError() << k_funcinfo << "NULL game object" << endl;
	return;
 }
 boGame->setLocalPlayer(localPlayer());
 if (cmdFrame()) {
	cmdFrame()->setLocalPlayer(localPlayer());
 }
 if (d->mChat) {
	d->mChat->setFromPlayer(localPlayer());
 }
}

void BosonWidgetBase::slotGrabScreenshot()
{
 boDebug() << k_funcinfo << "Taking screenshot!" << endl;

#warning FIXME
 // this should NOT do a reference to mTop!
 // use either parent() or a signal (i.e. move the code to TopWidget)
 QPixmap shot = QPixmap::grabWindow(mTop->winId());
 QString file = findSaveFileName("boson", "jpg");
 if (file.isNull()) {
	boWarning() << k_funcinfo << "Can't find free filename???" << endl;
	return;
 }
 // TODO: chat message about file location!
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

void BosonWidgetBase::slotLoadExternalStuff(QDataStream& stream)
{
 boDebug() << k_funcinfo << endl;
 // TODO: load camera
 // TODO: load unitgroups
}

void BosonWidgetBase::slotSaveExternalStuff(QDataStream& stream)
{
 boDebug() << k_funcinfo << endl;
 // TODO: save camera  (BosonBigDisplayBase?)
 // TODO: save unitgroups  (BoDisplayManager?)
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
 connect(dlg, SIGNAL(signalApply()),
		this, SLOT(slotApplyOptions()));

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
 minimap()->repaint(); // it automatically uses the new scaling factor
 displayManager()->slotUpdateIntervalChanged(boConfig->updateInterval()); // FIXME: no slot anymore
 displayManager()->setToolTipCreator(boConfig->toolTipCreator());
 displayManager()->setToolTipUpdatePeriod(boConfig->toolTipUpdatePeriod());
}

void BosonWidgetBase::slotSetDebugMapCoordinates(bool debug)
{
 boConfig->setDebugMapCoordinates(debug);
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

