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
template<class T> class QValueList;

#include "../bomath.h"
#include <qstring.h>

class BosonNetworkSynchronizerPrivate;
/**
 * This class is supposed to notice when the network goes out of sync. @ref
 * receiveAdvanceMessage is called whenever an advance message is received. It
 * collects several data about the current game, and if we are the ADMIN we send
 * an md5 string over these data out to all clients.
 *
 * The clients will receive the MD5 string sent by the client in @ref
 * receiveNetworkSyncCheck. They compare it to their own data that were saved
 * at the same time (speaking in terms of advance calls/messages of course) and
 * they send an ACK back to the ADMIN, indicating they received the sync
 * message. They also tell the ADMIN whether they are in sync or not.
 *
 * The ADMIN receives the ACK message sent by the clients in @ref
 * receiveNetworkSyncCheckAck. If they are in sync, the story ends here. Otherwise we
 * let the ADMIN know that the network is our of sync and we should do something
 * about it.
 *
 * In the future we might save the entire game once the game is out of sync and
 * send it to the clients, so they can load it again.
 * AB: note that this is now being worked on.
 *
 * @short Class to test if network is in sync, and optionally to sync them.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonNetworkSynchronizer
{
public:
	BosonNetworkSynchronizer();
	~BosonNetworkSynchronizer();

	void setGame(Boson* game);
	void setMessageLogger(BoMessageLogger*);

	void addChatSystemMessage(const QString& msg);

	/**
	 * This method makes sure that SyncCheck message are sent in regular
	 * intervals.
	 * See @ref BosonSyncChecker::receiveAdvanceMessage.
	 **/
	void receiveAdvanceMessage(BosonCanvas* canvas);

	/**
	 * See @ref BosonSyncChecker::receiveNetworkSyncCheck.
	 **/
	bool receiveNetworkSyncCheck(QDataStream& stream);

	/**
	 * See @ref BosonSyncChecker::receiveNetworkSyncCheckAck.
	 **/
	bool receiveNetworkSyncCheckAck(QDataStream& stream, Q_UINT32 sender);

	/**
	 * See @ref BosonNetworkSyncer::receiveNetworkRequestSync
	 **/
	bool receiveNetworkRequestSync(QDataStream& stream);


	/**
	 * See @ref BosonNetworkSyncer::receiveNetworkSync
	 **/
	bool receiveNetworkSync(QDataStream& stream);

	/**
	 * Unlock the game. This reenables message delivery.
	 **/
	bool receiveNetworkSyncUnlockGame(QDataStream& stream);

	/**
	 * This starts the network sync procedure. Note that from this point on,
	 * the synchronizer takes control of several aspects of the game (e.g.
	 * it will pause the game until the sync is done).
	 *
	 * The whole procedure works asynchronously - we first send out a
	 * RequestSync message and then (once it is received) actually start to
	 * sync.
	 *
	 * Syncing may take a lot of time (several seconds probably).
	 **/
	void syncNetwork();

	/**
	 * This sends a complete SyncCheck message, that should check the whole
	 * game (not only parts) for being in sync. Once all clients have sent
	 * an ACK to this SyncCheck message, the game is unlocked using a
	 * network message that is received by @ref
	 * receiveNetworkSyncUnlockGame.
	 **/
	void forceCompleteSyncCheckAndUnlockGame(BosonCanvas* canvas);

	/**
	 * Called by @ref BosonSyncChecker. Do not call manually.
	 **/
	void syncCheckingCompleted(const QValueList<Q_UINT32>& outOfSyncClients);

	bool acceptNetworkTransmission(int msgid) const;

protected:
	void unlockGame();

private:
	BosonNetworkSynchronizerPrivate* d;
	Boson* mGame;
};

