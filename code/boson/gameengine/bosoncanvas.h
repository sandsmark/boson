/*
    This file is part of the Boson game
    Copyright (C) 1999-2000 Thomas Capricelli (capricel@email.enst.fr)
    Copyright (C) 2001-2008 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOSONCANVAS_H
#define BOSONCANVAS_H

#include "bosoncollisions.h"
#include "../defines.h"
#include "../bomath.h"

#include <qobject.h>

class BosonMap;
class Cell;
class Player;
class Unit;
class UnitProperties;
class BoItemList;
class BosonItem;
class ProductionPlugin;
class BosonShot;
class BosonWeapon;
class BosonCanvasStatistics;
class BosonPath;
class BosonMoveData;
class BosonShotFragment;
class BoEventListener;
class BoEventManager;
class BoCanvasQuadTreeNode;
class BosonPlayerListManager;
template<class T> class BoVector2;
template<class T> class BoVector3;
typedef BoVector2<bofixed> BoVector2Fixed;
typedef BoVector3<bofixed> BoVector3Fixed;

class KPlayer;
class QDataStream;
class QDomElement;
template<class T> class QPtrList;
template<class T> class QValueList;
template<class T> class QPtrVector;
template<class T1, class T2> class QMap;
template<class T1, class T2> class QPair;

/**
 * Helper class for @ref BosonCanvas::createItem.
 **/
class ItemType
{
public:
	/**
	 * @param type The precise type of the item. The type depends on the @p
	 * rtti, i.e. a "1" for a unit means something totally different than
	 * for a shot. The class that will be created (new'ed) should usually
	 * be described perfectly by the type. See e.g. @ref UnitBase::type and
	 * @ref BosonShot::type
	 *
	 * @param group Many items need additional parameters for their
	 * constructors, mainly to define the model. This defines a group of
	 * available items, depending on the other parameters (e.g. a group for
	 * a @ref BosonShotMissile is the unittype where the weapon is defined).
	 * The group can be unused for some items.
	 *
	 * @param groupType This defines the unique item inside the @p group.
	 * For example a @ref BosonShotMissile of the group "Unit XYZ" (with 3
	 * weapons) can have the groupTypes 1,2 or 3 (each of the available
	 * weapons in the group).
	 * The groupType can be unused for some items.
	 **/
	ItemType(unsigned long int type, unsigned long int group = 0, unsigned long int groupType = 0)
	{
		mType = type;
		mGroup = group;
		mGroupType = groupType;
	}
	ItemType(const ItemType& t)
	{
		*this = t;
	}
	ItemType operator=(const ItemType& t)
	{
		mType = t.mType;
		mGroup = t.mGroup;
		mGroupType = t.mGroupType;
		return *this;
	}

	// AB: this class is meant as a "short" way of grouping all 3 parameters
	// to a single parameter for createItem(). no need to make members
	// private.
	unsigned long int mType;
	unsigned long int mGroup;
	unsigned long int mGroupType;

public:
	/**
	 * Convenience method for creating a ItemType object for a unit with @p
	 * unitType.
	 **/
	static ItemType typeForUnit(unsigned long int unitType);
	/**
	 * Convenience method for creating a ItemType object for a @ref
	 * BosonShotExplosion object
	 **/
	static ItemType typeForExplosion();
	/**
	 * Convenience method for creating a ItemType object for a @ref
	 * BosonShotFragment object
	 **/
	static ItemType typeForFragment();

	/**
	 * Convenience method for creating a ItemType object for a generic @ref
	 * BosonShot object
	 **/
	static ItemType typeForShot(unsigned long int shotType, unsigned long int unitType, unsigned long int weaponPropertyId);
};

