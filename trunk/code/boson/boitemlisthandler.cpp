/*
    This file is part of the Boson game
    Copyright (C) 2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "boitemlisthandler.h"
#include "boitemlisthandler.moc"
#include "boitemlist.h"
#include "bodebug.h"

#include <kstaticdeleter.h>

#include <qptrlist.h>
#include <qtimer.h>

// currently we delete the lists as soon as we return to the event loop
#define DELETION_DELAY 0

static KStaticDeleter<BoItemListHandler> sd;
BoItemListHandler* BoItemListHandler::mItemListHandler = 0;

class BoItemListHandlerPrivate
{
public:
	BoItemListHandlerPrivate()
	{
	}
	QPtrList<BoItemList> mLists;
	bool mTimerActive;
};

BoItemListHandler::BoItemListHandler() : QObject(0)
{
 d = new BoItemListHandlerPrivate();
 d->mTimerActive = false;
}

BoItemListHandler::~BoItemListHandler()
{
 delete d;
}

void BoItemListHandler::initStatic()
{
 if (!mItemListHandler) {
	sd.setObject(mItemListHandler, new BoItemListHandler());
 }
}

void BoItemListHandler::registerList(BoItemList* list)
{
 d->mLists.append(list);
 if (!d->mTimerActive) {
	// we need to delete this list once we return to the event loop.
	 QTimer::singleShot(DELETION_DELAY, this, SLOT(slotDeleteLists()));

	 // emit this signal only once.
	 d->mTimerActive = true;
 }
}

void BoItemListHandler::unregisterList(BoItemList* list)
{
 // note: d->mList is NOT autodelete=true!
 d->mLists.removeRef(list);
}

void BoItemListHandler::slotDeleteLists()
{
 d->mTimerActive = false;
 QPtrList<BoItemList> lists = d->mLists;

 // every list's unregisters itself in its d'tor, so let's keep that fast by
 // avoiding list lookups.
 d->mLists.clear();

 if (lists.count() > 0) {
	boDebug() << k_funcinfo << lists.count() << endl;
	QPtrListIterator<BoItemList> it(lists);
	for (; it.current(); ++it) {
		delete it.current();
	}
 }
}

