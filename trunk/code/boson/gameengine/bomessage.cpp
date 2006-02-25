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

#include "bomessage.h"

#include "../bomemory/bodummymemory.h"
#include <bodebug.h>
#include "boson.h"
#include "bosonmessageids.h"
#include "boeventloop.h"

#include <kgame/kgamemessage.h>
#include <kgame/kplayer.h>
#include <kgame/kgameproperty.h>
#include <kgame/kgamepropertyhandler.h>

#include <qptrlist.h>
#include <qptrqueue.h>
#include <qbuffer.h>
#include <qapplication.h>

BoMessage::BoMessage(QByteArray& _message, int _msgid, Q_UINT32 _receiver, Q_UINT32 _sender, Q_UINT32 _clientId, unsigned int _advanceCallsCount)
		: byteArray(_message),
		msgid(_msgid),
		receiver(_receiver),
		sender(_sender),
		clientId(_clientId),
		receivedOnAdvanceCallsCount(_advanceCallsCount),
		deliveredOnAdvanceCallsCount(0)
{
 mArrivalTime = QTime::currentTime();
}

BoMessage::BoMessage(QDataStream& stream, int _msgid, Q_UINT32 _receiver, Q_UINT32 _sender, Q_UINT32 _clientId, unsigned int _advanceCallsCount)
		: byteArray(((QBuffer*)stream.device())->readAll()),
		msgid(_msgid),
		receiver(_receiver),
		sender(_sender),
		clientId(_clientId),
		receivedOnAdvanceCallsCount(_advanceCallsCount),
		deliveredOnAdvanceCallsCount(0)
{
 mArrivalTime = QTime::currentTime();
}

void BoMessage::setDelivered()
{
 mDeliveryTime = QTime::currentTime();
}

QString BoMessage::debug(KGame* game)
{
 if (!game) {
	return QString::null;
 }
 QString m = QString("msgid=%1").arg(msgid);
 QString r = QString("receiver=%3").arg(receiver);
 QString s = QString("sender=%2").arg(sender);
 if (KGameMessage::isPlayer(receiver)) {
	KPlayer* p = game->findPlayerByKGameId(receiver);
	if (!p) {
		r += QString("(player cant be found)");
	} else {
		r += QString("(player %1)").arg(p->name());
	}
 } else if (KGameMessage::isGame(receiver)) {
	if (receiver == 0) {
		r += "(broadcast games)";
	} else if (receiver == game->gameId()) {
		r += "(local game)";
	} else {
		r += "(remote game)";
	}
 }

 if (KGameMessage::isPlayer(sender)) {
	KPlayer* p = game->findPlayerByKGameId(receiver);
	if (!p) {
		r += QString("(player cant be found)");
	} else {
		r += QString("(player %1)").arg(p->name());
	}
 } else if (KGameMessage::isGame(sender)) {
	if (sender == 0) {
		r += "(broadcast games)";
	} else if (sender == game->gameId()) {
		s += "(local game)";
	} else {
		s += "(remote game)";
	}
 }

 QString mname = KGameMessage::messageId2Text(msgid);
 if (!mname.isEmpty()) {
	m += QString("(%1)").arg(mname);
 }

 QString d;
 d = m + " " + r + " " + s;

 d += debugMore(game);
 return d;
}

QString BoMessage::debugMore(KGame* game)
{
 QString m;
 QDataStream s(byteArray, IO_ReadOnly);
 if (msgid == KGameMessage::IdGameProperty) {
	int propId;
	KGameMessage::extractPropertyHeader(s, propId);
	if (propId == KGamePropertyBase::IdCommand) {
		m = " is a property command";
		// we could use
		// KGameMessage::extractPropertyCommand() here
		// now
	} else {
		KGamePropertyBase* p;
		p = game->dataHandler()->dict().find(propId);
		if (!p) {
			m = QString(" property %1 can't be found").arg(propId);
		} else {
			m = QString(" property: %1(%2)").arg(propId).arg(game->dataHandler()->propertyName(propId));
		}
	}
 }
 return m;
}



BoMessageDelayer::BoMessageDelayer(Boson* b)
{
 mDelayedMessages = new QPtrQueue<BoMessage>();
 mBoson = b;
 mDelayedMessages->setAutoDelete(true);
 mIsLocked = false;
 mDelayedWaiting = false;
 mAdvanceMessageWaiting = 0;
}

BoMessageDelayer::~BoMessageDelayer()
{
 // remaining messages are not stored anywhere else. they are
 // deleted here.
 mDelayedMessages->clear();
 delete mDelayedMessages;
}

void BoMessageDelayer::lock()
{
 boDebug(300) << k_funcinfo << endl;
 mIsLocked = true;
}

void BoMessageDelayer::unlock()
{
 boDebug(300) << k_funcinfo << endl;
 mIsLocked = false;
 while (!mDelayedMessages->isEmpty() && !mIsLocked) {
	processDelayed();
 }
 if (mDelayedMessages->isEmpty()) {
	mBoson->setLoadFromLogComplete();
 }
}

unsigned int BoMessageDelayer::delayedMessageCount() const
{
 return mDelayedMessages->count();
}

void BoMessageDelayer::clearDelayedMessages()
{
 mDelayedMessages->clear();
 mAdvanceMessageWaiting = 0;
 mDelayedWaiting = false;
}

