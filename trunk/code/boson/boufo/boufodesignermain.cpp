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
#include "boufo.h"
#include <bogl.h>

#include "boufodesignermain.h"
#include "boufodesignermain.moc"

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
 return false;
}


BoConnection::BoConnection(bool isSender, QWidget* parent)
	: QWidget(parent)
{
 QHBoxLayout* layout = new QHBoxLayout(this);
 QVBoxLayout* connectionLayout = new QVBoxLayout(layout);
 QWidget* sender = new QWidget(this);
 QWidget* receiver = new QWidget(this);
 connectionLayout->addWidget(sender);
 connectionLayout->addWidget(receiver);

 QHBoxLayout* senderLayout = new QHBoxLayout(sender);
 QLabel* senderLabel = new QLabel(tr("Sender: "), sender);
 mSender = new QLineEdit(sender);
 QLabel* signalLabel = new QLabel(tr("Signal: "), sender);
 mSignal = new QLineEdit(sender);
 senderLayout->addWidget(senderLabel);
 senderLayout->addWidget(mSender);
 senderLayout->addWidget(signalLabel);
 senderLayout->addWidget(mSignal);

 QHBoxLayout* receiverLayout = new QHBoxLayout(receiver);
 QLabel* receiverLabel = new QLabel(tr("Receiver: "), receiver);
 mReceiver = new QLineEdit(receiver);
 QLabel* slotLabel = new QLabel(tr("Slot: "), receiver);
 mSlot = new QLineEdit(receiver);
 receiverLayout->addWidget(receiverLabel);
 receiverLayout->addWidget(mReceiver);
 receiverLayout->addWidget(slotLabel);
 receiverLayout->addWidget(mSlot);

 mIsSender = isSender;
 if (mIsSender) {
	sender->setEnabled(false);
	mSender->setText("this");
 } else {
	receiver->setEnabled(false);
	mReceiver->setText("this");
 }

 QPushButton* remove = new QPushButton(tr("Remove"), this);
 connect(remove, SIGNAL(clicked()), this, SLOT(slotRemove()));
 layout->addWidget(remove);
}

BoConnection::~BoConnection()
{
}

void BoConnection::setMethodName(const QString& name)
{
 if (mIsSender) {
	mSignal->setText(name);
 } else {
	mSlot->setText(name);
 }
}

void BoConnection::slotRemove()
{
 emit signalRemoved(this);
 delete this;
}

void BoConnection::setSenderText(const QString& t)
{
 mSender->setText(t);
}

void BoConnection::setSignalText(const QString& t)
{
 mSignal->setText(t);
}

void BoConnection::setReceiverText(const QString& t)
{
 mReceiver->setText(t);
}

void BoConnection::setSlotText(const QString& t)
{
 mSlot->setText(t);
}

QString BoConnection::senderText() const
{
 return mSender->text();
}

QString BoConnection::signalText() const
{
 return mSignal->text();
}

QString BoConnection::receiverText() const
{
 return mReceiver->text();
}

QString BoConnection::slotText() const
{
 return mSlot->text();
}


BoConnectionsContainer::BoConnectionsContainer(QWidget* parent)
	: QWidget(parent)
{
 QVBoxLayout* layout = new QVBoxLayout(this);
 mConnectionsLayout = new QVBoxLayout(layout);

 mAddConnection = new QWidgetStack(this);
 layout->addWidget(mAddConnection);

 mAddConnectionSignal = new QWidget(mAddConnection);
 mAddConnection->addWidget(mAddConnectionSignal);
 QHBoxLayout* signalLayout = new QHBoxLayout(mAddConnectionSignal);
 QPushButton* addReceiver = new QPushButton(tr("Add Receiver"), mAddConnectionSignal);
 QPushButton* addTrigger = new QPushButton(tr("Add Trigger"), mAddConnectionSignal);
 signalLayout->addWidget(addReceiver);
 signalLayout->addWidget(addTrigger);
 connect(addReceiver, SIGNAL(clicked()), this, SLOT(slotAddReceiver()));
 connect(addTrigger, SIGNAL(clicked()), this, SLOT(slotAddTrigger()));

 mAddConnectionSlot = new QWidget(mAddConnection);
 mAddConnection->addWidget(mAddConnectionSlot);
 QHBoxLayout* slotLayout = new QHBoxLayout(mAddConnectionSlot);
 QPushButton* addSlotTrigger = new QPushButton(tr("Add Trigger"), mAddConnectionSlot);
 slotLayout->addWidget(addSlotTrigger);
 connect(addSlotTrigger, SIGNAL(clicked()), this, SLOT(slotAddTrigger()));

 mIsSignal = false;
 mAddConnection->raiseWidget(mAddConnectionSlot);
}

