/*
    This file is part of the Boson game
    Copyright (C) 2004 Andreas Beckermann <b_mann@gmx.de>

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

#include "boglstatewidget.h"
#include "boglstatewidget.moc"

#include "../bomemory/bodummymemory.h"
#include "info/boglquerystates.h"
#include <bogl.h>

#include <bodebug.h>

#include <klocale.h>

#include <qstringlist.h>
#include <qmap.h>
#include <q3listview.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <q3hbox.h>
#include <qcombobox.h>
#include <qlabel.h>
//Added by qt3to4:
#include <Q3VBoxLayout>

#include <stdlib.h>

class BoGLStateWidgetPrivate
{
public:
	BoGLStateWidgetPrivate()
	{
		mListView = 0;
		mDisplayStates = 0;
		mDisplayAll = -1;
		mDisplayImplementation = -1;
		mDisplayDynamic = -1;
		mDisplayEnable = -1;
	}
	Q3ListView* mListView;

	QComboBox* mDisplayStates;
	int mDisplayAll;
	int mDisplayImplementation;
	int mDisplayDynamic;
	int mDisplayEnable;

	BoGLQueryStates mStates;
};

BoGLStateWidget::BoGLStateWidget(QWidget* parent)
	: QWidget(parent)
{
 d = new BoGLStateWidgetPrivate;
 Q3VBoxLayout* layout = new Q3VBoxLayout(this);
 d->mListView = new Q3ListView(this);
 d->mListView->addColumn(i18n("OpenGL state name"));
 d->mListView->addColumn(i18n("Value"));
 d->mListView->setAllColumnsShowFocus(true);
 layout->addWidget(d->mListView);

 Q3HBox* hbox = new Q3HBox(this);
 layout->addWidget(hbox);

 Q3HBox* stateSelection = new Q3HBox(hbox);
 (void)new QLabel(i18n("Display states:"), stateSelection);
 d->mDisplayStates = new QComboBox(stateSelection);
 connect(d->mDisplayStates, SIGNAL(activated(int)), this, SLOT(slotChangeStates(int)));
 initStateSelections();

 QPushButton* update = new QPushButton(i18n("Update"), hbox);
 connect(update, SIGNAL(clicked()), this, SLOT(slotUpdate()));


 d->mStates.init();
 slotUpdate();
}

BoGLStateWidget::~BoGLStateWidget()
{
 delete d;
}

void BoGLStateWidget::initStateSelections()
{
 QComboBox* c = d->mDisplayStates;
 d->mDisplayAll = c->count(); c->insertItem(i18n("All"));
 d->mDisplayImplementation = c->count(); c->insertItem(i18n("Implementation dependant"));
 d->mDisplayDynamic = c->count(); c->insertItem(i18n("Dynamic"));
 d->mDisplayEnable = c->count(); c->insertItem(i18n("Enable states"));
}

void BoGLStateWidget::slotUpdate()
{
 d->mStates.getStates();
 slotChangeStates(d->mDisplayStates->currentItem());
}

void BoGLStateWidget::slotChangeStates(int index)
{
 QStringList list;
 if (index == d->mDisplayAll) {
	// AB: do not update the states here
	list = d->mStates.oldStateList();
	list += d->mStates.implementationValueList();
 } else if (index == d->mDisplayImplementation) {
	list = d->mStates.implementationValueList();
 } else if (index == d->mDisplayDynamic) {
	list = d->mStates.oldStateList();
 } else if (index == d->mDisplayEnable) {
	list = d->mStates.oldStateEnabledList();
 } else {
		boError() << k_funcinfo << "invalid index " << index << endl;
 }
 list.sort();
 makeList(d->mListView, list);
}

void BoGLStateWidget::makeList(Q3ListView* l, const QStringList& list)
{
 if (!l) {
	return;
 }
 if (l->columns() < 2) {
	return;
 }
 l->clear();
 if (list.count() == 0) {
	new Q3ListViewItem(l, i18n("None"), i18n("-"));
	return;
 }
 QStringList::ConstIterator it;
 for (it = list.begin(); it != list.end(); ++it) {
	QStringList s = QStringList::split(" = ", *it);
	if (s.count() != 2) {
		boDebug() << s.count() << endl;
		boWarning() << k_funcinfo << "invalid string " << *it << endl;
		continue;
	}
	new Q3ListViewItem(l, s[0], s[1]);
 }
}

