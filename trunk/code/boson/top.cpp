/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include "top.h"
#include "top.moc"

#include "bosonwidget.h"
#include "bosonwelcomewidget.h"
#include "bosonnewgamewidget.h"
#include "bosonserveroptionswidget.h"
#include "bosonloadingwidget.h"
#include "bosonmusic.h"
#include "bosonconfig.h"
#include "boson.h"
#include "player.h"
#include "bosonplayfield.h"
#include "bosoncanvas.h"
#include "bosonmessage.h"
#include "bosonmap.h"
#include "speciestheme.h"

#include <kapplication.h>
#include <klocale.h>
#include <kstatusbar.h>
#include <kstdaction.h>
#include <kstdgameaction.h>
#include <kaction.h>
#include <kdebug.h>
#include <qwidgetstack.h>
#include <qtimer.h>
#include <kkeydialog.h>
#include <qwmatrix.h>
#include <qhbox.h>
#include <qptrdict.h>
#include <kpopupmenu.h>

#define ID_DEBUG_KILLPLAYER 0

class TopWidget::TopWidgetPrivate
{
public:
	TopWidgetPrivate() {
		 mWelcome = 0l;
		mNewGame = 0l;
		mServeroptions = 0l;
		mLoading = 0l;
		mBosonwidget = 0l;
	};

	BosonWelcomeWidget* mWelcome;
	BosonNewGameWidget* mNewGame;
	BosonServerOptionsWidget* mServeroptions;
	BosonLoadingWidget* mLoading;
	BosonWidget* mBosonwidget;

	KAction* mActionNewGame;
	KAction* mActionEndGame;
	KAction* mActionQuit;
	KAction* mActionKeys;
	KAction* mActionPreferences;
	KToggleAction* mActionStatusbar;
	KToggleAction* mActionChat;
	KToggleAction* mActionCmdFrame;
	KToggleAction* mActionSound;
	KToggleAction* mActionMusic;
	KAction* mActionDebug;
	KAction* mActionUnfog;
	KSelectAction* mActionDebugMode;
	KActionMenu* mActionDebugPlayers;
	KSelectAction* mActionZoom;
	KAction* mActionSplitH;
	KAction* mActionSplitV;
	KAction* mActionRemoveView;
	KToggleAction* mActionFullScreen;

	QPtrDict<KPlayer> mPlayers; // needed for debug only
};

TopWidget::TopWidget() : KDockMainWindow(0, "topwindow")
{
 d = new TopWidgetPrivate;
 mGame = false;
 mMainDock = createDockWidget("mainDock1", 0, this, i18n("Map"));
 mWs = new QWidgetStack(mMainDock);
 mMainDock->setWidget(mWs);
 mMainDock->setDockSite(KDockWidget::DockCorner);
 mMainDock->setEnableDocking(KDockWidget::DockNone);

 setView(mMainDock);
 setMainDockWidget(mMainDock);

 BosonConfig::initBosonConfig();

 initMusic();
 initBoson();
 initPlayer();
 initMap();

 setMinimumWidth(640);
 setMinimumHeight(480);

 initActions();
 enableGameActions(false);
 initStatusBar();
 showWelcomeWidget();
}

TopWidget::~TopWidget()
{
}

void TopWidget::saveProperties(KConfig *config)
{
 // the 'config' object points to the session managed
 // config file.  anything you write here will be available
 // later when this app is restored
 if (!config) {
	return;
 }
    
}

void TopWidget::readProperties(KConfig *config)
{
 // the 'config' object points to the session managed
 // config file.  this function is automatically called whenever
 // the app is being restored.  read in here whatever you wrote
 // in 'saveProperties'
 if (!config) {
	return;
 }
}

