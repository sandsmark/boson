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
#include "bosonserveroptionswidget.h" // TODO rename: bosonnetworkoptionswidget
#include "bosonloadingwidget.h"
#include "bosonstartupbasewidget.h"
#include "bosonmusic.h"
#include "bosonconfig.h"
#include "boson.h"
#include "player.h"
#include "bosonplayfield.h"
#include "bosoncanvas.h"
#include "bosonmessage.h"
#include "bosonmap.h"
#include "speciestheme.h"
#include "defines.h"

#include <kapplication.h>
#include <klocale.h>
#include <kstatusbar.h>
#include <kstdaction.h>
#include <kstdgameaction.h>
#include <kaction.h>
#include <kdebug.h>
#include <kkeydialog.h>
#include <kmenubar.h>
#include <kpopupmenu.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kstandarddirs.h>

#include <qwidgetstack.h>
#include <qtimer.h>
#include <qwmatrix.h>
#include <qhbox.h>
#include <qptrdict.h>
#include <qobjectlist.h>

#define ID_DEBUG_KILLPLAYER 0
#define ID_WIDGETSTACK_WELCOME 1
#define ID_WIDGETSTACK_NEWGAME 2
#define ID_WIDGETSTACK_BOSONWIDGET 4
#define ID_WIDGETSTACK_NETWORK 5
#define ID_WIDGETSTACK_LOADING 6

#ifndef NO_EDITOR
#include "bosonstarteditorwidget.h"
#define ID_WIDGETSTACK_STARTEDITOR 3
#endif

class TopWidget::TopWidgetPrivate
{
public:
	TopWidgetPrivate() {
		mWelcome = 0;
		mNewGame = 0;
#ifndef NO_EDITOR
		mStartEditor = 0;
#endif
		mNetworkOptions = 0;
		mLoading = 0;
		mBosonWidget = 0;
	};

	BosonWelcomeWidget* mWelcome;
	BosonNewGameWidget* mNewGame;
#ifndef NO_EDITOR
	BosonStartEditorWidget* mStartEditor;
#endif
	BosonNetworkOptionsWidget* mNetworkOptions;
	BosonLoadingWidget* mLoading;
	BosonWidget* mBosonWidget;

	KToggleAction* mActionStatusbar;
	KToggleAction* mActionChat;
	KToggleAction* mActionCmdFrame;
	KActionMenu* mActionDebugPlayers;
	KSelectAction* mActionZoom;
	KToggleAction* mActionFullScreen;
	KActionCollection* mGameActions;

	QPtrDict<KPlayer> mPlayers; // needed for debug only

#if KDE_VERSION < 310
	bool mLoadingDockConfig;
#endif
};

TopWidget::TopWidget() : KDockMainWindow(0, "topwindow")
{
 d = new TopWidgetPrivate;
#if KDE_VERSION < 310
 d->mLoadingDockConfig = false;
#endif
 mGame = false;
 mMainDock = createDockWidget("mainDock", 0, this, i18n("Map"));
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
 loadInitialDockConfig();
}

