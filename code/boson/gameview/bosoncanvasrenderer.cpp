/*
    This file is part of the Boson game
    Copyright (C) 2001-2006 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2001-2006 Rivo Laks (rivolaks@hot.ee)

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
#include "bosoncanvasrenderer.moc"

#include "../../bomemory/bodummymemory.h"
#include "../no_player.h"
#include "../defines.h"
#include "../gameengine/boson.h"
#include "../gameengine/bosoncanvas.h"
#include "../gameengine/bosonmap.h"
#include "../gameengine/cell.h"
#include "../gameengine/boitemlist.h"
#include "../gameengine/rtti.h"
#include "../gameengine/unit.h"
#include "../gameengine/unitproperties.h"
#include "../gameengine/unitplugins/radarplugin.h"
#include "../gameengine/unitplugins/radarjammerplugin.h"
#include "../gameengine/speciestheme.h"
#include "../speciesdata.h"
#include "../bosonconfig.h"
#include "boselection.h"
#include "../selectbox.h"
#include "../bosonprofiling.h"
#include "bosoneffect.h"
#include "bosoneffectparticle.h"
#include "bodebug.h"
#include "bosonitemrenderer.h"
#include "../modelrendering/bosonmodel.h"
#include "../bo3dtools.h"
#include "../gameengine/bosongroundtheme.h"
#include "../bogroundrenderer.h"
#include "../bogroundrenderermanager.h"
#include "../modelrendering/bomeshrenderermanager.h"
#include "../bolight.h"
#include "../bomaterial.h"
#include "../bowaterrenderer.h"
#include "../botexture.h"
#include "../bosondata.h"
#include "../boaction.h"
#include "../gameengine/playerio.h"
#include "../bocamera.h"
#include "../boshader.h"
#include "../bosonviewdata.h"
#include "../borendertarget.h"
#include "bosonlocalplayerinput.h"

#include <qvaluevector.h>
#include <qdatetime.h>

#include <kglobal.h>
#include <kstandarddirs.h>

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
		unsigned int* _lod = 0,
		RenderFlags flags = Default)
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
 itemRenderer->renderItem(lod, transparentMeshes, flags);
 glPopMatrix();
 if (_lod) {
	*_lod = lod;
 }
 return currentModel;
}



// AB: a derived class may implement isDone() e.g. so that it does not depend on
// constant time, but on a certain animation to be finished
class BoVisualFeedback
{
public:
	enum FeedbackRTTI {
		FeedbackItemTint = 1,
		FeedbackGroundDot
	};
public:
	BoVisualFeedback(int durationMS = 500)
	{
		init();
		mDurationMS = durationMS;
	}
	BoVisualFeedback(BoVector3Float pos, int durationMS = 500)
	{
		init();
		mHavePos = true;
		mPos = pos;
		mDurationMS = durationMS;
	}
	BoVisualFeedback(const BosonItem* item, int durationMS = 500)
	{
		init();
		mItem = item;
		mDurationMS = durationMS;
	}
	virtual ~BoVisualFeedback()
	{
	}

	virtual int rtti() const = 0;

	virtual bool isDone() const
	{
		if (mDurationMS < 0) {
			return false;
		}
		if (mStarted.elapsed() < mDurationMS) {
			return false;
		}
		return true;
	}

	const BosonItem* item() const
	{
		return mItem;
	}

	bool havePosition() const
	{
		return mHavePos;
	}
	const BoVector3Float& position() const
	{
		return mPos;
	}

	virtual void paintGL() = 0;

private:
	void init()
	{
		mStarted.start();
		mDurationMS = -1;
		mHavePos = false;
		mItem= 0;
	}

private:
	QTime mStarted;
	bool mHavePos;
	BoVector3Float mPos;
	const BosonItem* mItem;
	int mDurationMS;
};

class BoVisualFeedbackItemTint : public BoVisualFeedback
{
public:
	BoVisualFeedbackItemTint(const BosonItem* item, int durationMS = 500, const QColor& color = Qt::red)
		: BoVisualFeedback(item, durationMS)
	{
		mColor = color;
	}

	virtual int rtti() const
	{
		return FeedbackItemTint;
	}

	const QColor& color() const
	{
		return mColor;
	}

	virtual void paintGL()
	{
	}

private:
	QColor mColor;
};

// a "dot" on the map
// this is not actually supposed to be a "dot". it should be some kind of
// marker, indicating that the user clicked here.
class BoVisualFeedbackGroundDot : public BoVisualFeedback
{
public:
	BoVisualFeedbackGroundDot(const BoVector3Float& pos, int durationMS = 500, const QColor& color = Qt::red)
		: BoVisualFeedback(pos, durationMS)
	{
		mColor = color;
	}

	virtual int rtti() const
	{
		return FeedbackGroundDot;
	}

	const QColor& color() const
	{
		return mColor;
	}

	virtual void paintGL()
	{
		glPushAttrib(GL_ENABLE_BIT);
		glDisable(GL_BLEND);
		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		boTextureManager->disableTexturing();
		glColor3ub(mColor.red(), mColor.green(), mColor.blue());
		float x = position().x();
		float y = position().y();
		float z = position().z();
		glBegin(GL_LINES);
			glVertex3f(x - 0.25f, y - 0.25f, z);
			glVertex3f(x, y, z);

			glVertex3f(x - 0.25f, y + 0.25f, z);
			glVertex3f(x, y, z);

			glVertex3f(x + 0.25f, y - 0.25f, z);
			glVertex3f(x, y, z);

			glVertex3f(x + 0.25f, y + 0.25f, z);
			glVertex3f(x, y, z);
		glEnd();
		glPopAttrib();
	}

private:
	QColor mColor;
};


class BoVisualFeedbackContainer
{
public:
	BoVisualFeedbackContainer()
	{
	}
	~BoVisualFeedbackContainer()
	{
		while (!mFeedbacks.isEmpty()) {
			BoVisualFeedback* f = mFeedbacks.take(0);
			delete f;
		}
	}

	void addFeedback(BoVisualFeedback* f)
	{
		mFeedbacks.append(f);
		if (f->rtti() == BoVisualFeedback::FeedbackItemTint) {
			mFeedbackItemTint.append(f);
		}
	}
	void checkAlive()
	{
		QPtrList<BoVisualFeedback> dead;
		for (QPtrListIterator<BoVisualFeedback> it(mFeedbacks); it.current(); ++it) {
			if (it.current()->isDone()) {
				dead.append(it.current());
			}
		}
		while (!dead.isEmpty()) {
			BoVisualFeedback* f = dead.take(0);
			removeFeedback(f, true);
		}
	}
	void removeFeedback(BoVisualFeedback* f, bool del)
	{
		mFeedbacks.removeRef(f);
		mFeedbackItemTint.removeRef(f);
		if (del) {
			delete f;
		}
	}

	void paintGL()
	{
		for (QPtrListIterator<BoVisualFeedback> it(mFeedbacks); it.current(); ++it) {
			it.current()->paintGL();
		}

		checkAlive();
	}

	bool tintItem(const BosonItem* item) const
	{
		for (QPtrListIterator<BoVisualFeedback> it(mFeedbackItemTint); it.current(); ++it) {
			BoVisualFeedbackItemTint* t = (BoVisualFeedbackItemTint*)it.current();
			if (t->item() == item) {
				return true;
			}
		}
		return false;
	}
	QColor tintColor(const BosonItem* item) const
	{
		for (QPtrListIterator<BoVisualFeedback> it(mFeedbackItemTint); it.current(); ++it) {
			BoVisualFeedbackItemTint* t = (BoVisualFeedbackItemTint*)it.current();
			if (t->item() == item) {
				return t->color();
			}
		}
		return Qt::red;
	}

private:
	QPtrList<BoVisualFeedback> mFeedbacks;
	QPtrList<BoVisualFeedback> mFeedbackItemTint;
};




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
	BoRenderItem() { modelId = 0; item = 0; itemRenderer = 0; tintColor = QColor(255, 255, 255); }
	BoRenderItem(unsigned int _modelId, BosonItem* _item, BosonItemRenderer* _itemRenderer, const QColor& _tintColor)
	{
		modelId = _modelId;
		item = _item;
		itemRenderer = _itemRenderer;
		tintColor = _tintColor;
	}

	BosonItem* item;
	unsigned int modelId;
	BosonItemRenderer* itemRenderer;
	QColor tintColor;
};

/**
 * Helper class which stores rendertarget and texture(s) where the scene
 *  can be rendered onto.
 *
 * @internal
 **/
class BoSceneRenderTarget
{
public:
	BoSceneRenderTarget(int width, int height, bool needdepth = false)
	{
		// Create color texture
		texture = new BoTexture(0, width, height,
				BoTexture::FilterLinearMipmapLinear | BoTexture::FormatRGBA | BoTexture::DontCompress |
				BoTexture::ClampToEdge | BoTexture::EnableNPOT);

		// Create depth texture (if necessary) and rendertarget
		if (needdepth) {
			depthTexture = new BoTexture(0, width, height,
					BoTexture::FilterLinear | BoTexture::FormatDepth | BoTexture::DontCompress |
					BoTexture::ClampToEdge | BoTexture::EnableNPOT);
			renderTarget = new BoRenderTarget(width, height,
					BoRenderTarget::RGBA | BoRenderTarget::Depth, texture, depthTexture);
		} else {
			depthTexture = 0;
			renderTarget = new BoRenderTarget(width, height, BoRenderTarget::RGBA, texture);
		}

		// Init variables
		used = false;
	}

