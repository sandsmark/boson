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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "bosonufominimap.h"
#include "bosonufominimap.moc"

#include "../../bomemory/bodummymemory.h"
#include "../gameengine/bosonmap.h"
#include "../bosonconfig.h"
#include "../bo3dtools.h"
#include "../gameengine/bosoncanvas.h"
#include "../bosonprofiling.h"
#include "../botexture.h"
#include "bodebug.h"
#include "bosonglminimapview.h"
#include "bosonufominimapdisplay.h"
#include "../boufo/boufopushbutton.h"
#include <bogl.h>

#include <klocale.h>
#include <kstandarddirs.h>

#include <qgl.h>
#include <qimage.h>
#include <qpointarray.h>
#include <qfileinfo.h>

#define ZOOM_STEP 0.5

class BosonUfoMiniMapPrivate
{
public:
	BosonUfoMiniMapPrivate()
	{
		mMiniMapDisplay = 0;
		mLogoTexture = 0;
	}
	QString mImageTheme;

	QPointArray mSelectionRect;

	QImage mLogo;
	QImage mZoomIn;
	QImage mZoomOut;
	QImage mZoomDefault;

	BosonUfoMiniMapDisplay* mMiniMapDisplay;
	BoTexture* mLogoTexture;
};

BosonUfoMiniMap::BosonUfoMiniMap()
	: BoUfoCustomWidget()
{
 setName("ufoglminimap");
 setLayoutClass(BoUfoWidget::UHBoxLayout);

 d = new BosonUfoMiniMapPrivate;
 d->mSelectionRect.resize(8);

 d->mMiniMapDisplay = new BosonUfoMiniMapDisplay();
 addWidget(d->mMiniMapDisplay);
 connect(d->mMiniMapDisplay, SIGNAL(signalReCenterView(const QPoint&)),
		this, SIGNAL(signalReCenterView(const QPoint&)));
 connect(d->mMiniMapDisplay, SIGNAL(signalMoveSelection(int, int)),
		this, SIGNAL(signalMoveSelection(int, int)));

 BoUfoWidget* buttons = new BoUfoWidget();
 buttons->setLayoutClass(BoUfoWidget::UVBoxLayout);
 addWidget(buttons);

#if 1
 BoUfoPushButton* zoomIn = new BoUfoPushButton(i18n("+"));
 connect(zoomIn, SIGNAL(signalClicked()),
		this, SLOT(slotZoomIn()));
 buttons->addWidget(zoomIn);
 BoUfoPushButton* zoomOut = new BoUfoPushButton(i18n("-"));
 connect(zoomOut, SIGNAL(signalClicked()),
		this, SLOT(slotZoomOut()));
 buttons->addWidget(zoomOut);
#endif

 setOpaque(false);

 setImageTheme(QString::fromLatin1("standard"));
 slotShowMiniMap(false);
}

BosonUfoMiniMap::~BosonUfoMiniMap()
{
 delete d->mLogoTexture;
 delete d;
}

BosonGLMiniMapView* BosonUfoMiniMap::miniMapView() const
{
 return d->mMiniMapDisplay->miniMapView();
}

void BosonUfoMiniMap::quitGame()
{
 setLocalPlayerIO(0);
 d->mMiniMapDisplay->quitGame();
}

void BosonUfoMiniMap::setLocalPlayerIO(PlayerIO* io)
{
 d->mMiniMapDisplay->setLocalPlayerIO(io);
}

bool BosonUfoMiniMap::showMiniMap() const
{
 return d->mMiniMapDisplay->showMiniMap();
}

void BosonUfoMiniMap::slotShowMiniMap(bool s)
{
 d->mMiniMapDisplay->setShowMiniMap(s);
}

void BosonUfoMiniMap::slotMoveRect(const QPoint& topLeft, const QPoint& topRight, const QPoint& bottomLeft, const QPoint& bottomRight)
{
 d->mSelectionRect.setPoint(0, topLeft);
 d->mSelectionRect.setPoint(1, topRight);

 d->mSelectionRect.setPoint(2, topRight);
 d->mSelectionRect.setPoint(3, bottomRight);

 d->mSelectionRect.setPoint(4, bottomRight);
 d->mSelectionRect.setPoint(5, bottomLeft);

 d->mSelectionRect.setPoint(6, bottomLeft);
 d->mSelectionRect.setPoint(7, topLeft);
}

