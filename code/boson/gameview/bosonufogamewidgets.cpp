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

#include "bosonufogamewidgets.h"
#include "bosonufogamewidgets.moc"

#include "../no_player.h"
#include "bosoncanvasrenderer.h"
#include "../bosonpath.h"
#include "../bosoncanvas.h"
#include "../bosonmap.h"
#include "../botexture.h"
#include "../bosonmodel.h"
#include "../speciestheme.h"
#include "../boson.h"
#include "../bosongroundtheme.h"
#include "../playerio.h"
#include "../unitproperties.h"
#include "../bosoncursor.h"
#include "../bosonconfig.h"
#include "../bosonprofiling.h"
#include "../bosonfpscounter.h"
#include "bodebug.h"

#include <qtimer.h>
#include <qvaluelist.h>

class BosonUfoCanvasWidgetPrivate
{
public:
	BosonUfoCanvasWidgetPrivate()
	{
		mGameGLMatrices = 0;
		mCanvasRenderer = 0;
		mCamera = 0;
		mLocalPlayerIO = 0;
		mCanvas = 0;
	}
	const BoGLMatrices* mGameGLMatrices;
	BosonCanvasRenderer* mCanvasRenderer;
	BoGameCamera* mCamera;
	PlayerIO* mLocalPlayerIO;
	const BosonCanvas* mCanvas;
};

BosonUfoCanvasWidget::BosonUfoCanvasWidget()
		: BoUfoCustomWidget()
{
 d = new BosonUfoCanvasWidgetPrivate();

 d->mCanvasRenderer = new BosonCanvasRenderer();
 d->mCanvasRenderer->initGL();
}

BosonUfoCanvasWidget::~BosonUfoCanvasWidget()
{
 quitGame();
 delete d;
}

void BosonUfoCanvasWidget::setGameGLMatrices(const BoGLMatrices* m)
{
 d->mGameGLMatrices = m;
 d->mCanvasRenderer->setGameGLMatrices(d->mGameGLMatrices);
}

void BosonUfoCanvasWidget::setCamera(BoGameCamera* c)
{
 d->mCamera = c;
}

void BosonUfoCanvasWidget::setLocalPlayerIO(PlayerIO* io)
{
 d->mLocalPlayerIO = io;
}

void BosonUfoCanvasWidget::setCanvas(const BosonCanvas* canvas)
{
 d->mCanvas = canvas;
}

void BosonUfoCanvasWidget::setParticlesDirty(bool dirty)
{
 d->mCanvasRenderer->setParticlesDirty(dirty);
}

void BosonUfoCanvasWidget::quitGame()
{
 d->mCanvasRenderer->reset();
}

void BosonUfoCanvasWidget::paintWidget()
{
 PROFILE_METHOD;
 d->mCanvasRenderer->setCamera(d->mCamera);
 d->mCanvasRenderer->setLocalPlayerIO(d->mLocalPlayerIO);

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

 d->mCanvasRenderer->paintGL(d->mCanvas);

 glPopAttrib();

  // Restore the original libufo matrices
 glMatrixMode(GL_PROJECTION);
 glPopMatrix();
 glMatrixMode(GL_MODELVIEW);
 glPopMatrix();
}



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

class BosonUfoPlacementPreviewWidgetPrivate
{
public:
	BosonUfoPlacementPreviewWidgetPrivate()
	{
		mGameGLMatrices = 0;
		mCanvas = 0;
		mLocalPlayerIO = 0;
	}
	const BoGLMatrices* mGameGLMatrices;
	BosonCanvas* mCanvas;
	BoVector3Fixed mCursorCanvasVector;
	PlayerIO* mLocalPlayerIO;
	PlacementPreview mPlacementPreview;
	bool mShowPreview;
};

BosonUfoPlacementPreviewWidget::BosonUfoPlacementPreviewWidget()
		: BoUfoCustomWidget()
{
 d = new BosonUfoPlacementPreviewWidgetPrivate;
 d->mShowPreview = false;
}

