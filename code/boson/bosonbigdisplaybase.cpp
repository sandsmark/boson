/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2004 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bosonbigdisplaybase.h"
#include "bosonbigdisplaybase.moc"

#include "no_player.h"
#include "defines.h"
#include "bosoncanvas.h"
#include "bosoncanvasstatistics.h"
#include "bosonmap.h"
#include "cell.h"
#include "boitemlist.h"
#include "rtti.h"
#include "unit.h"
#include "unitproperties.h"
#include "speciestheme.h"
#include "playerio.h"
#include "bosoncursor.h"
#include "boselection.h"
#include "bosonconfig.h"
#include "selectbox.h"
#include "bosonglchat.h"
#include "bosonprofiling.h"
#include "bosoneffect.h"
#include "bosoneffectparticle.h"
#include "boson.h"
#include "bodebug.h"
#include "items/bosonshot.h"
#include "items/bosonitemrenderer.h"
#include "unitplugins.h"
#include "bosonmodel.h"
#include "bo3dtools.h"
#include "bosonbigdisplayinputbase.h"
#include "bogltooltip.h"
#include "bosongroundtheme.h"
#include "bocamera.h"
#include "boautocamera.h"
#include "bogroundrenderer.h"
#include "bogroundrenderermanager.h"
#include "bomeshrenderermanager.h"
#include "bolight.h"
#include "bosonglminimap.h"
#include "bomaterial.h"
#include "info/boinfo.h"
#include "script/bosonscript.h"
#include "script/bosonscriptinterface.h"
#include "bosonpath.h"
#include "bofullscreen.h"
#include "speciesdata.h"
#include "bowater.h"
#include "botexture.h"
#include "boufo/boufo.h"
#include "boufo/boufoaction.h"
#include "bosonufochat.h"
#include "bosonufominimap.h"
#include "commandframe/bosoncommandframe.h"
#include "bosonufogamewidget.h"
#include "bosonlocalplayerinput.h"
#include "bofullscreen.h"
#include "sound/bosonaudiointerface.h"
#include "bodebuglogdialog.h"
#include "bosonprofilingdialog.h"
#include "bosonplayfield.h"
#include "boglstatewidget.h"
#include "bosondata.h"
#include "boconditionwidget.h"
#include "bocamerawidget.h"
#include "bosonmessage.h"
#include "kgameunitdebug.h"
#include "kgameplayerdebug.h"
#include "kgameadvancemessagesdebug.h"
#include "bpfdescriptiondialog.h"
#include "botexmapimportdialog.h"
#include "optionsdialog.h"
#include "boaction.h"
#include "boeventlistener.h"
#ifdef BOSON_USE_BOMEMORY
#include "bomemory/bomemorydialog.h"
#endif

#include <kgame/kgameio.h>
#include <kgame/kplayer.h>
#include <kgame/kgamedebugdialog.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include <kshortcut.h>
#include <kfiledialog.h>
#include "boeventloop.h"

#include <qtimer.h>
#include <qcursor.h>
#include <qpointarray.h>
#include <qbuffer.h>
#include <qimage.h>
#include <qdir.h>
#include <qdom.h>
#include <qguardedptr.h>
#include <qsignalmapper.h>
#include <qlayout.h>
#include <qptrdict.h>
#include <qinputdialog.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qdialog.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qvaluestack.h>

#if HAVE_SYS_TIME_H
#include <sys/time.h>
#else
// won't compile anymore!
#warning You dont have sys/time.h - please report this problem to boson-devel@lists.sourceforge.net and provide us with information about your system!
#endif
//#include <iostream.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>


#include "bosonfont/bosonglfont.h"

#include <GL/glu.h>

#if HAVE_GL_GLEXT_H
#include <GL/glext.h>
#endif



#define ID_DEBUG_KILLPLAYER 0
#define ID_DEBUG_ADD_10000_MINERALS 1
#define ID_DEBUG_ADD_1000_MINERALS 2
#define ID_DEBUG_SUB_1000_MINERALS 3
#define ID_DEBUG_ADD_10000_OIL 4
#define ID_DEBUG_ADD_1000_OIL 5
#define ID_DEBUG_SUB_1000_OIL 6



class BoVisibleEffects
{
public:
	void clearAll()
	{
		// note that mParticelList is a special case and is not cleared
		mFogEffects.clear();
		mParticles.clear();
		mBulletEffects.clear();
		mFadeEffects.clear();
		mAll.clear();
	}

	QPtrList<BosonEffectFog> mFogEffects;
	QPtrList<BosonEffectParticle> mParticles;
	QPtrList<BosonEffectBulletTrail> mBulletEffects;
	QPtrList<BosonEffectFade> mFadeEffects;

	QPtrList<BosonEffect> mAll;

	BoParticleList mParticleList;
	bool mParticlesDirty;
};

static void updateEffects(BoVisibleEffects& v);


class BosonCanvasRendererPrivate
{
public:
	BosonCanvasRendererPrivate(const BoMatrix& modelview)
		: mModelviewMatrix(modelview)
	{
		mRenderItemList = 0;
		mSelectBoxData = 0;

		mCamera = 0;
		mLocalPlayerIO = 0;
		mViewFrustum = 0;
		mViewport = 0;
	}
	BoItemList* mRenderItemList;
	SelectBoxData* mSelectBoxData;
	BoVisibleEffects mVisibleEffects;
	unsigned int mRenderedItems;
	unsigned int mRenderedCells;
	unsigned int mRenderedParticles;
	int mTextureBindsCells;
	int mTextureBindsItems;
	int mTextureBindsWater;
	int mTextureBindsParticles;

	const BoMatrix& mModelviewMatrix;
	GLfloat* mViewFrustum;
	GLint* mViewport;
	BoGameCamera* mCamera;
	PlayerIO* mLocalPlayerIO;
};

BosonCanvasRenderer::BosonCanvasRenderer(const BoMatrix& modelviewMatrix, GLfloat* viewFrustum, GLint* viewport)
{
 d = new BosonCanvasRendererPrivate(modelviewMatrix);
 d->mViewFrustum = viewFrustum;
 d->mViewport = viewport;
 d->mRenderItemList = new BoItemList(1, false);
 d->mRenderedItems = 0;
 d->mRenderedCells = 0;
 d->mRenderedParticles = 0;
 d->mTextureBindsCells = 0;
 d->mTextureBindsItems = 0;
 d->mTextureBindsWater = 0;
 d->mTextureBindsParticles = 0;

 d->mVisibleEffects.mParticlesDirty = true;
}

BosonCanvasRenderer::~BosonCanvasRenderer()
{
 delete d->mRenderItemList;
 delete d->mSelectBoxData;
 delete d;
}

void BosonCanvasRenderer::initGL()
{
 d->mSelectBoxData = new SelectBoxData();
}

void BosonCanvasRenderer::setCamera(BoGameCamera* camera)
{
 d->mCamera = camera;
}

BoGameCamera* BosonCanvasRenderer::camera() const
{
 return d->mCamera;
}

void BosonCanvasRenderer::setLocalPlayerIO(PlayerIO* io)
{
 d->mLocalPlayerIO = io;
}

PlayerIO* BosonCanvasRenderer::localPlayerIO() const
{
 return d->mLocalPlayerIO;
}

GLfloat* BosonCanvasRenderer::viewFrustum() const
{
 return d->mViewFrustum;
}

unsigned int BosonCanvasRenderer::renderedItems() const
{
 return d->mRenderedItems;
}

unsigned int BosonCanvasRenderer::renderedCells() const
{
 return d->mRenderedCells;
}

unsigned int BosonCanvasRenderer::renderedParticles() const
{
 return d->mRenderedParticles;
}

int BosonCanvasRenderer::textureBindsCells() const
{
 return d->mTextureBindsCells;
}

int BosonCanvasRenderer::textureBindsItems() const
{
 return d->mTextureBindsItems;
}

int BosonCanvasRenderer::textureBindsWater() const
{
 return d->mTextureBindsWater;
}

int BosonCanvasRenderer::textureBindsParticles() const
{
 return d->mTextureBindsParticles;
}

void BosonCanvasRenderer::setParticlesDirty(bool dirty)
{
 d->mVisibleEffects.mParticlesDirty = dirty;
}

void BosonCanvasRenderer::reset()
{
 d->mVisibleEffects.mParticleList.clear();
}

void BosonCanvasRenderer::paintGL(const BosonCanvas* canvas)
{
 BO_CHECK_NULL_RET(viewFrustum());
 BO_CHECK_NULL_RET(localPlayerIO());
 BO_CHECK_NULL_RET(camera());
 BO_CHECK_NULL_RET(d->mSelectBoxData);
 BO_CHECK_NULL_RET(canvas);

 d->mRenderedItems = 0;
 d->mRenderedCells = 0;
 d->mRenderedParticles = 0;
 d->mTextureBindsCells = 0;
 d->mTextureBindsItems = 0;
 d->mTextureBindsWater = 0;
 d->mTextureBindsParticles = 0;

 createVisibleEffectsList(&d->mVisibleEffects, *canvas->effects(), canvas->mapWidth(), canvas->mapHeight());
 updateEffects(d->mVisibleEffects);

 renderFog(d->mVisibleEffects);

 boProfiling->renderCells(true);
 renderGround(canvas->map());
 boProfiling->renderCells(false);

 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "after ground rendering" << endl;
 }


 boProfiling->renderUnits(true);
 renderItems(canvas->allItems());
 boProfiling->renderUnits(false, d->mRenderedItems);

 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "after item rendering" << endl;
 }

 boProfiling->renderWater(true);
 renderWater();
 boProfiling->renderWater(false);

 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "after water rendering" << endl;
 }

 // Render particle systems
 boProfiling->renderParticles(true);
 renderParticles(d->mVisibleEffects);
 boProfiling->renderParticles(false);

 glDisable(GL_DEPTH_TEST);
 glDisable(GL_LIGHTING);
 glDisable(GL_NORMALIZE);

 renderBulletTrailEffects(d->mVisibleEffects);

 glMatrixMode(GL_PROJECTION);
 glPushMatrix();
 glLoadIdentity();
 gluOrtho2D(0.0, (GLfloat)d->mViewport[2], 0.0, (GLfloat)d->mViewport[3]);
 glMatrixMode(GL_MODELVIEW);
 glPushMatrix();
 glLoadIdentity();
 glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

 renderFadeEffects(d->mVisibleEffects);

 glMatrixMode(GL_PROJECTION);
 glPopMatrix();
 glMatrixMode(GL_MODELVIEW);
 glPopMatrix();
}

void BosonCanvasRenderer::renderGround(const BosonMap* map)
{
 BO_CHECK_NULL_RET(map);
 BoTextureManager::BoTextureBindCounter bindCounter(boTextureManager, &d->mTextureBindsCells);
 glEnable(GL_DEPTH_TEST);
 if (boConfig->useLight()) {
	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
 } else {
	glDisable(GL_COLOR_MATERIAL);
 }

 BO_CHECK_NULL_RET(BoGroundRendererManager::manager()->currentRenderer());
 d->mRenderedCells = BoGroundRendererManager::manager()->currentRenderer()->renderCells(map);

 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "OpenGL error" << endl;
 }
}

void BosonCanvasRenderer::renderBoundingBox(const BosonItem* item)
{
 // Corners of bb of item
 BoVector3Float c1(item->x(), item->y(), item->z());
 c1.canvasToWorld();
 BoVector3Float c2(item->x() + item->width(), item->y() + item->height(), item->z() + item->depth());
 c2.canvasToWorld();
 renderBoundingBox(c1, c2);
}

void BosonCanvasRenderer::renderBoundingBox(const BoVector3Float& c1, const BoVector3Float& c2)
{
 boTextureManager->disableTexturing();
 glLineWidth(1.0);
 glBegin(GL_LINES);
	glVertex3f(c1.x(), c1.y(), c1.z());  glVertex3f(c2.x(), c1.y(), c1.z());
	glVertex3f(c2.x(), c1.y(), c1.z());  glVertex3f(c2.x(), c2.y(), c1.z());
	glVertex3f(c2.x(), c2.y(), c1.z());  glVertex3f(c1.x(), c2.y(), c1.z());
	glVertex3f(c1.x(), c2.y(), c1.z());  glVertex3f(c1.x(), c1.y(), c1.z());

	glVertex3f(c1.x(), c1.y(), c2.z());  glVertex3f(c2.x(), c1.y(), c2.z());
	glVertex3f(c2.x(), c1.y(), c2.z());  glVertex3f(c2.x(), c2.y(), c2.z());
	glVertex3f(c2.x(), c2.y(), c2.z());  glVertex3f(c1.x(), c2.y(), c2.z());
	glVertex3f(c1.x(), c2.y(), c2.z());  glVertex3f(c1.x(), c1.y(), c2.z());

	glVertex3f(c1.x(), c1.y(), c1.z());  glVertex3f(c1.x(), c1.y(), c2.z());
	glVertex3f(c2.x(), c1.y(), c1.z());  glVertex3f(c2.x(), c1.y(), c2.z());
	glVertex3f(c2.x(), c2.y(), c1.z());  glVertex3f(c2.x(), c2.y(), c2.z());
	glVertex3f(c1.x(), c2.y(), c1.z());  glVertex3f(c1.x(), c2.y(), c2.z());
 glEnd();
}

void BosonCanvasRenderer::createRenderItemList(BoItemList* renderItemList, const BoItemList* allItems)
{
 BO_CHECK_NULL_RET(viewFrustum());
 BO_CHECK_NULL_RET(localPlayerIO());
 renderItemList->clear();
 BoItemList::const_iterator it = allItems->begin();
 for (; it != allItems->end(); ++it) {
	BosonItem* item = *it;

	if (!item->isVisible() || !item->itemRenderer()) {
		continue;
	}

	// TODO: performance: we can improve this greatly:
	// simply group the items to bigger sphere or boxes. every box is of
	// size of (maybe) 10.0*10.0. We maintain a list of items for *every*
	// box. we can simply test if the box is in the frustum and if so we
	// test every item of that list. if not we can skip every item of that
	// box.
	// Especially in bigger games with big maps and several hundred units
	// this would be a great speedup.
	// UPDATE: probably not *that* big, as rendering itself is a bigger
	// bottleneck.

	// UPDATE: we could instead use the "sectors" that we are planning to
	// use for collision detection and pathfinding also for the frustum
	// tests (they wouldn't do floating point calculations)
	if (!item->itemInFrustum(viewFrustum())) {
		// the unit is not visible, currently. no need to draw anything.
		continue;
	}

	// AB: note units are rendered in the *center* point of their
	// width/height.
	// but concerning z-position they are rendered from bottom to top!

	bool visible = localPlayerIO()->canSee(item);
	if (visible) {
		renderItemList->append(*it);
	}
 }
}

void BosonCanvasRenderer::renderItems(const BoItemList* allCanvasItems)
{
 BoTextureManager::BoTextureBindCounter bindCounter(boTextureManager, &d->mTextureBindsItems);
 BosonItemRenderer::startItemRendering();
 if (boConfig->wireFrames()) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
 }
 glEnable(GL_DEPTH_TEST);
 if (boConfig->useLight()) {
	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);
 }

 createRenderItemList(d->mRenderItemList, allCanvasItems); // AB: this is very fast. < 1.5ms on experimental5 for me

 bool useLOD = boConfig->useLOD();

 BoItemList::Iterator it = d->mRenderItemList->begin();
 for (; it != d->mRenderItemList->end(); ++it) {
	BosonItem* item = *it;

	// FIXME: can't we use BoVector3 and it's conversion methods here?
	GLfloat x = (item->x() + item->width() / 2);
	GLfloat y = -((item->y() + item->height() / 2));
	GLfloat z = item->z(); // this is already in the correct format!

	// AB: note units are rendered in the *center* point of their
	// width/height.
	// but concerning z-position they are rendered from bottom to top!

	glPushMatrix();
	glTranslatef(x, y, z);
	glRotatef(-(item->rotation()), 0.0, 0.0, 1.0);
	glRotatef(item->xRotation(), 1.0, 0.0, 0.0);
	glRotatef(item->yRotation(), 0.0, 1.0, 0.0);

	// Units will be tinted accordingly to how much health they have left
	if (RTTI::isUnit(item->rtti())) {
		if (((Unit*)item)->isDestroyed()) {
			glColor3f(0.4f, 0.4f, 0.4f);
		} else {
			float f = ((Unit*)item)->health() / (float)((Unit*)item)->unitProperties()->health() * 0.3;
			glColor3f(0.7f + f, 0.7f + f, 0.7f + f);
		}
	} else {
		glColor3ub(255, 255, 255);
	}

	unsigned int lod = 0;
	if (useLOD) {
		// TODO: we could compare squared distances here and get rid of sqrt()
		float dist = (camera()->cameraPos() - BoVector3Float(x, y, z)).length();
		lod = item->preferredLod(dist);
	}
	item->renderItem(lod);
	glColor3ub(255, 255, 255);
	glPopMatrix();

	if (boConfig->debugBoundingBoxes()) {
		renderBoundingBox(item);
	}
 }

 BoItemList* selectedItems = new BoItemList(0, false);
 createSelectionsList(selectedItems, d->mRenderItemList);
 renderSelections(selectedItems);
 delete selectedItems;
 selectedItems = 0;

 boTextureManager->invalidateCache();
 d->mRenderedItems += d->mRenderItemList->count();
 d->mRenderItemList->clear();

 BosonItemRenderer::stopItemRendering();
 if (boConfig->wireFrames()) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
 }
}

void BosonCanvasRenderer::createSelectionsList(BoItemList* selectedItems, const BoItemList* items)
{
 selectedItems->clear();
 BoItemList::const_iterator it = items->begin();
 for (; it != d->mRenderItemList->end(); ++it) {
	BosonItem* item = *it;
	if (item->isSelected()) {
		selectedItems->append(item);
	}
 }
}

void BosonCanvasRenderer::renderSelections(const BoItemList* selectedItems)
{
 BO_CHECK_NULL_RET(d->mSelectBoxData);
 BoItemList::const_iterator it = selectedItems->begin();
 glPushAttrib(GL_ENABLE_BIT | GL_TEXTURE_BIT);
 glDisable(GL_LIGHTING);
 glDisable(GL_NORMALIZE);
 BoMaterial::deactivate();
 while (it != selectedItems->end()) {
	BosonItem* item = *it;
	if (!item->isSelected()) {
		boError() << k_funcinfo << "not selected" << endl;
		++it;
		continue;
	}
	if (!item->selectBox()) {
		boError() << k_funcinfo << "selected but NULL selectBox" << endl;
		++it;
		continue;
	}

	GLfloat x = (item->x() + item->width() / 2);
	GLfloat y = -((item->y() + item->height() / 2));
	GLfloat z = item->z();

	GLfloat w = ((float)item->width());
	GLfloat h = ((float)item->height());
	GLfloat depth = item->glDepthMultiplier();
	glPushMatrix();
	glTranslatef(x, y, z);
	if (w != 1.0 || h != 1.0 || depth != 1.0) {
		glScalef(w, h, depth);
	}
	if (boConfig->alignSelectionBoxes()) {
		glRotatef(camera()->rotation(), 0.0, 0.0, 1.0);
	}
	GLuint list = d->mSelectBoxData->list(item->selectBox()->factor());
	glCallList(list);
	glPopMatrix();
	Unit* u = 0;
	if (RTTI::isUnit(item->rtti())) {
		u = (Unit*)item;
	}
/*
	if (u && u->waypointList().count() > 0) {
		// render a line from the current position of the unit to the
		// point it is moving to.
		// TODO: render one vertex per cell or so. this would fix
		// problem with heightmaps, when a line goes through mountains.
		// speed is hardly relevant at s point (rendering a few small
		// lines is fast).
		glColor3ub(0, 255, 0);
		QValueList<QPoint> list = u->waypointList();
		list.prepend(QPoint((int)(u->x() + u->width() / 2), (int)(u->y() + u->width() / 2)));
		renderPathLines(list, u->isFlying(), u->z());
	}
	if (u && u->pathPointList().count() > 0) {
		// render a line from the current position of the unit to the
		// point it is moving to.
		// TODO: render one vertex per cell or so. this would fix
		// problem with heightmaps, when a line goes through mountains.
		// speed is hardly relevant at this point (rendering a few small
		// lines is fast).
		QValueList<QPoint> list = u->pathPointList();
		list.prepend(QPoint((int)(u->x() + u->width() / 2), (int)(u->y() + u->width() / 2)));
		glColor3ub(255, 0, 0);
		renderPathLines(list, u->isFlying(), u->z());
	}
*/
	glColor3ub(255, 255, 255);

	++it;
 }
 glPopAttrib();
 boTextureManager->invalidateCache();
}

void BosonCanvasRenderer::createVisibleEffectsList(BoVisibleEffects* v, const QPtrList<BosonEffect>& allEffects, unsigned int mapWidth, unsigned int mapHeight)
{
 v->clearAll();

 QPtrListIterator<BosonEffect> it(allEffects);
 while (it.current()) {
	if (!it.current()->hasStarted()) {
		// nothing to do. effect hasn't started yet.
	} else if (it.current()->type() == BosonEffect::Fog) {
		v->mFogEffects.append((BosonEffectFog*)it.current());
		v->mAll.append(it.current());
	} else if (it.current()->type() == BosonEffect::BulletTrail) {
		// FIXME: in frustum?
		v->mBulletEffects.append((BosonEffectBulletTrail*)it.current());
		v->mAll.append(it.current());
	} else if (it.current()->type() == BosonEffect::Fade) {
		v->mFadeEffects.append((BosonEffectFade*)it.current());
		v->mAll.append(it.current());
	} else if (it.current()->type() > BosonEffect::Particle) {
		BosonEffectParticle* s = (BosonEffectParticle*)it.current();
		//boDebug(150) << k_funcinfo << "System: " << s << "; radius: " << s->boundingSphereRadius() << endl;
		// TODO: maybe we should just add particleDist() to bounding sphere radius
		//  of the system?
		if (Bo3dTools::sphereInFrustum(viewFrustum(), s->position(), s->boundingSphereRadius())) {
			if (!s->testFogged() ||
					(((s->position().x() < mapWidth) && (-s->position().y() < mapHeight)) &&
					localPlayerIO()->canSee((int)s->position().x(), -(int)s->position().y()))) {
				v->mParticles.append(s);
				v->mAll.append(it.current());
			}
		}
	} else if (it.current()->type() == BosonEffect::Light) {
		// Do nothing. Lights are not handled here, this is here just to avoid the
		//  warning.
	} else {
		boWarning() << k_funcinfo << "unexpected type " << it.current()->type();
		v->mAll.append(it.current());
	}
	++it;
 }
}

void BosonCanvasRenderer::renderWater()
{
 BoTextureManager::BoTextureBindCounter bindCounter(boTextureManager, &d->mTextureBindsWater);
 boWaterManager->render();
}

void BosonCanvasRenderer::renderFog(BoVisibleEffects& visible)
{
 // TODO: support multiple fog effects (ATM only 1st one is rendered)
 if (!visible.mFogEffects.isEmpty()) {
	BosonEffectFog* f = visible.mFogEffects.first();
	glEnable(GL_FOG);
	glFogfv(GL_FOG_COLOR, f->color().data());
	glFogf(GL_FOG_START, f->startDistance());
	glFogf(GL_FOG_END, f->endDistance());
	glFogi(GL_FOG_MODE, GL_LINEAR);
 } else {
	// Disable fog
	glDisable(GL_FOG);
 }
}

