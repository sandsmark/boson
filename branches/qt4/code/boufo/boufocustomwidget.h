/*
    This file is part of the Boson game
    Copyright (C) 2004-2006 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOUFOCUSTOMWIDGET_H
#define BOUFOCUSTOMWIDGET_H

#include "boufowidget.h"

/**
 * This widget uses a modified ufo widget as base. The methods @ref
 * BoUfoWidget::paint, @ref BoUfoWidget::paintWidget and @ref
 * BoUfoWidget::paintBorder are used for rendering.
 *
 * By default, they just call the original implementation of @ref ufo::UWidget.
 **/
class BoUfoCustomWidget : public BoUfoWidget
{
	Q_OBJECT
public:
	BoUfoCustomWidget();
	~BoUfoCustomWidget();

	/**
	 * You usually should not use this. This overwrites the whole painting
	 * procedure of the @ref ufo::UWidget object and in particular disables
	 * painting the children.
	 *
	 * Usually you should use @ref paintWidget instead.
	 *
	 * Note that pixel exact rendering (see redbook appendix "programming
	 * tips") in this method works only if you use @ref ufo::UGraphics
	 * methods or manually translate the modelview matrix by 0.375 in x and
	 * y directions.
	 **/
	virtual void paint();

	/**
	 * Paint the widget. Pretty much all GL operations of this widget should
	 * go here.
	 *
	 * Note that pixel exact rendering (see redbook appendix "programming
	 * tips") should work in this method, i.e. glRect(0, 0, width(),
	 * height()); should fill exactly this widget (neither more nor less).
	 **/
	virtual void paintWidget();

	/**
	 * See @ref ufo::UWidget::paintBorder. Usually you should overwrite @ref
	 * paintWidget instead.
	 *
	 * Note that unlike @ref paintWidget, this method does not provide
	 * support for pixel exact rendering (see redbook appendix "programming
	 * tips").
	 **/
	virtual void paintBorder();

	virtual QSize preferredSize(const QSize& maxSize) const;

};

#endif
