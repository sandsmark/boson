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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "boeventloop.h"
#include "boeventloop.moc"

#include "../../bomemory/bodummymemory.h"
#include "../global.h"
#include "bodebug.h"
#include "boson.h"

#include <qdatetime.h>
#include <qapplication.h>

/*
 * We have two options: either use sendEvent() or postEvent(). sendEvent() is
 * basically just a direct call to the event handler, postEvent() goes through
 * the event queue.
 *
 * The advantage of sendEvent() is obviously that we have more direct control on
 * it. However postEvent() is not blocking and it is sometimes useful to do
 * necessary calculations _before_ the advance call is made
 */
#define DO_SEND_NOT_POST 1


class BoEventLoopPrivate
{
public:
	BoEventLoopPrivate()
	{
		mGameSpeed = 0;
		mAdvanceCallsMade = 0;
		mAdvanceMessagesWaiting = 0;
	}
	int mAdvanceMessageInterval;

	// the game speed when the last advance message was received. this is
	// the number of advance calls that are made for that message.
	int mGameSpeed;
	int mAdvanceCallsMade;
	int mAdvanceMessagesWaiting;
	QTime mLastAdvanceMessage;
	QTime mNextAdvanceMessage;
	QTime mNextAdvanceCall;
};

BoEventLoop::BoEventLoop(QObject* parent, const char* name)
	: QEventLoop(parent, name)
{
 d = new BoEventLoopPrivate;
 d->mAdvanceMessageInterval = 250;
 mAdvanceObject = 0;
}

BoEventLoop::~BoEventLoop()
{
 delete d;
}

void BoEventLoop::setAdvanceMessageInterval(int interval)
{
 if (interval <= 0) {
	boError() << k_funcinfo << "interval must be > 0" << endl;
	interval = 250;
 }
 d->mAdvanceMessageInterval = interval;
}

void BoEventLoop::setAdvanceObject(QObject* o)
{
 mAdvanceObject = o;
 d->mAdvanceCallsMade = 0;
 d->mLastAdvanceMessage = QTime();
 d->mNextAdvanceMessage = QTime();
 d->mNextAdvanceCall = QTime();
 d->mGameSpeed = 0; // will be set by first advance message
 d->mAdvanceMessagesWaiting = 0;
}

void BoEventLoop::setAdvanceMessagesWaiting(int count)
{
 d->mAdvanceMessagesWaiting = count;
}

void BoEventLoop::postAdvanceCallEvent()
{
 BO_CHECK_NULL_RET(mAdvanceObject);
 BO_CHECK_NULL_RET(boGame);

 if (d->mGameSpeed <= 0) {
	boError() << k_funcinfo << "invalid game speed " << d->mGameSpeed << endl;
	return;
 }

 // first calculate when the next advance call will be made
 if (d->mAdvanceCallsMade + 1 < d->mGameSpeed) {
	if (d->mAdvanceMessagesWaiting == 0) {
		int nextCallNumber = d->mAdvanceCallsMade + 1;
		const int callAfterMs = d->mAdvanceMessageInterval / d->mGameSpeed;
		const int nextCall = nextCallNumber * callAfterMs;
		d->mNextAdvanceCall = d->mLastAdvanceMessage.addMSecs(nextCall);
	} else {
		d->mNextAdvanceCall = QTime::currentTime();
	}
 }

#if DO_SEND_NOT_POST
 QEvent e((QEvent::Type)((int)QEvent::User + QtEventAdvanceCall));
 qApp->sendEvent(mAdvanceObject, &e);
#else
 QEvent* e = new QEvent((QEvent::Type)((int)QEvent::User + QtEventAdvanceCall));
 qApp->postEvent(mAdvanceObject, e);
#endif


 d->mAdvanceCallsMade++;
 if (d->mAdvanceCallsMade == d->mGameSpeed) {
	// all advance calls for the current advance message were made. enable
	// delivery of the next advance message

#if DO_SEND_NOT_POST
	QEvent e((QEvent::Type)((int)QEvent::User + QtEventAdvanceMessageCompleted));
	qApp->sendEvent(mAdvanceObject, &e);
#else
	QEvent* e = new QEvent((QEvent::Type)((int)QEvent::User + QtEventAdvanceMessageCompleted));
	qApp->postEvent(mAdvanceObject, e);
#endif
 }
}

void BoEventLoop::receivedAdvanceMessage(int gameSpeed)
{
 QTime expected = d->mNextAdvanceMessage;
 d->mLastAdvanceMessage = QTime::currentTime();
 d->mNextAdvanceMessage = QTime::currentTime().addMSecs(d->mAdvanceMessageInterval);
 d->mGameSpeed = gameSpeed;

 int missed = expected.msecsTo(d->mLastAdvanceMessage);
 boDebug(300) << k_funcinfo << "advance message time missed by " << missed << endl;

 if (gameSpeed <= 0) {
	boError(300) << k_funcinfo << "received advance message with invalid gameSpeed " << gameSpeed << endl;
	return;
 }

 d->mAdvanceCallsMade = 0;
 d->mNextAdvanceCall = QTime::currentTime();
 postAdvanceCallEvent();
}

bool BoEventLoop::processEvents(ProcessEventsFlags flags)
{
 bool ret;
 ret = QEventLoop::processEvents(flags);
 if (d->mAdvanceMessagesWaiting) {
	ret = QEventLoop::processEvents(flags & ~WaitForMore);
 } else {
//	ret = QEventLoop::processEvents(flags & ~WaitForMore);
	ret = QEventLoop::processEvents(flags);
 }

 if (flags & (AllEvents | WaitForMore)) {
	// probably this was called from enterLoop(), but if not it's still fine
	// to do our stuff here, since WaitForMore was requested anyway.

	if (!ret) {
		// nothing found to do?? ha! we always have something to do!
		// FIXME: test whether this makes enough sense - it might be
		// possible that we block too long here. but probably its ok
		// AB: only do that if WaitForMore was removed when calling
		// processEvents()
//		if (boConfig->unusedCPUTimeForRendering())
		{
//			emit signalUpdateGL();
		}
	}

	int maxCalls = 4;
	// we do now up to maxCalls advance calls, but in the optimal case we
	// do only one.
	for (int i = 0; i < maxCalls; i++) {
		if (mAdvanceObject && d->mGameSpeed > 0 && d->mAdvanceCallsMade < d->mGameSpeed) {
			QTime now = QTime::currentTime();
			if (now.msecsTo(d->mNextAdvanceCall) <= 0) {
				postAdvanceCallEvent();
			} else {
				i = maxCalls;
			}
		}
	}

 }
 return ret;
}