void BosonCanvasRenderer::renderParticles(BoVisibleEffects& visible)
{
 BO_CHECK_NULL_RET(localPlayerIO());
 BoTextureManager::BoTextureBindCounter bindCounter(boTextureManager, &d->mTextureBindsParticles);
 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "OpenGL error x5" << endl;
 }
 // Return if there aren't any effects
 if (visible.mParticles.isEmpty()) {
	return;
 }

 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "OpenGL error x4" << endl;
 }
 // Resort list of particles if needed
 // This sorts all particles by distance from camera and may be pretty slow, so
 //  we don't resort the list if there hasn't been any advance() calls and
 //  camera hasn't changed either
 BosonParticle* p = 0;
 //bool wassorted = d->mVisibleEffects.mParticlesDirty;  // only for debug, commented because of compiler warning
 if (visible.mParticlesDirty) {
	float x, y, z;
	BoVector3Fixed dir;
	visible.mParticleList.clear();
	// Add all particles to the list
	QPtrListIterator<BosonEffectParticle> visibleIt(visible.mParticles);
	BosonEffectParticle* s = 0;
	for (; visibleIt.current(); ++visibleIt) {
		s = visibleIt.current();
		// If particleDist is non-zero, calculate vector for moving particles closer
		//  to camera
		if (s->particleDist() != 0.0f) {
			dir = (camera()->cameraPos() - s->position().toFloat()).toFixed();
			dir.scale(s->particleDist() / dir.length());
			s->setParticleDistVector(dir);
		}

		for (unsigned int i = 0; i < s->particleCount(); i++) {
			if (s->particle(i)->life > 0.0) {
				p = s->particle(i);
				// Calculate distance from camera. Note that for performance reasons,
				//  we don't calculate actual distance, but square of it.
				x = p->pos.x() - camera()->cameraPos().x();
				y = p->pos.y() - camera()->cameraPos().y();
				z = p->pos.z() - camera()->cameraPos().z();
				p->distance = (x*x + y*y + z*z);
				visible.mParticleList.append(p);
			}
		}
	}

	if (visible.mParticleList.count() == 0) {
		return;
	}

	// Sort the list
	visible.mParticleList.sort();
	setParticlesDirty(false);
 }

 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "OpenGL error x3" << endl;
 }
 /// Draw particles
 glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_CURRENT_BIT);
 glEnable(GL_DEPTH_TEST);
 glDepthMask(GL_FALSE);
 glEnable(GL_BLEND);
 glDisable(GL_LIGHTING);
 glDisable(GL_NORMALIZE);
 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "OpenGL error x2" << endl;
 }

 // Matrix stuff for aligned particles
 BoVector3Fixed x(d->mModelviewMatrix[0], d->mModelviewMatrix[4], d->mModelviewMatrix[8]);
 BoVector3Fixed y(d->mModelviewMatrix[1], d->mModelviewMatrix[5], d->mModelviewMatrix[9]);

 // Some cache variables
 int blendfunc = -1;
 BoTexture* texture = 0;
 bool betweenbeginend = false;  // If glBegin has been called, but glEnd() hasn't. Very hackish.
 BoVector3Fixed a, b, c, e;  // Vertex positions. e is used instead of d which clashes with private class

 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "OpenGL error x1" << endl;
 }
 QPtrListIterator<BosonParticle> it(visible.mParticleList);
 //boDebug(150) << k_funcinfo << "Drawing " << i.count() << " particles" << endl;
 for (; it.current(); ++it) {
	p = it.current();
	// We change blend function and texture only if it's necessary
	if (blendfunc != p->system->blendFunc()[1]) {
		// Note that we only check for dest blending function currently, because src
		//  is always same. If this changes in the future, change this as well!
		if (betweenbeginend) {
			glEnd();
			betweenbeginend = false;
		}
		glBlendFunc(p->system->blendFunc()[0], p->system->blendFunc()[1]);
		blendfunc = p->system->blendFunc()[1];
	}
	if (texture != p->tex) {
		if (betweenbeginend) {
			glEnd();
			betweenbeginend = false;
		}
		p->tex->bind();
		texture = p->tex;
	}
	if (!betweenbeginend) {
		glBegin(GL_QUADS);
		betweenbeginend = true;
	}

	// Is it worth to duplicate this code just to save few vector additions for
	//  systems with particleDist() == 0.0 ?
	if (p->system->particleDist() != 0.0) {
		if (p->system->alignParticles()) {
			a = p->pos + ((-x + y) * p->size) + p->system->particleDistVector();
			b = p->pos + (( x + y) * p->size) + p->system->particleDistVector();
			c = p->pos + (( x - y) * p->size + p->system->particleDistVector());
			e = p->pos + ((-x - y) * p->size) + p->system->particleDistVector();
		} else {
			a = p->pos + (BoVector3Fixed(-0.5, 0.5, 0.0) * p->size) + p->system->particleDistVector();
			b = p->pos + (BoVector3Fixed(0.5, 0.5, 0.0) * p->size) + p->system->particleDistVector();
			c = p->pos + (BoVector3Fixed(0.5, -0.5, 0.0) * p->size) + p->system->particleDistVector();
			e = p->pos + (BoVector3Fixed(-0.5, -0.5, 0.0) * p->size) + p->system->particleDistVector();
		}
	} else {
		if (p->system->alignParticles()) {
			a = p->pos + ((-x + y) * p->size);
			b = p->pos + (( x + y) * p->size);
			c = p->pos + (( x - y) * p->size);
			e = p->pos + ((-x - y) * p->size);
		} else {
			a = p->pos + (BoVector3Fixed(-0.5, 0.5, 0.0) * p->size);
			b = p->pos + (BoVector3Fixed(0.5, 0.5, 0.0) * p->size);
			c = p->pos + (BoVector3Fixed(0.5, -0.5, 0.0) * p->size);
			e = p->pos + (BoVector3Fixed(-0.5, -0.5, 0.0) * p->size);
		}
	}

	glColor4fv(p->color.data());  // Is it worth to cache color as well?
	glTexCoord2f(0.0, 1.0);  glVertex3fv(a.toFloat().data());
	glTexCoord2f(1.0, 1.0);  glVertex3fv(b.toFloat().data());
	glTexCoord2f(1.0, 0.0);  glVertex3fv(c.toFloat().data());
	glTexCoord2f(0.0, 0.0);  glVertex3fv(e.toFloat().data());
	d->mRenderedParticles++;
 }
 glEnd();
 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "OpenGL error 1" << endl;
 }

 // reset values
 glColor4ub(255, 255, 255, 255);
 glDepthMask(GL_TRUE);
 glPopAttrib();
 boTextureManager->invalidateCache();

 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "OpenGL error" << endl;
 }
}

void BosonCanvasRenderer::renderBulletTrailEffects(BoVisibleEffects& visible)
{
 if (!visible.mBulletEffects.isEmpty()) {
	BosonEffectBulletTrail* b;
	float currentwidth = -1.0f;
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBegin(GL_LINES);
	QPtrListIterator<BosonEffectBulletTrail> it(visible.mBulletEffects);
	while (it.current()) {
		b = it.current();
		if (b->width() != currentwidth) {
			glEnd();
			glLineWidth(b->width());
			currentwidth = b->width();
			glBegin(GL_LINES);
		}
		glColor4fv(b->color().data());
		glVertex3fv(b->startPoint().toFloat().data());
		glVertex3fv(b->endPoint().toFloat().data());
		++it;
	}
	glEnd();
	glDisable(GL_BLEND);
	glColor3ub(255, 255, 255);
 }
}

void BosonCanvasRenderer::renderFadeEffects(BoVisibleEffects& visible)
{
 if (!visible.mFadeEffects.isEmpty()) {
	BosonEffectFade* f;
//	glMatrixMode(GL_PROJECTION);
//	glPushMatrix();
	// Scale so that (0; 0) is bottom-left corner of the viewport and (1; 1) is
	//  top-right corner
//	glScalef(1 / (GLfloat)d->mViewport[2], 1 / (GLfloat)d->mViewport[3], 1);
	// FIXME!!!
	float xscale = (GLfloat)d->mViewport[2];
	float yscale = (GLfloat)d->mViewport[3];
	glEnable(GL_BLEND);
	boTextureManager->disableTexturing();
	QPtrListIterator<BosonEffectFade> it(visible.mFadeEffects);
	while (it.current()) {
		f = it.current();
		glBlendFunc(f->blendFunc()[0], f->blendFunc()[1]);
		glColor4fv(f->color().data());
		BoVector4Fixed geo = f->geometry();  // x, y, w, h
		glRectf(geo[0] * xscale, geo[1] * yscale, (geo[0] + geo[2]) * xscale, (geo[1] + geo[3]) * yscale);  // x, y, x2, y2
		++it;
	}
	glDisable(GL_BLEND);
	glColor3ub(255, 255, 255);
//	glPopMatrix();
//	glMatrixMode(GL_MODELVIEW);
 }
}


float textureUpperLeft[2] = { 0.0f, 1.0f };
float textureLowerLeft[2] = { 0.0f, 0.0f };
float textureLowerRight[2] = { 1.0f, 0.0f };
float textureUpperRight[2] = { 1.0f, 1.0f };

class SelectionRect
{
public:
	SelectionRect()
	{
		mVisible = false;
	}

	void widgetRect(QRect* rect) const
	{
		QRect r(mStartPos, mEndPos);
		*rect = r.normalize();
	}

	void setStartWidgetPos(const QPoint& pos)
	{
		mStartPos = pos;
		setEndWidgetPos(mStartPos);
	}

	void setEndWidgetPos(const QPoint& pos)
	{
		mEndPos = pos;
	}
	const QPoint& startPos() const
	{
		return mStartPos;
	}

	bool isVisible() const
	{
		return mVisible;
	}
	void setVisible(bool v)
	{
		mVisible = v;
	}
private:
	QPoint mStartPos;
	QPoint mEndPos;
	bool mVisible;
};


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
		if (mPlacementPreviewModel && mPlacementPreviewModel->frame(0) &&
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
	void setCanvasPos(const BoVector2Fixed& pos)
	{
		mCanvasPos = pos;
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

private:
	const UnitProperties* mPlacementPreviewProperties;
	BosonModel* mPlacementPreviewModel;
	bool mCanPlace;
	BoVector2Fixed mCanvasPos;
//	GLuint mCellPlacementTexture;
	unsigned int mGroundTextureCount;
	unsigned char* mGroundTextureAlpha;
};


class BosonBigDisplayBase::BosonBigDisplayBasePrivate
{
public:
	BosonBigDisplayBasePrivate()
	{
		mLocalPlayerIO = 0;
		mMouseIO = 0;
		mInput = 0;

		mCanvasRenderer = 0;

		mDefaultFont = 0;

		mFpsTime = 0;
		mFps = 0;
		mFrameCount = 0;

		mToolTips = 0;

		mControlPressed = false;
		mShiftPressed = false;

		mGLMiniMap = 0;

		mScriptConnector = 0;

		mLightWidget = 0;

		mUfoGameWidget = 0;

#if 0
		mActionMenubar = 0;
#endif
		mActionStatusbar = 0;
		mActionDebugPlayers = 0;
		mActionChat = 0;
		mActionFullScreen = 0;
		mActionEditorPlayer = 0;
		mActionEditorPlace = 0;

		mSelectMapper = 0;
		mCreateMapper = 0;
	}

	PlayerIO* mLocalPlayerIO;
	KGameMouseIO* mMouseIO;
	BosonBigDisplayInputBase* mInput;
	QIntDict<BoSelection> mSelectionGroups;

	QTimer mCursorEdgeTimer;
	int mCursorEdgeCounter;

	BosonCanvasRenderer* mCanvasRenderer;

	BoGameCamera mCamera;

	GLint mViewport[4]; // x,y,w,h of the viewport. see setViewport
	BoMatrix mProjectionMatrix;
	BoMatrix mModelviewMatrix;
	GLfloat mViewFrustum[6 * 4];

	GLfloat mFovY; // see gluPerspective
	GLfloat mAspect; // see gluPerspective

	BosonGLFont* mDefaultFont;// AB: maybe we should support several fonts

	bool mGrabMovie;

	long long int mFpsTime;
	double mFps;
	unsigned int mFrameCount;

	SelectionRect mSelectionRect;
	BoMouseMoveDiff mMouseMoveDiff;

	QTimer mUpdateTimer;
	int mUpdateInterval;

	BoVector3Fixed mCanvasVector;

	PlacementPreview mPlacementPreview;
	BoGLToolTip* mToolTips;

	bool mInputInitialized;

	int mTextureBindsParticles;

	bool mControlPressed;
	bool mShiftPressed;

	BosonGLMiniMap* mGLMiniMap;

	QValueList<BoLineVisualization> mLineVisualizationList;

	BosonBigDisplayScriptConnector* mScriptConnector;

	BoLightCameraWidget1* mLightWidget;

	BosonUfoGameWidget* mUfoGameWidget;

#if 0
	QGuardedPtr<BoUfoToggleAction> mActionMenubar;
#endif
	QGuardedPtr<BoUfoToggleAction> mActionStatusbar;
	QGuardedPtr<BoUfoActionMenu> mActionDebugPlayers;
	QGuardedPtr<BoUfoToggleAction> mActionChat;
	QGuardedPtr<BoUfoToggleAction> mActionFullScreen;
	QGuardedPtr<BoUfoSelectAction> mActionEditorPlayer;
	QGuardedPtr<BoUfoSelectAction> mActionEditorPlace;
	QGuardedPtr<BoUfoToggleAction> mActionEditorChangeHeight;
	QPtrList<KPlayer> mEditorPlayers;
	QPtrDict<KPlayer> mActionDebugPlayer2Player;
	QSignalMapper* mSelectMapper;
	QSignalMapper* mCreateMapper;
};

BosonBigDisplayBase::BosonBigDisplayBase(QWidget* parent)
		: BosonUfoGLWidget(parent, "bigdisplay", boConfig->wantDirect())
{
 boDebug() << k_funcinfo << endl;
 init();
 initGL();
}

BosonBigDisplayBase::~BosonBigDisplayBase()
{
 boDebug() << k_funcinfo << endl;

 // the bigdisplay destructor is the place where many systems tend to crash - so
 // we go back to the original (non-fullscreen) mode here
 BoFullScreen::enterOriginalMode();

 qApp->setGlobalMouseTracking(false);
 qApp->removeEventFilter(this);

 quitGame();
 d->mSelectionGroups.clear();
 delete d->mLightWidget;
 delete d->mScriptConnector;
 BoMeshRendererManager::manager()->unsetCurrentRenderer();
 BoGroundRendererManager::manager()->unsetCurrentRenderer();
 delete mSelection;
 delete d->mDefaultFont;
 delete d->mToolTips;
 delete d->mGLMiniMap;
 delete d->mCanvasRenderer;
 delete d->mUfoGameWidget;
 SpeciesData::clearSpeciesData();
 delete d;
 delete mCursor;
 boDebug() << k_funcinfo << "done" << endl;
}

void BosonBigDisplayBase::init()
{
 d = new BosonBigDisplayBasePrivate;
 setSendEventsToUfo(false); // we handle libufo events manually
 mCanvas = 0;
 mCursor = 0;
 d->mCursorEdgeCounter = 0;
 d->mUpdateInterval = 0;
 d->mInputInitialized = false;
 d->mFovY = 60.0f;
 d->mGrabMovie = false;

 d->mScriptConnector = new BosonBigDisplayScriptConnector(this);

 d->mSelectionGroups.setAutoDelete(true);
 for (int i = 0; i < 10; i++) {
	BoSelection* s = new BoSelection(this);
	d->mSelectionGroups.insert(i, s);
 }
 mSelection = new BoSelection(this);
 connect(mSelection, SIGNAL(signalSelectionChanged(BoSelection*)),
		this, SIGNAL(signalSelectionChanged(BoSelection*)));
 d->mToolTips = new BoGLToolTip(this);
 d->mCanvasRenderer = new BosonCanvasRenderer(d->mModelviewMatrix, d->mViewFrustum, d->mViewport);
 d->mGLMiniMap = new BosonGLMiniMap(this);
 connect(boGame, SIGNAL(signalCellChanged(int,int)),
		d->mGLMiniMap, SLOT(slotUpdateCell(int,int)));

 for (int i = 0; i < 4; i++) {
	d->mViewport[i] = 0;
 }
 for (int i = 0; i < 6; i++) {
	for (int j = 0; j < 4; j++) {
		d->mViewFrustum[i * 4 + j] = 0.0;
	}
 }
 BoGroundRendererManager::manager()->setMatrices(&d->mModelviewMatrix, &d->mProjectionMatrix, d->mViewport);
 BoGroundRendererManager::manager()->setViewFrustum(d->mViewFrustum);

 boWaterManager->setViewFrustum(d->mViewFrustum);

 BoMeshRendererManager::manager()->makeRendererCurrent(QString::null);
 BoGroundRendererManager::manager()->makeRendererCurrent(QString::null);

 setUpdatesEnabled(false);

 setFocusPolicy(WheelFocus);

 if (!isValid()) {
	boError() << k_funcinfo << "No OpenGL support present on this system??" << endl;
	return;
 }

 connect(&d->mUpdateTimer, SIGNAL(timeout()), this, SLOT(slotUpdateGL()));

 connect(&d->mCursorEdgeTimer, SIGNAL(timeout()),
		this, SLOT(slotCursorEdgeTimeout()));

 connect(BosonPathVisualization::pathVisualization(),
		SIGNAL(signalAddLineVisualization( const QValueList<BoVector3Fixed>&, const BoVector4Float&, bofixed, int, bofixed)),
		this,
		SLOT(slotAddLineVisualization(const QValueList<BoVector3Fixed>&, const BoVector4Float&, bofixed, int, bofixed)));

 setUpdateInterval(boConfig->updateInterval());

 qApp->setGlobalMouseTracking(true);
 qApp->installEventFilter(this);
}

void BosonBigDisplayBase::setCursor(BosonCursor* cursor)
{
 delete mCursor;
 mCursor = cursor;
}

void BosonBigDisplayBase::setLocalPlayerScript(BosonScript* script)
{
 // AB: there is no need to save the pointer atm.
 // we just need to do these connects.
 d->mScriptConnector->connectToScript(script);
}

void BosonBigDisplayBase::setCanvas(BosonCanvas* canvas)
{
 BosonCanvas* previousCanvas = mCanvas;
 if (mCanvas) {
	disconnect(previousCanvas, 0, this, 0);
	disconnect(previousCanvas, 0, mSelection, 0);
 }
 mCanvas = canvas;
 if (!mCanvas) {
	return;
 }

 QIntDictIterator<BoSelection> selectIt(d->mSelectionGroups);
 for (; selectIt.current(); ++selectIt) {
	connect(mCanvas, SIGNAL(signalRemovedItem(BosonItem*)),
			selectIt.current(), SLOT(slotRemoveItem(BosonItem*)));
 }
 connect(mCanvas, SIGNAL(signalUnitRemoved(Unit*)),
		this, SLOT(slotUnitRemoved(Unit*)));
 connect(mCanvas, SIGNAL(signalRemovedItem(BosonItem*)),
		this, SLOT(slotRemovedItemFromCanvas(BosonItem*)));
 connect(mCanvas, SIGNAL(signalRemovedItem(BosonItem*)),
		mSelection, SLOT(slotRemoveItem(BosonItem*)));
 if (d->mGLMiniMap) {
	if (previousCanvas) {
		disconnect(previousCanvas, 0, d->mGLMiniMap, 0);
		disconnect(d->mGLMiniMap, 0, previousCanvas, 0);
	}
	disconnect(d->mGLMiniMap, 0, this, 0);
	disconnect(d->mGLMiniMap, 0, displayInput(), 0);
	connect(mCanvas, SIGNAL(signalUnitMoved(Unit*, bofixed, bofixed)),
		d->mGLMiniMap, SLOT(slotUnitMoved(Unit*, bofixed, bofixed)));
	connect(mCanvas, SIGNAL(signalUnitRemoved(Unit*)),
		d->mGLMiniMap, SLOT(slotUnitDestroyed(Unit*)));

	connect(d->mGLMiniMap, SIGNAL(signalReCenterView(const QPoint&)),
			this, SLOT(slotReCenterDisplay(const QPoint&)));
	connect(d->mGLMiniMap, SIGNAL(signalMoveSelection(int, int)),
			displayInput(), SLOT(slotMoveSelection(int, int)));
 }

 d->mCamera.setCanvas(mCanvas);

 slotResetViewProperties();

 BO_CHECK_NULL_RET(mCanvas->map());
 boDebug() << k_funcinfo << endl;
 BosonMap* map = mCanvas->map();
 initializeGL();
 if (d->mGLMiniMap) {
	d->mGLMiniMap->createMap(map, d->mViewport);
 } else {
	BO_NULL_ERROR(d->mGLMiniMap);
 }

 if (!boGame->gameMode()) { // AB: is this valid at this point?
	d->mUfoGameWidget->setGroundTheme(map->groundTheme());
 }
}

void BosonBigDisplayBase::initializeGL()
{
 if (isInitialized()) {
	// already called initializeGL()
	return;
 }

 // AB: WARNING you must _not_ assume this gets called once only!
 // this can get called once per context! i.e. when frames for the movie are
 // generated, then this will get called as well!
 // (keep this in mind when allocating memory)

 if (!context()) {
	boError() << k_funcinfo << "NULL context" << endl;
	return;
 }
 static bool recursive = false;
 if (recursive) {
	// this can happen e.g. when a paint event occurs while we are in this
	// function (e.g. because of a messagebox)
	return;
 }
 recursive = true;
 // AB: we need at least GLU 1.3 for gluCheckExtension() !
 // TODO: find out if we might be able to run boson with older versions - if yes
 // use the code from http://www.mesa3d.org/brianp/sig97/exten.htm#Compile to
 // check for extensions
 // TODO: check for extensions on configure/compile time, too!
 /*
 const* string = (char*)glGetString(GL_EXTENSIONS);
 if (!gluCheckExtension("GL_EXT_texture_object", string)) { // introduced in OpenGL 1.1 as standard feature
	KMessageBox::sorry(this, i18n("Your OpenGL implementation seems not to support texture objects - don't even try to run boson without them!"));
	kapp->exit(1);
	return;
 }
 */
 glClearColor(0.0, 0.0, 0.0, 0.0);

 glDisable(GL_DITHER); // we don't need this (and its enabled by default)

 // for anti-aliased lines (currently unused):
 glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
 // for anti-aliased points (currently unused):
 glHint(GL_POINT_SMOOTH_HINT, GL_FASTEST);
 // for anti-aliased polygons (currently unused):
 glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);

 glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);

#if 0

#warning FIXME: is extenstion there?
 // AB: if everything is inside the view volume we can use this to skip the
 // volume clipping tests. should be faster
 // FIXME: do we actually render stuff thats inside the view volume only?
 glHint(GL_CLIP_VOLUME_CLIPPING_HINT_EXT, GL_FASTEST);

#endif

 // AB: GL_MODULATE is the default and the models don't use it. unortunately the
 // light code seems to have problems with GL_REPLACE. GL_REPLACE would be
 // faster
// glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);


 BoVector4Float lightDif(0.644f, 0.644f, 0.644f, 1.0f);
 BoVector4Float lightAmb(0.502f, 0.502f, 0.502f, 1.0f);
 BoVector3Float lightPos(1545.0, 4755.0, 2600.0);

 BoLight* l = newLight();
 // This is the "main" light, i.e. the Sun. It should always have id 0
 if (l->id() != 0) {
	boWarning() << k_funcinfo << "Main light has id " << l->id() << endl;
 }
 l->setAmbient(lightAmb);
 l->setDiffuse(lightDif);
 l->setSpecular(lightDif);
 l->setDirectional(true);
 l->setPosition3(lightPos);

 l->setEnabled(true);

 boWaterManager->setSun(l);

 d->mCanvasRenderer->initGL();

 if (checkError()) {
	boError() << k_funcinfo << endl;
 }

 struct timeval time;
 gettimeofday(&time, 0);
 d->mFpsTime = time.tv_sec * 1000000 + time.tv_usec;

 // this needs to be done in initializeGL():
 BoFontInfo font;
 font.fromString(boConfig->stringValue("GLFont"));
 setFont(font);

 updateOpenGLSettings();

 if (!context()->deviceIsPixmap()) {
	if (!directRendering()) {
		// baad.
		boWarning() << k_funcinfo << "direct rendering has NOT been enabled!" << endl;
		KMessageBox::information(this, i18n("Direct rendering is NOT enabled - boson will run very slow. You should ensure that direct rendering is enabled!"));
	}

	boDebug() << k_funcinfo << "starting timer" << endl;
	// start rendering (will also start the timer if necessary)
	QTimer::singleShot(d->mUpdateInterval, this, SLOT(slotUpdateGL()));

	// update system information (we initializeGL() must have been called before
	// this makes sense)
	BoInfo::boInfo()->update(this);
 }

 boTextureManager->initOpenGL();
 boWaterManager->initOpenGL();

 boConfig->setTextureFOW(boTextureManager->textureUnits() > 1);

 connect(kapp->eventLoop(), SIGNAL(signalUpdateGL()), this, SLOT(slotUpdateGL()));

 initUfoGUI();

 recursive = false;
}

void BosonBigDisplayBase::initUfoGUI()
{
 glPushAttrib(GL_ALL_ATTRIB_BITS);

 initUfo();

 // AB: note that BoUfo widgets differ from usual Qt widgets API-wise.
 // You need to create them without a parent and then add them to their parent
 // widget using parent->addWidget(child). This also adds child to the layout of
 // parent.
 // WARNING: ALL widget that are created MUST be added to another widget!
 // Otherwise the created widget won't be deleted!

 // the contentWidget is the ufo-widget covering the whole Qt-widget. I.e. it
 // is the complete OpenGL area.
 BoUfoWidget* contentWidget = ufoManager()->contentWidget();

 contentWidget->setLayoutClass(BoUfoWidget::UVBoxLayout);

 d->mUfoGameWidget = new BosonUfoGameWidget(d->mModelviewMatrix, d->mProjectionMatrix, d->mViewFrustum, d->mViewport, this);
 d->mUfoGameWidget->setGLMiniMap(d->mGLMiniMap);
 contentWidget->addWidget(d->mUfoGameWidget);

 // TODO: tooltips ?

 glPopAttrib();
}

