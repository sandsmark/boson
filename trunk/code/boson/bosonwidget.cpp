/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2002 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bosonwidget.h"

#include "bosonbigdisplay.h"
#include "bosonminimap.h"
#include "bosoncanvas.h"
#include "boson.h"
#include "player.h"
#include "unitproperties.h"
#include "unit.h"
#include "speciestheme.h"
#include "kspritetooltip.h"
#include "bosoncommandframe.h"
#include "bosonmessage.h"
#include "bosonmap.h"
#include "bosonplayfield.h"
#include "bosonscenario.h"
#include "bosonconfig.h"
#include "optionsdialog.h"
#include "newgamedialog.h"
#include "kgamedialogbosonconfig.h"
#include "kgamedialogcomputerconfig.h"
#include "kgameunitdebug.h"
#include "bosonmusic.h"
#include "bosoncursor.h"
#include "editorinput.h"
#include "commandinput.h"
#include "bodisplaymanager.h"
#include "gameoverdialog.h"
#include "global.h"

#include "defines.h"

#include <klocale.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <kgame/kgameio.h>
#include <kgame/kgamedebugdialog.h>
#include <kgame/kgamepropertyhandler.h>
#include <kgame/kgamechat.h>

#include <qlayout.h>
#include <qvbox.h>
#include <qptrlist.h>

#include "bosonwidget.moc"

class BosonWidget::BosonWidgetPrivate
{
public:
	BosonWidgetPrivate()
	{
		mMiniMap = 0;
		mCommandFrame = 0;
		mDisplayManager = 0;

		mCanvas = 0;

		mBoson = 0;
		mLocalPlayer = 0;
		mPlayField = 0;

		mUnitTips = 0;

		mTopLayout = 0;
		mViewLayout = 0;

		mChat = 0;

		mGameOverDialog = 0;

		mCursor = 0;
	}
	
	BosonMiniMap* mMiniMap;
	BosonCommandFrame* mCommandFrame;
	BoDisplayManager* mDisplayManager;

	BosonCanvas* mCanvas;
	
	Boson* mBoson;
	Player* mLocalPlayer;
	BosonPlayField* mPlayField;

	KSpriteToolTip* mUnitTips;

	QHBoxLayout* mTopLayout;
	QVBoxLayout* mViewLayout; // chat and bigdisplay
	CommandFramePosition mCommandPos;
	ChatFramePosition mChatPos;

	// performance variables:
	int mMobilesCount;
	int mFacilitiesCount;
	
	KGameChat* mChat;

	BosonCursor* mCursor;
	QString mCursorTheme; // path to cursor pixmaps

	QPtrDict<KGameMouseIO> mIOList;

	GameOverDialog* mGameOverDialog;

	bool mEditorMode;
};

BosonWidget::BosonWidget(QWidget* parent)
    : QWidget( parent, "BosonWidget" )
{
 init();

 boMusic->setSound(boConfig->sound());
 boMusic->setMusic(boConfig->music());
}

BosonWidget::~BosonWidget()
{
 kdDebug() << k_funcinfo << endl;
 if (d->mCursor->isA("BosonSpriteCursor")) {
	boConfig->saveCursorMode(CursorSprite);
 } else if (d->mCursor->isA("BosonExperimentalCursor")) {
	boConfig->saveCursorMode(CursorExperimental);
 } else if (d->mCursor->isA("BosonKDECursor")) {
	boConfig->saveCursorMode(CursorKDE);
 } else {
	boConfig->saveCursorMode(CursorNormal);
 }
 boConfig->saveCursorDir(d->mCursorTheme);
 d->mIOList.clear();
 delete d->mUnitTips;
 delete d->mCursor;
 delete d->mDisplayManager;

// delete the destroyed units first
 d->mCanvas->deleteDestroyed();
// now delte all KGame stuff. Also removed player and therefore the rest of the
// units.  Otherwise this is deleted later, when all units are already cleared by
// QCanvas (->crash)
 delete d->mBoson;
 delete d->mCanvas;
 delete d->mPlayField;
 
 delete d;
 kdDebug() << k_funcinfo << "done" << endl;
}