	~BoSceneRenderTarget()
	{
		delete renderTarget;
		delete texture;
		delete depthTexture;
	}

	void enable() { renderTarget->enable(); }
	void disable() { renderTarget->disable(); }

	int width() const { return renderTarget->width(); }
	int height() const { return renderTarget->height(); }
	bool hasDepth() const { return (depthTexture != 0); }
	bool valid() const { return renderTarget->valid(); }


	BoRenderTarget* renderTarget;
	BoTexture* texture;
	BoTexture* depthTexture;

	bool used;
};

/**
 * Helper class which can store multiple @ref BoSceneRenderTarget objects and
 *  return @ref BoSceneRenderTarget with requested size (creating new
 *  rendertarget if necessary).
 *
 * @internal
 **/
class BoSceneRenderTargetCache
{
public:
	BoSceneRenderTargetCache()
	{
	}

	~BoSceneRenderTargetCache()
	{
		deleteAllRenderTargets();
	}

	BoSceneRenderTarget* getRenderTarget(int width, int height, bool needdepth = false)
	{
		BoSceneRenderTarget* target = 0;
		QValueList<BoSceneRenderTarget*>::Iterator it;
		for (it = mRenderTargets.begin(); it != mRenderTargets.end(); it++) {
			if (!(*it)->used && (*it)->width() == width &&
					(*it)->height() == height && (*it)->hasDepth() == needdepth) {
				target = *it;
				break;
			}
		}
		if (!target) {
			// No match. Create new rendertarget
			target = new BoSceneRenderTarget(width, height, needdepth);
			mRenderTargets.append(target);
		}
		// Set used flag to true
		target->used = true;
		return target;
	}

	void finishedUsingRenderTarget(BoSceneRenderTarget* target)
	{
		target->used = false;
	}

	void deleteAllRenderTargets()
	{
		while (!mRenderTargets.isEmpty()) {
			BoSceneRenderTarget* t = mRenderTargets.first();
			mRenderTargets.pop_front();
			delete t;
		}
	}


private:
	QValueList<BoSceneRenderTarget*> mRenderTargets;
};

class BosonCanvasRendererPrivate
{
public:
	BosonCanvasRendererPrivate()
	{
		mCanvas = 0;
		mSelectBoxData = 0;

		mVisualFeedbacks = 0;

		mGameMatrices = 0;
		mCamera = 0;
		mLocalPlayerIO = 0;

		mShadowTarget = 0;
		mShadowTexture = 0;
		mShadowColorTexture = 0;
		mUnitShader = 0;

		mMainSceneRenderTarget = 0;
		mSceneRenderTargetCache = 0;

		mUnitIconLand = 0;
		mUnitIconAir = 0;
		mUnitIconFacility = 0;
		mJammingIcon = 0;
		mRadarIcon = 0;
	}
	const BosonCanvas* mCanvas;
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

	BoVisualFeedbackContainer* mVisualFeedbacks;

	const BoGLMatrices* mGameMatrices;
	BoGameCamera* mCamera;
	PlayerIO* mLocalPlayerIO;

	float mMinItemDist;
	float mMaxItemDist;

	BoRenderTarget* mShadowTarget;
	BoTexture* mShadowTexture;
	BoTexture* mShadowColorTexture;
	BoMatrix mShadowProjectionMatrix;
	BoMatrix mShadowViewMatrix;
	BoShader* mUnitShader;

	BoSceneRenderTarget* mMainSceneRenderTarget;
	BoSceneRenderTargetCache* mSceneRenderTargetCache;

	QValueList<Unit*> mRadarContactsList;
	QValueList<Unit*> mIconicUnits;
	BoTexture* mUnitIconLand;
	BoTexture* mUnitIconAir;
	BoTexture* mUnitIconFacility;
	BoTexture* mJammingIcon;
	BoTexture* mRadarIcon;
};

BosonCanvasRenderer::BosonCanvasRenderer()
	: QObject(0, "canvasrenderer")
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
 d->mVisualFeedbacks = new BoVisualFeedbackContainer();
 d->mSceneRenderTargetCache = new BoSceneRenderTargetCache();
}

BosonCanvasRenderer::~BosonCanvasRenderer()
{
 delete d->mSelectBoxData;
 delete d->mVisualFeedbacks;
 delete d->mSceneRenderTargetCache;
 delete d->mUnitShader;
 delete d->mShadowTarget;
 delete d->mShadowTexture;
 delete d->mShadowColorTexture;
 delete d->mUnitIconLand;
 delete d->mUnitIconAir;
 delete d->mUnitIconFacility;
 delete d->mJammingIcon;
 delete d->mRadarIcon;
 delete d;
}

void BosonCanvasRenderer::initGL()
{
 d->mSelectBoxData = new SelectBoxData();

 QStringList extensions = boglGetOpenGLExtensions();
 if (extensions.contains("GL_ARB_shader_objects") && extensions.contains("GL_ARB_fragment_shader") &&
		extensions.contains("GL_ARB_shadow")) {
	// Load unit shader
	d->mUnitShader = new BoShader("unit");
 }

 if (!extensions.contains("GL_EXT_framebuffer_object")) {
	// Effects requiring RTT (render-to-texture) will be disabled
	boDebug() << k_funcinfo << "GL_EXT_framebuffer_object not found. RTT is disabled" << endl;
 }

 QString path = KGlobal::dirs()->findResourceDir("data", "boson/themes/ui/standard/uniticon-land.png");
 if (path.isEmpty()) {
	 boError() << k_funcinfo << "Couldn't find path for unit icons!" << endl;
 }
 path += "boson/themes/ui/standard/";
 d->mUnitIconLand = new BoTexture(path + "uniticon-land.png");
 d->mUnitIconAir = new BoTexture(path + "uniticon-air.png");
 d->mUnitIconFacility = new BoTexture(path + "uniticon-facility.png");
 d->mJammingIcon = new BoTexture(path + "jamming.png");
 d->mRadarIcon = new BoTexture(path + "radar.png");
}

void BosonCanvasRenderer::slotWidgetResized()
{
 // Whenever the widget's size changes, we need to clear rendertarget cache
 //  because sizes of used rendertargets depend on widget size and thus the
 //  cache is unlikely to contain usable rendertargets now.
 d->mSceneRenderTargetCache->deleteAllRenderTargets();
}

void BosonCanvasRenderer::setCanvas(const BosonCanvas* canvas)
{
 if (d->mCanvas) {
	disconnect(d->mCanvas, 0, this, 0);
 }
 d->mCanvas = canvas;
 if (d->mCanvas) {
	connect(d->mCanvas, SIGNAL(signalRemovedItem(BosonItem*)),
			this, SLOT(slotItemRemoved(BosonItem*)));
 }
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
 if (localPlayerIO()) {
	BosonLocalPlayerInput* i = (BosonLocalPlayerInput*)localPlayerIO()->findRttiIO(BosonLocalPlayerInput::LocalPlayerInputRTTI);
	if (i) {
		disconnect(i, 0, this, 0);
	}
 }

 d->mLocalPlayerIO = io;

 if (!localPlayerIO()) {
	return;
 }
 BosonLocalPlayerInput* i = (BosonLocalPlayerInput*)localPlayerIO()->findRttiIO(BosonLocalPlayerInput::LocalPlayerInputRTTI);
 if (i) {
	connect(i, SIGNAL(signalAttackUnit(const QPtrList<Unit>&, const Unit*)),
			this, SLOT(slotAddFeedbackAttack(const QPtrList<Unit>&, const Unit*)));
	connect(i, SIGNAL(signalMoveUnitsTo(const QPtrList<Unit>&, const BoVector2Fixed&, bool)),
			this, SLOT(slotAddFeedbackMoveTo(const QPtrList<Unit>&, const BoVector2Fixed&, bool)));
 } else {
	boError() << k_funcinfo << "local player does not have any BosonLocalPlayerInput!" << endl;
 }
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
 d->mSceneRenderTargetCache->deleteAllRenderTargets();
}

