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
#include "bosonmap.h"
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
		mLoadingWidget = 0;
		mBosonWidget = 0;
	};

	BosonWelcomeWidget* mWelcome;
	BosonNewGameWidget* mNewGame;
	BosonStartEditorWidget* mStartEditor;
	BosonNetworkOptionsWidget* mNetworkOptions;
	BosonLoadingWidget* mLoadingWidget;
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
 BosonMusic::initBosonMusic();
 boMusic->setSound(boConfig->sound());
 boMusic->setMusic(boConfig->music());

// setMinimumWidth(640);
// setMinimumHeight(480);

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
 if (boGame) {
	editor = !boGame->gameMode();
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
	kdWarning() << k_funcinfo << "Oops - Boson object already present! deleting..." << endl;
	Boson::deleteBoson();
 }
 Boson::initBoson();

 // new games are handled in this order: ADMIN clicks on start games - this
 // sends an IdStartGame over network. Once this is received signalStartGame()
 // is emitted and we start here
 connect(boGame, SIGNAL(signalStartNewGame()), this, SLOT(slotStartNewGame()));

 // this signal gets emitted when starting a game (new games as well as loading
 // games). the admin sends the map and all clients (including admin) will
 // receive and use it.
 connect(boGame, SIGNAL(signalInitMap(const QByteArray&)), this, SLOT(slotReceiveMap(const QByteArray&)));
}

void TopWidget::initPlayer()
{
 Player* p = new Player;
 slotChangeLocalPlayer(p);
}

void TopWidget::initPlayField()
{
 mPlayField = new BosonPlayField(this);
}

