/*
    This file is part of the Boson game
    Copyright (C) 2002-2005 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2002-2005 Rivo Laks (rivolaks@hot.ee)

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
#ifndef BOSONITEM_H
#define BOSONITEM_H

#include "../defines.h"
#include "../bomath.h"
#include <bogl.h>

#include <qglobal.h>
//Added by qt3to4:
#include <Q3PtrList>

class BosonCanvas;
class BosonCollisions;
class BosonModel;
class BosonAnimation;
class Cell;
class BosonItemPropertyHandler;
class Player;
class SpeciesTheme;
template<class T> class BoVector2;
template<class T> class BoVector3;
template<class T> class BoRect2;
typedef BoRect2<bofixed> BoRect2Fixed;
typedef BoVector2<bofixed> BoVector2Fixed;
typedef BoVector3<bofixed> BoVector3Fixed;
class BoFrustum;

class KGamePropertyHandler;
class KGamePropertyBase;
class QColor;
template<class T> class Q3PtrList;
template<class T> class Q3PtrVector;
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
	 * @return A name for the specified property id or QString() if not
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
	enum PropertyIds {
		// BosonItem used IDs from 256 to 511.
		// all values above 511 are reserved for derived classes, up to
		// 28671 (28672 and greater might be used by KGame again for
		// automatic IDs)
	};

	/**
	 * Note: when you subclass this class you must set the width/height in
	 * order to make correct use of it! See @ref setSize
	 **/
	BosonItem(Player* owner, BosonCanvas*);
	virtual ~BosonItem();

	/**
	 * @return A string that identifies the model that should be used for
	 * this item. This model will get assigned to this model after
	 * construction using @ref BosonItemRenderer::setModel.
	 **/
	virtual QString getModelIdForItem() const = 0;

	/**
	 * Update the current animation mode using @ref getAnimationMode
	 **/
	void updateAnimationMode();

	/**
	 * @return The current animation mode, see also @ref
	 * updateAnimationMode.
	 **/
	int animationMode() const
	{
		return mAnimationMode;
	}

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

	inline BosonCanvas* canvas() const { return mCanvas; }
	BosonCollisions* collisions() const;
	inline Player* owner() const { return mOwner; }

	/**
	 * @return owner()->speciesTheme()
	 **/
	SpeciesTheme* speciesTheme() const;


	/**
	 * @return Unique identifier of this object type. E.g. RTTI::Unit for
	 * all units.
	 **/
	virtual int rtti() const = 0;

	/**
	 * Set a unique Id for this item. The Id <em>must</em> be unique for
	 * <em>all</em> items in the game. Otherwise the results are undefined.
	 **/
	void setId(quint32 id) { mId = id; }

	/**
	 * @return An id that identifies this item uniquely. There are never 2
	 * different items with the same Id.
	 **/
	inline quint32 id() const { return mId; }

	// TODO: change semantics of x() and y(): they should return centerX()
	// and centerY()!
#if 0
	inline bofixed x() const { return mX; }
	inline bofixed y() const { return mY; }