void BosonCanvasRenderer::paintGL(const QPtrList<BosonItemContainer>& allItems, const QPtrList<BosonEffect>& effects)
{
 PROFILE_METHOD;
 BO_CHECK_NULL_RET(localPlayerIO());
 BO_CHECK_NULL_RET(camera());
 BO_CHECK_NULL_RET(d->mSelectBoxData);
 BO_CHECK_NULL_RET(d->mCanvas);
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

 // Find out the visible effects and update them
 createVisibleEffectsList(&d->mVisibleEffects, effects, d->mCanvas->mapWidth(), d->mCanvas->mapHeight());
 updateEffects(d->mVisibleEffects);


 // Create list of visible items
 createRenderItemList(&d->mRenderItemList, &d->mRadarContactsList, allItems); // AB: this is very fast. < 1.5ms on experimental5 for me

 // Create list of visible terrain chunks and calculate their min/max distance
 // Not necessary, it's done in BosonGameView::cameraChanged()
 //BoGroundRendererManager::manager()->currentRenderer()->generateCellList(d->mCanvas->map());


 bool useUnitShadows = d->mUnitShader && boConfig->boolValue("UseUnitShaders");
 bool useGroundShadows = boConfig->boolValue("UseGroundShaders");
 if (useUnitShadows || useGroundShadows) {
	// Render the shadowmap
	renderShadowMap(d->mCanvas);
 }

 bool renderToTexture = mustRenderToTexture(d->mVisibleEffects);
 if (renderToTexture) {
	renderToTexture = startRenderingToTexture();
 }
 if (renderToTexture) {
	// Push near and far planes as close to each other as possible to make
	//  maximum use of the depth buffer (and texture).
	// TODO: this has some problems:
	// * groundrenderer statistics seem to be invalid
	// * code that maps viewport coords to world coords still uses old projection
	//  matrix and thus returns invalid z coordinate.
	BoGroundRendererStatistics* stats = BoGroundRendererManager::manager()->currentRenderer()->statistics();
	// Distances of new near/far plane _from current near plane_
	float neardist = QMIN(stats->minDistance(), d->mMinItemDist);
	float fardist = QMAX(stats->maxDistance(), d->mMaxItemDist);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(d->mGameMatrices->fovY(), d->mGameMatrices->aspect(), BO_GL_NEAR_PLANE + neardist, BO_GL_NEAR_PLANE + fardist);
	glMatrixMode(GL_MODELVIEW);
 }

 // Activate fog effect (if any)
 renderFog(d->mVisibleEffects);

 if (useGroundShadows) {
	activateShadowMap();
 }


 if (boConfig->boolValue("debug_render_ground")) {
	renderGround(d->mCanvas->map());
 }

 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "after ground rendering" << endl;
 }

 if(useGroundShadows && !useUnitShadows) {
	deactivateShadowMap();
 } else if(useUnitShadows && !useGroundShadows) {
	activateShadowMap();
 }


 if (boConfig->boolValue("debug_render_items")) {
   if (useUnitShadows) {
		d->mUnitShader->bind();
		renderItems();
		d->mUnitShader->unbind();
	} else {
		renderItems();
	}
 }

 if (useUnitShadows) {
	deactivateShadowMap();
 }

 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "after item rendering" << endl;
 }

 if (boConfig->boolValue("debug_render_water")) {
	renderWater();
 }

 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "after water rendering" << endl;
 }

 if (boConfig->boolValue("debug_render_particles")) {
	renderParticles(d->mVisibleEffects);
 }

 glDisable(GL_LIGHTING);
 glDisable(GL_NORMALIZE);

 renderBulletTrailEffects(d->mVisibleEffects);

 renderUnitIcons();

 d->mVisualFeedbacks->paintGL();

 if (renderToTexture) {
	stopRenderingToTexture();
 }

 glDisable(GL_LIGHTING);
 glDisable(GL_NORMALIZE);
 glDisable(GL_DEPTH_TEST);

 glMatrixMode(GL_PROJECTION);
 glLoadIdentity();
 gluOrtho2D(0.0, (GLfloat)d->mGameMatrices->viewport()[2], 0.0, (GLfloat)d->mGameMatrices->viewport()[3]);
 glMatrixMode(GL_MODELVIEW);
 glLoadIdentity();
 glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

 if (renderToTexture) {
	// Render scene texture
	BosonProfiler prof("RenderToTexture: RenderSceneTexture");
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	boTextureManager->activateTextureUnit(0);
	glColor3f(1, 1, 1);
	d->mMainSceneRenderTarget->texture->bind();
	float maxTextureXCoord = d->mMainSceneRenderTarget->texture->width() / (float)d->mGameMatrices->viewport()[2];
	float maxTextureYCoord = d->mMainSceneRenderTarget->texture->height() / (float)d->mGameMatrices->viewport()[3];
	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0);
		glVertex2f(0.0, 0.0);
		glTexCoord2f(maxTextureXCoord, 0.0);
		glVertex2f((GLfloat)d->mGameMatrices->viewport()[2], 0.0);
		glTexCoord2f(maxTextureXCoord, maxTextureYCoord);
		glVertex2f((GLfloat)d->mGameMatrices->viewport()[2], (GLfloat)d->mGameMatrices->viewport()[3]);
		glTexCoord2f(0.0,maxTextureYCoord);
		glVertex2f(0.0, (GLfloat)d->mGameMatrices->viewport()[3]);
	glEnd();
 }

 renderFadeEffects(d->mVisibleEffects, renderToTexture);

 if (renderToTexture) {
	d->mSceneRenderTargetCache->finishedUsingRenderTarget(d->mMainSceneRenderTarget);
 }

 /* // Visualize shadow textures
 glScalef(d->mGameMatrices->viewport()[2], d->mGameMatrices->viewport()[3], 1.0);
 glDisable(GL_BLEND);
 glDisable(GL_DEPTH_TEST);
 boTextureManager->activateTextureUnit(0);
 glColor3f(1, 1, 1);
 d->mShadowColorTexture->bind();
 glBegin(GL_QUADS);
  glTexCoord2f(0.0, 0.0);
  glVertex2f(0.05, 0.05);
  glTexCoord2f(1.0, 0.0);
  glVertex2f(0.4, 0.05);
  glTexCoord2f(1.0, 1.0);
  glVertex2f(0.4, 0.4);
  glTexCoord2f(0.0, 1.0);
  glVertex2f(0.05, 0.4);
 glEnd();

 d->mShadowTexture->bind();
 glTexParameteri(d->mShadowTexture->type(), GL_TEXTURE_COMPARE_MODE, GL_NONE);
 glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
 glBegin(GL_QUADS);
  glTexCoord2f(0.0, 0.0);
  glVertex2f(0.05, 0.45);
  glTexCoord2f(1.0, 0.0);
  glVertex2f(0.4, 0.45);
  glTexCoord2f(1.0, 1.0);
  glVertex2f(0.4, 0.8);
  glTexCoord2f(0.0, 1.0);
  glVertex2f(0.05, 0.8);
 glEnd();
 boTextureManager->disableTexturing();

 glEnable(GL_BLEND);
 glColor4f(0.5, 0.5, 0.5, 0.3);
 glLineWidth(1.0);
 glBegin(GL_LINE_LOOP);
  glVertex2f(0.05, 0.05);
  glVertex2f(0.4, 0.05);
  glVertex2f(0.4, 0.4);
  glVertex2f(0.05, 0.4);
 glEnd();
 glDisable(GL_BLEND);*/

 BoShader::setFogEnabled(false);
}

