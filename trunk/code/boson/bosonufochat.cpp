/*
    This file is part of the Boson game
    Copyright (C) 2002-2004 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bosonufochat.h"
#include "bosonufochat.moc"

#include "bosonconfig.h"
#include "bodebug.h"

#include <klocale.h>
#include <kgame/kgame.h>
#include <kgame/kplayer.h>
#include <kgame/kgamechat.h>

#include <qstringlist.h>
#include <qtimer.h>

class BosonUfoChat::BosonUfoChatPrivate
{
public:
	BosonUfoChatPrivate()
	{
		mLabel = 0;
	}

	QStringList mMessages;
	QPtrList<unsigned int> mTimes; // how long the messages in mMessages are here already
	QTimer mRemoveTimer;

	BoUfoLabel* mLabel;
};


BosonUfoChat::BosonUfoChat() : BoUfoWidget()
{
 d = new BosonUfoChatPrivate;
 mChat = 0;
 mGame = 0;
 d->mTimes.setAutoDelete(true);
 connect(&d->mRemoveTimer, SIGNAL(timeout()), this, SLOT(slotTimeout()));
 d->mRemoveTimer.start(1000);

 setLayoutClass(UVBoxLayout);
 d->mLabel = new BoUfoLabel();
 addWidget(d->mLabel);
}

BosonUfoChat::~BosonUfoChat()
{
 d->mLabel = 0;
 clear();
 delete d;
}

void BosonUfoChat::setChat(KGameChat* chat)
{
 mChat = chat;
 if (!mChat) {
	setKGame(0);
	return;
 }
 if (!mChat->game()) {
	boError() << k_funcinfo << "oops! the chat widget has no KGame!" << endl;
	return;
 }
 setKGame(mChat->game());
}

int BosonUfoChat::messageId() const
{
 if (mChat) {
	return mChat->messageId();
 }
 return -1;
}

void BosonUfoChat::setKGame(KGame* g)
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

void BosonUfoChat::slotUnsetKGame()
{
 if (!mGame) {
	return;
 }
 disconnect(mGame, 0, this, 0);
}

void BosonUfoChat::slotReceiveMessage(int msgid, const QByteArray& buffer, Q_UINT32, Q_UINT32 sender)
{
 if (!game()) {
	boError() << k_funcinfo << "Set a KGame first" << endl;
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

void BosonUfoChat::addMessage(unsigned int p, const QString& text)
{
 if (!game()) {
	boError() << k_funcinfo << "Set a KGame first" << endl;
	return;
 }
 addMessage(game()->findPlayer(p), text);
}

void BosonUfoChat::addMessage(KPlayer* p, const QString& text)
{
 if (!p) {
	boError() << k_funcinfo << "NULL player" << endl;
	return;
 }
 if (!mChat) {
	boError() << k_funcinfo << "NULL chat" << endl;
 }
 addMessage(i18n("%1: %2").arg(p->name()).arg(text));
}

void BosonUfoChat::addMessage(const QString& text)
{
 if (boConfig->chatScreenMaxItems() == 0) {
	// No messages allowed
	return;
 }
 if (boConfig->chatScreenMaxItems() > 0 &&
		d->mMessages.count() + 1 > (unsigned int)boConfig->chatScreenMaxItems()) {
	removeFirstMessage();
 }
 d->mMessages.append(text);
 unsigned int* time = new unsigned int;
 *time = 0;
 d->mTimes.append(time);
 updateChat();
}

void BosonUfoChat::removeFirstMessage()
{
 d->mMessages.remove(d->mMessages.begin());
 d->mTimes.removeFirst();
 updateChat();
}

void BosonUfoChat::clear()
{
 d->mMessages.clear();
 d->mTimes.clear();
 d->mRemoveTimer.stop();
 updateChat();
}

const QStringList& BosonUfoChat::messages() const
{
 return d->mMessages;
}

void BosonUfoChat::slotTimeout()
{
 if (boConfig->chatScreenRemoveTime() > 0) {
	QPtrListIterator<unsigned int> it(d->mTimes);
	for (; it.current(); ++it) {
		(*it.current())++;
	}

	while (d->mTimes.count() > 0 && *d->mTimes.first() > boConfig->chatScreenRemoveTime()) {
		removeFirstMessage();
	}
 }
}

void BosonUfoChat::updateChat()
{
 if (!d->mLabel) {
	return;
 }
 if (d->mMessages.count() == 0) {
	d->mLabel->setText("");
	return;
 }
 QString text;
 QStringList::Iterator it = d->mMessages.begin();
 for (; it != d->mMessages.end(); ++it) {
	text += *it + QString::fromLatin1("\n");
 }
 d->mLabel->setText(text);
}