void BosonWidget::init()
{
 d = new BosonWidgetPrivate;
 d->mMobilesCount = 0;
 d->mFacilitiesCount = 0;
 d->mIOList.setAutoDelete(true);
 d->mEditorMode = false; // in startGameMode() / startEditor()

 BosonMusic::initBosonMusic(); 

 d->mPlayField = new BosonPlayField(this);
 connect(d->mPlayField, SIGNAL(signalNewMap(BosonMap*)),
		this, SLOT(slotNewMap(BosonMap*)));

 d->mCanvas = new BosonCanvas(this);
 connect(d->mCanvas, SIGNAL(signalUnitDestroyed(Unit*)), 
		this, SLOT(slotRemoveUnit(Unit*)));
 connect(d->mCanvas, SIGNAL(signalOutOfGame(Player*)),
		this, SLOT(slotOutOfGame(Player*)));

 d->mDisplayManager = new BoDisplayManager(d->mCanvas, this);
 connect(this, SIGNAL(signalMineralsUpdated(int)),
		d->mDisplayManager, SLOT(slotUpdateMinerals(int)));
 connect(this, SIGNAL(signalOilUpdated(int)),
		d->mDisplayManager, SLOT(slotUpdateOil(int)));

 d->mBoson = new Boson(this);
 d->mBoson->setCanvas(d->mCanvas); // should not be stored here - but seems to be necessary :-(
 connect(d->mBoson, SIGNAL(signalAdvance(unsigned int)),
		d->mCanvas, SLOT(slotAdvance(unsigned int)));
 connect(d->mBoson, SIGNAL(signalAddUnit(Unit*, int, int)),
		d->mCanvas, SLOT(slotAddUnit(Unit*, int, int))); // needs a QCanvas - we need to call Boson::setCanvas for this
 connect(d->mBoson, SIGNAL(signalAddUnit(Unit*, int, int)),
		this, SLOT(slotAddUnit(Unit*, int, int)));
 connect(d->mBoson, SIGNAL(signalPlayerJoinedGame(KPlayer*)),
		this, SIGNAL(signalPlayerJoinedGame(KPlayer*)));
 connect(d->mBoson, SIGNAL(signalPlayerJoinedGame(KPlayer*)),
		this, SLOT(slotPlayerJoinedGame(KPlayer*)));
 connect(d->mBoson, SIGNAL(signalPlayerLeftGame(KPlayer*)),
		this, SIGNAL(signalPlayerLeftGame(KPlayer*)));
 connect(d->mBoson, SIGNAL(signalInitMap(const QByteArray&)),
		this, SLOT(slotReceiveMap(const QByteArray&)));
 connect(d->mBoson, SIGNAL(signalInitFogOfWar()),
		this, SLOT(slotInitFogOfWar()));
 connect(d->mBoson, SIGNAL(signalStartScenario()),
		this, SLOT(slotStartScenario()));
 connect(d->mBoson, SIGNAL(signalPlayFieldChanged(const QString&)),
		this, SLOT(slotLoadPlayField(const QString&)));
 connect(d->mBoson, SIGNAL(signalGameStarted()),
		this, SIGNAL(signalGameStarted()));
 connect(d->mBoson, SIGNAL(signalNotEnoughMinerals(Player*)),
		this, SLOT(slotNotEnoughMinerals(Player*)));
 connect(d->mBoson, SIGNAL(signalNotEnoughOil(Player*)),
		this, SLOT(slotNotEnoughOil(Player*)));
 connect(d->mBoson, SIGNAL(signalNewGroup(Unit*, QPtrList<Unit>)),
		d->mCanvas, SLOT(slotNewGroup(Unit*, QPtrList<Unit>)));

 slotChangeCursor(boConfig->readCursorMode(), boConfig->readCursorDir());
 slotChangeGroupMove(boConfig->readGroupMoveMode());

 initChat();

// 640*480 is probably not enough (KDE needs at least 800*600) but as a minimum
// should be ok.
 setMinimumWidth(640);
 setMinimumHeight(480);

 setFocusPolicy(StrongFocus); // accept key event
 setFocus();
 d->mBoson->slotSetGameSpeed(0); // pause

 d->mCommandPos = CmdFrameLeft;
 d->mChatPos = ChatFrameBottom;
 addMiniMap();
}

void BosonWidget::addMiniMap()
{
 d->mMiniMap = new BosonMiniMap(0);
 d->mMiniMap->hide();
 d->mMiniMap->setCanvas(d->mCanvas);
 connect(d->mBoson, SIGNAL(signalAddUnit(Unit*, int, int)),
		d->mMiniMap, SLOT(slotAddUnit(Unit*, int, int)));
 connect(d->mCanvas, SIGNAL(signalUnitMoved(Unit*, double, double)),
		d->mMiniMap, SLOT(slotMoveUnit(Unit*, double, double)));
 connect(d->mCanvas, SIGNAL(signalUnitDestroyed(Unit*)), 
		d->mMiniMap, SLOT(slotUnitDestroyed(Unit*)));

}

void BosonWidget::reparentMiniMap(QWidget* parent)
{
 d->mMiniMap->reparent(parent, QPoint(0, 0));
 d->mMiniMap->hide();
}

void BosonWidget::initChat()
{
 d->mChat = new KGameChat(d->mBoson, BosonMessage::IdChat, this);
 d->mChat->hide();
}

void BosonWidget::addLocalPlayer()
{
 d->mIOList.clear();
 delete d->mLocalPlayer;
 Player* p = new Player;
 p->setName(BosonConfig::readLocalPlayerName());
 connect(p, SIGNAL(signalUnfog(int, int)),
		this, SLOT(slotUnfog(int, int)));
 connect(p, SIGNAL(signalFog(int, int)),
		this, SLOT(slotFog(int, int)));
 connect(p, SIGNAL(signalShowMiniMap(bool)),
		d->mMiniMap, SLOT(slotShowMap(bool)));
 d->mBoson->addPlayer(p);

 CommandInput* cmdInput = new CommandInput;
 cmdInput->setCommandFrame(d->mCommandFrame);
 p->addGameIO(cmdInput);

 changeLocalPlayer(p);
 QPtrList<BosonBigDisplay> list = d->mDisplayManager->displays();
 QPtrListIterator<BosonBigDisplay> it(list);
 while (it.current()) {
	addMouseIO(it.current());
	++it;
 }
}

void BosonWidget::addDummyComputerPlayer(const QString& name)
{
 Player* p = new Player;
 p->setName(name);
 d->mBoson->addPlayer(p);
 p->loadTheme(SpeciesTheme::speciesDirectory(SpeciesTheme::defaultSpecies()), SpeciesTheme::defaultColor());// FIXME - should be selectable in new game dialog
}

void BosonWidget::slotPlayerJoinedGame(KPlayer* player)
{
 if (!player) {
	kdError() << k_funcinfo << "NULL player" << endl;
	return;
 }
 Player* p = (Player*)player;
 connect(p, SIGNAL(signalPropertyChanged(KGamePropertyBase*, KPlayer*)),
		this, SLOT(slotPlayerPropertyChanged(KGamePropertyBase*, KPlayer*)));
 if (d->mPlayField->map()) {
	p->initMap(d->mPlayField->map());
 }
 if (player == d->mLocalPlayer) {
	changeLocalPlayer(d->mLocalPlayer); // now with a KGame object
 }

 if (!d->mBoson->isAdmin()) {
	return;
 }
 if (!p->speciesTheme()) {
	return;
 }
 // check if the color of the new player is already taken
 QPtrListIterator<KPlayer> it(*d->mBoson->playerList());
 QColor playerColor = p->speciesTheme()->teamColor();
 while (it.current()) {
	if (it.current() != player) {
		Player* p2 = (Player*)it.current();
		if (!p2->speciesTheme()) {
			kdWarning() << k_lineinfo << "NULL speciesTheme for " << p2->id() << endl;
			++it;
			continue;
		}
		if (playerColor == p2->speciesTheme()->teamColor()) {
			sendChangeTeamColor(p, d->mBoson->availableTeamColors().first());
		}
	}
	++it;
 }
}

