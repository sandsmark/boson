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
#include "bodisplaymanager.h"
#include "bosonstarting.h"
#include "rtti.h"
#include "bodebug.h"
#include "bodebugdcopiface.h"
#include "startupwidgets/bosonstartupwidget.h"
#include "sound/bosonaudiointerface.h"
#include "bosondata.h"
#include "bosongroundtheme.h"
#include "bosonlocalplayerinput.h"

#include <kgameio.h>

#include <klocale.h>
#include <kstatusbar.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kglobal.h>

#include <qtimer.h>
#include <qhbox.h>
#include <qvbox.h>

#include <stdlib.h>
#include <unistd.h>
#include <config.h>


class TopWidget::TopWidgetPrivate
{
public:
	TopWidgetPrivate()
	{
		mStartup = 0;

		mDisplayManager = 0;
		mBosonWidget = 0;

		mStarting = 0;

		mIface = 0;
	}

	BosonStartupWidget* mStartup;

	BoDisplayManager* mDisplayManager;
	BosonWidgetBase* mBosonWidget;

	QTimer mStatusBarTimer;

	BosonStarting* mStarting;

	BoDebugDCOPIface* mIface;
};

TopWidget::TopWidget() : KDockMainWindow(0, "topwindow")
{
 d = new TopWidgetPrivate;

 mMainDock = createDockWidget("mainDock", 0, this, i18n("Map"));
 mMainDock->setDockSite(KDockWidget::DockCorner);
 mMainDock->setEnableDocking(KDockWidget::DockNone);

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

 initStatusBar();

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

QString TopWidget::checkInstallation()
{
 QPixmap p1(locate("data", "boson/pics/boson-startup-bg.png"));
 QPixmap p2(locate("data", "boson/pics/boson-startup-logo.png"));
  if (p1.isNull() || p2.isNull()) {
	return i18n("You seem not to have Boson data files installed!\n Please install data package of Boson and restart Boson.");
}
 if (!BosonGroundTheme::createGroundThemeList()) {
	return i18n("Unable to load groundThemes. Check your installation!");
 }
 if (!BosonPlayField::preLoadAllPlayFields()) {
	return i18n("Unable to preload playFields. Check your installation!");
 }
 return QString::null;
}

void TopWidget::initDisplayManager()
{
 d->mDisplayManager = new BoDisplayManager(0);
 connect(d->mDisplayManager, SIGNAL(signalToggleStatusbar(bool)),
		this, SLOT(slotToggleStatusbar(bool)));

 // add an initial display. this should happen asap, as we need the OpenGL
 // context for every texture that is to be loaded.
 // the display will not be deleted before the program is quit, so that we don't
 // have to load the textures several times.
 d->mDisplayManager->addInitialDisplay();

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
 (void)new QLabel(i18n("Effects: "), debug);
 QLabel* effectsLabel = new QLabel(QString::number(0), debug);
 connect(this, SIGNAL(signalEffectsCountUpdated(int)), effectsLabel, SLOT(setNum(int)));
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
 BO_CHECK_NULL_RET(boGame);
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

 connect(d->mBosonWidget, SIGNAL(signalChangeLocalPlayer(Player*)),
		this, SLOT(slotChangeLocalPlayer(Player*)));
 connect(d->mBosonWidget, SIGNAL(signalEndGame()),
		this, SLOT(slotEndGame()));
 connect(d->mBosonWidget, SIGNAL(signalQuit()),
		this, SLOT(close()));
 connect(d->mBosonWidget, SIGNAL(signalMobilesCount(int)),
		this, SIGNAL(signalSetMobilesCount(int)));
 connect(d->mBosonWidget, SIGNAL(signalFacilitiesCount(int)),
		this, SIGNAL(signalSetFacilitiesCount(int)));
 connect(d->mBosonWidget, SIGNAL(signalOilUpdated(int)),
		this, SIGNAL(signalOilUpdated(int)));
 connect(d->mBosonWidget, SIGNAL(signalMineralsUpdated(int)),
		this, SIGNAL(signalMineralsUpdated(int)));

 d->mBosonWidget->setDisplayManager(d->mDisplayManager);

 d->mBosonWidget->init(0); // this depends on several virtual methods and therefore can't be called in the c'tor

// factory()->addClient(d->mBosonWidget); // XMLClient-stuff. needs to be called *after* creation of KAction objects, so outside BosonWidget might be a good idea :-)
// createGUI("bosonui.rc", false);

}

void TopWidget::slotPlayFieldChanged(const QString& id)
{
 if (!d->mStarting) {
	boError() << k_funcinfo << "NULL starting object!!" << endl;
	return;
 }
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
 boDebug(270) << k_funcinfo << endl;

 // AB: we need to have boGame->localPlayer() be valid and non-NULL for the
 // starup widgets (atm).
 // but for the actual starting process it is much cleaner, if the local player
 // is NULL and gets applied at the end only.
 slotChangeLocalPlayer(0);

 d->mStartup->showLoadingWidget();

 initBosonWidget();

 if (d->mBosonWidget->localPlayer()) {
	boWarning(270) << k_funcinfo << "localPlayer should be NULL until game is started!" << endl;
 }

 // this will take care of all data loading, like models, textures and so. this
 // also initializes the map and will send IdStartScenario - in short this will
 // start the game. Once it's done it'll send IdGameIsStarted (see
 // Boson::signalGameStarted())
 //
 // Note that this return immediately - the loading is started using a
 // QTimer::singleShot()
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
 QByteArray data = d->mStarting->loadGame(fileName);
 if (data.size() == 0) {
	boError() << k_funcinfo << "failed loading from file" << endl;
	return;
 }

 QByteArray buffer;
 QDataStream stream(buffer, IO_WriteOnly);
 stream << (Q_INT8)1; // game mode
 stream << data;

 // actually start the game
 boGame->sendMessage(buffer, BosonMessage::IdNewGame);
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
	d->mStatusBarTimer.stop();
 }
 delete d->mBosonWidget;
 d->mBosonWidget = 0;
 Boson::deleteBoson();  // Easiest way to reset game info
 delete d->mStarting;
 d->mStarting = 0;
 boDebug() << k_funcinfo << "done" << endl;
}

