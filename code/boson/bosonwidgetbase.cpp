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

#include "bosonwidgetbase.h"

#include "defines.h"
#include "bosonminimap.h"
#include "bosoncanvas.h"
#include "boson.h"
#include "player.h"
#include "unit.h"
#include "bosonmessage.h"
#include "bosonplayfield.h"
#include "bosonmap.h"
#include "bosonscenario.h"
#include "bosonconfig.h"
#include "kgameunitdebug.h"
#include "kgameplayerdebug.h"
#include "kgamecelldebug.h"
#include "bosonprofilingdialog.h"
#include "bosoncursor.h"
#include "bodisplaymanager.h"
#include "boselection.h"
#include "global.h"
#include "top.h"
#include "bosonbigdisplaybase.h"
#include "bodebug.h"
#include "commandframe/bosoncommandframe.h"
#include "sound/bosonmusic.h"

#include <kapplication.h>
#include <klocale.h>
#include <kaction.h>
#include <kconfig.h>
#include <kpopupmenu.h>
#include <kgame/kgamedebugdialog.h>
#include <kgame/kgamepropertyhandler.h>
#include <kgame/kgamechat.h>

#include <qlayout.h>
#include <qvbox.h>
#include <qptrlist.h>
#include <qtimer.h>
#include <qptrdict.h>
#include <qsignalmapper.h>
#include <qfile.h>

#include "bosonwidgetbase.moc"

#define ID_DEBUG_KILLPLAYER 0

class BosonWidgetBase::BosonWidgetBasePrivate
{
public:
	BosonWidgetBasePrivate()
	{
		mCommandFrame = 0;
		mCommandFrameDock = 0;

		mChat = 0;
		mChatDock = 0;
	}

	BosonCommandFrameBase* mCommandFrame;
	KDockWidget* mCommandFrameDock;

	KGameChat* mChat;
	KDockWidget* mChatDock;

	KActionMenu* mActionDebugPlayers;
	KSelectAction* mActionZoom;
	KToggleAction* mActionChat;
	KToggleAction* mActionCmdFrame;

	QPtrDict<KPlayer> mPlayers; // needed for debug only

	bool mInitialized;
};

BosonWidgetBase::BosonWidgetBase(TopWidget* top, QWidget* parent, bool loading)
    : QWidget( parent, "BosonWidgetBase" ), KXMLGUIClient(/*FIXME: clientParent!*/)
{
 d = new BosonWidgetBasePrivate;
 d->mInitialized = false;
 mTop = top;
 mLoading = loading;

 mMiniMap = 0;
 mDisplayManager = 0;
 mCursor = 0;
 mLocalPlayer = 0;

}

BosonWidgetBase::~BosonWidgetBase()
{
 boDebug() << k_funcinfo << endl;
 d->mPlayers.clear();
 if (mTop && mTop->factory()) {
	mTop->factory()->removeClient(this);
 }
 delete d->mCommandFrameDock;
 delete d->mChatDock;

 delete mDisplayManager;

 delete d;
 boDebug() << k_funcinfo << "done" << endl;
}

BosonCanvas* BosonWidgetBase::canvas() const
{
 return mTop->canvas();
}

BosonPlayField* BosonWidgetBase::playField() const
{
 return mTop->playField();
}

#include <kstandarddirs.h> //locate()
void BosonWidgetBase::init()
{
 // NOTE: order of init* methods is very important here, so don't change it,
 //  unless you know what you're doing!
 if (d->mInitialized) {
	return;
 }
 d->mInitialized = true;
 initMiniMap();
 initChat();
 initCommandFrame();
 initDisplayManager();

 initConnections();
 actionCollection()->setWidget(this); // needs to be called *before* initKActions()
 initKActions();
 // XMLClient stuff. needs to be called *after* initKActions().
 setBosonXMLFile();
// setXML(top()->domDocument().toString());
 setXMLGUIBuildDocument(QDomDocument());
 // XMLClient stuff ends. note that there is a factory()->addClient() in
 // TopWidget!

 setFocusPolicy(StrongFocus); // accept key event
// setFocus(); // nonsense, since its still hidden

 initPlayersMenu();
}

