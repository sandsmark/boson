/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include <kstaticdeleter.h>

#include <qfile.h>

static KStaticDeleter<BosonProfiling> sd;
BosonProfiling* BosonProfiling::mProfiling = 0;

#define COMPARE_TIMES(time1, time2) ( ((time2.tv_sec - time1.tv_sec) * 1000000) + (time2.tv_usec - time1.tv_usec) )

#define MAX_ENTRIES 300
#define PROFILING_VERSION 0x05 // increase if you change the file format of saved files!

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

QDataStream& operator<<(QDataStream& s, const RenderGLTimes& t)
{
 s << (Q_UINT32)t.mUnitCount;
 for (int i = 0; i < 2; i++) {
	s << t.mClear[i];
	s << t.mCells[i];
	s << t.mUnits[i];
	s << t.mMissiles[i];
	s << t.mParticles[i];
	s << t.mFOW[i];
	s << t.mText[i];
	s << t.mFunction[i];
 }
 return s;
}

QDataStream& operator>>(QDataStream& s, RenderGLTimes& t)
{
 Q_UINT32 unitCount;
 s >> unitCount;
 t.mUnitCount = unitCount;
 for (int i = 0; i < 2; i++) {
	s >> t.mClear[i];
	s >> t.mCells[i];
	s >> t.mUnits[i];
	s >> t.mMissiles[i];
	s >> t.mParticles[i];
	s >> t.mFOW[i];
	s >> t.mText[i];
	s >> t.mFunction[i];
  }
 return s;
}

QDataStream& operator<<(QDataStream& s, const ProfileSlotAdvance& t)
{
 s << (Q_UINT32)t.mAdvanceCount;
 for (int i = 0; i < 2; i++) {
	s << t.mFunction[i];
	s << t.mAdvanceFunction[i];
	s << t.mDeleteUnusedShots[i];
	s << t.mParticles[i];
	s << t.mMaximalAdvanceCount[i];
 }
 return s;
}

QDataStream& operator>>(QDataStream& s, ProfileSlotAdvance& t)
{
 Q_UINT32 advanceCount;
 s >> advanceCount;
 t.mAdvanceCount = advanceCount;
 for (int i = 0; i < 2; i++) {
	s >> t.mFunction[i];
	s >> t.mAdvanceFunction[i];
	s >> t.mDeleteUnusedShots[i];
	s >> t.mParticles[i];
	s >> t.mMaximalAdvanceCount[i];
 }
 return s;
}


BosonProfiling::BosonProfiling()
{
 d = new BosonProfilingPrivate;
 d->mRenderTimes.setAutoDelete(true);
 d->mSlotAdvanceTimes.setAutoDelete(true);
}

BosonProfiling::BosonProfiling(const BosonProfiling& p)
{
 QByteArray buffer;
 QDataStream writeStream(buffer, IO_WriteOnly);
 p.save(writeStream);

 d = new BosonProfilingPrivate;
 QDataStream readStream(buffer, IO_ReadOnly);
 load(readStream);
}

BosonProfiling::~BosonProfiling()
{
 d->mRenderTimes.clear();
 delete d;
}

void BosonProfiling::initProfiling()
{
 if (mProfiling) {
	return;
 }
 sd.setObject(mProfiling, new BosonProfiling);
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
	gettimeofday(&d->mCurrentRenderTimes->mFunction[0], 0);
 } else {
	gettimeofday(&d->mCurrentRenderTimes->mFunction[1], 0);
	d->mRenderTimes.append(d->mCurrentRenderTimes);
	if (d->mRenderTimes.count() > MAX_ENTRIES) {
		d->mRenderTimes.removeFirst();
	}
	d->mCurrentRenderTimes = 0;
 }
}

void BosonProfiling::renderClear(bool start)
{
 gettimeofday(&d->mCurrentRenderTimes->mClear[start ? 0 : 1], 0);
}

void BosonProfiling::renderCells(bool start)
{
 gettimeofday(&d->mCurrentRenderTimes->mCells[start ? 0 : 1], 0);
}

