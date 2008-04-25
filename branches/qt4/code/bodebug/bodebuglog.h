/*
    This file is part of the Boson game
    Copyright (C) 2004-2008 Andreas Beckermann (b_mann@gmx.de)

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef BODEBUGLOG_H
#define BODEBUGLOG_H

#include <qstring.h>
#include <qobject.h>

/**
 * @short Class representing a debug message
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoDebugMessage
{
public:
	BoDebugMessage(const QString& m, int area, const QString& areaName, QtMsgType level, const QString& backtrace)
		: mMessage(m), mArea(area), mAreaName(areaName), mLevel(level), mBacktrace(backtrace)
	{
	}

	const QString& message() const { return mMessage; }
	int area() const { return mArea; }
	const QString& areaName() const { return mAreaName; }
	QtMsgType level() const { return mLevel; }
	const QString& backtrace() const { return mBacktrace; }

private:
	QString mMessage;
	int mArea;
	QString mAreaName;
	QtMsgType mLevel;
	QString mBacktrace;

};

/**
 * @short Log of debug messages (especially errors)
 *
 * This class provides methods to store all debug messages that were made using
 * boDebug(), boWarning(), boError() and boFatal() (the latter isn't used in
 * boson and logging them isn't useful anyway, as the programs aborts on a fatal
 * error).
 *
 * Whenever a debug message is emitted, it is added to this log. Several lists
 * are maintained, one per level (the level is whether boDebug() or boWarning()
 * or boError() was used) and an additional one (-1) that contains all messages.
 *
 * This class stores up to @ref maxCount messages per level. Use @ref
 * setMaxCount to limit the number of messages stored. Once that limit is
 * exceeded, a message is removed from the log whenever a new one comes in. The
 * message is removed either from the front (the oldest message) or from the
 * back (incoming message is never added). See @ref setPopAtFront.
 *
 * Sometimes you want to display a message to the user - @ref setEmitSignal is
 * meant for that. When you activate it, the signal such as @ref signalMessage
 * are emitted whenever a message with that level comes in.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoDebugLog : public QObject
{
	Q_OBJECT
public:
	~BoDebugLog();

	/**
	 * @internal
	 * Used internally to add the message.
	 **/
	void addEntry(const QString& string, int area, const QString& areaName, QtMsgType level);

	/**
	 * @param count How many items are stored before removing items.
	 **/
	void setMaxCount(QtMsgType level, unsigned int count);

	/**
	 * Like @ref setMaxCount, but influences the list that contains all
	 * message types/levels.
	 **/
	void setMaxCountOfFullLog(unsigned int count);

	/**
	 * @param front TRUE to pop from the beginning (old messages), otherwise
	 * the new/incoming message is not added.
	 **/
	void setPopAtFront(QtMsgType level, bool front);

	/**
	 * Like @ref setPopAtFront but influences the "full log" that contains
	 * all message types.
	 **/
	void setPopFullLogAtFront(bool front);

	/**
	 * @param level See @ref BoDebug::DebugLevels for possible values.
	 * Additionally you can use -1 to influence the separately maintained
	 * list that contains messages of all levels.
	 * @param e If TRUE, a signal is emitted whenever a message with level
	 * @p level comes in
	 **/
	void setEmitSignal(QtMsgType level, bool e);
	void setEmitSignalFullLog(bool e);

	unsigned int maxCount(QtMsgType level) const
	{
		if (msgType2Index(level) >= mLists) {
			return 0;
		}
		return mMaxCount[msgType2Index(level)];
	}
	unsigned int maxCountFullLog(QtMsgType level) const
	{
		return mAllMaxCount;
	}
	bool popAtFront(QtMsgType level) const
	{
		if (msgType2Index(level) >= mLists) {
			return false;
		}
		return mPopAtFront[msgType2Index(level)];
	}
	bool popFullLogAtFront() const
	{
		return mAllPopAtFront;
	}

	bool emitSignal(QtMsgType level) const
	{
		if (msgType2Index(level) >= mLists) {
			return false;
		}
		return mEmitSignal[msgType2Index(level)];
	}

	/**
	 * @return @ref maxCount messages with @p level. Whether these are the
	 * most recent or the oldest @ref maxCount messages depends on @ref
	 * popAtFront.
	 **/
	QList<BoDebugMessage*> messageLogLevel(QtMsgType level) const;
	QList<BoDebugMessage*> messageLogFull() const;

	static BoDebugLog* debugLog();
	static void deleteStatic();

signals:
	/**
	 * Emitted whenever a message comes in with a level for that 
	 * @ref emitSignal returns TRUE.
	 *
	 * If emitSignal(-1) returns TRUE, this is always emitted.
	 **/
	void signalMessage(const BoDebugMessage& m);

	/**
	 * Emitted whenever an error message comes in and @ref emitSignal
	 * returns true for error messages.
	 *
	 * If emitSignal(-1) returns TRUE, this is always emitted.
	 **/
	void signalError(const BoDebugMessage& m);

	/**
	 * Emitted whenever a warning comes in and @ref emitSignal
	 * returns true for warnings.
	 *
	 * If emitSignal(-1) returns TRUE, this is always emitted.
	 **/
	void signalWarn(const BoDebugMessage& m);

	/**
	 * Emitted whenever a debug message comes in and @ref emitSignal
	 * returns true for debug messages.
	 *
	 * If emitSignal(-1) returns TRUE, this is always emitted.
	 **/
	void signalDebug(const BoDebugMessage& m);

	/**
	 * Emitted whenever a fatal message comes in and @ref emitSignal
	 * returns true for fatal messages.
	 *
	 * If emitSignal(-1) returns TRUE, this is always emitted.
	 **/
	void signalFatal(const BoDebugMessage& m); // AB: unused by boson atm. we dont use fatal messages, as we need more detailed error handling than a exit(1)

private:
	BoDebugLog();

	int msgType2Index(QtMsgType type) const;

private:
	int mLists;

	QList<BoDebugMessage*>* mMessages;
	unsigned int* mMaxCount;
	bool* mPopAtFront;
	bool* mEmitSignal;

	QList<BoDebugMessage*> mAllMessages;
	unsigned int mAllMaxCount;
	bool mAllPopAtFront;
	bool mAllEmitSignal;

	static BoDebugLog* mDebugLog;
};

#endif

