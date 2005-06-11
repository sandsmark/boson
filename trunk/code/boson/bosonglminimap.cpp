/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2005 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bosonglminimap.h"
#include "bosonglminimap.moc"

#include "defines.h"
#include "cell.h"
#include "bosonmap.h"
#include "bosonconfig.h"
#include "bosongroundtheme.h"
#include "boitemlist.h"
#include "bo3dtools.h"
#include "unit.h"
#include "player.h"
#include "playerio.h"
#include "bodebug.h"
#include "bosonprofiling.h"
#include "botexture.h"
#include "bosonglwidget.h"
#include "bowater.h"
#include <bogl.h>

#include <klocale.h>
#include <kstandarddirs.h>

#include <qpixmap.h>
#include <qimage.h>
#include <qpointarray.h>
#include <qvaluelist.h>
#include <qfileinfo.h>
#include <qptrvector.h>
#include <qgl.h>

#define COLOR_UNKNOWN Qt::black // fog of war
#define ZOOM_STEP 0.5

/**
 * This returns the intersection of a line and a plane in @p intersection. Note
 * that a line always has infinite length - line segments (finite length) are
 * not handled here.
 *
 * This function assumes that an intersection actually exists, as this is what
 * we need in this file. Checking whether an intersection exists, is pretty
 * easy.
 *
 * @param linePoint Just a point on the line
 * @param lineVector The line vector, i.e. the direction of the line. This is
 * returned by @ref planes_intersect or can easily be calculated using (p0-p1)
 * if p0 and p1 are two different points on the line.
 **/
static void plane_line_intersect(const BoPlane& plane, const BoVector3Float& linePoint, BoVector3Float& lineVector, BoVector3Float* intersection);

class BosonGLMiniMapPrivate
{
public:
	BosonGLMiniMapPrivate()
	{
	}
	QString mImageTheme;

	QPointArray mSelectionRect;

	float mZoom;

	bool mShowMiniMap;
	QImage mLogo;
	QImage mZoomIn;
	QImage mZoomOut;
	QImage mZoomDefault;
};

BosonGLMiniMap::BosonGLMiniMap(QObject* parent, const char* name) : QObject(parent, name ? name : "glminimap")
{
 d = new BosonGLMiniMapPrivate;
 d->mShowMiniMap = false;
 d->mZoom = 1.0f;
 d->mSelectionRect.resize(8);

 mRenderer = 0;
 mLocalPlayerIO = 0;
 mMap = 0;

 setImageTheme(QString::fromLatin1("standard"));
 slotShowMiniMap(false);
}

BosonGLMiniMap::~BosonGLMiniMap()
{
 boDebug() << k_funcinfo << endl;
 delete mRenderer;
 delete d;
}

void BosonGLMiniMap::quitGame()
{
 setLocalPlayerIO(0);
 delete mRenderer;
 mRenderer = 0;
 mMap = 0;
}

void BosonGLMiniMap::setLocalPlayerIO(PlayerIO* io)
{
 mLocalPlayerIO = io;
 if (!io) {
	slotShowMiniMap(false);
 }
}

bool BosonGLMiniMap::showMiniMap() const
{
 return d->mShowMiniMap;
}

void BosonGLMiniMap::slotShowMiniMap(bool s)
{
 d->mShowMiniMap = s;
}

void BosonGLMiniMap::slotMoveRect(const QPoint& topLeft, const QPoint& topRight, const QPoint& bottomLeft, const QPoint& bottomRight)
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

void BosonGLMiniMap::slotUnitMoved(Unit* unit, bofixed oldX, bofixed oldY)
{
 BO_CHECK_NULL_RET(unit);
 QPtrVector<Cell> newCells;
 QPtrVector<Cell> oldCells;
 makeCellList(&newCells, unit, unit->x(), unit->y());
 makeCellList(&oldCells, unit, oldX, oldY);
 moveUnit(unit, &newCells, &oldCells);
}

void BosonGLMiniMap::slotUnitDestroyed(Unit* unit)
{
 BO_CHECK_NULL_RET(unit)
 QPtrVector<Cell> cells;
 makeCellList(&cells, unit, unit->x(), unit->y());
 moveUnit(unit, &cells, 0);
}

void BosonGLMiniMap::makeCellList(QPtrVector<Cell>* cells, const Unit* unit, bofixed x, bofixed y)
{
 BO_CHECK_NULL_RET(map());
 BO_CHECK_NULL_RET(unit);
 bofixed right = QMIN(x + unit->width(), bofixed(map()->width()));
 bofixed bottom = QMIN(y + unit->height(), bofixed(map()->height()));
 BosonItem::makeCells(map()->cells(), cells, BoRectFixed(x, y, right, bottom), map()->width(), map()->height());
}


