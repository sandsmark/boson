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
#ifndef BOUFOSLIDER_H
#define BOUFOSLIDER_H

#include "boufowidget.h"

/**
 * A BoUfo implementation of @ref ufo::USlider.
 *
 * Additionally this class provides support for float values.
 **/
class BoUfoSlider : public BoUfoWidget
{
	Q_OBJECT
public:
	// AB: we must not use a QObject parent here. otherwise garbage
	// collection of libufo and Qt may confuse each other.
	BoUfoSlider(Qt::Orientation = Horizontal);

	ufo::USlider* slider() const
	{
		return mSlider;
	}

	int value() const;
	float floatValue() const;
	void setValue(int v);
	void setStepSize(int);
	void setRange(int min, int max);

	void setFloatValue(float v);
	void setFloatRange(float min, float max, float step);

	float minimumValue() const { return mMin; }
	float maximumValue() const { return mMax; }
	float stepSize() const { return mStep; }

	virtual void setOpaque(bool o);

signals:
	void signalValueChanged(int);
	void signalFloatValueChanged(float);

private:
	void uslotValueChanged(ufo::UAbstractSlider*);

private:
	void init(Qt::Orientation);

private:
	float mMin;
	float mMax;
	float mStep;
	ufo::USlider* mSlider;
};

#endif
