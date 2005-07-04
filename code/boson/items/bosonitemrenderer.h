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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef BOSONITEMRENDERER_H
#define BOSONITEMRENDERER_H

#include "../defines.h"
#include <bogl.h>

#include <qglobal.h>

class BosonModel;
class BosonAnimation;
class BoFrame;
class Player;
class BosonItem;
class BoFrustum;

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
	 * For OpenGL performance <em>only</em>! Do <em>not</em> use outside
	 * OpenGL! Especially not in pathfinding!
	 * @ return The radius of the bounding sphere. See @ref
	 * BosonBigDisplayBase::sphereInFrustum
	 **/
	inline float boundingSphereRadius() const { return mBoundingSphereRadius; }

	void setBoundingSphereRadius(float r) { mBoundingSphereRadius = r; }

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
	virtual void renderItem(unsigned int lod = 0, bool transparentmeshes = false);

	/**
	 * @return TRUE if this item is in the @p frustum, otherwise FALSE.
	 **/
	bool itemInFrustum(const BoFrustum& frustum) const;

	BosonItem* item() const
	{
		return mItem;
	}

protected:
	const QColor* teamColor() const;

private:
	BosonItem* mItem;

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
	virtual void renderItem(unsigned int lod = 0, bool transparentmeshes = false);

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

private:
	BosonModel* mModel;
	BosonAnimation* mCurrentAnimation;
	float mCurrentFrame;
};

#endif