bool BoMessageDelayer::processMessage(BoMessage* m)
{
 if (!m) {
	return true;
 }
 if (mIsLocked || mDelayedWaiting) {
//	boDebug() << k_funcinfo << "delayed " << m->debug(mBoson) << endl;
	mDelayedMessages->enqueue(m);
	mDelayedWaiting = true;
	switch (m->msgid - KGameMessage::IdUser) {
		case BosonMessageIds::AdvanceN:
			mAdvanceMessageWaiting++;
			if (mAdvanceMessageWaiting > 1) {
				// one advance message waiting is ok, more is
				// not, usually.
				boWarning(300) << k_funcinfo << "advance call " << mBoson->advanceCallsCount() << ": now " << mAdvanceMessageWaiting << " advance messages delayed" << endl;
			}
			((BoEventLoop*)qApp->eventLoop())->setAdvanceMessagesWaiting(mAdvanceMessageWaiting);
			break;
		default:
			break;
	}

	// message delayed.
	return false;
 }
 return true; // not delayed.
}

bool BoMessageDelayer::delay(BoMessage* m)
{
 mDelayedMessages->enqueue(m);
 mDelayedWaiting = true;
 return true;
}

void BoMessageDelayer::processDelayed()
{
 BoMessage* m = mDelayedMessages->dequeue();
 if (!m) {
	boWarning() << k_funcinfo << "no message here" << endl;
	return;
 }
 QDataStream s(m->byteArray, IO_ReadOnly);
 mDelayedWaiting = false;
 switch (m->msgid - KGameMessage::IdUser) {
	case BosonMessageIds::AdvanceN:
//		boWarning(300) << k_funcinfo << "delayed advance msg will be sent!" << endl;
		mAdvanceMessageWaiting--;
		((BoEventLoop*)qApp->eventLoop())->setAdvanceMessagesWaiting(mAdvanceMessageWaiting);
		break;
	default:
		break;
 }
 mBoson->networkTransmission(m);
 mDelayedWaiting = !mDelayedMessages->isEmpty();
}


BoMessageLogger::BoMessageLogger()
{
 mLoggedMessages = new QPtrList<BoMessage>();
 mLoggedMessages->setAutoDelete(true);
}

BoMessageLogger::~BoMessageLogger()
{
 mLoggedMessages->clear();
 delete mLoggedMessages;
}

void BoMessageLogger::append(BoMessage* message)
{
 mLoggedMessages->append(message);
}

bool BoMessageLogger::saveHumanReadableMessageLog(QIODevice* logDevice)
{
 if (!logDevice) {
	BO_NULL_ERROR(logDevice);
	return false;
 }
 if (!logDevice->isOpen()) {
	boError() << k_funcinfo << "device not open" << endl;
	return false;
 }
 QTextStream log(logDevice);
 QPtrListIterator<BoMessage> it(*mLoggedMessages);
 while (it.current()) {
	const BoMessage* m = it.current();
	log << "Msg: " << m->deliveredOnAdvanceCallsCount << "  "
			<< m->msgid << "  "
			<< m->sender << " "
			<< m->receiver << " "
			<< m->clientId << "  ";
	log.writeRawBytes(m->byteArray.data(), m->byteArray.size());
	log << endl;
	++it;
 }
 return true;
}

bool BoMessageLogger::saveMessageLog(QIODevice* logDevice, unsigned int maxCount)
{
 if (!logDevice) {
	BO_NULL_ERROR(logDevice);
	return false;
 }
 if (!logDevice->isOpen()) {
	boError() << k_funcinfo << "device not open" << endl;
	return false;
 }
 QDataStream stream(logDevice);
 stream << (Q_UINT32)mLoggedMessages->count();
 QPtrListIterator<BoMessage> it(*mLoggedMessages);
 if (maxCount > 0 && mLoggedMessages->count() > maxCount) {
	unsigned int diff = mLoggedMessages->count() - maxCount;
	it += diff;
 }
 while (it.current()) {
	const BoMessage* m = it.current();
	stream << (Q_UINT32)m->deliveredOnAdvanceCallsCount;
	stream << (Q_INT32)m->msgid;
	stream << (Q_UINT32)m->sender;
	stream << (Q_UINT32)m->receiver;
	stream << (Q_UINT32)m->clientId;
	stream << m->mArrivalTime;
	stream << m->mDeliveryTime;
	stream << m->byteArray;
	++it;
 }
 return true;
}

bool BoMessageLogger::loadMessageLog(QIODevice* logDevice, QPtrList<BoMessage>* messages)
{
 if (!logDevice) {
	BO_NULL_ERROR(logDevice);
	return false;
 }
 if (!logDevice->isOpen()) {
	boError() << k_funcinfo << "device not open" << endl;
	return false;
 }
 if (!messages) {
	BO_NULL_ERROR(messages);
	return false;
 }
 if (!messages->isEmpty()) {
	boError() << k_funcinfo << "logged messages not empty" << endl;
	return false;
 }
 QDataStream stream(logDevice);
 Q_UINT32 count;
 stream >> count;
 for (unsigned int i = 0; i < count; i++) {
	// AB: we log when the message was delivered _only_
	// -> receiving of the message is not interesting and makes comparing
	// network logs very hard (diffs are useless then)
	Q_UINT32 deliveredOnAdvanceCallsCount;
	Q_INT32 msgid;
	Q_UINT32 sender;
	Q_UINT32 receiver;
	Q_UINT32 clientId;
	QTime arrivalTime;
	QTime deliveryTime;
	QByteArray byteArray;
	stream >> deliveredOnAdvanceCallsCount;
	stream >> msgid;
	stream >> sender;
	stream >> receiver;
	stream >> clientId;
	stream >> arrivalTime;
	stream >> deliveryTime;
	stream >> byteArray;

	BoMessage* m = new BoMessage(byteArray, msgid, receiver, sender, clientId, deliveredOnAdvanceCallsCount);
	m->mArrivalTime = arrivalTime;
	m->mDeliveryTime = deliveryTime;
	messages->append(m);
 }
 return true;
}

