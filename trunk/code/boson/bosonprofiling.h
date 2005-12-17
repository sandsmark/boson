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
#ifndef BOSONPROFILING_H
#define BOSONPROFILING_H


#include <sys/time.h>

class QString;
class QDataStream;
template<class T> class QPtrList;
template<class T> class QPtrStack;



#define boProfiling BosonProfiling::bosonProfiling()

#ifdef __GNUC__
#define prof_funcinfo QString("[%1]").arg(__PRETTY_FUNCTION__)
#else
#define prof_funcinfo QString("[%1: %2]").arg(__FILE__).arg(__LINE__)
#endif

/**
 * Use this macro to profile a method. Place it at the beginning of a method and
 * the profiling values appear in the profiling dialog
 **/
#define PROFILE_METHOD \
	BosonProfiler methodProfiler(prof_funcinfo);

/**
 * Same as above, but provides two parameters for advanced uses:
 * @param name The name of the @ref BosonProfiler object. You can use this to
 * stop the profiling object at some point for example.
 * @param desc A description of what is profiled, in addition to the
 * (automatically added) method name.
 **/
#define PROFILE_METHOD_2(name, desc) \
	BosonProfiler name(prof_funcinfo + " - " + desc);

class BosonProfilingItem
{
public:
	BosonProfilingItem();
	BosonProfilingItem(const QString& name);
	~BosonProfilingItem();

	BosonProfilingItem* clone() const;
	const QPtrList<BosonProfilingItem>* children() const { return mChildren; }

	bool save(QDataStream& stream) const;
	bool load(QDataStream& stream);

	/**
	 * Finalize this profiling item by storing the current time as end time.
	 * Once this has been called, @ref elapsed is valid.
	 *
	 * Note that you usually do not need to call this yourself, as @ref
	 * BosonProfiling does it internally. If you use this class directly in
	 * your code, you probably need @ref elapsedSinceStart only.
	 **/
	void stop();

	/**
	 * @return The time elapsed between constructing this object and calling
	 * @ref stop. Undefined if @ref stop was not called yet, see @ref
	 * elapsedSinceStart for this
	 **/
	long int elapsedTime() const;

	/**
	 * @return The time since constructing this object.
	 **/
	long int elapsedSinceStart() const;

	/**
	 * Add a child to this object. Note that this must not be done anymore
	 * once @ref stop was called.
	 **/
	void addChild(BosonProfilingItem* child);

	QString name() const;

	const struct timeval& startTime() const
	{
		return mStart;
	}
	const struct timeval& endTime() const
	{
		return mEnd;
	}

private:
	QString* mName;
	QPtrList<BosonProfilingItem>* mChildren;
	bool mEnded;

	struct timeval mStart;
	struct timeval mEnd;
};


/**
 * @internal
 **/
class BosonProfilingStorage
{
public:
	BosonProfilingStorage(const QString& name, int maxEntries);
	~BosonProfilingStorage();

	bool save(QDataStream& stream) const;
	bool load(QDataStream& stream);

	void clear();

	const QString& name() const;
	void setMaximalEntries(int max);
	int maximalEntries() const;

	void addItem(BosonProfilingItem* item);

	const QPtrList<BosonProfilingItem>* items() const;
	QPtrList<BosonProfilingItem> cloneItems() const;

private:
	QString* mName;
	QPtrList<BosonProfilingItem>* mItems;
	int mMaximalEntries;
};

/**
 * Base class for a pop-task, i.e. a task that is executed in @ref
 * BosonProfiling::pop.
 *
 * See @ref BosonProfiling::setPopTask
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonProfilingPopTask
{
public:
	BosonProfilingPopTask()
	{
	}

	virtual ~BosonProfilingPopTask()
	{
	}

	/**
	 * Called by @ref BosonProfiling::pop
	 **/
	virtual void pop() = 0;
};

