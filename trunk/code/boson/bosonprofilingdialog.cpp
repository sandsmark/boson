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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#include "bosonprofilingdialog.h"
#include "bosonprofilingdialog.moc"

#include "../bomemory/bodummymemory.h"
#include "bosonprofiling.h"
#include "bodebug.h"
#include "bofiledialog.h"
#include "qlistviewitemnumber.h"
#include "bosonprofilingdialoggui.h"

#include <klocale.h>
#include <klistview.h>
#include <kmessagebox.h>

#include <qlabel.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qdatetime.h>
#include <qintdict.h>
#include <qmemarray.h>
#include <qdict.h>

class ProfilingItem
{
public:
	ProfilingItem(const QString& name);
	~ProfilingItem();

	void addItem(BosonProfilingItem* item);
	void insertChild(BosonProfilingItem* child);

	const QString& name() const { return mName; }
	long int time() const { return mTime; }
	const QPtrList<ProfilingItem>& children() const { return mChildren; }
	const QPtrList<BosonProfilingItem>& items() const { return mItems; }

private:
	QString mName;
	long int mTime;
	QPtrList<ProfilingItem> mChildren;
	QDict<ProfilingItem> mName2Child;

	QPtrList<BosonProfilingItem> mItems;
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
 QPtrListIterator<BosonProfilingItem> it(*item->children());
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
	QListViewItemNumberTime(QListView* p) : QListViewItemNumber(p)
	{
	}
	QListViewItemNumberTime(QListViewItem* p) : QListViewItemNumber(p)
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
	}

	BosonProfilingDialogGUI* mGUI;

	BosonProfiling mProfiling;
	QPtrList<BosonProfilingItem> mItems;

	ProfilingItem* mTopItem;
};

BosonProfilingDialog::BosonProfilingDialog(QWidget* parent, bool modal)
		: KDialogBase(Plain, i18n("Boson Profiling"), Ok,
		Ok, parent, "bosonprofilingdialog", modal, true)
{
 d = new BosonProfilingDialogPrivate;

 d->mGUI = new BosonProfilingDialogGUI(plainPage());

 QVBoxLayout* layout = new QVBoxLayout(plainPage());
 layout->addWidget(d->mGUI);

 d->mGUI->mEvents->setColumnWidthMode(0, QListView::Manual);
 d->mGUI->mEvents->setColumnWidth(0, 400);
 d->mGUI->mRawTree->setColumnWidthMode(0, QListView::Manual);
 d->mGUI->mRawTree->setColumnWidth(0, 400);

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
 d->mGUI->mRawTree->clear();

 d->mItems.setAutoDelete(true);
 d->mItems.clear();
 delete d->mTopItem;
 delete d;
}

void BosonProfilingDialog::reset()
{
 resetEventsPage();
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
 QPtrListIterator<BosonProfilingItem> it(d->mItems);
 while (it.current()) {
	d->mTopItem->insertChild(it.current());
	++it;
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
 if (!f.open(IO_WriteOnly)) {
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
 if (!f.open(IO_ReadOnly)) {
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

void BosonProfilingDialog::resetRawTreePage()
{
 d->mGUI->mRawTree->clear();

 QPtrListIterator<BosonProfilingItem> it(d->mItems);
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

 QPtrListIterator<ProfilingItem> it(profilingItem->children());
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

 QPtrListIterator<BosonProfilingItem> it(*profilingItem->children());
 while (it.current()) {
	QListViewItemNumberTime* child;
	child = new QListViewItemNumberTime(item);
	initRawTreeProfilingItem(child, it.current(), totalTime);

	++it;
 }
}

