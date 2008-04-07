/*
    This file is part of the Boson game
    Copyright (C) 2002-2006 Andreas Beckermann (b_mann@gmx.de)

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
#include "bosonprofilingdialog.h"
#include "bosonprofilingdialog.moc"

#include "../bomemory/bodummymemory.h"
#include "bosonprofiling.h"
#include "bodebug.h"
#include "bofiledialog.h"
#include "qlistviewitemnumber.h"
//Added by qt3to4:
#include <Q3ValueList>
#include <Q3PtrList>
#include <Q3VBoxLayout>

#include <klocale.h>
#include <k3listview.h>
#include <kmessagebox.h>

#include <qlabel.h>
#include <qlayout.h>
#include <q3vbox.h>
#include <q3hbox.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qdatetime.h>
#include <q3intdict.h>
#include <q3memarray.h>
#include <q3dict.h>


/**
 * Stores all occurances of a certain event, no matter where in the tree it
 * occurs
 **/
class ProfilingEventSum
{
public:
	ProfilingEventSum(const QString& name)
	{
		mName = name;
		mTotalCalls = 0;
		mTime = 0;
	}
	~ProfilingEventSum()
	{
	}

	void addCall(const BosonProfilingItem* item, const BosonProfilingItem* caller)
	{
		BO_CHECK_NULL_RET(item);
		mTime += item->elapsedTime();
		mTotalCalls++;

		QString callerName;
		if (caller) {
			callerName = caller->name();
		}
		if (!mCallsBy.contains(callerName)) {
			mCallsBy.insert(callerName, 0);
		}
		mCallsBy[callerName] = mCallsBy[callerName] + 1;
	}

	const QString& name() const
	{
		return mName;
	}
	long int timeUs() const
	{
		return mTime;
	}
	unsigned int totalCalls() const
	{
		return mTotalCalls;
	}
	const QMap<QString, int>& callers() const
	{
		return mCallsBy;
	}

private:
	QString mName;
	long int mTime;
	unsigned int mTotalCalls;
	QMap<QString, int> mCallsBy;
};

class ProfilingEventSumCollection
{
public:
	ProfilingEventSumCollection()
	{
	}
	~ProfilingEventSumCollection()
	{
		QMap<QString, ProfilingEventSum*>::iterator it;
		for (it = mName2Sum.begin(); it != mName2Sum.end(); ++it) {
			delete it.data();
		}
		mName2Sum.clear();
	}

	void add(const BosonProfilingItem* item, const BosonProfilingItem* calledBy)
	{
		BO_CHECK_NULL_RET(item);
		QString name = item->name();
		ProfilingEventSum* sum = 0;
		if (!mName2Sum.contains(name)) {
			sum = new ProfilingEventSum(name);
			mName2Sum.insert(name, sum);
		} else {
			sum = mName2Sum[name];
		}
		sum->addCall(item, calledBy);

		if (!item->children()) {
			return;
		}
		for (Q3PtrListIterator<BosonProfilingItem> it(*item->children()); it.current(); ++it) {
			add(it.current(), item);
		}
	}

	const ProfilingEventSum* sumForEvent(const QString& name) const
	{
		if (!mName2Sum.contains(name)) {
			return 0;
		}
		return mName2Sum[name];
	}

	Q3ValueList<QString> allEventNames() const
	{
		return mName2Sum.keys();
	}

private:
	QMap<QString, ProfilingEventSum*> mName2Sum;
};



class ProfilingItem
{
public:
	ProfilingItem(const QString& name);
	~ProfilingItem();

	void addItem(BosonProfilingItem* item);
	void insertChild(BosonProfilingItem* child);

	const QString& name() const { return mName; }
	long int time() const { return mTime; }
	const Q3PtrList<ProfilingItem>& children() const { return mChildren; }
	const Q3PtrList<BosonProfilingItem>& items() const { return mItems; }

private:
	QString mName;
	long int mTime;
	Q3PtrList<ProfilingItem> mChildren;
	Q3Dict<ProfilingItem> mName2Child;

	Q3PtrList<BosonProfilingItem> mItems;
};

ProfilingItem::ProfilingItem(const QString& name)
	: mName(name),
	  mTime(0)
{
}