void BosonUfoMiniMap::setImageTheme(const QString& theme)
{
 d->mImageTheme = theme;
 QImage image = imageFromTheme(QString::fromLatin1("minimap-logo.png"), theme);
 if (image.isNull()) {
	boError() << k_funcinfo << "Could not load minimap-logo.png from " << theme << endl;
	if (d->mLogo.isNull()) {
		// create a dummy pixmap to avoid a crash
		int w, h;
		if (mapWidth() > 0 && mapHeight() > 0) {
			w = mapWidth();
			h = mapHeight();
		} else {
			w = 100;
			h = 100;
		}
		d->mLogo = QImage(w, h, 32);
		d->mLogo.fill(Qt::red.rgb());
	}
 } else {
	d->mLogo = image;
 }
 d->mLogo.setAlphaBuffer(true);
 d->mLogo = QGLWidget::convertToGLFormat(d->mLogo);
 delete d->mLogoTexture;
 d->mLogoTexture = new BoTexture(d->mLogo.bits(),
		d->mLogo.width(), d->mLogo.height(),
		BoTexture::FilterLinear | BoTexture::FormatRGBA |
		BoTexture::DontCompress | BoTexture::DontGenMipmaps);
 d->mMiniMapDisplay->setLogoTexture(d->mLogoTexture);

 image = imageFromTheme(QString::fromLatin1("minimap-zoom-in.png"), theme);
 if (image.isNull()) {
	boError() << k_funcinfo << "Could not load minimap-zoom-in.png from " << theme << endl;
	d->mZoomIn = QImage(16, 16, 32);
 } else {
	d->mZoomIn = image;
 }

 image = imageFromTheme(QString::fromLatin1("minimap-zoom-out.png"), theme);
 if (image.isNull()) {
	boError() << k_funcinfo << "Could not load minimap-zoom-out.png from " << theme << endl;
	d->mZoomOut = QImage(16, 16, 32);
 } else {
	d->mZoomOut = image;
 }

 image = imageFromTheme(QString::fromLatin1("minimap-zoom-default.png"), theme);
 if (image.isNull()) {
	boError() << k_funcinfo << "Could not load minimap-zoom-default.png from " << theme << endl;
	d->mZoomDefault = QImage(16, 16, 32);
 } else {
	d->mZoomDefault = image;
 }
 setZoomImages(d->mZoomIn, d->mZoomOut, d->mZoomDefault);
}

void BosonUfoMiniMap::setZoomImages(const QImage& in_, const QImage& out_, const QImage& defaultZoom_)
{
 int w = in_.width();
 w = QMAX(w, out_.width());
 w = QMAX(w, defaultZoom_.width());
 int h = in_.height();
 h = QMAX(h, out_.height());
 h = QMAX(h, defaultZoom_.height());
 QImage in, out, defaultZoom;
 in = in_.smoothScale(w, h, QImage::ScaleMin);
 out = out_.smoothScale(w, h, QImage::ScaleMin);
 defaultZoom = defaultZoom_.smoothScale(w, h, QImage::ScaleMin);
#if 0
 // AB: we could use plib/pui here...
 // maybe we should link to plib? after all we already use fnt in boson and
 // could make use of at least pui.
 d->mZoomInButton->setImage(in);
 d->mZoomOutButton->setImage(out);
 d->mZoomDefaultButton->setImage(defaultZoom);
#endif
}

void BosonUfoMiniMap::slotZoomIn()
{
#if 0
 if (boConfig->doubleValue("MiniMapZoom") + ZOOM_STEP > 3.0) {
	return;
 }
 boConfig->setDoubleValue("MiniMapZoom", boConfig->doubleValue("MiniMapZoom") + ZOOM_STEP);
#endif
 if (miniMapView()) {
	miniMapView()->zoomIn();
 }
}

void BosonUfoMiniMap::slotZoomOut()
{
#if 0
 if (boConfig->doubleValue("MiniMapZoom") - ZOOM_STEP <= 0.1) {
	return;
 }
 boConfig->setDoubleValue("MiniMapZoom", boConfig->doubleValue("MiniMapZoom") - ZOOM_STEP);
#endif
 if (miniMapView()) {
	miniMapView()->zoomOut();
 }
}

void BosonUfoMiniMap::slotZoomDefault()
{
 boConfig->setDoubleValue("MiniMapZoom", 1.0);
}

QImage BosonUfoMiniMap::imageFromTheme(const QString& file, const QString& theme) const
{
 QString f = locate("data", QString::fromLatin1("boson/themes/ui/%1/%2").arg(theme).arg(file));
 QFileInfo info(f);
 if (!info.exists()) {
	f = locate("data", QString::fromLatin1("boson/themes/ui/%1/%2").arg(QString::fromLatin1("standard").arg(file)));
	info.setFile(f);
	if (!info.exists()) {
		boError() << k_funcinfo << "Can't find " << f << " in " << theme << " or standard" << endl;
		return QImage();
	}
 }
 return QImage(f);
}


void BosonUfoMiniMap::createMap(BosonCanvas* c, const BoGLMatrices* gameGLMatrices)
{
 BO_CHECK_NULL_RET(c);

 d->mMiniMapDisplay->createMap(c, gameGLMatrices);
 if (!d->mImageTheme.isEmpty()) {
	setImageTheme(d->mImageTheme);
 }
}

void BosonUfoMiniMap::slotAdvance(unsigned int advanceCallsCount)
{
 if (miniMapView()) {
	miniMapView()->slotAdvance(advanceCallsCount);
 }
}

void BosonUfoMiniMap::slotUpdateTerrainAtCorner(int x, int y)
{
 if (miniMapView()) {
	miniMapView()->slotUpdateTerrainAtCorner(x, y);
 }
}

void BosonUfoMiniMap::slotExplored(int x, int y)
{
 if (miniMapView()) {
	miniMapView()->slotExplored(x, y);
 }
}

void BosonUfoMiniMap::slotUnexplored(int x, int y)
{
 if (miniMapView()) {
	miniMapView()->slotUnexplored(x, y);
 }
}

void BosonUfoMiniMap::initFogOfWar(PlayerIO* p)
{
 if (miniMapView()) {
	miniMapView()->initFogOfWar(p);
 }
}

unsigned int BosonUfoMiniMap::mapWidth() const
{
 if (miniMapView()) {
	return miniMapView()->mapWidth();
 }
 return 0;
}

unsigned int BosonUfoMiniMap::mapHeight() const
{
 if (miniMapView()) {
	return miniMapView()->mapHeight();
 }
 return 0;
}


