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
#include "bosonmap.h"
#include "bosonpath.h"
#include <bodebug.h>

#include <kgame/kmessageclient.h>

#include <kmdcodec.h>
#include <klocale.h>
#include <krandomsequence.h>

#include <qdatastream.h>
#include <qptrqueue.h>
#include <qptrlist.h>
#include <qbuffer.h>
#include <qvaluelist.h>
#include <qmap.h>
#include <qintdict.h>


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

// convenience macros that combine the above macros.
#define DECLARE_UNSTREAM(type, name) DECLARE(type, name) UNSTREAM(name)
#define DECLARE_UNSTREAM_COMPARE(type, name) DECLARE(type, name) UNSTREAM(name) COMPARE(name)


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
class BoSyncMessageBase
{
public:
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

QString BoSyncMessageBase::findError(const QByteArray& b1, const QByteArray& b2)
{
 KMD5 md5(b1);
 KMD5 md5_2(b2);
 if (md5.hexDigest() == md5_2.hexDigest()) {
	boDebug(370) << "error not in this part of the sync message" << endl;
	return QString::null;
 }
 boWarning(370) << k_funcinfo << "there must be an error in this log (MD5 sums to not match)!!" << endl;

 QString error = findLogError(b1, b2);
 if (error.isEmpty()) {
	boError(370) << k_funcinfo << "findLogError() has not returned a descriptive error string. don't know the error or cannot find it (but it must be here!)" << endl;
	error = i18n("Unknown error");
 }
 return error;
}


class BoGameSyncMessage : public BoSyncMessageBase
{
public:
	BoGameSyncMessage() : BoSyncMessageBase()
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
		QDataStream s(b, IO_WriteOnly);

		// note: this acutally _changes_ the random object.
		// however since we do this at the same time on all clients, it
		// is valid.
		s << (Q_ULONG)mGame->random()->getLong(100000);
		return b;
	}

protected:
	virtual QString findLogError(const QByteArray& b1, const QByteArray& b2) const
	{
		boDebug(370) << k_funcinfo << endl;
		QDataStream s1(b1, IO_ReadOnly);
		QDataStream s2(b2, IO_ReadOnly);

		DECLARE_UNSTREAM(Q_ULONG, random);

		if (random != random2) {
			return i18n("Random numbers differ. Found: %1 should be: %2").arg(random2).arg(random);
		}
		return i18n("There is an error in the game (i.e. the Boson class) log (MD5 sums don't match), but it could not be found.");
	}

private:
	Boson* mGame;
};

class BoPlayerSyncMessage : public BoSyncMessageBase
{
public:
	BoPlayerSyncMessage() : BoSyncMessageBase()
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
		QDataStream playersStream(playersBuffer, IO_WriteOnly);
		QPtrListIterator<KPlayer> playerIt(*mGame->playerList());
		playersStream << (Q_UINT32)mGame->playerList()->count();
		while (playerIt.current()) {
			Player* p = (Player*)playerIt.current();
			playersStream << (Q_UINT32)p->foggedCells();
			playersStream << (Q_UINT32)p->minerals();
			playersStream << (Q_UINT32)p->oil();
			++playerIt;
		}
		return playersBuffer;
	}

protected:
	virtual QString findLogError(const QByteArray& b1, const QByteArray& b2) const
	{
		boDebug(370) << k_funcinfo << endl;
		QDataStream s1(b1, IO_ReadOnly);
		QDataStream s2(b2, IO_ReadOnly);
		DECLARE_UNSTREAM(Q_UINT32, count);
		if (count != count2) {
			return i18n("Have players: %1 should be: %2").arg(count2).arg(count);
		}
		for (unsigned int i = 0; i < count; i++) {
			DECLARE_UNSTREAM(Q_UINT32, fogged);
			DECLARE_UNSTREAM(Q_UINT32, minerals);
			DECLARE_UNSTREAM(Q_UINT32, oil);

#define CHECK(x,x2) if (x != x2) { return i18n("Different players in players log: variable %1: found %2, expected %3").arg(#x).arg(x2).arg(x); }
			CHECK(fogged, fogged2);
			CHECK(minerals, minerals2);
			CHECK(oil, oil2);
#undef CHECK
		}
		return i18n("There is an error in the players log (MD5 sums don't match), but it could not be found.");
	}

