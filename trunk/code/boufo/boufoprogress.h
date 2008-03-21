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
#ifndef BOUFOPROGRESS_H
#define BOUFOPROGRESS_H

#include "boufocustomwidget.h"

#include <qcolor.h>

class BoUfoProgress : public BoUfoCustomWidget
{
	Q_OBJECT
	Q_PROPERTY(Orientation orientation READ orientation WRITE setOrientation);
	Q_PROPERTY(double minimumValue READ minimumValue WRITE setMinimumValue);
	Q_PROPERTY(double maximumValue READ maximumValue WRITE setMaximumValue);
	Q_PROPERTY(double value READ value WRITE setValue);
	Q_PROPERTY(QColor startColor READ startColor WRITE setStartColor);
	Q_PROPERTY(QColor endColor READ endColor WRITE setEndColor);
	Q_PROPERTY(QColor frameColor READ frameColor WRITE setFrameColor);
	Q_PROPERTY(bool hasFrame READ hasFrame WRITE setHasFrame);
public:
	// AB: we must not use a QObject parent here. otherwise garbage
	// collection of libufo and Qt may confuse each other.
	BoUfoProgress(Qt::Orientation = Horizontal);

	void setOrientation(Orientation o);
	Orientation orientation() const;

	double value() const;
	void setValue(double v);
	void setRange(double min, double max);

	/**
	 * See @ref setRange
	 **/
	void setMinimumValue(double min);
	/**
	 * See @ref setRange
	 **/
	void setMaximumValue(double max);
	double minimumValue() const;
	double maximumValue() const;

	void setHasFrame(bool has);
	/**
	 * If TRUE draw a frame around the progress bar
	 **/
	bool hasFrame() const;

	/**
	 * Set the color of the frame around the progress bar. See @ref
	 * hasFrame.
	 **/
	void setFrameColor(const QColor& color);
	const QColor& frameColor() const;

	/**
	 * The color the progress bar starts with.
	 **/
	void setStartColor(const QColor& color);
	const QColor& startColor() const;

	/**
	 * The color the progress bar ends with.
	 **/
	void setEndColor(const QColor& color);
	const QColor& endColor() const;

	/**
	 * Set both, @ref startColor and @ref endColor at once
	 **/
	void setColor(const QColor& color);

	virtual void setOpaque(bool o);


	virtual QSize preferredSize(const QSize& maxSize) const;
	virtual void paintWidget();

protected:
	virtual QRect progressBarRect() const;
	virtual QRect frameRect() const;
	void paintGradient(const QColor& from, const QColor& to);
	void paintFrame(const QColor& color);

	/**
	 * @return The value that equals to 0% of the gradient. This is by
	 * default equal to @ref minimumValue but may be overridden in derived
	 * clases.
	 **/
	virtual double gradientMinimumValue() const;

	/**
	 * @return The value that equals to 100% of the gradient. This is by
	 * default equal to @ref maximumValue but may be overridden in derived
	 * clases.
	 **/
	virtual double gradientMaximumValue() const;

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

/**
 * This class adds two "extension" areas to the @ref BoUfoProgress widget.
 *
 * The "start extension" is some space @em before the progress bar gradient
 * starts and is filled completely with the @ref startColor.
 *
 * The "end extension" is similar to the "start extension", but comes right
 * after the progress bar gradient and is filled with the @ref endColor.
 *
 * These extensions can be used to define e.g. "more than 100%" areas: when the
 * value exceeds 100% (i.e. the gradient reaches @ref endColor), every value
 * above that "100%" value fill up the "end extension" more.
 *
 * Both areas take by default no space at all, i.e. the widget behaves like a
 * @ref BoUfoProgress widget. This can be changed using @ref
 * setStartExtensionSizeFactor and @ref setEndExtensionSizeFactor. These methods
 * specify the size of the extension areas as a factor of the total widget size.
 *
 * The extensions also require acertain value range between @ref minimumValue
 * and @ref maximumValue. This range is considered the "extension" value range
 * then. See @ref setStartExtensionValueRange and @ref endExtensionValueRange
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoUfoExtendedProgress : public BoUfoProgress
{
	Q_OBJECT
	Q_PROPERTY(double startExtensionSizeFactor READ startExtensionSizeFactor WRITE setStartExtensionSizeFactor);
	Q_PROPERTY(double endExtensionSizeFactor READ endExtensionSizeFactor WRITE setEndExtensionSizeFactor);
	Q_PROPERTY(double startExtensionValueRange READ startExtensionValueRange WRITE setStartExtensionValueRange);
	Q_PROPERTY(double endExtensionValueRange READ endExtensionValueRange WRITE setEndExtensionValueRange);
	Q_PROPERTY(QColor startExtensionDecorationColor READ startExtensionDecorationColor WRITE setStartExtensionDecorationColor);
	Q_PROPERTY(QColor endExtensionDecorationColor READ endExtensionDecorationColor WRITE setEndExtensionDecorationColor);
	Q_PROPERTY(bool showDecoration READ showDecoration WRITE setShowDecoration);
public:
	BoUfoExtendedProgress(Qt::Orientation = Horizontal);

	virtual QSize preferredSize(const QSize& maxSize) const;
	virtual void paintWidget();

	/**
	 * Set the amount of space that should be used for the "start
	 * extension".
	 *
	 * A factor of >= 1.0 means the whole widget is taken by the extension,
	 * a value of <= 0.0 means the extension has 0 size.
	 **/
	void setStartExtensionSizeFactor(double factor);
	double startExtensionSizeFactor() const;

