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

#include "bosonzoomscrollviewport.h"
#include "bosonzoomscrollviewport.moc"

#include "../../bomemory/bodummymemory.h"
#include "../bo3dtools.h"
#include "bodebug.h"
#include <bogl.h>

#include <klocale.h>

#include <qvaluelist.h>
#include <qpair.h>

class BosonZoomScrollViewportPrivate
{
public:
	BosonZoomScrollViewportPrivate()
	{
	}

	unsigned int mDataWidth;
	unsigned int mDataHeight;

	unsigned int mViewWidth;
	unsigned int mViewHeight;

	int mViewCenterX;
	int mViewCenterY;

	float mZoomStep;
};

BosonZoomScrollViewport::BosonZoomScrollViewport(QObject* parent)
	: QObject(parent)
{
 d = new BosonZoomScrollViewportPrivate;

 d->mDataWidth = 0;
 d->mDataHeight = 0;
 d->mZoomStep = 1.0f;
 d->mViewCenterX = 0;
 d->mViewCenterY = 0;
 d->mViewWidth = 1;
 d->mViewHeight = 1;
}

BosonZoomScrollViewport::~BosonZoomScrollViewport()
{
 delete d;
}

void BosonZoomScrollViewport::setDataSize(unsigned int w, unsigned int h)
{
 d->mDataWidth = w;
 d->mDataHeight = h;
}

unsigned int BosonZoomScrollViewport::dataWidth() const
{
 return d->mDataWidth;
}

unsigned int BosonZoomScrollViewport::dataHeight() const
{
 return d->mDataHeight;
}

void BosonZoomScrollViewport::setViewSize(unsigned int w, unsigned int h)
{
 d->mViewWidth = w;
 d->mViewHeight = h;

 // avoid possible divisions by zero
 d->mViewWidth = QMAX(d->mViewWidth, 1);
 d->mViewHeight = QMAX(d->mViewHeight, 1);

 centerViewOnDataPoint(d->mViewCenterX, d->mViewCenterY);
}


void BosonZoomScrollViewport::centerViewOnDataPoint(int x, int y)
{
 x = QMIN(x, (int)dataWidth() - 1);
 y = QMIN(y, (int)dataHeight() - 1);
 x = QMAX(x, 0);
 y = QMAX(y, 0);
 d->mViewCenterX = x;
 d->mViewCenterY = y;
}

int BosonZoomScrollViewport::xTranslation() const
{
 if (d->mViewWidth * 1.0f / zoomOutFactor() >= dataWidth()) {
	// we can display the whole data
	return 0;
 }
 int w2 = (int)((d->mViewWidth / 2) * 1.0f / zoomOutFactor());
 int ret = d->mViewCenterX - w2;
 ret = QMAX(ret, 0);
 ret = QMIN(ret, (int)(dataWidth() - d->mViewWidth * 1.0f / zoomOutFactor()));

 return -ret;
}

int BosonZoomScrollViewport::yTranslation() const
{
 if (d->mViewHeight * 1.0 / zoomOutFactor() >= dataHeight()) {
	// we can display the whole data
	return (int)(d->mViewHeight * 1.0f / zoomOutFactor()- dataHeight());
 }

 // flip y: 0 is bottom
 int realCenterY = (int)(dataHeight() - d->mViewCenterY) - 1;
 realCenterY = QMAX(realCenterY, 0);

 int h2 = (int)((d->mViewHeight / 2) * 1.0f / zoomOutFactor());
 int ret = realCenterY - h2;
 ret = QMAX(ret, 0);
 ret = QMIN(ret, (int)(dataHeight() - d->mViewHeight * 1.0f / zoomOutFactor()));

 return -ret;
}

int BosonZoomScrollViewport::viewCenterX() const
{
 return d->mViewCenterX;
}

int BosonZoomScrollViewport::viewCenterY() const
{
 return d->mViewCenterY;
}

void BosonZoomScrollViewport::zoomIn()
{
 d->mZoomStep /= 2.0f;
 d->mZoomStep = QMAX(d->mZoomStep, 0.125f);

 centerViewOnDataPoint(d->mViewCenterX, d->mViewCenterY);
}

void BosonZoomScrollViewport::zoomOut()
{
 d->mZoomStep *= 2.0f;

 centerViewOnDataPoint(d->mViewCenterX, d->mViewCenterY);
}

float BosonZoomScrollViewport::zoomOutFactor() const
{
 float f = 1.0f;
 if (d->mZoomStep > 0.001f) {
	if (d->mZoomStep > 1.0f) {
		int scaledW = (int)floor(d->mZoomStep * ((float)d->mViewWidth));
		int scaledH = (int)floor(d->mZoomStep * ((float)d->mViewHeight));
		int dw = ((int)dataWidth()) - scaledW;
		int dh = ((int)dataHeight()) - scaledH;
		if (dw < 0 && dh < 0) {
			float view = 0.0f;
			float maxRequired = 0.0f;
			if (dw >= dh) {
				view = (float)d->mViewWidth;
				maxRequired = (float)dataWidth();
			} else {
				view = (float)d->mViewHeight;
				maxRequired = (float)dataHeight();
			}

			// we search the smallest zoomStep, so that
			//   view * zoomStep >= maxRequired
			// is still satisfied.
			float zoomStep = maxRequired / view;
			if (zoomStep <= 0.0001f) { // error
				zoomStep = 1.0f;
			}
			d->mZoomStep = zoomStep;
		}
	}

	f = 1.0f / d->mZoomStep;
 }
 return f;
}

QPoint BosonZoomScrollViewport::widgetPointToDataPoint(const QPoint& pos) const
{
 if (d->mViewWidth <= 0 || d->mViewHeight <= 0) {
	return QPoint(-1, -1);
 }
 if (pos.x() < 0 || pos.y() < 0 || pos.x() >= (int)d->mViewWidth || pos.y() >= (int)d->mViewHeight) {
	return QPoint(-1, -1);
 }

 if (fabsf(zoomOutFactor()) <= 0.0001f) {
	return QPoint(-1, -1);
 }

 const int xTrans = -xTranslation();
 const int yTrans = -yTranslation();

 float zoomFactor = 1.0f / zoomOutFactor();

 int xCell = xTrans + (int)(pos.x() * zoomFactor);

 int yPos = d->mViewHeight - pos.y() - 1;
 int y = yTrans + (int)(yPos * zoomFactor);
 int yCell = dataHeight() - y - 1;

 if (xCell < 0 || (unsigned int)xCell >= dataWidth() ||
	yCell < 0 || (unsigned int)yCell >= dataHeight()) {
	return QPoint(-1, -1);
 }

 return QPoint(xCell, yCell);
}

BoMatrix BosonZoomScrollViewport::transformationMatrix() const
{
 BoMatrix matrix;

 matrix.scale(zoomOutFactor(), zoomOutFactor(), 1.0f);
 matrix.translate(xTranslation(), yTranslation(), 0.0f);
 matrix.scale((float)dataWidth(), (float)dataHeight(), 1.0f);

 return matrix;
}