BoConnectionsContainer::~BoConnectionsContainer()
{
}

void BoConnectionsContainer::setMethodName(const QString& name)
{
 mMethodName = name;
 QPtrListIterator<BoConnection> it(mConnections);
 while (it.current()) {
	it.current()->setMethodName(mMethodName);
	++it;
 }
}

// add a connection that _receives_ this signal
void BoConnectionsContainer::slotAddReceiver()
{
 // Only signals can have a receiver, so this cannot be called if the method is
 // a slot.
 if (!mIsSignal) {
	boWarning() << k_funcinfo << " called although method is a slot" << endl;
	return;
 }
 addConnection(true);
}

// add a connection that _triggers_ this method
void BoConnectionsContainer::slotAddTrigger()
{
 // A signal can trigger both, signal and slot, so this can be called when the
 // method is a signal as well as when it is a slot.
 addConnection(false);
}

BoConnection* BoConnectionsContainer::addConnection(bool isSender)
{
 BoConnection* connection = new BoConnection(isSender, this);
 connection->setMethodName(mMethodName);
 connection->show();
 mConnections.append(connection);
 mConnectionsLayout->addWidget(connection);
 connect(connection, SIGNAL(signalRemoved(BoConnection*)),
		this, SLOT(slotConnectionRemoved(BoConnection*)));
 return connection;
}

unsigned int BoConnectionsContainer::connectionsCount() const
{
 return mConnections.count();
}

void BoConnectionsContainer::slotConnectionRemoved(BoConnection* connection)
{
 if (!connection) {
	return;
 }
 mConnections.removeRef(connection);
}

void BoConnectionsContainer::clearConnections()
{
 while (!mConnections.isEmpty()) {
	mConnections.first()->slotRemove();
 }
}

void BoConnectionsContainer::setSignal(bool isSignal)
{
 setEnabled(true);
 if (isSignal != mIsSignal) {
	clearConnections();
 }
 mIsSignal = isSignal;
 if (mIsSignal) {
	mAddConnection->raiseWidget(mAddConnectionSignal);
 } else {
	mAddConnection->raiseWidget(mAddConnectionSlot);
 }
}

bool BoConnectionsContainer::save(QDomElement& root) const
{
 QDomDocument doc = root.ownerDocument();
 QPtrListIterator<BoConnection> it(mConnections);
 while (it.current()) {
	BoConnection* c = it.current();
	QDomElement connection = doc.createElement("Connection");
	if (c->isSender()) {
		connection.setAttribute("type", "sender");
		connection.setAttribute("receiver", c->receiverText());
		connection.setAttribute("slot", c->slotText());
	} else {
		connection.setAttribute("type", "receiver");
		connection.setAttribute("sender", c->senderText());
		connection.setAttribute("signal", c->signalText());
	}
	root.appendChild(connection);
	++it;
 }

 return true;
}

bool BoConnectionsContainer::load(const QDomElement& root)
{
 clearConnections();

 QDomNode n;
 for (n = root.firstChild(); !n.isNull(); n = n.nextSibling()) {
	QDomElement e = n.toElement();
	if (e.isNull()) {
		continue;
	}
	if (e.tagName() != "Connection") {
		continue;
	}
	QString type = e.attribute("type");
	if (type != "sender" && type != "receiver") {
		boError() << k_funcinfo << "invalid type for connection" << endl;
		continue;
	}
	BoConnection* c = addConnection(type == "sender");
	if (type == "sender") {
		c->setReceiverText(e.attribute("receiver"));
		c->setSlotText(e.attribute("slot"));
	} else {
		c->setSenderText(e.attribute("sender"));
		c->setSignalText(e.attribute("signal"));
	}
 }

 return true;
}


