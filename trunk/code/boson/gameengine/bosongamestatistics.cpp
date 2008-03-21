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

#include "bosongamestatistics.h"
#include "bosongamestatistics.moc"

#include "boson.h"
#include "bosonmessageids.h"
#include "bosoncanvas.h"
#include "player.h"
#include "bodebug.h"

class BosonGameStatisticsPrivate
{
public:
	BosonGameStatisticsPrivate()
	{
		mBoson = 0;
	}
	Boson* mBoson;
	unsigned int mAdvanceMessageCounter;
};

BosonGameStatistics::BosonGameStatistics(QObject* parent)
	: QObject(parent)
{
 d = new BosonGameStatisticsPrivate();
 d->mAdvanceMessageCounter = 0;
}

BosonGameStatistics::~BosonGameStatistics()
{
 delete d;
}

void BosonGameStatistics::setGame(Boson* boson)
{
 d->mBoson = boson;
}

void BosonGameStatistics::receiveAdvanceMessage(BosonCanvas* canvas)
{
 BO_CHECK_NULL_RET(canvas);
 BO_CHECK_NULL_RET(d->mBoson);

 d->mAdvanceMessageCounter++;

 // send a status message every statusMessageInterval advance messages.
 // 1 == send every 250ms
 const int statusMessageInterval = 3;

 if (d->mAdvanceMessageCounter % statusMessageInterval != 0) {
	return;
 }

 QByteArray buffer;
 QDataStream stream(buffer, IO_WriteOnly);

 unsigned int playerCount = d->mBoson->gamePlayerCount();
 stream << (Q_UINT32)playerCount;
 for (unsigned int i = 0; i < playerCount; i++) {
	Player* p = d->mBoson->gamePlayerList()->at(i);
	stream << (Q_UINT32)p->bosonId();
	stream << (Q_UINT32)p->mobilesCount();
	stream << (Q_UINT32)p->facilitiesCount();
 }

 // WARNING: these information can easily be used for cheating!
 boGame->sendMessage(buffer, BosonMessageIds::IdStatus);
}

