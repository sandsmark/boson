/*
    This file is part of the Boson game
    Copyright (C) 2004 Andreas Beckermann (b_mann@gmx.de)

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

#ifndef BOSONNETWORKSYNCHRONIZER_H
#define BOSONNETWORKSYNCHRONIZER_H

class QDataStream;
class Player;
class Boson;
class Unit;
class BosonCanvas;
class BoMessageLogger;

#include "bomath.h"
#include <qstring.h>

class BosonNetworkSynchronizerPrivate;
/**
 * This class is supposed to notice when the network goes out of sync. @ref
 * receiveAdvanceMessage is called whenever an advance message is received. It
 * collects several data about the current game, and if we are the ADMIN we send
 * an md5 string over these data out to all clients.
 *
 * The clients will receive the MD5 string sent by the client in @ref
 * receiveNetworkSyncMessage. They compare it to their own data that were saved
 * at the same time (speaking in terms of advance calls/messages of course) and
 * they send an ACK back to the ADMIN, indicating they received the sync
 * message. They also tell the ADMIN whether they are in sync or not.
 *
 * The ADMIN receives the ACK message sent by the clients in @ref
 * receiveNetworkSyncAck. If they are in sync, the story ends here. Otherwise we
 * let the ADMIN know that the network is our of sync and we should do something
 * about it.
 *
 * In the future we might save the entire game once the game is out of sync and
 * send it to the clients, so they can load it again.
 * @short Class to test if network is in sync.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonNetworkSynchronizer
{
public:
	BosonNetworkSynchronizer();
	~BosonNetworkSynchronizer();

	/**
	 * When @ref Boson receives an advance message we store some network
	 * synchronization data and (if we are the ADMIN) send that data out to
	 * all clients.
	 *
	 * Of course we don't need to send/store the data on _all_ advance
	 * messages, so we ignore all calls to this method, except of every n-th
	 * call.
	 **/
	void receiveAdvanceMessage(BosonCanvas* canvas);

	/**
	 * This is called for the message that was sent out by the ADMIN in @ref
	 * receiveAdvanceMessage. Here we check whether the contents of the
	 * message match the expected contents (i.e. network is in sync) or not
	 * (network is out of sync - big trouble).
	 *
	 * In any case we send an ACK mesage back to the ADMIN to tell him we
	 * received and processed the message. That ACK also contains whether we
	 * are in sync or not.
	 * @return TRUE if everything is O.k. (i.e. network is in sync),
	 * otherwise FALSE (network out of sync).
	 **/
	bool receiveNetworkSyncMessage(QDataStream& stream);

	/**
	 * Must be called by ADMIN only. A ACK message is sent by the clients in
	 * response to our Network Sync message. This ACK tells us whether the
	 * network is in sync (i.e. everything fine) or not (big trouble).
	 **/
	bool receiveNetworkSyncAck(QDataStream& stream, Q_UINT32 sender);

	void setGame(Boson* game);
	void setMessageLogger(BoMessageLogger*);

protected:
	void storeLogAndSend(const QByteArray& log);
	void sendAck(const QCString& expectedString, bool verify, unsigned int syncId, const QByteArray& origLog);
	QByteArray createLongSyncLog(BosonCanvas* canvas, unsigned int advanceMessageCounter, unsigned int interval) const;

	void addChatSystemMessage(const QString& msg);

private:
	BosonNetworkSynchronizerPrivate* d;
	unsigned int mAdvanceMessageCounter;
	unsigned int mSyncId;
	Boson* mGame;
	BoMessageLogger* mMessageLogger;
};

#endif
