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

#include <qobject.h>

class QCanvas;
class KPlayer;
class KGame;
class KGameChat;

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class KGameCanvasChat : public QObject
{
	Q_OBJECT
public:
	KGameCanvasChat(QObject* parent);
	~KGameCanvasChat();

	void setChat(KGameChat* chat);

	QCanvas* canvas() const { return mCanvas; }
	void setCanvas(QCanvas* c) { mCanvas = c; }

	/**
	 * Maximal displayed messages. use -1 for unlimited (<em>very</em> bad
	 * idea).
	 *
	 * Default is 5
	 **/
	void setMaxItems(int max);

	/**
	 * See @ref setMaxItems
	 **/
	int maxItems() const;

	/**
	 * @return The id of the messages produced by KGameCanvasChat. This id
	 * is used by @ref KGame and you should not use it in any other message
	 * to @ref KGame.
	 **/
	int messageId() const;

	void setKGame(KGame* game);
	KGame* game() const;

	/**
	 * Equivalent to addMessage(game()->findPlayer(playerId));
	 **/
	void addMessage(unsigned int playerId, const QString& message);

	/**
	 * Forms a single string from the player name and message. Then calls
	 * @ref addMessage(string)
	 **/
	void addMessage(KPlayer* player, const QString& message);

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

protected slots:
	void slotUnsetKGame();
	void slotReceiveMessage(int msgid, const QByteArray&, Q_UINT32 receiver, Q_UINT32 sender);

private:
	class KGameCanvasChatPrivate;
	KGameCanvasChatPrivate* d;

	QCanvas* mCanvas;
};

#endif
