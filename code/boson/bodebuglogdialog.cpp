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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "bodebuglogdialog.h"
#include "bodebuglogdialog.moc"

#include "bodebug.h"
#include "bodebuglog.h"

#include <klocale.h>

#include <qlistview.h>
#include <qlayout.h>
#include <qvbox.h>

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

class BoDebugLogWidgetPrivate
{
public:
	BoDebugLogWidgetPrivate()
	{
	}
	QListView* mMessages;
};

BoDebugLogWidget::BoDebugLogWidget(QWidget* parent, const char* name) : QWidget(parent, name)
{
 d = new BoDebugLogWidgetPrivate;
 QVBoxLayout* layout = new QVBoxLayout(this);
 d->mMessages = new QListView(this);
 layout->addWidget(d->mMessages);

 d->mMessages->setAllColumnsShowFocus(true);
 d->mMessages->addColumn(i18n("Nr"));
 d->mMessages->addColumn(i18n("Area"));
 d->mMessages->addColumn(i18n("AreaNr"));
 d->mMessages->addColumn(i18n("Level"));
 d->mMessages->addColumn(i18n("Text"));
}

BoDebugLogWidget::~BoDebugLogWidget()
{
 delete d;
}

void BoDebugLogWidget::setMessages(const QPtrList<BoDebugMessage>& m)
{
 d->mMessages->clear();
 QPtrListIterator<BoDebugMessage> it(m);
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
	item->setText(0, QString::number(i));
	item->setText(1, it.current()->areaName());
	item->setText(2, QString::number(it.current()->area()));
	item->setText(3, level);
	item->setText(4, it.current()->message());
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

 QVBox* errorPage = addVBoxPage(i18n("Errors"));
 QVBox* warningPage = addVBoxPage(i18n("Warnings"));
 QVBox* debugPage = addVBoxPage(i18n("Debug"));
 QVBox* allPage = addVBoxPage(i18n("All"));

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
