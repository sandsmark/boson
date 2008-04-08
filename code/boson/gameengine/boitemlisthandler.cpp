/*
    This file is part of the Boson game
    Copyright (C) 2003 Andreas Beckermann (b_mann@gmx.de)

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

#include "boitemlisthandler.h"
#include "boitemlisthandler.moc"

#include "../../bomemory/bodummymemory.h"
#include "boitemlist.h"
#include "../boglobal.h"
#include "bodebug.h"

#include <q3ptrlist.h>
#include <qtimer.h>

// currently we delete the lists as soon as we return to the event loop
#define DELETION_DELAY 0

// AB: we use initFirst, because the item list hanlder must be deleted last (as
// BosonPlayField's cells depend on it).
// note tha this mean neither c'tor nor d'tor may use BosonConfig! same the
// other way round! (we can't say which of them gets constructed first)
static BoGlobalObject<BoItemListHandler> globalHandler(BoGlobalObjectBase::BoGlobalItemListHandler, true);

class BoItemListHandlerPrivate
{
public:
	BoItemListHandlerPrivate()
	{
	}
	Q3PtrList<BoItemList> mLists;
	bool mTimerActive;
	bool mEnableDeletionTimer;
};

BoItemListHandler::BoItemListHandler() : QObject(0)
{
 d = new BoItemListHandlerPrivate();
 d->mTimerActive = false;
 d->mEnableDeletionTimer = true;
}

BoItemListHandler::~BoItemListHandler()
{
 slotDeleteLists();
 delete d;
}

BoItemListHandler* BoItemListHandler::itemListHandler()
{
 return BoGlobal::boGlobal()->boItemListHandler();
}

void BoItemListHandler::setEnableDeletionTimer(bool e)
{
 d->mEnableDeletionTimer = e;
}

void BoItemListHandler::registerList(BoItemList* list)
{
 d->mLists.append(list);
 if (!d->mTimerActive && d->mEnableDeletionTimer) {
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
 Q3PtrList<BoItemList> lists = d->mLists;

 // every list's unregisters itself in its d'tor, so let's keep that fast by
 // avoiding list lookups.
 d->mLists.clear();

 if (lists.count() > 0) {
	Q3PtrListIterator<BoItemList> it(lists);
	for (; it.current(); ++it) {
		delete it.current();
	}
 }
}

