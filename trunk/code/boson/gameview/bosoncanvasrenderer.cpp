/*
    This file is part of the Boson game
    Copyright (C) 2001-2005 Andreas Beckermann (b_mann@gmx.de)
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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "bosoncanvasrenderer.h"

#include "../../bomemory/bodummymemory.h"
#include "../no_player.h"
#include "../defines.h"
#include "../bosoncanvas.h"
#include "../bosonmap.h"
#include "../cell.h"
#include "../boitemlist.h"
#include "../rtti.h"
#include "../unit.h"
#include "../unitproperties.h"
#include "../speciestheme.h"
#include "../bosonconfig.h"
#include "../boselection.h"
#include "../selectbox.h"
#include "../bosonprofiling.h"
#include "../bosoneffect.h"
#include "../bosoneffectparticle.h"
#include "bodebug.h"
#include "../items/bosonitemrenderer.h"
#include "../bosonmodel.h"
#include "../bo3dtools.h"
#include "../bosongroundtheme.h"
#include "../bogroundrenderer.h"
#include "../bogroundrenderermanager.h"
#include "../bomeshrenderermanager.h"
#include "../bolight.h"
#include "../bomaterial.h"
#include "../bowaterrenderer.h"
#include "../botexture.h"
#include "../bosondata.h"
#include "../boaction.h"
#include "../playerio.h"
#include "../bocamera.h"
#include "../boshader.h"

#include <qvaluevector.h>

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

static BosonModel* renderSingleItem(
		bool useLOD,
		BoCamera* camera,
		const BosonItem* item,
		BosonItemRenderer* itemRenderer,
		bool transparentMeshes,
		BosonModel* currentModel,
		unsigned int* _lod = 0)
{
 BO_CHECK_NULL_RET0(camera);
 BO_CHECK_NULL_RET0(item);
 BO_CHECK_NULL_RET0(itemRenderer);
 GLfloat x = item->centerX();
 GLfloat y = -item->centerY();
 GLfloat z = item->z();

 glPushMatrix();
 glTranslatef(x, y, z);
 glRotatef(-(item->rotation()), 0.0, 0.0, 1.0);
 glRotatef(item->xRotation(), 1.0, 0.0, 0.0);
 glRotatef(item->yRotation(), 0.0, 1.0, 0.0);

 unsigned int lod = 0;
 if (useLOD) {
	// TODO: we could compare squared distances here and get rid of sqrt()
	float dist = (camera->cameraPos() - BoVector3Float(x, y, z)).length();
	lod = itemRenderer->preferredLod(dist);
 }
 // If this item has different model then change current model
 if (itemRenderer->model() != currentModel) {
	currentModel = itemRenderer->model();
	currentModel->prepareRendering();
 }
 itemRenderer->renderItem(lod, transparentMeshes);
 glColor3ub(255, 255, 255);
 glPopMatrix();
 if (_lod) {
	*_lod = lod;
 }
 return currentModel;
}


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

class BoRenderItem
{
public:
	BoRenderItem() { modelId = 0; item = 0; }
	BoRenderItem(unsigned int _modelId, BosonItem* _item)
	{
		modelId = _modelId;
		item = _item;
	}

	BosonItem* item;
	unsigned int modelId;
};

class BosonCanvasRendererPrivate
{
public:
	BosonCanvasRendererPrivate()
	{
		mSelectBoxData = 0;

		mGameMatrices = 0;
		mCamera = 0;
		mLocalPlayerIO = 0;
	}
	QValueVector<BoRenderItem> mRenderItemList;
	SelectBoxData* mSelectBoxData;
	BoVisibleEffects mVisibleEffects;
	unsigned int mRenderedItems;
	unsigned int mRenderedCells;
	unsigned int mRenderedParticles;
	int mTextureBindsCells;
	int mTextureBindsItems;
	int mTextureBindsWater;
	int mTextureBindsParticles;

	const BoGLMatrices* mGameMatrices;
	BoGameCamera* mCamera;
	PlayerIO* mLocalPlayerIO;
};

BosonCanvasRenderer::BosonCanvasRenderer()
{
 d = new BosonCanvasRendererPrivate();
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
 delete d->mSelectBoxData;
 delete d;
}

void BosonCanvasRenderer::initGL()
{
 d->mSelectBoxData = new SelectBoxData();
}

void BosonCanvasRenderer::setGameGLMatrices(const BoGLMatrices* m)
{
 d->mGameMatrices = m;
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

const BoFrustum& BosonCanvasRenderer::viewFrustum() const
{
 return d->mGameMatrices->viewFrustum();
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
 PROFILE_METHOD;
 BO_CHECK_NULL_RET(localPlayerIO());
 BO_CHECK_NULL_RET(camera());
 BO_CHECK_NULL_RET(d->mSelectBoxData);
 BO_CHECK_NULL_RET(canvas);
 BO_CHECK_NULL_RET(d->mGameMatrices);

// boConfig->setBoolValue("UseLight", false);
 glDisable(GL_BLEND);
 glDisable(GL_LIGHTING);
 glDisable(GL_DITHER);
 glShadeModel(GL_SMOOTH);
 glColor3ub(255, 255, 255);

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

 renderGround(canvas->map());

 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "after ground rendering" << endl;
 }


 renderItems(canvas->allItems());

 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "after item rendering" << endl;
 }

 renderWater();

 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "after water rendering" << endl;
 }

 renderParticles(d->mVisibleEffects);

 glDisable(GL_DEPTH_TEST);
 glDisable(GL_LIGHTING);
 glDisable(GL_NORMALIZE);

 renderBulletTrailEffects(d->mVisibleEffects);

 glMatrixMode(GL_PROJECTION);
 glLoadIdentity();
 gluOrtho2D(0.0, (GLfloat)d->mGameMatrices->viewport()[2], 0.0, (GLfloat)d->mGameMatrices->viewport()[3]);
 glMatrixMode(GL_MODELVIEW);
 glLoadIdentity();
 glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

 renderFadeEffects(d->mVisibleEffects);

 BoShader::setFogEnabled(false);
}

void BosonCanvasRenderer::renderGround(const BosonMap* map)
{
 PROFILE_METHOD;
 BO_CHECK_NULL_RET(map);
 BoTextureManager::BoTextureBindCounter bindCounter(boTextureManager, &d->mTextureBindsCells);
 glEnable(GL_DEPTH_TEST);
 if (boConfig->boolValue("UseLight")) {
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
 BoVector3Float c1(item->x(), -item->y(), item->z());
 BoVector3Float c2(item->x() + item->width(), -(item->y() + item->height()), item->z() + item->depth());
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

void BosonCanvasRenderer::createRenderItemList(QValueVector<BoRenderItem>* renderItemList, const BoItemList* allItems)
{
 BO_CHECK_NULL_RET(localPlayerIO());

 renderItemList->clear();
 renderItemList->reserve(allItems->count());

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
		unsigned int modelid = 0;
		if (item->getModelForItem()) {
			modelid = item->getModelForItem()->id();
		}
		renderItemList->append(BoRenderItem(modelid, item));
	}
 }
}

void BosonCanvasRenderer::renderItems(const BoItemList* allCanvasItems)
{
 PROFILE_METHOD;
 BoTextureManager::BoTextureBindCounter bindCounter(boTextureManager, &d->mTextureBindsItems);
 BosonItemRenderer::startItemRendering();
 if (boConfig->boolValue("debug_wireframes")) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
 }
 glEnable(GL_DEPTH_TEST);
 if (boConfig->boolValue("UseLight")) {
	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);
 }

 createRenderItemList(&d->mRenderItemList, allCanvasItems); // AB: this is very fast. < 1.5ms on experimental5 for me


 unsigned int itemCount = d->mRenderItemList.count();

 {
	// Sort the list of to-be-rendered items by their models, so that items with
	//  same models are rendered after each other. This increases rendering
	//  performance (especially with vbos).
	// We use radix sort, which is (much) faster than quicksort with many items.
	// Radix sort
	unsigned int m = 1;
	unsigned int maxM = BosonModel::maxId();
	unsigned int k;
	BoRenderItem* helperlist = new BoRenderItem[itemCount];
	while (m <= maxM) {
		k = 0;
		for (unsigned int i = 0; i < itemCount; i++) {
			if((d->mRenderItemList[i].modelId & m) == 0) {
				helperlist[k++] = d->mRenderItemList[i];
			}
		}
		for(unsigned int i = 0; i < itemCount; i++) {
			if((d->mRenderItemList[i].modelId & m) == m) {
				helperlist[k++] = d->mRenderItemList[i];
			}
		}
		for(unsigned int i = 0; i < itemCount; i++) {
			d->mRenderItemList[i] = helperlist[i];
		}
		m *= 2;
	}
	delete[] helperlist;
 }

 bool useLOD = boConfig->boolValue("UseLOD");

 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "OpenGL error before rendering items" << endl;
 }

 // List of models with semi-transparent parts
 QValueVector<BoRenderItem> transparentModels;
 transparentModels.reserve((int)(itemCount * 0.25));

 // Model that is being used currently
 BosonModel* currentModel = 0;
 // Render all items
 for (unsigned int i = 0; i < itemCount; i++) {
	const BosonItem* item = d->mRenderItemList[i].item;
	BosonItemRenderer* itemRenderer = item->itemRenderer();
	if (!itemRenderer) {
		BO_NULL_ERROR(itemRenderer);
		continue;
	}

	// AB: note units are rendered in the *center* point of their
	// width/height.
	// but concerning z-position they are rendered from bottom to top!

	// Units will be tinted accordingly to how much health they have left
	if (RTTI::isUnit(item->rtti())) {
		if (((Unit*)item)->isDestroyed()) {
			glColor3f(0.4f, 0.4f, 0.4f);
		} else {
			float f = ((Unit*)item)->health() / (float)((Unit*)item)->maxHealth() * 0.3;
			glColor3f(0.7f + f, 0.7f + f, 0.7f + f);
		}
	} else {
		glColor3ub(255, 255, 255);
	}

	unsigned int lod;
	currentModel = renderSingleItem(useLOD, camera(), item, itemRenderer, false, currentModel, &lod);


	if (currentModel && currentModel->hasTransparentMeshes(lod)) {
		transparentModels.append(d->mRenderItemList[i]);
	}

	if (boConfig->boolValue("debug_boundingboxes")) {
		renderBoundingBox(item);
	}
 }

 // Render semi-transparent meshes of the models
 // TODO: sort the models by depth
 glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_CURRENT_BIT);
 glEnable(GL_DEPTH_TEST);
 //glDepthMask(GL_FALSE);
 glEnable(GL_BLEND);
 glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 glEnable(GL_ALPHA_TEST);
 glAlphaFunc(GL_GEQUAL, 0.2);
 glDisable(GL_CULL_FACE);
 //glDisable(GL_LIGHTING);
 for (unsigned int i = 0; i < transparentModels.count(); i++) {
	BosonItem* item = transparentModels[i].item;
	BosonItemRenderer* itemRenderer = item->itemRenderer();
	if (!itemRenderer) {
		BO_NULL_ERROR(itemRenderer);
		continue;
	}

	currentModel = renderSingleItem(useLOD, camera(), item, itemRenderer, true, currentModel);
 }
 glPopAttrib();

 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "OpenGL error before rendering selections" << endl;
 }

 BoItemList* selectedItems = new BoItemList(0, false);
 createSelectionsList(selectedItems, &d->mRenderItemList);
 renderSelections(selectedItems);
 delete selectedItems;
 selectedItems = 0;
 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "OpenGL error after rendering selections" << endl;
 }

 boTextureManager->invalidateCache();
 d->mRenderedItems += d->mRenderItemList.count();
 d->mRenderItemList.clear();

 BosonItemRenderer::stopItemRendering();
 if (boConfig->boolValue("debug_wireframes")) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
 }
}

void BosonCanvasRenderer::createSelectionsList(BoItemList* selectedItems, const QValueVector<BoRenderItem>* items)
{
 selectedItems->clear();
 unsigned int itemCount = items->count();
 for (unsigned int i = 0; i < itemCount; i++) {
	BosonItem* item = (*items)[i].item;
	if (item->isSelected()) {
		selectedItems->append(item);
	}
 }
}

void BosonCanvasRenderer::renderSelections(const BoItemList* selectedItems)
{
 PROFILE_METHOD;
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
	GLfloat depth = item->depth();
	glPushMatrix();
	glTranslatef(x, y, z);
	if (w != 1.0 || h != 1.0 || depth != 1.0) {
		glScalef(w, h, depth);
	}
	if (boConfig->boolValue("AlignSelectionBoxes")) {
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

void BosonCanvasRenderer::renderPathLines(const BosonCanvas* canvas, QValueList<QPoint>& path, bool isFlying, float _z)
{
 PROFILE_METHOD;
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
		z += canvas->heightAtPoint(x, y);
	}
	glVertex3f(x, y, z);
 }
 glEnd();

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
		if (viewFrustum().sphereInFrustum(s->position(), s->boundingSphereRadius())) {
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
 PROFILE_METHOD;
 BoTextureManager::BoTextureBindCounter bindCounter(boTextureManager, &d->mTextureBindsWater);
 boWaterRenderer->render();
}

void BosonCanvasRenderer::renderFog(BoVisibleEffects& visible)
{
 PROFILE_METHOD;
 // TODO: support multiple fog effects (ATM only 1st one is rendered)
 if (!visible.mFogEffects.isEmpty()) {
	BosonEffectFog* f = visible.mFogEffects.first();
	glEnable(GL_FOG);
	glFogfv(GL_FOG_COLOR, f->color().data());
	glFogf(GL_FOG_START, f->startDistance());
	glFogf(GL_FOG_END, f->endDistance());
	glFogi(GL_FOG_MODE, GL_LINEAR);
	BoShader::setFogEnabled(true);
 } else {
	// Disable fog
	glDisable(GL_FOG);
 }
}

void BosonCanvasRenderer::renderParticles(BoVisibleEffects& visible)
{
 PROFILE_METHOD;
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
 BoVector3Float camerapos = camera()->cameraPos();
 if (visible.mParticlesDirty) {
	float x, y, z;
	BoVector3Float dir;
	visible.mParticleList.clear();
	// Add all particles to the list
	QPtrListIterator<BosonEffectParticle> visibleIt(visible.mParticles);
	BosonEffectParticle* s = 0;
	for (; visibleIt.current(); ++visibleIt) {
		s = visibleIt.current();
		// If particleDist is non-zero, calculate vector for moving particles closer
		//  to camera
		if (s->particleDist() != 0.0f) {
			dir = camerapos - s->positionFloat();
			dir.scale(s->particleDist() / dir.length());
			s->setParticleDistVector(dir);
		}

		for (unsigned int i = 0; i < s->particleCount(); i++) {
			if (s->particle(i)->life > 0.0) {
				p = s->particle(i);
				// Calculate distance from camera. Note that for performance reasons,
				//  we don't calculate actual distance, but square of it.
				x = p->pos.x() - camerapos.x();
				y = p->pos.y() - camerapos.y();
				z = p->pos.z() - camerapos.z();
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
 const BoMatrix& modelview = d->mGameMatrices->modelviewMatrix();
 const BoVector3Float x(modelview[0], modelview[4], modelview[8]);
 const BoVector3Float y(modelview[1], modelview[5], modelview[9]);

 // Some cache variables
 int blendfunc = -1;
 BoTexture* texture = 0;
 bool betweenbeginend = false;  // If glBegin has been called, but glEnd() hasn't. Very hackish.
 BoVector3Float a, b, c, e;  // Vertex positions. e is used instead of d which clashes with private class

 // Precalculate relative particle corner positions
 const BoVector3Float upperleft(-0.5, 0.5, 0.0);
 const BoVector3Float upperright(0.5, 0.5, 0.0);
 const BoVector3Float lowerright(0.5, -0.5, 0.0);
 const BoVector3Float lowerleft(-0.5, -0.5, 0.0);
 const BoVector3Float alignedupperleft (-x + y);
 const BoVector3Float alignedupperright( x + y);
 const BoVector3Float alignedlowerright( x - y);
 const BoVector3Float alignedlowerleft (-x - y);

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

	if (p->system->particleDist() != 0.0f) {
		if (p->system->alignParticles()) {
			a = p->pos + (alignedupperleft  * p->size) + p->system->particleDistVector();
			b = p->pos + (alignedupperright * p->size) + p->system->particleDistVector();
			c = p->pos + (alignedlowerright * p->size + p->system->particleDistVector());
			e = p->pos + (alignedlowerleft  * p->size) + p->system->particleDistVector();
		} else {
			a = p->pos + (upperleft  * p->size) + p->system->particleDistVector();
			b = p->pos + (upperright * p->size) + p->system->particleDistVector();
			c = p->pos + (lowerright * p->size) + p->system->particleDistVector();
			e = p->pos + (lowerleft  * p->size) + p->system->particleDistVector();
		}
	} else {
		if (p->system->alignParticles()) {
			a = p->pos + (alignedupperleft  * p->size);
			b = p->pos + (alignedupperright * p->size);
			c = p->pos + (alignedlowerright * p->size);
			e = p->pos + (alignedlowerleft  * p->size);
		} else {
			a = p->pos + (upperleft  * p->size);
			b = p->pos + (upperright * p->size);
			c = p->pos + (lowerright * p->size);
			e = p->pos + (lowerleft  * p->size);
		}
	}

	glColor4fv(p->color.data());
	glTexCoord2f(0.0, 1.0);  glVertex3fv(a.data());
	glTexCoord2f(1.0, 1.0);  glVertex3fv(b.data());
	glTexCoord2f(1.0, 0.0);  glVertex3fv(c.data());
	glTexCoord2f(0.0, 0.0);  glVertex3fv(e.data());
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
 PROFILE_METHOD;
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
 PROFILE_METHOD;
 BO_CHECK_NULL_RET(d->mGameMatrices);
 if (!visible.mFadeEffects.isEmpty()) {
	BosonEffectFade* f;
//	glMatrixMode(GL_PROJECTION);
//	glPushMatrix();
	// Scale so that (0; 0) is bottom-left corner of the viewport and (1; 1) is
	//  top-right corner
//	glScalef(1 / (GLfloat)d->mViewport[2], 1 / (GLfloat)d->mViewport[3], 1);
	// FIXME!!!
	float xscale = (GLfloat)d->mGameMatrices->viewport()[2];
	float yscale = (GLfloat)d->mGameMatrices->viewport()[3];
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



static void updateEffects(BoVisibleEffects& v)
{
 BosonProfiler prof("updateEffects(): doDelayedUpdates");
 QPtrListIterator<BosonEffect> it(v.mAll);
 while (it.current()) {
	it.current()->doDelayedUpdates();
	++it;
 }
}

