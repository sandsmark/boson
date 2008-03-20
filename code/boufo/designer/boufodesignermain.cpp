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

#include <ufo/ufo.hpp>
#include <ufo/ux/ux.hpp>
#include "../boufo.h"
#include <bogl.h>

#include "boufodesignermain.h"
#include "boufodesignermain.moc"

#include "boufodebugwidget.h"
#include "bosignalsslotseditor.h"
#include "optionsdialog.h"
#include "formpreview.h"
#include "bowidgetlist.h"
#include "bowidgettree.h"
#include "bopropertieswidget.h"

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

class BoUfoDesignerMainPrivate
{
public:
	BoUfoDesignerMainPrivate()
	{
		mSaveAction = 0;
	}
	QAction* mSaveAction;
	QString mCurrentFile;
};


BoUfoDesignerMain::BoUfoDesignerMain()
	: QMainWindow(0, "mainwindow", WType_TopLevel | WDestructiveClose)
{
 d = new BoUfoDesignerMainPrivate();
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

 QSettings settings;
 settings.setPath("boson.eu.org", "boufodesigner");
 settings.writeEntry("/boufodesigner/MostRecentFile", d->mCurrentFile);

 delete mIface;
 delete d;
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

 d->mCurrentFile = QString::null;

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

 d->mCurrentFile = fileName;
 d->mSaveAction->setEnabled(true);

 return slotLoadFromXML(b, false);
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

 d->mCurrentFile = fileName;
 d->mSaveAction->setEnabled(true);

 return true;
}

bool BoUfoDesignerMain::slotLoadFromXML(const QByteArray& xml, bool resetFileName)
{
 if (resetFileName) {
	d->mCurrentFile = QString::null;
	d->mSaveAction->setEnabled(false);
 }
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

void BoUfoDesignerMain::slotSave()
{
 if (d->mCurrentFile.isEmpty()) {
	slotSaveAs();
	return;
 }
 if (!slotSaveAsFile(d->mCurrentFile)) {
	QMessageBox::critical(this, tr("Could not save"), tr("Unable to save to file %1").arg(d->mCurrentFile));
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
 d->mSaveAction = new QAction(tr("Save"), 0, this, "save");
 connect(fileNew, SIGNAL(activated()), this, SLOT(slotCreateNew()));
 connect(fileOpen, SIGNAL(activated()), this, SLOT(slotLoad()));
 connect(d->mSaveAction, SIGNAL(activated()), this, SLOT(slotSave()));
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
 d->mSaveAction->addTo(file);
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
 mPreview->ufoManager()->setDataDir(settings.readEntry("/boufodesigner/data_dir"));
}