void BosonWidgetBase::initMap()
{
 boDebug() << k_funcinfo << endl;

 canvas()->setMap(playField()->map());
 minimap()->setMap(playField()->map());
 minimap()->initMap();
// boGame->setPlayField(playField()); // already done on startup

 // AB: note that this meets the name "initMap" only slightly. We can't do this
 // when players are initialized, as the map must be known to them once we start
 // loading the units (for *loading* games)
 for (unsigned int i = 0; i < boGame->playerCount(); i++) {
	boDebug() << "init map for player " << i << endl;
	Player* p = (Player*)boGame->playerList()->at(i);
	if (p) {
		p->initMap(playField()->map(), boGame->gameMode());
	}
 }

}

void BosonWidgetBase::initMiniMap()
{
 mMiniMap = new BosonMiniMap(0);
 minimap()->hide();
 minimap()->setCanvas(canvas());
 minimap()->setBackgroundOrigin(WindowOrigin);

 connect(boGame, SIGNAL(signalAddUnit(Unit*, int, int)),
		minimap(), SLOT(slotAddUnit(Unit*, int, int)));
 connect(canvas(), SIGNAL(signalUnitMoved(Unit*, float, float)),
		minimap(), SLOT(slotMoveUnit(Unit*, float, float)));
 connect(canvas(), SIGNAL(signalUnitRemoved(Unit*)),
		minimap(), SLOT(slotUnitDestroyed(Unit*)));
}

void BosonWidgetBase::initConnections()
{
 connect(canvas(), SIGNAL(signalUnitRemoved(Unit*)),
		this, SLOT(slotRemoveUnit(Unit*)));

 connect(boGame, SIGNAL(signalAddUnit(Unit*, int, int)),
		canvas(), SLOT(slotAddUnit(Unit*, int, int))); 
 connect(boGame, SIGNAL(signalAddUnit(Unit*, int, int)),
		this, SLOT(slotAddUnit(Unit*, int, int)));

 connect(boGame, SIGNAL(signalGameStarted()),
		this, SIGNAL(signalGameStarted()));

 connect(boGame, SIGNAL(signalAddChatSystemMessage(const QString&,const QString&)),
		this, SLOT(slotAddChatSystemMessage(const QString&,const QString&)));
}

void BosonWidgetBase::initDisplayManager()
{
 mDisplayManager = new BoDisplayManager(canvas(), this, boGame->gameMode());
 connect(mDisplayManager, SIGNAL(signalActiveDisplay(BosonBigDisplayBase*, BosonBigDisplayBase*)),
		this, SLOT(slotSetActiveDisplay(BosonBigDisplayBase*, BosonBigDisplayBase*)));

 displayManager()->setLocalPlayer(localPlayer()); // this does nothing.
}

void BosonWidgetBase::addInitialDisplay()
{
 initBigDisplay(displayManager()->addInitialDisplay());

 // we need to add the display first (in order to create a valid GL context) and
 // then load the cursor
 slotChangeCursor(boConfig->readCursorMode(), boConfig->readCursorDir());
}

void BosonWidgetBase::initChat()
{
 // note: we can use the chat widget even for editor mode, e.g. for status
 // messages!
 d->mChatDock = mTop->createDockWidget("chat_dock", 0, 0, i18n("Chat"));
 d->mChatDock->setEnableDocking(KDockWidget::DockTop | KDockWidget::DockBottom);
 d->mChat = new KGameChat(boGame, BosonMessage::IdChat, d->mChatDock);
 d->mChatDock->setWidget(d->mChat);
 d->mChatDock->hide();

 connect(d->mChatDock, SIGNAL(iMBeingClosed()), this, SLOT(slotChatDockHidden()));
 connect(d->mChatDock, SIGNAL(hasUndocked()), this, SLOT(slotChatDockHidden()));
}