void BosonGLMiniMap::moveUnit(Unit* unit, const QPtrVector<Cell>* newCells, const QPtrVector<Cell>* oldCells)
{
 // all parameters use cell coordinates!
 // note that using unit->x() and unit->y() as well as unit->cells() and such
 // stuff can be undefined at this point! especially when adding units
 // (oldX==oldY==-1)!
 BO_CHECK_NULL_RET(localPlayerIO());
 BO_CHECK_NULL_RET(map());
 BO_CHECK_NULL_RET(unit);
 BO_CHECK_NULL_RET(newCells);
 if (oldCells && newCells->count() == oldCells->count()) {
	if (!unit->isDestroyed()) {
		bool moved = false;
		for (unsigned int i = 0; i < newCells->count() && !moved; i++) {
			if (newCells->at(i) != oldCells->at(i)) {
				moved = true;
			}
		}
		if (!moved) {
			// Unit is still on the same cells. Don't update (performance)
			return;
		}
	} else {
		// don't return - probably remove unit from the minimap
	}
 }

 if (oldCells && oldCells->count() != 0) {
	// unit is moving.
	// pretty much everything can happen here now. the cell that the unit
	// left can be fogged for the local player, can have another unit, ...
	// so we need to update all cells that the unit has left.
	for (unsigned int i = 0; i < oldCells->count(); i++) {
		bool found = false;
		for (unsigned int j = 0; j < newCells->count(); j++) {
			if (newCells->at(j) == oldCells->at(i)) {
				found = true;
				break;
			}
		}
		if (!found) {
			Cell* c = oldCells->at(i);
			if (c) {
				updateCell(c->x(), c->y());
			}
		}
	}
 }
 QColor color = unit->owner()->teamColor();
 for (unsigned int i = 0; i < newCells->count(); i++) {
	Cell* c = newCells->at(i);
	if (!c) {
		continue;
	}
	int x = c->x();
	int y = c->y();
	if (localPlayerIO() && !localPlayerIO()->canSee(x, y)) {
		// we don't call slotFog() here now, as it should be fogged
		// already. maybe we should do this anyway?
		continue;
	}
	if (!unit->isDestroyed()) {
		setPoint(x, y, color);
	} else {
		updateCell(x, y);
	}
 }
}

void BosonGLMiniMap::slotUpdateCell(int x, int y)
{
 updateCell(x, y);
}

void BosonGLMiniMap::updateCell(int x, int y)
{
 BO_CHECK_NULL_RET(map());
 BO_CHECK_NULL_RET(groundTheme());
 if (!map()->isValidCell(x, y)) {
	boError() << k_funcinfo << x << "," << y << " is no valid cell!" << endl;
	return;
 }
 Cell* cell = map()->cell(x, y);
 // AB: note that localPlayerIO() == NULL is valid in editor mode here!
 if (localPlayerIO()) {
	if (localPlayerIO()->isFogged(x, y)) {
		slotFog(x, y);
		return;
	}
 }
 QValueList<Unit*> list = cell->items()->units(false);
 if (list.isEmpty()) {
	calculateGround(x, y);
 } else {
	Unit* u = list.first();
	QPtrVector<Cell> cells;
	makeCellList(&cells, u, u->x(), u->y());
	moveUnit(u, &cells, 0);
 }
}


void BosonGLMiniMap::initFogOfWar(PlayerIO* p)
{
 BO_CHECK_NULL_RET(map());
 boDebug() << k_funcinfo << endl;

 if (mRenderer) {
	mRenderer->setUpdatesEnabled(false);
 }

 // AB: add a global PlayerIO to the editor. only use non-NULL IOs here then.
 if (!p) {
	// a NULL playerIO means that we should display the complete map. fog of
	// war gets disabled.
	for (unsigned int i = 0; i < map()->width(); i++) {
		for (unsigned int j = 0; j < map()->height(); j++) {
			slotUnfog(i, j);
		}
	}
 } else {
	for (unsigned int i = 0; i < map()->width(); i++) {
		for (unsigned int j = 0; j < map()->height(); j++) {
			if (p && !p->canSee(i, j)) {
				slotFog(i, j);
			} else {
				slotUnfog(i, j);
			}
		}
	}
 }

 if (mRenderer) {
	mRenderer->setUpdatesEnabled(true);
 }
}

float BosonGLMiniMap::zoom() const
{
 return 1.0f;
}

BosonGroundTheme* BosonGLMiniMap::groundTheme() const
{
 if (!map()) {
	return 0;
 }
 return map()->groundTheme();
}

