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

#include "defines.h"
#include "bosonwidget.h"
#include "editorwidget.h"
#include "bosonconfig.h"
#include "boson.h"
#include "player.h"
#include "boitemlisthandler.h"
#include "bosonplayfield.h"
#include "bosoncanvas.h"
#include "bosonmessage.h"
#include "speciestheme.h"
#include "bosonprofiling.h"
#include "bodisplaymanager.h"
#include "bosonstarting.h"
#include "rtti.h"
#include "bodebug.h"
#include "bodebugdcopiface.h"
#include "startupwidgets/bosonstartupwidget.h"
#include "sound/bosonmusic.h"
#include "info/boinfo.h"

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

#include <qcursor.h>
#include <qwidgetstack.h>
#include <qtimer.h>
#include <qhbox.h>

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
	};

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
};

TopWidget::TopWidget() : KDockMainWindow(0, "topwindow")
{
 d = new TopWidgetPrivate;
 mPlayField = 0;
 mCanvas = 0;
#if KDE_VERSION < 310
 d->mLoadingDockConfig = false;
#endif

 mMainDock = createDockWidget("mainDock", 0, this, i18n("Map"));
 mMainDock->setDockSite(KDockWidget::DockCorner);
 mMainDock->setEnableDocking(KDockWidget::DockNone);

 KGlobal::dirs()->addPrefix(BOSON_PREFIX);

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

 BoInfo::initBoInfo();
 BosonConfig::initBosonConfig();
 BosonProfiling::initProfiling();
 BosonMusic::initBosonMusic();
 BoItemListHandler::initStatic();
 boMusic->setSound(boConfig->sound());
 boMusic->setMusic(boConfig->music());

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
 KToggleAction* music = new KToggleAction(i18n("&Music"), 0, this,
		SLOT(slotToggleMusic()), actionCollection(), "options_music");
 music->setChecked(boConfig->music());

 // Display
 d->mActionFullScreen = new KToggleAction(i18n("&Fullscreen Mode"), CTRL+SHIFT+Key_F,
		this, SLOT(slotToggleFullScreen()), actionCollection(), "window_fullscreen");
 d->mActionFullScreen->setChecked(false);

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

 // new games are handled in this order: ADMIN clicks on start games - this
 // sends an IdStartGame over network. Once this is received signalStartNewGame()
 // is emitted and we start here
 connect(boGame, SIGNAL(signalStartNewGame()), this, SLOT(slotStartNewGame()));
 connect(boGame, SIGNAL(signalPlayFieldChanged(const QString&)),
		this, SLOT(slotPlayFieldChanged(const QString&)));
 connect(boGame, SIGNAL(signalGameStarted()), this, SLOT(slotGameStarted()));

 // this signal gets emitted when starting a game (new games as well as loading
 // games). the admin sends the map and all clients (including admin) will
 // receive and use it.
 connect(boGame, SIGNAL(signalInitMap(const QByteArray&)), d->mStarting, SLOT(slotReceiveMap(const QByteArray&)));


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
 mPlayField = new BosonPlayField(this);
}

void TopWidget::initCanvas()
{
 BO_CHECK_NULL_RET(boGame);
 mCanvas = new BosonCanvas(this);
 boGame->setCanvas(mCanvas);
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

 QHBox* fps = new QHBox(bar);
 (void)new QLabel(i18n("FPS: "), fps);
 QLabel* fpsLabel = new QLabel(QString::number(0.0), fps);
 connect(this, SIGNAL(signalFPSUpdated(double)), fpsLabel, SLOT(setNum(double)));
 bar->addWidget(fps);

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
	BosonWidget* w = new BosonWidget(this, mMainDock);
	connect(w, SIGNAL(signalSaveGame()), this, SLOT(slotSaveGame()));
	connect(w, SIGNAL(signalLoadGame()), this, SLOT(slotLoadGame()));
	connect(w, SIGNAL(signalGameOver()), this, SLOT(slotGameOver()));
	d->mBosonWidget = w;
 } else {
	EditorWidget* w = new EditorWidget(this, mMainDock);
	d->mBosonWidget = w;
 }
 d->mBosonWidget->setDisplayManager(d->mDisplayManager);

 // at this point the startup widget should already have called
 // slotChangeLocalPlayer()! if not .. then we're in trouble here...
 if (!boGame->localPlayer()) {
	boWarning() << k_funcinfo << "NULL local player - might cause trouble here!" << endl;
	// do not return, since it might even be allowed for editor (currently
	// it is not)
 }
 changeLocalPlayer(boGame->localPlayer(), false);
 d->mBosonWidget->init(); // this depends on several virtual methods and therefore can't be called in the c'tor
 factory()->addClient(d->mBosonWidget); // XMLClient-stuff. needs to be called *after* creation of KAction objects, so outside BosonWidget might be a good idea :-)