void BosonWidgetBase::initPlayer()
{
 boDebug() << k_funcinfo << endl;
 if (!localPlayer()) {
	boError() << k_funcinfo << "NULL local player" << endl;
	return;
 }

 setLocalPlayerRecursively(localPlayer());

 connect(localPlayer(), SIGNAL(signalPropertyChanged(KGamePropertyBase*, KPlayer*)),
		this, SLOT(slotPlayerPropertyChanged(KGamePropertyBase*, KPlayer*)));

 // Needed for loading game
 emit signalMineralsUpdated(localPlayer()->minerals());
 emit signalOilUpdated(localPlayer()->oil());
 emit signalMobilesCount(localPlayer()->mobilesCount());
 emit signalFacilitiesCount(localPlayer()->facilitiesCount());
}

void BosonWidgetBase::initGameMode()//FIXME: rename! we don't have a difference to initEditorMode anymore. maybe just initGame() or so??
{
 initLayout();
 if (!mLoading) {
	slotStartScenario();
 }
 mLoading = false;
}

void BosonWidgetBase::initBigDisplay(BosonBigDisplayBase* b)
{
 if (!b) {
	boError() << k_funcinfo << "NULL display" << endl;
	return;
 }
 b->setLocalPlayer(localPlayer());
 //FIXME: initBigDisplay should be done in sublcasses?
 b->setCursor(mCursor);
 b->setKGameChat(d->mChat);

 connect(b->selection(), SIGNAL(signalSelectionChanged(BoSelection*)),
		d->mCommandFrame, SLOT(slotSelectionChanged(BoSelection*)));

 b->makeActive();
}

void BosonWidgetBase::initCommandFrame()
{
 d->mCommandFrameDock = mTop->createDockWidget("cmdframe_dock", 0, 0, i18n("Command Frame"));
 d->mCommandFrameDock->setEnableDocking(KDockWidget::DockLeft | KDockWidget::DockRight);
 d->mCommandFrame = createCommandFrame(d->mCommandFrameDock);
 d->mCommandFrameDock->setWidget(d->mCommandFrame);
 d->mCommandFrameDock->hide();
 d->mCommandFrame->reparentMiniMap(minimap());

 connect(d->mCommandFrameDock, SIGNAL(iMBeingClosed()), this, SLOT(slotCmdFrameDockHidden()));
 connect(d->mCommandFrameDock, SIGNAL(hasUndocked()), this, SLOT(slotCmdFrameDockHidden()));

 // can we use the same input for the editor?
// d->mCmdInput = new CommandInput;
// d->mCmdInput->setCommandFrame(d->mCommandFrame);
}

void BosonWidgetBase::initLayout()
{
 d->mCommandFrameDock->manualDock(mTop->getMainDockWidget(), KDockWidget::DockLeft, 30);
 d->mChatDock->manualDock(mTop->getMainDockWidget(), KDockWidget::DockBottom, 80);

 QVBoxLayout* topLayout = new QVBoxLayout(this);
 topLayout->addWidget(displayManager());

 if(!kapp->config()->hasGroup("BosonGameDock")) {
	// Dock config isn't saved (probably first start). Hide chat dock (we only
	//  show commandframe by default)
	d->mChatDock->changeHideShowState();
 }
 else {
	mTop->loadGameDockConfig();
 }
}

void BosonWidgetBase::changeCursor(BosonCursor* cursor)
{
 if (!cursor) {
	boError() << k_funcinfo << "NULL cursor" << endl;
	return;
 }
 delete mCursor;
 mCursor = cursor;
 displayManager()->setCursor(mCursor);
}

void BosonWidgetBase::slotDebug()
{
 KGameDebugDialog* dlg = new KGameDebugDialog(boGame, this);
 
 QVBox* b = dlg->addVBoxPage(i18n("Debug &Units"));
 KGameUnitDebug* units = new KGameUnitDebug(b);
 units->setBoson(boGame);

 b = dlg->addVBoxPage(i18n("Debug &Boson Players"));
 KGamePlayerDebug* player = new KGamePlayerDebug(b);
 player->setBoson(boGame);

 b = dlg->addVBoxPage(i18n("Debug &Cells"));
 KGameCellDebug* cells = new KGameCellDebug(b);
 cells->setMap(playField()->map());

 connect(dlg, SIGNAL(finished()), dlg, SLOT(slotDelayedDestruct()));
 connect(dlg, SIGNAL(signalRequestIdName(int,bool,QString&)),
		this, SLOT(slotDebugRequestIdName(int,bool,QString&)));
 dlg->show();
}

