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
#include "bosonstarting.h"
#include "bodebug.h"
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

		mActionStatusbar = 0;
		mActionMenubar = 0;
		mActionFullScreen = 0;

		mStarting = 0;
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

	BosonStarting* mStarting;

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
#if KDE_VERSION < 310
 d->mLoadingDockConfig = false;
#endif

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
 boDebug() << k_funcinfo << endl;
 bool editor = false;
 if (boGame) {
	editor = !boGame->gameMode();
 }
 boConfig->save(editor);
 boDebug() << "endGame()" << endl;
 endGame();
 boDebug() << "endGame() done" << endl;
 delete d;
 boDebug() << k_funcinfo << "done" << endl;
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
 // sends an IdStartGame over network. Once this is received signalStartGame()
 // is emitted and we start here
 connect(boGame, SIGNAL(signalStartNewGame()), this, SLOT(slotStartNewGame()));

 // this signal gets emitted when starting a game (new games as well as loading
 // games). the admin sends the map and all clients (including admin) will
 // receive and use it.
 connect(boGame, SIGNAL(signalInitMap(const QByteArray&)), d->mStarting, SLOT(slotReceiveMap(const QByteArray&)));
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
	boWarning() << k_lineinfo << "NULL BosonWidgetBase!" << endl;
 }
}

void TopWidget::initStartupWidget(StartupWidgetIds id)
{
 if (mWs->widget((int)id)) {
	// already initialized
	return;
 }
 if (id == IdBosonWidget) {
	// bosonwidget gets initialized from within game starting, not
	// from here
	boError() << k_funcinfo << "BosonWidget must be initialized directly using initBosonWidget() !" << endl;
	return;
 }
 BosonStartupBaseWidget* startup = new BosonStartupBaseWidget(mWs);
 mWs->addWidget(startup, (int)id);
 QWidget* w = 0;
 switch (id) {
	case IdWelcome:
	{
		d->mWelcome = new BosonWelcomeWidget(startup->plainWidget());
		w = d->mWelcome;
		connect(d->mWelcome, SIGNAL(signalNewGame()), this, SLOT(slotNewGame()));
		connect(d->mWelcome, SIGNAL(signalLoadGame()), this, SLOT(slotLoadGame()));
		connect(d->mWelcome, SIGNAL(signalStartEditor()), this, SLOT(slotStartEditor()));
		connect(d->mWelcome, SIGNAL(signalQuit()), this, SLOT(close()));
		break;
	}
	case IdNewGame:
	{
		d->mNewGame = new BosonNewGameWidget(this, startup->plainWidget());
		w = d->mNewGame;
		connect(d->mNewGame, SIGNAL(signalCancelled()),
				this, SLOT(slotShowMainMenu()));
		connect(d->mNewGame, SIGNAL(signalShowNetworkOptions()),
				this, SLOT(slotShowNetworkOptions()));
		break;
	}
	case IdStartEditor:
	{
		d->mStartEditor = new BosonStartEditorWidget(this, startup->plainWidget());
		w = d->mStartEditor;
		connect(d->mStartEditor, SIGNAL(signalCancelled()), this, SLOT(slotShowMainMenu()));
		break;
	}
	case IdNetwork:
	{
		d->mNetworkOptions = new BosonNetworkOptionsWidget(startup->plainWidget());
		w = d->mNetworkOptions;
		connect(d->mNetworkOptions, SIGNAL(signalOkClicked()),
				this, SLOT(slotHideNetworkOptions()));
		break;
	}
	case IdLoading:
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
	case IdBosonWidget: // gets loaded elsewhere
	default:
	{
		mWs->removeWidget(startup);
		delete startup;
		boWarning() << k_funcinfo << "Invalid id " << id << endl;
		return;
	}
 }
 if (!w) {
	boError() << k_funcinfo << "NULL widget" << endl;
	return;
 }
 w->installEventFilter(this);
 startup->initBackgroundOrigin();
}

void TopWidget::showStartupWidget(StartupWidgetIds id)
{
 initStartupWidget(id);
 raiseWidget(id);
 switch (id) {
	case IdWelcome:
	case IdNewGame:
	case IdStartEditor:
		break;
	case IdBosonWidget:
	case IdNetwork:
	case IdLoading:
		showHideMenubar();
		break;
	default:
		boWarning() << k_funcinfo << "Invalid id " << id << endl;
		break;
 }
}

void TopWidget::removeStartupWidget(StartupWidgetIds id)
{
 QWidget* w = mWs->widget((int)id);
 if (!w) {
	return;
 }
 delete w; // note: this also deletes child widgets, such as e.g. d->mNewGame for IdNewGame
 switch (id) {
	case IdWelcome:
		d->mWelcome = 0;
		break;
	case IdNewGame:
		d->mNewGame = 0;
		break;
	case IdStartEditor:
		d->mStartEditor = 0;
		break;
	case IdBosonWidget:
		d->mBosonWidget = 0;
		break;
	case IdNetwork:
		d->mNetworkOptions = 0;
		break;
	case IdLoading:
		d->mLoadingWidget = 0;
		break;
	default:
		boWarning() << k_funcinfo << "Invalid id " << id << endl;
		break;
 }
}

