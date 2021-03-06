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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

// note the copyright above: this is LGPL!
#ifndef BOUFOTABWIDGET_H
#define BOUFOTABWIDGET_H

#include "boufowidget.h"

class BoUfoTabWidgetPrivate;
class BoUfoTabWidget : public BoUfoWidget
{
	Q_OBJECT
public:
	BoUfoTabWidget();
	~BoUfoTabWidget();

	int addTab(BoUfoWidget* widget, const QString& label);
	void removeTab(BoUfoWidget* widget);

	void setCurrentTab(int tab);
	BoUfoWidget* currentTab() const;

	int findId() const;

	virtual void setOpaque(bool o);

protected slots:
	void slotButtonClicked();

private:
	void init();

private:
	BoUfoTabWidgetPrivate* d;
};

#endif