TopWidget::~TopWidget()
{
 d->mPlayers.clear();
 delete d;
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
 d->mGameActions = new KActionCollection(this); // actions that are available in game mode only

 //FIXME: slotNewGame() is broken - endGame() is enough for now.
// (void)KStdGameAction::gameNew(this, SLOT(slotNewGame()), actionCollection()); //AB: game action?
 (void)KStdGameAction::end(this, SLOT(slotEndGame()), d->mGameActions);
// (void)KStdGameAction::pause(mBoson, SLOT(slotTogglePause()), d->mGameActions);
 (void)KStdGameAction::quit(this, SLOT(close()), actionCollection());

 // Settings
 (void)KStdAction::keyBindings(this, SLOT(slotConfigureKeys()), actionCollection());
 (void)KStdAction::preferences(this, SLOT(slotGamePreferences()), d->mGameActions);
 d->mActionStatusbar = KStdAction::showStatusbar(this, SLOT(slotToggleStatusbar()), d->mGameActions);

 // Dockwidgets show/hide
 d->mActionChat = new KToggleAction(i18n("Show &Chat"),
		KShortcut(Qt::CTRL+Qt::Key_C), this, SLOT(slotToggleChat()),
		d->mGameActions, "options_show_chat");
 d->mActionCmdFrame = new KToggleAction(i18n("Show C&ommandframe"),
		KShortcut(Qt::CTRL+Qt::Key_F), this, SLOT(slotToggleCmdFrame()),
		d->mGameActions, "options_show_cmdframe");

 // Sound & Music
 KToggleAction* sound = new KToggleAction(i18n("Soun&d"), 0, this,
		SLOT(slotToggleSound()), actionCollection(), "options_sound");
 sound->setChecked(boMusic->sound());
 KToggleAction* music = new KToggleAction(i18n("&Music"), 0, this,
		SLOT(slotToggleMusic()), actionCollection(), "options_music");
 music->setChecked(boMusic->music());

 // Debug - no i18n!
 (void)new KAction("Debug", KShortcut(), this,
		SLOT(slotDebug()), d->mGameActions, "debug_kgame");
 (void)new KAction("Unfog", KShortcut(), this,
		SLOT(slotUnfogAll()), d->mGameActions, "debug_unfog");
 KSelectAction* debugMode = new KSelectAction("Mode", KShortcut(), d->mGameActions, "debug_mode");
 connect(debugMode, SIGNAL(activated(int)), this, SLOT(slotDebugMode(int)));
 QStringList l;
 l.append("Normal");
 l.append("Debug Selection");
 debugMode->setItems(l);
 debugMode->setCurrentItem(0);
 d->mActionDebugPlayers = new KActionMenu("Players", d->mGameActions, "debug_players");

 // Zoom
 d->mActionZoom = new KSelectAction(i18n("&Zoom"), KShortcut(), d->mGameActions, "options_zoom");
 connect(d->mActionZoom, SIGNAL(activated(int)), this, SLOT(slotZoom(int)));
 QStringList items;
 items.append(QString::number(50));
 items.append(QString::number(100));
 items.append(QString::number(150));
 d->mActionZoom->setItems(items);

 // Display
 // note: the icons for these action need to have konqueror installed!
 (void)new KAction(i18n( "Split Display &Left/Right"), "view_left_right",
		   CTRL+SHIFT+Key_L, this, SLOT(slotSplitDisplayHorizontal()),
		   d->mGameActions, "splitviewh");
 (void)new KAction(i18n("Split Display &Top/Bottom"), "view_top_bottom",
		   CTRL+SHIFT+Key_T, this, SLOT(slotSplitDisplayVertical()),
		   d->mGameActions, "splitviewv");
 (void)new KAction(i18n("&Remove Active Display"), "view_remove",
		  CTRL+SHIFT+Key_R, this, SLOT(slotRemoveActiveDisplay()),
		  d->mGameActions, "removeview");
 d->mActionFullScreen = new KToggleAction(i18n("&Fullscreen Mode"), CTRL+SHIFT+Key_F,
		this, SLOT(slotToggleFullScreen()), actionCollection(), "window_fullscreen");
 d->mActionFullScreen->setChecked(false);

#if KDE_VERSION >= 310
 actionCollection()->addDocCollection(d->mGameActions);
#else
 KActionPtrList list = d->mGameActions->actions();
 KActionPtrList::Iterator it = list.begin();
 for (; it != list.end(); ++it) {
	actionCollection()->insert(*it);
 }
#endif
 createGUI();
 menuBar()->hide();
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
 if(enable && ! d->mBosonWidget) {
	kdWarning() << k_lineinfo << "NULL BosonWidget!" << endl;
 }
 KActionPtrList list = d->mGameActions->actions();
 KActionPtrList::Iterator it;
 for (it = list.begin(); it != list.end(); ++it) {
	(*it)->setEnabled(enable);
 }
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
 BosonStartupBaseWidget* startup = new BosonStartupBaseWidget(mWs);
 mWs->addWidget(startup, ID_WIDGETSTACK_WELCOME);
 d->mWelcome = new BosonWelcomeWidget(startup->plainWidget());
 startup->initBackgroundOrigin();
 connect(d->mWelcome, SIGNAL(signalNewGame()), this, SLOT(slotNewGame()));
 connect(d->mWelcome, SIGNAL(signalStartEditor()), this, SLOT(slotStartEditor()));
 connect(d->mWelcome, SIGNAL(signalQuit()), this, SLOT(close()));
}

