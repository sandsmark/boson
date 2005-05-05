/*
    This file is part of the Boson game
    Copyright (C) 2002-2005 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bosonprofiling.h"
#include "bodebug.h"
#include "boglobal.h"
#include "defines.h"

#include <qptrlist.h>
#include <qptrstack.h>
#include <qvaluestack.h>
#include <qdict.h>

static BoGlobalObject<BosonProfiling> globalProfiling(BoGlobalObjectBase::BoGlobalProfiling);

#define COMPARE_TIMES(time1, time2) ( ((time2.tv_sec - time1.tv_sec) * 1000000) + (time2.tv_usec - time1.tv_usec) )

unsigned long int compareTimes(const struct timeval& t1, const struct timeval& t2)
{
 return (((t2.tv_sec - t1.tv_sec) * 1000000) + (t2.tv_usec - t1.tv_usec));
}
unsigned long int compareTimes2(const struct timeval* t)
{
 if (!t) {
	return 0;
 }
 return compareTimes(t[0], t[1]);
}



BosonProfilingItem::BosonProfilingItem()
	: mName(0),
	  mChildren(0),
	  mEnded(false)
{
}

BosonProfilingItem::BosonProfilingItem(const QString& name)
	: mName(new QString(name)),
	  mChildren(0),
	  mEnded(false)
{
 gettimeofday(&mStart, 0);
}

BosonProfilingItem::~BosonProfilingItem()
{
 if (mChildren) {
	mChildren->setAutoDelete(true);
 }
 delete mChildren;
 delete mName;
}

BosonProfilingItem* BosonProfilingItem::clone() const
{
 BosonProfilingItem* item = 0;
 if (mName) {
	item = new BosonProfilingItem(*mName);
 } else {
	item = new BosonProfilingItem();
 }
 item->mEnded = mEnded;
 item->mStart.tv_sec = mStart.tv_sec;
 item->mStart.tv_usec = mStart.tv_usec;
 item->mEnd.tv_sec = mEnd.tv_sec;
 item->mEnd.tv_usec = mEnd.tv_usec;

 if (mChildren) {
	bool ended = item->mEnded;
	item->mEnded = false;
	QPtrListIterator<BosonProfilingItem> it(*mChildren);
	while (it.current()) {
		item->addChild(it.current()->clone());
		++it;
	}
	item->mEnded = ended;
 }
 return item;
}

void BosonProfilingItem::stop()
{
 mEnded = true;
 gettimeofday(&mEnd, 0);
}

long int BosonProfilingItem::elapsedTime() const
{
 if (!mEnded) {
	boError() << k_funcinfo << "called before stop() - did you mean elapsedSinceStart() ?" << endl;
	return 0;
 }
 return COMPARE_TIMES(mStart, mEnd);
}

long int BosonProfilingItem::elapsedSinceStart() const
{
 struct timeval t;
 gettimeofday(&t, 0);
 return COMPARE_TIMES(mStart, t);
}

QString BosonProfilingItem::name() const
{
 if (mName) {
	return *mName;
 }
 return QString::null;
}

void BosonProfilingItem::addChild(BosonProfilingItem* child)
{
 if (mEnded) {
	boError() << k_funcinfo << "cannot add a child after stop() was called!" << endl;
	delete child;
	return;
 }
 if (!mChildren) {
	mChildren = new QPtrList<BosonProfilingItem>();
 }
 mChildren->append(child);
}


BosonProfilingStorage::BosonProfilingStorage(const QString& name, int maxEntries)
	: mName(new QString(name)),
	  mItems(new QPtrList<BosonProfilingItem>()),
	  mMaximalEntries(maxEntries)
{
}

BosonProfilingStorage::~BosonProfilingStorage()
{
 mItems->setAutoDelete(true);
 mItems->clear();
 delete mName;
 delete mItems;
}

const QString& BosonProfilingStorage::name() const
{
 return *mName;
}

void BosonProfilingStorage::setMaximalEntries(int max)
{
 mMaximalEntries = max;
}

int BosonProfilingStorage::maximalEntries() const
{
 return mMaximalEntries;
}

void BosonProfilingStorage::addItem(BosonProfilingItem* item)
{
 mItems->append(item);
 if (maximalEntries() > 0) {
	while (mItems->count() > (unsigned int)maximalEntries()) {
		// remove the old entries
		BosonProfilingItem* item = mItems->take(0);
		delete item;
	}
 }
}

QPtrList<BosonProfilingItem> BosonProfilingStorage::cloneItems() const
{
 QPtrList<BosonProfilingItem> list;
 QPtrListIterator<BosonProfilingItem> it(*mItems);
 while (it.current()) {
	list.append(it.current()->clone());
	++it;
 }
 return list;
}


class BosonProfilingPrivate
{
public:
	BosonProfilingPrivate()
	{
		mCurrentStorage = 0;
	}
	QPtrStack<BosonProfilingItem> mStack;

	int mDefaultMaxEntries;
	BosonProfilingStorage* mCurrentStorage;
	QDict<BosonProfilingStorage> mStorages;
	QValueStack<QString> mStorageStack;
};


BosonProfiling::BosonProfiling()
{
 init();
}

void BosonProfiling::init()
{
 d = new BosonProfilingPrivate;
 d->mDefaultMaxEntries = 1000;
 d->mCurrentStorage = new BosonProfilingStorage("Default", d->mDefaultMaxEntries);
 d->mStorages.insert(d->mCurrentStorage->name(), d->mCurrentStorage);
}

BosonProfiling::~BosonProfiling()
{
 if (!d->mStack.isEmpty()) {
	boWarning() << k_funcinfo << "stack not empty" << endl;
	while (!d->mStack.isEmpty()) {
		BosonProfilingItem* item = d->mStack.pop();
		delete item;
	}
 }
 d->mStorages.setAutoDelete(true);
 d->mStorages.clear();
 delete d;
}

BosonProfiling* BosonProfiling::bosonProfiling()
{
 return BoGlobal::boGlobal()->bosonProfiling();
}

const BosonProfilingItem* BosonProfiling::push(const QString& name)
{
 BosonProfilingItem* item = new BosonProfilingItem(name);
 d->mStack.push(item);
 return item;
}

void BosonProfiling::pop()
{
 if (d->mStack.isEmpty()) {
	boError() << k_funcinfo << "no items on the stack (stack underflow)" << endl;
	return;
 }
 BosonProfilingItem* item = d->mStack.pop();
 item->stop();

 BosonProfilingItem* parent = d->mStack.top();
 if (parent) {
	parent->addChild(item);
 } else {
	d->mCurrentStorage->addItem(item);
 }
}

void BosonProfiling::switchStorage(const QString& name)
{
 if (name == d->mCurrentStorage->name()) {
	return;
 }
 BosonProfilingStorage* storage = d->mStorages[name];
 if (!storage) {
	storage = new BosonProfilingStorage(name, d->mDefaultMaxEntries);
	d->mStorages.insert(storage->name(), storage);
 }
 d->mCurrentStorage = storage;
}

void BosonProfiling::pushStorage(const QString& name)
{
 d->mStorageStack.push(d->mCurrentStorage->name());
 switchStorage(name);
}

void BosonProfiling::popStorage()
{
 if (d->mStorageStack.isEmpty()) {
	boError() << k_funcinfo << "stack underflow" << endl;
	return;
 }
 QString name = d->mStorageStack.pop();
 switchStorage(name);
}

void BosonProfiling::setMaximalEntriesAllStorages(int max)
{
 d->mDefaultMaxEntries = max;
 QDictIterator<BosonProfilingStorage> it(d->mStorages);
 while (it.current()) {
	it.current()->setMaximalEntries(max);
	++it;
 }
}

void BosonProfiling::setMaximalEntries(int max)
{
 d->mCurrentStorage->setMaximalEntries(max);
}

int BosonProfiling::maximalEntries() const
{
 return d->mCurrentStorage->maximalEntries();
}

void BosonProfiling::setMaximalEntries(const QString& storage, int max)
{
 BO_CHECK_NULL_RET(d->mCurrentStorage);
 QString current = d->mCurrentStorage->name();
 switchStorage(storage);
 setMaximalEntries(max);
 switchStorage(current);
}


QPtrList<BosonProfilingItem> BosonProfiling::cloneItems(const QString& storageName) const
{
 BosonProfilingStorage* storage = d->mStorages[storageName];
 if (!storage) {
	return QPtrList<BosonProfilingItem>();
 }
 return storage->cloneItems();
}

QPtrList<BosonProfilingItem> BosonProfiling::cloneItems() const
{
 QPtrList<BosonProfilingItem> list;
 QDictIterator<BosonProfilingStorage> it(d->mStorages);
 while (it.current()) {
	QPtrList<BosonProfilingItem> l = it.current()->cloneItems();
	QPtrListIterator<BosonProfilingItem> it2(l);
	while (it2.current()) {
		list.append(it2.current());
		++it2;
	}
	++it;
 }
 return list;
}

