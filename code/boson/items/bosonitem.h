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
#ifndef BOSONSPRITE_H
#define BOSONSPRITE_H

#include "../defines.h"

#include <GL/gl.h>

#include <qpointarray.h>

class BosonCanvas;
class SelectBox;
class BosonModel;
class BosonAnimation;
class BoFrame;
class QPointArray;
class QRect;


/**
 * This is the base class for all visual items, i.e. a OpenGL objects in boson.
 * Subclasses can be e.g. a missile object, units or mines.
 *
 * Every BosonItem must have a model assigned, which will be used to render the
 * object onto the screen. You also have to use a <em>unique</em> @ref rtti for
 * every item, i.e. a missile must have a different rtti than the units.
 *
 * Note that only <em>canvas</em> coordinates are stored in this class,
 * <em>not</em> OpenGL coordinates!
 * @short Base class for all visual OpenGL objects in boson.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonItem
{
public:
	/**
	 * Note: when you subclass this class you must set the width/height in
	 * order to make correct use of it! See @ref setSize
	 **/
	BosonItem(BosonModel* model, BosonCanvas*);
	virtual ~BosonItem();

	inline BosonCanvas* canvas() const { return mCanvas; }
	inline BosonModel* model() const { return mModel; }

	/**
	 * @return Unique identifier of this object type. E.g. RTTI::Unit for
	 * all units.
	 **/
	virtual int rtti() const = 0;

	/**
	 * @return The x-coordinate of the left edge of the object.Note that
	 * this has <em>nothing</em> to do with the OpenGL coordinates. 
	 * These are the internal canvas coordinates.
	 **/
	inline float x() const { return mX; }

	/**
	 * @return The y-coordinate of the unit on the canvas. Note that this is
	 * <em>not</em> the OpenGL coordinate! <em>NOT</em> canvas coordinates as
	 * @ref x() and @ref y() (acutally canvas and OpenGL coordiantes are
	 * equal for z)
	 **/
	inline float y() const { return mY; }

	/**
	 * @return The z-position of the unit. Currently 0.0 for all units,
	 * except flying units. Might be changed to the height of the ground one
	 * day
	 **/
	inline float z() const { return mZ; }

	/**
	 * @param width Width in cells * BO_TILE_SIZE. FIXME: should be changed to cell value (float!!)
	 * @param height Height in cells * BO_TILE_SIZE. FIXME: should be changed to cell value (float!!)
	 **/
	void setSize(int width, int height);

	// note: for GLunit all frames must have the same width/height!
	// different depth is ok!
	inline int width() const { return mWidth; }
	inline int height() const { return mHeight; }

	inline float leftEdge() const { return x(); }
	inline float topEdge() const { return y(); }
	inline float rightEdge() const { return (leftEdge() + width() - 1); }
	inline float bottomEdge() const { return (topEdge() + height() - 1); }

	QRect boundingRect() const;
	QRect boundingRectAdvanced() const;

	/**
	 * Move the item to @p nx, @p ny, @p nz. Note that it is moved without
	 * parameter checking, i.e. we don't check whether these cooridnates are
	 * valid.
	 **/
	inline void move(float nx, float ny, float nz)
	{
		moveBy(nx - x(), ny - y(), nz - z());
	}

	/**
	 * Move the item by the specified values. Note that no validity checking
	 * is done!
	 *
	 * Also note that when you move an item by e.g. (1,1,0) that this new
	 * position is <em>not</em> guaranteed to an actual position on the 
	 * ground. I mean when the item (e.g. a unit) is on grass and you move
	 * it by a certain amount so that it moves to another cell, then it is
	 * possible that that cell is at a different height than previous cell.
	 * You might end up with a "flying" unit or a unit that goes inside a
	 * mountain. You need to check that in your moving code!
	 *
	 * This is the central moving method of BosonItem, all other moving
	 * methods use this one. If you want to do special things (like validity
	 * checking) you should reimplement this method and call it in your
	 * implementation.
	 **/
	virtual void moveBy(float dx, float dy, float dz)
	{
		if (dx || dy || dz) {
			removeFromCells();
			setPos(x() + dx, y() + dy, z() + dz);
			addToCells();
		}
	}


	/**
	 * @return TRUE if the object is selected, i.e. a select box should be
	 * drawn around it. Otherwise FALSE.
	 **/
	inline bool isSelected() const { return mSelectBox != 0; }

	inline void setDisplayList(GLuint l)
	{
		mDisplayList = l;
	}

	inline GLuint displayList() const
	{
		return mDisplayList;
	}


	/**
	 * @return The select box of this item, or NULL if it is not selected.
	 * See @ref isSelected.
	 **/
	inline SelectBox* selectBox() const { return mSelectBox; }

	/**
	 * @return The (cell-)coordinates of the left-top cell this object
	 * occupies. This can be used for very efficient collision detection.
	 * See @ref rightBottomCell
	 **/
	inline void leftTopCell(int* left, int* top)  const
	{
		leftTopCell(left, top, leftEdge(), topEdge());
	}

	/**
	 * The same as the above version, but it isn't specific to a unit
	 * instance. You can use it to find out which cells a unit would occupy
	 * if it was at a certain position.
	 **/
	inline static void leftTopCell(int* left, int* top, float leftEdge, float topEdge)
	{
		*left = (int)(leftEdge / BO_TILE_SIZE);
		*top = (int)(topEdge / BO_TILE_SIZE);
	}

	/**
	 * @return The (cell-)coordinates of the lower-right cell this object
	 * occupies. This can be used for very efficient collision detection.
	 * See @ref leftTopCell
	 **/
	inline void rightBottomCell(int* right, int* bottom) const
	{
		rightBottomCell(right, bottom, rightEdge(), bottomEdge());
	}

	/**
	 * The same as the above version, but it isn't specific to a unit
	 * instance. You can use it to find out which cells a unit would occupy
	 * if it was at a certain position.
	 **/
	inline static void rightBottomCell(int* right, int* bottom, float rightEdge, float bottomEdge)
	{
		*right = (int)(rightEdge / BO_TILE_SIZE);
		*bottom= (int)(bottomEdge / BO_TILE_SIZE);
	}

	/**
	 * Note that this function caches the result and recalculates it only when
	 * item has moved - so usually it's not slow to call it.
	 * @return An array of all cells this unit occupies.
	 **/
	QPointArray cells();

	/**
	 * This is a more generic version of the above method. You can use it to
	 * calculate which cells the unit would occupy if it was at a certain
	 * position.
	 *
	 * WARNING: this method <em>will</em> return invalid cells in the point
	 * array, if you supply invalid parameters! It has no way to check
	 * whether left/right/top/bottom are still on the map! Use the
	 * overloaded method above to avoid this problem!
	 * @return An array of all cells this unit would occupy.
	 **/
	static QPointArray cells(int left, int right, int top, int bottom);

	bool bosonCollidesWith(BosonItem* item) const;

	inline float xVelocity() const { return mXVelocity; }
	inline float yVelocity() const { return mYVelocity; }
	inline float zVelocity() const { return mZVelocity; }
	void setVelocity(float vx, float vy, float vz = 0.0)
	{
		mXVelocity = vx;
		mYVelocity = vy;
		mZVelocity = vz;
	}

	/**
	 * Set the animation mode. Only possible if the construction of the unit
	 * is completed (i.e. the construction step is greater or equal to @ref
	 * glConstructionSteps).
	 *
	 * See @ref BosonModel::animation for more information about @p mode.
	 **/
	void setAnimationMode(int mode);


	/**
	 * Make this item animated, i.e. call @ref advance on it. You don't need
	 * to set it animated if you don't reimplement @ref advance. See also
	 * @ref BosonCanvas::addAnimation and @ref BosonCanvas::slotAdvance
	 **/
	void setAnimated(bool a);

	/**
	 * Increase the animation timer and once it exceeds the @ref
	 * BoAnimation::speed set a new frame.
	 **/
	void animate();

	/**
	 * See @ref Unit::advance
	 *
	 * This advance implementation manages the animation of the item. Call
	 * it in your implementation, if you want to provide animations!
	 **/
	inline virtual void advance(unsigned int )
	{
		animate();
	}

	/**
	 * See @ref Unit::advanceFunction
	 **/
	inline virtual void advanceFunction(unsigned int /*advanceCount*/) { }

	/**
	 * See @ref Unit::advanceFunction2
	 **/
	inline virtual void advanceFunction2(unsigned int /*advanceCount*/) { }

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

	void setGLConstructionStep(unsigned int step);

	/**
	 * Convenience method. Note that the glConstructionSteps differ from the
	 * @ref Facility::constructionSteps completely!
	 * @return model()->constructionSteps()
	 **/
	unsigned int glConstructionSteps() const;

	/**
	 * For OpenGL performance <em>only</em>! Do <em>not</em> use outside
	 * OpenGL! Especially not in pathfinding!
	 * @ return The radius of the bounding sphere. See @ref
	 * BosonBigDisplayBase::sphereInFrustum
	 **/
	inline float boundingSphereRadius() const { return mBoundingSphereRadius; }

	void setBoundingSphereRadius(float r) { mBoundingSphereRadius = r; }

	/**
	 * @return unit's current rotation around z-axis. This is used for rotating
	 * unit to correct direction when moving. Note that this value
	 * <em>mustn't</em> be used for pathfinding and similar actions, as of
	 * floating point operations!
	 **/
	inline float rotation() const { return mRotation; }
	void setRotation(float r) { mRotation = r; }

	inline float xRotation() const { return mXRotation; }
	void setXRotation(float r) { mXRotation = r; }

	inline float yRotation() const { return mYRotation; }
	void setYRotation(float r) { mYRotation = r; }


