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
		mBigDisplay = 0;
		mMiniMap = 0;
		mCommandFrame = 0;

		mCanvas = 0;

		mBoson = 0;
		mLocalPlayer = 0;
		mMap = 0;
		mScenario = 0;

		mUnitTips = 0;

		mTopLayout = 0;
		mViewLayout = 0;

		mChat = 0;

		mCursor = 0;
	}
	
	QPtrList<BosonBigDisplay> mDisplayList;
	BosonBigDisplay* mBigDisplay;
	BosonMiniMap* mMiniMap;
	BosonCommandFrame* mCommandFrame;

	BosonCanvas* mCanvas;
	
	Boson* mBoson;
	Player* mLocalPlayer;
	BosonMap* mMap;
	BosonScenario* mScenario;

	KSpriteToolTip* mUnitTips;

	QHBoxLayout* mTopLayout;
	QVBoxLayout* mViewLayout; // chat and bigdisplay
	CommandFramePosition mCommandPos;
	ChatFramePosition mChatPos;

	// performance variables:
	int mMobilesCount;
	int mFacilitiesCount;
	
	// options:
	int mArrowKeyStep;

	KGameChat* mChat;

	BosonCursor* mCursor;

	QPtrDict<KGameMouseIO> mIOList;
};

BosonWidget::BosonWidget(QWidget* parent)
    : QWidget( parent, "BosonWidget" )
{
 init();

 boMusic->setSound(boConfig->sound());
 boMusic->setMusic(boConfig->music());
}

void BosonWidget::init()
{
 d = new BosonWidgetPrivate;
 d->mArrowKeyStep = ARROW_KEY_STEP;
 d->mMobilesCount = 0;
 d->mFacilitiesCount = 0;
 d->mDisplayList.setAutoDelete(true);
 d->mIOList.setAutoDelete(true);

 BosonConfig::initBosonConfig(); // initialize global config
 BosonMusic::initBosonMusic(); 

 d->mCanvas = new BosonCanvas(this);
 connect(d->mCanvas, SIGNAL(signalUnitDestroyed(Unit*)), 
		this, SLOT(slotRemoveUnit(Unit*)));

 d->mBoson = new Boson(this);
 d->mBoson->setCanvas(d->mCanvas); // should not be stored here - but seems to be necessary :-(
 connect(d->mBoson, SIGNAL(signalAdvance()),
		d->mCanvas, SLOT(advance()));
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
 connect(d->mBoson, SIGNAL(signalMapChanged(const QString&)),
		this, SLOT(slotLoadMap(const QString&)));
 connect(d->mBoson, SIGNAL(signalScenarioChanged(const QString&)),
		this, SLOT(slotLoadScenario(const QString&)));
 connect(d->mBoson, SIGNAL(signalGameStarted()),
		this, SIGNAL(signalGameStarted()));
 connect(d->mBoson, SIGNAL(signalNotEnoughMinerals(Player*)),
		this, SLOT(slotNotEnoughMinerals(Player*)));
 connect(d->mBoson, SIGNAL(signalNotEnoughOil(Player*)),
		this, SLOT(slotNotEnoughOil(Player*)));

 slotChangeCursor(boConfig->readCursorMode());
 
 addBigDisplay();
 initChat();

 connect(d->mCanvas, SIGNAL(signalPlaySound(const QString&)), 
		boMusic, SLOT(slotPlaySound(const QString&)));

// 640*480 is probably not enough (KDE needs at least 800*600) but as a minimum
// should be ok.
 setMinimumWidth(640);
 setMinimumHeight(480);

 setFocusPolicy(StrongFocus); // accept key event
 setFocus();
 d->mBoson->slotSetGameSpeed(BosonConfig::readGameSpeed());

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
 connect(d->mBigDisplay, SIGNAL(contentsMoving(int, int)),
		d->mMiniMap, SLOT(slotMoveRect(int, int)));
 connect(d->mBigDisplay, SIGNAL(signalSizeChanged(int, int)),
		d->mMiniMap, SLOT(slotResizeRect(int, int)));
 connect(d->mMiniMap, SIGNAL(signalReCenterView(const QPoint&)),
		d->mBigDisplay, SLOT(slotReCenterView(const QPoint&)));
 connect(d->mCanvas, SIGNAL(signalUnitMoved(Unit*, double, double)),
		d->mMiniMap, SLOT(slotMoveUnit(Unit*, double, double)));
 connect(d->mCanvas, SIGNAL(signalUnitDestroyed(Unit*)), 
		d->mMiniMap, SLOT(slotUnitDestroyed(Unit*)));

// only for editor mode:
 connect(d->mBigDisplay, SIGNAL(signalAddCell(int,int, int, unsigned char)),
		d->mMiniMap, SLOT(slotAddCell(int, int, int, unsigned char)));
}