class BosonNetworkSyncCheckerPrivate;
/**
 * @internal
 * @short Helper class for @ref BosonNetworkSynchonizer that checks if clients
 * are in sync.
 *
 * This class is responsible for creating SyncCheck messages regulary. @ref
 * receiveAdvanceMessage should be called whenever an advance message is
 * received. This class will decide whether a SyncCheck message should be
 * generated.
 *
 * In certain intervals this class creates a SyncCheck messages and stores it
 * internally. Then it sends out the MD5 sum of that message to all clients.
 * When they receive it in @ref receiveNetworkSyncCheck, they send back an ACK
 * message indicating whether their own SyncCheck message matched the MD5 sum
 * that they received. If not, the ACK also contains the whole SyncCheck log
 * message for further investigation.
 *
 * The ACK messages from the clients are received in @ref
 * receiveNetworkSyncCheckAck by the ADMIN. If the ACK indicates that the client
 * is not in sync anymore, the method tries to find the reason for the error
 * (for debugging purposes as well as for bug reporting).
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonNetworkSyncChecker
{
public:
	BosonNetworkSyncChecker();
	~BosonNetworkSyncChecker();
	void setParent(BosonNetworkSynchronizer* parent) { mParent = parent; }


	void setGame(Boson* game) { mGame = game; }
	void setMessageLogger(BoMessageLogger* logger) { mMessageLogger = logger; }

	/**
	 * @return TRUE if there are logs queued that need to be ACKed. They are
	 * ACKed as soon as the ADMIN sends the SyncCheck message (see @ref
	 * receiveNetworkSyncCheck)
	 **/
	bool hasLogs() const;
	void clearLogs();

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
	void forceCompleteSyncCheck(BosonCanvas* canvas);

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
	bool receiveNetworkSyncCheck(QDataStream& stream);

	/**
	 * Must be called by ADMIN only. A ACK message is sent by the clients in
	 * response to our Network Sync message. This ACK tells us whether the
	 * network is in sync (i.e. everything fine) or not (big trouble).
	 *
	 * @return TRUE if the client is in sync, otherwise FALSE.
	 **/
	bool receiveNetworkSyncCheckAck(QDataStream& stream, Q_UINT32 sender);

protected:
	void storeLogAndSend(const QByteArray& log);
	void sendAck(const QCString& expectedString, bool verify, unsigned int syncId, const QByteArray& origLog);

	QByteArray createLongSyncCheckLog(BosonCanvas* canvas, unsigned int advanceMessageCounter, unsigned int interval) const;
	QByteArray createCompleteSyncCheckLog(BosonCanvas* canvas) const;

	void addChatSystemMessage(const QString& msg)
	{
		if (mParent) {
			mParent->addChatSystemMessage(msg);
		}
	}

private:
	BosonNetworkSyncCheckerPrivate* d;

	BosonNetworkSynchronizer* mParent;
	unsigned int mAdvanceMessageCounter;
	unsigned int mSyncId;
	Boson* mGame;
	BoMessageLogger* mMessageLogger;
};

/**
 * @internal
 * @short Helper class for @ref BosonNetworkSynchonizer. This tries to sync a
 * broken game.
 *
 * This class provides methods to fix a game that is out-of-sync. When such a
 * game is detected (see @ref BosonNetworkSyncChecker), the game should be
 * paused (remember to stop sending advance messages _immediately_ even before
 * the game is actually paused) and then a request for a sync should be sent to
 * the network. Once the message is received it is taken to @ref
 * receiveNetworkRequestSync. That method tries to save the game and sends it to
 * all clients (that is the sync message).
 *
 * When the sync message is received, the client loads the game from that message
 * (see @ref receiveNetworkSync).
 *
 *
 * Note that when the the sync request message is received the game should be
 * completely locked. This means it should be paused and message delivery should
 * be completely disabled, except for certain sync related messages. Every
 * incoming message has to be deleted without ever processing it. Only after
 * @ref receiveNetworkSync has been completed the game can be unlocked again.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonNetworkSyncer
{
public:
	BosonNetworkSyncer();
	~BosonNetworkSyncer();
	void setParent(BosonNetworkSynchronizer* parent) { mParent = parent; }

	void setGame(Boson* game) { mGame = game; }

	/**
	 * Send messages to all clients that are to supposed to make the
	 * clients in sync again.
	 *
	 * The game is supposed to be paused when this is called and the message
	 * queue should be empty.
	 *
	 * WARNING: the message queue has to remain empty until the sync message
	 * is received! No non-sync related messages may be delivered!
	 **/
	bool receiveNetworkRequestSync(QDataStream& stream);

	/**
	 * Try to load the game from the stream.
	 *
	 * As soon as this method has completed its work, the game can be
	 * unlocked again.
	 * @return FALSE if the game could not be loaded (i.e. could not be
	 * synced), or TRUE if syncing succeeded.
	 **/
	bool receiveNetworkSync(QDataStream& stream);

	void setGameLocked(bool l);
	bool gameLocked() const { return mGameLocked; }

	void addChatSystemMessage(const QString& msg)
	{
		if (mParent) {
			mParent->addChatSystemMessage(msg);
		}
	}

protected:
	/**
	 * Save the game
	 **/
	QByteArray createSyncMessage();

private:
	BosonNetworkSynchronizer* mParent;
	Boson* mGame;
	bool mGameLocked;
};

#endif