BosonUfoPlacementPreviewWidget::~BosonUfoPlacementPreviewWidget()
{
 quitGame();
 delete d;
}

void BosonUfoPlacementPreviewWidget::setGameGLMatrices(const BoGLMatrices* m)
{
 d->mGameGLMatrices = m;
}

void BosonUfoPlacementPreviewWidget::setCanvas(BosonCanvas* canvas)
{
 d->mCanvas = canvas;
}

const BosonCanvas* BosonUfoPlacementPreviewWidget::canvas() const
{
 return d->mCanvas;
}

void BosonUfoPlacementPreviewWidget::setCursorCanvasVector(const BoVector3Fixed& v)
{
 d->mCursorCanvasVector = v;
}

const BoVector3Fixed& BosonUfoPlacementPreviewWidget::cursorCanvasVector() const
{
 return d->mCursorCanvasVector;
}

void BosonUfoPlacementPreviewWidget::setLocalPlayerIO(PlayerIO* io)
{
 d->mLocalPlayerIO = io;
}

PlayerIO* BosonUfoPlacementPreviewWidget::localPlayerIO() const
{
 return d->mLocalPlayerIO;
}

void BosonUfoPlacementPreviewWidget::slotLockAction(bool locked, int actionType)
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

void BosonUfoPlacementPreviewWidget::quitGame()
{
 d->mPlacementPreview.clear();
}

void BosonUfoPlacementPreviewWidget::paintWidget()
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

void BosonUfoPlacementPreviewWidget::renderPlacementPreview()
{
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
 glEnable(GL_BLEND);
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
#warning FIXME: Rivo: is useCollisionDetection (replacement for d->mControlButton) correct here?
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
	f->renderFrame(&localPlayerIO()->teamColor());
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
}

void BosonUfoPlacementPreviewWidget::setPlacementPreviewData(const UnitProperties* prop, bool canPlace, bool freeMode, bool useCollisionDetection)
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
	BosonModel* m = theme->unitModel(prop->typeId()); // AB: this does a lookup in a list and therefore should be avoided (this method gets called at least whenever the mouse is moved!)
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