void BosonBigDisplayBase::initUfoActions(bool gameMode)
{
 BoUfoActionCollection* actionCollection = ufoManager()->actionCollection();
 BO_CHECK_NULL_RET(actionCollection);

 // AB: note that we have all actions from TopWidget, BosonWidgetBase,
 // BosonWidget and EditorWidget here.
 // in game mode all editor actions should be hidden and vice versa, as the
 // ui.rc files don't list them
 //
 //

 // TODO: help menu


 QSignalMapper* scrollMapper = new QSignalMapper(this);
 connect(scrollMapper, SIGNAL(mapped(int)), this, SLOT(slotScroll(int)));
 BoUfoAction* a;
 KShortcut scrollUp(Qt::Key_Up);
 scrollUp.append(KKeySequence(KKey(Qt::Key_W)));
 a = new BoUfoAction(i18n("Scroll Up"), scrollUp, scrollMapper,
		SLOT(map()), actionCollection,
		"scroll_up");
 scrollMapper->setMapping(a, ScrollUp);
 KShortcut scrollDown(Qt::Key_Down);
 scrollDown.append(KKeySequence(KKey(Qt::Key_S)));
 a = new BoUfoAction(i18n("Scroll Down"), scrollDown, scrollMapper,
		SLOT(map()), actionCollection,
		"scroll_down");
 scrollMapper->setMapping(a, ScrollDown);
 KShortcut scrollLeft(Qt::Key_Left);
 scrollLeft.append(KKeySequence(KKey(Qt::Key_A)));
 a = new BoUfoAction(i18n("Scroll Left"), scrollLeft, scrollMapper,
		SLOT(map()), actionCollection,
		"scroll_left");
 scrollMapper->setMapping(a, ScrollLeft);
 KShortcut scrollRight(Qt::Key_Right);
 scrollRight.append(KKeySequence(KKey(Qt::Key_D)));
 a = new BoUfoAction(i18n("Scroll Right"), scrollRight, scrollMapper,
		SLOT(map()), actionCollection,
		"scroll_right");
 scrollMapper->setMapping(a, ScrollRight);
 KShortcut rotateLeft(Qt::Key_Q);
 a = new BoUfoAction(i18n("Rotate Left"), rotateLeft, this,
		SLOT(slotRotateLeft()), actionCollection,
		"rotate_left");
 KShortcut rotateRight(Qt::Key_E);
 a = new BoUfoAction(i18n("Rotate Right"), rotateRight, this,
		SLOT(slotRotateRight()), actionCollection,
		"rotate_right");
 KShortcut zoomIn(Qt::Key_F);
 a = new BoUfoAction(i18n("Zoom In"), zoomIn, this,
		SLOT(slotZoomIn()), actionCollection,
		"zoom_in");
 KShortcut zoomOut(Qt::Key_V);
 a = new BoUfoAction(i18n("Zoom out"), zoomOut, this,
		SLOT(slotZoomOut()), actionCollection,
		"zoom_out");



 // Settings
// (void)BoUfoStdAction::keyBindings(this, SLOT(slotConfigureKeys()), actionCollection);
// d->mActionMenubar = BoUfoStdAction::showMenubar(this, SLOT(slotToggleMenubar()), actionCollection);
 d->mActionStatusbar = BoUfoStdAction::showStatusbar(this, SLOT(slotToggleStatusbar()), actionCollection);
 BoUfoToggleAction* sound = new BoUfoToggleAction(i18n("Soun&d"),
		KShortcut(), this, SLOT(slotToggleSound()),
		actionCollection, "options_sound");
 sound->setChecked(boConfig->sound());
 BoUfoToggleAction* music = new BoUfoToggleAction(i18n("M&usic"), KShortcut(),
		this, SLOT(slotToggleMusic()),
		actionCollection, "options_music");
 music->setChecked(boConfig->music());
 (void)new BoUfoAction(i18n("Maximal entries per event..."), KShortcut(), this,
		SLOT(slotChangeMaxProfilingEventEntries()), actionCollection, "options_profiling_max_event_entries");
 (void)new BoUfoAction(i18n("Maximal advance call entries..."), KShortcut(), this,
		SLOT(slotChangeMaxProfilingAdvanceEntries()), actionCollection, "options_profiling_max_advance_entries");
 (void)new BoUfoAction(i18n("Maximal rendering entries..."), KShortcut(), this,
		SLOT(slotChangeMaxProfilingRenderingEntries()), actionCollection, "options_profiling_max_rendering_entries");


 // Display
 d->mActionFullScreen = BoUfoStdAction::fullScreen(this, SLOT(slotToggleFullScreen()), actionCollection);
 d->mActionFullScreen->setChecked(false);

 // Debug
 (void)new BoUfoAction(i18n("&Profiling..."), KShortcut(), this,
		SLOT(slotProfiling()), actionCollection, "debug_profiling");
 (void)new BoUfoAction(i18n("&Debug KGame..."), KShortcut(), this,
		SLOT(slotDebugKGame()), actionCollection, "debug_kgame");
 (void)new BoUfoAction(i18n("Debug &BoDebug log..."), KShortcut(), this,
		SLOT(slotBoDebugLogDialog()), actionCollection, "debug_bodebuglog");
 (void)new BoUfoAction(i18n("sleep() 1s"), KShortcut(), this,
		SLOT(slotSleep1s()), actionCollection, "debug_sleep_1s");

 (void)new BoUfoAction(i18n("&Reset View Properties"), KShortcut(Qt::Key_R),
		this, SLOT(slotResetViewProperties()), actionCollection, "game_reset_view_properties");


 d->mActionChat = new BoUfoToggleAction(i18n("Show Cha&t"),
		KShortcut(Qt::CTRL+Qt::Key_C), this, SIGNAL(signalToggleChatVisible()),
		actionCollection, "options_show_chat");
 d->mActionChat->setChecked(d->mUfoGameWidget->isChatVisible());

 (void)new BoUfoAction(i18n("&Grab Screenshot"), KShortcut(Qt::CTRL + Qt::Key_G),
		this, SLOT(slotGrabScreenshot()), actionCollection, "game_grab_screenshot");
 (void)new BoUfoAction(i18n("Grab &Profiling data"), KShortcut(Qt::CTRL + Qt::Key_P),
		this, SLOT(slotGrabProfiling()), actionCollection, "game_grab_profiling");
 BoUfoToggleAction* movie = new BoUfoToggleAction(i18n("Grab &Movie"),
		KShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_M), 0, 0, actionCollection, "game_grab_movie");
 movie->setChecked(false);
 connect(movie, SIGNAL(signalToggled(bool)),
		this, SLOT(slotSetGrabMovie(bool)));

 BoUfoToggleAction* resources = new BoUfoToggleAction(i18n("Show resources"),
		KShortcut(), 0, 0, actionCollection, "show_resources");
 resources->setChecked(true);
 connect(resources, SIGNAL(signalToggled(bool)),
		this, SLOT(slotSetShowResources(bool)));
 BoUfoToggleAction* mapCoordinates = new BoUfoToggleAction(i18n("Debug &map coordinates"),
		KShortcut(), 0, 0, actionCollection, "debug_map_coordinates");
 mapCoordinates->setChecked(false);
 connect(mapCoordinates, SIGNAL(signalToggled(bool)),
		this, SLOT(slotSetDebugMapCoordinates(bool)));
 BoUfoToggleAction* PFData = new BoUfoToggleAction(i18n("Debug pathfinder data"),
		KShortcut(), 0, 0, actionCollection, "debug_pf_data");
 PFData->setChecked(false);
 connect(PFData, SIGNAL(signalToggled(bool)),
		this, SLOT(slotSetDebugPFData(bool)));
 BoUfoToggleAction* cellGrid = new BoUfoToggleAction(i18n("Show Cell &Grid"),
		KShortcut(), 0, 0, actionCollection, "debug_cell_grid");
 cellGrid->setChecked(false);
 connect(cellGrid, SIGNAL(signalToggled(bool)),
		this, SLOT(slotSetDebugShowCellGrid(bool)));
 BoUfoToggleAction* matrices = new BoUfoToggleAction(i18n("Debug Ma&trices"),
		KShortcut(), 0, 0, actionCollection, "debug_matrices");
 matrices->setChecked(false);
 connect(matrices, SIGNAL(signalToggled(bool)),
		this, SLOT(slotSetDebugMatrices(bool)));
 BoUfoToggleAction* works = new BoUfoToggleAction(i18n("Debug Item works"),
		KShortcut(), 0, 0, actionCollection, "debug_works");
 works->setChecked(false);
 connect(works, SIGNAL(signalToggled(bool)),
		this, SLOT(slotSetDebugItemWorks(bool)));
 BoUfoToggleAction* camera = new BoUfoToggleAction(i18n("Debug camera"),
		KShortcut(), 0, 0, actionCollection, "debug_camera");
 camera->setChecked(false);
 connect(camera, SIGNAL(signalToggled(bool)),
		this, SLOT(slotSetDebugCamera(bool)));
 BoUfoToggleAction* renderCounts = new BoUfoToggleAction(i18n("Debug Rendering counts"),
		KShortcut(), 0, 0, actionCollection, "debug_rendercounts");
 renderCounts->setChecked(false);
 connect(renderCounts, SIGNAL(signalToggled(bool)),
		this, SLOT(slotSetDebugRenderCounts(bool)));
 BoUfoToggleAction* cheating = new BoUfoToggleAction(i18n("Enable &Cheating"),
		KShortcut(), 0, 0, actionCollection, "debug_enable_cheating");
 connect(cheating, SIGNAL(signalToggled(bool)), this, SLOT(slotToggleCheating(bool)));
 BoUfoToggleAction* wireFrames = new BoUfoToggleAction(i18n("Render &Wireframes"),
		KShortcut(), 0, 0, actionCollection, "debug_wireframes");
 connect(wireFrames, SIGNAL(signalToggled(bool)), this, SLOT(slotSetDebugWireFrames(bool)));
 wireFrames->setChecked(false);
 slotSetDebugWireFrames(false);
 BoUfoToggleAction* boundingboxes = new BoUfoToggleAction(i18n("Render item's bounding boxes"),
		KShortcut(), 0, 0, actionCollection, "debug_boundingboxes");
 boundingboxes->setChecked(false);
 connect(boundingboxes, SIGNAL(signalToggled(bool)),
		this, SLOT(slotSetDebugBoundingBoxes(bool)));
 BoUfoToggleAction* fps = new BoUfoToggleAction(i18n("Debug FPS"),
		KShortcut(), 0, 0, actionCollection, "debug_fps");
 fps->setChecked(false);
 connect(fps, SIGNAL(signalToggled(bool)),
		this, SLOT(slotSetDebugFPS(bool)));
 BoUfoToggleAction* debugAdvanceCalls = new BoUfoToggleAction(i18n("Debug &Advance calls"),
		KShortcut(), 0, 0, actionCollection, "debug_advance_calls");
 connect(debugAdvanceCalls, SIGNAL(signalToggled(bool)), this, SLOT(slotSetDebugAdvanceCalls(bool)));
 BoUfoToggleAction* debugTextureMemory = new BoUfoToggleAction(i18n("Debug &Texture Memory"),
		KShortcut(), 0, 0, actionCollection, "debug_texture_memory");
 connect(debugTextureMemory, SIGNAL(signalToggled(bool)), this, SLOT(slotSetDebugTextureMemory(bool)));
 (void)new BoUfoAction(i18n("&Unfog"), KShortcut(), this,
		SLOT(slotUnfogAll()), actionCollection, "debug_unfog");
 (void)new BoUfoAction(i18n("Dump game &log"), KShortcut(), this,
		SLOT(slotDumpGameLog()), actionCollection, "debug_gamelog");
 BoUfoToggleAction* enableColorMap = new BoUfoToggleAction(i18n("Enable colormap"),
		KShortcut(), 0, 0, actionCollection, "debug_colormap_enable");
 enableColorMap->setChecked(false);
 connect(enableColorMap, SIGNAL(signalToggled(bool)),
		this, SLOT(slotSetEnableColorMap(bool)));
 (void)new BoUfoAction(i18n("Edit global conditions..."), KShortcut(), this,
		SLOT(slotEditConditions()), actionCollection,
		"debug_edit_conditions");


 BoUfoSelectAction* debugMode = new BoUfoSelectAction(i18n("Mode"), 0, 0,
		actionCollection, "debug_mode");
 connect(debugMode, SIGNAL(signalActivated(int)), this, SLOT(slotSetDebugMode(int)));
 QStringList l;
 l.append(i18n("Normal"));
 l.append(i18n("Debug Selection"));
 debugMode->setItems(l);
 debugMode->setCurrentItem(0);

 (void)new BoUfoAction(i18n("Show OpenGL states"), KShortcut(), this,
		SLOT(slotShowGLStates()), actionCollection,
		"debug_show_opengl_states");
 (void)new BoUfoAction(i18n("Reload &meshrenderer plugin"), KShortcut(), this,
		SLOT(slotReloadMeshRenderer()), actionCollection,
		"debug_lazy_reload_meshrenderer");
 (void)new BoUfoAction(i18n("Reload &groundrenderer plugin"), KShortcut(), this,
		SLOT(slotReloadGroundRenderer()), actionCollection,
		"debug_lazy_reload_groundrenderer");
 (void)new BoUfoAction(i18n("Light0..."), KShortcut(), this,
		SLOT(slotShowLight0Widget()), actionCollection,
		"debug_light0");
 (void)new BoUfoAction(i18n("Crash boson"), KShortcut(), this,
		SLOT(slotCrashBoson()), actionCollection,
		"debug_crash_boson");
#ifdef BOSON_USE_BOMEMORY
 (void)new BoUfoAction(i18n("Debug M&emory"), KShortcut(), this,
		SLOT(slotDebugMemory()), actionCollection,
		"debug_memory");
#endif
 createDebugPlayersMenu();


 QStringList files;
 files.append(boData->locateDataFile("boson/topui.rc"));
 files.append(boData->locateDataFile("boson/bosonbaseui.rc"));
 if (gameMode) {
	initUfoGameActions();
	files.append(boData->locateDataFile("boson/bosonui.rc"));
 } else {
	initUfoEditorActions();
	files.append(boData->locateDataFile("boson/editorui.rc"));
 }

 cheating->setChecked(DEFAULT_CHEAT_MODE);
 slotToggleCheating(DEFAULT_CHEAT_MODE);
 d->mActionStatusbar->setChecked(true);
 slotToggleStatusbar();

 actionCollection->createGUI(files);
}

void BosonBigDisplayBase::initUfoGameActions()
{
 BoUfoActionCollection* actionCollection = ufoManager()->actionCollection();
 BO_CHECK_NULL_RET(actionCollection);

 (void)BoUfoStdAction::gameQuit(this, SIGNAL(signalQuit()), actionCollection);
 (void)BoUfoStdAction::gameEnd(this, SIGNAL(signalEndGame()), actionCollection);
 (void)BoUfoStdAction::gameSave(this, SIGNAL(signalSaveGame()), actionCollection);
 (void)BoUfoStdAction::gamePause(boGame, SLOT(slotTogglePause()), actionCollection);
 (void)BoUfoStdAction::preferences(this, SLOT(slotPreferences()), actionCollection);
 (void)new BoUfoAction(i18n("Center &Home Base"),
		KShortcut(Qt::Key_H),
		this, SLOT(slotCenterHomeBase()),
		actionCollection, "game_center_base");
 (void)new BoUfoAction(i18n("Sync Network"),
		KShortcut(),
		this, SLOT(slotSyncNetwork()),
		actionCollection, "debug_sync_network");


 delete d->mSelectMapper;
 d->mSelectMapper = new QSignalMapper(this);
 delete d->mCreateMapper;
 d->mCreateMapper = new QSignalMapper(this);
 connect(d->mSelectMapper, SIGNAL(mapped(int)),
		this, SLOT(slotSelectSelectionGroup(int)));
 connect(d->mCreateMapper, SIGNAL(mapped(int)),
		this, SLOT(slotCreateSelectionGroup(int)));

 for (int i = 0; i < 10; i++) {
	BoUfoAction* a = new BoUfoAction(i18n("Select Group %1").arg(i == 0 ? 10 : i),
			Qt::Key_0 + i, d->mSelectMapper,
			SLOT(map()), actionCollection,
			QString("select_group_%1").arg(i));
	d->mSelectMapper->setMapping(a, i);
	a = new BoUfoAction(i18n("Create Group %1").arg(i == 0 ? 10 : i),
			Qt::CTRL + Qt::Key_0 + i, d->mCreateMapper,
			SLOT(map()), actionCollection,
			QString("create_group_%1").arg(i));
	d->mCreateMapper->setMapping(a, i);
 }

}

void BosonBigDisplayBase::initUfoEditorActions()
{
 BoUfoActionCollection* actionCollection = ufoManager()->actionCollection();
 BO_CHECK_NULL_RET(actionCollection);


 BoUfoStdAction::fileSaveAs(this, SLOT(slotEditorSavePlayFieldAs()), actionCollection, "file_save_playfield_as");
 BoUfoAction* close = BoUfoStdAction::fileClose(this, SIGNAL(signalEndGame()), actionCollection);

 // TODO
// close->setText(i18n("&End Editor"));
 BoUfoStdAction::fileQuit(this, SIGNAL(signalQuit()), actionCollection);
 (void)BoUfoStdAction::preferences(this, SLOT(slotPreferences()), actionCollection);

 d->mActionEditorPlayer = new BoUfoSelectAction(i18n("&Player"), 0, 0, actionCollection, "editor_player");
 connect(d->mActionEditorPlayer, SIGNAL(signalActivated(int)),
		this, SLOT(slotEditorChangeLocalPlayer(int)));

 QStringList list;
 list.append(i18n("&Facilities"));
 list.append(i18n("&Mobiles"));
 list.append(i18n("&Ground"));
 d->mActionEditorPlace = new BoUfoSelectAction(i18n("Place"), 0, 0, actionCollection, "editor_place");
 connect(d->mActionEditorPlace, SIGNAL(signalActivated(int)),
		this, SLOT(slotEditorPlace(int)));
 d->mActionEditorPlace->setItems(list);

 KShortcut s;
 s.append(KKeySequence(QKeySequence(Qt::Key_Delete)));
 s.append(KKeySequence(QKeySequence(Qt::Key_D)));
 (void)new BoUfoAction(i18n("Delete selected unit"), KShortcut(s), this,
		SLOT(slotEditorDeleteSelectedUnits()), actionCollection,
		"editor_delete_selected_unit");

 (void)new BoUfoAction(i18n("Map &description"), KShortcut(), this,
		SLOT(slotEditorEditMapDescription()), actionCollection,
		"editor_map_description");
 (void)new BoUfoAction(i18n("Edit &Minerals"), KShortcut(), this,
		SIGNAL(slotEditorEditPlayerMinerals()), actionCollection,
		"editor_player_minerals");
 (void)new BoUfoAction(i18n("Edit &Oil"), KShortcut(), this,
		SIGNAL(slotEditorEditPlayerOil()), actionCollection,
		"editor_player_oil");
 d->mActionEditorChangeHeight = new BoUfoToggleAction(i18n("Edit &Height"),
		KShortcut(), this, 0, actionCollection, "editor_height");
 connect(d->mActionEditorChangeHeight, SIGNAL(signalToggled(bool)),
		this, SLOT(slotEditorEditHeight(bool)));
 (void)new BoUfoAction(i18n("&Import height map"), KShortcut(), this,
		SLOT(slotEditorImportHeightMap()), actionCollection,
		"editor_import_heightmap");
 (void)new BoUfoAction(i18n("&Export height map"), KShortcut(), this,
		SLOT(slotEditorExportHeightMap()), actionCollection,
		"editor_export_heightmap");
 (void)new BoUfoAction(i18n("I&mport texmap"), KShortcut(), this,
		SLOT(slotEditorImportTexMap()), actionCollection,
		"editor_import_texmap");
 (void)new BoUfoAction(i18n("E&xport texmap"), KShortcut(), this,
		SLOT(slotEditorExportTexMap()), actionCollection,
		"editor_export_texmap");
 (void)new BoUfoAction(i18n("Edit global conditions"), KShortcut(), this,
		SLOT(slotEditConditions()), actionCollection,
		"editor_edit_conditions");

// KStdAction::preferences(bosonWidget(), SLOT(slotGamePreferences()), actionCollection); // FIXME: slotEditorPreferences()

 createEditorPlayerMenu();
 d->mActionEditorPlace->setCurrentItem(0);
 slotEditorPlace(0);
}

void BosonBigDisplayBase::createEditorPlayerMenu()
{
 BO_CHECK_NULL_RET(boGame);
 BO_CHECK_NULL_RET(d->mActionEditorPlayer);
 QPtrList<KPlayer> players = *boGame->playerList();
 QPtrListIterator<KPlayer> it(players);
 QStringList items;
 int current = -1;
 while (it.current()) {
	d->mEditorPlayers.append(it.current());
	items.append(it.current()->name());
	if (localPlayerIO()) {
		if (it.current() == (KPlayer*)localPlayerIO()->player()) {
			current = items.count() - 1;
		}
	}
	++it;
 }
 d->mActionEditorPlayer->setItems(items);

 if (current >= 0) {
	d->mActionEditorPlayer->setCurrentItem(current);

	// AB: this causes a recursion
//	slotEditorChangeLocalPlayer(current);
 }
}

