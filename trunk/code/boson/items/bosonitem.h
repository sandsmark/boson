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
#ifndef BOSONITEM_H
#define BOSONITEM_H

#include "../defines.h"

#include <GL/gl.h>

#include <qglobal.h>

class BosonCanvas;
class BosonCollisions;
class SelectBox;
class BosonModel;
class BosonAnimation;
class Cell;
class BosonEffect;
class BosonItemPropertyHandler;
class Player;
class BoVector3;
class BosonItemRenderer;
class BoVector2;
class BoRect;

class KGamePropertyHandler;
class KGamePropertyBase;
class QColor;
template<class T> class QPtrList;
template<class T> class QPtrVector;
template<class T1, class T2> class QMap;
class QDomElement;
class QDataStream;
class QString;

/**
 * This is the base class of @ref BosonItem. It exists only to logically
 * separate the code that provides access to @ref KGamePropertyHandler and
 * friends from all other code.
 *
 * You will most probably want to look at @ref BosonItem instead.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonItemProperties
{
public:
	BosonItemProperties();
	virtual ~BosonItemProperties();

	virtual int rtti() const = 0;

	/**
	 * Initialize static members
	 **/
	static void initStatic();

	/**
	 * Add a property ID to the list of properties. This must be done before
	 * calling @ref registerData.
	 * @param id A <em>unique</em> property ID - you must ensure that one
	 * unit never uses two identical ids.
	 * @param name The name of the property. Will be used in the debug
	 * dialog as well as e.g. in the scenario files. This name must be
	 * unique as well!
	 **/
	static void addPropertyId(int id, const QString& name);

	/**
	 * @return The id of the specified property name. Or -1 if not found.
	 * See @ref addPropertyId.
	 **/
	static int propertyId(const QString& name);

	/**
	 * @return A name for the specified property id or QString::null if not
	 * found. See also @ref addPropertyId
	 **/
	static QString propertyName(int id);

	/**
	 * Shortcut for
	 * <pre>
	 * prop->registerData(id, dataHandler(), KGamePropertyBase::PolicyLocal,
	 * propertyName(id));
	 * </pre>
	 *
	 * Note that you must call @ref addPropertyId before you are able to use
	 * registerData!
	 * @param prop The @ref KGamePropertyBase to be registered
	 * @param id The PropertyId for the @ref KGamePropertyBase. This must be
	 * unique for every property, i.e. a unit must never have two identical
	 * property ids.
	 * @param local If TRUE use @ref KGamePropertyBase::PolicyLocal,
	 * otherwise @ref KGamePropertyBase::PolicyClean. Don't use FALSE here
	 * unless you know what you're doing!
	 **/
	void registerData(KGamePropertyBase* prop, int id, bool local = true);

	/**
	 * @return The @ref KGamePropertyHandler for all the properties of this
	 * item. It is in fact a @ref BosonItemPropertyHandler, you can cast to
	 * it safely.
	 **/
	KGamePropertyHandler* dataHandler() const;

private:
	BosonItemPropertyHandler* mProperties;
	static QMap<int, QString>* mPropertyMap;

};


