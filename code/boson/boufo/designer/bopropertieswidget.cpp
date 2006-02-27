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

#include "bopropertieswidget.h"
#include "bopropertieswidget.moc"

#include <bodebug.h>

#include <qlayout.h>
#include <qlistview.h>
#include <qdom.h>
#include <qlabel.h>

#include <stdlib.h>

static void setElementText(QDomNode element, const QString& text)
{
 QDomElement e = element.toElement();
 if (e.isNull()) {
	return;
 }
 QDomNode n = e.firstChild();
 while (!n.isNull()) {
	if (n.isText()) {
		QDomNode n2 = n.nextSibling();
		e.removeChild(n);
		n = n2;
	} else {
		n = n.nextSibling();
	}
 }
 QDomDocument doc = e.ownerDocument();
 e.appendChild(doc.createTextNode(text));
}

class QEnumContainerListItem : public QCheckListItem
{
public:
	QEnumContainerListItem(QListView* lv, const QString& text)
		: QCheckListItem(lv, text, RadioButtonController)
	{
		mDisableItemChanged = false;
		setRenameEnabled(1, false);
	}

	/**
	 * Called by @ref QEnumCheckListItem only
	 **/
	void selectedItemChanged()
	{
		if (mDisableItemChanged) {
			return;
		}
		disableItemChanged(true);
		QListViewItem* child = firstChild();
		while (child) {
			if (child->rtti() == 1) {
				QCheckListItem* check = (QCheckListItem*)child;
				if (check->isOn()) {
					int col = 1;
					setRenameEnabled(col, true);
					setText(col, check->text(0));
					startRename(col);
					okRename(col);
					setRenameEnabled(col, false);
				}
			}
			child = child->nextSibling();
		}
		disableItemChanged(false);
	}

	void disableItemChanged(bool d)
	{
		mDisableItemChanged = d;
	}

private:
	bool mDisableItemChanged;
};

class QEnumCheckListItem : public QCheckListItem
{
public:
	QEnumCheckListItem(QEnumContainerListItem* item, const QString& text)
		: QCheckListItem(item, text, RadioButton)
	{
	}

	/**
	 * Call @ref setOn, but don't call@ ref
	 * QEnumContainerListItem::selectedItemChanged
	 **/
	void setManualOn(bool on)
	{
		((QEnumContainerListItem*)parent())->disableItemChanged(true);
		setOn(on);
		((QEnumContainerListItem*)parent())->disableItemChanged(false);
	}
	
protected:
	virtual void stateChange(bool s)
	{
		QCheckListItem::stateChange(s);
		BO_CHECK_NULL_RET(parent());
		QEnumContainerListItem* c = (QEnumContainerListItem*)parent();
		c->selectedItemChanged();
	}

};


BoPropertiesWidget::BoPropertiesWidget(QWidget* parent, const char* name) : QWidget(parent, name)
{
 QVBoxLayout* layout = new QVBoxLayout(this);
 mClassLabel = new QLabel(this);
 layout->addWidget(mClassLabel);
 mListView = new QListView(this);
 connect(mListView, SIGNAL(itemRenamed(QListViewItem*, int)),
		this, SLOT(slotItemRenamed(QListViewItem*, int)));
 mListView->setDefaultRenameAction(QListView::Accept);
 layout->addWidget(mListView);
 mListView->addColumn(tr("Property"));
 mListView->addColumn(tr("Value"));

 setClassLabel(QString::null);
}

BoPropertiesWidget::~BoPropertiesWidget()
{

}

void BoPropertiesWidget::setClassLabel(const QString& text)
{
 if (text.isEmpty()) {
	setClassLabel(tr("(Nothing selected)"));
	return;
 } else {
	mClassLabel->setText(text);
 }
}