void BosonBigDisplayBase::resizeGL(int w, int h)
{
 boDebug() << k_funcinfo << w << " " << h << endl;
 BosonUfoGLWidget::resizeGL(w, h);
 if (ufoManager()) {
	ufoManager()->sendResizeEvent(w, h);
 }
 setViewport(0, 0, (GLsizei)w, (GLsizei)h);
 glMatrixMode(GL_PROJECTION);
 glLoadIdentity();

 // IMO we don't need zooming. changing posY should be equally .. and even
 // better. no distortion
 GLfloat fovY = d->mFovY; // * d->mCamera.zoomFactor();
 d->mAspect = (float)w / (float)h;
 gluPerspective(fovY, d->mAspect, BO_GL_NEAR_PLANE, BO_GL_FAR_PLANE);

 // cache the composed projection matrix. we'll need it very often in
 // mapCoordinates()
 d->mProjectionMatrix.loadMatrix(GL_PROJECTION_MATRIX);
 extractFrustum(); // projection matrix changed
 glMatrixMode(GL_MODELVIEW);


 glClearDepth(1.0f);
 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#if 0
 // AB: this does not work dependable :(
 float depth = 1.0f;
 glReadPixels(0, 0, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
 if (fabsf(depth) > 0.001f && fabsf(depth - 1.0f) > 0.001f && isVisible()) {
	// i really cannot  imagine why this happens - but it does for me.
	// Radeon 9200 with ATI proprietary drivers, version 3.2.8
	boWarning() << k_funcinfo << "clearing depth buffer with 1.0 did caused depth buffer values of " << depth << ", but not 1.0! enabling workaround" << endl;
	Bo3dTools::enableReadDepthBufferWorkaround(depth);
 }
#endif


 if (canvas()) {
	// update the minimap
	camera()->setCameraChanged(true);
 }

 if (checkError()) {
	boError() << k_funcinfo << endl;
 }
}

void BosonBigDisplayBase::paintGL()
{
 if (!isInitialized()) {
	initGL();
	return;
 }
 if (!boGame) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderUfo();
	return;
 }
 if (boGame->gameStatus() == KGame::Init) {
	return;
 }
 BO_CHECK_NULL_RET(localPlayerIO());
 BO_CHECK_NULL_RET(displayInput());

 if (checkError()) {
	boError() << k_funcinfo << "OpenGL error at start of paintGL" << endl;
 }

 d->mFrameCount++;
 calcFPS();
 if (boGame->delayedMessageCount() >= 10 && d->mFrameCount != 0) {
	// d->mFrameCount is set to 0 every second. so if we have >= 10 delayed
	// messages we'll render one frame per second.
	// now we need to reset the framecount, so that we won't display several
	// hundred fps although there is only a single frame rendered :)
	// We incremented by one above, so we decrement here.
	if (d->mFrameCount > 1) {
		d->mFrameCount--;
	}
	return;
 }
 boProfiling->render(true);
 d->mUpdateTimer.stop();

 // If camera has been changed since last rendering, we need to reapply it
 if (camera()->isCameraChanged()) {
	cameraChanged();
 }

 boProfiling->renderClear(true);
 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 boProfiling->renderClear(false);

 boTextureManager->clearStatistics();

 glColor3ub(255, 255, 255);

 // note: we don't call BoGameCamera::applyCameraToScene() here because of performance. instead we just
 // push the matrix here and pop it at the end of paintGL() again.
 // applyCameraToScene() is called only whenever cameraChanged() is called.
 glPushMatrix();

 d->mCanvasRenderer->setCamera(camera());
 d->mCanvasRenderer->setLocalPlayerIO(localPlayerIO());

 d->mCanvasRenderer->paintGL(canvas());

 glEnable(GL_DEPTH_TEST);
 glEnable(GL_LIGHTING);
 glEnable(GL_NORMALIZE);
 // AB: I think this does not belong to the canvas renderer
 renderPlacementPreview();

 if (checkError()) {
	boError() << k_funcinfo << "preview rendered" << endl;
 }

 glDisable(GL_DEPTH_TEST);
 glDisable(GL_LIGHTING);
 glDisable(GL_NORMALIZE);


 // Render lineviz's
 boTextureManager->disableTexturing();
 QValueList<BoLineVisualization>::iterator it;
 for (it = d->mLineVisualizationList.begin(); it != d->mLineVisualizationList.end(); ++it) {
	glColor4fv((*it).color.data());
	glPointSize((*it).pointsize);
	glBegin(GL_LINE_STRIP);
	QValueList<BoVector3Fixed>::iterator pit;
	for (pit = (*it).points.begin(); pit != (*it).points.end(); ++pit) {
		glVertex3fv((*pit).toFloat().data());
	}
	glEnd();
 }

 glDisable(GL_FOG);

 boProfiling->renderUfo(true);
 renderUfo();
 boProfiling->renderUfo(false);

 boProfiling->renderText(true); // AB: actually this is more than just text

 glMatrixMode(GL_PROJECTION);
 glPushMatrix();
 glLoadIdentity();
 gluOrtho2D(0.0, (GLfloat)d->mViewport[2], 0.0, (GLfloat)d->mViewport[3]);
 glMatrixMode(GL_MODELVIEW);
 glPushMatrix();
 glLoadIdentity();

 glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

 renderCursor();

 boTextureManager->disableTexturing();
 renderSelectionRect();
 renderText();

 glMatrixMode(GL_PROJECTION);
 glPopMatrix();
 glMatrixMode(GL_MODELVIEW);
 glPopMatrix();
 boProfiling->renderText(false);

 glPopMatrix();

 bool showProfilingMessage = boProfiling->renderEntries() < MAX_PROFILING_ENTRIES;
 boProfiling->render(false);
 if (showProfilingMessage && boProfiling->renderEntries() >= MAX_PROFILING_ENTRIES) {
	boGame->slotAddChatSystemMessage(i18n("%1 frames have been recorded by boProfiling. You can make profiling snapshots using CTRL+P").arg(boProfiling->renderEntries()));
 }

 if (d->mUpdateInterval) {
	d->mUpdateTimer.start(d->mUpdateInterval);
 }
 if (checkError()) {
	boError() << k_funcinfo << "OpenGL error at end of paintGL" << endl;
 }
}

void BosonBigDisplayBase::renderPathLines(QValueList<QPoint>& path, bool isFlying, float _z)
{
 // render a line from the current position of the unit to the
 // point it is moving to.
 // TODO: render one vertex per cell or so. this would fix
 // problem with heightmaps, when a line goes through mountains.
 // speed is hardly relevant at this point (rendering a few small
 // lines is fast).
 boTextureManager->disableTexturing();
 glBegin(GL_LINE_STRIP);
 QValueList<QPoint>::Iterator it;
 bool done = false;
 for (it = path.begin(); it != path.end(); ++it) {
	if ((*it).x() < 0 || (*it).y() < 0) {
		done = true;
		break;
	}
	float x = ((float)(*it).x());
	float y = -((float)(*it).y());
	float z = 0.05f;
	if (isFlying) {
		z += _z;
	} else {
		z += canvas()->heightAtPoint(x, y);
	}
	glVertex3f(x, y, z);
 }
 glEnd();

}