private:
	Boson* mGame;
};
class BoPathSyncMessage : public BoSyncMessageBase
{
public:
	BoPathSyncMessage() : BoSyncMessageBase()
	{
		mMap = 0;
		mPathFinder = 0;
	}
	void setMap(BosonMap* m) { mMap = m; }
	void setPathfinder(BosonPath2* p) { mPathFinder = p; }
	virtual QByteArray makeLog();

protected:
	virtual QString findLogError(const QByteArray& b1, const QByteArray& b2) const;

	// AB: this is mostly a debugging method. we really should not stream
	// _all_ of this data.
	static bool streamRegion(QDataStream& stream, BosonPathRegion* r)
	{
		if (!r) {
			return false;
		}
		stream << (Q_INT32)r->id;
		stream << (Q_INT32)r->passabilityType;
		stream << (Q_INT32)r->cellsCount;
		stream << (float)r->centerx;
		stream << (float)r->centery;
		stream << (float)r->cost;
		stream << (Q_UINT32)r->neighbors.count();
		for (unsigned int i = 0; i < r->neighbors.count(); i++) {
			if (!r->neighbors[i].region) {
				boError(370) << k_funcinfo << "NULL neighbor region " << i << endl;
				return false;
			}
			stream << (Q_INT32)r->neighbors[i].region->id;
			stream << (float)r->neighbors[i].cost;
			stream << (Q_INT32)r->neighbors[i].bordercells;
		}
		if (r->group) {
			// TODO
		}
		if (r->parent) {
			// TODO?
		}
	}

	static bool unstreamRegion(QDataStream& stream, Q_INT32& id, Q_INT32& passabilityType, Q_INT32& cellsCount, float& centerx, float& centery, float& cost,
			Q_UINT32& nCount, QValueVector<Q_INT32>& nIds, QValueVector<float>& nCost, QValueVector<Q_INT32>& nBorderCells)
	{
		stream >> id;
		stream >> passabilityType;
		stream >> cellsCount;
		stream >> centerx;
		stream >> centery;
		stream >> cost;
		stream >> nCount;
		nIds.resize(nCount);
		nCost.resize(nCount);
		nBorderCells.resize(nCount);
		for (unsigned int i = 0; i < nCount; i++) {
			Q_UINT32 id;
			float cost;
			Q_UINT32 borderCells;
			stream >> id;
			stream >> cost;
			stream >> borderCells;
			nIds[i] = id;
			nCost[i] = cost;
			nBorderCells[i] = borderCells;
		}
		return true;
	}
private:
	BosonMap* mMap;
	BosonPath2* mPathFinder;
};

QByteArray BoPathSyncMessage::makeLog()
{
 QByteArray b;
 // TODO: do NOT stream all sectors! (too much data)
 QDataStream stream(b, IO_WriteOnly);
 stream << (Q_UINT32)mPathFinder->sectorWidth();
 stream << (Q_UINT32)mPathFinder->sectorHeight();
 unsigned int count = 0;
 for (unsigned int x = 0; x * mPathFinder->sectorWidth() < mMap->width(); x++) {
	for (unsigned int y = 0; y * mPathFinder->sectorHeight() < mMap->height(); y++) {
		count++;
	}
 }
 stream << (Q_UINT32)count;
 for (unsigned int x = 0; x * mPathFinder->sectorWidth() < mMap->width(); x++) {
	for (unsigned int y = 0; y * mPathFinder->sectorHeight() < mMap->height(); y++) {
		BosonPathSector* s = mPathFinder->sector(x, y);
		if (!s) {
			boError(370) << k_funcinfo << "NULL sector at " << x << "," << y << endl;
			return QByteArray();
		}
		if (s->pathfinder != mPathFinder) {
			boError(370) << k_funcinfo << "unexpected pathfinder pointer" << endl;
			return QByteArray();
		}
		stream << (Q_INT32)s->x;
		stream << (Q_INT32)s->y;
		stream << (Q_INT32)s->w;
		stream << (Q_INT32)s->h;

		stream << (Q_UINT32)s->regions.count();
		for (unsigned int i = 0; i < s->regions.count(); i++) {
			BosonPathRegion* r = s->regions[i];
			if (!r) {
				boError(370) << k_funcinfo << "NULL region at " << i << " in sector " << x << "," << y << endl;
				return QByteArray();
			}
			if (r->sector != s) {
				boError(370) << k_funcinfo << "r->sector is not parent sector of region!" << endl;
				return QByteArray();
			}
			if (!streamRegion(stream, r)) {
				boError(370) << k_funcinfo << "could not stream region " << i << " in sector " << x << "," << y << endl;
				return QByteArray();
			}
		}
	}
 }
 return b;
}

