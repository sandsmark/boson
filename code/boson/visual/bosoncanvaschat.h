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
#ifndef KGAMECANVASCHAT_H
#define KGAMECANVASCHAT_H

#include "../defines.h"

#ifdef NO_OPENGL
#include "bosonchat.h"

class QCanvas;
class KPlayer;
class KGame;
class KGameChat;

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonCanvasChat : public BosonChat
{
	Q_OBJECT
public:
	BosonCanvasChat(QObject* parent);
	~BosonCanvasChat();

	QCanvas* canvas() const { return mCanvas; }
	void setCanvas(QCanvas* c) { mCanvas = c; }

	virtual void addMessage(const QString& message);


	void clear();

	/**
	 * @param x The Left side (== x coordinate of all texts)
	 * @param y Currently the lowest possible value. TODO: make configurable
	 * - either the text is coming from above, moving down ; or coming from
	 * below, moving up (current situation).
	 * */
	void move(int x, int y);

	int z() const;
	void setZ(int z);

private:
	class BosonCanvasChatPrivate;
	BosonCanvasChatPrivate* d;

	QCanvas* mCanvas;
};

#endif // !NO_OPENGL

#endif