void BosonWidget::keyReleaseEvent(QKeyEvent* e)
{
 BosonBigDisplay* active = d->mDisplayManager->activeDisplay();
 if (!active) {
	return;
 }
 switch (e->key()) {
	case Key_Left:
		active->scrollBy(-boConfig->arrowKeyStep(), 0);
		break;
	case Key_Right:
		active->scrollBy(boConfig->arrowKeyStep(), 0);
		break;
	case Key_Up:
		active->scrollBy(0, -boConfig->arrowKeyStep());
		break;
	case Key_Down:
		active->scrollBy(0, boConfig->arrowKeyStep());
		break;
	default:
		break;
 }
}

void BosonWidget::slotDebug()
{
 KGameDebugDialog* dlg = new KGameDebugDialog(d->mBoson, this);
 QVBox* b = dlg->addVBoxPage(i18n("Debug &Units"));
 KGameUnitDebug* units = new KGameUnitDebug(b);
 units->setBoson(d->mBoson);
 connect(dlg, SIGNAL(finished()), dlg, SLOT(slotDelayedDestruct()));
 dlg->show();
}

void BosonWidget::slotArrowScrollChanged(int speed)
{
 boConfig->setArrowKeyStep(speed);
}

void BosonWidget::slotNewGame()
{
// kdDebug() << k_funcinfo << endl;
 if (d->mBoson->isRunning()) {
	if (KMessageBox::questionYesNo(this, i18n("Quit the running game?"))
			!= KMessageBox::Yes) {
		return;
	}
 }
 quitGame();

 if (BosonPlayField::availablePlayFields().count() == 0) {
	KMessageBox::sorry(this, i18n("Unable to find any playfield (*.bpf) files. Did you install data??"));
	return;
 }
 NewGameDialog* dialog = new NewGameDialog(d->mBoson, d->mLocalPlayer, this);
 connect(dialog, SIGNAL(finished()), dialog, SLOT(slotDelayedDestruct())); // is this called when "cancel"ed?

 // add our costum game config widget
 KGameDialogBosonConfig* bosonConfig = new KGameDialogBosonConfig(0);
 connect(bosonConfig, SIGNAL(signalStartGame()), this, SLOT(slotStartGame()));
 connect(d->mBoson, SIGNAL(signalPlayFieldChanged(const QString&)),
		bosonConfig, SLOT(slotPlayFieldChanged(const QString&)));
 connect(d->mBoson, SIGNAL(signalSpeciesChanged(Player*)),
		bosonConfig, SLOT(slotSpeciesChanged(Player*)));
 connect(d->mBoson, SIGNAL(signalTeamColorChanged(Player*)),
		bosonConfig, SLOT(slotTeamColorChanged(Player*)));
 connect(bosonConfig, SIGNAL(signalSpeciesChanged(const QString&)),
		this, SLOT(slotSendChangeSpecies(const QString&)));
 connect(bosonConfig, SIGNAL(signalTeamColorChanged(const QColor&)),
		this, SLOT(slotSendChangeTeamColor(const QColor&)));
 connect(dialog, SIGNAL(signalStartGame()), 
		bosonConfig, SIGNAL(signalStartGame()));
 // Note: KGameDialogBosonConfig does not emit signals for the important things,
 // but rather sends a message over the network (see BosonMessage). 
 // This is done to provide network transparency in the dialog - when the admin
 // changes the map then on all clients the map must change.
 // You can find the reactions to these messages in Boson::slotNetworkData().


 // add a connection and a chat widget
 dialog->addGameConfig(bosonConfig);
 QVBox* page = dialog->configPage(KGameDialog::GameConfig);
 dialog->addConfigWidget(new KGameDialogConnectionConfig(), page);
 dialog->addConfigWidget(new KGameDialogChatConfig(BosonMessage::IdChat), page);

 KGameDialogComputerConfig* computerConfig = new KGameDialogComputerConfig(0);
 connect(computerConfig, SIGNAL(signalAddComputerPlayer(Player*)), 
		this, SLOT(slotAddComputerPlayer(Player*)));
 QVBox* computerPage = dialog->addConfigPage(computerConfig, i18n("&Computer Player"));
 dialog->addConnectionList(new KGameDialogConnectionConfig(0), computerPage);

 // add a network config
 dialog->addNetworkConfig(new KGameDialogNetworkConfig(0));

 // a connection list - aka "ban this player" - also in game page (to see
 // the number of the players when selecting a map)
 QVBox* networkPage = dialog->configPage(KGameDialog::NetworkConfig);
 dialog->addConnectionList(new KGameDialogConnectionConfig(0), networkPage);

 // add a msgserver config
// dialog->addMsgServerConfig(new KGameDialogMsgServerConfig(0)); // FIXME: do
// we use this?

 // show the dialog
 dialog->show();

 bosonConfig->slotPlayFieldChanged(0);
}