void BosonBigDisplayBase::renderPlacementPreview()
{
 BO_CHECK_NULL_RET(displayInput());
 BO_CHECK_NULL_RET(canvas());
 BO_CHECK_NULL_RET(canvas()->map());
 if (!displayInput()->actionLocked()) {
	return;
 }
 if (displayInput()->actionType() != ActionPlacementPreview) {
	return;
 }
 if (!d->mPlacementPreview.hasPreview()) {
	return;
 }

 // AB: GL_MODULATE is currently default. if we every change it to
 // GL_REPLACE we should change it here:
// glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
 GLubyte color;
 if (d->mPlacementPreview.canPlace() || d->mShiftPressed) {
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
 if (d->mControlPressed && !boGame->gameMode()) {
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
	BoFrame* f = d->mPlacementPreview.model()->frame(0);
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

void BosonBigDisplayBase::renderUfo()
{
 if (ufoManager()) {
	boTextureManager->invalidateCache();
	glColor3ub(255, 255, 255);
	d->mUfoGameWidget->updateUfoLabels();
	ufoManager()->dispatchEvents();
	ufoManager()->render();
 }
}

void BosonBigDisplayBase::renderCursor()
{
 if (cursor()) {
	// FIXME: use cursorCanvasVector()
	QPoint pos = mapFromGlobal(QCursor::pos());
	GLfloat x = (GLfloat)pos.x();
	GLfloat y = (GLfloat)d->mViewport[3] - (GLfloat)pos.y();
	cursor()->renderCursor(x, y);
 }

 if (checkError()) {
	boError() << k_funcinfo << "OpenGL error" << endl;
 }
}

void BosonBigDisplayBase::renderSelectionRect()
{
 if (d->mSelectionRect.isVisible()) {
	glPushMatrix();

	glColor3ub(255, 0, 0); // FIXME hardcoded

	QRect rect;
	d->mSelectionRect.widgetRect(&rect);

	int x = rect.left();
	int w = rect.width();
	int y = d->mViewport[3] - rect.top();
	int h = rect.height();

	glBegin(GL_LINE_LOOP);
		glVertex3f(x, y, 0.0f);
		glVertex3f(x + w, y, 0.0f);
		glVertex3f(x + w, y - h, 0.0f);
		glVertex3f(x, y - h, 0.0f);
	glEnd();

	glColor3ub(255, 255, 255);
	glPopMatrix();
 }
}

void BosonBigDisplayBase::renderText()
{
 BO_CHECK_NULL_RET(d->mDefaultFont);
 BO_CHECK_NULL_RET(localPlayerIO());
 d->mDefaultFont->begin();

 // TODO: port to libufo and remove this code.
 if (d->mToolTips->showTip()) {
	QPoint pos = mapFromGlobal(QCursor::pos());
	d->mToolTips->renderToolTip(pos.x(), pos.y(), d->mViewport, d->mDefaultFont);
 }


 if (checkError()) {
	boError() << k_funcinfo << "OpenGL error" << endl;
 }
 boTextureManager->invalidateCache();
}

// one day we might support swapping LMB and RMB so let's use defines already to
// make that easier.
#define LEFT_BUTTON LeftButton
#define RIGHT_BUTTON RightButton

void BosonBigDisplayBase::slotMouseEvent(KGameIO* io, QDataStream& stream, QMouseEvent* e, bool *eatevent)
{
// AB: maybe we could move this function to the displayInput directly!
 BO_CHECK_NULL_RET(displayInput());
 GLfloat posX = 0.0;
 GLfloat posY = 0.0;
 GLfloat posZ = 0.0;
 if (!mapCoordinates(e->pos(), &posX, &posY, &posZ)) {
	boError() << k_funcinfo << "Cannot map coordinates" << endl;
	return;
 }
// boDebug() << posZ << endl;
 BoVector3Fixed canvasVector;
 worldToCanvas(posX, posY, posZ, &canvasVector);

 BoMouseEvent event;
 event.setCanvasVector(canvasVector);
 event.setWorldPos(posX, posY, posZ);
 if (e->type() != QEvent::Wheel) {
	event.setWidgetPos(e->pos());
	event.setControlButton(e->state() & ControlButton);
	event.setShiftButton(e->state() & ShiftButton);
	event.setAltButton(e->state() & AltButton);
 } else {
	QWheelEvent* w = (QWheelEvent*)e;
	event.setWidgetPos(w->pos());
	event.setControlButton(w->state() & ControlButton);
	event.setShiftButton(w->state() & ShiftButton);
	event.setAltButton(w->state() & AltButton);
 }

 // our actions are done on Button*Release*, not Press. That conflicts with
 // DblClick, so we store whether the last Press event was an actual press event
 // or a double click.
 static ButtonState isDoubleClick = NoButton;

 if (ufoManager()->sendEvent((QEvent*)e)) {
	return;
 }

 switch (e->type()) {
	case QEvent::Wheel:
	{
		QWheelEvent* wheel = (QWheelEvent*)e;
		bool send = false;
		float delta = -wheel->delta() / 120;//120: see QWheelEvent::delta()
		mouseEventWheel(delta, wheel->orientation(), event, stream, &send);
		if (send) {
			*eatevent = true;
		}
		wheel->accept();
		break;
	}
	case QEvent::MouseMove:
	{
		bool send = false;
		isDoubleClick = NoButton; // when the mouse was pressed twice but the second press is hold down and moved then it isn't a double click anymore.
		mouseEventMove(e->state(), event, stream, &send);
		if (send) {
			*eatevent = true;
		}
		e->accept();
		break;
	}
	case QEvent::MouseButtonDblClick:
	{
		isDoubleClick = e->button();
		// actual actions will happen on ButtonRelease!
		e->accept();
		break;
	}
	case QEvent::MouseButtonPress:
	{
		// no action should happen here!
		isDoubleClick = NoButton;
		if (e->button() == LEFT_BUTTON) {
			d->mSelectionRect.setStartWidgetPos(e->pos());
		} else if (e->button() == MidButton) {
			// nothing to be done here
		} else if (e->button() == RIGHT_BUTTON) {
			if (boConfig->rmbMove()) {
				//AB: this might be obsolete..
				d->mMouseMoveDiff.moveEvent(e->pos()); // set position, but do not yet start!
			}
		}
		e->accept();
		break;
	}
	case QEvent::MouseButtonRelease:
	{
		bool send = false;
		if (e->button() == isDoubleClick) {
			mouseEventReleaseDouble(e->button(), event, stream, &send);
		} else {
			mouseEventRelease(e->button(), event, stream, &send);
		}
		if (send) {
			*eatevent = true;
		}
		e->accept();
		break;
	}
	default:
		boWarning() << "unexpected mouse event " << e->type() << endl;
		e->ignore();
		return;
 }
}

void BosonBigDisplayBase::mouseEventWheel(float delta, Orientation orientation, const BoMouseEvent& boEvent, QDataStream&, bool*)
{
#warning FIXME: d->mCanvasPos/Vector are not fixed when zooming with wheel

 int action;
 if (boEvent.shiftButton()) {
	action = boConfig->mouseWheelShiftAction();
 } else {
	action = boConfig->mouseWheelAction();
 }
 switch (action) {
	case CameraMove:
	{
		int scrollX, scrollY;
		if (boEvent.controlButton()) {
			scrollX = width();
			scrollY = height();
		} else {
			scrollX = 20;
			scrollY = 20;
			delta *= QApplication::wheelScrollLines();
		}
		if (orientation == Horizontal) {
			scrollX *= (int)delta;
			scrollY = 0;
		} else {
			scrollX = 0;
			scrollY *= (int)delta;
		}
		scrollBy(scrollX, scrollY);
		break;
	}
	case CameraZoom:
		if (boEvent.controlButton()) {
			delta *= 3;
		} else {
			delta *= 1; // no effect, btw
		}
		zoom(delta);
		break;
	case CameraRotate:
		if (boEvent.controlButton()) {
			delta *= 30;
		} else {
			delta *= 10;
		}
		rotate(delta);
		break;
	default:
	{
		boWarning() << k_funcinfo << "invalid wheel action: " << action << endl;
		break;
	}
 }
}

void BosonBigDisplayBase::mouseEventMove(int buttonState, const BoMouseEvent& event, QDataStream&, bool*)
{
 float posX, posY, posZ;
 event.worldPos(&posX, &posY, &posZ);
 d->mMouseMoveDiff.moveEvent(event.widgetPos());
 if (event.altButton()) {
	// The Alt button is the camera modifier in boson.
	// Better don't do important stuff (like unit movement
	// or selections) here, since a single push on Alt gives
	// the focus to the menu which might be very confusing
	// during a game.
	if (buttonState & LEFT_BUTTON) {
		d->mMouseMoveDiff.start(LEFT_BUTTON);
		camera()->changeZ(d->mMouseMoveDiff.dy());
	} else if (buttonState & RIGHT_BUTTON) {
		d->mMouseMoveDiff.start(RIGHT_BUTTON);
		camera()->changeRotation(d->mMouseMoveDiff.dx());
		camera()->changeRadius(d->mMouseMoveDiff.dy());
	}
 } else if (buttonState & LEFT_BUTTON) {
	if (!displayInput()->actionLocked()) {
		// selection rect gets drawn.
		// other modifiers are ignored
		d->mSelectionRect.setVisible(true);
		moveSelectionRect(event.widgetPos());
	} else if (!boGame->gameMode() && displayInput()->actionType() != ActionChangeHeight) {
		// In editor mode, try to place unit/ground whenever mouse moves. This
		//  enables you to edit big areas easily. But it's not done for height
		//  changing (yet).
		displayInput()->actionClicked(event);
	}
 } else if (buttonState & RIGHT_BUTTON) {
	// RMB+MouseMove does *not* depend on CTRL or Shift. the
	// map is moved in all cases (currently - we have some
	// free buttons here :))
	if (boConfig->rmbMove()) {
		// problem is that QCursor::setPos() also causes
		// a mouse move event. we can use this hack in
		// order to check whether it is a real mouse
		// move event or we caused it here.
		// TODO: use d->mMouseMoveDiff.x()/y() in
		// paintGL() for the cursor, not QCursor::pos()
//		static bool a = false;
//		if (a) {
//			a = false;
//			break;
//		}
//		a = true;moveLookAtBy
//		QPoint pos = mapToGlobal(QPoint(d->mMouseMoveDiff.oldX(), d->mMouseMoveDiff.oldY()));
//		QCursor::setPos(pos);

		// modifiers are ignored.
		d->mMouseMoveDiff.start(RIGHT_BUTTON);
		GLfloat dx, dy;
		int moveX = d->mMouseMoveDiff.dx();
		int moveY = d->mMouseMoveDiff.dy();
		mapDistance(moveX, moveY, &dx, &dy);
		camera()->changeLookAt(BoVector3Float(dx, dy, 0));
	} else {
		d->mMouseMoveDiff.stop();
	}
 } else if (buttonState & MidButton) {
	// currently unused
 }
 QPoint widgetPos = mapFromGlobal(QCursor::pos());
 GLfloat x = 0.0, y = 0.0, z = 0.0;
 mapCoordinates(widgetPos, &x, &y, &z);
 worldToCanvas(x, y, z, &(d->mCanvasVector)); // AB: are these already real z coordinates?
 displayInput()->updatePlacementPreviewData();

 // AB: we might want to use a timer here instead - then we would also be able
 // to change the cursor type when units move under the cursor. i don't want to
 // call updateCursor() from BosonCanvas::slotAdvance() as it would get called
 // too often then
 displayInput()->updateCursor();
}

void BosonBigDisplayBase::mouseEventRelease(ButtonState button, const BoMouseEvent& event, QDataStream&, bool*)
{
 switch (button) {
	case LEFT_BUTTON:
	{
		if (displayInput()->actionLocked()) {
			// basically the same as a normal RMB
			displayInput()->actionClicked(event);
		} else if (event.shiftButton()) {
			BoItemList* items = selectionRectItems();
			displayInput()->unselectArea(items);
			d->mSelectionRect.setVisible(false);
		} else if (event.controlButton()) {
			removeSelectionRect(false);
		} else {
			// select the unit(s) below the cursor/inside the
			// selection rect
			removeSelectionRect(true);
		}
		break;
	}
	case MidButton:
	{
		// we ignore all modifiers here, currently.
		if (boConfig->mmbMove()) {
			float posX, posY, posZ;
			event.worldPos(&posX, &posY, &posZ);
			int cellX, cellY;
			cellX = (int)(posX);
			cellY = (int)(-posY);
			slotReCenterDisplay(QPoint(cellX, cellY));
			displayInput()->updateCursor();
		}
		break;
	}
	case RIGHT_BUTTON:
	{
		if (!d->mMouseMoveDiff.isStopped()) {
			d->mMouseMoveDiff.stop();
		} else if (displayInput()->actionLocked()) {
			displayInput()->unlockAction();
			displayInput()->updateCursor();
		} else {
			displayInput()->actionClicked(event);
		}
		break;
	}
	default:
		boError() << k_funcinfo << "invalid mouse button " << button << endl;
		break;
 }
}

void BosonBigDisplayBase::mouseEventReleaseDouble(ButtonState button, const BoMouseEvent& event, QDataStream& , bool* )
{
 switch (button) {
	case LEFT_BUTTON:
	{
		// we ignore UnitAction is locked here
		// currently!
		bool replace = !event.controlButton();
		bool onScreenOnly = !event.shiftButton();
		Unit* unit = localPlayerIO()->findUnitAt(canvas(), event.canvasVector());
		if (unit) {
			if (onScreenOnly) {
				boDebug() << "TODO: select only those that are currently on the screen!" << endl;
			}
			if (!displayInput()->selectAll(unit->unitProperties(), replace)) {
				displayInput()->selectSingle(unit, replace);
			}
		}
		break;
	}
	case MidButton:
	case RIGHT_BUTTON:
	{
		// we ignore all other (RMB, MMB) for now. we
		// might use this one day.
		break;
	}
	default:
		boError() << k_funcinfo << "invalid mouse button " << button << endl;
		break;
 }
}

void BosonBigDisplayBase::addMouseIO(PlayerIO* io)
{
 boDebug() << k_funcinfo << endl;
 BO_CHECK_NULL_RET(io);
 if (d->mMouseIO) {
	boError() << k_funcinfo << "mouse IO already present for this display!" << endl;
	return;
 }
 if (io->hasRtti(KGameIO::MouseIO)) {
	// FIXME: this is only invalid if the IO is for the same big display!
	boWarning() << k_funcinfo << "player already has a mouse IO" << endl;
	return;
 }
 d->mMouseIO = new KGameMouseIO(this, true);
 connect(d->mMouseIO, SIGNAL(signalMouseEvent(KGameIO*, QDataStream&, QMouseEvent*, bool*)),
		this, SLOT(slotMouseEvent(KGameIO*, QDataStream&, QMouseEvent*, bool*)));
 connect(d->mMouseIO, SIGNAL(destroyed()),
		this, SLOT(slotMouseIODestroyed()));
 io->addGameIO(d->mMouseIO);
}

void BosonBigDisplayBase::setLocalPlayerIO(PlayerIO* io)
{
 boDebug() << k_funcinfo << endl;
 resetGameMode();
 if (localPlayerIO() && io) {
	// note that we do this even if io == d->mLocalPlayerIO.
	// we do this to guarantee that _all_ objects are properly initialized
	// with the new player, even if they did not exist yet when the player was set
	// the first time
	boDebug() << k_funcinfo << "already a local playerIO present! unset..." << endl;
	setLocalPlayerIO(0);
 }

 PlayerIO* previousPlayerIO = localPlayerIO();
 d->mLocalPlayerIO = io;

 BoGroundRendererManager::manager()->setLocalPlayerIO(localPlayerIO());

 delete d->mMouseIO;
 d->mMouseIO = 0;

 boWaterManager->setLocalPlayerIO(localPlayerIO());

 if (d->mGLMiniMap) {
	if (previousPlayerIO) {
		previousPlayerIO->disconnect(0, d->mGLMiniMap, 0);
	}
	if (localPlayerIO()) {
		PlayerIO* io = localPlayerIO();
		io->connect(SIGNAL(signalFog(int, int)),
				d->mGLMiniMap, SLOT(slotFog(int, int)));
		io->connect(SIGNAL(signalUnfog(int, int)),
				d->mGLMiniMap, SLOT(slotUnfog(int, int)));
		io->connect(SIGNAL(signalFog(int, int)),
				this, SLOT(slotFog(int, int)));
		io->connect(SIGNAL(signalUnfog(int, int)),
				this, SLOT(slotUnfog(int, int)));
		if (boGame->gameMode()) {
			io->connect(SIGNAL(signalShowMiniMap(bool)),
					d->mGLMiniMap, SLOT(slotShowMiniMap(bool)));
			d->mGLMiniMap->slotShowMiniMap(io->hasMiniMap());
		} else {
			d->mGLMiniMap->slotShowMiniMap(true);
		}
	}
	d->mGLMiniMap->setLocalPlayerIO(localPlayerIO());
 }

 if (previousPlayerIO) {
	// AB: we should probably add such a signal to the IO and use the one in
	// the IO then!
	disconnect((KPlayer*)previousPlayerIO->player(), SIGNAL(signalUnitChanged(Unit*)), this, 0);
 }

 d->mUfoGameWidget->setLocalPlayerIO(localPlayerIO());

 if (!localPlayerIO()) {
	return;
 }

 // at this point the game mode is already fixed, so calling this here should be
 // ok
 setGameMode(boGame->gameMode());

 // AB: we should probably add such a signal to the IO and use the one in
 // the IO then!
 connect((KPlayer*)localPlayerIO()->player(), SIGNAL(signalUnitChanged(Unit*)),
		this, SLOT(slotUnitChanged(Unit*)));


 addMouseIO(localPlayerIO());

 BosonLocalPlayerInput* i = (BosonLocalPlayerInput*)localPlayerIO()->findRttiIO(BosonLocalPlayerInput::LocalPlayerInputRTTI);
 if (i) {
	if (i->eventListener()) {
		setLocalPlayerScript(i->eventListener()->script());
	}
 } else {
	boError() << k_funcinfo << "local player does not have any BosonLocalPlayerInput!" << endl;
 }

 if (canvas()) {
	slotInitMiniMapFogOfWar();
 }
}

PlayerIO* BosonBigDisplayBase::localPlayerIO() const
{
 return d->mLocalPlayerIO;
}

void BosonBigDisplayBase::slotCenterHomeBase()
{
 boDebug() << k_funcinfo << endl;
 BO_CHECK_NULL_RET(localPlayerIO());
 slotReCenterDisplay(localPlayerIO()->homeBase());
}

void BosonBigDisplayBase::slotResetViewProperties()
{
 BO_CHECK_NULL_RET(canvas());
 d->mFovY = 60.0;
 d->mAspect = 1.0;
 setCamera(BoGameCamera(canvas()));
 resizeGL(d->mViewport[2], d->mViewport[3]);
}

void BosonBigDisplayBase::slotReCenterDisplay(const QPoint& pos)
{
//TODO don't center the corners - e.g. 0;0 should be top left, never center
 camera()->setLookAt(BoVector3Float(((float)pos.x()), -((float)pos.y()), 0));
}

void BosonBigDisplayBase::worldToCanvas(GLfloat x, GLfloat y, GLfloat /*z*/, QPoint* pos) const
{
 pos->setX((int)(x));
 pos->setY((int)(-y));
 // AB: z remains as-is
}

void BosonBigDisplayBase::worldToCanvas(GLfloat x, GLfloat y, GLfloat z, BoVector3Fixed* pos) const
{
 pos->set(x, -y, z);
}

void BosonBigDisplayBase::canvasToWorld(int x, int y, float z, GLfloat* glx, GLfloat* gly, GLfloat* glz) const
{
 *glx = (((GLfloat)x));
 *gly = (((GLfloat)-y));
 *glz = z;
}

bool BosonBigDisplayBase::mapCoordinates(const QPoint& pos, GLfloat* posX, GLfloat* posY, GLfloat* posZ, bool useRealDepth) const
{
 return Bo3dTools::mapCoordinates(d->mModelviewMatrix, d->mProjectionMatrix, d->mViewport,
		pos, posX, posY, posZ, useRealDepth);
}

bool BosonBigDisplayBase::mapDistance(int windx, int windy, GLfloat* dx, GLfloat* dy) const
{
 return Bo3dTools::mapDistance(d->mModelviewMatrix, d->mProjectionMatrix, d->mViewport,
		windx, windy, dx, dy);
}

bool BosonBigDisplayBase::mapCoordinatesToCell(const QPoint& pos, QPoint* cell)
{
 GLfloat x, y, z;
 if (!canvas()) {
	BO_NULL_ERROR(canvas());
	return false;
 }
 if (!mapCoordinates(pos, &x, &y, &z)) {
	return false;
 }
 y *= -1;
 int cellX = (int)(x);
 int cellY = (int)(y);
 cellX = QMAX(0, QMIN((int)canvas()->mapWidth(), cellX));
 cellY = QMAX(0, QMIN((int)canvas()->mapHeight(), cellY));
 cell->setX(cellX);
 cell->setY(cellY);
 return true;
}

void BosonBigDisplayBase::enterEvent(QEvent*)
{
 if (!cursor()) {
	// don't generate an error, since a NULL cursor might be valid for
	// editor mode
	return;
 }
// cursor()->showCursor();
}

void BosonBigDisplayBase::leaveEvent(QEvent*)
{
 if (!cursor()) {
	// don't generate an error, since a NULL cursor might be valid for
	// editor mode
	return;
 }
// cursor()->hideCursor();
}

void BosonBigDisplayBase::quitGame()
{
 boDebug() << k_funcinfo << endl;
 for (int i = 0; i < 10; i++) {
	slotClearSelectionGroup(i);
 }
 setLocalPlayerIO(0);
 setCanvas(0);

 // these are the important things - they *must* be cleared in order to avoid
 // crashes
 d->mToolTips->hideTip();
 setCursor(0);
 selection()->clear();
 d->mPlacementPreview.clear();
 d->mCanvasRenderer->reset();
 delete d->mMouseIO;
 d->mMouseIO = 0;
 delete d->mInput,
 d->mInput = 0;
 setLocalPlayerIO(0);
 setCanvas(0);

 // these are rather cosmetic. we won't crash if we don't clear
 d->mCursorEdgeTimer.stop();
 d->mCursorEdgeCounter = 0;
 d->mFps = 0;
 d->mFrameCount = 0;
 d->mSelectionRect.setVisible(false);
 d->mSelectionRect.setEndWidgetPos(d->mSelectionRect.startPos());
 d->mMouseMoveDiff.stop();
// setCamera(BoGameCamera()); do not do this! it calls cameraChanged() which generates cell list and all that stuff
 d->mCamera = BoGameCamera(canvas());
 d->mCamera.setCameraChanged(false);  // to prevent generating cell list and all that stuff
 delete d->mLightWidget;
 d->mLightWidget = 0;
 if (d->mGLMiniMap) {
	d->mGLMiniMap->quitGame();
 }

 setInputInitialized(false);
}

bool BosonBigDisplayBase::eventFilter(QObject* o, QEvent* e)
{
 switch (e->type()) {
	case QEvent::MouseMove:
		if (!d->mCursorEdgeTimer.isActive()) {
			slotCursorEdgeTimeout();
		}
		break;
	case QEvent::KeyPress:
	case QEvent::KeyRelease:
		d->mControlPressed = (((QKeyEvent*)e)->stateAfter() & Qt::ControlButton);
		d->mShiftPressed = (((QKeyEvent*)e)->stateAfter() & Qt::ShiftButton);

		// key events are sent to ufo here, mouse events elsewhere
		ufoManager()->sendEvent(e);
		break;
	default:
		break;
 }
 return BosonUfoGLWidget::eventFilter(o, e);
}

void BosonBigDisplayBase::slotUnitChanged(Unit* unit)
{
// FIXME: we might want to place this code into BoSelection directly (cleaner)
 if (!unit) {
	boError() << k_funcinfo << "NULL unit" << endl;
	return;
 }
 if (unit->isDestroyed()) {
	if (selection()->contains(unit)) {
		selection()->removeUnit(unit);
	}
 }
}

void BosonBigDisplayBase::moveSelectionRect(const QPoint& widgetPos)
{
 if (d->mSelectionRect.isVisible()) {
	d->mSelectionRect.setEndWidgetPos(widgetPos);
	// would be great here but is a huge performance
	// problem (therefore in releasebutton):
	// selectArea();
 }
}

void BosonBigDisplayBase::removeSelectionRect(bool replace)
{
 BO_CHECK_NULL_RET(displayInput());
 BO_CHECK_NULL_RET(localPlayerIO());
#warning FIXME: move to PlayerIO
 // only PlayerIO should be allowed to select units! there we should always
 // check whether a unit is fogged or not before selecting it!
 if (d->mSelectionRect.isVisible()) {
	// here as there is a performance problem in
	// mousemove:
	BoItemList* items = selectionRectItems();
	displayInput()->selectArea(items, replace);

	d->mSelectionRect.setVisible(false);
	if (!selection()->isEmpty()) {
		Unit* u = selection()->leader();
		if (localPlayerIO()->ownsUnit(u)) {
			// TODO: do not play sound here
			// instead make virtual and play in derived class
			u->playSound(SoundOrderSelect);
		}
	}
 } else {
	// a simple click on the map
	GLfloat x,y,z;
	if (!mapCoordinates(d->mSelectionRect.startPos(), &x, &y, &z)) {
		boError() << k_funcinfo << "Cannot map coordinates" << endl;
		return;
	}
	BoVector3Fixed canvasVector;
	worldToCanvas(x, y, z, &canvasVector);
	Unit* unit = 0;
	if (!canvas()->onCanvas(canvasVector)) {
		boError() << k_funcinfo << canvasVector.x() << "," << canvasVector.y() << " is not on the canvas!" << endl;
		return;
	}
	// this is not good: isFogged() should get checked *everywhere* where a
	// player tries to select a unit!
	// maybe in selectSingle() or so.
	if (!localPlayerIO()->isFogged((int)(canvasVector.x()), (int)(canvasVector.y()))) {
		unit = canvas()->collisions()->findUnitAt(canvasVector);
	}
	if (unit) {
		boDebug() << k_funcinfo << "select unit at " << canvasVector.x() << "," << canvasVector.y() << " (canvas)" << endl;
		displayInput()->selectSingle(unit, replace);
		// cannot be placed into selection() cause we don't have localPlayer
		// there
		if (localPlayerIO()->ownsUnit(unit)) {
			unit->playSound(SoundOrderSelect);
		}
	} else {
		if (replace) {
			selection()->clear();
		}
	}
 }
}


BoItemList* BosonBigDisplayBase::selectionRectItems()
{
 if (!canvas()) {
	return new BoItemList();
 }
 const bool debugMe = false;

 QRect widgetRect;
 d->mSelectionRect.widgetRect(&widgetRect);

 GLfloat maxX, maxY;
 GLfloat minX, minY;
 calculateWorldRect(widgetRect, &minX, &minY, &maxX, &maxY);
 maxY /= -1.0f;
 minY /= -1.0f;

 if (debugMe) {
	boDebug() << k_funcinfo << "maxX: " << maxX << " maxY: " << maxY
			<< " minX: " << minX
			<< " minY: " << minY
			<< endl;
 }


 // now the really ugly part starts. we really should improve this.
 // the rect (pos1, pos2) is the smallest possible 2d rect that contains all
 // cells (with 2d rect i mean: parallel to x and y world-axis).
 // but we we need an actual world rect, as the view is most probably rotated
 // and so a lot of cells that are in the parallel rect are not in the actual
 // rect.
 // to achieve this we loop through all cells in the parallel rect and check
 // whether they are in the actual rect.
 // someone with some maths knowledge and/or some time can surely speed this up
 // by using some proper formulas!

 QRect cellRect = QRect(QPoint((int)minX, (int)minY), QPoint((int)maxX, (int)maxY)).normalize();
 QPoint win;
 QValueList<Cell*> cells;
 if (debugMe) {
	boDebug() << k_funcinfo << cellRect.left()
			<< " " << cellRect.right()
			<< " " << cellRect.top()
			<< " " << cellRect.bottom() << endl;
	boDebug() << "widgetrect: " << widgetRect.left()
			<< " " << widgetRect.right()
			<< " " << widgetRect.top()
			<< " " << widgetRect.bottom() << endl;
 }
 for (int x = cellRect.left(); x <= cellRect.right(); x++) {
	for (int y = cellRect.top(); y <= cellRect.bottom(); y++) {
		Cell* c = canvas()->cell(x, y);
		if (!c) {
			boDebug() << "NULL cell at " << x << "," << y << endl;
			continue;
		}
		// AB: we use z==0.0, that is not really nice here. anyway,
		// let's wait until it causes trouble.
		// (read: I don't know which kind of trouble it will cause)
		GLfloat glx, gly, glz;
		// top left corner of cell
		glx = x;
		gly = -y;
		glz = canvas()->map()->heightAtCorner(x, y);
		boProject(glx, gly, glz, &win);
		if (widgetRect.contains(win)) {
			if (debugMe) {
				boDebug() << "adding cell at " << x << "," << y << endl;
			}
			cells.append(c);
			continue;
		}
		// top right corner of cell
		glx = (x + 1);
		gly = -y;
		glz = canvas()->map()->heightAtCorner(x + 1, y);
		boProject(glx, gly, glz, &win);
		if (widgetRect.contains(win)) {
			if (debugMe) {
				boDebug() << "adding cell at " << x << "," << y << endl;
			}
			cells.append(c);
			continue;
		}
		// bottom left corner of cell
		glx = x;
		gly = -(y + 1);
		glz = canvas()->map()->heightAtCorner(x, y + 1);
		boProject(glx, gly, glz, &win);
		if (widgetRect.contains(win)) {
			if (debugMe) {
				boDebug() << "adding cell at " << x << "," << y << endl;
			}
			cells.append(c);
			continue;
		}
		// bottom right corner of cell
		glx = (x + 1);
		gly = -(y + 1);
		glz = canvas()->map()->heightAtCorner(x + 1, y + 1);
		boProject(glx, gly, glz, &win);
		if (widgetRect.contains(win)) {
			if (debugMe) {
				boDebug() << "adding cell at " << x << "," << y << endl;
			}
			cells.append(c);
			continue;
		}
		if (debugMe) {
			boDebug() << "not adding cell at " << x << "," << y
					<< " at winx=" << win.x() << " "
					<< " winy=" << win.y() << endl;
		}
	}
 }

 // another ugly part of this function...
 QPtrVector<Cell> cellVector(cells.count());
 QValueList<Cell*>::Iterator it;
 int i = 0;
 for (it = cells.begin(); it != cells.end(); ++it) {
	cellVector.insert(i, *it);
	i++;
 }

 return localPlayerIO()->unitsAtCells(&cellVector);
}

void BosonBigDisplayBase::addChatMessage(const QString& message)
{
 d->mUfoGameWidget->addChatMessage(message);
}

void BosonBigDisplayBase::slotCursorEdgeTimeout()
{
 BO_CHECK_NULL_RET(camera());
 if (!boGame) {
	return;
 }
 if (boGame->gameStatus() == KGame::Init) {
	// probably startup screen visible
	return;
 }
 float x = 0;
 float y = 0;
 const int sensity = boConfig->cursorEdgeSensity();
 QWidget* w = qApp->mainWidget();
 BO_CHECK_NULL_RET(w);
 QPoint pos = w->mapFromGlobal(QCursor::pos());

 const int move = 20; // FIXME hardcoded - use BosonConfig instead
 if (pos.x() <= sensity && pos.x() > -1) {
	x = -move;
 } else if (pos.x() >= w->width() - sensity && pos.x() <= w->width()) {
	x = move;
 }
 if (pos.y() <= sensity && pos.y() > -1) {
	y = -move;
 } else if (pos.y() >= w->height() - sensity && pos.y() <= w->height()) {
	y = move;
 }
 if (!x && !y || !sensity) {
	d->mCursorEdgeTimer.stop();
	d->mCursorEdgeCounter = 0;
 } else {
	GLfloat dx, dy;
	mapDistance((int)x, (int)y, &dx, &dy);
	if (!d->mCursorEdgeTimer.isActive()) {
		d->mCursorEdgeTimer.start(20);
	}
	d->mCursorEdgeCounter++;
	if (d->mCursorEdgeCounter > 30) {
		camera()->changeLookAt(BoVector3Float(dx, dy, 0));
	}
 }
}


void BosonBigDisplayBase::scrollBy(int dx, int dy)
{
 BO_CHECK_NULL_RET(camera());
 GLfloat x, y;
 mapDistance(dx, dy, &x, &y);
 camera()->changeLookAt(BoVector3Float(x, y, 0));
}

void BosonBigDisplayBase::slotRotateLeft(float factor)
{
 BO_CHECK_NULL_RET(camera());
 rotate(factor);
}

void BosonBigDisplayBase::slotRotateRight(float factor)
{
 BO_CHECK_NULL_RET(camera());
 rotate(-factor);
}

void BosonBigDisplayBase::rotate(float delta)
{
 camera()->changeRotation(delta);
}

void BosonBigDisplayBase::slotZoomIn(float factor)
{
 BO_CHECK_NULL_RET(camera());

 float delta = factor; // ?

 zoom(-delta);
}

void BosonBigDisplayBase::slotZoomOut(float factor)
{
 BO_CHECK_NULL_RET(camera());

 float delta = factor; // ?

 zoom(delta);
}

void BosonBigDisplayBase::zoom(float delta)
{
 BO_CHECK_NULL_RET(camera());
 BO_CHECK_NULL_RET(canvas());
 BO_CHECK_NULL_RET(canvas()->map());

#warning FIXME: d->mCanvasPos, d->mCanvasVector

 camera()->changeZ(delta);
}

// we have this here and in BoGroundRenderer!
#warning TODO: move to Bo3dTools
void BosonBigDisplayBase::calculateWorldRect(const QRect& rect, float* minX, float* minY, float* maxX, float* maxY) const
{
 BO_CHECK_NULL_RET(canvas());
 const BosonMap* map = canvas()->map();
 BO_CHECK_NULL_RET(map);
 GLfloat posX, posY;
 GLfloat posZ;
 mapCoordinates(rect.topLeft(), &posX, &posY, &posZ);
 *maxX = *minX = posX;
 *maxY = *minY = -posY;
 mapCoordinates(rect.topRight(), &posX, &posY, &posZ);
 *maxX = QMAX(*maxX, posX);
 *maxY = QMAX(*maxY, -posY);
 *minX = QMIN(*minX, posX);
 *minY = QMIN(*minY, -posY);
 mapCoordinates(rect.bottomLeft(), &posX, &posY, &posZ);
 *maxX = QMAX(*maxX, posX);
 *maxY = QMAX(*maxY, -posY);
 *minX = QMIN(*minX, posX);
 *minY = QMIN(*minY, -posY);
 mapCoordinates(rect.bottomRight(), &posX, &posY, &posZ);
 *maxX = QMAX(*maxX, posX);
 *maxY = QMAX(*maxY, -posY);
 *minX = QMIN(*minX, posX);
 *minY = QMIN(*minY, -posY);

 *maxX = QMAX(0, *maxX);
 *maxY = QMAX(0, *maxY);
 *minX = QMAX(0, *minX);
 *minY = QMAX(0, *minY);
 *maxX = QMIN((map->width() - 1), *maxX);
 *minX = QMIN((map->width() - 1), *minX);
 *maxY = QMIN((map->height() - 1), *maxY);
 *minY = QMIN((map->height() - 1), *minY);
 *minY *= -1;
 *maxY *= -1;
}

void BosonBigDisplayBase::setCamera(const BoGameCamera& camera)
{
 d->mCamera = camera;
}

void BosonBigDisplayBase::cameraChanged()
{
 if (!isInitialized()) {
	initGL();
 }
 makeCurrent();

 camera()->applyCameraToScene();

 if (checkError()) {
	boError() << k_funcinfo << "after BoGameCamera::applyCameraToScene()" << endl;
 }

 // the applyCameraToScene() above is the most important call for the modelview matrix.
 // everything else will be discarded by glPushMatrix/glPopMatrix anyway (in
 // paintGL()). So we cache the matrix here, for mapCoordinates() and some other
 // stuff
 d->mModelviewMatrix.loadMatrix(GL_MODELVIEW_MATRIX);

 extractFrustum(); // modelview matrix changed
 BoGroundRenderer* renderer = BoGroundRendererManager::manager()->currentRenderer();
 if (renderer) {
	BosonMap* map = 0;
	if (canvas()) {
		map = canvas()->map();
	}
	renderer->generateCellList(map);
 }

 d->mCanvasRenderer->setParticlesDirty(true);

 const QValueVector<BoLight*>* lights = BoLightManager::lights();
 for (unsigned int i = 0; i < lights->size(); i++) {
	if (lights->at(i) != 0) {
	  lights->at(i)->refreshPosition();
	}
 }

 // Update position of environmental effects
 QPtrListIterator<BosonEffect> it(*canvas()->effects());
 while (it.current()) {
	if (it.current()->type() == BosonEffect::ParticleEnvironmental) {
		it.current()->setPosition(camera()->cameraPos().toFixed());
	}
	++it;
 }


 boWaterManager->modelviewMatrixChanged(d->mModelviewMatrix);
 boWaterManager->setCameraPos(camera()->cameraPos());

 QPoint cellTL; // topleft cell
 QPoint cellTR; // topright cell
 QPoint cellBL; // bottomleft cell
 QPoint cellBR; // bottomright cell
 mapCoordinatesToCell(QPoint(0, 0), &cellTL);
 mapCoordinatesToCell(QPoint(0, d->mViewport[3]), &cellBL);
 mapCoordinatesToCell(QPoint(d->mViewport[2], 0), &cellTR);
 mapCoordinatesToCell(QPoint(d->mViewport[2], d->mViewport[3]), &cellBR);
 emit signalChangeViewport(this, cellTL, cellTR, cellBL, cellBR);
}

BoGameCamera* BosonBigDisplayBase::camera() const
{
 return &d->mCamera;
}

BoAutoGameCamera* BosonBigDisplayBase::autoCamera() const
{
 return camera()->autoGameCamera();
}

bool BosonBigDisplayBase::checkError() const
{
 return Bo3dTools::checkError();
}

void BosonBigDisplayBase::setUpdateInterval(unsigned int ms)
{
 boDebug() << k_funcinfo << ms << endl;
 d->mUpdateInterval = ms;
 boProfiling->setGLUpdateInterval(ms);
 QTimer::singleShot(d->mUpdateInterval, this, SLOT(slotUpdateGL()));
}

void BosonBigDisplayBase::calcFPS()
{
 static long long int now = 0;
 struct timeval time;
 gettimeofday(&time, 0);
 now = time.tv_sec * 1000000 + time.tv_usec;
 // FPS is updated once per second
 if ((now - d->mFpsTime) >= 1000000) {
	d->mFps = d->mFrameCount / ((now - d->mFpsTime) / 1000000.0);
	d->mFpsTime = now;
	d->mFrameCount = 0;
//	boDebug() << k_funcinfo << "FPS: " << d->mFps << endl;
 }
}

double BosonBigDisplayBase::fps() const
{
  return d->mFps;
}

void BosonBigDisplayBase::setViewport(int x, int y, GLsizei w, GLsizei h)
{
 if (!isInitialized()) {
	initGL();
 }
 makeCurrent();
 glViewport(x, y, w, h);
 // AB: we could use glGetIntegerv(GL_VIEWPORT, d->mViewport); here. But our own
 // version should be the same
 d->mViewport[0] = x;
 d->mViewport[1] = y;
 d->mViewport[2] = w;
 d->mViewport[3] = h;
}

void BosonBigDisplayBase::extractFrustum()
{
 // modelview or projection matrix was changed (and therefore the frustum).
 GLfloat t;

 // Combine the two matrices (multiply projection by modelview)
 BoMatrix clip(d->mProjectionMatrix);
 clip.multiply(&d->mModelviewMatrix);


 // Extract the numbers for the RIGHT plane
 d->mViewFrustum[0 * 4 + 0] = clip[3] - clip[0];
 d->mViewFrustum[0 * 4 + 1] = clip[7] - clip[4];
 d->mViewFrustum[0 * 4 + 2] = clip[11] - clip[8];
 d->mViewFrustum[0 * 4 + 3] = clip[15] - clip[12];

 // Normalize the result
 // ( AB: normalizing means to make a unit vector, i.e. a vector with length 1! )
 // ( AB: the length of a vector v is |v| == sqrt(v[0]^2 + v[1]^2 + v[2]^2) )
 // ( AB: you can normalize a vector by doing v / |v| )
 t = sqrt(d->mViewFrustum[0 * 4 + 0] * d->mViewFrustum[0 * 4 + 0] +
		d->mViewFrustum[0 * 4 + 1] * d->mViewFrustum[0 * 4 + 1] +
		d->mViewFrustum[0 * 4 + 2] * d->mViewFrustum[0 * 4 + 2]);
 d->mViewFrustum[0 * 4 + 0] /= t;
 d->mViewFrustum[0 * 4 + 1] /= t;
 d->mViewFrustum[0 * 4 + 2] /= t;
 d->mViewFrustum[0 * 4 + 3] /= t;

 // Extract the numbers for the LEFT plane
 d->mViewFrustum[1 * 4 + 0] = clip[3] + clip[0];
 d->mViewFrustum[1 * 4 + 1] = clip[7] + clip[4];
 d->mViewFrustum[1 * 4 + 2] = clip[11] + clip[8];
 d->mViewFrustum[1 * 4 + 3] = clip[15] + clip[12];

 // Normalize the result
 t = sqrt(d->mViewFrustum[1 * 4 + 0] * d->mViewFrustum[1 * 4 + 0] +
		d->mViewFrustum[1 * 4 + 1] * d->mViewFrustum[1 * 4 + 1] +
		d->mViewFrustum[1 * 4 + 2] * d->mViewFrustum[1 * 4 + 2]);
 d->mViewFrustum[1 * 4 + 0] /= t;
 d->mViewFrustum[1 * 4 + 1] /= t;
 d->mViewFrustum[1 * 4 + 2] /= t;
 d->mViewFrustum[1 * 4 + 3] /= t;

 // Extract the BOTTOM plane
 d->mViewFrustum[2 * 4 + 0] = clip[3] + clip[1];
 d->mViewFrustum[2 * 4 + 1] = clip[7] + clip[5];
 d->mViewFrustum[2 * 4 + 2] = clip[11] + clip[9];
 d->mViewFrustum[2 * 4 + 3] = clip[15] + clip[13];

 // Normalize the result
 t = sqrt(d->mViewFrustum[2 * 4 + 0] * d->mViewFrustum[2 * 4 + 0] +
		d->mViewFrustum[2 * 4 + 1] * d->mViewFrustum[2 * 4 + 1] +
		d->mViewFrustum[2 * 4 + 2] * d->mViewFrustum[2 * 4 + 2]);
 d->mViewFrustum[2 * 4 + 0] /= t;
 d->mViewFrustum[2 * 4 + 1] /= t;
 d->mViewFrustum[2 * 4 + 2] /= t;
 d->mViewFrustum[2 * 4 + 3] /= t;

 // Extract the TOP plane
 d->mViewFrustum[3 * 4 + 0] = clip[3] - clip[1];
 d->mViewFrustum[3 * 4 + 1] = clip[7] - clip[5];
 d->mViewFrustum[3 * 4 + 2] = clip[11] - clip[9];
 d->mViewFrustum[3 * 4 + 3] = clip[15] - clip[13];

 // Normalize the result
 t = sqrt(d->mViewFrustum[3 * 4 + 0] * d->mViewFrustum[3 * 4 + 0] +
		d->mViewFrustum[3 * 4 + 1] * d->mViewFrustum[3 * 4 + 1] +
		d->mViewFrustum[3 * 4 + 2] * d->mViewFrustum[3 * 4 + 2]);
 d->mViewFrustum[3 * 4 + 0] /= t;
 d->mViewFrustum[3 * 4 + 1] /= t;
 d->mViewFrustum[3 * 4 + 2] /= t;
 d->mViewFrustum[3 * 4 + 3] /= t;

 // Extract the FAR plane
 d->mViewFrustum[4 * 4 + 0] = clip[3] - clip[2];
 d->mViewFrustum[4 * 4 + 1] = clip[7] - clip[6];
 d->mViewFrustum[4 * 4 + 2] = clip[11] - clip[10];
 d->mViewFrustum[4 * 4 + 3] = clip[15] - clip[14];

 // Normalize the result
 t = sqrt(d->mViewFrustum[4 * 4 + 0] * d->mViewFrustum[4 * 4 + 0] +
		d->mViewFrustum[4 * 4 + 1] * d->mViewFrustum[4 * 4 + 1] +
		d->mViewFrustum[4 * 4 + 2] * d->mViewFrustum[4 * 4 + 2]);
 d->mViewFrustum[4 * 4 + 0] /= t;
 d->mViewFrustum[4 * 4 + 1] /= t;
 d->mViewFrustum[4 * 4 + 2] /= t;
 d->mViewFrustum[4 * 4 + 3] /= t;

 // Extract the NEAR plane
 d->mViewFrustum[5 * 4 + 0] = clip[3] + clip[2];
 d->mViewFrustum[5 * 4 + 1] = clip[7] + clip[6];
 d->mViewFrustum[5 * 4 + 2] = clip[11] + clip[10];
 d->mViewFrustum[5 * 4 + 3] = clip[15] + clip[14];

 // Normalize the result
 t = sqrt(d->mViewFrustum[5 * 4 + 0] * d->mViewFrustum[5 * 4 + 0] +
		d->mViewFrustum[5 * 4 + 1] * d->mViewFrustum[5 * 4 + 1] +
		d->mViewFrustum[5 * 4 + 2] * d->mViewFrustum[5 * 4 + 2]);
 d->mViewFrustum[5 * 4 + 0] /= t;
 d->mViewFrustum[5 * 4 + 1] /= t;
 d->mViewFrustum[5 * 4 + 2] /= t;
 d->mViewFrustum[5 * 4 + 3] /= t;
}

float BosonBigDisplayBase::sphereInFrustum(const BoVector3Fixed& pos, float radius) const
{
 return Bo3dTools::sphereInFrustum(d->mViewFrustum, pos, radius);
}

const BoVector3Fixed& BosonBigDisplayBase::cursorCanvasVector() const
{
 return d->mCanvasVector;
}

void BosonBigDisplayBase::setParticlesDirty(bool dirty)
{
 d->mCanvasRenderer->setParticlesDirty(dirty);
}

void BosonBigDisplayBase::setPlacementPreviewData(const UnitProperties* prop, bool canPlace)
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
	BoFrame* f = m->frame(0);
	if (!f) {
		boError() << k_funcinfo << "NULL frame 0" << endl;
		return;
	}
	d->mPlacementPreview.setData(prop, m);
 }
 d->mPlacementPreview.setCanPlace(canPlace);
 d->mPlacementPreview.setCanvasPos(BoVector2Fixed(cursorCanvasVector().x(), cursorCanvasVector().y()));
}

void BosonBigDisplayBase::setPlacementCellPreviewData(unsigned int textureCount, unsigned char* alpha, bool canPlace)
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
 d->mPlacementPreview.setCanvasPos(BoVector2Fixed(cursorCanvasVector().x(), cursorCanvasVector().y()));
}

