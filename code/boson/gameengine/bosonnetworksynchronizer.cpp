/*
    This file is part of the Boson game
    Copyright (C) 2004-2005 Andreas Beckermann (b_mann@gmx.de)

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

#include "bosonnetworksynchronizer.h"

#include "../bomemory/bodummymemory.h"
#include "bomessage.h"
#include "boson.h"
#include "bosoncanvas.h"
#include "bosonmessageids.h"
#include "bosonprofiling.h"
#include "boitemlist.h"
#include "rtti.h"
#include "bosonitem.h"
#include "unit.h"
#include "player.h"
#include "bosonmap.h"
#include "bosonpath.h"
#include "bosonshot.h"
#include <bodebug.h>

#include <kgame/kmessageclient.h>
#include <kgame/kgamemessage.h>

#include <kcodecs.h>
#include <klocale.h>
#include <krandomsequence.h>

#include <qdatastream.h>
#include <q3ptrqueue.h>
#include <q3ptrlist.h>
#include <qbuffer.h>
#include <q3valuelist.h>
#include <qmap.h>
#include <q3intdict.h>
#include <qdom.h>
//Added by qt3to4:
#include <Q3CString>


// debugging code to find a network bug. will be removed soon
#warning remove asap
#define NET_DEBUG 0
#define NET_DEBUG_2 0


// AB: generally I consider macros evil, but it time has shown that they _do_
// make sense here. they reduce the probability of typos _greatly_

// compare name and name2, and give an error message containing both values.
// you need to do this on your own, if QString::arg() is not defined for the
// type of name/name2

 // declare name and name2
#define DECLARE(type, name) type name, name##2;

 // unstream name and name2 (streams must be named "s1" and "s2")
#define UNSTREAM(name) s1 >> name; s2 >> name##2;

// compare name and name2 and add an error to the QString variable "error". note
// that this works only if type of name/name2 is compatible with QString::arg().
#define COMPARE(name) if (name != name##2) { error += i18n("Variables not equal: %1(%2) and %3(%4)\n").arg(#name).arg(name).arg(#name "2").arg(name##2); }
#define COMPAREITEM(name, id, rtti, type) if (name != name##2) { error += i18n("Variables not equal for item %1 (%2/%3): %4(%5) and %6(%7)\n").arg(id).arg(rtti).arg(type).arg(#name).arg(name).arg(#name "2").arg(name##2); }

// convenience macros that combine the above macros.
#define DECLARE_UNSTREAM(type, name) DECLARE(type, name) UNSTREAM(name)
#define DECLARE_UNSTREAM_COMPARE(type, name) DECLARE(type, name) UNSTREAM(name) COMPARE(name)


class BoAwaitAck
{
public:
	BoAwaitAck()
	{
	}

	void sendLog(Boson* game, const QByteArray& log, quint32 syncId)
	{
		if (!game->isAdmin()) {
			boError(370) << k_funcinfo << "must not be called if not admin!!" << endl;
			return;
		}
		mLog = log;
		KMD5 md5(mLog);
		QByteArray buffer;
		QDataStream stream(&buffer, QIODevice::WriteOnly);
		stream << (quint32)syncId;
		stream << md5.hexDigest();
		game->sendMessage(buffer, BosonMessageIds::IdNetworkSyncCheck);
		mClientsLeft = game->messageClient()->clientList();
		return;
	}
	bool receiveAck(quint32 sender)
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

	void addOutOfSyncClient(quint32 client)
	{
		mOutOfSyncClients.append(client);
	}
	Q3ValueList<quint32> outOfSyncClients() const
	{
		return mOutOfSyncClients;
	}

private:
	QByteArray mLog;
	Q3ValueList<quint32> mClientsLeft;
	Q3ValueList<quint32> mOutOfSyncClients;
};


/**
 * Base class for a sync message. A sync message contains a few information
 * about one client that can be used to find out whether two clients are in sync
 * or out of sync. Note that it is <em>not</em> necessary to provide data to fix
 * a client if it is out of sync! This should not be done here!
 *
 * You should try to keep the amount of data very low, so that the sync message
 * can be sent as often as possible without causing performance or bandwidth
 * problems.
 **/
class BoSyncCheckMessageBase
{
public:
	virtual ~BoSyncCheckMessageBase()
	{
	}

	/**
	 * Called to create the sync message.
	 **/
	virtual QByteArray makeLog() = 0;

	/**
	 * When an error was detected, this method is called to find the
	 * difference of two logs. If the error is in this sync message, it
	 * should return a string describing the error. Otherwise it returns
	 * QString::null to indicate that this sync message doesn't contain an
	 * error.
	 *
	 * You can implement the logic for your subclass in @ref findLogError.
	 *
	 * Note that this method should behave like a static method (i.e. should
	 * not touch member variables but compare the parameters only). It is
	 * implemented as non-static in order to make @ref findLogError
	 * possible.
	 *
	 * @return A string describing the error or @ref QString::null to
	 * indicate that there is no error in the logs.
	 **/
	QString findError(const QByteArray& b1, const QByteArray& b2);

protected:
	/**
	 * Find the difference in @p b1 and @p b2 (there must be one if this is
	 * called!).
	 *
	 * You should not use any member variables from this class. This method
	 * is supposed to behave like a static method, however it is implemented
	 * as a pure virtual method in order to force the programmer to actually
	 * write it.
	 *
	 * @return A string describing the error.
	 **/
	virtual QString findLogError(const QByteArray& b1, const QByteArray& b2) const = 0;
};

