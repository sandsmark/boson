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

#include "bosonnetworksynchronizer.h"

#include "bomessage.h"
#include "boson.h"
#include "bosoncanvas.h"
#include "bosonmessage.h"
#include "bosonprofiling.h"
#include "boitemlist.h"
#include "rtti.h"
#include "items/bosonitem.h"
#include "unit.h"
#include "player.h"
#include <bodebug.h>

#include <kgame/kmessageclient.h>

#include <kmdcodec.h>
#include <klocale.h>

#include <qdatastream.h>
#include <qptrqueue.h>
#include <qptrlist.h>
#include <qbuffer.h>
#include <qvaluelist.h>
#include <qmap.h>
#include <qintdict.h>

class BoAwaitAck
{
public:
	void sendLog(Boson* game, const QByteArray& log, unsigned long int syncId)
	{
		if (!game->isAdmin()) {
			boError(370) << k_funcinfo << "must not be called if not admin!!" << endl;
			return;
		}
		mLog = log;
		KMD5 md5(mLog);
		QByteArray buffer;
		QDataStream stream(buffer, IO_WriteOnly);
		stream << (Q_UINT32)syncId;
		stream << md5.hexDigest();
		game->sendMessage(buffer, BosonMessage::IdNetworkSync);
		mClientsLeft = game->messageClient()->clientList();
		return;
	}
	bool receiveAck(Q_UINT32 sender)
	{
		mClientsLeft.remove(sender);
		if (mClientsLeft.isEmpty()) {
			return true;
		}
		return false;
	}

	const QByteArray& log() const
	{
		return mLog;
	}
private:
	QByteArray mLog;
	QValueList<Q_UINT32> mClientsLeft;
};

class BoLongSyncMessage
{
public:
	void setMessageLogger(BoMessageLogger* l)
	{
		mMessageLogger = l;
	}
	void setGame(Boson* game)
	{
		mGame = game;
	}