QString BoPathSyncMessage::findLogError(const QByteArray& b1, const QByteArray& b2) const
{
 boDebug(370) << k_funcinfo << endl;
 QDataStream s1(b1, IO_ReadOnly);
 QDataStream s2(b2, IO_ReadOnly);
 QString error;
 DECLARE_UNSTREAM_COMPARE(Q_UINT32, sectorWidth);
 DECLARE_UNSTREAM_COMPARE(Q_UINT32, sectorHeight);
 DECLARE_UNSTREAM_COMPARE(Q_UINT32, sectorCount);
 for (unsigned int i = 0; i < sectorCount; i++) {
	DECLARE_UNSTREAM_COMPARE(Q_INT32, x);
	DECLARE_UNSTREAM_COMPARE(Q_INT32, y);
	DECLARE_UNSTREAM_COMPARE(Q_INT32, w);
	DECLARE_UNSTREAM_COMPARE(Q_INT32, h);
	DECLARE_UNSTREAM_COMPARE(Q_UINT32, regionCount);

	for (unsigned int j = 0; j < regionCount; j++) {
		DECLARE(Q_INT32, id);
		DECLARE(Q_INT32, passabilityType);
		DECLARE(Q_INT32, cellsCount);
		DECLARE(float, centerx);
		DECLARE(float, centery);
		DECLARE(float, cost);
		DECLARE(Q_UINT32, nCount);
		DECLARE(QValueVector<Q_INT32>, nIds);
		DECLARE(QValueVector<float>, nCost);
		DECLARE(QValueVector<Q_INT32>, nBorderCells);
		unstreamRegion(s1, id, passabilityType, cellsCount, centerx, centery, cost, nCount, nIds, nCost, nBorderCells);
		unstreamRegion(s2, id2, passabilityType2, cellsCount2, centerx2, centery2, cost2, nCount2, nIds2, nCost2, nBorderCells2);

		COMPARE(id);
		COMPARE(passabilityType);
		COMPARE(cellsCount);
		COMPARE(centerx);
		COMPARE(centery);
		COMPARE(cost);
		COMPARE(nCount);
		if (nIds.count() != nIds2.count()) {
			return error;
		}
		for (unsigned int k = 0; k < nCount; k++) {
			if (nIds[k] != nIds2[k]) {
				error += i18n("neightbor ids at %1 don't match in region %2 of sector %3\n").arg(k).arg(j).arg(i);
			}
			if (nCost[k] != nCost2[k]) {
				error += i18n("neightbor costs at %1 don't match in region %2 of sector %3\n").arg(k).arg(j).arg(i);
			}
			if (nBorderCells[k] != nBorderCells[k]) {
				error += i18n("neightbor bordercells at %1 don't match in region %2 of sector %3\n").arg(k).arg(j).arg(i);
			}
		}
	}
 }



 return error;
}


// just for debugging currently. remove asap!
#warning remove asap
#define PATH_LOG 1
class BoCanvasSyncMessage : public BoSyncMessageBase
{
public:
	BoCanvasSyncMessage() : BoSyncMessageBase()
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
		QDataStream s1(b1, IO_ReadOnly);
		QDataStream s2(b2, IO_ReadOnly);
		DECLARE_UNSTREAM(Q_UINT32, items);
		if (items != items2) {
			return i18n("Different item counts in canvas log: found %1, expected %2").arg(items2).arg(items);
		}
		for (unsigned int i = 0; i < items; i++) {
			QString error = findItemError(s1, s2);
			if (!error.isNull()) {
				return error;
			}
		}