QString BoSyncCheckMessageBase::findError(const QByteArray& b1, const QByteArray& b2)
{
 KMD5 md5(b1);
 KMD5 md5_2(b2);
 if (md5.hexDigest() == md5_2.hexDigest()) {
	boDebug(370) << "error not in this part of the sync message" << endl;
	return QString::null;
 }
 boWarning(370) << k_funcinfo << "there must be an error in this log (MD5 sums to not match)!!" << endl;

 if (b1.size() == 0) {
	QString error = i18n("ADMIN stream (b1) is empty");
	boError(370) << k_funcinfo << error << endl;
	return error;
 }
 if (b2.size() == 0) {
	QString error = i18n("client stream (b2) is empty");
	boError(370) << k_funcinfo << error << endl;
	return error;
 }

 QString error = findLogError(b1, b2);
 if (error.isEmpty()) {
	boError(370) << k_funcinfo << "findLogError() has not returned a descriptive error string. don't know the error or cannot find it (but it must be here!)" << endl;
	error = i18n("Unknown error");
 }
 return error;
}


class BoGameSyncCheckMessage : public BoSyncCheckMessageBase
{
public:
	BoGameSyncCheckMessage() : BoSyncCheckMessageBase()
	{
		mGame = 0;
	}
	void setGame(Boson* game)
	{
		mGame = game;
	}

	virtual QByteArray makeLog()
	{
		QByteArray b;
		QDataStream s(&b, QIODevice::WriteOnly);

		// note: this acutally _changes_ the random object.
		// however since we do this at the same time on all clients, it
		// is valid.
		s << (quint32)mGame->random()->getLong(100000);
		s << mGame->random()->getDouble();
		return b;
	}

protected:
	virtual QString findLogError(const QByteArray& b1, const QByteArray& b2) const
	{
		boDebug(370) << k_funcinfo << endl;
		QDataStream s1(b1);
		QDataStream s2(b2);

		DECLARE_UNSTREAM(quint32, randomint);
		DECLARE_UNSTREAM(double, randomdouble);

		if (randomint != randomint2) {
			return i18n("Random integer numbers differ. Found: %1 should be: %2").arg(randomint2).arg(randomint);
		}
		if (randomdouble != randomdouble2) {
			return i18n("Random double numbers differ. Found: %1 should be: %2").arg(randomdouble2).arg(randomdouble);
		}
		return i18n("There is an error in the game (i.e. the Boson class) log (MD5 sums don't match), but it could not be found.");
	}

private:
	Boson* mGame;
};

class BoPlayerSyncCheckMessage : public BoSyncCheckMessageBase
{
public:
	BoPlayerSyncCheckMessage() : BoSyncCheckMessageBase()
	{
		mGame = 0;
	}
	void setGame(Boson* game)
	{
		mGame = game;
	}

	virtual QByteArray makeLog()
	{
		QByteArray playersBuffer;
		QDataStream playersStream(&playersBuffer, QIODevice::WriteOnly);
		playersStream << (quint32)mGame->allPlayerList().count();
		foreach (Player* p, mGame->allPlayerList()) {
			playersStream << (quint32)p->unfoggedCells();
			playersStream << (quint32)p->exploredCells();
			playersStream << (quint32)p->minerals();
			playersStream << (quint32)p->oil();
			playersStream << (quint32)p->ammunition("Generic");
		}
		return playersBuffer;
	}

protected:
	virtual QString findLogError(const QByteArray& b1, const QByteArray& b2) const
	{
		boDebug(370) << k_funcinfo << endl;
		QDataStream s1(b1);
		QDataStream s2(b2);
		DECLARE_UNSTREAM(quint32, count);
		if (count != count2) {
			return i18n("Have players: %1 should be: %2").arg(count2).arg(count);
		}
		for (unsigned int i = 0; i < count; i++) {
			DECLARE_UNSTREAM(quint32, fogged);
			DECLARE_UNSTREAM(quint32, explored);
			DECLARE_UNSTREAM(quint32, minerals);
			DECLARE_UNSTREAM(quint32, oil);
			DECLARE_UNSTREAM(quint32, genericAmmunition);

#define CHECK(x,x2) if (x != x2) { return i18n("Different players in players log: variable %1: found %2, expected %3").arg(#x).arg(x2).arg(x); }
			CHECK(fogged, fogged2);
			CHECK(explored, explored2);
			CHECK(minerals, minerals2);
			CHECK(oil, oil2);
			CHECK(genericAmmunition, genericAmmunition2);
#undef CHECK
		}
		return i18n("There is an error in the players log (MD5 sums don't match), but it could not be found.");
	}

private:
	Boson* mGame;
};
class BoPathSyncCheckMessage : public BoSyncCheckMessageBase
{
public:
	BoPathSyncCheckMessage() : BoSyncCheckMessageBase()
	{
		mMap = 0;
		mPathFinder = 0;
	}
	void setMap(BosonMap* m) { mMap = m; }
	void setPathfinder(BosonPath* p) { mPathFinder = p; }
	virtual QByteArray makeLog();

protected:
	virtual QString findLogError(const QByteArray& b1, const QByteArray& b2) const;

private:
	BosonMap* mMap;
	BosonPath* mPathFinder;
};