	QByteArray makeLog(BosonCanvas* canvas);
	static QString findError(QDataStream& s1, QDataStream& s2);

protected:
	QByteArray makeCanvasLog(BosonCanvas* canvas);
	static QString findCanvasError(const QByteArray& b1, const QByteArray& b2)
	{
		if (b1.size() == 0 || b2.size() == 0) {
			boError(370) << k_funcinfo << "empty byte array for canvas log" << endl;
		}
		KMD5 md5(b1);
		KMD5 md5_2(b2);
		if (md5.hexDigest() != md5_2.hexDigest()) {
			boWarning(370) << k_funcinfo << "there must be an error in the canvas log!!" << endl;
		} else {
			boDebug(370) << "error is not in the canvas log" << endl;
			return QString::null;
		}
		QDataStream s1(b1, IO_ReadOnly);
		QDataStream s2(b2, IO_ReadOnly);
		Q_UINT32 items;
		Q_UINT32 items2;
		s1 >> items;
		s2 >> items2;
		if (items != items2) {
			return i18n("Different item counts in canvas log: found %1, expected %2").arg(items2).arg(items);
		}
		for (unsigned int i = 0; i < items; i++) {
			QString error = findItemError(s1, s2);
			if (!error.isNull()) {
				return error;
			}
		}
		Q_UINT32 units;
		Q_UINT32 units2;
		s1 >> units;
		s2 >> units2;
		if (units != units2) {
			return i18n("Different unit counts in canvas log: found %1, expected %2").arg(units2).arg(units);
		}
		for (unsigned int i = 0; i < units; i++) {
			QString error = findUnitError(s1, s2);
			if (!error.isNull()) {
				return error;
			}
		}
		return i18n("There is an error in the canvas log (MD5 sums don't match), but it could not be found.");
	}
	static QString findPlayersError(const QByteArray& b1, const QByteArray& b2)
	{
		KMD5 md5(b1);
		KMD5 md5_2(b2);
		if (md5.hexDigest() != md5_2.hexDigest()) {
			boWarning(370) << k_funcinfo << "there must be an error in the players log!!" << endl;
		} else {
			boDebug(370) << "error is not in the players log" << endl;
			return QString::null;
		}
		QDataStream s1(b1, IO_ReadOnly);
		QDataStream s2(b2, IO_ReadOnly);
		Q_UINT32 count, count2;
		s1 >> count;
		s1 >> count2;
		if (count != count2) {
			return i18n("Have players: %1 should be: %2").arg(count2).arg(count);
		}
		for (unsigned int i = 0; i < count; i++) {
			Q_UINT32 minerals, minerals2;
			Q_UINT32 oil, oil2;
			s1 >> minerals;
			s2 >> minerals2;
			s1 >> oil;
			s2 >> oil2;
#define CHECK(x,x2) if (x != x2) { return i18n("Different players in players log: variable %1: found %2, expected %3").arg(#x).arg(x2).arg(x); }
			CHECK(minerals, minerals2);
			CHECK(oil, oil2);
#undef CHECK
		}
		return i18n("There is an error in the players log (MD5 sums don't match), but it could not be found.");
	}

protected:
	static void streamItem(QDataStream& stream, BosonItem* i)
	{
		stream << (Q_UINT32)i->id();
		stream << i->x();
		stream << i->y();
		stream << i->z();
		stream << i->rotation();
		stream << i->xRotation();
		stream << i->yRotation();
	}
	static void unstreamItem(QDataStream& stream, Q_UINT32& id, float& x, float& y, float& z, float& rotation, float& xrotation, float& yrotation)
	{
		stream >> id;
		stream >> x;
		stream >> y;
		stream >> z;
		stream >> rotation;
		stream >> xrotation;
		stream >> yrotation;
	}
	static QString findItemError(QDataStream& s1, QDataStream& s2)
	{
		Q_UINT32 id, id2;
		float x, x2;
		float y, y2;
		float z, z2;
		float rotation, rotation2;
		float xrotation, xrotation2;
		float yrotation, yrotation2;
		unstreamItem(s1, id, x, y, z, rotation, xrotation, yrotation);
		unstreamItem(s2, id2, x2, y2, z2, rotation2, xrotation2, yrotation2);
#define CHECK(x,x2) if (x != x2) { return i18n("Different items in canvas log: variable %1: found %2, expected %3").arg(#x).arg(x2).arg(x); }
		CHECK(x, x2);
		CHECK(y, y2);
		CHECK(z, z2);
		CHECK(rotation, rotation2);
		CHECK(xrotation, xrotation2);
		CHECK(yrotation, yrotation2);
#undef CHECK
		return QString::null;
	}

	static void streamUnit(QDataStream& stream, Unit* u)
	{
		stream << (Q_UINT32)u->id();
		stream << (Q_INT32)u->work();
		stream << (Q_UINT32)u->health();
	}
	static void unstreamUnit(QDataStream& stream, Q_UINT32& id, Q_INT32& work, Q_UINT32& health)
	{
		stream >> id;
		stream >> work;
		stream >> health;
	}

	static QString findUnitError(QDataStream& s1, QDataStream& s2)
	{
		Q_UINT32 id, id2;
		Q_INT32 work, work2;
		Q_UINT32 health, health2;
		unstreamUnit(s1, id, work, health);
		unstreamUnit(s2, id2, work2, health2);
#define CHECK(x,x2) if (x != x2) { return i18n("Different units in canvas log: variable %1: found %2, expected %3").arg(#x).arg(x2).arg(x); }
		CHECK(id, id2);
		CHECK(work, work2);
		CHECK(health, health2);
#undef CHECK
		return QString::null;
	}

private:
	BoMessageLogger* mMessageLogger;
	Boson* mGame;
};

QByteArray BoLongSyncMessage::makeLog(BosonCanvas* canvas)
{
 QMap<QString, QByteArray> streams;
 streams.insert("CanvasStream", makeCanvasLog(canvas));

 QByteArray playersBuffer;
 QDataStream playersStream(playersBuffer, IO_WriteOnly);
 QPtrListIterator<KPlayer> playerIt(*mGame->playerList());
 playersStream << (Q_UINT32)mGame->playerList()->count();
 while (playerIt.current()) {
	Player* p = (Player*)playerIt.current();
	playersStream << (Q_UINT32)p->minerals();
	playersStream << (Q_UINT32)p->oil();
	++playerIt;
 }
 streams.insert("PlayersStream", playersBuffer);

 // AB: we cannot use this yet.
 // the message log may contain old messages as well, from a previous game or
 // so. these would be present for one client only.
 QByteArray messagesBuffer;
#if 0
 QBuffer messagesBuffer2(messagesBuffer);
 messagesBuffer2.open(IO_WriteOnly);
 mMessageLogger->saveMessageLog(&messagesBuffer2, 100);
 stream << messages;
#endif
 streams.insert("MessagesStream", messagesBuffer);

 QByteArray b;
 QDataStream logStream(b, IO_WriteOnly);
 logStream << streams;

 return b;
}

