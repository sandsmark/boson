/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bosonprofiling.h"
#include "bosonprofilingprivate.h"

#include <klocale.h>
#include <klistview.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include <qlabel.h>
#include <qvbox.h>
#include <qpushbutton.h>

class QListViewItemNumber : public QListViewItem
{
public:
	QListViewItemNumber(QListView* p) : QListViewItem(p)
	{
	}
	QListViewItemNumber(QListViewItem* p) : QListViewItem(p)
	{
	}

	virtual int compare(QListViewItem* i, int col, bool ascending) const
	{
		bool ok = true;
		bool ok2 = true;
		int n = key(col, ascending).toInt(&ok);
		int n2 = i->key(col, ascending).toInt(&ok2);
		// numbers first - then letters
		if (ok && ok2) {
			if (n == n2) {
				return 0;
			} else if (n > n2) {
				return 1;
			} else {
				return -1;
			}
		} else if (ok) {
			// this is a number, i is not. this comes first.
			return -1;
		} else if (ok2) {
			// this is a noat number, i is. i comes first.
			return 1;
		} else {
			return QListViewItem::compare(i, col, ascending);
		}
	}
};

class BosonProfilingDialog::BosonProfilingDialogPrivate
{
public:
	BosonProfilingDialogPrivate()
	{
		mUnits = 0;
		mRender = 0;
		mEvents = 0;
		mCurrentFile = 0;

		mData = 0;
	}

	BosonProfiling* data() const { return mData ? mData : boProfiling; }

	KListView* mUnits;
	KListView* mRender;
	KListView* mEvents;
	QLabel* mCurrentFile;
	QString mCurrentFileName;

	BosonProfiling* mData;
};

BosonProfilingDialog::BosonProfilingDialog(QWidget* parent, bool modal)
		: KDialogBase(Tabbed, i18n("Boson Profiling"), Ok,
		Ok, parent, "bosonprofilingdialog", modal, true)
{
 d = new BosonProfilingDialogPrivate;

 initLoadUnitPage();
 initRenderPage();
 initEventsPage();
 initFilesPage();
 reset();
}

BosonProfilingDialog::~BosonProfilingDialog()
{
 d->mUnits->clear();
 d->mRender->clear();
 d->mEvents->clear();
 delete d->mData;
 delete d;
}

void BosonProfilingDialog::initLoadUnitPage()
{
 QVBox* vbox = addVBoxPage(i18n("&Load Unit"));

 d->mUnits = new KListView(vbox);
 d->mUnits->setRootIsDecorated(true);
 d->mUnits->addColumn(i18n("UnitTypes"));
 d->mUnits->addColumn(i18n("Time"));
 d->mUnits->addColumn(i18n("Time (ms)"));
 d->mUnits->addColumn(i18n("Time (s)"));
}

void BosonProfilingDialog::initRenderPage()
{
 QVBox* vbox = addVBoxPage(i18n("&Render"));

 // column ids
 d->mRender = new KListView(vbox);
 d->mRender->setRootIsDecorated(true);
 d->mRender->addColumn(i18n("Call number"));
 d->mRender->addColumn(i18n("Type"));
 d->mRender->addColumn(i18n("Time"));
 d->mRender->addColumn(i18n("Time (ms)"));
 d->mRender->addColumn(i18n("Time (s)"));
}

void BosonProfilingDialog::initEventsPage()
{
 QVBox* vbox = addVBoxPage(i18n("&Events"));
 d->mEvents = new KListView(vbox);
 d->mEvents->setRootIsDecorated(true);
 d->mEvents->addColumn(i18n("Event"));
 d->mEvents->addColumn(i18n("Time"));
 d->mEvents->addColumn(i18n("Time (ms)"));
 d->mEvents->addColumn(i18n("Time (s)"));
}

QString BosonProfilingDialog::profilingName(int profilingEvent) const
{
 QString name;
 switch ((BosonProfiling::ProfilingEvent)profilingEvent) {
	case BosonProfiling::ProfilingStart:
		name = i18n("ProfilingStart (Invalid here! Please send a Bug report");
		break;
	case BosonProfiling::ProfilingEnd:
		name = i18n("ProfilingEnd (Invalid here! Please send a Bug report");
		break;
	case BosonProfiling::LoadGameData1:
		name = i18n("LoadGameData1");
		break;
	case BosonProfiling::LoadGameData2:
		name = i18n("LoadGameData2");
		break;
	case BosonProfiling::LoadGameData3:
		name = i18n("LoadGameData3");
		break;
	case BosonProfiling::LoadModel:
		name = i18n("LoadModel");
		break;
	case BosonProfiling::LoadModelTextures:
		name = i18n("LoadModelTextures");
		break;
	case BosonProfiling::LoadModelDisplayLists:
		name = i18n("LoadModelDisplayLists");
		break;
	case BosonProfiling::LoadModelDummy:
		name = i18n("LoadModelDummy");
		break;
	default:
		name = i18n("Unknown %1").arg(profilingEvent);
		break;
 }
 return name;
}