void TopWidget::initActions()
{
 // Main actions: Game start/end and quit
 d->mActionNewGame = KStdGameAction::gameNew(this, SLOT(slotNewGame()), actionCollection());
 d->mActionEndGame = KStdGameAction::end(this, SLOT(slotEndGame()), actionCollection());
 d->mActionQuit = KStdGameAction::quit(this, SLOT(slotQuit()), actionCollection());

 // Settings
 d->mActionKeys = KStdAction::keyBindings(this, SLOT(slotConfigureKeys()), actionCollection());
 d->mActionPreferences = KStdAction::preferences(this, SLOT(slotGamePreferences()), actionCollection());
 d->mActionStatusbar = KStdAction::showStatusbar(this, SLOT(slotToggleStatusbar()), actionCollection());

 // Dockwidgets show/hide
 d->mActionChat = new KToggleAction(i18n("Show &Chat"),
		KShortcut(Qt::CTRL+Qt::Key_C), this, SLOT(slotToggleChat()),
		actionCollection(), "options_show_chat");
 d->mActionCmdFrame = new KToggleAction(i18n("Show C&ommandframe"),
		KShortcut(Qt::CTRL+Qt::Key_F), this, SLOT(slotToggleCmdFrame()),
		actionCollection(), "options_show_cmdframe");

 // Sound & Music
 d->mActionSound = new KToggleAction(i18n("Soun&d"), 0, this,
		SLOT(slotToggleSound()), actionCollection(), "options_sound");
 d->mActionSound->setChecked(boMusic->sound());
 d->mActionMusic = new KToggleAction(i18n("&Music"), 0, this,
		SLOT(slotToggleMusic()), actionCollection(), "options_music");
 d->mActionMusic->setChecked(boMusic->music());

 // Debug - no i18n!
 d->mActionDebug = new KAction("Debug", KShortcut(), this,
		SLOT(slotDebug()), actionCollection(), "debug_kgame");
 d->mActionUnfog = new KAction("Unfog", KShortcut(), this,
		SLOT(slotUnfogAll()), actionCollection(), "debug_unfog");
 d->mActionDebugMode = new KSelectAction("Mode", KShortcut(), actionCollection(), "debug_mode");
 connect(d->mActionDebugMode, SIGNAL(activated(int)), this, SLOT(slotDebugMode(int)));
 QStringList l;
 l.append("Normal");
 l.append("Debug Selection");
 d->mActionDebugMode->setItems(l);
 d->mActionDebugMode->setCurrentItem(0);
 d->mActionDebugPlayers = new KActionMenu("Players", actionCollection(), "debug_players");

 // Zoom
 d->mActionZoom = new KSelectAction(i18n("&Zoom"), KShortcut(), actionCollection(), "options_zoom");
 connect(d->mActionZoom, SIGNAL(activated(int)), this, SLOT(slotZoom(int)));
 QStringList items;
 items.append(QString::number(50));
 items.append(QString::number(100));
 items.append(QString::number(150));
 d->mActionZoom->setItems(items);

 // Display
 // note: the icons for these action need to have konqueror installed!
 d->mActionSplitH = new KAction(i18n( "Split Display &Left/Right"), "view_left_right",
		   CTRL+SHIFT+Key_L, this, SLOT(slotSplitDisplayHorizontal()),
		   actionCollection(), "splitviewh");
 d->mActionSplitV = new KAction(i18n("Split Display &Top/Bottom"), "view_top_bottom",
		   CTRL+SHIFT+Key_T, this, SLOT(slotSplitDisplayVertical()),
		   actionCollection(), "splitviewv");
 d->mActionRemoveView = new KAction(i18n("&Remove Active Display"), "view_remove",
		  CTRL+SHIFT+Key_R, this, SLOT(slotRemoveActiveDisplay()),
		  actionCollection(), "removeview");
 d->mActionFullScreen = new KToggleAction(i18n("&Fullscreen Mode"), CTRL+SHIFT+Key_F,
		this, SLOT(slotToggleFullScreen()), actionCollection(), "window_fullscreen");
 d->mActionFullScreen->setChecked(false);

 createGUI();
}

void TopWidget::initMusic()
{
 // IMO it's too early for this here, but Player::loadTheme() crashes when it's
 //  not done here
// kdDebug() << k_funcinfo << "BEGIN" << endl;
 BosonMusic::initBosonMusic();
 boMusic->setSound(boConfig->sound());
 boMusic->setMusic(boConfig->music());
// kdDebug() << k_funcinfo << "END" << endl;
}

