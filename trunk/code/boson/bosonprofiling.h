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
#ifndef BOSONPROFILING_H
#define BOSONPROFILING_H

#define boProfiling BosonProfiling::bosonProfiling()

class QString;
class QDataStream;
class BosonProfilingDialog;

/**
 * Information about the rendering times. Helper class for @ref BosonProfiling.
 *
 * We avoid functions here in order to make it as fast as possible
 **/
class RenderGLTimes
{
public:
	RenderGLTimes()
	{
		mClear = 0;
		mCells = 0;
		mUnits = 0;
		mUnitCount = 0;
		mMissiles = 0;
		mParticles = 0;
		mFOW = 0;
		mText = 0;
		mFunction = 0;
	}
	RenderGLTimes(const RenderGLTimes& c)
	{
		*this = c;
	}
	RenderGLTimes& operator=(const RenderGLTimes& c)
	{
		mClear = c.mClear;
		mCells = c.mCells;
		mUnits = c.mUnits;
		mUnitCount = c.mUnitCount;
		mMissiles = c.mMissiles;
		mParticles = c.mParticles;
		mFOW = c.mFOW;
		mText = c.mText;
		mFunction = c.mFunction;
		return *this;
	}

	// remember to update operator= and default c'tor if you change
	// something!
	// also update operator>>() and operator<<() in the .cpp file
	long int mClear;
	long int mCells;
	long int mUnits;
	unsigned int mUnitCount;
	long int mMissiles;
	long int mParticles;
	long int mFOW;
	long int mText;
	long int mFunction;
};

// note that there are several workarounds in this class to reduce the number of
// #includes as far as possible. i want to be able to place this header to about
// every other class without increasing compile-time.
// all QValueList,QMap,... #includes are in the .cpp file
/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonProfiling
{
public:
	enum ProfilingEvent {
		// note that all entries here need to be in order, so that we
		// can iterate them easily in a Profiling dialog.
		ProfilingStart = 0, // must remain the first entry!
		LoadGameData1,
		LoadGameData2,
		LoadGameData3,
		LoadModel,
		LoadModelTextures,
		LoadModelDisplayLists,
		LoadModelDummy,

		ProfilingEnd // must remain the last entry!
	};
	BosonProfiling();
	BosonProfiling(const BosonProfiling& profiling);
	~BosonProfiling();

	static void initProfiling();
	static BosonProfiling* bosonProfiling() { return mProfiling; }

	/**
	 * Start the timer for profiling. Note that nesting timers <em>are</em>
	 * possible, as long as you use different events. Example:
	 * <pre>
	 * boProfiling->start(0);
	 * doSomething();
	 * boProfiling->start(1);
	 * doMore();
	 * boProfiling->stop(1);
	 * doTheRest();
	 * boProfiling->stop(0);
	 * </pre>
	 * The two timers are completely independant of each other. However the
	 * two timers <em>must</em> have different events.
	 **/
	void start(ProfilingEvent event);

	/**
	 * Stop the event timer and append the resulting time to the list. If
	 * the list contains more than MAX_ENTRIES the first item is removed.
	 * See also @ref start
	 **/
	void stop(ProfilingEvent event);

	void loadUnit();
	void loadUnitDone(unsigned long int typeId);

	void render(bool start); // always call this first, before any other render*()
	// note that you must NOT use nested calls of render*()! e.g.
	// renderCells(true); renderUnits(true); renderUnits(false); renderCells(false);
	// would *NOT* work!
	void renderClear(bool start);
	void renderCells(bool start);
	void renderUnits(bool start, unsigned int number = 0);
	void renderMissiles(bool start);
	void renderParticles(bool start);
	void renderFOW(bool start);
	void renderText(bool start);
	void debugRender();

	bool saveToFile(const QString& fileName);
	bool loadFromFile(const QString& fileName);

	bool save(QDataStream& stream) const;
	bool load(QDataStream& stream);

private:
	class BosonProfilingPrivate;
	BosonProfilingPrivate* d;
	friend class BosonProfilingDialog;

	static BosonProfiling* mProfiling;
};

#endif