// TODO: add something like virtual bool canBeSelected() const = 0; or so! some
// objects (like missiles) just can't be selected usually.
	/**
	 * Select this unit, i.e. construct the select box - see @ref isSelected.
	 * Note that @ref SelectBox is constructed very fast - the box is
	 * constructed only once and the display list is stored as a static
	 * variable.
	 * @param markAsLeader The leader of a group of units/items will have a
	 * slightly different select box (e.g. another color or so).
	 **/
	virtual void select(bool markAsLeader = false);

	/**
	 * Unselect the unit, i.e. delete the select box.
	 **/
	virtual void unselect();

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

private:
	/**
	 * Change position of the item. WARNING: you <em>must</em> call @ref
	 * removeFromCells <em>before</em> calling this function!
	 *
	 * You should call @ref addToCells after this function.
	 **/
	inline void setPos(float x, float y, float z)
	{
		mX = x;
		mY = y;
		mZ = z;
		mCellsDirty = true;
	}

	void setCurrentFrame(BoFrame* frame);

	/**
	 * Add the item to the cells on the canvas. This should get called
	 * whenever the item has been moved in any way (i.e. also when its size
	 * was changed)
	 **/
	void addToCells();

	/**
	 * Remove the item from the cells it was added to. This <em>must</em>
	 * (really!!) be called before the item is moved or resized.
	 *
	 * Otherwise you'll most certainly get a crash at a later point.
	 **/
	void removeFromCells();

private:
	BosonCanvas* mCanvas;
	BosonModel* mModel;
	BosonAnimation* mCurrentAnimation;
	// FIXME: use KGameProperty here. We can do so, since we don't use
	// QCanvasSprite anymore.
	float mX;
	float mY;
	float mZ;
	int mWidth;
	int mHeight;

	float mXVelocity;
	float mYVelocity;
	float mZVelocity;

// OpenGL values. should not be used for pathfinding and so on. Most stoff
// shouldn't be stored in save() either
	float mRotation;
	float mXRotation;
	float mYRotation;
	float mGLDepthMultiplier;
	GLuint mDisplayList;
	unsigned int mGLConstructionStep;
	unsigned int mFrame;
	unsigned int mAnimationCounter;

	// these are for OpenGL performance only. no need to store in save() and
	// don't use for anything except OpenGL!
//	float mCenterX;
//	float mCenterY;
//	float mCenterZ;
	float mBoundingSphereRadius;


	bool mIsAnimated;
	SelectBox* mSelectBox;

	QPointArray mCells;
	bool mCellsDirty;
};

#endif