void TopWidget::initBoson()
{
 mBoson = new Boson(this);

 // new games are handled in this order: ADMIN clicks on start games - this
 // sends an IdStartGame over network. Once this is received signalStartGame()
 // is emitted and we start here
 connect(mBoson, SIGNAL(signalStartNewGame()), this, SLOT(slotStartGame()));
}

void TopWidget::initPlayer()
{
 mPlayer = new Player;
}

void TopWidget::initMap()
{
 mMap = new BosonPlayField(this);
}

void TopWidget::initStatusBar()
{
 KStatusBar* bar = statusBar();
 QHBox* box = new QHBox(bar);
 (void)new QLabel(i18n("Mobiles: "), box);
 QLabel* mobilesLabel = new QLabel(QString::number(0), box);
 connect(this, SIGNAL(signalSetMobilesCount(int)), mobilesLabel, SLOT(setNum(int)));
 (void)new QLabel(i18n("Facilities: "), box);
 QLabel* facilitiesLabel = new QLabel(QString::number(0), box);
 connect(this, SIGNAL(signalSetFacilitiesCount(int)), facilitiesLabel, SLOT(setNum(int)));
 bar->addWidget(box);

 QHBox* resources = new QHBox(bar);
 (void)new QLabel(i18n("Minerals: "), resources);
 QLabel* mineralLabel = new QLabel(QString::number(0), resources);
 connect(this, SIGNAL(signalMineralsUpdated(int)), mineralLabel, SLOT(setNum(int)));
 (void)new QLabel(i18n("Oil: "), resources);
 QLabel* oilLabel = new QLabel(QString::number(0), resources);
 connect(this, SIGNAL(signalOilUpdated(int)), oilLabel, SLOT(setNum(int)));
 bar->addWidget(resources);

 bar->hide();
}

void TopWidget::enableGameActions(bool enable)
{
 if(enable && ! d->mBosonwidget) {
	kdWarning() << k_lineinfo << "NULL BosonWidget!" << endl;
 }
 d->mActionEndGame->setEnabled(enable);
 d->mActionPreferences->setEnabled(enable);
 d->mActionStatusbar->setEnabled(enable);
 d->mActionChat->setEnabled(enable);
 d->mActionCmdFrame->setEnabled(enable);
 d->mActionDebug->setEnabled(enable);
 d->mActionUnfog->setEnabled(enable);
 d->mActionDebugMode->setEnabled(enable);
 d->mActionDebugPlayers->setEnabled(enable);
 d->mActionZoom->setEnabled(enable);
 d->mActionSplitH->setEnabled(enable);
 d->mActionSplitV->setEnabled(enable);
 d->mActionRemoveView->setEnabled(enable);
}

void TopWidget::initDebugPlayersMenu()
{
 QPtrListIterator<KPlayer> it(*(mBoson->playerList()));
 while (it.current()) {
	// note: NOT listed in the *ui.rc files! we create it dynamically when the player enters ; not using the xml framework
	KActionMenu* menu = new KActionMenu(it.current()->name(), this, QString("debug_players_%1").arg(it.current()->name()));

	connect(menu->popupMenu(), SIGNAL(activated(int)),
			this, SLOT(slotDebugPlayer(int)));
	menu->popupMenu()->insertItem("Kill Player", ID_DEBUG_KILLPLAYER);

	d->mActionDebugPlayers->insert(menu);
	d->mPlayers.insert(menu, it.current());

 ++it;
 }
}

void TopWidget::initWelcomeWidget()
{
 if(d->mWelcome) {
	return;
 }
 d->mWelcome = new BosonWelcomeWidget(mWs);
 connect(d->mWelcome, SIGNAL(signalNewGame()), this, SLOT(slotNewGame()));
 connect(d->mWelcome, SIGNAL(signalQuit()), this, SLOT(slotQuit()));
 mWs->addWidget(d->mWelcome, 1);
}

void TopWidget::showWelcomeWidget()
{
 if(!d->mWelcome) {
	initWelcomeWidget();
 }
 mWs->raiseWidget(1);
}

void TopWidget::initNewGameWidget()
{
 if(d->mNewGame) {
	return;
 }
 d->mNewGame = new BosonNewGameWidget(this, mWs);
 connect(d->mNewGame, SIGNAL(signalCancelled()), this, SLOT(slotShowMainMenu()));
 connect(d->mNewGame, SIGNAL(signalShowServerOptions()), this, SLOT(slotShowServerOptions()));
 mWs->addWidget(d->mNewGame, 2);
}