void BosonBigDisplayBase::setDisplayInput(BosonBigDisplayInputBase* input)
{
 if (d->mInput) {
	boWarning() << k_funcinfo << "existing input non-NULL" << endl;
	delete d->mInput;
 }
 d->mInput = input;

 connect(d->mUfoGameWidget, SIGNAL(signalPlaceGround(unsigned int, unsigned char*)),
		input, SLOT(slotPlaceGround(unsigned int, unsigned char*)));
 connect(d->mUfoGameWidget, SIGNAL(signalPlaceUnit(unsigned int, Player*)),
		input, SLOT(slotPlaceUnit(unsigned int, Player*)));
}

BosonBigDisplayInputBase* BosonBigDisplayBase::displayInput() const
{
 return d->mInput;
}

void BosonBigDisplayBase::setToolTipCreator(int type)
{
 d->mToolTips->setToolTipCreator(type);
}

void BosonBigDisplayBase::setToolTipUpdatePeriod(int ms)
{
 d->mToolTips->setUpdatePeriod(ms);
}

bool BosonBigDisplayBase::boProject(GLfloat x, GLfloat y, GLfloat z, QPoint* pos) const
{
 return Bo3dTools::boProject(d->mModelviewMatrix, d->mProjectionMatrix, d->mViewport,
		x, y, z, pos);
}

bool BosonBigDisplayBase::boUnProject(const QPoint& pos, BoVector3Float* ret, float z) const
{
 return Bo3dTools::boUnProject(d->mModelviewMatrix, d->mProjectionMatrix, d->mViewport,
		pos, ret, z);
}

void BosonBigDisplayBase::slotRemovedItemFromCanvas(BosonItem* item)
{
 // be careful with the item pointer! this is usually called from the BosonItem
 // destructor, its functions might be destroyed already!
 BO_CHECK_NULL_RET(item);
 d->mToolTips->unsetItem(item);
}

void BosonBigDisplayBase::slotUnitRemoved(Unit* unit)
{
 // AB: this slot is already called when the unit is destroyed, not only when it
 // is completely removed from the canvas
 for(int i = 0; i < 10; i++) {
	d->mSelectionGroups[i]->removeUnit(unit);
 }
}

void BosonBigDisplayBase::slotMouseIODestroyed()
{
 // the mouse IO sometimes gets destroyed outside this widget (when the player
 // is removed). we need to set the pointer to NULL then...
 boDebug() << k_funcinfo << endl;
 d->mMouseIO = 0;
}

void BosonBigDisplayBase::loadFromXML(const QDomElement& root)
{
 boDebug() << k_funcinfo << endl;

 // Load selection groups
 QDomElement unitGroups = root.namedItem(QString::fromLatin1("UnitGroups")).toElement();
 if (unitGroups.isNull()) {
	boError(260) << k_funcinfo << "no UnitGroups tag" << endl;
	return;
 }
 QDomNodeList list = unitGroups.elementsByTagName(QString::fromLatin1("Group"));
 if (list.count() == 0) {
	boWarning(260) << k_funcinfo << "no unitgroups" << endl;
	return;
 }
 for (unsigned int i = 0; i < list.count(); i++) {
	QDomElement e = list.item(i).toElement();
	if (e.isNull()) {
		boError(260) << k_funcinfo << i << " is not an element" << endl;
		return;
	}
	if (!e.hasAttribute("Id")) {
		boError(260) << k_funcinfo << "missing attribute: Id for Group " << i << endl;
		continue;
	}
	int id;
	bool ok;
	id = e.attribute("Id").toInt(&ok);
	if (!ok) {
		boError(260) << k_funcinfo << "Invalid Id for Group " << i << endl;
		continue;
	}
	if (!d->mSelectionGroups[id]) {
		boError(260) <<k_funcinfo << "no unitgroup with id=" << id << endl;
		continue;
	}
	d->mSelectionGroups[id]->loadFromXML(e);
 }


 QDomElement displays = root.namedItem(QString::fromLatin1("Displays")).toElement();
 if (displays.isNull()) {
	boError(260) << k_funcinfo << "no displays" << endl;
	return;
 }
 QDomElement display = displays.namedItem(QString::fromLatin1("Display")).toElement();
 if (display.isNull()) {
	boError(260) << k_funcinfo << "no display" << endl;
	return;
 }

 // Load camera
 QDomElement cam = display.namedItem(QString::fromLatin1("Camera")).toElement();
 if (!cam.isNull()) {
	camera()->loadFromXML(cam);
 } else {
	boError(260) << k_funcinfo << "no camera" << endl;
 }
 // Load selection
 QDomElement sel = display.namedItem(QString::fromLatin1("Selection")).toElement();
 if (!sel.isNull()) {
	selection()->loadFromXML(sel, true);
 } else {
	boError(260) << k_funcinfo << "no selection" << endl;
	selection()->clear(); // just in case...
 }
}

void BosonBigDisplayBase::saveAsXML(QDomElement& root)
{
 boDebug() << k_funcinfo << endl;
 QDomDocument doc = root.ownerDocument();

 // Save selection groups
 QDomElement unitGroups = doc.createElement(QString::fromLatin1("UnitGroups"));
 for(int i = 0; i < 10; i++) {
	QDomElement group = doc.createElement(QString::fromLatin1("Group"));
	group.setAttribute("Id", i);
	d->mSelectionGroups[i]->saveAsXML(group);
	unitGroups.appendChild(group);
 }
 root.appendChild(unitGroups);


 // we use a Displays and a Display tag for historic reasons. we have only one
 // Display these days.
 QDomElement displays = doc.createElement(QString::fromLatin1("Displays"));
 QDomElement display = doc.createElement(QString::fromLatin1("Display"));
 displays.appendChild(display);
 root.appendChild(displays);

 // Save camera
 QDomElement cam = doc.createElement(QString::fromLatin1("Camera"));
 camera()->saveAsXML(cam);
 display.appendChild(cam);
 // Save current selection
 QDomElement sel = doc.createElement(QString::fromLatin1("Selection"));
 selection()->saveAsXML(sel);
 display.appendChild(sel);
}

void BosonBigDisplayBase::showEvent(QShowEvent* e)
{
 BosonUfoGLWidget::showEvent(e);
 if (displayInput()) {
	displayInput()->updateCursor();
 }
}

bool BosonBigDisplayBase::isInputInitialized()
{
 return d->mInputInitialized;
}

void BosonBigDisplayBase::setInputInitialized(bool initialized)
{
 d->mInputInitialized = initialized;
}

QByteArray BosonBigDisplayBase::grabMovieFrame()
{
 boDebug() << k_funcinfo << "Grabbing movie frame..." << endl;

#if 0
 QByteArray data;
 QDataStream stream(data, IO_WriteOnly);
 QDomDocument doc("BosonMovie");
 QDomElement root = doc.createElement("BosonMovie");
 doc.appendChild(root);
 {
	QDomElement c = doc.createElement("Camera");
	root.appendChild(c);
	camera()->saveAsXML(c);
 }

 createRenderItemList();
 {
	QMap<unsigned int, QDomElement> owner2Items;
	for (KPlayer* p = boGame->playerList()->first(); p; p = boGame->playerList()->next()) {
		QDomElement items = doc.createElement(QString::fromLatin1("Items"));
		items.setAttribute(QString::fromLatin1("OwnerId"), p->id());
		root.appendChild(items);
		owner2Items.insert(p->id(), items);
	}

	BoItemList::Iterator it;
	for (it = d->mRenderItemList->begin(); it != d->mRenderItemList->end(); ++it) {
		BosonItem* i = *it;
		QDomElement items;
		if (RTTI::isShot(i->rtti())) {
			BosonShot* s = (BosonShot*)i;
			if (!s->owner()) {
				BO_NULL_ERROR(s->owner());
				continue;
			}
			unsigned int id = s->owner()->id();
			items = owner2Items[id];
		} else if (RTTI::isUnit(i->rtti())) {
			Unit* u = (Unit*)i;
			if (!u->owner()) {
				BO_NULL_ERROR(u->owner());
				continue;
			}
			unsigned int id = u->owner()->id();
			items = owner2Items[id];
		}
		if (items.isNull()) {
			boError() << k_funcinfo << "no Items element found" << endl;
			continue;
		}
		QDomElement item = doc.createElement("Item");
		if (!i->saveAsXML(item)) {
			boError() << k_funcinfo << "error saving item" << endl;
			continue;
		}
		items.appendChild(item);
	}
 }



 stream << doc.toString();

 return data;
#else
 // Repaint
 slotUpdateGL();
 glFinish();

 // Slots in Qt can be called in any order, so it is possible that particle
 //  systems haven't been advanced yet. If particles' dirty flag would remain
 //  false, then if some particles or particle systems would be deleted, we'd
 //  have 0 pointers in particle list next time.
 // No need to do this before repainting, because it has already been done in
 //  BoDisplayManager (setParticlesDirty() is called before grabbing movie
 //  frame)
 d->mCanvasRenderer->setParticlesDirty(true);

 // WARNING this is NOT dependable! e.g. if boson has the focus, but another
 // window is in front of it, then the other window will be grabbed as well!
 // better render to a pixmap instead.
 QPixmap shot = QPixmap::grabWindow(winId());

 QByteArray ba;
 QBuffer b(ba);
 b.open(IO_WriteOnly);
 QImageIO io(&b, "JPEG");
 io.setImage(shot.convertToImage());
 io.setQuality(90);
 io.write();
 return ba;
#endif
}

void BosonBigDisplayBase::advanceCamera()
{
 autoCamera()->advance();
}

void BosonBigDisplayBase::updateOpenGLSettings()
{
 if (!isInitialized()) {
	initGL();
 }
 makeCurrent();

 // AB: note there seems to be hardly a difference between flat and smooth
 // shading (in both quality and speed)
 if (boConfig->boolValue("SmoothShading", true)) {
	glShadeModel(GL_SMOOTH);
 } else {
	glShadeModel(GL_FLAT);
 }
}

void BosonBigDisplayBase::changeGroundRenderer(int renderer)
{
 bool ret = BoGroundRendererManager::manager()->makeRendererIdCurrent(renderer);
 BoGroundRenderer* r = BoGroundRendererManager::manager()->currentRenderer();
 if (!ret) {
	if (r) {
		KMessageBox::sorry(this, i18n("Unable to load renderer with id=%1. Continue with old renderer.").arg(renderer));
	} else {
		BoGroundRendererManager::manager()->makeRendererCurrent(QString::null);
		r = BoGroundRendererManager::manager()->currentRenderer();
		if (!r) {
			KMessageBox::sorry(this, i18n("Unable to load any ground renderer, check your installation! Quitting now."));
			exit(1);
		}
	}
	return;
 }
 if (!r) {
	KMessageBox::sorry(this, i18n("New ground renderer has been loaded successfully, but can't be accessed. Weird bug - please report (including debug output on konsole)!\nQuitting now"));
	exit(1);
	return;
 }
}

void BosonBigDisplayBase::generateMovieFrames(const QValueList<QByteArray>& data, const QString& directory)
{
 if (data.count() == 0) {
	return;
 }
#if 0
 BO_CHECK_NULL_RET(d->mGroundRenderer);
 QDir dir(directory);
 if (!dir.exists()) {
	boError() << k_funcinfo << "direcotry " << directory << " does not exist" << endl;
	return;
 }
 // TODO QDir has no isWritable() :-(
#if 0
 if (!dir.isWritable()) {
	boError() << k_funcinfo << "directory " << dir.absPath() << " is not writable!" << endl;
	return;
 }
#endif
 if (data.count() > 999999999) {
	// currently filenames are fixed to 8 digits, so we cannot have more
	// files.
	boError() << k_funcinfo << "too many files!" << endl;
	return;
 }
 QString prefix = "boson-movie";
 QString suffix = ".jpg";
 bool ok = false;

 // try to find a filename prefix for data.count() files.
 unsigned int tries = 0;
 do {
	tries++;
	QString prefix2 = prefix;
	if (tries > 1) {
		prefix2 += "_";
		prefix2 += QString::number(tries);
	}
	QString file;
	bool success = true;
	for (unsigned int i = 0; i < data.count() && success; i++) {
		file.sprintf("%s-%08d%s", prefix2.latin1(), i, suffix.latin1());
		if (dir.exists(file)) {
			success = false;
		}
	}
	if (success) {
		ok = true;
		prefix = prefix2;
	}

 } while (!ok && tries < 100);
 if (!ok) {
	boError() << k_funcinfo << "could not find usable filename prefix for " << data.count() << " files" << endl;
	return;
 }

 // TODO: we should save the game here and load it again when we are done.
 // all units will be removed from the game when generating the frames...
 //
 // maybe we just use a different canvas?

 BosonCanvas* old = mCanvas;

 // WARNING: units that are added to the canvas get added to the player, too!
 // --> we must remove them asap again, as game can continue
 mCanvas = new BosonCanvas(this);
 mCanvas->setMap(old->map());

 // now generate the frames
 BoPixmapRenderer* renderer = new BoPixmapRenderer(this);
 for (unsigned int i = 0; i < data.count(); i++) {
	QString file;
	file.sprintf("%s-%08d%s", prefix.latin1(), i, suffix.latin1());
	file = dir.filePath(file);
	renderer->startPixmap();
	generateMovieFrame(data[i], renderer);
	renderer->pixmapDone(file);
	renderer->flush();
 }
 delete renderer;
 delete mCanvas;
 mCanvas = old;
#endif
}

void BosonBigDisplayBase::generateMovieFrame(const QByteArray& data, BoPixmapRenderer* renderer)
{
#if 0
 if (data.size() == 0) {
	return;
 }
 if (!renderer) {
	return;
 }
 BO_CHECK_NULL_RET(d->mGroundRenderer);

 QDataStream stream(data, IO_ReadOnly);
 QString xml;
 stream >> xml;
 if (xml.isEmpty()) {
	boError() << "empty xml string" << endl;
	return;
 }
 QDomDocument doc;
 doc.setContent(xml);
 QDomElement root = doc.documentElement();

 QDomNodeList list;
 list = root.elementsByTagName("Camera");
 if (list.count() == 0) {
	boError() << k_funcinfo << "no camera node" << endl;
	return;
 }
 QDomElement cameraElement = list.item(0).toElement();
 if (cameraElement.isNull()) {
	boError() << k_funcinfo << "invalid camera element" << endl;
	return;
 }
 camera()->loadFromXML(cameraElement);


 // remove _all_ items from the canvas.
 canvas()->allItems()->clear();
 mCanvas->loadFromXML(root);

 // remove _all_ particle systems from the canvas
 canvas()->particleSystems()->clear();
// canvas()->particleSystems()->append(data.particleSystems); // TODO

 // AB: placement preview, cursor and selection rect are not rendered into the
 // movie

 // TODO: text
 // --> player data must be saved! - e.g. minerals/oil
 // localPlayer()->load(data.localPlayer);


 slotUpdateGL();
#endif
}

BoLight* BosonBigDisplayBase::light(int id) const
{
 return BoLightManager::light(id);
}

BoLight* BosonBigDisplayBase::newLight()
{
 return BoLightManager::createLight();
}

void BosonBigDisplayBase::removeLight(int id)
{
 BoLightManager::deleteLight(id);
}

QImage BosonBigDisplayBase::screenShot()
{
 glFinish();
 // btw: equal to width() and height()
 int w = d->mViewport[2];
 int h = d->mViewport[3];
 unsigned char* buffer = new unsigned char[w * h * 4];
 glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
 QImage image(w, h, 32);
 for (int y = 0; y < h; y++) {
	QRgb* line = (QRgb*)image.scanLine(y); // AB: use setPixel() instead of scanLine() ! -> endianness must be handled
	int opengl_y = h - y;
	for (int x = 0; x < w; x++) {
		unsigned char* pixel = &buffer[(opengl_y * w + x) * 4];
		line[x] = qRgb(pixel[0], pixel[1], pixel[2]);
	}
 }
 delete[] buffer;
 return image;
}

void BosonBigDisplayBase::slotInitMiniMapFogOfWar()
{
 BO_CHECK_NULL_RET(d->mGLMiniMap);
 if (boGame->gameMode()) {
	d->mGLMiniMap->initFogOfWar(localPlayerIO());
 } else {
	d->mGLMiniMap->initFogOfWar(0);
 }
}

void BosonBigDisplayBase::addLineVisualization(BoLineVisualization v)
{
 d->mLineVisualizationList.append(v);
}


// FIXME: someone who know what these are supposed to be for, could probably
// group most parameters into a single "type" integer
void BosonBigDisplayBase::slotAddLineVisualization(const QValueList<BoVector3Fixed>& points, const BoVector4Float& color, bofixed pointSize, int timeout, bofixed zOffset)
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

void BosonBigDisplayBase::advanceLineVisualization()
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

void BosonBigDisplayBase::setFont(const BoFontInfo& font)
{
 makeCurrent();
 delete d->mDefaultFont;
 boDebug() << k_funcinfo << font.name() << " " << font.pointSize() << endl;
 d->mDefaultFont = new BosonGLFont(font);
}

BosonBigDisplayScriptConnector::BosonBigDisplayScriptConnector(BosonBigDisplayBase* parent)
	: QObject(parent, "script_to_display_connector")
{
 mDisplay = parent;
 if (!mDisplay) {
	BO_NULL_ERROR(mDisplay);
 }
}

BosonBigDisplayScriptConnector::~BosonBigDisplayScriptConnector()
{
}

void BosonBigDisplayScriptConnector::reconnect(const QObject* sender, const char* signal, const QObject* receiver, const char* slot)
{
 // make sure noone else connects to that signal
 disconnect(sender, signal, 0, 0);
 connect(sender, signal, receiver, slot);
}

void BosonBigDisplayScriptConnector::connectToScript(BosonScript* script)
{
 BO_CHECK_NULL_RET(script);
 BosonScriptInterface* i = script->interface();
 BO_CHECK_NULL_RET(i);

 // AB: the slots often provide the return value for a signal, so there must be
 // at most 1 slot to a signal. reconnect() ensures that.

 reconnect(i, SIGNAL(signalAddLight(int*)), this, SLOT(slotAddLight(int*)));
 reconnect(i, SIGNAL(signalRemoveLight(int)), this, SLOT(slotRemoveLight(int)));
 reconnect(i, SIGNAL(signalGetLightPos(int, BoVector4Float*)),
		this, SLOT(slotGetLightPos(int, BoVector4Float*)));
 reconnect(i, SIGNAL(signalGetLightAmbient(int, BoVector4Float*)),
		this, SLOT(slotGetLightAmbient(int, BoVector4Float*)));
 reconnect(i, SIGNAL(signalGetLightDiffuse(int, BoVector4Float*)),
		this, SLOT(slotGetLightDiffuse(int, BoVector4Float*)));
 reconnect(i, SIGNAL(signalGetLightSpecular(int, BoVector4Float*)),
		this, SLOT(slotGetLightSpecular(int, BoVector4Float*)));
 reconnect(i, SIGNAL(signalGetLightAttenuation(int, BoVector3Float*)),
		this, SLOT(slotGetLightAttenuation(int, BoVector3Float*)));
 reconnect(i, SIGNAL(signalGetLightEnabled(int, bool*)),
		this, SLOT(slotGetLightEnabled(int, bool*)));
 reconnect(i, SIGNAL(signalSetLightPos(int, const BoVector4Float&)),
		this, SLOT(slotSetLightPos(int, const BoVector4Float&)));
 reconnect(i, SIGNAL(signalSetLightAmbient(int, const BoVector4Float&)),
		this, SLOT(slotSetLightAmbient(int, const BoVector4Float&)));
 reconnect(i, SIGNAL(signalSetLightDiffuse(int, const BoVector4Float&)),
		this, SLOT(slotSetLightDiffuse(int, const BoVector4Float&)));
 reconnect(i, SIGNAL(signalSetLightSpecular(int, const BoVector4Float&)),
		this, SLOT(slotSetLightSpecular(int, const BoVector4Float&)));
 reconnect(i, SIGNAL(signalSetLightAttenuation(int, const BoVector3Float&)),
		this, SLOT(slotSetLightAttenuation(int, const BoVector3Float&)));
 reconnect(i, SIGNAL(signalSetLightEnabled(int, bool)),
		this, SLOT(slotSetLightEnabled(int, bool)));

 reconnect(i, SIGNAL(signalGetCameraPos(BoVector3Float*)),
		this, SLOT(slotGetCameraPos(BoVector3Float*)));
 reconnect(i, SIGNAL(signalGetCameraLookAt(BoVector3Float*)),
		this, SLOT(slotGetCameraLookAt(BoVector3Float*)));
 reconnect(i, SIGNAL(signalGetCameraUp(BoVector3Float*)),
		this, SLOT(slotGetCameraUp(BoVector3Float*)));
 reconnect(i, SIGNAL(signalGetCameraRotation(float*)),
		this, SLOT(slotGetCameraRotation(float*)));
 reconnect(i, SIGNAL(signalGetCameraRadius(float*)),
		this, SLOT(slotGetCameraRadius(float*)));
 reconnect(i, SIGNAL(signalGetCameraZ(float*)),
		this, SLOT(slotGetCameraZ(float*)));
 reconnect(i, SIGNAL(signalSetUseCameraLimits(bool)),
		this, SLOT(slotSetUseCameraLimits(bool)));
 reconnect(i, SIGNAL(signalSetCameraFreeMovement(bool)),
		this, SLOT(slotSetCameraFreeMovement(bool)));
 reconnect(i, SIGNAL(signalSetCameraPos(const BoVector3Float&)),
		this, SLOT(slotSetCameraPos(const BoVector3Float&)));
 reconnect(i, SIGNAL(signalSetCameraLookAt(const BoVector3Float&)),
		this, SLOT(slotSetCameraLookAt(const BoVector3Float&)));
 reconnect(i, SIGNAL(signalSetCameraUp(const BoVector3Float&)),
		this, SLOT(slotSetCameraUp(const BoVector3Float&)));
 reconnect(i, SIGNAL(signalAddCameraPosPoint(const BoVector3Float&, float)),
		this, SLOT(slotAddCameraPosPoint(const BoVector3Float&, float)));
 reconnect(i, SIGNAL(signalAddCameraLookAtPoint(const BoVector3Float&, float)),
		this, SLOT(slotAddCameraLookAtPoint(const BoVector3Float&, float)));
 reconnect(i, SIGNAL(signalAddCameraUpPoint(const BoVector3Float&, float)),
		this, SLOT(slotAddCameraUpPoint(const BoVector3Float&, float)));
 reconnect(i, SIGNAL(signalSetCameraRotation(float)),
		this, SLOT(slotSetCameraRotation(float)));
 reconnect(i, SIGNAL(signalSetCameraRadius(float)),
		this, SLOT(slotSetCameraRadius(float)));
 reconnect(i, SIGNAL(signalSetCameraZ(float)),
		this, SLOT(slotSetCameraZ(float)));
 reconnect(i, SIGNAL(signalSetCameraMoveMode(int)),
		this, SLOT(slotSetCameraMoveMode(int)));
 reconnect(i, SIGNAL(signalSetCameraInterpolationMode(int)),
		this, SLOT(slotSetCameraInterpolationMode(int)));
 reconnect(i, SIGNAL(signalCommitCameraChanges(int)),
		this, SLOT(slotCommitCameraChanges(int)));
 reconnect(i, SIGNAL(signalSetAcceptUserInput(bool)),
		this, SLOT(slotSetAcceptUserInput(bool)));
}

