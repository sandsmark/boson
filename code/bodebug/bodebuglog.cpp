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

#include "bodebuglog.h"
#include "bodebuglog.moc"

#include "bodebug.h"

#include <QCoreApplication> // qAddPostRoutine()


static const int maxLevels = 4; // see QtMsgType

BoDebugLog* BoDebugLog::mDebugLog = 0;

BoDebugLog::BoDebugLog()
{
 if (mDebugLog) {
	kFatal() << "debug log already created. can create only one instance of BoDebugLog!";
	return;
 }
 BoDebugLog::mDebugLog = this;
 mLists = maxLevels;
 mMessages = new QList<BoDebugMessage*>[mLists];
 mMaxCount = new unsigned int[mLists];
 mPopAtFront = new bool[mLists];
 mEmitSignal = new bool[mLists];
 for (int i = 0; i < mLists; i++) {
	mPopAtFront[i] = false;
	mMaxCount[i] = 1000;
	mEmitSignal[i] = false;
 }
 mAllMaxCount = 1000;
 mAllPopAtFront = false;
 mAllEmitSignal = false;

 // note: for QtCriticalMsg and QtWarningMsg , we will never reach these limits.
 // if QtCriticalMsg ever goes beyound 10000 entries, we have a lot of more grave
 // problems anyway.
 // but INFO can easily reach _huge_ numbers
 setMaxCount(QtCriticalMsg, 1000);
 setMaxCount(QtWarningMsg, 500);
 setMaxCount(QtDebugMsg, 2000);

 // the most recent errors are the important ones
 setPopAtFront(QtDebugMsg, true);


 // the separate list that is independant of the level
 setMaxCountOfFullLog(5000);
 setPopFullLogAtFront(true);
}

BoDebugLog::~BoDebugLog()
{
 for (int i = 0; i < mLists; i++) {
	qDeleteAll(mMessages[i]);
	mMessages[i].clear();
 }
 delete[] mMessages;
 delete[] mMaxCount;
 delete[] mPopAtFront;
 delete[] mEmitSignal;

 qDeleteAll(mAllMessages);
 mAllMessages.clear();


 // set static pointer back to NULL. this is safe, as there is only at most one
 // object of this class
 BoDebugLog::mDebugLog = 0;
}

BoDebugLog* BoDebugLog::debugLog()
{
 if (!mDebugLog) {
	mDebugLog = new BoDebugLog();
	qAddPostRoutine(deleteStatic);
 }
 return mDebugLog;
}

void BoDebugLog::deleteStatic()
{
 delete mDebugLog;
 mDebugLog = 0;
}

#warning TODO: add line, file, funcinfo parameters. storing and displaying them separately from the message string may be very useful!
void BoDebugLog::addEntry(const QString& string, int area, const QString& areaName, QtMsgType level_)
{
 int levelIndex = msgType2Index(level_);
 if (levelIndex >= mLists) {
	return;
 }
 bool addToLevel = true;
 bool addToAll = true;
 int countLevel = mMessages[levelIndex].count();
 int countAll = mAllMessages.count();
 QString backtrace;
 if ((unsigned int)countLevel >= mMaxCount[levelIndex]) {
	if (mPopAtFront[levelIndex]) {
		BoDebugMessage* first = mMessages[levelIndex].takeFirst();
		delete first;
	} else {
		// pop from the end - aka don't add anything
		addToLevel = false;
	}
 }
 if ((unsigned int)countAll >= mAllMaxCount) {
	if (mAllPopAtFront) {
		BoDebugMessage* first = mAllMessages.takeFirst();
		delete first;
	} else {
		// pop from the end - aka don't add anything
		addToAll = false;
	}
 }
 if (level_ != QtDebugMsg) {
	backtrace = boBacktrace(-1);
 }
 if (addToLevel) {
	BoDebugMessage* m = new BoDebugMessage(string, area, areaName, level_, backtrace);
	mMessages[levelIndex].append(m);
 }
 if (addToAll) {
	BoDebugMessage* m = new BoDebugMessage(string, area, areaName, level_, backtrace);
	mAllMessages.append(m);
 }
 if (mEmitSignal[levelIndex] || mAllEmitSignal) {
	BoDebugMessage m(string, area, areaName, level_, backtrace);
	emit signalMessage(m);
	switch (level_) {
		case QtDebugMsg:
			emit signalDebug(m);
			break;
		case QtWarningMsg:
			emit signalWarn(m);
			break;
		case QtCriticalMsg:
			emit signalError(m);
			break;
		case QtFatalMsg:
			emit signalFatal(m);
			break;
		default:
			break;
	}
 }
}

void BoDebugLog::setMaxCount(QtMsgType level, unsigned int count)
{
 if (msgType2Index(level) >= mLists) {
	return;
 }
 mMaxCount[msgType2Index(level)] = count;
}

void BoDebugLog::setMaxCountOfFullLog(unsigned int count)
{
 mAllMaxCount = count;
}

void BoDebugLog::setPopAtFront(QtMsgType level, bool front)
{
 if (msgType2Index(level) >= mLists) {
	return;
 }
 mPopAtFront[msgType2Index(level)] = front;
}

void BoDebugLog::setPopFullLogAtFront(bool front)
{
 mAllPopAtFront = front;
}

void BoDebugLog::setEmitSignal(QtMsgType level, bool e)
{
 if (msgType2Index(level) >= mLists) {
	return;
 }
 mEmitSignal[msgType2Index(level)] = e;
}

void BoDebugLog::setEmitSignalFullLog(bool e)
{
 mAllEmitSignal = e;
}

QList<BoDebugMessage*> BoDebugLog::messageLogLevel(QtMsgType level) const
{
 if (msgType2Index(level) >= mLists) {
	return QList<BoDebugMessage*>();
 }
 return mMessages[msgType2Index(level)];
}

QList<BoDebugMessage*> BoDebugLog::messageLogFull() const
{
 return mAllMessages;
}

int BoDebugLog::msgType2Index(QtMsgType type) const
{
 switch (type) {
	case QtDebugMsg:
		return 0;
	case QtWarningMsg:
		return 1;
	case QtCriticalMsg:
		return 2;
	case QtFatalMsg:
		return 3;
	default:
		return 0;
 }
 return 0;
}