void TopWidget::showNewGameWidget()
{
 if(!d->mNewGame) {
	initNewGameWidget();
 }
 mWs->raiseWidget(2);
}

void TopWidget::initBosonWidget()
{
 if(d->mBosonwidget) {
	return;
 }
 d->mBosonwidget = new BosonWidget(this, mWs);
 mWs->addWidget(d->mBosonwidget, 3);
}

void TopWidget::showBosonWidget()
{
 if(!d->mBosonwidget) {
	initBosonWidget();
 }
 mWs->raiseWidget(3);
}

void TopWidget::initServerOptions()
{
 if(d->mServeroptions) {
	return;
 }
 d->mServeroptions = new BosonServerOptionsWidget(this, mWs);
 connect(d->mServeroptions, SIGNAL(signalOkClicked()), this, SLOT(slotHideServerOptions()));
 mWs->addWidget(d->mServeroptions, 4);
}

void TopWidget::showServerOptions()
{
 if(!d->mServeroptions) {
	initServerOptions();
 }
 mWs->raiseWidget(4);
}

void TopWidget::initLoadingWidget()
{
 if(d->mLoading) {
	return;
 }
 d->mLoading = new BosonLoadingWidget(mWs);
 mWs->addWidget(d->mLoading, 5);
}

void TopWidget::showLoadingWidget()
{
 if(!d->mLoading) {
	initLoadingWidget();
 }
 mWs->raiseWidget(5);
}

void TopWidget::slotNewGame()
{
 showNewGameWidget();
}

void TopWidget::slotQuit()
{
// FIXME saveGameDockConfig() is just wrong here. we have to ensure that there
// is actually a game running (not newgame widget or so) and it also has to be
// called when the player clicks e.g. on the "x", not only when menu->quit is
// clicked
 saveGameDockConfig();
 // First delete all widgets to give them change to save config
 if(d->mBosonwidget) {
	d->mBosonwidget->slotEndGame();
	delete d->mBosonwidget;
 }
 if(d->mLoading) {
	// TODO: cancel loading
	delete d->mLoading;
 }
 if(d->mServeroptions) {
	delete d->mServeroptions;
 }
 if(d->mNewGame) {
	delete d->mNewGame;
 }
 delete d->mWelcome;
 kapp->quit();
}

void TopWidget::slotStartGame()
{
 showLoadingWidget();
 loadGameData1();
}

void TopWidget::slotShowMainMenu()
{
 disconnect(d->mNewGame);
 delete d->mNewGame;
 d->mNewGame = 0;
 // Delete all players to remove added AI players, then re-init local player
 mBoson->removeAllPlayers();
 initPlayer();
 showWelcomeWidget();
}

void TopWidget::slotShowServerOptions()
{
 showServerOptions();
}

void TopWidget::slotHideServerOptions()
{
 disconnect(d->mServeroptions);
 delete d->mServeroptions;
 d->mServeroptions = 0;
 d->mNewGame->slotSetAdmin(mBoson->isAdmin());
 showNewGameWidget();
}

void TopWidget::loadGameData1()
{
 d->mLoading->setSteps(5000);
 d->mLoading->setProgress(0);
 checkEvents();
 // Receive map (first send it if we're admin)
 connect(mBoson, SIGNAL(signalInitMap(const QByteArray&)), this, SLOT(slotReceiveMap(const QByteArray&)));
 if(mBoson->isAdmin()) {
	d->mLoading->setLoading(BosonLoadingWidget::SendMap);
	checkEvents();
	QByteArray buffer;
	QDataStream stream(buffer, IO_WriteOnly);
	mMap->saveMap(stream);
	d->mLoading->setProgress(50);
	checkEvents();
	// send the loaded map via network
	mBoson->sendMessage(stream, BosonMessage::InitMap);
	d->mLoading->setProgress(100);
	checkEvents();
 }
 d->mLoading->setLoading(BosonLoadingWidget::ReceiveMap);
 checkEvents();
}

