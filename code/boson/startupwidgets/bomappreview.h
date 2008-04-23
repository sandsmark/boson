/*
    This file is part of the Boson game
    Copyright (C) 2006-2008 Andreas Beckermann (b_mann@gmx.de)

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

#ifndef BOMAPPREVIEW_H
#define BOMAPPREVIEW_H

#include "../boufo/boufowidget.h"
#include "../boufo/boufocustomwidget.h"
#include <QWheelEvent>
#include <QMouseEvent>
#include <QWidget>

class BosonPlayField;
class QImage;
class BPFPreview;

class BoMapPreviewPrivate;
/**
 * @short Preview of the map - pretty much like a minimap
 *
 * This widget is meant to provide a "minimap" to be displayed when selecting
 * the playfield.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoMapPreview : public QWidget
{
	Q_OBJECT
public:
	BoMapPreview(QWidget* parent = 0);
	~BoMapPreview();

	void setPlayField(const BPFPreview& preview);

public slots:

private:
	BoMapPreviewPrivate* d;
};

class BoMapPreviewDisplayPrivate;
/**
 * @internal
 * @short Helper class for @ref BoMapPreview.
 *
 * This class simply displays the map itself, without any buttons or anything
 * else around it.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoMapPreviewDisplay : public QWidget//BoUfoCustomWidget
{
	Q_OBJECT
public:
	BoMapPreviewDisplay(QWidget* parent = 0);
	~BoMapPreviewDisplay();

	virtual QSize sizeHint() const;

	void setPreview(const QImage& image);

protected slots:
	void slotWidgetResized();
	void slotMouseEvent(QMouseEvent* e);
	void slotWheelEvent(QWheelEvent* e);

protected:
	virtual void paintEvent(QPaintEvent*);

private:
	BoMapPreviewDisplayPrivate* d;
};

#endif