QByteArray BoPathSyncCheckMessage::makeLog()
{
 QByteArray b;
 QDataStream stream(&b, QIODevice::WriteOnly);

 // Stream dirty cell statuses
 stream << (quint32)mPathFinder->mCellStatusDirtyCount;
 stream << (quint32)mPathFinder->mCellStatusDirtySize;
 for (unsigned int i = 0; i < mPathFinder->mCellStatusDirtyCount; i++) {
	int dirtyindex = mPathFinder->mCellStatusDirty[i];
	stream << (qint32)dirtyindex;
	stream << (quint32)mPathFinder->mCellStatus[dirtyindex].flags;
 }

 // Stream some info about blocks
 // TODO: maybe don't send all blocks (too much data)?
 unsigned int movedatacount = mPathFinder->mMoveDatas.count();
 stream << (quint32)movedatacount;
 stream << (qint32)mPathFinder->mBlockSize;
 stream << (qint32)mPathFinder->mBlocksCountX;
 stream << (qint32)mPathFinder->mBlocksCountY;
 for (int i = 0; i < mPathFinder->mBlocksCountX * mPathFinder->mBlocksCountY; i++) {
	BosonPath::BlockInfo* block = &mPathFinder->mBlocks[i];
	for(unsigned int j = 0; j < movedatacount; j++) {
		stream << (qint32)block->centerx[j];
		stream << (qint32)block->centery[j];
	}
 }
 stream << (qint32)mPathFinder->mBlockConnectionsCount;
 for(int i = 0; i < mPathFinder->mBlockConnectionsCount; i++) {
	stream << mPathFinder->mBlockConnections[i];
 }
 stream << mPathFinder->mChangedBlocks.count();
 stream << mPathFinder->mDirtyConnections.count();
 stream << mPathFinder->mBlockStatusDirty.count();

 return b;
}

QString BoPathSyncCheckMessage::findLogError(const QByteArray& b1, const QByteArray& b2) const
{
 boDebug(370) << k_funcinfo << endl;
 QDataStream s1(b1);
 QDataStream s2(b2);
 QString error;

 DECLARE_UNSTREAM_COMPARE(quint32, cellStatusDirtyCount);
 DECLARE_UNSTREAM_COMPARE(quint32, cellStatusDirtySize);
 for (unsigned int i = 0; i < cellStatusDirtyCount; i++) {
	DECLARE_UNSTREAM_COMPARE(qint32, dirtyIndex);
	DECLARE_UNSTREAM_COMPARE(quint32, dirtyFlags);
 }

 DECLARE_UNSTREAM_COMPARE(quint32, movedataCount);
 DECLARE_UNSTREAM_COMPARE(qint32, blockSize);
 DECLARE_UNSTREAM_COMPARE(qint32, blocksCountX);
 DECLARE_UNSTREAM_COMPARE(qint32, blocksCountY);
 for (int i = 0; i < blocksCountX * blocksCountY; i++) {
	for(unsigned int j = 0; j < movedataCount; j++) {
		DECLARE_UNSTREAM_COMPARE(qint32, centerx);
		DECLARE_UNSTREAM_COMPARE(qint32, centery);
	}
 }
 DECLARE_UNSTREAM_COMPARE(qint32, blockConnectionsCount);
 for(int i = 0; i < blockConnectionsCount; i++) {
	DECLARE_UNSTREAM_COMPARE(quint32, connection);
 }
 DECLARE_UNSTREAM_COMPARE(quint32, changedBlocksCount);
 DECLARE_UNSTREAM_COMPARE(quint32, dirtyConnectionsCount);
 DECLARE_UNSTREAM_COMPARE(quint32, blockStatusDirtyCount);


 return error;
}


#define PATH_LOG NET_DEBUG
class BoCanvasSyncCheckMessage : public BoSyncCheckMessageBase
{
public:
	BoCanvasSyncCheckMessage() : BoSyncCheckMessageBase()
	{
		mGame = 0;
		mCanvas = 0;
		mAdvanceMessageCount = 0;
		mInterval = 0;
	}
	void setGame(Boson* game)
	{
		mGame = game;
	}
	void setCanvas(BosonCanvas* canvas, unsigned int advanceMessageCount, unsigned int interval)
	{
		mCanvas = canvas;
		mAdvanceMessageCount = advanceMessageCount;
		mInterval = interval;
	}

	/**
	 * @param interval This is supposed to be the interval of how often this
	 * log is created (in advance messages). You can use -1 if you don't
	 * know that value, however you get the best results by using it. E.g.
	 * if you call it if (advanceMessageCounter % 10 == 5), then the
	 * interval is 10.
	 **/
	virtual QByteArray makeLog()
	{
		int itemCount = mCanvas->allItems()->count();
		int size = 40;
		if (itemCount == 0 || itemCount <= size) {
			return makeLog(-1, -1);
		}
		if (mInterval == 0) {
			// interval == 0 means log everything.
			return makeLog(-1, -1);
		}

		// this formula increases the start index by count after every
		// log.
		// it also makes sure that exactly size items end up in the log.
		int start = ((mAdvanceMessageCount / mInterval) * size) % itemCount;
		return makeLog(start, size);
	}

protected:
	QByteArray makeLog(int start, int count);

