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
#include <kdebug.h>

#include <qvbox.h>
#include <klistview.h>

class BosonProfilingDialog::BosonProfilingDialogPrivate
{
public:
	BosonProfilingDialogPrivate()
	{
		mUnits = 0;
		mRender = 0;
		mEvents = 0;
	}

	KListView* mUnits;
	KListView* mRender;
	KListView* mEvents;
};

BosonProfilingDialog::BosonProfilingDialog(QWidget* parent, bool modal)
		: KDialogBase(Tabbed, i18n("Boson Profiling"), Ok,
		Ok, parent, "bosonprofilingdialog", modal, true)
{
 d = new BosonProfilingDialogPrivate;
 
 initLoadUnitPage();
 initRenderPage();
 initEventsPage();
}

BosonProfilingDialog::~BosonProfilingDialog()
{
 d->mUnits->clear();
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
 BosonProfiling::BosonProfilingPrivate* pd = boProfiling->d;
 QMap<int, BosonProfiling::BosonProfilingPrivate::TimesList>::Iterator it = pd->mUnitTimes.begin();
 for (; it != pd->mUnitTimes.end(); ++it) {
	QListViewItem* unit = new QListViewItem(d->mUnits);
	QString unitType;
	unitType.sprintf("%05d", it.key());
	unit->setText(0, unitType);
	BosonProfiling::BosonProfilingPrivate::TimesList::Iterator timesIt = (*it).begin(); // wow what a line ;)
	long int time = 0;
	int i = 0;
	for (; timesIt != (*it).end(); ++timesIt, i++) {
		QListViewItem* item = new QListViewItem(unit);
		QString num;
		num.sprintf("%03d", i);
		item->setText(0, num);
		item->setText(1, QString::number(*timesIt));
		item->setText(2, QString::number((double)*timesIt / 1000));
		item->setText(3, QString::number((double)*timesIt / 1000000));
		time += *timesIt;
	}
	unit->setText(1, QString::number(time)); // all summed up
	unit->setOpen(true);
 }
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

 BosonProfiling::BosonProfilingPrivate* pd = boProfiling->d;
 QValueList<RenderGLTimes>::Iterator it = pd->mRenderTimes.begin();
 int i = 0;
 for (; it != pd->mRenderTimes.end(); ++it, i++) {
	QListViewItem* item = new QListViewItem(d->mRender);
	QString num;
	num.sprintf("%03d", i);
	item->setText(0, num);
	createRenderItem(item, i18n("Clearing"), (*it).mClear);
	createRenderItem(item, i18n("Cells"), (*it).mCells);
	createRenderItem(item, i18n("Units"), (*it).mUnits);
	createRenderItem(item, i18n("Text"), (*it).mText);
	createRenderItem(item, i18n("Function"), (*it).mFunction);
	item->setOpen(true);
 }
}

void BosonProfilingDialog::createRenderItem(QListViewItem* parent, const QString& type, long int time)
{
 QListViewItem* item = new QListViewItem(parent);
 item->setText(1, type);
 item->setText(2, QString::number(time));
 item->setText(3, QString::number((double)time / 1000));
 item->setText(4, QString::number((double)time / 1000000));
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
 BosonProfiling::BosonProfilingPrivate* pd = boProfiling->d;
 QMap<BosonProfiling::ProfilingEvent, BosonProfiling::BosonProfilingPrivate::TimesList>::Iterator it = pd->mTimes.begin(); // now *that* is an ugly line! ggg
 for (; it != pd->mTimes.end(); ++it) {
	QListViewItem* event = new QListViewItem(d->mEvents);
	event->setText(0, profilingName(it.key()));
	BosonProfiling::BosonProfilingPrivate::TimesList::Iterator timesIt = (*it).begin();
	int i = 0;
	for (; timesIt != (*it).end(); ++timesIt, i++) {
		QListViewItem* item = new QListViewItem(event);
		item->setText(0, QString::number(i));
		QString num;
		num.sprintf("%03d", i);
		item->setText(0, num);
		item->setText(1, QString::number(*timesIt));
		item->setText(2, QString::number((double)*timesIt / 1000));
		item->setText(3, QString::number((double)*timesIt / 1000000));
	}
	// probably it doesn't make sense to sum all values up
	event->setOpen(true);
 }
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

