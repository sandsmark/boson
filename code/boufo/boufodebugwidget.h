/*
    This file is part of the Boson game
    Copyright (C) 2005 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BODEBUGUFOWIDGET_H
#define BODEBUGUFOWIDGET_H

#include <qwidget.h>

class QMouseEvent;
class QWheelEvent;
class QKeyEvent;
template<class T1, class T2> class QMap;
class QDomElement;
class QListViewItem;

class BoUfoImage;
class BoUfoDrawable;
class BoUfoFontInfo;

class BoUfoManager;
class BoUfoWidget;

namespace ufo {
	class UWidget;
};

class BoUfoDebugSingleWidgetPrivate;
class BoUfoDebugSingleWidget : public QWidget
{
	Q_OBJECT
public:
	BoUfoDebugSingleWidget(QWidget* parent);
	~BoUfoDebugSingleWidget();

	void setWidget(ufo::UWidget* u, BoUfoWidget* w);

private:
	BoUfoDebugSingleWidgetPrivate* d;
};

class BoUfoDebugWidgetPrivate;
class BoUfoDebugWidget : public QWidget
{
	Q_OBJECT
public:
	// AB: we must not use a QObject parent here. otherwise garbage
	// collection of libufo and Qt may confuse each other.
	BoUfoDebugWidget(QWidget* parent = 0);
	~BoUfoDebugWidget();

	void setBoUfoManager(BoUfoManager* m);

protected:
	void addWidget(ufo::UWidget* w, QListViewItem* item);

protected slots:
	void slotWidgetChanged(QListViewItem* item);

private:
	BoUfoDebugWidgetPrivate* d;
};

#endif