ProfilingItem::~ProfilingItem()
{
 mItems.clear(); // no delete!
 mName2Child.clear(); // no delete!
 mChildren.setAutoDelete(true);
 mChildren.clear();
}

void ProfilingItem::addItem(BosonProfilingItem* item)
{
 BO_CHECK_NULL_RET(item);
 mItems.append(item);
 mTime += item->elapsedTime();
 if (!item->children()) {
	return;
 }
 Q3PtrListIterator<BosonProfilingItem> it(*item->children());
 while (it.current()) {
	insertChild(it.current());
	++it;
 }
}

void ProfilingItem::insertChild(BosonProfilingItem* item)
{
 BO_CHECK_NULL_RET(item);
 if (!mName2Child[item->name()]) {
	ProfilingItem* p = new ProfilingItem(item->name());
	mChildren.append(p);
	mName2Child.insert(item->name(), p);
 }
 ProfilingItem* p = mName2Child[item->name()];
 p->addItem(item);
}

class QListViewItemNumberTime : public QListViewItemNumber
{
public:
	QListViewItemNumberTime(Q3ListView* p) : QListViewItemNumber(p)
	{
	}
	QListViewItemNumberTime(Q3ListViewItem* p) : QListViewItemNumber(p)
	{
	}

	/**
	 * Note: this will take 4 columns! %, Time, Time (ms) and Time (s) !
	 * @param firstColumn The first time-column. We need 4 columns here!
	 * @param time How much time this action took
	 * @param function A reference value for @p time - usually how much time
	 * the entire function took where this action was made.
	 **/
	void setTime(int firstColumn, unsigned long int time, unsigned long int function)
	{
		setText(firstColumn, QString::number(double(time * 100) / function));
		setTime3(firstColumn + 1, time);
	}
	void setTime3(int firstColumn, unsigned long int time)
	{
		setText(firstColumn, QString::number(time));
		setText(firstColumn + 1, QString::number((double)time / 1000));
		setText(firstColumn + 2, QString::number((double)time / 1000000));
	}
};

class BosonProfilingDialogPrivate
{
public:
	BosonProfilingDialogPrivate()
	{
		mTopItem = 0;
		mEventSumCollection = 0;
	}

	BosonProfilingDialogGUI* mGUI;

	BosonProfiling mProfiling;
	Q3PtrList<BosonProfilingItem> mItems;

	ProfilingItem* mTopItem;
	ProfilingEventSumCollection* mEventSumCollection;
};

BosonProfilingDialog::BosonProfilingDialog(QWidget* parent, bool modal)
		: KDialog()
{
 setWindowTitle(makeStandardCaption(i18n("Boson Profiling")));
 setButtons(KDialog::Ok);
 setDefaultButton(KDialog::Ok);
 d = new BosonProfilingDialogPrivate;

 d->mGUI = new BosonProfilingDialogGUI(mainWidget());

 Q3VBoxLayout* layout = new Q3VBoxLayout(mainWidget());
 layout->addWidget(d->mGUI);

 d->mGUI->mEvents->setColumnWidthMode(0, Q3ListView::Manual);
 d->mGUI->mEvents->setColumnWidth(0, 400);
 d->mGUI->mEventLeafs->setColumnWidthMode(0, Q3ListView::Manual);
 d->mGUI->mEventLeafs->setColumnWidth(0, 400);
 d->mGUI->mRawTree->setColumnWidthMode(0, Q3ListView::Manual);
 d->mGUI->mRawTree->setColumnWidth(0, 400);

 connect(d->mGUI->mEvents, SIGNAL(currentChanged(Q3ListViewItem*)),
		this, SLOT(slotShowSumForEvent(Q3ListViewItem*)));
 connect(d->mGUI->mLoadFromFile, SIGNAL(clicked()),
		this, SLOT(slotLoadFromFile()));
 connect(d->mGUI->mSaveToFile, SIGNAL(clicked()),
		this, SLOT(slotSaveToFile()));
 connect(d->mGUI->mUpdateProfilingData, SIGNAL(clicked()),
		this, SLOT(slotUpdateFromGlobalProfiling()));

 slotUpdateFromGlobalProfiling();
}

BosonProfilingDialog::~BosonProfilingDialog()
{
 d->mGUI->mEvents->clear();
 d->mGUI->mEventLeafs->clear();
 d->mGUI->mRawTree->clear();

 d->mItems.setAutoDelete(true);
 d->mItems.clear();
 delete d->mTopItem;
 delete d->mEventSumCollection;
 delete d;
}