#endif

	/**
	 * @return The z-position of the item.
	 **/
	// TODO: currently z() is the "bottom" of the item.
	//       we probably should use centerZ() instead!
	//       -> (x,y,z) would then be exactly the center point of an item
	inline bofixed z() const { return mZ; }

	/**
	 * @param width Width in cells
	 * @param height Height in cells
	 **/
	void setSize(bofixed width, bofixed height, bofixed depth);

	// note: for GLunit all frames must have the same width/height!
	// different depth is ok!
	inline bofixed width() const { return mWidth; }
	inline bofixed height() const { return mHeight; }
	/**
	 * @return item's height in z-direction.
	 * This does not depend on any OpenGL stuff (model, frame etc) and should be
	 * same on all clients, so it can be used for collision detection and
	 * pathfinding
	 **/
	inline bofixed depth() const { return mDepth; }

	inline bofixed leftEdge() const { return mX - width() / 2; }
	inline bofixed topEdge() const { return mY - height() / 2; }
	inline bofixed rightEdge() const { return leftEdge() + width(); }
	inline bofixed bottomEdge() const { return topEdge() + height(); }

	inline bofixed centerX() const { return mX; }
	inline bofixed centerY() const { return mY; }
	inline bofixed centerZ() const { return z() + depth() / 2; };
	BoVector2Fixed center() const;

	BoRect2Fixed boundingRect() const;
	BoRect2Fixed boundingRectAdvanced() const;

	/**
	 * Move the item to @p nx, @p ny, @p nz. Note that it is moved without
	 * parameter checking, i.e. we don't check whether these cooridnates are
	 * valid.
	 **/
	inline void moveLeftTopTo(bofixed nx, bofixed ny, bofixed nz)
	{
		moveBy(nx - leftEdge(), ny - topEdge(), nz - z());
	}
	inline void moveCenterTo(bofixed nx, bofixed ny, bofixed nz) // FIXME: nz is still NOT the center (see z())
	{
		moveBy(nx - centerX(), ny - centerY(), nz - z());
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
	virtual void moveBy(bofixed dx, bofixed dy, bofixed dz)
	{
		if (dx || dy || dz) {
			itemAboutToMove(dx, dy, dz);
			mX += dx;
			mY += dy;
			mZ += dz;
			itemHasMoved(dx, dy, dz);
		}
	}


	/**
	 * @return TRUE if the object is selected, i.e. a select box should be
	 * drawn around it. Otherwise FALSE.
	 **/
	bool isSelected(bool* isGroupLeader = 0) const
	{
		if (isGroupLeader) {
			*isGroupLeader = mIsGroupLeaderOfSelection;
		}
		return mIsSelected;
	}

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
	inline static void leftTopCell(int* left, int* top, bofixed leftEdge, bofixed topEdge)
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
	inline static void rightBottomCell(int* right, int* bottom, bofixed rightEdge, bofixed bottomEdge)
	{
		*right = (int)(rightEdge);
		*bottom= (int)(bottomEdge);
	}

	/**
	 * Note that this function caches the result and recalculates it only when
	 * item has moved - so usually it's not slow to call it.
	 * @return An array of all cells this unit occupies.
	 **/
	Q3PtrVector<Cell>* cells();

	/**
	 * This does <em>not</em> recalculate the cells, as @ref cells does when
	 * the data is not valid anymore. It can be used when you need a const
	 * function where you don't have to depend on the data to be current (do
	 * not use in collision detection or pathfinder code! rather for
	 * tooltips and that kind)
	 **/
	Q3PtrVector<Cell>* cellsConst() const;

	/**
	 * This is a more generic version of the above method. You can use it to
	 * calculate which cells the unit would occupy if it was at a certain
	 * position.
	 **/
	static void makeCells(Cell* allCells, Q3PtrVector<Cell>* cells, const BoRect2<bofixed>& rect, int mapWidth, int mapHeight);

	/**
	 * @return Whether this unit collides with given unit.
	 * It uses width, height and depth of the unit for accurate collision
	 * detection.
	 **/
	bool bosonCollidesWith(BosonItem* item) const;

	/**
	 * Same as above, but uses box with given coords instead of actual item.
	 **/
	bool bosonCollidesWith(const BoVector3Fixed& v1, const BoVector3Fixed& v2) const;

	inline bofixed xVelocity() const { return mXVelocity; }
	inline bofixed yVelocity() const { return mYVelocity; }
	inline bofixed zVelocity() const { return mZVelocity; }
	void setVelocity(bofixed vx, bofixed vy, bofixed vz = 0)
	{
		mXVelocity = vx;
		mYVelocity = vy;
		mZVelocity = vz;
	}

	/**
	 * @return Current speed of this item
	 **/
	inline bofixed speed() const { return mCurrentSpeed; }
	inline void setSpeed(bofixed s) { mCurrentSpeed = s; }
	/**
	 * @return Maximum speed this item may have
	 **/
	inline bofixed maxSpeed() const { return mMaxSpeed; }
	inline void setMaxSpeed(bofixed maxspeed) { mMaxSpeed = maxspeed; }
	/**
	 * Raises speed by @ref accelerationSpeed unless @ref currentSpeed is
	 * @ref maxSpeed
	 **/
	inline void accelerate() { mCurrentSpeed = qMin(maxSpeed(), speed() + accelerationSpeed()); }
	/**
	 * Lowers speed by @ref decelerationSpeed unless @ref currentSpeed is 0
	 **/
	inline void decelerate() { mCurrentSpeed = qMax(bofixed(0), speed() - decelerationSpeed()); }
	/**
	 * @return How fast this unit accelerates.
	 * Acceleration speed shows how much speed of unit changes per advance call.
	 **/
	inline bofixed accelerationSpeed() const { return mAccelerationSpeed; }
	inline void setAccelerationSpeed(bofixed s) { mAccelerationSpeed = s; }
	/**
	 * @return How fast this unit decelerates.
	 * Deceleration speed shows how much speed of unit changes per advance call.
	 **/
	inline bofixed decelerationSpeed() const { return mDecelerationSpeed; }
	inline void setDecelerationSpeed(bofixed s) { mDecelerationSpeed = s; }
	/**
	 * @return How much this unit moves before stopping completely
	 * This is distance that item will move before it completely stops when it
	 * starts deceleration now and continues it until stopping
	 **/
	inline bofixed decelerationDistance() const { return (mCurrentSpeed / mDecelerationSpeed) / 2 * mCurrentSpeed; }


	inline void setVisible(bool v) { mIsVisible = v; }
	inline bool isVisible() const { return mIsVisible; }

	/**
	 * Reloads everything that can be reloaded. For example in @ref Unit
	 * this does weapon reloading and shield reloading currently.
	 *
	 * This could also do self-reperature, if the item provides such
	 * features.
	 *
	 * This method is not called every advance call, but in a certain
	 * interval only. The parameter @p count specifies how much reloading
	 * should be done. For example if this is called every 5 advance calls,
	 * then @p count will probably be 5.
	 **/
	virtual void reload(unsigned int count)
	{
		Q_UNUSED(count);
	}

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
	inline bofixed rotation() const { return mRotation; }
	void setRotation(bofixed r) { mRotation = r; setEffectsRotationDirty(true); }

	inline bofixed xRotation() const { return mXRotation; }
	void setXRotation(bofixed r) { mXRotation = r; setEffectsRotationDirty(true); }

	inline bofixed yRotation() const { return mYRotation; }
	void setYRotation(bofixed r) { mYRotation = r; setEffectsRotationDirty(true); }


	/**
	 * Select this unit, i.e. construct the select box - see @ref isSelected.
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
	 * some items you might want to return simply a NULL QColor (which is perfectly
	 * valid)
	 **/
	virtual QColor teamColor() const = 0;

	inline void setEffectsPositionDirty(bool d)
	{
		mEffectsPositionIsDirty = d;
	}
	bool isEffectsPositionDirty() const
	{
		return mEffectsPositionIsDirty;
	}

	inline void setEffectsRotationDirty(bool d)
	{
		mEffectsRotationIsDirty = d;
	}
	bool isEffectsRotationDirty() const
	{
		return mEffectsRotationIsDirty;
	}

protected:
	/**
	 * @return The current animation mode. @ref UnitAnimationIdle by
	 * default. Note that currently only units use animations, but they
	 * should be supported for every item. See also @ref
	 * updateAnimationMode.
	 **/
	virtual int getAnimationMode() const
	{
		return UnitAnimationIdle;
	}

private:
	/**
	 * Called right before changing the position of this item.
	 **/
	void itemAboutToMove(bofixed dx, bofixed dy, bofixed dy);

	/**
	 * Called right after changing the position of this item.
	 **/
	void itemHasMoved(bofixed dx, bofixed dy, bofixed dy);

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
	Player* mOwner;
	// FIXME: use KGameProperty here. We can do so, since we don't use
	// QCanvasSprite anymore.
	quint32 mId;
	bofixed mX; // centerX
	bofixed mY; // centerY
	bofixed mZ; // NOT the center! still the bottom of the unit (should be changed to centerZ too!)
	bofixed mWidth;
	bofixed mHeight;
	bofixed mDepth;

	bofixed mXVelocity;
	bofixed mYVelocity;
	bofixed mZVelocity;

	bofixed mCurrentSpeed;
	bofixed mMaxSpeed;
	bofixed mAccelerationSpeed;
	bofixed mDecelerationSpeed;

	bofixed mRotation;
	bofixed mXRotation;
	bofixed mYRotation;

	bool mIsSelected;
	bool mIsGroupLeaderOfSelection;

	Q3PtrVector<Cell>* mCells;
	bool mCellsDirty;
	bool mIsVisible;

	bool mEffectsPositionIsDirty;
	bool mEffectsRotationIsDirty;

	// AB: this is NOT saved to any file! it is calculated on the fly by
	// updateAnimationMode() and getAnimationMode()
	int mAnimationMode;
};

#endif

