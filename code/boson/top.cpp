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
#include "bosonplayfield.h"
#include "bosoncanvas.h"
#include "bosonmessage.h"
#include "speciestheme.h"
#include "bosonprofiling.h"
#include "bodisplaymanager.h"
#include "bosonbigdisplaybase.h"
#include "startupwidgets/bosonwelcomewidget.h"
#include "startupwidgets/bosonnewgamewidget.h"
#include "startupwidgets/bosonloadingwidget.h"
#include "startupwidgets/bosonstarteditorwidget.h"
#include "startupwidgets/bosonnetworkoptionswidget.h"
#include "startupwidgets/bosonstartupbasewidget.h"
#include "sound/bosonmusic.h"

#include <kapplication.h>
#include <klocale.h>
#include <kstatusbar.h>
#include <kstdaction.h>
#include <kstdgameaction.h>
#include <kaction.h>
#include <kdebug.h>
#include <kkeydialog.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kfiledialog.h>

#include <qcursor.h>
#include <qwidgetstack.h>
#include <qtimer.h>
#include <qhbox.h>
#include <qfile.h>

#define ID_WIDGETSTACK_WELCOME 1
#define ID_WIDGETSTACK_NEWGAME 2
#define ID_WIDGETSTACK_STARTEDITOR 3
#define ID_WIDGETSTACK_BOSONWIDGET 4
#define ID_WIDGETSTACK_NETWORK 5
#define ID_WIDGETSTACK_LOADING 6

class TopWidget::TopWidgetPrivate
{
public:
	TopWidgetPrivate()
	{
		mWelcome = 0;
		mNewGame = 0;
		mStartEditor = 0;
		mNetworkOptions = 0;
		mLoading = 0;
		mBosonWidget = 0;
	};

	BosonWelcomeWidget* mWelcome;
	BosonNewGameWidget* mNewGame;
	BosonStartEditorWidget* mStartEditor;
	BosonNetworkOptionsWidget* mNetworkOptions;
	BosonLoadingWidget* mLoading;
	BosonWidgetBase* mBosonWidget;

	KToggleAction* mActionStatusbar;
	KToggleAction* mActionMenubar;
	KToggleAction* mActionFullScreen;

	QTimer mFpstimer;

#if KDE_VERSION < 310
	bool mLoadingDockConfig;
#endif
};

TopWidget::TopWidget() : KDockMainWindow(0, "topwindow")
{
 d = new TopWidgetPrivate;
 mBoson = 0;
 mPlayer = 0;
 mPlayField = 0;
 mCanvas = 0;
 mGame = false;
#if KDE_VERSION < 310
 d->mLoadingDockConfig = false;
#endif
 mGame = false;
 mLoading = false;

 mMainDock = createDockWidget("mainDock", 0, this, i18n("Map"));
 mWs = new QWidgetStack(mMainDock);
 mMainDock->setWidget(mWs);
 mMainDock->setDockSite(KDockWidget::DockCorner);
 mMainDock->setEnableDocking(KDockWidget::DockNone);

 setView(mMainDock);
 setMainDockWidget(mMainDock);

 BosonConfig::initBosonConfig();
 BosonProfiling::initProfiling();

 setMinimumWidth(640);
 setMinimumHeight(480);

 initMusic();

 initKActions();
 initStatusBar();

 // "re" is not entirely correct ;)
 reinitGame();

 loadInitialDockConfig();
}

TopWidget::~TopWidget()
{
 kdDebug() << k_funcinfo << endl;
 bool editor = false;
 if (mBoson) {
	editor = !mBoson->gameMode();
 }
 boConfig->save(editor);
 kdDebug() << "endGame()" << endl;
 endGame();
 kdDebug() << "endGame() done" << endl;
 delete d;
 kdDebug() << k_funcinfo << "done" << endl;
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
 // note: mBoson and similar are *not* yet constructed!
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
 sound->setChecked(boMusic->sound());
 KToggleAction* music = new KToggleAction(i18n("&Music"), 0, this,
		SLOT(slotToggleMusic()), actionCollection(), "options_music");
 music->setChecked(boMusic->music());

 // Display
 d->mActionFullScreen = new KToggleAction(i18n("&Fullscreen Mode"), CTRL+SHIFT+Key_F,
		this, SLOT(slotToggleFullScreen()), actionCollection(), "window_fullscreen");
 d->mActionFullScreen->setChecked(false);

 createGUI("topui.rc", false);

 hideMenubar();
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
 Player* p = new Player;
 changeLocalPlayer(p);
}