void BosonWidgetBase::slotProfiling()
{
 BosonProfilingDialog* dlg = new BosonProfilingDialog(this, false); // note that dialog won't get updated while it is running, even if its non-modal!
 connect(dlg, SIGNAL(finished()), dlg, SLOT(slotDelayedDestruct()));
 dlg->exec();
}

void BosonWidgetBase::slotMiniMapScaleChanged(double scale)
{
 boConfig->setMiniMapScale(scale);
 minimap()->repaint();
}

void BosonWidgetBase::slotHack1()
{
 QSize size = displayManager()->activeDisplay()->size();
 displayManager()->activeDisplay()->resize(size.width() - 1, size.height() - 1);
 displayManager()->activeDisplay()->resize(size);
}

void BosonWidgetBase::slotAddUnit(Unit* unit, int, int)
{
 if (!unit) {
	boError() << k_funcinfo << "NULL unit" << endl;
	return;
 }
 Player* p = unit->owner();
 if (!p) {
	boError() << k_funcinfo << "NULL owner" << endl;
	return;
 }
 if (p != localPlayer()) {
	return;
 }

 emit signalMobilesCount(p->mobilesCount());
 emit signalFacilitiesCount(p->facilitiesCount());
}

void BosonWidgetBase::slotRemoveUnit(Unit* unit)
{
 if (unit->owner() != localPlayer()) {
	return;
 }

 emit signalMobilesCount(unit->owner()->mobilesCount());
 emit signalFacilitiesCount(unit->owner()->facilitiesCount());
}

void BosonWidgetBase::setZoomFactor(float factor)
{
 if (displayManager()->activeDisplay()) {
	displayManager()->activeDisplay()->setZoomFactor(factor);
 }
}

void BosonWidgetBase::slotFog(int x, int y)
{
 // very time critical function!!

 // slotFog() and slotUnfog() exist here so that we need only a single slot
 // instead of two (on ein minimap and one to actually create/remove the fog)
 // should save some performance (at least I hope)
 minimap()->slotFog(x, y); // FIXME: no need for slot
}

void BosonWidgetBase::slotUnfog(int x, int y)
{
 minimap()->slotUnfog(x, y); // FIXME: no need for slot
}

void BosonWidgetBase::slotPlayerPropertyChanged(KGamePropertyBase* prop, KPlayer* p)
{
 if (p != localPlayer()) {
	// not yet used
	return;
 }
 switch (prop->id()) {
	case Player::IdMinerals:
		emit signalMineralsUpdated(localPlayer()->minerals());
		break;
	case Player::IdOil:
		emit signalOilUpdated(localPlayer()->oil());
		break;
	default:
		break;
 }
}

void BosonWidgetBase::slotInitFogOfWar()
{
// AB: could be placed into BosonWidget, since Editor doesnt have this
 minimap()->initFogOfWar(localPlayer());
}

bool BosonWidgetBase::sound() const
{
 return boMusic->sound();
}

bool BosonWidgetBase::music() const
{
 return boMusic->music();
}

void BosonWidgetBase::slotToggleSound()
{
 boMusic->setSound(!boMusic->sound());
 boConfig->setSound(boMusic->sound());
}

void BosonWidgetBase::slotToggleMusic()
{
 boMusic->setMusic(!boMusic->music());
 boConfig->setMusic(boMusic->music());
}

void BosonWidgetBase::slotAddChatSystemMessage(const QString& fromName, const QString& text)
{
 // add a chat system-message *without* sending it over network (makes no sense
 // for system messages)
 d->mChat->addSystemMessage(fromName, text);

 displayManager()->addChatMessage(i18n("--- %1: %2").arg(fromName).arg(text));
}

void BosonWidgetBase::slotSetCommandButtonsPerRow(int b)
{
 d->mCommandFrame->slotSetButtonsPerRow(b);
}