void BosonProfilingDialog::reset()
{
 resetEventsPage();
 resetEventLeafsPage();
 resetRawTreePage();
 resetFilesPage();
}

void BosonProfilingDialog::slotUpdateFromGlobalProfiling()
{
 d->mProfiling = *boProfiling;

 // update data from d->mProfiling
 slotUpdate();
}

void BosonProfilingDialog::slotUpdate()
{
 d->mItems.setAutoDelete(true);
 d->mItems.clear();
 d->mItems.setAutoDelete(false);

 // AB: d->mItems is kinda obsolete. we already clone all items by copying the
 //     profiling object (to d->mProfiling).
 d->mItems = d->mProfiling.cloneItems();
 delete d->mTopItem;

 d->mTopItem = new ProfilingItem(QString::null);
 for (Q3PtrListIterator<BosonProfilingItem> it(d->mItems); it.current(); ++it) {
	d->mTopItem->insertChild(it.current());
 }
 delete d->mEventSumCollection;
 d->mEventSumCollection = new ProfilingEventSumCollection();
 for (Q3PtrListIterator<BosonProfilingItem> it(d->mItems); it.current(); ++it) {
	d->mEventSumCollection->add(it.current(), 0);
 }

 reset();
}

void BosonProfilingDialog::slotSaveToFile()
{
 QString file = BoFileDialog::getSaveFileName();
 if (file.isEmpty()) {
	return;
 }
 if (QFile::exists(file)) {
	int ret = KMessageBox::questionYesNo(this, i18n("File %1 already exists. Overwrite?").arg(file));
	if (ret != KMessageBox::Yes) {
		return;
	}
 }
 QFile f(file);
 if (!f.open(QIODevice::WriteOnly)) {
	KMessageBox::sorry(this, i18n("File %1 could not be opened").arg(file));
	return;
 }
 QDataStream stream(&f);
 if (!d->mProfiling.save(stream)) {
	KMessageBox::sorry(this, i18n("Error while saving to %1").arg(file));
	return;
 }
 f.close();
}

void BosonProfilingDialog::slotLoadFromFile()
{
 int ret = KMessageBox::questionYesNo(this, i18n("This will clear all current profiling data. Do you reall want to load from a file?"));
 if (ret != KMessageBox::Yes) {
	return;
 }
 QString file = BoFileDialog::getOpenFileName();
 if (file.isEmpty()) {
	return;
 }
 loadFromFile(file);
}

void BosonProfilingDialog::loadFromFile(const QString& file)
{
 QFile f(file);
 if (!f.open(QIODevice::ReadOnly)) {
	KMessageBox::sorry(this, i18n("File %1 could not be opened").arg(file));
	return;
 }
 QDataStream stream(&f);
 if (!d->mProfiling.load(stream)) {
	KMessageBox::sorry(this, i18n("Error while loading from %1").arg(file));
	return;
 }
 f.close();

 slotUpdate();
}

void BosonProfilingDialog::resetFilesPage()
{
}

void BosonProfilingDialog::resetEventsPage()
{
 d->mGUI->mEvents->clear();

 initProfilingItem(0, d->mTopItem, -1);
}

void BosonProfilingDialog::resetEventLeafsPage()
{
 d->mGUI->mEventLeafs->clear();

 Q3ValueList<QString> events = d->mEventSumCollection->allEventNames();
 for (Q3ValueList<QString>::iterator it = events.begin(); it != events.end(); ++it) {
	const ProfilingEventSum* sum = d->mEventSumCollection->sumForEvent(*it);
	if (!sum) {
		continue;
	}

	QListViewItemNumberTime* item = new QListViewItemNumberTime(d->mGUI->mEventLeafs);
	item->setText(0, *it);

	unsigned int calls = sum->totalCalls();
	unsigned long int sumUs = sum->timeUs();
	unsigned long int averageUs = 0;
	if (calls > 0) {
		averageUs = sumUs / calls;
	}
	item->setText(1, QString::number(sumUs));
	item->setTime3(1, sumUs);
	item->setTime3(4, averageUs);
	item->setText(7, QString::number(calls));
 }

// initProfilingItem(0, d->mTopItem, -1);
}

