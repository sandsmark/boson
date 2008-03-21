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
#ifndef BOUFOWIDGETSTACK_H
#define BOUFOWIDGETSTACK_H

#include "boufowidget.h"

template<class T1, class T2> class QMap;

class BoUfoWidgetStack : public BoUfoWidget
{
	Q_OBJECT
public:
	BoUfoWidgetStack();
	~BoUfoWidgetStack();

	int insertStackWidget(BoUfoWidget*, int id = -1);
	void raiseStackWidget(BoUfoWidget*);
	void raiseStackWidget(int id);
	void removeStackWidget(BoUfoWidget*);
	void removeStackWidget(int id);
	BoUfoWidget* stackWidget(int id) const;
	BoUfoWidget* visibleWidget() const
	{
		return mVisibleWidget;
	}
	int id(BoUfoWidget* widget) const;

signals:
	/**
	 * Emitted when the currently visible widget is changed, e.g. when a
	 * widget is raised (see @ref raiseStackWidget).
	 *
	 * Note that @p visibleWidget may be NULL!
	 **/
	void signalVisibleWidgetChanged(BoUfoWidget* visibleWidget);

private:
	void init();

private:
	QMap<int, BoUfoWidget*>* mId2Widget;
	BoUfoWidget* mVisibleWidget;
};


#endif