void BosonUfoPlacementPreviewWidget::setPlacementCellPreviewData(unsigned int textureCount, unsigned char* alpha, bool canPlace)
{
 // we clear anyway - the new texture will be set below
 d->mPlacementPreview.clear();
 BO_CHECK_NULL_RET(canvas());
 BO_CHECK_NULL_RET(canvas()->map());
 BO_CHECK_NULL_RET(canvas()->map()->texMap());
 BO_CHECK_NULL_RET(canvas()->map()->groundTheme());
 if (textureCount != canvas()->map()->groundTheme()->textureCount()) {
	boError() << k_funcinfo << "texture count is invalid - doesn't fit to groundTheme" << endl;
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

void BosonUfoPlacementPreviewWidget::slotSetPlacementPreviewData(const UnitProperties* prop, bool canPlace, bool freeMode, bool useCollisionDetection)
{
 setPlacementPreviewData(prop, canPlace, freeMode, useCollisionDetection);
}

void BosonUfoPlacementPreviewWidget::slotSetPlacementCellPreviewData(unsigned int textureCount, unsigned char* alpha, bool canPlace)
{
 setPlacementCellPreviewData(textureCount, alpha, canPlace);
}






class BosonUfoLineVisualizationWidgetPrivate
{
public:
	BosonUfoLineVisualizationWidgetPrivate()
	{
		mGameGLMatrices = 0;
		mCanvas = 0;
	}
	const BoGLMatrices* mGameGLMatrices;
	QValueList<BoLineVisualization> mLineVisualizationList;
	const BosonCanvas* mCanvas;
};

BosonUfoLineVisualizationWidget::BosonUfoLineVisualizationWidget()
	: BoUfoCustomWidget()
{
 d = new BosonUfoLineVisualizationWidgetPrivate();

 connect(BosonPathVisualization::pathVisualization(),
		SIGNAL(signalAddLineVisualization( const QValueList<BoVector3Fixed>&, const BoVector4Float&, bofixed, int, bofixed)),
		this,
		SLOT(slotAddLineVisualization(const QValueList<BoVector3Fixed>&, const BoVector4Float&, bofixed, int, bofixed)));
}

BosonUfoLineVisualizationWidget::~BosonUfoLineVisualizationWidget()
{
 delete d;
}

void BosonUfoLineVisualizationWidget::setGameGLMatrices(const BoGLMatrices* m)
{
 d->mGameGLMatrices = m;
}

void BosonUfoLineVisualizationWidget::setCanvas(const BosonCanvas* canvas)
{
 d->mCanvas = canvas;
}

const BosonCanvas* BosonUfoLineVisualizationWidget::canvas() const
{
 return d->mCanvas;
}

void BosonUfoLineVisualizationWidget::paintWidget()
{
 PROFILE_METHOD;
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
 QValueList<BoLineVisualization>::iterator it;
 for (it = d->mLineVisualizationList.begin(); it != d->mLineVisualizationList.end(); ++it) {
	glColor4fv((*it).color.data());
	glPointSize((*it).pointsize);
	glBegin(GL_LINE_STRIP);
	QValueList<BoVector3Fixed>::iterator pit;
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
}

void BosonUfoLineVisualizationWidget::addLineVisualization(BoLineVisualization v)
{
 d->mLineVisualizationList.append(v);
}

void BosonUfoLineVisualizationWidget::slotAddLineVisualization(const QValueList<BoVector3Fixed>& points, const BoVector4Float& color, bofixed pointSize, int timeout, bofixed zOffset)
{
 if (!canvas()) {
	return;
 }
 BoLineVisualization viz;
 viz.pointsize = pointSize;
 viz.timeout = timeout;
 viz.color = color;
 viz.points = points;
 QValueList<BoVector3Fixed>::Iterator it;
 for (it = viz.points.begin(); it != viz.points.end(); ++it) {
	(*it).setZ(canvas()->heightAtPoint((*it).x(), -(*it).y()) + zOffset);
 }
 addLineVisualization(viz);
}

void BosonUfoLineVisualizationWidget::slotAdvance(unsigned int, bool)
{
 advanceLineVisualization();
}

void BosonUfoLineVisualizationWidget::advanceLineVisualization()
{
 QValueList<BoLineVisualization>::iterator it;
 for (it = d->mLineVisualizationList.begin(); it != d->mLineVisualizationList.end(); ++it) {
	(*it).timeout--;
	if ((*it).timeout == 0) {
		// expired - remove it
		d->mLineVisualizationList.erase(it);
		--it;
	}
 }
}



class BosonUfoCursorWidgetPrivate
{
public:
	BosonUfoCursorWidgetPrivate()
	{
		mGameGLMatrices = 0;
		mCursorCollection = 0;
		mCursorWidgetPos = 0;
	}

	const BoGLMatrices* mGameGLMatrices;
	BosonCursorCollection* mCursorCollection;
	const QPoint* mCursorWidgetPos;
};

BosonUfoCursorWidget::BosonUfoCursorWidget()
	: BoUfoCustomWidget()
{
 d = new BosonUfoCursorWidgetPrivate();
 d->mCursorCollection = new BosonCursorCollection(this);
 connect(d->mCursorCollection, SIGNAL(signalSetWidgetCursor(BosonCursor*)),
		this, SIGNAL(signalSetWidgetCursor(BosonCursor*)));
}

BosonUfoCursorWidget::~BosonUfoCursorWidget()
{
 delete d->mCursorCollection;
 delete d;
}

void BosonUfoCursorWidget::setGameGLMatrices(const BoGLMatrices* m)
{
 d->mGameGLMatrices = m;
}

void BosonUfoCursorWidget::setCursorWidgetPos(const QPoint* pos)
{
 d->mCursorWidgetPos = pos;
}

BosonCursor* BosonUfoCursorWidget::cursor() const
{
 return d->mCursorCollection->cursor();
}

void BosonUfoCursorWidget::paintWidget()
{
 PROFILE_METHOD;
 BO_CHECK_NULL_RET(d->mCursorWidgetPos);
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

void BosonUfoCursorWidget::slotChangeCursor(int mode, const QString& cursorDir)
{
 boDebug() << k_funcinfo << endl;
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
 }
}



class BosonUfoSelectionRectWidgetPrivate
{
public:
	BosonUfoSelectionRectWidgetPrivate()
	{
		mGameGLMatrices = 0;
	}
	const BoGLMatrices* mGameGLMatrices;
	bool mSelectionRectVisible;
	QRect mSelectionRect;
};

BosonUfoSelectionRectWidget::BosonUfoSelectionRectWidget()
	: BoUfoCustomWidget()
{
 d = new BosonUfoSelectionRectWidgetPrivate();
 d->mSelectionRectVisible = false;
}

BosonUfoSelectionRectWidget::~BosonUfoSelectionRectWidget()
{
 delete d;
}

void BosonUfoSelectionRectWidget::setGameGLMatrices(const BoGLMatrices* m)
{
 d->mGameGLMatrices = m;
}

void BosonUfoSelectionRectWidget::slotSelectionRectVisible(bool v)
{
 d->mSelectionRectVisible = v;
}

void BosonUfoSelectionRectWidget::slotSelectionRectChanged(const QRect& r)
{
 d->mSelectionRect = r;
}

void BosonUfoSelectionRectWidget::paintWidget()
{
 PROFILE_METHOD;
 BO_CHECK_NULL_RET(d->mGameGLMatrices);
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
	QValueList<float> mData;
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


class BosonUfoFPSGraphWidgetPrivate
{
public:
	BosonUfoFPSGraphWidgetPrivate()
	{
		mGameGLMatrices = 0;
		mGameFPSCounter = 0;
	}
	const BoGLMatrices* mGameGLMatrices;
	const BosonGameFPSCounter* mGameFPSCounter;

	FPSGraphData mFPSData;
	FPSGraphData mSkippedFPSData;
};

BosonUfoFPSGraphWidget::BosonUfoFPSGraphWidget()
	: BoUfoCustomWidget()
{
 d = new BosonUfoFPSGraphWidgetPrivate();
 QTimer* timer = new QTimer(this);
 connect(timer, SIGNAL(timeout()),
		this, SLOT(slotAddData()));

 d->mFPSData.mColor = Qt::green;
 d->mSkippedFPSData.mColor = Qt::red;

 timer->start(100);
}

BosonUfoFPSGraphWidget::~BosonUfoFPSGraphWidget()
{
 delete d;
}

void BosonUfoFPSGraphWidget::setGameGLMatrices(const BoGLMatrices* m)
{
 d->mGameGLMatrices = m;
}

void BosonUfoFPSGraphWidget::setGameFPSCounter(const BosonGameFPSCounter* c)
{
 d->mGameFPSCounter = c;
}

void BosonUfoFPSGraphWidget::paintWidget()
{
 PROFILE_METHOD;
 BO_CHECK_NULL_RET(d->mGameGLMatrices);

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

 paintFPS(d->mFPSData);
 paintFPS(d->mSkippedFPSData);

 glPopAttrib();

 glMatrixMode(GL_PROJECTION);
 glPopMatrix();
 glMatrixMode(GL_MODELVIEW);
 glPopMatrix();
}

void BosonUfoFPSGraphWidget::paintFPS(const FPSGraphData& data)
{
 glColor3ub(data.mColor.red(), data.mColor.green(), data.mColor.blue());

 float widgetHeight = (float)height();

 float x = 0.0f;
 glBegin(GL_LINE_STRIP);
	for (QValueList<float>::const_iterator it = data.mData.begin(); it != data.mData.end(); ++it) {
		float factor = ((*it) - data.mMin) / data.mMax;
		float y = factor * widgetHeight;
		glVertex2f(x, y);

		x += data.mDataWidth;
	}
 glEnd();

 // TODO: add a label describing this data
}

void BosonUfoFPSGraphWidget::slotAddData()
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

