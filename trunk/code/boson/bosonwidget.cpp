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

#include "defines.h"
#include "bosonminimap.h"
#include "bosoncanvas.h"
#include "boson.h"
#include "player.h"
#include "bosonmessage.h"
#include "bosonplayfield.h"
#include "bosonconfig.h"
#include "optionsdialog.h"
#include "bosoncursor.h"
#include "bodisplaymanager.h"
#include "global.h"
#include "bosonbigdisplay.h"
#include "commandinput.h"
#include "gameoverdialog.h"
#include "commandframe/bosoncommandframe.h"
#include "sound/bosonmusic.h"

#include <kstdgameaction.h>
#include <klocale.h>
#include <kaction.h>
#include <kdebug.h>
#include <kdeversion.h>

#include <qregexp.h>

#include "bosonwidget.moc"

class BosonWidget::BosonWidgetPrivate
{
public:
	BosonWidgetPrivate()
	{
		mCmdInput = 0;

		mGameOverDialog = 0;
	}

	CommandInput* mCmdInput;

	GameOverDialog* mGameOverDialog;
};

BosonWidget::BosonWidget(TopWidget* top, QWidget* parent, bool loading)
    : BosonWidgetBase(top, parent, loading)
{
 d = new BosonWidgetPrivate;
}

BosonWidget::~BosonWidget()
{
 kdDebug() << k_funcinfo << endl;
 delete d;
 kdDebug() << k_funcinfo << "done" << endl;
}

void BosonWidget::initDisplayManager()
{
 BosonWidgetBase::initDisplayManager();
 connect(cmdFrame(), SIGNAL(signalAction(int)),
		displayManager(), SLOT(slotUnitAction(int)));
}

void BosonWidget::initConnections()
{
 BosonWidgetBase::initConnections();
 connect(canvas(), SIGNAL(signalOutOfGame(Player*)),
		this, SLOT(slotOutOfGame(Player*)));

 // this does the actual game. note that editor must not have this!
 connect(boGame, SIGNAL(signalAdvance(unsigned int, bool)),
		canvas(), SLOT(slotAdvance(unsigned int, bool)));

 connect(boGame, SIGNAL(signalInitFogOfWar()),
		this, SLOT(slotInitFogOfWar()));
}

void BosonWidget::initPlayer()
{
 BosonWidgetBase::initPlayer();
 if (!d->mCmdInput) {
	kdError() << k_funcinfo << "NULL command input" << endl;
 } else {
	localPlayer()->addGameIO(d->mCmdInput);
 }

 connect(localPlayer(), SIGNAL(signalUnfog(int, int)),
		this, SLOT(slotUnfog(int, int)));
 connect(localPlayer(), SIGNAL(signalFog(int, int)),
		this, SLOT(slotFog(int, int)));
 connect(localPlayer(), SIGNAL(signalShowMiniMap(bool)),
		minimap(), SLOT(slotShowMap(bool)));

 minimap()->slotShowMap(localPlayer()->hasMiniMap());
}

BosonCommandFrameBase* BosonWidget::createCommandFrame(QWidget* parent)
{
 BosonCommandFrame* frame = new BosonCommandFrame(parent);
 connect(boGame, SIGNAL(signalUpdateProduction(Unit*)),
		frame, SLOT(slotUpdateProduction(Unit*)));
 connect(boGame, SIGNAL(signalUpdateProductionOptions()),
		frame, SLOT(slotUpdateProductionOptions()));

 //AB: can we use the same input for the editor?
 d->mCmdInput = new CommandInput;
 d->mCmdInput->setCommandFrame(frame);
 return frame;
}

void BosonWidget::slotChangeCursor(int mode, const QString& cursorDir_)
{
 kdDebug() << k_funcinfo << endl;
 if (!boGame->gameMode()) {
	// editor mode
	mode = CursorKDE;
 }
 BosonCursor* b;
 switch (mode) {
	case CursorOpenGL:
		b = new BosonOpenGLCursor;
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
	if (!cursor() && mode != CursorKDE) { // loading *never* fails for CursorKDE. we check here anyway.
		// load fallback cursor
		slotChangeCursor(CursorKDE, QString::null);
		return;
	}
	// continue to use the old cursor
	return;
 }
 changeCursor(b);

 mCursorTheme = cursorDir;
}

void BosonWidget::slotStartScenario()
{
 BosonWidgetBase::slotStartScenario();
 boMusic->startLoop();
}

void BosonWidget::slotGamePreferences()
{
 CursorMode mode;
 if (cursor()) {
	if (cursor()->isA("BosonOpenGLCursor")) {
		mode = CursorOpenGL;
	} else if (cursor()->isA("BosonKDECursor")) {
		mode = CursorKDE;
	} else {
		mode = CursorNormal;
	}
 } else {
	mode = CursorNormal;
 }

 OptionsDialog* dlg = new OptionsDialog(this);
 dlg->setGame(boGame);
 dlg->setPlayer(localPlayer());
 dlg->slotLoad();

 connect(dlg, SIGNAL(finished()), dlg, SLOT(slotDelayedDestruct())); // seems not to be called if you quit with "cancel"!
 dlg->setCursor(mode);

 connect(dlg, SIGNAL(signalCursorChanged(int, const QString&)),
		this, SLOT(slotChangeCursor(int, const QString&)));
 connect(dlg, SIGNAL(signalCmdBackgroundChanged(const QString&)),
		this, SLOT(slotCmdBackgroundChanged(const QString&)));
 connect(dlg, SIGNAL(signalMiniMapScaleChanged(double)),
		this, SLOT(slotMiniMapScaleChanged(double)));
 connect(dlg, SIGNAL(signalUpdateIntervalChanged(unsigned int)),
		displayManager(), SLOT(slotUpdateIntervalChanged(unsigned int)));

 dlg->show();
}

