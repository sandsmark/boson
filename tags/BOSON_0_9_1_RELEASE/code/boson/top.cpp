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
#include "top.h"
#include "top.moc"

#include "defines.h"
#include "bosonwidget.h"
#include "editorwidget.h"
#include "bosonconfig.h"
#include "boson.h"
#include "bosonsaveload.h"
#include "player.h"
#include "bosonplayfield.h"
#include "bosoncanvas.h"
#include "bosoncanvasstatistics.h"
#include "bosonmessage.h"
#include "speciestheme.h"
#include "bosonprofiling.h"
#include "bodisplaymanager.h"
#include "bosonstarting.h"
#include "rtti.h"
#include "bodebug.h"
#include "bodebugdcopiface.h"
#include "startupwidgets/bosonstartupwidget.h"
#include "sound/bosonaudiointerface.h"
#include "kgameunitdebug.h"
#include "kgameplayerdebug.h"
#include "kgamecelldebug.h"
#include "bosonprofilingdialog.h"
#include "bosondata.h"
#include "bosongroundtheme.h"

#include <kapplication.h>
#include <klocale.h>
#include <kstatusbar.h>
#include <kstdaction.h>
#include <kstdgameaction.h>
#include <kaction.h>
#include <kkeydialog.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kgame/kgamedebugdialog.h>

#include <qcursor.h>
#include <qwidgetstack.h>
#include <qtimer.h>
#include <qhbox.h>
#include <qvbox.h>

#include <stdlib.h>
#include <config.h>


class TopWidget::TopWidgetPrivate
{
public:
	TopWidgetPrivate()
	{
		mStartup = 0;

		mDisplayManager = 0;
		mBosonWidget = 0;

		mActionStatusbar = 0;
		mActionMenubar = 0;
		mActionFullScreen = 0;

		mStarting = 0;

		mIface = 0;

		mChatDock = 0;
		mCommandFrameDock = 0;
	}

	BosonStartupWidget* mStartup;

	BoDisplayManager* mDisplayManager;
	BosonWidgetBase* mBosonWidget;

	KToggleAction* mActionStatusbar;
	KToggleAction* mActionMenubar;
	KToggleAction* mActionFullScreen;

	QTimer mStatusBarTimer;

	BosonStarting* mStarting;

#if KDE_VERSION < 310
	bool mLoadingDockConfig;
#endif

	BoDebugDCOPIface* mIface;

	KDockWidget* mChatDock;
	KDockWidget* mCommandFrameDock;
};

TopWidget::TopWidget() : KDockMainWindow(0, "topwindow")
{
 d = new TopWidgetPrivate;
 mPlayField = 0;
#if KDE_VERSION < 310
 d->mLoadingDockConfig = false;
#endif

 mMainDock = createDockWidget("mainDock", 0, this, i18n("Map"));
 mMainDock->setDockSite(KDockWidget::DockCorner);
 mMainDock->setEnableDocking(KDockWidget::DockNone);
 initGameDockWidgets(false);

 if (!BosonGroundTheme::createGroundThemeList()) {
	boError() << k_funcinfo << "Unable to create groundTheme list" << endl;
	KMessageBox::sorry(this, i18n("Unable to load groundThemes. Check your installation!"));
	exit(1);
 }
 if (!BosonPlayField::preLoadAllPlayFields()) {
	boError() << k_funcinfo << "Unable to preload playFields" << endl;
	KMessageBox::sorry(this, i18n("Unable to preload playFields. Check your installation!"));
	exit(1);
 }
 boAudio->setSound(boConfig->sound());
 boAudio->setMusic(boConfig->music());


 d->mStartup = new BosonStartupWidget(mMainDock);
 connect(d->mStartup, SIGNAL(signalAddLocalPlayer()),
		this, SLOT(slotAddLocalPlayer()));
 connect(d->mStartup, SIGNAL(signalResetGame()),
		this, SLOT(slotResetGame()));
 connect(d->mStartup, SIGNAL(signalQuit()), this, SLOT(close()));
 connect(d->mStartup, SIGNAL(signalLoadGame(const QString&)),
		this, SLOT(slotLoadGame(const QString&)));
 connect(d->mStartup, SIGNAL(signalSaveGame(const QString&, const QString&)),
		this, SLOT(slotSaveGame(const QString&, const QString&)));
 connect(d->mStartup, SIGNAL(signalCancelLoadSave()),
		this, SLOT(slotCancelLoadSave()));

 connect(&d->mStatusBarTimer, SIGNAL(timeout()), this, SLOT(slotUpdateStatusBar()));

 mMainDock->setWidget(d->mStartup);

 setView(mMainDock);
 setMainDockWidget(mMainDock);

 initKActions();
 initStatusBar();

 // this will also call slotResetGame() and therefore init the game
 d->mStartup->slotShowWelcomeWidget();

 loadInitialDockConfig();

 d->mIface = new BoDebugDCOPIface();

 initDisplayManager();
}