void BosonWidget::slotStartGame()
{
 if (!d->mBoson->isAdmin()) {
	KMessageBox::sorry(this, i18n("Only ADMIN can start the game"));
	kdWarning() << "not admin" << endl;
	return;
 }
 if (d->mBoson->isRunning()) {
	KMessageBox::sorry(this, i18n("The game is already running"));
	kdWarning() << "game already running" << endl;
	return;
 }
 if (d->mBoson->playerCount() < d->mBoson->minPlayers()) {
	KMessageBox::sorry(this, i18n("Need at least %1 players").arg(d->mBoson->minPlayers()));
	kdError() << "not enough players" << endl;
	return;
 }
 if (d->mBoson->maxPlayers() > 0 && (int)d->mBoson->playerCount() > d->mBoson->maxPlayers()) {
	KMessageBox::sorry(this, i18n("Maximal %1 players").arg(d->mBoson->maxPlayers()));
	kdError() << "too many players" << endl;
	return;
 }

// put fog of war on the map. We don't do this before so that the player can
// look at the map and choose whether they want to playe there.
 d->mBoson->sendMessage(0, BosonMessage::IdInitFogOfWar);

// start the chosen scenario.
 d->mBoson->sendMessage(0, BosonMessage::IdStartScenario);
}

void BosonWidget::slotStartScenario()
{
// kdDebug() << k_funcinfo << endl;
 if (!d->mPlayField->scenario()) {
	kdError() << k_funcinfo << ": NULL scenario" << endl;
	return;
 }
 d->mPlayField->scenario()->startScenario(d->mBoson);
 boMusic->startLoop();

 // TODO as soon as it is implemented the map file should also contain the
 // species of the player. The NewGameDialog should enable the player to choose
 // the desired species and especially the color. This selection should be
 // stored as a KGameProperty. When this (slotReceiveMap()) is called we can
 // read (1) the map-specie and (2) the player's selection. And according to
 // these settings the correct theme should be loaded. 
 // We have to ensure that the files are actually available *before* the game 
 // is started!!
 // UPDATE (01/11/19): should be in scenario file!
 for (unsigned int i = 0; i < d->mBoson->playerCount(); i++) {
//	Color color = SpeciesTheme::defaultColor();
//	QString species = "human";
//	((Player*)d->mBoson->playerList()->at(i))->loadTheme(species, color);
 }
 d->mBoson->startGame(); // correct here? should be so.

 // as soon as this message is received the game is actually started
 d->mBoson->sendMessage(0, BosonMessage::IdGameIsStarted);
}

void BosonWidget::slotGamePreferences()
{

 OptionsDialog* dlg = new OptionsDialog(this);
 connect(dlg, SIGNAL(finished()), dlg, SLOT(slotDelayedDestruct())); // seems not to be called if you quit with "cancel"!
 dlg->setGameSpeed(d->mBoson->gameSpeed());
 dlg->setArrowScrollSpeed(boConfig->arrowKeyStep());
 dlg->setCommandFramePosition(d->mCommandPos);
 dlg->setChatFramePosition(d->mChatPos);

 connect(dlg, SIGNAL(signalArrowScrollChanged(int)),
		this, SLOT(slotArrowScrollChanged(int)));

// TODO: not in editor mode!! in editor the game is always paused!
 connect(dlg, SIGNAL(signalSpeedChanged(int)),
		d->mBoson, SLOT(slotSetGameSpeed(int)));

 connect(dlg, SIGNAL(signalCommandFramePositionChanged(int)),
		this, SLOT(slotCommandFramePosition(int)));
 connect(dlg, SIGNAL(signalChatFramePositionChanged(int)),
		this, SLOT(slotChatFramePosition(int)));

 connect(dlg, SIGNAL(signalCursorChanged(int, const QString&)),
		this, SLOT(slotChangeCursor(int, const QString&)));

 connect(dlg, SIGNAL(signalGroupMoveChanged(int)),
		this, SLOT(slotChangeGroupMove(int)));

 dlg->show();
}

void BosonWidget::slotAddUnit(Unit* unit, int, int)
{
 if (!unit) {
	kdError() << k_funcinfo << ": NULL unit" << endl;
	return;
 }
 Player* player = unit->owner();
 if (!player) {
	kdError() << k_funcinfo << ": NULL owner" << endl;
	return;
 }
 d->mUnitTips->add(unit->rtti(), unit->unitProperties()->name());
 if (unit->owner() != d->mLocalPlayer) {
	return;
 }
 if (unit->unitProperties()->isMobile()) {
	d->mMobilesCount++;
 } else {
	d->mFacilitiesCount++;
 }
 emit signalMobilesCount(d->mMobilesCount);
 emit signalFacilitiesCount(d->mFacilitiesCount);
}

void BosonWidget::slotRemoveUnit(Unit* unit)
{
 if (unit->owner() != d->mLocalPlayer) {
	return;
 }
 if (unit->unitProperties()->isMobile()) {
	d->mMobilesCount--;
 } else {
	d->mFacilitiesCount--;
 }
 emit signalMobilesCount(d->mMobilesCount);
 emit signalFacilitiesCount(d->mFacilitiesCount);
}

void BosonWidget::quitGame()
{
// TODO: set SpeciesTheme::defaultColor() back!! is a static variable!!
 d->mIOList.clear();
 d->mCanvas->quitGame();
 d->mBoson->quitGame();
 
 
 d->mLocalPlayer = 0;

 d->mPlayField->quit();

 // now re-add the local player
 addLocalPlayer();
}


void BosonWidget::slotEndGame()
{
 // the game has ended but don't quit boson
 quitGame();
}

void BosonWidget::slotEditorConstructionChanged(int index)
{ // called by the map editor only
// FIXME
// kdDebug() << k_funcinfo << endl;
 if (d->mLocalPlayer) {
	kdDebug() << "local player: " << d->mLocalPlayer->id() << endl;
 }
 d->mCommandFrame->slotEditorProduction(index, d->mLocalPlayer);
}

void BosonWidget::slotNewMap(BosonMap* map)
{
 kdDebug() << k_funcinfo << endl;
 if (!map) {
	//TODO: disconnect all receivers!
	return;
 }
 d->mCanvas->setMap(map);
 d->mMiniMap->setMap(map);
 
 QPtrList<BosonBigDisplay> list = d->mDisplayManager->displays();
 QPtrListIterator<BosonBigDisplay> it(list);
 while (it.current()) {
	// no need to disconnect (I hope) - the old map should already be
	// deleted
	connect(it.current(), SIGNAL(signalAddCell(int,int, int, unsigned char)),
			map, SLOT(changeCell(int, int, int, unsigned char)));
	++it;
 }
}

