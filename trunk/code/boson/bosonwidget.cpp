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

#include "bosonwidget.h"

#include "defines.h"
#include "bosoncanvas.h"
#include "boson.h"
#include "player.h"
#include "bosonconfig.h"
#include "optionsdialog.h"
#include "bosoncursor.h"
#include "bodisplaymanager.h"
#include "global.h"
#include "gameoverdialog.h"
#include "bodebug.h"
#include "sound/bosonaudiointerface.h"

#include <kstdgameaction.h>
#include <klocale.h>
#include <kaction.h>
#include <kdeversion.h>

#include <qsignalmapper.h>
#include <qtimer.h>

#include "bosonwidget.moc"

class BosonWidget::BosonWidgetPrivate
{
public:
	BosonWidgetPrivate()
	{
		mGameOverDialog = 0;
	}

	GameOverDialog* mGameOverDialog;
};

BosonWidget::BosonWidget(QWidget* parent)
    : BosonWidgetBase(parent)
{
 d = new BosonWidgetPrivate;
}

BosonWidget::~BosonWidget()
{
 boDebug() << k_funcinfo << endl;
 delete d;
 boDebug() << k_funcinfo << "done" << endl;
}

void BosonWidget::initConnections()
{
 BosonWidgetBase::initConnections();
 connect(boGame, SIGNAL(signalPlayerKilled(Player*)),
		this, SLOT(slotPlayerKilled(Player*)));
}

void BosonWidget::initPlayer()
{
 BosonWidgetBase::initPlayer();
 if (!localPlayer()) {
	boError() << k_funcinfo << "NULL local player" << endl;
	return;
 }
}

void BosonWidget::slotChangeCursor(int mode, const QString& cursorDir_)
{
 boDebug() << k_funcinfo << endl;
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
	default:
		b = new BosonKDECursor;
		mode = CursorKDE; // in case we had an unknown/invalid mode
		break;
 }

 QString cursorDir = cursorDir_;
 if (cursorDir.isNull()) {
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
	boError() << k_funcinfo << "Could not load cursor mode " << mode << " from " << cursorDir << endl;
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

 boConfig->setCursorMode(mode);
 boConfig->setCursorDir(cursorDir);
}

void BosonWidget::startScenarioAndGame()
{
 BosonWidgetBase::startScenarioAndGame();
 if (boGame->isAdmin()) {
	if (boGame->gameSpeed() == 0) {
		// don't do this if gameSpeed() != 0, as it was set already
		// (e.g. due to a savegame)
		boGame->slotSetGameSpeed(BosonConfig::readGameSpeed());
	}
 }
 boMusic->startLoop();
}

void BosonWidget::slotGamePreferences()
{
 OptionsDialog* dlg = gamePreferences(false);
 if (!dlg) {
	boError() << k_funcinfo << "NULL options dialog" << endl;
	return;
 }
 dlg->show();
}

void BosonWidget::slotPlayerKilled(Player* p)
{
 // TODO write BosonGameOverWidget, add it to widgetstack in TopWidget and then
 //  use it instead
 int inGame = 0;
 Player* winner = 0;
 for (unsigned int i = 0; i < boGame->playerList()->count() - 1; i++) { // AB: playerList()->count()-1 is the neutral player, it is never the winner
	if (!((Player*)boGame->playerList()->at(i))->isOutOfGame()) {
		winner = (Player*)boGame->playerList()->at(i);
		inGame++;
	}
 }
 if (inGame <= 1 && winner) {
	boDebug() << k_funcinfo << "We have a winner! id=" << winner->id() << endl;
	delete d->mGameOverDialog;
	d->mGameOverDialog = new GameOverDialog(this);
	d->mGameOverDialog->createStatistics(boGame, winner, localPlayer());
	d->mGameOverDialog->show();
	connect(d->mGameOverDialog, SIGNAL(finished()), this, SLOT(slotGameOverDialogFinished()));
	boGame->setGameStatus(KGame::End);
 } else if (!winner) {
	boError() << k_funcinfo << "no player left ?!" << endl;
	return;
 }
}

void BosonWidget::initKActions()
{
 QSignalMapper* selectMapper = new QSignalMapper(this);
 QSignalMapper* createMapper = new QSignalMapper(this);
 connect(selectMapper, SIGNAL(mapped(int)), displayManager(), SLOT(slotSelectGroup(int)));
 connect(createMapper, SIGNAL(mapped(int)), displayManager(), SLOT(slotCreateGroup(int)));

 QString slotSelect = SLOT(slotSelectGroup());
 QString slotCreate = SLOT(slotCreateGroup());
 for (int i = 0; i < 10; i++) {
	KAction* a = new KAction(i18n("Select Group %1").arg(i == 0 ? 10 : i),
			Qt::Key_0 + i, selectMapper,
			SLOT(map()), actionCollection(),
			QString("select_group_%1").arg(i));
	selectMapper->setMapping(a, i);
	a = new KAction(i18n("Create Group %1").arg(i == 0 ? 10 : i),
			Qt::CTRL + Qt::Key_0 + i, createMapper,
			SLOT(map()), actionCollection(),
			QString("create_group_%1").arg(i));
	createMapper->setMapping(a, i);
 }
 (void)KStdGameAction::save(this, SIGNAL(signalSaveGame()), actionCollection());
 (void)KStdGameAction::end(this, SIGNAL(signalEndGame()), actionCollection());
 (void)KStdGameAction::quit(this, SIGNAL(signalQuit()), actionCollection());

 (void)KStdAction::preferences(this, SLOT(slotGamePreferences()), actionCollection());
}

void BosonWidget::saveConfig()
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
 BosonWidgetBase::saveConfig();

 BosonConfig::saveLocalPlayerName(localPlayer()->name());
 BosonConfig::saveGameSpeed(boGame->gameSpeed());
// boConfig->save(editor); //FIXME
 boDebug() << k_funcinfo << "done" << endl;
}

void BosonWidget::slotGameOverDialogFinished()
{
 d->mGameOverDialog->deleteLater();

 // we must not emit directly, as it'd delete the BosonWidget and therefore the
 // GameOverDialog, but that is later deleted through the event loop
 QTimer::singleShot(0, this, SIGNAL(signalGameOver()));
}

void BosonWidget::setBosonXMLFile()
{
 BosonWidgetBase::setBosonXMLFile();
 setXMLFile("bosonui.rc", true);
}