/**
 * @short Class that takes care of game management
 *
 * BosonCanvas is one of the most important classes in Boson.
 * It holds lists with all items and calls advance() methods for them
 *
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonCanvas : public QObject
{
	Q_OBJECT
public:
	enum PropertyIds {
		IdNextItemId = 10000 // must be >= KGamePropertyBase::IdUser
	};

public:
	/**
	 * Create a new canvas. Call @ref init before using this canvas!
	 *
	 * @param gameMode TRUE for normal game mode, FALSE for editor mode.
	 **/
	BosonCanvas(QObject* parent, bool gameMode = true);
	~BosonCanvas();

	/**
	 * Initialize this canvas. This @em must be called after the
	 * constructor.
	 *
	 * Note that this does @em not do the complete initializing! After
	 * calling this, you will at the very least have to call
	 * @ref loadCanvas or @ref loadFromXML.
	 *
	 * The canvas is NOT fully initialized and NOT usable before it is
	 * loaded as well! (in particular the pathfinder is still NULL)
	 **/
	bool init(BosonMap* map, BosonPlayerListManager* playerListManager, BoEventManager* eventManager);

	unsigned long int nextItemId();

	BosonCanvasStatistics* canvasStatistics() const;

	void initPathFinder();
	BosonPath* pathFinder() const;

	void registerQuadTree(BoCanvasQuadTreeNode* tree);
	void unregisterQuadTree(BoCanvasQuadTreeNode* tree);

	inline BosonCollisions* collisions() const { return mCollisions; }

	/**
	 * See @ref createItem.
	 *
	 * This will also do the necessary steps for newly added items, such as
	 * loading unit defaults and adding the unit to the @p owner.
	 **/
	BosonItem* createNewItem(int rtti, Player* owner, const ItemType& type, const BoVector3Fixed& pos);

	/**
	 * Test whether the unit can go over rect. This method only tests for
	 * the ground <em>not</em> for collisions with other units. See @ref Unit for
	 * this.
	 **/
	bool canGo(const UnitProperties* prop, const BoRect2Fixed& rect, bool _default = false) const;
	bool canGo(const UnitProperties* prop, int x, int y, bool _default = false) const;

	BosonMap* map() const;

	/**
	 * @return @ref BosonMap::width
	 **/
	unsigned int mapHeight() const;

	/**
	 * @return @ref BosonMap::height
	 **/
	unsigned int mapWidth() const;

	/**
	 * @return @ref BosonMap::heightMap
	 **/
	const float* heightMap() const;

	/**
	 * See @ref BosonMap::setHeightAtCorner
	 **/
	void setHeightAtCorner(int x, int y, float height);

	/**
	 * See @ref BosonMap::setHeightsAtCorners
	 **/
	void setHeightsAtCorners(const QValueList< QPair<QPoint, float> >& heights);

	/**
	 * @return BosonMap::heightAtCorner
	 **/
	float heightAtCorner(int x, int y) const;

	/**
	 * @return BosonMap::waterDepthAtCorner
	 **/
	float waterDepthAtCorner(int x, int y) const;

	/**
	 * @return Height at point x,y (in canvas coordinates)
	 * This is calculated from heights at the corners of the cell that the point
	 * is on. Water surface level is also taken insto account
	 **/
	float heightAtPoint(bofixed x, bofixed y) const;

	/**
	 * @return Height at point x,y (in canvas coordinates)
	 * This is calculated from heights at the corners of the cell that the point
	 *  is on. Note that this method _does not_ take water level into account.
	 **/
	float terrainHeightAtPoint(bofixed x, bofixed y) const;

	/**
	 * Request to change the advance list in the next advance phase.
	 **/
	void changeAdvanceList(BosonItem*);

	/**
	 * Add @p item to the list of items in the canvas. This should get
	 * called in the constructor of @ref BosonItem and you should not need
	 * to care about this.
	 *
	 * See also @ref removeItem
	 **/
	void addItem(BosonItem* item);

	/**
	 * @return A complete list of <em>all</em> items on the canvas. See @ref
	 * addItem
	 **/
	BoItemList* allItems() const;
	unsigned int allItemsCount() const;



	/**
	 * Called by @ref Unit. This informs the canvas about a moved
	 * unit. Should e.g. adjust the destination of units which have this
	 * unit as target.
	 *
	 * Also adjust the mini map - see @ref signalUnitMoved
	 **/
	void unitMoved(Unit* unit, bofixed oldX, bofixed oldY);

	void unitMovingStatusChanges(Unit* u, int oldstatus, int newstatus);

	/**
	 * Called by @ref Unit. One unit damages/shoots at another unit.
	 * All it does is to create new @ref BosonMissile and play shooting sound
	 **/
	void newShot(BosonShot* shot);

	/**
	 * Called when missile explodes. This just calls @ref explosion with correct
	 * values
	 **/
	void shotHit(BosonShot* m);

	/**
	 * Called when something explodes. Usually it's unit or missile.
	 * @param pos Position of the center of the explosion
	 * @param damage How much unit will be damaged if it's in explosion area
	 * @param range Radius of explosion. All units range or less cells away will be damaged
	 * @param owner Player who caused the explosion. Used for statistics. May be null
	 **/
	void explosion(const BoVector3Fixed& pos, long int damage, bofixed range, bofixed fullrange, Player* owner);

	/**
	 * Called when unit is damaged (usually by missile).
	 * It calculates new health for the unit, creates effects if needed
	 * and marks unit as destoyed if it doesn't have any hitpoints left anymore.
	 **/
	void unitDamaged(Unit* unit, long int damage);

	/**
	 * Mark the unit as destroyed and play the destroyed sound.
	 * The unit will be deleted after a certain time.
	 **/
	void destroyUnit(Unit* unit);

	/**
	 * @internal
	 * Used by @ref BosonWeapon::shoot
	 *
	 * This causes the correct effects (visual as well as sound effects) to
	 * be emitted
	 *
	 * See also @ref signalShotFired.
	 **/
	void shotFired(BosonShot* shot, BosonWeapon* weapon);

	/**
	 * Prepare the unit to be deleted. Remove the unit from the player and
	 * so on. This doesn't play any sound or so, so it can be used in
	 * editor, too.
	 *
	 * For game mode please use @ref destroyUnit instead, which is a
	 * frontend for this.
	 *
	 * Note that this function doesn't add the unit to any deletion list and
	 * it doesn't delete the unit either.
	 **/
	void removeUnit(Unit* unit);

	void updateSight(Unit* unit, bofixed oldX, bofixed oldY);
	void addSight(Unit* unit);
	void removeSight(Unit* unit);

	void addRadar(Unit* unit);
	void removeRadar(Unit* unit);
	const QValueList<const Unit*>* radarUnits() const;

	void addRadarJammer(Unit* unit);
	void removeRadarJammer(Unit* unit);
	const QValueList<const Unit*>* radarJammerUnits() const;

	Cell* cellAt(Unit* unit) const;

	/**
	 * @return The cell at @p x, @p y in <em>canvas</em>-coordinates
	 **/
	Cell* cellAt(bofixed x, bofixed y) const;

	/**
	 * @return The cell at @p x, @p y (in cell coordinates - see @ref cellAt
	 * for other coordinates)
	 **/
	Cell* cell(int x, int y) const;

	/**
	 * @return BosonMap::cells. Use with care!
	 **/
	Cell* cells() const;

	void deleteDestroyed();
	void deleteUnusedShots();

	BosonItem* findItem(unsigned long int id) const;
	Unit* findUnit(unsigned long int id) const;

	/**
	 * Convenience method. See @ref BosonCollisions::findUnitAt
	 **/
	Unit* findUnitAt(const BoVector3Fixed& pos) const { return collisions()->findUnitAt(pos); }

	/**
	 * Convenience method. See @ref BosonCollisions::findItemAt
	 **/
	BosonItem* findItemAt(const BoVector3Fixed& pos) const { return collisions()->findItemAt(pos); }

	/**
	 * Convenience method. See @ref BosonCollisions::findUnitAtCell
	 **/
	Unit* findUnitAtCell(int x, int y, bofixed z) const { return collisions()->findUnitAtCell(x, y, z); }

	/**
	 * Convenience method. See @ref BosonCollisions::findItemAtCell
	 **/
	BosonItem* findItemAtCell(int x, int y, bofixed z, bool unitOnly) const { return collisions()->findItemAtCell(x, y, z, unitOnly); }

	/**
	 * Convenience method. See @ref BosonCollisions::cellOccupied
	 **/
	bool cellOccupied(int x, int y) const { return collisions()->cellOccupied(x, y); }

	/**
	 * Convenience method. See @ref BosonCollisions::cellOccupied
	 **/
	bool cellOccupied(int x, int y, Unit* u, bool excludeMoving = false) const { return collisions()->cellOccupied(x, y, u, excludeMoving); }

	/**
	 * Checks if any items, except exclude, collide with given box
	 **/
	QValueList<Unit*> collisionsInBox(const BoVector3Fixed& v1, const BoVector3Fixed& v2, BosonItem* exclude) const { return collisions()->collisionsInBox(v1, v2, exclude); }

	/**
	 * @param pos The location where the unit should get placed.
	 * This point specifies the <em>upper-left</em> corner of the unit.
	 * @param factory If NULL then BUILD_RANGE is ignored. Otherwise
	 * facilities must be in range of BUILD_RANGE of any player unit and
	 * mobile units in BUILD_RANGE of the facility.
	 * @return TRUE if the unit can be placed at pos, otherwise FALSE
	 **/
	bool canPlaceUnitAt(const UnitProperties* unit, const BoVector2Fixed& pos, ProductionPlugin* factory) const;

	/**
	 * Clear the canvas, preparing it for destruction.
	 *
	 * This method is meant to eliminate all pointers and all allocated
	 * memory from the canvas. For notifying the game that a player has won,
	 * see @ref signalGameOver.
	 **/
	void quitGame();

	void addToCells(BosonItem* u);
	void removeFromCells(BosonItem* u);

	bool onCanvas(const BoVector2Fixed& pos) const;
	bool onCanvas(const BoVector3Fixed& pos) const;
	bool onCanvas(bofixed x, bofixed y) const
	{
		return x >= 0.0f && y >= 0.0f && x <= mapWidth() && y <= mapHeight();
	}

	bool advanceFunctionLocked() const { return mAdvanceFunctionLocked; }

	/**
	 * This method uses @ref loadFromXML to load this canvas from @p
	 * canvasData. @p canvasData is interpreted as the contents of an XML
	 * file with the root element describing the canvas.
	 **/
	bool loadCanvas(const QString& canvasData);

	/**
	 * @return The canvas saved to an XML file, converted to a string. See
	 * also @ref QDomDocument::toCString() and @ref saveAsXML. This string
	 * can be used to load the canvas again using @ref loadCanvas.
	 **/
	QCString saveCanvas() const;

	/**
	 * This method is meant for use in the editor or in test programs, it is
	 * of no use for the game itself (you always have a .bpf file to load
	 * then).
	 *
	 * @return A @ref QDomDocument (saved as a string, see @ref
	 * QDomDocument::toCString) describing an empty canvas. This can be used
	 * with @ref loadCanvas.
	 **/
	static QCString emptyCanvasFile(unsigned int playerCount);

	bool loadFromXML(const QDomElement& root);
	bool saveAsXML(QDomElement& root) const;

	/**
	 * Load the conditions for the @ref BoCanvasEventListener (i.e. the
	 * global @ref BoEventListener) from @p root.
	 *
	 * Note that the conditions are loaded <em>only</em>, not the whole
	 * event listener.
	 *
	 * See also @ref BoEventListener::loadConditions
	 **/
	bool loadConditions(const QDomElement& root);

	/**
	 * Save the conditions from the @ref BoCanvasEventListener to @p root.
	 * This @ref QDomElement can be used for loading the conditions using
	 * @ref loadConditions again.
	 *
	 * See also @ref BoEventListener::saveConditions
	 **/
	bool saveConditions(QDomElement& root) const;

	/**
	 * Valid in editor mode only. Delete the specified items.
	 **/
	void deleteItems(const QValueList<unsigned long int>& items);

	BoEventManager* eventManager() const;
	BoEventListener* eventListener() const;

	/**
	 * @internal
	 * Used by @ref BosonPath
	 **/
	void clearMoveDatas();

	void insertMoveData(const UnitProperties* prop, BosonMoveData* data);
	BosonMoveData* moveData(const UnitProperties* prop) const;