void BosonWidget::reparentMiniMap(QWidget* parent)
{
 d->mMiniMap->reparent(parent, QPoint(0, 0));
 d->mMiniMap->show();
}

void BosonWidget::initChat()
{
 d->mChat = new KGameChat(d->mBoson, BosonMessage::IdChat, this);
 d->mChat->hide();
 d->mBigDisplay->setKGameChat(d->mChat);
}


BosonWidget::~BosonWidget()
{
 kdDebug() << k_funcinfo << endl;
 if (d->mCursor->isA("BosonSpriteCursor")) {
	boConfig->saveCursorMode(CursorSprite);
 } else if (d->mCursor->isA("BosonExperimentalCursor")) {
	boConfig->saveCursorMode(CursorExperimental);
 } else {
	boConfig->saveCursorMode(CursorNormal);
 }
 d->mIOList.clear();
 delete d->mUnitTips;
 d->mDisplayList.clear();
 d->mBigDisplay = 0;
 delete d->mCursor;

// delete the destroyed units first
 d->mCanvas->deleteDestroyed();
// now delte all KGame stuff. Also removed player and therefore the rest of the
// units.  Otherwise this is deleted later, when all units are already cleared by
// QCanvas (->crash)
 delete d->mBoson;
 delete d->mCanvas;
 if (d->mMap) {
	delete d->mMap;
 }
 if (d->mScenario) {
	delete d->mScenario;
 }
 
 delete d;
 kdDebug() << k_funcinfo << "done" << endl;
}