void TopWidget::showWelcomeWidget()
{
 if(!d->mWelcome) {
	initWelcomeWidget();
 }
 raiseWidget(ID_WIDGETSTACK_WELCOME);
}

void TopWidget::initNewGameWidget()
{
 if(d->mNewGame) {
	return;
 }
 BosonStartupBaseWidget* startup = new BosonStartupBaseWidget(mWs);
 mWs->addWidget(startup, ID_WIDGETSTACK_NEWGAME);
 d->mNewGame = new BosonNewGameWidget(this, startup->plainWidget());
 startup->initBackgroundOrigin();
 connect(d->mNewGame, SIGNAL(signalCancelled()), this, SLOT(slotShowMainMenu()));
 connect(d->mNewGame, SIGNAL(signalShowNetworkOptions()), this, SLOT(slotShowNetworkOptions()));
 d->mNewGame->show();
}

void TopWidget::showNewGameWidget()
{
 if(!d->mNewGame) {
	initNewGameWidget();
 }
 raiseWidget(ID_WIDGETSTACK_NEWGAME);
}

void TopWidget::initStartEditorWidget()
{
#ifndef NO_EDITOR
 if(d->mStartEditor) {
	return;
 }
 BosonStartupBaseWidget* startup = new BosonStartupBaseWidget(mWs);
 mWs->addWidget(startup, ID_WIDGETSTACK_STARTEDITOR);
 d->mStartEditor = new BosonStartEditorWidget(this, startup->plainWidget());
 startup->initBackgroundOrigin();
 connect(d->mStartEditor, SIGNAL(signalCancelled()), this, SLOT(slotShowMainMenu()));
#endif
}

void TopWidget::showStartEditorWidget()
{
#ifndef NO_EDITOR
 if(!d->mStartEditor) {
	initStartEditorWidget();
 }
 raiseWidget(ID_WIDGETSTACK_STARTEDITOR);
#endif
}

void TopWidget::initBosonWidget()
{
 if(d->mBosonWidget) {
	//should not happen!
	kdWarning() << k_funcinfo << "widget already allocated!" << endl;
	return;
 }
 d->mBosonWidget = new BosonWidget(this, mWs);
 mWs->addWidget(d->mBosonWidget, ID_WIDGETSTACK_BOSONWIDGET);
}

void TopWidget::showBosonWidget()
{
 if(!d->mBosonWidget) {
	initBosonWidget();
 }
 raiseWidget(ID_WIDGETSTACK_BOSONWIDGET);
 menuBar()->show();
}

void TopWidget::initNetworkOptions()
{
 if(d->mNetworkOptions) {
	return;
 }
 BosonStartupBaseWidget* startup = new BosonStartupBaseWidget(mWs);
 mWs->addWidget(startup, ID_WIDGETSTACK_NETWORK);
 d->mNetworkOptions = new BosonNetworkOptionsWidget(this, startup->plainWidget());
 startup->initBackgroundOrigin();
 connect(d->mNetworkOptions, SIGNAL(signalOkClicked()), this, SLOT(slotHideNetworkOptions()));
}

void TopWidget::showNetworkOptions()
{
 if(!d->mNetworkOptions) {
	initNetworkOptions();
 }
 raiseWidget(ID_WIDGETSTACK_NETWORK);
 menuBar()->hide();
}

void TopWidget::initLoadingWidget()
{
 if(d->mLoading) {
	return;
 }
 BosonStartupBaseWidget* startup = new BosonStartupBaseWidget(mWs);
 mWs->addWidget(startup, ID_WIDGETSTACK_LOADING);
 d->mLoading = new BosonLoadingWidget(startup->plainWidget());
 startup->initBackgroundOrigin();
}