		DECLARE_UNSTREAM(Q_UINT32, units);
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
		QString error;

		DECLARE(Q_UINT32, id);
		DECLARE(float, x);
		DECLARE(float, y);
		DECLARE(float, z);
		DECLARE(float, rotation);
		DECLARE(float, xrotation);
		DECLARE(float, yrotation);

		unstreamItem(s1, id, x, y, z, rotation, xrotation, yrotation);
		unstreamItem(s2, id2, x2, y2, z2, rotation2, xrotation2, yrotation2);

		COMPARE(x);
		COMPARE(y);
		COMPARE(z);
		COMPARE(rotation);
		COMPARE(xrotation);
		COMPARE(yrotation);

		return error;
	}

	static void streamUnit(QDataStream& stream, Unit* u)
	{
		stream << (Q_UINT32)u->id();
		stream << (Q_INT32)u->work();
		stream << (Q_UINT32)u->health();
#if PATH_LOG
		// stream some info from the pathinfo object.
		// we ignore:
		// - u
		// - hlpath
		// - llpath
		// - possibleDestRegions
		BosonPathInfo* i = u->pathInfo();
		if (u != i->unit && i->unit) {
			boError() << k_funcinfo "u != unit in for pathinfo" << endl;
		}
		stream << (Q_UINT32)i->hlstep;
		stream << i->start;
		stream << i->dest;
		stream << (Q_INT32)i->range;
#if 0
		if (i->startRegion) {
			stream << (Q_INT32)i->startRegion->id;
		} else {
			stream << (Q_INT32)-1;
		}
		if (i->destRegion) {
			stream << (Q_INT32)i->destRegion->id;
		} else {
			stream << (Q_INT32)-1;
		}
#endif
		stream << (Q_INT32)-2; // AB: region pointers are often invalid (regions deleted), so we cannot use it.
		stream << (Q_INT32)-2; // AB: region pointers are often invalid (regions deleted), so we cannot use it.
		stream << (Q_INT8)i->passable;
		stream << (Q_INT8)i->canMoveOnLand;
		stream << (Q_INT8)i->canMoveOnWater;
		stream << (Q_INT8)i->flying;
		stream << (Q_INT32)i->passability;
		stream << (Q_INT8)i->moveAttacking;
		stream << (Q_INT8)i->slowDownAtDest;
		stream << (Q_INT32)i->waiting;
		stream << (Q_INT32)i->pathrecalced;
#endif
	}
#if !PATH_LOG
	static void unstreamUnit(QDataStream& stream, Q_UINT32& id, Q_INT32& work, Q_UINT32& health)
#else
	static void unstreamUnit(QDataStream& stream, Q_UINT32& id, Q_INT32& work, Q_UINT32& health,
			Q_UINT32& hlstep, QPoint& start, QPoint&  dest, Q_INT32& range, Q_INT32& startRegId, Q_INT32& destRegId, Q_INT8& passable,
			Q_INT8& canL, Q_INT8& canW, Q_INT8& flying, Q_INT32& passability, Q_INT8& moveAttacking, Q_INT8& slowDownAtDest,
			Q_INT32& waiting, Q_INT32& pathrecalced)
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

		DECLARE(Q_UINT32, id);
		DECLARE(Q_INT32, work);
		DECLARE(Q_UINT32, health);
#if !PATH_LOG
		unstreamUnit(s1, id, work, health);
		unstreamUnit(s2, id2, work2, health2);
#else
		DECLARE(Q_UINT32, hlstep);
		DECLARE(QPoint, start);
		DECLARE(QPoint, dest);
		DECLARE(Q_INT32, range);
		DECLARE(Q_INT32, startRegId);
		DECLARE(Q_INT32, destRegId);
		DECLARE(Q_INT32, passability);
		DECLARE(Q_INT32, waiting);
		DECLARE(Q_INT32, pathrecalced);
		DECLARE(Q_INT8, passable);
		DECLARE(Q_INT8, canL);
		DECLARE(Q_INT8, canW);
		DECLARE(Q_INT8, flying);
		DECLARE(Q_INT8, moveAttacking);
		DECLARE(Q_INT8, slowDown);
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

