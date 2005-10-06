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
#ifndef BOSIGNALSSLOTSEDITOR_H
#define BOSIGNALSSLOTSEDITOR_H

#include <qdialog.h>
#include <qmap.h>
#include <qptrlist.h>

class QListBox;
class QListBoxItem;
class QListView;
class QListViewItem;
class QLabel;
class QPushButton;
class QComboBox;
class QLineEdit;
class QWidgetStack;
class QVBoxLayout;
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
	QVBoxLayout* mConnectionsLayout;
	QPtrList<BoConnection> mConnections;
	bool mIsSignal;
	QString mMethodName;

	QWidgetStack* mAddConnection;
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
	QListViewItem* slotAddMethod();
	void slotDeleteMethod();

	void slotMethodReturnTypeChanged(const QString&);
	void slotMethodNameChanged(const QString&);
	void slotMethodTypeChanged(const QString&);
	void slotMethodTypeChanged(int);
	void slotMethodVisibilityChanged(int);
	void slotMethodVisibilityChanged(const QString&);

	void slotCurrentMethodChanged(QListViewItem*);

private:
	QListView* mMethods;
	int mMethodReturnIndex;
	int mMethodNameIndex;
	int mMethodTypeIndex;
	int mMethodVisibilityIndex;

	QLineEdit* mMethodReturnType;
	QLineEdit* mMethodName;
	QComboBox* mMethodType;
	QComboBox* mMethodVisibility;

	QWidgetStack* mConnections;
	QMap<QListViewItem*, BoConnectionsContainer*> mMethod2Connections;
};

#endif

