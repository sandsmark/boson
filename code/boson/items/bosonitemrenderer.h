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
class BoVector3;
class BosonItem;

class QColor;
class QString;

class BosonItemRenderer
{
public:
	BosonItemRenderer(BosonItem* item);
	virtual ~BosonItemRenderer();

	bool setModel(BosonModel* model);

	inline BosonModel* model() const { return mModel; }

	/**
	 * Render the item. This assumes the modelview matrix was already
	 * translated and rotated to the correct position.
	 * @param lod See @ref BoFrame::renderFrame
	 **/
	void renderItem(unsigned int lod = 0);

	/**
	 * Set the animation mode. Only possible if the construction of the unit
	 * is completed (i.e. the construction step is greater or equal to @ref
	 * glConstructionSteps).
	 *
	 * See @ref BosonModel::animation for more information about @p mode.
	 **/
	void setAnimationMode(int mode);

	/**
	 * Increase the animation timer and once it exceeds the @ref
	 * BoAnimation::speed set a new frame.
	 **/
	void animate();

	/**
	 * @return The factor you need to multiply BO_GL_CELL_SIZE with to
	 * achieve the depth (height in z-direction) of the unit. Note that this
	 * value <em>must not</em> be used in pathfinding or so, but only in
	 * OpenGL!
	 **/
	inline float glDepthMultiplier() const { return mGLDepthMultiplier; }

	/**
	 * See @ref glDepthMultiplier. Note that the depth of the unit
	 * (i.e. the height in z-direction) is allowed to change for
	 * different frames, but the depth multiplier not!
	 **/
	void setGLDepthMultiplier(float d);

	/**
	 * Set the @ref BosonModel construction step that corresponds to the @p
	 * unitConstructionStep in the item.
	 * @param unitConstructionStep The current construction step of the @ref
	 * Facility , i.e. @ref Facility::currentConstructionStep.
	 * @param totalUnitConstructionStep The total number of construction
	 * steps that the facility has, i.e. @ref Facility::constructionSteps.
	 **/
	void setGLConstructionStep(unsigned int unitConstructionStep, unsigned int totalUnitConstructionSteps)
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

	void setShowGLConstructionSteps(bool show);
	bool showGLConstructionSteps() const
	{
		return mShowGLConstructionSteps;
	}

	/**
	 * For OpenGL performance <em>only</em>! Do <em>not</em> use outside
	 * OpenGL! Especially not in pathfinding!
	 * @ return The radius of the bounding sphere. See @ref
	 * BosonBigDisplayBase::sphereInFrustum
	 **/
	inline float boundingSphereRadius() const { return mBoundingSphereRadius; }

	void setBoundingSphereRadius(float r) { mBoundingSphereRadius = r; }

	/**
	 * @return See @ref BosonModel::lodCount.
	 **/
	unsigned int lodCount() const;

	/**
	 * @return See @ref BosonModel::PreferredLod
	 **/
	unsigned int preferredLod(float distanceFromCamera) const;

	/**
	 * @return TRUE if this item is in the @p frustum, otherwise FALSE.
	 **/
	bool itemInFrustum(const float* frustum) const;

protected:
	const QColor* teamColor() const;

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
	BosonItem* mItem;
	BosonModel* mModel;
	BosonAnimation* mCurrentAnimation;

	float mGLDepthMultiplier;
	BoFrame* mCurrentFrame;
	bool mShowGLConstructionSteps;
	unsigned int mGLConstructionStep;
	unsigned int mFrame;
	unsigned int mAnimationCounter;

	float mBoundingSphereRadius;
};

#endif