void BosonWidgetBase::slotUnfogAll(Player* pl)
{
 if (!playField()->map()) {
	boError() << k_funcinfo << "NULL map" << endl;
	return;
 }
 QPtrList<KPlayer> list;
 if (!pl) {
	list = *boGame->playerList();
 } else {
	list.append(pl);
 }
 for (unsigned int i = 0; i < list.count(); i++) {
	Player* p = (Player*)list.at(i);
	for (unsigned int x = 0; x < playField()->map()->width(); x++) {
		for (unsigned int y = 0; y < playField()->map()->height(); y++) {
			p->unfog(x, y);
		}
	}
	boGame->slotAddChatSystemMessage(i18n("Debug"), i18n("Unfogged player %1 - %2").arg(p->id()).arg(p->name()));
 }
}

void BosonWidgetBase::slotSplitDisplayHorizontal()
{
 initBigDisplay(displayManager()->splitActiveDisplayHorizontal());
}

void BosonWidgetBase::slotSplitDisplayVertical()
{
 initBigDisplay(displayManager()->splitActiveDisplayVertical());
}

void BosonWidgetBase::slotRemoveActiveDisplay()
{
 displayManager()->removeActiveDisplay();
}

void BosonWidgetBase::slotSetActiveDisplay(BosonBigDisplayBase* active, BosonBigDisplayBase* old)
{
 if (!active) {
	boWarning() << k_funcinfo << "NULL display" << endl;
	return;
 }

 if (old) {
	disconnect(old, SIGNAL(signalChangeViewport(const QPoint&,
			const QPoint&, const QPoint&, const QPoint&)),
			minimap(), SLOT(slotMoveRect(const QPoint&,
			const QPoint&, const QPoint&, const QPoint&)));
	disconnect(minimap(), SIGNAL(signalReCenterView(const QPoint&)),
			old, SLOT(slotReCenterDisplay(const QPoint&)));
	connect(d->mCommandFrame, SIGNAL(signalSelectUnit(Unit*)), 
			old->selection(), SLOT(slotSelectSingleUnit(Unit*)));
 }
 connect(active, SIGNAL(signalChangeViewport(const QPoint&,const QPoint&,
		const QPoint&, const QPoint&)),
		minimap(), SLOT(slotMoveRect(const QPoint&, const QPoint&,
		const QPoint&, const QPoint&)));
 connect(minimap(), SIGNAL(signalReCenterView(const QPoint&)),
		active, SLOT(slotReCenterDisplay(const QPoint&)));
 connect(d->mCommandFrame, SIGNAL(signalSelectUnit(Unit*)), 
		active->selection(), SLOT(slotSelectSingleUnit(Unit*)));

 if (old) {
	disconnect(minimap(), SIGNAL(signalMoveSelection(int, int)),
			old, SLOT(slotMoveSelection(int, int)));
 }
 connect(minimap(), SIGNAL(signalMoveSelection(int, int)),
		active, SLOT(slotMoveSelection(int, int)));

 // note: all other bigdisplays should unselect now.

 if (!localPlayer()) {
	boWarning() << k_funcinfo << "NULL localplayer" << endl;
	return;
 }

 // BosonBigDisplay knows whether a unit was selected. If a unit changed forward
 // the signal to the big display and let it decide whether the
 // signalSingleUnitSelected should be emitted
 if (old) {
	disconnect(localPlayer(), SIGNAL(signalUnitChanged(Unit*)),
			old, SLOT(slotUnitChanged(Unit*)));
 }
 connect(localPlayer(), SIGNAL(signalUnitChanged(Unit*)),
		active, SLOT(slotUnitChanged(Unit*)));
}

void BosonWidgetBase::debugKillPlayer(KPlayer* p)
{
 canvas()->killPlayer((Player*)p);
 boGame->slotAddChatSystemMessage(i18n("Debug"), i18n("Killed player %1 - %2").arg(p->id()).arg(p->name()));
}

