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
#ifndef BOSONUFOCHAT_H
#define BOSONUFOCHAT_H

#include "boufo/boufo.h"

class BosonGLFont;
class KPlayer;
class KGame;
class QStringList;

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonUfoChat : public BoUfoWidget
{
	Q_OBJECT
public:
	BosonUfoChat();
	~BosonUfoChat();

	/**
	 * @return The id of the messages produced by KGameCanvasChat. This id
	 * is used by @ref KGame and you should not use it in any other message
	 * to @ref KGame.
	 **/
	int messageId() const;

	void setMessageId(int msgid);
	/**
	 * @param msgid Ignored if < 0, otherwise sets the @ref messageId
	 **/
	void setKGame(KGame* game, int msgid = -1);
	KGame* game() const { return mGame; }

	/**
	 * Equivalent to addMessage(game()->findPlayer(playerId));
	 **/
	void addMessage(unsigned int playerId, const QString& message);

	/**
	 * Forms a single string from the player name and message. Then calls
	 * @ref addMessage(string)
	 **/
	void addMessage(KPlayer* player, const QString& message);

	/**
	 * Final addMessage function that gets called by all overloaded
	 * versions. The text message will be displayed without further
	 * modification
	 **/
	virtual void addMessage(const QString& message);

	const QStringList& messages() const;

	virtual void clear();

protected slots:
	void slotUnsetKGame();
	void slotReceiveMessage(int msgid, const QByteArray&, Q_UINT32 receiver, Q_UINT32 sender);

	void slotTimeout();

protected:
	void removeFirstMessage();
	void updateChat();

private:
	class BosonUfoChatPrivate;
	BosonUfoChatPrivate* d;

	KGame* mGame;
	int mMessageId;
};

#endif