void BosonGLMiniMap::calculateGround(int x, int y)
{
 BO_CHECK_NULL_RET(groundTheme());
 BO_CHECK_NULL_RET(map());
 BO_CHECK_NULL_RET(map()->texMap());
 if (!map()->isValidCell(x, y)) {
	return;
 }
 if (localPlayerIO() && !localPlayerIO()->cell(x, y)) {
	return;
 }

 // every cell has four corners - we mix them together to get the actual minimap
 // color.
 unsigned int cornerX[4] = { x, x + 1, x + 1,     x };
 unsigned int cornerY[4] = { y,     y, y + 1, y + 1 };
 int r = 0;
 int g = 0;
 int b = 0;
 for (int j = 0; j < 4; j++) {
	int alphaSum = 0; // sum of all textures
	int cornerRed = 0;
	int cornerGreen = 0;
	int cornerBlue = 0;
	if (boWaterManager->underwater(cornerX[j], cornerY[j])) {
		// Water is dark-blue, a bit greenish
		cornerRed += 0;
		cornerGreen += 64;
		cornerBlue += 192;
		alphaSum += 255;
	} else {
		for (unsigned int i = 0; i < map()->groundTheme()->groundTypeCount(); i++) {
			int alpha = (int)map()->texMapAlpha(i, cornerX[j], cornerY[j]);
			alphaSum += alpha;

			QRgb rgb = map()->miniMapColor(i);
			int red = qRed(rgb);
			int green = qGreen(rgb);
			int blue = qBlue(rgb);
			cornerRed += red * alpha / 255;
			cornerGreen += green * alpha / 255;
			cornerBlue += blue * alpha / 255;
		}
	}
	if (alphaSum == 0) {
		// nothing to do for this corner.
		continue;
	}
	cornerRed = cornerRed * 255 / alphaSum;
	cornerGreen = cornerGreen * 255 / alphaSum;
	cornerBlue = cornerBlue * 255 / alphaSum;

	r += cornerRed;
	g += cornerGreen;
	b += cornerBlue;
 }

 r /= 4;
 g /= 4;
 b /= 4;

 setPoint(x, y, QColor(r, g, b));
}

void BosonGLMiniMap::setPoint(int x, int y, const QColor& color)
{
 BO_CHECK_NULL_RET(mRenderer);
 mRenderer->setPoint(x, y, color);
}

