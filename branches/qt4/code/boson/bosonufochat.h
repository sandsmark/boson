/*
    This file is part of the Boson game
    Copyright (C) 2002-2005 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOSONUFOCHAT_H
#define BOSONUFOCHAT_H

#include "boufo/boufo.h"

class BosonGLFont;
class KPlayer;
class KGame;
class KGamePropertyBase;
class QStringList;

class BosonUfoChatPrivate;
/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonUfoChat : public BoUfoWidget
{
	Q_OBJECT
public:
	BosonUfoChat();
	~BosonUfoChat();

	void setFromPlayer(KPlayer* player);
	KPlayer* fromPlayer() const;

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
	KGame* game() const;

	/**
	 * Equivalent to addMessage(game()->findPlayerByKGameId(playerId));
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


	/**
	 * Insert a sending entry ("send to ...") at position @p index. Use @p id to identify the entry.
	 *
	 * @param The desired unique ID of the sending entry. Use -1 to let this
	 * be assigned automatically. Note that automatically assigned IDs are
	 * always >= 1024, so you can use values below 1024 hardcoded in your code, if desired.
	 **/
	int insertSendingEntry(const QString& text, int id = -1, int index = -1);

	/**
	 * Specialized version of @ref insertSendingEntry
	 **/
	int insertPlayerSendingEntry(const QString& player, int id = -1, int index = -1);

	/**
	 * Specialized version of @ref insertSendingEntry
	 **/
	int insertSendToAllSendingEntry(int id = -1, int index = -1);

	void changeSendingEntry(const QString& text, int id);
	void removeSendingEntry(int id);

	/**
	 * @return The unique ID of the sending entry that was selected (in the
	 * "send to" combobox)
	 **/
	int sendingEntry() const;

	/**
	 * @return TRUE if the @p id describes the "send to my group" entry, otherwise FALSE.
	 *
	 * @param id See also @ref sendingEntry
	 **/
	bool isToMyGroupMessage(int id) const;

	bool isToPlayerMessage(int id) const;

	bool isSendToAllMessage(int id) const;


	void setSendBoxVisible(bool v);

protected slots:
	void slotUnsetKGame();
	void slotReceivePrivateMessage(int msgid, const QByteArray& buffer, Q_UINT32 sender, KPlayer* me);
	void slotReceiveMessage(int msgid, const QByteArray&, Q_UINT32 receiver, Q_UINT32 sender);

	void slotTimeout();
	void slotSendText(const QString& text);

	void slotAddPlayer(KPlayer* p);
	void slotRemovePlayer(KPlayer* p);
	void slotPropertyChanged(KGamePropertyBase*, KPlayer* p);

protected:
	void removeFirstMessage();
	void updateChat();

private:
	BosonUfoChatPrivate* d;
};

#endif