void BosonCanvasRenderer::renderShadowMap(const BosonCanvas* canvas)
{
// Size of the shadow texture (more = better quality)
 int shadowResolution = boConfig->intValue("ShadowMapResolution");
 if (!d->mShadowTarget || d->mShadowTarget->width() != shadowResolution) {
	// Cleanup
	delete d->mShadowTexture;
	delete d->mShadowColorTexture;
	delete d->mShadowTarget;
	// Init textures
	d->mShadowTexture = new BoTexture(0, shadowResolution, shadowResolution,
			BoTexture::FilterLinear | BoTexture::FormatDepth | BoTexture::DontCompress | BoTexture::ClampToEdge);
	// This is not used because we only need depth. It's here just for debugging
	//d->mShadowColorTexture = new BoTexture(0, shadowResolution, shadowResolution,
	//		BoTexture::FilterLinear | BoTexture::FormatRGBA | BoTexture::DontCompress | BoTexture::ClampToEdge);

	// Init rendertarget
	d->mShadowTarget = new BoRenderTarget(shadowResolution, shadowResolution,
			BoRenderTarget::RGBA | BoRenderTarget::Depth, 0, d->mShadowTexture);
	boDebug() << k_funcinfo << "Target type: " << d->mShadowTarget->type() << endl;
 }

 if (!d->mShadowTarget->valid()) {
	return;
 }


 // STEP 1:
 // Calculate new temporary projection matrix which contains only stuff which
 //  casts/receives shadows and is as small as possible.
 // Note that side planes of the new frustum will be same as before, but we'll
 //  change distances of near and far planes (to make the depth range as small
 //  as possible).
 BoGroundRendererStatistics* stats = BoGroundRendererManager::manager()->currentRenderer()->statistics();
 // Distances of new near/far plane _from current near plane_
 float neardist = QMIN(stats->minDistance(), d->mMinItemDist);
 float fardist = QMAX(stats->maxDistance(), d->mMaxItemDist);
 /*boDebug() << "Near plane will be pushed by " << neardist << "; far plane by " << fardist << endl <<
		"  (items: " << d->mMinItemDist << "/" << d->mMaxItemDist <<
		"; ground: " << stats->minDistance() << "/" << stats->maxDistance() << ")" << endl;*/
 glMatrixMode(GL_PROJECTION);
 glPushMatrix();
 glLoadIdentity();
 gluPerspective(d->mGameMatrices->fovY(), d->mGameMatrices->aspect(), BO_GL_NEAR_PLANE + neardist, BO_GL_NEAR_PLANE + fardist);
 // Extract view frustum
 BoMatrix compactProjectionMatrix = createMatrixFromOpenGL(GL_PROJECTION_MATRIX);
 BoFrustum compactFrustum;
 compactFrustum.loadViewFrustum(d->mGameMatrices->modelviewMatrix(), compactProjectionMatrix);
 // Restore old matrix
 glPopMatrix();
 glMatrixMode(GL_MODELVIEW);

 // Extract points from the compact viewfrustum
 // Order of the vertices: BLF, BRF, BRN, BLN, TLF, TRF, TRN, TLN;
 BoVector3Float F[8];
 extractViewFrustum(F, compactFrustum);


 // STEP 2:
 // Calculate parameters for settings up view and projection matrices of the
 //  light. This involves finding eye and lookat points as well as calculating
 //  actual near/far plane distances for the light projection matrix so that
 //  the resulting frustum fully contains our earlier-calculated "compact" view
 //  frustum (which in turn contains all items and terrain relevant for
 //  shadowing)
 BoVector3Float lpos(boLightManager->activeLight(0)->position3());
 BoVector3Float focus(camera()->lookAt());
 // Calculate near and far plane distances for the light frustum
 BoVector3Float lightViewDir(focus - lpos);
 lightViewDir.normalize();
 float near = (F[0] - lpos).dotProduct(lightViewDir);
 float far = near;
 for (int i = 1; i < 8; i++) {
	// This is z-coordinate of F[i] in light eye space.
	float d = (F[i] - lpos).dotProduct(lightViewDir);
	near = QMIN(near, d);
	far = QMAX(far, d);
 }
 /*boDebug() << k_funcinfo << "lpos: (" << lpos.x() << "; " << lpos.y() << "; " << lpos.z() << "); focus: (" <<
		focus.x() << "; " << focus.y() << "; " << focus.z() << ");" <<
		" dist: " << (focus - lpos).length() << "; near: " << near << "; far: " << far << endl;*/


 // STEP 3:
 // Set up initial light-space view and projection matrices as well as OpenGL
 //  states.
 // Note that I say initial because the matrices will be modified later to
 //  maximize efficiency.
 glPushAttrib(GL_ALL_ATTRIB_BITS);
 d->mShadowTarget->enable();
 // Set up the viewport
 glViewport(0, 0, shadowResolution, shadowResolution);
 glDisable(GL_SCISSOR_TEST);
 // Clear framebuffer
 glClearDepth(1.0);
 glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

 // Init opengl states to reasonable values
 glDepthFunc(GL_LEQUAL);
 glEnable(GL_DEPTH_TEST);
 glDepthMask(GL_TRUE);
 glColorMask(0, 0, 0, 0);  // only for testing
 glPolygonOffset(4, 15);  // TODO: tune this
 glEnable(GL_POLYGON_OFFSET_FILL);

 // Set up the perspective projection
 glMatrixMode(GL_PROJECTION);
 glPushMatrix();
 glLoadIdentity();
 gluPerspective(45, 1, near, far);
 d->mShadowProjectionMatrix = createMatrixFromOpenGL(GL_PROJECTION_MATRIX);

 // Set up view matrix
 glMatrixMode(GL_MODELVIEW);
 glPushMatrix();
 glLoadIdentity();
 gluLookAt(lpos.x(), lpos.y(), lpos.z(),   focus.x(), focus.y(), focus.z(),   0, 0, 1);
 d->mShadowViewMatrix = createMatrixFromOpenGL(GL_MODELVIEW_MATRIX);
 glDisable(GL_LIGHTING);  // just for testing
 boLightManager->updateAllStates();


 /*boDebug() << "View frustum in world space: NEAR: (" <<
		F[3].x() << "; " << F[3].y() << "; " << F[3].z() << "); (" <<
		F[2].x() << "; " << F[2].y() << "; " << F[2].z() << "); (" <<
		F[6].x() << "; " << F[6].y() << "; " << F[6].z() << "); (" <<
		F[7].x() << "; " << F[7].y() << "; " << F[7].z() << ");" << endl << "    FAR: (" <<
		F[0].x() << "; " << F[0].y() << "; " << F[0].z() << "); (" <<
		F[1].x() << "; " << F[1].y() << "; " << F[1].z() << "); (" <<
		F[5].x() << "; " << F[5].y() << "; " << F[5].z() << "); (" <<
		F[4].x() << "; " << F[4].y() << "; " << F[4].z() << ")" << endl;*/

 // STEP 4:
 // Modify projection matrix to maximize shadow texture efficiency (by using as
 //  much of the shadow texture's area as possible).
 // First tranform view frustum points into post-perspective space of the light
 BoVector4Float E[8];
 for (int i = 0; i < 8; i++) {
	BoVector4Float tmp;
	BoVector4Float in(F[i].x(), F[i].y(), F[i].z(), 1.0f);
	d->mShadowViewMatrix.transform(&tmp, &in);
	d->mShadowProjectionMatrix.transform(&E[i], &tmp);
	// Divide by w to get eucleidian coordinates
	E[i] = E[i] / E[i].w();
 }
 /*boDebug() << "View frustum in pp-space of light: NEAR: (" <<
		E[3].x() << "; " << E[3].y() << "; " << E[3].z() << "; " << E[3].w() << "); (" <<
		E[2].x() << "; " << E[2].y() << "; " << E[2].z() << "; " << E[2].w() << "); (" <<
		E[6].x() << "; " << E[6].y() << "; " << E[6].z() << "; " << E[6].w() << "); (" <<
		E[7].x() << "; " << E[7].y() << "; " << E[7].z() << "; " << E[7].w() << ");" << endl << "    FAR: (" <<
		E[0].x() << "; " << E[0].y() << "; " << E[0].z() << "; " << E[0].w() << "); (" <<
		E[1].x() << "; " << E[1].y() << "; " << E[1].z() << "; " << E[1].w() << "); (" <<
		E[5].x() << "; " << E[5].y() << "; " << E[5].z() << "; " << E[5].w() << "); (" <<
		E[4].x() << "; " << E[4].y() << "; " << E[4].z() << "; " << E[4].w() << ")" << endl;*/

 // Translate and scale the projection matrix to obtain maximum usefulness
 BoVector3Float Emin(E[0].x(), E[0].y(), E[0].z());
 BoVector3Float Emax(E[0].x(), E[0].y(), E[0].z());
 for (int i = 0; i < 8; i++) {
	Emin.setX(QMIN(Emin.x(), E[i].x()));
	Emin.setY(QMIN(Emin.y(), E[i].y()));
	Emin.setZ(QMIN(Emin.z(), E[i].z()));
	Emax.setX(QMAX(Emax.x(), E[i].x()));
	Emax.setY(QMAX(Emax.y(), E[i].y()));
	Emax.setZ(QMAX(Emax.z(), E[i].z()));
 }

 BoVector3Float Emid = (Emin + Emax) / 2;

 glMatrixMode(GL_PROJECTION);
 /*boDebug() << "mid: (" << Emid.x() << "; " << Emid.y() << "; " << Emid.z() << ")" << endl <<
		"glTranslatef(" << -Emid.x() << ", " << -Emid.y() << ", " << -Emid.z() << ");" << endl <<
		"glScalef(" << 2 / (Emax.x() - Emin.x()) << ", " << 2 / (Emax.y() - Emin.y()) << ", " << 2 / (Emax.z() - Emin.z()) << ");" << endl;*/
 BoVector3Float scalev(2 / (Emax.x() - Emin.x()), 2 / (Emax.y() - Emin.y()), 2 / (Emax.z() - Emin.z()));
 glScalef(scalev.x(), scalev.y(), scalev.z());
 glTranslatef(-Emid.x() * scalev.x(), -Emid.y() * scalev.y(), -Emid.z() * scalev.z());
 // Update our stored matrix
 d->mShadowProjectionMatrix = createMatrixFromOpenGL(GL_PROJECTION_MATRIX);
 glMatrixMode(GL_MODELVIEW);

 /*for (int i = 0; i < 8; i++) {
	BoVector4Float tmp;
	BoVector4Float in(F[i].x(), F[i].y(), F[i].z(), 1.0f);
	d->mShadowViewMatrix.transform(&tmp, &in);
	d->mShadowProjectionMatrix.transform(&E[i], &tmp);
	// Divide by w to get eucleidian coordinates
	E[i] = E[i] / E[i].w();
 }
 boDebug() << "View frustum in modified pp-space of light: NEAR: (" <<
		E[3].x() << "; " << E[3].y() << "; " << E[3].z() << "; " << E[3].w() << "); (" <<
		E[2].x() << "; " << E[2].y() << "; " << E[2].z() << "; " << E[2].w() << "); (" <<
		E[6].x() << "; " << E[6].y() << "; " << E[6].z() << "; " << E[6].w() << "); (" <<
		E[7].x() << "; " << E[7].y() << "; " << E[7].z() << "; " << E[7].w() << ");" << endl << "    FAR: (" <<
		E[0].x() << "; " << E[0].y() << "; " << E[0].z() << "; " << E[0].w() << "); (" <<
		E[1].x() << "; " << E[1].y() << "; " << E[1].z() << "; " << E[1].w() << "); (" <<
		E[5].x() << "; " << E[5].y() << "; " << E[5].z() << "; " << E[5].w() << "); (" <<
		E[4].x() << "; " << E[4].y() << "; " << E[4].z() << "; " << E[4].w() << ")" << endl;*/

#if 0
 // Find centers of near and far plane
 // TODO: find out if this approach really works
 BoVector4Float nearCenter = (E[2] + E[3] + E[6] + E[7]) / 4;
 BoVector4Float farCenter = (E[0] + E[1] + E[4] + E[5]) / 4;


 // Calculate 2d convex hull of E
 //int hullpoints = calculateConvexHull(E, 8);
 BoVector3Float Emin(3, 3, 3);
 BoVector3Float Emax(-3, -3, -3);
 for (int i = 0; i < 8; i++) {
	Emin.setX(QMIN(Emin.x(), E[i].x()));
	Emin.setY(QMIN(Emin.y(), E[i].y()));
	Emin.setZ(QMIN(Emin.z(), E[i].z()));
	Emax.setX(QMAX(Emax.x(), E[i].x()));
	Emax.setY(QMAX(Emax.y(), E[i].y()));
	Emax.setZ(QMAX(Emax.z(), E[i].z()));
 }

 BoVector3Float Emid = (Emin + Emax) / 2;

 glMatrixMode(GL_PROJECTION);
 glTranslatef(-Emid.x(), -Emid.y(), -Emid.z());
 glScalef(2 / (Emax.x() - Emin.x()), 2 / (Emax.y() - Emin.y()), 2 / (Emax.z() - Emin.z()));
 glMatrixMode(GL_MODELVIEW);
#endif

 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "after shadow target setup" << endl;
 }


 // STEP 5: render everything that casts shadows!
 renderGround(canvas->map(), DepthOnly);

 renderItems(DepthOnly);


 // Frustum lines
 glDepthMask(GL_FALSE);
 glLineWidth(5.0f);
 glColor3f(255, 255, 192);
 glBegin(GL_LINE_LOOP);
  glVertex3fv(F[3].data());
  glVertex3fv(F[2].data());
  glVertex3fv(F[6].data());
  glVertex3fv(F[7].data());
 glEnd();
 glBegin(GL_LINE_LOOP);
  glVertex3fv(F[0].data());
  glVertex3fv(F[1].data());
  glVertex3fv(F[5].data());
  glVertex3fv(F[4].data());
 glEnd();
 glBegin(GL_LINES);
  glVertex3fv(F[3].data());
  glVertex3fv(F[0].data());
  glVertex3fv(F[2].data());
  glVertex3fv(F[1].data());
  glVertex3fv(F[6].data());
  glVertex3fv(F[5].data());
  glVertex3fv(F[7].data());
  glVertex3fv(F[4].data());
 glEnd();
 glDepthMask(GL_TRUE);

 // STEP 6: uninit everything
 glDisable(GL_POLYGON_OFFSET_FILL);

 glMatrixMode(GL_PROJECTION);
 glPopMatrix();
 glMatrixMode(GL_MODELVIEW);
 glPopMatrix();

 d->mShadowTarget->disable();
 glPopAttrib();

 // Done!
}

