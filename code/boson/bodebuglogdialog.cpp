/*
    This file is part of the Boson game
    Copyright (C) 2004 Andreas Beckermann (b_mann@gmx.de)

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

#include "bodebuglogdialog.h"
#include "bodebuglogdialog.moc"

#include "../bomemory/bodummymemory.h"
#include "bodebug.h"
#include "bodebuglog.h"
#include "qlistviewitemnumber.h"
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <Q3PtrList>

#include <klocale.h>

#include <q3listview.h>
#include <qlayout.h>
#include <q3vbox.h>
#include <q3textedit.h>
#include <qlabel.h>

class BoDebugLogWidgetPrivate
{
public:
	BoDebugLogWidgetPrivate()
	{
	}
	Q3ListView* mMessages;
	Q3TextEdit* mBacktrace;
	QMap<Q3ListViewItem*, QString> mItem2Backtrace;
};

BoDebugLogWidget::BoDebugLogWidget(QWidget* parent, const char* name) : QWidget(parent, name)
{
 d = new BoDebugLogWidgetPrivate;
 Q3VBoxLayout* topLayout = new Q3VBoxLayout(this);
 QSplitter* splitter = new QSplitter(Vertical, this);
 topLayout->addWidget(splitter);
 d->mMessages = new Q3ListView(splitter);
 connect(d->mMessages, SIGNAL(selectionChanged(Q3ListViewItem*)), this, SLOT(slotMessageSelected(Q3ListViewItem*)));

 d->mMessages->setAllColumnsShowFocus(true);
 d->mMessages->addColumn(i18n("Nr"));
 d->mMessages->addColumn(i18n("Area"));
 d->mMessages->addColumn(i18n("AreaNr"));
 d->mMessages->addColumn(i18n("Level"));
 d->mMessages->addColumn(i18n("Text"));

 QWidget* backtraceBox = new QWidget(splitter);
 QLabel* backtraceLabel = new QLabel(i18n("Backtrace"), backtraceBox);
 d->mBacktrace = new Q3TextEdit(backtraceBox);
 d->mBacktrace->setReadOnly(true);
 Q3VBoxLayout* backtraceLayout = new Q3VBoxLayout(backtraceBox);
 backtraceLayout->addWidget(backtraceLabel);
 backtraceLayout->addWidget(d->mBacktrace);
}

BoDebugLogWidget::~BoDebugLogWidget()
{
 d->mItem2Backtrace.clear();
 delete d;
}

void BoDebugLogWidget::slotMessageSelected(Q3ListViewItem* item)
{
 QString backtrace;
 if (item) {
	backtrace = d->mItem2Backtrace[item];
 }
 if (backtrace.isEmpty()) {
	backtrace = i18n("No backtrace found");
 }
 d->mBacktrace->setText(backtrace);
}

void BoDebugLogWidget::setMessages(const Q3PtrList<BoDebugMessage>& m)
{
 d->mMessages->clear();
 d->mItem2Backtrace.clear();
 d->mBacktrace->setText(QString::null);
 Q3PtrListIterator<BoDebugMessage> it(m);
 int i = 0;
 while (it.current()) {
	QListViewItemNumber* item = new QListViewItemNumber(d->mMessages);
	QString level;
	switch (it.current()->level()) {
		case BoDebug::KDEBUG_INFO:
			level = i18n("Info");
			break;
		case BoDebug::KDEBUG_WARN:
			level = i18n("Warning");
			break;
		case BoDebug::KDEBUG_ERROR:
			level = i18n("Error");
			break;
		case BoDebug::KDEBUG_FATAL:
			level = i18n("Fatal");
			break;
		default:
			level = i18n("Unknown");
			break;
	}
	QString text = it.current()->message();
	if (!text.isEmpty() && text[text.length() - 1] == '\n') {
		text = text.left(text.length() - 1);
	}
	item->setText(0, QString::number(i));
	item->setText(1, it.current()->areaName());
	item->setText(2, QString::number(it.current()->area()));
	item->setText(3, level);
	item->setText(4, text);
	d->mItem2Backtrace.insert(item, it.current()->backtrace());
	if (text.contains('\n')) {
		item->setMultiLinesEnabled(true);
	}
	++it;
	i++;
 }
}

class BoDebugLogDialogPrivate
{
public:
	BoDebugLogDialogPrivate()
	{
	}
	QMap<int, BoDebugLogWidget*> mLevel2Widget;
};

BoDebugLogDialog::BoDebugLogDialog(QWidget* parent, const char* name)
	: KDialogBase(Tabbed, i18n("BoDebug messages"), Ok, Ok, parent, name, false, true)
{
 d = new BoDebugLogDialogPrivate;

 KVBox* errorPage = addVBoxPage(i18n("Errors"));
 KVBox* warningPage = addVBoxPage(i18n("Warnings"));
 KVBox* debugPage = addVBoxPage(i18n("Debug"));
 KVBox* allPage = addVBoxPage(i18n("All"));

 BoDebugLogWidget* error = new BoDebugLogWidget(errorPage);
 BoDebugLogWidget* warning = new BoDebugLogWidget(warningPage);
 BoDebugLogWidget* debug = new BoDebugLogWidget(debugPage);
 BoDebugLogWidget* all = new BoDebugLogWidget(allPage);
 d->mLevel2Widget.insert(BoDebug::KDEBUG_ERROR, error);
 d->mLevel2Widget.insert(BoDebug::KDEBUG_WARN, warning);
 d->mLevel2Widget.insert(BoDebug::KDEBUG_INFO, debug);
 d->mLevel2Widget.insert(-1, all);
}

BoDebugLogDialog::~BoDebugLogDialog()
{
 boDebug() << k_funcinfo << endl;
 delete d;
}

void BoDebugLogDialog::slotUpdate()
{
 updateWidget(-1);
 updateWidget(BoDebug::KDEBUG_ERROR);
 updateWidget(BoDebug::KDEBUG_WARN);
 updateWidget(BoDebug::KDEBUG_INFO);
}

void BoDebugLogDialog::updateWidget(int level)
{
 BoDebugLog* debugLog = BoDebugLog::debugLog();
 BO_CHECK_NULL_RET(debugLog);
 BoDebugLogWidget* w = d->mLevel2Widget[level];
 BO_CHECK_NULL_RET(w);
 w->setMessages(*debugLog->messageLogLevel(level));
}