void TopWidget::initPlayField()
{
 mPlayField = new BosonPlayField(this);
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
	kdWarning() << k_lineinfo << "NULL BosonWidgetBase!" << endl;
 }
}

void TopWidget::initWelcomeWidget()
{
 if (d->mWelcome) {
	return;
 }
 BosonStartupBaseWidget* startup = new BosonStartupBaseWidget(mWs);
 mWs->addWidget(startup, ID_WIDGETSTACK_WELCOME);
 d->mWelcome = new BosonWelcomeWidget(startup->plainWidget());
 d->mWelcome->installEventFilter(this);
 startup->initBackgroundOrigin();
 connect(d->mWelcome, SIGNAL(signalNewGame()), this, SLOT(slotNewGame()));
 connect(d->mWelcome, SIGNAL(signalLoadGame()), this, SLOT(slotLoadGame()));
 connect(d->mWelcome, SIGNAL(signalStartEditor()), this, SLOT(slotStartEditor()));
 connect(d->mWelcome, SIGNAL(signalQuit()), this, SLOT(close()));
}

void TopWidget::showWelcomeWidget()
{
 if (!d->mWelcome) {
	initWelcomeWidget();
 }
 raiseWidget(ID_WIDGETSTACK_WELCOME);
}

void TopWidget::initNewGameWidget()
{
 if (d->mNewGame) {
	return;
 }
 BosonStartupBaseWidget* startup = new BosonStartupBaseWidget(mWs);
 mWs->addWidget(startup, ID_WIDGETSTACK_NEWGAME);
 d->mNewGame = new BosonNewGameWidget(this, startup->plainWidget());
 d->mNewGame->installEventFilter(this);
 startup->initBackgroundOrigin();
 connect(d->mNewGame, SIGNAL(signalCancelled()), this, SLOT(slotShowMainMenu()));
 connect(d->mNewGame, SIGNAL(signalShowNetworkOptions()), this, SLOT(slotShowNetworkOptions()));
 d->mNewGame->show();
}

void TopWidget::showNewGameWidget()
{
 if (!d->mNewGame) {
	initNewGameWidget();
 }
 raiseWidget(ID_WIDGETSTACK_NEWGAME);
}

void TopWidget::initStartEditorWidget()
{
 if (d->mStartEditor) {
	return;
 }
 BosonStartupBaseWidget* startup = new BosonStartupBaseWidget(mWs);
 mWs->addWidget(startup, ID_WIDGETSTACK_STARTEDITOR);
 d->mStartEditor = new BosonStartEditorWidget(this, startup->plainWidget());
 d->mStartEditor->installEventFilter(this);
 startup->initBackgroundOrigin();
 connect(d->mStartEditor, SIGNAL(signalCancelled()), this, SLOT(slotShowMainMenu()));
}

void TopWidget::showStartEditorWidget()
{
 if (!d->mStartEditor) {
	initStartEditorWidget();
 }
 raiseWidget(ID_WIDGETSTACK_STARTEDITOR);
}