	virtual QString findLogError(const QByteArray& b1, const QByteArray& b2) const
	{
		boDebug(370) << k_funcinfo << endl;
		QDataStream s1(b1);
		QDataStream s2(b2);
		DECLARE_UNSTREAM(quint32, items);
		if (items != items2) {
			return i18n("Different item counts in canvas log: found %1, expected %2").arg(items2).arg(items);
		}
		for (unsigned int i = 0; i < items; i++) {
			QString error = findItemError(s1, s2);
			if (!error.isNull()) {
				return error;
			}
		}

		DECLARE_UNSTREAM(quint32, units);
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

	static void streamItem(QDataStream& stream, BosonItem* i)
	{
		stream << (quint32)i->id();
		stream << (qint32)i->rtti();
		if (i->rtti() == RTTI::Shot) {
			stream << (qint32)((BosonShot*)i)->type();
		} else {
			stream << (qint32)-1;
		}
		stream << i->centerX();
		stream << i->centerY();
		stream << i->z();
		stream << i->rotation();
		stream << i->xRotation();
		stream << i->yRotation();
	}
	static void unstreamItem(QDataStream& stream, quint32& id, qint32& rtti, qint32& type, bofixed& x, bofixed& y, bofixed& z, bofixed& rotation, bofixed& xrotation, bofixed& yrotation)
	{
		stream >> id;
		stream >> rtti;
		stream >> type;
		stream >> x;
		stream >> y;
		stream >> z;
		stream >> rotation;
		stream >> xrotation;
		stream >> yrotation;
	}
	static QString findItemError(QDataStream& s1, QDataStream& s2)
	{
		QString error;

		DECLARE(quint32, id);
		DECLARE(qint32, rtti);
		DECLARE(qint32, type);
		DECLARE(bofixed, x);
		DECLARE(bofixed, y);
		DECLARE(bofixed, z);
		DECLARE(bofixed, rotation);
		DECLARE(bofixed, xrotation);
		DECLARE(bofixed, yrotation);

		unstreamItem(s1, id, rtti, type, x, y, z, rotation, xrotation, yrotation);
		unstreamItem(s2, id2, rtti2, type2, x2, y2, z2, rotation2, xrotation2, yrotation2);

		COMPARE(id);
		COMPARE(rtti);
		COMPARE(type);
		COMPAREITEM(x, id, rtti, type);
		COMPAREITEM(y, id, rtti, type);
		COMPAREITEM(z, id, rtti, type);
		COMPAREITEM(rotation, id, rtti, type);
		COMPAREITEM(xrotation, id, rtti, type);
		COMPAREITEM(yrotation, id, rtti, type);

		return error;
	}

	static void streamUnit(QDataStream& stream, Unit* u)
	{
		stream << (quint32)u->id();
		stream << (qint32)u->advanceWork();
		stream << (quint32)u->health();
#if PATH_LOG
		// stream some info from the pathinfo object.
		// we ignore:
		// - u
		// - hlpath
		// - llpath
		// - possibleDestRegions
		BosonPathInfo* i = u->pathInfo();
		if (u != i->unit && i->unit) {
			boError(370) << k_funcinfo "u != unit in for pathinfo" << endl;
		}
		stream << (quint32)i->hlstep;
		stream << i->start;
		stream << i->dest;
		stream << (qint32)i->range;
#if 0
		if (i->startRegion) {
			stream << (qint32)i->startRegion->id;
		} else {
			stream << (qint32)-1;
		}
		if (i->destRegion) {
			stream << (qint32)i->destRegion->id;
		} else {
			stream << (qint32)-1;
		}
#endif
		stream << (qint32)-2; // AB: region pointers are often invalid (regions deleted), so we cannot use it.
		stream << (qint32)-2; // AB: region pointers are often invalid (regions deleted), so we cannot use it.
		stream << (qint8)i->passable;
		stream << (qint8)i->canMoveOnLand;
		stream << (qint8)i->canMoveOnWater;
		stream << (qint8)i->flying;
		stream << (qint32)i->passability;
		stream << (qint8)i->moveAttacking;
		stream << (qint8)i->slowDownAtDest;
		stream << (qint32)i->waiting;
		stream << (qint32)i->pathrecalced;
#endif
	}
#if !PATH_LOG
	static void unstreamUnit(QDataStream& stream, quint32& id, qint32& work, quint32& health)
#else
	static void unstreamUnit(QDataStream& stream, quint32& id, qint32& work, quint32& health,
			quint32& hlstep, QPoint& start, QPoint&  dest, qint32& range, qint32& startRegId, qint32& destRegId, qint8& passable,
			qint8& canL, qint8& canW, qint8& flying, qint32& passability, qint8& moveAttacking, qint8& slowDownAtDest,
			qint32& waiting, qint32& pathrecalced)
#endif
	{
		stream >> id;
		stream >> work;
		stream >> health;
#if PATH_LOG
		stream >> hlstep;
		stream >> start;
		stream >> dest;
		stream >> range;
		stream >> startRegId;
		stream >> destRegId;
		stream >> passable;
		stream >> canL;
		stream >> canW;
		stream >> flying;
		stream >> passability;
		stream >> moveAttacking;
		stream >> slowDownAtDest;
		stream >> waiting;
		stream >> pathrecalced;
#endif
	}

	static QString findUnitError(QDataStream& s1, QDataStream& s2)
	{
		QString error;

		DECLARE(quint32, id);
		DECLARE(qint32, work);
		DECLARE(quint32, health);
#if !PATH_LOG
		unstreamUnit(s1, id, work, health);
		unstreamUnit(s2, id2, work2, health2);
#else
		DECLARE(quint32, hlstep);
		DECLARE(QPoint, start);
		DECLARE(QPoint, dest);
		DECLARE(qint32, range);
		DECLARE(qint32, startRegId);
		DECLARE(qint32, destRegId);
		DECLARE(qint32, passability);
		DECLARE(qint32, waiting);
		DECLARE(qint32, pathrecalced);
		DECLARE(qint8, passable);
		DECLARE(qint8, canL);
		DECLARE(qint8, canW);
		DECLARE(qint8, flying);
		DECLARE(qint8, moveAttacking);
		DECLARE(qint8, slowDown);
		unstreamUnit(s1, id, work, health,
				hlstep, start, dest, range, startRegId, destRegId, passable,
				canL, canW, flying, passability, moveAttacking, slowDown,
				waiting, pathrecalced);
		unstreamUnit(s2, id2, work2, health2,
				hlstep2, start2, dest2, range2, startRegId2, destRegId2, passable2,
				canL2, canW2, flying2, passability2, moveAttacking2, slowDown2,
				waiting2, pathrecalced2);
#endif
		if (id != id2) {
			error += i18n("Different unit ids: %1 != %2\n").arg(id).arg(id2);
			// it makes no sense to collect more
			return error;
		}
#define CHECK(x) if (x != x##2) { error += i18n("Unit %1: ").arg(id); COMPARE(x) }
		CHECK(id);
		CHECK(work);
		CHECK(health);
#if PATH_LOG
		CHECK(hlstep);
		if (start != start2) {
			error += i18n("Unit %1: start != start2: (%2,%3) != (%4,%5)\n").
					arg(id).arg(start.x()).arg(start.y()).arg(start2.x()).arg(start2.y());
		}
		if (dest != dest2) {
			error += i18n("Unit %1: dest != dest2: (%1,%2) != (%3,%4)\n").
					arg(id).arg(dest.x()).arg(dest.y()).arg(dest2.x()).arg(dest2.y());
		}
		CHECK(range);
		CHECK(startRegId);
		CHECK(destRegId);
		CHECK(passable);
		CHECK(canL);
		CHECK(canW);
		CHECK(flying);
		CHECK(passability);
		CHECK(moveAttacking);
		CHECK(slowDown);
		CHECK(waiting);
		CHECK(pathrecalced);
#endif
#undef CHECK
		return error;
	}



private:
	Boson* mGame;
	BosonCanvas* mCanvas;
	unsigned int mAdvanceMessageCount;
	unsigned int mInterval;
};

QByteArray BoCanvasSyncCheckMessage::makeLog(int start, int count)
{
 if (start < 0 || count < 0) {
	start = 0;
	count = mCanvas->allItems()->count();
 }

 BoItemList list;
 BoItemList::Iterator it;
 int index = 0;
 for (it = mCanvas->allItems()->begin(); it != mCanvas->allItems()->end() && (int)list.count() < count; ++it) {
	if (index >= start || index < ((start + count) % (int)mCanvas->allItems()->count())) {
		list.append(*it);
	}
	index++;
 }

 QByteArray buffer;
 QDataStream stream(&buffer, QIODevice::WriteOnly);
 stream << (quint32)list.count();
 unsigned int unitsCount = 0;
 for (it = list.begin(); it != list.end(); ++it) {
	BosonItem* i = *it;
	streamItem(stream, i);
	if (RTTI::isUnit(i->rtti())) {
		unitsCount++;
	}
 }

 stream << (quint32)unitsCount;
 for (it = list.begin(); it != list.end(); ++it) {
	if (RTTI::isUnit((*it)->rtti())) {
		Unit* u = (Unit*)(*it);
		streamUnit(stream, u);
	}
 }
 return buffer;
}



class BoLongSyncCheckMessage : public BoSyncCheckMessageBase
{
public:
	BoLongSyncCheckMessage() : BoSyncCheckMessageBase()
	{
		mMessageLogger = 0;
		mGame = 0;
	}
	void setMessageLogger(BoMessageLogger* l)
	{
		mMessageLogger = l;
	}
	void setGame(Boson* game)
	{
		mGame = game;
		mPlayerSync.setGame(mGame);
		mBosonSync.setGame(mGame);
	}
	void setCanvas(BosonCanvas* canvas, unsigned int advanceMessageCount, unsigned int interval)
	{
		mCanvasSync.setCanvas(canvas, advanceMessageCount, interval);
		mPathSync.setMap(canvas->map());
		mPathSync.setPathfinder(canvas->pathFinder());
	}

