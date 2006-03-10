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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <bogl.h>

#include "boufoprogress.h"
#include "boufoprogress.moc"

#include <bodebug.h>

// AB: these are the default values. pretty random values though
// -> a user can and probably will use widget->setPreferredSize() anyway
#define PREFERRED_WIDTH 20
#define PREFERRED_HEIGHT 10


BoUfoProgress::BoUfoProgress(Qt::Orientation o)
	: BoUfoCustomWidget()
{
 init(o);
}

void BoUfoProgress::init(Qt::Orientation o)
{
 mStartColor = QColor(255, 0, 0);
 mEndColor = QColor(0, 255, 0);
 mFrameColor = QColor(64, 64, 64);
 mHasFrame = true;
 mValue = 50.0;
 mMinimumValue = 0.0;
 mMaximumValue = 100.0;
 mOrientation = o;
}

void BoUfoProgress::setOpaque(bool o)
{
 BoUfoWidget::setOpaque(o);
}

void BoUfoProgress::setOrientation(Orientation o)
{
 mOrientation = o;
 invalidate();
}

Qt::Orientation BoUfoProgress::orientation() const
{
 return mOrientation;
}

double BoUfoProgress::value() const
{
 return mValue;
}

double BoUfoProgress::minimumValue() const
{
 return mMinimumValue;
}

double BoUfoProgress::maximumValue() const
{
 return mMaximumValue;
}

void BoUfoProgress::setValue(double v)
{
 v = QMAX(v, minimumValue());
 v = QMIN(v, maximumValue());
 mValue = v;
}

void BoUfoProgress::setRange(double min, double max)
{
 max = QMAX(max, min);
 mMinimumValue = min;
 mMaximumValue = max;

 mValue = QMAX(mValue, minimumValue());
 mValue = QMIN(mValue, maximumValue());
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
 mHasFrame = has;
}

bool BoUfoProgress::hasFrame() const
{
 return mHasFrame;
}

void BoUfoProgress::setFrameColor(const QColor& c)
{
 mFrameColor = c;
}

const QColor& BoUfoProgress::frameColor() const
{
 return mFrameColor;
}

void BoUfoProgress::setStartColor(const QColor& c)
{
 mStartColor = c;
}

const QColor& BoUfoProgress::startColor() const
{
 return mStartColor;
}

void BoUfoProgress::setEndColor(const QColor& c)
{
 mEndColor = c;
}

const QColor& BoUfoProgress::endColor() const
{
 return mEndColor;
}

void BoUfoProgress::setColor(const QColor& c)
{
 setStartColor(c);
 setEndColor(c);
}

QSize BoUfoProgress::preferredSize(const QSize& maxSize) const
{
 QSize size;
 if (orientation() == Qt::Horizontal) {
	size = QSize(PREFERRED_WIDTH, PREFERRED_HEIGHT);
 } else {
	size = QSize(PREFERRED_HEIGHT, PREFERRED_HEIGHT);
 }
 return size.boundedTo(maxSize);
}

void BoUfoProgress::paintWidget()
{
 // TODO: support an icon
 paintGradient(startColor(), endColor());
 if (hasFrame()) {
	paintFrame(frameColor());
 }
}

void BoUfoProgress::paintGradient(const QColor& from, const QColor& to)
{
 double l = maximumValue() - minimumValue();
 double factor;
 if (l != 0.0) {
	factor = (value() - minimumValue()) / l;
 } else {
	factor = 0.0;
 }

 glColor3ub(from.red(), from.green(), from.blue());
 glPushAttrib(GL_LIGHTING_BIT);
 glShadeModel(GL_SMOOTH);

 int realToR = (to.red()   * factor + from.red()   * (1.0 - factor));
 int realToG = (to.green() * factor + from.green() * (1.0 - factor));
 int realToB = (to.blue()  * factor + from.blue()  * (1.0 - factor));
 int realToA = 255;

 if (orientation() == Qt::Horizontal) {
	glBegin(GL_QUADS);
		glVertex2i(0, 0);
		glVertex2i(0, height());
		glColor4ub(realToR, realToG, realToB, realToA);
		glVertex2i((int)(width() * factor), height());
		glVertex2i((int)(width() * factor), 0);
	glEnd();
 } else {
	glBegin(GL_QUADS);
		glVertex2i(0, height());
		glVertex2i(width(), height());
		glColor4ub(realToR, realToG, realToB, realToA);
		glVertex2i(width(), height() - (int)(height() * factor));
		glVertex2i(0, height() - (int)(height() * factor));
	glEnd();
 }
 glPopAttrib();
}

void BoUfoProgress::paintFrame(const QColor& color)
{
 glColor3ub(color.red(), color.green(), color.blue());

 glBegin(GL_LINE_LOOP);
	glVertex2i(0, 0);
	glVertex2i(0, height());
	glVertex2i(width(), height());
	glVertex2i(width(), 0);
 glEnd();
}