void BoPropertiesWidget::displayProperties(const QDomElement& e)
{
 mListView->clear();
 setClassLabel(QString::null);
 mWidgetElement = e;
 if (e.isNull()) {
	return;
 }
 createProperties(e);

 QString className = e.namedItem("ClassName").toElement().text();
 QMetaObject* metaObject = 0;

 // WARNING: trolltech marks this as internal!
 // but it is sooo useful
 metaObject = QMetaObject::metaObject(className);

 if (!metaObject) {
	boError() << k_funcinfo << "cannot find class " << className << endl;
	return;
 }

 QDomElement properties = e.namedItem("Properties").toElement();
 QListViewItem* child = mListView->firstChild();
 for (; child; child = child->nextSibling()) {
	QString name = child->text(0);
	int index = metaObject->findProperty(name, true);
	if (index < 0) {
		boWarning() << k_funcinfo << "don't know property " << name << " in class " << className << endl;
		continue;
	}
	const QMetaProperty* prop = metaObject->property(index, true);
	QString value = properties.namedItem(name).toElement().text();
	if (prop->isSetType()) {
		boWarning() << k_funcinfo << "property is a set - this is not supported yet" << endl;
		// it'll be displayed as normal text.

		// a set can be implemented just like a normal enum, but instead
		// of radio buttons we use checkboxes (values can be ORed
		// together).
		// but we probably don't need that anyway
	}
	if (prop->isEnumType() && !prop->isSetType()) {
		QEnumContainerListItem* item = dynamic_cast<QEnumContainerListItem*>(child);
		if (!item) {
			boError() << k_funcinfo << "child is not QEnumContainerListItem, but should be!" << endl;
			continue;
		}
		item->setText(1, value);

		QListViewItem* c = item->firstChild();
		for (; c; c = c->nextSibling()) {
			QEnumCheckListItem* check = dynamic_cast<QEnumCheckListItem*>(c);
			if (!check) {
				boError() << k_funcinfo << "not a QEnumCheckListItem" << endl;
				continue;
			}
			if (check->text(0) == value) {
				check->setManualOn(true);
			}
		}
	} else {
		child->setText(1, value);
	}
 }
 if (className.isEmpty() && e.tagName() == "Widgets") {
	className = tr("BoUfoWidget (root widget)");
 }
 setClassLabel(className);
}

// this method creates the QListViewItem objects, but does not assign any
// content to them
void BoPropertiesWidget::createProperties(const QDomElement& e)
{
 QString className;
 QMetaObject* metaObject = 0;

 className = e.namedItem("ClassName").toElement().text();
 if (!className.isEmpty()) {
	// WARNING: trolltech marks this as internal!
	// but it is sooo useful
	metaObject = QMetaObject::metaObject(className);
 }

 if (!metaObject) {
	boError() << k_funcinfo << "cannot find class " << className << endl;
	return;
 }

 QDomElement properties = e.namedItem("Properties").toElement();
 if (properties.isNull()) {
	boError() << k_funcinfo << "Cannot find Properties element" << endl;
	return;
 }
 for (QDomNode n = properties.firstChild(); !n.isNull(); n = n.nextSibling()) {
	QDomElement e = n.toElement();
	if (e.isNull()) {
		continue;
	}
	int index = metaObject->findProperty(e.tagName(), true);
	if (index < 0) {
		boWarning() << k_funcinfo << "don't know property " << e.tagName() << " in class " << className << endl;
		continue;
	}
	const QMetaProperty* prop = metaObject->property(index, true);
	if (prop->isSetType()) {
		boWarning() << k_funcinfo << "property is a set - this is not supported yet" << endl;
		// it'll be displayed as normal text.

		// a set can be implemented just like a normal enum, but instead
		// of radio buttons we use checkboxes (values can be ORed
		// together).
		// but we probably don't need that anyway
	}
	if (prop->isEnumType() && !prop->isSetType()) {
		QEnumContainerListItem* item = new QEnumContainerListItem(mListView, e.tagName());
		item->setOpen(true);
		QStrList enums = prop->enumKeys();
		QStrListIterator it(enums);
		while (it.current()) {
			(void)new QEnumCheckListItem(item, QString::fromLatin1(it.current()));
			++it;
		}
	} else {
		QListViewItem* item = new QListViewItem(mListView, e.tagName(), e.text());
		item->setRenameEnabled(1, true);
	}


 }

}

void BoPropertiesWidget::slotItemRenamed(QListViewItem* item, int col)
{
 if (col != 1) {
	boError() << k_funcinfo << "column other than Value renamed?! col=" << col << endl;
	return;
 }
 QString name = item->text(0);
 QString value = item->text(1);

 QDomElement properties = mWidgetElement.namedItem("Properties").toElement();
 if (properties.isNull()) {
	boError() << k_funcinfo << "NULL Properties element" << endl;
	return;
 }
 setElementText(properties.namedItem(name), value);
 emit signalChanged(mWidgetElement);

 // TODO: if the name of the widget was changed: update the widget tree!
}

