/*
    This file is part of the Boson game
    Copyright (C) 2001-2004 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef BOSONPATH_H
#define BOSONPATH_H

#include <qobject.h>

/***  COMMON STUFF  ***/

// When destination is reached (path ends) and unit should stop moving, we add
//  point (PF_END_CODE; PF_END_CODE)
#define PF_END_CODE -1
// When path reaches next region and pathfinder has to be called again (to get
//  low-level path to next region), we add point (PF_NEXT_REGION; PF_NEXT_REGION)
#define PF_NEXT_REGION -2
// When we can't go to destination, we add point (PF_CANNOT_GO; PF_CANNOT_GO)
#define PF_CANNOT_GO -3

class BosonPathInfo;
class BoVector3;
class BoVector4;
class BoVector2;


/***  OLD PATHFINDER  ***/

#include "global.h"

#include <qvaluelist.h>


#define SEARCH_STEPS 25  // How many steps of path to find

// Defines whether next-generation pathfinder will be used or not
// NOTE: TNG is a bit buggy atm, especially concerning collision-detection, with
//  big groups
#define PATHFINDER_TNG


class Unit;
class Player;
class BosonBigDisplayBase;
class BosonCanvas;
class BoVector2;

class PathNode;

class QPoint;
template<class T> class QValueList;

/**
 * Boson's pathfinder class
 * This class is used to get best path from point a (start) to point b (goal)
 * It is used by units
 * @author Rivo Laks <rivolaks@hot.ee>
 */
class BosonPath
{
  public:

    /**
     * Constructs BosonPath
     * @param unit Pointer to unit, for what path is searched
     * @param startx x-coordinate of start point
     * @param starty y-coordinate of start point
     * @param goalx x-coordinate of goal point
     * @param goaly y-coordinate of goal point
     */
    BosonPath(Unit* unit, int startx, int starty, int goalx, int goaly, int range = 0);
    ~BosonPath();

    /**
     * Finds path. Path is stored in @ref path
     * Note that if path wasn't found goal may be set to last waypoint
     * @return TRUE if path was found, FALSE otherwise
     */
    bool findPath();

    /**
     * @param unit The unit that will be moved
     * @param goalx the x <em>coordinate</em> of the goal. Not the cell!
     * @param goaly the y <em>coordinate</em> of the goal. Not the cell!
     **/
    static QValueList<BoVector2> findPath(BosonPathInfo* info);

    enum ResourceType { Minerals, Oil, EnemyBuilding, EnemyUnit };
    static QValueList<BoVector2> findLocations(Player* player, int x, int y, int n, int radius, ResourceType type);

    /**
     * Returns lenght of path (in tiles)
     */
    int pathLength() const { return mPathLength; };

    /**
     * Returns cost of path
     */
    float pathCost() const { return mPathCost; };

  protected:
    /**
     * In this list are waypoints of path
     */
    QValueList<BoVector2> path;

  private:
    class Marking;
    float dist(int ax, int ay, int bx, int by);
    float cost(int x, int y);
    static void getFirst(QValueList<PathNode>& list, PathNode& n);
    static void addNode(QValueList<PathNode>& list, const PathNode& n);
    static void neighbor(int& x, int& y, Direction d);
    Direction reverseDir(Direction d);
    bool inRange(int x, int y);

    void debug() const;

    bool findFastPath();
    bool findSlowPath();
    bool rangeCheck();

  private:
    int mStartx;
    int mStarty;
    int mGoalx;
    int mGoaly;

    Unit* mUnit;

    int mNodesRemoved;
    int mPathLength;
    float mPathCost;
    int mRange;

    class Marking
    {
      public:
        Marking() { dir = DirNone; f = -1; g = -1; c = -1; level = -1; }
        Direction dir;
        float f;
        float g;
        float c; // Cost of cell
        short int level;
    };
    Marking mMark[SEARCH_STEPS * 2 + 1][SEARCH_STEPS * 2 + 1];
};