void TopWidget::initBosonWidget(bool loading)
{
 if (d->mBosonWidget) {
	//should not happen!
	kdWarning() << k_funcinfo << "widget already allocated!" << endl;
	return;
 }
 if (game()->gameMode()) {
	BosonWidget* w = new BosonWidget(this, mWs, loading);
	connect(w, SIGNAL(signalSaveGame()), this, SLOT(slotSaveGame()));
	connect(w, SIGNAL(signalLoadGame()), this, SLOT(slotLoadGame()));
	d->mBosonWidget = w;
 } else {
	EditorWidget* w = new EditorWidget(this, mWs, loading);
	d->mBosonWidget = w;
 }
 d->mBosonWidget->setLocalPlayer(player());
 d->mBosonWidget->init(); // this depends on several virtual methods and therefore can't be called in the c'tor
 factory()->addClient(d->mBosonWidget); // XMLClient-stuff. needs to be called *after* creation of KAction objects, so outside BosonWidget might be a good idea :-)
// createGUI("bosonui.rc", false);
 mWs->addWidget(d->mBosonWidget, ID_WIDGETSTACK_BOSONWIDGET);

 connect(d->mBosonWidget, SIGNAL(signalEndGame()), this, SLOT(slotEndGame()));
 connect(d->mBosonWidget, SIGNAL(signalQuit()), this, SLOT(slotGameOver()));
 connect(d->mBosonWidget, SIGNAL(signalMobilesCount(int)), this, SIGNAL(signalSetMobilesCount(int)));
 connect(d->mBosonWidget, SIGNAL(signalFacilitiesCount(int)), this, SIGNAL(signalSetFacilitiesCount(int)));
 connect(d->mBosonWidget, SIGNAL(signalOilUpdated(int)), this, SIGNAL(signalOilUpdated(int)));
 connect(d->mBosonWidget, SIGNAL(signalMineralsUpdated(int)), this, SIGNAL(signalMineralsUpdated(int)));
}

void TopWidget::showBosonWidget()
{
 if (!d->mBosonWidget) {
	initBosonWidget();
 }
 raiseWidget(ID_WIDGETSTACK_BOSONWIDGET);
 showHideMenubar();
}

void TopWidget::initNetworkOptions()
{
 if (d->mNetworkOptions) {
	return;
 }
 BosonStartupBaseWidget* startup = new BosonStartupBaseWidget(mWs);
 mWs->addWidget(startup, ID_WIDGETSTACK_NETWORK);
 d->mNetworkOptions = new BosonNetworkOptionsWidget(this, startup->plainWidget());
 d->mNetworkOptions->installEventFilter(this);
 startup->initBackgroundOrigin();
 connect(d->mNetworkOptions, SIGNAL(signalOkClicked()), this, SLOT(slotHideNetworkOptions()));
}

void TopWidget::showNetworkOptions()
{
 if (!d->mNetworkOptions) {
	initNetworkOptions();
 }
 raiseWidget(ID_WIDGETSTACK_NETWORK);
 showHideMenubar();
}

void TopWidget::initLoadingWidget()
{
 if (d->mLoading) {
	return;
 }
 BosonStartupBaseWidget* startup = new BosonStartupBaseWidget(mWs);
 mWs->addWidget(startup, ID_WIDGETSTACK_LOADING);
 d->mLoading = new BosonLoadingWidget(startup->plainWidget());
 d->mLoading->installEventFilter(this);
 startup->initBackgroundOrigin();
}

void TopWidget::showLoadingWidget()
{
 if (!d->mLoading) {
	initLoadingWidget();
 }
 raiseWidget(ID_WIDGETSTACK_LOADING);
 showHideMenubar();
}

void TopWidget::slotNewGame()
{
 showNewGameWidget();
}

void TopWidget::slotStartEditor()
{
 showStartEditorWidget();
}

void TopWidget::slotStartGame()
{
 mLoading = false;
 showLoadingWidget();
 loadGameData1();
}

void TopWidget::slotSaveGame()
{
 QString file = KFileDialog::getSaveFileName(QString::null, "*.bsg|Boson SaveGame", this);
 if (file == QString::null) {
	return;
 }
 if (file.findRev('.') == -1) {
	file += ".bsg";
 }

 QFile f(file);
 f.open(IO_WriteOnly);
 QDataStream s(&f);
 mBoson->save(s, true);
 f.close();
}