QByteArray BoLongSyncMessage::makeCanvasLog(BosonCanvas* canvas)
{
 BoItemList list;
 BoItemList::Iterator it;
 for (it = canvas->allItems()->begin(); it != canvas->allItems()->end(); ++it) {
	// AB it would be sufficient to stream data of a couple of items only.
	// that would be a lot faster.
	// e.g. only every nth item, where n is rotating, i.e. first log uses
	// items (n=10) 0, 10, 20, ... second log uses items 1, 11, 21, ... and
	// so on. we'd still cover all items but would have to stream a lot data
	// less
	list.append(*it);
 }

 QByteArray buffer;
 QDataStream stream(buffer, IO_WriteOnly);
 stream << (Q_UINT32)list.count();
 unsigned int unitsCount = 0;
 for (it = list.begin(); it != list.end(); ++it) {
	BosonItem* i = *it;
	streamItem(stream, i);
	if (RTTI::isUnit(i->rtti())) {
		unitsCount++;
	}
 }

 stream << (Q_UINT32)unitsCount;
 for (it = list.begin(); it != list.end(); ++it) {
	if (RTTI::isUnit((*it)->rtti())) {
		Unit* u = (Unit*)(*it);
		streamUnit(stream, u);
	}
 }
 return buffer;
}

QString BoLongSyncMessage::findError(QDataStream& s1, QDataStream& s2)
{
 QMap<QString, QByteArray> streams, streams2;
 s1 >> streams;
 s2 >> streams2;

 QString error;
 if (streams.count() != streams2.count()) {
	error = i18n("Different streams count: %1, but should be %2").arg(streams2.count()).arg(streams.count());
	return error;
 }
 error = findCanvasError(streams["CanvasStream"], streams2["CanvasStream"]);
 if (!error.isNull()) {
	return error;
 }
 error = findPlayersError(streams["PlayersStream"], streams2["PlayersStream"]);
 if (!error.isNull()) {
	return error;
 }
 error = i18n("Error is not in canvas or in players. Somwhere else.");

 return error;
}

class BosonNetworkSynchronizerPrivate
{
public:
	BosonNetworkSynchronizerPrivate()
	{
	}
	QPtrQueue<QByteArray> mLogs;
	QIntDict<BoAwaitAck> mAwaitAcks;
};

BosonNetworkSynchronizer::BosonNetworkSynchronizer()
{
 mAdvanceMessageCounter = 0;
 mSyncId = 0;
 mGame = 0;
 mMessageLogger = 0;
 d = new BosonNetworkSynchronizerPrivate;
 d->mLogs.setAutoDelete(true);
 d->mAwaitAcks.setAutoDelete(true);
}

BosonNetworkSynchronizer::~BosonNetworkSynchronizer()
{
 d->mLogs.clear();
 delete d;
}

void BosonNetworkSynchronizer::setGame(Boson* game)
{
 mGame = game;
}

void BosonNetworkSynchronizer::setMessageLogger(BoMessageLogger* logger)
{
 mMessageLogger = logger;
}

void BosonNetworkSynchronizer::receiveAdvanceMessage(BosonCanvas* canvas)
{
 BO_CHECK_NULL_RET(mGame);
 BO_CHECK_NULL_RET(mMessageLogger);
 BO_CHECK_NULL_RET(canvas);
 // a message is sent every 250 ms
 mAdvanceMessageCounter++;
 if (mAdvanceMessageCounter % 10 == 5) { // every 2,5s
	QByteArray log = createLongSyncLog(canvas);
	storeLogAndSend(log);
 }
 if (mAdvanceMessageCounter % 100 == 50) { // every 25s
 }
}

