/*
    This file is part of the Boson game
    Copyright (C) 2004 Andreas Beckermann <b_mann@gmx.de>

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
#ifndef BOGLSTATEWIDGET_H
#define BOGLSTATEWIDGET_H

class QStringList;
class QListView;

#include <qwidget.h>

class BoGLStateWidgetPrivate;
/**
 * @short Fronted to @ref BoGLQueryStates
 **/
class BoGLStateWidget : public QWidget
{
	Q_OBJECT
public:
	BoGLStateWidget(QWidget* parent = 0, const char* name = 0, WFlags f = 0);
	virtual ~BoGLStateWidget();

#if 0
	/**
	 * This compares two lists that come from two @ref stateList calls and
	 * returns a list containing the differences of these lists.
	 *
	 * If one list contains a key entry that is not in the other list, that
	 * entry is ignored. Only differences of values are returned.
	 * @param _l1 A list that is expected to be from @ref stateList. An
	 * entry that does not match the expected format causes a warning.
	 * @param _l2 See @p _l1
	 **/
	static QStringList getDifferences(const QStringList& _l1, const QStringList& _l2);
	static bool showDifferences(QListView* listview, const QStringList& _l1, const QStringList& _l2);
	static bool showList(QListView* listview, const QStringList& list);
#endif

protected slots:
	void slotUpdate();
	void slotChangeStates(int);

protected:
	void makeList(QListView* l, const QStringList& items);

private:
	void initStateSelections();

private:
	BoGLStateWidgetPrivate* d;
};

#endif