void BosonWidget::addEditorCommandFrame(QWidget* parent)
{ // remember to call this *after* init() - otherwise connect()s won't work
 d->mCommandFrame = new BosonCommandFrame(parent, true);

 connect(d->mCommandFrame, SIGNAL(signalCellSelected(int)), 
		d->mDisplayManager, SLOT(slotEditorWillPlaceCell(int)));
 connect(this, SIGNAL(signalEditorLoadTiles(const QString&)), 
		d->mCommandFrame, SLOT(slotEditorLoadTiles(const QString&)));

// note: this signal should be connected to something like a CommandInput. But
// we need the values in BosonBigDisplay. So as a temporary solution we have
// this connect here.
 connect(d->mCommandFrame, SIGNAL(signalProduceUnit(int, UnitBase*, KPlayer*)), 
		d->mDisplayManager, SLOT(slotEditorWillPlaceUnit(int, UnitBase*, KPlayer*)));

 // AB???

 slotChatFramePosition(BosonConfig::readChatFramePosition());
}

void BosonWidget::addGameCommandFrame(QWidget* parent)
{ // remember to call this *after* init() - otherwise connect()s won't work
 d->mCommandFrame = new BosonCommandFrame(parent, false);

 connect(d->mBoson, SIGNAL(signalUpdateProduction(Facility*)),
		d->mCommandFrame, SLOT(slotFacilityProduces(Facility*)));
 connect(d->mBoson, SIGNAL(signalCompletedProduction(Facility*)),
		d->mCommandFrame, SLOT(slotProductionCompleted(Facility*)));
 
 slotChatFramePosition(BosonConfig::readChatFramePosition());
}

void BosonWidget::startGameMode()
{
 d->mEditorMode = false;
 d->mBoson->slotSetGameSpeed(BosonConfig::readGameSpeed());


 initBigDisplay(d->mDisplayManager->addInitialDisplay());
 recreateLayout();
}

void BosonWidget::startEditor()
{
 d->mEditorMode = true;

 initBigDisplay(d->mDisplayManager->addInitialDisplay());
 recreateLayout();


 slotChangeCursor(CursorKDE, boConfig->readCursorDir());
	

// FIXME - the stuff below should be replaced by a proper dialog and config
// implementation

 addDummyComputerPlayer(i18n("Computer 1"));
 addDummyComputerPlayer(i18n("Computer 2"));

 // load default playfield 
 // FIXME: should be loaded by a dialog!
 slotLoadPlayField(BosonPlayField::defaultPlayField());

// start the chosen scenario
 d->mBoson->sendMessage(0, BosonMessage::IdStartScenario);

// as BosonCanvas::advance() is disabled:
 d->mCanvas->setUpdatePeriod(500); // AB: too high?
}

void BosonWidget::slotLoadPlayField(const QString& identifier)
{
// the map is first loaded locally from the file. Then the data is sent over
// network. It is initialized (i.e. the cells are shown in the canvas) when the
// data is received from network, in slotReceiveMap()
//
// Then, when this is done the scenario is initialized (the scenario is also
// loaded in the playfield, together with the map).
 if (!d->mBoson->isAdmin()) {
	kdWarning() << k_funcinfo << ": not ADMIN" << endl;
	return;
 }
 if (!d->mPlayField->loadPlayField(BosonPlayField::playFieldFileName(identifier))) {
	kdDebug() << k_funcinfo << "Error loading playfield " << identifier << endl;
	return;
 }
 QByteArray buffer;
 QDataStream stream(buffer, IO_WriteOnly);
 d->mPlayField->saveMap(stream);

 // send the loaded map via network. It will be initialized in slotReceiveMap
 d->mBoson->sendMessage(stream, BosonMessage::InitMap);

// also load scenario settings:
 d->mBoson->setMinPlayers(d->mPlayField->scenario()->minPlayers());
 d->mBoson->setMaxPlayers(d->mPlayField->scenario()->maxPlayers());

}

void BosonWidget::slotReceiveMap(const QByteArray& buffer)
{
 kdDebug() << k_funcinfo << endl;
 QDataStream stream(buffer, IO_ReadOnly);
 d->mPlayField->loadMap(stream);

 QString tiles = "earth.png"; // TODO: should be selectable

 // load tiles if in editor mode - otherwise this does nothing
 emit signalEditorLoadTiles(tiles);

 for (unsigned int i = 0; i < d->mBoson->playerCount(); i++) {
	Player* p = (Player*)d->mBoson->playerList()->at(i);
	if (p) {
		p->initMap(d->mPlayField->map());
	}
 }
// kdDebug() << "init minimap" << endl;
 d->mMiniMap->initMap(); // very fast function
 kdDebug() << "init map" << endl;
 d->mCanvas->initMap(tiles); // takes most of startup time!

 kdDebug() << k_funcinfo << " done" << endl;
}

void BosonWidget::slotChangeLocalPlayer(int index)
{
// kdDebug() << k_funcinfo << endl;
 Player* p = (Player*)d->mBoson->playerList()->at(index);
 changeLocalPlayer(p);
 d->mCommandFrame->slotEditorProduction(-1, d->mLocalPlayer);
}

void BosonWidget::changeLocalPlayer(Player* localPlayer)
{
 d->mLocalPlayer = localPlayer;
 d->mCommandFrame->setLocalPlayer(d->mLocalPlayer);
 d->mMiniMap->setLocalPlayer(d->mLocalPlayer);

 QPtrList<BosonBigDisplay> list = d->mDisplayManager->displays();
 QPtrListIterator<BosonBigDisplay> it(list);
 while (it.current()) {
	it.current()->setLocalPlayer(d->mLocalPlayer);
	++it;
 }

 d->mChat->setFromPlayer(d->mLocalPlayer);
 slotSetActiveDisplay(d->mDisplayManager->activeDisplay() ? 
		d->mDisplayManager->activeDisplay() : 
		d->mDisplayManager->displays().first());
}