void TopWidget::initCanvas()
{
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

void TopWidget::initStartupWidget(int id)
{
 if (mWs->widget(id)) {
	// already initialized
	return;
 }
 if (id == ID_WIDGETSTACK_BOSONWIDGET) {
	// bosonwidget gets initialized from within game starting, not
	// from here
	kdError() << k_funcinfo << "BosonWidget must be initialized directly using initBosonWidget() !" << endl;
	return;
 }
 BosonStartupBaseWidget* startup = new BosonStartupBaseWidget(mWs);
 mWs->addWidget(startup, id);
 QWidget* w = 0;
 switch (id) {
	case ID_WIDGETSTACK_WELCOME:
	{
		d->mWelcome = new BosonWelcomeWidget(startup->plainWidget());
		w = d->mWelcome;
		connect(d->mWelcome, SIGNAL(signalNewGame()), this, SLOT(slotNewGame()));
		connect(d->mWelcome, SIGNAL(signalLoadGame()), this, SLOT(slotLoadGame()));
		connect(d->mWelcome, SIGNAL(signalStartEditor()), this, SLOT(slotStartEditor()));
		connect(d->mWelcome, SIGNAL(signalQuit()), this, SLOT(close()));
		break;
	}
	case ID_WIDGETSTACK_NEWGAME:
	{
		d->mNewGame = new BosonNewGameWidget(this, startup->plainWidget());
		w = d->mNewGame;
		connect(d->mNewGame, SIGNAL(signalCancelled()),
				this, SLOT(slotShowMainMenu()));
		connect(d->mNewGame, SIGNAL(signalShowNetworkOptions()),
				this, SLOT(slotShowNetworkOptions()));
		d->mNewGame->show();
		break;
	}
	case ID_WIDGETSTACK_STARTEDITOR:
	{
		d->mStartEditor = new BosonStartEditorWidget(this, startup->plainWidget());
		w = d->mStartEditor;
		connect(d->mStartEditor, SIGNAL(signalCancelled()), this, SLOT(slotShowMainMenu()));
		break;
	}
	case ID_WIDGETSTACK_NETWORK:
	{
		d->mNetworkOptions = new BosonNetworkOptionsWidget(this, startup->plainWidget());
		w = d->mNetworkOptions;
		connect(d->mNetworkOptions, SIGNAL(signalOkClicked()),
				this, SLOT(slotHideNetworkOptions()));
		break;
	}
	case ID_WIDGETSTACK_LOADING:
	{
		d->mLoadingWidget = new BosonLoadingWidget(startup->plainWidget());
		w = d->mLoadingWidget;

		// If we're loading game, we don't know number of players here
		// If game is loaded, we disable progressbar in loading widget, but still set
		// steps and progress to make code less messy (it's better than having
		// if (!mLoading) { ... }  everywhere)
		d->mLoadingWidget->setTotalSteps(3400, boGame->playerCount());
		d->mLoadingWidget->setProgress(0);
		break;
	}
	case ID_WIDGETSTACK_BOSONWIDGET: // gets loaded elsewhere
	default:
	{
		mWs->removeWidget(startup);
		delete startup;
		kdWarning() << k_funcinfo << "Invalid id " << id << endl;
		return;
	}
 }
 if (!w) {
	kdError() << k_funcinfo << "NULL widget" << endl;
	return;
 }
 w->installEventFilter(this);
 startup->initBackgroundOrigin();
}

void TopWidget::showStartupWidget(int id)
{
 initStartupWidget(id);
 raiseWidget(id);
 switch (id) {
	case ID_WIDGETSTACK_WELCOME:
	case ID_WIDGETSTACK_NEWGAME:
	case ID_WIDGETSTACK_STARTEDITOR:
		break;
	case ID_WIDGETSTACK_BOSONWIDGET:
	case ID_WIDGETSTACK_NETWORK:
	case ID_WIDGETSTACK_LOADING:
		showHideMenubar();
		break;
	default:
		kdWarning() << k_funcinfo << "Invalid id " << id << endl;
		break;
 }
}

void TopWidget::initBosonWidget(bool loading)
{
 if (d->mBosonWidget) {
	//should not happen!
	kdWarning() << k_funcinfo << "widget already allocated!" << endl;
	return;
 }
 if (boGame->gameMode()) {
	BosonWidget* w = new BosonWidget(this, mWs, loading);
	connect(w, SIGNAL(signalSaveGame()), this, SLOT(slotSaveGame()));
	connect(w, SIGNAL(signalLoadGame()), this, SLOT(slotLoadGame()));
	d->mBosonWidget = w;
 } else {
	EditorWidget* w = new EditorWidget(this, mWs, loading);
	d->mBosonWidget = w;
 }
 changeLocalPlayer(player(), false);
 d->mBosonWidget->init(); // this depends on several virtual methods and therefore can't be called in the c'tor
 factory()->addClient(d->mBosonWidget); // XMLClient-stuff. needs to be called *after* creation of KAction objects, so outside BosonWidget might be a good idea :-)
// createGUI("bosonui.rc", false);
 mWs->addWidget(d->mBosonWidget, ID_WIDGETSTACK_BOSONWIDGET);

 connect(d->mBosonWidget, SIGNAL(signalChangeLocalPlayer(Player*)), this, SLOT(slotChangeLocalPlayer(Player*)));
 connect(d->mBosonWidget, SIGNAL(signalEndGame()), this, SLOT(slotEndGame()));
 connect(d->mBosonWidget, SIGNAL(signalQuit()), this, SLOT(slotGameOver()));
 connect(d->mBosonWidget, SIGNAL(signalMobilesCount(int)), this, SIGNAL(signalSetMobilesCount(int)));
 connect(d->mBosonWidget, SIGNAL(signalFacilitiesCount(int)), this, SIGNAL(signalSetFacilitiesCount(int)));
 connect(d->mBosonWidget, SIGNAL(signalOilUpdated(int)), this, SIGNAL(signalOilUpdated(int)));
 connect(d->mBosonWidget, SIGNAL(signalMineralsUpdated(int)), this, SIGNAL(signalMineralsUpdated(int)));


 // finally add the initial display
 // note that this call also loads the cursor
 d->mBosonWidget->addInitialDisplay();
}

void TopWidget::slotNewGame()
{
 showStartupWidget(ID_WIDGETSTACK_NEWGAME);
}

void TopWidget::slotStartEditor()
{
 showStartupWidget(ID_WIDGETSTACK_STARTEDITOR);
}

void TopWidget::slotStartNewGame()
{
 mLoading = false;
 showStartupWidget(ID_WIDGETSTACK_LOADING);

 if (boGame->isAdmin()) {
	d->mLoadingWidget->setLoading(BosonLoadingWidget::SendMap);
	QByteArray buffer;
	QDataStream stream(buffer, IO_WriteOnly);
	mPlayField->saveMap(stream);
	d->mLoadingWidget->setProgress(50);
	checkEvents();
	// send the loaded map via network
	boGame->sendMessage(stream, BosonMessage::InitMap);
	d->mLoadingWidget->setProgress(100);
	checkEvents();
 }

 initCanvas();
 initBosonWidget(false);

 // before actually starting the game we need to wait for the map (which is sent
 // by the ADMIN)
 d->mLoadingWidget->setLoading(BosonLoadingWidget::ReceiveMap);
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
 boGame->save(s, true);
 f.close();
}

void TopWidget::slotLoadGame()
{
 kdDebug() << k_funcinfo << " START" << endl;

 // Get filename
 QString loadingFileName = KFileDialog::getOpenFileName(QString::null, "*.bsg|Boson SaveGame", this);
 if (loadingFileName == QString::null) {
	return;
 }

 showStartupWidget(ID_WIDGETSTACK_LOADING);

 // This must be called manually as we don't send IdNewGame message
 boGame->setGameMode(true);

 initCanvas();
 initBosonWidget(true);

 // Start loading
 // FIXME: this should be a slot and it should be called from network, just as
 // slotStartNewGame() gets called. This way we could support loading network
 // games.
 slotLoadGame(loadingFileName);

 kdDebug() << k_funcinfo << "END" << endl;
}

void TopWidget::slotLoadGame(const QString& loadingFileName)
{
 // If mLoading true, then we're loading saved game; if it's false, we're
 //  starting new game
 mLoading = true;
 if (loadingFileName == QString::null) {
	kdError() << k_funcinfo << "Cannot load game with NULL filename" << endl;
	// do not return - we'll return to the welcome widget below
	// anyway
 }
 // We are loading a saved game
 // Delete mPlayer aka local player because it will be set later by Boson
 //  (It's not known yet)
 delete mPlayer;
 mPlayer = 0;
 slotChangeLocalPlayer(0);

 // Open file and QDataStream on it
 QFile f(loadingFileName);
 f.open(IO_ReadOnly);
 QDataStream s(&f);

 // Load game
 d->mLoadingWidget->setLoading(BosonLoadingWidget::LoadMap);
 d->mLoadingWidget->showProgressBar(false);
 bool loaded = boGame->load(s, true);

 // Close file
 f.close();

 // Check if loading was completed without errors
 if (!loaded) {
	// There was a loading error
	// Find out what went wrong...
	Boson::LoadingStatus status = boGame->loadingStatus();
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
	showStartupWidget(ID_WIDGETSTACK_WELCOME);
	mLoading = false;
 }
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
 boGame->removeAllPlayers();
 initPlayer();
 showStartupWidget(ID_WIDGETSTACK_WELCOME);
}

void TopWidget::slotShowNetworkOptions()
{
 showStartupWidget(ID_WIDGETSTACK_NETWORK);
}

void TopWidget::slotHideNetworkOptions()
{
 disconnect(d->mNetworkOptions);
 delete d->mNetworkOptions;
 d->mNetworkOptions = 0;
 d->mNewGame->slotSetAdmin(boGame->isAdmin());
 showStartupWidget(ID_WIDGETSTACK_NEWGAME);
}

void TopWidget::loadGameData3() // FIXME rename!
{
 boProfiling->start(BosonProfiling::LoadGameData3);

 loadPlayerData(); // FIXME: most of the stuff below should be in this method, too!
 int progress = 0;

 // Load unit datas (pixmaps and sounds), but only if we're starting new game,
 //  because if we're loading saved game, then units are already constructed
 //  and unit datas loaded
 if (!mLoading) {
	QPtrListIterator<KPlayer> it(*(boGame->playerList()));
	progress = 3000;
	while (it.current()) {
		loadUnitDatas(((Player*)it.current()), progress);
		((Player*)it.current())->speciesTheme()->loadTechnologies();
		((Player*)it.current())->speciesTheme()->loadObjectModels();
		++it;
		progress += BosonLoadingWidget::unitDataLoadingFactor();
	}
 }
 mPlayer->speciesTheme()->loadParticles(); // FIXME: why load only particles for one player??

 // these are sounds like minimap activated.
 // FIXME: are there sounds of other player (i.e. non-localplayers) we need,
 // too?
 // FIXME: do we need to support player-independant sounds?
 mPlayer->speciesTheme()->loadGeneralSounds();

 d->mLoadingWidget->setProgress(progress);
 d->mLoadingWidget->setLoading(BosonLoadingWidget::InitGame);
 checkEvents();
 if (boGame->isAdmin() && !mLoading) {
	// Send InitFogOfWar and StartScenario messages if we're starting new game
	if (boGame->gameMode()) {
		boGame->sendMessage(0, BosonMessage::IdInitFogOfWar);
	}
	boGame->sendMessage(0, BosonMessage::IdStartScenario);
 } else if (mLoading) {
	// If we're loading saved game, init fog of war for local player
	d->mBosonWidget->slotInitFogOfWar();
 }

 progress += 100;
 d->mLoadingWidget->setProgress(progress);
 d->mLoadingWidget->setLoading(BosonLoadingWidget::StartingGame);
 checkEvents();

 // Show BosonWidgetBase
 showStartupWidget(ID_WIDGETSTACK_BOSONWIDGET);
 delete d->mNewGame;
 d->mNewGame = 0;
 delete d->mStartEditor;
 d->mStartEditor = 0;

 // Init some stuff
 statusBar()->show();
 d->mBosonWidget->initGameMode();
 enableGameActions(true);
 d->mFpstimer.start(1000);
 connect(&d->mFpstimer, SIGNAL(timeout()), this, SLOT(slotUpdateFPS()));

 // FIXME: why isn't this where d->mBosonWidget new'ed ???
 if (boGame->gameMode()) {
	connect(d->mBosonWidget, SIGNAL(signalGameOver()), this, SLOT(slotGameOver()));
 }

 progress += 300;
 d->mLoadingWidget->setProgress(progress);
 d->mLoadingWidget->setLoading(BosonLoadingWidget::LoadingDone);  // FIXME: This is probably meaningless

 if (mLoading) {
	// These are from BosonWidgetBase::slotStartScenario() which can't be used when
	//  we're loading saved game
	boGame->startGame();
	boGame->sendMessage(0, BosonMessage::IdGameIsStarted);
	boGame->slotSetGameSpeed(BosonConfig::readGameSpeed());
 }

 // mGame indicates that game is running (and BosonWidgetBase shown and game game
 //  actions enabled etc.)
 mGame = true;
 boProfiling->stop(BosonProfiling::LoadGameData3);
}

void TopWidget::loadPlayerData()
{
 // If we're loading saved game, local player isn't set and inited, because it
 //  was not known (not loaded) when BosonWidgetBase was constructed. Set and init
 //  it now
 if (mLoading) {
	kdDebug() << k_funcinfo << "set local player for loaded games now" << endl;
	if (!boGame->localPlayer()) {
		kdWarning() << k_funcinfo << "NULL player" << endl;
	}
	slotChangeLocalPlayer(boGame->localPlayer());
//	d->mBosonWidget->initPlayer();
 }


}

void TopWidget::slotLoadTiles()
{
 // Load map tiles. This takes most startup time

 // This slot method is called from slotReceiveMap(), which in turn is called when map
 // is received in Boson class.
 //
 // Note that slotReceiveMap() calls this using a QTimer::singleShot(), so you
 // can safely use checkEvents() here

 if (!boGame) {
	kdError() << k_funcinfo << "NULL boson object" << endl;
	return;
 }
 if (!playField()) {
	kdError() << k_funcinfo << "NULL playField" << endl;
	return;
 }
 if (!playField()->map()) {
	kdError() << k_funcinfo << "NULL map" << endl;
	return;
 }
 boProfiling->start(BosonProfiling::LoadTiles);

 d->mLoadingWidget->setProgress(600);
 d->mLoadingWidget->setLoading(BosonLoadingWidget::LoadTiles);
 // just in case.. disconnect before connecting. the map should be newly
 // created, bu i don't know if this will stay this way.
 disconnect(playField()->map(), 0, this, 0);
 connect(playField()->map(), SIGNAL(signalTilesLoading(int)), this, SLOT(slotTilesLoading(int)));
 checkEvents();

 // Note that next call doesn't return before tiles are fully loaded (because
 //  second argument is false; if it would be true, then it would return
 //  immediately). This is needed for loading saved game. GUI is
 //  still non-blocking though, because qApp->processEvents() is called while
 //  loading tiles
 playField()->map()->loadTiles(QString("earth"), false);

 d->mLoadingWidget->setProgress(3000);
 QTimer::singleShot(0, this, SLOT(loadGameData3())); // AB: I guess we don't need the singleShot() anymore, but it doesn't really hurt either

 boProfiling->stop(BosonProfiling::LoadTiles);
}

void TopWidget::slotTilesLoading(int tiles)
{
 d->mLoadingWidget->setTileProgress(600, tiles);
 // No checkEvents() here as events are already processed in BosonTiles::???
}

void TopWidget::loadUnitDatas(Player* p, int progress)
{
 // This loads all unit datas for player p
 d->mLoadingWidget->setProgress(progress);
 d->mLoadingWidget->setLoading(BosonLoadingWidget::LoadUnits);
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
	d->mLoadingWidget->setUnitProgress(progress, current, total);
 }
}

