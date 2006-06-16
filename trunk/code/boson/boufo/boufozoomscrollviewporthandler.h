/*
    This file is part of the Boson game
    Copyright (C) 2001-2006 Andreas Beckermann (b_mann@gmx.de)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef BOUFOZOOMSCROLLVIEWPORTHANDLER_H
#define BOUFOZOOMSCROLLVIEWPORTHANDLER_H

#include <qobject.h>

#include "../bomath.h"
#include <bogl.h>

class BoMatrix;

class BoUfoZoomScrollViewportHandlerPrivate;
/**
 * Provides the means for developing a viewport widget that supports zooming and
 * scrolling.
 *
 * This class maps a certain "viewport" rectangle, i.e. a rectangle that is
 * actually meant to be visible, onto a "data" rectangle.
 *
 * In addition zooming (see @ref zoomIn and @ref zoomOut) and scrolling (see
 * @ref centerViewOnDataPoint) is supported. A @ref transformationMatrix is
 * provided to easily apply these values to the actual widget.
 *
 * When the displayed data cannot match the viewport size (e.g. because the data
 * is wider than the viewport width, but not as high as the viewport height),
 * this class makes sure that the data is displayed in the top-left corner of
 * the viewport.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoUfoZoomScrollViewportHandler : public QObject
{
	Q_OBJECT
public:
	BoUfoZoomScrollViewportHandler(QObject* parent);
	virtual ~BoUfoZoomScrollViewportHandler();

	/**
	 * The size of the to-be-displayed data. For example the size of the
	 * map, if this class is meant to hold a (mini-)map.
	 **/
	void setDataSize(unsigned int w, unsigned int h);

	/**
	 * @return See @ref setDataSize
	 **/
	unsigned int dataWidth() const;

	/**
	 * @return See @ref setDataSize
	 **/
	unsigned int dataHeight() const;

	/**
	 * The size (in pixels) of the viewport
	 **/
	void setViewSize(unsigned int w, unsigned int h);

	/**
	 * Center the view on (@p x, @p y) in the data. It is ensured that @p x
	 * and @p y are actually valid values for data points. See also @ref
	 * setDataSize.
	 *
	 * Note that both, @p x and @p y, use y=0 as top and y=height-1 as
	 * bottom!
	 **/
	void centerViewOnDataPoint(int x, int y);

	/**
	 * @return See @ref viewCenterX
	 **/
	int viewCenterX() const;

	/**
	 * @return See @ref viewCenterX
	 **/
	int viewCenterY() const;

	void zoomIn();
	void zoomOut();

	/**
	 * @return The point in the data that is displayed at @p pos in the
	 * viewport. This method honors scaling and scrolling.
	 *
	 * @param pos Point in the viewport. Note that we assume that a widget
	 * coordinate uses y=0 as top and y=height-1 as bottom!!!
	 **/
	QPoint widgetPointToDataPoint(const QPoint& pos) const;

	/**
	 * Builds and returns a transformation matrix for the current zooming
	 * and scrolling values.
	 *
	 * @return A transformation matrix describing the current zooming and
	 * scrolling values.
	 **/
	BoMatrix transformationMatrix() const;

protected:
	int xTranslation() const;
	int yTranslation() const;
	float zoomOutFactor() const;

	void fixZoomStep();
	float calculateValidZoomStep(float desiredStep) const;

private:
	BoUfoZoomScrollViewportHandlerPrivate* d;
};

#endif