void TopWidget::slotLoadGame()
{
 kdDebug() << k_funcinfo << " START" << endl;

 // Get filename
 mLoadingFileName = KFileDialog::getOpenFileName(QString::null, "*.bsg|Boson SaveGame", this);
 if (mLoadingFileName == QString::null) {
	return;
 }

 // If mLoading is true, then we're loading saved game; if it's false, we're
 //  starting new game
 mLoading = true;
 showLoadingWidget();

 // This must be called manually as we don't send IdNewGame message
 mBoson->setGameMode(true);

 // Start loading
 loadGameData1();

 kdDebug() << k_funcinfo << "END" << endl;
}

void TopWidget::slotShowMainMenu()
{
 if (d->mNewGame) {
	disconnect(d->mNewGame);
	delete d->mNewGame;
	d->mNewGame = 0;
 }
 if (d->mStartEditor) {
	delete d->mStartEditor;
	d->mStartEditor = 0;
 }
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
 // FIXME: all loadGameData*() methods are very messy and complex (partially
 //  because of loading support). Clean them up!

 // If we're loading game, we don't know number of players here
 // If game is loaded, we disable progressbar in loading widget, but still set
 //  steps and progress to make code less messy (it's better than having
 //  if (!mLoading) { ... }  everywhere)
 boProfiling->start(BosonProfiling::LoadGameData1);
 d->mLoading->setSteps(3400 + mBoson->playerCount() * UNITDATAS_LOADINGFACTOR);
 d->mLoading->setProgress(0);
 checkEvents();

 connect(mBoson, SIGNAL(signalInitMap(const QByteArray&)), this, SLOT(slotReceiveMap(const QByteArray&)));
 if (!mLoading) {
	// If we're starting new game, receive map (first send it if we're admin)
	if (mBoson->isAdmin()) {
		d->mLoading->setLoading(BosonLoadingWidget::SendMap);
		checkEvents();
		QByteArray buffer;
		QDataStream stream(buffer, IO_WriteOnly);
		mPlayField->saveMap(stream);
		d->mLoading->setProgress(50);
		checkEvents();
		// send the loaded map via network
		mBoson->sendMessage(stream, BosonMessage::InitMap);
		d->mLoading->setProgress(100);
		checkEvents();
	}
	d->mLoading->setLoading(BosonLoadingWidget::ReceiveMap);
 } else {
	// We are loading a saved game
	// Delete mPlayer aka local player because it will be set later by Boson
	//  (It's not known yet)
	delete mPlayer;
	mPlayer = 0;
	changeLocalPlayer(0);

	// Open file and QDataStream on it
	QFile f(mLoadingFileName);
	f.open(IO_ReadOnly);
	QDataStream s(&f);

	// Load game
	d->mLoading->setLoading(BosonLoadingWidget::LoadMap);
	d->mLoading->showProgressBar(false);
	bool loaded = mBoson->load(s, true);

	// Close file
	f.close();

	// Check if loading was completed without errors
	if (!loaded) {
		// There was a loading error
		// Find out what went wrong...
		Boson::LoadingStatus status = mBoson->loadingStatus();
		QString text, caption;
		if (status == Boson::InvalidFileFormat || status == Boson::InvalidCookie) {
			text = i18n("This file is not Boson SaveGame!");
			caption = i18n("Invalid file format!");
		} else if (status == Boson::InvalidVersion) {
			text = i18n("This file has unsupported saving format!\n"
					"Probably it is saved with too old version of Boson");
			caption = i18n("Unsupported file format version!");
		} else if (status == Boson::KGameError) {
			text = i18n("Error loading saved game!");
			caption = i18n("An error occured while loading saved game!\n"
					"Probably the game wasn't saved properly");
		} else {
			// This should never be reached
			kdError() << k_funcinfo << "Invalid error type or no error (while loading saved game)!!!" << endl;
		}

		// ... and show messagebox
		KMessageBox::sorry(this, text, caption);

		// We also need to re-init player
		initPlayer();

		// Then return to welcome screen
		showWelcomeWidget();
		mLoading = false;
	}
 }
 boProfiling->stop(BosonProfiling::LoadGameData1);
}