	virtual QByteArray makeLog();

protected:
	virtual QString findLogError(const QByteArray& b1, const QByteArray& b2) const;

private:
	BoMessageLogger* mMessageLogger;
	Boson* mGame;

	BoPlayerSyncCheckMessage mPlayerSync;
	BoCanvasSyncCheckMessage mCanvasSync;
	BoGameSyncCheckMessage mBosonSync;
	BoPathSyncCheckMessage mPathSync;
};

QByteArray BoLongSyncCheckMessage::makeLog()
{
 QMap<QString, QByteArray> streams;

#if NET_DEBUG_2
 streams.insert("PathStream", mPathSync.makeLog());
#endif
 streams.insert("CanvasStream", mCanvasSync.makeLog());
 streams.insert("BosonStream", mBosonSync.makeLog());
 streams.insert("PlayersStream", mPlayerSync.makeLog());

 // AB: we cannot use this yet.
 // the message log may contain old messages as well, from a previous game or
 // so. these would be present for one client only.
 QByteArray messagesBuffer;
#if 0
 QBuffer messagesBuffer2(messagesBuffer);
 messagesBuffer2.open(QIODevice::WriteOnly);
 mMessageLogger->saveMessageLog(&messagesBuffer2, 100);
 stream << messages;
#endif
 streams.insert("MessagesStream", messagesBuffer);

 QByteArray b;
 QDataStream logStream(&b, QIODevice::WriteOnly);
 logStream << streams;

 return b;
}

QString BoLongSyncCheckMessage::findLogError(const QByteArray& b1, const QByteArray& b2) const
{
 QDataStream s1(b1);
 QDataStream s2(b2);
 QMap<QString, QByteArray> streams, streams2;
 s1 >> streams;
 s2 >> streams2;

 QString error;
 if (streams.count() != streams2.count()) {
	error = i18n("Different streams count: %1, but should be %2").arg(streams2.count()).arg(streams.count());
	return error;
 }
 BoGameSyncCheckMessage gameSync;
 error = gameSync.findError(streams["BosonStream"], streams2["BosonStream"]);
 if (!error.isNull()) {
	return error;
 }

#if NET_DEBUG_2
 BoPathSyncCheckMessage pathSync;
 error = pathSync.findError(streams["PathStream"], streams2["PathStream"]);
 if (!error.isNull()) {
	return error;
 }
#endif
 BoCanvasSyncCheckMessage canvasSync;
 error = canvasSync.findError(streams["CanvasStream"], streams2["CanvasStream"]);
 if (!error.isNull()) {
	return error;
 }

 BoPlayerSyncCheckMessage playerSync;
 error = playerSync.findError(streams["PlayersStream"], streams2["PlayersStream"]);
 if (!error.isNull()) {
	return error;
 }

 error = i18n("Error not found. Bug in findLogError()");

 return error;
}

class BosonNetworkSynchronizerPrivate
{
public:
	BosonNetworkSynchronizerPrivate()
	{
	}
	BosonNetworkSyncChecker mSyncChecker;
	BosonNetworkSyncer mSyncer;
};

BosonNetworkSynchronizer::BosonNetworkSynchronizer()
{
 d = new BosonNetworkSynchronizerPrivate;
 d->mSyncChecker.setParent(this);
 d->mSyncer.setParent(this);
 mGame = 0;
}

BosonNetworkSynchronizer::~BosonNetworkSynchronizer()
{
 delete d;
}

void BosonNetworkSynchronizer::setGame(Boson* game)
{
 mGame = game;
 d->mSyncChecker.setGame(mGame);
 d->mSyncer.setGame(mGame);
}

void BosonNetworkSynchronizer::setMessageLogger(BoMessageLogger* logger)
{
 d->mSyncChecker.setMessageLogger(logger);
}

void BosonNetworkSynchronizer::receiveAdvanceMessage(BosonCanvas* canvas)
{
 d->mSyncChecker.receiveAdvanceMessage(canvas);
}

bool BosonNetworkSynchronizer::receiveNetworkSyncCheck(QDataStream& stream)
{
 return d->mSyncChecker.receiveNetworkSyncCheck(stream);
}

bool BosonNetworkSynchronizer::receiveNetworkSyncCheckAck(QDataStream& stream, quint32 sender)
{
 return d->mSyncChecker.receiveNetworkSyncCheckAck(stream, sender);
}

bool BosonNetworkSynchronizer::receiveNetworkRequestSync(QDataStream& stream)
{
 return d->mSyncer.receiveNetworkRequestSync(stream);
}

bool BosonNetworkSynchronizer::receiveNetworkSync(QDataStream& stream)
{
 if (!mGame) {
	BO_NULL_ERROR(mGame);
	return false;
 }
 bool synced = d->mSyncer.receiveNetworkSync(stream);
 if (!synced) {
	// the game is most probably really unusable now, because this can
	// happen only if something could not be loaded - but we have already
	// started loading (and therefore deleted old data).
	boError(370) << k_funcinfo << "loading from sync message failed. game unusable now" << endl;
	return false;
 }
 boDebug(370) << k_funcinfo << "sync message successfully loaded" <<endl;

 forceCompleteSyncCheckAndUnlockGame(mGame->canvasNonConst());

 return true;
}

bool BosonNetworkSynchronizer::receiveNetworkSyncUnlockGame(QDataStream& stream)
{
 Q_UNUSED(stream);
 if (!d->mSyncer.gameLocked()) {
	boError(370) << k_funcinfo << "cannot unlock game - game not locked" << endl;
	return false;
 }
 d->mSyncer.setGameLocked(false);
 return true;
}

void BosonNetworkSynchronizer::syncCheckingCompleted(const Q3ValueList<quint32>& outOfSyncClients)
{
 if (!outOfSyncClients.isEmpty()) {
	boDebug(370) << k_funcinfo << outOfSyncClients.count() << " clients out of sync" << endl;
 }
 if (d->mSyncer.gameLocked()) {
	if (outOfSyncClients.isEmpty()) {
		addChatSystemMessage(i18n("Network Sync completed. Clients are in sync now. Unpause the game to continue"));
	} else {
		addChatSystemMessage(i18n("Network Sync completed, however there are still %1 clients out of sync. Sync failed, the game is probably broken permanently for you now. Sorry.").arg(outOfSyncClients.count()));
		// AB: we are out of sync, but if the player wants to, he can
		// still continue the game by unpausing it. we don't prevent him
		// from doing so (maybe he's lucky and we succeed at syncing
		// later)
	}

	// we need to send a last message to network in order to continue the
	// game.
	// all messages after this point will be delivered again!
	unlockGame();
 }
}

void BosonNetworkSynchronizer::syncNetwork()
{
 BO_CHECK_NULL_RET(mGame);
 boDebug(370) << k_funcinfo << endl;
 if (!mGame->isAdmin()) {
	boWarning(370) << k_funcinfo << "may be called by ADMIN only" << endl;
	return;
 }
 if (d->mSyncer.gameLocked()) {
	boError(370) << k_funcinfo << "game is already locked" << endl;
	return;
 }

 // note that Boson::gamePaused() stays at false even after this call. it will
 // return true only once the property has been received from network.
 // however the game timer is stopped immediately, so no new advance messages
 // are sent.
 mGame->forcePauseGame();

 // on receiving of this message we generate the sync message.
 // the clients will load their game from the sync message and
 // then they'll hopefully be in sync again
 mGame->sendMessage(0, BosonMessageIds::IdNetworkRequestSync);
}

void BosonNetworkSynchronizer::unlockGame()
{
 BO_CHECK_NULL_RET(mGame);
 // this is being called when sync is completed.
 // note that we don't know whether syncing was successfull.
 // now we undo the locking that was made by lockGame().
 //
 // note that we keep the game in paused mode - the player needs to unpause
 // manually. this is primarily intended to give him a chance to read whether
 // syncing succeeded.

 // AB: game timer will be restarted automatically when unpausing the game.

 if (!mGame->isAdmin()) {
	// AB: this is never reached. this is called only for ADMIN.
	return;
 }
 mGame->sendMessage(0, BosonMessageIds::IdNetworkSyncUnlockGame);
}

void BosonNetworkSynchronizer::forceCompleteSyncCheckAndUnlockGame(BosonCanvas* canvas)
{
 // AB: note that the game is unlocked once all clients returned an
 // ACK message (positive or negative)

 BO_CHECK_NULL_RET(canvas);
 if (!d->mSyncer.gameLocked()) {
	boError(370) << k_funcinfo << "game not locked. aborting." << endl;
	return;
 }

 if (d->mSyncChecker.hasLogs()) {
	// AB: this must never happen. when this method is called the message
	// queue (in Boson) must be empty, so there can't be any SyncCheck
	// message in the air. but once the SyncCheck message is received, the
	// log is removed from the queue. so the queue must be empty at this
	// point, or there must be a bug.

	boWarning(370) << k_funcinfo << "log queue is not empty. clearing it now." << endl;
	d->mSyncChecker.clearLogs();
 }

 // AB: this makes (and if ADMIN sends) a sync check message. when all ACKs have
 // been received, the game is unlocked again.
 d->mSyncChecker.forceCompleteSyncCheck(canvas);
}

void BosonNetworkSynchronizer::addChatSystemMessage(const QString& msg)
{
 BO_CHECK_NULL_RET(mGame);
 mGame->slotAddChatSystemMessage(i18n("NetworkSync"), msg);
}

bool BosonNetworkSynchronizer::acceptNetworkTransmission(int msgid) const
{
 if (!d->mSyncer.gameLocked()) {
	// accept all messages. sync not active.
	return true;
 }

 // we are in a network sync run. only certain messages are allowed right now
 if (msgid < KGameMessage::IdUser) {
	return false;
 }
 switch (msgid - KGameMessage::IdUser) {
	case BosonMessageIds::IdNetworkSync:
	case BosonMessageIds::IdNetworkSyncCheck:
	case BosonMessageIds::IdNetworkSyncCheckACK:
	case BosonMessageIds::IdNetworkSyncUnlockGame:
		return true;
	default:
		return false;
 }
 return false;
}





class BosonNetworkSyncCheckerPrivate
{
public:
	BosonNetworkSyncCheckerPrivate()
	{
	}
	Q3PtrQueue<QByteArray> mLogs;
	Q3IntDict<BoAwaitAck> mAwaitAcks;
};

BosonNetworkSyncChecker::BosonNetworkSyncChecker()
{
 mParent = 0;
 mAdvanceMessageCounter = 0;
 mSyncId = 0;
 mGame = 0;
 mMessageLogger = 0;
 d = new BosonNetworkSyncCheckerPrivate;
 d->mLogs.setAutoDelete(true);
 d->mAwaitAcks.setAutoDelete(true);
}

BosonNetworkSyncChecker::~BosonNetworkSyncChecker()
{
 d->mLogs.clear();
 d->mAwaitAcks.clear();
 delete d;
}

bool BosonNetworkSyncChecker::hasLogs() const
{
 return !d->mLogs.isEmpty();
}

void BosonNetworkSyncChecker::clearLogs()
{
 d->mLogs.clear();
}

void BosonNetworkSyncChecker::receiveAdvanceMessage(BosonCanvas* canvas)
{
 BO_CHECK_NULL_RET(mGame);
 BO_CHECK_NULL_RET(mMessageLogger);
 BO_CHECK_NULL_RET(canvas);
 // a message is sent every 250 ms
 mAdvanceMessageCounter++;

 unsigned int shortInterval = 10; // every 2,5s
 unsigned int longInterval = 100; // every 25s
 if (mAdvanceMessageCounter % shortInterval == 5) {
	QByteArray log = createLongSyncCheckLog(canvas, mAdvanceMessageCounter, shortInterval);
	storeLogAndSend(log);
 }
 if (mAdvanceMessageCounter % longInterval == 50) {
 }
}

void BosonNetworkSyncChecker::forceCompleteSyncCheck(BosonCanvas* canvas)
{
 BO_CHECK_NULL_RET(canvas);
 QByteArray log = createCompleteSyncCheckLog(canvas);
 storeLogAndSend(log);
}

void BosonNetworkSyncChecker::storeLogAndSend(const QByteArray& log)
{
 d->mLogs.enqueue(new QByteArray(log));
 if (mGame->isAdmin()) {
	BoAwaitAck* wait = new BoAwaitAck();
	wait->sendLog(mGame, log, mSyncId);
	d->mAwaitAcks.insert(mSyncId, wait);
	mSyncId++;
 }
}

bool BosonNetworkSyncChecker::receiveNetworkSyncCheck(QDataStream& stream)
{
 if (!mGame) {
	return false;
 }
 if (d->mLogs.isEmpty()) {
	boError(370) << k_funcinfo << "no logs available" << endl;
	return false;
 }
 quint32 syncId;
 Q3CString md5String;
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

bool BosonNetworkSyncChecker::receiveNetworkSyncCheckAck(QDataStream& stream, quint32 sender)
{
 if (!mParent) {
	BO_NULL_ERROR(mParent);
	return false;
 }
 if (!mGame) {
	BO_NULL_ERROR(mGame);
	return false;
 }
 if (!mGame->isAdmin()) {
	boError(370) << k_funcinfo << "Only ADMIN is allowed to call this" << endl;
	return false;
 }
 quint32 id;
 Q3CString md5;
 qint8 verify;
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
		await->addOutOfSyncClient(sender);

		// try to find the error
		QByteArray correct = await->log();
		QByteArray broken = brokenLog;
		BoLongSyncCheckMessage longSync;
		QString error = longSync.findError(correct, broken);
		addChatSystemMessage(i18n("Error message: %1").arg(error));
		boDebug(370) << k_funcinfo << "Error message: " << error << endl;
	}
 }

