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

#include "bosonbigdisplaybase.h"
#include "bosonminimap.h"
#include "bosoncanvas.h"
#include "boson.h"
#include "player.h"
#include "unitproperties.h"
#include "unit.h"
#include "bosoncommandframe.h"
#include "bosonmessage.h"
#include "bosonmap.h"
#include "bosonplayfield.h"
#include "bosonscenario.h"
#include "bosonconfig.h"
#include "optionsdialog.h"
#include "kgameunitdebug.h"
#include "bosonmusic.h"
#include "bosoncursor.h"
#include "commandinput.h"
#include "bodisplaymanager.h"
#include "gameoverdialog.h"
#include "boselection.h"
#include "global.h"
#include "top.h"

#include "defines.h"

#include <kapplication.h>
#include <klocale.h>
#include <kaction.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kdockwidget.h>
#include <kgame/kgamedebugdialog.h>
#include <kgame/kgamepropertyhandler.h>
#include <kgame/kgamechat.h>

#include <qlayout.h>
#include <qvbox.h>
#include <qptrlist.h>
#include <qpainter.h>
#include <qtimer.h>

#include "bosonwidget.moc"

// the kaction patch (will) make it possible to make it emit integer values. but
// currently it doesn't - so use 4 slots instead of one
#define OLD_KACTION 1

class BosonWidget::BosonWidgetPrivate
{
public:
	BosonWidgetPrivate()
	{
		mCommandFrame = 0;
		mCommandFrameDock = 0;

		mChat = 0;
		mChatDock = 0;

		mGameOverDialog = 0;
	}

	BosonCommandFrame* mCommandFrame;
	KDockWidget* mCommandFrameDock;

	KGameChat* mChat;
	KDockWidget* mChatDock;

	GameOverDialog* mGameOverDialog;
};

BosonWidget::BosonWidget(TopWidget* top, QWidget* parent)
    : QWidget( parent, "BosonWidget" ), KXMLGUIClient(/*FIXME: clientParent!*/top)
{
 d = new BosonWidgetPrivate;
 mTop = top;

 mCursor = 0;
 mMiniMap = 0;
 mDisplayManager = 0;

 mMobilesCount = 0;
 mFacilitiesCount = 0;
		
// XMLClient stuff - not all of this is necessary! most cut'n'pasted from
// elsewhere
 actionCollection()->setWidget(this);
 setXMLFile(top->xmlFile());
// XMLClient stuff end

 init();

// again XMLClient stuff - this needs to be called *after* creation of the
// KAction objects.
 mTop->factory()->addClient(this);
}

BosonWidget::~BosonWidget()
{
 kdDebug() << k_funcinfo << endl;
 if (mTop && mTop->factory()) {
	mTop->factory()->removeClient(this);
 }
 delete d->mCommandFrameDock;
 delete d->mChatDock;

 delete mCursor;
 delete mDisplayManager;

 delete d;
 kdDebug() << k_funcinfo << "done" << endl;
}

inline BosonCanvas* BosonWidget::canvas() const
{
 return mTop->canvas();
}

inline BosonPlayField* BosonWidget::map() const
{
 return mTop->map();
}

inline Player* BosonWidget::player() const
{
 return mTop->player();
}

inline Boson* BosonWidget::game() const
{
 return mTop->game();
}

void BosonWidget::init()
{
 // NOTE: order of init* methods is very important here, so don't change it,
 //  unless you know what you're doing!
 initMiniMap();
 initChat();
 initGameCommandFrame();
 initDisplayManager();
 initMap();
 initPlayer();

 initConnections();
 initKeys();

 slotChangeCursor(boConfig->readCursorMode(), boConfig->readCursorDir());
 slotChangeGroupMove(boConfig->readGroupMoveMode());

 setFocusPolicy(StrongFocus); // accept key event
 setFocus();

 emit signalInitDone();
}

