/*
    This file is part of the Boson game
    Copyright (C) 2006 Andreas Beckermann (b_mann@gmx.de)

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

#include "bonetworktrafficwidget.h"
#include "bonetworktrafficwidget.moc"

#include "bodebug.h"
#include "../../bosonconfig.h"
#include "../../gameengine/boson.h"
#include "../../gameengine/bosonnetworktraffic.h"

#include <qsignalmapper.h>

#include <klocale.h>

class BoNetworkTrafficWidgetPrivate
{
public:
	BoNetworkTrafficWidgetPrivate()
	{
		mTotalSent = 0;
		mTotalReceived = 0;
		mMostTraffic = 0;
		mRecentlySent = 0;
		mRecentlyReceived = 0;
	}
	BoUfoLabel* mTotalSent;
	BoUfoLabel* mTotalReceived;
	BoUfoLabel* mMostTraffic;
	BoUfoLabel* mRecentlySent;
	BoUfoLabel* mRecentlyReceived;
};



BoNetworkTrafficWidget::BoNetworkTrafficWidget()
	: BoUfoWidget()
{
 d = new BoNetworkTrafficWidgetPrivate();

 d->mTotalSent = new BoUfoLabel();
 d->mTotalReceived = new BoUfoLabel();
 d->mMostTraffic = new BoUfoLabel();
 d->mRecentlySent = new BoUfoLabel();
 d->mRecentlyReceived = new BoUfoLabel();
 addWidget(d->mTotalSent);
 addWidget(d->mTotalReceived);
 addWidget(d->mMostTraffic);
 addWidget(d->mRecentlySent);
 addWidget(d->mRecentlyReceived);

 setLayoutClass(BoUfoWidget::UVBoxLayout);
}

BoNetworkTrafficWidget::~BoNetworkTrafficWidget()
{
 delete d;
}

void BoNetworkTrafficWidget::slotUpdate()
{
 if (!boGame) {
	return;
 }
 const BosonNetworkTraffic* traffic = boGame->networkTraffic();
 BO_CHECK_NULL_RET(traffic);

 d->mTotalSent->setText(i18n("Total bytes sent: %1").arg(traffic->totalBytesSent()));
 d->mTotalReceived->setText(i18n("Total bytes received: %1").arg(traffic->totalBytesReceived()));

 if (traffic->statistics().count() > 0) {
	int mostCount = 5;

	QValueList<const BosonNetworkTrafficStatistics*> most;
	const QPtrListIterator<BosonNetworkTrafficStatistics>& statistics = traffic->statistics();
	const BosonNetworkTrafficStatistics* previousMax = 0;
	long long previousMaxTraffic = 0;
	for (int i = 0; i < mostCount; i++) {
		const BosonNetworkTrafficStatistics* max = 0;
		long long maxTraffic = 0;
		for (QPtrListIterator<BosonNetworkTrafficStatistics> it(statistics); it.current(); ++it) {
			long long traffic = it.current()->totalBytesReceived() + it.current()->totalBytesSent();
			if (previousMax && (it.current() == previousMax || traffic > previousMaxTraffic)) {
				continue;
			}
			if (traffic > maxTraffic) {
				maxTraffic = traffic;
				max = it.current();
			}
		}
		if (!max) {
			break;
		}
		previousMax = max;
		previousMaxTraffic = maxTraffic;
		most.append(max);
	}

	QString mostString;
	mostString += i18n("Most traffic caused by message IDs:\n");
	for (QValueList<const BosonNetworkTrafficStatistics*>::iterator it = most.begin(); it != most.end(); ++it) {
		const BosonNetworkTrafficStatistics* s = *it;
		mostString += i18n("ID=%1 (%2) bytes sent=%3 (%4 messages) bytes received=%5 (%6 messages)\n").arg(s->msgid()).arg(s->userMsgid()).arg(s->totalBytesSent()).arg(s->messagesSent()).arg(s->totalBytesReceived()).arg(s->messagesReceived());
	}
	d->mMostTraffic->setText(mostString);
 }

 const int pastSeconds = 5;
 QTime now = QTime::currentTime();
 const QPtrList<BosonNetworkTrafficDetails>& details = traffic->messageDetails();
 QPtrListIterator<BosonNetworkTrafficDetails> detailsIt(details);
 unsigned int trafficSent = 0;
 unsigned int trafficReceived = 0;
 for (detailsIt.toLast(); detailsIt.current(); --detailsIt) {
	if (detailsIt.current()->timeStamp().secsTo(now) < pastSeconds) {
		if (detailsIt.current()->sentMessage()) {
			trafficSent += detailsIt.current()->bytes();
		} else {
			trafficReceived += detailsIt.current()->bytes();
		}
	} else {
		break;
	}
 }
 d->mRecentlySent->setText(i18n("Bytes sent in last %1 seconds: %2").arg(pastSeconds).arg(trafficSent));
 d->mRecentlyReceived->setText(i18n("Bytes received in last %1 seconds: %2").arg(pastSeconds).arg(trafficReceived));
}
