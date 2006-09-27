/*
    This file is part of the Boson game
    Copyright (C) 2006 Andreas Beckermann (b_mann@gmx.de)

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

#ifndef BOSONNETWORKTRAFFIC_H
#define BOSONNETWORKTRAFFIC_H

#include <qobject.h>
#include <qdatetime.h>

class Boson;
template<class T> class QPtrList;

/**
 * @short Details about network traffic caused by a single message.
 *
 * See @ref BosonNetworkTraffic::messageDetails
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonNetworkTrafficDetails
{
public:
	BosonNetworkTrafficDetails(bool sent, Q_UINT32 bytes, int msgid, int usermsgid, Q_UINT32 receiver, Q_UINT32 sender)
	{
		mSent = sent;
		mBytes = bytes;
		mMsgid = msgid;
		mUserMsgid = usermsgid;
		mReceiver = receiver;
		mSender = sender;
		mTimeStamp = QTime::currentTime();
	}

	bool sentMessage() const
	{
		return mSent;
	}
	bool receivedMessage() const
	{
		return !sentMessage();
	}
	Q_UINT32 bytes() const
	{
		return mBytes;
	}
	int msgid() const
	{
		return mMsgid;
	}
	int userMsgid() const
	{
		return mUserMsgid;
	}
	Q_UINT32 receiver() const
	{
		return mReceiver;
	}
	Q_UINT32 sender() const
	{
		return mSender;
	}
	const QTime& timeStamp() const
	{
		return mTimeStamp;
	}
private:
	bool mSent;
	Q_UINT32 mBytes;
	int mMsgid;
	int mUserMsgid;
	Q_UINT32 mReceiver;
	Q_UINT32 mSender;
	QTime mTimeStamp;
};

/**
 * @short This class provides information about traffic caused by a specific
 * message ID.
 *
 * See @ref BosonNetworkTraffic::statistics
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonNetworkTrafficStatistics
{
public:
	BosonNetworkTrafficStatistics(int msgid, int usermsgid)
	{
		mMsgid = msgid;
		mUserMsgid = usermsgid;

		mTotalBytesSent = 0;
		mTotalBytesReceived = 0;
		mMessagesSent = 0;
		mMessagesReceived = 0;
	}

	int msgid() const
	{
		return mMsgid;
	}
	int userMsgid() const
	{
		return mUserMsgid;
	}

	void addMessage(BosonNetworkTrafficDetails* message);
	void clear();

	long long totalBytesSent() const
	{
		return mTotalBytesSent;
	}
	long long totalBytesReceived() const
	{
		return mTotalBytesReceived;
	}

	unsigned int messagesSent() const
	{
		return mMessagesSent;
	}

	unsigned int messagesReceived() const
	{
		return mMessagesReceived;
	}

private:
	int mMsgid;
	int mUserMsgid;
	long long mTotalBytesSent;
	long long mTotalBytesReceived;
	unsigned int mMessagesSent;
	unsigned int mMessagesReceived;
};

class BosonNetworkTrafficPrivate;
/**
 * @short Collects and provides information about network traffic.
 *
 * Note that the "network" traffic is not 100% accurate: all kinds of traffic
 * caused by @ref KGame::sendSystemMessage are collected, even if a message is
 * sent to a local receiver only.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonNetworkTraffic : public QObject
{
	Q_OBJECT
public:
	BosonNetworkTraffic(QObject* parent);
	~BosonNetworkTraffic();

	void setBoson(Boson* b);

	void clear();
	void clearMessageDetails();
	void clearStatistics();

	/**
	 * Keep message details for at least @p seconds seconds. Message details
	 * older than this can get deleted.
	 *
	 * The total values (e.g. @ref totalBytesReceived and @ref
	 * totalBytesSent) are not influenced by this.
	 **/
	void setKeepMessageDetailsFor(Q_UINT32 seconds);

	long long totalBytesReceived() const;
	long long totalBytesSent() const;

	/**
	 * Amount of bytes received by messages internal to KGame.
	 **/
	long long totalKGameBytesReceived() const;
	/**
	 * Amount of bytes sent by messages internal to KGame.
	 **/
	long long totalKGameBytesSent() const;
	/**
	 * Amount of bytes received by Boson messages.
	 **/
	long long totalBosonBytesReceived() const;
	/**
	 * Amount of bytes sent by Boson messages.
	 **/
	long long totalBosonBytesSent() const;

	const QPtrList<BosonNetworkTrafficDetails>& messageDetails() const;
	const QPtrList<BosonNetworkTrafficStatistics>& statistics() const;

protected slots:
	void slotSendBytes(Q_UINT32 bytes, int msgid, int usermsgid, Q_UINT32 sender, Q_UINT32 receiver);
	void slotReceiveBytes(Q_UINT32 bytes, int msgid, int usermsgid, Q_UINT32 sender, Q_UINT32 receiver);

protected:
	void removeOldMessages();
	void addToStatistics(BosonNetworkTrafficDetails* message);

private:
	BosonNetworkTrafficPrivate* d;
	Boson* mBoson;
};

#endif