void BosonWidgetBase::slotCmdBackgroundChanged(const QString& file)
{
 if (file == QString::null) {
	d->mCommandFrame->unsetPalette();
	return;
 }
 QPixmap p(file);
 if (p.isNull()) {
	boError() << k_funcinfo << "Could not load " << file << endl;
	d->mCommandFrame->unsetPalette();
	return;
 }
 d->mCommandFrame->setPaletteBackgroundPixmap(p);
}

void BosonWidgetBase::initKActions()
{
 QSignalMapper* scrollMapper = new QSignalMapper(this);
 connect(scrollMapper, SIGNAL(mapped(int)), displayManager(), SLOT(slotScroll(int)));
 KAction* a;
 a = new KAction(i18n("Scroll Up"), Qt::Key_Up, scrollMapper,
		SLOT(map()), actionCollection(),
		"scroll_up");
 scrollMapper->setMapping(a, BoDisplayManager::ScrollUp);
 a = new KAction(i18n("Scroll Down"), Qt::Key_Down, scrollMapper,
		SLOT(map()), actionCollection(),
		"scroll_down");
 scrollMapper->setMapping(a, BoDisplayManager::ScrollDown);
 a = new KAction(i18n("Scroll Left"), Qt::Key_Left, scrollMapper,
		SLOT(map()), actionCollection(),
		"scroll_left");
 scrollMapper->setMapping(a, BoDisplayManager::ScrollLeft);
 a = new KAction(i18n("Scroll Right"), Qt::Key_Right, scrollMapper,
		SLOT(map()), actionCollection(),
		"scroll_right");
 scrollMapper->setMapping(a, BoDisplayManager::ScrollRight);

 // FIXME: the editor should not have a "game" menu, so what to do with this?
 (void)new KAction(i18n("&Reset View Properties"), KShortcut(Qt::Key_R),
		displayManager(), SLOT(slotResetViewProperties()), actionCollection(), "game_reset_view_properties");

 // Dockwidgets show/hide
 d->mActionChat = new KToggleAction(i18n("Show &Chat"),
		KShortcut(Qt::CTRL+Qt::Key_C), this, SLOT(slotToggleChatVisible()),
		actionCollection(), "options_show_chat");
 d->mActionCmdFrame = new KToggleAction(i18n("Show C&ommandframe"),
		KShortcut(Qt::CTRL+Qt::Key_F), this, SLOT(slotToggleCmdFrameVisible()),
		actionCollection(), "options_show_cmdframe");

 // Screenshot
 (void)new KAction(i18n("&Grab Screenshot"), KShortcut(Qt::CTRL + Qt::Key_G),
		this, SLOT(slotGrabScreenshot()), actionCollection(), "grab_screenshot");

 // Zoom
 d->mActionZoom = new KSelectAction(i18n("&Zoom"), KShortcut(), actionCollection(), "options_zoom");
 connect(d->mActionZoom, SIGNAL(activated(int)), this, SLOT(slotZoom(int)));
 QStringList items;
 items.append(QString::number(50));
 items.append(QString::number(100));
 items.append(QString::number(150));
 items.append(i18n("Free Zoom"));
 d->mActionZoom->setItems(items);


 // Debug - no i18n!
 (void)new KAction("Profiling", KShortcut(), this,
		SLOT(slotProfiling()), actionCollection(), "debug_profiling");
 (void)new KAction("Unfog", KShortcut(), this,
		SLOT(slotUnfogAll()), actionCollection(), "debug_unfog");
 (void)new KAction("Debug", KShortcut(), this,
		SLOT(slotDebug()), actionCollection(), "debug_kgame");

 KSelectAction* debugMode = new KSelectAction("Mode", KShortcut(), actionCollection(), "debug_mode");
 connect(debugMode, SIGNAL(activated(int)), this, SLOT(slotDebugMode(int)));
 QStringList l;
 l.append("Normal");
 l.append("Debug Selection");
 debugMode->setItems(l);
 debugMode->setCurrentItem(0);
 d->mActionDebugPlayers = new KActionMenu("Players", actionCollection(), "debug_players");

 checkDockStatus();
}

