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

#include <bogl.h>

// AB: first include the ufo headers, otherwise we conflict with Qt
#include <ufo/ufo.hpp>

// AB: make sure that we are compatible to system that have QT_NO_STL defined
#ifndef QT_NO_STL
#define QT_NO_STL
#endif

#include "boufowidgetstack.h"
#include "boufowidgetstack.moc"

#include <qmap.h>

#include <bodebug.h>

BoUfoWidgetStack::BoUfoWidgetStack() : BoUfoWidget()
{
 init();
}

BoUfoWidgetStack::~BoUfoWidgetStack()
{
 delete mId2Widget;
}

void BoUfoWidgetStack::init()
{
 mId2Widget = new QMap<int, BoUfoWidget*>();
 mVisibleWidget = 0;

 // we just need some layout. there is only a single widget visible at any time,
 // so it doesnt matter so much which layout we use
 setLayoutClass(BoUfoWidget::UVBoxLayout);
}

int BoUfoWidgetStack::insertStackWidget(BoUfoWidget* widget, int id)
{
 if (id < 0) {
	id = mId2Widget->count();
	while (mId2Widget->contains(id)) {
		id++;
	}
 }
 mId2Widget->insert(id, widget);
 widget->hide();
 if (!mVisibleWidget) {
	raiseStackWidget(id);
 }
 return id;
}

void BoUfoWidgetStack::raiseStackWidget(BoUfoWidget* widget)
{
 raiseStackWidget(id(widget));
}

void BoUfoWidgetStack::raiseStackWidget(int id)
{
 if (mVisibleWidget) {
	mVisibleWidget->hide();
 }
 if (mId2Widget->contains(id)) {
	mVisibleWidget = *mId2Widget->find(id);
 } else {
	mVisibleWidget = 0;
 }
 if (mVisibleWidget) {
	mVisibleWidget->show();
 }

 emit signalVisibleWidgetChanged(visibleWidget());
}

void BoUfoWidgetStack::removeStackWidget(BoUfoWidget* w)
{
 removeStackWidget(id(w));
}

void BoUfoWidgetStack::removeStackWidget(int id)
{
 if (id < 0) {
	return;
 }
 BoUfoWidget* w = stackWidget(id);
 mId2Widget->remove(id);
 if (mVisibleWidget == w) {
	if (mId2Widget->count() > 0) {
		raiseStackWidget(mId2Widget->begin().key());
	} else {
		raiseStackWidget(-1);
	}
 }
}

BoUfoWidget* BoUfoWidgetStack::stackWidget(int id) const
{
 if (id < 0) {
	return 0;
 }
 if (!mId2Widget->contains(id)) {
	return 0;
 }
 return *mId2Widget->find(id);
}

int BoUfoWidgetStack::id(BoUfoWidget* widget) const
{
 QMap<int, BoUfoWidget*>::iterator it;
 for (it = mId2Widget->begin(); it != mId2Widget->end(); ++it) {
	if ((*it) == widget) {
		return it.key();
	}
 }
 return -1;
}

