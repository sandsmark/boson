/*
    This file is part of the Boson game
    Copyright (C) 2002-2005 Andreas Beckermann (b_mann@gmx.de)

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

#include "bosonprofiling.h"

#include "../bomemory/bodummymemory.h"
#include "bodebug.h"
#include "boglobal.h"
#include "defines.h"

#include <q3ptrlist.h>
#include <q3ptrstack.h>
#include <q3valuestack.h>
#include <q3dict.h>
//Added by qt3to4:
#include <Q3PtrCollection>

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
 gettimeofday(&mStart, 0);
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

bool BosonProfilingItem::save(QDataStream& stream) const
{
 stream << (qint8)mEnded;
 stream << (qint32)mStart.tv_sec;
 stream << (qint32)mStart.tv_usec;
 stream << (qint32)mEnd.tv_sec;
 stream << (qint32)mEnd.tv_usec;
 if (mChildren) {
	stream << (quint32)mChildren->count();
	for (Q3PtrListIterator<BosonProfilingItem> it(*mChildren); it.current(); ++it) {
		stream << it.current()->name();
		if (!it.current()->save(stream)) {
			return false;
		}
	}
 } else {
	stream << (quint32)0;
 }
 return true;
}

bool BosonProfilingItem::load(QDataStream& stream)
{
 qint8 ended;
 stream >> ended;
 qint32 startS;
 qint32 startUS;
 qint32 endS;
 qint32 endUS;
 stream >> startS;
 stream >> startUS;
 stream >> endS;
 stream >> endUS;
 mStart.tv_sec = startS;
 mStart.tv_usec = startUS;
 mEnd.tv_sec = endS;
 mEnd.tv_usec = endUS;
 quint32 children;
 stream >> children;
 if (children > 1000000) {
	boError() << k_funcinfo << "invalid childcount " << children << " of item " << name() << endl;
	return false;
 }
 mEnded = false;
 for (unsigned int i = 0; i < children; i++) {
	QString name;
	stream >> name;
	BosonProfilingItem* item = new BosonProfilingItem(name);
	if (!item->load(stream)) {
		delete item;
		return false;
	}
	addChild(item);
 }
 mEnded = ended;
 return true;
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
	Q3PtrListIterator<BosonProfilingItem> it(*mChildren);
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
	mChildren = new Q3PtrList<BosonProfilingItem>();
 }
 mChildren->append(child);
}


BosonProfilingStorage::BosonProfilingStorage(const QString& name, int maxEntries)
	: mName(new QString(name)),
	  mItems(new Q3PtrList<BosonProfilingItem>()),
	  mMaximalEntries(maxEntries)
{
}

BosonProfilingStorage::~BosonProfilingStorage()
{
 clear();
 delete mName;
 delete mItems;
}

const QString& BosonProfilingStorage::name() const
{
 return *mName;
}

void BosonProfilingStorage::clear()
{
 mItems->setAutoDelete(true);
 mItems->clear();
}

bool BosonProfilingStorage::save(QDataStream& stream) const
{
 stream << (qint32)maximalEntries();
 stream << (quint32)mItems->count();
 for (Q3PtrListIterator<BosonProfilingItem> it(*mItems); it.current(); ++it) {
	stream << it.current()->name();
	if (!it.current()->save(stream)) {
		return false;
	}
 }
 return true;
}

bool BosonProfilingStorage::load(QDataStream& stream)
{
 clear();
 qint32 maxEntries;
 stream >> maxEntries;
 quint32 count;
 stream >> count;
 if (count > 1000000) {
	boError() << k_funcinfo << "invalid item count " << count << endl;
	return false;
 }
 setMaximalEntries(-1);
 for (unsigned int i = 0; i < count; i++) {
	QString name;
	stream >> name;
	BosonProfilingItem* item = new BosonProfilingItem(name);
	if (!item->load(stream)) {
		delete item;
		return false;
	}
	addItem(item);
 }
 setMaximalEntries(maxEntries);
 return true;
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
 if (maximalEntries() >= 0) {
	while (mItems->count() > (unsigned int)maximalEntries()) {
		// remove the old entries
		BosonProfilingItem* item = mItems->take(0);
		delete item;
	}
 }
}

const Q3PtrList<BosonProfilingItem>* BosonProfilingStorage::items() const
{
 return mItems;
}

Q3PtrList<BosonProfilingItem> BosonProfilingStorage::cloneItems() const
{
 Q3PtrList<BosonProfilingItem> list;
 Q3PtrListIterator<BosonProfilingItem> it(*mItems);
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

		mPopTask = 0;
	}
	Q3PtrStack<BosonProfilingItem> mStack;

	int mDefaultMaxEntries;
	BosonProfilingStorage* mCurrentStorage;
	Q3Dict<BosonProfilingStorage> mStorages;
	Q3ValueStack<QString> mStorageStack;

	BosonProfilingPopTask* mPopTask;
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

BosonProfiling& BosonProfiling::operator=(const BosonProfiling& p)
{
 // AB: note that we do not touch d->mStack or d->mCurrentStorage!
 //     -> all current data remains as-is and is not touched.

 QByteArray b;
 QDataStream writeStream(&b, QIODevice::WriteOnly);
 if (!p.save(writeStream)) {
	boError() << k_funcinfo << "unable to save data to stream" << endl;
	return *this;
 }
 QDataStream readStream(b);
 if (!load(readStream)) {
	boError() << k_funcinfo << "unable to read data from stream" << endl;
	return *this;
 }
 return *this;
}

bool BosonProfiling::save(QDataStream& stream) const
{
 // AB: do NOT store the current values (including the stacks)
 stream << (quint32)d->mStorages.count();
 for (Q3DictIterator<BosonProfilingStorage> it(d->mStorages); it.current(); ++it) {
	stream << it.current()->name();
	if (!it.current()->save(stream)) {
		return false;
	}
 }
 return true;
}

bool BosonProfiling::load(QDataStream& stream)
{
 // AB: we do NOT load/clear the current values (including the stacks)
 for (Q3DictIterator<BosonProfilingStorage> it(d->mStorages); it.current(); ++it) {
	it.current()->clear();
 }

 quint32 storages;
 stream >> storages;
 if (storages > 1000) {
	boError() << k_funcinfo << "invalid storages count " << storages << endl;
	return false;
 }
 for (unsigned int i = 0; i < storages; i++) {
	QString name;
	stream >> name;
	if (name.isEmpty()) {
		boError() << k_funcinfo << "empty storage name" << endl;
		return false;
	}
	QString currentStorage = d->mCurrentStorage->name();
	switchStorage(name); // ensure that storage really exists
	bool ret = true;
	if (!d->mStorages[name]) {
		BO_NULL_ERROR(d->mStorages[name]);
		ret = false;
	} else {
		ret = d->mStorages[name]->load(stream);
	}
	switchStorage(currentStorage);
	if (!ret) {
		boError() << k_funcinfo << "loading storage " << name << " failed" << endl;
		return false;
	}
 }
 return true;
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
 if (d->mPopTask) {
	d->mPopTask->pop();
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
 Q3DictIterator<BosonProfilingStorage> it(d->mStorages);
 while (it.current()) {
	it.current()->setMaximalEntries(max);
	++it;
 }
}

void BosonProfiling::setMaximalEntries(int max)
{
 BO_CHECK_NULL_RET(d->mCurrentStorage);
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

void BosonProfiling::clearStorage()
{
 BO_CHECK_NULL_RET(d->mCurrentStorage);
 d->mCurrentStorage->clear();
}

void BosonProfiling::clearStorage(const QString& storage)
{
 BO_CHECK_NULL_RET(d->mCurrentStorage);
 QString current = d->mCurrentStorage->name();
 switchStorage(storage);
 clearStorage();
 switchStorage(current);
}

void BosonProfiling::clearAllStorages()
{
 Q3DictIterator<BosonProfilingStorage> it(d->mStorages);
 while (it.current()) {
	clearStorage(it.current()->name());
	++it;
 }
}


void BosonProfiling::getItemsSince(Q3PtrList<const BosonProfilingItem>* ret, const struct timeval& since) const
{
 BO_CHECK_NULL_RET(ret);
 ret->clear();

 Q3DictIterator<BosonProfilingStorage> storageIt(d->mStorages);
 for (; storageIt.current(); ++storageIt) {
	const Q3PtrList<BosonProfilingItem>* list = storageIt.current()->items();
	Q3PtrListIterator<BosonProfilingItem> itemIt(*list);

	// search backwards for the first item that starts after since
	itemIt.toLast(); // AB: this is O(1) (important!)
	while (itemIt.current()) {
		if (itemIt.current()->startTime() < since) {
			// make the iterator point to the first item that starts
			// after since
			++itemIt;
			break;
		} else if (itemIt.atFirst()) {
			break;
		}
		--itemIt;
	}

	if (!itemIt.current()) {
		continue;
	}

	for (; itemIt.current(); ++itemIt) {
		ret->append(itemIt.current());
	}
 }
}

class MySortedProfilingList : public Q3PtrList<const BosonProfilingItem>
{
protected:
	virtual int compareItems (Q3PtrCollection::Item item1, Q3PtrCollection::Item item2)
	{
		BosonProfilingItem* p1 = (BosonProfilingItem*)item1;
		BosonProfilingItem* p2 = (BosonProfilingItem*)item2;

		// AB: note that some combinations are _not_ possible:
		// -> if p1 starts before p2, then it will also end before p2
		//    starts. items cannot overlap, i.e. are disjoint.
		//    therefore it is sufficient to compare startTimes only

		if (p1->startTime() < p2->startTime()) {
			return -1;
		} else if (p1->startTime() == p2->startTime()) {
			return 0;
		}
		return 1;
	}

};

void BosonProfiling::getItemsSinceSorted(Q3PtrList<const BosonProfilingItem>* ret, const struct timeval& since) const
{
 BO_CHECK_NULL_RET(ret);
 ret->clear();

 MySortedProfilingList list;
 getItemsSince(&list, since);

 // AB: we could probably make this faster by writing our own sorting algorithm.
 // the storages are already sorted correctly, we just need to merge the items
 // from all storages into a single (sorted) list.
 list.sort();

 Q3PtrListIterator<const BosonProfilingItem> it(list);
 while (it.current()) {
	ret->append(it.current());
	++it;
 }
}

Q3PtrList<BosonProfilingItem> BosonProfiling::cloneItems(const QString& storageName) const
{
 BosonProfilingStorage* storage = d->mStorages[storageName];
 if (!storage) {
	return Q3PtrList<BosonProfilingItem>();
 }
 return storage->cloneItems();
}

Q3PtrList<BosonProfilingItem> BosonProfiling::cloneItems() const
{
 Q3PtrList<BosonProfilingItem> list;
 Q3DictIterator<BosonProfilingStorage> it(d->mStorages);
 while (it.current()) {
	Q3PtrList<BosonProfilingItem> l = it.current()->cloneItems();
	Q3PtrListIterator<BosonProfilingItem> it2(l);
	while (it2.current()) {
		list.append(it2.current());
		++it2;
	}
	++it;
 }
 return list;
}

void BosonProfiling::setPopTask(BosonProfilingPopTask* task)
{
 d->mPopTask = task;
}

BosonProfiler::BosonProfiler(const QString& name, const QString& storageName)
	: mPopped(false),
	mPopStorage(false)
{
 mItem = boProfiling->push(name);
 if (!storageName.isEmpty()) {
	boProfiling->pushStorage(storageName);
	mPopStorage = true;
 }
}

