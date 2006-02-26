/*
    This file is part of the Boson game
    Copyright (C) 2004-2005 Andreas Beckermann (b_mann@gmx.de)

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

#include <ufo/ufo.hpp>
#include <ufo/ux/ux.hpp>
#include "../boufo.h"
#include <bogl.h>

#include "boufodesignermain.h"
#include "boufodesignermain.moc"

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

//static const char *version = BOSON_VERSION_STRING;

#if 0
static void removeElementChildren(QDomElement& e)
{
 if (e.isNull() || !e.hasChildren()) {
	return;
 }
 while (!e.firstChild().isNull()) {
	e.removeChild(e.firstChild());
 }
}
#endif

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

static void convertFromAttributeFormat(QDomElement& widgets)
{
 for (QDomNode n = widgets.firstChild(); !n.isNull(); n = n.nextSibling()) {
	QDomElement e = n.toElement();
	if (e.isNull()) {
		continue;
	}
	if (e.tagName() != "Widget") {
		continue;
	}
	convertFromAttributeFormat(e);
 }

 QDomDocument doc = widgets.ownerDocument();
 QDomElement className = doc.createElement("ClassName");
 widgets.insertBefore(className, widgets.firstChild());
 className.appendChild(doc.createTextNode(widgets.attribute("ClassName")));

 QDomElement properties = doc.createElement("Properties");
 widgets.insertAfter(properties, className);

 QDomNamedNodeMap attributes = widgets.attributes();
 for (unsigned int i = 0; i < attributes.count(); i++) {
	QDomAttr a = attributes.item(i).toAttr();
	if (a.name() == "ClassName") {
		continue;
	}
	QDomElement e = doc.createElement(a.name());
	e.appendChild(doc.createTextNode(a.value()));
	properties.appendChild(e);
 }
 while (widgets.attributes().count() != 0) {
	widgets.removeAttributeNode(widgets.attributes().item(0).toAttr());
 }
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


// TODO: provide this information in the BoUfoFactory!
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
 return false;
}



FormPreview::FormPreview(const QGLFormat& format, QWidget* parent) : QGLWidget(format, parent)
{
// qApp->setGlobalMouseTracking(true);
// qApp->installEventFilter(this);
 setMouseTracking(true);
// setFocusPolicy(StrongFocus);

 setMinimumSize(200, 200);

 mUfoManager = 0;
 mPlacementMode = false;

 QTimer* updateTimer = new QTimer(this);
 connect(updateTimer, SIGNAL(timeout()), this, SLOT(updateGL()));
 updateTimer->start(40);
 setUpdatesEnabled(false);
}

FormPreview::~FormPreview()
{
 boDebug() << k_funcinfo << endl;
 delete mUfoManager;
 boDebug() << k_funcinfo << "done" << endl;
// qApp->setGlobalMouseTracking(false);
}

BoUfoWidget* FormPreview::getWidgetAt(int x, int y)
{
 ufo::UWidget* widget = 0;
 widget = mUfoManager->contentWidget()->ufoWidget()->getWidgetAt(x, y);
 if (!widget) {
	return 0;
 }
 BoUfoWidget* w = mUfoWidget2Widget[widget];

 while (!w && widget) {
	widget = widget->getParent();
	w = mUfoWidget2Widget[widget];
 }

 return w;
}

BoUfoWidget* FormPreview::getContainerWidgetAt(int x, int y)
{
 BoUfoWidget* w = getWidgetAt(x, y);
 if (!w) {
	return w;
 }
 QDomElement e = mUfoWidget2Element[w->ufoWidget()];
 QString className;
 if (!e.isNull()) {
	className = e.namedItem("ClassName").toElement().text();
 }
 while (w && !isContainerWidget(className)) {
	ufo::UWidget* widget = w->ufoWidget();
	w = 0;
	while (!w && widget) {
		widget = widget->getParent();
		w = mUfoWidget2Widget[widget];
	}
	if (!w) {
		return 0;
	}
	e = mUfoWidget2Element[w->ufoWidget()];
	if (e.isNull()) {
		boError() << k_funcinfo << "NULL element for BoUfoWidget() !" << endl;
		return 0;
	}
	className = e.namedItem("ClassName").toElement().text();
 }
 return w;
}

void FormPreview::updateGUI(const QDomElement& root)
{
 glInit();
 BO_CHECK_NULL_RET(mUfoManager);
 makeCurrent();
 mUfoManager->contentWidget()->ufoWidget()->removeAll();
 mContentWidget->loadPropertiesFromXML(root.namedItem("Properties").toElement());

 mUfoWidget2Element.clear();
 mUfoWidget2Widget.clear();
 addWidget(mContentWidget, root);
 updateGUI(root, mContentWidget);
}

void FormPreview::updateGUI(const QDomElement& root, BoUfoWidget* parent)
{
 boDebug() << k_funcinfo << endl;
 if (root.isNull() || !parent) {
	return;
 }
 bool useConvenienceBackground = false;
 QDomNode n;
 static int depth = 0;
 bool useRed = true;
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
	BoUfoWidget* widget = BoUfoFactory::createWidget(className);
	if (!widget) {
		boError() << k_funcinfo << "could not create widget with ClassName " << className << endl;
		continue;
	}
	parent->addWidget(widget);
	addWidget(widget, e);

	if (useConvenienceBackground) {
		int blue = depth;
		if (blue > 255) {
			blue = 255;
		}
		if (useRed) {
			widget->setBackgroundColor(QColor(255, 0, blue));
		} else {
			widget->setBackgroundColor(QColor(0, 255, blue));
		}
		useRed = !useRed;
	}

	widget->loadPropertiesFromXML(e.namedItem("Properties").toElement());

	if (useConvenienceBackground) {
		widget->setOpaque(true);
	}

	if (widget->name() == mNameOfSelectedWidget) {
		widget->setBorderType(BoUfoWidget::LineBorder);
	}

	depth += 40;
	updateGUI(e, widget);
	depth -= 40;
 }
}

void FormPreview::addWidget(BoUfoWidget* widget, const QDomElement& e)
{
 BO_CHECK_NULL_RET(widget);
 BO_CHECK_NULL_RET(widget->ufoWidget());
 if (e.isNull()) {
	boError() << k_funcinfo << "NULL element" << endl;
	return;
 }
 mUfoWidget2Element.insert(widget->ufoWidget(), e);
 mUfoWidget2Widget.insert(widget->ufoWidget(), widget);
}

void FormPreview::setPlacementMode(bool m)
{
 mPlacementMode = m;
}

void FormPreview::initializeGL()
{
 static bool recursive = false;
 static bool initialized = false;
 if (recursive) {
	return;
 }
 if (initialized) {
	return;
 }
 recursive = true;
 makeCurrent();

 glDisable(GL_DITHER);

 boDebug() << k_funcinfo << endl;
 mUfoManager = new BoUfoManager(width(), height(), true);
 mContentWidget = mUfoManager->contentWidget();

 recursive = false;
 initialized = true;
}

void FormPreview::resizeGL(int w, int h)
{
 makeCurrent();
 if (mUfoManager) {
	// WARNING: FIXME: we use size == oldsize
	// doesn't harm atm, as BoUfo does not use oldsize
	QResizeEvent r(QSize(w, h), QSize(w, h));

	mUfoManager->sendEvent(&r);
 }
}

void FormPreview::paintGL()
{
 if (mUfoManager) {
	mUfoManager->dispatchEvents();
	mUfoManager->render(false);
 }
}

bool FormPreview::eventFilter(QObject* o, QEvent* e)
{
 return QObject::eventFilter(o, e);
}

void FormPreview::mouseMoveEvent(QMouseEvent* e)
{
 if (mUfoManager) {
	mUfoManager->sendEvent(e);
 }
}

void FormPreview::mousePressEvent(QMouseEvent* e)
{
 Q_UNUSED(e);
#if 0
 // AB: we display the ufo widgets only, we don't use them. so don't deliver
 // this event.
 if (mUfoManager) {
	mUfoManager->sendEvent(e);
 }
#endif

 if (mPlacementMode) {
	// we need a _container_ widget. it doesn't make sense to add children
	// to a button or so.
	QPoint pos = mapFromGlobal(QCursor::pos());
	BoUfoWidget* w = getContainerWidgetAt(pos.x(), pos.y());
	QDomElement parent;
	if (w) {
		parent = mUfoWidget2Element[w->ufoWidget()];
	}
	emit signalPlaceWidget(parent);
 } else {
	selectWidgetUnderCursor();
 }
}

void FormPreview::mouseReleaseEvent(QMouseEvent* e)
{
 Q_UNUSED(e);
#if 0
 // AB: we display the ufo widgets only, we don't use them. so don't deliver
 // this event.
 if (mUfoManager) {
	mUfoManager->sendEvent(e);
 }
#endif
}

void FormPreview::wheelEvent(QWheelEvent* e)
{
 if (mUfoManager) {
	mUfoManager->sendEvent(e);
 }
}

void FormPreview::keyPressEvent(QKeyEvent* e)
{
 if (mUfoManager) {
	mUfoManager->sendEvent(e);
 }
 QGLWidget::keyPressEvent(e);
}

void FormPreview::keyReleaseEvent(QKeyEvent* e)
{
 if (mUfoManager) {
	mUfoManager->sendEvent(e);
 }
 QGLWidget::keyReleaseEvent(e);
}

void FormPreview::selectWidgetUnderCursor()
{
 QPoint pos = mapFromGlobal(QCursor::pos());
 BoUfoWidget* w = getWidgetAt(pos.x(), pos.y());
 selectWidget(w);
}

void FormPreview::selectWidget(BoUfoWidget* widget)
{
 QDomElement widgetUnderCursor;
 if (widget && widget->ufoWidget()) {
	widgetUnderCursor = mUfoWidget2Element[widget->ufoWidget()];
 }

 emit signalSelectWidget(widgetUnderCursor);
}

void FormPreview::setSelectedWidget(const QDomElement& widget)
{
 // called e.g. when a widget in the widgettree is selected
 mNameOfSelectedWidget = QString::null;
 if (widget.isNull()) {
	return;
 }
 QDomElement properties = widget.namedItem("Properties").toElement();
 if (properties.isNull()) {
	boError() << k_funcinfo << "no Properties" << endl;
	return;
 }
 mNameOfSelectedWidget = properties.namedItem("name").toElement().text();

 BoUfoWidget* contentWidget = mUfoManager->contentWidget();
 BO_CHECK_NULL_RET(contentWidget);
 QDomElement root = mUfoWidget2Element[contentWidget->ufoWidget()];
 if (root.isNull()) {
	boError() << k_funcinfo << "cannot find root widget" << endl;
	return;
 }
 updateGUI(root);
}



BoWidgetList::BoWidgetList(QWidget* parent, const char* name) : QWidget(parent, name)
{
 QVBoxLayout* l = new QVBoxLayout(this);
 mListBox = new QListBox(this);
 connect(mListBox, SIGNAL(highlighted(QListBoxItem*)),
		this, SLOT(slotWidgetHighlighted(QListBoxItem*)));
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
 QListBoxItem* item = 0;
 if (index >= 0) {
	item = mListBox->item(index);
 }
 if (item) {
	return item->text();
 }
 return QString::null;
}

void BoWidgetList::slotWidgetHighlighted(QListBoxItem* item)
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
 QListBoxItem* item = mListBox->selectedItem();
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
 QListBoxItem* item = 0;
 if (index >= 0) {
	item = mListBox->item(index);
 }
 if (item) {
	mListBox->setSelected(item, false);
	emit signalWidgetSelected(QString::null);
 }
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

BoUfoDesignerMain::BoUfoDesignerMain()
	: QMainWindow(0, "mainwindow", WType_TopLevel | WDestructiveClose)
{
 mIface = new BoDebugDCOPIface();
 QWidget* topWidget = new QWidget(this);
 setCentralWidget(topWidget);
 mOptionsDialog = 0;

 QHBoxLayout* layout = new QHBoxLayout(topWidget);

 QSplitter* splitter = new QSplitter(topWidget);
 layout->addWidget(splitter);

 mWidgets = new BoWidgetList(splitter);
 splitter->setResizeMode(mWidgets, QSplitter::KeepSize);
 connect(mWidgets, SIGNAL(signalWidgetSelected(const QString&)),
		this, SLOT(slotWidgetClassSelected(const QString&)));
 mWidgets->show();

 QGLFormat format;
// format.setDirectRendering(false);
 mPreview = new FormPreview(format, splitter);
 connect(mPreview, SIGNAL(signalPlaceWidget(const QDomElement&)),
		this, SLOT(slotPlaceWidget(const QDomElement&)));
 connect(mPreview, SIGNAL(signalSelectWidget(const QDomElement&)),
		this, SLOT(slotSelectWidget(const QDomElement&)));
 mPreview->show();

 QSplitter* vsplitter = new QSplitter(Vertical, splitter);
 splitter->setResizeMode(vsplitter, QSplitter::KeepSize);
 mWidgetTree = new BoWidgetTree(vsplitter);
 connect(mWidgetTree, SIGNAL(signalWidgetSelected(const QDomElement&)),
		this, SLOT(slotSelectWidget(const QDomElement&)));
 connect(mWidgetTree, SIGNAL(signalInsertWidget(const QDomElement&)),
		this, SLOT(slotPlaceWidget(const QDomElement&)));
 connect(mWidgetTree, SIGNAL(signalRemoveWidget(const QDomElement&)),
		this, SLOT(slotRemoveWidget(const QDomElement&)));
 connect(mWidgetTree, SIGNAL(signalHierarchyChanged()),
		this, SLOT(slotTreeHierarchyChanged()));
 mWidgetTree->show();

 mProperties = new BoPropertiesWidget(vsplitter);
 mProperties->show();
 connect(mProperties, SIGNAL(signalChanged(const QDomElement&)),
		this, SLOT(slotPropertiesChanged(const QDomElement&)));

 initActions();

 slotApplyOptions();
}

BoUfoDesignerMain::~BoUfoDesignerMain()
{
 boDebug() << k_funcinfo << endl;
 delete mIface;
}

bool BoUfoDesignerMain::slotCreateNew()
{
 QDomDocument doc("BoUfoDesigner");
 QDomElement root = doc.createElement("BoUfoDesigner");
 doc.appendChild(root);
 QDomElement widgets = doc.createElement("Widgets");
 root.appendChild(widgets);
 initProperties(widgets, "BoUfoWidget"); // root widget
 setElementText(widgets.namedItem("Properties").namedItem("name"), "BoClassName"); // the name of the root widget is the name of the generated class

 return slotLoadFromXML(doc.toCString());
}

bool BoUfoDesignerMain::slotLoadFromFile(const QString& fileName)
{
 QFile file(fileName);
 if (!file.open(IO_ReadOnly)) {
	boError() << k_funcinfo << "could not open " << fileName << endl;
	return false;
 }
 QByteArray b = file.readAll();
 file.close();
 if (b.size() == 0) {
	boError() << k_funcinfo << "nothing read from file" << endl;
	return false;
 }
 return slotLoadFromXML(b);
}

bool BoUfoDesignerMain::slotSaveAsFile(const QString& fileName)
{
 QFile file(fileName);
 if (!file.open(IO_WriteOnly)) {
	boError() << k_funcinfo << "could not open " << fileName << " for writing" << endl;
	return false;
 }
 QTextStream t(&file);
 t << mDocument;
// file.writeBlock(mDocument.toCString());
 file.close();
 return true;
}

bool BoUfoDesignerMain::slotLoadFromXML(const QByteArray& xml)
{
 if (xml.size() == 0) {
	boError() << k_funcinfo << "empty xml" << endl;
	return false;
 }
 QDomDocument doc;
 if (!doc.setContent(QString(xml))) {
	// TODO: print parse error
	boError() << k_funcinfo << "parsing error in xml" << endl;
	return false;
 }
 QDomElement root = doc.documentElement();
 if (root.isNull()) {
	boError() << k_funcinfo << "NULL root element" << endl;
	return false;
 }
 QDomElement widgetsRoot = root.namedItem("Widgets").toElement();
 if (widgetsRoot.isNull()) {
	boError() << k_funcinfo << "no Widgets element" << endl;
	return false;
 }

 if (widgetsRoot.hasAttribute("ClassName") && widgetsRoot.namedItem("ClassName").isNull()) {
	boDebug() << k_funcinfo << "converting from obsolete file format..." << endl;
	convertFromAttributeFormat(widgetsRoot);
	boDebug() << k_funcinfo << "converted." << endl;
 }

 initProperties(widgetsRoot, widgetsRoot.namedItem("ClassName").toElement().text());
 QDomNodeList list = widgetsRoot.elementsByTagName("Widget");
 for (unsigned int i = 0; i < list.count(); i++) {
	QDomElement e = list.item(i).toElement();
	initProperties(e, e.namedItem("ClassName").toElement().text());
 }
 boDebug() << k_funcinfo << endl;
 mDocument = doc;
 slotUpdateGUI();
 return true;
}

void BoUfoDesignerMain::slotWidgetClassSelected(const QString& w)
{
 mPlaceWidgetClass = w;
 if (mPlaceWidgetClass.isEmpty()) {
	mPreview->setPlacementMode(false);
 } else {
	mPreview->setPlacementMode(true);
 }
}


void BoUfoDesignerMain::slotPlaceWidget(const QDomElement& _parent)
{
 QDomElement parent = _parent;
 if (parent.isNull()) {
	boDebug() << k_funcinfo << "null parent - use root widget" << endl;
	QDomElement root = mDocument.documentElement();
	parent = root.namedItem("Widgets").toElement();
 }
 if (parent.isNull()) {
	boError() << k_funcinfo << "NULL parent element" << endl;
	return;
 }
 if (mPlaceWidgetClass.isEmpty()) {
	boDebug() << k_funcinfo << "no widget selected" << endl;
	return;
 }
 if (!isContainerWidget(parent.namedItem("ClassName").toElement().text())) {
	boError() << k_funcinfo << "Can add to container widgets only. selected parent is a "
			<< parent.namedItem("ClassName").toElement().text()
			<< " which is not a container widget" << endl;
	return;
 }

 if (!BoUfoFactory::widgets().contains(mPlaceWidgetClass)) {
	boError() << k_funcinfo << mPlaceWidgetClass << " is not a known class name" << endl;
	return;
 }

 boDebug() << k_funcinfo << endl;

 QDomElement widget = parent.ownerDocument().createElement("Widget");
 parent.appendChild(widget);

 QDomNodeList widgetList = widget.ownerDocument().documentElement().elementsByTagName("Widget");
 int n = widgetList.count();
 QString name;
 bool ok;
 do {
	n++;
	name = QString("mWidget%1").arg(n);
	ok = true;
	// AB: this is pain slow. but who cares..
	// (atm the widget number will always be low enough, so this won't
	// matter)
	for (unsigned int i = 0; i < widgetList.count(); i++) {
		QDomElement e = widgetList.item(i).toElement();
		if (e == widget) {
			continue;
		}
		if (e.namedItem("Properties").namedItem("name").toElement().text() == name) {
			ok = false;
			break;
		}
	}
 } while (!ok);
 initProperties(widget, mPlaceWidgetClass);
 setElementText(widget.namedItem("Properties").namedItem("name"), name);

 slotUpdateGUI();
}

void BoUfoDesignerMain::slotRemoveWidget(const QDomElement& widget)
{
 if (widget.isNull()) {
	return;
 }
 QDomNode parent = widget.parentNode();
 if (parent.isNull()) {
	boError() << k_funcinfo << "parent node is NULL" << endl;
	return;
 }
 parent.removeChild(widget);
 slotUpdateGUI();
}

void BoUfoDesignerMain::initProperties(QDomElement& widget, const QString& className)
{
 if (className.isEmpty()) {
	boError() << k_funcinfo << "empty ClassName" << endl;
	return;
 }
 if (widget.isNull()) {
	boError() << k_funcinfo << "NULL widget element" << endl;
	return;
 }
 QDomDocument doc = widget.ownerDocument();
 QDomElement classNameElement = widget.namedItem("ClassName").toElement();
 if (classNameElement.isNull()) {
	classNameElement = doc.createElement("ClassName");
	widget.appendChild(classNameElement);
 }
 setElementText(classNameElement, className);

 // WARNING: trolltech marks this as internal!
 // but it is sooo useful
 QMetaObject* metaObject = QMetaObject::metaObject(className);

 if (!metaObject) {
	boError() << k_funcinfo << "don't know class " << className << endl;
	return;
 }

 QDomElement propertiesElement = widget.namedItem("Properties").toElement();
 if (propertiesElement.isNull()) {
	propertiesElement = doc.createElement("Properties");
	widget.appendChild(propertiesElement);
 }

 // AB: non-boson superclasses are QObject and Qt. Only QObject has properties
 // and even there only "name". We use "name" as variable name.
 QStrList properties = metaObject->propertyNames(true);
 QStrListIterator it(properties);
 while (it.current()) {
	provideProperty(widget, *it);
	++it;
 }

 // TODO: we somehow must retrieve decent default values.
 // the easiest way would be to create an object of that class and then save all
 // properties to XML.
 // unfortunately we cannot create BoUfo widgets here.
 // so atm we take care of the layout property only, which is the most important
 // property that has a default value in some widgets.
 if (className == "BoUfoVBox") {
	setElementText(propertiesElement.namedItem("layout"), "UVBoxLayout");
 } else if (className == "BoUfoHBox") {
	setElementText(propertiesElement.namedItem("layout"), "UHBoxLayout");
 }
}

void BoUfoDesignerMain::slotSelectWidget(const QDomElement& widget)
{
// boDebug() << k_funcinfo << endl;
 mProperties->displayProperties(widget);
 mWidgetTree->selectWidget(widget);
 mPreview->setSelectedWidget(widget); // paint a "rect" around the widget
}

void BoUfoDesignerMain::slotPropertiesChanged(const QDomElement& widget)
{
 QString name = widget.namedItem("Properties").namedItem("name").toElement().text();
 QDomNodeList list = mDocument.documentElement().elementsByTagName("Widget");
 for (unsigned int i = 0; i < list.count(); i++) {
	QDomElement e = list.item(i).toElement();
	if (e == widget) {
		continue;
	}
	if (e.namedItem("Properties").namedItem("name").toElement().text() == name) {
		// TODO: somehow revert to the old name
		boError() << k_funcinfo << "used the same name more than once - this won't compile eventually!" << endl;
		break;
	}
 }

 // WARNING: we are in a slot here and MUST NOT delete items from mProperties !!

 // here we need to update the preview only, as only a property has changed.
 // neither the widget hierarchy, nor the available properties have changed.
 QDomElement root = mDocument.documentElement();
 QDomElement widgetsRoot;
 if (!root.isNull()) {
	widgetsRoot = root.namedItem("Widgets").toElement();
 }
 mPreview->updateGUI(widgetsRoot);
}

void BoUfoDesignerMain::slotTreeHierarchyChanged()
{
 // AB: we don't need to rebuild the tree here. just the preview, nothing else

 QDomElement root = mDocument.documentElement();
 QDomElement widgetsRoot;
 if (!root.isNull()) {
	widgetsRoot = root.namedItem("Widgets").toElement();
 }

 mPreview->updateGUI(widgetsRoot);

 // TODO: select currently selected widget in the preview
}

void BoUfoDesignerMain::slotUpdateGUI()
{
 QDomElement root = mDocument.documentElement();
 QDomElement widgetsRoot;
 if (!root.isNull()) {
	widgetsRoot = root.namedItem("Widgets").toElement();
 }
 mPreview->updateGUI(widgetsRoot);
 mWidgetTree->updateGUI(widgetsRoot);
 mProperties->displayProperties(QDomElement());

 mWidgets->clearSelection();
}

void BoUfoDesignerMain::provideProperty(QDomElement& e, const QString& property)
{
 if (e.namedItem("Properties").namedItem(property).toElement().isNull()) {
	QDomDocument doc = e.ownerDocument();
	e.namedItem("Properties").appendChild(doc.createElement(property));
 }
}

void BoUfoDesignerMain::slotLoad()
{
 QString fileName = QFileDialog::getOpenFileName(QString::null, tr("boui files (*.boui)"), this, "open dialog");
 if (fileName.isEmpty()) {
	return;
 }
 if (!slotLoadFromFile(fileName)) {
	QMessageBox::critical(this, tr("Could not load"), tr("Unable to load from file %1").arg(fileName));
 }
}

void BoUfoDesignerMain::slotSaveAs()
{
 QString fileName = QFileDialog::getSaveFileName(QString::null, tr("boui files (*.boui)"), this, "save dialog");
 if (fileName.isEmpty()) {
	return;
 }

 if (fileName.find('.') < 0) {
	fileName.append(".boui");
 }

 if (!slotSaveAsFile(fileName)) {
	QMessageBox::critical(this, tr("Could not save"), tr("Unable to save to file %1").arg(fileName));
 }
}

void BoUfoDesignerMain::slotEditSignalsSlots()
{
 BoSignalsSlotsEditor editor(this);
 QDomElement root = mDocument.documentElement();
 if (!editor.load(root)) {
	boError() << k_funcinfo << "Error loading methods" << endl;
 }
 int ret = editor.exec();
 if (ret == QDialog::Accepted) {
	if (!editor.save(root)) {
		boError() << k_funcinfo << "Error saving methods" << endl;
	}
 }
}

void BoUfoDesignerMain::initActions()
{
 QAction* fileNew = new QAction(tr("New"), CTRL + Key_N, this, "new");
 QAction* fileOpen = new QAction(tr("Open..."), CTRL + Key_O, this, "open");
 QAction* fileSaveAs = new QAction(tr("Save as..."), 0, this, "saveas");
 connect(fileNew, SIGNAL(activated()), this, SLOT(slotCreateNew()));
 connect(fileOpen, SIGNAL(activated()), this, SLOT(slotLoad()));
 connect(fileSaveAs, SIGNAL(activated()), this, SLOT(slotSaveAs()));

 QAction* editSignalsSlots = new QAction(tr("Edit &Signals and Slots..."), 0, this, "editsignalsslots");
 connect(editSignalsSlots, SIGNAL(activated()), this, SLOT(slotEditSignalsSlots()));

 QAction* debugUfo = new QAction(tr("Debug Ufo..."), 0, this, "debug_ufo");
 connect(debugUfo, SIGNAL(activated()), this, SLOT(slotDebugUfo()));

 QAction* configure = new QAction(tr("&Configure..."), 0, this, "configure");
 connect(configure, SIGNAL(activated()), this, SLOT(slotConfigure()));

 QPopupMenu* file = new QPopupMenu(this);
 menuBar()->insertItem("&File", file);
 fileNew->addTo(file);
 fileOpen->addTo(file);
 fileSaveAs->addTo(file);
 QPopupMenu* edit = new QPopupMenu(this);
 menuBar()->insertItem("&Edit", edit);
 editSignalsSlots->addTo(edit);
 QPopupMenu* debug = new QPopupMenu(this);
 menuBar()->insertItem("&Debug", debug);
 debugUfo->addTo(debug);
 QPopupMenu* settingsMenu = new QPopupMenu(this);
 menuBar()->insertItem("&Settings", settingsMenu);
 configure->addTo(settingsMenu);
}

void BoUfoDesignerMain::closeEvent(QCloseEvent* e)
{
 int ret = QMessageBox::question(this, tr("Quit"), tr("Do you want to quit without saving?"), QMessageBox::Yes, QMessageBox::No);
 switch (ret) {
	case QMessageBox::Yes:
		e->accept();
		break;
	default:
	case QMessageBox::No:
		e->ignore();
		break;
 }
}

void BoUfoDesignerMain::slotDebugUfo()
{
 QDialog* dialog = new QDialog(0,"ufodebugdialog", false, Qt::WDestructiveClose);
 QVBoxLayout* l = new QVBoxLayout(dialog);
 BoUfoDebugWidget* debug = new BoUfoDebugWidget(dialog);
 debug->setBoUfoManager(mPreview->ufoManager());
 l->addWidget(debug);
 dialog->show();
}

void BoUfoDesignerMain::slotConfigure()
{
 if (mOptionsDialog) {
	mOptionsDialog->show();
	return;
 }
 mOptionsDialog = new OptionsDialog(this);
 connect(mOptionsDialog, SIGNAL(signalApplyOptions()),
		this, SLOT(slotApplyOptions()));
 mOptionsDialog->show();
}

void BoUfoDesignerMain::slotApplyOptions()
{
 BO_CHECK_NULL_RET(mPreview);
 mPreview->updateGL(); // enfore call to initializeGL()
 BO_CHECK_NULL_RET(mPreview->ufoManager());
 QSettings settings;
 settings.setPath("boson.eu.org", "boufodesigner");
 mPreview->ufoManager()->setDataDir(settings.readEntry("/ufo/data_dir"));
}