void BosonProfilingDialog::initFilesPage()
{
 QVBox* vbox = addVBoxPage(i18n("&Files"));
 d->mCurrentFile = new QLabel(vbox);
 QHBox* hbox = new QHBox(vbox);
 QPushButton* open = new QPushButton(hbox);
 open->setText(i18n("&Load From File"));
 connect(open, SIGNAL(clicked()), this, SLOT(slotLoadFromFile()));

 QPushButton* save = new QPushButton(hbox);
 save->setText(i18n("&Save To File"));
 connect(save, SIGNAL(clicked()), this, SLOT(slotSaveToFile()));

 QPushButton* reset = new QPushButton(vbox);
 reset->setText(i18n("Update profiling data (inaccurate because the profiling dialog is on top of the GL screen!"));
 connect(reset, SIGNAL(clicked()), this, SLOT(slotUpdate()));
}

void BosonProfilingDialog::reset()
{
 resetLoadUnitPage();
 resetRenderPage();
 resetEventsPage();
 resetFilesPage();
}

void BosonProfilingDialog::resetLoadUnitPage()
{
 d->mUnits->clear();
 BosonProfiling::BosonProfilingPrivate* pd = d->data()->d;
 QMap<unsigned long int, BosonProfiling::BosonProfilingPrivate::TimesList>::Iterator it = pd->mUnitTimes.begin();
 for (; it != pd->mUnitTimes.end(); ++it) {
	QListViewItemNumber* unit = new QListViewItemNumber(d->mUnits);
	unit->setText(0, QString::number(it.key()));
	BosonProfiling::BosonProfilingPrivate::TimesList::Iterator timesIt = (*it).begin(); // wow what a line ;)
	long int time = 0;
	int i = 0;
	for (; timesIt != (*it).end(); ++timesIt, i++) {
		QListViewItemNumber* item = new QListViewItemNumber(unit);
		item->setText(0, QString::number(i));
		item->setText(1, QString::number(*timesIt));
		item->setText(2, QString::number((double)*timesIt / 1000));
		item->setText(3, QString::number((double)*timesIt / 1000000));
		time += *timesIt;
	}
	unit->setText(1, QString::number(time)); // all summed up
	unit->setOpen(true);
 }
}

void BosonProfilingDialog::resetRenderPage()
{
 d->mRender->clear();
 BosonProfiling::BosonProfilingPrivate* pd = d->data()->d;
 QValueList<RenderGLTimes>::Iterator it = pd->mRenderTimes.begin();
 int i = 0;
 for (; it != pd->mRenderTimes.end(); ++it, i++) {
	QListViewItemNumber* item = new QListViewItemNumber(d->mRender);
	item->setText(0, QString::number(i));
	createRenderItem(item, i18n("Clearing"), (*it).mClear);
	createRenderItem(item, i18n("Cells"), (*it).mCells);
	createRenderItem(item, i18n("Units"), (*it).mUnits);
	createRenderItem(item, i18n("FOW"), (*it).mFOW);
	createRenderItem(item, i18n("Text"), (*it).mText);
	createRenderItem(item, i18n("Function"), (*it).mFunction);
	item->setOpen(true);
 }
}

void BosonProfilingDialog::resetFilesPage()
{
 if (d->mData) {
	d->mCurrentFile->setText(i18n("Using file: %1").arg(d->mCurrentFileName));
 } else {
	d->mCurrentFile->setText(i18n("Using current data"));
 }
}

void BosonProfilingDialog::createRenderItem(QListViewItem* parent, const QString& type, long int time)
{
 QListViewItemNumber* item = new QListViewItemNumber(parent);
 item->setText(1, type);
 item->setText(2, QString::number(time));
 item->setText(3, QString::number((double)time / 1000));
 item->setText(4, QString::number((double)time / 1000000));
}

void BosonProfilingDialog::resetEventsPage()
{
 d->mEvents->clear();
 BosonProfiling::BosonProfilingPrivate* pd = d->data()->d;
 QMap<int, BosonProfiling::BosonProfilingPrivate::TimesList>::Iterator it = pd->mTimes.begin(); // now *that* is an ugly line! ggg
 for (; it != pd->mTimes.end(); ++it) {
	QListViewItemNumber* event = new QListViewItemNumber(d->mEvents);
	event->setText(0, profilingName(it.key()));
	BosonProfiling::BosonProfilingPrivate::TimesList::Iterator timesIt = (*it).begin();
	int i = 0;
	for (; timesIt != (*it).end(); ++timesIt, i++) {
		QListViewItemNumber* item = new QListViewItemNumber(event);
		item->setText(0, QString::number(i));
		item->setText(1, QString::number(*timesIt));
		item->setText(2, QString::number((double)*timesIt / 1000));
		item->setText(3, QString::number((double)*timesIt / 1000000));
	}
	// probably it doesn't make sense to sum all values up
	event->setOpen(true);
 }
}

void BosonProfilingDialog::slotUpdate()
{
 d->mData = 0;
 reset();
}

void BosonProfilingDialog::slotSaveToFile()
{
 QString file;
 file = KFileDialog::getSaveFileName();
 if (file == QString::null) {
	return;
 }
 if (!d->data()->saveToFile(file)) {
	KMessageBox::sorry(this, i18n("Unable to save to file %1").arg(file));
 }
}

void BosonProfilingDialog::slotLoadFromFile()
{
 QString file;
 file = KFileDialog::getOpenFileName();
 if (file == QString::null) {
	return;
 }
 loadFromFile(file);
}

void BosonProfilingDialog::loadFromFile(const QString& file)
{
 delete d->mData;
 d->mData = new BosonProfiling;
 if (!d->mData->loadFromFile(file)) {
	KMessageBox::sorry(this, i18n("Unable to load from file %1").arg(file));
	delete d->mData;
	return;
 }
 d->mCurrentFileName = file;
 reset();
}

