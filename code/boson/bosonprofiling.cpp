/*
    This file is part of the Boson game
    Copyright (C) 2002-2004 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bosonprofiling.h"
#include "bosonprofilingprivate.h"
#include "bodebug.h"
#include "bosonconfig.h"
#include "boglobal.h"
#include "defines.h"

#include <qfile.h>

static BoGlobalObject<BosonProfiling> globalProfiling(BoGlobalObjectBase::BoGlobalProfiling);

#define COMPARE_TIMES(time1, time2) ( ((time2.tv_sec - time1.tv_sec) * 1000000) + (time2.tv_usec - time1.tv_usec) )

#define PROFILING_VERSION 0x09 // increase if you change the file format of saved files!

unsigned long int compareTimes(const struct timeval& t1, const struct timeval& t2)
{
 return (((t2.tv_sec - t1.tv_sec) * 1000000) + (t2.tv_usec - t1.tv_usec));
}
unsigned long int compareTimes2(const struct timeval* t)
{
 if (!t) {
	return 0;
 }
 return compareTimes(t[0], t[1]);
}

QDataStream& operator<<(QDataStream& s, const struct timeval& t)
{
 s << t.tv_sec;
 s << t.tv_usec;
 return s;
}

QDataStream& operator>>(QDataStream& s, struct timeval& t)
{
 s >> t.tv_sec;
 s >> t.tv_usec;
 return s;
}
QDataStream& operator<<(QDataStream& s, const ProfilingEntry& e)
{
 s << e.mData[0];
 s << e.mData[1];
 return s;
}
QDataStream& operator>>(QDataStream& s, ProfilingEntry& e)
{
 s >> e.mData[0];
 s >> e.mData[1];
 return s;
}

QDataStream& operator<<(QDataStream& s, const RenderGLTimes& t)
{
 s << (Q_UINT32)t.mUnitCount;
 s << t.mClear;
 s << t.mCells;
 s << t.mUnits;
 s << t.mMissiles;
 s << t.mParticles;
 s << t.mFOW;
 s << t.mText;
 s << t.mFunction;
 return s;
}

QDataStream& operator>>(QDataStream& s, RenderGLTimes& t)
{
 Q_UINT32 unitCount;
 s >> unitCount;
 t.mUnitCount = unitCount;
 s >> t.mClear;
 s >> t.mCells;
 s >> t.mUnits;
 s >> t.mMissiles;
 s >> t.mParticles;
 s >> t.mFOW;
 s >> t.mText;
 s >> t.mFunction;
 return s;
}

QDataStream& operator<<(QDataStream& s, const ProfileItemAdvance& t)
{
 s << t.mFunction;
 s << t.mAdvance;
 s << t.mAdvanceFunction;
 s << t.mMove;
 s << (Q_INT32)t.mRtti;
 s << (Q_UINT32)t.mId;
 s << (Q_INT32)t.mWork;
 return s;
}
QDataStream& operator>>(QDataStream& s, ProfileItemAdvance& t)
{
 Q_INT32 rtti;
 Q_UINT32 id;
 Q_INT32 work;
 s >> t.mFunction;
 s >> t.mAdvance;
 s >> t.mAdvanceFunction;
 s >> t.mMove;
 s >> rtti;
 s >> id;
 s >> work;
 t.mRtti = rtti;
 t.mId = id;
 t.mWork = work;
 return s;
}

QDataStream& operator<<(QDataStream& s, const ProfileSlotAdvance& t)
{
 s << (Q_UINT32)t.mAdvanceCallsCount;
 s << t.mFunction;
 s << t.mAdvanceFunction;
 s << t.mDeleteUnusedShots;
 s << t.mEffects;
 s << t.mMaximalAdvanceCount;

 // now the items (mostly units):
 s << (Q_UINT32)t.mItems.count();
 QPtrListIterator<ProfileItemAdvance> it(t.mItems);
 for (; it.current(); ++it) {
	s << *it.current();
 }
 return s;
}

QDataStream& operator>>(QDataStream& s, ProfileSlotAdvance& t)
{
 Q_UINT32 advanceCallsCount;
 s >> advanceCallsCount;
 t.mAdvanceCallsCount = advanceCallsCount;
 s >> t.mFunction;
 s >> t.mAdvanceFunction;
 s >> t.mDeleteUnusedShots;
 s >> t.mEffects;
 s >> t.mMaximalAdvanceCount;

 // now the items (mostly units):
 Q_UINT32 items;
 s >> items;
 for (unsigned int i = 0; i < items; i++) {
	ProfileItemAdvance* item = new ProfileItemAdvance();
	s >> *item;
	t.mItems.append(item);
 }
 return s;
}


BosonProfiling::BosonProfiling()
{
 init();
}

BosonProfiling::BosonProfiling(const BosonProfiling& p)
{
 QByteArray buffer;
 QDataStream writeStream(buffer, IO_WriteOnly);
 p.save(writeStream);

 init();
 QDataStream readStream(buffer, IO_ReadOnly);
 load(readStream);
}

void BosonProfiling::init()
{
 d = new BosonProfilingPrivate;
 d->mRenderTimes.setAutoDelete(true);
 d->mSlotAdvanceTimes.setAutoDelete(true);
 d->mGLUpdateInterval = 0; // profiling logs can make use of the OpenGL update interval - makes anlalyzations easier
 d->mGameSpeed = -1;
 d->mVersion = PROFILING_VERSION;
 d->mNextDynamicEventId = 1;
}

BosonProfiling::~BosonProfiling()
{
 d->mRenderTimes.clear();
 delete d;
}

BosonProfiling* BosonProfiling::bosonProfiling()
{
 return BoGlobal::boGlobal()->bosonProfiling();
}

void BosonProfiling::loadUnit()
{
 gettimeofday(&d->mTimeLoadUnit, 0);
}

void BosonProfiling::loadUnitDone(unsigned long int typeId)
{
 struct timeval time2;
 gettimeofday(&time2, 0);
 long long time = COMPARE_TIMES(d->mTimeLoadUnit, time2);
 d->mUnitTimes[typeId].append(time);
}

void BosonProfiling::render(bool start)
{
 if (start) {
	if (d->mCurrentRenderTimes) {
		boError() << k_funcinfo << "current rendertime object is non-NULL" << endl;
		delete d->mCurrentRenderTimes;
	}
	d->mCurrentRenderTimes = new RenderGLTimes();
	d->mCurrentRenderTimes->mFunction.start();
 } else {
	d->mCurrentRenderTimes->mFunction.stop();
	if (d->mBenchmark) {
		d->mBenchmark->addRender(d->mCurrentRenderTimes);
	}
	d->mRenderTimes.append(d->mCurrentRenderTimes);
	if (d->mRenderTimes.count() > MAX_PROFILING_ENTRIES) {
		d->mRenderTimes.removeFirst();
	}
	d->mCurrentRenderTimes = 0;
 }
}

void BosonProfiling::renderClear(bool start)
{
 d->mCurrentRenderTimes->mClear.getTime(start);
}

void BosonProfiling::renderCells(bool start)
{
 d->mCurrentRenderTimes->mCells.getTime(start);
}

void BosonProfiling::renderUnits(bool start, unsigned int units)
{
 if (start) {
	d->mCurrentRenderTimes->mUnits.start();
 } else {
	d->mCurrentRenderTimes->mUnits.stop();
	d->mCurrentRenderTimes->mUnitCount = units;
 }
}

void BosonProfiling::renderMissiles(bool start)
{
 d->mCurrentRenderTimes->mMissiles.getTime(start);
}

void BosonProfiling::renderParticles(bool start)
{
 d->mCurrentRenderTimes->mParticles.getTime(start);
}

void BosonProfiling::renderFOW(bool start)
{
 d->mCurrentRenderTimes->mFOW.getTime(start);
}

void BosonProfiling::renderText(bool start)
{
 d->mCurrentRenderTimes->mText.getTime(start);
}

unsigned int BosonProfiling::renderEntries() const
{
 return d->mRenderTimes.count();
}

void BosonProfiling::advance(bool start, unsigned int advanceCallsCount)
{
 if (start) {
	if (d->mCurrentSlotAdvanceTimes) {
		boError() << k_funcinfo << "current SlotAdvanceTime object is non-NULL" << endl;
		delete d->mCurrentSlotAdvanceTimes;
	}
	d->mCurrentSlotAdvanceTimes = new ProfileSlotAdvance(advanceCallsCount);
	d->mCurrentSlotAdvanceTimes->mFunction.start();
 } else {
	d->mCurrentSlotAdvanceTimes->mFunction.stop();
	if (d->mBenchmark) {
		d->mBenchmark->addAdvance(d->mCurrentSlotAdvanceTimes);
	}
	d->mSlotAdvanceTimes.append(d->mCurrentSlotAdvanceTimes);
	if (d->mSlotAdvanceTimes.count() > MAX_PROFILING_ENTRIES) {
		d->mSlotAdvanceTimes.removeFirst();
	}
	if (d->mCurrentSlotAdvanceTimes->mAdvanceCallsCount != advanceCallsCount) {
		// the profiling data will be useless
		boError() << k_funcinfo << "Internal advance profiling error!! - advance count differs from expected advancecallscount!" << endl;
		d->mCurrentSlotAdvanceTimes->mAdvanceCallsCount = advanceCallsCount;
	}
	d->mCurrentSlotAdvanceTimes = 0;
 }
}

void BosonProfiling::advanceFunction(bool start)
{
 d->mCurrentSlotAdvanceTimes->mAdvanceFunction.getTime(start);
}

void BosonProfiling::advanceDeleteUnusedShots(bool start)
{
 d->mCurrentSlotAdvanceTimes->mDeleteUnusedShots.getTime(start);
}

void BosonProfiling::advanceEffects(bool start)
{
 d->mCurrentSlotAdvanceTimes->mEffects.getTime(start);
}

void BosonProfiling::advanceMaximalAdvanceCount(bool start)
{
 d->mCurrentSlotAdvanceTimes->mMaximalAdvanceCount.getTime(start);
}

void BosonProfiling::advanceItemStart(int rtti, unsigned int unitId, int work)
{
 d->mCurrentItemAdvanceTimes = new ProfileItemAdvance(rtti, unitId, work);
 d->mCurrentItemAdvanceTimes->mFunction.start();
}

void BosonProfiling::advanceItem(bool start)
{
 d->mCurrentItemAdvanceTimes->mAdvance.getTime(start);
}

void BosonProfiling::advanceItemFunction(bool start)
{
 d->mCurrentItemAdvanceTimes->mAdvanceFunction.getTime(start);
}

void BosonProfiling::advanceItemMove(bool start)
{
 d->mCurrentItemAdvanceTimes->mMove.getTime(start);
}

void BosonProfiling::advanceItemStop()
{
 d->mCurrentItemAdvanceTimes->mFunction.stop();
 d->mCurrentSlotAdvanceTimes->mItems.append(d->mCurrentItemAdvanceTimes);
 // AB: there is NO limit on unit number here!
 d->mCurrentItemAdvanceTimes = 0;
}

void BosonProfiling::start(int event)
{
 // AB: d->mProfilingTimes[event] is a map lookup and therefore pretty slow!
 gettimeofday(&d->mProfilingTimes[event], 0);
}

long int BosonProfiling::elapsed(int event) const
{
 struct timeval time;
 gettimeofday(&time, 0);
 // AB: d->mProfilingTimes[event] is a map lookup and therefore pretty slow!
 return COMPARE_TIMES(d->mProfilingTimes[event], time);
}

long int BosonProfiling::stop(int event, bool appendToList)
{
 long int _elapsed = elapsed(event);

 if (appendToList) {
	d->mTimes[event].append(_elapsed);
	if (d->mTimes[event].count() > MAX_PROFILING_ENTRIES) {
		d->mTimes[event].remove(d->mTimes[event].begin());
	}
 }
 return _elapsed;
}

int BosonProfiling::requestEventId(const QString& name)
{
 int id = BosonProfiling::LastFixedEventId + d->mNextDynamicEventId;
 d->mNextDynamicEventId++;
 if (!name.isEmpty()) {
	// note: this ID can NOT be used in the profiling dialog!
	d->mDynamicEventId2Name.insert(id, name);
 }
 return id;
}

QString BosonProfiling::eventName(int id) const
{
 if (!d->mDynamicEventId2Name.contains(id)) {
	return QString::null;
 }
 return d->mDynamicEventId2Name[id];
}

bool BosonProfiling::saveToFile(const QString& fileName)
{
 QFile file(fileName);
 if (!file.open(IO_WriteOnly)) {
	boError() << k_funcinfo << "Could not open " << fileName << " for writing" << endl;
	return false;
 }
 QDataStream stream(&file);
 return save(stream);
}

bool BosonProfiling::save(QDataStream& stream) const
{
 if (d->mVersion != PROFILING_VERSION) {
	boError() << k_funcinfo << "Can save new logs only! old logs can't be saved!" << endl;
	return false;
 }
 stream << QString::fromLatin1("BosonProfiling");
 stream << (Q_INT32)PROFILING_VERSION;

 stream << (Q_UINT32)d->mGLUpdateInterval;
 stream << (Q_INT32)d->mGameSpeed;

 stream << d->mUnitTimes;
 stream << d->mTimes;

 {
	// now the render times. we use a ptrlist here, so its more tricky
	stream << (Q_UINT32)d->mRenderTimes.count();
	QPtrListIterator<RenderGLTimes> it(d->mRenderTimes);
	for (; it.current(); ++it) {
		stream << *it.current();
	}
 }
 {
	// same about the advance times
	stream << (Q_UINT32)d->mSlotAdvanceTimes.count();
	QPtrListIterator<ProfileSlotAdvance> it(d->mSlotAdvanceTimes);
	for (; it.current(); ++it) {
		stream << *it.current();
	}
 }

 stream << (Q_INT32)d->mNextDynamicEventId;
 stream << d->mDynamicEventId2Name;

 return true;
}

bool BosonProfiling::loadFromFile(const QString& fileName)
{
 QFile file(fileName);
 if (!file.open(IO_ReadOnly)) {
	boError() << k_funcinfo << "Could not open " << fileName << " for reading" << endl;
	return false;
 }
 QDataStream stream(&file);
 bool ret = load(stream);
 if (!ret) {
	boError() << k_funcinfo << "Invalid file " << fileName << endl;
 }
 file.close();
 return ret;
}

bool BosonProfiling::load(QDataStream& stream)
{
 QString s;
 stream >> s;
 if (s != QString::fromLatin1("BosonProfiling")) {
	boError() << k_funcinfo << "Invalid stream - not a profiling stream" << endl;
	return false;
 }
 Q_INT32 version;
 stream >> version;
 if (version != PROFILING_VERSION && version < 0x06) {
	boError() << k_funcinfo << "Invalid profiling format version " << version << endl;
	return false;
 }
 d->mVersion = version;

 Q_UINT32 glUpdateInterval = 0;
 Q_INT32 gameSpeed = -1;
 if (version >= 0x07) {
	stream >> glUpdateInterval;
 }
 if (version >= 0x08) {
	stream >> gameSpeed;
 }
 d->mGLUpdateInterval = glUpdateInterval;
 d->mGameSpeed = gameSpeed;

 // from here on we assume the stream is ok
 d->mUnitTimes.clear();
 d->mRenderTimes.clear();
 d->mTimes.clear();

 stream >> d->mUnitTimes;
 stream >> d->mTimes;

 Q_UINT32 renderTimesCount;
 stream >> renderTimesCount;
 for (unsigned int i = 0; i < renderTimesCount; i++) {
	RenderGLTimes* t = new RenderGLTimes;
	stream >> *t;
	d->mRenderTimes.append(t);
 }

 Q_UINT32 slotAdvanceTimesCount;
 stream >> slotAdvanceTimesCount;
 for (unsigned int i = 0; i < slotAdvanceTimesCount; i++) {
	ProfileSlotAdvance* t = new ProfileSlotAdvance(0);
	stream >> *t;
	d->mSlotAdvanceTimes.append(t);
 }

 if (version >= 0x09) {
	Q_INT32 id;
	stream >> id;
	d->mNextDynamicEventId = id;
	stream >> d->mDynamicEventId2Name;
 }

 return true;
}

void BosonProfiling::setGLUpdateInterval(unsigned int ms)
{
 d->mGLUpdateInterval = ms;
}

unsigned int BosonProfiling::glUpdateInterval() const
{
 return d->mGLUpdateInterval;
}

void BosonProfiling::setGameSpeed(int speed)
{
 d->mGameSpeed = speed;
}

int BosonProfiling::gameSpeed() const
{
 return d->mGameSpeed;
}

void BosonProfiling::startBenchmark()
{
 boDebug() << k_funcinfo << endl;
 if (d->mBenchmark) {
	boError() << k_funcinfo << "Old benchmark exists! Deleting";
	delete d->mBenchmark;
 }
 d->mBenchmark = new ProfileBenchmark;
 d->mBenchmark->mInterval.start();
 boDebug() << k_funcinfo << "benchmark is now " << d->mBenchmark << endl;
}

void BosonProfiling::endBenchmark(const QString& name)
{
 boDebug() << k_funcinfo << endl;
 if (!d->mBenchmark) {
	boError() << k_funcinfo << "No benchmark" << endl;
	return;
 }

 d->mBenchmark->mInterval.stop();

 if (name == QString::null) {
	boDebug() << k_funcinfo << "BENCHMARK RESULTS:" << endl << endl;
 } else {
	boDebug() << k_funcinfo << "BENCHMARK RESULTS for " << name << ":" << endl << endl;
 }

 // Advance stuff
 boDebug() << "ADVANCE RESULTS:" << endl;
 boDebug() << d->mBenchmark->mAdvanceProfiles.count() << " advance calls were profiled" << endl;
 QValueList<ProfileSlotAdvance>::iterator advit;
 unsigned long int advfunction = 0, advancefunction = 0, deleteshots = 0, effects = 0, maximaladvancecount = 0;
 unsigned long int maxadvfunction = 0, maxadvancefunction = 0, maxdeleteshots = 0, maxeffects = 0, maxmaximaladvancecount = 0;
 unsigned long int count = 0;
 for (advit = d->mBenchmark->mAdvanceProfiles.begin(); advit != d->mBenchmark->mAdvanceProfiles.end(); ++advit) {
	advfunction += (*advit).dFunction();
	advancefunction += (*advit).dAdvanceFunction();
	deleteshots += (*advit).dDeleteUnusedShots();
	effects += (*advit).dEffects();
	maximaladvancecount += (*advit).dMaximalAdvanceCount();

	maxadvfunction = QMAX(maxadvfunction, (*advit).dFunction());
	maxadvancefunction = QMAX(maxadvancefunction, (*advit).dAdvanceFunction());
	maxdeleteshots = QMAX(maxdeleteshots, (*advit).dDeleteUnusedShots());
	maxeffects = QMAX(maxeffects, (*advit).dEffects());
	maxmaximaladvancecount = QMAX(maxmaximaladvancecount, (*advit).dMaximalAdvanceCount());

	count++;
 }
 (boDebug()).form("%10s |%15s |%15s |%15s |%15s |%15s",
		"", "function", "advancefunc", "deleteshots", "effects", "maxadvcount") << endl;
 (boDebug()).form("%10s |%15.3f |%15.3f |%15.3f |%15.3f |%15.3f",
		"TOTAL", advfunction / 1000.0, advancefunction / 1000.0, deleteshots / 1000.0,
		effects / 1000.0, maximaladvancecount / 1000.0) << endl;
 (boDebug()).form("%10s |%15.3f |%15.3f |%15.3f |%15.3f |%15.3f",
		"average", advfunction / 1000.0 / count, advancefunction / 1000.0 / count, deleteshots / 1000.0 / count,
		effects / 1000.0 / count, maximaladvancecount / 1000.0 / count) << endl;
 (boDebug()).form("%10s |%15.3f |%15.3f |%15.3f |%15.3f |%15.3f",
		"max", maxadvfunction / 1000.0, maxadvancefunction / 1000.0, maxdeleteshots / 1000.0,
		maxeffects / 1000.0, maxmaximaladvancecount / 1000.0) << endl << endl;

 // Render stuff
 boDebug() << "RENDER RESULTS:" << endl;
 boDebug() << d->mBenchmark->mRenderProfiles.count() << " rendered frames were profiled" << endl;
 QValueList<RenderGLTimes>::iterator glit;
 unsigned long int glfunction = 0, clear = 0, cells = 0, units = 0, glparticles = 0, text = 0;
 unsigned long int maxglfunction = 0, maxclear = 0, maxcells = 0, maxunits = 0, maxglparticles = 0, maxtext = 0;
 count = 0;
 for (glit = d->mBenchmark->mRenderProfiles.begin(); glit != d->mBenchmark->mRenderProfiles.end(); ++glit) {
	glfunction += (*glit).dFunction();
	clear += (*glit).dClear();
	cells += (*glit).dCells();
	units += (*glit).dUnits();
	glparticles += (*glit).dParticles();
	text += (*glit).dText();

	maxglfunction = QMAX(maxglfunction, (*glit).dFunction());
	maxclear = QMAX(maxclear, (*glit).dClear());
	maxcells = QMAX(maxcells, (*glit).dCells());
	maxunits = QMAX(maxunits, (*glit).dUnits());
	maxglparticles = QMAX(maxglparticles, (*glit).dParticles());
	maxtext = QMAX(maxtext, (*glit).dText());

	count++;
 }
 (boDebug()).form("%10s |%15s |%15s |%15s |%15s |%15s |%15s",
		"", "function", "clear", "cells", "items", "particles", "text") << endl;
 (boDebug()).form("%10s |%15.3f |%15.3f |%15.3f |%15.3f |%15.3f |%15.3f",
		"TOTAL", glfunction / 1000.0, clear / 1000.0, cells / 1000.0,
		units / 1000.0, glparticles / 1000.0, text / 1000.0) << endl;
 (boDebug()).form("%10s |%15.3f |%15.3f |%15.3f |%15.3f |%15.3f |%15.3f",
		"average", glfunction / 1000.0 / count, clear / 1000.0 / count, cells / 1000.0 / count,
		units / 1000.0 / count, glparticles / 1000.0 / count, text / 1000.0 / count) << endl;
 (boDebug()).form("%10s |%15.3f |%15.3f |%15.3f |%15.3f |%15.3f |%15.3f",
		"max", maxglfunction / 1000.0, maxclear / 1000.0, maxcells / 1000.0,
		maxunits / 1000.0, maxglparticles / 1000.0, maxtext / 1000.0) << endl;
 (boDebug()).form("%d frames were rendered in %.3f sec - average FPS was %.2f",
		count, d->mBenchmark->mInterval.diff() / 1000000.0, count / (d->mBenchmark->mInterval.diff() / 1000000.0)) << endl;


 delete d->mBenchmark;
 d->mBenchmark = 0;
}