void BosonProfiling::renderUnits(bool start, unsigned int units)
{
 if (start) {
	gettimeofday(&d->mCurrentRenderTimes->mUnits[0], 0);
 } else {
	gettimeofday(&d->mCurrentRenderTimes->mUnits[1], 0);
	d->mCurrentRenderTimes->mUnitCount = units;
 }
}

void BosonProfiling::renderMissiles(bool start)
{
 gettimeofday(&d->mCurrentRenderTimes->mMissiles[start ? 0 : 1], 0);
}

void BosonProfiling::renderParticles(bool start)
{
 gettimeofday(&d->mCurrentRenderTimes->mParticles[start ? 0 : 1], 0);
}

void BosonProfiling::renderFOW(bool start)
{
 gettimeofday(&d->mCurrentRenderTimes->mFOW[start ? 0 : 1], 0);
}

void BosonProfiling::renderText(bool start)
{
 gettimeofday(&d->mCurrentRenderTimes->mText[start ? 0 : 1], 0);
}

void BosonProfiling::advance(bool start, unsigned int advanceCount)
{
 if (start) {
	if (d->mCurrentSlotAdvanceTimes) {
		boError() << k_funcinfo << "current SlotAdvanceTime object is non-NULL" << endl;
		delete d->mCurrentSlotAdvanceTimes;
	}
	d->mCurrentSlotAdvanceTimes = new ProfileSlotAdvance(advanceCount);
	gettimeofday(&d->mCurrentSlotAdvanceTimes->mFunction[0], 0);
 } else {
	gettimeofday(&d->mCurrentSlotAdvanceTimes->mFunction[1], 0);
	d->mSlotAdvanceTimes.append(d->mCurrentSlotAdvanceTimes);
	if (d->mSlotAdvanceTimes.count() > MAX_ENTRIES) {
		d->mSlotAdvanceTimes.removeFirst();
	}
	if (d->mCurrentSlotAdvanceTimes->mAdvanceCount != advanceCount) {
		// the profiling data will be useless
		boError() << k_funcinfo << "Internal advance profiling error!! - advance count differs from expected advancecount!" << endl;
		d->mCurrentSlotAdvanceTimes->mAdvanceCount = advanceCount;
	}
	d->mCurrentSlotAdvanceTimes = 0;
 }
}

void BosonProfiling::advanceFunction(bool start)
{
 gettimeofday(&d->mCurrentSlotAdvanceTimes->mAdvanceFunction[start ? 0 : 1], 0);
}

void BosonProfiling::advanceDeleteUnusedShots(bool start)
{
 gettimeofday(&d->mCurrentSlotAdvanceTimes->mDeleteUnusedShots[start ? 0 : 1], 0);
}

void BosonProfiling::advanceParticles(bool start)
{
 gettimeofday(&d->mCurrentSlotAdvanceTimes->mParticles[start ? 0 : 1], 0);
}

void BosonProfiling::advanceMaximalAdvanceCount(bool start)
{
 gettimeofday(&d->mCurrentSlotAdvanceTimes->mMaximalAdvanceCount[start ? 0 : 1], 0);
}

void BosonProfiling::start(ProfilingEvent event)
{
 gettimeofday(&d->mProfilingTimes[event], 0);
}

void BosonProfiling::stop(ProfilingEvent event)
{
 struct timeval time;
 gettimeofday(&time, 0);

 d->mTimes[event].append(COMPARE_TIMES(d->mProfilingTimes[event], time));
 if (d->mTimes[event].count() > MAX_ENTRIES) {
	d->mTimes[event].remove(d->mTimes[event].begin());
 }
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
 stream << QString::fromLatin1("BosonProfiling");
 stream << (Q_INT32)PROFILING_VERSION;

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
 if (version != PROFILING_VERSION) {
	boError() << k_funcinfo << "Invalid profiling format version " << version << endl;
	return false;
 }

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
	ProfileSlotAdvance* t = new ProfileSlotAdvance(MAXIMAL_ADVANCE_COUNT + 10); // invalid number - will get replaced below
	stream >> *t;
	d->mSlotAdvanceTimes.append(t);
 }
return true;
}