void BosonWidgetBase::slotDebugRequestIdName(int msgid, bool , QString& name)
{
 // we don't use i18n() for debug messages... not worth the work
 switch (msgid) {
	case BosonMessage::InitMap:
		name = "Init Map";
		break;
	case BosonMessage::ChangeSpecies:
		name = "Change Species";
		break;
	case BosonMessage::ChangePlayField:
		name = "Change PlayField";
		break;
	case BosonMessage::ChangeTeamColor:
		name = "Change TeamColor";
		break;
	case BosonMessage::IdInitFogOfWar:
		name = "Init Fog of War";
		break;
	case BosonMessage::IdStartScenario:
		name = "Start Scenario";
		break;
	case BosonMessage::AddUnit:
		name = "Add Unit";
		break;
	case BosonMessage::AddUnitsXML:
		name = "Add Units from XML";
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

void BosonWidgetBase::quitGame()
{
// this needs to be done first, before the players are removed
 displayManager()->quitGame();
 canvas()->deleteDestroyed();
 boGame->quitGame();
}

bool BosonWidgetBase::isCmdFrameVisible() const
{
 return d->mCommandFrameDock->isVisible();
}

bool BosonWidgetBase::isChatVisible() const
{
 return d->mChatDock->isVisible();
}

void BosonWidgetBase::setCmdFrameVisible(bool visible)
{
 if (visible && d->mCommandFrameDock->mayBeShow()) {
	d->mCommandFrameDock->show();
 } else if (! visible && d->mCommandFrameDock->mayBeHide()) {
	d->mCommandFrameDock->hide();
 }
}

void BosonWidgetBase::setChatVisible(bool visible)
{
 if (visible && d->mChatDock->mayBeShow()) {
	d->mChatDock->show();
 } else if (! visible && d->mChatDock->mayBeHide()) {
	d->mChatDock->hide();
 }
}

void BosonWidgetBase::slotToggleCmdFrameVisible()
{
 d->mCommandFrameDock->changeHideShowState();
 checkDockStatus();
}

void BosonWidgetBase::slotToggleChatVisible()
{
 d->mChatDock->changeHideShowState();
 checkDockStatus();
}

void BosonWidgetBase::checkDockStatus()
{
 d->mActionChat->setChecked(isChatVisible());
 d->mActionCmdFrame->setChecked(isCmdFrameVisible());
}


void BosonWidgetBase::saveConfig()
{
 // note: the game is *not* saved here! just general settings like game speed,
 // player name, ...
 boDebug() << k_funcinfo << endl;
 if (!boGame) {
	boError() << k_funcinfo << "NULL game" << endl;
	return;
 }
 if (!localPlayer()) {
	boError() << k_funcinfo << "NULL local player" << endl;
	return;
 }

// boConfig->save(editor); //FIXME - what is this for?
 boDebug() << k_funcinfo << "done" << endl;
}

void BosonWidgetBase::slotStartScenario()
{
 playField()->scenario()->startScenario(boGame);
 boMusic->startLoop();

 // This DOES NOT work correctly
 boGame->startGame(); // correct here? should be so.

 // as soon as this message is received the game is actually started
 if (boGame->isAdmin()) {
	boGame->sendMessage(0, BosonMessage::IdGameIsStarted);
	if (boGame->gameMode()) {
		boGame->slotSetGameSpeed(BosonConfig::readGameSpeed());
	}
 }

 #warning FIXME
 // this is a strange bug: we need to resize the widget once it is shown - otherwise we'll have a VERY slot frame rate.
 // I can't find out where the problem resides :-(
 QTimer::singleShot(500, this, SLOT(slotHack1()));
}

void BosonWidgetBase::slotDebugMode(int index)
{
 boConfig->setDebugMode((BosonConfig::DebugMode)index);
}

void BosonWidgetBase::slotZoom(int index)
{
boDebug() << "zoom index=" << index << endl;
 float percent = d->mActionZoom->items()[index].toFloat(); // bahh!!! 
 float factor = (float)percent / 100;
 setZoomFactor(factor);
}

void BosonWidgetBase::initPlayersMenu()
{
 QPtrListIterator<KPlayer> it(*(boGame->playerList()));
 while (it.current()) {
	slotPlayerJoinedGame(it.current());
	++it;
 }
}

void BosonWidgetBase::slotDebugPlayer(int index)
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
	boError() << k_funcinfo << "player not found" << endl;
	return;
 }

 switch (index) {
	case ID_DEBUG_KILLPLAYER:
		debugKillPlayer(p);
		break;
	default:
		boError() << k_funcinfo << "unknown index " << index << endl;
		break;
 }
}