void TopWidget::loadGameData2() //FIXME rename!
{
 // This method is called from slotInitMap(), which in turn is called when map
 //  is received in Boson class
 // When map is received then, when we're starting new game, loadGameData1()
 //  has already returned, if we're loading saved game, then it hasn't, because
 //  map is initialized immediately when it's loaded from file

 boProfiling->start(BosonProfiling::LoadGameData2);
 // Init canvas
 d->mLoading->setLoading(BosonLoadingWidget::InitClasses);
 checkEvents();
 mCanvas = new BosonCanvas(this);
 mBoson->setCanvas(mCanvas);

 // Init BosonWidgetBase
 initBosonWidget(mLoading);

 // Load map tiles. This takes most time
 d->mLoading->setProgress(600);
 d->mLoading->setLoading(BosonLoadingWidget::LoadTiles);
 connect(mCanvas, SIGNAL(signalTilesLoading(int)), this, SLOT(slotCanvasTilesLoading(int)));
 connect(mCanvas, SIGNAL(signalTilesLoaded()), this, SLOT(slotCanvasTilesLoaded()));
 checkEvents();
 // Note that next call doesn't return before tiles are fully loaded (because
 //  second argument is false; if it would be true, then it would return
 //  immediately). This is needed for loading saved game. GUI is
 //  still non-blocking though, because qApp->processEvents() is called while
 //  loading tiles
 mCanvas->loadTiles(QString("earth"), false);

 // also loads the cursor and the map display list!
 d->mBosonWidget->addInitialDisplay();
 boProfiling->stop(BosonProfiling::LoadGameData2);
}

void TopWidget::loadGameData3() // FIXME rename!
{
 boProfiling->start(BosonProfiling::LoadGameData3);
 // If we're loading saved game, local player isn't set and inited, because it
 //  was not known (not loaded) when BosonWidgetBase was constructed. Set and init
 //  it now
 if (mLoading) {
	kdDebug() << k_funcinfo << "set local player for loaded games now" << endl;
	if (!mBoson->localPlayer()) {
		kdWarning() << k_funcinfo << "NULL player" << endl;
	}
	changeLocalPlayer(mBoson->localPlayer());
	d->mBosonWidget->initPlayer();
 }

 int progress = 0;

 // Load unit datas (pixmaps and sounds), but only if we're starting new game,
 //  because if we're loading saved game, then units are already constructed
 //  and unit datas loaded
 if (!mLoading) {
	QPtrListIterator<KPlayer> it(*(mBoson->playerList()));
	progress = 3000;
	while (it.current()) {
		loadUnitDatas(((Player*)it.current()), progress);
		((Player*)it.current())->speciesTheme()->loadTechnologies();
		++it;
		progress += UNITDATAS_LOADINGFACTOR;
	}
 }
 // these are sounds like minimap activated. 
 // FIXME: are there sounds of other player (i.e. non-localplayers) we need,
 // too?
 // FIXME: do we need to support player-independant sounds?
 mPlayer->speciesTheme()->loadGeneralSounds();

 d->mLoading->setProgress(progress);
 d->mLoading->setLoading(BosonLoadingWidget::InitGame);
 checkEvents();
 if (mBoson->isAdmin() && !mLoading) {
	// Send InitFogOfWar and StartScenario messages if we're starting new game
	if (mBoson->gameMode()) {
		mBoson->sendMessage(0, BosonMessage::IdInitFogOfWar);
	}
	mBoson->sendMessage(0, BosonMessage::IdStartScenario);
 } else if (mLoading) {
	// If we're loading saved game, init fog of war for local player
	d->mBosonWidget->slotInitFogOfWar();
 }

 progress += 100;
 d->mLoading->setProgress(progress);
 d->mLoading->setLoading(BosonLoadingWidget::StartingGame);
 checkEvents();

 // Show BosonWidgetBase
 showBosonWidget();
 if (d->mNewGame) {
	delete d->mNewGame;
	d->mNewGame = 0;
 }
 if (d->mStartEditor) {
	delete d->mStartEditor;
	d->mStartEditor = 0;
 }
 // Init some stuff
 statusBar()->show();
 d->mBosonWidget->initGameMode();
 enableGameActions(true);
 d->mFpstimer.start(1000);
 connect(&d->mFpstimer, SIGNAL(timeout()), this, SLOT(slotUpdateFPS()));

 // FIXME: why isn't this where d->mBosonWidget new'ed ???
 if (game()->gameMode()) {
	connect(d->mBosonWidget, SIGNAL(signalGameOver()), this, SLOT(slotGameOver()));
 }

 progress += 300;
 d->mLoading->setProgress(progress);
 d->mLoading->setLoading(BosonLoadingWidget::LoadingDone);  // FIXME: This is probably meaningless

 if (mLoading) {
	// These are from BosonWidgetBase::slotStartScenario() which can't be used when
	//  we're loading saved game
	mBoson->startGame();
	mBoson->sendMessage(0, BosonMessage::IdGameIsStarted);
	mBoson->slotSetGameSpeed(BosonConfig::readGameSpeed());
 }

 // mGame indicates that game is running (and BosonWidgetBase shown and game game
 //  actions enabled etc.)
 mGame = true;
 boProfiling->stop(BosonProfiling::LoadGameData3);
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
 // This loads all unit datas for player p
 d->mLoading->setProgress(progress);
 d->mLoading->setLoading(BosonLoadingWidget::LoadUnits);
 checkEvents();
 // First get all id's of units
 QValueList<unsigned long int> unitIds = p->speciesTheme()->allFacilities();
 unitIds += p->speciesTheme()->allMobiles();
 QValueList<unsigned long int>::iterator it;
 int current = 0;
 int total = unitIds.count();
 for (it = unitIds.begin(); it != unitIds.end(); ++it) {
	current++;
	p->speciesTheme()->loadUnit(*it);
	d->mLoading->setProgress((int)(progress + ((double)current / total * UNITDATAS_LOADINGFACTOR)));
 }
}

