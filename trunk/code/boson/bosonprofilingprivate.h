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

#ifndef BOSONPROFILINGPRIVATE_H
#define BOSONPROFILINGPRIVATE_H

#include "bosonprofiling.h"
#include "defines.h"

#include <klocale.h>

#include <qmap.h>
#include <qvaluelist.h>
#include <qptrlist.h>

#include <sys/time.h>

class ProfilingEntry
{
public:
	ProfilingEntry()
	{
		clear();
	}
	void clear()
	{
		timerclear(&mData[0]);
		timerclear(&mData[1]);
	}
	unsigned long int diff() const { return compareTimes2(mData); }
	inline void getTime(bool start) { gettimeofday(&mData[start ? 0 : 1], 0); }
	inline void start() { gettimeofday(&mData[0], 0); }
	inline void stop() { gettimeofday(&mData[1], 0); }

	struct timeval mData[2];
};

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
		mUnitCount = 0;
	}
	QValueList<unsigned long int> values() const
	{
		QValueList<unsigned long int> values;
		values.append(mFunction.diff()); // MUST be the first entry!
		values.append(mUnits.diff()); // MUST be the second entry!
		values.append(mClear.diff());
		values.append(mCells.diff());
		values.append(mMissiles.diff());
		values.append(mParticles.diff());
		values.append(mFOW.diff());
		values.append(mText.diff());
		return values;
	}
	static QValueList<QString> names()
	{
		// note that count AND order must match count and order of
		// values() exactly!
		QValueList<QString> names;
		names.append(i18n("Function"));
		names.append(i18n("Units"));
		names.append(i18n("Clearing"));
		names.append(i18n("Cells"));
		names.append(i18n("Missiles"));
		names.append(i18n("Particles"));
		names.append(i18n("FOW"));
		names.append(i18n("Text"));
		return names;
	}

	unsigned long int dFunction() const { return mFunction.diff(); }
	unsigned long int dClear() const { return mClear.diff(); }
	unsigned long int dCells() const { return mCells.diff(); }
	unsigned long int dUnits() const { return mUnits.diff(); }
	unsigned long int dMissiles() const { return mMissiles.diff(); }
	unsigned long int dParticles() const { return mParticles.diff(); }
	unsigned long int dFOW() const { return mFOW.diff(); }
	unsigned long int dText() const { return mText.diff(); }

	// we use array of size 2 - the first is the start time, the second the
	// stop time. the difference of both is the consumed time then.

	// remember to update operator>>() and operator<<() in the .cpp file if
	// you change something here!
	ProfilingEntry mFunction;
	ProfilingEntry mClear;
	ProfilingEntry mCells;
	ProfilingEntry mUnits;
	ProfilingEntry mMissiles;
	ProfilingEntry mParticles;
	ProfilingEntry mFOW;
	ProfilingEntry mText;
	unsigned int mUnitCount;
};

class ProfileItemAdvance
{
public:
	ProfileItemAdvance()
	{
	}
	ProfileItemAdvance(int rtti, unsigned int id, int work)
	{
		mRtti = rtti;
		mId = id;
		mWork = work;
	}
	QValueList<unsigned long int> values() const
	{
		QValueList<unsigned long int> values;
		values.append(mFunction.diff()); // MUST be the first entry!
		values.append(mAdvance.diff());
		values.append(mAdvanceFunction.diff());
		values.append(mMove.diff());
		return values;
	}
	static QValueList<QString> names()
	{
		// note that count AND order must match count and order of
		// values() exactly!
		QValueList<QString> names;
		names.append(i18n("Function"));
		names.append(i18n("Advance"));
		names.append(i18n("AdvanceFunction"));
		names.append(i18n("Move"));
		return names;
	}

	ProfilingEntry mFunction;
	ProfilingEntry mAdvance;
	ProfilingEntry mAdvanceFunction;
	ProfilingEntry mMove;
	int mRtti;
	unsigned int mId;
	int mWork;
};

class ProfileSlotAdvance
{
public:
	ProfileSlotAdvance(unsigned int advanceCallsCount)
	{
		mAdvanceCallsCount = advanceCallsCount;
		mItems.setAutoDelete(true);
	}
	// Dummy ctor for QValueList. You shouldn't normally use it
	ProfileSlotAdvance()
	{
		mItems.setAutoDelete(true);
	}
	unsigned long int dFunction() const { return mFunction.diff(); }
	unsigned long int dAdvanceFunction() const { return mAdvanceFunction.diff(); }
	unsigned long int dDeleteUnusedShots() const { return mDeleteUnusedShots.diff(); }
	unsigned long int dEffects() const { return mEffects.diff(); }
	unsigned long int dMaximalAdvanceCount() const { return mMaximalAdvanceCount.diff(); }


	QValueList<unsigned long int> values() const
	{
		QValueList<unsigned long int> values;
		values.append(dFunction()); // MUST be the first entry!
		values.append(dAdvanceFunction());
		values.append(dDeleteUnusedShots());
		values.append(dEffects());
		values.append(dMaximalAdvanceCount());
		return values;
	}
	static QValueList<QString> names()
	{
		// note that count AND order must match count and order of
		// values() exactly!
		QValueList<QString> names;
		names.append(i18n("Function"));
		names.append(i18n("Advance Function"));
		names.append(i18n("Delete Unused Shots"));
		names.append(i18n("Effects"));
		names.append(i18n("MaximalAdvanceCount"));
		return names;
	}


	// remember to update operator>>() and operator<<() in the .cpp file if
	// you change something here!
	ProfilingEntry mFunction; // the entire slotAdvance() function
	ProfilingEntry mAdvanceFunction; // the advanceFunction()/advanceFunction2() stuff
	ProfilingEntry mDeleteUnusedShots;
	ProfilingEntry mEffects;
	ProfilingEntry mMaximalAdvanceCount;
	unsigned int mAdvanceCallsCount;

	QPtrList<ProfileItemAdvance> mItems;
};

class ProfileBenchmark
{
public:
	void addAdvance(ProfileSlotAdvance* adv)
	{
		mAdvanceProfiles.append(*adv);
	}
	void addRender(RenderGLTimes* gl)
	{
		mRenderProfiles.append(*gl);
	}

	QValueList<ProfileSlotAdvance> mAdvanceProfiles;
	QValueList<RenderGLTimes> mRenderProfiles;
	ProfilingEntry mInterval;
};

class BosonProfilingPrivate
{
public:
	BosonProfilingPrivate()
	{
		mCurrentRenderTimes = 0;
		mCurrentSlotAdvanceTimes = 0;
		mCurrentItemAdvanceTimes = 0;
		mBenchmark = 0;
	}
	typedef QValueList<long int> TimesList;

	struct timeval mTimeLoadUnit;

	QMap<unsigned long int, TimesList> mUnitTimes;

	QPtrList<RenderGLTimes> mRenderTimes;
	RenderGLTimes* mCurrentRenderTimes;

	QPtrList<ProfileSlotAdvance> mSlotAdvanceTimes;
	ProfileSlotAdvance* mCurrentSlotAdvanceTimes;
	ProfileItemAdvance* mCurrentItemAdvanceTimes;

	QMap<int, struct timeval> mProfilingTimes;
	QMap<int, TimesList> mTimes;
	int mNextDynamicEventId;
	QMap<int, QString> mDynamicEventId2Name;

	unsigned int mGLUpdateInterval;
	int mGameSpeed;
	int mVersion;

	ProfileBenchmark* mBenchmark;
};

#endif
