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

#include <bogl.h>

#include "bosignalsslotseditor.h"
#include "bosignalsslotseditor.moc"

#include <bodebug.h>

#include <qlayout.h>
#include <qlistview.h>
#include <qdom.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qvgroupbox.h>
#include <qwidgetstack.h>

#include <math.h>
#include <stdlib.h>


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


