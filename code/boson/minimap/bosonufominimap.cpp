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
		mLocalPlayerIO = 0;
		mCanvas = 0;

		mLogoTexture = 0;
		mMiniMapView = 0;
	}
	PlayerIO* mLocalPlayerIO;
	BosonCanvas* mCanvas;

	QString mImageTheme;

	QPointArray mSelectionRect;

	float mZoom;

	bool mShowMiniMap;
	QImage mLogo;
	QImage mZoomIn;
	QImage mZoomOut;
	QImage mZoomDefault;

	BoTexture* mLogoTexture;
	BosonGLMiniMapView* mMiniMapView;

};

BosonUfoMiniMap::BosonUfoMiniMap()
	: BoUfoCustomWidget()
{
 d = new BosonUfoMiniMapPrivate;
 d->mShowMiniMap = false;
 d->mZoom = 1.0f;
 d->mSelectionRect.resize(8);

 setName("ufoglminimap");

 setOpaque(false);
 setMouseEventsEnabled(true, true);

 setImageTheme(QString::fromLatin1("standard"));
 slotShowMiniMap(false);

 setPreferredWidth(150);
 setPreferredHeight(150);
 setMinimumWidth(150);
 setMinimumHeight(150);
 setSize(150, 150);


 connect(this, SIGNAL(signalMouseMoved(QMouseEvent*)),
		this, SLOT(slotMouseEvent(QMouseEvent*)));
 connect(this, SIGNAL(signalMouseDragged(QMouseEvent*)),
		this, SLOT(slotMouseEvent(QMouseEvent*)));
 connect(this, SIGNAL(signalMousePressed(QMouseEvent*)),
		this, SLOT(slotMouseEvent(QMouseEvent*)));
 connect(this, SIGNAL(signalMouseReleased(QMouseEvent*)),
		this, SLOT(slotMouseEvent(QMouseEvent*)));
// connect(this, SIGNAL(signalMouseClicked(ufo::UMouseEvent*)),
//		this, SLOT(slotMouseEvent(ufo::UMouseEvent*)));

 connect(this, SIGNAL(signalWidgetResized()),
		this, SLOT(slotWidgetResized()));

}

BosonUfoMiniMap::~BosonUfoMiniMap()
{
 boDebug() << k_funcinfo << endl;
 delete d->mMiniMapView;
 delete d->mLogoTexture;
 delete d;
}

void BosonUfoMiniMap::quitGame()
{
 setLocalPlayerIO(0);
 d->mCanvas = 0;
}

void BosonUfoMiniMap::setLocalPlayerIO(PlayerIO* io)
{
 d->mLocalPlayerIO = io;
 if (d->mMiniMapView) {
	d->mMiniMapView->setLocalPlayerIO(io);
 }
 if (!io) {
	slotShowMiniMap(false);
 }
}

bool BosonUfoMiniMap::showMiniMap() const
{
 return d->mShowMiniMap;
}

