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
#ifndef BOUFODESIGNERMAIN_H
#define BOUFODESIGNERMAIN_H

#include "bodebugdcopiface.h"

#include <qgl.h> // AB: _Q_GLWidget
#include <qmainwindow.h>
#include <qdialog.h>
#include <qdom.h>
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


/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class FormPreview : public QGLWidget
{
	Q_OBJECT
public:
	FormPreview(QWidget*);
	~FormPreview();

	void setPlacementMode(bool m);

	void updateGUI(const QDomElement& root);

	BoUfoWidget* getWidgetAt(int x, int y);
	BoUfoWidget* getContainerWidgetAt(int x, int y);

	void setSelectedWidget(const QDomElement& widget);

signals:
	void signalPlaceWidget(const QDomElement& parent);
	void signalSelectWidget(const QDomElement& widget);

protected:
	virtual void initializeGL();
	virtual void paintGL();
	virtual void resizeGL(int, int);

	virtual bool eventFilter(QObject* o, QEvent* e);
	virtual void mouseMoveEvent(QMouseEvent*);
	virtual void mousePressEvent(QMouseEvent*);
	virtual void mouseReleaseEvent(QMouseEvent*);
	virtual void wheelEvent(QWheelEvent*);
	virtual void keyPressEvent(QKeyEvent* e);
	virtual void keyReleaseEvent(QKeyEvent* e);

	void updateGUI(const QDomElement& root, BoUfoWidget* parent);
	void selectWidgetUnderCursor();
	void selectWidget(BoUfoWidget* widget);

private:
	void addWidget(BoUfoWidget*, const QDomElement&);

private:
	BoUfoManager* mUfoManager;
	BoUfoWidget* mContentWidget;
	bool mPlacementMode;
	QMap<void*, QDomElement> mUfoWidget2Element;
	QMap<void*, BoUfoWidget*> mUfoWidget2Widget;

	QString mNameOfSelectedWidget;
};


// displays a list of widgets that can be placed
class BoWidgetList : public QWidget
{
	Q_OBJECT
public:
	BoWidgetList(QWidget* parent, const char* name = 0);
	~BoWidgetList();

	QString widget() const;
	void clearSelection();

signals:
	void signalWidgetSelected(const QString&);

private slots:
	void slotWidgetHighlighted(QListBoxItem*);
	void slotWidgetSelectionChanged();

private:
	QListBox* mListBox;
};


// displays the current form, as it is in the internal xml file
class BoWidgetTree : public QWidget
{
	Q_OBJECT
public:
	BoWidgetTree(QWidget* parent, const char* name = 0);
	~BoWidgetTree();

	void updateGUI(const QDomElement& root);

	void selectWidget(const QDomElement& widget);

protected:
	bool isContainer(QListViewItem* item) const;
	bool isContainer(const QDomElement&) const;

signals:
	void signalWidgetSelected(const QDomElement& widget);
	void signalRemoveWidget(const QDomElement& widget);
	void signalInsertWidget(const QDomElement& parent);
	void signalHierarchyChanged();

protected:
	void updateGUI(const QDomElement& root, QListViewItem* item);
	void moveElement(QListViewItem* widget, QListViewItem* parent, QListViewItem* before);

protected slots:
	void slotSelectionChanged(QListViewItem*);
	void slotInsert();
	void slotRemove();
	void slotMoveUp();
	void slotMoveDown();

private:
	QListView* mListView;
	QPushButton* mInsertWidget;
	QPushButton* mRemoveWidget;
	QPushButton* mMoveUp;
	QPushButton* mMoveDown;

	QMap<QListViewItem*, QDomElement> mItem2Element;
};

class BoPropertiesWidget : public QWidget
{
	Q_OBJECT
public:
	BoPropertiesWidget(QWidget* parent, const char* name = 0);
	~BoPropertiesWidget();

	void displayProperties(const QDomElement& root);

signals:
	void signalChanged(const QDomElement&);

protected:
	void setClassLabel(const QString& text);
	void createProperties(const QDomElement& root);

protected slots:
	void slotItemRenamed(QListViewItem* item, int col);

private:
	QLabel* mClassLabel;
	QListView* mListView;
	QDomElement mWidgetElement;
};

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoUfoDesignerMain : public QMainWindow
{
	Q_OBJECT
public:
	BoUfoDesignerMain();
	~BoUfoDesignerMain();

public slots:
	bool slotCreateNew();
	bool slotLoadFromFile(const QString& fileName);
	bool slotLoadFromXML(const QByteArray& xml);
	bool slotSaveAsFile(const QString& fileName);

	void slotLoad();
	void slotSaveAs();
	void slotEditSignalsSlots();

protected:
	void initProperties(QDomElement& widget, const QString& className);
	void provideProperty(QDomElement& element, const QString& property);

	void initActions();

	virtual void closeEvent(QCloseEvent*);

protected slots:
	void slotUpdateGUI();

private slots:
	void slotWidgetClassSelected(const QString&);
	void slotPlaceWidget(const QDomElement& parent);
	void slotRemoveWidget(const QDomElement& parent);
	void slotSelectWidget(const QDomElement& parent);
	void slotPropertiesChanged(const QDomElement& parent);
	void slotTreeHierarchyChanged();

private:
	FormPreview* mPreview;
	BoDebugDCOPIface* mIface;
	BoWidgetList* mWidgets;
	BoWidgetTree* mWidgetTree;
	BoPropertiesWidget* mProperties;

	QDomDocument mDocument;
	QString mPlaceWidgetClass;
};

#endif