void TopWidget::showLoadingWidget()
{
 if(!d->mLoading) {
	initLoadingWidget();
 }
 raiseWidget(ID_WIDGETSTACK_LOADING);
 menuBar()->hide();
}

void TopWidget::slotNewGame()
{
 showNewGameWidget();
}

void TopWidget::slotStartEditor()
{
#ifndef NO_EDITOR
 showStartEditorWidget();
#endif
}

void TopWidget::slotStartGame()
{
 showLoadingWidget();
 loadGameData1();
}

void TopWidget::slotShowMainMenu()
{
 if (d->mNewGame) {
	disconnect(d->mNewGame);
	delete d->mNewGame;
	d->mNewGame = 0;
 }
#ifndef NO_EDITOR
 if (d->mStartEditor) {
	delete d->mStartEditor;
	d->mStartEditor = 0;
 }
#endif
 // Delete all players to remove added AI players, then re-init local player
 mBoson->removeAllPlayers();
 initPlayer();
 showWelcomeWidget();
}

void TopWidget::slotShowNetworkOptions()
{
 showNetworkOptions();
}

void TopWidget::slotHideNetworkOptions()
{
 disconnect(d->mNetworkOptions);
 delete d->mNetworkOptions;
 d->mNetworkOptions = 0;
 d->mNewGame->slotSetAdmin(mBoson->isAdmin());
 showNewGameWidget();
}