void BosonUfoMiniMap::slotShowMiniMap(bool s)
{
 d->mShowMiniMap = s;
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

float BosonUfoMiniMap::zoom() const
{
 return 1.0f;
}

void BosonUfoMiniMap::setImageTheme(const QString& theme)
{
 d->mImageTheme = theme;
 if (!d->mMiniMapView) {
	return;
 }
 QImage image = imageFromTheme(QString::fromLatin1("minimap-logo.png"), theme);
 if (image.isNull()) {
	boError() << k_funcinfo << "Could not load minimap-logo.png from " << theme << endl;
	if (d->mLogo.isNull()) {
		// create a dummy pixmap to avoid a crash
		int w, h;
		if (canvas()) {
			w = canvas()->mapWidth();
			h = canvas()->mapHeight();
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
 if (boConfig->doubleValue("MiniMapZoom") + ZOOM_STEP > 3.0) {
	return;
 }
 boConfig->setDoubleValue("MiniMapZoom", boConfig->doubleValue("MiniMapZoom") + ZOOM_STEP);
}

void BosonUfoMiniMap::slotZoomOut()
{
 if (boConfig->doubleValue("MiniMapZoom") - ZOOM_STEP <= 0.1) {
	return;
 }
 boConfig->setDoubleValue("MiniMapZoom", boConfig->doubleValue("MiniMapZoom") - ZOOM_STEP);
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
 boDebug() << k_funcinfo << endl;
 d->mCanvas = c;

 delete d->mMiniMapView;
 d->mMiniMapView = new BosonGLMiniMapView(gameGLMatrices, this);
 d->mMiniMapView->setMiniMapScreenSize(width(), height());
 d->mMiniMapView->setCanvas(canvas());
 d->mMiniMapView->setLocalPlayerIO(localPlayerIO());
 d->mMiniMapView->createMap(c->mapWidth(), c->mapHeight());
 if (!d->mImageTheme.isEmpty()) {
	setImageTheme(d->mImageTheme);
 }
 boDebug() << k_funcinfo << "done" << endl;
}

void BosonUfoMiniMap::render()
{
 if (!canvas() || !d->mMiniMapView) {
	return;
 }

// d->mMiniMapView->setZoom(zoom());

 if (d->mShowMiniMap) {
	renderMiniMap();
 } else {
	renderLogo();
 }
}

void BosonUfoMiniMap::renderMiniMap()
{
 d->mMiniMapView->render();
}

void BosonUfoMiniMap::renderLogo()
{
 BO_CHECK_NULL_RET(d->mLogoTexture);
 glPushAttrib(GL_ENABLE_BIT);
 glEnable(GL_TEXTURE_2D);
 glColor3ub(255, 255, 255);
 d->mLogoTexture->bind();

 glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(0.0f, 0.0f);

	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(1.0f, 0.0f);

	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(1.0f, 1.0f);

	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(0.0f, 1.0f);
 glEnd();

 glPopAttrib();
 boTextureManager->invalidateCache();
}

BosonCanvas* BosonUfoMiniMap::canvas() const
{
 return d->mCanvas;
}

PlayerIO* BosonUfoMiniMap::localPlayerIO() const
{
 return d->mLocalPlayerIO;
}

void BosonUfoMiniMap::slotMouseEvent(QMouseEvent* e)
{
 QPoint pos = e->pos();

 // AB: when using click+move, the coordinates may go off this widget. we don't
 // want this.
 pos.setX(QMAX(0, pos.x()));
 pos.setY(QMAX(0, pos.y()));
 pos.setX(QMIN(pos.x(), width()));
 pos.setY(QMIN(pos.y(), height()));

 QPoint cell = widgetToCell(pos);

 // we accept all mouse events except mousemove events. this means that only
 // mouse move events are propagated to the parent (necessary for updating
 // cursor position)
 switch (e->type()) {
	case QMouseEvent::MouseMove:
		if (!(e->state() & Qt::LeftButton)) {
			// MouseMove is ignored when LMB is not pressed only
			e->ignore();
		} else {
			e->accept();
		}
		break;
	default:
		e->accept();
		break;
 }

 if (!showMiniMap()) {
	return;
 }

 int button = e->button();
 switch (e->type()) {
	case QMouseEvent::MouseMove:
		button = Qt::NoButton;
		if (e->state() & Qt::LeftButton) {
			button = Qt::LeftButton;
		} else {
			break;
		}
		// fall through intended, for LMB+Move
	case QMouseEvent::MouseButtonPress:
	{
		if (button == Qt::LeftButton) {
			emit signalReCenterView(cell);
		} else if (button == Qt::RightButton) {
			emit signalMoveSelection(cell.x(), cell.y());
		}
		break;
	}
	case QMouseEvent::MouseButtonRelease:
		break;
//	case QMouseEvent::MouseClicked:
//		break;
//	case QMouseEvent::MouseDragged:
//		break;
	default:
		break;
 }
}

QPoint BosonUfoMiniMap::widgetToCell(const QPoint& pos)
{
 // TODO: the correct cell depends on the modelview matrix of the minimap (zooming, ...). we should use something like d->mGLMiniMap->widgetToCell(), which would take the mModelView of the minimap into account.
 BosonMap* map = canvas()->map();
 if (!map) {
	return QPoint();
 }
 if (width() == 0 || height() == 0) {
	return QPoint();
 }
 QPoint cell = QPoint((pos.x() * map->width()) / width(),
		(pos.y() * map->height()) / height());
 return cell;
}

void BosonUfoMiniMap::paintWidget()
{
 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "GL error at the beginning of this method" << endl;
 }
 PROFILE_METHOD
 boTextureManager->invalidateCache();
 glPushMatrix();
 glTranslatef(0.0f, (float)height(), 0.0f);
 glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
 glScalef((float)width(), (float)height(), 0.0f);
 render();
 glPopMatrix();
 boTextureManager->invalidateCache();
 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "GL error at the end of this method" << endl;
 }
}

void BosonUfoMiniMap::slotAdvance(unsigned int advanceCallsCount)
{
 if (d->mMiniMapView) {
	d->mMiniMapView->slotAdvance(advanceCallsCount);
 }
}

void BosonUfoMiniMap::slotUnitMoved(Unit* unit, bofixed oldX, bofixed oldY)
{
 if (d->mMiniMapView) {
	d->mMiniMapView->slotUnitMoved(unit, oldX, oldY);
 }
}

void BosonUfoMiniMap::slotUnitRemoved(Unit* unit)
{
 if (d->mMiniMapView) {
	d->mMiniMapView->slotUnitRemoved(unit);
 }
}

void BosonUfoMiniMap::slotItemAdded(BosonItem* item)
{
 if (d->mMiniMapView) {
	d->mMiniMapView->slotItemAdded(item);
 }
}

void BosonUfoMiniMap::slotFacilityConstructed(Unit* fac)
{
 if (d->mMiniMapView) {
	d->mMiniMapView->slotFacilityConstructed(fac);
 }
}

void BosonUfoMiniMap::slotUpdateTerrainAtCorner(int x, int y)
{
 if (d->mMiniMapView) {
	d->mMiniMapView->slotUpdateTerrainAtCorner(x, y);
 }
}

void BosonUfoMiniMap::slotExplored(int x, int y)
{
 if (d->mMiniMapView) {
	d->mMiniMapView->slotExplored(x, y);
 }
}

void BosonUfoMiniMap::slotUnexplored(int x, int y)
{
 if (d->mMiniMapView) {
	d->mMiniMapView->slotUnexplored(x, y);
 }
}

void BosonUfoMiniMap::initFogOfWar(PlayerIO* p)
{
 if (d->mMiniMapView) {
	d->mMiniMapView->initFogOfWar(p);
 }
}

void BosonUfoMiniMap::slotWidgetResized()
{
 if (d->mMiniMapView) {
	d->mMiniMapView->setMiniMapScreenSize(width(), height());
 }
}

