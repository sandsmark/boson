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

#include "bosoncanvaschat.h"

#ifdef NO_OPENGL

#include <qcanvas.h>

#include <klocale.h>
#include <kgame/kgame.h>
#include <kgame/kplayer.h>
#include <kgame/kgamechat.h>

#include "bosoncanvaschat.moc"

class BosonCanvasChat::BosonCanvasChatPrivate
{
public:
	BosonCanvasChatPrivate()
	{
	}

	int mZ;
	int mX;
	int mY;
	bool mCanSend;

	QPtrList<QCanvasText> mMessages;
};


BosonCanvasChat::BosonCanvasChat(QObject* parent) : BosonChat(parent)
{
 d = new BosonCanvasChatPrivate;

 mCanvas = 0;

 d->mMessages.setAutoDelete(true);
 move(0, 0);
 d->mZ = 0;
}

BosonCanvasChat::~BosonCanvasChat()
{
 clear();
 delete d;
}

void BosonCanvasChat::setZ(int z)
{
 d->mZ = z;
 QPtrListIterator<QCanvasText> it(d->mMessages);
 while (it.current()) {
	it.current()->setZ(d->mZ);
	++it;
 }
}

int BosonCanvasChat::z() const
{
 return d->mZ;
}

void BosonCanvasChat::addMessage(const QString& text)
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

void BosonCanvasChat::clear()
{
 d->mMessages.clear();
}

void BosonCanvasChat::move(int x, int y)
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
