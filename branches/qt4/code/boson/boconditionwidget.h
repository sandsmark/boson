/*
    This file is part of the Boson game
    Copyright (C) 2004 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOCONDITIONWIDGET_H
#define BOCONDITIONWIDGET_H

#include <qwidget.h>

class QDomElement;
class QDomNodeList;
class QComboBox;
class QDomDocument;
class QListBoxItem;
class QLineEdit;
class QPushButton;
class QCheckBox;
class QListBox;
class KListBox;
class KIntNumInput;

class BoEventMatching;
class BoEventMatchingWidget;

class BoConditionWidgetPrivate;

class BoConditionWidget : public QWidget
{
	Q_OBJECT
public:
	BoConditionWidget(QWidget* parent);
	virtual ~BoConditionWidget();

	bool loadConditions(const QDomElement& root);
	const QDomDocument& conditionsDocument();
	QString toString();

protected slots:
	/**
	 * Add a condition to the bottom
	 **/
	void slotAddCondition();

	void slotShowCondition(int);
	void slotShowAction();
	void slotShowEvents();
	void slotShowStatusConditions();
	void slotDeleteCondition(int);

protected:
	void saveCurrentCondition();
	void addCondition(const QDomElement& condition);

private:
	BoConditionWidgetPrivate* d;
};

class BoConditionEventsWidget : public QWidget
{
	Q_OBJECT
public:
	BoConditionEventsWidget(QWidget* parent);
	~BoConditionEventsWidget();

	void reset();

	bool loadCondition(const QDomElement&);
	QDomDocument eventsDocument();

	void updateEventMatching(const QDomElement& root, int index);
	void unselectEventMatching();
	void reloadEventMatchings();

signals:
	void signalEditEventMatching(BoConditionEventsWidget*, int index, const QDomElement&);

protected slots:
	void slotSelectedEventMatching(int);
	void slotAddEventMatching();
	void slotRemoveCurrentEventMatching();
	void slotEditEventMatching(int index, const QDomElement& m);
	void slotEventMatchingUpdated(const QDomElement& root);

protected:
	void updateEventMatching(int index, const BoEventMatching* m);
	QListBoxItem* createEventMatchingItem(const BoEventMatching* m);
	void clearXML();

private:
	QComboBox* mForPlayer;
	KListBox* mEventMatchings;
	QPushButton* mAddMatching;
	QPushButton* mRemoveMatching;
	BoEventMatchingWidget* mEventMatchingWidget;
	int mCurrentEventMatchingIndex;

	QDomDocument* mConditionDocument;
};

class BoConditionActionWidget : public QWidget
{
	Q_OBJECT
public:
	BoConditionActionWidget(QWidget* parent);
	~BoConditionActionWidget();

	void reset();

	bool loadCondition(const QDomElement&);
	QDomDocument actionDocument();

private:
	BoEventMatchingWidget* mAction;
};

class BoEventMatchingWidget : public QWidget
{
	Q_OBJECT
public:
	/**
	 * @param eventMatching If TRUE (default) this widget edits an
	 * eventMatching. If FALSE, this widget edits an event only.
	 **/
	BoEventMatchingWidget(QWidget* parent = 0, bool eventMatching = true);
	~BoEventMatchingWidget();

	bool displayEventMatching(const QDomElement& matching);
	bool displayEvent(const QDomElement& event);

	QDomElement event() const;
	QDomElement eventMatching() const;

public slots:
	void slotClear();
	void slotApply();

signals:
	void signalEvent(const QDomElement&);
	void signalEventMatching(const QDomElement&);

protected slots:
	void slotIgnoreUnitIdChanged(bool on);
	void slotIgnorePlayerIdChanged(bool on);
	void slotIgnoreData1Changed(bool on);
	void slotIgnoreData2Changed(bool on);

private:
	bool mIsEventMatching;
	QLineEdit* mName;
	KIntNumInput* mUnitId;
	QCheckBox* mIgnoreUnitId;
	KIntNumInput* mPlayerId;
	QCheckBox* mIgnorePlayerId;
	QLineEdit* mData1;
	QCheckBox* mIgnoreData1;
	QLineEdit* mData2;
	QCheckBox* mIgnoreData2;
};

class BoConditionOverviewWidget : public QWidget
{
	Q_OBJECT
public:
	BoConditionOverviewWidget(QWidget* parent);
	~BoConditionOverviewWidget();

	void addCondition(const QString&);
	void deleteCondition(int index);
	int currentCondition() const;
	void reset();

signals:
	void signalSelectCondition(int);
	void signalShowAction();
	void signalShowEvents();
	void signalShowStatusConditions();

	void signalAddNewCondition();
	void signalDeleteCondition(int);

private slots:
	void slotDeleteCurrentCondition();

private:
	QListBox* mConditions;
	QPushButton* mEvents;
	QPushButton* mStatusConditions;
	QPushButton* mAction;

	QPushButton* mAddCondition;
	QPushButton* mDeleteCondition;
};

#endif

