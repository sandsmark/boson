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

#include "kgameadvancemessagesdebug.h"
#include "kgameadvancemessagesdebug.moc"

#include "../bomemory/bodummymemory.h"
#include "bodebug.h"
#include "bosoncanvas.h"
#include "boson.h"
#include "qlistviewitemnumber.h"

#include <klocale.h>

#include <qptrlist.h>
#include <qlistview.h>
#include <qlayout.h>
#include <qsplitter.h>

#include <sys/time.h>

// returns how many ms (!) passed since the "last" measured time to the "newest"
// time
static unsigned long int compareTimes(const struct timeval* newest, const struct timeval* last)
{
 if (!newest || !last) {
	return 0;
 }
 return (((newest->tv_sec - last->tv_sec) * 1000000) + (newest->tv_usec - last->tv_usec)) / 1000;
}


class KGameAdvanceMessagesDebugPrivate
{
public:
	KGameAdvanceMessagesDebugPrivate()
	{
	}
	QListView* mMessages;
	QListView* mCalls;
};


KGameAdvanceMessagesDebug::KGameAdvanceMessagesDebug(QWidget* parent, const char* name)
	: QWidget(parent, name)
{
 d = new KGameAdvanceMessagesDebugPrivate;
 mGame = 0;

 QVBoxLayout* layout = new QVBoxLayout(this);

 QSplitter* splitter = new QSplitter(this);
 layout->addWidget(splitter);

 d->mMessages = new QListView(splitter);
 d->mMessages->setAllColumnsShowFocus(true);
 d->mMessages->addColumn(i18n("Message"));
 d->mMessages->addColumn(i18n("Time since last message"));

 d->mCalls = new QListView(splitter);
 d->mCalls->setAllColumnsShowFocus(true);
 d->mCalls->addColumn(i18n("Message"));
 d->mCalls->addColumn(i18n("Msg call"));
 d->mCalls->addColumn(i18n("Call"));
 d->mCalls->addColumn(i18n("Time since message"));
 d->mCalls->addColumn(i18n("Should since message"));
 d->mCalls->addColumn(i18n("Error"));
 d->mCalls->addColumn(i18n("Time since last call"));
 d->mCalls->addColumn(i18n("Should since call"));
 d->mCalls->addColumn(i18n("Error"));

}

KGameAdvanceMessagesDebug::~KGameAdvanceMessagesDebug()
{
 delete d;
}

void KGameAdvanceMessagesDebug::setBoson(Boson* b)
{
 mGame = b;
 slotUpdate();
}

void KGameAdvanceMessagesDebug::slotUpdate()
{
 BO_CHECK_NULL_RET(mGame);
 QPtrListIterator<BoAdvanceMessageTimes> it(mGame->advanceMessageTimes());
 const int advanceInterval = mGame->advanceMessageInterval();
 int messageNumber = 0;
 int callNumber = 0;
 struct timeval* lastMessage = 0;
 struct timeval* lastCall = 0;
 int nextCall = 0;
 while (it.current()) {
	QListViewItemNumber* message = new QListViewItemNumber(d->mMessages);
	message->setText(0, QString::number(messageNumber));
	message->setText(1, QString::number(compareTimes(&it.current()->mAdvanceMessage, lastMessage)));

	int msPerCall = advanceInterval / it.current()->mGameSpeed;
	for (int i = 0; i < it.current()->mGameSpeed; i++) {
		QListViewItemNumber* call = new QListViewItemNumber(d->mCalls);
		call->setText(0, QString::number(messageNumber));
		call->setText(1, QString::number(i));
		call->setText(2, QString::number(callNumber));

		int timeSinceMessage = compareTimes(&it.current()->mAdvanceCalls[i], &it.current()->mAdvanceMessage);
		int timeShouldSinceMessage = msPerCall * i;
		call->setText(3, QString::number(timeSinceMessage));
		call->setText(4, QString::number(timeShouldSinceMessage));
		call->setText(5, QString::number(timeSinceMessage - timeShouldSinceMessage));

		int timeSinceCall = compareTimes(&it.current()->mAdvanceCalls[i], lastCall);
		int timeShouldSinceCall = nextCall;
		call->setText(6, QString::number(timeSinceCall));
		call->setText(7, QString::number(timeShouldSinceCall));
		call->setText(8, QString::number(timeSinceCall - timeShouldSinceCall));

		// the next call should have been makde msPerCall * (i+1) ms after the
		// message has been received.
		int shouldSinceMessage = msPerCall * (i + 1);
		int shouldSinceThisCall = shouldSinceMessage - (compareTimes(&it.current()->mAdvanceCalls[i], &it.current()->mAdvanceMessage));
		nextCall = shouldSinceThisCall;

		lastCall = &it.current()->mAdvanceCalls[i];
		callNumber++;

	}

	lastMessage = &it.current()->mAdvanceMessage;
	messageNumber++;
	++it;
 }
}

