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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "bowidgettree.h"
#include "bowidgettree.moc"

#include "boufodebugwidget.h"
#include "bosignalsslotseditor.h"
#include "optionsdialog.h"

#include <bodebug.h>

#include <qtimer.h>
#include <qlayout.h>
#include <qlistbox.h>
#include <qlistview.h>
#include <qsplitter.h>
#include <qfile.h>
#include <qdom.h>
#include <qlabel.h>
#include <qcursor.h>
#include <qaction.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qvgroupbox.h>
#include <qwidgetstack.h>
#include <qsettings.h>

#include <math.h>
#include <stdlib.h>

// TODO: provide this information in the BoUfoFactory!
#warning FIXME: code duplication
static bool isContainerWidget(const QString& className)
{
 if (className.isEmpty()) {
	return false;
 }
 if (className == "BoUfoWidget") {
	return true;
 }
 if (className == "BoUfoHBox") {
	return true;
 }
 if (className == "BoUfoVBox") {
	return true;
 }
 if (className == "BoUfoWidgetStack") {
	return true;
 }
 if (className == "BoUfoLayeredPane") {
	return true;
 }
 if (className == "BoUfoButtonGroupWidget") {
	return true;
 }
 if (className == "BoUfoGroupBox") {
	return true;
 }
 return false;
}

BoWidgetTree::BoWidgetTree(QWidget* parent, const char* name) : QWidget(parent, name)
{
 QVBoxLayout* layout = new QVBoxLayout(this);
 mListView = new QListView(this);
 connect(mListView, SIGNAL(selectionChanged(QListViewItem*)),
		this, SLOT(slotSelectionChanged(QListViewItem*)));
 mListView->setRootIsDecorated(true);
 mListView->setSorting(-1);
 layout->addWidget(mListView);

 mListView->addColumn(tr("ClassName"));
 mListView->addColumn(tr("name"));

 QHBoxLayout* buttonLayout = new QHBoxLayout(layout);
 mInsertWidget = new QPushButton(tr("Insert"), this);
 connect(mInsertWidget, SIGNAL(clicked()),
		this, SLOT(slotInsert()));
 mRemoveWidget = new QPushButton(tr("Remove"), this);
 connect(mRemoveWidget, SIGNAL(clicked()),
		this, SLOT(slotRemove()));
 mMoveUp = new QPushButton(tr("Up"), this);
 connect(mMoveUp, SIGNAL(clicked()),
		this, SLOT(slotMoveUp()));
 mMoveDown = new QPushButton(tr("Down"), this);
 connect(mMoveDown, SIGNAL(clicked()),
		this, SLOT(slotMoveDown()));
 buttonLayout->addWidget(mInsertWidget);
 buttonLayout->addWidget(mRemoveWidget);
 buttonLayout->addWidget(mMoveUp);
 buttonLayout->addWidget(mMoveDown);
}

BoWidgetTree::~BoWidgetTree()
{
}

bool BoWidgetTree::isContainer(QListViewItem* item) const
{
 if (!item) {
	return false;
 }
 return isContainer(mItem2Element[item]);
}

bool BoWidgetTree::isContainer(const QDomElement& e) const
{
 if (e.isNull()) {
	return false;
 }
 QString className = e.namedItem("ClassName").toElement().text();
 return isContainerWidget(className);
}

void BoWidgetTree::slotInsert()
{
 QListViewItem* item = mListView->selectedItem();
 if (item) {
	QDomElement e = mItem2Element[item];
	emit signalInsertWidget(e);
 } else {
	boError() << k_funcinfo << "nothing selected" << endl;
 }
}

void BoWidgetTree::slotRemove()
{
 QListViewItem* item = mListView->selectedItem();
 if (item) {
	if (!item->parent()) {
		boWarning() << k_funcinfo << "Cannot remove root" << endl;
		return;
	}
	QDomElement e = mItem2Element[item];
	emit signalRemoveWidget(e);
 } else {
	boError() << k_funcinfo << "nothing selected" << endl;
 }
}

void BoWidgetTree::slotMoveUp()
{
 QListViewItem* item = mListView->selectedItem();
 if (!item) {
	boError() << k_funcinfo << "nothing selected" << endl;
	return;
 }
 QListViewItem* parent = item->parent();
 if (item && !parent) {
	boWarning() << k_funcinfo << "Cannot move root" << endl;
	return;
 }

 QListViewItem* prev = 0;
 QListViewItem* i;
 for (i = parent->firstChild(); i; i = i->nextSibling()) {
	if (i == item) {
		break;
	}
	prev = i;
 }
 if (!prev) {
	// no previous item for this parent - move item one level up
	prev = parent;
	parent = prev->parent();
	if (!parent) {
		// item is already the first item
		boDebug() << k_funcinfo << "cannot move up any further" << endl;
		return;
	}
 } else {
	if (isContainer(prev)) {
		parent = prev;
		prev = 0;
	}
 }

 BO_CHECK_NULL_RET(item);
 BO_CHECK_NULL_RET(parent);
 moveElement(item, parent, prev);
}