void BosonWidgetBase::slotChatDockHidden()
{
 d->mActionChat->setChecked(false);
}

void BosonWidgetBase::slotCmdFrameDockHidden()
{
 d->mActionCmdFrame->setChecked(false);
}

void BosonWidgetBase::setBosonXMLFile()
{
 setXMLFile(locate("config", "ui/ui_standards.rc", instance()));
 setXMLFile("bosonbaseui.rc", true);
}

void BosonWidgetBase::slotPlayerJoinedGame(KPlayer* player)
{
 if (!player) {
	return;
 }
 // note: NOT listed in the *ui.rc files! we create it dynamically when the player enters ; not using the xml framework
 KActionMenu* menu = new KActionMenu(player->name(), this, QString("debug_players_%1").arg(player->name()));

 connect(menu->popupMenu(), SIGNAL(activated(int)),
		this, SLOT(slotDebugPlayer(int)));
 menu->popupMenu()->insertItem("Kill Player", ID_DEBUG_KILLPLAYER);

 d->mActionDebugPlayers->insert(menu);
 d->mPlayers.insert(menu, player);
}

void BosonWidgetBase::slotPlayerLeftGame(KPlayer* player)
{
 if (!player) {
	return;
 }
 KActionMenu* menu = 0;
 QPtrDictIterator<KPlayer> it(d->mPlayers);
 for (; it.current() && !menu; ++it) {
	if (it.current() == player) {
		menu = (KActionMenu*)it.currentKey();
	}
 }
 if (!menu) {
	boWarning() << k_funcinfo << "NULL player debug menu" << endl;
	return;
 }
 d->mActionDebugPlayers->remove(menu);
 d->mPlayers.remove(player);
 delete menu;
}

BosonCommandFrameBase* BosonWidgetBase::cmdFrame() const
{
 return d->mCommandFrame;
}

void BosonWidgetBase::setLocalPlayer(Player* p, bool init) //AB: probably init = false is obsolete!
{
 // FIXME: ensure that d->mCmdInput gets removed in BosonWidget before calling
 // this! or maybe while calling this!
 setLocalPlayerRecursively(p);

 if (init) {
	if (!p) {
		boDebug() << k_funcinfo << "NULL player" << endl;
		
		return;
	}
	initPlayer();
 }
}

void BosonWidgetBase::setLocalPlayerRecursively(Player* p)
{
 mLocalPlayer = p;
 if (minimap()) {
	minimap()->setLocalPlayer(localPlayer());
 }
 if (displayManager()) {
	displayManager()->setLocalPlayer(localPlayer());
 }
 if (boGame) {
	boGame->setLocalPlayer(localPlayer());
 }
 if (d->mCommandFrame) {
	d->mCommandFrame->setLocalPlayer(localPlayer());
 }
 if (d->mChat) {
	d->mChat->setFromPlayer(localPlayer());
 }
}

void BosonWidgetBase::slotGrabScreenshot()
{
 kdDebug() << k_funcinfo << "Taking screenshot!" << endl;
 QPixmap ss = QPixmap::grabWindow(mTop->winId());
 QString file;
 for(int i = 0; i < 1000; i++) {
	file.sprintf("boson-%03d.png", i);
	kdDebug() << "Checking if file " << file << " exists" << endl;
	if(!QFile::exists(file)) {
		kdDebug() << "    File doesn't exist, breaking" << endl;
		break;
	} else {
		kdDebug() << "    File exists, continuing" << endl;
	}
 }
 kdDebug() << k_funcinfo << "Saving screenshot to " << file << endl;
 ss.save(file, "PNG");
}