void BosonWidget::slotAddComputerPlayer(Player* computer)
{
 d->mBoson->addPlayer(computer);
}

void BosonWidget::editorSavePlayField(const QString& fileName)
{
 d->mPlayField->applyScenario(d->mBoson);
 if (d->mPlayField->savePlayField(fileName)) {
	if (d->mPlayField->map()) {
		d->mPlayField->map()->setModified(false);
	}
	if (d->mPlayField->scenario()) {
		d->mPlayField->scenario()->setModified(false);
	}
 }
}

void BosonWidget::saveConfig(bool editor)
{
 // note: the game is *not* saved here! just general settings like game speed,
 // player name, ...
 if (!d->mBoson) {
	kdError() << k_funcinfo << ": NULL game" << endl;
	return;
 }
 if (!d->mLocalPlayer) {
	kdError() << k_funcinfo << ": NULL local player" << endl;
	return;
 }
 kdDebug() << k_funcinfo << endl;
 if (!editor) {
	BosonConfig::saveLocalPlayerName(d->mLocalPlayer->name());
	BosonConfig::saveGameSpeed(d->mBoson->gameSpeed());
 }
 BosonConfig::saveChatFramePosition(d->mChatPos);
 boConfig->save(editor);
 kdDebug() << k_funcinfo << "done" << endl;
}

void BosonWidget::slotSendChangeSpecies(const QString& species)
{
 if (!d->mLocalPlayer) {
	kdError() << k_funcinfo << "NULL localplayer" << endl;
	return;
 }
// kdDebug() << "new species: " << species << endl;
 QByteArray buffer;
 QDataStream stream(buffer, IO_WriteOnly);
 stream << (Q_UINT32)d->mLocalPlayer->id();
 stream << species;
 stream << d->mBoson->availableTeamColors().first().rgb();
 d->mBoson->sendMessage(buffer, BosonMessage::ChangeSpecies);
 // the species is actually loaded in the class Boson when the message is
 // received
}

void BosonWidget::slotSendChangeTeamColor(const QColor& color)
{
 if (!d->mLocalPlayer) {
	kdError() << k_funcinfo << "NULL localplayer" << endl;
	return;
 }
 sendChangeTeamColor(d->mLocalPlayer, color);
}

void BosonWidget::sendChangeTeamColor(Player* player, const QColor& color)
{
 if (!player) {
	kdError() << k_funcinfo << "NULL player" << endl;
	return;
 }
 QByteArray buffer;
 QDataStream stream(buffer, IO_WriteOnly);
 stream << (Q_UINT32)player->id();
 stream << (Q_UINT32)color.rgb();
 d->mBoson->sendMessage(buffer, BosonMessage::ChangeTeamColor);
 // the color is actually changed in the class Boson when the message is
 // received
}

void BosonWidget::zoom(const QWMatrix& m)
{
 if (d->mDisplayManager->activeDisplay()) {
	d->mDisplayManager->activeDisplay()->setWorldMatrix(m);
 }
}

void BosonWidget::slotFog(int x, int y)
{
 // very time critical function!!

 // FIXME: don't connect to this slot if in editor mode!
 // slotFog() and slotUnfog() exist here so that we need only a single slot
 // instead of two (on ein minimap and one to actually create/remove the fog)
 // should save some performance (at least I hope)
 d->mMiniMap->slotFog(x, y); // FIXME: no need for slot
 d->mCanvas->fogLocal(x, y);
}

void BosonWidget::slotUnfog(int x, int y)
{
 d->mMiniMap->slotUnfog(x, y); // FIXME: no need for slot
 d->mCanvas->unfogLocal(x, y);
}

void BosonWidget::slotPlayerPropertyChanged(KGamePropertyBase* prop, KPlayer* p)
{
 if (p != d->mLocalPlayer) {
	// not yet used
	return;
 }
 switch (prop->id()) {
	case Player::IdMinerals:
		emit signalMineralsUpdated(d->mLocalPlayer->minerals());
		break;
	case Player::IdOil:
		emit signalOilUpdated(d->mLocalPlayer->oil());
		break;
	default:
		break;
 }
}

void BosonWidget::slotInitFogOfWar()
{
 d->mCanvas->initFogOfWar(d->mLocalPlayer); //could be used for editor as well - to see what a player can see
 d->mMiniMap->initFogOfWar(d->mLocalPlayer);
}

void BosonWidget::slotCommandFramePosition(int pos)
{
 emit signalMoveCommandFrame(pos);
 d->mCommandPos = (CommandFramePosition)pos;
}

void BosonWidget::slotChatFramePosition(int chatPos)
{
 d->mChatPos = (ChatFramePosition)chatPos;
 recreateLayout();
}

void BosonWidget::recreateLayout()
{
 delete d->mViewLayout;
 d->mViewLayout = 0;
 delete d->mTopLayout;
// redo layout
 d->mTopLayout = new QHBoxLayout(this, 5); // FIXME: 5 is hardcoded
 d->mViewLayout = new QVBoxLayout();

 if (d->mChatPos == ChatFrameTop) {
	d->mViewLayout->addWidget(d->mChat);
 }
 d->mViewLayout->addWidget(d->mDisplayManager);
 if (d->mChatPos != ChatFrameTop) {
	d->mViewLayout->addWidget(d->mChat);
 }

 d->mTopLayout->addLayout(d->mViewLayout);

 d->mTopLayout->activate();
}

bool BosonWidget::sound() const
{
 return boMusic->sound();
}

bool BosonWidget::music() const
{
 return boMusic->music();
}

void BosonWidget::slotToggleSound()
{
 boMusic->setSound(!boMusic->sound());
 boConfig->setSound(boMusic->sound());
}