/**
 * This is the base class for all visual items, i.e. a OpenGL objects in boson.
 * Subclasses can be e.g. a missile objects, units or mines.
 *
 * Every BosonItem should have a model assigned, which will be used to render
 * the object onto the screen. If you don't want the item to be rendered or if
 * you don't have a model, you can use @ref setVisible to prevent item from
 * being rendered
 * You also have to use a <em>unique</em> @ref rtti for every item, i.e. a
 * missile must have a different rtti than the units.
 *
 * Note that only <em>canvas</em> coordinates are stored in this class,
 * <em>not</em> OpenGL coordinates!
 * @short Base class for all visual OpenGL objects in boson.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonItem : public BosonItemProperties
{
public:
	/**
	 * Note: when you subclass this class you must set the width/height in
	 * order to make correct use of it! See @ref setSize
	 **/
	BosonItem(Player* owner, BosonCanvas*);
	virtual ~BosonItem();

	/**
	 * @return The model that should be used for this item. This model will
	 * get assigned to this model after construction using @ref
	 * BosonItemRenderer::setModel.
	 **/
	virtual BosonModel* getModelForItem() const = 0;

	/**
	 * Called after the constructor. You can do all kinds of initializations
	 * here - try to use this instead of the constructor whenever possible.
	 *
	 * This is in particular useful when you need to ensure that a certain
	 * condition applies, e.g. that canvas() is non-NULL. You can simply
	 * return FALSE in that case and the item will be deleted.
	 *
	 * @return TRUE on success or FALSE on failure (the item will be deleted
	 * then)
	 **/
	virtual bool init() { return true; }

	BosonItemRenderer* itemRenderer() const { return mRenderer; }
	inline BosonCanvas* canvas() const { return mCanvas; }
	BosonCollisions* collisions() const;
	inline Player* owner() const { return mOwner; }


	/**
	 * @return Unique identifier of this object type. E.g. RTTI::Unit for
	 * all units.
	 **/
	virtual int rtti() const = 0;

	/**
	 * Set a unique Id for this item. The Id <em>must</em> be unique for
	 * <em>all</em> items in the game. Otherwise the results are undefined.
	 **/
	void setId(unsigned long int id) { mId = id; }

	/**
	 * @return An id that identifies this item uniquely. There are never 2
	 * different items with the same Id.
	 **/
	inline unsigned long int id() const { return mId; }

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
	 * @return The z-position of the item.
	 **/
	inline float z() const { return mZ; }

	/**
	 * @param width Width in cells
	 * @param height Height in cells
	 **/
	void setSize(float width, float height, float depth);

	// note: for GLunit all frames must have the same width/height!
	// different depth is ok!
	inline float width() const { return mWidth; }
	inline float height() const { return mHeight; }
	/**
	 * @return item's height in z-direction.
	 * This does not depend on any OpenGL stuff (model, frame etc) and should be
	 * same on all clients, so it can be used for collision detection and
	 * pathfinding
	 **/
	inline float depth() const { return mDepth; }

	inline float leftEdge() const { return x(); }
	inline float topEdge() const { return y(); }
	inline float rightEdge() const { return leftEdge() + width(); }
	inline float bottomEdge() const { return topEdge() + height(); }

	inline float centerX() const { return x() + width() / 2.0f; };
	inline float centerY() const { return y() + height() / 2.0f; };
	BoVector2 center() const;

	BoRect boundingRect() const;
	BoRect boundingRectAdvanced() const;

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
			setEffectsPosition(x(), y(), z());
		}
	}


	/**
	 * @return TRUE if the object is selected, i.e. a select box should be
	 * drawn around it. Otherwise FALSE.
	 **/
	bool isSelected() const { return mSelectBox != 0; }

	/**
	 * Render the item. This assumes the modelview matrix was already
	 * translated and rotated to the correct position.
	 * @param lod See @ref BoFrame::renderFrame
	 **/
	void renderItem(unsigned int lod = 0);

	/**
	 * @return itemRenderer()->preferredLod(dist)
	 **/
	unsigned int preferredLod(float dist) const;

	/**
	 * @return itemRenderer()->glDepthMultiplier()
	 **/
	float glDepthMultiplier() const;

	/**
	 * Tell the @ref itemRenderer to operate in editor mode, e.g. display
	 * non-constructed facilities in constructed state
	 **/
	void setRendererToEditorMode();

	bool initItemRenderer();

	/**
	 * @return TRUE if the item is in @p frustum, otherwise FALSE
	 **/
	bool itemInFrustum(const float* frustum) const;

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
		*left = (int)(leftEdge);
		*top = (int)(topEdge);
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
		*right = (int)(rightEdge);
		*bottom= (int)(bottomEdge);
	}

	/**
	 * Note that this function caches the result and recalculates it only when
	 * item has moved - so usually it's not slow to call it.
	 * @return An array of all cells this unit occupies.
	 **/
	QPtrVector<Cell>* cells();

	/**
	 * This does <em>not</em> recalculate the cells, as @ref cells does when
	 * the data is not valid anymore. It can be used when you need a const
	 * function where you don't have to depend on the data to be current (do
	 * not use in collision detection or pathfinder code! rather for
	 * tooltips and that kind)
	 **/
	QPtrVector<Cell>* cellsConst() const;

	/**
	 * This is a more generic version of the above method. You can use it to
	 * calculate which cells the unit would occupy if it was at a certain
	 * position.
	 **/
	static void makeCells(Cell* allCells, QPtrVector<Cell>* cells, const BoRect& rect, int mapWidth, int mapHeight);

	/**
	 * @return Whether this unit collides with given unit.
	 * It uses width, height and depth of the unit for accurate collision
	 * detection.
	 **/
	bool bosonCollidesWith(BosonItem* item) const;

	/**
	 * Same as above, but uses box with given coords instead of actual item.
	 **/
	bool bosonCollidesWith(const BoVector3& v1, const BoVector3& v2) const;

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
	 * @return Current speed of this item
	 **/
	inline float speed() const { return mCurrentSpeed; }
	inline void setSpeed(float s) { mCurrentSpeed = s; }
	/**
	 * @return Maximum speed this item may have
	 **/
	inline float maxSpeed() const { return mMaxSpeed; }
	inline void setMaxSpeed(float maxspeed) { mMaxSpeed = maxspeed; }
	/**
	 * Raises speed by @ref accelerationSpeed unless @ref currentSpeed is
	 * @ref maxSpeed
	 **/
	inline void accelerate() { mCurrentSpeed = QMIN(maxSpeed(), speed() + accelerationSpeed()); }
	/**
	 * Lowers speed by @ref decelerationSpeed unless @ref currentSpeed is 0
	 **/
	inline void decelerate() { mCurrentSpeed = QMAX(0, speed() - decelerationSpeed()); }
	/**
	 * @return How fast this unit accelerates.
	 * Acceleration speed shows how much speed of unit changes per advance call.
	 **/
	inline float accelerationSpeed() const { return mAccelerationSpeed; }
	inline void setAccelerationSpeed(float s) { mAccelerationSpeed = s; }
	/**
	 * @return How fast this unit decelerates.
	 * Deceleration speed shows how much speed of unit changes per advance call.
	 **/
	inline float decelerationSpeed() const { return mDecelerationSpeed; }
	inline void setDecelerationSpeed(float s) { mDecelerationSpeed = s; }
	/**
	 * @return How much this unit moves before stopping completely
	 * This is distance that item will move before it completely stops when it
	 * starts deceleration now and continues it until stopping
	 **/
	inline float decelerationDistance() const { return (mCurrentSpeed / mDecelerationSpeed) / 2 * mCurrentSpeed; }


	inline void setVisible(bool v) { mIsVisible = v; }
	inline bool isVisible() const { return mIsVisible; }

	/**
	 * See @ref Unit::advance
	 *
	 * This advance implementation manages the animation of the item. Call
	 * it in your implementation, if you want to provide animations!
	 **/
	virtual void advance(unsigned int );

	/**
	 * See @ref Unit::advanceFunction
	 **/
	inline virtual void advanceFunction(unsigned int /*advanceCallCount*/) { }

	/**
	 * See @ref Unit::advanceFunction2
	 **/
	inline virtual void advanceFunction2(unsigned int /*advanceCallCount*/) { }

	/**
	 * Used to synchronize the advance function for the next advance call.
	 * See @ref Unit::syncAdvanceFunction.
	 *
	 * DO NOT CALL THIS unless you REALLY know what youre doing! Call it
	 * from @ref BosonCanvas::slotAdvance ONLY!
	 **/
	inline virtual void syncAdvanceFunction() { }

	/**
	 * Used to synchronize the advance function for the next advance call.
	 * See @ref Unit::syncAdvanceFunction.
	 **/
	inline virtual void syncAdvanceFunction2() { }

	/**
	 * Called when @p item is removed from the canvas (right before the item
	 * is deleted).
	 * If this item stores any references to @p item, it should remove them
	 * NOW.
	 *
	 * The default implementation does nothing
	 **/
	virtual void itemRemoved(BosonItem* item) { Q_UNUSED(item); }

	/**
	 * @return unit's current rotation around z-axis. This is used for rotating
	 * unit to correct direction when moving.
	 **/
	inline float rotation() const { return mRotation; }
	void setRotation(float r) { mRotation = r; setEffectsRotation(mXRotation, mYRotation, mRotation); }

	inline float xRotation() const { return mXRotation; }
	void setXRotation(float r) { mXRotation = r; setEffectsRotation(mXRotation, mYRotation, mRotation); }

	inline float yRotation() const { return mYRotation; }
	void setYRotation(float r) { mYRotation = r; setEffectsRotation(mXRotation, mYRotation, mRotation); }


	void setEffectsPosition(float x, float y, float z);
	void setEffectsRotation(float xrot, float yrot, float zrot);

	/**
	 * @return List of active effects this item has.
	 * This may include e.g. smoke for factories.
	 **/
	virtual const QPtrList<BosonEffect>* effects() const;
	virtual void setEffects(const QPtrList<BosonEffect>& effects, bool addtocanvas = true);
	virtual void addEffect(BosonEffect* e, bool addtocanvas = true);

	/**
	 * Clear the effects list. Note that the effects are
	 * <em>not</em> deleted - @ref BosonCanvas will take care of this
	 * anyway. Just the @ref effects list is meant to be cleared.
	 **/
	virtual void clearEffects();

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

	virtual bool saveAsXML(QDomElement&);
	virtual bool loadFromXML(const QDomElement&);

	/**
	 * @return The team color this item should get rendered with. This
	 * should be the @ref Player::teamColor of the owner, if applicable. For
	 * some items you might want to return simply NULL (which is perfectly
	 * valid)
	 **/
	virtual const QColor* teamColor() const = 0;

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
	BosonItemRenderer* mRenderer;
	BosonCanvas* mCanvas;
	Player* mOwner;
	// FIXME: use KGameProperty here. We can do so, since we don't use
	// QCanvasSprite anymore.
	unsigned long int mId;
	float mX;
	float mY;
	float mZ;
	float mWidth;
	float mHeight;
	float mDepth;

	float mXVelocity;
	float mYVelocity;
	float mZVelocity;

	float mCurrentSpeed;
	float mMaxSpeed;
	float mAccelerationSpeed;
	float mDecelerationSpeed;

	float mRotation;
	float mXRotation;
	float mYRotation;

	bool mIsAnimated;
	SelectBox* mSelectBox;

	QPtrVector<Cell>* mCells;
	bool mCellsDirty;
	bool mIsVisible;

	QPtrList<BosonEffect>* mEffects;
};

#endif

