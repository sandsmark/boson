/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef GLSPRITE_H
#define GLSPRITE_H

#include "../defines.h"

#include <GL/gl.h>

class BosonCanvas;
class BosonModel;
class BoFrame;
class QRect;

class GLSprite
{
public:
	GLSprite(BosonModel* mode, BosonCanvas*);
	virtual ~GLSprite();

	BosonCanvas* canvas() const { return mCanvas; }

	/**
	 * @return Exactly the same as @ref leftEdge. Note that it has
	 * <em>nothing</em> to do with the OpenGL coordinates. these are the
	 * internal canvas coordinates.
	 **/
	inline float x() const { return mX; }
	inline float y() const { return mY; }
	inline float z() const { return mZ; }

	inline BosonModel* model() const { return mModel; }

	// note: for GLunit all frames must have the same width/height!
	// different depth is ok!
	inline int width() const { return mWidth; }
	inline int height() const { return mHeight; }

	/**
	 * @return The factor you need to multiply BO_GL_CELL_SIZE with to
	 * achieve the depth (height in z-direction) of the unit. Note that this
	 * value <em>must not</em> be used in pathfinding or so, but only in
	 * OpenGL!
	 **/
	inline float glDepthMultiplier() const { return mGLDepthMultiplier; }

	void setWidth(int w); // width in cells * BO_TILE_SIZE // FIXME: should be changed to cell value (float!!)
	void setHeight(int w); // height in cells * BO_TILE_SIZE // FIXME: should be changed to cell value (float!!)
	/**
	 * See @ref glDepthMultiplier. Note that the depth of the unit
	 * (i.e. the height in z-direction) is allowed to change for
	 * different frames!
	 **/
	void setGLDepthMultiplier(float d);

	void setGLConstructionStep(unsigned int step);

	inline void setDisplayList(GLuint l)
	{
		mDisplayList = l;
	}


	float leftEdge() const { return x(); }
	float topEdge() const { return y(); } // note: in OpenGL the top is y-coordinate + height!!
	float rightEdge() const { return (leftEdge() + width() - 1); }
	float bottomEdge() const { return (topEdge() + height() - 1); }
	QRect boundingRect() const;
	QRect boundingRectAdvanced() const;


	inline void move(float nx, float ny, float nz)
	{
		moveBy(nx - x(), ny - y(), nz - z());
	}
	virtual void moveBy(float dx, float dy, float dz)
	{
		// in Unit we MUST add a addToCells() / removeFromCells() !!
		if (dx || dy || dz) {
			setX(x() + dx);
			setY(y() + dy);
			setZ(z() + dz);
		}
	}

	void setFrame(int _frame);
	inline int frame() const { return mFrame; }
	unsigned int frameCount() const;

	inline float xVelocity() const { return mXVelocity; }
	inline float yVelocity() const { return mYVelocity; }
	void setXVelocity(float vx) { mXVelocity = vx; }
	void setYVelocity(float vy) { mYVelocity = vy; }
	void setVelocity(float vx, float vy) 
	{
		mXVelocity = vx;
		mYVelocity = vy;
	}

	inline GLuint displayList() const
	{
		return mDisplayList;
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
	 * Returns unit's current rotation around z-axis. This is used for rotating
	 * unit to correct direction when moving
	 **/
	inline float rotation() const { return mRotation; }

	inline void setRotation(float r) { mRotation = r; }

private:
	inline void setX(float x) { mX = x; }
	inline void setY(float y) { mY = y; }
	inline void setZ(float z) { mZ = z; }

	void setCurrentFrame(BoFrame* frame);

private:
	// FIXME: use KGameProperty here. We can do so, since we don't use
	// QCanvasSprite anymore.
	BosonCanvas* mCanvas;
	float mX;
	float mY;
	float mZ;
	int mWidth;
	int mHeight;
	float mRotation;

	float mGLDepthMultiplier;
	GLuint mDisplayList;
	unsigned int mFrame;
	unsigned int mGLConstructionStep;

	float mXVelocity;
	float mYVelocity;

	BosonModel* mModel;

	// these are for OpenGL performance only. no need to store on save() and
	// don't use for anything except OpenGL!
//	float mCenterX;
//	float mCenterY;
//	float mCenterZ;
	float mBoundingSphereRadius;
};

#endif // GLUNIT_H