void BosonWidget::slotToggleMusic()
{
 boMusic->setMusic(!boMusic->music());
 boConfig->setMusic(boMusic->music());
}

void BosonWidget::displayAllItems(bool display)
{
 QCanvasItemList all = d->mCanvas->allItems();
 for (unsigned int i = 0; i < all.count(); i++) {
	all[i]->setVisible(display);
 }
}

void BosonWidget::setShowChat(bool s)
{
 if (s) {
	d->mChat->show();
 } else {
	d->mChat->hide();
 }
}

void BosonWidget::slotNotEnoughMinerals(Player* p)
{
 if (p != d->mLocalPlayer) {
	return;
 }
 addChatSystemMessage(i18n("Boson"), i18n("You have not enough minerals!"));
}

void BosonWidget::slotNotEnoughOil(Player* p)
{
 if (p != d->mLocalPlayer) {
	return;
 }
 addChatSystemMessage(i18n("Boson"), i18n("You have not enough oil!"));
}

void BosonWidget::addChatSystemMessage(const QString& fromName, const QString& text)
{
 d->mChat->addSystemMessage(fromName, text);

 // FIXME: only to the current display or to all displays ??
 if (d->mDisplayManager->activeDisplay()) {
	d->mDisplayManager->activeDisplay()->addChatMessage(i18n("--- %1: %2").arg(fromName).arg(text));
 }
}

void BosonWidget::slotSetCommandButtonsPerRow(int b)
{
 d->mCommandFrame->slotSetButtonsPerRow(b);
}

void BosonWidget::slotUnfogAll(Player* player)
{
 if (!d->mPlayField->map()) {
	kdError() << k_funcinfo << "NULL map" << endl;
	return;
 }
 QPtrList<KPlayer> list;
 if (!player) {
	list = *d->mBoson->playerList();
 } else {
	list.append(player);
 }
 for (unsigned int i = 0; i < list.count(); i++) {
	Player* p = (Player*)list.at(i);
	for (unsigned int x = 0; x < d->mPlayField->map()->width(); x++) {
		for (unsigned int y = 0; y < d->mPlayField->map()->height(); y++) {
			p->unfog(x, y);
		}
	}
 }
}

void BosonWidget::initBigDisplay(BosonBigDisplay* b)
{
 if (!b) {
	kdError() << k_funcinfo << "NULL display" << endl;
	return;
 }
// BosonBigDisplay* b = new BosonBigDisplay(d->mCanvas, d->mDisplayManager);
 connect(b, SIGNAL(signalMakeActive(BosonBigDisplay*)), 
		this, SLOT(slotSetActiveDisplay(BosonBigDisplay*)));
 b->setLocalPlayer(d->mLocalPlayer);
 b->setCursor(d->mCursor);
 b->setKGameChat(d->mChat);


 if (d->mEditorMode) {
	connect(b, SIGNAL(signalAddCell(int,int, int, unsigned char)),
			d->mCanvas, SLOT(slotAddCell(int, int, int, unsigned char)));
	connect(b, SIGNAL(signalBuildUnit(int,int, int, Player*)),
			d->mBoson, SLOT(slotSendAddUnit(int, int, int, Player*)));
	// this manages the mouse input for bosonBigDisplay. In non-editor mode this is
	// done by KGameMouseIO
	EditorInput* input = new EditorInput(b->viewport());
	connect(input, SIGNAL(signalMouseEvent(QMouseEvent*, bool*)), 
			b, SLOT(slotEditorMouseEvent(QMouseEvent*, bool*)));
 } else {
	connect(b, SIGNAL(signalSingleUnitSelected(Unit*)),
			d->mCommandFrame, SLOT(slotSetProduction(Unit*)));
	addMouseIO(b);
 }
		
 static bool init = false;
 if (!init) {
	// TODO these should also be done for ALL displays... maybe use static
	// lists in KSpriteToolTip so we can add them to several views
	// tooltips - added in slotAddUnit
	d->mUnitTips = new KSpriteToolTip(b);
	init = true;
 }

 slotSetActiveDisplay(b);
}

void BosonWidget::addMouseIO(BosonBigDisplay* b)
{
 if (d->mLocalPlayer) {
	if (d->mIOList[b]) {
		kdError() << "This view already has a mouse io!!" << endl;
		return;
	}
	KGameMouseIO* bigDisplayIO = 
			new KGameMouseIO(b->viewport(), true);
	connect(bigDisplayIO, SIGNAL(signalMouseEvent(KGameIO*, QDataStream&,
			QMouseEvent*, bool*)),
			b, SLOT(slotMouseEvent(KGameIO*,
			QDataStream&, QMouseEvent*, bool*)));
	d->mLocalPlayer->addGameIO(bigDisplayIO);
	d->mIOList.insert(b, bigDisplayIO);
 }
}

void BosonWidget::slotSplitDisplayHorizontal()
{
 initBigDisplay(d->mDisplayManager->splitActiveDisplayHorizontal());
}

void BosonWidget::slotSplitDisplayVertical()
{
 initBigDisplay(d->mDisplayManager->splitActiveDisplayVertical());
}

void BosonWidget::slotRemoveActiveDisplay()
{
 d->mDisplayManager->removeActiveDisplay();
}