void BosonBigDisplayScriptConnector::slotAddLight(int* id)
{
 BoLight* l = mDisplay->newLight();
 if (!l) {
	*id = -1;
 } else {
	*id = l->id();
 }
}

void BosonBigDisplayScriptConnector::slotRemoveLight(int id)
{
 mDisplay->removeLight(id);
}

void BosonBigDisplayScriptConnector::slotGetLightPos(int id, BoVector4Float* v)
{
 BoLight* l = mDisplay->light(id);
 if (!l) {
	boError() << k_funcinfo << "no light with id " << id << endl;
	*v = BoVector4Float();
	return;
 } else {
	*v = l->position();
 }
}

void BosonBigDisplayScriptConnector::slotGetLightAmbient(int id, BoVector4Float* v)
{
 BoLight* l = mDisplay->light(id);
 if (!l) {
	boError() << k_funcinfo << "no light with id " << id << endl;
	*v = BoVector4Float();
	return;
 } else {
	*v = l->ambient();
 }
}

void BosonBigDisplayScriptConnector::slotGetLightDiffuse(int id, BoVector4Float* v)
{
 BoLight* l = mDisplay->light(id);
 if (!l) {
	boError() << k_funcinfo << "no light with id " << id << endl;
	*v = BoVector4Float();
	return;
 } else {
	*v = l->diffuse();
 }
}

void BosonBigDisplayScriptConnector::slotGetLightSpecular(int id, BoVector4Float* v)
{
 BoLight* l = mDisplay->light(id);
 if (!l) {
	boError() << k_funcinfo << "no light with id " << id << endl;
	*v = BoVector4Float();
	return;
 } else {
	*v = l->specular();
 }
}

void BosonBigDisplayScriptConnector::slotGetLightAttenuation(int id, BoVector3Float* v)
{
 BoLight* l = mDisplay->light(id);
 if (!l) {
	boError() << k_funcinfo << "no light with id " << id << endl;
	*v = BoVector3Float();
	return;
 } else {
	*v = l->attenuation();
 }
}

void BosonBigDisplayScriptConnector::slotGetLightEnabled(int id, bool* e)
{
 BoLight* l = mDisplay->light(id);
 if (!l) {
	boError() << k_funcinfo << "no light with id " << id << endl;
	*e = false;
	return;
 } else {
	*e = l->isEnabled();
 }
}

void BosonBigDisplayScriptConnector::slotSetLightPos(int id, const BoVector4Float& v)
{
 BoLight* l = mDisplay->light(id);
 if (!l) {
	boError() << k_funcinfo << "no light with id " << id << endl;
	return;
 } else {
	l->setPosition(v);
 }
}

void BosonBigDisplayScriptConnector::slotSetLightAmbient(int id, const BoVector4Float& v)
{
 BoLight* l = mDisplay->light(id);
 if (!l) {
	boError() << k_funcinfo << "no light with id " << id << endl;
	return;
 } else {
	l->setAmbient(v);
 }
}

void BosonBigDisplayScriptConnector::slotSetLightDiffuse(int id, const BoVector4Float& v)
{
 BoLight* l = mDisplay->light(id);
 if (!l) {
	boError() << k_funcinfo << "no light with id " << id << endl;
	return;
 } else {
	l->setDiffuse(v);
 }
}

void BosonBigDisplayScriptConnector::slotSetLightSpecular(int id, const BoVector4Float& v)
{
 BoLight* l = mDisplay->light(id);
 if (!l) {
	boError() << k_funcinfo << "no light with id " << id << endl;
	return;
 } else {
	l->setSpecular(v);
 }
}

void BosonBigDisplayScriptConnector::slotSetLightAttenuation(int id, const BoVector3Float& v)
{
 BoLight* l = mDisplay->light(id);
 if (!l) {
	boError() << k_funcinfo << "no light with id " << id << endl;
	return;
 } else {
	l->setAttenuation(v);
 }
}

void BosonBigDisplayScriptConnector::slotSetLightEnabled(int id, bool e)
{
 BoLight* l = mDisplay->light(id);
 if (!l) {
	boError() << k_funcinfo << "no light with id " << id << endl;
	return;
 } else {
	l->setEnabled(e);
 }
}

void BosonBigDisplayScriptConnector::slotGetCameraPos(BoVector3Float* v)
{
 BO_CHECK_NULL_RET(mDisplay->camera());
 *v = mDisplay->camera()->cameraPos();
}

void BosonBigDisplayScriptConnector::slotGetCameraLookAt(BoVector3Float* v)
{
 BO_CHECK_NULL_RET(mDisplay->camera());
 *v = mDisplay->camera()->lookAt();
}

void BosonBigDisplayScriptConnector::slotGetCameraUp(BoVector3Float* v)
{
 BO_CHECK_NULL_RET(mDisplay->camera());
 *v = mDisplay->camera()->up();
}

void BosonBigDisplayScriptConnector::slotGetCameraRotation(float* v)
{
 BO_CHECK_NULL_RET(mDisplay->camera());
 *v = mDisplay->camera()->rotation();
}

void BosonBigDisplayScriptConnector::slotGetCameraRadius(float* v)
{
 BO_CHECK_NULL_RET(mDisplay->camera());
 *v = mDisplay->camera()->radius();
}

void BosonBigDisplayScriptConnector::slotGetCameraZ(float* v)
{
 BO_CHECK_NULL_RET(mDisplay->camera());
 *v = mDisplay->camera()->z();
}

void BosonBigDisplayScriptConnector::slotSetUseCameraLimits(bool u)
{
 BO_CHECK_NULL_RET(mDisplay->camera());
 mDisplay->camera()->setUseLimits(u);
}

void BosonBigDisplayScriptConnector::slotSetCameraFreeMovement(bool u)
{
 BO_CHECK_NULL_RET(mDisplay->camera());
 mDisplay->camera()->setFreeMovement(u);
}

void BosonBigDisplayScriptConnector::slotSetCameraPos(const BoVector3Float& v)
{
 BO_CHECK_NULL_RET(mDisplay->autoCamera());
 mDisplay->autoCamera()->setCameraPos(v);
}

void BosonBigDisplayScriptConnector::slotSetCameraLookAt(const BoVector3Float& v)
{
 BO_CHECK_NULL_RET(mDisplay->autoCamera());
 mDisplay->autoCamera()->setLookAt(v);
}

void BosonBigDisplayScriptConnector::slotSetCameraUp(const BoVector3Float& v)
{
 BO_CHECK_NULL_RET(mDisplay->autoCamera());
 mDisplay->autoCamera()->setUp(v);
}

void BosonBigDisplayScriptConnector::slotAddCameraPosPoint(const BoVector3Float& v, float time)
{
 BO_CHECK_NULL_RET(mDisplay->autoCamera());
 mDisplay->autoCamera()->addCameraPosPoint(v, time);
}

void BosonBigDisplayScriptConnector::slotAddCameraLookAtPoint(const BoVector3Float& v, float time)
{
 BO_CHECK_NULL_RET(mDisplay->autoCamera());
 mDisplay->autoCamera()->addLookAtPoint(v, time);
}

void BosonBigDisplayScriptConnector::slotAddCameraUpPoint(const BoVector3Float& v, float time)
{
 BO_CHECK_NULL_RET(mDisplay->autoCamera());
 mDisplay->autoCamera()->addUpPoint(v, time);
}

void BosonBigDisplayScriptConnector::slotSetCameraRotation(float v)
{
 BO_CHECK_NULL_RET(mDisplay->autoCamera());
 mDisplay->autoCamera()->setRotation(v);
}

void BosonBigDisplayScriptConnector::slotSetCameraRadius(float v)
{
 BO_CHECK_NULL_RET(mDisplay->autoCamera());
 mDisplay->autoCamera()->setRadius(v);
}

void BosonBigDisplayScriptConnector::slotSetCameraZ(float v)
{
 BO_CHECK_NULL_RET(mDisplay->autoCamera());
 mDisplay->autoCamera()->setZ(v);
}

void BosonBigDisplayScriptConnector::slotSetCameraMoveMode(int v)
{
 BO_CHECK_NULL_RET(mDisplay->autoCamera());
 mDisplay->autoCamera()->setMoveMode((BoAutoCamera::MoveMode)v);
}

void BosonBigDisplayScriptConnector::slotSetCameraInterpolationMode(int v)
{
 BO_CHECK_NULL_RET(mDisplay->autoCamera());
 mDisplay->autoCamera()->setInterpolationMode((BoAutoCamera::InterpolationMode)v);
}

void BosonBigDisplayScriptConnector::slotCommitCameraChanges(int ticks)
{
 BO_CHECK_NULL_RET(mDisplay->autoCamera());
 mDisplay->autoCamera()->commitChanges(ticks);
}

void BosonBigDisplayScriptConnector::slotSetAcceptUserInput(bool accept)
{
 QPtrList<KGameIO>* iolist = mDisplay->localPlayerIO()->ioList();
 QPtrListIterator<KGameIO> it(*iolist);
 while (it.current()) {
	(*it)->blockSignals(!accept);
	++it;
 }
}

void BosonBigDisplayBase::slotFog(int x, int y)
{
 BoGroundRenderer* r = BoGroundRendererManager::manager()->currentRenderer();
 if (r) {
	r->cellChanged(x, y);
 }
}

void BosonBigDisplayBase::slotUnfog(int x, int y)
{
 BoGroundRenderer* r = BoGroundRendererManager::manager()->currentRenderer();
 if (r) {
	r->cellChanged(x, y);
 }
}

static void updateEffects(BoVisibleEffects& v)
{
 static int id = boProfiling->requestEventId("updateEffects(): doDelayedUpdates");
 BosonProfiler prof(id);
 QPtrListIterator<BosonEffect> it(v.mAll);
 while (it.current()) {
	it.current()->doDelayedUpdates();
	++it;
 }
}

void BosonBigDisplayBase::resetGameMode()
{
 BO_CHECK_NULL_RET(ufoManager());

 slotChangeCursor(boConfig->cursorMode(), boConfig->cursorDir());

 d->mUfoGameWidget->setGameMode(true);

 // TODO: delete BoUfoActions
 BoUfoActionCollection* c = ufoManager()->actionCollection();
 ufoManager()->setActionCollection(0);
 delete c;
 c = 0;
 BoUfoActionCollection::initActionCollection(ufoManager());
 ufoManager()->actionCollection()->setAccelWidget(this);
}

void BosonBigDisplayBase::setGameMode(bool mode)
{
 BO_CHECK_NULL_RET(ufoManager());
 resetGameMode();
 d->mUfoGameWidget->setGameMode(mode);
 initUfoActions(mode);
}

void BosonBigDisplayBase::slotToggleSound()
{
 boAudio->setSound(!boAudio->sound());
 boConfig->setSound(boAudio->sound());
}

void BosonBigDisplayBase::slotToggleMusic()
{
 boAudio->setMusic(!boAudio->music());
 boConfig->setMusic(boAudio->music());
}

void BosonBigDisplayBase::slotToggleFullScreen()
{
 if (!d->mActionFullScreen) {
	return;
 }
 if (d->mActionFullScreen->isChecked()) {
	BoFullScreen::enterMode(-1);
 } else {
	BoFullScreen::leaveFullScreen();
 }
}

void BosonBigDisplayBase::slotBoDebugLogDialog()
{
 BoDebugLogDialog* dialog = new BoDebugLogDialog(0);
 connect(dialog, SIGNAL(finished()), dialog, SLOT(deleteLater()));
 dialog->slotUpdate();
 dialog->show();
#if 0
 BoUfoDebugLogDialog* dialog = new BoUfoDebugLogDialog(ufoManager());
 dialog->slotUpdate();
 dialog->show();
#endif
}

void BosonBigDisplayBase::slotProfiling()
{
 BosonProfilingDialog* dialog = new BosonProfilingDialog(0, false);
 connect(dialog, SIGNAL(finished()), dialog, SLOT(deleteLater()));
 dialog->show();
}

void BosonBigDisplayBase::slotSleep1s()
{
 sleep(1);
}

static QString findSaveFileName(const QString& prefix, const QString& suffix)
{
 QString file;
 for (int i = 0; i < 1000; i++) {
	file.sprintf("%s-%03d.%s", prefix.latin1(), i, suffix.latin1());
	if (!QFile::exists(file)) {
		return QFileInfo(file).absFilePath();
		return file;
	}
 }
 return QString::null;
}

void BosonBigDisplayBase::slotGrabProfiling()
{
 QString file = findSaveFileName("boprofiling", "boprof");
 if (file.isNull()) {
	boWarning() << k_funcinfo << "Can't find free filename???" << endl;
	return;
 }
 // TODO: chat message about file location!
 boDebug() << k_funcinfo << "Saving profiling to " << file << endl;
 bool ok = boProfiling->saveToFile(file);
 if (!ok) {
	boError() << k_funcinfo << "Error saving profiling to " << file << endl;
	boGame->slotAddChatSystemMessage(i18n("An error occured while saving profiling log to %1").arg(file));
 } else {
	boGame->slotAddChatSystemMessage(i18n("Profiling log saved to %1").arg(file));
 }
}

void BosonBigDisplayBase::slotGrabScreenshot()
{
 boDebug() << k_funcinfo << "Taking screenshot!" << endl;

 QPixmap shot = QPixmap::grabWindow(qApp->mainWidget()->winId());
 if (shot.isNull()) {
	boError() << k_funcinfo << "NULL image returned" << endl;
	return;
 }
 QString file = findSaveFileName("boson", "jpg");
 if (file.isNull()) {
	boWarning() << k_funcinfo << "Can't find free filename???" << endl;
	return;
 }
 boDebug() << k_funcinfo << "Saving screenshot to " << file << endl;
 bool ok = shot.save(file, "JPEG", 90);
 if (!ok) {
	boError() << k_funcinfo << "Error saving screenshot to " << file << endl;
	boGame->slotAddChatSystemMessage(i18n("An error occured while saving screenshot to %1").arg(file));
 } else {
	boGame->slotAddChatSystemMessage(i18n("Screenshot saved to %1").arg(file));
 }
}

void BosonBigDisplayBase::slotSetEnableColorMap(bool enable)
{
 boConfig->setEnableColormap(enable);
}

void BosonBigDisplayBase::slotSetDebugMapCoordinates(bool debug)
{
 boConfig->setDebugMapCoordinates(debug);
}

void BosonBigDisplayBase::slotSetDebugPFData(bool debug)
{
 boConfig->setDebugPFData(debug);
}

void BosonBigDisplayBase::slotSetDebugShowCellGrid(bool debug)
{
 boConfig->setDebugShowCellGrid(debug);
}

void BosonBigDisplayBase::slotSetDebugMatrices(bool debug)
{
 boConfig->setDebugOpenGLMatrices(debug);
}

void BosonBigDisplayBase::slotSetDebugItemWorks(bool debug)
{
 boConfig->setDebugItemWorkStatistics(debug);
}

void BosonBigDisplayBase::slotSetDebugCamera(bool debug)
{
 boConfig->setDebugOpenGLCamera(debug);
}

void BosonBigDisplayBase::slotSetDebugRenderCounts(bool debug)
{
 boConfig->setDebugRenderCounts(debug);
}

void BosonBigDisplayBase::slotSetDebugBoundingBoxes(bool debug)
{
 boConfig->setDebugBoundingBoxes(debug);
}

void BosonBigDisplayBase::slotSetDebugFPS(bool debug)
{
 boConfig->setDebugFPS(debug);
}

void BosonBigDisplayBase::slotSetDebugAdvanceCalls(bool debug)
{
 boConfig->setDebugAdvanceCalls(debug);
}

void BosonBigDisplayBase::slotSetDebugTextureMemory(bool debug)
{
 boConfig->setDebugTextureMemory(debug);
}

void BosonBigDisplayBase::slotSetDebugWireFrames(bool on)
{
 boConfig->setWireFrames(on);
}

void BosonBigDisplayBase::slotSetShowResources(bool show)
{
 boConfig->setShowResources(show);
}

void BosonBigDisplayBase::slotDumpGameLog()
{
 boGame->saveGameLogs("boson");
}

void BosonBigDisplayBase::slotSetDebugMode(int index)
{
 boConfig->setDebugMode((BosonConfig::DebugMode)index);
}

void BosonBigDisplayBase::slotShowGLStates()
{
 boDebug() << k_funcinfo << endl;
 BoGLStateWidget* w = new BoGLStateWidget(0, 0, WDestructiveClose);
 w->show();
}

void BosonBigDisplayBase::slotReloadMeshRenderer()
{
 bool unusable = false;
 bool r = BoMeshRendererManager::manager()->reloadPlugin(&unusable);
 if (r) {
	return;
 }
 boError() << "meshrenderer reloading failed" << endl;
 if (unusable) {
	KMessageBox::sorry(this, i18n("Reloading meshrenderer failed, library is now unusable. quitting."));
	exit(1);
 } else {
	KMessageBox::sorry(this, i18n("Reloading meshrenderer failed but library should still be usable"));
 }
}

void BosonBigDisplayBase::slotReloadGroundRenderer()
{
 bool unusable = false;
 bool r = BoGroundRendererManager::manager()->reloadPlugin(&unusable);
 if (r) {
	return;
 }
 boError() << "groundrenderer reloading failed" << endl;
 if (unusable) {
	KMessageBox::sorry(this, i18n("Reloading groundrenderer failed, library is now unusable. quitting."));
	exit(1);
 } else {
	KMessageBox::sorry(this, i18n("Reloading groundrenderer failed but library should still be usable"));
 }
}

void BosonBigDisplayBase::slotCrashBoson()
{
 ((QObject*)0)->name();
}

void BosonBigDisplayBase::slotSyncNetwork()
{
 boGame->syncNetwork();
}

void BosonBigDisplayBase::slotEditConditions()
{
 KDialogBase* dialog = new KDialogBase(KDialogBase::Plain, i18n("Conditions"),
		KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok, 0,
		"editconditions", true, true);
 QVBoxLayout* layout = new QVBoxLayout(dialog->plainPage());
 BoConditionWidget* widget = new BoConditionWidget(dialog->plainPage());
 layout->addWidget(widget);

 {
	QDomDocument doc;
	QDomElement root = doc.createElement("Conditions");
	doc.appendChild(root);

	if (!boGame->saveCanvasConditions(root)) {
		boError() << k_funcinfo << "unable to save canvas conditions from game" << endl;
		KMessageBox::information(this, i18n("Canvas conditions could not be imported to the widget"));
	} else {
		QValueStack<QDomElement> stack;
		stack.push(root);
		while (!stack.isEmpty()) {
			QDomElement e = stack.pop();
			for (QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling()) {
				QDomElement e2 = n.toElement();
				if (e2.isNull()) {
					continue;
				}
				stack.push(e2);
			}
			if (e.hasAttribute("PlayerId")) {
				bool ok = false;
				int index = e.attribute("PlayerId").toInt(&ok);
				if (!ok) {
					boError() << k_funcinfo << "PlayerId attribute not a valid number" << endl;
					continue;
				}
				KPlayer* p = boGame->playerList()->at(index);
				e.setAttribute("PlayerId", p->id());
			}
		}

		widget->loadConditions(root);
	}
;
 }

 int ret = dialog->exec();
 QString xml = widget->toString();
 delete widget;
 widget = 0;
 delete dialog;
 dialog = 0;
 if (ret == KDialogBase::Accepted) {
	QDomDocument doc;
	bool ret = doc.setContent(xml);
	QDomElement root = doc.documentElement();
	if (!ret || root.isNull()) {
		boError() << k_funcinfo << "invalid XML document created" << endl;
		KMessageBox::sorry(this, i18n("Oops - an invalid XML document was created. Internal error."));
		return;
	}
	boDebug() << k_funcinfo << "applying canvas conditions" << endl;
	boGame->loadCanvasConditions(root);
 }
}

void BosonBigDisplayBase::slotScroll(int dir)
{
 switch ((ScrollDirection)dir) {
	case ScrollUp:
		scrollBy(0, -boConfig->arrowKeyStep());
		break;
	case ScrollRight:
		scrollBy(boConfig->arrowKeyStep(), 0);
		break;
	case ScrollDown:
		scrollBy(0, boConfig->arrowKeyStep());
		break;
	case ScrollLeft:
		scrollBy(-boConfig->arrowKeyStep(), 0);
		break;
	default:
		return;
 }
}

void BosonBigDisplayBase::slotToggleCheating(bool on)
{
 if (!ufoManager() || !ufoManager()->actionCollection()) {
	return;
 }
 ufoManager()->actionCollection()->setActionEnabled("debug_unfog", on);
 ufoManager()->actionCollection()->setActionEnabled("debug_players", on);
}

void BosonBigDisplayBase::slotShowLight0Widget()
{
 delete d->mLightWidget;
 d->mLightWidget = new BoLightCameraWidget1(0, true);
 d->mLightWidget->show();
 d->mLightWidget->setLight(light(0), context());
}

void BosonBigDisplayBase::slotDebugMemory()
{
#ifdef BOSON_USE_BOMEMORY
 boDebug() << k_funcinfo << endl;
 BoMemoryDialog* dialog = new BoMemoryDialog(this);
 connect(dialog, SIGNAL(finished()), dialog, SLOT(deleteLater()));
 boDebug() << k_funcinfo << "update data" << endl;
 dialog->slotUpdate();
 dialog->show();
 boDebug() << k_funcinfo << "done" << endl;
#endif
}

void BosonBigDisplayBase::createDebugPlayersMenu()
{
 // note: NOT listed in the *ui.rc files! we create it dynamically when the player enters ; not using the xml framework
 if (d->mActionDebugPlayers) {
	boError() << k_funcinfo << "menu already created" << endl;
	return;
 }
 BO_CHECK_NULL_RET(boGame);
 BO_CHECK_NULL_RET(ufoManager());
 BO_CHECK_NULL_RET(ufoManager()->actionCollection());
 d->mActionDebugPlayers = new BoUfoActionMenu(i18n("Players"),
		ufoManager()->actionCollection(), "debug_players");

 QPtrList<KPlayer> players = *boGame->playerList();
 QPtrListIterator<KPlayer> it(players);
 for (; it.current(); ++it) {
	KPlayer* player = it.current();
	BoUfoActionMenu* menu = new BoUfoActionMenu(player->name(),
			ufoManager()->actionCollection(),
			QString("debug_players_%1").arg(player->name()));

	connect(menu, SIGNAL(signalActivated(int)),
			this, SLOT(slotDebugPlayer(int)));
	menu->insertItem(i18n("Kill Player"), ID_DEBUG_KILLPLAYER);
	menu->insertItem(i18n("Minerals += 10000"), ID_DEBUG_ADD_10000_MINERALS);
	menu->insertItem(i18n("Minerals += 1000"), ID_DEBUG_ADD_1000_MINERALS);
	menu->insertItem(i18n("Minerals -= 1000"), ID_DEBUG_SUB_1000_MINERALS);
	menu->insertItem(i18n("Oil += 10000"), ID_DEBUG_ADD_10000_OIL);
	menu->insertItem(i18n("Oil += 1000"), ID_DEBUG_ADD_1000_OIL);
	menu->insertItem(i18n("Oil -= 1000"), ID_DEBUG_SUB_1000_OIL);

	d->mActionDebugPlayers->insert(menu);
	d->mActionDebugPlayer2Player.insert(menu, player);
 }
}