TopWidget::~TopWidget()
{
 boDebug() << k_funcinfo << endl;
 bool editor = false;
 if (boGame) {
	editor = !boGame->gameMode();
 }
 boConfig->save(editor);
 endGame();
 d->mDisplayManager->quitGame();
 delete d->mDisplayManager;
 delete d->mIface;
 delete d;
 boDebug() << k_funcinfo << "done" << endl;
}

void TopWidget::initDisplayManager()
{
 d->mDisplayManager = new BoDisplayManager(0);
 d->mDisplayManager->hide();
}

void TopWidget::initGameDockWidgets(bool display)
{
 if (!d->mChatDock) {
	d->mChatDock = createDockWidget("chat_dock", 0, 0, i18n("Chat"));
 }
 if (!d->mCommandFrameDock) {
	d->mCommandFrameDock = createDockWidget("cmdframe_dock", 0, 0, i18n("Command Frame"));
 }

 d->mChatDock->setEnableDocking(KDockWidget::DockTop | KDockWidget::DockBottom);
 d->mChatDock->setDockSite(KDockWidget::DockNone);

 d->mCommandFrameDock->setEnableDocking(KDockWidget::DockLeft | KDockWidget::DockRight);
 d->mCommandFrameDock->setDockSite(KDockWidget::DockNone);

 // we are initializing, so we should do the initial docking positions as well.
 // will be overridden once a dock layout is loaded.
 d->mChatDock->manualDock(getMainDockWidget(), KDockWidget::DockBottom, 80);
 d->mCommandFrameDock->manualDock(getMainDockWidget(), KDockWidget::DockLeft, 30);

 if (!display) {
	hideGameDockWidgets();
 } else {
	makeDockVisible(d->mChatDock);
	makeDockVisible(d->mCommandFrameDock);
 }
}

void TopWidget::slotLoadBosonGameDock()
{
 if (!kapp->config()->hasGroup("BosonGameDock")) {
	boDebug() << k_funcinfo << "dock config does not exist" << endl;
	// Dock config isn't saved (probably first start). Hide chat dock (we only
	//  show commandframe by default)
	makeDockInvisible(d->mChatDock);
	makeDockVisible(d->mCommandFrameDock);
	if (d->mDisplayManager) {
		d->mDisplayManager->updateGeometry();  // Hack? Bug in BoDisplayManager?
	}
 } else {
	boDebug() << k_funcinfo << "dock config exists, loading" << endl;
	loadGameDockConfig();
 }
 slotCheckGameDockStatus();
}

void TopWidget::slotCheckGameDockStatus()
{
 if (d->mBosonWidget) {
	d->mBosonWidget->setActionChat(d->mChatDock->isVisible());
	d->mBosonWidget->setActionCmdFrame(d->mCommandFrameDock->isVisible());
 }
}

void TopWidget::hideGameDockWidgets(bool deinit)
{
 makeDockInvisible(d->mChatDock);
 makeDockInvisible(d->mCommandFrameDock);

 if (deinit) {
	d->mChatDock->setWidget(0);
	d->mCommandFrameDock->setWidget(00);
 }
}

void TopWidget::slotToggleChatDockVisible()
{
 d->mChatDock->changeHideShowState();
 slotCheckGameDockStatus();
}

void TopWidget::slotToggleCmdFrameDockVisible()
{
 d->mCommandFrameDock->changeHideShowState();
 slotCheckGameDockStatus();
}

