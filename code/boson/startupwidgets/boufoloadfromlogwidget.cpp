/*
    This file is part of the Boson game
    Copyright (C) 2006 Andreas Beckermann (b_mann@gmx.de)

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "boufoloadfromlogwidget.h"
#include "boufoloadfromlogwidget.moc"

#include "../../bomemory/bodummymemory.h"
#include "../defines.h"
#include "../bosonconfig.h"
#include "../gameengine/player.h"
#include "../gameengine/speciestheme.h"
#include "../gameengine/bosoncomputerio.h"
#include "../gameengine/boson.h"
#include "../gameengine/bosonplayfield.h"
#include "../gameengine/bosonmap.h"
#include "../gameengine/bomessage.h"
#include "../gameengine/bosonmessageids.h"
#include "../bosondata.h"
#include "bosonstartupnetwork.h"
#include "bodebug.h"

#include <klocale.h>
#include <kgame/kgamemessage.h>
#include <kmessagebox.h>

#include <qtimer.h>
#include <qfile.h>
//Added by qt3to4:
#include <Q3PtrList>

class BoUfoLoadFromLogWidgetPrivate
{
public:
	BoUfoLoadFromLogWidgetPrivate()
	{
	}
	Q3PtrList<BoMessage> mLogMessages;
};


BoUfoLoadFromLogWidget::BoUfoLoadFromLogWidget(BosonStartupNetwork* interface)
    : BoUfoLoadFromLogWidgetBase()
{
 boDebug() << k_funcinfo << endl;
 BO_CHECK_NULL_RET(boGame);
 BO_CHECK_NULL_RET(interface);
 d = new BoUfoLoadFromLogWidgetPrivate;
 mNetworkInterface = interface;

 connect(networkInterface(), SIGNAL(signalStartGameClicked()),
		this, SLOT(slotNetStart()));
}

BoUfoLoadFromLogWidget::~BoUfoLoadFromLogWidget()
{
 boDebug() << k_funcinfo << endl;
 d->mLogMessages.setAutoDelete(true);
 d->mLogMessages.clear();

 delete d;
}

void BoUfoLoadFromLogWidget::slotCancel()
{
 // AB: we use a timer, so that the newgame widget can be deleted in the slot
 // (otherwise this would not be allowed, as we are in a pushbutton click)
 QTimer::singleShot(0, this, SIGNAL(signalCancelled()));
}

bool BoUfoLoadFromLogWidget::loadFromLog(const QString& file)
{
 if (!d->mLogMessages.isEmpty()) {
	boError() << k_funcinfo << "still messages from a previous run around! clearing..." << endl;
	d->mLogMessages.setAutoDelete(true);
	d->mLogMessages.clear();
	d->mLogMessages.setAutoDelete(false);
 }
 if (!boGame) {
	BO_NULL_ERROR(boGame);
	return false;
 }
 if (file.isEmpty()) {
	boError() << k_funcinfo << "empty log filename" << endl;
	return false;
 }
 QFile f(file);
 if (!f.open(QIODevice::ReadOnly)) {
	boError() << k_funcinfo << "could not open " << file << " for reading" << endl;
	return false;
 }

 Q3PtrList<BoMessage> messages;
 messages.setAutoDelete(true);
 BoMessageLogger::loadMessageLog(&f, &messages);
 BoMessage* start = 0;
 Q3PtrListIterator<BoMessage> it(messages);
 while (it.current()) {
//	if (it.current()->msgid == KGameMessage::IdUser + BosonMessageIds::IdGameIsStarted) {
	if (it.current()->msgid == KGameMessage::IdUser + BosonMessageIds::IdNewGame) {
		start = it.current();
	}
	++it;
 }
 if (!start) {
	boError() << k_funcinfo << "no IdNewGame message found" << endl;
	return false;
 }

 while (!messages.isEmpty() && messages.getFirst() != start) {
	messages.removeFirst();
 }
 if (messages.getFirst() != start) {
	boError() << k_funcinfo << "oops - something went wrong" << endl;
	return false;
 }

 d->mLogMessages = messages;
 d->mLogMessages.setAutoDelete(false);
 messages.setAutoDelete(false);

 // TODO: add players

 boDebug() << k_funcinfo << "log messages loaded. waiting for game start." << endl;
 return true;
}

void BoUfoLoadFromLogWidget::slotStartGame()
{
 boDebug() << k_funcinfo << endl;

 if (!boGame->isAdmin()) {
	return;
 }

 boWarning() << k_funcinfo << "using hardcoded teamcolor values and default species for all players" << endl;
 int red = 0;
 foreach (Player* p, boGame->gamePlayerList()) {
	QColor c(red, 100, 100);
	networkInterface()->sendChangeSpecies(p, SpeciesTheme::defaultSpecies(), c);
	red += 20;
 }

 networkInterface()->addNeutralPlayer(false);

 // when this message is received, the neutral player has been added.
 networkInterface()->sendStartGameClicked();
}

void BoUfoLoadFromLogWidget::slotNetStart()
{
 // AB: at this point all players (including the neutral player!!) must have
 // been added already

 boDebug() << k_funcinfo << endl;
 BO_CHECK_NULL_RET(boGame);
 if (!boGame->isAdmin()) {
	KMessageBox::sorry(0, i18n("You are not the ADMIN of the game. This is an internal error when loading from log!\nCannot start game"));
	return;
 }
 if (d->mLogMessages.isEmpty()) {
	KMessageBox::sorry(0, i18n("Have no log messages. Cannot load from log."));
	return;
 }
 QTimer::singleShot(0, this, SLOT(slotNetStartLoadingFromLog()));
}

void BoUfoLoadFromLogWidget::slotNetStartLoadingFromLog()
{
 boDebug() << k_funcinfo << "LOADING FROM LOG NOW!" << endl;
 boDebug() << k_funcinfo << d->mLogMessages.count() << endl;
 bool ret = boGame->loadFromLog(&d->mLogMessages);
 if (!ret) {
	KMessageBox::sorry(0, i18n("Loading from log failed"));
	return;
 }
}