void BosonBigDisplayBase::slotDebugPlayer(int index)
{
 BO_CHECK_NULL_RET(sender());
 if (!sender()->inherits("BoUfoActionMenu")) {
	boError() << k_funcinfo << "sender() is not a BoUfoActionMenu" << endl;
	return;
 }
 QPtrDictIterator<KPlayer> it(d->mActionDebugPlayer2Player);
 BoUfoActionMenu* menu = (BoUfoActionMenu*)sender();
 KPlayer* p = 0;
 while (it.current() && !p) {
	BoUfoActionMenu* m = (BoUfoActionMenu*)it.currentKey();
	if (m == menu) {
		p = it.current();
	}
	++it;
 }

 if (!p) {
	boError() << k_funcinfo << "player not found" << endl;
	return;
 }

 QByteArray b;
 QDataStream stream(b, IO_WriteOnly);
 stream << (Q_UINT32)p->id();
 switch (index) {
	case ID_DEBUG_KILLPLAYER:
		boGame->sendMessage(b, BosonMessage::IdKillPlayer);
		break;
	case ID_DEBUG_ADD_10000_MINERALS:
		stream << (Q_INT32)10000;
		boGame->sendMessage(b, BosonMessage::IdModifyMinerals);
		break;
	case ID_DEBUG_ADD_1000_MINERALS:
		stream << (Q_INT32)1000;
		boGame->sendMessage(b, BosonMessage::IdModifyMinerals);
		break;
	case ID_DEBUG_SUB_1000_MINERALS:
		stream << (Q_INT32)-1000;
		boGame->sendMessage(b, BosonMessage::IdModifyMinerals);
		break;
	case ID_DEBUG_ADD_1000_OIL:
		stream << (Q_INT32)1000;
		boGame->sendMessage(b, BosonMessage::IdModifyOil);
		break;
	case ID_DEBUG_ADD_10000_OIL:
		stream << (Q_INT32)10000;
		boGame->sendMessage(b, BosonMessage::IdModifyOil);
		break;
	case ID_DEBUG_SUB_1000_OIL:
		stream << (Q_INT32)-1000;
		boGame->sendMessage(b, BosonMessage::IdModifyOil);
		break;
	default:
		boError() << k_funcinfo << "unknown index " << index << endl;
		break;
 }
}


void BosonBigDisplayBase::slotChangeMaxProfilingEventEntries()
{
 bool ok = true;
 unsigned int max = boConfig->maxProfilingEventEntries();
 max = (unsigned int)QInputDialog::getInteger(i18n("Profiling event entries"),
		i18n("Maximal number of profiling entries per event"),
		(int)max, 0, 100000, 1, &ok, this);
 if (ok) {
	boConfig->setMaxProfilingEventEntries(max);
	boProfiling->setMaxEventEntries(boConfig->maxProfilingEventEntries());
 }
}

void BosonBigDisplayBase::slotChangeMaxProfilingAdvanceEntries()
{
 bool ok = true;
 unsigned int max = boConfig->maxProfilingAdvanceEntries();
 max = (unsigned int)QInputDialog::getInteger(i18n("Profiling advance entries"),
		i18n("Maximal number of profiled advance calls"),
		(int)max, 0, 100000, 1, &ok, this);
 if (ok) {
	boConfig->setMaxProfilingAdvanceEntries(max);
	boProfiling->setMaxAdvanceEntries(boConfig->maxProfilingAdvanceEntries());
 }
}

void BosonBigDisplayBase::slotChangeMaxProfilingRenderingEntries()
{
 bool ok = true;
 unsigned int max = boConfig->maxProfilingRenderingEntries();
 max = (unsigned int)QInputDialog::getInteger(i18n("Profiling rendering entries"),
		i18n("Maximal number of profiled frames"),
		(int)max, 0, 100000, 1, &ok, this);
 if (ok) {
	boConfig->setMaxProfilingRenderingEntries(max);
	boProfiling->setMaxRenderingEntries(boConfig->maxProfilingRenderingEntries());
 }
}

void BosonBigDisplayBase::slotToggleStatusbar()
{
 BO_CHECK_NULL_RET(d->mActionStatusbar);
 emit signalToggleStatusbar(d->mActionStatusbar->isChecked());
}

void BosonBigDisplayBase::slotDebugKGame()
{
 if (!boGame) {
	boError() << k_funcinfo << "NULL game" << endl;
	return;
 }
 KGameDebugDialog* dlg = new KGameDebugDialog(boGame, this, false);

 QVBox* b = dlg->addVBoxPage(i18n("Debug &Units"));
 KGameUnitDebug* units = new KGameUnitDebug(b);
 units->setBoson(boGame);

 b = dlg->addVBoxPage(i18n("Debug &Boson Players"));
 KGamePlayerDebug* player = new KGamePlayerDebug(b);
 player->setBoson(boGame);

 b = dlg->addVBoxPage(i18n("Debug &Advance messages"));
 KGameAdvanceMessagesDebug* messages = new KGameAdvanceMessagesDebug(b);
 messages->setBoson(boGame);

#if 0
 if (boGame->playField()) {
	BosonMap* map = boGame->playField()->map();
	if (!map) {
		boError() << k_funcinfo << "NULL map" << endl;
		return;
	}
	b = dlg->addVBoxPage(i18n("Debug &Cells"));

	// AB: this hardly does anything atm (04/04/23), but it takes a lot of
	// time and memory to be initialized on big maps (on list item per cell,
	// on a 500x500 map thats a lot)
	KGameCellDebug* cells = new KGameCellDebug(b);
	cells->setMap(map);
 }
#endif

 connect(dlg, SIGNAL(signalRequestIdName(int,bool,QString&)),
		this, SLOT(slotDebugRequestIdName(int,bool,QString&)));

 connect(dlg, SIGNAL(finished()), dlg, SLOT(deleteLater()));
 dlg->show();
}

void BosonBigDisplayBase::slotEditorSavePlayFieldAs()
{
 boDebug() << k_funcinfo << endl;
 QString startIn; // shall we provide this??
 QString fileName = KFileDialog::getSaveFileName(startIn, "*.bpf", this);
 if (fileName.isNull()) {
	return;
 }
 QFileInfo info(fileName);
 if (info.extension().isEmpty()) {
	fileName += ".bpf";
 }
 if (info.exists()) {
	int r = KMessageBox::warningYesNoCancel(this, i18n("The file \"%1\" already exists. Are you sure you want to overwrite it?").arg(info.fileName()), i18n("Overwrite File?"));
	if (r != KMessageBox::Yes) {
		return;
	}
 }
 bool ret = boGame->savePlayFieldToFile(fileName);
 if (!ret) {
	KMessageBox::sorry(this, i18n("An error occurred while saving the playfield. Unable to save."));
 }
}

void BosonBigDisplayBase::slotEditorChangeLocalPlayer(int index)
{
 Player* p = 0;
 p = (Player*)d->mEditorPlayers.at(index);
 if (p) {
	emit signalEditorChangeLocalPlayer((Player*)p);
	if (d->mActionEditorPlace->currentItem() >= 0) {
		slotEditorPlace(d->mActionEditorPlace->currentItem());
	}
 } else {
	boWarning() << k_funcinfo << "NULL player for index " << index << endl;
 }
}

void BosonBigDisplayBase::slotEditorDeleteSelectedUnits()
{
 displayInput()->deleteSelectedUnits();
}

void BosonBigDisplayBase::slotEditorEditMapDescription()
{
 BO_CHECK_NULL_RET(boGame);
 BO_CHECK_NULL_RET(boGame->playField());
 BO_CHECK_NULL_RET(boGame->playField()->description());


// TODO: non-modal might be fine. one could use that for translations (one
// dialog the original language, one the translated language)
 BPFDescriptionDialog* dialog = new BPFDescriptionDialog(this, true);
 dialog->setDescription(boGame->playField()->description());
 dialog->exec();

 delete dialog;
}

void BosonBigDisplayBase::slotEditorPlace(int index)
{
 boDebug() << k_funcinfo << "index: " << index << endl;
 switch (index) {
	case 0:
		d->mUfoGameWidget->slotShowPlaceFacilities(localPlayerIO());
		break;
	case 1:
		d->mUfoGameWidget->slotShowPlaceMobiles(localPlayerIO());
		break;
	case 2:
		d->mUfoGameWidget->slotShowPlaceGround();
		break;
	default:
		boError() << k_funcinfo << "Invalid index " << index << endl;
		return;
 }
}

void BosonBigDisplayBase::slotEditorImportHeightMap()
{
 BO_CHECK_NULL_RET(boGame);
 BO_CHECK_NULL_RET(boGame->playField());
 BO_CHECK_NULL_RET(boGame->playField()->map());
 boDebug() << k_funcinfo << endl;
 QString fileName = KFileDialog::getOpenFileName(QString::null, "*.png", this);
 if (fileName.isNull()) {
	return;
 }
 // first load the file as an image. We need it to be in greyscale png in boson,
 // and we can easily convert that image.
 QImage image(fileName);
 if (image.isNull()) {
	boError() << k_funcinfo << "unbable to load file " << fileName << endl;
	KMessageBox::sorry(this, i18n("Unable to load %1\nSeems not to be a valid image.").arg(fileName));
	return;
 }
 BosonMap* map = boGame->playField()->map();
 if ((unsigned int)image.width() != map->width() + 1 ||
		(unsigned int)image.height() != map->height() + 1) {
	KMessageBox::sorry(this, i18n("This image can't be used as height map for this map. The map is a %1x%2 map, meaning you need a %3x%4 image.\nThe image selected %5 was %6x%7").
			arg(map->width()).arg(map->height()).
			arg(map->width() + 1).arg(map->height() + 1).
			arg(fileName).
			arg(image.width()).arg(image.height()));
	return;
 }
 if (!image.isGrayscale()) {
	KMessageBox::sorry(this, i18n("%1 is not a greyscale image").arg(fileName));
	return;
 }
 boGame->playField()->importHeightMapImage(image);
 // TODO: update unit positions!
}

void BosonBigDisplayBase::slotEditorExportHeightMap()
{
 BO_CHECK_NULL_RET(boGame);
 BO_CHECK_NULL_RET(boGame->playField());
 BO_CHECK_NULL_RET(boGame->playField()->map());
 boDebug() << k_funcinfo << endl;
 QString fileName = KFileDialog::getSaveFileName(QString::null, "*.png", this);
 if (fileName.isNull()) {
	return;
 }
 QByteArray buffer = boGame->playField()->exportHeightMap();
 if (buffer.size() == 0) {
	boError() << k_funcinfo << "Could not export heightMap" << endl;
	KMessageBox::sorry(this, i18n("Unable to export heightMap"));
	return;
 }
 QImage image(buffer);
 if (image.isNull()) {
	boError() << k_funcinfo << "an invalid image has been generated" << endl;
	KMessageBox::sorry(this, i18n("An invalid heightmop image has been generated."));
 }
 if (!image.save(fileName, "PNG")) {
	boError() << k_funcinfo << "unable to save image to " << fileName << endl;
	KMessageBox::sorry(this, i18n("Unable to save image to %1.").arg(fileName));
	return;
 }
}

void BosonBigDisplayBase::slotEditorImportTexMap()
{
 boDebug() << k_funcinfo << endl;
 BoTexMapImportDialog* dialog = new BoTexMapImportDialog(this);
 connect(dialog, SIGNAL(finished()),
		dialog, SLOT(deleteLater()));

 BosonMap* map = boGame->playField()->map();
 dialog->setMap(map);

 dialog->show();
 dialog->slotSelectTexMapImage();
}

void BosonBigDisplayBase::slotEditorExportTexMap()
{
 boDebug() << k_funcinfo << endl;
 BO_CHECK_NULL_RET(boGame);
 BO_CHECK_NULL_RET(boGame->playField());
 BO_CHECK_NULL_RET(boGame->playField()->map());
 BO_CHECK_NULL_RET(boGame->playField()->map()->groundTheme());

 BosonMap* map = boGame->playField()->map();
 QStringList textures;
 for (unsigned int i = 0; i < map->groundTheme()->textureCount(); i++) {
	textures.append(map->groundTheme()->textureFileName(i));
 }

 QDialog* d = new QDialog(0, 0, true);
 QVBoxLayout* layout = new QVBoxLayout(d);
 QLabel* label = new QLabel(i18n("Select texture to export:"), d);
 QComboBox* combo = new QComboBox(d);
 QPushButton* button = new QPushButton(i18n("Ok"), d);
 connect(button, SIGNAL(clicked()), d, SLOT(accept()));
 combo->insertStringList(textures);
 layout->addWidget(label);
 layout->addWidget(combo);
 layout->addWidget(button);
 d->exec();
 unsigned int tex = (unsigned int)combo->currentItem();
 boDebug() << k_funcinfo << "tex: " << tex << endl;
 delete d;

 QString fileName = KFileDialog::getSaveFileName(QString::null, "*.png", this);
 if (fileName.isNull()) {
	return;
 }
 QByteArray buffer = boGame->playField()->exportTexMap(tex);
 if (buffer.size() == 0) {
	boError() << k_funcinfo << "Could not export texmap" << endl;
	KMessageBox::sorry(this, i18n("Unable to export texmap"));
	return;
 }
 QImage image(buffer);
 if (image.isNull()) {
	boError() << k_funcinfo << "an invalid image has been generated" << endl;
	KMessageBox::sorry(this, i18n("An invalid texmap image has been generated."));
 }
 if (!image.save(fileName, "PNG")) {
	boError() << k_funcinfo << "unable to save image to " << fileName << endl;
	KMessageBox::sorry(this, i18n("Unable to save image to %1.").arg(fileName));
	return;
 }
}

void BosonBigDisplayBase::slotEditorEditHeight(bool on)
{
 BO_CHECK_NULL_RET(displayInput());
 if (on) {
	BoSpecificAction action;
	action.setType(ActionChangeHeight);
	displayInput()->action(action);
 } else {
	displayInput()->unlockAction();
 }
}

void BosonBigDisplayBase::slotEditorEditPlayerMinerals()
{
 // atm disabled, as we must not include player.h in this file
 // these methods (e.g. all editor dependent methods) should get moved to a
 // different file anyway.
 // maybe all game/editor menu items will be handled by a dedicated KGameIO
 // class.
#if 0
 BO_CHECK_NULL_RET(localPlayer());
 bool ok = false;
 QString value = QString::number(localPlayer()->minerals());
 QIntValidator val(this);
 val.setBottom(0);
 val.setTop(1000000); // we need to set a top, because int is limited. this should be enough, i hope (otherwise feel free to increase)
 value = KLineEditDlg::getText(i18n("Minerals for player %1").arg(localPlayer()->name()), value, &ok, this, &val);
 if (!ok) {
	// cancel pressed
	return;
 }
 boDebug() << k_funcinfo << value << endl;
 unsigned long int v = value.toULong(&ok);
 if (!ok) {
	boWarning() << k_funcinfo << "value " << value << " not valid" << endl;
	return;
 }
 localPlayer()->setMinerals(v);
#endif
}

void BosonBigDisplayBase::slotEditorEditPlayerOil()
{
 // atm disabled, as we must not include player.h in this file
#if 0
 BO_CHECK_NULL_RET(localPlayer());
 bool ok = false;
 QString value = QString::number(localPlayer()->oil());
 QIntValidator val(this);
 val.setBottom(0);
 val.setTop(1000000); // we need to set a top, because int is limited. this should be enough, i hope (otherwise feel free to increase)
 value = KLineEditDlg::getText(i18n("Oil for player %1").arg(localPlayer()->name()), value, &ok, this, &val);
 if (!ok) {
	return;
 }
 boDebug() << k_funcinfo << value << endl;
 unsigned long int v = value.toULong(&ok);
 if (!ok) {
	boWarning() << k_funcinfo << "value " << value << " not valid" << endl;
	return;
 }
 localPlayer()->setOil(v);
#endif
}

void BosonBigDisplayBase::slotPreferences()
{
 if (!boGame) {
	boWarning() << k_funcinfo << "NULL boGame object" << endl;
	return;
 }
 OptionsDialog* dlg = new OptionsDialog(!boGame->gameMode(), this);
 dlg->setGame(boGame);
 dlg->setPlayer(localPlayerIO()->player());
 dlg->slotLoad();

 connect(dlg, SIGNAL(finished()), dlg, SLOT(deleteLater())); // seems not to be called if you quit with "cancel"!

 connect(dlg, SIGNAL(signalCursorChanged(int, const QString&)),
		this, SLOT(slotChangeCursor(int, const QString&)));
// connect(dlg, SIGNAL(signalCmdBackgroundChanged(const QString&)),
//		this, SLOT(slotCmdBackgroundChanged(const QString&)));
 connect(dlg, SIGNAL(signalOpenGLSettingsUpdated()),
		this, SLOT(slotUpdateOpenGLSettings()));
 connect(dlg, SIGNAL(signalApply()),
		this, SLOT(slotPreferencesApply()));
// connect(dlg, SIGNAL(signalFontChanged(const BoFontInfo&)),
//		displayManager(), SLOT(slotChangeFont(const BoFontInfo&)));

 dlg->show();
}

void BosonBigDisplayBase::slotPreferencesApply()
{
 // apply all options from boConfig to boson, that need to be applied. all
 // options that are stored in boConfig only don't need to be touched.
 // AB: cursor is still a special case and not handled here.
 // AB: FIXME: cmdbackground is not yet stored in boConfig! that option should
 // be managed here!
 boDebug() << k_funcinfo << endl;
 setUpdateInterval(boConfig->updateInterval());
 setToolTipCreator(boConfig->toolTipCreator());
 setToolTipUpdatePeriod(boConfig->toolTipUpdatePeriod());
}

void BosonBigDisplayBase::slotUpdateOpenGLSettings()
{
 updateOpenGLSettings();
}

void BosonBigDisplayBase::slotChangeCursor(int mode, const QString& cursorDir_)
{
 BO_CHECK_NULL_RET(boGame);
 boDebug() << k_funcinfo << endl;
 if (!boGame->gameMode()) {
	// editor mode
	mode = CursorKDE;
 }
 BosonCursor* b;
 switch (mode) {
	case CursorOpenGL:
		b = new BosonOpenGLCursor;
		break;
	case CursorKDE:
	default:
		b = new BosonKDECursor;
		mode = CursorKDE; // in case we had an unknown/invalid mode
		break;
 }

 QString cursorDir = cursorDir_;
 if (cursorDir.isNull()) {
	cursorDir = BosonCursor::defaultTheme();
 }

 bool ok = true;
 if (!b->insertMode(CursorMove, cursorDir, QString::fromLatin1("move"))) {
	ok = false;
 }
 if (!b->insertMode(CursorAttack, cursorDir, QString::fromLatin1("attack"))) {
	ok = false;
 }
 if (!b->insertMode(CursorDefault, cursorDir, QString::fromLatin1("default"))) {
	ok = false;
 }
 if (!ok) {
	boError() << k_funcinfo << "Could not load cursor mode " << mode << " from " << cursorDir << endl;
	delete b;
	if (!cursor() && mode != CursorKDE) { // loading *never* fails for CursorKDE. we check here anyway.
		// load fallback cursor
		slotChangeCursor(CursorKDE, QString::null);
		return;
	}
	// continue to use the old cursor
	return;
 }
 setCursor(b);

 boConfig->setCursorMode(mode);
 boConfig->setCursorDir(cursorDir);
}

void BosonBigDisplayBase::slotUnfogAll(Player* pl)
{
 // AB: disabled, because we cannot use player.h here.
 // see also comment in slotEditorEditPlayerMinerals()
#if 0
 if (!boGame) {
	boError() << k_funcinfo << "NULL game" << endl;
	return;
 }
 if (!boGame->playField()) {
	boError() << k_funcinfo << "NULL playField" << endl;
	return;
 }
 BosonMap* map = boGame->playField()->map();
 if (!map) {
	boError() << k_funcinfo << "NULL map" << endl;
	return;
 }
 QPtrList<KPlayer> list;
 if (!pl) {
	list = *boGame->playerList();
 } else {
	list.append(pl);
 }
 for (unsigned int i = 0; i < list.count(); i++) {
	Player* p = (Player*)list.at(i);
	for (unsigned int x = 0; x < map->width(); x++) {
		for (unsigned int y = 0; y < map->height(); y++) {
			p->unfog(x, y);
		}
	}
	boGame->slotAddChatSystemMessage(i18n("Debug"), i18n("Unfogged player %1 - %2").arg(p->id()).arg(p->name()));
 }
#endif
}

void BosonBigDisplayBase::slotDebugRequestIdName(int msgid, bool , QString& name)
{
 // we don't use i18n() for debug messages... not worth the work
 switch (msgid) {
	case BosonMessage::ChangeSpecies:
		name = "Change Species";
		break;
	case BosonMessage::ChangePlayField:
		name = "Change PlayField";
		break;
	case BosonMessage::ChangeTeamColor:
		name = "Change TeamColor";
		break;
	case BosonMessage::AdvanceN:
		name = "Advance";
		break;
	case BosonMessage::IdChat:
		name = "Chat Message";
		break;
	case BosonMessage::IdGameIsStarted:
		name = "Game is started";
		break;
	case BosonMessage::MoveMove:
		name = "PlayerInput: Move";
		break;
	case BosonMessage::MoveAttack:
		name = "PlayerInput: Attack";
		break;
	case BosonMessage::MoveBuild:
		name = "PlayerInput: Build";
		break;
	case BosonMessage::MoveProduce:
		name = "PlayerInput: Produce";
		break;
	case BosonMessage::MoveProduceStop:
		name = "PlayerInput: Produce Stop";
		break;
	case BosonMessage::MoveMine:
		name = "PlayerInput: Mine";
		break;
	case BosonMessage::UnitPropertyHandler:
	default:
		// a unit property was changed
		// all ids > UnitPropertyHandler will be a unit property. we
		// don't check further...
		break;
 }
// boDebug() << name << endl;
}

void BosonBigDisplayBase::slotSelectSelectionGroup(int number)
{
 if (number < 0 || number >= 10) {
	boError() << k_funcinfo << "Invalid group " << number << endl;
	return;
 }
 if (!d->mSelectionGroups[number]) {
	boError() << k_funcinfo << "NULL group " << number << endl;
	return;
 }
 selection()->copy(d->mSelectionGroups[number]);
}

void BosonBigDisplayBase::slotCreateSelectionGroup(int number)
{
 if (number < 0 || number >= 10) {
	boError() << k_funcinfo << "Invalid group " << number << endl;
	return;
 }
 if (!d->mSelectionGroups[number]) {
	boError() << k_funcinfo << "NULL group " << number << endl;
	return;
 }
 d->mSelectionGroups[number]->copy(selection());
}

void BosonBigDisplayBase::slotClearSelectionGroup(int number)
{
 if (number < 0 || number >= 10) {
	boError() << k_funcinfo << "Invalid group " << number << endl;
	return;
 }
 if (!d->mSelectionGroups[number]) {
	boError() << k_funcinfo << "NULL group " << number << endl;
	return;
 }
 d->mSelectionGroups[number]->clear();
}

void BosonBigDisplayBase::slotSetGrabMovie(bool grab)
{
 d->mGrabMovie = grab;
}

void BosonBigDisplayBase::grabMovieFrameAndSave()
{
 if (!d->mGrabMovie) {
	return;
 }
 QByteArray shot = grabMovieFrame();

 if (shot.size() == 0) {
	return;
 }

 // Save frame
 static int frame = -1;
 QString file;
 if (frame == -1) {
	int i;
	for (i = 0; i <= 10000; i++) {
		file.sprintf("%s-%04d.%s", "boson-movie", i, "jpg");
		if (!QFile::exists(file)) {
			frame = i;
			break;
		}
	}
	if (i == 10000) {
		boWarning() << k_funcinfo << "Can't find free filename???" << endl;
		frame = 50000;
	}
 }
 file.sprintf("%s-%04d.%s", "boson-movie", frame++, "jpg");
 file = QFileInfo(file).absFilePath();

 //boDebug() << k_funcinfo << "Saving movie frame to " << file << endl;
 bool ok = QPixmap(shot).save(file, "JPEG", 90);
 if (!ok) {
	boError() << k_funcinfo << "Error saving screenshot to " << file << endl;
	return;
 }
 boDebug() << k_funcinfo << "Movie frame saved to file " << file << endl;

#if 0
 static QValueList<QByteArray> allMovieFrames;
 allMovieFrames.append(shot);


 // TODO: use a shortcut for this. do not do this after a certain number of
 // frames, but when a key was pressed.
 if (allMovieFrames.count() == 10) {
	boDebug() << k_funcinfo << "generating " << allMovieFrames.count() << " frames" << endl;
	d->mActiveDisplay->generateMovieFrames(allMovieFrames, "./11/");
	allMovieFrames.clear();
 }
#endif
}

void BosonBigDisplayBase::slotAdvance(unsigned int, bool)
{
 // AB: note that in the big display no game logic must be done!
 // -> this slotAdvance() is here for certain optimizations on rendering, not
 //    for advancing the game itself
 setParticlesDirty(true);
 advanceCamera();
 advanceLineVisualization();
 grabMovieFrameAndSave();
}

void BosonBigDisplayBase::slotAction(const BoSpecificAction& action)
{
 if (!displayInput()) {
	return;
 }
 displayInput()->action(action);
}

