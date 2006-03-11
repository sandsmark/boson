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
#define PREFERRED_WIDTH 40
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

double BoUfoProgress::gradientMinimumValue() const
{
 return minimumValue();
}

double BoUfoProgress::gradientMaximumValue() const
{
 return maximumValue();
}

QSize BoUfoProgress::preferredSize(const QSize& maxSize) const
{
 QSize s;
 if (orientation() == Qt::Horizontal) {
	s = QSize(PREFERRED_WIDTH, PREFERRED_HEIGHT);
 } else {
	s = QSize(PREFERRED_HEIGHT, PREFERRED_WIDTH);
 }
 return s;
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
 double l = gradientMaximumValue() - gradientMinimumValue();
 double factor;
 if (l > 0.0) {
	factor = (value() - gradientMinimumValue()) / l;
	if (factor < 0.0) {
		factor = 0.0;
	}
	if (factor > 1.0) {
		factor = 1.0;
	}
 } else {
	factor = 0.0;
 }

 glColor3ub(from.red(), from.green(), from.blue());
 glPushAttrib(GL_LIGHTING_BIT);
 glShadeModel(GL_SMOOTH);

 int realToR = (int)(to.red()   * factor + from.red()   * (1.0 - factor));
 int realToG = (int)(to.green() * factor + from.green() * (1.0 - factor));
 int realToB = (int)(to.blue()  * factor + from.blue()  * (1.0 - factor));
 int realToA = 255;

 QRect barRect = progressBarRect();
 if (orientation() == Qt::Horizontal) {
	int x1 = barRect.x();
	int y1 = barRect.y();
	int x2 = x1 + (int)((barRect.width() - 1) * factor);
	int y2 = y1 + barRect.height() - 1;
	glBegin(GL_QUADS);
		glVertex2i(x1, y1);
		glVertex2i(x1, y2);
		glColor4ub(realToR, realToG, realToB, realToA);
		glVertex2i(x2, y2);
		glVertex2i(x2, y1);
	glEnd();
 } else {
	int x1 = barRect.x();
	int y1 = barRect.y() + barRect.height() - 1;
	int x2 = x1 + barRect.width() - 1;
	int y2 = y1 - (int)((barRect.height() - 1) * factor);
	glBegin(GL_QUADS);
		glVertex2i(x1, y1);
		glVertex2i(x2, y1);
		glColor4ub(realToR, realToG, realToB, realToA);
		glVertex2i(x2, y2);
		glVertex2i(x1, y2);
	glEnd();
 }
 glPopAttrib();
}

void BoUfoProgress::paintFrame(const QColor& color)
{
 glColor3ub(color.red(), color.green(), color.blue());

 QRect rect = frameRect();
 glBegin(GL_LINE_LOOP);
	glVertex2i(rect.left(), rect.top());
	glVertex2i(rect.left(), rect.bottom());
	glVertex2i(rect.right(), rect.bottom());
	glVertex2i(rect.right(), rect.top());
 glEnd();
}

QRect BoUfoProgress::progressBarRect() const
{
 // AB: by default we take up all the space we have, in both dimensions, no
 //     matter which orientation we use
 QRect r = QRect(0, 0, width(), height());

 return r;
}

QRect BoUfoProgress::frameRect() const
{
 return progressBarRect();
}

BoUfoExtendedProgress::BoUfoExtendedProgress(Qt::Orientation o)
	: BoUfoProgress(o)
{
 mStartExtensionSizeFactor = 0.0;
 mEndExtensionSizeFactor = 0.0;
 mStartExtensionValueRange = 0.0f;
 mEndExtensionValueRange = 0.0f;
 mShowDecoration = true;
 mStartExtensionDecorationColor = Qt::black;
 mEndExtensionDecorationColor = Qt::black;
 mDecorationSizeFactor = 1.25;
}

void BoUfoExtendedProgress::setStartExtensionSizeFactor(double e)
{
 e = QMAX(0.0, e);
 e = QMIN(1.0, e);
 if (endExtensionSizeFactor() + e > 1.0) {
	e = 1.0 - endExtensionSizeFactor();
 }
 mStartExtensionSizeFactor = e;
}

double BoUfoExtendedProgress::startExtensionSizeFactor() const
{
 return mStartExtensionSizeFactor;
}

void BoUfoExtendedProgress::setEndExtensionSizeFactor(double e)
{
 e = QMAX(0.0, e);
 e = QMIN(1.0, e);
 if (startExtensionSizeFactor() + e > 1.0) {
	e = 1.0 - startExtensionSizeFactor();
 }
 mEndExtensionSizeFactor = e;
}

double BoUfoExtendedProgress::endExtensionSizeFactor() const
{
 return mEndExtensionSizeFactor;
}

void BoUfoExtendedProgress::setStartExtensionValueRange(double range)
{
 mStartExtensionValueRange = range;
}

