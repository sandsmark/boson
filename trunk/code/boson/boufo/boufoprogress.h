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

// note the copyright above: this is LGPL!
#ifndef BOUFOPROGRESS_H
#define BOUFOPROGRESS_H

#include "boufocustomwidget.h"

#include <qcolor.h>

class BoUfoProgress : public BoUfoCustomWidget
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
	Qt::Orientation orientation() const;

	double value() const;
	void setValue(double v);
	void setRange(double min, double max);

	void setMinimumValue(double min);
	void setMaximumValue(double max);
	double minimumValue() const;
	double maximumValue() const;

	void setHasFrame(bool has);
	bool hasFrame() const;

	void setFrameColor(const QColor& color);
	const QColor& frameColor() const;
	void setStartColor(const QColor& color);
	const QColor& startColor() const;
	void setEndColor(const QColor& color);
	const QColor& endColor() const;
	void setColor(const QColor& color);

	virtual void setOpaque(bool o);


	virtual QSize preferredSize(const QSize& maxSize) const;
	virtual void paintWidget();

protected:
	void paintGradient(const QColor& from, const QColor& to);
	void paintFrame(const QColor& color);

private:
	void init(Qt::Orientation);

private:
	ufo::UBoProgress* mProgress;

	QColor mStartColor;
	QColor mEndColor;
	QColor mFrameColor;
	bool mHasFrame;
	double mMinimumValue;
	double mMaximumValue;
	double mValue;
	Qt::Orientation mOrientation;
};

#endif