void BosonCanvasRenderer::extractViewFrustum(BoVector3Float* points, const BoFrustum& viewFrustum)
{
 // we have planes RIGHT, LEFT, BOTTOM, TOP, FAR, NEAR, we are going to name
 // lines and points accordingly (point at LEFT/BOTTOM/NEAR planes is LBN)
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

 // now retrieve all points using the lines.
 // note that we must not do line-line intersection, as that would be highly
 // inaccurate. we use line-plane intersection instead, which provides more
 // accurate results
 // Order of the vertices: BLF, BRF, BRN, BLN, TLF, TRF, TRN, TLN;
 planeBottom.intersectLine(LF_point, LF_dir, &points[0]);
 planeBottom.intersectLine(RF_point, RF_dir, &points[1]);
 planeBottom.intersectLine(RN_point, RN_dir, &points[2]);
 planeBottom.intersectLine(LN_point, LN_dir, &points[3]);
 planeTop.intersectLine(LF_point, LF_dir, &points[4]);
 planeTop.intersectLine(RF_point, RF_dir, &points[5]);
 planeTop.intersectLine(RN_point, RN_dir, &points[6]);
 planeTop.intersectLine(LN_point, LN_dir, &points[7]);
}

void BosonCanvasRenderer::activateShadowMap()
{
 // Shadow tex will go to texunit 3
 boTextureManager->activateTextureUnit(3);
 d->mShadowTexture->bind();

 BoMatrix texMatrix;
 GLenum planes[] = { GL_S, GL_T, GL_R, GL_Q };
 for(int i = 0; i < 4; i++)
 {
  BoVector4Float plane(texMatrix.data() + i*4);
  glTexGenfv(planes[i], GL_EYE_PLANE, plane.data());
  glTexGeni(planes[i], GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
 }
 glEnable(GL_TEXTURE_GEN_S);
 glEnable(GL_TEXTURE_GEN_T);
 glEnable(GL_TEXTURE_GEN_R);
 glEnable(GL_TEXTURE_GEN_Q);
 glTexParameteri(d->mShadowTexture->type(), GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
 glTexParameteri(d->mShadowTexture->type(), GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
 glTexParameteri(d->mShadowTexture->type(), GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
 glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

 glMatrixMode(GL_TEXTURE);
 glLoadIdentity();
 glTranslatef(0.5, 0.5, 0.5);
 glScalef(0.5, 0.5, 0.5);
 glMultMatrixf(d->mShadowProjectionMatrix.data());
 glMultMatrixf(d->mShadowViewMatrix.data());
 glMatrixMode(GL_MODELVIEW);


 //glAlphaFunc(GL_GEQUAL, 0.99);
 //glEnable(GL_ALPHA_TEST);
 boTextureManager->activateTextureUnit(0);

 //d->mShadowShader->bind();
}

void BosonCanvasRenderer::deactivateShadowMap()
{
 boTextureManager->activateTextureUnit(3);
 glDisable(GL_TEXTURE_GEN_S);
 glDisable(GL_TEXTURE_GEN_T);
 glDisable(GL_TEXTURE_GEN_R);
 glDisable(GL_TEXTURE_GEN_Q);
 glMatrixMode(GL_TEXTURE);
 glLoadIdentity();
 glMatrixMode(GL_MODELVIEW);
 boTextureManager->disableTexturing();
 boTextureManager->activateTextureUnit(0);
}

void BosonCanvasRenderer::renderGround(const BosonMap* map, RenderFlags flags)
{
 PROFILE_METHOD;
 BO_CHECK_NULL_RET(map);
 BoTextureManager::BoTextureBindCounter bindCounter(boTextureManager, &d->mTextureBindsCells);
 glEnable(GL_DEPTH_TEST);
 if (boConfig->boolValue("UseLight") && !(flags & DepthOnly)) {
	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
 } else {
	glDisable(GL_COLOR_MATERIAL);
 }

 BO_CHECK_NULL_RET(BoGroundRendererManager::manager()->currentRenderer());
 d->mRenderedCells = BoGroundRendererManager::manager()->currentRenderer()->renderCells(map, flags);

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
 glPushAttrib(GL_ALL_ATTRIB_BITS);
 glDisable(GL_TEXTURE_2D);
 glDisable(GL_LIGHTING);
 glColor3ub(127, 127, 127);
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
 glPopAttrib();
}

bool BosonCanvasRenderer::mustRenderToTexture(BoVisibleEffects& visible)
{
 // TODO: use dedicated boconfig key
 if (!boConfig->boolValue("UseUnitShaders")) {
	return false;
 } else if (!boglGetOpenGLExtensions().contains("GL_EXT_framebuffer_object")) {
	// FBO is required for RTT
	return false;
 }

 QPtrListIterator<BosonEffectFade> it(visible.mFadeEffects);
 while (it.current()) {
	if (it.current()->passes() > 0) {
		return true;
	}
	++it;
 }
 return false;
}

bool BosonCanvasRenderer::startRenderingToTexture()
{
 int widgetwidth = d->mGameMatrices->viewport()[2];
 int widgetheight = d->mGameMatrices->viewport()[3];

 d->mMainSceneRenderTarget = d->mSceneRenderTargetCache->getRenderTarget(widgetwidth, widgetheight, true);
 if (!d->mMainSceneRenderTarget->valid()) {
	return false;
 }

 // Enable the rendertarget
 glPushAttrib(GL_ALL_ATTRIB_BITS);
 d->mMainSceneRenderTarget->enable();

 // Load viewport and matrices
 glViewport(d->mGameMatrices->viewport()[0], d->mGameMatrices->viewport()[1],
		d->mGameMatrices->viewport()[2], d->mGameMatrices->viewport()[3]);
 glMatrixMode(GL_PROJECTION);
 glLoadMatrixf(d->mGameMatrices->projectionMatrix().data());
 glMatrixMode(GL_MODELVIEW);
 glLoadMatrixf(d->mGameMatrices->modelviewMatrix().data());

 // Clear framebuffer
 glClearDepth(1.0);
 glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

 // Init some opengl states
 glDisable(GL_SCISSOR_TEST);
 glDepthFunc(GL_LEQUAL);
 glEnable(GL_DEPTH_TEST);

 return true;
}

void BosonCanvasRenderer::stopRenderingToTexture()
{
 d->mMainSceneRenderTarget->disable();
 glPopAttrib();
}

void BosonCanvasRenderer::createRenderItemList(QValueVector<BoRenderItem>* renderItemList, QValueList<Unit*>* radarContactList, const QPtrList<BosonItemContainer>& allItems)
{
 BO_CHECK_NULL_RET(localPlayerIO());

 renderItemList->clear();
 renderItemList->reserve(allItems.count());
 radarContactList->clear();

 d->mMinItemDist = 1000000.0f;
 d->mMaxItemDist = 0.0f;

 BoVector3Float camerapos = camera()->cameraPos();

 for (QPtrListIterator<BosonItemContainer> it(allItems); it.current(); ++it) {
	BosonItem* item = it.current()->item();
	BosonItemRenderer* itemRenderer = it.current()->itemRenderer();

	if (!item->isVisible() || !itemRenderer) {
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
	float dist = itemRenderer->itemInFrustum(viewFrustum());
	if (dist == 0.0f) {
		// the unit is not visible, currently. no need to draw anything.
		continue;
	}

	// AB: note units are rendered in the *center* point of their
	// width/height.
	// but concerning z-position they are rendered from bottom to top!

	if (RTTI::isUnit(item->rtti())) {
		Unit* u = (Unit*)item;
		if (!(u->visibleStatus(localPlayerIO()->playerId()) & (UnitBase::VS_Visible | UnitBase::VS_Earlier))) {
			if (u->radarSignalStrength(localPlayerIO()->playerId()) >= 1) {
				radarContactList->append(u);
			}
			continue;
		}
	} else {
		// It's an item, not unit
		if (!localPlayerIO()->canSee(item)) {
			continue;
		}
	}

	unsigned int modelid = 0;
	if (itemRenderer->model()) {
		modelid = itemRenderer->model()->id();
	}

	// Units will be tinted accordingly to how much health they have left
	QColor tintColor(255, 255, 255);
	if (RTTI::isUnit(item->rtti())) {
		Unit* u = (Unit*)item;
		if (u->isDestroyed()) {
			tintColor = QColor(102, 102, 102);
		} else {
			float f = u->health() / (float)u->maxHealth();
			float a = f * 0.3f + 0.7f;
			a = QMIN(a, 1.0f);
			int c = (int)(255 * a);
			tintColor = QColor(c, c, c);
		}
	}

	if (d->mVisualFeedbacks->tintItem(item)) {
		tintColor = d->mVisualFeedbacks->tintColor(item);
	}

	// TODO: what was this dist for? is it still necessary?
	renderItemList->append(BoRenderItem(modelid, item, itemRenderer, tintColor));

	d->mMinItemDist = QMIN(d->mMinItemDist, dist - 2*itemRenderer->boundingSphereRadius());
	d->mMaxItemDist = QMAX(d->mMaxItemDist, dist);
 }

 d->mMinItemDist = QMAX(0, d->mMinItemDist);
 d->mMaxItemDist = QMAX(0, d->mMaxItemDist);
 if (renderItemList->isEmpty()) {
	d->mMinItemDist = d->mMinItemDist = 0;
 }
}

void BosonCanvasRenderer::renderItems(RenderFlags flags)
{
 PROFILE_METHOD;
 BoTextureManager::BoTextureBindCounter bindCounter(boTextureManager, &d->mTextureBindsItems);
 BosonItemRenderer::startItemRendering();
 if (boConfig->boolValue("debug_wireframes") && !(flags & DepthOnly)) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
 }
 glEnable(GL_DEPTH_TEST);
 glDisable(GL_ALPHA_TEST);
 glDisable(GL_BLEND);
 if (boConfig->boolValue("UseLight") && !(flags & DepthOnly)) {
	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
 } else {
	glDisable(GL_COLOR_MATERIAL);
 }

 unsigned int itemCount = d->mRenderItemList.count();

 {
	// Sort the list of to-be-rendered items by their models, so that items with
	//  same models are rendered after each other. This increases rendering
	//  performance (especially with vbos).

	int idCount = BosonModel::maxId() + 1;

	// Bucketsort
	QValueList<BoRenderItem>** lists = new QValueList<BoRenderItem>*[idCount];
	for (int i = 0; i < idCount; i++) {
		lists[i] = 0;
	}
	for (unsigned int i = 0; i < itemCount; i++) {
		int id = d->mRenderItemList[i].modelId;
		if (!lists[id]) {
			lists[id] = new QValueList<BoRenderItem>();
		}
		lists[id]->append(d->mRenderItemList[i]);
	}
	unsigned int pos = 0;
	QValueList<BoRenderItem>::iterator it;
	for (int i = 0; i < idCount; i++) {
		if (!lists[i]) {
			continue;
		}
		for (it = lists[i]->begin(); it != lists[i]->end(); ++it) {
			d->mRenderItemList[pos] = *it;
			pos++;
		}
		delete lists[i];
		lists[i] = 0;
	}
	delete[] lists;
 }

 bool useLOD = boConfig->boolValue("UseLOD");

 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "OpenGL error before rendering items" << endl;
 }

 // List of models with semi-transparent parts
 QValueVector<BoRenderItem> transparentModels;
 transparentModels.reserve((int)(itemCount * 0.25));

 d->mIconicUnits.clear();
 const float baseIconifyDist = 80.0;

 // Model that is being used currently
 BosonModel* currentModel = 0;
 // Render all items
 for (unsigned int i = 0; i < itemCount; i++) {
	const BosonItem* item = d->mRenderItemList[i].item;
	BosonItemRenderer* itemRenderer = d->mRenderItemList[i].itemRenderer;
	if (!itemRenderer) {
		BO_NULL_ERROR(itemRenderer);
		continue;
	}

	float iconifyDist = baseIconifyDist * sqrt(item->width());
	float distSq = (camera()->cameraPos() - BoVector3Float(item->centerX(), -item->centerY(), item->z())).dotProduct();
	if (distSq >= iconifyDist*iconifyDist && (flags & DepthOnly)) {
		continue;
	}

	// AB: note units are rendered in the *center* point of their
	// width/height.
	// but concerning z-position they are rendered from bottom to top!

	if (!(flags & DepthOnly)) {
		const QColor& c = d->mRenderItemList[i].tintColor;
		glColor3ub(c.red(), c.green(), c.blue());
	}

	if (distSq < iconifyDist*iconifyDist) {
		unsigned int lod;
		currentModel = renderSingleItem(useLOD, camera(), item, itemRenderer, false, currentModel, &lod, flags);


		if (currentModel && currentModel->hasTransparentMeshes(lod)) {
			transparentModels.append(d->mRenderItemList[i]);
		}

		if (boConfig->boolValue("debug_boundingboxes") && !(flags & DepthOnly)) {
			renderBoundingBox(item);
		}
	} else if (RTTI::isUnit(item->rtti())) {
		Unit* u = (Unit*) item;
		if (!u->isDestroyed()) {
			d->mIconicUnits.append(u);
		}
	}
 }

 if (transparentModels.count() > 0) {
	// Render semi-transparent meshes of the models
	// TODO: sort the models by depth
	glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_CURRENT_BIT);
	glEnable(GL_DEPTH_TEST);
	//glDepthMask(GL_FALSE);
	if (!(flags & DepthOnly)) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GEQUAL, 0.2);
	glDisable(GL_CULL_FACE);
	//glDisable(GL_LIGHTING);
	for (unsigned int i = 0; i < transparentModels.count(); i++) {
		BosonItem* item = transparentModels[i].item;
		BosonItemRenderer* itemRenderer = transparentModels[i].itemRenderer;
		if (!itemRenderer) {
			BO_NULL_ERROR(itemRenderer);
			continue;
		}

		if (!(flags & DepthOnly)) {
			const QColor& c = transparentModels[i].tintColor;
			glColor3ub(c.red(), c.green(), c.blue());
		}

		unsigned int lod;
		currentModel = renderSingleItem(useLOD, camera(), item, itemRenderer, true, currentModel, &lod, flags);
	}
	glPopAttrib();
 }
 glColor3ub(255, 255, 255);

 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "OpenGL error before rendering selections" << endl;
 }

 if (!(flags & DepthOnly)) {
	BoItemList* selectedItems = new BoItemList(0, false);
	createSelectionsList(selectedItems, &d->mRenderItemList);
	renderSelections(selectedItems);
	delete selectedItems;
	selectedItems = 0;
	if (Bo3dTools::checkError()) {
		boError() << k_funcinfo << "OpenGL error after rendering selections" << endl;
	}
 }

 boTextureManager->invalidateCache();
 d->mRenderedItems += d->mRenderItemList.count();

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
	Unit* u = 0;
	if (RTTI::isUnit(item->rtti())) {
		u = (Unit*)item;
	}
	float factor = 1.0f;
	if (u) {
		factor = ((float)u->health()) / ((float)u->maxHealth());
	}
	GLuint list = d->mSelectBoxData->list(factor);
	glCallList(list);
	glPopMatrix();
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

void BosonCanvasRenderer::renderUnitIcons()
{
 // Taken from renderParticles()
 const BoMatrix& modelview = d->mGameMatrices->modelviewMatrix();
 const BoVector3Float x(modelview[0], modelview[4], modelview[8]);
 const BoVector3Float y(modelview[1], modelview[5], modelview[9]);
 const BoVector3Float z(modelview[2], modelview[6], modelview[10]);

 const BoVector3Float upperleft (-x + y);
 const BoVector3Float upperright( x + y);
 const BoVector3Float lowerright( x - y);
 const BoVector3Float lowerleft (-x - y);

 const BoVector4Float enemyColor(0.8, 0.0, 0.0, 0.8);
 const BoVector4Float neutralColor(0.0, 0.4, 0.0, 0.3);
 const BoVector4Float alliedColor(0.0, 0.0, 1.0, 0.8);


 glEnable(GL_BLEND);
 glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 glEnable(GL_ALPHA_TEST);
 glAlphaFunc(GL_GEQUAL, 0.1);

 for (QValueList<Unit*>::Iterator it = d->mIconicUnits.begin(); it != d->mIconicUnits.end(); ++it) {
	Unit* u = *it;
	BoVector3Float pos(u->centerX(), -u->centerY(), u->centerZ());
	float distSq = (camera()->cameraPos() - pos).dotProduct();
	float dist = sqrt(distSq);
	float sqrtwidth = sqrt(u->width());

	// Select icon size
	float iconsize = 0.6f * (dist / 70) * sqrtwidth;
	BoVector3Float shift = z * iconsize;

	// Select color
	BoVector4Float color;
	if (localPlayerIO()->isEnemy(u)) {
		glColor4fv(enemyColor.data());
	} else if (localPlayerIO()->isNeutral(u)) {
		glColor4fv(neutralColor.data());
	} else {
		glColor4fv(alliedColor.data());
	}

	// Select texture
	if (u->unitProperties()->isFacility()) {
		d->mUnitIconFacility->bind();
	} else if (u->unitProperties()->isAircraft()) {
		d->mUnitIconAir->bind();
	} else {
		d->mUnitIconLand->bind();
	}

	// Render icon
	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 1.0);  glVertex3fv((pos + (upperleft  * iconsize) + shift).data());
		glTexCoord2f(1.0, 1.0);  glVertex3fv((pos + (upperright * iconsize) + shift).data());
		glTexCoord2f(1.0, 0.0);  glVertex3fv((pos + (lowerright * iconsize) + shift).data());
		glTexCoord2f(0.0, 0.0);  glVertex3fv((pos + (lowerleft  * iconsize) + shift).data());
	glEnd();
 }

 // Render radar contacts
 d->mUnitIconFacility->bind();
 for (QValueList<Unit*>::Iterator it = d->mRadarContactsList.begin(); it != d->mRadarContactsList.end(); ++it) {
	Unit* u = *it;
	// Radars and jammers will be rendered separately
	if (u->plugin(UnitPlugin::Radar) || u->plugin(UnitPlugin::RadarJammer)) {
		continue;
	}

	BoVector3Float pos(u->centerX(), -u->centerY(), u->centerZ());
	float distSq = (camera()->cameraPos() - pos).dotProduct();
	float dist = sqrt(distSq);
	float sqrtwidth = sqrt(u->width());

	// Select icon size
	float iconsize = 0.5f * (dist / 70) * sqrtwidth;
	BoVector3Float shift = z * iconsize;
	float alpha = 0.3 + QMIN((float)u->radarSignalStrength(localPlayerIO()->playerId()) / 20.0f, 0.5f);
	glColor4f(0.4, 0.4, 0.4, alpha);

	// Render icon
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 1.0);  glVertex3fv((pos + (upperleft  * iconsize) + shift).data());
	glTexCoord2f(1.0, 1.0);  glVertex3fv((pos + (upperright * iconsize) + shift).data());
	glTexCoord2f(1.0, 0.0);  glVertex3fv((pos + (lowerright * iconsize) + shift).data());
	glTexCoord2f(0.0, 0.0);  glVertex3fv((pos + (lowerleft  * iconsize) + shift).data());
	glEnd();
 }

 // Render radars
 d->mRadarIcon->bind();
 const QValueList<const Unit*>* radars = d->mCanvas->radarUnits();
 for (QValueList<const Unit*>::const_iterator rit = radars->begin(); rit != radars->end(); ++rit) {
	const Unit* u = *rit;
	if (u->radarSignalStrength(localPlayerIO()->playerId()) < 1 || u->ownerIO() == localPlayerIO()) {
		continue;
	}

	RadarPlugin* radar = (RadarPlugin*)u->plugin(UnitPlugin::Radar);
	BoVector3Float pos(u->centerX(), -u->centerY(), u->centerZ());
	float distSq = (camera()->cameraPos() - pos).dotProduct();
	float dist = sqrt(distSq);
	float sqrtwidth = sqrt(u->width());

	// Select icon size
	float iconsize = 0.02f * (dist / 70) * sqrtf(radar->transmittedPower());
	BoVector3Float shift = z * iconsize;
	//float alpha = 0.3 + QMIN((float)u->radarSignalStrength(localPlayerIO()->playerId()) / 20.0f, 0.5f);
	glColor4f(1.0, 1.0, 1.0, 0.6);

	// Render icon
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 1.0);  glVertex3fv((pos + (upperleft  * iconsize) + shift).data());
	glTexCoord2f(1.0, 1.0);  glVertex3fv((pos + (upperright * iconsize) + shift).data());
	glTexCoord2f(1.0, 0.0);  glVertex3fv((pos + (lowerright * iconsize) + shift).data());
	glTexCoord2f(0.0, 0.0);  glVertex3fv((pos + (lowerleft  * iconsize) + shift).data());
	glEnd();
 }

 // Render jammers
 d->mJammingIcon->bind();
 const QValueList<const Unit*>* jammers = d->mCanvas->radarJammerUnits();
 for (QValueList<const Unit*>::const_iterator it = jammers->begin(); it != jammers->end(); ++it) {
	const Unit* u = *it;
	if (u->radarSignalStrength(localPlayerIO()->playerId()) < 1 || u->ownerIO() == localPlayerIO()) {
		continue;
	}
	RadarJammerPlugin* jammer = (RadarJammerPlugin*)u->plugin(UnitPlugin::RadarJammer);
	BoVector3Float pos(u->centerX(), -u->centerY(), u->centerZ());
	float distSq = (camera()->cameraPos() - pos).dotProduct();
	float dist = sqrt(distSq);
	float sqrtwidth = sqrt(u->width());

	// Select icon size
	float iconsize = 0.06f * (dist / 70) * sqrtf(jammer->transmittedPower());
	BoVector3Float shift = z * iconsize;
	//float alpha = 0.3 + QMIN((float)u->radarSignalStrength(localPlayerIO()->playerId()) / 20.0f, 0.5f);
	glColor4f(1.0, 1.0, 1.0, 0.6);

	// Render icon
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 1.0);  glVertex3fv((pos + (upperleft  * iconsize) + shift).data());
	glTexCoord2f(1.0, 1.0);  glVertex3fv((pos + (upperright * iconsize) + shift).data());
	glTexCoord2f(1.0, 0.0);  glVertex3fv((pos + (lowerright * iconsize) + shift).data());
	glTexCoord2f(0.0, 0.0);  glVertex3fv((pos + (lowerleft  * iconsize) + shift).data());
	glEnd();
 }

 glColor4ub(255, 255, 255, 255);
 glDisable(GL_BLEND);
 glDisable(GL_ALPHA_TEST);
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
 // TODO:
