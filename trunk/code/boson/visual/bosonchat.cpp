/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bosonchat.h"
#include "bosonchat.moc"

#include <klocale.h>
#include <kgame/kgame.h>
#include <kgame/kplayer.h>
#include <kgame/kgamechat.h>

#include <qstringlist.h>
#include <qtimer.h>

class BosonChat::BosonChatPrivate
{
public:
	BosonChatPrivate()
	{
	}

	QStringList mMessages;
	QPtrList<unsigned int> mTimes; // how long the messages in mMessages are here already
	QTimer mRemoveTimer;
};


BosonChat::BosonChat(QObject* parent) : QObject(parent)
{
 d = new BosonChatPrivate;
 mChat = 0;
 mGame = 0;
 d->mTimes.setAutoDelete(true);
 connect(&d->mRemoveTimer, SIGNAL(timeout()), this, SLOT(slotTimeout()));

 setMaxItems(5);
 setRemoveTime(10); // 10s is default
}

BosonChat::~BosonChat()
{
 clear();
 delete d;
}

void BosonChat::setChat(KGameChat* chat)
{
 mChat = chat;
 if (!mChat) {
	return;
 }
 if (!mChat->game()) {
	kdError() << k_funcinfo << "oops! the chat widget has no KGame!" << endl;
	return;
 }
 setKGame(mChat->game());
}

void BosonChat::setMaxItems(int max)
{
 mMaxItems = max;
}

int BosonChat::messageId() const
{
 if (mChat) {
	return mChat->messageId();
 }
 return -1;
}

void BosonChat::setKGame(KGame* g)
{
 if (g == mGame) {
	return;
 }
 if (mGame) {
	slotUnsetKGame();
 }
 mGame = g;
 if (!mGame) {
	return;
 }
 connect(mGame, SIGNAL(destroyed()), this, SLOT(slotUnsetKGame()));
 connect(mGame, SIGNAL(signalNetworkData(int, const QByteArray&, Q_UINT32, Q_UINT32)),
		this, SLOT(slotReceiveMessage(int, const QByteArray&, Q_UINT32, Q_UINT32)));
}

void BosonChat::slotUnsetKGame()
{
 if (!mGame) {
	return;
 }
 disconnect(mGame, 0, this, 0);
}

void BosonChat::slotReceiveMessage(int msgid, const QByteArray& buffer, Q_UINT32, Q_UINT32 sender)
{
 if (!game()) {
	kdError() << k_funcinfo << "Set a KGame first" << endl;
	return;
 }
 if (msgid != messageId()) {
	return;
 }
 QDataStream msg(buffer, IO_ReadOnly);
 QString text;
 msg >> text;
 
 addMessage(sender, text);
}

void BosonChat::addMessage(unsigned int p, const QString& text)
{
 if (!game()) {
	kdError() << k_funcinfo << "Set a KGame first" << endl;
	return;
 }
 addMessage(game()->findPlayer(p), text);
}

void BosonChat::addMessage(KPlayer* p, const QString& text)
{
 if (!p) {
	kdError() << k_funcinfo << "NULL player" << endl;
	return;
 }
 if (!mChat) {
	kdError() << k_funcinfo << "NULL chat" << endl;
 }
 addMessage(i18n("%1: %2").arg(p->name()).arg(text));
}

void BosonChat::addMessage(const QString& text)
{
 if (maxItems() >= 0 && d->mMessages.count() + 1 > (unsigned int)maxItems()) {
	removeFirstMessage();
 }
 d->mMessages.append(text);
 unsigned int* time = new unsigned int;
 *time = 0;
 d->mTimes.append(time);
 startTimer();
}

void BosonChat::removeFirstMessage()
{
 d->mMessages.remove(d->mMessages.begin());
 d->mTimes.removeFirst();
 if (d->mMessages.count() == 0) {
	d->mRemoveTimer.stop();
 }
}

void BosonChat::clear()
{
 d->mMessages.clear();
 d->mTimes.clear();
 d->mRemoveTimer.stop();
}

const QStringList& BosonChat::messages() const
{
 return d->mMessages;
}

void BosonChat::startTimer()
{
 if (!d->mRemoveTimer.isActive()) {
	d->mRemoveTimer.start(1000);
 }
}

void BosonChat::setRemoveTime(unsigned int s)
{
 mRemoveTime = s;
 if (removeTime() == 0) {
	d->mRemoveTimer.stop();
 }
}

void BosonChat::slotTimeout()
{
 if (removeTime() == 0) {
	d->mRemoveTimer.stop();
	return;
 }
 if (removeTime() > 0) {
	QPtrListIterator<unsigned int> it(d->mTimes);
	for (; it.current(); ++it) {
		(*it.current())++;
	}

	while (d->mTimes.count() > 0 && *d->mTimes.first() >= removeTime()) {
		removeFirstMessage();
	}
 }
}

