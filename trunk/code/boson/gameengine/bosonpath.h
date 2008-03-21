/*
    This file is part of the Boson game
    Copyright (C) 2001-2005 Rivo Laks (rivolaks@hot.ee)

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
#ifndef BOSONPATH_H
#define BOSONPATH_H


// AB: i am sure we can avoid lots of these
#include <qobject.h>
#include <qptrvector.h>
#include <qvaluevector.h>
#include <qptrlist.h>
#include <qvaluelist.h>

#include "../bomath.h"
#include "../bo3dtools.h"



class BosonPathInfo;
class BosonPathNode;
class BosonPathHighLevelPath;
class BosonPathLowLevelData;
class BosonPathHighLevelData;

class BosonMap;
class Cell;
class BosonBigDisplayBase;
class BosonCanvas;
class BoColorMap;
class Player;
class Unit;
class BosonMoveData;
class PlayerIO;
class BosonItem;

class QDomElement;




/**
 * @short Pathfinder for Boson
 *
 * This is the 3rd generation of Boson pathfinder
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonPath
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
     * Resource type, as given to @ref unitMovingStatusChanges method.
     **/
    enum ResourceType { Minerals = 0, Oil, EnemyBuilding, EnemyUnit };

    enum Result { None = 0, GoalReached, NoPath, OutOfRange };


    /**
     * Construct pathfinder, using given map
     **/
    BosonPath(BosonMap* map);
    ~BosonPath();


    /**
     * The main (and probably most important pathfinder method).
     * It finds the path, using data from the given info object. The resulting
     *  data will be written into the same object.
     **/
    void findPath(BosonPathInfo* info);
    void preparePathInfo(BosonPathInfo* info);

    void cellsOccupiedStatusChanged(int x1, int y1, int x2, int y2);

    bool saveAsXML(QDomElement& root) const;
    bool loadFromXML(const QDomElement& root);


    /**
     * Initializes the pathfinder
     **/
    void init(BosonCanvas* canvas);

    /**
     * Advances the pathfinder. This includes e.g. updating changed blocks.
     **/
    void advance();

    /**
     * Use this to notify the pathfinder that moving status of a unit has
     *  changed.
     **/
    void unitMovingStatusChanges(Unit* u, int oldstatus, int newstatus);


    /**
     * @return Debug string for the given coordinates.
     **/
    QString debugText(bofixed x, bofixed y);

    QValueList<BoVector2Fixed> findLocations(Player* player, int x, int y, int n, int radius, ResourceType type);



  protected:
    // TODO: add const to more methods
    /*****  Init methods  *****/
    void initMoveDatas(BosonCanvas* canvas);
    void initCellStatusArray();
    void initCellPassabilityMaps();
    void initBlocks();
    void initOffsets();


    /*****  Generic stuff  *****/
    /**
     * @return Array with slopes of each cell
     **/
    bofixed* calculateSlopemap();
    bofixed* calculateForestmap();

    void cellChanged(Cell* c);


    /*****  Flying-unit pathfinder  *****/
    // Flying-unit pathfinder methods
    void findFlyingUnitPath(BosonPathInfo* info);

    bofixed flyingDistToGoal(bofixed x, bofixed y, bofixed rot, BosonPathInfo* info);
    bofixed flyingCost(bofixed x, bofixed y, bofixed rot, BosonPathInfo* info);


    /*****  Low-level pathfinder  *****/
    Result getLowLevelPath(BosonPathInfo* info);
    void getPartialLowLevelPath(BosonPathInfo* info);
    void resetDirtyCellStatuses();
    Result lowLevelDoSearch(BosonPathLowLevelData* data);
    bool lowLevelSearchNeighbor(BosonPathLowLevelData* data, const BosonPathNode& n, unsigned int dir);
    void calculateCellStatus(BosonPathInfo* info, int x, int y);
    void lowLevelFinishSearch(BosonPathLowLevelData* data);
    bofixed lowLevelDistToGoal(BosonPathLowLevelData* data, int x, int y) const;
    void lowLevelSetAreaBoundary(int x1, int y1, int x2, int y2);
    void markTargetGoal(BosonPathInfo* info);

    unsigned int nodeStatus(BosonPathInfo* info, int x, int y);

    bool goalPassable(BosonPathInfo* info);
    int findClosestFreeGoalCell(BosonPathInfo* info);


    /*****  High-level pathfinder  *****/
    bool getHighLevelPath(BosonPathInfo* info);
    Result highLevelDoSearch(BosonPathHighLevelData* data);
    void highLevelSearchNeighbor(BosonPathHighLevelData* data, const BosonPathNode& n, unsigned int dir);
    void highLevelFinishSearch(BosonPathHighLevelData* data);
    bofixed highLevelDistToGoal(BosonPathHighLevelData* data, const BosonPathNode& n);
    void resetDirtyBlockStatuses();
    // Block-management stuff
    void findBlockCenter(int blockpos, BosonMoveData* movedata);
    void findBlockConnections(int blockpos, BosonMoveData* movedata);
    void calculateBlockConnection(int blockpos, BosonMoveData* movedata, int dir);
    void createBlockColormap(BosonMoveData* movedata);
    void markBlockChanged(Cell* c);
    void updateChangedBlocks();


    /**
     * @return Cell at given pos
     **/
    Cell* cell(int x, int y) const;
    /**
     * @return Whether cell at (x; y) is valid (i.e. it's on the map)
     **/
    bool isValidCell(int x, int y) const;
    bool cellForested(int x, int y) const;

    void setCellStatusDirty(int pos);
    void setBlockConnectionDirty(int pos);



  private:
    class CellStatus
    {
      public:
        unsigned int flags;
        bofixed cost;
    };
    // TODO: maybe rename to just block?
    class BlockInfo
    {
      public:
        BlockInfo()  { centerx = 0; centery = 0; cost = 0; flags = 0; }
        ~BlockInfo()  { delete[] centerx; delete[] centery; }

        // Center of the block's passable area for every movedata
        int* centerx;
        int* centery;

        // Cost of the block (when finding path)
        bofixed cost;
        // Status flags for pathfinder
        unsigned int flags;
    };

    BosonMap* mMap;

    // TODO: maybe use unsigned char?
    bofixed* mSlopeMap;
    bofixed* mForestMap;
    BoColorMap* mSlopeColormap;
    BoColorMap* mForestColormap;

    QValueVector<BosonMoveData*> mMoveDatas;

    int mXOffset[9];
    int mYOffset[9];

    /*****  Low-level pf stuff  *****/
    CellStatus* mCellStatus;
    int* mCellStatusDirty;
    unsigned int mCellStatusDirtyCount;
    unsigned int mCellStatusDirtySize;

    /*****  High-level pf (path estimator) stuff  *****/
    BlockInfo* mBlocks;
    int mBlocksCountX;
    int mBlocksCountY;
    int mBlockSize;
    bofixed* mBlockConnections;
    int mBlockConnectionsCount;
    bool* mBlockConnectionsDirty;
    QValueList<int> mChangedBlocks;
    QValueList<int> mDirtyConnections;
    QValueList<int> mBlockStatusDirty;

    friend class BoPathSyncCheckMessage;
};