#if 0
 if (currentAdvanceCall != lastAdvanceCall) {
	int advanceDiff = (currentAdvanceCall - lastAdvanceCall);
	if (advanceDiff > 0) {
		boWaterRenderer->update(advanceDiff * 0.05);
	}
 }
 // TODO (somewhere else probably):
 lastAdvanceCall = currentAdvanceCall;
#endif
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

void BosonCanvasRenderer::renderFadeEffects(BoVisibleEffects& visible, bool enableShaderEffects)
{
 PROFILE_METHOD;
 BO_CHECK_NULL_RET(d->mGameMatrices);
 if (visible.mFadeEffects.isEmpty()) {
	return;
 }

 BosonEffectFade* effect;

 // Width/height of the scene
 int widgetwidth = d->mGameMatrices->viewport()[2];
 int widgetheight = d->mGameMatrices->viewport()[3];

 // Scale so that (0; 0) is bottom-left corner of the viewport and (1; 1) is
 //  top-right corner
 glMatrixMode(GL_PROJECTION);
 glLoadIdentity();
 gluOrtho2D(0.0, 1.0, 0.0, 1.0);
 glMatrixMode(GL_MODELVIEW);
 glLoadIdentity();

 // Disable texturing
 boTextureManager->activateTextureUnit(0);
 boTextureManager->disableTexturing();
 glEnable(GL_BLEND);

 // Render NON-SHADER effects
 int rendered = 0;
 QPtrListIterator<BosonEffectFade> it(visible.mFadeEffects);
 for (; it.current(); ++it) {
	effect = it.current();
	if (effect->passes() != 0) {
		continue;
	}

	glBlendFunc(effect->blendFunc()[0], effect->blendFunc()[1]);
	glColor4fv(effect->color().data());

	BoVector4Fixed geo = effect->geometry();  // x, y, w, h
	glBegin(GL_QUADS);
		glTexCoord2f(geo[0], geo[1]);
		glVertex2f(geo[0], geo[1]);
		glTexCoord2f(geo[0] + geo[2], geo[1]);
		glVertex2f(geo[0] + geo[2], geo[1]);
		glTexCoord2f(geo[0] + geo[2], geo[1] + geo[3]);
		glVertex2f(geo[0] + geo[2], geo[1] + geo[3]);
		glTexCoord2f(geo[0], geo[1] + geo[3]);
		glVertex2f(geo[0], geo[1] + geo[3]);
	glEnd();
	rendered++;
 }


 // Restore OpenGL states and return if there's no shader effects to render
 if (!enableShaderEffects || rendered == (int)visible.mFadeEffects.count()) {
	glDisable(GL_BLEND);
	glColor3ub(255, 255, 255);
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(0.0, (GLfloat)widgetwidth, 0.0, (GLfloat)widgetheight);
	glMatrixMode(GL_MODELVIEW);
	return;
 }


 // render SHADER effects
 // Clear color buffer
 glClear(GL_COLOR_BUFFER_BIT);
 glColor3ub(255, 255, 255);

 // We'll modify texture matrices so that (1; 1) is always top-right corner
 //  of the _screen_, no matter what the texture size is.
 glMatrixMode(GL_TEXTURE);
 float sceneTexScaleX = d->mMainSceneRenderTarget->texture->width() / (float)widgetwidth;
 float sceneTexScaleY = d->mMainSceneRenderTarget->texture->height() / (float)widgetheight;
 // Bind scene color tex to texunit 0 and change tex matrix
 boTextureManager->activateTextureUnit(0);
 d->mMainSceneRenderTarget->texture->bind();
 // Generate mipmaps for the scene texture
 glGenerateMipmapEXT(GL_TEXTURE_2D);
 glLoadIdentity();
 glScalef(sceneTexScaleX, sceneTexScaleY, 1.0);
  // Bind scene depth tex to texunit 1 and change tex matrix
 boTextureManager->activateTextureUnit(1);
 d->mMainSceneRenderTarget->depthTexture->bind();
 glLoadIdentity();
 glScalef(sceneTexScaleX, sceneTexScaleY, 1.0);
 boTextureManager->activateTextureUnit(2);


 // Render all effects
 for (it.toFirst(); it.current(); ++it) {
	effect = it.current();
	if (effect->passes() == 0) {
		continue;
	}
	BoSceneRenderTarget* inputTarget = d->mMainSceneRenderTarget;
	BoSceneRenderTarget* outputTarget = 0;

	for (int pass = 0; pass < effect->passes(); pass++) {
		// Downscale factor of this pass
		int downscale = effect->downscale(pass);

		// Check whether we need to render this pass to texture. This is the case
		//  when we're rendering an intermediate pass or when the last pass uses
		//  downscaling.
		bool renderToTexture = (downscale > 1) || (pass < effect->passes() - 1);
		if (renderToTexture) {
			// This is not the last pass. Create temporary rendertarget and texture
			outputTarget = d->mSceneRenderTargetCache->getRenderTarget(widgetwidth / downscale, widgetheight / downscale);

			// Enable rendertarget
			glPushAttrib(GL_ALL_ATTRIB_BITS);
			outputTarget->enable();

			// Init viewport and some other states
			glViewport(0, 0, widgetwidth / downscale, widgetheight / downscale);
			glClear(GL_COLOR_BUFFER_BIT);
			glDisable(GL_DEPTH_TEST);
			glDisable(GL_SCISSOR_TEST);
			// TODO: load states, matrices, etc if we don't use FBO
			// Init modelview and projection matrices
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluOrtho2D(0.0, 1.0, 0.0, 1.0);
			glMatrixMode(GL_TEXTURE);
		}


		// Outputs of intermediate passes will be in texunit 2
		boTextureManager->activateTextureUnit(2);
		inputTarget->texture->bind();
		glLoadIdentity();
		// Find out downscale factor of the previous pass and use it
		int lastDownscale = (pass > 0) ? effect->downscale(pass - 1) : 1;
		glScalef(inputTarget->texture->width() / (float)(widgetwidth / lastDownscale),
				inputTarget->texture->height() / (float)(widgetheight / lastDownscale), 1.0);
		// Generate mipmaps for input texture, if necessary
		glGenerateMipmapEXT(GL_TEXTURE_2D);


		// Bind shader
		BoShader* s = effect->shader(pass);
		s->bind();
		s->setUniform("pixelWidth", 1.0f / (widgetwidth / downscale));
		s->setUniform("pixelHeight", 1.0f / (widgetheight / downscale));

		// Set blend func and color
		//glBlendFunc(effect->blendFunc()[0], effect->blendFunc()[1]);
		glColor4fv(effect->color().data());

		// Render the quad
		BoVector4Fixed geo = effect->geometry();  // x, y, w, h
		glBegin(GL_QUADS);
			glTexCoord2f(geo[0], geo[1]);
			glVertex2f(geo[0], geo[1]);
			glTexCoord2f(geo[0] + geo[2], geo[1]);
			glVertex2f(geo[0] + geo[2], geo[1]);
			glTexCoord2f(geo[0] + geo[2], geo[1] + geo[3]);
			glVertex2f(geo[0] + geo[2], geo[1] + geo[3]);
			glTexCoord2f(geo[0], geo[1] + geo[3]);
			glVertex2f(geo[0], geo[1] + geo[3]);
		glEnd();

		// Uninit
		glLoadIdentity();
		boTextureManager->disableTexturing();
		s->unbind();


		if (renderToTexture) {
			// Finish rendering to rendertarget
			outputTarget->disable();
			glPopAttrib();

			// The input texture won't be used anymore unless this is
			//  the 1st pass (in this case, scene tex is the input tex)
			if (pass > 0) {
				d->mSceneRenderTargetCache->finishedUsingRenderTarget(inputTarget);
			}

			// Output of this pass will be input of next pass
			inputTarget = outputTarget;

			if (pass == effect->passes() - 1) {
				// This was the last pass. We now need to render the (downscaled)
				//  result onto the screen.
				glPushAttrib(GL_ALL_ATTRIB_BITS);
				// Disable texturing in texunit 1 and bind output tex to texunit 0
				boTextureManager->activateTextureUnit(1);
				boTextureManager->disableTexturing();
				boTextureManager->activateTextureUnit(0);
				outputTarget->texture->bind();
				// Use blendfunc and color specified by the effect
				glBlendFunc(effect->blendFunc()[0], effect->blendFunc()[1]);
				glColor4fv(effect->color().data());
				// Calculate max texcoords to use
				float maxTextureXCoord = (float)(widgetwidth / lastDownscale) / outputTarget->texture->width();
				float maxTextureYCoord = (float)(widgetheight / lastDownscale) / outputTarget->texture->height();
				// Render the textured quad
				glBegin(GL_QUADS);
					glTexCoord2f(0.0, 0.0);
					glVertex2f(0.0, 0.0);
					glTexCoord2f(maxTextureXCoord, 0.0);
					glVertex2f(1.0, 0.0);
					glTexCoord2f(maxTextureXCoord, maxTextureYCoord);
					glVertex2f(1.0, 1.0);
					glTexCoord2f(0.0, maxTextureYCoord);
					glVertex2f(0.0, 1.0);
				glEnd();
				glPopAttrib();
				// Output target won't be used anymore
				d->mSceneRenderTargetCache->finishedUsingRenderTarget(outputTarget);
			}
		} else if (pass > 0) {
			d->mSceneRenderTargetCache->finishedUsingRenderTarget(inputTarget);
		}

	}
 }

 boTextureManager->activateTextureUnit(1);
 boTextureManager->disableTexturing();
 glLoadIdentity();
 boTextureManager->activateTextureUnit(0);
 boTextureManager->disableTexturing();
 glLoadIdentity();

 glDisable(GL_BLEND);
 glColor3ub(255, 255, 255);
 glMatrixMode(GL_PROJECTION);
 glLoadIdentity();
 gluOrtho2D(0.0, (GLfloat)widgetwidth, 0.0, (GLfloat)widgetheight);
 glMatrixMode(GL_MODELVIEW);
 glLoadIdentity();
}