/***********************************************************
*****           N E W   P A T H F I N D E R
***********************************************************/

// AB: i am sure we can avoid lots of these
#include <qptrvector.h>
#include <qvaluevector.h>
#include <qptrlist.h>
#include <qvaluelist.h>

#include "bo3dtools.h"


// Cell passage costs
// This shouldn't be used, as we shouldn't touch occupied cells
#define PF_TNG_COST_STANDING_UNIT 1000.0f
#define PF_TNG_COST_MOVING_UNIT 1.0f
#define PF_TNG_COST_WAITING_UNIT 2.5f
#define PF_TNG_COST_ENGAGING_UNIT 3.5f
#define PF_TNG_COST_MUSTSEARCH_UNIT 0.5f
#define PF_TNG_COST_INTERNAL_UNIT 100.0f
#define PF_TNG_EPSILON 0.0001f


class BosonPathSector;
class BosonPathRegion;
class BosonPathRegionGroup;
class BosonPathNode;
class BosonPath2;
class BosonPathHighLevelPath;

class BosonMap;
class Cell;
class BosonBigDisplayBase;
class BosonCanvas;



/**
 * @short Next-generation pathfinder for Boson
 *
 * This is the next generation of Boson pathfinder
 * It uses hierarchical pathfinding, first, high-level path is found using
 * regions, then low level path is found between every two regions.
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonPath2
{
  public:
    /**
     * Passability type of a cell or region
     * @li NotPassable  not passable for some reason (e.g. too big slope)
     *    Note that this does _not_ check if tile is occupied
     * @li Land  land units can move on this cell/region
     * @li Water  water units can move on this cell/region
     **/
    enum PassabilityType { NotPassable = 0, Land, Water };


    /**
     * Construct pathfinder, using given map
     **/
    BosonPath2(BosonMap* map);
    ~BosonPath2();


    /**
     * Finds path
     * All info related to path is read from info and will also be written there
     **/
    void findPath(BosonPathInfo* info);


    /**
     * Finds high-level path for given data struct
     **/
    void findHighLevelPath(BosonPathInfo* info);
    /**
     * Finds low-level path for given data struct
     **/
    void findLowLevelPath(BosonPathInfo* info);

    void findFlyingUnitPath(BosonPathInfo* info);

    bool rangeCheck(BosonPathInfo* info);


    void cellsOccupiedStatusChanged(int x1, int y1, int x2, int y2);

    void releaseHighLevelPath(BosonPathHighLevelPath* hlpath);


    /**
     * Initializes all necessary data structures
     **/
    void init();
    /**
     * Constructs and inits sectors
     **/
    void initSectors();
    /**
     * Constructs and inits regions
     **/
    void initRegions();
    /**
     * Calculates for every cell whether it's passable (slope <= 45 degrees) or
     * not
     **/
    void initCellPassability();
    /**
     * Finds neighbors in given area
     **/
    void findRegionNeighbors(int x1, int y1, int x2, int y2);
    /**
     * Calculates passing cost for each region/neighbor pair in given list
     **/
    void initRegionCosts(QPtrVector<BosonPathRegion>& regions);
    void initRegionGroups(QPtrVector<BosonPathRegion>& regions);
    void findRegionsInGroup(BosonPathRegionGroup* group, BosonPathRegion* start);


    void colorizeRegions();


    /**
     * Deletes high-level path and removes it from cache
     **/
    void removeHighLevelPath(BosonPathHighLevelPath* path);
    /**
     * Tries to find high-level path from cache
     * @return found path, or 0 if there was no suitable path in cache
     **/
    BosonPathHighLevelPath* findCachedHighLevelPath(BosonPathInfo* info);
    /**
     * Adds given gigh-level path to cache
     **/
    void addCachedHighLevelPath(BosonPathHighLevelPath* path);
    /**
     * Searches high level path (aka high-level pathfinder)
     **/
    void searchHighLevelPath(BosonPathInfo* info);
    void findHighLevelGoal(BosonPathInfo* info);

    float highLevelDistToGoal(BosonPathRegion* r, BosonPathInfo* info);
    float highLevelCost(BosonPathRegion* r, BosonPathInfo* info);
    float lowLevelDistToGoal(int x, int y, BosonPathInfo* info);
    float lowLevelCost(int x, int y, BosonPathInfo* info);
    float lowLevelCostAir(int x, int y, BosonPathInfo* info);

    static void neighbor(int& x, int& y, Direction d);


    /**
     * Returns sector at given pos. Note that these are "sector coordinates",
     * not cell or canvas ones. You shouldn't need to use this method.
     **/
    BosonPathSector* sector(int x, int y);
    unsigned int sectorWidth() const  { return mSectorWidth; }
    unsigned int sectorHeight() const  { return mSectorHeight; }

    /**
     * @return region that has cell at given pos (in cell coords) or 0 if no
     *  region has this cell (usually because cell is occupied or not passable)
     **/
    BosonPathRegion* cellRegion(int x, int y);
    BosonPathRegion* cellRegion(const BoVector2& p)  { return cellRegion((int)p.x(), (int)p.y()); }
    /**
     * @return passability type of given cell
     **/
    PassabilityType cellPassability(int x, int y);
    /**
     * @return whether given cell is occupied or not
     **/
    bool cellOccupied(int x, int y);
    /**
     * @return cost of given cell
     **/
    float cellCost(int x, int y);
    /**
     * @return Cell at given pos
     **/
    Cell* cell(int x, int y);
    /**
     * @return Whether cell at (x; y) is valid (i.e. it's on the map)
     **/
    bool isValidCell(int x, int y);
    /**
     * Adds region r to the list of regions
     * @return id for new region
     * Note that this method does not create new regions
     **/
    int addRegion(BosonPathRegion* r);
    /**
     * Removes region r from the list of regions and frees it's id
     * Note that this method does not delete any regions
     **/
    void removeRegion(BosonPathRegion* r);


  private:
    BosonMap* mMap;
    BosonPathSector* mSectors;
    unsigned int mSectorWidth;
    unsigned int mSectorHeight;
    QPtrList<BosonPathHighLevelPath> mHLPathCache;
    bool* mRegionIdUsed;
    QPtrVector<BosonPathRegion> mRegions;
};

