/*
    This file is part of the Boson game
    Copyright (C) 2002-2005 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOSONITEMRENDERER_H
#define BOSONITEMRENDERER_H

#include "../defines.h"
#include "../global.h"
#include <bogl.h>

#include <qglobal.h>
#include <q3valuevector.h>

class BosonModel;
class BosonAnimation;
class BoFrame;
class Player;
class BosonItem;
class BoFrustum;
class BoMatrix;

class QColor;
class QString;

/**
 * Base class for @ref BosonItemModelRenderer. You can use this class directly,
 * however it does not use @ref BosonModel and therefore it is useful for
 * debugging only (i.e. when you don't want to load any models, but still want
 * to see the location of the units)
 **/
class BosonItemRenderer
{
public:
	enum RTTI {
		SimpleRenderer = 0,
		ModelRenderer = 1
	};

	BosonItemRenderer(BosonItem* item);
	virtual ~BosonItemRenderer();

	virtual int rtti() const { return SimpleRenderer; }

	static void startItemRendering();
	static void stopItemRendering();

	virtual bool setModel(BosonModel*) { return true; }
	virtual BosonModel* model() const { return 0; }

	virtual void setAnimationMode(int ) { }

	int animationMode() const
	{
		return mAnimationMode;
	}

	virtual void animate() { }

	/**
	 * Render the item. This assumes the modelview matrix was already
	 * translated and rotated to the correct position.
	 * @param lod See @ref BoFrame::renderFrame
	 **/
	virtual void renderItem(unsigned int lod = 0, bool transparentmeshes = false, RenderFlags flags = Default);

	/**
	 * @return distance of the item from the NEAR plane (or 0 if it's not in @p frustum)
	 **/
	float itemInFrustum(const BoFrustum& frustum) const;

	/**
	 * Like @ref itemInFrustum but slower and more precise. This method
	 * currently does both, a bounding sphere and a bounding box test.
	 *
	 * As a result of the additional tests, this method returns only whether
	 * the item is in the frustum, not the distance from the NEAR plane.
	 **/
	bool itemInFrustumSlow(const BoFrustum& frustum) const;

	virtual unsigned int preferredLod(float distanceFromCamera) const
	{
		Q_UNUSED(distanceFromCamera);
		return 0;
	}

	/**
	 * @ return The radius of the bounding sphere. See @ref
	 * BosonBigDisplayBase::sphereInFrustum
	 **/
	inline float boundingSphereRadius() const { return mBoundingSphereRadius; }

	BosonItem* item() const
	{
		return mItem;
	}


protected:
	int mAnimationMode;

protected:
	QColor teamColor() const;

	void setBoundingSphereRadius(float r) { mBoundingSphereRadius = r; }

	float itemSphereInFrustum(const BoFrustum& frustum) const;
	bool itemBoxInFrustum(const BoFrustum& frustum) const;

private:
	BosonItem* mItem;

	float mBoundingSphereRadius;
};

class BosonItemModelRenderer : public BosonItemRenderer
{
public:
	BosonItemModelRenderer(BosonItem* item);
	virtual ~BosonItemModelRenderer();

	virtual int rtti() const { return ModelRenderer; }

	virtual bool setModel(BosonModel* model);

	virtual inline BosonModel* model() const { return mModel; }

	/**
	 * Render the item. This assumes the modelview matrix was already
	 * translated and rotated to the correct position.
	 * @param lod See @ref BoFrame::renderFrame
	 **/
	virtual void renderItem(unsigned int lod = 0, bool transparentmeshes = false, RenderFlags flags = Default);

	/**
	 * Set the animation mode.
	 *
	 * See @ref BosonModel::animation for more information about @p mode.
	 **/
	virtual void setAnimationMode(int mode);

	/**
	 * Increase the animation timer and once it exceeds the @ref
	 * BoAnimation::speed set a new frame.
	 **/
	virtual void animate();

	/**
	 * @return See @ref BosonModel::lodCount.
	 **/
	unsigned int lodCount() const;

	/**
	 * @return See @ref BosonModel::PreferredLod
	 **/
	virtual unsigned int preferredLod(float distanceFromCamera) const;

	float currentFrame() const
	{
		return mCurrentFrame;
	}

private:
	BosonModel* mModel;
	BosonAnimation* mCurrentAnimation;
	float mCurrentFrame;

	// one matrix per mesh ("node") per frame in each lod
	Q3ValueVector< Q3ValueVector<const BoMatrix*> > mItemMatrices;

	unsigned int mMaxFramesInModel;
	Q3ValueVector<const BoFrame*> mFramePointers;
};

#endif

