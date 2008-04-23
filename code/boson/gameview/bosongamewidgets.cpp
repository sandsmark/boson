/*
    This file is part of the Boson game
    Copyright (C) 2001-2008 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2001-2005 Rivo Laks (rivolaks@hot.ee)

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

#include "bosongamewidgets.h"
#include "bosongamewidgets.moc"

#include "../bomemory/bodummymemory.h"
#include "../no_player.h"
#include "../gameengine/bosonpath.h"
#include "../gameengine/bosoncanvas.h"
#include "../gameengine/bosonmap.h"
#include "../botexture.h"
#include "../modelrendering/bosonmodel.h"
#include "../gameengine/speciestheme.h"
#include "../speciesdata.h"
#include "../gameengine/boson.h"
#include "../gameengine/bosongroundtheme.h"
#include "../gameengine/playerio.h"
#include "../gameengine/unitproperties.h"
#include "../bosoncursor.h"
#include "../bosonconfig.h"
#include "../bosonprofiling.h"
#include "../bosonfpscounter.h"
#include "../bosonviewdata.h"
#include "../bolayeredwidget.h"
#include "bodebug.h"

#include <klocale.h>

#include <qtimer.h>
#include <QLabel>
#include <QCheckBox>
#include <QSlider>
#include <QVBoxLayout>
#include <QHBoxLayout>


class PlacementPreview
{
public:
	PlacementPreview()
	{
		mPlacementPreviewProperties = 0;
		mPlacementPreviewModel = 0;
		mCanPlace = false;
		mGroundTextureCount = 0;
		mGroundTextureAlpha = 0;
		mFreeMode = false;
		mUseCollisionDetection = true;
	}
	~PlacementPreview()
	{
		clear();
	}

	bool hasPreview() const
	{
		if (isModelPreview()) {
			return true;
		} else if (isGroundPreview()) {
			return true;
		}
		return false;
	}
	bool isModelPreview() const
	{
		if (mPlacementPreviewModel && mPlacementPreviewModel->lod(0)->frame(0) &&
				mPlacementPreviewProperties) {
			return true;
		}
		return false;
	}
	bool isGroundPreview() const
	{
		if (mGroundTextureCount > 0 && mGroundTextureAlpha) {
			return true;
		}
		return false;
	}
	const BoVector2Fixed& canvasPos() const
	{
		return mCanvasPos;
	}
	void setCanvasVector(const BoVector3Fixed& pos)
	{
		mCanvasPos = BoVector2Fixed(pos.x(), pos.y());
	}

	void setCanPlace(bool canPlace)
	{
		mCanPlace = canPlace;
	}
	bool canPlace() const
	{
		return mCanPlace;
	}

	void setData(const UnitProperties* prop, BosonModel* model)
	{
		mPlacementPreviewProperties = prop;
		mPlacementPreviewModel = model;
	}
	void setData(unsigned int texCount, unsigned char* alpha)
	{
		if (texCount > 500) {
			boError() << k_funcinfo << "invalid texCount: " << texCount << endl;
			return;
		}
		mGroundTextureCount = texCount;
		delete[] mGroundTextureAlpha;
		mGroundTextureAlpha = new unsigned char[texCount];
		for (unsigned int i = 0; i < texCount; i++) {
			mGroundTextureAlpha[i] = alpha[i];
		}
	}
	const UnitProperties* unitProperties() const
	{
		return mPlacementPreviewProperties;
	}
	BosonModel* model() const
	{
		return mPlacementPreviewModel;
	}

	void clear()
	{
//		mGroundPlacementTexture = 0;
		mPlacementPreviewProperties = 0;
		mPlacementPreviewModel = 0;
		mCanPlace = false;
		mGroundTextureCount = 0;
		delete[] mGroundTextureAlpha;
		mGroundTextureAlpha = 0;
	}

	void setFreeMode(bool free)
	{
		mFreeMode = free;
	}
	bool freeMode() const
	{
		return mFreeMode;
	}
	void setUseCollisionDetection(bool use)
	{
		mUseCollisionDetection = use;
	}
	bool useCollisionDetection() const
	{
		return mUseCollisionDetection;
	}

private:
	const UnitProperties* mPlacementPreviewProperties;
	BosonModel* mPlacementPreviewModel;
	bool mCanPlace;
	BoVector2Fixed mCanvasPos;
//	GLuint mCellPlacementTexture;
	unsigned int mGroundTextureCount;
	unsigned char* mGroundTextureAlpha;
	bool mFreeMode;
	bool mUseCollisionDetection;
};

class BosonPlacementPreviewWidgetPrivate
{
public:
	BosonPlacementPreviewWidgetPrivate()
	{
		mGameGLMatrices = 0;
		mCanvas = 0;
		mCursorCanvasVector = 0;
		mLocalPlayerIO = 0;
	}
	const BoGLMatrices* mGameGLMatrices;
	BosonCanvas* mCanvas;
	const BoVector3Fixed* mCursorCanvasVector;
	PlayerIO* mLocalPlayerIO;
	PlacementPreview mPlacementPreview;
	bool mShowPreview;
};

BosonPlacementPreviewWidget::BosonPlacementPreviewWidget(QWidget* parent)
		: QWidget(parent)
{
 setObjectName("BosonPlacementPreviewWidget");
 d = new BosonPlacementPreviewWidgetPrivate;
 d->mShowPreview = false;
}

BosonPlacementPreviewWidget::~BosonPlacementPreviewWidget()
{
 quitGame();
 delete d;
}

void BosonPlacementPreviewWidget::setGameGLMatrices(const BoGLMatrices* m)
{
 d->mGameGLMatrices = m;
}

void BosonPlacementPreviewWidget::setCanvas(BosonCanvas* canvas)
{
 d->mCanvas = canvas;
}

const BosonCanvas* BosonPlacementPreviewWidget::canvas() const
{
 return d->mCanvas;
}

void BosonPlacementPreviewWidget::setCursorCanvasVectorPointer(const BoVector3Fixed* v)
{
 d->mCursorCanvasVector = v;
}

const BoVector3Fixed& BosonPlacementPreviewWidget::cursorCanvasVector() const
{
 return *d->mCursorCanvasVector;
}

void BosonPlacementPreviewWidget::setLocalPlayerIO(PlayerIO* io)
{
 d->mLocalPlayerIO = io;
}

PlayerIO* BosonPlacementPreviewWidget::localPlayerIO() const
{
 return d->mLocalPlayerIO;
}

void BosonPlacementPreviewWidget::slotLockAction(bool locked, int actionType)
{
 if (!locked) {
	if (actionType != ActionInvalid) {
		boError() << k_funcinfo << "API changed!! actionType must be ActionInvalid if locked is FALSE!!" << endl;
		actionType = ActionInvalid;
	}
 }

 if (((UnitAction)actionType) == ActionPlacementPreview) {
	d->mShowPreview = true;
 } else {
	d->mShowPreview = false;
 }
}

void BosonPlacementPreviewWidget::quitGame()
{
 d->mPlacementPreview.clear();
}

void BosonPlacementPreviewWidget::paintWidget()
{
 PROFILE_METHOD;
 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "GL error at the beginning of this method" << endl;
 }

 // Store the original libufo matrices and set our 3d matrices
 glMatrixMode(GL_PROJECTION);
 glPushMatrix();
 glLoadMatrixf(d->mGameGLMatrices->projectionMatrix().data());
 glMatrixMode(GL_MODELVIEW);
 glPushMatrix();
 glLoadMatrixf(d->mGameGLMatrices->modelviewMatrix().data());

 glPushAttrib(GL_ALL_ATTRIB_BITS);
 glViewport(d->mGameGLMatrices->viewport()[0],
		d->mGameGLMatrices->viewport()[1],
		d->mGameGLMatrices->viewport()[2],
		d->mGameGLMatrices->viewport()[3]);
 glEnable(GL_DEPTH_TEST);
 glEnable(GL_LIGHTING);
 glEnable(GL_NORMALIZE);

 renderPlacementPreview();

 glPopAttrib();

  // Restore the original libufo matrices
 glMatrixMode(GL_PROJECTION);
 glPopMatrix();
 glMatrixMode(GL_MODELVIEW);
 glPopMatrix();

 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "GL error at the end of this method" << endl;
 }
}

void BosonPlacementPreviewWidget::renderPlacementPreview()
{
 BO_CHECK_NULL_RET(localPlayerIO());
 BO_CHECK_NULL_RET(canvas());
 BO_CHECK_NULL_RET(canvas()->map());
 if (!d->mShowPreview) {
	return;
 }
 if (!d->mPlacementPreview.hasPreview()) {
	return;
 }

 // AB: GL_MODULATE is currently default. if we every change it to
 // GL_REPLACE we should change it here:
// glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
 GLubyte color;
 if (d->mPlacementPreview.canPlace() || d->mPlacementPreview.freeMode()) {
	color = 255;
 } else {
	color = PLACEMENTPREVIEW_DISALLOW_COLOR;
 }
 glPushAttrib(GL_ALL_ATTRIB_BITS);

 glEnable(GL_BLEND);
 glDisable(GL_LIGHTING);
 glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 glColor4ub(255, color, color, PLACEMENTPREVIEW_ALPHA);
 // Disable depth buffer writes. If we'd write to depth buffer, screen-to-world
 //  mapping functions wouldn't work correctly anymore.
 glDepthMask(GL_FALSE);

 bool modelPreview = d->mPlacementPreview.isModelPreview();
 bool groundPreview = d->mPlacementPreview.isGroundPreview();

 BoVector2Fixed pos(d->mPlacementPreview.canvasPos());

 bofixed w = 0;
 bofixed h = 0;
 if (modelPreview) {
	w = d->mPlacementPreview.unitProperties()->unitWidth();
	h = d->mPlacementPreview.unitProperties()->unitHeight();
 }
 // This is _center_ pos of the unit
 bofixed x;
 bofixed y;
#warning FIXME: Rivo: is useCollisionDetection (replacement for d->mQt::ControlModifier) correct here?
 if (!d->mPlacementPreview.useCollisionDetection() && !boGame->gameMode()) {
	x = pos.x() + w / 2;
	y = pos.y() + h / 2;
 } else {
	x = ((rintf(pos.x()) + w / 2));
	y = ((rintf(pos.y()) + h / 2));
 }
 // Calculate z for units. This code is taken from Unit::updateZ()
 float z = 0;
 if (modelPreview) {
	if (d->mPlacementPreview.unitProperties()->isAircraft() ||
			d->mPlacementPreview.unitProperties()->canGoOnWater()) {
		z = canvas()->heightAtPoint(x, y);
		if (!d->mPlacementPreview.unitProperties()->isAircraft()) {
			z -= 0.05;
		}
	} else
	{
		z = canvas()->terrainHeightAtPoint(x, y);
	}
	if (d->mPlacementPreview.unitProperties()->isAircraft()) {
		z += 2.0f;  // Flying units are always 2 units above the ground
	}
 }
 glTranslatef(x, -y, z);
 if (modelPreview) {
	BoFrame* f = d->mPlacementPreview.model()->lod(0)->frame(0);
	BosonModel::startModelRendering();
	d->mPlacementPreview.model()->prepareRendering();
	Q3ValueVector<const BoMatrix*> itemMatrices(f->nodeCount());
	f->renderFrame(itemMatrices, localPlayerIO()->teamColor());
	BosonModel::stopModelRendering();
 } else if (groundPreview) {
#warning TODO: cell placement preview
#if 0
	glBindTexture(GL_TEXTURE_2D, d->mPlacementPreview.cellTexture());
	glBegin(GL_QUADS);
		glTexCoord2fv(textureUpperLeft);
		glVertex3f(0.0f, 0.0f, 0.0f);

		glTexCoord2fv(textureLowerLeft);
		glVertex3f(0.0f, - 1.0f, 0.0f);

		glTexCoord2fv(textureLowerRight);
		glVertex3f(1.0f, -1.0f, 0.0f);

		glTexCoord2fv(textureUpperRight);
		glVertex3f(1.0f, 0.0f, 0.0f);
	glEnd();
#endif
 }
 glTranslatef(-x, y, -z);
 glColor4ub(255, 255, 255, 255);
 glDisable(GL_BLEND);
 glDepthMask(GL_TRUE);
 // AB: see above. if GL_REPLACES ever becomes default we have to set it
 // here again.
// glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

 glPopAttrib();
}

void BosonPlacementPreviewWidget::setPlacementPreviewData(const UnitProperties* prop, bool canPlace, bool freeMode, bool useCollisionDetection)
{
 d->mPlacementPreview.clear();
 if (!prop) {
	return;
 }
 if (!localPlayerIO()) {
	boError() << k_funcinfo << "NULL local playerIO" << endl;
	return;
 }
 SpeciesTheme* theme = localPlayerIO()->speciesTheme();
 if (!theme) {
	boError() << k_funcinfo << "NULL theme" << endl;
	return;
 }
 if (d->mPlacementPreview.unitProperties() != prop) {
	BosonModel* m = boViewData->speciesData(theme)->unitModel(prop->typeId()); // AB: this does a lookup in a list and therefore should be avoided (this method gets called at least whenever the mouse is moved!)
	if (!m) {
		boError() << k_funcinfo << "NULL model for " << prop->typeId() << endl;
		return;
	}
	BoFrame* f = m->lod(0)->frame(0);
	if (!f) {
		boError() << k_funcinfo << "NULL frame 0" << endl;
		return;
	}
	d->mPlacementPreview.setData(prop, m);
 }
 d->mPlacementPreview.setCanPlace(canPlace);
 d->mPlacementPreview.setCanvasVector(cursorCanvasVector());
 d->mPlacementPreview.setFreeMode(freeMode);
 d->mPlacementPreview.setUseCollisionDetection(useCollisionDetection);
}

void BosonPlacementPreviewWidget::setPlacementCellPreviewData(unsigned int textureCount, unsigned char* alpha, bool canPlace)
{
 // we clear anyway - the new texture will be set below
 d->mPlacementPreview.clear();
 BO_CHECK_NULL_RET(canvas());
 BO_CHECK_NULL_RET(canvas()->map());
 BO_CHECK_NULL_RET(canvas()->map()->texMap());
 BO_CHECK_NULL_RET(canvas()->map()->groundTheme());
 if (textureCount != canvas()->map()->groundTheme()->groundTypeCount()) {
	boError() << k_funcinfo << "groundtype count is invalid - doesn't fit to groundTheme" << endl;
	return;
 }
 if (textureCount == 0) {
	boError() << k_funcinfo << "no textures" << endl;
	return;
 }
 d->mPlacementPreview.setData(textureCount, alpha);
 d->mPlacementPreview.setCanPlace(canPlace);
 d->mPlacementPreview.setCanvasVector(cursorCanvasVector());
}

void BosonPlacementPreviewWidget::slotSetPlacementPreviewData(const UnitProperties* prop, bool canPlace, bool freeMode, bool useCollisionDetection)
{
 setPlacementPreviewData(prop, canPlace, freeMode, useCollisionDetection);
}

void BosonPlacementPreviewWidget::slotSetPlacementCellPreviewData(unsigned int textureCount, unsigned char* alpha, bool canPlace)
{
 setPlacementCellPreviewData(textureCount, alpha, canPlace);
}





class BosonLineVisualizationWidgetPrivate
{
public:
	BosonLineVisualizationWidgetPrivate()
	{
		mGameGLMatrices = 0;
		mCanvas = 0;
	}
	const BoGLMatrices* mGameGLMatrices;
	Q3ValueList<BoLineVisualization> mLineVisualizationList;
	const BosonCanvas* mCanvas;
};

BosonLineVisualizationWidget::BosonLineVisualizationWidget(QWidget* parent)
	: QWidget(parent)
{
 setObjectName("BosonLineVisualizationWidget");
 d = new BosonLineVisualizationWidgetPrivate();

 connect(BosonPathVisualization::pathVisualization(),
		SIGNAL(signalAddLineVisualization( const Q3ValueList<BoVector3Fixed>&, const BoVector4Float&, bofixed, int, bofixed)),
		this,
		SLOT(slotAddLineVisualization(const Q3ValueList<BoVector3Fixed>&, const BoVector4Float&, bofixed, int, bofixed)));
}

BosonLineVisualizationWidget::~BosonLineVisualizationWidget()
{
 delete d;
}

void BosonLineVisualizationWidget::setGameGLMatrices(const BoGLMatrices* m)
{
 d->mGameGLMatrices = m;
}

void BosonLineVisualizationWidget::setCanvas(const BosonCanvas* canvas)
{
 d->mCanvas = canvas;
}

const BosonCanvas* BosonLineVisualizationWidget::canvas() const
{
 return d->mCanvas;
}

void BosonLineVisualizationWidget::paintWidget()
{
 PROFILE_METHOD;
 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "GL error at the beginning of this method" << endl;
 }
 BO_CHECK_NULL_RET(d->mGameGLMatrices);
 // Store the original libufo matrices and set our 3d matrices
 glMatrixMode(GL_PROJECTION);
 glPushMatrix();
 glLoadMatrixf(d->mGameGLMatrices->projectionMatrix().data());
 glMatrixMode(GL_MODELVIEW);
 glPushMatrix();
 glLoadMatrixf(d->mGameGLMatrices->modelviewMatrix().data());

 glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_VIEWPORT_BIT);
 glViewport(d->mGameGLMatrices->viewport()[0],
		d->mGameGLMatrices->viewport()[1],
		d->mGameGLMatrices->viewport()[2],
		d->mGameGLMatrices->viewport()[3]);
 glDisable(GL_DEPTH_TEST);
 glDisable(GL_LIGHTING);
 glDisable(GL_NORMALIZE);
 boTextureManager->disableTexturing();
 Q3ValueList<BoLineVisualization>::iterator it;
 for (it = d->mLineVisualizationList.begin(); it != d->mLineVisualizationList.end(); ++it) {
	glColor4fv((*it).color.data());
	glPointSize((*it).pointsize);
	glBegin(GL_LINE_STRIP);
	Q3ValueList<BoVector3Fixed>::iterator pit;
	for (pit = (*it).points.begin(); pit != (*it).points.end(); ++pit) {
		glVertex3fv((*pit).toFloat().data());
	}	glEnd();
 }
 glPopAttrib();

  // Restore the original libufo matrices
 glMatrixMode(GL_PROJECTION);
 glPopMatrix();
 glMatrixMode(GL_MODELVIEW);
 glPopMatrix();
 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "GL error at the end of this method" << endl;
 }
}

void BosonLineVisualizationWidget::addLineVisualization(BoLineVisualization v)
{
 d->mLineVisualizationList.append(v);
}

void BosonLineVisualizationWidget::slotAddLineVisualization(const Q3ValueList<BoVector3Fixed>& points, const BoVector4Float& color, bofixed pointSize, int timeout, bofixed zOffset)
{
 if (!canvas()) {
	return;
 }
 BoLineVisualization viz;
 viz.pointsize = pointSize;
 viz.timeout = timeout;
 viz.color = color;
 viz.points = points;
 Q3ValueList<BoVector3Fixed>::Iterator it;
 for (it = viz.points.begin(); it != viz.points.end(); ++it) {
	(*it).setZ(canvas()->heightAtPoint((*it).x(), -(*it).y()) + zOffset);
 }
 addLineVisualization(viz);
}

void BosonLineVisualizationWidget::slotAdvance(unsigned int, bool)
{
 advanceLineVisualization();
}

void BosonLineVisualizationWidget::advanceLineVisualization()
{
 Q3ValueList<BoLineVisualization>::iterator it;
 for (it = d->mLineVisualizationList.begin(); it != d->mLineVisualizationList.end(); ++it) {
	(*it).timeout--;
	if ((*it).timeout == 0) {
		// expired - remove it
		d->mLineVisualizationList.erase(it);
		--it;
	}
 }
}



class BosonCursorWidgetPrivate
{
public:
	BosonCursorWidgetPrivate()
	{
		mGameGLMatrices = 0;
		mCursorCollection = 0;
		mCursorWidgetPos = 0;
	}

	const BoGLMatrices* mGameGLMatrices;
	BosonCursorCollection* mCursorCollection;
	const QPoint* mCursorWidgetPos;
};

BosonCursorWidget::BosonCursorWidget(QWidget* parent)
	: QWidget(parent)
{
 setObjectName("BosonCursorWidget");
 d = new BosonCursorWidgetPrivate();
 d->mCursorCollection = new BosonCursorCollection(this);
 connect(d->mCursorCollection, SIGNAL(signalSetWidgetCursor(BosonCursor*)),
		this, SIGNAL(signalSetWidgetCursor(BosonCursor*)));
}

BosonCursorWidget::~BosonCursorWidget()
{
 delete d->mCursorCollection;
 delete d;
}

void BosonCursorWidget::setGameGLMatrices(const BoGLMatrices* m)
{
 d->mGameGLMatrices = m;
}

void BosonCursorWidget::setCursorWidgetPos(const QPoint* pos)
{
 d->mCursorWidgetPos = pos;
}

BosonCursor* BosonCursorWidget::cursor() const
{
 return d->mCursorCollection->cursor();
}

void BosonCursorWidget::paintWidget()
{
 PROFILE_METHOD;
 BO_CHECK_NULL_RET(d->mCursorWidgetPos);
 BO_CHECK_NULL_RET(d->mGameGLMatrices);
 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "OpenGL error before method" << endl;
 }

 glMatrixMode(GL_PROJECTION);
 glPushMatrix();
 glLoadIdentity();
 gluOrtho2D(0.0, (GLfloat)width(), 0.0, (GLfloat)height());
 glMatrixMode(GL_MODELVIEW);
 glPushMatrix();
 glLoadIdentity();

 glPushAttrib(GL_COLOR_BUFFER_BIT | GL_CURRENT_BIT | GL_VIEWPORT_BIT);
 glViewport(d->mGameGLMatrices->viewport()[0],
		d->mGameGLMatrices->viewport()[1],
		d->mGameGLMatrices->viewport()[2],
		d->mGameGLMatrices->viewport()[3]);
 glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 glColor3ub(255, 255, 255);

 if (cursor()) {
	QPoint pos = *d->mCursorWidgetPos;
	GLfloat x = (GLfloat)pos.x();
	GLfloat y = (GLfloat)(height()) - (GLfloat)pos.y();
	cursor()->renderCursor(x, y);
 }

 glPopAttrib();

 glMatrixMode(GL_PROJECTION);
 glPopMatrix();
 glMatrixMode(GL_MODELVIEW);
 glPopMatrix();

 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "OpenGL error at the end of method" << endl;
 }
}

void BosonCursorWidget::slotChangeCursor(int mode, const QString& cursorDir)
{
 if (boGame) {
	if (!boGame->gameMode()) {
		// editor mode
		mode = CursorKDE;
	}
 }
 if (d->mCursorCollection->changeCursor(mode, cursorDir)) {
	// TODO: rename setCursorMode() to setCursorType()
	boConfig->setIntValue("CursorMode", d->mCursorCollection->cursorType());
	boConfig->setStringValue("CursorDir", d->mCursorCollection->cursorDir());

	emit signalSetCursor(cursor());
 }
}



class BosonSelectionRectWidgetPrivate
{
public:
	BosonSelectionRectWidgetPrivate()
	{
		mGameGLMatrices = 0;
	}
	const BoGLMatrices* mGameGLMatrices;
	bool mSelectionRectVisible;
	QRect mSelectionRect;
};

BosonSelectionRectWidget::BosonSelectionRectWidget(QWidget* parent)
	: QWidget(parent)
{
 setObjectName("BosonSelectionRectWidget");
 d = new BosonSelectionRectWidgetPrivate();
 d->mSelectionRectVisible = false;
}

BosonSelectionRectWidget::~BosonSelectionRectWidget()
{
 delete d;
}

void BosonSelectionRectWidget::setGameGLMatrices(const BoGLMatrices* m)
{
 d->mGameGLMatrices = m;
}

void BosonSelectionRectWidget::slotSelectionRectVisible(bool v)
{
 d->mSelectionRectVisible = v;
}

void BosonSelectionRectWidget::slotSelectionRectChanged(const QRect& r)
{
 d->mSelectionRect = r;
}

void BosonSelectionRectWidget::paintWidget()
{
 PROFILE_METHOD;
 BO_CHECK_NULL_RET(d->mGameGLMatrices);
 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "GL error at the beginning of this method" << endl;
 }
 if (!d->mSelectionRectVisible) {
	return;
 }
 glMatrixMode(GL_PROJECTION);
 glPushMatrix();
 glLoadIdentity();
 gluOrtho2D(0.0, (GLfloat)width(), 0.0, (GLfloat)height());
 glMatrixMode(GL_MODELVIEW);
 glPushMatrix();
 glLoadIdentity();

 glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_VIEWPORT_BIT);
 glViewport(d->mGameGLMatrices->viewport()[0],
		d->mGameGLMatrices->viewport()[1],
		d->mGameGLMatrices->viewport()[2],
		d->mGameGLMatrices->viewport()[3]);
 boTextureManager->disableTexturing();

 glColor3ub(255, 0, 0); // FIXME hardcoded

 QRect rect = d->mSelectionRect;

 int x = rect.left();
 int w = rect.width();

 int y = d->mGameGLMatrices->viewport()[3] - rect.top();
 int h = rect.height();

 glBegin(GL_LINE_LOOP);
	glVertex3f(x, y, 0.0f);
	glVertex3f(x + w, y, 0.0f);
	glVertex3f(x + w, y - h, 0.0f);
	glVertex3f(x, y - h, 0.0f);
 glEnd();

 glPopAttrib();

 glMatrixMode(GL_PROJECTION);
 glPopMatrix();
 glMatrixMode(GL_MODELVIEW);
 glPopMatrix();
 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "GL error at the end of this method" << endl;
 }
}



// AB: note that this can be used for things other than FPS, too
class FPSGraphData
{
public:
	FPSGraphData();

	void addData(float data);
	float fullWidth() const;
	void ensureMaxWidth(float width);

	float mMin;
	float mMax;
	Q3ValueList<float> mData;
	float mDataWidth;
	QColor mColor;
};

FPSGraphData::FPSGraphData()
{
 mMin = 0.0f;
 mMax = 100.0f;
 mDataWidth = 1.0;
 mColor = Qt::red;
}

void FPSGraphData::addData(float data)
{
 mData.append(data);
}

float FPSGraphData::fullWidth() const
{
 return mData.count() * mDataWidth;
}
void FPSGraphData::ensureMaxWidth(float width)
{
 while (fullWidth() > width) {
	mData.pop_front();
 }
}


class BosonFPSGraphWidgetPrivate
{
public:
	BosonFPSGraphWidgetPrivate()
	{
		mGameGLMatrices = 0;
		mGameFPSCounter = 0;
	}
	const BoGLMatrices* mGameGLMatrices;
	const BosonGameFPSCounter* mGameFPSCounter;

	FPSGraphData mFPSData;
	FPSGraphData mSkippedFPSData;
};

BosonFPSGraphWidget::BosonFPSGraphWidget(QWidget* parent)
	: QWidget(parent)
{
 setObjectName("BosonFPSGraphWidget");
 d = new BosonFPSGraphWidgetPrivate();
 QTimer* timer = new QTimer(this);
 connect(timer, SIGNAL(timeout()),
		this, SLOT(slotAddData()));

 d->mFPSData.mColor = Qt::green;
 d->mSkippedFPSData.mColor = Qt::red;

 timer->start(100);
}

BosonFPSGraphWidget::~BosonFPSGraphWidget()
{
 delete d;
}

void BosonFPSGraphWidget::setGameGLMatrices(const BoGLMatrices* m)
{
 d->mGameGLMatrices = m;
}

void BosonFPSGraphWidget::setGameFPSCounter(const BosonGameFPSCounter* c)
{
 d->mGameFPSCounter = c;
}

void BosonFPSGraphWidget::paintWidget()
{
 PROFILE_METHOD;
 BO_CHECK_NULL_RET(d->mGameGLMatrices);
 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "GL error at the beginning of this method" << endl;
 }

 glMatrixMode(GL_PROJECTION);
 glPushMatrix();
 glLoadIdentity();
 gluOrtho2D(0.0, (GLfloat)width(), 0.0, (GLfloat)height());
 glMatrixMode(GL_MODELVIEW);
 glPushMatrix();
 glLoadIdentity();

 glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_VIEWPORT_BIT);
 glViewport(d->mGameGLMatrices->viewport()[0],
		d->mGameGLMatrices->viewport()[1],
		d->mGameGLMatrices->viewport()[2],
		d->mGameGLMatrices->viewport()[3]);
 boTextureManager->disableTexturing();

 if (boConfig->boolValue("debug_fps")) {
	paintFPS(d->mFPSData);
	paintFPS(d->mSkippedFPSData);
 }
#if 0
 if (boConfig->boolValue("debug_something_else")) {
	paintFPS(d->mSomethingElse);
 }
#endif

 glPopAttrib();

 glMatrixMode(GL_PROJECTION);
 glPopMatrix();
 glMatrixMode(GL_MODELVIEW);
 glPopMatrix();
 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "GL error at the end of this method" << endl;
 }
}

void BosonFPSGraphWidget::paintFPS(const FPSGraphData& data)
{
 glColor3ub(data.mColor.red(), data.mColor.green(), data.mColor.blue());

 float widgetHeight = (float)height();

 float x = 0.0f;
 glBegin(GL_LINE_STRIP);
	for (Q3ValueList<float>::const_iterator it = data.mData.begin(); it != data.mData.end(); ++it) {
		float factor = ((*it) - data.mMin) / data.mMax;
		float y = factor * widgetHeight;
		glVertex2f(x, y);

		x += data.mDataWidth;
	}
 glEnd();

 // TODO: add a label describing this data
}

void BosonFPSGraphWidget::slotAddData()
{
 if (d->mGameFPSCounter) {
	double skippedFps;
	double fps = d->mGameFPSCounter->counter()->fps(&skippedFps);
	d->mFPSData.addData((float)fps);
	d->mFPSData.ensureMaxWidth((float)width());

	d->mSkippedFPSData.addData((float)skippedFps);
	d->mSkippedFPSData.ensureMaxWidth((float)width());
 }

}





class ProfilingGraphType
{
public:
	ProfilingGraphType()
	{
		mY = 0;
	}
	QString mName;
	QColor mColor;
	int mY;
};

class ProfilingGraphItem
{
public:
	ProfilingGraphItem()
	{
		mType = 0;
		mStart = 0.0;
		mLength = 0.0;
	}
	ProfilingGraphType* mType;
	double mStart;
	double mLength;
};

class BosonProfilingGraphWidgetPrivate
{
public:
	BosonProfilingGraphWidgetPrivate()
	{
		mGameGLMatrices = 0;

		mUpdateTimer = 0;

		mLayeredWidget = 0;
		mLabelsWidget = 0;
		mEnableUpdates = 0;
		mUpdateInterval = 0;
		mUpdateIntervalLabel = 0;
		mOneLinePerType = 0;
	}
	const BoGLMatrices* mGameGLMatrices;
	Q3PtrList<ProfilingGraphItem> mItems;
	QMap<QString, ProfilingGraphType*> mProfilingTypes;

	QTimer* mUpdateTimer;

	QList<QColor> mAvailableColors;

	BoLayeredWidget* mLayeredWidget;
	QWidget* mLabelsWidget;
	QList<QLabel*> mLabels;
	QCheckBox* mEnableUpdates;
	QSlider* mUpdateInterval;
	QLabel* mUpdateIntervalLabel;
	QCheckBox* mOneLinePerType;
};

BosonProfilingGraphWidget::BosonProfilingGraphWidget(QWidget* parent)
	: QWidget(parent)
{
 setObjectName("BosonProfilingGraphWidget");
 d = new BosonProfilingGraphWidgetPrivate();
 d->mUpdateTimer = new QTimer(this);
 connect(d->mUpdateTimer, SIGNAL(timeout()),
		this, SLOT(slotUpdateData()));
 d->mUpdateTimer->start(100);

 QVBoxLayout* topLayout = new QVBoxLayout(this);
 d->mLayeredWidget = new BoLayeredWidget(this);
 topLayout->addWidget(d->mLayeredWidget);

 d->mLabelsWidget = new QWidget(d->mLayeredWidget);

 QWidget* control = new QWidget(d->mLayeredWidget);
 QHBoxLayout* controlLayout = new QHBoxLayout(control);
 controlLayout->addStretch(1);

 QVBoxLayout* vboxLayout = new QVBoxLayout(controlLayout);
 vboxLayout->addSpacing(100);

 d->mEnableUpdates = new QCheckBox(control);
 d->mEnableUpdates->setText(i18n("Enable Updates"));
 d->mEnableUpdates->setChecked(true);
// d->mEnableUpdates->setForegroundColor(Qt::white);
 vboxLayout->addWidget(d->mEnableUpdates);

 QWidget* hbox = new QWidget(control);
 vboxLayout->addWidget(hbox);
 QHBoxLayout* hboxLayout = new QHBoxLayout(hbox);
 QLabel* intervalLabel = new QLabel(i18n("Update interval (ms): "), hbox);
// intervalLabel->setForegroundColor(Qt::white);
 hboxLayout->addWidget(intervalLabel);
 d->mUpdateInterval = new QSlider(hbox);
 hboxLayout->addWidget(d->mUpdateInterval);
 d->mUpdateIntervalLabel = new QLabel(hbox);
// d->mUpdateIntervalLabel->setForegroundColor(Qt::white);
 hboxLayout->addWidget(d->mUpdateIntervalLabel);
 connect(d->mUpdateInterval, SIGNAL(signalValueChanged(int)),
		this, SLOT(slotSetUpdateInterval(int)));
 d->mUpdateInterval->setRange(20, 4000);
 d->mUpdateInterval->setValue(1000);

 d->mOneLinePerType = new QCheckBox(control);
 d->mOneLinePerType->setText(i18n("Display sum of elapsed time"));
 d->mOneLinePerType->setChecked(true);
// d->mOneLinePerType->setForegroundColor(Qt::white);
 vboxLayout->addWidget(d->mOneLinePerType);

 vboxLayout->addStretch(1);



 d->mItems.setAutoDelete(true);


 d->mAvailableColors.append(QColor(255,   0,   0));
 d->mAvailableColors.append(QColor(0,   255,   0));
 d->mAvailableColors.append(QColor(0,     0, 255));
 d->mAvailableColors.append(QColor(255, 255,   0));
 d->mAvailableColors.append(QColor(255,   0, 255));
 d->mAvailableColors.append(QColor(0,   255, 255));
 d->mAvailableColors.append(QColor(0,     0,   0));

 d->mAvailableColors.append(QColor(255, 127,   0));
 d->mAvailableColors.append(QColor(255,   0, 127));
 d->mAvailableColors.append(QColor(255, 127, 127));
 d->mAvailableColors.append(QColor(127, 255,   0));
 d->mAvailableColors.append(QColor(0,   255, 127));
 d->mAvailableColors.append(QColor(127, 255, 127));
 d->mAvailableColors.append(QColor(127,   0, 255));
 d->mAvailableColors.append(QColor(0,   127, 255));
 d->mAvailableColors.append(QColor(127, 127, 255));

 d->mAvailableColors.append(QColor(255,  63,   0));
 d->mAvailableColors.append(QColor(255,   0,  63));
 d->mAvailableColors.append(QColor(255,  63,  63));
 d->mAvailableColors.append(QColor(63,  255,   0));
 d->mAvailableColors.append(QColor(0,   255,  63));
 d->mAvailableColors.append(QColor(63,  255,  63));
 d->mAvailableColors.append(QColor(63,    0, 255));
 d->mAvailableColors.append(QColor(0,    63, 255));
 d->mAvailableColors.append(QColor(63,   63, 255));

 d->mAvailableColors.append(QColor(255, 255, 255));
}

BosonProfilingGraphWidget::~BosonProfilingGraphWidget()
{
 d->mItems.setAutoDelete(true);
 d->mItems.clear();
 resetProfilingTypes();
 delete d;
}

void BosonProfilingGraphWidget::resetProfilingTypes()
{
 // make sure no item still references one of the types
 d->mItems.setAutoDelete(true);
 d->mItems.clear();

 QMap<QString, ProfilingGraphType*>::iterator it;
 for (it = d->mProfilingTypes.begin(); it != d->mProfilingTypes.end(); ++it) {
	delete *it;
 }
 d->mProfilingTypes.clear();

 foreach (QLabel* label, d->mLabels) {
	label->hide();
 }
}

void BosonProfilingGraphWidget::setGameGLMatrices(const BoGLMatrices* m)
{
 d->mGameGLMatrices = m;
}

void BosonProfilingGraphWidget::paintWidget()
{
 PROFILE_METHOD;
 BO_CHECK_NULL_RET(d->mGameGLMatrices);
 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "GL error at the beginning of this method" << endl;
 }
 if (!boConfig->boolValue("debug_profiling_graph")) {
	d->mLayeredWidget->hide();
	return;
 }
 d->mLayeredWidget->show();

 glMatrixMode(GL_PROJECTION);
 glPushMatrix();
 glLoadIdentity();
 gluOrtho2D(0.0, (GLfloat)width(), 0.0, (GLfloat)height());
 glMatrixMode(GL_MODELVIEW);
 glPushMatrix();
 glLoadIdentity();

 glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_VIEWPORT_BIT);
 glViewport(d->mGameGLMatrices->viewport()[0],
		d->mGameGLMatrices->viewport()[1],
		d->mGameGLMatrices->viewport()[2],
		d->mGameGLMatrices->viewport()[3]);
 boTextureManager->disableTexturing();

 // AB: other than the color, we also use the y position of the lines to
 // distinguish types. all types start between minY and maxY
 int minY = height() / 4;
 int maxY = (height() * 3) / 4;


 int yDist = 30;
 if ((int)(minY + yDist * d->mProfilingTypes.count()) > maxY) {
	yDist = (maxY - minY) / d->mProfilingTypes.count();
 }
 int y = minY;
 for (QMap<QString, ProfilingGraphType*>::iterator it = d->mProfilingTypes.begin(); it != d->mProfilingTypes.end(); ++it) {
	(*it)->mY = y;
	y += yDist;
 }

 glBegin(GL_LINES);
 Q3PtrListIterator<ProfilingGraphItem> it(d->mItems);
 for (; it.current(); ++it) {
	const ProfilingGraphType* type = it.current()->mType;
	int x = (int)(width() * it.current()->mStart);
	int x2 = (int)(width() * (it.current()->mStart + it.current()->mLength));
	int y = type->mY;
	glColor3ub(type->mColor.red(), type->mColor.green(), type->mColor.blue());
	glVertex2i(x, y);
	glVertex2i(x2, y);
 }
 glEnd();


 int labelIndex = 0;
 QMap<QString, ProfilingGraphType*>::iterator typeIt = d->mProfilingTypes.begin();
 while (typeIt != d->mProfilingTypes.end()) {
	if (labelIndex >= d->mLabels.count()) {
		boError() << "oops: labelIndex out of range";
		break;
	}
	int x = 100;
	int y = height() - (*typeIt)->mY;
	d->mLabels[labelIndex]->move(x, y);
//	d->mLabels[labelIndex]->setSize(d->mLabels[labelIndex]->preferredWidth(), d->mLabels[labelIndex]->preferredHeight());
	d->mLabels[labelIndex]->resize(d->mLabels[labelIndex]->sizeHint());
	++typeIt;
	labelIndex++;
 }

 glPopAttrib();

 glMatrixMode(GL_PROJECTION);
 glPopMatrix();
 glMatrixMode(GL_MODELVIEW);
 glPopMatrix();
 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "GL error at the end of this method" << endl;
 }
}

void BosonProfilingGraphWidget::slotUpdateData()
{
 BO_CHECK_NULL_RET(d->mGameGLMatrices);
 if (!d->mEnableUpdates->isChecked()) {
	return;
 }
 PROFILE_METHOD
 struct timeval now;
 struct timeval since;
 gettimeofday(&now, 0);
 since.tv_sec = now.tv_sec;
 since.tv_usec = now.tv_usec;

 const int displaySeconds = 2;
 since.tv_sec -= displaySeconds;

 d->mItems.setAutoDelete(true);
 d->mItems.clear();
 resetProfilingTypes();

 Q3PtrList<const BosonProfilingItem> itemList;
 boProfiling->getItemsSinceSorted(&itemList, since);

 bool oneLinePerType = d->mOneLinePerType->isChecked();

 QMap<ProfilingGraphType*, unsigned long int> elapsedSumOfType;
 unsigned long int displayTime = compareTimes(since, now);
 double fdisplayTime = (double)displayTime;
 for (Q3PtrListIterator<const BosonProfilingItem> it(itemList); it.current(); ++it) {
	const BosonProfilingItem* profilingItem = it.current();
	ProfilingGraphType* type = 0;
	if (d->mProfilingTypes.contains(profilingItem->name())) {
		type = d->mProfilingTypes[profilingItem->name()];
	} else {
		type = new ProfilingGraphType();
		type->mName = profilingItem->name();

		d->mProfilingTypes.insert(profilingItem->name(), type);
		elapsedSumOfType.insert(type, 0);
	}

	unsigned long int startedAfter = compareTimes(since, profilingItem->startTime());
	unsigned long int elapsed = compareTimes(profilingItem->startTime(), profilingItem->endTime());

	elapsedSumOfType[type] += elapsed;

	if (!oneLinePerType) {
		ProfilingGraphItem* item = new ProfilingGraphItem();
		item->mType = type;

		item->mStart = ((double)startedAfter) / fdisplayTime;
		item->mLength = ((double)elapsed) / fdisplayTime;

		d->mItems.append(item);
	}
 }

 if (oneLinePerType) {
	QMap<ProfilingGraphType*, unsigned long int>::iterator it;
	for (it = elapsedSumOfType.begin(); it != elapsedSumOfType.end(); ++it) {
		ProfilingGraphItem* item = new ProfilingGraphItem();
		item->mType = it.key();

		item->mStart = 0.0;
		item->mLength = ((double)it.data()) / fdisplayTime;

		d->mItems.append(item);
	}

 }

 ensureLabels(d->mProfilingTypes.count());
 int labelIndex = 0;
 int colorIndex = 0;
 QMap<QString, ProfilingGraphType*>::iterator typeIt = d->mProfilingTypes.begin();
 while (typeIt != d->mProfilingTypes.end()) {
	if (labelIndex >= d->mLabels.count()) {
		boError() << "oops: labelIndex out of range";
		break;
	}
	if (colorIndex >= d->mAvailableColors.count()) {
		boError() << "oops: colorIndex out of range";
		break;
	}
//	(*typeIt)->mColor = d->mAvailableColors[colorIndex];
	(*typeIt)->mColor = Qt::white;

	d->mLabels[labelIndex]->setText((*typeIt)->mName);
	d->mLabels[labelIndex]->show();

	// AB: we use the last color for all remaining types
	colorIndex++;
	if (colorIndex == d->mAvailableColors.count()) {
		colorIndex--;
	}

	++typeIt;
	labelIndex++;
 }
 while (labelIndex < d->mLabels.count()) {
	d->mLabels[labelIndex]->hide();
	labelIndex++;
 }
}

void BosonProfilingGraphWidget::ensureLabels(int count)
{
 while (d->mLabels.count() < count) {
	QLabel* label = new QLabel(d->mLabelsWidget);
	d->mLabels.append(label);
//	label->setForegroundColor(Qt::white);

	label->hide();
 }
}

void BosonProfilingGraphWidget::slotSetUpdateInterval(int interval)
{
 d->mUpdateIntervalLabel->setText(QString::number(interval));
 if (interval < 0) {
	boError() << k_funcinfo << interval << endl;
	return;
 }
 d->mUpdateTimer->stop();
 d->mUpdateTimer->start(interval);
}


