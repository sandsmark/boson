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

	virtual void paint();
	virtual void paintWidget();
	virtual void paintBorder();
};

#endif
