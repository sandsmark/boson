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
#include "defines.h"

#include <qcanvas.h>

#include <klocale.h>
#include <kgame/kgame.h>
#include <kgame/kplayer.h>

#include "kgamecanvaschat.moc"

class KGameCanvasChat::KGameCanvasChatPrivate
{
public:
	KGameCanvasChatPrivate()
	{
		mFromPlayer = 0;
		mGame = 0;
	}

	int mMaxItems;
	int mMessageId;
	int mZ;
	bool mCanSend;

	KGame* mGame;
	KPlayer* mFromPlayer;

	QPtrList<QCanvasText> mMessages;
};


KGameCanvasChat::KGameCanvasChat(QObject* parent, int id) : QObject(parent)
{
 d = new KGameCanvasChatPrivate;

 mCanvas = 0;

 d->mMessages.setAutoDelete(true);
 d->mMessageId = id;
 setMaxItems(5);
 setCanSend(true);
}

KGameCanvasChat::~KGameCanvasChat()
{
 clear();
 delete d;
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
 return d->mMessageId;
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

void KGameCanvasChat::setFromPlayer(KPlayer* player)
{
 d->mFromPlayer = player;
 if (player && !d->mGame) {
	setKGame(player->game());
 }
}

void KGameCanvasChat::setCanSend(bool s)
{
 d->mCanSend = s;
}

bool KGameCanvasChat::canSend() const
{
 return d->mCanSend;
}

void KGameCanvasChat::setKGame(KGame* g)
{
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

KGame* KGameCanvasChat::game() const
{
 if (d->mGame) {
	return d->mGame;
 }
 if (d->mFromPlayer) {
	return d->mFromPlayer->game();
 }
 return 0;
}

void KGameCanvasChat::slotUnsetKGame()
{
 if (!d->mGame) {
	return;
 }
 disconnect(d->mGame, 0, this, 0);
}

void KGameCanvasChat::slotReceiveMessage(int msgid, const QByteArray& buffer, Q_UINT32, Q_UINT32 sender)
{
 if (msgid != messageId()) {
	return;
 }
 kdDebug() << "receive chat message" << endl;
 QDataStream msg(buffer, IO_ReadOnly);
 QString text;
 msg >> text;
 
 addMessage(sender, text);
}

void KGameCanvasChat::addMessage(unsigned int p, const QString& text)
{
 if (!game()) {
	kdError() << k_funcinfo << "must set a KGame first" << endl;
	return;
 }
 addMessage(game()->findPlayer(p), text);
}

//TODO: add non-kplayer addMessage()
void KGameCanvasChat::addMessage(KPlayer* p, const QString& text)
{
 if (!p) {
	kdError() << k_funcinfo << "NULL player" << endl;
	return;
 }
 // TODO: move all existings up/down
 QCanvasText* t = new QCanvasText(i18n("%1: %2").arg(p->name()).arg(text), canvas());
 t->setZ(Z_CANVASTEXT);
 t->setColor(DEFAULT_CANVASTEXT_COLOR);
 t->setVisible(true);
 d->mMessages.append(t);
}

void KGameCanvasChat::sendMessage(const QString& text) // TODO: only to a specific group, only to a specific player
{
 if (!game()) {
	kdError() << k_funcinfo << "NULL game" << endl;
	return;
 }
 if (!d->mFromPlayer) {
	kdError() << k_funcinfo << "NULL from-player" << endl;
	return;
 }
 game()->sendMessage(text, messageId(), 0, d->mFromPlayer->id());
}

void KGameCanvasChat::clear()
{
 d->mMessages.clear();
}

void KGameCanvasChat::move(int x, int y)
{
 QPtrListIterator<QCanvasText> it(d->mMessages);
 while (it.current()) {
	y -= it.current()->boundingRect().height();
	it.current()->move(x, y);
//	kdDebug() << "x=" << x << ",y=" << y << endl;
	y -= 5;
	++it;
 }
}

void KGameCanvasChat::slotAddPlayer(KPlayer* p)
{
}

void KGameCanvasChat::slotRemovePlayer(KPlayer* p)
{
}