// createGUI("bosonui.rc", false);
// mWs->addWidget(d->mBosonWidget, IdBosonWidget);

 connect(d->mBosonWidget, SIGNAL(signalChangeLocalPlayer(Player*)), this, SLOT(slotChangeLocalPlayer(Player*)));
 connect(d->mBosonWidget, SIGNAL(signalEndGame()), this, SLOT(slotEndGame()));
 connect(d->mBosonWidget, SIGNAL(signalQuit()), this, SLOT(close()));
 connect(d->mBosonWidget, SIGNAL(signalMobilesCount(int)), this, SIGNAL(signalSetMobilesCount(int)));
 connect(d->mBosonWidget, SIGNAL(signalFacilitiesCount(int)), this, SIGNAL(signalSetFacilitiesCount(int)));
 connect(d->mBosonWidget, SIGNAL(signalOilUpdated(int)), this, SIGNAL(signalOilUpdated(int)));
 connect(d->mBosonWidget, SIGNAL(signalMineralsUpdated(int)), this, SIGNAL(signalMineralsUpdated(int)));


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

 d->mStarting->setPlayField(mPlayField);

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
 d->mStartup->showLoadingWidget();

 initCanvas();
 initBosonWidget();

 // This must be called manually as we don't send IdNewGame message
 boGame->setGameMode(true);

 // We are loading a saved game
 changeLocalPlayer(0, true); // remove player from all classes (AB: this should be redundand, as no player should be set currently...)

 if (!d->mStarting) {
	boError() << k_funcinfo << "NULL starting object!!" << endl;
	return;
 }
 d->mStarting->setPlayField(mPlayField);


 // Start loading
 if (!d->mStarting->loadGame(fileName)) {
	// There was a loading error
	// Find out what went wrong...
	Boson::LoadingStatus status = boGame->loadingStatus();
	QString text, caption;
	if (status == Boson::InvalidFileFormat || status == Boson::InvalidCookie) {
		text = i18n("This file is not a Boson SaveGame!");
		caption = i18n("Invalid file format!");
	} else if (status == Boson::InvalidVersion) {
		text = i18n("This file has unsupported saving format!\n"
				"Probably it is saved with too old version of Boson");
		caption = i18n("Unsupported file format version!");
	} else if (status == Boson::KGameError) {
		text = i18n("Error loading saved game!");
		caption = i18n("An error occured while loading saved game!\n"
				"Probably the game wasn't saved properly");
	} else if (status == Boson::InvalidXML || status == Boson::BSGFileError) {
		text = i18n("Error loading saved game!");
		caption = i18n("An error occured while loading saved game!\n"
				"Probably the game wasn't saved properly or this file is not a Boson SaveGame!");
	} else {
		// This should never be reached
		// AB: but it will be! I can't provide valid error codes for all
		// cases at this point of development. feel free to fix all
		// "return false" at all points in loading code...
		text = i18n("Error loading saved game!");
		boError() << k_funcinfo << "Invalid error type or no error (while loading saved game)!!!" << endl;
	}

	// ... and show messagebox
	KMessageBox::sorry(this, text, caption);

	// return to the start and reset the game
	d->mStartup->slotShowWelcomeWidget();
 }

 boDebug() << k_funcinfo << "done" << endl;
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
 boMusic->setSound(!boMusic->sound());
 boConfig->setSound(boMusic->sound());
}

void TopWidget::slotToggleMusic()
{
 boMusic->setMusic(!boMusic->music());
 boConfig->setMusic(boMusic->music());
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
	disconnect(d->mBosonWidget, 0, 0, 0);
	d->mStatusBarTimer.stop();
	// This prevent wrong dock config from getting saved when loading fails
	if (boGame->gameStatus() != KGame::Init) {
		saveGameDockConfig();
	}
 }
 delete d->mBosonWidget;
 d->mBosonWidget = 0;
 Boson::deleteBoson();  // Easiest way to reset game info
 delete mCanvas;
 mCanvas = 0;
 delete mPlayField;
 mPlayField = 0;
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
 connect(d->mStarting, SIGNAL(signalLoadingTile(int)),
			d->mStartup, SLOT(slotLoadingTile(int)));
 connect(d->mStarting, SIGNAL(signalLoadingShowProgressBar(bool)),
			d->mStartup, SLOT(slotLoadingShowProgressBar(bool)));

 initBoson();
 // do NOT add a player here! we do that in newgame widget (using a signal)
// initPlayer();
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
 if (boGame->gameStatus() != KGame::Init) {
	// note that even a startup widget might be on top here (e.g. when
	// saving a game)!
	d->mBosonWidget->saveConfig();
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
 BO_CHECK_NULL_RET(mCanvas);
 // AB: some statusbar labels are *not* updated here (e.g. minerals and units),
 // but whenever their value actually changes.
 emit signalFPSUpdated(d->mBosonWidget->displayManager()->fps());
 emit signalParticlesCountUpdated(mCanvas->particleSystemsCount());
 emit signalCanvasItemsCountUpdated(mCanvas->allItemsCount());
 emit signalCanvasAnimationsCountUpdated(mCanvas->animationsCount());

 // AB: this *might* be an expensive calculation. this can be a pretty big loop
 // (> 1000 entries), but there are simple calculations only. maybe we should
 // add a slotUpdateStatusBarExpensive() or so which gets called every 5 seconds
 // only
 mCanvas->updateItemCount();
 emit signalUnitsUpdated(mCanvas->itemCount(RTTI::UnitStart));
 emit signalShotsUpdated(mCanvas->itemCount(RTTI::Shot));
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
 boGame->addPlayer(p);
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

void TopWidget::slotStartEditor(KCmdLineArgs* args)
{
 BO_CHECK_NULL_RET(boGame);
 if (!d->mStartup) {
	boError() << k_funcinfo << "NULL startup widget" << endl;
	return;
 }
 d->mStartup->slotStartEditor(args);
}