void BosonWidget::addLocalPlayer()
{
 if (d->mLocalPlayer) {
	d->mIOList.clear();
	delete d->mLocalPlayer;
 }
 Player* p = new Player;
 p->setName(BosonConfig::readLocalPlayerName());
 connect(p, SIGNAL(signalUnfog(int, int)),
		this, SLOT(slotUnfog(int, int)));
 connect(p, SIGNAL(signalFog(int, int)),
		this, SLOT(slotFog(int, int)));
 d->mBoson->addPlayer(p);

 CommandInput* cmdInput = new CommandInput;
 cmdInput->setCommandFrame(d->mCommandFrame);
 p->addGameIO(cmdInput);

 changeLocalPlayer(p);
 for (unsigned int i = 0; i < d->mDisplayList.count(); i++) {
	addMouseIO(d->mDisplayList.at(i));
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
	kdError() << k_funcinfo << ": NULL player" << endl;
	return;
 }
 Player* p = (Player*)player;
 // BosonBigDisplay knows whether a unit was selected. If a unit changed forward
 // the signal to the big display and let it decide whether the
 // signalSingleUnitSelected should be emitted
 connect(p, SIGNAL(signalUnitChanged(Unit*)), 
		d->mBigDisplay, SLOT(slotUnitChanged(Unit*)));
 connect(p, SIGNAL(signalPropertyChanged(KGamePropertyBase*, KPlayer*)),
		this, SLOT(slotPlayerPropertyChanged(KGamePropertyBase*, KPlayer*)));
 if (d->mMap) {
	p->initMap(d->mMap);
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
 switch (e->key()) {
	case Key_Left:
		d->mBigDisplay->scrollBy(-d->mArrowKeyStep, 0);
		break;
	case Key_Right:
		d->mBigDisplay->scrollBy(d->mArrowKeyStep, 0);
		break;
	case Key_Up:
		d->mBigDisplay->scrollBy(0, -d->mArrowKeyStep);
		break;
	case Key_Down:
		d->mBigDisplay->scrollBy(0, d->mArrowKeyStep);
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
 d->mArrowKeyStep = speed;
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
 NewGameDialog* dialog = new NewGameDialog(d->mBoson, d->mLocalPlayer, this);
 connect(dialog, SIGNAL(finished()), dialog, SLOT(slotDelayedDestruct())); // is this called when "cancel"ed?

 // add our costum game config widget
 KGameDialogBosonConfig* bosonConfig = new KGameDialogBosonConfig(0);
 connect(bosonConfig, SIGNAL(signalStartGame()), this, SLOT(slotStartGame()));
 connect(d->mBoson, SIGNAL(signalMapChanged(const QString&)),
		bosonConfig, SLOT(slotMapChanged(const QString&)));
 connect(d->mBoson, SIGNAL(signalScenarioChanged(const QString&)),
		bosonConfig, SLOT(slotScenarioChanged(const QString&)));
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
 bosonConfig->slotMapChanged(0);
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
 if (!d->mScenario) {
	kdError() << k_funcinfo << ": NULL scenario" << endl;
	return;
 }
 d->mScenario->startScenario(d->mBoson);
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
 dlg->setArrowScrollSpeed(d->mArrowKeyStep);
 dlg->setCommandFramePosition(d->mCommandPos);
 dlg->setChatFramePosition(d->mChatPos);

 connect(dlg, SIGNAL(signalArrowScrollChanged(int)),
		this, SLOT(slotArrowScrollChanged(int)));
 connect(dlg, SIGNAL(signalSpeedChanged(int)),
		d->mBoson, SLOT(slotSetGameSpeed(int)));
 connect(dlg, SIGNAL(signalCommandFramePositionChanged(int)),
		this, SLOT(slotCommandFramePosition(int)));
 connect(dlg, SIGNAL(signalChatFramePositionChanged(int)),
		this, SLOT(slotChatFramePosition(int)));

// note: this is difficult for several views! probably d->mBigDisplay should be
// the primary view then.
 connect(dlg, SIGNAL(signalCursorChanged(int)),
		this, SLOT(slotChangeCursor(int)));

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

 if (d->mMap) {
	delete d->mMap;
	d->mMap = 0;
 }

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
 d->mCommandFrame->slotEditorConstruction(index, d->mLocalPlayer);
}

void BosonWidget::recreateMap()
{
 if (d->mMap) {
	delete d->mMap;
 }
 d->mMap = new BosonMap;
 d->mCanvas->setMap(d->mMap);
 d->mMiniMap->setMap(d->mMap);
 connect(d->mBigDisplay, SIGNAL(signalAddCell(int,int, int, unsigned char)),
		d->mMap, SLOT(changeCell(int, int, int, unsigned char)));
}

void BosonWidget::addEditorCommandFrame(QWidget* parent)
{ // remember to call this *after* init() - otherwise connect()s won't work
 d->mCommandFrame = new BosonCommandFrame(parent, true);

 connect(d->mBigDisplay, SIGNAL(signalSingleUnitSelected(Unit*)), 
		d->mCommandFrame, SLOT(slotShowSingleUnit(Unit*)));
 connect(d->mCommandFrame, SIGNAL(signalCellSelected(int)), 
		d->mBigDisplay, SLOT(slotWillPlaceCell(int)));
 connect(this, SIGNAL(signalEditorLoadTiles(const QString&)), 
		d->mCommandFrame, SLOT(slotEditorLoadTiles(const QString&)));

// note: this signal should be connected to something like a CommandInput. But
// we need the values in BosonBigDisplay. So as a temporary solution we have
// this connect here.
 connect(d->mCommandFrame, SIGNAL(signalProduceUnit(int, UnitBase*, KPlayer*)), 
		d->mBigDisplay, SLOT(slotWillConstructUnit(int, UnitBase*, KPlayer*)));

 // AB???
 connect(d->mBigDisplay, SIGNAL(signalSelectUnit(Unit*)), 
		d->mCommandFrame, SLOT(slotShowUnit(Unit*)));


 slotChatFramePosition(BosonConfig::readChatFramePosition());
}

void BosonWidget::addGameCommandFrame(QWidget* parent)
{ // remember to call this *after* init() - otherwise connect()s won't work
 d->mCommandFrame = new BosonCommandFrame(parent, false);

// connect(d->mCommandFrame, SIGNAL(signalProduceUnit(int, Unit*, Player*)),
//		d->mBigDisplay, SLOT(slotWillConstructUnit(int, Unit*, Player*))); // in addEditorCommandFrame()
 connect(d->mBigDisplay, SIGNAL(signalSingleUnitSelected(Unit*)),
		d->mCommandFrame, SLOT(slotShowSingleUnit(Unit*)));
 connect(d->mBigDisplay, SIGNAL(signalSingleUnitSelected(Unit*)),
		d->mCommandFrame, SLOT(slotSetConstruction(Unit*)));
 connect(d->mBigDisplay, SIGNAL(signalSelectUnit(Unit*)), 
		d->mCommandFrame, SLOT(slotShowUnit(Unit*)));

 connect(d->mBoson, SIGNAL(signalUpdateProduction(Facility*)),
		d->mCommandFrame, SLOT(slotFacilityProduces(Facility*)));
 connect(d->mBoson, SIGNAL(signalCompletedProduction(Facility*)),
		d->mCommandFrame, SLOT(slotProductionCompleted(Facility*)));
 
 slotChatFramePosition(BosonConfig::readChatFramePosition());
}

void BosonWidget::startEditor()
{
 connect(d->mBigDisplay, SIGNAL(signalBuildUnit(int,int, int, Player*)),
		d->mBoson, SLOT(slotSendAddUnit(int, int, int, Player*)));
	
 // this manages the mouse input for bosonBigDisplay. In non-editor mode this is
 // done by KGameMouseIO
 EditorInput* input = new EditorInput(d->mBigDisplay->viewport());
 connect(input, SIGNAL(signalMouseEvent(QMouseEvent*, bool*)), 
		d->mBigDisplay, SLOT(slotEditorMouseEvent(QMouseEvent*, bool*)));


// FIXME - the stuff below should be replaced by a proper dialog and config
// implementation

 addDummyComputerPlayer(i18n("Computer 1"));
 addDummyComputerPlayer(i18n("Computer 2"));

 // load default map
 // FIXME: should be loaded by a dialog!
 slotLoadMap(BosonMap::defaultMap());
 slotLoadScenario(BosonScenario::defaultScenario()); // perhaps this should load the map as well - as it depends on the map...


// start the chosen scenario
 d->mBoson->sendMessage(0, BosonMessage::IdStartScenario);
}

void BosonWidget::slotLoadMap(const QString& map)
{
// the map is first loaded locally from the file. Then the data is sent over
// network. It is initialized (i.e. the cells are shown in the canvas) when the
// data is received from network, in slotReceiveMap()
 if (!d->mBoson->isAdmin()) {
	kdWarning() << k_funcinfo << ": not ADMIN" << endl;
	return;
 }
 recreateMap();
 // load the map into d->mMap
 kdDebug() << "load map " << map << endl;
 if (!d->mMap->loadMap(BosonMap::mapFileName(map))) {
	return;
 }

 QByteArray buffer;
 QDataStream stream(buffer, IO_WriteOnly);
 d->mMap->saveMapGeo(stream);
 d->mMap->saveCells(stream);

 // send the loaded map via network. It will be initialized in slotReceiveMap
 d->mBoson->sendMessage(stream, BosonMessage::InitMap);

 delete d->mMap; // we create it again when InitMap is received again.
 d->mMap = 0;
}

void BosonWidget::slotLoadScenario(const QString& scenario)
{
 if (!d->mBoson->isAdmin()) {
	kdWarning() << k_funcinfo << ": not ADMIN" << endl;
	return;
 }
 if (d->mScenario) {
	delete d->mScenario;
 }
 d->mScenario = new BosonScenario;
 d->mScenario->loadScenario(BosonScenario::scenarioFileName(scenario));
 d->mBoson->setMinPlayers(d->mScenario->minPlayers());
 d->mBoson->setMaxPlayers(d->mScenario->maxPlayers());
}

void BosonWidget::slotReceiveMap(const QByteArray& buffer)
{
 kdDebug() << k_funcinfo << endl;
 QDataStream stream(buffer, IO_ReadOnly);
 recreateMap();
 QString tiles = "earth.png"; // TODO: should be selectable
 d->mMap->loadMapGeo(stream);
 d->mMap->loadCells(stream);

 // load tiles if in editor mode - otherwise this does nothing
 emit signalEditorLoadTiles(tiles);

 for (unsigned int i = 0; i < d->mBoson->playerCount(); i++) {
	Player* p = (Player*)d->mBoson->playerList()->at(i);
	if (p) {
		p->initMap(d->mMap);
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
 d->mCommandFrame->slotEditorConstruction(-1, d->mLocalPlayer);
}

void BosonWidget::changeLocalPlayer(Player* localPlayer)
{
 d->mLocalPlayer = localPlayer;
 d->mBigDisplay->setLocalPlayer(d->mLocalPlayer);
 d->mCommandFrame->setLocalPlayer(d->mLocalPlayer);
 d->mMiniMap->setLocalPlayer(d->mLocalPlayer);

 d->mChat->setFromPlayer(d->mLocalPlayer);
}

void BosonWidget::slotAddComputerPlayer(Player* computer)
{
 d->mBoson->addPlayer(computer);
}

void BosonWidget::slotEditorSaveMap(const QString& fileName)
{
 if (!d->mMap) {
	kdError() << k_funcinfo << ": NULL map" << endl;
	return;
 }
 // TODO: let the user choose - binary or XML.
 d->mMap->saveMap(fileName, false);
 setModified(false);
}

void BosonWidget::slotEditorSaveScenario(const QString& fileName)
{
 if (!d->mScenario) {
	kdError() << k_funcinfo << ": NULL scenario" << endl;
	return;
 }
 // TODO: let the user choose - binary or XML? XML is far better here. binary is
 // probably useless (scenario files are not that big).
 d->mScenario->saveScenario(fileName, false);
 setModified(false);
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
 d->mBigDisplay->setWorldMatrix(m);
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
 recreateLayout(chatPos);
}

void BosonWidget::recreateLayout(int chatPos)
{
 if (d->mViewLayout) {
	delete d->mViewLayout;
	d->mViewLayout = 0;
 }
 if (d->mTopLayout) {
	delete d->mTopLayout;
 }
// redo layout
 d->mTopLayout = new QHBoxLayout(this, 5); // FIXME: 5 is hardcoded
 d->mViewLayout = new QVBoxLayout();

 if (chatPos == ChatFrameTop) {
	d->mViewLayout->addWidget(d->mChat);
 }
 for (unsigned int vpos = 0; vpos < d->mDisplayList.count(); vpos++) {
	QPtrList<BosonBigDisplay> list;
	QHBoxLayout* l = 0;
	for (unsigned int i = 0; i < d->mDisplayList.count(); i++) {
		if (d->mDisplayList.at(i)->vPos() == vpos) {
			list.append(d->mDisplayList.at(i));
		}
	}
	if (list.count() > 0) {
		l = new QHBoxLayout(d->mViewLayout);
	}
	for (unsigned int i = 0; i < list.count(); i++) {
		l->addWidget(list.at(i));
		l->activate();
	}
 }
 if (chatPos != ChatFrameTop) {
	d->mViewLayout->addWidget(d->mChat);
 }

 d->mTopLayout->addLayout(d->mViewLayout);

 d->mChatPos = (ChatFramePosition)chatPos;
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

bool BosonWidget::isModified() const
{
 return d->mBigDisplay->isModified();
}

void BosonWidget::setModified(bool m)
{
 d->mBigDisplay->setModified(m);
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
 d->mBigDisplay->addChatMessage(i18n("--- %1: %2").arg(fromName).arg(text));
}

void BosonWidget::slotSetCommandButtonsPerRow(int b)
{
 d->mCommandFrame->slotSetButtonsPerRow(b);
}

void BosonWidget::slotUnfogAll(Player* player)
{
 if (!d->mMap) {
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
	for (unsigned int x = 0; x < d->mMap->width(); x++) {
		for (unsigned int y = 0; y < d->mMap->height(); y++) {
			p->unfog(x, y);
		}
	}
 }
}

void BosonWidget::addBigDisplay()
{
 BosonBigDisplay* b = new BosonBigDisplay(d->mCanvas, this);
 b->setCursor(d->mCursor);
 b->show();
 d->mDisplayList.append(b);

 connect(this, SIGNAL(signalMineralsUpdated(int)),
		b, SLOT(slotUpdateMinerals(int)));
 connect(this, SIGNAL(signalOilUpdated(int)),
		b, SLOT(slotUpdateOil(int)));


// this signal is for editor mode only. No effect in game mode at all.
 connect(b, SIGNAL(signalAddCell(int,int, int, unsigned char)),
		d->mCanvas, SLOT(slotAddCell(int, int, int, unsigned char)));
		

 static bool init = false;
 if (!init) {
	// TODO these should also be done for ALL displays... maybe use static
	// lists in KSpriteToolTip so we can add them to several views
	// tooltips - added in slotAddUnit
	d->mUnitTips = new KSpriteToolTip(b);
	init = true;
 }

 d->mBigDisplay = b;
 addMouseIO(b);

}

void BosonWidget::addMouseIO(BosonBigDisplay* b)
{
 if (d->mLocalPlayer) {
	if (d->mIOList[b]) {
		kdError() << "This view already has a mouse io!!" << endl;
		return;
	}
	KGameMouseIO* bigDisplayIO = 
			new KGameMouseIO(d->mBigDisplay->viewport(), true);
	connect(bigDisplayIO, SIGNAL(signalMouseEvent(KGameIO*, QDataStream&,
			QMouseEvent*, bool*)),
			d->mBigDisplay, SLOT(slotMouseEvent(KGameIO*,
			QDataStream&, QMouseEvent*, bool*)));
	d->mLocalPlayer->addGameIO(bigDisplayIO);
	d->mIOList.insert(b, bigDisplayIO);
 }
}

void BosonWidget::slotSplitViewHorizontal()
{
 int vp = d->mBigDisplay->vPos();
 int hp = d->mBigDisplay->hPos();
 addBigDisplay();
 d->mBigDisplay->setVPos(vp + 1);
 d->mBigDisplay->setVPos(hp);
 recreateLayout(d->mChatPos);
}

void BosonWidget::slotSplitViewVertical()
{
 int vp = d->mBigDisplay->vPos();
 int hp = d->mBigDisplay->hPos();
 addBigDisplay();
 d->mBigDisplay->setVPos(vp);
 d->mBigDisplay->setVPos(hp + 1);
 recreateLayout(d->mChatPos);
}

void BosonWidget::slotRemoveActiveView()
{
 if (d->mDisplayList.count() <= 1) {
	return;
 }
 if (!d->mBigDisplay) {
	return;
 }
 delete d->mBigDisplay;
 d->mBigDisplay = d->mDisplayList.first();
}

void BosonWidget::slotChangeCursor(int mode)
{
 // note: this doesn't make sense here when we use several views!
 BosonCursor* b;
 switch (mode) {
	case CursorSprite:
		b = new BosonSpriteCursor;
		break;
	case CursorExperimental:
		b = new BosonExperimentalCursor;
		break;
	case CursorNormal:
	default:
		b = new BosonNormalCursor;
		break;
 }
 if (d->mCursor) {
	delete d->mCursor;
 }
 d->mCursor = b;
 for (unsigned int i = 0; i < d->mDisplayList.count(); i++) {
	d->mDisplayList.at(i)->setCursor(d->mCursor);
 }

 QString cursorDir = KGlobal::dirs()->findResourceDir("data", 
		"boson/themes/cursors/move/index.desktop") +
		QString::fromLatin1("boson/themes/cursors");
 d->mCursor->insertMode(CursorMove, cursorDir, QString::fromLatin1("move"));
 d->mCursor->insertMode(CursorAttack, cursorDir, QString::fromLatin1("attack"));
 d->mCursor->insertMode(CursorDefault, cursorDir, QString::fromLatin1("default"));
// d->mCursor->setWidgetCursor(d->mBigDisplay);

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