void TopWidget::slotRemoveFromGUIFactory(QObject* obj)
{
 if (!obj) {
	return;
 }
 if (obj->inherits("BosonWidgetBase")) {
	BosonWidgetBase* w = (BosonWidgetBase*)obj;
	if (factory()) {
		factory()->removeClient(w);
	}
 }
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

void TopWidget::initKActions()
{
 // note: boGame and similar are *not* yet constructed!
 // Main actions: Game start/end and quit

 //FIXME: slotNewGame() is broken - endGame() is enough for now.
// (void)KStdGameAction::gameNew(this, SLOT(slotNewGame()), actionCollection()); 
// (void)KStdGameAction::quit(this, SLOT(close()), actionCollection());

 // Settings
 (void)KStdAction::keyBindings(this, SLOT(slotConfigureKeys()), actionCollection());
 d->mActionMenubar = KStdAction::showMenubar(this, SLOT(slotToggleMenubar()), actionCollection());
 d->mActionStatusbar = KStdAction::showStatusbar(this, SLOT(slotToggleStatusbar()), actionCollection());

 // Sound & Music
 KToggleAction* sound = new KToggleAction(i18n("Soun&d"), 0, this,
		SLOT(slotToggleSound()), actionCollection(), "options_sound");
 sound->setChecked(boConfig->sound());
 KToggleAction* music = new KToggleAction(i18n("M&usic"), 0, this,
		SLOT(slotToggleMusic()), actionCollection(), "options_music");
 music->setChecked(boConfig->music());

 // Display
 d->mActionFullScreen = new KToggleAction(i18n("&Fullscreen Mode"), CTRL+SHIFT+Key_F,
		this, SLOT(slotToggleFullScreen()), actionCollection(), "window_fullscreen");
 d->mActionFullScreen->setChecked(false);

 // Debug
 (void)new KAction(i18n("&Profiling..."), KShortcut(), this,
		SLOT(slotProfiling()), actionCollection(), "debug_profiling");
 (void)new KAction(i18n("&Debug KGame..."), KShortcut(), this,
		SLOT(slotDebugKGame()), actionCollection(), "debug_kgame");

 createGUI("topui.rc", false);

 hideMenubar();
}

void TopWidget::initBoson()
{
 if (Boson::boson()) {
	boWarning() << k_funcinfo << "Oops - Boson object already present! deleting..." << endl;
	Boson::deleteBoson();
 }
 Boson::initBoson();
 if (!d->mStarting) {
	BO_NULL_ERROR(d->mStarting);
 }
 boGame->setStartingObject(d->mStarting);

 // new games are handled in this order: ADMIN clicks on start games - this
 // sends an IdStartGame over network. Once this is received signalStartNewGame()
 // is emitted and we start here
 connect(boGame, SIGNAL(signalStartNewGame()), this, SLOT(slotStartNewGame()));
 connect(boGame, SIGNAL(signalPlayFieldChanged(const QString&)),
		this, SLOT(slotPlayFieldChanged(const QString&)));
 connect(boGame, SIGNAL(signalGameStarted()), this, SLOT(slotGameStarted()));

 // for editor (new maps)
 connect(boGame, SIGNAL(signalEditorNewMap(const QByteArray&)),
		this, SLOT(slotEditorNewMap(const QByteArray&)));
}

void TopWidget::initPlayer()
{
 boWarning() << k_funcinfo << endl;
 Player* p = new Player;
 slotChangeLocalPlayer(p);
}

void TopWidget::initPlayField()
{
 delete mPlayField;
 mPlayField = new BosonPlayField(this);
}

void TopWidget::initCanvas()
{
 boGame->createCanvas();
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

 QHBox* debug = new QHBox(bar);
 (void)new QLabel(i18n("Particles: "), debug);
 QLabel* particlesLabel = new QLabel(QString::number(0), debug);
 connect(this, SIGNAL(signalParticlesCountUpdated(int)), particlesLabel, SLOT(setNum(int)));
 (void)new QLabel(i18n("Canvas items: "), debug);
 QLabel* itemsLabel = new QLabel(QString::number(0), debug);
 connect(this, SIGNAL(signalCanvasItemsCountUpdated(int)), itemsLabel, SLOT(setNum(int)));
 (void)new QLabel(i18n("Animated items: "), debug);
 QLabel* animatedLabel = new QLabel(QString::number(0), debug);
 connect(this, SIGNAL(signalCanvasAnimationsCountUpdated(int)), animatedLabel, SLOT(setNum(int)));
 bar->addWidget(debug);
 (void)new QLabel(i18n("Units: "), debug);
 QLabel* unitsLabel = new QLabel(QString::number(0), debug);
 connect(this, SIGNAL(signalUnitsUpdated(int)), unitsLabel, SLOT(setNum(int)));
 (void)new QLabel(i18n("Shots: "), debug);
 QLabel* shotsLabel = new QLabel(QString::number(0), debug);
 connect(this, SIGNAL(signalShotsUpdated(int)), shotsLabel, SLOT(setNum(int)));

 bar->hide();
}

void TopWidget::enableGameActions(bool enable)
{
 if (enable && ! d->mBosonWidget) {
	boWarning() << k_lineinfo << "NULL BosonWidgetBase!" << endl;
 }
}

void TopWidget::initBosonWidget()
{
 if (d->mBosonWidget) {
	//should not happen!
	boWarning() << k_funcinfo << "widget already allocated!" << endl;
	return;
 }
 BO_CHECK_NULL_RET(d->mDisplayManager);
 if (boGame->gameMode()) {
	BosonWidget* w = new BosonWidget(mMainDock);
	connect(w, SIGNAL(signalSaveGame()), this, SLOT(slotSaveGame()));
	connect(w, SIGNAL(signalLoadGame()), this, SLOT(slotLoadGame()));
	connect(w, SIGNAL(signalGameOver()), this, SLOT(slotGameOver()));
	d->mBosonWidget = w;
 } else {
	EditorWidget* w = new EditorWidget(mMainDock);
	d->mBosonWidget = w;
 }

 connect(d->mBosonWidget, SIGNAL(destroyed(QObject*)), this, SLOT(slotRemoveFromGUIFactory(QObject*)));
 connect(d->mBosonWidget, SIGNAL(signalLoadBosonGameDock()), this, SLOT(slotLoadBosonGameDock()));
 connect(d->mBosonWidget, SIGNAL(signalToggleChatVisible()), this, SLOT(slotToggleChatDockVisible()));
 connect(d->mBosonWidget, SIGNAL(signalToggleCmdFrameVisible()), this, SLOT(slotToggleCmdFrameDockVisible()));
 connect(d->mBosonWidget, SIGNAL(signalCheckDockStatus()), this, SLOT(slotCheckGameDockStatus()));
 connect(d->mBosonWidget, SIGNAL(signalChangeLocalPlayer(Player*)), this, SLOT(slotChangeLocalPlayer(Player*)));
 connect(d->mBosonWidget, SIGNAL(signalEndGame()), this, SLOT(slotEndGame()));
 connect(d->mBosonWidget, SIGNAL(signalQuit()), this, SLOT(close()));
 connect(d->mBosonWidget, SIGNAL(signalMobilesCount(int)), this, SIGNAL(signalSetMobilesCount(int)));
 connect(d->mBosonWidget, SIGNAL(signalFacilitiesCount(int)), this, SIGNAL(signalSetFacilitiesCount(int)));
 connect(d->mBosonWidget, SIGNAL(signalOilUpdated(int)), this, SIGNAL(signalOilUpdated(int)));
 connect(d->mBosonWidget, SIGNAL(signalMineralsUpdated(int)), this, SIGNAL(signalMineralsUpdated(int)));

 initGameDockWidgets(false);
 d->mBosonWidget->setDisplayManager(d->mDisplayManager);

 // at this point the startup widget should already have called
 // slotChangeLocalPlayer()! if not .. then we're in trouble here...
 if (!boGame->localPlayer()) {
	boWarning() << k_funcinfo << "NULL local player - might cause trouble here!" << endl;
	// do not return, since it might even be allowed for editor (currently
	// it is not)
 }
 changeLocalPlayer(boGame->localPlayer(), false);
 d->mBosonWidget->init(d->mChatDock, d->mCommandFrameDock); // this depends on several virtual methods and therefore can't be called in the c'tor

 factory()->addClient(d->mBosonWidget); // XMLClient-stuff. needs to be called *after* creation of KAction objects, so outside BosonWidget might be a good idea :-)
// createGUI("bosonui.rc", false);

 // finally add the initial display
 // note that this call also loads the cursor
 d->mBosonWidget->addInitialDisplay();
}

void TopWidget::slotPlayFieldChanged(const QString& id)
{
 if (!d->mStarting) {
	boError() << k_funcinfo << "NULL starting object!!" << endl;
	return;
 }
 d->mStarting->setPlayFieldId(id);
}

void TopWidget::slotStartNewGame()
{
 BO_CHECK_NULL_RET(boGame);
 if (!d->mStarting) {
	boError() << k_funcinfo << "NULL starting object!!" << endl;
	return;
 }
 if (!d->mStartup) {
	boError() << k_funcinfo << "NULL startup widget" << endl;
	return;
 }
 boDebug() << k_funcinfo << endl;

 // Save initial dock config
 saveInitialDockConfig();

 d->mStartup->showLoadingWidget();

 d->mStarting->setDestPlayField(mPlayField);

 initCanvas();
 initBosonWidget();

 changeLocalPlayer(boGame->localPlayer());
 d->mBosonWidget->initPlayer();

 // this will take care of all data loading, like models, textures and so. this
 // also initializes the map and will send IdStartScenario - in short this will
 // start the game. Once it's done it'll send IdGameIsStarted (see
 // Boson::signalGameStarted())
 d->mStarting->startNewGame();
}

void TopWidget::slotLoadGame(const QString& fileName)
{
 boDebug() << k_funcinfo << endl;
 BO_CHECK_NULL_RET(boGame);
 if (fileName.isEmpty()) {
	return;
 }
 if (!boGame) {
//	reinitGame();
 }
 if (!d->mStarting) {
	boError() << k_funcinfo << "NULL starting object!!" << endl;
	return;
 }

 // load the file into memory
 if (!d->mStarting->loadGame(fileName)) {
	boError() << k_funcinfo << "failed loading from file" << endl;
	return;
 }

 // actually start the game
 QTimer::singleShot(0, this, SLOT(slotStartNewGame()));
}

void TopWidget::slotSaveGame(const QString& fileName, const QString& description)
{
 boDebug() << k_funcinfo << endl;
 QString file = fileName;
 if (file.isNull()) {
	return;
 }
 if (file[0] != '/') {
	boError() << k_funcinfo << "filename must be absolute" << endl;
	return;
 }
 if (file.findRev('.') == -1) {
	file += ".bsg";
 }
 if (KStandardDirs::exists(file)) {
	int r = KMessageBox::questionYesNo(this, i18n("File %1 already exists. Overwrite?").arg(file));
	if (r != KMessageBox::Yes) {
		return;
	}
 }

 boDebug() << k_funcinfo << file << endl;
 bool ok = boGame->saveToFile(file);

 if (ok) {
	slotCancelLoadSave();
 } else {
	KMessageBox::sorry(this, i18n("Error while saving!"));
 }
}

void TopWidget::slotCancelLoadSave()
{
 boDebug() << k_funcinfo << endl;
 if (boGame && boGame->gameStatus() != KGame::Init) {
	// called from a running game - but the player doesn't want to load/save a
	// game anymore
	if (!d->mBosonWidget) {
		boError() << k_funcinfo << "NULL bosonwidget" << endl;
		return;
	}
	if (d->mStartup) {
		d->mStartup->hide();
	}
	d->mBosonWidget->show();
	mMainDock->setWidget(d->mBosonWidget);
 } else {
	if (!d->mStartup) {
		boError() << k_funcinfo << "NULL startup widget??" << endl;
		return;
	}
	d->mStartup->slotShowWelcomeWidget();
 }
}

void TopWidget::slotAssignMap()
{
 if (boGame->gameStatus() != KGame::Init) {
	boWarning() << k_funcinfo << "not in Init status" << endl;
	return;
 }
 boDebug() << k_funcinfo << endl;
 d->mBosonWidget->initMap();
}

void TopWidget::slotToggleSound()
{
 boAudio->setSound(!boAudio->sound());
 boConfig->setSound(boAudio->sound());
}

void TopWidget::slotToggleMusic()
{
 boAudio->setMusic(!boAudio->music());
 boConfig->setMusic(boAudio->music());
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

void TopWidget::slotToggleFullScreen()
{
 if (d->mActionFullScreen->isChecked()) {
	showFullScreen();
 } else {
	showNormal();
 }
}

void TopWidget::endGame()
{
 boDebug() << k_funcinfo << endl;
 // This must be done before BosonWidget::quitGame() is called, because latter
 //  calls boGame->quitGame() which in turn deletes all players and units and if
 //  something was selected, we would have crash later when trying to unselect them
 if (d->mDisplayManager) {
	d->mDisplayManager->quitGame();
 }
 if (d->mBosonWidget) {
	d->mBosonWidget->quitGame();
//	disconnect(d->mBosonWidget, 0, 0, 0); // we must not do this, so that slotRemoveFromGUIFactory() is called. do we need this disconnect at all?
	d->mStatusBarTimer.stop();
	// This prevent wrong dock config from getting saved when loading fails
	if (boGame->gameStatus() != KGame::Init) {
		saveGameDockConfig();
	}
 }
 delete d->mBosonWidget;
 d->mBosonWidget = 0;
 Boson::deleteBoson();  // Easiest way to reset game info
 delete mPlayField;
 mPlayField = 0;
 hideGameDockWidgets(true);
 boDebug() << k_funcinfo << "done" << endl;
}

void TopWidget::reinitGame()
{
 endGame();

 delete d->mStarting;
 d->mStarting = new BosonStarting(this); // manages startup of games
 connect(d->mStarting, SIGNAL(signalAssignMap()), this, SLOT(slotAssignMap()));
 connect(d->mStarting, SIGNAL(signalLoadingReset()),
		d->mStartup, SLOT(slotLoadingReset()));
 connect(d->mStarting, SIGNAL(signalLoadingSetAdmin(bool)),
		d->mStartup, SLOT(slotLoadingSetAdmin(bool)));
 connect(d->mStarting, SIGNAL(signalLoadingSetLoading(bool)),
		d->mStartup, SLOT(slotLoadingSetLoading(bool)));
 connect(d->mStarting, SIGNAL(signalLoadingType(int)),
		d->mStartup, SLOT(slotLoadingType(int)));
 connect(d->mStarting, SIGNAL(signalLoadingPlayersCount(int)),
		d->mStartup, SLOT(slotLoadingPlayersCount(int)));
 connect(d->mStarting, SIGNAL(signalLoadingPlayer(int)),
		d->mStartup, SLOT(slotLoadingPlayer(int)));
 connect(d->mStarting, SIGNAL(signalLoadingUnitsCount(int)),
		d->mStartup, SLOT(slotLoadingUnitsCount(int)));
 connect(d->mStarting, SIGNAL(signalLoadingUnit(int)),
		d->mStartup, SLOT(slotLoadingUnit(int)));
 connect(d->mStarting, SIGNAL(signalLoadingShowProgressBar(bool)),
		d->mStartup, SLOT(slotLoadingShowProgressBar(bool)));
 connect(d->mStarting, SIGNAL(signalStartingFailed()),
		this, SLOT(slotStartingFailed()));

 initBoson();
 initPlayField();

 d->mActionStatusbar->setChecked(false);
 slotToggleStatusbar();
 enableGameActions(false);
 showHideMenubar();
}

void TopWidget::slotGameOver()
{
 endGame();

 // this also resets the game!
 // if you replace this by something else you must call slotResetGame()
 // manually!
 d->mStartup->slotShowWelcomeWidget();
 mMainDock->setWidget(d->mStartup);
 loadInitialDockConfig();
}

void TopWidget::slotSplitDisplayHorizontal()
{
 if (!d->mBosonWidget) {
	return;
 }
 d->mBosonWidget->slotSplitDisplayHorizontal();
}

void TopWidget::slotSplitDisplayVertical()
{
 if (!d->mBosonWidget) {
	return;
 }
 d->mBosonWidget->slotSplitDisplayVertical();
}

void TopWidget::slotRemoveActiveDisplay()
{
 if (!d->mBosonWidget) {
	return;
 }
 d->mBosonWidget->slotRemoveActiveDisplay();
}

// FIXME: nonsense name. this doesn't toggle anything, but it applies the
// d->mActionStatusbar status to the actual statusbar
void TopWidget::slotToggleStatusbar()
{
 if (d->mActionStatusbar->isChecked()) {
	statusBar()->show();
 } else {
	statusBar()->hide();
 }
}

// FIXME: nonsense name. this doesn't toggle anything, but it applies the
// d->mActionMenubar status to the actual menubar
void TopWidget::slotToggleMenubar()
{
 if (boGame->gameStatus() != KGame::Init) {
	if (boGame->gameMode()) {
		// editor without menubar is pretty useless :)
		boConfig->setShowMenubarInGame(d->mActionMenubar->isChecked());
	}
 } else if (mMainDock->getWidget() == d->mStartup) {
	// note: startup can be on top during the game as well, e.g. when
	// loading/saving a game.
	// but at this point it is really a startup widget.
	boConfig->setShowMenubarOnStartup(d->mActionMenubar->isChecked());
 } else {
	boError() << k_funcinfo << "Game in init status but no startup widget on top?!" << endl;
 }
 showHideMenubar();
}

void TopWidget::loadGameDockConfig()
{
 boDebug() << k_funcinfo << endl;
 // readDockConfig() is broken.
 // it uses frameGeometry() to save the geometry which is correct. but then it
 // uses setGeometry() to load it again, and since this doesn't work correctly
 // with titlebar and frame we need to adjust here
#if KDE_VERSION < 310
 d->mLoadingDockConfig = true;
#endif
 KConfig* conf = kapp->config();
 conf->setGroup("BosonGameDock");
 bool fs = conf->readBoolEntry("FullScreen", false);
 d->mActionFullScreen->setChecked(fs);
 readDockConfig(kapp->config(), "BosonGameDock");
 slotToggleFullScreen();
#if KDE_VERSION < 310
 d->mLoadingDockConfig = false;
#endif
}

void TopWidget::loadInitialDockConfig()
{
 boDebug() << k_funcinfo << endl;
#if KDE_VERSION < 310
 d->mLoadingDockConfig = true;
#endif
 KConfig* conf = kapp->config();
 conf->setGroup("BosonInitialDock");
 bool fs = conf->readBoolEntry("FullScreen", false);
 d->mActionFullScreen->setChecked(fs);
 
 if (isVisible()) {
	slotToggleFullScreen();
	readDockConfig(kapp->config(), "BosonInitialDock");
 } else {
	readDockConfig(kapp->config(), "BosonInitialDock");
	slotToggleFullScreen();
 }
#if KDE_VERSION < 310
 d->mLoadingDockConfig = false;
#endif
}

void TopWidget::saveGameDockConfig()
{
 boDebug() << k_funcinfo << endl;
 writeDockConfig(kapp->config(), "BosonGameDock");
 KConfig* conf = kapp->config();
 conf->setGroup("BosonGameDock");
 conf->writeEntry("FullScreen", d->mActionFullScreen->isChecked());
}

void TopWidget::saveInitialDockConfig()
{
 boDebug() << k_funcinfo << endl;
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
 boDebug() << k_funcinfo << endl;
 if (boGame->gameStatus() != KGame::Init) {
	int answer = KMessageBox::warningYesNo(this, i18n("Are you sure you want to quit Boson?\n"
			"This will end current game."), i18n("Are you sure?"), KStdGuiItem::yes(),
			KStdGuiItem::no(), "ConfirmQuitWhenGameRunning");
	if (answer == KMessageBox::Yes) {
		return true;
	}
	else if (answer == KMessageBox::No) {
		return false;
	}
 }
 return true;
}

bool TopWidget::queryExit()
{
 if (!boGame) {
	// note: this _will_ happen if exit() is called before boGame was
	// created!
	return true;
 }
 if (boGame->gameStatus() != KGame::Init) {
	// note that even a startup widget might be on top here (e.g. when
	// saving a game)!
	d->mBosonWidget->saveConfig();
	d->mDisplayManager->quitGame();
	d->mBosonWidget->quitGame();
	if (boGame->gameMode()) {
		saveGameDockConfig();
	} else {
		boDebug() << k_funcinfo << "TODO: add saveEditorDockConfig()" << endl;
	}
	return true;
 }
 if (mMainDock->getWidget() != d->mStartup) {
	boWarning() << k_funcinfo << "oops! - game in init mode but no startup widget on top!" << endl;
	boDebug() << k_funcinfo << "maindock's widget: " << mMainDock->getWidget() << endl;
	boDebug() << k_funcinfo << "startup widget: " << d->mStartup << endl;
	return true;
 }

 // startup widget on top
 saveInitialDockConfig();
 return true;
}

void TopWidget::slotUpdateStatusBar()
{
 BO_CHECK_NULL_RET(d->mBosonWidget);
 BO_CHECK_NULL_RET(d->mBosonWidget->displayManager());
 const BosonCanvas* canvas = boGame->canvas();
 BO_CHECK_NULL_RET(canvas);
 BosonCanvasStatistics* stat = canvas->canvasStatistics();
 BO_CHECK_NULL_RET(stat);
 // AB: some statusbar labels are *not* updated here (e.g. minerals and units),
 // but whenever their value actually changes.
 emit signalParticlesCountUpdated(stat->particleSystemsCount());
 emit signalCanvasItemsCountUpdated(stat->allItemsCount());
 emit signalCanvasAnimationsCountUpdated(stat->animationsCount());

 // AB: this *might* be an expensive calculation. this can be a pretty big loop
 // (> 1000 entries), but there are simple calculations only. maybe we should
 // add a slotUpdateStatusBarExpensive() or so which gets called every 5 seconds
 // only
 stat->updateItemCount();
 emit signalUnitsUpdated(stat->itemCount(RTTI::UnitStart));
 emit signalShotsUpdated(stat->itemCount(RTTI::Shot));
}

void TopWidget::hideMenubar()
{
 d->mActionMenubar->setChecked(false);
 menuBar()->hide();
}

void TopWidget::showMenubar()
{
 d->mActionMenubar->setChecked(true);
 menuBar()->show();
}

void TopWidget::showHideMenubar()
{
 if (!boGame) {
	boError() << k_funcinfo << "NULL game object" << endl;
	return;
 }
 if (boGame->gameStatus() != KGame::Init) {
	// we always display menu in editor mode (pretty useless without it)
	if (boConfig->showMenubarInGame() || !boGame->gameMode()) {
		showMenubar();
	} else {
		hideMenubar();
	}
 } else {
	if (boConfig->showMenubarOnStartup()) {
		showMenubar();
	} else {
		hideMenubar();
	}
 }
}



void TopWidget::changeLocalPlayer(Player* p, bool init)
{
 boDebug() << k_funcinfo << p << endl;

 // AB: note that both, p == 0 AND p == currentplayer are valid and must be executed!
 if (d->mStarting) {
	d->mStarting->setLocalPlayer(p);
 }
 if (!boGame) {
	boError() << k_funcinfo << "NULL game object" << endl;
	return;
 }
 // AB: d->mBosonWidget->setLocalPlayer() also calls Boson::setLocalPlayer(),
 // but it is NULL when the startup widgets call it
 boGame->setLocalPlayer(p);
 if (d->mBosonWidget) {
	d->mBosonWidget->setLocalPlayer(p, init);
 }
}

void TopWidget::slotEndGame()
{
 int answer = KMessageBox::warningYesNo(this, i18n("Are you sure you want to end this game?"),
		i18n("Are you sure?"), KStdGuiItem::yes(), KStdGuiItem::no(), "ConfirmEndGame");
 if (answer == KMessageBox::No) {
	return;
 }
 slotGameOver();
}

void TopWidget::slotAddLocalPlayer()
{
 if (!boGame) {
	boError() << k_funcinfo << "NULL game object" << endl;
	return;
 }
 if (boGame->localPlayer()) {
	boError() << k_funcinfo << "already a local player present! remove first!" << endl;// maybe use boWarning() only
	return;
 }
 if (boGame->playerCount() != 0) {
	boError() << k_funcinfo << "there are already players in the game!" << endl;
	return;
 }
 boDebug() << k_funcinfo << endl;
 Player* p = new Player;
 boGame->bosonAddPlayer(p);
 slotChangeLocalPlayer(p);
}

void TopWidget::slotGameStarted()
{
 boDebug() << k_funcinfo << endl;
 BO_CHECK_NULL_RET(boGame);
 if (boGame->gameStatus() != KGame::Run) {
	boWarning() << k_funcinfo << "not in Run status" << endl;
	return;
 }
// AB: first init map (see slotAssginMap()), THEN init player. we need map for
// player loading (unit positions, ...)

 boDebug() << k_funcinfo << "init player" << endl;
 if (!boGame->localPlayer()) {
	boError() << k_funcinfo << "NULL local player" << endl;
	return;
 }
 changeLocalPlayer(boGame->localPlayer());
 d->mBosonWidget->initPlayer();

 // now show the bosonwidget and hide the startup widgets.
 mMainDock->setWidget(d->mBosonWidget);
 d->mBosonWidget->show();
 d->mStartup->hide();
 setMinimumSize(BOSON_MINIMUM_WIDTH, BOSON_MINIMUM_HEIGHT);
 setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

 int progress = 0; // FIXME: wrong value!

 // we don't need these anymore. lets save the memory.
 d->mStartup->resetWidgets();

 // Init some stuff
 d->mActionStatusbar->setChecked(true); // we do not yet remember user settings here! TODO
 slotToggleStatusbar();// AB: doesn't really toggle!
 enableGameActions(true);
 d->mStatusBarTimer.start(1000);
 d->mBosonWidget->initGameMode();

 showHideMenubar(); // depends on boGame->gameStatus()
}

void TopWidget::slotResetGame()
{
 if (boGame) {
	// Delete all players to remove added AI and local player (will get
	// added by newgame widget)
	boGame->removeAllPlayers();
 }
 reinitGame();
}

void TopWidget::slotEditorNewMap(const QByteArray& buffer)
{
 if (boGame->gameStatus() != KGame::Init) {
	boError() << k_funcinfo << "game not in init status anymore" << endl;
	return;
 }
 if (!d->mStarting) {
	boError() << k_funcinfo << "NULL starting object" << endl;
	return;
 }
 boDebug() << k_funcinfo << endl;
 d->mStarting->setEditorMap(buffer);
}

void TopWidget::slotLoadGame(KCmdLineArgs* args)
{
 if (!d->mStartup) {
	boError() << k_funcinfo << "NULL startup widget" << endl;
	return;
 }
 d->mStartup->slotLoadGame();
 d->mStartup->show();
 if (d->mBosonWidget) {
	d->mBosonWidget->hide();
 }
 mMainDock->setWidget(d->mStartup);
}

void TopWidget::slotSaveGame()
{
 // TODO: pause the game!
 if (!d->mStartup) {
	boError() << k_funcinfo << "NULL startup widget" << endl;
	return;
 }
 d->mStartup->slotSaveGame();
 d->mStartup->show();
 if (d->mBosonWidget) {
	d->mBosonWidget->hide();
 }
 mMainDock->setWidget(d->mStartup);
}

void TopWidget::slotNewGame(KCmdLineArgs* args)
{
 if (!d->mStartup) {
	boError() << k_funcinfo << "NULL startup widget" << endl;
	return;
 }
 d->mStartup->slotNewGame(args);
}

void TopWidget::slotStartingFailed()
{
 boDebug() << k_funcinfo << endl;

 // TODO: display reason
 KMessageBox::sorry(this, i18n("Game starting failed"));
 slotGameOver();
}

void TopWidget::slotStartEditor(KCmdLineArgs* args)
{
 BO_CHECK_NULL_RET(boGame);
 if (!d->mStartup) {
	boError() << k_funcinfo << "NULL startup widget" << endl;
	return;
 }
 d->mStartup->slotStartEditor(args);
}

void TopWidget::slotDebugKGame()
{
 if (!boGame) {
	boError() << k_funcinfo << "NULL game" << endl;
	return;
 }
 KGameDebugDialog* dlg = new KGameDebugDialog(boGame, this, false);

 QVBox* b = dlg->addVBoxPage(i18n("Debug &Units"));
 KGameUnitDebug* units = new KGameUnitDebug(b);
 units->setBoson(boGame);

 b = dlg->addVBoxPage(i18n("Debug &Boson Players"));
 KGamePlayerDebug* player = new KGamePlayerDebug(b);
 player->setBoson(boGame);

 if (boGame->playField()) {
	BosonMap* map = boGame->playField()->map();
	if (!map) {
		boError() << k_funcinfo << "NULL map" << endl;
		return;
	}
	b = dlg->addVBoxPage(i18n("Debug &Cells"));
	KGameCellDebug* cells = new KGameCellDebug(b);
	cells->setMap(map);
 }

 connect(dlg, SIGNAL(signalRequestIdName(int,bool,QString&)),
		this, SLOT(slotDebugRequestIdName(int,bool,QString&)));

 displayNonModalDialog(dlg);
}

void TopWidget::slotProfiling()
{
 BosonProfilingDialog* dlg = new BosonProfilingDialog(this, false); // note that dialog won't get updated while it is running, even if its non-modal!
 displayNonModalDialog(dlg);
}

void TopWidget::displayNonModalDialog(KDialogBase* dialog)
{
 connect(dialog, SIGNAL(finished()), dialog, SLOT(deleteLater()));
 dialog->show();
}

void TopWidget::slotDebugRequestIdName(int msgid, bool , QString& name)
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