double BoUfoExtendedProgress::startExtensionValueRange() const
{
 return mStartExtensionValueRange;
}

void BoUfoExtendedProgress::setEndExtensionValueRange(double range)
{
 mEndExtensionValueRange = range;
}

double BoUfoExtendedProgress::endExtensionValueRange() const
{
 return mEndExtensionValueRange;
}

void BoUfoExtendedProgress::setStartExtensionDecorationColor(const QColor& c)
{
 mStartExtensionDecorationColor = c;
}

const QColor& BoUfoExtendedProgress::startExtensionDecorationColor() const
{
 return mStartExtensionDecorationColor;
}

void BoUfoExtendedProgress::setEndExtensionDecorationColor(const QColor& c)
{
 mEndExtensionDecorationColor = c;
}

const QColor& BoUfoExtendedProgress::endExtensionDecorationColor() const
{
 return mEndExtensionDecorationColor;
}

void BoUfoExtendedProgress::setDecorationColor(const QColor& c)
{
 setStartExtensionDecorationColor(c);
 setEndExtensionDecorationColor(c);
}

void BoUfoExtendedProgress::setShowDecoration(bool s)
{
 mShowDecoration = s;
}

bool BoUfoExtendedProgress::showDecoration() const
{
 return mShowDecoration;
}

QSize BoUfoExtendedProgress::preferredSize(const QSize& maxSize) const
{
 QSize s = BoUfoProgress::preferredSize(maxSize);
 return s;
}

void BoUfoExtendedProgress::paintWidget()
{
 paintExtensions(startColor(), endColor());
 BoUfoProgress::paintWidget();
 if (showDecoration() && (startExtensionValueRange() > 0.0 || endExtensionValueRange() > 0.0)) {
	paintExtensionDecorations(startExtensionDecorationColor(), endExtensionDecorationColor());
 }
}

QRect BoUfoExtendedProgress::progressBarRect() const
{
 QRect r = BoUfoProgress::progressBarRect();
 QRect barRect(r);

 barRect = removeDecorationsFromRect(barRect);

 barRect = removeExtensionsFromRect(barRect);

 return barRect;
}

QRect BoUfoExtendedProgress::frameRect() const
{
 QRect r = BoUfoProgress::progressBarRect();
 r = removeDecorationsFromRect(r);
 return r;
}

double BoUfoExtendedProgress::gradientMinimumValue() const
{
 return BoUfoProgress::minimumValue() + startExtensionValueRange();
}

double BoUfoExtendedProgress::gradientMaximumValue() const
{
 if (endExtensionValueRange() > BoUfoProgress::maximumValue()) {
	return gradientMinimumValue();
 }
 return BoUfoProgress::maximumValue() - endExtensionValueRange();
}

void BoUfoExtendedProgress::paintExtensions(const QColor& start, const QColor& end)
{
 double startFactor;
 if (value() >= gradientMinimumValue()) {
	startFactor = 1.0f;
 } else {
	double min = minimumValue();
	double max = gradientMinimumValue();
	double l = max - min;
	if (l <= 0.0) {
		startFactor = 0.0;
	} else {
		startFactor = (value() - min) / l;
	}
 }

 double endFactor;
 if (value() <= gradientMaximumValue()) {
	endFactor = 0.0f;
 } else {
	double min = gradientMaximumValue();
	double max = maximumValue();
	double l = max - min;
	if (l <= 0.0) {
		endFactor = 0.0;
	} else {
		endFactor = (value() - min) / l;
	}
 }

 startFactor = QMIN(startFactor, 1.0);
 startFactor = QMAX(startFactor, 0.0);
 endFactor = QMIN(endFactor, 1.0);
 endFactor = QMAX(endFactor, 0.0);

 QRect barRect = progressBarRect();
 if (orientation() == Qt::Horizontal) {
	int startExtensionSize = barRect.x();
	int endExtensionSize = width() - startExtensionSize - barRect.width();

	int x1, x2, y1, y2;
	glColor3ub(start.red(), start.green(), start.blue());
	x1 = 0;
	y1 = barRect.y();
	x2 = x1 + (int)((startExtensionSize - 1)* startFactor);
	y2 = y1 + barRect.height() - 1;
	glBegin(GL_QUADS);
		glVertex2i(x1, y1);
		glVertex2i(x1, y2);
		glVertex2i(x2, y2);
		glVertex2i(x2, y1);
	glEnd();

	if (endExtensionSize < 0) {
		boError() << k_funcinfo << "internal error" << endl;
		endExtensionSize = 0;
	}
	glColor3ub(end.red(), end.green(), end.blue());
	x1 = barRect.x() + barRect.width() - 1;
	y1 = barRect.y();
	x2 = x1 + (int)(endExtensionSize * endFactor);
	y2 = y1 + barRect.height() - 1;
	glBegin(GL_QUADS);
		glVertex2i(x1, y1);
		glVertex2i(x1, y2);
		glVertex2i(x2, y2);
		glVertex2i(x2, y1);
	glEnd();
 } else {
	int endExtensionSize = barRect.y();
	int startExtensionSize = height() - barRect.height() - endExtensionSize;

	int x1, x2, y1, y2;
	x1 = barRect.x();
	y1 = barRect.y() + barRect.height() - 1 + startExtensionSize;
	x2 = x1 + barRect.width() - 1;
	y2 = y1 - (int)(startExtensionSize * startFactor);
	glColor3ub(start.red(), start.green(), start.blue());
	glBegin(GL_QUADS);
		glVertex2i(x1, y1);
		glVertex2i(x1, y2);
		glVertex2i(x2, y2);
		glVertex2i(x2, y1);
	glEnd();

	glColor3ub(end.red(), end.green(), end.blue());
	x1 = barRect.x();
	y1 = endExtensionSize;
	x2 = x1 + barRect.width() - 1;
	y2 = y1 - (int)(endExtensionSize * endFactor);
	glBegin(GL_QUADS);
		glVertex2i(x1, y1);
		glVertex2i(x1, y2);
		glVertex2i(x2, y2);
		glVertex2i(x2, y1);
	glEnd();
 }
}