BoSignalsSlotsEditor::BoSignalsSlotsEditor(QWidget* parent)
	: QDialog(parent)
{
 QVBoxLayout* layout = new QVBoxLayout(this, 5, 5);
 mMethods = new QListView(this);
 layout->addWidget(mMethods);
 mMethodReturnIndex = mMethods->addColumn(tr("Return type"));
 mMethodNameIndex = mMethods->addColumn(tr("Method"));
 mMethodTypeIndex = mMethods->addColumn(tr("Type")); // signal/slot/method
 mMethodVisibilityIndex = mMethods->addColumn(tr("Visibility")); // public/protected/private
 connect(mMethods, SIGNAL(currentChanged(QListViewItem*)),
		this, SLOT(slotCurrentMethodChanged(QListViewItem*)));

 QWidget* w = new QWidget(this);
 layout->addWidget(w);
 QHBoxLayout* hboxLayout = new QHBoxLayout(w);
 QPushButton* add = new QPushButton(tr("Add method"), w);
 QPushButton* del = new QPushButton(tr("Delete method"), w);
 hboxLayout->addWidget(add);
 hboxLayout->addWidget(del);
 connect(add, SIGNAL(clicked()), this, SLOT(slotAddMethod()));
 connect(del, SIGNAL(clicked()), this, SLOT(slotDeleteMethod()));

 QVGroupBox* editBox = new QVGroupBox(tr("Edit Method"), this);
 layout->addWidget(editBox);
 QWidget* editWidget = new QWidget(editBox);
 QVBoxLayout* editLayout = new QVBoxLayout(editWidget);
 QWidget* method = new QWidget(editWidget);
 editLayout->addWidget(method);
 QHBoxLayout* methodLayout = new QHBoxLayout(method);
 QLabel* methodReturnLabel = new QLabel(tr("Return type: "), method);
 mMethodReturnType = new QLineEdit(method);
 QLabel* methodNameLabel = new QLabel(tr("Name: "), method);
 mMethodName = new QLineEdit(method);
 methodLayout->addWidget(methodReturnLabel);
 methodLayout->addWidget(mMethodReturnType);
 methodLayout->addWidget(methodNameLabel);
 methodLayout->addWidget(mMethodName);
 connect(mMethodReturnType, SIGNAL(textChanged(const QString&)),
		this, SLOT(slotMethodReturnTypeChanged(const QString&)));
 connect(mMethodName, SIGNAL(textChanged(const QString&)),
		this, SLOT(slotMethodNameChanged(const QString&)));
 QWidget* methodProperties = new QWidget(editWidget);
 editLayout->addWidget(methodProperties);
 QHBoxLayout* methodPropertiesLayout = new QHBoxLayout(methodProperties);
 QLabel* methodTypeLabel = new QLabel(tr("Type: "), methodProperties);
 mMethodType = new QComboBox(methodProperties);
 QLabel* methodVisibilityLabel = new QLabel(tr("Visibility: "), methodProperties);
 mMethodVisibility = new QComboBox(methodProperties);
 methodPropertiesLayout->addWidget(methodTypeLabel);
 methodPropertiesLayout->addWidget(mMethodType);
 methodPropertiesLayout->addWidget(methodVisibilityLabel);
 methodPropertiesLayout->addWidget(mMethodVisibility);
 mMethodType->insertItem(tr("slot"));
 mMethodType->insertItem(tr("signal"));
 mMethodType->insertItem(tr("method"));
 mMethodVisibility->insertItem(tr("public"));
 mMethodVisibility->insertItem(tr("protected"));
 mMethodVisibility->insertItem(tr("private"));
 connect(mMethodType, SIGNAL(activated(int)),
		this, SLOT(slotMethodTypeChanged(int)));
 connect(mMethodVisibility, SIGNAL(activated(int)),
		this, SLOT(slotMethodVisibilityChanged(int)));

 QVGroupBox* connectionsBox = new QVGroupBox(tr("Connections"), this);
 layout->addWidget(connectionsBox);
 mConnections = new QWidgetStack(connectionsBox);


 w = new QWidget(this);
 layout->addWidget(w);
 hboxLayout = new QHBoxLayout(w);
 QPushButton* ok = new QPushButton(tr("Ok"), w);
 QPushButton* cancel = new QPushButton(tr("Cancel"), w);
 hboxLayout->addWidget(ok);
 hboxLayout->addWidget(cancel);
 connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
 connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));

 slotCurrentMethodChanged(0);
}