void BosonProfilingDialog::resetRawTreePage()
{
 d->mGUI->mRawTree->clear();

 Q3PtrListIterator<BosonProfilingItem> it(d->mItems);
 while (it.current()) {
	initRawTreeProfilingItem(new QListViewItemNumberTime(d->mGUI->mRawTree), it.current(), -1);
	++it;
 }
}

void BosonProfilingDialog::initProfilingItem(QListViewItemNumberTime* item, ProfilingItem* profilingItem, long int totalTime)
{
 BO_CHECK_NULL_RET(profilingItem);
 if (item) {
	long int time = profilingItem->time();
	item->setText(0, profilingItem->name());
	if (totalTime < 0) { // top item
		totalTime = time;
	}

	// Sum:
	item->setTime(1, time, totalTime);

	// Average:
	if (profilingItem->items().isEmpty()) {
		item->setTime3(5, 0);
	} else {
		item->setTime3(5, time / profilingItem->items().count());
	}

	item->setText(8, QString::number(profilingItem->items().count()));
 }

 Q3PtrListIterator<ProfilingItem> it(profilingItem->children());
 while (it.current()) {
	QListViewItemNumberTime* child;
	if (item) {
		child = new QListViewItemNumberTime(item);
	} else {
		child = new QListViewItemNumberTime(d->mGUI->mEvents);
	}
	initProfilingItem(child, it.current(), totalTime);

	++it;
 }
}

void BosonProfilingDialog::initRawTreeProfilingItem(QListViewItemNumberTime* item, BosonProfilingItem* profilingItem, long int totalTime)
{
 BO_CHECK_NULL_RET(item);
 BO_CHECK_NULL_RET(profilingItem);
 long int time = profilingItem->elapsedTime();
 item->setText(0, profilingItem->name());
 if (totalTime < 0) { // top item
	totalTime = time;
 }
 item->setTime(1, time, totalTime);
 if (!profilingItem->children()) {
	return;
 }

 Q3PtrListIterator<BosonProfilingItem> it(*profilingItem->children());
 while (it.current()) {
	QListViewItemNumberTime* child;
	child = new QListViewItemNumberTime(item);
	initRawTreeProfilingItem(child, it.current(), totalTime);

	++it;
 }
}

void BosonProfilingDialog::slotShowSumForEvent(Q3ListViewItem* item)
{
 QString eventName;
 int calls = 0;
 long int sumUs = 0;
 double averageUs = 0;
 QMap<QString, int> callers;
 d->mGUI->mSelectedEventCalledBy->clear();
 if (item) {
	eventName = item->text(0);
	const ProfilingEventSum* sum = d->mEventSumCollection->sumForEvent(eventName);
	if (!sum) {
		eventName = i18n("(Could not find event. Internal error.)");
	} else {
		calls = sum->totalCalls();
		sumUs = sum->timeUs();
		callers = sum->callers();
	}
 }
 if (eventName.isNull()) {
	eventName = i18n("(no event selected)");
 }
 if (calls > 0) {
	averageUs = ((double)sumUs) / calls;
 }
 d->mGUI->mSelectedEventName->setText(eventName);
 d->mGUI->mSelectedEventCalls->setText(QString::number(calls));
 d->mGUI->mSelectedEventSum->setText(i18n("%1/%2/%3")
		.arg(QString::number(sumUs))
		.arg(QString::number(((double)sumUs) / 1000.0))
		.arg(QString::number(((double)sumUs) / 1000000.0)));
 d->mGUI->mSelectedEventAverage->setText(i18n("%1/%2/%3")
		.arg(QString::number(averageUs))
		.arg(QString::number(averageUs / 1000.0))
		.arg(QString::number(averageUs / 1000000.0)));

 d->mGUI->mSelectedEventCalledBy->clear();
 for (QMap<QString, int>::iterator it = callers.begin(); it != callers.end(); ++it) {
	Q3ListViewItem* item = new QListViewItemNumber(d->mGUI->mSelectedEventCalledBy);
	QString callerName = it.key();
	if (callerName.isNull()) {
		callerName = i18n("(toplevel event)");
	}
	item->setText(0, callerName);
	item->setText(1, QString::number(it.data()));
 }
}