/**
 * Helper class for pathfinder
 *
 * This class holds information about a sector.
 * Sector is a rectangular area on the map. Sectors are divided into regions
 * (collections of fully-passable cells inside a sector).
 **/
class BosonPathSector
{
  public:
    BosonPathSector();

    void setPathfinder(BosonPath2* pf);

    void setGeometry(int x, int y, int w, int h);
    bool hasCell(int x, int y);

    void initRegions();
    void reinitRegions();
    void updateRegions();

    // List of regions in this sector. Note that it can be empty list
    QPtrVector<BosonPathRegion> regions;
    // Dimensions of the sector - upper-left corner and size
    int x, y;
    int w, h;
    // Pointer to pathfinder object
    BosonPath2* pathfinder;
};

/**
 * Helper class for pathfinder
 *
 * Region is a collection of fully connected cells inside a single sector.
 **/
class BosonPathRegion
{
  public:
    class Neighbor
    {
      public:
        Neighbor()  {region = 0; cost = 0.0f; bordercells = 0; }

        BosonPathRegion* region;
        float cost;
        int bordercells;
    };

    BosonPathRegion(BosonPathSector* sector);
    ~BosonPathRegion();

    void findCells(int x, int y);
    void findBorderCells();
    void calculateCosts();
    void calculateCosts(BosonPathRegion* neighbor);
    void calculateCosts(unsigned int index);
    void addNeighbor(BosonPathRegion* r);
    void removeNeighbor(BosonPathRegion* r);

