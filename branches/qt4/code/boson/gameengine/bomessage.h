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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef BOMESSAGE_H
#define BOMESSAGE_H

#include <qdatetime.h>
#include <qstring.h>
//Added by qt3to4:
#include <Q3PtrList>

class KGame;
class Boson;
template<class T> class Q3PtrQueue;
template<class T> class Q3PtrList;
class QIODevice;
class BoAdvanceControl;

/**
 * @short Helper class for @ref Boson.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoMessage
{
public:
	QByteArray byteArray;
	int msgid;
	quint32 receiver;
	quint32 sender;
	quint32 clientId;
	unsigned int receivedOnAdvanceCallsCount;
	unsigned int deliveredOnAdvanceCallsCount;
	QTime mArrivalTime;
	QTime mDeliveryTime;

	/**
	 * Construct a new message. The tinmstamp is set to the current time.
	 * @param _advanceCallsCount See @ref Boson::advanceCallsCount. This
	 * parameter is optional as it is informational only (not an actual part
	 * of the message, but handy for debugging/logging).
	 **/
	BoMessage(QByteArray& _message, int _msgid, quint32 _receiver, quint32 _sender, quint32 _clientId, unsigned int _advanceCallsCount = 0);

	BoMessage(QDataStream& stream, int _msgid, quint32 _receiver, quint32 _sender, quint32 _clientId, unsigned int _advanceCallsCount = 0);

	/**
	 * Set the delivery time to the current time. Both, arrival and delivery
	 * time are (nearly) equal, except if a message has been delayed.
	 **/
	void setDelivered();

	QString debug(KGame* game);

	QString debugMore(KGame* game);
};

class BoMessageDelayer
{
public:
	BoMessageDelayer(Boson* b);
	~BoMessageDelayer();

	/**
	 * Lock message delivery. All incoming messages are delayed until @ref
	 * unlock is called.
	 **/
	void lock();

	/**
	 * Unlock message delivery (see @ref lock) and deliver all delayed
	 * messages.
	 **/
	void unlock(BoAdvanceControl* advanceControl);

	bool isLocked() const
	{
		return mIsLocked;
	}

	unsigned int delayedMessageCount() const;

	unsigned int delayedAdvanceMessageCount() const
	{
		return mAdvanceMessageWaiting;
	}

	/**
	 * Just delete all delayed messages. They are completely discarded.
	 **/
	void clearDelayedMessages();

	/**
	 * Process a network message. Call this before @ref
	 * KGame::networkTransmission is executed!
	 *
	 * This will test whether the message should get delayed and, if that is
	 * the case, it will do so. @ref KGame::networkTransmission should not
	 * be called then. Otherwise you can simply proceed and call @ref
	 * KGame::networkTransmission for this message.
	 *
	 * The message @p m is stored in this class if the message is delayed.
	 * That object will be used to deliver it later - do NOT delete the
	 * message if it is delayed!
	 *
	 * @return TRUE if the message can be delivered normally, FALSE if it
	 * got delayed.
	 **/
	bool processMessage(BoMessage* m, BoAdvanceControl* advanceControl);

	// force to delay @p m
	bool delay(BoMessage* m);

protected:
	/**
	 * Process the first delayed message for delivery. Called by @ref unlock
	 * to deliver delayed messages.
	 *
	 * Note that a delayed message might call @ref lock before all delayed
	 * messages have been deliverd! (this happens e.g. when 2 advance
	 * messages got delayed)
	 **/
	void processDelayed(BoAdvanceControl* advanceControl);

private:
	Boson* mBoson;
	Q3PtrQueue<BoMessage>* mDelayedMessages;
	bool mIsLocked;
	bool mDelayedWaiting; // FIXME bad name!
	int mAdvanceMessageWaiting;
};


/**
 * @short This class keeps a log of all messages received by now
 **/
// AB: after a certain time / number of messages we should delete old messages,
// in order to save memory
class BoMessageLogger
{
public:
	BoMessageLogger();
	~BoMessageLogger();

	/**
	 * Append @p message to the log. Note that ownership is taken, i.e. you
	 * must not delete it after calling this!
	 **/
	void append(BoMessage* message);

	bool saveHumanReadableMessageLog(QIODevice* logDevice);
	bool saveMessageLog(QIODevice* logDevice, unsigned int maxCount = 0);
	static bool loadMessageLog(QIODevice* logDevice, Q3PtrList<BoMessage>* messages);

private:
	Q3PtrList<BoMessage>* mLoggedMessages;
};

#endif