void TopWidget::slotReceiveMap(const QByteArray& buffer)
{
 if (!boGame) {
	kdError() << k_funcinfo << "NULL boson object" << endl;
	return;
 }
 if (boGame->gameStatus() != KGame::Init) {
	kdError() << k_funcinfo << "Boson must be in init status to receive map!" << endl;
	kdDebug() << k_funcinfo << "Current status: " << boGame->gameStatus() << endl;
	return;
 }


 QDataStream stream(buffer, IO_ReadOnly);
 mPlayField->loadMap(stream);
 boGame->setPlayField(mPlayField);

 d->mLoadingWidget->setProgress(300);

 // we need to do this with timer because we can't add checkEvents() here. there is a
 // (more or less) KGame bug in KDE < 3.1 that causes KGame to go into an
 // infinite loop when calling checkEvents() from a slot that gets called from a
 // network message (exact: from KMessageDirect::received())
 QTimer::singleShot(0, this, SLOT(slotLoadTiles()));

 // If we're loading game, almost everything except map (players, units...) are
 //  loaded after this method returns. So we set correct loading label
 if (mLoading) {
	d->mLoadingWidget->setLoading(BosonLoadingWidget::LoadGame);
 }
}

void TopWidget::checkEvents()
{
 if (qApp->hasPendingEvents()) {
	qApp->processEvents(100);
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
 Boson::deleteBoson();  // Easiest way to reset game info
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
 showStartupWidget(ID_WIDGETSTACK_WELCOME);
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
	{
		QWidget* w = mWs->widget(id);
		setMinimumSize(w->size());
		setMaximumSize(w->size());
		if (boConfig->showMenubarOnStartup()) {
			showMenubar();
		} else {
			hideMenubar();
		}
		break;
	}
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
 if (o != d->mWelcome &&
		o != d->mLoadingWidget &&
		o != d->mStartEditor &&
		o != d->mNetworkOptions &&
		o != d->mNewGame) {
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



void TopWidget::changeLocalPlayer(Player* p, bool init)
{
 // AB: note that both, p == 0 AND p == mPlayer are valid and must be executed!
 mPlayer = p;
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