class BosonProfilingPrivate;
/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonProfiling
{
public:
	BosonProfiling();
	~BosonProfiling();

	/**
	 * @return BoGlobal::boGlobal()->bosonProfiling();
	 **/
	static BosonProfiling* bosonProfiling();

	bool save(QDataStream& stream) const;
	bool load(QDataStream& stream);

	BosonProfiling& operator=(const BosonProfiling& p);

	/**
	 * Start profiling something named @p name.
	 *
	 * All subsequent calls to this method add profiling items as child to
	 * this item. So for example the calls push("foo"); push("bar"); pop();
	 * push("end"); pop(); pop(); generate the following tree:
	 * <code>
	 * - foo
	 * -- bar
	 * -- end
	 * </code>
	 *
	 * Call @p pop to end profiling. Note that you always have exactly one
	 * @ref pop call for every push call!
	 **/
	const BosonProfilingItem* push(const QString& name);

	/**
	 * End profiling a previously started (using @ref push) profiling item.
	 *
	 * Note that you must have exactly as many @ref pop calls as @ref push
	 * calls. If you call @ref pop more often, you have stack underflows!
	 **/
	void pop();

	void pushStorage(const QString& name);
	void popStorage();

	/**
	 * Clear all items from the current storage (see @ref switchStorage)
	 * If there is an item currently
	 * being profiled (@ref push was called more often than @ref pop), it is
	 * not affected.
	 **/
	void clearStorage();

	/**
	 * @overload
	 * Like the above version, but clears @p storage instead of the current
	 * storage.
	 **/
	void clearStorage(const QString& storage);

	/**
	 * Like @ref clearStorage, but clears all storages.
	 **/
	void clearAllStorages();

	void switchStorage(const QString& name);

	/**
	 * Call @ref setMaximalEntries for all storages. See @ref switchStorage.
	 **/
	void setMaximalEntriesAllStorages(int max);

	/**
	 * Change the maximal number of items that get stored by @ref pop.
	 *
	 * Note that this number applies to toplevel items only, not to
	 * children. I.e. by calling push("foo"); push("bar"); pop(); pop(); you
	 * have only a single entry!
	 **/
	void setMaximalEntries(int max);
	int maximalEntries() const;

	/**
	 * @overload
	 * Changes the maximal number of stored items for @p storage. See also
	 * @ref switchStorage.
	 **/
	void setMaximalEntries(const QString& storage, int max);

	/**
	 * This returns all items currently stored which are not older than @p
	 * since. The sorting of the list depends on the internal storage
	 * structure, i.e. is NOT sorted by time. See also @ref getItemsSinceSorted
	 *
	 * @param ret Returns the list of profiling items since @p since.
	 **/
	void getItemsSince(QPtrList<const BosonProfilingItem>* ret, const struct timeval& since) const;

	/**
	 * This behaves exactly like @ref getItemsSince, but the list will be
	 * sorted by time.
	 *
	 * @param ret Returns the sorted list of profiling items since @p since.
	 **/
	void getItemsSinceSorted(QPtrList<const BosonProfilingItem>* ret, const struct timeval& since) const;

	/**
	 * @return A copy of all items currently stored. WARNING: you must
	 * delete all items in this list to prevent a memory leak!
	 **/
	QPtrList<BosonProfilingItem> cloneItems() const;
	QPtrList<BosonProfilingItem> cloneItems(const QString& storageName) const;

	/**
	 * Set object that adds an additional task to @ref pop. The
	 * BosonProfilingPopTask::pop method of this object is called right
	 * before @ref pop is called.
	 *
	 * By default the pop-task is NULL.
	 **/
	void setPopTask(BosonProfilingPopTask* task);

private:
	void init();

private:
	BosonProfilingPrivate* d;
};

unsigned long int compareTimes(const struct timeval& t1, const struct timeval& t2);
unsigned long int compareTimes2(const struct timeval* t1); // takes an array of 2 and compares them


/**
 * This class aids in profiling methods, by calling @ref BosonProfiling::push on
 * construction and @ref BosonProfiling::pop on destruction.
 *
 * You do not need this class if you don't want to use the push/pop feature.
 * Just use @ref BosonProfilingItem directly.
 *
 * Sample use:
 * <code>
 * void foobar(bool foo)
 * {
 *   BosonProfiler profiler("Profiling foobar()");
 *   if (foo) {
 *     // profiler.pop(); is not required here!
 *     return;
 *   }
 *   doSomething();
 *
 *   // profiler.pop(); is not required here!
 * }
 * </code>
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonProfiler
{
public:
	BosonProfiler(const QString& name)
		: mPopped(false),
		mPopStorage(false)
	{
		mItem = boProfiling->push(name);
	}
	BosonProfiler(const QString& name, const QString& storageName);

	~BosonProfiler()
	{
		pop();
	}

	void pop()
	{
		if (mPopped) {
			return;
		}
		mPopped = true;
		boProfiling->pop();
		if (mPopStorage) {
			boProfiling->popStorage();
			mPopStorage = false;
		}
	}
	long int popElapsed()
	{
		pop();
		return mItem->elapsedTime();
	}

	long int elapsedSinceStart() const
	{
		return mItem->elapsedSinceStart();
	}

private:
	bool mPopped;
	bool mPopStorage;
	const BosonProfilingItem* mItem;
};


inline bool operator<(const struct timeval& t1, const struct timeval& t2)
{
 if (t1.tv_sec < t2.tv_sec) {
	return true;
 } else if (t1.tv_sec > t2.tv_sec) {
	return false;
 }
 if (t1.tv_usec < t2.tv_usec) {
	return true;
 }
 return false;
}
inline bool operator<=(const struct timeval& t1, const struct timeval& t2)
{
 if (t1.tv_sec <= t2.tv_sec) {
	return true;
 } else if (t1.tv_sec > t2.tv_sec) {
	return false;
 }
 if (t1.tv_usec <= t2.tv_usec) {
	return true;
 }
 return false;
}
inline bool operator>(const struct timeval& t1, const struct timeval& t2)
{
 if (t1.tv_sec > t2.tv_sec) {
	return true;
 } else if (t1.tv_sec < t2.tv_sec) {
	return false;
 }
 if (t1.tv_usec > t2.tv_usec) {
	return true;
 }
 return false;
}
inline bool operator>=(const struct timeval& t1, const struct timeval& t2)
{
 if (t1.tv_sec >= t2.tv_sec) {
	return true;
 } else if (t1.tv_sec < t2.tv_sec) {
	return false;
 }
 if (t1.tv_usec >= t2.tv_usec) {
	return true;
 }
 return false;
}
inline bool operator==(const struct timeval& t1, const struct timeval& t2)
{
 if (t1.tv_sec == t2.tv_sec && t1.tv_usec == t2.tv_usec) {
	return true;
 }
 return false;
}
inline bool operator!=(const struct timeval& t1, const struct timeval& t2)
{
 if (t1.tv_sec != t2.tv_sec || t1.tv_usec != t2.tv_usec) {
	return true;
 }
 return false;
}


#endif
