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
class ProfilingEntry;
class BosonProfilingDialog;
class RenderGLTimes;
class ProfileSlotAdvance;
struct timeval;

class BosonProfilingPrivate;
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
		LoadGameData1 = 0, // currently unused
		LoadTiles = 1,
		LoadGameData3 = 2,
		LoadModel = 3,
		LoadModelTextures = 4,
		LoadModelDisplayLists = 5,
		LoadModelDummy = 6,
		AddUnitsXML = 7,
		SaveGameToXML = 20,
		SaveKGameToXML = 21,
		SavePlayersToXML = 22,
		SavePlayerToXML = 23,
		SavePlayFieldToXML = 24,
		SaveGameToXMLWriteFile = 25
	};
	BosonProfiling();
	BosonProfiling(const BosonProfiling& profiling);
	~BosonProfiling();

	static void initProfiling();
	static BosonProfiling* bosonProfiling() { return mProfiling; }

	/**
	 * Change the OpenGL update interval. This value can be useful when
	 * analyzing profiling logs.
	 **/
	void setGLUpdateInterval(unsigned int interval);

	/**
	 * @return The OpenGL update interval. Be careful with this value - when
	 * the interval was changed then you can end up in one half of the
	 * profiling log to be with the old interval, the second half with the
	 * new interval - but only the new interval is recorded into the log.
	 **/
	unsigned int glUpdateInterval() const;

	/**
	 * Change the game speed. Can be useful for analyzing profiling logs.
	 **/
	void setGameSpeed(int gameSpeed);

	/**
	 * @return The game speed when the log was recorded. Be careful with
	 * this value - the same problem as with @ref glUpdateInterval applies
	 * to this one!
	 **/
	int gameSpeed() const;

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
	void start(int event);

	/**
	 * Stop the event timer and append the resulting time to the list. If
	 * the list contains more than MAX_ENTRIES the first item is removed.
	 * See also @ref start
	 **/
	void stop(int event);

	void loadUnit();
	void loadUnitDone(unsigned long int typeId);

	// WARNING: do !NOT! call render*() or advance*() before you called
	// render(true)/advance(true) or after render(false)/advance(false) !
	// that would crash! (no NULL check in favor of performance)
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

	/**
	 * @return The number of recoreded frames
	 **/
	unsigned int renderEntries() const;

	// AB: the syntax of these is the same as for render() above. e.g. you
	// mustn't call the advance*() stuff here recursive or so
	void advance(bool start, unsigned int advanceCount);
	void advanceFunction(bool start);
	void advanceDeleteUnusedShots(bool start);
	void advanceParticles(bool start);
	void advanceMaximalAdvanceCount(bool start); // in MAXIMAL_ADVANCE_COUNT we do some interesting stuff (especially deleting unused stuff - e.g. wreckages
	// these are tricky now - they get called multiple times in every
	// advance call (note that this is a pretty big overhead!)
	void advanceItemStart(int rtti, unsigned int unitId, int work); // begin a new unit
	void advanceItem(bool start);
	void advanceItemFunction(bool start);
	void advanceItemMove(bool start);
	void advanceItemStop(); // complete a unit

	/**
	 * Save The current profiling data to @p fileName.
	 **/
	bool saveToFile(const QString& fileName);

	/**
	 * Save The current profiling data from @p fileName.
	 **/
	bool loadFromFile(const QString& fileName);

	/**
	 * Save the current profiling data to @p stream
	 **/
	bool save(QDataStream& stream) const;

	/**
	 * Load the current profiling data from @p stream, which must have been
	 * saved using @ref save
	 **/
	bool load(QDataStream& stream);

private:
	void init();

private:
	BosonProfilingPrivate* d;
	friend class BosonProfilingDialog;

	static BosonProfiling* mProfiling;
};

unsigned long int compareTimes(const struct timeval& t1, const struct timeval& t2);
unsigned long int compareTimes2(const struct timeval* t1); // takes an array of 2 and compares them
QDataStream& operator<<(QDataStream& s, const struct timeval& t);
QDataStream& operator>>(QDataStream& s, struct timeval& t);
QDataStream& operator<<(QDataStream& s, const ProfilingEntry& e);
QDataStream& operator>>(QDataStream& s, ProfilingEntry& e);
QDataStream& operator<<(QDataStream& s, const RenderGLTimes& t);
QDataStream& operator>>(QDataStream& s, RenderGLTimes& t);
QDataStream& operator<<(QDataStream& s, const ProfileSlotAdvance& t);
QDataStream& operator>>(QDataStream& s, ProfileSlotAdvance& t);


/**
 * This class can help you to profile certaint events. You provide the number of
 * the event, just like you would do with @ref BosonProfiling::start. The
 * constructor of this class then calls @ref BosonProfiling::start and the
 * destructor will call @ref BosonProfiling::stop.
 *
 * This can be helpful if you have a few return statements in that function and
 * don't want to call @ref BosonProfiling::stop on your own whenever the
 * function returns. Create an object on the stack and it will get destroyed
 * once the functions returns. Example:
 * <pre>
 * void profileMe()
 * {
 *  BosonProfiler profiler(MyProfilingEvent);
 *  doFunnyStuff();
 *  doEvenMoreStuff();
 *  if (weWantToReturn()) {
 *      return;
 *  }
 *  doAgainALotOfStuff();
 * }
 * </pre>
 * The event MyProfilingEvent will now give useful data on the time spent in
 * profileMe().
 *
 * If you have a better name for this class: feel free to tell me!
 **/
class BosonProfiler
{
public:
	BosonProfiler(int event)
		: mEvent(event),
		mEventStopped(false)
	{
		boProfiling->start(mEvent);
	}
	~BosonProfiler()
	{
		if (!mEventStopped) {
			boProfiling->stop(mEvent);
		}
	}

	/**
	 * This stops profiling the event immediately. The destructor will do
	 * nothing when you call this.
	 **/
	void stop()
	{
		boProfiling->stop(mEvent);
		mEventStopped = true;
	}

private:
	int mEvent;
	bool mEventStopped;
};


#endif