 if (await) {
	if (await->receiveAck(sender)) {
		mParent->syncCheckingCompleted(await->outOfSyncClients());
		d->mAwaitAcks.remove(id);
		await = 0;
	}
 } else {
	boWarning(370) << k_funcinfo << "oops - not waiting for ack of " << id << endl;
 }

 return verify;
}

void BosonNetworkSyncChecker::sendAck(const Q3CString& md5, bool verify, unsigned int syncId, const QByteArray& origLog)
{
 QByteArray buffer;
 QDataStream stream(&buffer, QIODevice::WriteOnly);
 stream << (quint32)syncId;
 stream << md5;
 stream << (qint8)verify;
 if (!verify) {
	stream << origLog;
 }
 mGame->sendMessage(buffer, BosonMessageIds::IdNetworkSyncCheckACK);
}

QByteArray BosonNetworkSyncChecker::createCompleteSyncCheckLog(BosonCanvas* canvas) const
{
 // AB: we do no (yet?) have a dedicated class for this. we just use the normal
 // BoLongSyncCheckMessage, but we don't use any limits (i.e. we log _all_ units, not
 // just a few)
 BoLongSyncCheckMessage m;
 m.setGame(mGame);
 m.setMessageLogger(mMessageLogger);
 m.setCanvas(canvas, 0, 0); // AB: use advancemessagecount==interval==0 to log everything
 return m.makeLog();
}

