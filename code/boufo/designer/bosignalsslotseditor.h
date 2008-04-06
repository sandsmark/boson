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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef BOSIGNALSSLOTSEDITOR_H
#define BOSIGNALSSLOTSEDITOR_H

#include <qdialog.h>
#include <qmap.h>
#include <q3ptrlist.h>
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <QLabel>

class Q3ListBox;
class Q3ListBoxItem;
class Q3ListView;
class Q3ListViewItem;
class QLabel;
class QPushButton;
class QComboBox;
class QLineEdit;
class Q3WidgetStack;
class Q3VBoxLayout;
class QDomElement;
class BoUfoWidget;

class BoConnection : public QWidget
{
	Q_OBJECT
public:
	BoConnection(bool isSender, QWidget* parent);
	~BoConnection();

	void setMethodName(const QString&);

	QString senderText() const;
	QString signalText() const;
	QString receiverText() const;
	QString slotText() const;
	bool isSender() const { return mIsSender; }
	void setSenderText(const QString&);
	void setSignalText(const QString&);
	void setReceiverText(const QString&);
	void setSlotText(const QString&);

signals:
	void signalRemoved(BoConnection* me);

public slots:
	void slotRemove();

private:
	bool mIsSender;
	QLineEdit* mSender;
	QLineEdit* mSignal;
	QLineEdit* mReceiver;
	QLineEdit* mSlot;
};

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoConnectionsContainer : public QWidget
{
	Q_OBJECT
public:
	BoConnectionsContainer(QWidget* parent);
	~BoConnectionsContainer();

	void setMethodName(const QString&);
	void setSignal(bool isSignal);

	unsigned int connectionsCount() const;
	void clearConnections();

	bool save(QDomElement& root) const;
	bool load(const QDomElement& root);

protected slots:
	void slotConnectionRemoved(BoConnection* connection);
	void slotAddReceiver();
	void slotAddTrigger();

protected:
	/**
	 * @param isSignal Specifies whether the currently selected method is
	 * the sender (TRUE) or the receiver (FALSE) of the connection.
	 **/
	BoConnection* addConnection(bool isSender);

private:
	Q3VBoxLayout* mConnectionsLayout;
	Q3PtrList<BoConnection> mConnections;
	bool mIsSignal;
	QString mMethodName;

	Q3WidgetStack* mAddConnection;
	QWidget* mAddConnectionSignal;
	QWidget* mAddConnectionSlot;
};

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoSignalsSlotsEditor : public QDialog
{
	Q_OBJECT
public:
	BoSignalsSlotsEditor(QWidget* parent = 0);
	~BoSignalsSlotsEditor();

	bool save(QDomElement& root) const;
	bool load(const QDomElement& root);

protected:
	bool saveMethods(QDomElement& root) const;
	bool loadMethods(const QDomElement& root);

	bool isValidType(const QString& type) const;
	bool isValidVisibility(const QString& type) const;

protected slots:
	Q3ListViewItem* slotAddMethod();
	void slotDeleteMethod();

	void slotMethodReturnTypeChanged(const QString&);
	void slotMethodNameChanged(const QString&);
	void slotMethodTypeChanged(const QString&);
	void slotMethodTypeChanged(int);
	void slotMethodVisibilityChanged(int);
	void slotMethodVisibilityChanged(const QString&);

	void slotCurrentMethodChanged(Q3ListViewItem*);

private:
	Q3ListView* mMethods;
	int mMethodReturnIndex;
	int mMethodNameIndex;
	int mMethodTypeIndex;
	int mMethodVisibilityIndex;

	QLineEdit* mMethodReturnType;
	QLineEdit* mMethodName;
	QComboBox* mMethodType;
	QComboBox* mMethodVisibility;

	Q3WidgetStack* mConnections;
	QMap<Q3ListViewItem*, BoConnectionsContainer*> mMethod2Connections;
};

#endif

