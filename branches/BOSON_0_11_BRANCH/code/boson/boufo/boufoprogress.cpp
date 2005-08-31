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
#include "ufoext/uboprogress.h"

// AB: make sure that we are compatible to system that have QT_NO_STL defined
#ifndef QT_NO_STL
#define QT_NO_STL
#endif

#include "boufoprogress.h"
#include "boufoprogress.moc"

#include <bodebug.h>

BoUfoProgress::BoUfoProgress(Qt::Orientation o) : BoUfoWidget()
{
 init(o);
}

void BoUfoProgress::init(Qt::Orientation o)
{
 setLayoutClass(UHBoxLayout);

 mProgress = new ufo::UBoProgress();
// mProgress->updateUI();
 setOrientation(o);
 widget()->add(mProgress);
}

void BoUfoProgress::setOpaque(bool o)
{
 BoUfoWidget::setOpaque(o);
 mProgress->setOpaque(o);
}

void BoUfoProgress::setOrientation(Orientation o)
{
 if (o == Horizontal) {
	mProgress->setOrientation(ufo::Horizontal);
 } else {
	mProgress->setOrientation(ufo::Vertical);
 }
}

double BoUfoProgress::value() const
{
 return mProgress->getValue();
}

double BoUfoProgress::minimumValue() const
{
 return mProgress->getMinimumValue();
}

double BoUfoProgress::maximumValue() const
{
 return mProgress->getMaximumValue();
}

void BoUfoProgress::setValue(double v)
{
 mProgress->setValue(v);
}

void BoUfoProgress::setRange(double min, double max)
{
 mProgress->setMinimumValue(min);
 mProgress->setMaximumValue(max);
}

void BoUfoProgress::setMinimumValue(double min)
{
 double max = QMAX(min, maximumValue());
 setRange(min, max);
}

void BoUfoProgress::setMaximumValue(double max)
{
 double min = QMIN(max, minimumValue());
 setRange(min, max);
}

void BoUfoProgress::setHasFrame(bool has)
{
 mProgress->setHasFrame(has);
}

bool BoUfoProgress::hasFrame() const
{
 return mProgress->getHasFrame();
}

void BoUfoProgress::setFrameColor(const QColor& c)
{
 mProgress->setFrameColor(ufo::UColor(c.red(), c.green(), c.blue()));
}

QColor BoUfoProgress::frameColor() const
{
 ufo::UColor c = mProgress->frameColor();
 return QColor((int)(c.getRed() * 255), (int)(c.getGreen() * 255), (int)(c.getBlue() * 255));
}

void BoUfoProgress::setStartColor(const QColor& c)
{
 mProgress->setStartColor(ufo::UColor(c.red(), c.green(), c.blue()));
}

QColor BoUfoProgress::startColor() const
{
 ufo::UColor c = mProgress->startColor();
 return QColor((int)(c.getRed() * 255), (int)(c.getGreen() * 255), (int)(c.getBlue() * 255));
}

void BoUfoProgress::setEndColor(const QColor& c)
{
 mProgress->setEndColor(ufo::UColor(c.red(), c.green(), c.blue()));
}

QColor BoUfoProgress::endColor() const
{
 ufo::UColor c = mProgress->endColor();
 return QColor((int)(c.getRed() * 255), (int)(c.getGreen() * 255), (int)(c.getBlue() * 255));
}

void BoUfoProgress::setColor(const QColor& c)
{
 mProgress->setColor(ufo::UColor(c.red(), c.green(), c.blue()));
}


