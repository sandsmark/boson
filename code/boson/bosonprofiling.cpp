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

#include <sys/time.h>

#include <qmap.h>
#include <qvaluelist.h>

#include <kstaticdeleter.h>
#include <kdebug.h>

static KStaticDeleter<BosonProfiling> sd;
BosonProfiling* BosonProfiling::mProfiling = 0;

#define COMPARE_TIMES(time1, time2) ( ((time2.tv_sec - time1.tv_sec) * 1000000) + (time2.tv_usec - time1.tv_usec) )

class BosonProfiling::BosonProfilingPrivate
{
public:
	BosonProfilingPrivate()
	{
	}
	typedef QValueList<long int> TimesList;

	struct timeval mTimeLoadUnit;
	struct timeval mTimeRenderFunction; // entire function
	struct timeval mTimeRenderPart; // a part of the function

	QMap<int, TimesList> mUnitTimes;
	QValueList<RenderGLTimes> mRenderTimes;
	RenderGLTimes mCurrentRenderTimes;
};

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

void BosonProfiling::loadUnitDone(int typeId)
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

	// TODO: limit the size of the list (100-200 or so)
	d->mRenderTimes.append(d->mCurrentRenderTimes);
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

void BosonProfiling::renderUnits(bool start)
{
 if (start) {
	gettimeofday(&d->mTimeRenderPart, 0);
 } else {
	struct timeval time;
	gettimeofday(&time, 0);
	d->mCurrentRenderTimes.mUnits = COMPARE_TIMES(d->mTimeRenderPart, time);
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

int BosonProfiling::renderCount() const
{
 return d->mRenderTimes.count();
}

RenderGLTimes BosonProfiling::renderTimes(unsigned int i) const
{
 if (i >= d->mRenderTimes.count()) {
	kdError() << k_funcinfo << "only " << d->mRenderTimes.count() << " elements!" << endl;
	return RenderGLTimes();
 }
 return d->mRenderTimes[i];
}