void TopWidget::loadGameData1() // FIXME rename!
{
 d->mLoading->setSteps(3400 + mBoson->playerCount() * UNITDATAS_LOADINGFACTOR);
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

void TopWidget::loadGameData2() //FIXME rename!
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

void TopWidget::loadGameData3() // FIXME rename!
{
 // Load unit pixmaps
 QPtrListIterator<KPlayer> it(*(mBoson->playerList()));
 int progress = 3000;
 while (it.current()) {
	loadUnitDatas(((Player*)it.current()), progress);
	++it;
	progress += UNITDATAS_LOADINGFACTOR;
 }

 d->mLoading->setProgress(progress);
 d->mLoading->setLoading(BosonLoadingWidget::InitGame);
 checkEvents();
 if(mBoson->isAdmin()) {
	if (mBoson->gameMode()) {
		mBoson->sendMessage(0, BosonMessage::IdInitFogOfWar);
	}
	mBoson->sendMessage(0, BosonMessage::IdStartScenario);
 }
 progress += 100;
 d->mLoading->setProgress(progress);
 d->mLoading->setLoading(BosonLoadingWidget::StartingGame);
 checkEvents();

 connect(d->mBosonWidget, SIGNAL(signalMobilesCount(int)), this, SIGNAL(signalSetMobilesCount(int)));
 connect(d->mBosonWidget, SIGNAL(signalFacilitiesCount(int)), this, SIGNAL(signalSetFacilitiesCount(int)));
 connect(d->mBosonWidget, SIGNAL(signalOilUpdated(int)), this, SIGNAL(signalOilUpdated(int)));
 connect(d->mBosonWidget, SIGNAL(signalMineralsUpdated(int)), this, SIGNAL(signalMineralsUpdated(int)));

 showBosonWidget();
 delete d->mNewGame;
 d->mNewGame = 0;
#ifndef NO_EDITOR
 delete d->mStartEditor;
 d->mStartEditor = 0;
#endif
 statusBar()->show();
 d->mBosonWidget->initGameMode();
 enableGameActions(true);
 initDebugPlayersMenu();
 checkDockStatus();

 connect(d->mBosonWidget, SIGNAL(signalChatDockHidden()), this, SLOT(slotChatDockHidden()));
 connect(d->mBosonWidget, SIGNAL(signalCmdFrameDockHidden()), this, SLOT(slotCmdFrameDockHidden()));
 connect(d->mBosonWidget, SIGNAL(signalGameOver()), this, SLOT(slotGameOver()));

 progress += 300;
 d->mLoading->setProgress(progress);
 d->mLoading->setLoading(BosonLoadingWidget::LoadingDone);  // FIXME: This is probably meaningless

 mGame = true;
}

void TopWidget::slotCanvasTilesLoading(int progress)
{
 d->mLoading->setProgress((int)(600 + (progress / 1244.0 * MAPTILES_LOADINGFACTOR)));
 // No checkEvents() here as events are already processed in BosonTiles::???
}

void TopWidget::slotCanvasTilesLoaded()
{
 checkEvents();
 d->mLoading->setProgress(3000);
 QTimer::singleShot(0, this, SLOT(loadGameData3()));
}

void TopWidget::loadUnitDatas(Player* p, int progress)
{
 d->mLoading->setProgress(progress);
 d->mLoading->setLoading(BosonLoadingWidget::LoadUnits);
 checkEvents();
 // First get all id's of units
 QValueList<int> unitIds = p->speciesTheme()->allFacilities();
 unitIds += p->speciesTheme()->allMobiles();
 QValueList<int>::iterator it;
 int current = 0;
 int total = unitIds.count();
 for(it = unitIds.begin(); it != unitIds.end(); ++it) {
	current++;
	p->speciesTheme()->loadUnit(*it);
	d->mLoading->setProgress((int)(progress + ((double)current / total * UNITDATAS_LOADINGFACTOR)));
 }
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
 boConfig->setSound(boMusic->sound());
}

void TopWidget::slotToggleMusic()
{
 boMusic->setMusic(!boMusic->music());
 boConfig->setMusic(boMusic->music());
}

void TopWidget::slotToggleChat()
{
 d->mBosonWidget->toggleChatVisible();
 checkDockStatus();
}

void TopWidget::slotToggleCmdFrame()
{
 d->mBosonWidget->toggleCmdFrameVisible();
 checkDockStatus();
}

void TopWidget::checkDockStatus()
{
 d->mActionChat->setChecked(d->mBosonWidget->isChatVisible());
 d->mActionCmdFrame->setChecked(d->mBosonWidget->isCmdFrameVisible());
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
 QPtrList<KXMLGUIClient> clients = guiFactory()->clients();
 QPtrListIterator<KXMLGUIClient> it(clients);
 while (it.current()) {
	dlg.insert((*it)->actionCollection());
	++it;
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
 double percent = d->mActionZoom->items()[index].toDouble(); // bahh!!! 
 double factor = (double)percent / 100;
 QWMatrix m;
 m.scale(factor, factor);
 d->mBosonWidget->zoom(m);
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
 int answer = KMessageBox::warningYesNo(this, i18n("Are you sure you want to end this game?"),
		i18n("Are you sure?"), KStdGuiItem::yes(), KStdGuiItem::no(), "ConfirmEndGame");
 if(answer == KMessageBox::No) {
	return;
 }
 endGame();
}

void TopWidget::endGame()
{
 d->mBosonWidget->slotEndGame();
 saveGameDockConfig();
 disconnect(d->mBosonWidget, 0, 0, 0);
 // Delete all objects
 delete d->mBosonWidget;
 d->mBosonWidget = 0;
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

void TopWidget::slotGameOver()
{
 endGame();
}

void TopWidget::slotGamePreferences()
{
 d->mBosonWidget->slotGamePreferences();
}

void TopWidget::slotDebug()
{
 d->mBosonWidget->slotDebug();
}

void TopWidget::slotUnfogAll()
{
 d->mBosonWidget->slotUnfogAll();
}

void TopWidget::slotSplitDisplayHorizontal()
{
 d->mBosonWidget->slotSplitDisplayHorizontal();
}

void TopWidget::slotSplitDisplayVertical()
{
 d->mBosonWidget->slotSplitDisplayVertical();
}

void TopWidget::slotRemoveActiveDisplay()
{
 d->mBosonWidget->slotRemoveActiveDisplay();
}

void TopWidget::slotToggleStatusbar()
{
 if(d->mActionStatusbar->isChecked()) {
	statusBar()->show();
 } else {
	statusBar()->hide();
 }
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
		d->mBosonWidget->debugKillPlayer(p);
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
#if KDE_VERSION < 310
 d->mLoadingDockConfig = true;
#endif
 readDockConfig(kapp->config(), "BosonGameDock");
#if KDE_VERSION < 310
 d->mLoadingDockConfig = false;
#endif
 KConfig* conf = kapp->config();
 conf->setGroup("BosonGameDock");
 bool fs = conf->readBoolEntry("FullScreen", false);
 d->mActionFullScreen->setChecked(fs);
 slotToggleFullScreen();
}

void TopWidget::loadInitialDockConfig()
{
#if KDE_VERSION < 310
 d->mLoadingDockConfig = true;
#endif
 readDockConfig(kapp->config(), "BosonInitialDock");
#if KDE_VERSION < 310
 d->mLoadingDockConfig = false;
#endif
 KConfig* conf = kapp->config();
 conf->setGroup("BosonInitialDock");
 bool fs = conf->readBoolEntry("FullScreen", false);
 d->mActionFullScreen->setChecked(fs);
 slotToggleFullScreen();
}

void TopWidget::saveGameDockConfig()
{
 writeDockConfig(kapp->config(), "BosonGameDock");
 KConfig* conf = kapp->config();
 conf->setGroup("BosonGameDock");
 conf->writeEntry("FullScreen", d->mActionFullScreen->isChecked());
}

void TopWidget::saveInitialDockConfig()
{
 writeDockConfig(kapp->config(), "BosonInitialDock");
 KConfig* conf = kapp->config();
 conf->setGroup("BosonInitialDock");
 conf->writeEntry("FullScreen", d->mActionFullScreen->isChecked());
}

#if KDE_VERSION < 310
void TopWidget::setGeometry(const QRect& r)
{
 if (d->mLoadingDockConfig) {
	move(r.topLeft());
	resize(r.size());
 }
}
#endif

bool TopWidget::queryClose()
{
 kdDebug() << k_funcinfo << endl;
 if(mGame) {
	int answer = KMessageBox::warningYesNo(this, i18n("Are you sure you want to quit Boson?\n"
			"This will end current game."), i18n("Are you sure?"), KStdGuiItem::yes(),
			KStdGuiItem::no(), "ConfirmQuitWhenGameRunning");
	if(answer == KMessageBox::Yes) {
		return true;
	}
	else if(answer == KMessageBox::No) {
		return false;
	}
 }
 return true;
}

bool TopWidget::queryExit()
{
 QWidget* w = mWs->visibleWidget();
 if (!w) {
	return true;
 }
 switch (mWs->id(w)) {
	case ID_WIDGETSTACK_BOSONWIDGET:
		d->mBosonWidget->saveConfig(false);
		d->mBosonWidget->slotEndGame();
		saveGameDockConfig();
		return true;
	case ID_WIDGETSTACK_NEWGAME:
	case ID_WIDGETSTACK_LOADING:
	case ID_WIDGETSTACK_NETWORK:
	case ID_WIDGETSTACK_WELCOME:
		saveInitialDockConfig();
		return true;
	default:
		return true;
 }
 return true;
}

void TopWidget::raiseWidget(int id)
{
 switch (id) {
	case ID_WIDGETSTACK_WELCOME:
		setMinimumSize(d->mWelcome->size());
		setMaximumSize(d->mWelcome->size());
		menuBar()->hide();
		break;
	case ID_WIDGETSTACK_NEWGAME:
	case ID_WIDGETSTACK_LOADING:
	case ID_WIDGETSTACK_NETWORK:
		setMinimumSize(BOSON_MINIMUM_WIDTH, BOSON_MINIMUM_HEIGHT);
		setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
		menuBar()->hide();
		break;
	case ID_WIDGETSTACK_BOSONWIDGET:
#ifndef NO_EDITOR
	case ID_WIDGETSTACK_STARTEDITOR:
#endif
		setMinimumSize(BOSON_MINIMUM_WIDTH, BOSON_MINIMUM_HEIGHT);
		setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
		menuBar()->show();
		break;
	default:
		kdDebug() << k_funcinfo << "unknown id " << id << endl;
		break;
 }
 mWs->raiseWidget(id);
}