QByteArray BosonNetworkSyncChecker::createLongSyncCheckLog(BosonCanvas* canvas, unsigned int advanceMessageCounter, unsigned int interval) const
{
 BosonProfiler profiler("CreateLongSyncCheckLog");

 BoLongSyncCheckMessage m;
 m.setGame(mGame);
 m.setMessageLogger(mMessageLogger);
 m.setCanvas(canvas, advanceMessageCounter, interval);
 return m.makeLog();
}



BosonNetworkSyncer::BosonNetworkSyncer()
{
 mParent = 0;
 mGame = 0;
 mGameLocked = false;
}

BosonNetworkSyncer::~BosonNetworkSyncer()
{
}

bool BosonNetworkSyncer::receiveNetworkRequestSync(QDataStream&)
{
 if (!mGame) {
	BO_NULL_ERROR(mGame);
	return false;
 }

 // this locks message delivery
 setGameLocked(true);

 // make sure the message queue is empty
 mGame->clearDelayedMessages();

 if (!mGame->isAdmin()) {
	return true;
 }
 QByteArray syncBuffer = createSyncMessage();
 if (syncBuffer.size() == 0) {
	boError(370) << k_funcinfo << "could not create sync message." << endl;
	setGameLocked(false);
	return false;
 }
 mGame->sendMessage(syncBuffer, BosonMessageIds::IdNetworkSync);
 return true;
}

