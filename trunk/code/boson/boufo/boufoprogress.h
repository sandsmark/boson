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
#ifndef BOUFOPROGRESS_H
#define BOUFOPROGRESS_H

#include "boufowidget.h"

class BoUfoProgress : public BoUfoWidget
{
	Q_OBJECT
	Q_PROPERTY(double minimumValue READ minimumValue WRITE setMinimumValue);
	Q_PROPERTY(double maximumValue READ maximumValue WRITE setMaximumValue);
	Q_PROPERTY(double value READ value WRITE setValue);
	Q_PROPERTY(QColor startColor READ startColor WRITE setStartColor);
	Q_PROPERTY(QColor endColor READ endColor WRITE setEndColor);
public:
	// AB: we must not use a QObject parent here. otherwise garbage
	// collection of libufo and Qt may confuse each other.
	BoUfoProgress(Qt::Orientation = Horizontal);

	void setOrientation(Qt::Orientation o);

	ufo::UBoProgress* progress() const
	{
		return mProgress;
	}

	double value() const;
	void setValue(double v);
	void setRange(double min, double max);

	void setMinimumValue(double min);
	void setMaximumValue(double max);
	double minimumValue() const;
	double maximumValue() const;

	void setStartColor(const QColor& color);
	QColor startColor() const;
	void setEndColor(const QColor& color);
	QColor endColor() const;
	void setColor(const QColor& color);

	virtual void setOpaque(bool o);

private:
	void init(Qt::Orientation);

private:
	ufo::UBoProgress* mProgress;
};

#endif