public slots:
	/**
	 * @param See @ref Boson::signalAdvance
	 **/
	void slotAdvance(unsigned int advanceCallsCount, bool advanceFlag);

signals:
	void signalItemAdded(BosonItem* item);
	void signalUnitMoved(Unit* unit, bofixed oldX, bofixed oldY);
	void signalUnitRemoved(Unit* unit);

	/**
	 * Like @ref signalUnitRemoved, but the unit actually got destroyed
	 * (e.g. this is not emitted in editor mode)
	 **/
	void signalUnitDestroyed(Unit* unit);

	/**
	 * Signal to notify the GUI that a fragment was created. A corresponding
	 * effect can be created in a GUI class now.
	 *
	 * Note that this signal is ugly. However fragments need additional
	 * initialization after their creation, therefore ·@ref signalItemAdded
	 * can not be used directly.
	 **/
	void signalFragmentCreated(BosonShotFragment* fragment);

	/**
	 * Emitted by @ref removeItem just after the item is removed from the
	 * canvas. Note that this usually happens from the destructor of an
	 * item!
	 *
	 * Do not use this in the game, as it isn't reliable enough. For example
	 * you must not use this to ensure that @ref Unit::target points to a
	 * valid unit, as it is not fully sure when a unit gets removed from the
	 * canvas / is deleted. It might happen at different moments on
	 * different clients.
	 *
	 * You can safely use this for any GUI related problems (like ensuring
	 * that the tooltip item points to a valid item)
	 **/
	void signalRemovedItem(BosonItem* item);

	void signalShotFired(BosonShot* shot, BosonWeapon* weapon);
	void signalShotHit(BosonShot* shot);

	/**
	 * Emitted when the winning conditions are fullfilled. See @ref
	 * BoCanvasEventListener.
	 *
	 * @param fullfilledWinningConditions The list of players that have
	 * completed their goals of this scenario, i.e. the players that have
	 * won. If this list is empty, no player has won.
	 **/
	void signalGameOver();