void TopWidget::slotReceiveMap(const QByteArray& buffer)
{
 disconnect(mBoson, SIGNAL(signalInitMap(const QByteArray&)), this, SLOT(slotReceiveMap(const QByteArray&)));
 QDataStream stream(buffer, IO_ReadOnly);
 mPlayField->loadMap(stream);
 mBoson->setPlayField(mPlayField);

 d->mLoading->setProgress(300);

 checkEvents();
 loadGameData2();

 // If we're loading game, almost everything except map (players, units...) are
 //  loaded after this method returns. So we set correct loading label
 if (mLoading) {
	d->mLoading->setLoading(BosonLoadingWidget::LoadGame);
 }
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
 kdDebug() << k_funcinfo << endl;
 if (d->mBosonWidget) {
	d->mBosonWidget->quitGame();
	disconnect(d->mBosonWidget, 0, 0, 0);
	d->mFpstimer.stop();
	disconnect(&d->mFpstimer, 0, 0, 0);
	saveGameDockConfig();
 }
 // Delete all objects
 delete d->mBosonWidget;
 d->mBosonWidget = 0;
 delete mBoson;  // Easiest way to reset game info
 mBoson = 0;
 delete mCanvas;
 mCanvas = 0;
 delete mPlayField;
 mPlayField = 0;
 kdDebug() << k_funcinfo << "done" << endl;
}

