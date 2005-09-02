/*
    This file is part of the Boson game
    Copyright (C) 2004 Andreas Beckermann (b_mann@gmx.de)

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

#include "bodebuglog.h"
#include "bodebuglog.moc"

#include "bodebug.h"

#include <kstaticdeleter.h>

static const int maxLevels = 4; // see BoDebug::DebugLevels

static KStaticDeleter<BoDebugLog> sd;
BoDebugLog* BoDebugLog::mDebugLog = 0;

BoDebugLog::BoDebugLog()
{
 mLists = maxLevels;
 mAllMessages.setAutoDelete(true);
 mMessages = new QPtrList<BoDebugMessage>[mLists];
 mMaxCount = new unsigned int[mLists];
 mPopAtFront = new bool[mLists];
 mEmitSignal = new bool[mLists];
 for (int i = 0; i < mLists; i++) {
	mMessages[i].setAutoDelete(true);
	mPopAtFront[i] = false;
	mMaxCount[i] = 1000;
	mEmitSignal[i] = false;
 }
 mAllMaxCount = 1000;
 mAllPopAtFront = false;
 mAllEmitSignal = false;

 // note: for ERROR and WARN, we will never reach these limits.
 // if ERROR ever goes beyound 10000 entries, we have a lot of more grave
 // problems anyway.
 // but INFO can easily reach _huge_ numbers
 setMaxCount(BoDebug::KDEBUG_ERROR, 1000);
 setMaxCount(BoDebug::KDEBUG_WARN, 500);
 setMaxCount(BoDebug::KDEBUG_INFO, 2000);

 // the most recent errors are the important ones
 setPopAtFront(BoDebug::KDEBUG_INFO, true);


 // the separate list that is independant of the level
 setMaxCount(-1, 5000);
 setPopAtFront(-1, true);
}

BoDebugLog::~BoDebugLog()
{
 for (int i = 0; i < mLists; i++) {
	mMessages[i].clear();
 }
 delete[] mMessages;
 delete[] mMaxCount;
 delete[] mPopAtFront;
 delete[] mEmitSignal;


 // set static pointer back to NULL. this is safe, as there is only at most one
 // object of this class
 mDebugLog = 0;
}

void BoDebugLog::initStatic()
{
 static bool initialized = false;
 if (initialized) {
	// AB: note that even if mDebugLog == 0, initialized may still be false.
	// this can happen on destruction, if some debug message is emitted
	// after the log has been deleted already. we don't want to re-create
	// it, and so we use that initialized variable.
	return;
 }
 initialized = true;
 mDebugLog = new BoDebugLog;
 sd.setObject(mDebugLog);
}

void BoDebugLog::addEntry(const QString& string, int area, const QString& areaName, int level)
{
 if (level < 0 || level >= mLists) {
	return;
 }
 bool addToLevel = true;
 bool addToAll = true;
 unsigned int countLevel = mMessages[level].count();
 unsigned int countAll = mAllMessages.count();
 QString backtrace;
 if (countLevel >= mMaxCount[level]) {
	if (mPopAtFront[level]) {
		mMessages[level].removeFirst();
	} else {
		// pop from the end - aka don't add anything
		addToLevel = false;
	}
 }
 if (countAll >= mAllMaxCount) {
	if (mAllPopAtFront) {
		mAllMessages.removeFirst();
	} else {
		// pop from the end - aka don't add anything
		addToAll = false;
	}
 }
 if (level != BoDebug::KDEBUG_INFO) {
	backtrace = boBacktrace(-1);
 }
 if (addToLevel) {
	BoDebugMessage* m = new BoDebugMessage(string, area, areaName, level, backtrace);
	mMessages[level].append(m);
 }
 if (addToAll) {
	BoDebugMessage* m = new BoDebugMessage(string, area, areaName, level, backtrace);
	mAllMessages.append(m);
 }
 if (mEmitSignal[level] || mAllEmitSignal) {
	BoDebugMessage m(string, area, areaName, level, backtrace);
	emit signalMessage(m);
	switch (level) {
		case BoDebug::KDEBUG_INFO:
			emit signalDebug(m);
			break;
		case BoDebug::KDEBUG_WARN:
			emit signalWarn(m);
			break;
		case BoDebug::KDEBUG_ERROR:
			emit signalError(m);
			break;
		case BoDebug::KDEBUG_FATAL:
			emit signalFatal(m);
			break;
		default:
			break;
	}
 }
}

void BoDebugLog::setMaxCount(int level, unsigned int count)
{
 if (level >= mLists) {
	return;
 }
 if (level < 0) {
	mAllMaxCount = count;
 } else {
	mMaxCount[level] = count;
 }
}

void BoDebugLog::setPopAtFront(int level, bool front)
{
 if (level >= mLists) {
	return;
 }
 if (level < 0) {
	mAllPopAtFront = front;
 } else {
	mPopAtFront[level] = front;
 }
}

void BoDebugLog::setEmitSignal(int level, bool e)
{
 if (level >= mLists) {
	return;
 }
 if (level < 0) {
	mAllEmitSignal = e;
 } else {
	mEmitSignal[level] = e;
 }
}

const QPtrList<BoDebugMessage>* BoDebugLog::messageLogLevel(int level) const
{
 if (level >= mLists) {
	return 0;
 }
 if (level < 0) {
	return &mAllMessages;
 }
 return &mMessages[level];
}