void BosonWidget::initMap()
{
 kdDebug() << k_funcinfo << endl;

 canvas()->setMap(map()->map());
 for (unsigned int i = 0; i < game()->playerCount(); i++) {
	Player* p = (Player*)game()->playerList()->at(i);
	if (p) {
		p->initMap(map()->map());
	}
 }
 minimap()->setMap(map()->map());
 minimap()->initMap();
}

void BosonWidget::initMiniMap()
{
 mMiniMap = new BosonMiniMap(0);
 minimap()->hide();
 minimap()->setCanvas(canvas());
 minimap()->setBackgroundOrigin(WindowOrigin);
 minimap()->setLocalPlayer(player());

 connect(game(), SIGNAL(signalAddUnit(Unit*, int, int)),
		minimap(), SLOT(slotAddUnit(Unit*, int, int)));
 connect(canvas(), SIGNAL(signalUnitMoved(Unit*, double, double)),
		minimap(), SLOT(slotMoveUnit(Unit*, double, double)));
 connect(canvas(), SIGNAL(signalUnitDestroyed(Unit*)),
		minimap(), SLOT(slotUnitDestroyed(Unit*)));
}

void BosonWidget::initConnections()
{
 connect(canvas(), SIGNAL(signalUnitDestroyed(Unit*)),
		this, SLOT(slotRemoveUnit(Unit*)));
 connect(canvas(), SIGNAL(signalOutOfGame(Player*)),
		this, SLOT(slotOutOfGame(Player*)));

 connect(game(), SIGNAL(signalAdvance(unsigned int)),
		canvas(), SLOT(slotAdvance(unsigned int)));
 connect(game(), SIGNAL(signalAddUnit(Unit*, int, int)),
		canvas(), SLOT(slotAddUnit(Unit*, int, int))); // needs a QCanvas - we need to call Boson::setCanvas for this
 connect(game(), SIGNAL(signalNewGroup(Unit*, QPtrList<Unit>)),
		canvas(), SLOT(slotNewGroup(Unit*, QPtrList<Unit>)));

 connect(game(), SIGNAL(signalAddUnit(Unit*, int, int)),
		this, SLOT(slotAddUnit(Unit*, int, int)));
 connect(game(), SIGNAL(signalInitFogOfWar()),
		this, SLOT(slotInitFogOfWar()));
/* connect(game(), SIGNAL(signalStartScenario()),
		this, SLOT(slotStartScenario()));*/
 connect(game(), SIGNAL(signalGameStarted()),
		this, SIGNAL(signalGameStarted()));
 connect(game(), SIGNAL(signalNotEnoughMinerals(Player*)),
		this, SLOT(slotNotEnoughMinerals(Player*)));
 connect(game(), SIGNAL(signalNotEnoughOil(Player*)),
		this, SLOT(slotNotEnoughOil(Player*)));
}

void BosonWidget::initDisplayManager()
{
 mDisplayManager = new BoDisplayManager(canvas(), this, game()->gameMode());
 connect(mDisplayManager, SIGNAL(signalActiveDisplay(BosonBigDisplayBase*, BosonBigDisplayBase*)),
		this, SLOT(slotSetActiveDisplay(BosonBigDisplayBase*, BosonBigDisplayBase*)));
 canvas()->setDisplayManager(displaymanager());
 displaymanager()->setLocalPlayer(player());
 initBigDisplay(displaymanager()->addInitialDisplay());
}

void BosonWidget::initChat()
{
 d->mChatDock = mTop->createDockWidget("chat_dock", 0, 0, i18n("Chat"));
 d->mChat = new KGameChat(game(), BosonMessage::IdChat, d->mChatDock);
 d->mChatDock->setWidget(d->mChat);
 d->mChatDock->hide();
 d->mChat->setFromPlayer(player());

 connect(d->mChatDock, SIGNAL(iMBeingClosed()), this, SIGNAL(signalChatDockHidden()));
 connect(d->mChatDock, SIGNAL(hasUndocked()), this, SIGNAL(signalChatDockHidden()));
}