void TopWidget::reinitGame()
{
 endGame();

 delete d->mStarting;
 d->mStarting = new BosonStarting(this); // manages startup of games
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

 enableGameActions(false);
}

void TopWidget::slotGameOver()
{
 endGame();

 // this also resets the game!
 // if you replace this by something else you must call slotResetGame()
 // manually!
 d->mStartup->slotShowWelcomeWidget();
 mMainDock->setWidget(d->mStartup);
}

void TopWidget::slotToggleStatusbar(bool show)
{
 if (show) {
	statusBar()->show();
 } else {
	statusBar()->hide();
 }
}

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
	return true;
 }
 if (mMainDock->getWidget() != d->mStartup) {
	boWarning() << k_funcinfo << "oops! - game in init mode but no startup widget on top!" << endl;
	boDebug() << k_funcinfo << "maindock's widget: " << mMainDock->getWidget() << endl;
	boDebug() << k_funcinfo << "startup widget: " << d->mStartup << endl;
	return true;
 }

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
 emit signalEffectsCountUpdated(stat->effectsCount());
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


void TopWidget::changeLocalPlayer(Player* p)
{
 boDebug() << k_funcinfo << p << endl;

 // AB: note that both, p == 0 AND p == currentplayer are valid and must be executed!
 if (!boGame) {
	boError() << k_funcinfo << "NULL game object" << endl;
	return;
 }
 if (d->mBosonWidget) {
	d->mBosonWidget->setLocalPlayer(p);
 }

 // AB: note: the startup widgets don't need to know the new local player
 // -> they need the unique local player only (new game widget), which is set
 // right after construction of the player.
}

