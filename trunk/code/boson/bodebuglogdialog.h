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
#ifndef BODEBUGLOGDIALOG_H
#define BODEBUGLOGDIALOG_H

#include <kdialogbase.h>

class BoDebugMessage;
template<class T> class QPtrList;

class BoDebugLogWidgetPrivate;
class BoDebugLogWidget : public QWidget
{
	Q_OBJECT
public:
	BoDebugLogWidget(QWidget* parent, const char* name = 0);
	~BoDebugLogWidget();

	void setMessages(const QPtrList<BoDebugMessage>& m);

protected slots:
	void slotMessageSelected(QListViewItem*);

private:
	BoDebugLogWidgetPrivate* d;
};

class BoDebugLogDialogPrivate;
class BoDebugLogDialog : public KDialogBase
{
	Q_OBJECT
public:
	BoDebugLogDialog(QWidget* parent, const char* name = 0);
	~BoDebugLogDialog();

public slots:
	void slotUpdate();

protected:
	void updateWidget(int level);

private:
	BoDebugLogDialogPrivate* d;
};

#endif