void BoWidgetTree::slotMoveDown()
{
// boDebug() << k_funcinfo << endl;
 QListViewItem* item = mListView->selectedItem();
 if (!item) {
	boError() << k_funcinfo << "nothing selected" << endl;
	return;
 }
 QListViewItem* parent = item->parent();
 if (item && !parent) {
	boWarning() << k_funcinfo << "Cannot move root" << endl;
	return;
 }

 QListViewItem* prev = 0;
 QListViewItem* next = item->nextSibling();
 if (next) {
	// parent is a container widget anyway, but we need to check it for the
	// sibling
	prev = next;
	while (next && !isContainer(next)) {
		next = next->nextSibling();
	}
	if (next) {
		prev = 0;
		parent = next;
		next = parent->firstChild();
	} else {
		next = prev->nextSibling();
	}
 } else {
	// no next item for this parent - move item one level up
	prev = parent;
	next = prev->nextSibling();
	parent = parent->parent();
	if (!parent) {
		boDebug() << k_funcinfo << "cannot move down any further" << endl;
		return;
	}
 }

 BO_CHECK_NULL_RET(item);
 BO_CHECK_NULL_RET(parent);
 moveElement(item, parent, next);
}

void BoWidgetTree::moveElement(QListViewItem* widget, QListViewItem* parent, QListViewItem* before)
{
 BO_CHECK_NULL_RET(widget);
 BO_CHECK_NULL_RET(parent);
 QListViewItem* oldParent = widget->parent();
 BO_CHECK_NULL_RET(oldParent);

 QDomElement w = mItem2Element[widget];
 QDomElement p = mItem2Element[parent];
 QDomElement b;
 if (before) {
	b = mItem2Element[before];
	if (b.isNull()) {
		boError() << k_funcinfo << "oops - null element for before" << endl;
		return;
	}
 }
 if (w.isNull()) {
	boError() << k_funcinfo << "oops - null element for widget" << endl;
	return;
 }
 if (p.isNull()) {
	boError() << k_funcinfo << "oops - null element for parent" << endl;
	return;
 }

 bool selected = widget->isSelected(); // should be the case
 oldParent->takeItem(widget);
 parent->insertItem(widget);

 if (b.isNull()) {
	p.appendChild(w);
	QListViewItem* after = parent->firstChild();
	while (after->nextSibling()) {
		after = after->nextSibling();
	}
	widget->moveItem(after);
 } else {
	p.insertBefore(w, b);

	QListViewItem* after = 0;
	QListViewItem* n = parent->firstChild();
	for (; n; n = n->nextSibling()) {
		if (n == before) {
			break;
		}
		after = n;
	}
	widget->moveItem(after);
 }
 widget->listView()->setSelected(widget, selected);

 emit signalHierarchyChanged();
}

void BoWidgetTree::updateGUI(const QDomElement& root)
{
 boDebug() << k_funcinfo << endl;
 mListView->clear();
// mWidgetRoot = root;
 mItem2Element.clear();
 if (root.isNull()) {
	boDebug() << k_funcinfo << "NULL root element" << endl;
	return;
 }
 if (root.namedItem("ClassName").toElement().text() != "BoUfoWidget" || root.tagName() != "Widgets") {
	boError() << k_funcinfo << "invalid root element" << endl;
	boDebug() << root.ownerDocument().toString() << endl;
	return;
 }
 QListViewItem* itemRoot = new QListViewItem(mListView,
		root.namedItem("ClassName").toElement().text(),
		root.namedItem("Properties").namedItem("name").toElement().text());
 itemRoot->setOpen(true);
 mItem2Element.insert(itemRoot, root);
 updateGUI(root, itemRoot);

 slotSelectionChanged(0);
}

void BoWidgetTree::updateGUI(const QDomElement& root, QListViewItem* itemParent)
{
 QListViewItem* after = 0;
 QDomNode n;
 for (n = root.firstChild(); !n.isNull(); n = n.nextSibling()) {
	QDomElement e = n.toElement();
	if (e.isNull()) {
		continue;
	}
	if (e.tagName() != "Widget") {
		continue;
	}
	QString className = e.namedItem("ClassName").toElement().text();
	if (className.isEmpty()) {
		boWarning() << k_funcinfo << "empty ClassName" << endl;
	}
	QString name = e.namedItem("Properties").namedItem("name").toElement().text();
	QListViewItem* item = new QListViewItem(itemParent, after, className, name);
	after = item; // new items are appended to the end
	item->setOpen(true);
	mItem2Element.insert(item, e);
	updateGUI(e, item);
 }
}

void BoWidgetTree::slotSelectionChanged(QListViewItem* item)
{
// boDebug() << k_funcinfo << endl;
 QDomElement e;
 if (item) {
	e = mItem2Element[item];
 }
 if (item) {
	mInsertWidget->setEnabled(true);
 } else {
	mInsertWidget->setEnabled(false);
 }
 if (item && item->parent()) {
	mRemoveWidget->setEnabled(true);
	mMoveUp->setEnabled(true);
	mMoveDown->setEnabled(true);
 } else {
	mRemoveWidget->setEnabled(false);
	mMoveUp->setEnabled(false);
	mMoveDown->setEnabled(false);
 }
 emit signalWidgetSelected(e);
}

void BoWidgetTree::selectWidget(const QDomElement& widget)
{
 QListViewItem* item = 0;
 if (!widget.isNull()) {
	QMap<QListViewItem*,QDomElement>::Iterator it;
	for (it = mItem2Element.begin(); it != mItem2Element.end(); ++it) {
		if (it.data() == widget) {
			item = it.key();
			break;
		}
	}
 }
 mListView->setSelected(item, true);
}