void TopWidget::initBosonWidget(bool loading)
{
 if (d->mBosonWidget) {
	//should not happen!
	boWarning() << k_funcinfo << "widget already allocated!" << endl;
	return;
 }
 if (boGame->gameMode()) {
	BosonWidget* w = new BosonWidget(this, mWs, loading);
	connect(w, SIGNAL(signalSaveGame()), this, SLOT(slotSaveGame()));
	connect(w, SIGNAL(signalLoadGame()), this, SLOT(slotLoadGame()));
	connect(w, SIGNAL(signalGameOver()), this, SLOT(slotGameOver()));
	d->mBosonWidget = w;
 } else {
	EditorWidget* w = new EditorWidget(this, mWs, loading);
	d->mBosonWidget = w;
 }
 changeLocalPlayer(player(), false);
 d->mBosonWidget->init(); // this depends on several virtual methods and therefore can't be called in the c'tor
 factory()->addClient(d->mBosonWidget); // XMLClient-stuff. needs to be called *after* creation of KAction objects, so outside BosonWidget might be a good idea :-)
// createGUI("bosonui.rc", false);
 mWs->addWidget(d->mBosonWidget, IdBosonWidget);

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
 showStartupWidget(IdNewGame);
}

void TopWidget::slotStartEditor()
{
 showStartupWidget(IdStartEditor);
}

void TopWidget::slotStartNewGame()
{
 showStartupWidget(IdLoading);

 d->mStarting->setLoadingWidget(d->mLoadingWidget);
 d->mStarting->setPlayField(mPlayField);
 d->mStarting->setLocalPlayer(mPlayer);

 initCanvas();
 initBosonWidget(false);

 // this will take care of all data loading, like models, textures and so. this
 // also initializes the map and will send IdStartScenario - in short this will
 // start the game. Once it's done it'll emit signalStartGame().
 d->mStarting->startNewGame();
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
 if (QFile::exists(file)) {
	int r = KMessageBox::questionYesNo(this, i18n("File already exists. Overwrite?"));
	if (r != KMessageBox::Yes) {
		return;
	}
 }

 QFile f(file);
 f.open(IO_WriteOnly);
 QDataStream s(&f);
 boGame->save(s, true);
 f.close();
}

void TopWidget::slotLoadGame()
{
 boDebug() << k_funcinfo << endl;

 // Get filename
 QString loadingFileName = KFileDialog::getOpenFileName(QString::null, "*.bsg|Boson SaveGame", this);
 if (loadingFileName == QString::null) {
	return;
 }

 // AB: the stuff below should be a separate slot and should be called from
 // network, just as slotStartNewGame() gets called. This way we could 
 // support loading network games.

 showStartupWidget(IdLoading);

 initCanvas();
 initBosonWidget(true);

 // This must be called manually as we don't send IdNewGame message
 boGame->setGameMode(true);

 // We are loading a saved game
 // Delete mPlayer aka local player because it will be set later by Boson
 //  (It's not known yet)
 Player* old = mPlayer;
 changeLocalPlayer(0, true); // remove player from all classes
 delete old;
 old = 0;

 d->mStarting->setLoadingWidget(d->mLoadingWidget);
 d->mStarting->setPlayField(mPlayField);
 d->mStarting->setLocalPlayer(mPlayer);


 // Start loading
 if (!d->mStarting->loadGame(loadingFileName)) {
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
		boError() << k_funcinfo << "Invalid error type or no error (while loading saved game)!!!" << endl;
	}

	// ... and show messagebox
	KMessageBox::sorry(this, text, caption);

	// We also need to re-init the game (player, ...)
	reinitGame();

	// Then return to welcome screen
	showStartupWidget(IdWelcome);
 }

 boDebug() << k_funcinfo << "done" << endl;
}

void TopWidget::slotShowMainMenu()
{
 removeStartupWidget(IdNewGame);
 removeStartupWidget(IdStartEditor);

 // Delete all players to remove added AI players, then re-init local player
 boGame->removeAllPlayers();
 initPlayer();
 showStartupWidget(IdWelcome);
}

void TopWidget::slotShowNetworkOptions()
{
 showStartupWidget(IdNetwork);
}