void BosonNetworkSynchronizer::storeLogAndSend(const QByteArray& log)
{
 d->mLogs.enqueue(new QByteArray(log));
 if (mGame->isAdmin()) {
	BoAwaitAck* wait = new BoAwaitAck();
	wait->sendLog(mGame, log, mSyncId);
	d->mAwaitAcks.insert(mSyncId, wait);
	mSyncId++;
 }
}

bool BosonNetworkSynchronizer::receiveNetworkSyncMessage(QDataStream& stream)
{
 if (!mGame) {
	return false;
 }
 if (d->mLogs.isEmpty()) {
	boError(370) << k_funcinfo << "no logs available" << endl;
	return false;
 }
 Q_UINT32 syncId;
 QCString md5String;
 stream >> syncId;
 stream >> md5String;
 if (md5String.isEmpty()) {
	boError(370) << k_funcinfo << "empty md5 string" << endl;
	return false;
 }

 QByteArray* log = d->mLogs.dequeue();
 KMD5 md5(*log);
 bool verify = md5.verify(md5String);

 sendAck(md5String, verify, syncId, *log);

 delete log;
 log = 0;

 if (!verify) {
	boError(370) << k_funcinfo << "md5 strings of logs don't match!" << endl;
	boError(370) << k_funcinfo << "network is out of sync (or we have a sync bug)" << endl;
	addChatSystemMessage(i18n("Network out of sync for this client !!! Big trouble!"));
	addChatSystemMessage(i18n("A message containing more information has been sent to ADMIN for investigation (debugging)"));
	return false;
 }

 boDebug(370) << k_funcinfo << "network sync OK" << endl;
 return true;
}

bool BosonNetworkSynchronizer::receiveNetworkSyncAck(QDataStream& stream, Q_UINT32 sender)
{
 if (!mGame->isAdmin()) {
	boError(370) << k_funcinfo << "Only ADMIN is allowed to call this" << endl;
	return false;
 }
 Q_UINT32 id;
 QCString md5;
 Q_INT8 verify;
 QByteArray brokenLog;
 stream >> id;
 stream >> md5;
 stream >> verify;
 if (!verify) {
	stream >> brokenLog;
 }

 BoAwaitAck* await = d->mAwaitAcks[id];

 if (!verify) {
	boWarning(370) << k_funcinfo << "network out of sync for client " << sender << endl;
	addChatSystemMessage(i18n("Network out of sync for client %1").arg(sender));
	if (await) {
		QDataStream correct(await->log(), IO_ReadOnly);
		QDataStream broken(brokenLog, IO_ReadOnly);
		QString error = BoLongSyncMessage::findError(correct, broken);
		addChatSystemMessage(i18n("Error message: %1").arg(error));
	}
 }

 if (await) {
	if (await->receiveAck(sender)) {
		d->mAwaitAcks.remove(id);
		await = 0;
	}
 } else {
	boWarning(370) << k_funcinfo << "oops - not waiting for ack of " << id << endl;
 }

 return verify;
}

void BosonNetworkSynchronizer::sendAck(const QCString& md5, bool verify, unsigned int syncId, const QByteArray& origLog)
{
 QByteArray buffer;
 QDataStream stream(buffer, IO_WriteOnly);
 stream << (Q_UINT32)syncId;
 stream << md5;
 stream << (Q_INT8)verify;
 if (!verify) {
	stream << origLog;
 }
 mGame->sendMessage(buffer, BosonMessage::IdNetworkSyncACK);
}


QByteArray BosonNetworkSynchronizer::createLongSyncLog(BosonCanvas* canvas) const
{
 static int myProfilingId = boProfiling->requestEventId("CreateLongSyncLog");
 BosonProfiler profiler(myProfilingId);

 BoLongSyncMessage m;
 m.setGame(mGame);
 m.setMessageLogger(mMessageLogger);
 return m.makeLog(canvas);
}

void BosonNetworkSynchronizer::addChatSystemMessage(const QString& msg)
{
 BO_CHECK_NULL_RET(mGame);
 mGame->slotAddChatSystemMessage(i18n("NetworkSync"), msg);
}