void BosonWidget::slotOutOfGame(Player* p)
{
 // TODO write BosonGameOverWidget, add it to widgetstack in TopWidget and then
 //  use it instead
 int inGame = 0;
 Player* winner = 0;
 for (unsigned int i = 0; i < boGame->playerList()->count(); i++) {
	if (!((Player*)boGame->playerList()->at(i))->isOutOfGame()) {
		winner = (Player*)boGame->playerList()->at(i);
		inGame++;
	}
 }
 if (inGame <= 1 && winner) {
	kdDebug() << k_funcinfo << "We have a winner! id=" << winner->id() << endl;
	delete d->mGameOverDialog;
	d->mGameOverDialog = new GameOverDialog(this);
	d->mGameOverDialog->createStatistics(boGame, winner, localPlayer());
	d->mGameOverDialog->show();
	connect(d->mGameOverDialog, SIGNAL(finished()), this, SLOT(slotGameOverDialogFinished()));
	boGame->setGameStatus(KGame::End);
 } else if (!winner) {
	kdError() << k_funcinfo << "no player left ?!" << endl;
	return;
 }
}

void BosonWidget::initKActions()
{
 BosonWidgetBase::initKActions();
#if KDE_VERSION < 310 // old kactions
 QString slotSelect = SLOT(slotSelectGroup());
 QString slotCreate = SLOT(slotCreateGroup());
 for (int i = 0; i < 10; i++) {
	QString s = slotSelect;
	s = s.replace(QRegExp("slotSelectGroup"), QString("slotSelectGroup%1").arg(i));
	(void)new KAction(i18n("Select Group %1").arg(i == 0 ? 10 : i), 
			Qt::Key_0 + i, displayManager(), 
			s, actionCollection(),
			QString("select_group_%1").arg(i));
	s = slotCreate;
	s = s.replace(QRegExp("slotCreateGroup"), QString("slotCreateGroup%1").arg(i));
	(void)new KAction(i18n("Create Group %1").arg(i == 0 ? 10 : i), 
			Qt::CTRL + Qt::Key_0 + i, displayManager(), 
			s, actionCollection(),
			QString("create_group_%1").arg(i));
 }
#else
 // KAction supports slots that take integer parameter :-)
 for (int i = 0; i < 10; i++) {
	(void)new KAction(i18n("Select Group %1").arg(i == 0 ? 10 : i), 
			Qt::Key_0 + i, displayManager(), 
			SLOT(slotSelectGroup(int)), actionCollection(),
			QString("select_group {%1}").arg(i));
	(void)new KAction(i18n("Create Group %1").arg(i == 0 ? 10 : i), 
			Qt::CTRL + Qt::Key_0 + i, displayManager(), 
			SLOT(slotSelectGroup(int)), actionCollection(),
			QString("create_group {%1}").arg(i));
 }
#endif
 (void)new KAction(i18n("Center &Home Base"), KShortcut(Qt::Key_H), 
		displayManager(), SLOT(slotCenterHomeBase()), actionCollection(), "game_center_base");
// (void)KStdAction::gameNew(this, SLOT(), actionCollection()); //TODO
 (void)KStdGameAction::save(this, SIGNAL(signalSaveGame()), actionCollection());
// (void)KStdGameAction::pause(this, SLOT(slotPause()), actionCollection());
// (void)KStdGameAction::pause(mBoson, SLOT(slotTogglePause()), d->mGameActions);
 (void)KStdGameAction::end(this, SIGNAL(signalEndGame()), actionCollection());
 (void)KStdGameAction::quit(this, SIGNAL(signalQuit()), actionCollection());

 (void)KStdAction::preferences(this, SLOT(slotGamePreferences()), actionCollection());



}

void BosonWidget::saveConfig()
{
  // note: the game is *not* saved here! just general settings like game speed,
  // player name, ...
 kdDebug() << k_funcinfo << endl;
 if (!boGame) {
	kdError() << k_funcinfo << "NULL game" << endl;
	return;
 }
 if (!localPlayer()) {
	kdError() << k_funcinfo << "NULL local player" << endl;
	return;
 }
 BosonWidgetBase::saveConfig();

 BosonConfig::saveLocalPlayerName(localPlayer()->name());
 BosonConfig::saveGameSpeed(boGame->gameSpeed());
 if (cursor()) {
	if (cursor()->isA("BosonOpenGLCursor")) {
		boConfig->saveCursorMode(CursorOpenGL);
	} else if (cursor()->isA("BosonKDECursor")) {
		boConfig->saveCursorMode(CursorKDE);
	} else {
		boConfig->saveCursorMode(CursorNormal);
	}
 } else {
	boConfig->saveCursorMode(CursorNormal);
 }
 boConfig->saveCursorDir(mCursorTheme);
// boConfig->save(editor); //FIXME
 kdDebug() << k_funcinfo << "done" << endl;
}

void BosonWidget::slotGameOverDialogFinished()
{
 d->mGameOverDialog->delayedDestruct();
 emit signalGameOver();
}

void BosonWidget::setBosonXMLFile()
{
 BosonWidgetBase::setBosonXMLFile();
 setXMLFile("bosonui.rc", true);
}