BoSignalsSlotsEditor::~BoSignalsSlotsEditor()
{
}

bool BoSignalsSlotsEditor::load(const QDomElement& root)
{
 QDomElement methods = root.namedItem("Methods").toElement();
 if (!loadMethods(methods)) {
	boError() << k_funcinfo << "could not load methods" << endl;
	return false;
 }

 slotCurrentMethodChanged(0);
 return true;
}

bool BoSignalsSlotsEditor::loadMethods(const QDomElement& root)
{
 if (root.isNull()) {
	// nothing to load
	boDebug() << k_funcinfo << "no methods" << endl;
	return true;
 }
 mMethods->clear();

 QDomNode n;
 for (n = root.firstChild(); !n.isNull(); n = n.nextSibling()) {
	QDomElement e = n.toElement();
	if (e.isNull()) {
		continue;
	}
	if (e.tagName() != "Method") {
		continue;
	}
	QString ret = e.attribute("return");
	QString name = e.attribute("name");
	QString type = e.attribute("type");
	QString visibility = e.attribute("visibility");
	if (ret.isEmpty()) {
		boError() << k_funcinfo << "empty return type" << endl;
		continue;
	}
	if (name.isEmpty()) {
		boError() << k_funcinfo << "empty method name" << endl;
		continue;
	}
	if (!isValidType(type)) {
		boError() << k_funcinfo << "invalid type" << type << endl;
		continue;
	}
	if (!isValidVisibility(visibility)) {
		boError() << k_funcinfo << "invalid type" << type << endl;
		continue;
	}

	QListViewItem* item = slotAddMethod();
	item->setText(mMethodReturnIndex, ret);
	item->setText(mMethodNameIndex, name);
	item->setText(mMethodTypeIndex, type);
	item->setText(mMethodVisibilityIndex, visibility);

	BoConnectionsContainer* container = mMethod2Connections[item];
	if (!container) {
		continue;
	}
	container->setMethodName(name);
	QDomElement connections = e.namedItem("Connections").toElement();
	if (connections.isNull()) {
		continue;
	}
	if (!container->load(connections)) {
		boError() << k_funcinfo << "Unable to load connections for method " << name << endl;
		return false;
	}
 }

 // make sure that we change the current item at least once after loading all
 // items, so that input widgets get updated
 mMethods->setCurrentItem(mMethods->firstChild());

 return true;
}

bool BoSignalsSlotsEditor::save(QDomElement& root) const
{
 QDomDocument doc = root.ownerDocument();
 QDomElement methods = doc.createElement("Methods");

 if (!saveMethods(methods)) {
	boError() << k_funcinfo << "Could not save methods" << endl;
	return false;
 }

 QDomElement origMethods = root.namedItem("Methods").toElement();
 if (!origMethods.isNull()) {
	root.removeChild(origMethods);
 }
 root.appendChild(methods);

 return true;
}

