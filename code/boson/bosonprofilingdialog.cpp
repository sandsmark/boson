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
#include <qintdict.h>
#include <qmemarray.h>

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

		label = new QLabel(i18n("Update interval: "), this);
		mUpdateIntervalLabel = new QLabel(this);
		topLayout->addWidget(label, row, column);
		column++;
		topLayout->addWidget(mUpdateIntervalLabel, row, column);
		column++;
	}
	~RenderSummary()
	{
	}

	void clear()
	{
		mStartSec = 0;
		mStartUSec = 0;
		mEndSec = 0;
		mEndUSec = 0;
		mCount = 0;
		mStartLabel->setText("");
		mEndLabel->setText("");
		mSeconds->setText("");
		mCountLabel->setText("");
		mFPSLabel->setText("");
		mUpdateIntervalLabel->setText("");
	}

	void set(struct timeval* start, struct timeval* end, int count)
	{
		if (!start || !end || count == 0) {
			clear();
			return;
		}
		mStartSec = start->tv_sec;
		mStartUSec = start->tv_usec;
		mEndSec = end->tv_sec;
		mEndUSec = end->tv_usec;
		mCount = count;
	}
	void setUpdateInterval(int i)
	{
		mUpdateInterval = i;
	}

	void apply()
	{
		QDateTime s, e;
		s.setTime_t(mStartSec);
		e.setTime_t(mEndSec);
		mStartLabel->setText(s.time().toString());
		mEndLabel->setText(e.time().toString());
		mCountLabel->setText(QString::number(mCount));

		double diff = (mEndSec - mStartSec) * 1000000 + (mEndUSec - mStartUSec);
		diff /= 1000000;
		mSeconds->setText(QString::number(diff));
		double fps = ((double)mCount) / diff;
		mFPSLabel->setText(QString::number(fps));

		if (mUpdateInterval == -1) {
			mUpdateIntervalLabel->setText("(unknown)");
		} else {
			mUpdateIntervalLabel->setText(QString::number(mUpdateInterval));
		}
	}

private:
	long mStartSec;
	long mStartUSec;
	long mEndSec;
	long mEndUSec;
	int mCount;
	int mUpdateInterval;
	QLabel* mStartLabel;
	QLabel* mEndLabel;
	QLabel* mSeconds;
	QLabel* mCountLabel;
	QLabel* mFPSLabel;
	QLabel* mUpdateIntervalLabel;
};

class BosonProfilingDialogPrivate
{
public:
	BosonProfilingDialogPrivate()
	{
		mUnits = 0;
		mRender = 0;
		mRenderSummary = 0;
		mSlotAdvance = 0;
		mSlotAdvanceSummaryOnly = 0;
		mSlotAdvanceSummaryForCount = 0;
		mItemAdvance = 0;
		mItemAdvanceSummaryOnly = 0;
		mEvents = 0;
		mCurrentFile = 0;

		mData = 0;
	}

	BosonProfiling* data() const { return mData ? mData : boProfiling; }

	KListView* mUnits;
	KListView* mRender;
	RenderSummary* mRenderSummary;
	KListView* mSlotAdvance;
	QCheckBox* mSlotAdvanceSummaryOnly;
	QCheckBox* mSlotAdvanceSummaryForCount;
	KListView* mItemAdvance;
	QCheckBox* mItemAdvanceSummaryOnly;
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
 QWidget* page = addPage(i18n("&Slot Advance"));
 QVBoxLayout* layout = new QVBoxLayout(page, 0, KDialog::spacingHint());

 QLabel* slotAdvanceLabel = new QLabel(i18n("Slot advance function:"), page);
 d->mSlotAdvance = new KListView(page);
 d->mSlotAdvance->setRootIsDecorated(true);
 d->mSlotAdvance->addColumn(i18n("Number"));
 d->mSlotAdvance->addColumn(i18n("Advance Count"));
 d->mSlotAdvance->addColumn(i18n("Type"));
 d->mSlotAdvance->addColumn(i18n("Time"));
 d->mSlotAdvance->addColumn(i18n("Time (ms)"));
 d->mSlotAdvance->addColumn(i18n("Time (s)"));
 d->mSlotAdvance->addColumn(i18n("%"));
 QHBox* slotAdvanceControlBox = new QHBox(page);
 d->mSlotAdvanceSummaryOnly = new QCheckBox(i18n("Summary only"), slotAdvanceControlBox);
 d->mSlotAdvanceSummaryOnly->setChecked(true);
 d->mSlotAdvanceSummaryForCount = new QCheckBox(i18n("Summary for AdvanceCount"), slotAdvanceControlBox);
 d->mSlotAdvanceSummaryForCount->setChecked(true);

