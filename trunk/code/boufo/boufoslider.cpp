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

#include <bogl.h>

// AB: first include the ufo headers, otherwise we conflict with Qt
#include <ufo/ufo.hpp>

// AB: make sure that we are compatible to system that have QT_NO_STL defined
#ifndef QT_NO_STL
#define QT_NO_STL
#endif

#include "boufoslider.h"
#include "boufoslider.moc"

#include <bodebug.h>

#include <math.h>

BoUfoSlider::BoUfoSlider(Qt::Orientation o) : BoUfoWidget()
{
 init(o);
}

void BoUfoSlider::init(Qt::Orientation o)
{
 mMin = 0.0f;
 mMax = 100.0f;
 mStep = 1.0f;
 setLayoutClass(UHBoxLayout);

 if (o == Horizontal) {
	mSlider = new ufo::USlider(ufo::Horizontal);
 } else {
	mSlider = new ufo::USlider(ufo::Vertical);
 }
 ufoWidget()->add(mSlider);

 CONNECT_UFO_TO_QT(BoUfoSlider, mSlider, ValueChanged);

 setRange(0, 100);
 setValue(50);
}

void BoUfoSlider::setOpaque(bool o)
{
 BoUfoWidget::setOpaque(o);
 mSlider->setOpaque(o);
}

void BoUfoSlider::uslotValueChanged(ufo::UAbstractSlider*)
{
 emit signalValueChanged(value());
 emit signalFloatValueChanged(floatValue());
}

void BoUfoSlider::setValue(int v)
{
 mSlider->setValue(v);
}

void BoUfoSlider::setFloatValue(float v)
{
 mSlider->setValue(lrint(v / mStep));
}

void BoUfoSlider::setRange(int min, int max)
{
 setFloatRange(min, max, 1.0f);
}

void BoUfoSlider::setFloatRange(float min, float max, float step)
{
 if (max < min) {
	boError() << k_funcinfo << "max < min" << endl;
	max = min;
 }
 mMin = min;
 mMax = max;
 mStep = step;

 mSlider->setMinimum((int)(mMin/mStep));
 mSlider->setMaximum((int)(mMax/mStep));
}

int BoUfoSlider::value() const
{
 return mSlider->getValue();
}

float BoUfoSlider::floatValue() const
{
 return mStep * ((float)value());
}