void TopWidget::reinitGame()
{
 initBoson();
 initPlayer();
 initPlayField();

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
 reinitGame();
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
 if (!mWs->visibleWidget()) {
	return;
 }
 int id = mWs->id(mWs->visibleWidget());
 switch (id) {
	case ID_WIDGETSTACK_WELCOME:
	case ID_WIDGETSTACK_NEWGAME:
	case ID_WIDGETSTACK_LOADING:
	case ID_WIDGETSTACK_NETWORK:
		boConfig->setShowMenubarOnStartup(d->mActionMenubar->isChecked());
		break;
	case ID_WIDGETSTACK_BOSONWIDGET:
		boConfig->setShowMenubarInGame(d->mActionMenubar->isChecked());
		break;
	case ID_WIDGETSTACK_STARTEDITOR:
		// editor without menubar is pretty useless, i guess
		break;
	default:
		kdDebug() << k_funcinfo << "unknown id " << id << endl;
		break;
 }
 showHideMenubar();
/*
 if (d->mActionMenubar->isChecked()) {
	menuBar()->show();
 } else {
	menuBar()->hide();
 }
*/
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
 if (mGame) {
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
 QWidget* w = mWs->visibleWidget();
 if (!w) {
	return true;
 }
 switch (mWs->id(w)) {
	case ID_WIDGETSTACK_BOSONWIDGET:
		d->mBosonWidget->saveConfig();
		d->mBosonWidget->quitGame();
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
		if (boConfig->showMenubarOnStartup()) {
			showMenubar();
		} else {
			hideMenubar();
		}
		break;
	case ID_WIDGETSTACK_NEWGAME:
	case ID_WIDGETSTACK_LOADING:
	case ID_WIDGETSTACK_NETWORK:
		setMinimumSize(BOSON_MINIMUM_WIDTH, BOSON_MINIMUM_HEIGHT);
		setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
		if (boConfig->showMenubarOnStartup()) {
			showMenubar();
		} else {
			hideMenubar();
		}
		break;
	case ID_WIDGETSTACK_BOSONWIDGET:
		if (boConfig->showMenubarInGame()) {
			showMenubar();
		} else {
			hideMenubar();
		}
		setMinimumSize(BOSON_MINIMUM_WIDTH, BOSON_MINIMUM_HEIGHT);
		setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
		break;
	case ID_WIDGETSTACK_STARTEDITOR:
		showMenubar();
		setMinimumSize(BOSON_MINIMUM_WIDTH, BOSON_MINIMUM_HEIGHT);
		setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
		break;
	default:
		kdDebug() << k_funcinfo << "unknown id " << id << endl;
		break;
 }
 mWs->raiseWidget(id);
}

void TopWidget::slotUpdateFPS()
{
 emit signalFPSUpdated(d->mBosonWidget->displayManager()->activeDisplay()->fps());// damn this call sucks!
}

bool TopWidget::eventFilter(QObject* o, QEvent* e)
{
 // we display the popup widget for startup widgets only
 if (o != d->mWelcome && o != d->mLoading && o != d->mStartEditor &&
		o != d->mNetworkOptions && o != d->mNewGame) {
	return false;
 }
 // FIXME: we should display for the BosonStartupBaseWidgets, too

 switch (e->type()) {
	case QEvent::MouseButtonPress:
		if (((QMouseEvent*)e)->button() == RightButton) {
			QPopupMenu* p = (QPopupMenu*)factory()->container("welcomepopup", this);
			if (p) {
				p->popup(QCursor::pos());
			}
			return true;
		} else {
			return false;
		}
	default:
		return false;
 }
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
 if (!mWs->visibleWidget()) {
	return;
 }
 int id = mWs->id(mWs->visibleWidget());
 switch (id) {
	case ID_WIDGETSTACK_WELCOME:
	case ID_WIDGETSTACK_NEWGAME:
	case ID_WIDGETSTACK_LOADING:
	case ID_WIDGETSTACK_NETWORK:
		if (boConfig->showMenubarOnStartup()) {
			showMenubar();
		} else {
			hideMenubar();
		}
		break;
	case ID_WIDGETSTACK_BOSONWIDGET:
		if (boConfig->showMenubarInGame()) {
			showMenubar();
		} else {
			hideMenubar();
		}
		break;
	case ID_WIDGETSTACK_STARTEDITOR:
		// editor without menubar is pretty useless, i guess
		showMenubar();
		break;
	default:
		kdDebug() << k_funcinfo << "unknown id " << id << endl;
		break;
 }
}



void TopWidget::changeLocalPlayer(Player* p)
{
 mPlayer = p;
 if (d->mBosonWidget) {
	d->mBosonWidget->setLocalPlayer(p);
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