bool BoSignalsSlotsEditor::saveMethods(QDomElement& root) const
{
 QDomDocument doc = root.ownerDocument();
 QListViewItem* method = mMethods->firstChild();
 for (; method; method = method->nextSibling()) {
	QDomElement m = doc.createElement("Method");
	QString ret = method->text(mMethodReturnIndex);
	QString name = method->text(mMethodNameIndex);
	QString type = method->text(mMethodTypeIndex);
	QString visibility = method->text(mMethodVisibilityIndex);
	if (ret.isEmpty()) {
		boError() << k_funcinfo << "empty return type" << endl;
		continue;
	}
	if (name.isEmpty()) {
		boError() << k_funcinfo << "empty method name" << endl;
		continue;
	}
	if (!isValidType(type)) {
		boError() << k_funcinfo << "invalid type" << type << endl;
		continue;
	}
	if (!isValidVisibility(visibility)) {
		boError() << k_funcinfo << "invalid type" << type << endl;
		continue;
	}
	m.setAttribute("return", ret);
	m.setAttribute("name", name);
	m.setAttribute("type", type);
	m.setAttribute("visibility", visibility);
	root.appendChild(m);

	BoConnectionsContainer* container = mMethod2Connections[method];
	if (!container) {
		continue;
	}
	QDomElement connections = doc.createElement("Connections");
	if (!container->save(connections)) {
		boError() << k_funcinfo << "Unable to save connections for method " << name << endl;
		continue;
	}
	m.appendChild(connections);
 }

 return true;
}

bool BoSignalsSlotsEditor::isValidType(const QString& type) const
{
 if (type == "signal") {
	return true;
 }
 if (type == "slot") {
	return true;
 }
 if (type == "method") {
	return true;
 }
 return false;
}

bool BoSignalsSlotsEditor::isValidVisibility(const QString& v) const
{
 if (v == "public") {
	return true;
 }
 if (v == "protected") {
	return true;
 }
 if (v == "private") {
	return true;
 }
 return false;
}


void BoSignalsSlotsEditor::slotCurrentMethodChanged(QListViewItem* item)
{
 mMethodName->setEnabled(item != 0);
 mMethodReturnType->setEnabled(item != 0);
 mMethodType->setEnabled(item != 0);
 mMethodVisibility->setEnabled(item != 0);
 if (!item) {
	return;
 }
 mMethodName->setText(item->text(mMethodNameIndex));
 mMethodReturnType->setText(item->text(mMethodReturnIndex));
 QString type = item->text(mMethodTypeIndex);
 for (int i = 0; i < mMethodType->count(); i++) {
	if (mMethodType->text(i) == type) {
		mMethodType->setCurrentItem(i);
	}
 }
 QString visibility = item->text(mMethodVisibilityIndex);
 for (int i = 0; i < mMethodVisibility->count(); i++) {
	if (mMethodVisibility->text(i) == visibility) {
		mMethodVisibility->setCurrentItem(i);
	}
 }
 BoConnectionsContainer* connection = mMethod2Connections[item];
 if (!connection) {
	boError() << k_funcinfo << "NULL connection" << endl;
	return;
 }
 mConnections->raiseWidget(connection);
}

QListViewItem* BoSignalsSlotsEditor::slotAddMethod()
{
 QListViewItem* item = new QListViewItem(mMethods);
 item->setText(mMethodReturnIndex, "void");
 item->setText(mMethodNameIndex, "randomName()"); // TODO: use a really random name
 item->setText(mMethodTypeIndex, "slot");
 item->setText(mMethodVisibilityIndex, "public");

 BoConnectionsContainer* connection = new BoConnectionsContainer(mConnections);
 connection->show();
 mMethod2Connections.insert(item, connection);
 mConnections->addWidget(connection);

 if (mMethods->childCount() == 1) {
	// by some reason currentChanged() is not emitted for the very first
	// item
	slotCurrentMethodChanged(mMethods->currentItem());
 }

 return item;
}

void BoSignalsSlotsEditor::slotDeleteMethod()
{
 QListViewItem* item = mMethods->currentItem();
 if (!item) {
	return;
 }
 delete item;
}

void BoSignalsSlotsEditor::slotMethodReturnTypeChanged(const QString& type)
{
 QListViewItem* item = mMethods->currentItem();
 if (!item) {
	return;
 }
 item->setText(mMethodReturnIndex, type);
}