bool BosonNetworkSyncer::receiveNetworkSync(QDataStream& stream)
{
 if (!mGame) {
	BO_NULL_ERROR(mGame);
	return false;
 }
 QString xml;
 stream >> xml;

 QDomDocument doc;
 if (!doc.setContent(xml)) {
	boError(370) << k_funcinfo << "unable to load XML from stream" << endl;
	return false;
 }
 QDomElement root = doc.documentElement();

 {
	QDomElement players = root.namedItem("Players").toElement();
	if (players.isNull()) {
		boError(370) << k_funcinfo << "no Players tag found" << endl;
		return false;
	}
	foreach (Player* player, mGame->allPlayerList()) {
		QDomElement playerElement;
		for (QDomNode n = players.firstChild(); !n.isNull() && playerElement.isNull(); n = n.nextSibling()) {
			QDomElement e = n.toElement();
			if (e.isNull()) {
				continue;
			}
			if (e.tagName() != "Player") {
				continue;
			}
			bool ok = false;
			int id = e.attribute(QString::fromLatin1("PlayerId")).toInt(&ok);
			if (id < 0) {
				ok = false;
			}
			if (!ok) {
				boError(370) << k_funcinfo << "Invalid PlayerId attribute" << endl;
				return false;
			}
			if (id == player->bosonId()) {
				playerElement = e;
			}
		}
		if (playerElement.isNull()) {
			boError(370) << k_funcinfo << "No Player tag found for player " << player->bosonId() << endl;
			return false;
		}

		// Delete old data (e.g. units)
		BosonMap* map = player->map();
		player->quitGame();
		player->initMap(map);
		if (!player->loadFromXML(playerElement)) {
			boError(370) << k_funcinfo << "could not load player " << player->bosonId() << endl;
			return false;
		}
	}
 }

 QDomElement canvasElement = root.namedItem("Canvas").toElement();
 if (canvasElement.isNull()) {
	boError(370) << k_funcinfo << "no Canvas tag found" << endl;
	return false;
 }

 BosonCanvas* canvas = mGame->canvasNonConst();
#warning FIXME: this removes all units. notify all GUI objects! -> either using signalRemoveUnit()/Item(), or by calling quitGame() on them, too!
 canvas->quitGame();
 canvas->loadFromXML(canvasElement);


 return true;
}

void BosonNetworkSyncer::setGameLocked(bool l)
{
 mGameLocked = l;
}

QByteArray BosonNetworkSyncer::createSyncMessage()
{
 QByteArray b;
 if (!mGame) {
	BO_NULL_ERROR(mGame);
	return b;
 }

 QDomDocument doc;
 QDomElement root = doc.createElement("Sync");
 doc.appendChild(root);
 {
	QDomElement players = doc.createElement("Players");
	foreach (Player* p, mGame->allPlayerList()) {
		QDomElement player = doc.createElement("Player");
		if (!p->saveAsXML(player)) {
			boError(370) << k_funcinfo << "unable to save player " << p->bosonId() << endl;
			return b;
		}
		players.appendChild(player);
	}
	root.appendChild(players);
 }

 {
	QDomElement canvas = doc.createElement("Canvas");
	mGame->canvas()->saveAsXML(canvas);
	root.appendChild(canvas);
 }

 // TODO: save Boson (especially random number - we probably need a design
 // change for the random number class here)
 boWarning(370) << k_funcinfo << "TODO: save the Boson object to the xml document" << endl;


 QDataStream stream(&b, QIODevice::WriteOnly);
 stream << doc.toString();

 return b;
}





#undef DECLARE
#undef UNSTREAM
#undef DECLARE_UNSTREAM
#undef DECLARE_UNSTREAM_COMPARE
#undef DECLARE_UNSTREAM_COMPARE