class BosonPathNode
{
  public:
    BosonPathNode() { x = 0; y = 0; pos = 0; g = 0; h = 0; depth = 0; }

    inline bool operator<(const BosonPathNode& n2)
    {
      return ((g + h) < (n2.g + n2.h));
    }

    int x;
    int y;
    // TODO: do we _really_ need this???
    int pos;

    bofixed g;
    bofixed h;

    int depth;
};

template<class T> class BosonPathPointerNode
{
  public:
    BosonPathPointerNode() { p = 0; g = 0; h = 0; }
    BosonPathPointerNode(T* _p) { p = _p; g = 0; h = 0; }
    BosonPathPointerNode(T* _p, bofixed _g) { p = _p; g = _g; h = 0; }

    T* p;

    bofixed g;
    bofixed h;
};

class BosonPathFlyingNode
{
  public:
    BosonPathFlyingNode()  { x = 0; y = 0; rot = 0; g = 0; h = 0; depth = 0; parent = 0; }

    bofixed x;
    bofixed y;

    bofixed g;
    bofixed h;

    bofixed rot;

    int depth;

    BosonPathFlyingNode* parent;
};


template<class T> class BosonPathPointerHeap : public QValueList<T*>
{
  public:
    inline void add(T*& x)
    {
      QValueListIterator<T*> it;
      for(it = this->begin(); it != this->end(); ++it)
      {
        if((x->g + x->h) <= ((*it)->g + (*it)->h))
        {
          insert(it, x);
          break;
        }
      }
      if(it == this->end()) {
        append(x);
      }
    }

    inline void takeFirst(T*& x)
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
    BosonPathInfo()  { reset(); }
    void reset()
    {
      unit = 0;
      player = 0;
      movedata = 0;
      result = BosonPath::None;
      llpath.clear();
      hlpath.clear();
      start.reset();
      dest.reset();
      target = 0;
      range = -1;
      needpath = true;
      flying = false;
      pathcost = 0;
      moveAttacking = true;
      slowDownAtDest = true;
      waiting = 0;
      pathrecalced = 0;
    }

    bool saveAsXML(QDomElement& root);
    bool loadFromXML(const QDomElement& root);

    // Unit/player that we're searching path for. These can be 0!!!
    Unit* unit;
    PlayerIO* player;

    BosonMoveData* movedata;

    // Result of the last pathfinder query
    BosonPath::Result result;

    // Low-level path, containing waypoints
    QValueVector<BoVector2Fixed> llpath;

    QValueVector<BoVector2Fixed> hlpath;

    // Start and destination point
    BoVector2Fixed start;
    BoVector2Fixed dest;
    // If this is non-NULL, this will be used as destination and pathfinder
    //  will try to get within range cells from this item (e.g. 0 means next to
    //  the target)
    BosonItem* target;

    // Range, in cells
    // If range is -1, we try to get as close to destination point as possible,
    //  if it's bigger than -1, we either get exactly to this range, or won't
    //  find path (if it isn't possible)
    int range;

    // Whether the path should be returned. If it's false, pathfinder will only
    //  check if the path can be found or not and sets the passable variable
    //  accordingly
    bool needpath;

    bool flying;

    bofixed pathcost;


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
    void addLineVisualization(const QValueList<BoVector3Fixed>& points, const BoVector4Float& color, bofixed pointSize = 1.0f, int timeout = 60, bofixed zOffset = 0.5f);

    /**
     * @overload
     * Just like above, but with a default color
     **/
    void addLineVisualization(const QValueList<BoVector3Fixed>& points, bofixed pointSize = 1.0f, int timeout = 60, bofixed zOffset = 0.5f);

  signals:
    void signalAddLineVisualization(const QValueList<BoVector3Fixed>& points, const BoVector4Float& color, bofixed pointSize, int timeout, bofixed zOffset);

  private:
    BosonPathVisualization(QObject* parent);

  private:
    static BosonPathVisualization* mPathVisualization;
};


#endif // BOSONPATH_H

/*
 * vim: et sw=2
 */
