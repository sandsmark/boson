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
#include "bodebug.h"
#include "rtti.h"

#include <klocale.h>
#include <klistview.h>
#include <kfiledialog.h>
#include <kmessagebox.h>

#include <qlabel.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qdatetime.h>

#warning workaround only!
// FIXME: these should not be in this file at all (not even in the class). maybe
// as parameters to addItemAdvance() or so... but not global in any way
unsigned long int itemFunctionSum;
unsigned long int itemAdvanceSum;
unsigned long int itemAdvanceFunctionSum;
unsigned long int itemMoveSum;
unsigned int itemAllItems;

class QListViewItemNumber : public QListViewItem
{
public:
	QListViewItemNumber(QListView* p) : QListViewItem(p)
	{
	}
	QListViewItemNumber(QListViewItem* p) : QListViewItem(p)
	{
	}

	/**
	 * Note: this will take 4 columns! Time, Time (ms), Time (s) and % !
	 * @param firstColumn The first time-column. We need 4 columns here!
	 * @param time How much time this action took
	 * @param function A reference value for @p time - usually how much time
	 * the entire function took where this action was made.
	 **/
	void setTime(int firstColumn, unsigned long int time, unsigned long int function)
	{
		setText(firstColumn, QString::number(time));
		setText(firstColumn + 1, QString::number((double)time / 1000));
		setText(firstColumn + 2, QString::number((double)time / 1000000));
		setText(firstColumn + 3, QString::number(double(time * 100) / function));
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

class RenderSummary : public QWidget
{
public:
	RenderSummary(QWidget* parent) : QWidget(parent, "RenderSummary")
	{
		QGridLayout* topLayout = new QGridLayout(this);
		int row = 0;
		int column = 0;
		QLabel* label = new QLabel(i18n("First profiled frame at:"), this);
		mStartLabel = new QLabel(this);
		topLayout->addWidget(label, row, column);
		column++;
		topLayout->addWidget(mStartLabel, 0, column);
		column++;

		label = new QLabel(i18n("Last profiled frame at:"), this);
		mEndLabel = new QLabel(this);
		topLayout->addWidget(label, 0, column);
		column++;
		topLayout->addWidget(mEndLabel, 0, column);

		row++;
		column = 0;
		label = new QLabel(i18n("Elapsed seconds: "), this);
		mSeconds = new QLabel(this);
		topLayout->addWidget(label, row, column);
		column++;
		topLayout->addWidget(mSeconds, row, column);

		row++;
		column = 0;
		label = new QLabel(i18n("Frame count in this time: "), this);
		mCountLabel = new QLabel(this);
		topLayout->addWidget(label, row, column);
		column++;
		topLayout->addWidget(mCountLabel, row, column);
		column++;

		label = new QLabel(i18n("FPS: "), this);
		mFPSLabel = new QLabel(this);
		topLayout->addWidget(label, row, column);
		column++;
		topLayout->addWidget(mFPSLabel, row, column);
		column++;
	}
	~RenderSummary()
	{
	}

	void clear()
	{
		mStartLabel->setText("");
		mEndLabel->setText("");
		mSeconds->setText("");
		mCountLabel->setText("");
		mFPSLabel->setText("");
	}

	void set(struct timeval* start, struct timeval* end, int count)
	{
		if (!start || !end || count == 0) {
			clear();
			return;
		}
		QDateTime s, e;
		s.setTime_t(start->tv_sec);
		e.setTime_t(end->tv_sec);
		mStartLabel->setText(s.time().toString());
		mEndLabel->setText(e.time().toString());
		mCountLabel->setText(QString::number(count));

		double diff = (end->tv_sec - start->tv_sec) * 1000000 + (end->tv_usec - start->tv_usec);
		diff /= 1000000;
		mSeconds->setText(QString::number(diff));
		double fps = ((double)count) / diff;
		mFPSLabel->setText(QString::number(fps));
	}

private:
	QLabel* mStartLabel;
	QLabel* mEndLabel;
	QLabel* mSeconds;
	QLabel* mCountLabel;
	QLabel* mFPSLabel;
};

class BosonProfilingDialog::BosonProfilingDialogPrivate
{
public:
	BosonProfilingDialogPrivate()
	{
		mUnits = 0;
		mRender = 0;
		mRenderSummary = 0;
		mSlotAdvance = 0;
		mItemAdvance = 0;
		mCreateItemAdvanceSummaryOnly = 0;
		mEvents = 0;
		mCurrentFile = 0;

		mData = 0;
	}

	BosonProfiling* data() const { return mData ? mData : boProfiling; }

	KListView* mUnits;
	KListView* mRender;
	RenderSummary* mRenderSummary;
	KListView* mSlotAdvance;
	KListView* mItemAdvance;
	QCheckBox* mCreateItemAdvanceSummaryOnly;
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
 if (boProfiling) {
	d->mData = new BosonProfiling(*boProfiling);
 }

 initLoadUnitPage();
 initRenderPage();
 initSlotAdvancePage();
 initEventsPage();
 initFilesPage();
 reset();
}

BosonProfilingDialog::~BosonProfilingDialog()
{
 d->mUnits->clear();
 d->mRender->clear();
 d->mSlotAdvance->clear();
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
 d->mUnits->addColumn(i18n("%")); // unused, but we can use convenience methods then when creating the items
}

void BosonProfilingDialog::initRenderPage()
{
 QVBox* vbox = addVBoxPage(i18n("&Render"));

 d->mRender = new KListView(vbox);
 d->mRender->setRootIsDecorated(true);
 d->mRender->addColumn(i18n("Call number"));
 d->mRender->addColumn(i18n("Type"));
 d->mRender->addColumn(i18n("Time"));
 d->mRender->addColumn(i18n("Time (ms)"));
 d->mRender->addColumn(i18n("Time (s)"));
 d->mRender->addColumn(i18n("%"));

 d->mRenderSummary = new RenderSummary(vbox);
}

void BosonProfilingDialog::initSlotAdvancePage()
{
 QVBox* vbox = addVBoxPage(i18n("&Slot Advance"));

 d->mSlotAdvance = new KListView(vbox);
 d->mSlotAdvance->setRootIsDecorated(true);
 d->mSlotAdvance->addColumn(i18n("Number"));
 d->mSlotAdvance->addColumn(i18n("Advance Count"));
 d->mSlotAdvance->addColumn(i18n("Type"));
 d->mSlotAdvance->addColumn(i18n("Time"));
 d->mSlotAdvance->addColumn(i18n("Time (ms)"));
 d->mSlotAdvance->addColumn(i18n("Time (s)"));
 d->mSlotAdvance->addColumn(i18n("%"));

 d->mItemAdvance = new KListView(vbox);
 d->mItemAdvance->setRootIsDecorated(true);
 d->mItemAdvance->addColumn(i18n("Rtti"));
 d->mItemAdvance->addColumn(i18n("IsUnit"));
 d->mItemAdvance->addColumn(i18n("ID"));
 d->mItemAdvance->addColumn(i18n("Work"));
 d->mItemAdvance->addColumn(i18n("Advance Count"));
 d->mItemAdvance->addColumn(i18n("Type"));
 d->mItemAdvance->addColumn(i18n("Time"));
 d->mItemAdvance->addColumn(i18n("Time (ms)"));
 d->mItemAdvance->addColumn(i18n("Time (s)"));
 d->mItemAdvance->addColumn(i18n("%"));

 QHBox* controlBox = new QHBox(vbox);
 d->mCreateItemAdvanceSummaryOnly = new QCheckBox(i18n("Item Advance Summary Only"), controlBox);
 d->mCreateItemAdvanceSummaryOnly->setChecked(true);
 QPushButton* reset = new QPushButton(i18n("Reset Values"), controlBox);
 connect(reset, SIGNAL(clicked()), this, SLOT(slotResetSlotAdvancePage()));
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
 d->mEvents->addColumn(i18n("%")); // unused, but we can use convenience methods then when creating the items
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
		name = i18n("LoadGameData1 (currently unused)");
		break;
	case BosonProfiling::LoadTiles:
		name = i18n("LoadTiles");
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
 slotResetSlotAdvancePage();
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
	unsigned long int time = 0;
	int i = 0;
	for (; timesIt != (*it).end(); ++timesIt, i++) {
		QListViewItemNumber* item = new QListViewItemNumber(unit);
		item->setText(0, QString::number(i));
		item->setTime(1, time, time);
		time += *timesIt;
	}
	unit->setText(1, QString::number(time)); // all summed up
	unit->setOpen(true);
 }
}

void BosonProfilingDialog::resetRenderPage()
{
 d->mRender->clear();
 d->mRenderSummary->clear();
 BosonProfiling::BosonProfilingPrivate* pd = d->data()->d;
 QPtrListIterator<RenderGLTimes> it(pd->mRenderTimes);
 int i = 0;
 // average values:
 long int aClear = 0;
 long int aCells = 0;
 long int aUnits = 0;
 unsigned int aUnitCount = 0; // hmm average for this does not really make sense
 long int aMissiles = 0;
 long int aParticles = 0;
 long int aFOW = 0;
 long int aText = 0;
 unsigned long int aFunction = 0;
 for (; it.current(); ++it, i++) {
	unsigned long int func = it.current()->dFunction();
	QListViewItemNumber* item = new QListViewItemNumber(d->mRender);
	item->setText(0, QString::number(i));
	initRenderItem(item, i18n("Function"), it.current()->dFunction(), func);
	aFunction += it.current()->dFunction();

	initRenderItem(new QListViewItemNumber(item), i18n("Clearing"), it.current()->dClear(), func);
	aClear += it.current()->dClear();
	initRenderItem(new QListViewItemNumber(item), i18n("Cells"), it.current()->dCells(), func);
	aCells += it.current()->dCells();
	initRenderItem(new QListViewItemNumber(item), i18n("Units (%1)").arg(it.current()->mUnitCount), it.current()->dUnits(), func);
	aUnitCount += it.current()->mUnitCount;
	aUnits += it.current()->dUnits();
	initRenderItem(new QListViewItemNumber(item), i18n("Missiles"), it.current()->dMissiles(), func);
	aMissiles += it.current()->dMissiles();
	initRenderItem(new QListViewItemNumber(item), i18n("Particles"), it.current()->dParticles(), func);
	aParticles += it.current()->dParticles();
	initRenderItem(new QListViewItemNumber(item), i18n("FOW"), it.current()->dFOW(), func);
	aFOW += it.current()->dFOW();
	initRenderItem(new QListViewItemNumber(item), i18n("Text"), it.current()->dText(), func);
	aText += it.current()->dText();
	item->setOpen(true);
 }

 unsigned int count = pd->mRenderTimes.count();
 if (!count) {
	d->mRenderSummary->set(0, 0, 0);
	return;
 }
 unsigned long int func = aFunction / count;
 QListViewItemNumber* average = new QListViewItemNumber(d->mRender);
 average->setText(0, i18n("Average - use with care"));
 initRenderItem(average, i18n("Function"), aFunction / count, func);
 initRenderItem(new QListViewItemNumber(average), i18n("Clearing"), aClear / count, func);
 initRenderItem(new QListViewItemNumber(average), i18n("Cells"), aCells / count, func);
 initRenderItem(new QListViewItemNumber(average), i18n("Units (%1)").arg(aUnitCount / count), aUnits / count, func);
 initRenderItem(new QListViewItemNumber(average), i18n("Missiles"), aMissiles / count, func);
 initRenderItem(new QListViewItemNumber(average), i18n("Particles"), aParticles / count, func);
 initRenderItem(new QListViewItemNumber(average), i18n("FOW"), aFOW / count, func);
 initRenderItem(new QListViewItemNumber(average), i18n("Text"), aText / count, func);
 average->setOpen(true);


 d->mRenderSummary->set(&pd->mRenderTimes.first()->mFunction.mData[0],
		&pd->mRenderTimes.last()->mFunction.mData[1],
		pd->mRenderTimes.count());
}

void BosonProfilingDialog::slotResetSlotAdvancePage()
{
 if (!d->mCreateItemAdvanceSummaryOnly->isChecked()) {
	int r = KMessageBox::questionYesNo(this, i18n("You selected to display advance information for ALL items - this will take a very long time.\nDo you really want to do this?"));
	if (r != KMessageBox::Yes) {
		return;
	}
 }
 d->mSlotAdvance->clear();
 d->mItemAdvance->clear();
 itemFunctionSum = itemAdvanceSum = itemAdvanceFunctionSum = itemMoveSum = 0;
 itemAllItems = 0;
 BosonProfiling::BosonProfilingPrivate* pd = d->data()->d;
 QPtrListIterator<ProfileSlotAdvance> it(pd->mSlotAdvanceTimes);
 int i = 0;
 unsigned long int functionSum = 0;
 unsigned long int advanceFunctionSum = 0;
 unsigned long int deleteUnusedShotsSum = 0;
 unsigned long int particlesSum = 0;
 unsigned long int maximalAdvanceCountSum = 0;

 for (; it.current(); ++it, i++) {
	unsigned long int func = it.current()->dFunction();
	unsigned int call = it.current()->mAdvanceCount;
	QListViewItemNumber* item = new QListViewItemNumber(d->mSlotAdvance);
	item->setText(0, QString::number(i));
	initSlotAdvanceItem(item, call,i18n("Function"), it.current()->dFunction(), func);
	functionSum += it.current()->dFunction();

	initSlotAdvanceItem(new QListViewItemNumber(item), call, i18n("AdvanceFunctions"), it.current()->dAdvanceFunction(), func);
	advanceFunctionSum += it.current()->dAdvanceFunction();
	initSlotAdvanceItem(new QListViewItemNumber(item), call, i18n("DeleteUnusedShots"), it.current()->dDeleteUnusedShots(), func);
	deleteUnusedShotsSum += it.current()->dDeleteUnusedShots();
	initSlotAdvanceItem(new QListViewItemNumber(item), call, i18n("Particles"), it.current()->dParticles(), func);
	particlesSum += it.current()->dParticles();
	initSlotAdvanceItem(new QListViewItemNumber(item), call, i18n("MaximalAdvancCount"), it.current()->dMaximalAdvanceCount(), func);
	maximalAdvanceCountSum += it.current()->dMaximalAdvanceCount();

	item->setOpen(true);

	// add values for the units, missiles, ...
	addItemAdvance(it.current());
 }

 unsigned int count = pd->mSlotAdvanceTimes.count();
 if (!count) {
	return;
 }
 addItemAdvanceSummary();

 unsigned long int func = functionSum / count;
 QListViewItemNumber* average = new QListViewItemNumber(d->mSlotAdvance);
 average->setText(0, i18n("Average - use with care"));
 initSlotAdvanceItem(average, 0, i18n("Function"), functionSum / count, func);
 initSlotAdvanceItem(new QListViewItemNumber(average), 0, i18n("AdvanceFunctions"), advanceFunctionSum / count, func);
 initSlotAdvanceItem(new QListViewItemNumber(average), 0, i18n("DeleteUnusedShots"), deleteUnusedShotsSum / count, func);
 initSlotAdvanceItem(new QListViewItemNumber(average), 0, i18n("Particles"), particlesSum / count, func);
 initSlotAdvanceItem(new QListViewItemNumber(average), 0, i18n("MaximalAdvanceCount"), maximalAdvanceCountSum/ count, func);
 average->setOpen(true);
}

void BosonProfilingDialog::addItemAdvance(ProfileSlotAdvance* slotAdvance)
{
 if (!slotAdvance) {
	boError() << k_funcinfo << endl;
	return;
 }
 QPtrListIterator<ProfileItemAdvance> it(slotAdvance->mItems);
 if (!d->mCreateItemAdvanceSummaryOnly->isChecked()) {
	for (; it.current(); ++it, itemAllItems++) {
		unsigned long int function = it.current()->mFunction.diff();
//		initItemAdvanceItem(new QListViewItemNumber(d->mItemAdvance),
//				it.current(), slotAdvance->mAdvanceCount, 
//				i18n("Function"), it.current()->mFunction.diff(), function);
		initItemAdvanceItem(new QListViewItemNumber(d->mItemAdvance),
				it.current(), slotAdvance->mAdvanceCount,
				i18n("Advance"), it.current()->mAdvance.diff(), function);
		initItemAdvanceItem(new QListViewItemNumber(d->mItemAdvance),
				it.current(), slotAdvance->mAdvanceCount,
				i18n("AdvanceFunction"), it.current()->mAdvanceFunction.diff(), function);
		initItemAdvanceItem(new QListViewItemNumber(d->mItemAdvance),
				it.current(), slotAdvance->mAdvanceCount,
				i18n("Move"), it.current()->mMove.diff(), function);
	}
 }
 it.toFirst();
 for (; it.current(); ++it, itemAllItems++) {
	unsigned long int function = it.current()->mFunction.diff();
	itemFunctionSum += it.current()->mFunction.diff();
	itemAdvanceSum += it.current()->mAdvance.diff();
	itemAdvanceFunctionSum += it.current()->mAdvanceFunction.diff();
	itemMoveSum += it.current()->mMove.diff();
 }
}

void BosonProfilingDialog::addItemAdvanceSummary()
{
 if (!itemAllItems) {
	boDebug() << k_funcinfo << "no items here yet" << endl;
	return;
 }
 initItemAdvanceItemSummary(new QListViewItemNumber(d->mItemAdvance), i18n("Sum (%1 items)").arg(itemAllItems), i18n("Advance"), itemAdvanceSum, itemFunctionSum);
 initItemAdvanceItemSummary(new QListViewItemNumber(d->mItemAdvance), i18n("Sum (%1 items)").arg(itemAllItems), i18n("AdvanceFunction"), itemAdvanceFunctionSum, itemFunctionSum);
 initItemAdvanceItemSummary(new QListViewItemNumber(d->mItemAdvance), i18n("Sum (%1 items)").arg(itemAllItems), i18n("Move"), itemMoveSum, itemFunctionSum);

 initItemAdvanceItemSummary(new QListViewItemNumber(d->mItemAdvance), i18n("Average (%1 items)").arg(itemAllItems), i18n("Advance"), itemAdvanceSum  / itemAllItems, itemFunctionSum / itemAllItems);
 initItemAdvanceItemSummary(new QListViewItemNumber(d->mItemAdvance), i18n("Average (%1 items)").arg(itemAllItems), i18n("AdvanceFunction"), itemAdvanceFunctionSum  / itemAllItems, itemFunctionSum / itemAllItems);
 initItemAdvanceItemSummary(new QListViewItemNumber(d->mItemAdvance), i18n("Average (%1 items)").arg(itemAllItems), i18n("Move"), itemMoveSum  / itemAllItems, itemFunctionSum  / itemAllItems);
}

void BosonProfilingDialog::initItemAdvanceItem(QListViewItemNumber* item, ProfileItemAdvance* a, unsigned int advanceCount, const QString& type, unsigned long int time, unsigned long int function)

{
 QString isUnit;
 QString id;
 QString work;
 if (RTTI::isUnit(a->mRtti)) {
	isUnit = i18n("Yes");
	id = QString::number(a->mId);
	work = QString::number(a->mWork); // TODO: work_id->string
 } else {
	isUnit = i18n("No");
	id = i18n("-");
	work = i18n("-");
 }
 item->setText(0, QString::number(a->mRtti));
 item->setText(1, isUnit);
 item->setText(2, id);
 item->setText(3, work);
 item->setText(4, QString::number(advanceCount));
 item->setText(5, type);
 item->setTime(6, time, function);
}

void BosonProfilingDialog::initItemAdvanceItemSummary(QListViewItemNumber* item, const QString& description, const QString& type, unsigned long int time, unsigned long int function)
{
 item->setText(0, i18n("-"));
 item->setText(1, i18n("-"));
 item->setText(2, i18n("-"));
 item->setText(3, i18n("-"));
 item->setText(4, description);
 item->setText(5, type);
 item->setTime(6, time, function);
}

void BosonProfilingDialog::resetFilesPage()
{
 if (d->mData) {
	d->mCurrentFile->setText(i18n("Using file: %1").arg(d->mCurrentFileName));
 } else {
	d->mCurrentFile->setText(i18n("Using current data"));
 }
}

void BosonProfilingDialog::initRenderItem(QListViewItemNumber* item, const QString& type, long int time, long int function)
{
 if (!function) {
	boError() << k_funcinfo << "function == 0" << endl;
	return;
 }
 item->setText(1, type);
 item->setTime(2, time, function);
}

void BosonProfilingDialog::initSlotAdvanceItem(QListViewItemNumber* item, unsigned int advanceCall, const QString& type, long int time, long int function)
{
 if (!function) {
	boError() << k_funcinfo << "function == 0" << endl;
	return;
 }
 item->setText(1, QString::number(advanceCall));
 item->setText(2, type);
 item->setTime(3, time, function);
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
	long int sum = 0;
	for (; timesIt != (*it).end(); ++timesIt, i++) {
		QListViewItemNumber* item = new QListViewItemNumber(event);
		item->setText(0, QString::number(i));
		item->setTime(1, *timesIt, *timesIt);
		sum += *timesIt;
	}
	event->setOpen(true);
	if (i) {
		QListViewItemNumber* s = new QListViewItemNumber(event);
		s->setText(0, i18n("Sum"));
		s->setTime(1, sum, sum);

		QListViewItemNumber* average = new QListViewItemNumber(event);
		average->setText(0, i18n("Average"));
		average->setTime(1, sum / i, sum / i);
	}
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
 setCaption(file);
 reset();
}