QByteArray BoCanvasSyncMessage::makeLog(int start, int count)
{
 // AB it would be sufficient to stream data of a couple of items only.
 // that would be a lot faster.
 // e.g. only every nth item, where n is rotating, i.e. first log uses
 // items (n=10) 0, 10, 20, ... second log uses items 1, 11, 21, ... and
 // so on. we'd still cover all items but would have to stream a lot data
 // less

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



class BoLongSyncMessage : public BoSyncMessageBase
{
public:
	BoLongSyncMessage() : BoSyncMessageBase()
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
		mPathSync.setPathfinder(canvas->pathfinder());
	}

	virtual QByteArray makeLog();

protected:
	virtual QString findLogError(const QByteArray& b1, const QByteArray& b2) const;

private:
	BoMessageLogger* mMessageLogger;
	Boson* mGame;

	BoPlayerSyncMessage mPlayerSync;
	BoCanvasSyncMessage mCanvasSync;
	BoGameSyncMessage mBosonSync;
	BoPathSyncMessage mPathSync;
};

QByteArray BoLongSyncMessage::makeLog()
{
 QMap<QString, QByteArray> streams;

 streams.insert("PathStream", mPathSync.makeLog());
 streams.insert("CanvasStream", mCanvasSync.makeLog());
 streams.insert("BosonStream", mBosonSync.makeLog());
 streams.insert("PlayersStream", mPlayerSync.makeLog());

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

QString BoLongSyncMessage::findLogError(const QByteArray& b1, const QByteArray& b2) const
{
 QDataStream s1(b1, IO_ReadOnly);
 QDataStream s2(b2, IO_ReadOnly);
 QMap<QString, QByteArray> streams, streams2;
 s1 >> streams;
 s2 >> streams2;

 QString error;
 if (streams.count() != streams2.count()) {
	error = i18n("Different streams count: %1, but should be %2").arg(streams2.count()).arg(streams.count());
	return error;
 }
 BoGameSyncMessage gameSync;
 error = gameSync.findError(streams["BosonStream"], streams2["BosonStream"]);
 if (!error.isNull()) {
	return error;
 }

 BoPathSyncMessage pathSync;
 error = pathSync.findError(streams["PathStream"], streams2["PathStream"]);
 if (!error.isNull()) {
	return error;
 }
 BoCanvasSyncMessage canvasSync;
 error = canvasSync.findError(streams["CanvasStream"], streams2["CanvasStream"]);
 if (!error.isNull()) {
	return error;
 }

 BoPlayerSyncMessage playerSync;
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

 unsigned int shortInterval = 10; // every 2,5s
 unsigned int longInterval = 100; // every 25s
 if (mAdvanceMessageCounter % shortInterval == 5) {
	QByteArray log = createLongSyncLog(canvas, mAdvanceMessageCounter, shortInterval);
	storeLogAndSend(log);
 }
 if (mAdvanceMessageCounter % longInterval == 50) {
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
		QByteArray correct = await->log();
		QByteArray broken = brokenLog;
		BoLongSyncMessage longSync;
		QString error = longSync.findError(correct, broken);
		addChatSystemMessage(i18n("Error message: %1").arg(error));
		boDebug(370) << k_funcinfo << "Error message: " << error << endl;
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


QByteArray BosonNetworkSynchronizer::createLongSyncLog(BosonCanvas* canvas, unsigned int advanceMessageCounter, unsigned int interval) const
{
 static int myProfilingId = boProfiling->requestEventId("CreateLongSyncLog");
 BosonProfiler profiler(myProfilingId);

 BoLongSyncMessage m;
 m.setGame(mGame);
 m.setMessageLogger(mMessageLogger);
 m.setCanvas(canvas, advanceMessageCounter, interval);
 return m.makeLog();
}

void BosonNetworkSynchronizer::addChatSystemMessage(const QString& msg)
{
 BO_CHECK_NULL_RET(mGame);
 mGame->slotAddChatSystemMessage(i18n("NetworkSync"), msg);
}







#undef DECLARE
#undef UNSTREAM
#undef DECLARE_UNSTREAM
#undef DECLARE_UNSTREAM_COMPARE
#undef DECLARE_UNSTREAM_COMPARE