void BosonGLMiniMap::setImageTheme(const QString& theme)
{
 d->mImageTheme = theme;
 if (!mRenderer) {
	// this method will be called again once a renderer got created
	return;
 }
 QImage image = imageFromTheme(QString::fromLatin1("minimap-logo.png"), theme);
 if (image.isNull()) {
	boError() << k_funcinfo << "Could not load minimap-logo.png from " << theme << endl;
	if (d->mLogo.isNull()) {
		// create a dummy pixmap to avoid a crash
		int w, h;
		if (mMap) {
			w = mMap->width();
			h = mMap->height();
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
 mRenderer->setLogo(d->mLogo);

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
 mRenderer->setZoomImages(d->mZoomIn, d->mZoomOut, d->mZoomDefault);
}

void BosonGLMiniMap::slotZoomIn()
{
 if (boConfig->doubleValue("MiniMapZoom") + ZOOM_STEP > 3.0) {
	return;
 }
 boConfig->setDoubleValue("MiniMapZoom", boConfig->doubleValue("MiniMapZoom") + ZOOM_STEP);
}

void BosonGLMiniMap::slotZoomOut()
{
 if (boConfig->doubleValue("MiniMapZoom") - ZOOM_STEP <= 0.1) {
	return;
 }
 boConfig->setDoubleValue("MiniMapZoom", boConfig->doubleValue("MiniMapZoom") - ZOOM_STEP);
}

void BosonGLMiniMap::slotZoomDefault()
{
 boConfig->setDoubleValue("MiniMapZoom", 1.0);
}

QImage BosonGLMiniMap::imageFromTheme(const QString& file, const QString& theme) const
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


void BosonGLMiniMap::createMap(BosonMap* map, const BoGLMatrices* gameGLMatrices)
{
 BO_CHECK_NULL_RET(map);
 boDebug() << k_funcinfo << endl;
 mMap = map;
 delete mRenderer;
 mRenderer = new BosonGLMiniMapRenderer(gameGLMatrices);
 mRenderer->createMap(map->width(), map->height(), map->groundTheme());
 if (!d->mImageTheme.isEmpty()) {
	setImageTheme(d->mImageTheme);
 }
}

void BosonGLMiniMap::renderMiniMap()
{
 BO_CHECK_NULL_RET(mRenderer);

 // AB: we set all settings in the renderer in every frame. this is a small
 // overhead (a few set*() functions) but saves us from a bigger overhead (and
 // more probable bugs) in a correct settings framework.
 //
 // also it will make maintaining the renderer a lot more easy - changing the
 // renderer code does not necessarily involve modifying the settings framework
 // this way.
 if (d->mShowMiniMap) {
	mRenderer->setType(BosonGLMiniMapRenderer::MiniMap);
 } else {
	mRenderer->setType(BosonGLMiniMapRenderer::Logo);
 }
 mRenderer->setZoom(zoom());
 mRenderer->setAlignment(Qt::Left | Qt::Top);

 mRenderer->render();
}

unsigned int BosonGLMiniMap::miniMapWidth() const
{
 if (!mRenderer) {
	return 0;
 }
 return mRenderer->miniMapWidth();
}

unsigned int BosonGLMiniMap::miniMapHeight() const
{
 if (!mRenderer) {
	return 0;
 }
 return mRenderer->miniMapHeight();
}

void BosonGLMiniMap::slotUnfog(int x, int y)
{
 if (!localPlayerIO()) {
	// don't use fog of war at all (editor mode)
	return;
 }
 if (!map()) {
	return;
 }
 BO_CHECK_NULL_RET(groundTheme());
 if (!localPlayerIO()->isValidCell(x, y)) {
	boError() << k_funcinfo << "invalid cell " << x << "," << y << endl;
	return;
 }
 Cell* cell = localPlayerIO()->cell(x, y);
 if (!cell) {
	// should not happen anymore! must be unfogged already!
	boError() << k_funcinfo << "cannot unfog cell for minimap - it is still fogged for the player!" << endl;
	return;
 }
 QValueList<Unit*> list = cell->items()->units(false);
 if (!list.isEmpty()) {
	Unit* u = list.first();
	QPtrVector<Cell> cells;
	makeCellList(&cells, u, u->x(), u->y());
	moveUnit(u, &cells, 0);
 } else {
	calculateGround(x, y);
 }
}

void BosonGLMiniMap::slotFog(int x, int y)
{
 if (!localPlayerIO()) {
	// don't use fog of war at all (editor mode)
	return;
 }
 if (!map()) {
	return;
 }
 if (!localPlayerIO()->isValidCell(x, y)) {
	boError() << k_funcinfo << "invalid cell " << x << "," << y << endl;
	return;
 }
 setPoint(x, y, COLOR_UNKNOWN);
}

bool BosonGLMiniMap::mouseEvent(KGameIO*, QDataStream&, QMouseEvent* e, bool* send)
{
 *send = false;
 static int state = Qt::NoButton;
 if (!mRenderer) {
	state = Qt::NoButton;
	return false;
 }
 if (!d->mShowMiniMap) {
	state = Qt::NoButton;
	return false;
 }
 QPoint cell;
 bool inside = mRenderer->windowToCell(e->pos(), &cell);

 if (!inside) {
	state = state & QEvent::MouseButtonMask; // we dont care about keyboard buttons
	if (state != Qt::NoButton) { // the minimap is responsible for this event
		if (e->type() == QEvent::MouseButtonRelease) {
			// we grab these, but dont use them
			state = state & ~e->button();
			return true;
		}
		return true;
	}
	return false;
 }

 // the mouse is in the minimap, but the user is pressing a button down that was
 // not pressed inside the minimap. dont catch it
 if (state != e->state()) {
	return false;
 }
 switch (e->type()) {
	case QEvent::MouseButtonRelease:
		state = e->stateAfter();
		return true;
	case QEvent::MouseButtonPress:
		state = e->state() | e->button();
		break;
	case QEvent::MouseMove:
		if (e->state() == state && state != Qt::NoButton) {
			return true;
		} else {
			return false;
		}
		break;
	case QEvent::MouseButtonDblClick:
		state = e->state() | e->button();
		return true;
		break;
	default:
		return true;
 }
 if (e->type() != QEvent::MouseButtonPress) {
	boWarning() << k_funcinfo << "oops - not a press event?!" << endl;
	state = e->state();
	return true;
 }
 if (e->button() == Qt::LeftButton) {
	emitSignalReCenterView(cell);
	return true;
 }
 if (e->button() == Qt::RightButton) {
	emitSignalMoveSelection(cell);
	return true;
 }
 return false;
}

void BosonGLMiniMap::emitSignalReCenterView(const QPoint& cell)
{
 emit signalReCenterView(cell);
}

void BosonGLMiniMap::emitSignalMoveSelection(const QPoint& cell)
{
 emit signalMoveSelection(cell.x(), cell.y());
}



class BosonGLMiniMapRendererPrivate
{
public:
	BosonGLMiniMapRendererPrivate()
	{
		mGameGLMatrices = 0;
		mMapTexture = 0;
		mGLMapTexture = 0;

		mLogoTexture = 0;
	}
	BoMatrix mModelviewMatrix;
	QImage mOrigLogo;
	const BoGLMatrices* mGameGLMatrices;

	GLubyte* mMapTexture;
	int mMapTextureWidth;
	int mMapTextureHeight;
	BoTexture* mGLMapTexture;

	BoTexture* mLogoTexture;


	bool mUpdatesEnabled;
	int mMiniMapChangesSinceRendering;
};

BosonGLMiniMapRenderer::BosonGLMiniMapRenderer(const BoGLMatrices* gameGLMatrices)
{
 d = new BosonGLMiniMapRendererPrivate;

 d->mGameGLMatrices = gameGLMatrices;

 mMapWidth = 0;
 mMapHeight = 0;
 mUseFog = true;
 mType = Logo;
 mTextureMaxWidth = 1.0f;
 mTextureMaxHeight = 1.0f;
 mMiniMapWidth = 0;
 mMiniMapHeight = 0;
 d->mMapTextureWidth = 0;
 d->mMapTextureHeight = 0;
 d->mUpdatesEnabled = true;
 d->mMiniMapChangesSinceRendering = 0;

 mPosX = distanceFromEdge();
 mPosY = distanceFromEdge();

 // default size of the displayed minimap quad
 setMiniMapSize(150, 150);
}

BosonGLMiniMapRenderer::~BosonGLMiniMapRenderer()
{
 delete d->mGLMapTexture;
 delete[] d->mMapTexture;
 delete d->mLogoTexture;
 delete d;
}

void BosonGLMiniMapRenderer::setUpdatesEnabled(bool e)
{
 d->mUpdatesEnabled = e;
 if (e) {
	delete d->mGLMapTexture;
	d->mGLMapTexture = new BoTexture(d->mMapTexture,
			d->mMapTextureWidth, d->mMapTextureHeight,
			BoTexture::FilterLinear | BoTexture::FormatRGBA |
			BoTexture::DontCompress | BoTexture::DontGenMipmaps);
 }
}

void BosonGLMiniMapRenderer::setMiniMapSize(unsigned int width, unsigned int height)
{
 mMiniMapWidth = width;
 mMiniMapHeight = height;
}

void BosonGLMiniMapRenderer::createMap(unsigned int w, unsigned int h, BosonGroundTheme* theme)
{
 BO_CHECK_NULL_RET(theme);

 mMapWidth = w;
 mMapHeight = h;

 unsigned int w2 = BoTexture::nextPower2(mMapWidth);
 unsigned int h2 = BoTexture::nextPower2(mMapHeight);
 mTextureMaxWidth = (float)w / (float)w2;
 mTextureMaxHeight = (float)h / (float)h2;

 delete[] d->mMapTexture;
 d->mMapTexture = new GLubyte[w2 * h2 * 4];
 d->mMapTextureWidth = w2;
 d->mMapTextureHeight = h2;

 for (int y = 0; y < d->mMapTextureHeight; y++) {
	for (int x = 0; x < d->mMapTextureWidth; x++) {
		d->mMapTexture[(y * w + x) * 4 + 0] = 0;
		d->mMapTexture[(y * w + x) * 4 + 1] = 0;
		d->mMapTexture[(y * w + x) * 4 + 2] = 0;
		d->mMapTexture[(y * w + x) * 4 + 3] = 0;
	}
 }

 // AB: d->mMapTexture has been resized to 2^n * 2^m. replace the alpha values for
 // all coordinates that are relevant to us.
 for (unsigned int x = 0; x < w; x++) {
	for (unsigned int y = 0; y < h; y++) {
		// FIXME: endianness?
		d->mMapTexture[(y * w + x) * 4 + 3] = 0;
	}
 }

 setUpdatesEnabled(false);
 setUpdatesEnabled(true);
}

void BosonGLMiniMapRenderer::render()
{
 glColor3ub(255, 255, 255);
 d->mMiniMapChangesSinceRendering = 0;
 renderGimmicks();
 switch (mType) {
	case MiniMap:
		renderMiniMap();
		break;
	case Logo:
		renderLogo();
		break;
	default:
		boError() << k_funcinfo << "invalid type " << (int)mType << endl;
 }
}

void BosonGLMiniMapRenderer::renderGimmicks()
{
}

void BosonGLMiniMapRenderer::renderLogo()
{
 BO_CHECK_NULL_RET(d->mLogoTexture);
 glPushAttrib(GL_ENABLE_BIT);
 glEnable(GL_TEXTURE_2D);
 d->mLogoTexture->bind();

 // AB: note that renderQuad() is for renderMiniMap() only, not for this one.
 glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2i(0, 0);

	glTexCoord2f(1.0f, 0.0f);
	glVertex2i(miniMapWidth(), 0);

	glTexCoord2f(1.0f, 1.0f);
	glVertex2i(miniMapWidth(), miniMapHeight());

	glTexCoord2f(0.0f, 1.0f);
	glVertex2i(0, miniMapHeight());
 glEnd();

 glPopAttrib();
 boTextureManager->invalidateCache();
}

void BosonGLMiniMapRenderer::renderMiniMap()
{
 BO_CHECK_NULL_RET(d->mMapTexture);
 BO_CHECK_NULL_RET(d->mGLMapTexture);
 glPushMatrix();

 // AB: this is only for pre-ufo use
// glLoadIdentity();

 glPushAttrib(GL_ENABLE_BIT);
 glEnable(GL_TEXTURE_2D);
 d->mModelviewMatrix.loadIdentity();
 d->mModelviewMatrix.translate((float)mPosX, mPosY, 0.0f);

 d->mModelviewMatrix.scale(mZoom, mZoom, 1.0f); // AB: maybe do this on the texture matrix stack
// glScalef(mZoom, mZoom, 1.0f); // AB: maybe do this on the texture matrix stack

 if (!d->mUpdatesEnabled) {
	setUpdatesEnabled(true);
 }
 d->mGLMapTexture->bind();

 renderQuad();

 renderCamera();

 glPopAttrib();
 glPopMatrix();
}

void BosonGLMiniMapRenderer::renderQuad()
{
// glMultMatrixf(d->mModelviewMatrix.data());
 // AB: I'd like to use glTexCoord2i(), but at least ATIs implementation makes a
 // glTexCoord2f(1,1) out of glTexCoord2i(1, 1) which is not what I expect here.
 // so instead we use alpha blending to avoid having large parts of the rendered
 // quad just black.
 glBegin(GL_QUADS);
	// AB: note that the y-coordinate needs to be flipped
	// (mapHeight() -> 0.0 ; 0 -> 1.0)
	glTexCoord2f(0.0f, mTextureMaxHeight);
	glVertex3i(0, 0, 0);

	glTexCoord2f(mTextureMaxWidth, mTextureMaxHeight);
	glVertex3i(miniMapWidth(), 0, 0);

	glTexCoord2f(mTextureMaxWidth, 0.0f);
	glVertex3i(miniMapWidth(), miniMapHeight(), 0);

	glTexCoord2f(0.0f, 0.0f);
	glVertex3i(0, miniMapHeight(), 0);
 glEnd();
}

static void cutLine(BoVector3Float& p1_, BoVector3Float& p2_)
{
 BoVector3Float* p1 = &p1_;
 BoVector3Float* p2 = &p2_;
 if (p1->z() < 0.0f && p2->z() < 0.0f) {
	return;
 }
 if (p2->z() >= 0.0f) {
	if (p1->z() >= 0.0f) {
		return;
	}
	p2 = &p1_;
	p1 = &p2_;
 }
 // now p1 >= 0 && p2 < 0

 BoVector3Float u = *p1 - *p2;
 float s = -p2->z() / u.z();
 p2->setX(p2->x() + s * u.x());
 p2->setY(p2->y() + s * u.y());
 p2->setZ(0.0f);
}

static void drawLine(const BoVector3Float& p1, const BoVector3Float& p2)
{
 glVertex3f(p1.x(), p1.y(), p1.z());
 glVertex3f(p2.x(), p2.y(), p2.z());
}

void BosonGLMiniMapRenderer::renderCamera()
{
 // extract points from the viewfrustum
 // we have planes RIGHT, LEFT, BOTTOM, TOP, FAR, NEAR, we are going to name
 // lines and points accordingly (point at LEFT/BOTTOM/NEAR planes is LBN)
 const BoFrustum& viewFrustum = d->mGameGLMatrices->viewFrustum();
 const BoPlane& planeRight  = viewFrustum.right();
 const BoPlane& planeLeft   = viewFrustum.left();
 const BoPlane& planeBottom = viewFrustum.bottom();
 const BoPlane& planeTop    = viewFrustum.top();
 const BoPlane& planeFar    = viewFrustum.far();
 const BoPlane& planeNear   = viewFrustum.near();

 // intersecting lines first
 // every line consists of a point and a direction
 BoVector3Float LF_point;
 BoVector3Float LF_dir;
 BoPlane::intersectPlane(planeLeft, planeFar, &LF_point, &LF_dir);

 BoVector3Float RF_point;
 BoVector3Float RF_dir;
 BoPlane::intersectPlane(planeRight, planeFar, &RF_point, &RF_dir);

 BoVector3Float RN_point;
 BoVector3Float RN_dir;
 BoPlane::intersectPlane(planeRight, planeNear, &RN_point, &RN_dir);

 BoVector3Float LN_point;
 BoVector3Float LN_dir;
 BoPlane::intersectPlane(planeLeft, planeNear, &LN_point, &LN_dir);

 // AB: we do not need all possible lines
#if 0
 BoVector3Float TN_point;
 BoVector3Float TN_dir;
 BoPlane::intersectPlane(planeTop, planeNear, &TN_point, &TN_dir);

 BoVector3Float TL_point;
 BoVector3Float TL_dir;
 BoPlane::intersectPlane(planeTop, planeLeft, &TL_point, &TL_dir);

 BoVector3Float TF_point;
 BoVector3Float TF_dir;
 BoPlane::intersectPlane(planeTop, planeFar, &TF_point, &TF_dir);

 BoVector3Float TR_point;
 BoVector3Float TR_dir;
 BoPlane::intersectPlane(planeTop, planeRight, &TR_point, &TR_dir);

 BoVector3Float BN_point;
 BoVector3Float BN_dir;
 BoPlane::intersectPlane(planeBottom, planeNear, &BN_point, &BN_dir);

 BoVector3Float BL_point;
 BoVector3Float BL_dir;
 BoPlane::intersectPlane(planeBottom, planeLeft, &BL_point, &BL_dir);

 BoVector3Float BF_point;
 BoVector3Float BF_dir;
 BoPlane::intersectPlane(planeBottom, planeFar, &BF_point, &BF_dir);

 BoVector3Float BR_point;
 BoVector3Float BR_dir;
 BoPlane::intersectPlane(planeBottom, planeRight, &BR_point, &BR_dir);
#endif

 // now retrieve all points using the lines.
 // note that we must not do line-line intersection, as that would be highly
 // inaccurate. we use line-plane intersection instead, which provides more
 // accurate results
 BoVector3Float BLF;
 BoVector3Float BRF;
 BoVector3Float BRN;
 BoVector3Float BLN;
 BoVector3Float TLF;
 BoVector3Float TRF;
 BoVector3Float TRN;
 BoVector3Float TLN;
 plane_line_intersect(planeBottom, LF_point, LF_dir, &BLF);
 plane_line_intersect(planeBottom, RF_point, RF_dir, &BRF);
 plane_line_intersect(planeBottom, RN_point, RN_dir, &BRN);
 plane_line_intersect(planeBottom, LN_point, LN_dir, &BLN);
 plane_line_intersect(planeTop, LF_point, LF_dir, &TLF);
 plane_line_intersect(planeTop, RF_point, RF_dir, &TRF);
 plane_line_intersect(planeTop, RN_point, RN_dir, &TRN);
 plane_line_intersect(planeTop, LN_point, LN_dir, &TLN);

 // now intersect with the z=0 plane.
 // this is a special case intersection and it can be done much simpler:
 // AB: maybe use plane_line_intersect anyway because of consistency?
 //     or rather implement plane_line_segment_intersect
 //     -> if segment doesnt intersect: do nothing. if it does: replace point of
 //     segment with z < 0 by the intersection point
 cutLine(BLF, BRF);
 cutLine(BRF, BRN);
 cutLine(BRN, BLN);
 cutLine(BLN, BLF);
 cutLine(TLF, TRF);
 cutLine(TRF, TRN);
 cutLine(TRN, TLN);
 cutLine(TLN, TLF);
 cutLine(BLF, TLF);
 cutLine(BRF, TRF);
 cutLine(BRN, TRN);
 cutLine(BLN, TLN);

 glDisable(GL_TEXTURE_2D);
 glColor3ub(255, 255, 255);

 // map world-coordinates to minimap-coordinates
 BLF.setX((BLF.x() * miniMapWidth()) / mMapWidth);
 BLF.setY((BLF.y() * miniMapHeight()) / mMapHeight + (float)miniMapHeight());
 BRF.setX((BRF.x() * miniMapWidth()) / mMapWidth);
 BRF.setY((BRF.y() * miniMapHeight()) / mMapHeight + (float)miniMapHeight());
 BRN.setX((BRN.x() * miniMapWidth()) / mMapWidth);
 BRN.setY((BRN.y() * miniMapHeight()) / mMapHeight + (float)miniMapHeight());
 BLN.setX((BLN.x() * miniMapWidth()) / mMapWidth);
 BLN.setY((BLN.y() * miniMapHeight()) / mMapHeight + (float)miniMapHeight());
 TLF.setX((TLF.x() * miniMapWidth()) / mMapWidth);
 TLF.setY((TLF.y() * miniMapHeight()) / mMapHeight + (float)miniMapHeight());
 TRF.setX((TRF.x() * miniMapWidth()) / mMapWidth);
 TRF.setY((TRF.y() * miniMapHeight()) / mMapHeight + (float)miniMapHeight());
 TRN.setX((TRN.x() * miniMapWidth()) / mMapWidth);
 TRN.setY((TRN.y() * miniMapHeight()) / mMapHeight + (float)miniMapHeight());
 TLN.setX((TLN.x() * miniMapWidth()) / mMapWidth);
 TLN.setY((TLN.y() * miniMapHeight()) / mMapHeight + (float)miniMapHeight());

 // now the points should be final - we can draw our lines onto the minimap
 glColor3ub(255, 255, 255);
 glBegin(GL_LINES);
	drawLine(BLF, BRF);
	drawLine(BRF, BRN);
	drawLine(BRN, BLN);
	drawLine(BLN, BLF);
	drawLine(TLF, TRF);
	drawLine(TRF, TRN);
	drawLine(TRN, TLN);
	drawLine(TLN, TLF);
	drawLine(BLF, TLF);
	drawLine(BRF, TRF);
	drawLine(BRN, TRN);
	drawLine(BLN, TLN);
 glEnd();
}

void BosonGLMiniMapRenderer::setFogged(int x, int y)
{
 if (!mUseFog) {
	return;
 }
 if (x < 0 || y < 0 || (unsigned int)x >= mapWidth() || (unsigned int)y >= mapHeight()) {
	boError() << k_funcinfo << "invalid cell " << x << "," << y << endl;
	return;
 }
 setPoint(x, y, COLOR_UNKNOWN);
}

void BosonGLMiniMapRenderer::setPoint(int x, int y, const QColor& color)
{
 BO_CHECK_NULL_RET(d->mMapTexture);
 if (d->mMapTextureWidth <= 0 || d->mMapTextureHeight <= 0) {
	boError() << k_funcinfo << "invalid map texture size" << endl;
 }
 // FIXME: endianness ?
 d->mMapTexture[(y * d->mMapTextureWidth + x) * 4 + 0] = color.red();
 d->mMapTexture[(y * d->mMapTextureWidth + x) * 4 + 1] = color.green();
 d->mMapTexture[(y * d->mMapTextureWidth + x) * 4 + 2] = color.blue();
 d->mMapTexture[(y * d->mMapTextureWidth + x) * 4 + 3] = 255; // redundant! is already set on initialization
 if (d->mGLMapTexture && d->mUpdatesEnabled) {
	d->mGLMapTexture->bind();
	glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE,
			&d->mMapTexture[(y * d->mMapTextureWidth + x) * 4]);
	d->mMiniMapChangesSinceRendering++;
	if (d->mMiniMapChangesSinceRendering > 20) {
		// we've done many changes to the minimap without ever rendering
		// a single change. probably there are lots of changes going on
		// and we expect even more.
		// disable updates until minimap is being rendered again
		setUpdatesEnabled(false);
	}
 }
}

void BosonGLMiniMapRenderer::setAlignment(int f)
{
 BO_CHECK_NULL_RET(d->mGameGLMatrices);
 if (f & Qt::AlignLeft) {
	mPosX = distanceFromEdge();
 } else {
	mPosX = d->mGameGLMatrices->viewport()[2] - miniMapWidth() - distanceFromEdge();
 }
 if (f & Qt::AlignBottom) {
	mPosY = distanceFromEdge();
 } else {
	mPosY = d->mGameGLMatrices->viewport()[3] - miniMapHeight() - distanceFromEdge();
 }
}

bool BosonGLMiniMapRenderer::windowToCell(const QPoint& pos, QPoint* cell) const
{
 if (!d->mGameGLMatrices) {
	BO_NULL_ERROR(d->mGameGLMatrices);
	return false;
 }
 int realy = d->mGameGLMatrices->viewport()[3] - pos.y();
 if (pos.x() < (int)mPosX) {
	return false;
 }
 if (realy < (int)mPosY) {
	return false;
 }
 if (pos.x() >= (int)(mPosX + miniMapWidth())) {
	return false;
 }
 if (realy >= (int)(mPosY + miniMapHeight())) {
	return false;
 }
 if (mType != MiniMap) {
	return false;
 }

 BoMatrix invModelviewMatrix;
 d->mModelviewMatrix.invert(&invModelviewMatrix);
 BoVector3Float v(pos.x(), realy, 0.0f);
 BoVector3Float v2;
 invModelviewMatrix.transform(&v2, &v);
 v2.setX(v2.x() * ((float)mapWidth() / (float)miniMapWidth()));
 v2.setY(v2.y() * ((float)mapHeight() / (float)miniMapHeight()));

 cell->setX((int)v2.x());
 cell->setY(mapHeight() - (int)(v2.y()));

 return true;
}

void BosonGLMiniMapRenderer::setLogo(const QImage& image)
{
 boDebug() << k_funcinfo << endl;
 if (miniMapWidth() * miniMapHeight() == 0) {
	boError() << k_funcinfo << "invalid minimap size" << endl;
	return;
 }
 if (image.isNull()) {
	boError() << k_funcinfo << "invalid logo image" << endl;
	return;
 }
 QImage image_ = image;
 image_.setAlphaBuffer(true);
 d->mOrigLogo = image_;

 QImage gl = QGLWidget::convertToGLFormat(d->mOrigLogo);

 delete d->mLogoTexture;
 d->mLogoTexture = new BoTexture(gl.bits(),
		gl.width(), gl.height(),
		BoTexture::FilterLinear | BoTexture::FormatRGBA |
		BoTexture::DontCompress | BoTexture::DontGenMipmaps);
}

void BosonGLMiniMapRenderer::setZoomImages(const QImage& in_, const QImage& out_, const QImage& defaultZoom_)
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

static void plane_line_intersect(const BoPlane& plane, const BoVector3Float& linePoint, BoVector3Float& lineVector, BoVector3Float* intersection)
{
 // AB: see http://geometryalgorithms.com/Archive/algorithm_0104/algorithm_0104B.htm#intersect3D_SegPlane()
 float NdotLine = BoVector3Float::dotProduct(plane.normal(), lineVector);
 if (fabsf(NdotLine) <= 0.001) {
	boError() << k_funcinfo << "line is parallel to plane. not allowed in this function!" << endl;
	// intersection still possible, if the line is on the plane.
	return;
 }

 // AB: note that our normals are directed to the _inside_ of the frustum,
 // therefore we need to multiply be the negative normal (or by the negative
 // distance) to get a point on the plane
 float foo = -BoVector3Float::dotProduct(plane.normal(), linePoint - plane.pointOnPlane());

 float d = foo / NdotLine;

 *intersection = linePoint + lineVector * d;
}

