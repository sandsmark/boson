/*
    This file is part of the Boson game
    Copyright (C) 2004-2005 Andreas Beckermann (b_mann@gmx.de)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

// note the copyright above: this is LGPL!
#ifndef BOUFOINPUTDIALOG_H
#define BOUFOINPUTDIALOG_H

#include "boufointernalframe.h"

class BoUfoNumInput;
class BoUfoLabel;
class BoUfoPushButton;
class BoUfoLineEdit;

class BoUfoInputDialog : public BoUfoInternalFrame
{
	Q_OBJECT
public:
	BoUfoInputDialog(BoUfoManager* manager, const QString& label, const QString& title = QString::null);
	~BoUfoInputDialog();

	void setLabel(const QString& label);


	// note that due to the design of libufo we cannot make modal dialogs.
	// so we can't return the integer value here.
	// AB: the returned pointer can be discarded usually.
	static BoUfoInputDialog* getIntegerWidget(BoUfoManager* manager, const QObject* receiver, const char* slot, const QString& label, const QString& title = QString::null, int value = 0, int min = 0, int max = 1000, int step = 1);
	static BoUfoInputDialog* getFloatWidget(BoUfoManager* manager, const QObject* receiver, const char* slot, const QString& label, const QString& title = QString::null, float value = 0.0, float min = 0.0, float max = 1000.0, float step = 1.0);
	static BoUfoInputDialog* getStringWidget(BoUfoManager* manager, const QObject* receiver, const char* slot, const QString& label, const QString& title = QString::null, const QString& value = QString::null);

signals:
	/**
	 * Emitted right before the frame is removed from the ufo manager.
	 *
	 * Note that this widget is closed shortly after this and therfore will
	 * get deleted then.
	 **/
	void signalClosed();

	/**
	 * Emitted when Cancel is clicked, right before the widget is being closed.
	 *
	 * Note that this widget is closed shortly after this and therfore will
	 * get deleted then.
	 **/
	void signalCancelled();
	void signalValueEntered(int);
	void signalValueEntered(float);
	void signalValueEntered(const QString&);

protected:
	BoUfoNumInput* numInput();
	BoUfoLineEdit* lineEdit();
	void close();

	static BoUfoInputDialog* createNumDialog(BoUfoManager* manager, const QString& label, const QString& title, float value, float min, float max, float step);

protected slots:
	void slotOkClicked();
	void slotCancelled();

private:
	void init();

private:
	BoUfoManager* mUfoManager;
	BoUfoLabel* mLabel;
	BoUfoWidget* mContents;
	BoUfoPushButton* mOk;
	BoUfoPushButton* mCancel;
	BoUfoNumInput* mNumInput;
	BoUfoLineEdit* mLineEdit;
};

#endif