void BoSignalsSlotsEditor::slotMethodNameChanged(const QString& name)
{
 QListViewItem* item = mMethods->currentItem();
 if (!item) {
	return;
 }
 item->setText(mMethodNameIndex, name);
 BoConnectionsContainer* connections = mMethod2Connections[item];
 if (!connections) {
	boError() << k_funcinfo << "NULL connections" << endl;
	return;
 }
 connections->setMethodName(name);
}

void BoSignalsSlotsEditor::slotMethodTypeChanged(int index)
{
 switch (index) {
	case 0:
		slotMethodTypeChanged("slot");
		break;
	case 1:
		slotMethodTypeChanged("signal");
		break;
	case 2:
		slotMethodTypeChanged("method");
		break;
	default:
		return;
 }
}

void BoSignalsSlotsEditor::slotMethodTypeChanged(const QString& type)
{
 QListViewItem* item = mMethods->currentItem();
 if (!item) {
	return;
 }
 QString oldType = item->text(mMethodTypeIndex);
 BoConnectionsContainer* connections = mMethod2Connections[item];
 if (!connections) {
	boError() << k_funcinfo << "NULL connections" << endl;
	return;
 }
 if (connections->connectionsCount() > 0 && oldType != type) {
	int ret = QMessageBox::question(this, tr("Method Type Changed"),
			tr("When you change the type of the method all connections will be lost. Change anyway?"),
			QMessageBox::Yes, QMessageBox::No);
	if (ret != QMessageBox::Yes) {
		return;
	}
	connections->clearConnections();
 }
 if (type == "signal") {
	connections->setSignal(true);
 } else if (type == "slot") {
	connections->setSignal(false);
 } else {
	connections->setEnabled(false);
 }
 item->setText(mMethodTypeIndex, type);
}

void BoSignalsSlotsEditor::slotMethodVisibilityChanged(int index)
{
 switch (index) {
	case 0:
		slotMethodVisibilityChanged("public");
		break;
	case 1:
		slotMethodVisibilityChanged("protected");
		break;
	case 2:
		slotMethodVisibilityChanged("private");
		break;
	default:
		return;
 }
}

void BoSignalsSlotsEditor::slotMethodVisibilityChanged(const QString& v)
{
 QListViewItem* item = mMethods->currentItem();
 if (!item) {
	return;
 }
 item->setText(mMethodVisibilityIndex, v);
}



FormPreview::FormPreview(QWidget* parent) : QGLWidget(parent)
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
 widget = mUfoManager->contentWidget()->widget()->getWidgetAt(x, y);
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
 QDomElement e = mUfoWidget2Element[w->widget()];
 QString className;
 if (!e.isNull()) {
	className = e.namedItem("ClassName").toElement().text();
 }
 while (w && !isContainerWidget(className)) {
	ufo::UWidget* widget = w->widget();
	w = 0;
	while (!w && widget) {
		widget = widget->getParent();
		w = mUfoWidget2Widget[widget];
	}
	if (!w) {
		return 0;
	}
	e = mUfoWidget2Element[w->widget()];
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
 mUfoManager->contentWidget()->widget()->removeAll();
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
	BoUfoWidget* widget = BoUfoFactory::createWidget(className);
	if (!widget) {
		boError() << k_funcinfo << "could not create widget with ClassName " << className << endl;
		continue;
	}
	parent->addWidget(widget);
	addWidget(widget, e);

	widget->loadPropertiesFromXML(e.namedItem("Properties").toElement());

	updateGUI(e, widget);
 }
}

void FormPreview::addWidget(BoUfoWidget* widget, const QDomElement& e)
{
 BO_CHECK_NULL_RET(widget);
 BO_CHECK_NULL_RET(widget->widget());
 if (e.isNull()) {
	boError() << k_funcinfo << "NULL element" << endl;
	return;
 }
 mUfoWidget2Element.insert(widget->widget(), e);
 mUfoWidget2Widget.insert(widget->widget(), widget);
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
	mUfoManager->sendResizeEvent(w, h);
 }
}

void FormPreview::paintGL()
{
 if (mUfoManager) {
	mUfoManager->dispatchEvents();
	mUfoManager->render();
 }
}

