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

#include "kgamecanvaschat.h"

#ifdef NO_OPENGL

#include <qcanvas.h>

#include <klocale.h>
#include <kgame/kgame.h>
#include <kgame/kplayer.h>
#include <kgame/kgamechat.h>

#include "kgamecanvaschat.moc"

class KGameCanvasChat::KGameCanvasChatPrivate
{
public:
	KGameCanvasChatPrivate()
	{
		mGame = 0;
		mChat = 0;
	}

	int mMaxItems;
	int mZ;
	int mX;
	int mY;
	bool mCanSend;


	QPtrList<QCanvasText> mMessages;

	KGame* mGame;
	KGameChat* mChat;
};


KGameCanvasChat::KGameCanvasChat(QObject* parent) : QObject(parent)
{
 d = new KGameCanvasChatPrivate;

 mCanvas = 0;

 d->mMessages.setAutoDelete(true);
 setMaxItems(5);
 move(0, 0);
}

KGameCanvasChat::~KGameCanvasChat()
{
 clear();
 delete d;
}

void KGameCanvasChat::setChat(KGameChat* chat)
{
 d->mChat = chat;
 if (!d->mChat) {
	return;
 }
 if (!d->mChat->game()) {
	kdError() << k_funcinfo << "oops! the chat widget has no KGame!" << endl;
	return;
 }
 setKGame(d->mChat->game());
}

void KGameCanvasChat::setMaxItems(int max)
{
 d->mMaxItems = max;
}

int KGameCanvasChat::maxItems() const
{
 return d->mMaxItems;
}

int KGameCanvasChat::messageId() const
{
 if (d->mChat) {
	return d->mChat->messageId();
 }
 return -1;
}

void KGameCanvasChat::setZ(int z)
{
 d->mZ = z;
 QPtrListIterator<QCanvasText> it(d->mMessages);
 while (it.current()) {
	it.current()->setZ(d->mZ);
	++it;
 }
}

int KGameCanvasChat::z() const
{
 return d->mZ;
}

void KGameCanvasChat::setKGame(KGame* g)
{
 if (g == d->mGame) {
	return;
 }
 if (d->mGame) {
	slotUnsetKGame();
 }
 d->mGame = g;
 if (!d->mGame) {
	return;
 }
 connect(d->mGame, SIGNAL(destroyed()), this, SLOT(slotUnsetKGame()));
 connect(d->mGame, SIGNAL(signalNetworkData(int, const QByteArray&, Q_UINT32, Q_UINT32)),
		this, SLOT(slotReceiveMessage(int, const QByteArray&, Q_UINT32, Q_UINT32)));
}

void KGameCanvasChat::slotUnsetKGame()
{
 if (!d->mGame) {
	return;
 }
 disconnect(d->mGame, 0, this, 0);
}

KGame* KGameCanvasChat::game() const
{
 return d->mGame;
}

void KGameCanvasChat::slotReceiveMessage(int msgid, const QByteArray& buffer, Q_UINT32, Q_UINT32 sender)
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

void KGameCanvasChat::addMessage(unsigned int p, const QString& text)
{
 if (!game()) {
	kdError() << k_funcinfo << "Set a KGame first" << endl;
	return;
 }
 addMessage(game()->findPlayer(p), text);
}

void KGameCanvasChat::addMessage(KPlayer* p, const QString& text)
{
 if (!p) {
	kdError() << k_funcinfo << "NULL player" << endl;
	return;
 }
 if (!d->mChat) {
	kdError() << k_funcinfo << "NULL chat" << endl;
 }
 addMessage(i18n("%1: %2").arg(p->name()).arg(text));
}

void KGameCanvasChat::addMessage(const QString& text)
{
 QCanvasText* t = new QCanvasText(text, canvas());
 t->setZ(Z_CANVASTEXT);
 t->setColor(DEFAULT_CANVASTEXT_COLOR);
 if (maxItems() >= 0 && d->mMessages.count() + 1 > (unsigned int)maxItems()) {
	d->mMessages.removeLast();
 }
 d->mMessages.insert(0, t);
 move(d->mX, d->mY);
 t->setVisible(true);
}

void KGameCanvasChat::clear()
{
 d->mMessages.clear();
}

void KGameCanvasChat::move(int x, int y)
{
 d->mX = x;
 d->mY = y;
 QPtrListIterator<QCanvasText> it(d->mMessages);
 while (it.current()) {
	y -= it.current()->boundingRect().height();
	it.current()->move(x, y);
	y -= 5;
	++it;
 }
}

#endif // NO_OPENGL