    // Neighbors of this region (have common edge with this region)
    //  Note that neighbors are always in other sectors than this region
    QValueVector<Neighbor> neighbors;
    // Sector, to which this region belongs
    BosonPathSector* sector;

    // Passability type of this region - whether land or water units can use it
    BosonPath2::PassabilityType passabilityType;
    // How many cells this region has
    int cellsCount;
    // Center of this region (average of centers of all cells)
    float centerx, centery;
    // Cost of passing this region
    float cost;
    // Unique id of this region
    int id;
    // Group that this region belongs to
    BosonPathRegionGroup* group;

    // FIXME: HACK!!!
    // Upper node in the path (the one we came from), used to traceback path.
    //  Note that only region is saved, no costs (they're not needed... I guess)
    BosonPathRegion* parent;
};

/**
 * Helper class for pathfinder
 *
 * RegionGroup is a group of connected regions. If two points are inside a
 * single RegionGroup, then there's a path between them
 **/
class BosonPathRegionGroup
{
  public:
    // Passability type of this region - whether land or water units can use it
    BosonPath2::PassabilityType passabilityType;
    //int id;
    // All regions in this group
    QPtrList<BosonPathRegion> regions;
};

class BosonPathNode
{
  public:
    BosonPathNode() { x = 0; y = 0; g = 0; h = 0; }
    BosonPathNode(int _x, int _y) { x = _x; y = _y; g = 0; h = 0; }

    int x;
    int y;

    float g;
    float h;
};

class BosonPathFlyingNode : public BosonPathNode
{
  public:
    BosonPathFlyingNode() : BosonPathNode()  { depth = 0; }
    BosonPathFlyingNode(int _x, int _y) : BosonPathNode()  { depth = 0; }

    int depth;
};

template<class T> class BosonPathHeap : public QValueList<T>
{
  public:
    inline void add(const T& x)
    {
      QValueListIterator<T> it;
      for(it = this->begin(); it != this->end(); ++it)
      {
        if(((x.g + x.h) - ((*it).g + (*it).h)) < PF_TNG_EPSILON)
        {
          // (x.g + x.h) <= ((*it).g + (*it).h)
          insert(it, x);
          break;
        }
      }
      if(it == this->end()) {
        append(x);
      }
    }

    /*inline void changeCost(const T& x)
    {
      QValueList<PathNode>::iterator it;
      for(it = list.begin(); it != list.end(); ++it)
      {
        if(*it == x)
        {
          // Change cost
          *it = x;
          break;
        }
      }
      if(it == list.end()) {
        // ERROR: No such item in the list
        return;
      }
    }*/

    inline void takeFirst(T& x)
    {
      x = this->first();
      this->pop_front();
    }
};

/**
 * Helper class for pathfinder
 *
 * This class stores neccessary information when finding path for a unit.
 * Pathfinding results will also be stored here.
 * This class acts as a link between pathfinder and unit (pathfinder client)
 **/
class BosonPathInfo
{
  public:
    BosonPathInfo()  { reset(); range = 0; waiting = 0; pathrecalced = 0; }
    void reset()
    {
      unit = 0;
      hlpath = 0;
      hlstep = 0;
      llpath.clear();
      start.reset();
      dest.reset();
      range = 0;
      startRegion = 0;
      destRegion = 0;
      possibleDestRegions.clear();
      passable = true;
      canMoveOnLand = true;
      canMoveOnWater = false;
      flying = false;
      passability = BosonPath2::Land;
      moveAttacking = true;
      slowDownAtDest = true;
      waiting = 0;
      pathrecalced = 0;
    }

    // Unit that we're searching path for
    Unit* unit;

    // Pointer to high-level path
    BosonPathHighLevelPath* hlpath;
    // High-level path step used atm
    unsigned int hlstep;