void TopWidget::slotHideNetworkOptions()
{
 removeStartupWidget(IdNetwork);
 d->mNewGame->slotSetAdmin(boGame->isAdmin());
 showStartupWidget(IdNewGame);
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

void TopWidget::slotStartGame()
{
 boDebug() << k_funcinfo << endl;
 if (boGame->gameStatus() != KGame::Init) {
	boWarning() << k_funcinfo << "not in Init status" << endl;
	return;
 }
// AB: first init map (see slotAssginMap()), THEN init player. we need map for
// player loading (unit positions, ...)

 boDebug() << k_funcinfo << "init player" << endl;
 if (!mPlayer) {
	// the local player is in boGame for loaded games
	mPlayer = boGame->localPlayer();
	if (!mPlayer) {
		boError() << k_funcinfo << "NULL local player" << endl;
		return;
	}
 }
 changeLocalPlayer(mPlayer);
 d->mBosonWidget->initPlayer();
/*
 boDebug() << k_funcinfo << "init map" << endl;
 d->mBosonWidget->initMap();//AB: see slotAssingMap()!! // FIXME REMOVE
 */

 int progress = 0; // FIXME: wrong value!
 showStartupWidget(IdBosonWidget);
 removeStartupWidget(IdNewGame);
 removeStartupWidget(IdStartEditor);
 removeStartupWidget(IdLoading);

 // Init some stuff
 statusBar()->show();
 d->mBosonWidget->initGameMode();
 enableGameActions(true);
 d->mFpstimer.start(1000);
 connect(&d->mFpstimer, SIGNAL(timeout()), this, SLOT(slotUpdateFPS()));
}

void TopWidget::slotStartGameLoadWorkaround() // I know the name sucks - its intended!
{
 // These are from BosonWidgetBase::slotStartScenario() which can't be used when
 //  we're loading saved game
 boGame->startGame();
 boGame->sendMessage(0, BosonMessage::IdGameIsStarted);
 boGame->slotSetGameSpeed(BosonConfig::readGameSpeed());
 boDebug() << "speed: " << boGame->gameSpeed() << endl;
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
 removeStartupWidget(IdBosonWidget); // redundant, btw
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
 connect(d->mStarting, SIGNAL(signalStartGame()), this, SLOT(slotStartGame()));
 connect(d->mStarting, SIGNAL(signalAssignMap()), this, SLOT(slotAssignMap()));
 connect(d->mStarting, SIGNAL(signalStartGameLoadWorkaround()), this, SLOT(slotStartGameLoadWorkaround()));

 initBoson();
 initPlayer();
 initPlayField();

 d->mActionStatusbar->setChecked(false);
 slotToggleStatusbar();
 enableGameActions(false);
 showStartupWidget(IdWelcome);
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
 switch ((StartupWidgetIds)id) {
	case IdWelcome:
	case IdNewGame:
	case IdLoading:
	case IdNetwork:
		boConfig->setShowMenubarOnStartup(d->mActionMenubar->isChecked());
		break;
	case IdBosonWidget:
		boConfig->setShowMenubarInGame(d->mActionMenubar->isChecked());
		break;
	case IdStartEditor:
		// editor without menubar is pretty useless, i guess
		break;
	default:
		boDebug() << k_funcinfo << "unknown id " << id << endl;
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
 QWidget* w = mWs->visibleWidget();
 if (!w) {
	return true;
 }
 switch ((StartupWidgetIds)mWs->id(w)) {
	case IdBosonWidget:
		d->mBosonWidget->saveConfig();
		d->mBosonWidget->quitGame();
		saveGameDockConfig();
		return true;
	case IdNewGame:
	case IdLoading:
	case IdNetwork:
	case IdWelcome:
		saveInitialDockConfig();
		return true;
	default:
		return true;
 }
 return true;
}

void TopWidget::raiseWidget(StartupWidgetIds id)
{
 switch (id) {
	case IdWelcome:
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
	case IdNewGame:
	case IdLoading:
	case IdNetwork:
		setMinimumSize(BOSON_MINIMUM_WIDTH, BOSON_MINIMUM_HEIGHT);
		setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
		if (boConfig->showMenubarOnStartup()) {
			showMenubar();
		} else {
			hideMenubar();
		}
		break;
	case IdBosonWidget:
		if (boConfig->showMenubarInGame()) {
			showMenubar();
		} else {
			hideMenubar();
		}
		setMinimumSize(BOSON_MINIMUM_WIDTH, BOSON_MINIMUM_HEIGHT);
		setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
		break;
	case IdStartEditor:
		showMenubar();
		setMinimumSize(BOSON_MINIMUM_WIDTH, BOSON_MINIMUM_HEIGHT);
		setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
		break;
	default:
		boDebug() << k_funcinfo << "unknown id " << id << endl;
		break;
 }
 if (!mWs->widget((int)id)) {
	kdWarning() << k_funcinfo << "NULL widget " << id << endl;
	return;
 }
 mWs->raiseWidget((int)id);
 mWs->widget((int)id)->show();
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
 switch ((StartupWidgetIds)id) {
	case IdWelcome:
	case IdNewGame:
	case IdLoading:
	case IdNetwork:
		if (boConfig->showMenubarOnStartup()) {
			showMenubar();
		} else {
			hideMenubar();
		}
		break;
	case IdBosonWidget:
		if (boConfig->showMenubarInGame()) {
			showMenubar();
		} else {
			hideMenubar();
		}
		break;
	case IdStartEditor:
		// editor without menubar is pretty useless, i guess
		showMenubar();
		break;
	default:
		boDebug() << k_funcinfo << "unknown id " << id << endl;
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