void BosonWidget::slotChangeCursor(int mode, const QString& cursorDir_)
{
 BosonCursor* b;
 switch (mode) {
	case CursorSprite:
		b = new BosonSpriteCursor;
		break;
	case CursorExperimental:
		b = new BosonExperimentalCursor;
		break;
	case CursorKDE:
		b = new BosonKDECursor;
		break;
	case CursorNormal:
	default:
		b = new BosonNormalCursor;
		break;
 }

 QString cursorDir = cursorDir_;
 if (cursorDir == QString::null) { 
	cursorDir = BosonCursor::defaultTheme();
 }

 bool ok = true;
 if (!b->insertMode(CursorMove, cursorDir, QString::fromLatin1("move"))) {
	ok = false;
 }
 if (!b->insertMode(CursorAttack, cursorDir, QString::fromLatin1("attack"))) {
	ok = false;
 }
 if (!b->insertMode(CursorDefault, cursorDir, QString::fromLatin1("default"))) {
	ok = false;
 }
 if (!ok) {
	kdError() << k_funcinfo << "Could not load cursor mode " << mode << " from " << cursorDir << endl;
	if (!d->mCursor) {
		// load fallback cursor
		slotChangeCursor(CursorKDE, QString::null);
		return;
	}
	// continue to use the old cursor
	delete b;
	return;
 }
 delete d->mCursor;
 d->mCursor = b;
 d->mDisplayManager->setCursor(d->mCursor);
 d->mCursorTheme = cursorDir;

 // some cursors need special final initializations. do them now
 switch (mode) {
	case CursorSprite:
		((BosonSpriteCursor*)d->mCursor)->setCanvas(d->mCanvas,
				CursorDefault, Z_CANVAS_CURSOR);
		break;
	case CursorExperimental:
		break;
	case CursorNormal:
	default:
		break;

 }
}

void BosonWidget::slotChangeGroupMove(int mode)
{
 boConfig->saveGroupMoveMode((GroupMoveMode)mode);
}

void BosonWidget::slotSetActiveDisplay(BosonBigDisplay* display)
{
 if (display == d->mDisplayManager->activeDisplay()) {
	return;
 }

 BosonBigDisplay* old = d->mDisplayManager->activeDisplay();
 d->mDisplayManager->setActiveDisplay(display);
 if (!display) {
	kdWarning() << k_funcinfo << "NULL display" << endl;
	return;
 }

 if (old) {
	disconnect(old, SIGNAL(contentsMoving(int, int)),
		d->mMiniMap, SLOT(slotMoveRect(int, int)));
	disconnect(old, SIGNAL(signalSizeChanged(int, int)),
			d->mMiniMap, SLOT(slotResizeRect(int, int)));
	disconnect(d->mMiniMap, SIGNAL(signalReCenterView(const QPoint&)),
			old, SLOT(slotReCenterView(const QPoint&)));
	disconnect(old, SIGNAL(signalSingleUnitSelected(Unit*)), 
			d->mCommandFrame, SLOT(slotShowSingleUnit(Unit*)));
	disconnect(old, SIGNAL(signalSelectUnit(Unit*)), 
			d->mCommandFrame, SLOT(slotShowUnit(Unit*)));
 }
 connect(display, SIGNAL(contentsMoving(int, int)),
		d->mMiniMap, SLOT(slotMoveRect(int, int)));
 connect(display, SIGNAL(signalSizeChanged(int, int)),
		d->mMiniMap, SLOT(slotResizeRect(int, int)));
 connect(d->mMiniMap, SIGNAL(signalReCenterView(const QPoint&)),
		display, SLOT(slotReCenterView(const QPoint&)));
 connect(display, SIGNAL(signalSingleUnitSelected(Unit*)), 
		d->mCommandFrame, SLOT(slotShowSingleUnit(Unit*)));
 connect(display, SIGNAL(signalSelectUnit(Unit*)), 
		d->mCommandFrame, SLOT(slotShowUnit(Unit*)));


 if (d->mEditorMode) {
	if (old) {
		disconnect(old, SIGNAL(signalAddCell(int,int, int, unsigned char)),
				d->mMiniMap, SLOT(slotAddCell(int, int, int, unsigned char)));
	}
	connect(display, SIGNAL(signalAddCell(int,int, int, unsigned char)),
			d->mMiniMap, SLOT(slotAddCell(int, int, int, unsigned char)));
 } else {
	if (old) {
		disconnect(d->mMiniMap, SIGNAL(signalMoveSelection(int, int)),
				old, SLOT(slotMoveSelection(int, int)));
	}
	connect(d->mMiniMap, SIGNAL(signalMoveSelection(int, int)),
			display, SLOT(slotMoveSelection(int, int)));
 }




 
 // note: all other bigdisplays should unselect now.
 // d->mLocalPlayer must be known at this point. If it isn't we just return. in
 // changeCurrentPlayer() we call slotSetActiveDisplay() again.
 if (!d->mLocalPlayer) {
	return;
 }
 // BosonBigDisplay knows whether a unit was selected. If a unit changed forward
 // the signal to the big display and let it decide whether the
 // signalSingleUnitSelected should be emitted
 if (old) {
	disconnect(d->mLocalPlayer, SIGNAL(signalUnitChanged(Unit*)), 
			old, SLOT(slotUnitChanged(Unit*)));
 }
 connect(d->mLocalPlayer, SIGNAL(signalUnitChanged(Unit*)), 
		display, SLOT(slotUnitChanged(Unit*)));
}

void BosonWidget::slotOutOfGame(Player* p)
{
 int inGame = 0;
 Player* winner = 0;
 for (unsigned int i = 0; i < d->mBoson->playerList()->count(); i++) {
	if (!((Player*)d->mBoson->playerList()->at(i))->isOutOfGame()) {
		winner = (Player*)d->mBoson->playerList()->at(i);
		inGame++;
	}
 }
 if (inGame <= 1 && winner) {
	kdDebug() << k_funcinfo << "We have a winner! id=" << winner->id() << endl;
	 delete d->mGameOverDialog;
	 d->mGameOverDialog = new GameOverDialog(this);
	 d->mGameOverDialog->createStatistics(d->mBoson, winner, d->mLocalPlayer);
	 d->mGameOverDialog->show();
	 d->mBoson->setGameStatus(KGame::End);
 } else if (!winner) {
	kdError() << k_funcinfo << "no player left ?!" << endl;
	return;
 }
}

void BosonWidget::debugKillPlayer(KPlayer* p)
{
 d->mCanvas->killPlayer((Player*)p);
}
