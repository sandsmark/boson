/*
    This file is part of the Boson game
    Copyright (C) 2008 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOLAYEREDWIDGET_H
#define BOLAYEREDWIDGET_H

#include <QWidget>

/**
 * Widget that keeps all its children
 * @li on top of each other
 * @li at maximum size (i.e. the size of this widget)
 *
 * To change the order (i.e. the z value) of the child widgets, use @ref
 * QWidget::raise, @ref QWidget::lower and @ref QWidget::stackUnder.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoLayeredWidget : public QWidget
{
	Q_OBJECT
public:
	BoLayeredWidget(QWidget* parent = 0);
	~BoLayeredWidget();

	virtual QSize sizeHint() const;

protected:
	virtual void resizeEvent(QResizeEvent* e);
	virtual void childEvent(QChildEvent* e);
};

#endif

