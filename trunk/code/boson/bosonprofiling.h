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
		BosonStartingStart = 8,
		SaveGameToXML = 20,
		SaveKGameToXML = 21,
		SavePlayersToXML = 22,
		SavePlayerToXML = 23,
		SavePlayFieldToXML = 24,
		SaveGameToXMLWriteFile = 25,
		SaveCanvasToXML = 26,
		SaveExternalToXML = 27,
		PreLoadPlayFields = 40,
		LoadPlayField = 41,
		FindPath = 100,
		WriteGameLog = 200,
		SaveGameLogs = 201,
		MakeGameLog = 202,
		GenerateLOD = 300,
		BuildLOD = 301,


		LastFixedEventId = 5000000
		// do not add any entries after this point
	};
	BosonProfiling();
	BosonProfiling(const BosonProfiling& profiling);
	~BosonProfiling();

	/**
	 * @return BoGlobal::boGlobal()->bosonProfiling();
	 **/
	static BosonProfiling* bosonProfiling();

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
	 * Sample use:
	 * <pre>
	 * static int myId = boProfiling->requestEventId("MyEvent");
	 * boProfiling->start(myId);
	 * doSomeStuff();
	 * boProfiling->stop(myId);
	 * </pre>
	 *
	 * Note that you should use static in your code for myId, so that it
	 * will get only a single Id in the application. Calling requestEventId
	 * twice will give you two totally different events, which is probably
	 * not what you want.
	 * @return A dynamic event Id for @ref start. Two different calls will
	 * return two different ids.
	 * @param name A name for the even that will be used for displaying it
	 * in the dialog. Two different events with the same name are totally
	 * valid
	 **/
	int requestEventId(const QString& name);

	/**
	 * @return The name for the event Id @p id, as provided to @ref
	 * requestEventId or @ref QString::null if no such id was requested
	 * before.
	 **/
	QString eventName(int id) const;

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
	 * @return The elapsed time since calling @ref start with this event
	 * number (in usec). Undefined if @ref start has not yet been called
	 * with this event number.
	 **/
	long int elapsed(int event) const;

	/**
	 * Stop the event timer and append the resulting time to the list. If
	 * the list contains more than @ref maxEventEntries the first item is
	 * removed. See also @ref start
	 *
	 * @param appendToList If TRUE (default) the resulting time will be
	 * appended to the list for this event, and can be analyzed using @ref
	 * BosonProfilingDialog. Otherwise the elapsed time is returned only.
	 *
	 * @return See @ref elapsed (note: usec!)
	 **/
	long int stop(int event, bool appendToList = true);

	void loadUnit();
	void loadUnitDone(unsigned long int typeId);

	/**
	 * Starts benchmark
	 * During benchmark, all profiling info about advance calls and rendered
	 * frames is logged and some statistics are printed out when you call
	 * @ref endBenchmark()
	 **/
	void startBenchmark();
	/**
	 * Ends benchmark and prints out some useful info about logged things.
	 *
	 * @param name Optional name of the benchmark. If given it's printed out, so
	 * you can later indentify your benchmark
	 **/
	void endBenchmark(const QString& name);

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
	void renderWater(bool start);
	void renderFOW(bool start);
	void renderText(bool start);
	void renderUfo(bool start);

	/**
	 * @return The number of recoreded frames
	 **/
	unsigned int renderEntries() const;

	// AB: the syntax of these is the same as for render() above. e.g. you
	// mustn't call the advance*() stuff here recursive or so
	void advance(bool start, unsigned int advanceCallsCount);
	void advanceFunction(bool start);
	void advanceDeleteUnusedShots(bool start);
	void advanceEffects(bool start);
	void advanceWater(bool start);
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

	void setMaxEventEntries(unsigned int max)
	{
		mMaxEventEntries = max;
	}
	void setMaxAdvanceEntries(unsigned int max)
	{
		mMaxAdvanceEntries = max;
	}
	void setMaxRenderingEntries(unsigned int max)
	{
		mMaxRenderingEntries = max;
	}
	inline unsigned int maxEventEntries() const
	{
		return mMaxEventEntries;
	}
	inline unsigned int maxAdvanceEntries() const
	{
		return mMaxAdvanceEntries;
	}
	inline unsigned int maxRenderingEntries() const
	{
		return mMaxRenderingEntries;
	}

private:
	void init();

private:
	BosonProfilingPrivate* d;
	friend class BosonProfilingDialog;
	unsigned int mMaxEventEntries;
	unsigned int mMaxRenderingEntries;
	unsigned int mMaxAdvanceEntries;
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
 * Note that MyProfilingEvent needs to be a constant number, i.e. it must always
 * be the same when calling the function multiple times. Also it must be unique
 * to this function. You can easily achieve this by the following code:
 * <pre>
 * static int MyProfilingEvent = boProfiling->requestEventId("My Event");
 * </pre>
 *
 * Often you don't want to save the whole event, but need the profiled time
 * only, i.e. you want to use @ref elapsed once and then forget it. This can
 * easily be done by the following code:
 * <pre>
 * static int eventid = -boProfiling->requestEventId("My Event"); // note the "-" !
 * BosonProfiler profiler(eventid);
 * doStuff();
 *
 * // display the elapsed time (warning: the boDebug() call takes much time, so the data may be inaccurate
 * boDebug() << profiler.elapsed() << endl;
 * </pre>
 *
 * @short Convenience class for profiling events
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonProfiler
{
public:
	/**
	 * Start profiling using @p event. This is equivalent to using @ref
	 * BosonProfiling::start and @ref BosonProfiling::stop with that event
	 * id, if it is >= 0.
	 *
	 * If @p event is negative, the profiling data are <em>not</em>saved on
	 * destruction, i.e. the appendToList parameter of @ref
	 * BosonProfiling::stop is false. This can be very useful if you don't
	 * need to display the data in a dialog later, but need the value of
	 * @ref elapsed once only.
	 **/
	BosonProfiler(int event)
		: mEvent(event),
		mEventStopped(false),
		mAppendToList(event >= 0)
	{
		boProfiling->start(mEvent);
	}

	~BosonProfiler()
	{
		if (!mEventStopped) {
			stop();
		}
	}

	/**
	 * This stops profiling the event immediately. The destructor will do
	 * nothing when you call this.
	 * @return See @ref BosonProfiling::stop
	 **/
	long int stop()
	{
		mEventStopped = true;
		return boProfiling->stop(mEvent, mAppendToList);
	}

	/**
	 * @return See @ref BosonProfiling::elapsed
	 **/
	long int elapsed() const
	{
		return boProfiling->elapsed(mEvent);
	}

private:
	int mEvent;
	bool mEventStopped;
	bool mAppendToList;
};


#endif