void TopWidget::loadGameData2()
{
 // Init some data structures
 d->mLoading->setLoading(BosonLoadingWidget::InitClasses);
 checkEvents();
 mCanvas = new BosonCanvas(this);
 mBoson->setCanvas(mCanvas);
 // Init BosonWidget
 initBosonWidget();

 // Load map tiles. This takes most time
 d->mLoading->setProgress(600);
 d->mLoading->setLoading(BosonLoadingWidget::LoadTiles);
 checkEvents();
 connect(mCanvas, SIGNAL(signalTilesLoading(int)), this, SLOT(slotCanvasTilesLoading(int)));
 connect(mCanvas, SIGNAL(signalTilesLoaded()), this, SLOT(slotCanvasTilesLoaded()));
 checkEvents();
 mCanvas->loadTiles(QString("earth"));
}

void TopWidget::loadGameData3()
{
 // Load unit pixmaps
 d->mLoading->setProgress(3000);
 d->mLoading->setLoading(BosonLoadingWidget::LoadUnits);
 checkEvents();
 // First get all id's of units
 QValueList<int> unitIds = player()->speciesTheme()->allFacilities();
 unitIds += player()->speciesTheme()->allMobiles();
 QValueList<int>::iterator it;
 int current = 0;
 int total = unitIds.count();
 for(it = unitIds.begin(); it != unitIds.end(); ++it) {
	current++;
	player()->speciesTheme()->loadUnit(*it);
	d->mLoading->setProgress(3000 + ((double)current / total * 1600));
 }


 d->mLoading->setProgress(4600);
 d->mLoading->setLoading(BosonLoadingWidget::InitGame);
 checkEvents();
 if(mBoson->isAdmin()) {
	mBoson->sendMessage(0, BosonMessage::IdInitFogOfWar);
	mBoson->sendMessage(0, BosonMessage::IdStartScenario);
 }
 d->mLoading->setProgress(4700);
 d->mLoading->setLoading(BosonLoadingWidget::StartingGame);
 checkEvents();

 showBosonWidget();
 delete d->mNewGame;
 d->mNewGame = 0;
 statusBar()->show();
 d->mBosonwidget->initGameMode();
 enableGameActions(true);
 initDebugPlayersMenu();
 checkDockStatus();

 connect(d->mBosonwidget, SIGNAL(signalMobilesCount(int)), this, SIGNAL(signalSetMobilesCount(int)));
 connect(d->mBosonwidget, SIGNAL(signalFacilitiesCount(int)), this, SIGNAL(signalSetFacilitiesCount(int)));
 connect(d->mBosonwidget, SIGNAL(signalOilUpdated(int)), this, SIGNAL(signalOilUpdated(int)));
 connect(d->mBosonwidget, SIGNAL(signalMineralsUpdated(int)), this, SIGNAL(signalMineralsUpdated(int)));

 connect(d->mBosonwidget, SIGNAL(signalChatDockHidden()), this, SLOT(slotChatDockHidden()));
 connect(d->mBosonwidget, SIGNAL(signalCmdFrameDockHidden()), this, SLOT(slotCmdFrameDockHidden()));

 d->mLoading->setProgress(5000);
 d->mLoading->setLoading(BosonLoadingWidget::LoadingDone);  // FIXME: This is probably meaningless

 mGame = true;
}

void TopWidget::slotCanvasTilesLoading(int progress)
{
 d->mLoading->setProgress(600 + (progress / 1244.0 * 2200));
 // No checkEvents() here as events are already processed in BosonTiles::???
}

void TopWidget::slotCanvasTilesLoaded()
{
 checkEvents();
 d->mLoading->setProgress(3000);
 QTimer::singleShot(0, this, SLOT(loadGameData3()));
}

void TopWidget::slotReceiveMap(const QByteArray& buffer)
{
 disconnect(mBoson, SIGNAL(signalInitMap(const QByteArray&)), this, SLOT(slotReceiveMap(const QByteArray&)));
 QDataStream stream(buffer, IO_ReadOnly);
 mMap->loadMap(stream);
 d->mLoading->setProgress(300);
 checkEvents();
 loadGameData2();
}

void TopWidget::checkEvents()
{
 if (qApp->hasPendingEvents()) {
	qApp->processEvents(200);
 }
}

