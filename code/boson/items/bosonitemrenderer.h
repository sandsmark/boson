/*
    This file is part of the Boson game
    Copyright (C) 2002-2004 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef BOSONITEMRENDERER_H
#define BOSONITEMRENDERER_H

#include "../defines.h"

#include <GL/gl.h>

#include <qglobal.h>

class BosonModel;
class BosonAnimation;
class BoFrame;
class Player;
class BosonItem;

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
	BosonItemRenderer(BosonItem* item);
	virtual ~BosonItemRenderer();

	static void startItemRendering();
	static void stopItemRendering();

	virtual bool setModel(BosonModel*) { return true; }

	/**
	 * See @ref glDepthMultiplier. Note that the depth of the unit
	 * (i.e. the height in z-direction) is allowed to change for
	 * different frames, but the depth multiplier not!
	 **/
	void setGLDepthMultiplier(float d);

	/**
	 * @return The factor you need to multiply 1.0 with to
	 * achieve the depth (height in z-direction) of the unit. Note that this
	 * value <em>must not</em> be used in pathfinding or so, but only in
	 * OpenGL!
	 **/
	inline float glDepthMultiplier() const { return mGLDepthMultiplier; }

	/**
	 * For OpenGL performance <em>only</em>! Do <em>not</em> use outside
	 * OpenGL! Especially not in pathfinding!
	 * @ return The radius of the bounding sphere. See @ref
	 * BosonBigDisplayBase::sphereInFrustum
	 **/
	inline float boundingSphereRadius() const { return mBoundingSphereRadius; }

	void setBoundingSphereRadius(float r) { mBoundingSphereRadius = r; }

	virtual void setShowGLConstructionSteps(bool s)
	{
		mShowGLConstructionSteps = s;
	}
	bool showGLConstructionSteps() const
	{
		return mShowGLConstructionSteps;
	}

	virtual void setGLConstructionStep(unsigned int unitConstructionStep, unsigned int totalUnitConstructionSteps)
	{
		Q_UNUSED(unitConstructionStep);
		Q_UNUSED(totalUnitConstructionSteps);
	}

	virtual unsigned int preferredLod(float distanceFromCamera) const
	{
		Q_UNUSED(distanceFromCamera);
		return 0;
	}
	
	virtual void setAnimationMode(int ) { }

	virtual void animate() { }

	/**
	 * Render the item. This assumes the modelview matrix was already
	 * translated and rotated to the correct position.
	 * @param lod See @ref BoFrame::renderFrame
	 **/
	virtual void renderItem(unsigned int lod = 0);

	/**
	 * @return TRUE if this item is in the @p frustum, otherwise FALSE.
	 **/
	bool itemInFrustum(const float* frustum) const;

protected:
	const QColor* teamColor() const;

private:
	BosonItem* mItem;

	bool mShowGLConstructionSteps;
	float mGLDepthMultiplier;

	float mBoundingSphereRadius;
};

class BosonItemModelRenderer : public BosonItemRenderer
{
public:
	BosonItemModelRenderer(BosonItem* item);
	virtual ~BosonItemModelRenderer();

	virtual bool setModel(BosonModel* model);

	inline BosonModel* model() const { return mModel; }

	/**
	 * Render the item. This assumes the modelview matrix was already
	 * translated and rotated to the correct position.
	 * @param lod See @ref BoFrame::renderFrame
	 **/
	virtual void renderItem(unsigned int lod = 0);

	/**
	 * Set the animation mode. Only possible if the construction of the unit
	 * is completed (i.e. the construction step is greater or equal to @ref
	 * glConstructionSteps).
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
	 * Set the @ref BosonModel construction step that corresponds to the @p
	 * unitConstructionStep in the item.
	 * @param unitConstructionStep The current construction step of the @ref
	 * Facility , i.e. @ref Facility::currentConstructionStep.
	 * @param totalUnitConstructionStep The total number of construction
	 * steps that the facility has, i.e. @ref Facility::constructionSteps.
	 **/
	virtual void setGLConstructionStep(unsigned int unitConstructionStep, unsigned int totalUnitConstructionSteps)
	{
		unsigned int modelStep = 0;
		if (unitConstructionStep >= totalUnitConstructionSteps) {
			modelStep = glConstructionSteps();
		} else {
			modelStep = glConstructionSteps() * unitConstructionStep / totalUnitConstructionSteps;
		}
		setGLConstructionStep(modelStep);
	}

	/**
	 * Convenience method. Note that the glConstructionSteps differ from the
	 * @ref Facility::constructionSteps completely!
	 * @return model()->constructionSteps()
	 **/
	unsigned int glConstructionSteps() const;

	virtual void setShowGLConstructionSteps(bool show);

	/**
	 * @return See @ref BosonModel::lodCount.
	 **/
	unsigned int lodCount() const;

	/**
	 * @return See @ref BosonModel::PreferredLod
	 **/
	virtual unsigned int preferredLod(float distanceFromCamera) const;

private:
	/**
	 * Change the currently displayed frame. Note that you can't set the
	 * construction frames here, as they are generated on the fly and don't
	 * reside as an actual frame in the .3ds file.
	 *
	 * You usually don't want to call this, but rather @ref setAnimationMode
	 * instead.
	 **/
	void setFrame(int _frame);
	inline int frame() const { return mFrame; }
	unsigned int frameCount() const;
	void setGLConstructionStep(unsigned int step);

private:
	void setCurrentFrame(BoFrame* frame);

private:
	BosonModel* mModel;
	BosonAnimation* mCurrentAnimation;

	BoFrame* mCurrentFrame;
	unsigned int mGLConstructionStep;
	unsigned int mFrame;
	unsigned int mAnimationCounter;

};

#endif