 layout->addWidget(slotAdvanceLabel);
 layout->addWidget(d->mSlotAdvance);
 layout->addWidget(slotAdvanceControlBox);
 layout->addSpacing(10);

 QLabel* itemAdvanceLabel = new QLabel(i18n("Values per item:"), page);
 d->mItemAdvance = new KListView(page);
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
 layout->addWidget(itemAdvanceLabel);
 layout->addWidget(d->mItemAdvance);

 QHBox* controlBox = new QHBox(page);
 d->mItemAdvanceSummaryOnly = new QCheckBox(i18n("Item Advance Summary Only"), controlBox);
 d->mItemAdvanceSummaryOnly->setChecked(true);
 layout->addWidget(controlBox);

 QPushButton* reset = new QPushButton(i18n("Reset Values"), page);
 connect(reset, SIGNAL(clicked()), this, SLOT(slotResetSlotAdvancePage()));
 layout->addWidget(reset);
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
	case BosonProfiling::AddUnitsXML:
		name = i18n("AddUnitsXML");
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
 BosonProfilingPrivate* pd = d->data()->d;
 QMap<unsigned long int, BosonProfilingPrivate::TimesList>::Iterator it = pd->mUnitTimes.begin();
 for (; it != pd->mUnitTimes.end(); ++it) {
	QListViewItemNumber* unit = new QListViewItemNumber(d->mUnits);
	unit->setText(0, QString::number(it.key()));
	BosonProfilingPrivate::TimesList::Iterator timesIt = (*it).begin(); // wow what a line ;)
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
 BosonProfilingPrivate* pd = d->data()->d;
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
	d->mRenderSummary->setUpdateInterval(-1);
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
 if (pd->mVersion <= 0x06) {
	d->mRenderSummary->setUpdateInterval(-1);
 } else {
	d->mRenderSummary->setUpdateInterval(pd->mGLUpdateInterval);
 }
 d->mRenderSummary->apply();
}

void BosonProfilingDialog::slotResetSlotAdvancePage()
{
 if (!d->mItemAdvanceSummaryOnly->isChecked()) {
	int r = KMessageBox::questionYesNo(this, i18n("You selected to display advance information for ALL items - this will take a very long time.\nDo you really want to do this?"));
	if (r != KMessageBox::Yes) {
		return;
	}
 }
 d->mSlotAdvance->clear();
 d->mItemAdvance->clear();
 BosonProfilingPrivate* pd = d->data()->d;
 if (pd->mSlotAdvanceTimes.isEmpty()) {
	return;
 }

 QValueList<QString> slotAdvanceValueNames = ProfileSlotAdvance::names();
 QMemArray<unsigned long int> slotAdvanceSums(slotAdvanceValueNames.count());
 for (unsigned int i = 0; i < slotAdvanceSums.count(); i++) {
	slotAdvanceSums[i] = 0;
 }

 QPtrListIterator<ProfileSlotAdvance> it(pd->mSlotAdvanceTimes);
 int slotAdvanceCount = 0;
 for (; it.current(); ++it, slotAdvanceCount++) {
	QValueList<unsigned long int> values = it.current()->values();
	for (unsigned int i = 0; i < values.count(); i++) {
		slotAdvanceSums[i] += values[i];
	}
	if (!d->mSlotAdvanceSummaryOnly->isChecked()) {
		unsigned long int func = values[0];
		unsigned int call = it.current()->mAdvanceCount;
		QListViewItemNumber* item = new QListViewItemNumber(d->mSlotAdvance);
		initSlotAdvanceItem(item, call, slotAdvanceValueNames[0], values[0], func);
		item->setText(0, QString::number(slotAdvanceCount));
		for (unsigned int i = 1; i < values.count(); i++) { // skip function entry
			initSlotAdvanceItem(new QListViewItemNumber(item), call, slotAdvanceValueNames[i], values[i], func);
		}
		item->setOpen(true);
	}

 }

 if (d->mSlotAdvanceSummaryForCount->isChecked()) {
	it.toFirst();
	QIntDict< QPtrList<ProfileSlotAdvance> > advanceCounts;
	advanceCounts.setAutoDelete(true);
	for (; it.current(); ++it) {
		QPtrList<ProfileSlotAdvance>* prof = advanceCounts.find(it.current()->mAdvanceCount);
		if (!prof) {
			prof = new QPtrList<ProfileSlotAdvance>();
			advanceCounts.insert(it.current()->mAdvanceCount, prof);
		}
		prof->append(it.current());
	}
 }

 QMemArray<unsigned long int> itemAdvanceSums(4); // FIXME hardcoded
 int itemAdvanceCount = 0;
 for (unsigned int i = 0; i < itemAdvanceSums.count(); i++) {
	itemAdvanceSums[i] = 0;
 }

 it.toFirst();
 for (; it.current(); ++it) {
	// values per item for the units, missiles, ...
	addItemAdvance(it.current());
	QPtrListIterator<ProfileItemAdvance> itemsIt(it.current()->mItems);
	// warning: this loop is very speed sensitive! there are a lot of items
	// here...
	for (; itemsIt.current(); ++itemsIt) {
		QValueList<unsigned long int> values = itemsIt.current()->values();
		for (int i = 0; i < values.count(); i++) {
			itemAdvanceSums[i] += values[i];
		}
		itemAdvanceCount++;
	}
 }



 unsigned int count = pd->mSlotAdvanceTimes.count();
 if (!count) {
	// hmm we already checked above?!
	boError() << k_funcinfo << "internal error - count == 0" << endl;
	return;
 }

 // slot advance summary
 unsigned long int func = slotAdvanceSums[0] / count;
 QListViewItemNumber* average = new QListViewItemNumber(d->mSlotAdvance);
 average->setText(0, i18n("Average - use with care"));
 initSlotAdvanceItem(average, -1, slotAdvanceValueNames[0], slotAdvanceSums[0] / count, func);
 for (unsigned int i = 1; i < slotAdvanceSums.count(); i++) { // we skip the function entry!
	initSlotAdvanceItem(new QListViewItemNumber(average), -1, slotAdvanceValueNames[i], slotAdvanceSums[i] / count, func);
 }
 average->setOpen(true);
 // slot advance summary (end)

 // item advance summary
 if (!itemAdvanceCount) {
	return;
 }
 QValueList<QString> itemAdvanceNames = ProfileItemAdvance::names();
 for (unsigned int i = 1; i < itemAdvanceSums.count(); i++) {
	initItemAdvanceItemSummary(new QListViewItemNumber(d->mItemAdvance), i18n("Sum (%1 items)").arg(itemAdvanceCount), itemAdvanceNames[i], itemAdvanceSums[i], itemAdvanceSums[0]);
	initItemAdvanceItemSummary(new QListViewItemNumber(d->mItemAdvance), i18n("Average (%1 items)").arg(itemAdvanceCount), itemAdvanceNames[i], itemAdvanceSums[i] / itemAdvanceCount, itemAdvanceSums[0] / itemAdvanceCount);
 }
 // item advance summary (end)
}

void BosonProfilingDialog::addItemAdvance(ProfileSlotAdvance* slotAdvance)
{
 if (!slotAdvance) {
	boError() << k_funcinfo << endl;
	return;
 }
 QPtrListIterator<ProfileItemAdvance> it(slotAdvance->mItems);
 if (d->mItemAdvanceSummaryOnly->isChecked()) {
	return;
 }
 for (; it.current(); ++it) {
	unsigned long int function = it.current()->mFunction.diff();
//	initItemAdvanceItem(new QListViewItemNumber(d->mItemAdvance),
//			it.current(), slotAdvance->mAdvanceCount, 
//			i18n("Function"), it.current()->mFunction.diff(), function);
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

void BosonProfilingDialog::initSlotAdvanceItem(QListViewItemNumber* item, int advanceCall, const QString& type, long int time, long int function)
{
 if (!function) {
	boError() << k_funcinfo << "function == 0" << endl;
	return;
 }
 if (advanceCall == -1) {
	item->setText(1, QString::fromLatin1("-"));
 } else {
	item->setText(1, QString::number(advanceCall));
 }
 item->setText(2, type);
 item->setTime(3, time, function);
}

void BosonProfilingDialog::resetEventsPage()
{
 d->mEvents->clear();
 BosonProfilingPrivate* pd = d->data()->d;
 QMap<int, BosonProfilingPrivate::TimesList>::Iterator it = pd->mTimes.begin(); // now *that* is an ugly line! ggg
 for (; it != pd->mTimes.end(); ++it) {
	QListViewItemNumber* event = new QListViewItemNumber(d->mEvents);
	event->setText(0, profilingName(it.key()));
	BosonProfilingPrivate::TimesList::Iterator timesIt = (*it).begin();
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