void BoUfoExtendedProgress::paintExtensionDecorations(const QColor& start, const QColor& end)
{
 QRect rect = progressBarRect();
 if (orientation() == Qt::Horizontal) {
	int x;
	int y1, y2;
	x = rect.left();
	y1 = rect.top() - (int)((rect.height() * mDecorationSizeFactor) / 2.0);
	y2 = rect.bottom() + (int)((rect.height() * mDecorationSizeFactor) / 2.0);
	if (startExtensionValueRange() > 0.0) {
		glColor3ub(start.red(), start.green(), start.blue());
		glBegin(GL_LINES);
			glVertex2i(x, y1);
			glVertex2i(x, y2);
		glEnd();
	}

	x = rect.right();
	if (endExtensionValueRange() > 0.0) {
		glColor3ub(end.red(), end.green(), end.blue());
		glBegin(GL_LINES);
			glVertex2i(x, y1);
			glVertex2i(x, y2);
		glEnd();
	}
 } else {
	int x1, x2;
	int y;
	x1 = rect.left() - (int)((rect.width() * mDecorationSizeFactor) / 2.0);
	x2 = rect.right() + (int)((rect.width() * mDecorationSizeFactor) / 2.0);
	y = rect.bottom();

	if (startExtensionValueRange() > 0.0) {
		glColor3ub(start.red(), start.green(), start.blue());
		glBegin(GL_LINES);
			glVertex2i(x1, y);
			glVertex2i(x2, y);
		glEnd();
	}

	y = rect.top();
	if (endExtensionValueRange() > 0.0) {
		glColor3ub(end.red(), end.green(), end.blue());
		glBegin(GL_LINES);
			glVertex2i(x1, y);
			glVertex2i(x2, y);
		glEnd();
	}
 }
}

QRect BoUfoExtendedProgress::removeExtensionsFromRect(const QRect& barRect_) const
{
 double barFactor = 1.0 - (startExtensionSizeFactor() + endExtensionSizeFactor());
 QRect barRect(barRect_);
 if (orientation() == Qt::Horizontal) {
	int w = barRect.width();
	barRect.setX(barRect.x() + (int)(barRect.width() * startExtensionSizeFactor()));
	barRect.setWidth((int)(w * barFactor));
 } else {
	int h = barRect.height();
	barRect.setY(barRect.y() + (int)(barRect.height() * endExtensionSizeFactor()));
	barRect.setHeight((int)(h * barFactor));
 }
 return barRect;
}

QRect BoUfoExtendedProgress::removeDecorationsFromRect(const QRect& barRect_) const
{
 QRect barRect(barRect_);
 if (showDecoration() && (startExtensionValueRange() > 0.0 || endExtensionValueRange() > 0.0)) {
	double bar;
	if (mDecorationSizeFactor <= 1.0) {
		bar = 1.0;
	} else {
		bar = 1.0 / mDecorationSizeFactor;
	}
	bar = QMIN(bar, 1.0);
	bar = QMAX(bar, 0.0);

	if (orientation() == Qt::Horizontal) {
		double totalH = (double)barRect.height();
		double barH = totalH * bar;
		double diff = (totalH - barH);
		diff = QMAX(diff, 0.0);

		barRect.setY(barRect.y() + (int)(diff / 2.0));
		barRect.setHeight((int)barH);
	} else {
		double totalW = (double)barRect.width();
		double barW = totalW * bar;
		double diff = (totalW - barW);
		diff = QMAX(diff, 0.0);

		barRect.setX(barRect.x() + (int)(diff / 2.0));
		barRect.setWidth((int)barW);

	}
 }
 return barRect;
}

