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

#ifndef BOSONPROFILINGPRIVATE_H
#define BOSONPROFILINGPRIVATE_H

#include "bosonprofiling.h"
#include "defines.h"

#include <qmap.h>
#include <qvaluelist.h>
#include <qptrlist.h>

#include <sys/time.h>

/**
 * Information about the rendering times. Helper class for @ref BosonProfiling.
 *
 * We avoid functions here in order to make it as fast as possible
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class RenderGLTimes
{
public:
	RenderGLTimes()
	{
		// i dont want to unroll this loop (even though it adds some
		// overhead), because it makes adding new variables more
		// difficult
		for (int i = 0; i < 2; i++) {
			timerclear(&mFunction[i]);
			timerclear(&mClear[i]);
			timerclear(&mCells[i]);
			timerclear(&mUnits[i]);
			timerclear(&mMissiles[i]);
			timerclear(&mParticles[i]);
			timerclear(&mFOW[i]);
			timerclear(&mText[i]);
		}
		mUnitCount = 0;
	}
	unsigned long int dFunction() const { return compareTimes2(mFunction); }
	unsigned long int dClear() const { return compareTimes2(mClear); }
	unsigned long int dCells() const { return compareTimes2(mCells); }
	unsigned long int dUnits() const { return compareTimes2(mUnits); }
	unsigned long int dMissiles() const { return compareTimes2(mMissiles); }
	unsigned long int dParticles() const { return compareTimes2(mParticles); }
	unsigned long int dFOW() const { return compareTimes2(mFOW); }
	unsigned long int dText() const { return compareTimes2(mText); }

	// we use array of size 2 - the first is the start time, the second the
	// stop time. the difference of both is the consumed time then.

	// remember to update operator>>() and operator<<() in the .cpp file if
	// you change something here!
	struct timeval mFunction[2];
	struct timeval mClear[2];
	struct timeval mCells[2];
	struct timeval mUnits[2];
	struct timeval mMissiles[2];
	struct timeval mParticles[2];
	struct timeval mFOW[2];
	struct timeval mText[2];
	unsigned int mUnitCount;
};

class ProfileSlotAdvance
{
public:
	ProfileSlotAdvance(unsigned int advanceCount)
	{
		for (int i = 0; i < 2; i++) {
			timerclear(&mFunction[i]);
			timerclear(&mAdvanceFunction[i]);
			timerclear(&mDeleteUnusedShots[i]);
			timerclear(&mParticles[i]);
			timerclear(&mMaximalAdvanceCount[i]);
		}
		mAdvanceCount = advanceCount;
	}
	unsigned long int dFunction() const { return compareTimes2(mFunction); }
	unsigned long int dAdvanceFunction() const { return compareTimes2(mAdvanceFunction); }
	unsigned long int dDeleteUnusedShots() const { return compareTimes2(mDeleteUnusedShots); }
	unsigned long int dParticles() const { return compareTimes2(mParticles); }
	unsigned long int dMaximalAdvanceCount() const { return compareTimes2(mMaximalAdvanceCount); }


	// remember to update operator>>() and operator<<() in the .cpp file if
	// you change something here!
	struct timeval mFunction[2]; // the entire slotAdvance() function
	struct timeval mAdvanceFunction[2]; // the advanceFunction()/advanceFunction2() stuff
	struct timeval mDeleteUnusedShots[2];
	struct timeval mParticles[2];
	struct timeval mMaximalAdvanceCount[2];
	unsigned int mAdvanceCount;
};


class BosonProfiling::BosonProfilingPrivate
{
public:
	BosonProfilingPrivate()
	{
		mCurrentRenderTimes = 0;
		mCurrentSlotAdvanceTimes = 0;
	}
	typedef QValueList<long int> TimesList;

	struct timeval mTimeLoadUnit;

	QMap<unsigned long int, TimesList> mUnitTimes;

	QPtrList<RenderGLTimes> mRenderTimes;
	RenderGLTimes* mCurrentRenderTimes;

	QPtrList<ProfileSlotAdvance> mSlotAdvanceTimes;
	ProfileSlotAdvance* mCurrentSlotAdvanceTimes;

	QMap<ProfilingEvent, struct timeval> mProfilingTimes;
	QMap<int, TimesList> mTimes;
};

#endif
