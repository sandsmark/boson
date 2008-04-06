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
#ifndef BODEBUGLOGDIALOG_H
#define BODEBUGLOGDIALOG_H

#include <kdialogbase.h>
//Added by qt3to4:
#include <Q3PtrList>

class BoDebugMessage;
template<class T> class Q3PtrList;

class BoDebugLogWidgetPrivate;
class BoDebugLogWidget : public QWidget
{
	Q_OBJECT
public:
	BoDebugLogWidget(QWidget* parent, const char* name = 0);
	~BoDebugLogWidget();

	void setMessages(const Q3PtrList<BoDebugMessage>& m);

protected slots:
	void slotMessageSelected(Q3ListViewItem*);

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

