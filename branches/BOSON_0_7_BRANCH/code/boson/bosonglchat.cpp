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

#include "bosonglchat.h"
#include "bosonglchat.moc"

#include "bosonconfig.h"
#include "bosonglfont.h"
#include "bodebug.h"

#include <klocale.h>
#include <kgame/kgame.h>
#include <kgame/kplayer.h>
#include <kgame/kgamechat.h>

#include <qstringlist.h>
#include <qtimer.h>

#include <GL/gl.h>

class BosonGLChat::BosonGLChatPrivate
{
public:
	BosonGLChatPrivate()
	{
	}

	QStringList mMessages;
	QPtrList<unsigned int> mTimes; // how long the messages in mMessages are here already
	QTimer mRemoveTimer;
};


BosonGLChat::BosonGLChat(QObject* parent) : QObject(parent)
{
 d = new BosonGLChatPrivate;
 mChat = 0;
 mGame = 0;
 d->mTimes.setAutoDelete(true);
 connect(&d->mRemoveTimer, SIGNAL(timeout()), this, SLOT(slotTimeout()));
 d->mRemoveTimer.start(1000);
}

BosonGLChat::~BosonGLChat()
{
 clear();
 delete d;
}

void BosonGLChat::setChat(KGameChat* chat)
{
 mChat = chat;
 if (!mChat) {
	return;
 }
 if (!mChat->game()) {
	boError() << k_funcinfo << "oops! the chat widget has no KGame!" << endl;
	return;
 }
 setKGame(mChat->game());
}

int BosonGLChat::messageId() const
{
 if (mChat) {
	return mChat->messageId();
 }
 return -1;
}

void BosonGLChat::setKGame(KGame* g)
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

void BosonGLChat::slotUnsetKGame()
{
 if (!mGame) {
	return;
 }
 disconnect(mGame, 0, this, 0);
}

void BosonGLChat::slotReceiveMessage(int msgid, const QByteArray& buffer, Q_UINT32, Q_UINT32 sender)
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

void BosonGLChat::addMessage(unsigned int p, const QString& text)
{
 if (!game()) {
	boError() << k_funcinfo << "Set a KGame first" << endl;
	return;
 }
 addMessage(game()->findPlayer(p), text);
}

void BosonGLChat::addMessage(KPlayer* p, const QString& text)
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

void BosonGLChat::addMessage(const QString& text)
{
 if (boConfig->chatScreenMaxItems() >= 0 &&
		d->mMessages.count() + 1 > (unsigned int)boConfig->chatScreenMaxItems()) {
	removeFirstMessage();
 }
 d->mMessages.append(text);
 unsigned int* time = new unsigned int;
 *time = 0;
 d->mTimes.append(time);
}

void BosonGLChat::removeFirstMessage()
{
 d->mMessages.remove(d->mMessages.begin());
 d->mTimes.removeFirst();
}

void BosonGLChat::clear()
{
 d->mMessages.clear();
 d->mTimes.clear();
 d->mRemoveTimer.stop();
}

const QStringList& BosonGLChat::messages() const
{
 return d->mMessages;
}

void BosonGLChat::slotTimeout()
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

void BosonGLChat::renderMessages(int x, int y, BosonGLFont* font)
{
// TODO: line break?
 if (!font) {
	boError() << k_funcinfo << "NULL font" << endl;
	return;
 }
 // AB: this is a redundant call, since we use the same in BosonBigDisplayBase::paintGL().
 // but we might support different fonts one day and so we need it anyway.
 // and a single call doesn't hurt anyway :)
 glListBase(font->displayList());

 QStringList::Iterator it = d->mMessages.end();
 --it;
 for (; it != d->mMessages.begin(); --it) {
	glRasterPos2i(x, y);
	glCallLists((*it).length(), GL_UNSIGNED_BYTE, (GLubyte*)(*it).latin1());
	y += font->height();
 }
 glRasterPos2i(x, y);
 glCallLists((*it).length(), GL_UNSIGNED_BYTE, (GLubyte*)(*it).latin1()); // list.begin()
}