void BosonWidget::initPlayer()
{
 connect(player(), SIGNAL(signalUnfog(int, int)),
		this, SLOT(slotUnfog(int, int)));
 connect(player(), SIGNAL(signalFog(int, int)),
		this, SLOT(slotFog(int, int)));
 connect(player(), SIGNAL(signalShowMiniMap(bool)),
		minimap(), SLOT(slotShowMap(bool)));
 connect(player(), SIGNAL(signalPropertyChanged(KGamePropertyBase*, KPlayer*)),
		this, SLOT(slotPlayerPropertyChanged(KGamePropertyBase*, KPlayer*)));
}

void BosonWidget::slotChangeCursor(int mode, const QString& cursorDir_)
{
 kdDebug() << k_funcinfo << endl;
 if (!game()->gameMode()) {
	mode = CursorKDE;
 }
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
	delete b;
	if (!mCursor && mode != CursorKDE) { // loading *never* fails for CursorKDE. we check here anyway.
		// load fallback cursor
		slotChangeCursor(CursorKDE, QString::null);
		return;
	}
	// continue to use the old cursor
	return;
 }
 if(mCursor) {
	delete mCursor;
 }
 mCursor = b;
 displaymanager()->setCursor(mCursor);
 mCursorTheme = cursorDir;

 // some cursors need special final initializations. do them now
 switch (mode) {
	case CursorSprite:
		((BosonSpriteCursor*)mCursor)->setCanvas(canvas(),
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

void BosonWidget::initGameMode()//FIXME: rename! we don't have a difference to initEditorMode anymore. maybe just initGame() or so??
{
 initLayout();
 slotStartScenario();
}

void BosonWidget::initBigDisplay(BosonBigDisplayBase* b)
{
 if (!b) {
	kdError() << k_funcinfo << "NULL display" << endl;
	return;
 }
 b->setLocalPlayer(player());
 b->setCursor(mCursor);
 b->setKGameChat(d->mChat);


 connect(b->selection(), SIGNAL(signalSingleUnitSelected(Unit*)),
		d->mCommandFrame, SLOT(slotSetAction(Unit*)));

 connect(b->selection(), SIGNAL(signalSingleUnitSelected(Unit*)),
		d->mCommandFrame, SLOT(slotShowSingleUnit(Unit*)));
 connect(b->selection(), SIGNAL(signalSelectUnit(Unit*)),
		d->mCommandFrame, SLOT(slotShowUnit(Unit*)));

 b->makeActive();
}

void BosonWidget::initGameCommandFrame()
{
 d->mCommandFrameDock = mTop->createDockWidget("cmdframe_dock", 0, 0, i18n("Command Frame"));
 d->mCommandFrame = new BosonCommandFrame(d->mCommandFrameDock, false);
 d->mCommandFrameDock->setWidget(d->mCommandFrame);
 d->mCommandFrameDock->hide();
 d->mCommandFrame->reparentMiniMap(minimap());
 d->mCommandFrame->setLocalPlayer(player());

 connect(game(), SIGNAL(signalUpdateProduction(Facility*)),
		d->mCommandFrame, SLOT(slotUpdateProduction(Facility*)));
 connect(d->mCommandFrameDock, SIGNAL(iMBeingClosed()), this, SIGNAL(signalCmdFrameDockHidden()));
 connect(d->mCommandFrameDock, SIGNAL(hasUndocked()), this, SIGNAL(signalCmdFrameDockHidden()));

 CommandInput* cmdInput = new CommandInput;
 cmdInput->setCommandFrame(d->mCommandFrame);
 player()->addGameIO(cmdInput);
}

void BosonWidget::initLayout()
{
 d->mCommandFrameDock->manualDock(mTop->getMainDockWidget(), KDockWidget::DockLeft, 30);
 d->mChatDock->manualDock(mTop->getMainDockWidget(), KDockWidget::DockBottom, 80);

 QVBoxLayout* topLayout = new QVBoxLayout(this);
 topLayout->addWidget(displaymanager());
 topLayout->activate();
 if(!kapp->config()->hasGroup("BosonGameDock")) {
	// Dock config isn't saved (probably first start). Hide chat dock (we only
	//  show commandframe by default)
	d->mChatDock->changeHideShowState();
 }
 else {
	mTop->loadGameDockConfig();
 }
}

void BosonWidget::slotDebug()
{
 KGameDebugDialog* dlg = new KGameDebugDialog(game(), this);
 QVBox* b = dlg->addVBoxPage(i18n("Debug &Units"));
 KGameUnitDebug* units = new KGameUnitDebug(b);
 units->setBoson(game());
 connect(dlg, SIGNAL(finished()), dlg, SLOT(slotDelayedDestruct()));
 connect(dlg, SIGNAL(signalRequestIdName(int,bool,QString&)),
		this, SLOT(slotDebugRequestIdName(int,bool,QString&)));
 dlg->show();
}

void BosonWidget::slotArrowScrollChanged(int speed)
{
 boConfig->setArrowKeyStep(speed);
}

void BosonWidget::slotMiniMapScaleChanged(double scale)
{
 boConfig->setMiniMapScale(scale);
 minimap()->repaint();
}

void BosonWidget::slotStartScenario()
{
 map()->scenario()->startScenario(game());
 boMusic->startLoop();

 // This DOES NOT work correctly
 game()->startGame(); // correct here? should be so.

 // as soon as this message is received the game is actually started
 if (game()->isAdmin()) {
	game()->sendMessage(0, BosonMessage::IdGameIsStarted);
	if (game()->gameMode()) {
		game()->slotSetGameSpeed(BosonConfig::readGameSpeed());
	}
 }
}

void BosonWidget::slotGamePreferences()
{
 CursorMode mode;
 if (mCursor->isA("BosonSpriteCursor")) {
	mode = CursorSprite;
 } else if (mCursor->isA("BosonExperimentalCursor")) {
	mode = CursorExperimental;
 } else if (mCursor->isA("BosonKDECursor")) {
	mode = CursorKDE;
 } else {
	mode = CursorNormal;
 }

 OptionsDialog* dlg = new OptionsDialog(this);
 connect(dlg, SIGNAL(finished()), dlg, SLOT(slotDelayedDestruct())); // seems not to be called if you quit with "cancel"!
 dlg->setGameSpeed(game()->gameSpeed());
 dlg->setArrowScrollSpeed(boConfig->arrowKeyStep());
 dlg->setMiniMapScale(boConfig->miniMapScale());
 dlg->setRMBScrolling(boConfig->rmbMove());
 dlg->setMMBScrolling(boConfig->mmbMove());
 dlg->setCursor(mode);
 dlg->setCursorEdgeSensity(boConfig->cursorEdgeSensity());

 connect(dlg, SIGNAL(signalArrowScrollChanged(int)),
		this, SLOT(slotArrowScrollChanged(int)));

 connect(dlg, SIGNAL(signalSpeedChanged(int)),
		game(), SLOT(slotSetGameSpeed(int)));

 connect(dlg, SIGNAL(signalCursorChanged(int, const QString&)),
		this, SLOT(slotChangeCursor(int, const QString&)));
 connect(dlg, SIGNAL(signalGroupMoveChanged(int)),
		this, SLOT(slotChangeGroupMove(int)));
 connect(dlg, SIGNAL(signalCmdBackgroundChanged(const QString&)),
		this, SLOT(slotCmdBackgroundChanged(const QString&)));
 connect(dlg, SIGNAL(signalMiniMapScaleChanged(double)),
		this, SLOT(slotMiniMapScaleChanged(double)));

 dlg->show();
}

void BosonWidget::slotAddUnit(Unit* unit, int, int)
{
 if (!unit) {
	kdError() << k_funcinfo << "NULL unit" << endl;
	return;
 }
 Player* p = unit->owner();
 if (!p) {
	kdError() << k_funcinfo << "NULL owner" << endl;
	return;
 }
 if (p != player()) {
	return;
 }
 if (unit->unitProperties()->isMobile()) {
	mMobilesCount++;
 } else {
	mFacilitiesCount++;
 }
 emit signalMobilesCount(mMobilesCount);
 emit signalFacilitiesCount(mFacilitiesCount);
}

void BosonWidget::slotRemoveUnit(Unit* unit)
{
 if (unit->owner() != player()) {
	return;
 }
 if (unit->unitProperties()->isMobile()) {
	mMobilesCount--;
 } else {
	mFacilitiesCount--;
 }
 emit signalMobilesCount(mMobilesCount);
 emit signalFacilitiesCount(mFacilitiesCount);
}

void BosonWidget::zoom(const QWMatrix& m)
{
 if (displaymanager()->activeDisplay()) {
	displaymanager()->activeDisplay()->setWorldMatrix(m);
 }
}

void BosonWidget::slotFog(int x, int y)
{
 // very time critical function!!

 // slotFog() and slotUnfog() exist here so that we need only a single slot
 // instead of two (on ein minimap and one to actually create/remove the fog)
 // should save some performance (at least I hope)
 minimap()->slotFog(x, y); // FIXME: no need for slot
 canvas()->fogLocal(x, y);
}

void BosonWidget::slotUnfog(int x, int y)
{
 minimap()->slotUnfog(x, y); // FIXME: no need for slot
 canvas()->unfogLocal(x, y);
}

void BosonWidget::slotPlayerPropertyChanged(KGamePropertyBase* prop, KPlayer* p)
{
 if (p != player()) {
	// not yet used
	return;
 }
 switch (prop->id()) {
	case Player::IdMinerals:
		emit signalMineralsUpdated(player()->minerals());
		break;
	case Player::IdOil:
		emit signalOilUpdated(player()->oil());
		break;
	default:
		break;
 }
}

void BosonWidget::slotInitFogOfWar()
{
 canvas()->initFogOfWar(player());
 minimap()->initFogOfWar(player());
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
 QCanvasItemList all = canvas()->allItems();
 for (unsigned int i = 0; i < all.count(); i++) {
	all[i]->setVisible(display);
 }
}

void BosonWidget::slotNotEnoughMinerals(Player* p)
{
 if (p != player()) {
	return;
 }
 addChatSystemMessage(i18n("Boson"), i18n("You have not enough minerals!"));
}

void BosonWidget::slotNotEnoughOil(Player* p)
{
 if (p != player()) {
	return;
 }
 addChatSystemMessage(i18n("Boson"), i18n("You have not enough oil!"));
}

void BosonWidget::addChatSystemMessage(const QString& fromName, const QString& text)
{
 d->mChat->addSystemMessage(fromName, text);

 // FIXME: only to the current display or to all displays ??
 if (displaymanager()->activeDisplay()) {
	displaymanager()->activeDisplay()->addChatMessage(i18n("--- %1: %2").arg(fromName).arg(text));
 }
}

void BosonWidget::slotSetCommandButtonsPerRow(int b)
{
 d->mCommandFrame->slotSetButtonsPerRow(b);
}

void BosonWidget::slotUnfogAll(Player* pl)
{
 if (!map()->map()) {
	kdError() << k_funcinfo << "NULL map" << endl;
	return;
 }
 QPtrList<KPlayer> list;
 if (!pl) {
	list = *game()->playerList();
 } else {
	list.append(pl);
 }
 for (unsigned int i = 0; i < list.count(); i++) {
	Player* p = (Player*)list.at(i);
	for (unsigned int x = 0; x < map()->map()->width(); x++) {
		for (unsigned int y = 0; y < map()->map()->height(); y++) {
			p->unfog(x, y);
		}
	}
 }
}

void BosonWidget::slotSplitDisplayHorizontal()
{
 initBigDisplay(displaymanager()->splitActiveDisplayHorizontal());
}

void BosonWidget::slotSplitDisplayVertical()
{
 initBigDisplay(displaymanager()->splitActiveDisplayVertical());
}

void BosonWidget::slotRemoveActiveDisplay()
{
 displaymanager()->removeActiveDisplay();
}

void BosonWidget::slotSetActiveDisplay(BosonBigDisplayBase* active, BosonBigDisplayBase* old)
{
 if (!active) {
	kdWarning() << k_funcinfo << "NULL display" << endl;
	return;
 }

 if (old) {
	disconnect(old, SIGNAL(contentsMoving(int, int)),
			minimap(), SLOT(slotMoveRect(int, int)));
	disconnect(old, SIGNAL(signalSizeChanged(int, int)),
			minimap(), SLOT(slotResizeRect(int, int)));
	disconnect(minimap(), SIGNAL(signalReCenterView(const QPoint&)),
			old, SLOT(slotReCenterView(const QPoint&)));
 }
 connect(active, SIGNAL(contentsMoving(int, int)),
		minimap(), SLOT(slotMoveRect(int, int)));
 connect(active, SIGNAL(signalSizeChanged(int, int)),
		minimap(), SLOT(slotResizeRect(int, int)));
 connect(minimap(), SIGNAL(signalReCenterView(const QPoint&)),
		active, SLOT(slotReCenterView(const QPoint&)));

 if (old) {
	disconnect(minimap(), SIGNAL(signalMoveSelection(int, int)),
			old, SLOT(slotMoveSelection(int, int)));
 }
 connect(minimap(), SIGNAL(signalMoveSelection(int, int)),
		active, SLOT(slotMoveSelection(int, int)));

  // note: all other bigdisplays should unselect now.

 // BosonBigDisplay knows whether a unit was selected. If a unit changed forward
 // the signal to the big display and let it decide whether the
 // signalSingleUnitSelected should be emitted
 if (old) {
	disconnect(player(), SIGNAL(signalUnitChanged(Unit*)),
			old, SLOT(slotUnitChanged(Unit*)));
 }
 connect(player(), SIGNAL(signalUnitChanged(Unit*)),
		active, SLOT(slotUnitChanged(Unit*)));
}

void BosonWidget::slotOutOfGame(Player* p)
{
 // TODO write BosonGameOverWidget, add it to widgetstack in TopWidget and then
 //  use it instead
 int inGame = 0;
 Player* winner = 0;
 for (unsigned int i = 0; i < game()->playerList()->count(); i++) {
	if (!((Player*)game()->playerList()->at(i))->isOutOfGame()) {
		winner = (Player*)game()->playerList()->at(i);
		inGame++;
	}
 }
 if (inGame <= 1 && winner) {
	kdDebug() << k_funcinfo << "We have a winner! id=" << winner->id() << endl;
	delete d->mGameOverDialog;
	d->mGameOverDialog = new GameOverDialog(this);
	d->mGameOverDialog->createStatistics(game(), winner, player());
	d->mGameOverDialog->show();
	connect(d->mGameOverDialog, SIGNAL(finished()), this, SLOT(slotGameOverDialogFinished()));
	game()->setGameStatus(KGame::End);
 } else if (!winner) {
	kdError() << k_funcinfo << "no player left ?!" << endl;
	return;
 }
}

void BosonWidget::debugKillPlayer(KPlayer* p)
{
 canvas()->killPlayer((Player*)p);
}

void BosonWidget::slotCmdBackgroundChanged(const QString& file)
{
 if (file == QString::null) {
	d->mCommandFrame->unsetPalette();
	return;
 }
 QPixmap p(file);
 if (p.isNull()) {
	kdError() << k_funcinfo << "Could not load " << file << endl;
	d->mCommandFrame->unsetPalette();
	return;
 }
 d->mCommandFrame->setPaletteBackgroundPixmap(p);
}

void BosonWidget::initKeys()
{
#ifdef OLD_KACTION
 (void)new KAction(i18n("Scroll Up"), Qt::Key_Up, mDisplayManager,
		SLOT(slotScrollUp()), actionCollection(),
		"scroll_up");
 (void)new KAction(i18n("Scroll Down"), Qt::Key_Down, mDisplayManager,
		SLOT(slotScrollDown()), actionCollection(),
		"scroll_down");
 (void)new KAction(i18n("Scroll Left"), Qt::Key_Left, mDisplayManager,
		SLOT(slotScrollLeft()), actionCollection(),
		"scroll_left");
 (void)new KAction(i18n("Scroll Right"), Qt::Key_Right, mDisplayManager,
		SLOT(slotScrollRight()), actionCollection(),
		"scroll_right");
#else
 (void)new KAction(i18n("Scroll Up"), Qt::Key_Up, mDisplayManager,
		SLOT(slotScroll(int)), actionCollection(),
		QString("scroll_up {%1}").arg(ScrollUp));
 (void)new KAction(i18n("Scroll Down"), Qt::Key_Down, mDisplayManager,
		SLOT(slotScroll(int)), actionCollection(),
		QString("scroll_down {%1}").arg(ScrollDown));
 (void)new KAction(i18n("Scroll Left"), Qt::Key_Left, mDisplayManager,
		SLOT(slotScroll(int)), actionCollection(),
		QString("scroll_left {%1}").arg(ScrollLeft));
 (void)new KAction(i18n("Scroll Right"), Qt::Key_Right, mDisplayManager,
		SLOT(slotScroll(int)), actionCollection(),
		QString("scroll_right {%1}").arg(ScrollRight));
#endif
}

void BosonWidget::slotDebugRequestIdName(int msgid, bool , QString& name)
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
// kdDebug() << name << endl;
}

void BosonWidget::slotEndGame()
{
// this needs to be done first, before the players are removed
 canvas()->deleteDestroyed();
 game()->quitGame();
}

bool BosonWidget::isCmdFrameVisible() const
{
 return d->mCommandFrameDock->isVisible();
}

bool BosonWidget::isChatVisible() const
{
 return d->mChatDock->isVisible();
}

void BosonWidget::setCmdFrameVisible(bool visible)
{
 if(visible && d->mCommandFrameDock->mayBeShow())
	d->mCommandFrameDock->show();
 else if(! visible && d->mCommandFrameDock->mayBeHide())
	d->mCommandFrameDock->hide();
}

void BosonWidget::setChatVisible(bool visible)
{
 if(visible && d->mChatDock->mayBeShow())
	d->mChatDock->show();
 else if(! visible && d->mChatDock->mayBeHide())
	d->mChatDock->hide();
}

void BosonWidget::toggleCmdFrameVisible()
{
 d->mCommandFrameDock->changeHideShowState();
}

void BosonWidget::toggleChatVisible()
{
 d->mChatDock->changeHideShowState();
}

void BosonWidget::saveConfig(bool editor)
{
  // note: the game is *not* saved here! just general settings like game speed,
  // player name, ...
 kdDebug() << k_funcinfo << endl;
 if (!game()) {
	kdError() << k_funcinfo << "NULL game" << endl;
	return;
 }
 if (!player()) {
	kdError() << k_funcinfo << "NULL local player" << endl;
	return;
 }

 if (!editor) {
	BosonConfig::saveLocalPlayerName(player()->name());
	BosonConfig::saveGameSpeed(game()->gameSpeed());
 } else {
	 
 }
 if (mCursor->isA("BosonSpriteCursor")) {
	boConfig->saveCursorMode(CursorSprite);
 } else if (mCursor->isA("BosonExperimentalCursor")) {
	boConfig->saveCursorMode(CursorExperimental);
 } else if (mCursor->isA("BosonKDECursor")) {
	boConfig->saveCursorMode(CursorKDE);
 } else {
	boConfig->saveCursorMode(CursorNormal);
 }
 boConfig->saveCursorDir(mCursorTheme);
 boConfig->save(editor);
 kdDebug() << k_funcinfo << "done" << endl;
}

void BosonWidget::slotGameOverDialogFinished()
{
 d->mGameOverDialog->delayedDestruct();
 emit signalGameOver();
}