    // Low-level path, containing waypoints
    QValueVector<BoVector2> llpath;

    // Start and destination point
    BoVector2 start;
    BoVector2 dest;

    // Range, in cells
    // If range is 0, we try to get as close to destination point as possible,
    //  if it's bigger than 0, we either get exactly to this range, or won't
    //  find path (if it isn't possible)
    int range;

    // Regions containing start and dest points
    BosonPathRegion* startRegion;
    // Note that this is the real destination region where we're going to. It
    //  may also change in case better destination region will become available
    //  from destination regions list.
    BosonPathRegion* destRegion;
    // List of all possible destination regions. Usually this includes all
    //  regions within given range from destination point. Actual destination
    //  point is chosen from this list
    QPtrVector<BosonPathRegion> possibleDestRegions;

    // IS it possible to get from start to detination?
    // FIXME: better name
    bool passable;

    bool canMoveOnLand;
    bool canMoveOnWater;
    bool flying;
    // This is only for the pathfinder
    BosonPath2::PassabilityType passability;


    // Are these ok here???
    // If true, then unit attacks enemies in sight while moving
    bool moveAttacking;
    // If true, unit will decelerate before reaching destination
    bool slowDownAtDest;
    // How many advance calls unit has been waiting since path was last recalced
    int waiting;
    // How many times path has been recalculated for unit (while waiting)
    int pathrecalced;
};

class BosonPathHighLevelPath
{
  public:
    BosonPathHighLevelPath()
    {
      startRegion = 0; destRegion = 0; valid = false; users = 0;
      passability = BosonPath2::NotPassable;
    }
    // Starting region
    BosonPathRegion* startRegion;
    // Destination region
    BosonPathRegion* destRegion;

    // All the regions in this path, starting from start
    QPtrVector<BosonPathRegion> path;

    // Is this path still valid?
    bool valid;

    // Passability of this path. I.e. if it's land, then this path is passable
    //  for land units
    BosonPath2::PassabilityType passability;

    // This is for reference counting. Once nobody uses the path, it will be
    //  deleted
    int users;
};

class BosonPathHighLevelNode
{
  public:
    BosonPathHighLevelNode()  { region = 0; g = 0; h = 0; }

    inline bool operator==(const BosonPathHighLevelNode& x)  { return (region == x.region); }

    BosonPathRegion* region;
    float g;
    float h;
};

/**
 * @short interface between @ref BosonBigDisplayBase / @ref BosonCanvas and @ref
 * BosonPath.
 *
 * The @ref BosonPath (and friends) classes are supposed to tell this class
 * about path visualization issues (e.g. where to paint which lines and so on).
 * This class then takes care about letting @ref BosonBigDisplayBase and / or
 * @ref BosonCanvas know about these (e.g. by emitting a signal).
 **/
class BosonPathVisualization : public QObject
{
  Q_OBJECT
  public:
    ~BosonPathVisualization();

    /**
     * @return The static BosonPathVisualization object. There is only a single
     * object in the game.
     **/
    static BosonPathVisualization* pathVisualization();

    /**
     * @param points The points of the line visualization. Note that the z
     * coordinates are overwritten later, you do not have to specify them!
     **/
    void addLineVisualization(const QValueList<BoVector3>& points, const BoVector4& color, float pointSize = 1.0f, int timeout = 60, float zOffset = 0.5f);

    /**
     * @overload
     * Just like above, but with a default color
     **/
    void addLineVisualization(const QValueList<BoVector3>& points, float pointSize = 1.0f, int timeout = 60, float zOffset = 0.5f);

  signals:
    void signalAddLineVisualization(const QValueList<BoVector3>& points, const BoVector4& color, float pointSize, int timeout, float zOffset);

  private:
    BosonPathVisualization(QObject* parent);

  private:
    static BosonPathVisualization* mPathVisualization;
};


#endif // BOSONPATH_H

/*
 * vim: et sw=2
 */