void BosonCanvasRenderer::slotAddFeedbackAttack(const QPtrList<Unit>& attacker, const Unit* unit)
{
 // TODO: add some "ItemTint" for the items that were ordered to attack
 BoVisualFeedbackItemTint* f = new BoVisualFeedbackItemTint(unit, 250, Qt::red);
 d->mVisualFeedbacks->addFeedback(f);
}

void BosonCanvasRenderer::slotAddFeedbackMoveTo(const QPtrList<Unit>& units, const BoVector2Fixed& cell, bool withAttack)
{
 // TODO: add a green "ItemTint" for the items that were ordered to move
 Q_UNUSED(withAttack);
 BO_CHECK_NULL_RET(d->mCanvas);

 float z = d->mCanvas->heightAtPoint(cell.x(), cell.y());
 BoVector3Float pos(cell.x(), -cell.y(), z);
 BoVisualFeedbackGroundDot* f = new BoVisualFeedbackGroundDot(pos, 500, Qt::green);
 d->mVisualFeedbacks->addFeedback(f);
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


void BosonCanvasRenderer::slotItemRemoved(BosonItem* item)
{
 for (QValueVector<BoRenderItem>::iterator it = d->mRenderItemList.begin(); it != d->mRenderItemList.end(); ++it) {
	if ((*it).item == item) {
		d->mRenderItemList.erase(it);
		return;
	}
 }
}

// AB: large parts are from Mesa-5.1/src/glu/mesa/glu.c: gluPickMatrix().
QValueList<BosonItem*> BosonCanvasRenderer::emulatePickItems(const QRect& pickRect) const
{
 PROFILE_METHOD
 BoRect2Float _pickRect((float)pickRect.x(), (float)pickRect.y(), (float)(pickRect.x() + pickRect.width()), (float)(pickRect.y() + pickRect.height()));
 BoFrustum viewFrustum;
 viewFrustum.loadPickViewFrustum(_pickRect, d->mGameMatrices->viewport(), d->mGameMatrices->modelviewMatrix(), d->mGameMatrices->projectionMatrix());


 QValueList<BosonItem*> items;
 for (QValueVector<BoRenderItem>::const_iterator it = d->mRenderItemList.begin(); it != d->mRenderItemList.end(); ++it) {
	BosonItem* item = (*it).item;
	BosonItemRenderer* itemRenderer = (*it).itemRenderer;
	if (!itemRenderer) {
		continue;
	}

	// AB: note: we don't have to check for localPlayerIO()->canSee(item),
	//     as all items in d->mRenderItemList are visible to the local
	//     player anyway

	if (!itemRenderer->itemInFrustumSlow(viewFrustum)) {
		continue;
	}
	items.append(item);
 }

 return items;
}

