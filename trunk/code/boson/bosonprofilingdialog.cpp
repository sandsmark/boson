/*
    This file is part of the Boson game
    Copyright (C) 2002-2004 The Boson Team (boson-devel@lists.sourceforge.net)

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
		double n = key(col, ascending).toDouble(&ok);
		double n2 = i->key(col, ascending).toDouble(&ok2);
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
			// this is not a number, i is. i comes first.
			return 1;
		} else {
			return QListViewItem::compare(i, col, ascending);
		}
	}
};

void SummaryWidgetBase::set(struct timeval* start, struct timeval* end, unsigned int count)
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

QString SummaryWidgetBase::startTime() const
{
 QDateTime s;
 s.setTime_t(mStartSec);
 return s.time().toString();
}

QString SummaryWidgetBase::endTime() const
{
 QDateTime e;
 e.setTime_t(mEndSec);
 return e.time().toString();
}

double SummaryWidgetBase::elapsed() const
{
 double diff = (mEndSec - mStartSec) * 1000000 + (mEndUSec - mStartUSec);
 diff /= 1000000;
 return diff;
}

class RenderSummary : public SummaryWidgetBase
{
public:
	RenderSummary(QWidget* parent) : SummaryWidgetBase(parent, "RenderSummary")
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

	virtual void clear()
	{
		SummaryWidgetBase::clear();
		mUpdateInterval = -1;
		mStartLabel->setText("");
		mEndLabel->setText("");
		mSeconds->setText("");
		mCountLabel->setText("");
		mFPSLabel->setText("");
		mUpdateIntervalLabel->setText("");
	}
	void setUpdateInterval(int i)
	{
		mUpdateInterval = i;
	}
	void apply()
	{
		mStartLabel->setText(startTime());
		mEndLabel->setText(endTime());
		mCountLabel->setText(QString::number(count()));
		mSeconds->setText(QString::number(elapsed()));
		mFPSLabel->setText(QString::number(perSecond()));

		if (mUpdateInterval == -1) {
			mUpdateIntervalLabel->setText("(unknown)");
		} else {
			mUpdateIntervalLabel->setText(QString::number(mUpdateInterval));
		}
	}

private:
	int mUpdateInterval;
	QLabel* mStartLabel;
	QLabel* mEndLabel;
	QLabel* mSeconds;
	QLabel* mCountLabel;
	QLabel* mFPSLabel;
	QLabel* mUpdateIntervalLabel;
};

class SlotAdvanceSummary : public SummaryWidgetBase
{
public:
	SlotAdvanceSummary(QWidget* parent) : SummaryWidgetBase(parent, "SlotAdvanceSummary")
	{
		QGridLayout* topLayout = new QGridLayout(this);
		int row = 0;
	int column = 0;
		QLabel* label = new QLabel(i18n("First profiled advance call at:"), this);
		mStartLabel = new QLabel(this);
		topLayout->addWidget(label, row, column);
		column++;
		topLayout->addWidget(mStartLabel, 0, column);
		column++;

		label = new QLabel(i18n("Last profiled advance call at:"), this);
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
		label = new QLabel(i18n("Advance call count in this time: "), this);
		mCountLabel = new QLabel(this);
		topLayout->addWidget(label, row, column);
		column++;
		topLayout->addWidget(mCountLabel, row, column);
		column++;

		label = new QLabel(i18n("Advance calls per second: "), this);
		mPerSecondLabel = new QLabel(this);
		topLayout->addWidget(label, row, column);
		column++;
		topLayout->addWidget(mPerSecondLabel, row, column);
		column++;

		label = new QLabel(i18n("Game speed: "), this);
		mGameSpeedLabel = new QLabel(this);
		topLayout->addWidget(label, row, column);
		column++;
		topLayout->addWidget(mGameSpeedLabel, row, column);
		column++;
	}
	~SlotAdvanceSummary()
	{
	}

	void clear()
	{
		SummaryWidgetBase::clear();
		mGameSpeed = -1;
		mStartLabel->setText("");
		mEndLabel->setText("");
		mSeconds->setText("");
		mCountLabel->setText("");
		mPerSecondLabel->setText("");
		mGameSpeedLabel->setText("");
	}

	void setGameSpeed(int i)
	{
		mGameSpeed = i;
	}

	void apply()
	{
		mStartLabel->setText(startTime());
		mEndLabel->setText(endTime());
		mCountLabel->setText(QString::number(count()));
		mSeconds->setText(QString::number(elapsed()));
		mPerSecondLabel->setText(QString::number(perSecond()));

		if (mGameSpeed == -1) {
			mGameSpeedLabel->setText("(unknown)");
		} else {
			mGameSpeedLabel->setText(QString::number(mGameSpeed));
		}
	}

private:
	int mGameSpeed;
	QLabel* mStartLabel;
	QLabel* mEndLabel;
	QLabel* mSeconds;
	QLabel* mCountLabel;
	QLabel* mPerSecondLabel;
	QLabel* mGameSpeedLabel;
};

template<class T> class DelPointerArray
{
public:
	DelPointerArray(T* p)
	{
		mPointer = p;
	}
	~DelPointerArray()
	{
		delete[] mPointer;
	}
	T* mPointer;
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
		mSlotAdvanceSumAverageOnly = 0;
		mSlotAdvanceSumAverageForCount = 0;
		mSlotAdvanceSummary = 0;
		mItemAdvance = 0;
		mItemAdvanceSumAverageOnly = 0;
//		mItemAdvanceSummary = 0;
		mEvents = 0;
		mCurrentFile = 0;

		mData = 0;
	}

	BosonProfiling* data() const { return mData ? mData : boProfiling; }

	KListView* mUnits;
	KListView* mRender;
	RenderSummary* mRenderSummary;
	KListView* mSlotAdvance;
	QCheckBox* mSlotAdvanceSumAverageOnly;
	QCheckBox* mSlotAdvanceSumAverageForCount;
	SlotAdvanceSummary* mSlotAdvanceSummary;
	KListView* mItemAdvance;
	QCheckBox* mItemAdvanceSumAverageOnly;
//	ItemAdvanceSummary* mItemAdvanceSummary;
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
 QSplitter* splitter = new QSplitter(Vertical, page);
 layout->addWidget(splitter);

 QWidget* slotAdvanceWidget = new QWidget(splitter);
 initSlotAdvanceWidget(slotAdvanceWidget);

 QWidget* itemAdvanceWidget = new QWidget(splitter);
 initItemAdvanceWidget(itemAdvanceWidget);

 QPushButton* reset = new QPushButton(i18n("Reset Values"), page);
 connect(reset, SIGNAL(clicked()), this, SLOT(slotResetSlotAdvancePage()));
 layout->addWidget(reset);
}

void BosonProfilingDialog::initSlotAdvanceWidget(QWidget* widget)
{
 QVBoxLayout* layout = new QVBoxLayout(widget, 0, KDialog::spacingHint());
 QLabel* slotAdvanceLabel = new QLabel(i18n("Slot advance function:"), widget);
 d->mSlotAdvance = new KListView(widget);
 d->mSlotAdvance->setRootIsDecorated(true);
 d->mSlotAdvance->addColumn(i18n("Number"));
 d->mSlotAdvance->addColumn(i18n("Advance Count"));
 d->mSlotAdvance->addColumn(i18n("Type"));
 d->mSlotAdvance->addColumn(i18n("Time"));
 d->mSlotAdvance->addColumn(i18n("Time (ms)"));
 d->mSlotAdvance->addColumn(i18n("Time (s)"));
 d->mSlotAdvance->addColumn(i18n("%"));
 QHBox* slotAdvanceControlBox = new QHBox(widget);
 d->mSlotAdvanceSumAverageOnly = new QCheckBox(i18n("Average and sum only"), slotAdvanceControlBox);
 d->mSlotAdvanceSumAverageOnly->setChecked(true);
 d->mSlotAdvanceSumAverageForCount = new QCheckBox(i18n("Average and sum for AdvanceCount"), slotAdvanceControlBox);
 d->mSlotAdvanceSumAverageForCount->setChecked(true);

 d->mSlotAdvanceSummary = new SlotAdvanceSummary(widget);

 layout->addWidget(slotAdvanceLabel);
 layout->addWidget(d->mSlotAdvance);
 layout->addWidget(slotAdvanceControlBox);
 layout->addWidget(d->mSlotAdvanceSummary);
}

void BosonProfilingDialog::initItemAdvanceWidget(QWidget* widget)
{
 QVBoxLayout* layout = new QVBoxLayout(widget, 0, KDialog::spacingHint());
 QLabel* itemAdvanceLabel = new QLabel(i18n("Values per item:"), widget);
 d->mItemAdvance = new KListView(widget);
 d->mItemAdvance->setRootIsDecorated(true);
 d->mItemAdvance->addColumn(i18n("Advance Count"));
 d->mItemAdvance->addColumn(i18n("IsUnit"));
 d->mItemAdvance->addColumn(i18n("ID"));
 d->mItemAdvance->addColumn(i18n("Work"));
 d->mItemAdvance->addColumn(i18n("Rtti"));
 d->mItemAdvance->addColumn(i18n("Type"));
 d->mItemAdvance->addColumn(i18n("Time"));
 d->mItemAdvance->addColumn(i18n("Time (ms)"));
 d->mItemAdvance->addColumn(i18n("Time (s)"));
 d->mItemAdvance->addColumn(i18n("%"));
 layout->addWidget(itemAdvanceLabel);
 layout->addWidget(d->mItemAdvance);

 QHBox* controlBox = new QHBox(widget);
 d->mItemAdvanceSumAverageOnly = new QCheckBox(i18n("Item Advance Sum and Average Only"), controlBox);
 d->mItemAdvanceSumAverageOnly->setChecked(true);
 layout->addWidget(controlBox);
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
 bool found = true;
 switch ((BosonProfiling::ProfilingEvent)profilingEvent) {
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
	case BosonProfiling::BosonStartingStart:
		name = i18n("BosonStartingStart");
		break;
	case BosonProfiling::SaveGameToXML:
		name = i18n("SaveGameToXML");
		break;
	case BosonProfiling::SaveKGameToXML:
		name = i18n("SaveKGameToXML");
		break;
	case BosonProfiling::SavePlayersToXML:
		name = i18n("SavePlayersToXML");
		break;
	case BosonProfiling::SavePlayerToXML:
		name = i18n("SavePlayerToXML");
		break;
	case BosonProfiling::SavePlayFieldToXML:
		name = i18n("SavePlayFieldToXML");
		break;
	case BosonProfiling::SaveGameToXMLWriteFile:
		name = i18n("SaveGameToXMLWriteFile");
		break;
	case BosonProfiling::SaveCanvasToXML:
		name = i18n("SaveCanvasToXML");
		break;
	case BosonProfiling::SaveExternalToXML:
		name = i18n("SaveExternalToXML");
		break;
	case BosonProfiling::PreLoadPlayFields:
		name = i18n("PreLoadPlayFields");
		break;
	case BosonProfiling::LoadPlayField:
		name = i18n("LoadPlayField");
		break;
	case BosonProfiling::FindPath:
		name = i18n("FindPath");
		break;
	case BosonProfiling::WriteGameLog:
		name = i18n("WriteGameLog");
		break;
	case BosonProfiling::SaveGameLogs:
		name = i18n("SaveGameLogs");
		break;
	case BosonProfiling::MakeGameLog:
		name = i18n("MakeGameLog");
		break;
	case BosonProfiling::GenerateLOD:
		name = i18n("GenerateLOD");
		break;
	case BosonProfiling::BuildLOD:
		name = i18n("BuildLOD");
		break;
	default:
		found = false;
		name = i18n("Unknown %1").arg(profilingEvent);
		break;
 }
 if (!found && profilingEvent >= BosonProfiling::LastFixedEventId) {
	QString n = boProfiling->eventName(profilingEvent);
	if (!n.isEmpty()) {
		name = n;
	}
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
		// sum the times up first
		time += *timesIt;
	}
	timesIt = (*it).begin();
	for (; timesIt != (*it).end(); ++timesIt, i++) {
		QListViewItemNumber* item = new QListViewItemNumber(unit);
		item->setText(0, QString::number(i));
		item->setTime(1, *timesIt, time);
	}
	unit->setTime(1, time, time); // all summed up
	unit->setOpen(true);
 }
}

void BosonProfilingDialog::resetRenderPage()
{
 d->mRender->clear();
 d->mRenderSummary->clear();
 BosonProfilingPrivate* pd = d->data()->d;
 QPtrListIterator<RenderGLTimes> it(pd->mRenderTimes);
 QValueList<QString> renderNames = RenderGLTimes::names();
 unsigned long long* renderSums = new unsigned long long[renderNames.count()];
 DelPointerArray<unsigned long long> delit(renderSums);
 for (unsigned int i = 0; i < renderNames.count(); i++) {
	renderSums[i] = 0;
 }

 int unitCount = 0;
 int i = 0;
 for (; it.current(); ++it, i++) {
	QValueList<unsigned long int> values = it.current()->values();

	unsigned long int func = values[0];
	QListViewItemNumber* item = new QListViewItemNumber(d->mRender);
	item->setText(0, QString::number(i));
	initRenderItem(item, renderNames[0], values[0], func);

	renderSums[0] += values[0];
	for (unsigned int i = 1; i < renderNames.count(); i++) {
		QString name = renderNames[i];
		if (i == 1) {
			// the second entry is the unit value.
			// here the unit count can be very useful
			name = i18n("Units (%1)").arg(it.current()->mUnitCount);
		}
		renderSums[i] += values[i];

		initRenderItem(new QListViewItemNumber(item),
				name, values[i], func);

	}
	// for average and sum values we need to know how many units
	// have been rendered
	unitCount += it.current()->mUnitCount;
 }

 unsigned int count = pd->mRenderTimes.count();
 if (!count) {
	d->mRenderSummary->set(0, 0, 0);
	d->mRenderSummary->setUpdateInterval(-1);
	d->mRenderSummary->apply();
	return;
 }
 unsigned long int func = renderSums[0] / count;
 QListViewItemNumber* average = new QListViewItemNumber(d->mRender);
 average->setText(0, i18n("Average - use with care"));
 initRenderItem(average, renderNames[0], renderSums[0] / count, func);

 QListViewItemNumber* sum = new QListViewItemNumber(d->mRender);
 sum->setText(0, i18n("Sum"));
 initRenderItem(sum, renderNames[0], renderSums[0], renderSums[0]);

 for (unsigned int i = 1; i < renderNames.count(); i++) {
	QString name = renderNames[i];
	QString nameAverage = renderNames[i];
	if (i == 1) {
		name = i18n("Units (%1)").arg(unitCount);
		nameAverage = i18n("Units (%1)").arg(unitCount / count);
	}
	initRenderItem(new QListViewItemNumber(average),
			nameAverage, renderSums[i] / count, func);
	initRenderItem(new QListViewItemNumber(sum),
			name, renderSums[i], renderSums[0]);
 }
 average->setOpen(true);
 sum->setOpen(true);


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


// ensure that an allocated pointer is really deleted when the function returns
class ProfileSum
{
public:
	ProfileSum()
	{
	}
	~ProfileSum()
	{
	}
	void reset()
	{
		mSum = 0;
		mCount = 0;
	}
	void add(unsigned long int value)
	{
		mSum += value;
		mCount++;
	}
	unsigned long long sum() const { return mSum; }
	unsigned long average() const { return mSum / mCount; }
	unsigned int count() const { return mCount; }

private:
	unsigned int mCount;
	unsigned long long mSum;
};

void BosonProfilingDialog::slotResetSlotAdvancePage()
{
 if (!d->mItemAdvanceSumAverageOnly->isChecked()) {
	int r = KMessageBox::questionYesNo(this, i18n("You selected to display advance information for ALL items - this will take a very long time.\nDo you really want to do this?"));
	if (r != KMessageBox::Yes) {
		return;
	}
 }

 resetSlotAdvanceWidget();
 resetItemAdvanceWidget();
}

void BosonProfilingDialog::resetSlotAdvanceWidget()
{
 d->mSlotAdvance->clear();
 d->mSlotAdvanceSummary->clear();
 BosonProfilingPrivate* pd = d->data()->d;
 if (pd->mSlotAdvanceTimes.isEmpty()) {
	d->mSlotAdvanceSummary->set(0, 0, 0);
	d->mSlotAdvanceSummary->setGameSpeed(-1);
	d->mSlotAdvanceSummary->apply();
	return;
 }

 QValueList<QString> slotAdvanceValueNames = ProfileSlotAdvance::names();
 ProfileSum* slotAdvanceSums = new ProfileSum[slotAdvanceValueNames.count()];
 DelPointerArray<ProfileSum> delit(slotAdvanceSums);
 for (unsigned int i = 0; i < slotAdvanceValueNames.count(); i++) {
	slotAdvanceSums[i].reset();
 }

 QPtrListIterator<ProfileSlotAdvance> it(pd->mSlotAdvanceTimes);
 int slotAdvanceCount = 0;
 for (; it.current(); ++it, slotAdvanceCount++) {
	QValueList<unsigned long int> values = it.current()->values();
	for (unsigned int i = 0; i < slotAdvanceValueNames.count(); i++) {
		slotAdvanceSums[i].add(values[i]);
	}
	if (!d->mSlotAdvanceSumAverageOnly->isChecked()) {
		// TODO: we should display sums, too
		unsigned long int func = values[0];
		unsigned int call = it.current()->mAdvanceCallsCount;
		QListViewItemNumber* item = new QListViewItemNumber(d->mSlotAdvance);
		initSlotAdvanceItem(item, call, slotAdvanceValueNames[0], values[0], func);
		item->setText(0, QString::number(slotAdvanceCount));
		for (unsigned int i = 1; i < values.count(); i++) { // skip function entry
			initSlotAdvanceItem(new QListViewItemNumber(item), call, slotAdvanceValueNames[i], values[i], func);
		}
		item->setOpen(true);
	}

 }

 QListViewItemNumber* average = new QListViewItemNumber(d->mSlotAdvance);
 QListViewItemNumber* sum = new QListViewItemNumber(d->mSlotAdvance);
 average->setText(0, i18n("Average - use with care"));
 sum->setText(0, i18n("Sum"));
 initSlotAdvanceItem(average, -1, slotAdvanceValueNames[0], slotAdvanceSums[0].average(), slotAdvanceSums[0].average());
 initSlotAdvanceItem(sum, -1, slotAdvanceValueNames[0], slotAdvanceSums[0].sum(), slotAdvanceSums[0].sum());
 for (unsigned int i = 1; i < slotAdvanceValueNames.count(); i++) { // we skip the function entry!
	initSlotAdvanceItem(new QListViewItemNumber(average), -1, slotAdvanceValueNames[i], slotAdvanceSums[i].average(), slotAdvanceSums[0].average());
	initSlotAdvanceItem(new QListViewItemNumber(sum), -1, slotAdvanceValueNames[i], slotAdvanceSums[i].sum(), slotAdvanceSums[0].sum());
 }
 average->setOpen(true);
 sum->setOpen(true);

 d->mSlotAdvanceSummary->set(&pd->mSlotAdvanceTimes.first()->mFunction.mData[0],
		&pd->mSlotAdvanceTimes.last()->mFunction.mData[1],
		pd->mSlotAdvanceTimes.count());
 d->mSlotAdvanceSummary->setGameSpeed(pd->mGameSpeed);
 d->mSlotAdvanceSummary->apply();
}

void BosonProfilingDialog::resetItemAdvanceWidget()
{
 d->mItemAdvance->clear();
 BosonProfilingPrivate* pd = d->data()->d;
 if (pd->mSlotAdvanceTimes.isEmpty()) {
	return;
 }
 QValueList<QString> itemAdvanceNames = ProfileItemAdvance::names();


 QMemArray<unsigned long int> itemAdvanceSums(itemAdvanceNames.count());
 int itemAdvanceCount = 0;
 for (unsigned int i = 0; i < itemAdvanceNames.count(); i++) {
	itemAdvanceSums[i] = 0;
 }
 QMap< int, QMemArray<unsigned long int> > advanceSumsByWork;
 QMap< int, int > advanceSumsByWorkCount;

 QPtrListIterator<ProfileSlotAdvance> it(pd->mSlotAdvanceTimes);
 for (; it.current(); ++it) {
	// values per item for the units, missiles, ...
	addItemAdvance(it.current());
	QPtrListIterator<ProfileItemAdvance> itemsIt(it.current()->mItems);
	// warning: this loop is very speed sensitive! there are a lot of items
	// here...
	for (; itemsIt.current(); ++itemsIt) {
		QValueList<unsigned long int> values = itemsIt.current()->values();
		if (!advanceSumsByWork.contains((*itemsIt)->mWork)) {
			QMemArray<unsigned long int> *a = &advanceSumsByWork[(*itemsIt)->mWork];
			a->resize(itemAdvanceNames.count());
			for (unsigned int i = 0; i < itemAdvanceNames.count(); i++) {
				(*a)[i] = 0;
			}
			advanceSumsByWorkCount[(*itemsIt)->mWork] = 0;
		}
		QMemArray<unsigned long int> *a = &advanceSumsByWork[(*itemsIt)->mWork];
		for (unsigned int i = 0; i < values.count(); i++) {
			itemAdvanceSums[i] += values[i];
			(*a)[i] += values[i];
		}
		itemAdvanceCount++;
		advanceSumsByWorkCount[(*itemsIt)->mWork]++;
	}
 }

 if (!pd->mSlotAdvanceTimes.count()) {
	// hmm we already checked above?!
	boError() << k_funcinfo << "internal error - count == 0" << endl;
	return;
 }
 if (!itemAdvanceCount) {
	return;
 }
 for (unsigned int i = 0; i < itemAdvanceSums.count(); i++) {
	initItemAdvanceItemSummary(new QListViewItemNumber(d->mItemAdvance), i18n("Sum (%1 items)").arg(itemAdvanceCount), itemAdvanceNames[i], itemAdvanceSums[i], itemAdvanceSums[0]);
	initItemAdvanceItemSummary(new QListViewItemNumber(d->mItemAdvance), i18n("Average (%1 items)").arg(itemAdvanceCount), itemAdvanceNames[i], itemAdvanceSums[i] / itemAdvanceCount, itemAdvanceSums[0] / itemAdvanceCount);
 }


 QListViewItemNumber* averageByWork = new QListViewItemNumber(d->mItemAdvance);
 QListViewItemNumber* sumByWork = new QListViewItemNumber(d->mItemAdvance);
 initItemAdvanceItemSummary(averageByWork, i18n("Average by work"), "", 0 , 1);
 initItemAdvanceItemSummary(sumByWork, i18n("Sum by work"), "", 0 , 1);
 QMap< int, QMemArray<unsigned long int> >::Iterator workIt = advanceSumsByWork.begin();
 for (; workIt != advanceSumsByWork.end(); ++workIt) {
	QListViewItemNumber* average = new QListViewItemNumber(averageByWork);
	QListViewItemNumber* sum = new QListViewItemNumber(sumByWork);
	QString labelAverage;
	QString labelSum;
	int count = advanceSumsByWorkCount[workIt.key()];
	if (workIt.key() == -1) {
		labelAverage = i18n("Average Non-Units (%1 items)").arg(count);
		labelSum = i18n("Sum Non-Units (%1 items)").arg(count);
	} else {
		labelAverage = i18n("Average (%1 units)").arg(count);
		labelSum = i18n("Sum (%1 units)").arg(count);
	}
	initItemAdvanceItemSummary(average, labelAverage, itemAdvanceNames[0], (*workIt)[0] / count , (*workIt)[0] / count, workIt.key());
	initItemAdvanceItemSummary(sum, labelSum, itemAdvanceNames[0], (*workIt)[0], (*workIt)[0], workIt.key());
	for (unsigned int i = 1; i < itemAdvanceSums.count(); i++) {
		initItemAdvanceItemSummary(new QListViewItemNumber(average), labelAverage, itemAdvanceNames[i], (*workIt)[i] / count , (*workIt)[0] / count, workIt.key());
		initItemAdvanceItemSummary(new QListViewItemNumber(sum), labelSum, itemAdvanceNames[i], (*workIt)[i], (*workIt)[0], workIt.key());
	}
 }

}

void BosonProfilingDialog::addItemAdvance(ProfileSlotAdvance* slotAdvance)
{
 if (!slotAdvance) {
	boError() << k_funcinfo << endl;
	return;
 }
 QPtrListIterator<ProfileItemAdvance> it(slotAdvance->mItems);
 if (d->mItemAdvanceSumAverageOnly->isChecked()) {
	return;
 }
 for (; it.current(); ++it) {
	unsigned long int function = it.current()->mFunction.diff();
//	initItemAdvanceItem(new QListViewItemNumber(d->mItemAdvance),
//			it.current(), slotAdvance->mAdvanceCount, 
//			i18n("Function"), it.current()->mFunction.diff(), function);
	initItemAdvanceItem(new QListViewItemNumber(d->mItemAdvance),
			it.current(), slotAdvance->mAdvanceCallsCount,
			i18n("Advance"), it.current()->mAdvance.diff(), function);
	initItemAdvanceItem(new QListViewItemNumber(d->mItemAdvance),
			it.current(), slotAdvance->mAdvanceCallsCount,
			i18n("AdvanceFunction"), it.current()->mAdvanceFunction.diff(), function);
	initItemAdvanceItem(new QListViewItemNumber(d->mItemAdvance),
			it.current(), slotAdvance->mAdvanceCallsCount,
			i18n("Move"), it.current()->mMove.diff(), function);
 }
}

void BosonProfilingDialog::initItemAdvanceItem(QListViewItemNumber* item, ProfileItemAdvance* a, unsigned int advanceCallsCount, const QString& type, unsigned long int time, unsigned long int function)

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
 item->setText(0, QString::number(advanceCallsCount));
 item->setText(1, isUnit);
 item->setText(2, id);
 item->setText(4, QString::number(a->mRtti));
 item->setText(3, work);
 item->setText(5, type);
 item->setTime(6, time, function);
}

void BosonProfilingDialog::initItemAdvanceItemSummary(QListViewItemNumber* item, const QString& description, const QString& type, unsigned long int time, unsigned long int function, int work)
{
 item->setText(0, description);
 item->setText(1, i18n("-"));
 item->setText(2, i18n("-"));
 if (work < 0) {
	item->setText(3, i18n("-"));
 } else {
	item->setText(3, QString::number(work));
 }
 item->setText(4, i18n("-"));
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
	event->setText(0, i18n("%1 (%2)").arg(profilingName(it.key())).arg(it.key()));
	BosonProfilingPrivate::TimesList::Iterator timesIt = (*it).begin();
	int i = 0;
	long int sum = 0;
	for (; timesIt != (*it).end(); ++timesIt, i++) {
		QListViewItemNumber* item = new QListViewItemNumber(event);
		item->setText(0, QString::number(i));
		item->setTime(1, *timesIt, *timesIt);
		sum += *timesIt;
	}
//	event->setOpen(true);
	if (i) {
		QListViewItemNumber* s = new QListViewItemNumber(event);
		s->setText(0, i18n("Sum"));
		s->setTime(1, sum, sum);

		event->setTime(1, sum, sum);

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
 if (file.isNull()) {
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
 if (file.isNull()) {
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

