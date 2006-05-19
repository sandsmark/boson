/*
    This file is part of the Boson game
    Copyright (C) 2002-2005 Andreas Beckermann (b_mann@gmx.de)

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

#include "bosonfpscounter.h"
#include "bosonfpscounter.moc"

#include <config.h>

#include "../bomemory/bodummymemory.h"
#include "bodebug.h"

#include <qptrlist.h>

#if HAVE_SYS_TIME_H
#include <sys/time.h>
#else
// won't compile anymore!
#warning You dont have sys/time.h - please report this problem to boson-devel@lists.sourceforge.net and provide us with information about your system!
#endif
#include <math.h>
#include <stdlib.h>
#include <unistd.h>


#define COMPARE_TIMES(time1, time2) ( ((time2.tv_sec - time1.tv_sec) * 1000000) + (time2.tv_usec - time1.tv_usec) )


class Frame {
public:
	Frame()
	{
		mTime = 0;
		mSkipped = false;
		gettimeofday(&mFrameStart, 0);
	}
	void calculateTime()
	{
		mTime = COMPARE_TIMES(mFrameEnd, mFrameStart);
	}
	struct timeval mFrameStart;
	struct timeval mFrameEnd;
	long long mTime;
	bool mSkipped;
};

class BosonFPSCounterPrivate
{
public:
	BosonFPSCounterPrivate()
	{
		mCurrentFrame = 0;
	}
	QPtrList<Frame> mFrameQueue;
	Frame* mCurrentFrame;

	double mFPSCache;
	double mSkippedFPSCache;
	struct timeval mCacheTime;
};

BosonFPSCounter::BosonFPSCounter(QObject* parent)
	: QObject(parent)
{
 d = new BosonFPSCounterPrivate();
 mMaximumAge = 20;
 d->mFPSCache = 0.0;
 d->mSkippedFPSCache = 0.0;
 d->mCacheTime.tv_sec = 0;
 d->mCacheTime.tv_usec = 0;

 reset();
}

BosonFPSCounter::~BosonFPSCounter()
{
 reset();
 delete d->mCurrentFrame;
 delete d;
}

void BosonFPSCounter::reset()
{
 if (d->mCurrentFrame) {
	boError() << k_funcinfo << "called while still rendering a frame" << endl;
 }
 delete d->mCurrentFrame;
 d->mCurrentFrame = 0;
 while (!d->mFrameQueue.isEmpty()) {
	Frame* f = d->mFrameQueue.take(0);
	delete f;
 }
 d->mSkippedFPSCache = 0.0;
 d->mCacheTime.tv_sec = 0;
 d->mCacheTime.tv_usec = 0;
}

double BosonFPSCounter::calculateFPS(unsigned int frames, const struct timeval& now, const struct timeval& since) const
{
 double diff = (double)(now.tv_sec - since.tv_sec)
		+ ((double)(now.tv_usec - since.tv_usec)) / 1000000;
 return ((double)frames) / diff;
}

unsigned int BosonFPSCounter::countFramesSince(const struct timeval& since, unsigned int* skippedFrames) const
{
 unsigned int frames = 0;
 QPtrListIterator<Frame> it(d->mFrameQueue);
 for (it.toLast(); it.current(); --it) {
	Frame* frame = it.current();
	if (frame->mFrameEnd.tv_sec >= since.tv_sec) {
		if (frame->mFrameEnd.tv_sec > since.tv_sec ||
				frame->mFrameEnd.tv_usec >= since.tv_usec) {
			if (frame->mSkipped) {
				if (skippedFrames) {
					*skippedFrames += 1;
				}
			} else {
				frames++;
			}
		}
	} else {
		// we reached a frame that is before 'since'. Therefore all
		// following frames are before 'since' as well (we do reverse
		// iteration). We can stop here.
		return frames;
	}
 }
 return frames;
}

double BosonFPSCounter::cachedFps(double* skippedPerSecond)
{
 struct timeval now;
 gettimeofday(&now, 0);
 if (d->mCacheTime.tv_sec > now.tv_sec) {
	// the cache timestamp got invalid somehow
	d->mCacheTime.tv_sec = 0;
	d->mCacheTime.tv_usec = 0;
 }

 const int updateInterval = 1; // in s
 if (now.tv_sec - d->mCacheTime.tv_sec >= updateInterval) {
	if (now.tv_sec - d->mCacheTime.tv_sec > updateInterval ||
			now.tv_usec - d->mCacheTime.tv_usec > 0) {
		d->mFPSCache = fps(&d->mSkippedFPSCache);
	}
 }
 if (skippedPerSecond) {
	*skippedPerSecond = d->mSkippedFPSCache;
 }
 return d->mFPSCache;
}

double BosonFPSCounter::fps(double* skippedPerSecond) const
{
 struct timeval t;
 gettimeofday(&t, 0);

 // we count all frames that have been rendered up to two seconds ago. this is
 // more accurate than counting one second only.
 //
 // we could increase accuracy of the average fps even further by increasing
 // this value, but it would then be the average fps only, not the "current"
 // fps. 2 should be a good value.
 t.tv_sec -= 2;
 return fps(t, skippedPerSecond);
}

double BosonFPSCounter::fps(const struct timeval& since, double* skippedPerSecond) const
{
 struct timeval now;
 gettimeofday(&now, 0);
 if (now.tv_sec - 1 < since.tv_sec) {
	boError() << k_funcinfo << "requested fps based on a period of "
			<< (now.tv_sec - since.tv_sec)
			<< " seconds - this is of little use and not supported"
			<< endl;
	return 0.0;
 }
 double seconds = (double)(now.tv_sec - since.tv_sec);

 // pretty imprecise, but that doesn't matter :-)
 // (taking seconds into account only, would already be sufficient
 seconds += ((double)(now.tv_usec - since.tv_usec)) / 1000000.0;

 unsigned int skippedFrames = 0;
 double frames = (double)countFramesSince(since, &skippedFrames);

 if (skippedPerSecond) {
	double skipped = (double)skippedFrames;
	*skippedPerSecond = (skipped / seconds);
 }
 return (frames / seconds);
}

void BosonFPSCounter::skipFrame()
{
 if (!d->mCurrentFrame) {
	boError() << k_funcinfo << "no frame started" << endl;
	return;
 }
 d->mCurrentFrame->mSkipped = true;
 emit signalSkipFrame();
}

void BosonFPSCounter::startFrame()
{
 if (d->mCurrentFrame) {
	boError() << k_funcinfo << "previous frame not yet ended!" << endl;
	return;
 }
 d->mCurrentFrame = new Frame();
}

void BosonFPSCounter::endFrame()
{
 if (!d->mCurrentFrame) {
	boError() << k_funcinfo << "no frame started" << endl;
	return;
 }
 gettimeofday(&d->mCurrentFrame->mFrameEnd, 0);
 d->mCurrentFrame->calculateTime();
 d->mFrameQueue.append(d->mCurrentFrame);

 cleanOldFrames();

 d->mCurrentFrame = 0;
}

void BosonFPSCounter::cleanOldFrames()
{
 struct timeval now;
 gettimeofday(&now, 0);
 while (d->mFrameQueue.getFirst() && d->mFrameQueue.getFirst()->mFrameEnd.tv_sec < now.tv_sec - maximumAge()) {
	Frame* f = d->mFrameQueue.take(0);
	delete f;
 }
}

long long BosonFPSCounter::timeSinceLastFrame(bool onlyNonSkippedFrames) const
{
 struct timeval now;
 gettimeofday(&now, 0);
 Frame* f = 0;
 QPtrListIterator<Frame> it(d->mFrameQueue);
 for (it.toLast(); it.current() && !f; --it) {
	if (onlyNonSkippedFrames && it.current()->mSkipped) {
		continue;
	}
	f = it.current();
 }
 if (f) {
	return (now.tv_sec - f->mFrameEnd.tv_sec )* 1000000 + (now.tv_usec - f->mFrameEnd.tv_usec);
 }

 // return a large value (10 minutes)
 return 10 * 60 * 1000000;
}


BosonGameFPSCounter::BosonGameFPSCounter(BosonFPSCounter* parent)
	: QObject(parent)
{
 mFPSCounter = parent;
}

void BosonGameFPSCounter::skipFrame()
{
 mFPSCounter->skipFrame();
}

double BosonGameFPSCounter::cachedFps(double* skippedPerSecond)
{
 return mFPSCounter->cachedFps(skippedPerSecond);
}