void TopWidget::slotToggleSound()
{
 boMusic->setSound(!boMusic->sound());
}

void TopWidget::slotToggleMusic()
{
 boMusic->setMusic(!boMusic->music());
}

void TopWidget::slotToggleChat()
{
 d->mBosonwidget->toggleChatVisible();
 checkDockStatus();
}

void TopWidget::slotToggleCmdFrame()
{
 d->mBosonwidget->toggleCmdFrameVisible();
 checkDockStatus();
}

void TopWidget::checkDockStatus()
{
 d->mActionChat->setChecked(d->mBosonwidget->isChatVisible());
 d->mActionCmdFrame->setChecked(d->mBosonwidget->isCmdFrameVisible());
}

void TopWidget::slotChatDockHidden()
{
 d->mActionChat->setChecked(false);
}

void TopWidget::slotCmdFrameDockHidden()
{
 d->mActionCmdFrame->setChecked(false);
}

void TopWidget::slotConfigureKeys()
{
 KKeyDialog dlg(true, this);
 dlg.insert(actionCollection());
 if(mGame) {
	dlg.insert(d->mBosonwidget->actionCollection());
 }
 dlg.configure(true);
}

void TopWidget::slotDebugMode(int index)
{
 boConfig->setDebugMode((BosonConfig::DebugMode)index);
}

void TopWidget::slotZoom(int index)
{
kdDebug() << "zoom index=" << index << endl;
 double percent = d->mActionZoom->items()[index].toDouble();
 double factor = (double)percent / 100;
 QWMatrix m;
 m.scale(factor, factor);
 d->mBosonwidget->zoom(m);
}

void TopWidget::slotToggleFullScreen()
{
 if (d->mActionFullScreen->isChecked()) {
	showFullScreen();
 } else {
	showNormal();
 }
}

void TopWidget::slotEndGame()
{
 d->mBosonwidget->slotEndGame();
 disconnect(d->mBosonwidget, 0, 0, 0);
 // Delete all objects
 delete d->mBosonwidget;
 d->mBosonwidget = 0;
 delete mBoson;  // Easiest way to reset game info
 delete mCanvas;
 delete mMap;

 // Then re-init needed stuff
 initBoson();
 initPlayer();
 initMap();

 // Change menus and show welcome widget
 mGame = false;
 d->mActionStatusbar->setChecked(false);
 slotToggleStatusbar();
 enableGameActions(false);
 showWelcomeWidget();
}

void TopWidget::slotGamePreferences()
{
 d->mBosonwidget->slotGamePreferences();
}

void TopWidget::slotDebug()
{
 d->mBosonwidget->slotDebug();
}

void TopWidget::slotUnfogAll()
{
 d->mBosonwidget->slotUnfogAll();
}

void TopWidget::slotSplitDisplayHorizontal()
{
 d->mBosonwidget->slotSplitDisplayHorizontal();
}

void TopWidget::slotSplitDisplayVertical()
{
 d->mBosonwidget->slotSplitDisplayVertical();
}

void TopWidget::slotRemoveActiveDisplay()
{
 d->mBosonwidget->slotRemoveActiveDisplay();
}

void TopWidget::slotToggleStatusbar()
{
 if(d->mActionStatusbar->isChecked())
	statusBar()->show();
 else
	statusBar()->hide();
}

void TopWidget::slotDebugPlayer(int index)
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
	kdError() << k_funcinfo << "player not found" << endl;
	return;
 }

 switch (index) {
	case ID_DEBUG_KILLPLAYER:
		d->mBosonwidget->debugKillPlayer(p);
		break;
	default:
		kdError() << k_funcinfo << "unknown index " << index << endl;
		break;
 }
}

void TopWidget::loadGameDockConfig()
{
 // readDockConfig() is broken.
 // it uses frameGeometry() to save the geometry which is correct. but then it
 // uses setGeometry() to load it again, and since this doesn't work correctly
 // with titlebar and frame we need to adjust here
// readDockConfig(kapp->config(), "BosonGameDock"); // FIXME
}

void TopWidget::saveGameDockConfig()
{
 writeDockConfig(kapp->config(), "BosonGameDock");
}
