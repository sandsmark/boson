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

#include <kstaticdeleter.h>
#include <kdebug.h>

#include <qfile.h>

static KStaticDeleter<BosonProfiling> sd;
BosonProfiling* BosonProfiling::mProfiling = 0;

#define COMPARE_TIMES(time1, time2) ( ((time2.tv_sec - time1.tv_sec) * 1000000) + (time2.tv_usec - time1.tv_usec) )

#define MAX_ENTRIES 300
#define PROFILING_VERSION 0x01 // increase if you change the file format of saved files!

QDataStream& operator<<(QDataStream& s, const RenderGLTimes& t)
{
 s << (Q_LONG)t.mClear;
 s << (Q_LONG)t.mCells;
 s << (Q_LONG)t.mUnits;
 s << (Q_UINT32)t.mUnitCount;
 s << (Q_LONG)t.mFOW;
 s << (Q_LONG)t.mText;
 s << (Q_LONG)t.mFunction;
 return s;
}

QDataStream& operator>>(QDataStream& s, RenderGLTimes& t)
{
 Q_LONG clear;
 Q_LONG cells;
 Q_LONG units;
 Q_UINT32 unitCount;
 Q_LONG fow;
 Q_LONG text;
 Q_LONG function;
 s >> clear;
 s >> cells;
 s >> units;
 s >> unitCount;
 s >> fow;
 s >> text;
 s >> function;
 t.mClear = clear;
 t.mCells = cells;
 t.mUnits = units;
 t.mUnitCount = unitCount;
 t.mFOW = fow;
 t.mText = text;
 t.mFunction = function;
 return s;
}

BosonProfiling::BosonProfiling()
{
 d = new BosonProfilingPrivate;
}

BosonProfiling::~BosonProfiling()
{
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
	gettimeofday(&d->mTimeRenderFunction, 0);
 } else {
	struct timeval time;
	gettimeofday(&time, 0);
	d->mCurrentRenderTimes.mFunction = COMPARE_TIMES(d->mTimeRenderFunction, time);
	d->mRenderTimes.append(d->mCurrentRenderTimes);
	if (d->mRenderTimes.count() >= MAX_ENTRIES) {
		d->mRenderTimes.remove(d->mRenderTimes.begin());
	}
 }
}

void BosonProfiling::renderClear(bool start)
{
 if (start) {
	gettimeofday(&d->mTimeRenderPart, 0);
 } else {
	struct timeval time;
	gettimeofday(&time, 0);
	d->mCurrentRenderTimes.mClear = COMPARE_TIMES(d->mTimeRenderPart, time);
 }
}

void BosonProfiling::renderCells(bool start)
{
 if (start) {
	gettimeofday(&d->mTimeRenderPart, 0);
 } else {
	struct timeval time;
	gettimeofday(&time, 0);
	d->mCurrentRenderTimes.mCells = COMPARE_TIMES(d->mTimeRenderPart, time);
 }
}

void BosonProfiling::renderUnits(bool start, unsigned int units)
{
 if (start) {
	gettimeofday(&d->mTimeRenderPart, 0);
 } else {
	struct timeval time;
	gettimeofday(&time, 0);
	d->mCurrentRenderTimes.mUnits = COMPARE_TIMES(d->mTimeRenderPart, time);
	d->mCurrentRenderTimes.mUnitCount = units;
 }
}

void BosonProfiling::renderFOW(bool start)
{
 if (start) {
	gettimeofday(&d->mTimeRenderPart, 0);
 } else {
	struct timeval time;
	gettimeofday(&time, 0);
	d->mCurrentRenderTimes.mFOW = COMPARE_TIMES(d->mTimeRenderPart, time);
 }
}

void BosonProfiling::renderText(bool start)
{
 if (start) {
	gettimeofday(&d->mTimeRenderPart, 0);
 } else {
	struct timeval time;
	gettimeofday(&time, 0);
	d->mCurrentRenderTimes.mText = COMPARE_TIMES(d->mTimeRenderPart, time);
 }
}

void BosonProfiling::debugRender()
{
 kdDebug()
		<< "Clear: " << d->mCurrentRenderTimes.mClear << endl
		<< "Cells: " << d->mCurrentRenderTimes.mCells << endl
		<< "Units: " << d->mCurrentRenderTimes.mUnits << endl
		<< "Text:  " << d->mCurrentRenderTimes.mText << endl
		<< "Function: " << d->mCurrentRenderTimes.mFunction << endl;
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
 if (d->mTimes[event].count() >= MAX_ENTRIES) {
	d->mTimes[event].remove(d->mTimes[event].begin());
 }
}


bool BosonProfiling::saveToFile(const QString& fileName)
{
 QFile file(fileName);
 if (!file.open(IO_WriteOnly)) {
	kdError() << k_funcinfo << "Could not open " << fileName << " for writing" << endl;
	return false;
 }
 QDataStream stream(&file);
 stream << QString::fromLatin1("BosonProfiling");
 stream << (Q_INT32)PROFILING_VERSION;

 stream << d->mUnitTimes;
 stream << d->mRenderTimes;
 stream << d->mTimes;
 return true;
}

bool BosonProfiling::loadFromFile(const QString& fileName)
{
 QFile file(fileName);
 if (!file.open(IO_ReadOnly)) {
	kdError() << k_funcinfo << "Could not open " << fileName << " for reading" << endl;
	return false;
 }
 QDataStream stream(&file);
 QString s;
 stream >> s;
 if (s != QString::fromLatin1("BosonProfiling")) {
	kdError() << k_funcinfo << "Invalid file " << fileName << endl;
	file.close();
	return false;
 }
 Q_INT32 version;
 stream >> version;
 if (version != PROFILING_VERSION) {
	kdError() << k_funcinfo << "Invalid file version " << version << endl;
	file.close();
	return false;
 }

 // from here on we assume the file is ok
 d->mUnitTimes.clear();
 d->mRenderTimes.clear();
 d->mTimes.clear();

 stream >> d->mUnitTimes;
 stream >> d->mRenderTimes;
 stream >> d->mTimes;
 return true;
}

