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

#include "bosonnetworktraffic.h"
#include "bosonnetworktraffic.moc"

#include "../bomemory/bodummymemory.h"
#include "boson.h"
#include "bodebug.h"

#include <qptrlist.h>
#include <qmap.h>

void BosonNetworkTrafficStatistics::addMessage(BosonNetworkTrafficDetails* message)
{
 if (!message) {
	return;
 }
 if (message->msgid() != msgid()) {
	return;
 }
 if (message->sentMessage()) {
	mMessagesSent++;
	mTotalBytesSent += message->bytes();
 } else {
	mMessagesReceived++;
	mTotalBytesReceived += message->bytes();
 }
}

void BosonNetworkTrafficStatistics::clear()
{
 mTotalBytesSent = 0;
 mTotalBytesReceived = 0;
 mMessagesSent = 0;
 mMessagesReceived = 0;
}

class BosonNetworkTrafficPrivate
{
public:
	BosonNetworkTrafficPrivate()
	{
		mTotalBytesSent = 0;
		mTotalBytesReceived = 0;

		mTotalKGameBytesSent = 0;
		mTotalKGameBytesReceived = 0;
		mTotalBosonBytesSent = 0;
		mTotalBosonBytesReceived = 0;

		mKeepMessageDetailsSeconds = 0;
	}

	long long mTotalBytesSent;
	long long mTotalBytesReceived;

	long long mTotalKGameBytesSent;
	long long mTotalKGameBytesReceived;
	long long mTotalBosonBytesSent;
	long long mTotalBosonBytesReceived;

	int mKeepMessageDetailsSeconds;
	QPtrList<BosonNetworkTrafficDetails> mMessageDetails;

	QPtrList<BosonNetworkTrafficStatistics> mStatistics;
	QMap<int, BosonNetworkTrafficStatistics*> mMsgid2Statistics;
};

BosonNetworkTraffic::BosonNetworkTraffic(QObject* parent)
	: QObject(parent)
{
 d = new BosonNetworkTrafficPrivate;
 mBoson = 0;

 // keep message details for 30 minutes by default
 setKeepMessageDetailsFor(60 * 30);
}

BosonNetworkTraffic::~BosonNetworkTraffic()
{
 clear();
 d->mMessageDetails.setAutoDelete(true);
 d->mMessageDetails.clear();
 d->mStatistics.setAutoDelete(true);
 d->mStatistics.clear();
 d->mMsgid2Statistics.clear();
 delete d;
}

void BosonNetworkTraffic::setBoson(Boson* b)
{
 if (mBoson) {
	disconnect(mBoson, 0, this, 0);
 }
 mBoson = b;
 clear();
 if (mBoson) {
	connect(mBoson, SIGNAL(signalSendBytes(Q_UINT32, int, int, Q_UINT32, Q_UINT32)),
			this, SLOT(slotSendBytes(Q_UINT32, int, int, Q_UINT32, Q_UINT32)));
	connect(mBoson, SIGNAL(signalReceiveBytes(Q_UINT32, int, int, Q_UINT32, Q_UINT32)),
			this, SLOT(slotReceiveBytes(Q_UINT32, int, int, Q_UINT32, Q_UINT32)));
 }
}

void BosonNetworkTraffic::clear()
{
 d->mTotalBytesReceived = 0;
 d->mTotalBytesSent = 0;
 d->mTotalKGameBytesReceived = 0;
 d->mTotalKGameBytesSent = 0;
 d->mTotalBosonBytesReceived = 0;
 d->mTotalBosonBytesSent = 0;
 clearMessageDetails();
 clearStatistics();
}

void BosonNetworkTraffic::clearMessageDetails()
{
 d->mMessageDetails.setAutoDelete(true);
 d->mMessageDetails.clear();
}

void BosonNetworkTraffic::clearStatistics()
{
 d->mStatistics.setAutoDelete(true);
 d->mStatistics.clear();
 d->mMsgid2Statistics.clear();
}

void BosonNetworkTraffic::slotSendBytes(Q_UINT32 bytes, int msgid, int usermsgid, Q_UINT32 sender, Q_UINT32 receiver)
{
 d->mTotalBytesSent += bytes;
 if (usermsgid < 0) {
	d->mTotalKGameBytesSent += bytes;
 } else {
	d->mTotalBosonBytesSent += bytes;
 }
 BosonNetworkTrafficDetails* details = new BosonNetworkTrafficDetails(true, bytes, msgid, usermsgid, receiver, sender);
 d->mMessageDetails.append(details);
 addToStatistics(details);
 removeOldMessages();
}

void BosonNetworkTraffic::slotReceiveBytes(Q_UINT32 bytes, int msgid, int usermsgid, Q_UINT32 sender, Q_UINT32 receiver)
{
 d->mTotalBytesReceived += bytes;
 if (usermsgid < 0) {
	d->mTotalKGameBytesReceived += bytes;
 } else {
	d->mTotalBosonBytesReceived += bytes;
 }
 BosonNetworkTrafficDetails* details = new BosonNetworkTrafficDetails(false, bytes, msgid, usermsgid, receiver, sender);
 d->mMessageDetails.append(details);
 addToStatistics(details);
 removeOldMessages();
}

void BosonNetworkTraffic::setKeepMessageDetailsFor(Q_UINT32 seconds)
{
 d->mKeepMessageDetailsSeconds = seconds;
}

long long BosonNetworkTraffic::totalBytesReceived() const
{
 return d->mTotalBytesReceived;
}

long long BosonNetworkTraffic::totalBytesSent() const
{
 return d->mTotalBytesSent;
}

long long BosonNetworkTraffic::totalKGameBytesReceived() const
{
 return d->mTotalKGameBytesReceived;
}

long long BosonNetworkTraffic::totalKGameBytesSent() const
{
 return d->mTotalKGameBytesSent;
}

long long BosonNetworkTraffic::totalBosonBytesReceived() const
{
 return d->mTotalBosonBytesReceived;
}

long long BosonNetworkTraffic::totalBosonBytesSent() const
{
 return d->mTotalBosonBytesSent;
}

const QPtrList<BosonNetworkTrafficDetails>& BosonNetworkTraffic::messageDetails() const
{
 return d->mMessageDetails;
}

void BosonNetworkTraffic::removeOldMessages()
{
 BosonNetworkTrafficDetails* details = d->mMessageDetails.getFirst();
 while (details && details->timeStamp().addSecs(d->mKeepMessageDetailsSeconds) < QTime::currentTime()) {
	d->mMessageDetails.take(0);
	delete details;
	details = d->mMessageDetails.getFirst();
 }
}

void BosonNetworkTraffic::addToStatistics(BosonNetworkTrafficDetails* message)
{
 if (!message) {
	return;
 }
 if (!d->mMsgid2Statistics[message->msgid()]) {
	BosonNetworkTrafficStatistics* stat = new BosonNetworkTrafficStatistics(message->msgid(), message->userMsgid());
	d->mStatistics.append(stat);
	d->mMsgid2Statistics.insert(message->msgid(), stat);
 }
 BosonNetworkTrafficStatistics* stat = d->mMsgid2Statistics[message->msgid()];
 BO_CHECK_NULL_RET(stat);
 stat->addMessage(message);
}

const QPtrList<BosonNetworkTrafficStatistics>& BosonNetworkTraffic::statistics() const
{
 return d->mStatistics;
}