	/**
	 * Set the amount of space that should be used for the "end
	 * extension".
	 *
	 * A factor of >= 1.0 means the whole widget is taken by the extension,
	 * a value of <= 0.0 means the extension has 0 size.
	 **/
	void setEndExtensionSizeFactor(double factor);
	double endExtensionSizeFactor() const;

	/**
	 * Set the value range of the "start extension". The values @ref
	 * minimumValue up to @ref minimumValue + @p range are used for the
	 * "start extenstion", only values above that are used for the gradient.
	 **/
	void setStartExtensionValueRange(double range);
	double startExtensionValueRange() const;

	/**
	 * Like @ref setStartExtensionValueRange but for the "end extension".
	 **/
	void setEndExtensionValueRange(double range);
	double endExtensionValueRange() const;

	/**
	 * Set the color of the line that separates the normal gradient and the
	 * "start extension".
	 **/
	void setStartExtensionDecorationColor(const QColor& color);
	const QColor& startExtensionDecorationColor() const;

	/**
	 * Set the color of the line that separates the normal gradient and the
	 * "end extension".
	 **/
	void setEndExtensionDecorationColor(const QColor& color);
	const QColor& endExtensionDecorationColor() const;

	/**
	 * Short for @ref setStartExtensionDecorationColor and @ref
	 * setEndExtensionDecorationColor
	 **/
	void setDecorationColor(const QColor& c);

	void setShowDecoration(bool show);

	/**
	 * @return Whether the decorations (the lines that separate the actual
	 * gradient and the "start" and "end" extensions" should be drawn. This
	 * has an effect only if at least one of @ref startExtensionValueRange
	 * and @ref endExtensionValueRange is > 0.0
	 **/
	bool showDecoration() const;

	/**
	 * Set the size of the extension decorations. 1.0 means it covers
	 * exactly the width (if orientation==Vertical) or height (if
	 * orientation==Horizontal) of the bar.
	 *
	 * Values < 1.0 are not possible. The default is 1.25.
	 **/
	void setDecorationSizeFactor(double f);
	double decorationSizeFactor() const;

protected:
	virtual QRect progressBarRect() const;
	virtual QRect frameRect() const;
	virtual double gradientMinimumValue() const;
	virtual double gradientMaximumValue() const;
	void paintExtensions(const QColor& start, const QColor& end);
	void paintExtensionDecorations(const QColor& start, const QColor& end);
	QRect removeExtensionsFromRect(const QRect& barRect_) const;
	QRect removeDecorationsFromRect(const QRect& barRect_) const;

private:
	double mStartExtensionSizeFactor;
	double mEndExtensionSizeFactor;

	double mStartExtensionValueRange;
	double mEndExtensionValueRange;

	QColor mStartExtensionDecorationColor;
	QColor mEndExtensionDecorationColor;

	bool mShowDecoration;
	double mDecorationSizeFactor;
};

#endif
