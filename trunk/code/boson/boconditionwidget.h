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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
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
class KListBox;
class KIntNumInput;

class BoOneConditionWidget;
class BoEventMatching;
class BoEventMatchingWidget;

class BoConditionWidgetPrivate;

// displays/configures n conditions, using BoOneConditionWidget for each of them
class BoConditionWidget : public QWidget
{
	Q_OBJECT
public:
	BoConditionWidget(QWidget* parent);
	virtual ~BoConditionWidget();

	bool loadConditions(const QDomElement& root);
	QString toString();

protected slots:
	/**
	 * Add a condition to the bottom
	 **/
	void slotAddCondition();

	/**
	 * Remove the last condition (i.e. the most recently added)
	 **/
	void slotRemoveCondition();

	void slotEventMatchingUpdated(const QDomElement&);

	void slotEditEventMatching(BoOneConditionWidget* c, int index, const QDomElement& m);

private:
	BoConditionWidgetPrivate* d;
};

// displays/configures a single condition only
class BoOneConditionWidget : public QWidget
{
	Q_OBJECT
public:
	BoOneConditionWidget(QWidget* parent);
	~BoOneConditionWidget();

	bool loadCondition(const QDomElement&);
	QDomElement element();

	void updateEventMatching(const QDomElement& root, int index);
	void unselectEventMatching();
	void reloadEventMatchings();

signals:
	void signalEditEventMatching(BoOneConditionWidget*, int index, const QDomElement&);

protected slots:
	void slotSelectedEventMatching(int);
	void slotAddEventMatching();
	void slotRemoveCurrentEventMatching();

protected:
	void updateEventMatching(int index, const BoEventMatching* m);
	QListBoxItem* createEventMatchingItem(const BoEventMatching* m);
	void clearXML();

private:
	QComboBox* mForPlayer;
	KListBox* mEventMatchings;
	QPushButton* mAddMatching;
	QPushButton* mRemoveMatching;
	BoEventMatchingWidget* mAction;

	QDomDocument* mConditionDocument;
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

#endif