void TopWidget::slotEndGame()
{
 BO_CHECK_NULL_RET(boGame);
 int answer;
 if (boGame->gameMode()) {
	answer = KMessageBox::warningYesNo(this, i18n("Are you sure you want to end this game?"),
		i18n("Are you sure?"), KStdGuiItem::yes(), KStdGuiItem::no(), "ConfirmEndGame");
 } else {
	answer = KMessageBox::warningYesNo(this, i18n("Are you sure you want to end this editor session?"),
		i18n("Are you sure?"), KStdGuiItem::yes(), KStdGuiItem::no(), "ConfirmEndGame");
 }
 if (answer != KMessageBox::Yes) {
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
 if (boGame->playerCount() != 0) {
	boError() << k_funcinfo << "there are already players in the game!" << endl;
	return;
 }
 boDebug() << k_funcinfo << endl;
 Player* p = new Player;
 p->addGameIO(new BosonLocalPlayerInput()); // we can use the RTTI of this to identify the local player
 boGame->bosonAddPlayer(p);
 if (d->mStartup) {
	// this must be done _now_, we cannot delay it!
	// -> the newgame widget must know about the local player
	d->mStartup->setLocalPlayer(p);
 }
}

// TODO: when this fails we should go back to the welcome widget!
void TopWidget::slotGameStarted()
{
 boDebug(270) << k_funcinfo << endl;
 BO_CHECK_NULL_RET(boGame);
 if (boGame->gameStatus() != KGame::Run) {
	boWarning(270) << k_funcinfo << "not in Run status" << endl;
	return;
 }

 boDebug(270) << k_funcinfo << "init player" << endl;
 Player* localPlayer = 0;
 for (unsigned int i = 0; i < boGame->playerCount(); i++) {
	Player* p = (Player*)boGame->playerList()->at(i);
	if (!p->isVirtual()) {
		// a non-virtual player is a player that is running on this host
		// (either the local player or a computer player - never a
		// network player)
		if (p->hasRtti(BosonLocalPlayerInput::LocalPlayerInputRTTI)) {
			if (p->hasRtti(KGameIO::ComputerIO)) {
				boError(270) << k_funcinfo << "a player must not be both - local player and computer player!" << endl;
				return;
			}
			if (localPlayer) {
				boError(270) << k_funcinfo << "two local players found?!" << endl;
				return;
			}
			localPlayer = p;

			// when starting the game, a player must not have a
			// BosonLocalPlayerInput!
			KGameIO* io = p->findRttiIO(BosonLocalPlayerInput::LocalPlayerInputRTTI);
			p->removeGameIO(io);
		}
	}
 }

 if (!localPlayer) {
	// this can happen in two cases
	// 1. editor mode
	// 2. loading a game
	
	if (!boGame->gameMode()) {
		// pick one player for editor mode
		// (it doesnt matter which)
		boDebug() << k_funcinfo << "picking a local player for editor mode" << endl;
		localPlayer = (Player*)boGame->playerList()->at(0);
	} else {
		// we are loading a game
		// we are chosing the first player here - this is
		// valid as long as we don't support loading network games.
		// later we may allow selecting a player in the startup widgets
		//
		// if we ever do that, we should remove these line here!
		boDebug() << k_funcinfo << "picking a local player for loading games" << endl;
		localPlayer = (Player*)boGame->playerList()->at(0);
	}
 }
 if (!localPlayer) {
	boError(270) << k_funcinfo << "NULL local player" << endl;
	return;
 }
 if (d->mBosonWidget->localPlayer()) {
	boWarning(270) << k_funcinfo << "localPlayer should ne NULL here!" << endl;
 }
 slotChangeLocalPlayer(localPlayer);
 d->mBosonWidget->initPlayer();

 if (d->mBosonWidget->canvas()) {
	boWarning(270) << k_funcinfo << "BosonWidget::canvas() is non-NULL ("
			<< d->mBosonWidget->canvas()
			<< ")! This is very unexpected!" << endl;
	boWarning(270) << k_funcinfo << "setting it to " << d->mBosonWidget->canvas() << endl;

 }
 d->mBosonWidget->setCanvas(boGame->canvasNonConst());
 d->mBosonWidget->initMap();

 d->mDisplayManager->setCanvas(boGame->canvasNonConst());

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
 enableGameActions(true);
 d->mStatusBarTimer.start(1000);
 d->mBosonWidget->initGameMode();
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

void TopWidget::slotLoadFromLog(const QString& logFile)
{
 if (!d->mStartup) {
	boError() << k_funcinfo << "NULL startup widget" << endl;
	return;
 }
 BO_CHECK_NULL_RET(boGame);
 boDebug() << k_funcinfo << "trying to load from log file " << logFile << endl;
 d->mStartup->slotNewGame(0);
 BO_CHECK_NULL_RET(d->mStarting);
 d->mStarting->setLoadFromLogFile(logFile);
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

void TopWidget::slotChangeLocalPlayer(Player* p)
{
 changeLocalPlayer(p);
}