protected:
	/**
	 * This is a minimalistic method for creating @ref BosonItem objects.
	 * Only the actual creation happens here, no additional configuration
	 * (beside the constructor) is made. Such configuration (such as adding
	 * the item to the @p owner) should be made in the calling method, e.g.
	 * @ref createNewItem for new items or @ref createItemFromXML for loading items.
	 *
	 * @param rtti The RTTI of the item that is to be created. See @ref RTTI
	 * and @ref BosonItem::rtti. The RTTI groups the items in certain
	 * classes, such as units and shots.
	 * @param owner The player that will own the item. NULL might be allowed
	 * for certain items (atm we dont have such items)
	 *
	 * @param type See @ref ItemType
	 *
	 * @param id A unique ID for the item. See @ref BosonItem::setId
	 **/
	BosonItem* createItem(int rtti, Player* owner, const ItemType& type, const BoVector3Fixed& pos, unsigned long int id);

	Unit* createUnit(Player* owner, unsigned long int unitType);
	BosonShot* createShot(Player* owner, unsigned long int shotType, unsigned long int unitType, unsigned long int weaponPropertyId);

	/**
	 * Load <em>all</em> items from @p root - this means for all players!
	 *
	 * Used by @ref loadFromXML.
	 **/
	bool loadItemsFromXML(const QDomElement& root);
	bool saveItemsAsXML(QDomElement& root) const;

	/**
	 * Parses @p item and will create a @ref BosonItem correspoding to the
	 * attributes and elements found there.
	 * @return A new @ref BosonItem object accorind to @p item
	 **/
	BosonItem* createItemFromXML(const QDomElement& item, Player* owner);

	bool loadItemFromXML(const QDomElement& element, BosonItem* item);

	/**
	 * NEVER delete an item directly - always use this method to do so!
	 *
	 * You should usually not call this, as it will get called when needed
	 * (e.g. in @ref deleteUnits and @ref deleteUnusedShots and so on)
	 **/
	void deleteItem(BosonItem* item);

	/**
	 * Delete a list of items. See also @ref deleteItem
	 **/
	void deleteItems(QPtrList<BosonItem>& items);
	void deleteItems(BoItemList& items);

	/**
	 * Remove @p item from the list of items.
	 *
	 * Be <em>very</em> careful when you call this manually!
	 **/
	void removeItem(BosonItem* item);

	void lockAdvanceFunction() { mAdvanceFunctionLocked = true; }
	void unlockAdvanceFunction() { mAdvanceFunctionLocked = false; }

	void removeFromAdvanceLists(BosonItem* item);

private:
	friend class BoCanvasAdvance;
	friend class BoCanvasSightManager;
	class BosonCanvasPrivate;
	BosonCanvasPrivate* d;
	BosonCollisions* mCollisions;

	bool mAdvanceFunctionLocked;
};

#endif
