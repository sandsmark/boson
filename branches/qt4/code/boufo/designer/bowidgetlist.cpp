/*
    This file is part of the Boson game
    Copyright (C) 2004-2006 Andreas Beckermann (b_mann@gmx.de)

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

#include "bowidgetlist.h"
#include "bowidgetlist.moc"

#include "boufodebugwidget.h"
#include "bosignalsslotseditor.h"
#include "optionsdialog.h"
#include "../boufofactory.h"
#include <bodebug.h>

#include <qlayout.h>
#include <q3listbox.h>
//Added by qt3to4:
#include <Q3VBoxLayout>

#include <stdlib.h>

BoWidgetList::BoWidgetList(QWidget* parent, const char* name) : QWidget(parent, name)
{
 Q3VBoxLayout* l = new Q3VBoxLayout(this);
 mListBox = new Q3ListBox(this);
 connect(mListBox, SIGNAL(highlighted(Q3ListBoxItem*)),
		this, SLOT(slotWidgetHighlighted(Q3ListBoxItem*)));
 connect(mListBox, SIGNAL(selectionChanged()),
		this, SLOT(slotWidgetSelectionChanged()));
 l->addWidget(mListBox);

 QStringList widgets = BoUfoFactory::widgets();
 for (unsigned int i = 0; i < widgets.count(); i++) {
	mListBox->insertItem(widgets[i]);
 }
 clearSelection();
}

BoWidgetList::~BoWidgetList()
{
}

QString BoWidgetList::widget() const
{
 int index = mListBox->currentItem();
 Q3ListBoxItem* item = 0;
 if (index >= 0) {
	item = mListBox->item(index);
 }
 if (item) {
	return item->text();
 }
 return QString::null;
}

void BoWidgetList::slotWidgetHighlighted(Q3ListBoxItem* item)
{
// boDebug() << k_funcinfo << endl;
 if (item) {
	mListBox->setSelected(item, true);
 }
 slotWidgetSelectionChanged();
}

void BoWidgetList::slotWidgetSelectionChanged()
{
// boDebug() << k_funcinfo << endl;
 Q3ListBoxItem* item = mListBox->selectedItem();
 if (item) {
	QString widget;
	widget = item->text();
	emit signalWidgetSelected(widget);
 } else {
	clearSelection();
 }
}

void BoWidgetList::clearSelection()
{
 int index = mListBox->currentItem();
 Q3ListBoxItem* item = 0;
 if (index >= 0) {
	item = mListBox->item(index);
 }
 if (item) {
	mListBox->setSelected(item, false);
	emit signalWidgetSelected(QString::null);
 }
}