bool FormPreview::eventFilter(QObject* o, QEvent* e)
{
 return QObject::eventFilter(o, e);
}

void FormPreview::mouseMoveEvent(QMouseEvent* e)
{
 if (mUfoManager) {
	mUfoManager->sendMouseMoveEvent(e);
 }
}

void FormPreview::mousePressEvent(QMouseEvent* e)
{
 Q_UNUSED(e);
#if 0
 // AB: we display the ufo widgets only, we don't use them. so don't deliver
 // this event.
 if (mUfoManager) {
	mUfoManager->sendMousePressEvent(e);
 }
#endif

 if (mPlacementMode) {
	// we need a _container_ widget. it doesn't make sense to add children
	// to a button or so.
	QPoint pos = mapFromGlobal(QCursor::pos());
	BoUfoWidget* w = getContainerWidgetAt(pos.x(), pos.y());
	QDomElement parent;
	if (w) {
		parent = mUfoWidget2Element[w->widget()];
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
	mUfoManager->sendMouseReleaseEvent(e);
 }
#endif
}

void FormPreview::wheelEvent(QWheelEvent* e)
{
 if (mUfoManager) {
	mUfoManager->sendWheelEvent(e);
 }
}

void FormPreview::keyPressEvent(QKeyEvent* e)
{
 if (mUfoManager) {
	mUfoManager->sendKeyPressEvent(e);
 }
 QGLWidget::keyPressEvent(e);
}

void FormPreview::keyReleaseEvent(QKeyEvent* e)
{
 if (mUfoManager) {
	mUfoManager->sendKeyReleaseEvent(e);
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
 if (widget && widget->widget()) {
	widgetUnderCursor = mUfoWidget2Element[widget->widget()];
 }

 emit signalSelectWidget(widgetUnderCursor);
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

 QHBoxLayout* layout = new QHBoxLayout(topWidget);

 QSplitter* splitter = new QSplitter(topWidget);
 layout->addWidget(splitter);

 mWidgets = new BoWidgetList(splitter);
 splitter->setResizeMode(mWidgets, QSplitter::KeepSize);
 connect(mWidgets, SIGNAL(signalWidgetSelected(const QString&)),
		this, SLOT(slotWidgetClassSelected(const QString&)));
 mWidgets->show();

 mPreview = new FormPreview(splitter);
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
// mPreview->selectWidget(widget); // paint a "rect" around the widget
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

 QPopupMenu* file = new QPopupMenu(this);
 menuBar()->insertItem("&File", file);
 fileNew->addTo(file);
 fileOpen->addTo(file);
 fileSaveAs->addTo(file);
 QPopupMenu* edit = new QPopupMenu(this);
 menuBar()->insertItem("&Edit", edit);
 editSignalsSlots->addTo(edit);
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

int main(int argc, char **argv)
{
 std::cout << "resolving GL, GLX and GLU symbols" << std::endl;
 if (!boglResolveGLSymbols()) {
#warning TODO: messagebox
	// TODO: open a messagebox
	std::cerr << "Could not resolve all symbols!" << std::endl;
    return 1;
 }
 std::cout << "GL, GLX and GLU symbols successfully resolved" << std::endl;


 QApplication app(argc, argv);
 QObject::connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));

 BoUfoDesignerMain* main = new BoUfoDesignerMain();
 app.setMainWidget(main);
 main->show();

 QString file;
 for (int i = 0; i < app.argc(); i++) {
	QString a = app.argv()[i];
	if (a == "--file") {
		if (i + 1 >= app.argc()) {
			boError() << "--file expects a filename" << endl;
			return 1;
		}
		file = app.argv()[i + 1];
	}
 }

 if (file.isEmpty()) {
	if (!main->slotCreateNew()) {
		boError() << k_funcinfo << "could not create new document" << endl;
	}
 } else {
	if (!main->slotLoadFromFile(file)) {
		boError() << k_funcinfo << "could not load " << file << endl;
	}
 }

 return app.exec();
}

