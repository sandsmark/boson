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
#include "bosonpath.h"
#include "bosonpath.moc"

#include "cell.h"
#include "unit.h"
#include "bosoncanvas.h"
#include "defines.h"
#include "player.h"
#include "bodebug.h"
#include "boson.h"
#include "unitproperties.h"
#include "bosonprofiling.h"
#include "bosonmap.h"
#include "unitplugins/resourcemineplugin.h"
#include "speciestheme.h"
#include "playerio.h"

#include <qptrqueue.h>
#include <qdom.h>

#include <kstaticdeleter.h>
#include <kmdcodec.h>


// If this is defined, BoLineVisualization will be used to show found paths
//#define VISUALIZE_PATHS

//#define LOW_PRIORITY_PROFILERS


BosonPathVisualization* BosonPathVisualization::mPathVisualization = 0;

static KStaticDeleter<BosonPathVisualization> sd;


const int xoffsets[] = {  0,  1,  1,  1,  0, -1, -1, -1};
const int yoffsets[] = { -1, -1,  0,  1,  1,  1,  0, -1};

/*
#define TNG_HIGH_DIST_MULTIPLIER 0.2f
#define TNG_LOW_DIST_MULTIPLIER 1.75f
//#define TNG_MAX_BORDER_COST (cost / 2.0f)
#define TNG_MAX_BORDER_COST 20
#define LOW_CROSS_DIVIDER 100.0f
#define HIGH_CROSS_DIVIDER 400.0f
#define TNG_NEAREST_G_FACTOR 0.2f
#define TNG_LOW_BASE_COST 1.5f
#define TNG_FLYING_STEPS 15
#define TNG_RANGE_STEPS TNG_FLYING_STEPS
#define TNG_FLYING_TUNRING_COST 1.0f
*/

// TODO: document the defines
#define NEAREST_G_FACTOR 0.5

#define MIN_FORESTED_VALUE 0.2
#define MAX_SHOWN_FORESTED_VALUE 0.6


#define MAXDIST_LOW 40

#define SQRT_2 1.414


#define LOW_CROSS_DIVIDER 100
#define LOW_DIST_MULTIPLIER 1.0
#define LOW_OTHERUNIT_COST 3.0
#define LOW_MOVINGUNIT_COST 1.5
#define LOW_BASE_COST 1.0
// Cost of rotating (no matter how much you rotate)
//#define LOW_ROTATION_COST 0.35
#define LOW_ROTATION_COST 0.0
// Maximum number of nodes to be looked through
#define LOW_MAX_NODES 1000
// How far from starting pos to go at most
#define LOW_MAX_RANGE 50


#define BLOCK_COST_DIST_MULTIPLIER 0.2
#define HIGH_DIST_MULTIPLIER 0.75
#define HIGH_MAX_NODES 600
#define HIGH_ROTATION_COST 1.0


// Max steps (nodes) to search ahead
#define FLYING_MAX_STEPS 50
// Distance between every two nodes
#define FLYING_NODE_DIST 1
// Maximum distance from the actual goal to count it as being on the goal
#define FLYING_MAX_GOAL_DIST FLYING_NODE_DIST
// Maximum turn the aircraft can make between two nodes (in degrees)
#define FLYING_MAX_TURN 20
// How close nodes with different rotation values will be placed (in degrees)
#define FLYING_TURN_STEP 10
// Per degree!!! This will be added to cost for every turn (helps to keep paths mostly straight)
#define FLYING_TURNING_PENALTY 0.03
// If number of nodes (in both open and closed lists) gets bigger than this, the search is interrupted
#define FLYING_MAX_NODES 1500
#define FLYING_BASE_COST 0.75
#define FLYING_DIST_MULTPLIER 1.5
#define FLYING_TURNDIST_MULTPLIER 3.0


/*****  Cell status flags  *****/
// Per-node stuff (single node can "occupy" several cells if unit is bigger than 1x1)
#define STATUS_UP              1
#define STATUS_UP_RIGHT        2
#define STATUS_RIGHT           3
#define STATUS_DOWN_RIGHT      4
#define STATUS_DOWN            5
#define STATUS_DOWN_LEFT       6
#define STATUS_LEFT            7
#define STATUS_UP_LEFT         8

#define STATUS_DIR         (STATUS_UP | STATUS_UP_RIGHT | STATUS_RIGHT | STATUS_DOWN_RIGHT | \
    STATUS_DOWN | STATUS_DOWN_LEFT | STATUS_LEFT | STATUS_UP_LEFT)
#define STATUS_OPEN         (1 << 4)
#define STATUS_CLOSED       (1 << 5)
#define STATUS_START        (1 << 6)
#define STATUS_GOAL         (1 << 7)
#define STATUS_BLOCKED      (1 << 8)
#define STATUS_DIRTY        (1 << 9)

// Per-cell stuff
#define STATUS_CALCULATED   (1 << 15)
#define STATUS_CANTGO       (1 << 16)
#define STATUS_MOVING       (1 << 17)
#define STATUS_OTHERUNIT    (1 << 18)

// Block stuff
#define STATUS_CHANGED      (1 << 25)



#define ISDIAGONALDIR(d)  (((d) % 2) == 0 ? 1 : 0)

// Low priority profiler macros
#ifdef LOW_PRIORITY_PROFILERS
#define LP_PROFILE_METHOD PROFILE_METHOD
#define LP_PROFILE_METHOD_2(var, name) PROFILE_METHOD_2(var, name)
#else
#define LP_PROFILE_METHOD
#define LP_PROFILE_METHOD_2(var, name)
#endif




template<class T> class BosonPathHeap
{
  public:
    inline BosonPathHeap(unsigned int maxitems = LOW_MAX_NODES + 2)
    {
      mCapacity = maxitems;
      mCount = 0;
      mHeap = new T[mCapacity];
    }
    ~BosonPathHeap()
    {
      delete[] mHeap;
    }

    inline void add(const T& x)
    {
      if(mCount == mCapacity)
      {
        resize(mCapacity + 200);
      }
      // Add the entry to the back of the heap
      unsigned int pos = mCount;
      mHeap[pos] = x;
      mCount++;
      // Fix the heap
      fix_upward(pos);
    }

    inline void takeFirst(T& x)
    {
      x = mHeap[0];
      mCount--;

      if(mCount > 0)
      {
        mHeap[0] = mHeap[mCount];
        fix_downward(0);
      }
    }

    bool isEmpty() const  { return (mCount == 0); }


  protected:
    inline unsigned int parentPos(unsigned int pos)  { return (pos - 1) / 2; }
    inline unsigned int leftPos(unsigned int pos)  { return pos * 2 + 1; }
    inline unsigned int rightPos(unsigned int pos)  { return pos * 2 + 2; }

    void resize(unsigned int newsize)
    {
      // Allocate new heap
      T* newheap = new T[newsize];
      // Copy items
      for(unsigned int i = 0; i < mCount; i++)
      {
        newheap[i] = mHeap[i];
      }
      // Delete current heap
      delete[] mHeap;
      mHeap = newheap;
      mCapacity = newsize;
    }

    void fix_upward(unsigned int pos)
    {
      if(pos == 0)
      {
        return;
      }
      unsigned int parent = parentPos(pos);
      if(mHeap[pos] < mHeap[parent])
      {
        // Swap
        T temp = mHeap[parent];
        mHeap[parent] = mHeap[pos];
        mHeap[pos] = temp;
        fix_upward(parent);
      }
    }

    void fix_downward(unsigned int pos)
    {
      unsigned int child = leftPos(pos);
      if(child >= mCount)
      {
        return;
      }

      // Select the smaller one of the children
      if((child+1 < mCount) && (mHeap[child+1] < mHeap[child]))
      {
        child++;
      }

      // Test the child
      if(mHeap[child] < mHeap[pos])
      {
        // Swap
        T temp = mHeap[child];
        mHeap[child] = mHeap[pos];
        mHeap[pos] = temp;
        fix_downward(child);
      }
    }


  private:
    T* mHeap;
    unsigned int mCapacity;
    unsigned int mCount;
};



class BosonPathLowLevelData
{
  public:
    BosonPathLowLevelData()  { openednodes = 0; closednodes = 0; }

    BosonPathHeap<BosonPathNode> open;
    int areax1;
    int areay1;
    int areax2;
    int areay2;
    int halfsize;
    // TODO: BETTER NAAAAAME!!!!!
    int edgedist1;
    int edgedist2;
    BosonPathNode goalnode;
    BosonPathNode nearest;
    // Actual (int) start/dest points, as used by the pathfinder
    int startx;
    int starty;
    int destx;
    int desty;

    int maxdepth;

    int mapwidth;
    BosonPathInfo* info;

    int openednodes;
    int closednodes;
};

class BosonPathHighLevelData
{
  public:
    BosonPathHighLevelData()  { openednodes = 0; closednodes = 0; }

    BosonPathHeap<BosonPathNode> open;
    BosonPathNode goalnode;
    BosonPathNode nearest;
    // Start/dest blocks
    int startblockx;
    int startblocky;
    int destblockx;
    int destblocky;

    int maxdepth;

    int mapwidth;
    BosonPathInfo* info;
    int movedataid;

    int openednodes;
    int closednodes;
};




/* NOTE:
 * Lot of code in this file is based on the pathfinder of the TA Spring project
 *  (http://taspring.clan-sy.com/), also licensed under GPL.
 */


/*****  BosonPath  *****/
BosonPath::BosonPath(BosonMap* map)
{
  boDebug(500) << k_funcinfo << endl;
  mMap = map;
  boDebug(500) << k_funcinfo << "END" << endl;
}

BosonPath::~BosonPath()
{
  delete[] mSlopeMap;
  //delete[] mForestMap;

  //mMap->removeColorMap("Forestation");
  mMap->removeColorMap("Slopes");

  delete[] mCellStatus;
  delete[] mCellStatusDirty;

  delete[] mBlocks;
  delete[] mBlockConnections;
  delete[] mBlockConnectionsDirty;
}

void BosonPath::init(BosonCanvas* canvas)
{
  PROFILE_METHOD;
  boDebug(500) << k_funcinfo << endl;
  BosonProfilingItem profiler;

  initOffsets();

  mSlopeMap = calculateSlopemap();
  //mForestMap = calculateForestmap();
  mForestMap = 0;

  initMoveDatas(canvas);
  initCellPassabilityMaps();
  initCellStatusArray();
  initBlocks();

  long int elapsed = profiler.elapsedSinceStart();
  boDebug(500) << k_funcinfo << "END, elapsed: " << elapsed / 1000.0 << " ms" << endl;
}

void BosonPath::advance()
{
  updateChangedBlocks();
}

void BosonPath::findPath(BosonPathInfo* info)
{
  //boDebug(500) << k_funcinfo << endl;
  PROFILE_METHOD;

  preparePathInfo(info);

  if(info->flying)
  {
    // Flying unit
    findFlyingUnitPath(info);
  }
  else
  {

    if(info->target)
    {
      markTargetGoal(info);
    }
    // Make sure the destination can be reached
    if(!goalPassable(info))
    {
      info->result = NoPath;
      return;
    }

    int oldrange = info->range;
    if(info->range < 0)
    {
      // Find the closest passable cell to the goal and update the range
      info->range = findClosestFreeGoalCell(info);
    }

    int dist = (int)QMAX(QABS(info->dest.x() - info->start.x()), QABS(info->dest.y() - info->start.y()));
    // Select the pathfinder according to the distance
    if(dist <= MAXDIST_LOW)
    {
      // Use the lowlevel pf
      getLowLevelPath(info);
    }
    else
    {
      // Use the high-level pathfinder
      if(getHighLevelPath(info) != NoPath)
      {
        boDebug(500) << "Found hlpath of " << info->hlpath.count() << " steps" << endl;
        getPartialLowLevelPath(info);
      }
    }
    info->range = oldrange;
  }

  // Reset statuses in the search area
  resetDirtyCellStatuses();

  long int elapsed = methodProfiler.popElapsed();
  boDebug(500) << k_funcinfo << (info->flying ? "flying " : "") <<
      "unit " << (info->unit ? (int)info->unit->id() : -1) << "; took " << elapsed/1000.0f << " ms;   result: " <<
      (info->result == GoalReached ? "GoalReached" : (info->result == OutOfRange ? "OutOfRange" :
      (info->result == NoPath ? "NoPath" : "None"))) << ", path length: " << info->llpath.count() << endl;
}

void BosonPath::preparePathInfo(BosonPathInfo* info)
{
  // Clear the pathinfo
  info->llpath.clear();
  info->hlpath.clear();
  info->result = None;

  if(info->unit)
  {
    info->movedata = info->unit->moveData();
    info->player = info->unit->ownerIO();
  }
}



bool BosonPath::goalPassable(BosonPathInfo* info)
{
  if(info->range < 0)
  {
    // -1 means to get as close as possible.
    return true;
  }

  int left = QMAX((int)info->dest.x() - info->range, info->movedata->edgedist1);
  int top = QMAX((int)info->dest.y() - info->range, info->movedata->edgedist1);
  int right = QMAX((int)info->dest.x() + info->range, (int)mMap->width() - 1 - info->movedata->edgedist2);
  int bottom = QMAX((int)info->dest.y() + info->range, (int)mMap->height() - 1 - info->movedata->edgedist2);

  for(int y = top; y <= bottom; y++)
  {
    for(int x = left; x <= right; x++)
    {
      // Check if the unit could go onto this cell
      if(!(nodeStatus(info, x, y) & STATUS_CANTGO))
      {
        // Unit could go onto this cell
        return true;
      }
    }
  }

  return false;
}

int BosonPath::findClosestFreeGoalCell(BosonPathInfo* info)
{
  int goalx = (int)info->dest.x();
  int goaly = (int)info->dest.y();

  // Find closest unoccupied cell to the destination
  for(int range = 0; range < 10; range++)
  {
    // First check upper and lower sides of "rectangle"
    for(int x = goalx - range; x <= goalx + range; x++)
    {
      if(!(nodeStatus(info, x, goaly - range) & STATUS_CANTGO))
      {
        return range;
      }
      if(!(nodeStatus(info, x, goaly + range) & STATUS_CANTGO))
      {
        return range;
      }
    }
    // Then right and left sides. Note that corners are already checked
    for(int y = goaly - range + 1; y < goaly + range; y++)
    {
      if(!(nodeStatus(info, goalx - range, y) & STATUS_CANTGO))
      {
        return range;
      }
      if(!(nodeStatus(info, goalx + range, y) & STATUS_CANTGO))
      {
        return range;
      }
    }
  }

  return -1;
}

void BosonPath::getPartialLowLevelPath(BosonPathInfo* info)
{
  // Find highlevel path point which is at a fitting distance
  /*int lasthlstep = MAXDIST_LOW / mBlockSize - 1;
  if(lasthlstep >= info->hlpath.count())
  {
    lasthlstep = info->hlpath.count() - 1;
  }*/
  BoVector2Fixed origdest = info->dest;
  int origrange = info->range;
  int lasthlstep = 0;
  if(info->hlpath.isEmpty())
  {
    boWarning(500) << k_funcinfo << "hlpath is empty!" << endl;
  }
  else
  {
    lasthlstep = QMIN(3, (int)info->hlpath.count() - 1);
    info->dest = info->hlpath[lasthlstep];
    // Getting near the hlpath's point should be enough
    info->range = 4;
  }

  // Find the path
  getLowLevelPath(info);

  if((lasthlstep < (int)info->hlpath.count() - 1) && (info->result == GoalReached))
  {
    // The actual goal wasn't reached, because we didn't use the last hlpath
    //  step. Replace GoalReached with OutOfRange
    info->result = OutOfRange;
  }

  // Copy back the original datas
  info->range = origrange;
  info->dest = origdest;
}

BosonPath::Result BosonPath::getLowLevelPath(BosonPathInfo* info)
{
  if((((int)info->start.x()) == ((int)info->dest.x())) && (((int)info->start.y()) == ((int)info->dest.y())))
  {
    info->result = GoalReached;
    return GoalReached;
  }
  // Create data object
  BosonPathLowLevelData* data = new BosonPathLowLevelData;

  data->mapwidth = (int)mMap->width();
  data->info = info;

  data->startx = (int)info->start.x();
  data->starty = (int)info->start.y();
  data->destx = (int)info->dest.x();
  data->desty = (int)info->dest.y();
  //boDebug(500) << "Start: (" << data->startx << "; " << data->starty <<
  //    "); dest: (" << data->destx << "; " << data->desty << ");  range: " << info->range << endl;

  {
  PROFILE_METHOD;
  // Create the first node
  BosonPathNode n;
  n.x = data->startx;
  n.y = data->starty;
  n.pos = n.y * data->mapwidth + n.x;
  n.g = 0;
  n.h = lowLevelDistToGoal(data, n.x, n.y);

  // Calculate area boundaries
  data->areax1 = QMAX(n.x - LOW_MAX_RANGE, 0);
  data->areay1 = QMAX(n.y - LOW_MAX_RANGE, 0);
  data->areax2 = QMIN(n.x + LOW_MAX_RANGE, (int)mMap->width() - 1);
  data->areay2 = QMIN(n.y + LOW_MAX_RANGE, (int)mMap->height() - 1);

  // We can't use the whole area because of the unit's size. We need to keep a
  //  border of cells which won't be used for nodes but which will be taken
  //  into account when checking for passability.
  // TODO: actually it should probably be the other way around: we should first
  //  clean a bigger area, then shrink it
  data->areax1 += data->info->movedata->edgedist1;
  data->areay1 += data->info->movedata->edgedist1;
  data->areax2 -= data->info->movedata->edgedist2;
  data->areay2 -= data->info->movedata->edgedist2;

  data->maxdepth = LOW_MAX_RANGE - QMAX(data->info->movedata->edgedist1, data->info->movedata->edgedist2);

  // Set starting cell's flags
  mCellStatus[n.pos].flags = STATUS_OPEN | STATUS_START;
  setCellStatusDirty(n.pos);

  // Add first node to open list
  data->open.add(n);
  data->openednodes++;
  data->nearest = n;


  // Do the search
  info->result = lowLevelDoSearch(data);
  //boDebug(500) << k_funcinfo << "res: " << res << endl;
  if(info->result != NoPath)
  {
    lowLevelFinishSearch(data);
  }
  else
  {
    //boDebug(500) << k_funcinfo << "No path found" << endl;
  }

  }
  //boDebug(500) << k_funcinfo << "Nodes opened: " << data->openednodes <<
  //    "; nodes closed: " << data->closednodes << "; path length: " << info->llpath.count() << endl;

  delete data;

  return info->result;
}

BosonPath::Result BosonPath::lowLevelDoSearch(BosonPathLowLevelData* data)
{
  LP_PROFILE_METHOD;
  // Is the path found?
  bool pathfound = false;
  bool goalreached = false;

  BosonPathNode n;
  // Main loop
  while(!data->open.isEmpty())
  {
    // Take first node from open
    data->open.takeFirst(n);
    //boDebug(500) << "  Got node from open: pos: (" << n.x << "; " << n.y << "); g: " << n.g << "; h: " << n.h << endl;
    data->closednodes++;

    // Check if we're in goal range
    if((data->info->range == -1) && (n.x == data->destx) && (n.y == data->desty))
    {
      // Couldn't get any closer...
      data->goalnode = n;
      pathfound = true;
      goalreached = true;
      break;
    }
    else if((mCellStatus[n.pos].flags & STATUS_GOAL) ||
        (QMAX(QABS(n.x - data->destx), QABS(n.y - data->desty)) <= data->info->range))
    {
      data->goalnode = n;
      pathfound = true;
      goalreached = true;
      break;
    }
    else if(data->openednodes > LOW_MAX_NODES)
    {
      data->goalnode = data->nearest;
      pathfound = true;
      break;
    }
    else if(n.depth >= data->maxdepth)
    {
      data->goalnode = data->nearest;
      pathfound = true;
      break;
    }

    // This node is now closed
    mCellStatus[n.pos].flags |= STATUS_CLOSED;

    // Search neighbors of the current node
    lowLevelSearchNeighbor(data, n, STATUS_UP);
    lowLevelSearchNeighbor(data, n, STATUS_DOWN);
    lowLevelSearchNeighbor(data, n, STATUS_LEFT);
    lowLevelSearchNeighbor(data, n, STATUS_RIGHT);
    lowLevelSearchNeighbor(data, n, STATUS_UP_LEFT);
    lowLevelSearchNeighbor(data, n, STATUS_UP_RIGHT);
    lowLevelSearchNeighbor(data, n, STATUS_DOWN_LEFT);
    lowLevelSearchNeighbor(data, n, STATUS_DOWN_RIGHT);
    /*
    bool up = lowLevelSearchNeighbor(data, n, STATUS_UP);
    bool down = lowLevelSearchNeighbor(data, n, STATUS_DOWN);
    bool left = lowLevelSearchNeighbor(data, n, STATUS_LEFT);
    bool right = lowLevelSearchNeighbor(data, n, STATUS_RIGHT);
    if(up)
    {
      if(left)
      {
        lowLevelSearchNeighbor(data, n, STATUS_UP_LEFT);
      }
      if(right)
      {
        lowLevelSearchNeighbor(data, n, STATUS_UP_RIGHT);
      }
    }
    if(down)
    {
      if(left)
      {
        lowLevelSearchNeighbor(data, n, STATUS_DOWN_LEFT);
      }
      if(right)
      {
        lowLevelSearchNeighbor(data, n, STATUS_DOWN_RIGHT);
      }
    }
    */
  }

  if(pathfound)
  {
    if(goalreached)
    {
      return GoalReached;
    }
    else
    {
      // Partial path was found
      // TODO: when range == -1, we should probably return GoalReached in some occasions
      return OutOfRange;
    }
  }
  else
  {
    return NoPath;
  }
}

bool BosonPath::lowLevelSearchNeighbor(BosonPathLowLevelData* data, const BosonPathNode& n, unsigned int dir)
{
  LP_PROFILE_METHOD;
  BosonPathNode n2;
  n2.x = n.x + mXOffset[dir];
  n2.y = n.y + mYOffset[dir];
  //boDebug(500) << "    Checking pos (" << n2.x << "; " << n2.y <<
  //    "):  dir: " << dir << "; flags: " << mCellStatus[n2.y * data->mapwidth + n2.x].flags << endl;
  // Make sure the node is in search area
  if((n2.x < data->areax1) || (n2.x > data->areax2) ||
      (n2.y < data->areay1) || (n2.y > data->areay2))
  {
    //boDebug(500) << "      Out of search area" << endl;
    return false;
  }

  n2.pos = n2.y * data->mapwidth + n2.x;

  if(mCellStatus[n2.pos].flags & STATUS_BLOCKED)
  {
    //boDebug(500) << "      Blocked" << endl;
    return false;
  }
  else if(mCellStatus[n2.pos].flags & STATUS_CLOSED)
  {
    //boDebug(500) << "      Closed " << endl;
    return true;
  }

  // Check if OPEN already has node with this pos
  if(mCellStatus[n2.pos].flags & STATUS_OPEN)
  {
    // TODO: the name 'cost' is a bit misleading here...
    if(n.g >= mCellStatus[n2.pos].cost)
    {
      // Previous route has better cost
      //boDebug(500) << "      In open with better cost (" << mCellStatus[n2.pos].cost << " vs " << n.g << ")" << endl;
      return true;
    }
    else
    {
      // Delete old direction
      mCellStatus[n2.pos].flags &= ~STATUS_DIR;
      // TODO: shouldn't the old node be deleted from open?
    }
  }

  setCellStatusDirty(n2.pos);

  LP_PROFILE_METHOD_2(avprof, "Available");
  mCellStatus[n2.pos].cost = n.g;

  // Check for occupied status
  bool movingunit = false;  // Moving unit on one of the cells
  bool otherunit = false;  // Other unit (e.g. attacking) on one of the cells
  for(int x = n2.x - data->info->movedata->edgedist1; x <= n2.x + data->info->movedata->edgedist2; x++)
  {
    for(int y = n2.y - data->info->movedata->edgedist1; y <= n2.y + data->info->movedata->edgedist2; y++)
    {
      // Calculate cell's status if it hasn't been done yet
      if(!(mCellStatus[y * data->mapwidth + x].flags & STATUS_CALCULATED))
      {
        calculateCellStatus(data->info, x, y);
      }

      // Check the status flags
      if(mCellStatus[y * data->mapwidth + x].flags & STATUS_CANTGO)
      {
        // Can't quite use this cell
        mCellStatus[n2.pos].flags |= STATUS_BLOCKED;
        //boDebug(500) << "      Can't go onto cell (" << x << "; " << y << ")" << endl;
        return false;
      }
      else if(mCellStatus[y * data->mapwidth + x].flags & STATUS_OTHERUNIT)
      {
        otherunit = true;
      }
      else if(mCellStatus[y * data->mapwidth + x].flags & STATUS_MOVING)
      {
        movingunit = true;
      }
    }
  }
  LP_PROFILE_METHOD_2(noprof, "Not occupied");

  n2.depth = n.depth + 1;

  // Calculate costs
  if(otherunit)
  {
    n2.g = LOW_OTHERUNIT_COST;
  }
  else if(movingunit)
  {
    n2.g = LOW_MOVINGUNIT_COST;
  }
  else
  {
    n2.g = LOW_BASE_COST;
  }

  // Diagonal movement costs more (because you actually travel longer)
  if(ISDIAGONALDIR(dir))
  {
    n2.g *= SQRT_2;
  }

  // Penalty for rotating
  if((mCellStatus[n.pos].flags & STATUS_DIR) != dir)
  {
    n2.g += LOW_ROTATION_COST;
  }

  n2.g += n.g;
  n2.h = lowLevelDistToGoal(data, n2.x, n2.y);

  // Check if it's the nearest node so far
  if((n2.h + n2.g * NEAREST_G_FACTOR) < (data->nearest.h + data->nearest.g * NEAREST_G_FACTOR))
  {
    data->nearest = n2;
  }

  // Add the node to open
  //boDebug(500) << "      Adding to open; g: " << n2.g << "; h: " << n2.h << endl;
  data->open.add(n2);
  data->openednodes++;
  mCellStatus[n2.pos].flags |= (STATUS_OPEN | dir);

  return true;
}

void BosonPath::lowLevelFinishSearch(BosonPathLowLevelData* data)
{
  LP_PROFILE_METHOD;
  //boDebug(500) << k_funcinfo << endl;

  data->info->pathcost = data->goalnode.g;

  if(!data->info->needpath)
  {
    return;
  }

  bofixed add = (((data->info->movedata->size % 2) == 1) ? 0.5 : 0);

  QValueList<BoVector2Fixed> temp;
  // Coordinate of the last node in the path (destination)
  int x = data->goalnode.x;
  int y = data->goalnode.y;

  data->info->pathcost = data->goalnode.g;

  // Add all nodes until we reach the start node
  while(true)
  {
    int pos = y * data->mapwidth + x;  // Necessary because we'll change x and y
    //boDebug(500) << "    Tracing at (" << x << "; " << y << "); flags: " << mCellStatus[pos].flags << endl;
    if(mCellStatus[pos].flags & STATUS_START)
    {
      // The STATUS_START flag might confuse next pf query, so remove it
      mCellStatus[pos].flags &= ~STATUS_START;
      //boDebug(500) << "  Starting cell found. Break." << endl;
      break;
    }
    // Failsafe
    if(temp.count() > 10000)
    {
      boError(500) << k_funcinfo << "Temp is too big! Last pos: (" << x << "; " << y <<
          "); flags: " << mCellStatus[pos].flags << endl;
      break;
    }

    // Add node to the path
    temp.prepend(BoVector2Fixed(x + add, y + add));

    // And take the next one
    x -= mXOffset[mCellStatus[pos].flags & STATUS_DIR];
    y -= mYOffset[mCellStatus[pos].flags & STATUS_DIR];
  }

  // Copy temp path to real path vector
  data->info->llpath.clear();
  data->info->llpath.reserve(temp.count());
  QValueList<BoVector2Fixed>::iterator it;
  for(it = temp.begin(); it != temp.end(); ++it)
  {
    data->info->llpath.append(*it);
  }
  //boDebug(500) << k_funcinfo << "found path has " << data->info->llpath.count() << " steps" << endl;
}

bofixed BosonPath::lowLevelDistToGoal(BosonPathLowLevelData* data, int x, int y) const
{
  int dx1 = x - data->destx;
  int dy1 = y - data->desty;
  int dx2 = data->startx - x;
  int dy2 = data->starty - y;
  int cross = dx1 * dy2 - dx2 * dy1;
  if(cross < 0)
  {
    cross = -cross;
  }
  dx1 = QABS(dx1);
  dy1 = QABS(dy1);

  // Estimate of the true straight-line distance
  return (bofixed(cross) / LOW_CROSS_DIVIDER) + (QMAX(dx1, dy1) + QMIN(dx1, dy1) * 0.4) * LOW_DIST_MULTIPLIER;
}

void BosonPath::calculateCellStatus(BosonPathInfo* info, int x, int y)
{
  LP_PROFILE_METHOD;
  BosonMoveData* movedata = info->movedata;

  // This cell's occupied status will now be calculated
  int pos = y * mMap->width() + x;
  mCellStatus[pos].flags |= STATUS_CALCULATED;
  setCellStatusDirty(pos);

  if(!movedata->cellPassable[pos])
  {
    mCellStatus[pos].flags |= STATUS_CANTGO;
    return;
  }

  // Go through all items on the cell and look for interesting ones
  const BoItemList* items = cell(x, y)->items();
  for(BoItemList::ConstIterator it = items->begin(); it != items->end(); ++it)
  {
    if(RTTI::isUnit((*it)->rtti()))
    {
      Unit* u = (Unit*)*it;
      if(u->isFlying())
      {
        // We don't care about air units
        continue;
      }
      else if(u == info->unit)
      {
        continue;
      }

      // Maybe we can just crush the obstacle
      if(u->maxHealth() <= movedata->crushDamage)
      {
        if(u->owner()->isNeutralPlayer())
        {
          // Crush it
          // TODO: maybe have additional cost for crushing neutral stuff???
          continue;
        }
        else if(info->player)
        {
          // Check player's relationship with the u's owner
          if(info->player->isEnemy(u))
          {
            // Crush the damn enemy :-)
            continue;
          }
          else if(info->player->isNeutral(u))
          {
            // Also crush it
            // TODO: maybe have additional cost for crushing neutral stuff???
            continue;
          }
        }
      }

      if(u->movingStatus() == UnitBase::Standing)
      {
        // This one's occupying the cell
        mCellStatus[pos].flags |= STATUS_CANTGO;
        // If it's occupied, we don't care about other stuff
        return;
      }
      else if(u->movingStatus() == UnitBase::Moving)
      {
        mCellStatus[pos].flags |= STATUS_MOVING;
      }
      else
      {
        mCellStatus[pos].flags |= STATUS_OTHERUNIT;
      }
    }
    // TODO: check for e.g. mines
  }
}

unsigned int BosonPath::nodeStatus(BosonPathInfo* info, int x, int y)
{
  if(x < info->movedata->edgedist1 || y < info->movedata->edgedist1 ||
    x > (int)mMap->width() - 1 - info->movedata->edgedist2 || y > (int)mMap->height() - 1 - info->movedata->edgedist2)
  {
    return STATUS_CANTGO;
  }

  unsigned int status = 0;
  for(int x2 = x - info->movedata->edgedist1; x2 <= x + info->movedata->edgedist2; x2++)
  {
    for(int y2 = y - info->movedata->edgedist1; y2 <= y + info->movedata->edgedist2; y2++)
    {
      if(!(mCellStatus[y2 * mMap->width() + x2].flags & STATUS_CALCULATED))
      {
        calculateCellStatus(info, x2, y2);
      }
      status |= mCellStatus[y2 * mMap->width() + x2].flags;
    }
  }

  return status;
}

void BosonPath::lowLevelSetAreaBoundary(int x1, int y1, int x2, int y2)
{
  // Upper edge
  if(y1 - 1 >= 0)
  {
    for(int x = x1; x <= x2; x++)
    {
      int pos = (y1-1) * mMap->width() + x;
      mCellStatus[pos].flags |= STATUS_BLOCKED;
      setCellStatusDirty(pos);
    }
  }
  // Bottom edge
  if(y2 + 1 < (int)mMap->height())
  {
    for(int x = x1; x <= x2; x++)
    {
      int pos = (y2+1) * mMap->width() + x;
      mCellStatus[pos].flags |= STATUS_BLOCKED;
      setCellStatusDirty(pos);
    }
  }
  // Left edge
  if(x1 - 1 >= 0)
  {
    for(int y = y1; y <= y2; y++)
    {
      int pos = y * mMap->width() + (x1-1);
      mCellStatus[pos].flags |= STATUS_BLOCKED;
      setCellStatusDirty(pos);
    }
  }
  // Right edge
  if(x2 + 1 < (int)mMap->height())
  {
    for(int y = y1; y <= y2; y++)
    {
      int pos = y * mMap->width() + (x2+1);
      mCellStatus[pos].flags |= STATUS_BLOCKED;
      setCellStatusDirty(pos);
    }
  }
}

void BosonPath::markTargetGoal(BosonPathInfo* info)
{
  // Mark the target and 1-cell border around it as goal
  BosonItem* target = info->target;
  int left = QMAX((int)target->leftEdge() - 1 - info->range - info->movedata->edgedist2, 0);
  int top = QMAX((int)target->topEdge() - 1 - info->range - info->movedata->edgedist2, 0);
  int right = QMIN((int)ceilf(target->rightEdge()) + info->range
      + info->movedata->edgedist1, (int)mMap->width() - 1);
  int bottom = QMIN((int)ceilf(target->bottomEdge()) + info->range
      + info->movedata->edgedist1, (int)mMap->height() - 1);

  for(int x = left; x <= right; x++)
  {
    for(int y = top; y <= bottom; y++)
    {
      mCellStatus[y * mMap->width() + x].flags |= STATUS_GOAL;
      setCellStatusDirty(y * mMap->width() + x);
    }
  }

  // Alter destination point
  info->dest.set(target->center());
  // Alter range to get within given range _from the target_
  /*if(info->range >= 0)
  {
    info->range += (int)ceilf(target->width() / 2) + info->movedata->edgedist1;
  }*/
}

void BosonPath::resetDirtyCellStatuses()
{
  //boDebug(500) << k_funcinfo << "Resetting " << mCellStatusDirtyCount << " dirty cells" << endl;
  LP_PROFILE_METHOD;
  for(unsigned int i = 0; i < mCellStatusDirtyCount; i++)
  {
    // Reset cell status
    mCellStatus[mCellStatusDirty[i]].flags = 0;
  }
  mCellStatusDirtyCount = 0;
}

void BosonPath::setCellStatusDirty(int pos)
{
  if(mCellStatus[pos].flags & STATUS_DIRTY)
  {
    return;
  }

  if(mCellStatusDirtyCount == mCellStatusDirtySize)
  {
    // Allocate new, bigger array and copy old items
    int* oldarray = mCellStatusDirty;
    mCellStatusDirtySize *= 2;
    mCellStatusDirty = new int[mCellStatusDirtySize];
    for(unsigned int i = 0; i < mCellStatusDirtyCount; i++)
    {
      mCellStatusDirty[i] = oldarray[i];
    }
    delete[] oldarray;
  }
  mCellStatusDirty[mCellStatusDirtyCount++] = pos;
  mCellStatus[pos].flags |= STATUS_DIRTY;
}



bool BosonPath::getHighLevelPath(BosonPathInfo* info)
{
  // Create data object
  BosonPathHighLevelData* data = new BosonPathHighLevelData;

  data->mapwidth = (int)mMap->width();
  data->info = info;
  data->movedataid = info->movedata->id;
  // TODO: use a meaningful value
  data->maxdepth = 50;

  data->startblockx = (int)info->start.x() / mBlockSize;
  data->startblocky = (int)info->start.y() / mBlockSize;
  data->destblockx = (int)info->dest.x() / mBlockSize;
  data->destblocky = (int)info->dest.y() / mBlockSize;

  //boDebug(500) << "Start: (" << data->startblockx << "; " << data->startblocky <<
  //    "); dest: (" << data->destblockx << "; " << data->destblocky << ");  range: " << info->range << endl;

  BosonPathNode start;
  start.x = data->startblockx;
  start.y = data->startblocky;
  start.pos = start.y * mBlocksCountX + start.x;
  start.g = 0;
  start.h = highLevelDistToGoal(data, start);
  mBlocks[start.pos].flags = STATUS_START | STATUS_OPEN;
  mBlockStatusDirty.append(start.pos);

  data->open.add(start);
  data->openednodes++;
  data->nearest = start;


  // Do the search
  Result res = highLevelDoSearch(data);
  //boDebug(500) << k_funcinfo << "res: " << res << endl;
  if(res != NoPath)
  {
    highLevelFinishSearch(data);
  }
  else
  {
    boDebug(500) << k_funcinfo << "No path found" << endl;
    info->result = NoPath;
  }

  //boDebug(500) << k_funcinfo << "Nodes opened: " << data->openednodes <<
  //    "; nodes closed: " << data->closednodes << "; path length: " << info->hlpath.count() << endl;
  // Reset statuses in the search area
  resetDirtyBlockStatuses();

  delete data;

  return res;
}

BosonPath::Result BosonPath::highLevelDoSearch(BosonPathHighLevelData* data)
{
  LP_PROFILE_METHOD;
  // Is the path found?
  bool pathfound = false;
  bool goalreached = false;

  BosonPathNode n;
  // Main loop
  while(!data->open.isEmpty())
  {
    // Take first node from open
    data->open.takeFirst(n);
    //boDebug(500) << "  Got node from open: pos: (" << n.x << "; " << n.y <<
    //    "); g: " << n.g << "; h: " << n.h << "; total: " << n.g + n.h << endl;
    data->closednodes++;

    // Check if we're in goal range
    // TODO: support range!
    if(QMAX(QABS(n.x - data->destblockx), QABS(n.y - data->destblocky)) == 0)
    {
      data->goalnode = n;
      pathfound = true;
      goalreached = true;
      break;
    }
    else if(data->openednodes > HIGH_MAX_NODES)
    {
      data->goalnode = data->nearest;
      pathfound = true;
      break;
    }
    else if(n.depth >= data->maxdepth)
    {
      data->goalnode = data->nearest;
      pathfound = true;
      break;
    }

    // This node is now closed
    mBlocks[n.pos].flags |= STATUS_CLOSED;

    // Search neighbors of the current node
    highLevelSearchNeighbor(data, n, STATUS_UP);
    highLevelSearchNeighbor(data, n, STATUS_DOWN);
    highLevelSearchNeighbor(data, n, STATUS_LEFT);
    highLevelSearchNeighbor(data, n, STATUS_RIGHT);
    highLevelSearchNeighbor(data, n, STATUS_UP_LEFT);
    highLevelSearchNeighbor(data, n, STATUS_UP_RIGHT);
    highLevelSearchNeighbor(data, n, STATUS_DOWN_LEFT);
    highLevelSearchNeighbor(data, n, STATUS_DOWN_RIGHT);
  }

  if(pathfound)
  {
    if(goalreached)
    {
      return GoalReached;
    }
    else
    {
      // Partial path was found
      return OutOfRange;
    }
  }
  else
  {
    return NoPath;
  }
}

void BosonPath::highLevelSearchNeighbor(BosonPathHighLevelData* data, const BosonPathNode& n, unsigned int dir)
{
  LP_PROFILE_METHOD;
  BosonPathNode n2;
  n2.x = n.x + mXOffset[dir];
  n2.y = n.y + mYOffset[dir];
  //boDebug(500) << "    Checking pos (" << n2.x << "; " << n2.y <<
  //    "):  dir: " << dir << "; flags: " << mBlocks[n2.y * mBlocksCountX + n2.x].flags << endl;
  // Make sure the node is in search area
  if((n2.x < 0) || (n2.x >= mBlocksCountX) ||
      (n2.y < 0) || (n2.y >= mBlocksCountY))
  {
    //boDebug(500) << "      Out of search area" << endl;
    return;
  }

  n2.pos = n2.y * mBlocksCountX + n2.x;

  if(mBlocks[n2.pos].flags & (STATUS_CLOSED | STATUS_BLOCKED))
  {
    //boDebug(500) << "      Closed or blocked" << endl;
    return;
  }

  if(mBlocks[n2.pos].centerx[data->movedataid] == -1)
  {
    // TODO: maybe set block's status to STATUS_BLOCKED?
    //boDebug(500) << "      Not passable" << endl;
    return;
  }

  // Calculate positions of the connection between n2 and n
  int connectionpos = data->movedataid * mBlocksCountX * mBlocksCountY * 4;
  if(dir <= 4)
  {
    // Connection goes from n to n2
    connectionpos += n.pos * 4 + (((int)dir) - 1);
  }
  else
  {
    // Connection goes from n2 to n
    connectionpos += n2.pos * 4 + (((int)dir) - 1 - 4);
  }

  // Check if the connection exists
  if(mBlockConnections[connectionpos] == -1)
  {
    // TODO: maybe set block's status to blocked
    //boDebug(500) << "      No connection" << endl;
    return;
  }


  // Check if OPEN already has node with this pos
  if(mBlocks[n2.pos].flags & STATUS_OPEN)
  {
    // TODO: the name 'cost' is a bit misleading here...
    if(n.g >= mBlocks[n2.pos].cost)
    {
      // Previous route has better cost
      //boDebug(500) << "      In open with better cost (" << mBlocks[n2.pos].cost << " vs " << n.g << ")" << endl;
      return;
    }
    else
    {
      // Delete old direction
      mBlocks[n2.pos].flags &= ~STATUS_DIR;
      // TODO: shouldn't the old node be deleted from open?
    }
  }

  mBlockStatusDirty.append(n2.pos);

  mBlocks[n2.pos].cost = n.g;

  n2.depth = n.depth + 1;


  // Set cost of coming from n to n2
  n2.g = mBlockConnections[connectionpos];
  //boDebug(500) << "      Connection cost: " << n2.g << endl;

  // Penalty for rotating
  if((mBlocks[n.pos].flags & STATUS_DIR) != dir)
  {
    n2.g += HIGH_ROTATION_COST;
  }

  n2.g += n.g;
  n2.h = highLevelDistToGoal(data, n2);

  // Check if it's the nearest node so far
  if((n2.h + n2.g * NEAREST_G_FACTOR) < (data->nearest.h + data->nearest.g * NEAREST_G_FACTOR))
  {
    data->nearest = n2;
  }

  // Add the node to open
  //boDebug(500) << "      Adding to open; g: " << n2.g << "; h: " << n2.h << "; total: " << n2.g + n2.h << endl;
  data->open.add(n2);
  data->openednodes++;
  mBlocks[n2.pos].flags |= (STATUS_OPEN | dir);
}

void BosonPath::highLevelFinishSearch(BosonPathHighLevelData* data)
{
  LP_PROFILE_METHOD;
  boDebug(500) << k_funcinfo << endl;

  data->info->pathcost = data->goalnode.g;

  if(!data->info->needpath)
  {
    return;
  }

  bofixed add = (((data->info->movedata->size % 2) == 1) ? 0.5 : 0);

  QValueList<BoVector2Fixed> temp;
  // Coordinate of the last node in the path (destination)
  int x = data->goalnode.x;
  int y = data->goalnode.y;

  data->info->pathcost = data->goalnode.g;

  // Add all nodes until we reach the start node
  while(true)
  {
    int pos = y * mBlocksCountX + x;  // Necessary because we'll change x and y
    //boDebug(500) << "    Tracing at (" << x << "; " << y << "); flags: " << mBlocks[pos].flags << endl;
    if(mBlocks[pos].flags & STATUS_START)
    {
      //boDebug(500) << "  Starting block found. Break." << endl;
      // The STATUS_START flag might confuse next pf query, so remove it
      mBlocks[pos].flags &= ~STATUS_START;
      break;
    }
    // Failsafe
    if(temp.count() > 10000)
    {
      boError(500) << k_funcinfo << "Temp is too big! Last pos: (" << x << "; " << y <<
          "); flags: " << mBlocks[pos].flags << endl;
      break;
    }

    // Add node to the path
    int realx = mBlocks[pos].centerx[data->movedataid];
    int realy = mBlocks[pos].centery[data->movedataid];
    temp.prepend(BoVector2Fixed(realx + add, realy + add));

    // And take the next one
    x -= mXOffset[mBlocks[pos].flags & STATUS_DIR];
    y -= mYOffset[mBlocks[pos].flags & STATUS_DIR];
  }


  // Copy temp path to real path vector
  data->info->hlpath.clear();
  data->info->hlpath.reserve(temp.count());
  QValueList<BoVector2Fixed>::iterator it;
  for(it = temp.begin(); it != temp.end(); ++it)
  {
    data->info->hlpath.append(*it);
  }
#ifdef VISUALIZE_PATHS
  {
    QValueList<BoVector3Fixed> points;
    for(unsigned int point = 0; point < data->info->hlpath.count(); point++)
    {
      bofixed x = data->info->hlpath[point].x();
      bofixed y = data->info->hlpath[point].y();
      points.append(BoVector3Fixed(x, -y, 0.0f));
    }
    bofixed pointSize = 5.0f;
    int timeout = 100;
    bofixed zOffset = 0.5f;
    BoVector4Float color(0.5, 0.5f, 0.75f, 1.08f);
    BosonPathVisualization::pathVisualization()->addLineVisualization(points, color, pointSize, timeout, zOffset);
  }
#endif
  //boDebug(500) << k_funcinfo << "found path has " << data->info->llpath.count() << " steps" << endl;
}

bofixed BosonPath::highLevelDistToGoal(BosonPathHighLevelData* data, const BosonPathNode& n)
{
  BlockInfo* dest = &mBlocks[data->destblocky * mBlocksCountX + data->destblockx];
  BlockInfo* current = &mBlocks[n.y * mBlocksCountX + n.x];
  int dx = QABS(current->centerx[data->movedataid] - dest->centerx[data->movedataid]);
  int dy = QABS(current->centery[data->movedataid] - dest->centery[data->movedataid]);

  return (QMAX(dx, dy) + QMIN(dx, dy) * 0.4) * HIGH_DIST_MULTIPLIER;
}

void BosonPath::resetDirtyBlockStatuses()
{
  //boDebug(500) << k_funcinfo << "Resetting " << mCellStatusDirtyCount << " dirty cells" << endl;
  LP_PROFILE_METHOD;
  while(!mBlockStatusDirty.isEmpty())
  {
    int pos = mBlockStatusDirty.first();
    mBlockStatusDirty.pop_front();
    mBlocks[pos].flags = 0;
  }
}



void BosonPath::findFlyingUnitPath(BosonPathInfo* info)
{
  PROFILE_METHOD;
  // List of open nodes
  BosonPathPointerHeap<BosonPathFlyingNode> open;
  QValueList<BosonPathFlyingNode*> closed;

  // Create the first node
  BosonPathFlyingNode* n;
  n = new BosonPathFlyingNode;
  n->x = info->start.x();
  n->y = info->start.y();
  n->depth = 0;
//  n->rot = (360 - info->unit->rotation()) + 90;
  n->rot = info->unit->rotation() - 90;
  n->g = 0;
  n->h = flyingDistToGoal(n->x, n->y, n->rot, info);
  /*boDebug(500) << "  " << "First node " << n << ": pos: (" << n->x << "; " << n->y << ")" <<
      "; rot = " << n->rot <<
      ";  g = " << n->g <<
      "; h = " << n->h << endl;*/

  open.add(n);

  // Is the path found?
  bool pathfound = false;
  bool goalReached = false;

  // When range is -1 and we can't get exactly to destination point, we will go
  //  to nearest possible point
  BosonPathFlyingNode* nearest = n;


  // Main loop
  while(!open.isEmpty())
  {
    // Take first node from open
    open.takeFirst(n);
    // Add it to closed list
    closed.append(n);
    //boDebug(500) << "Got node " << n << " from OPEN" << endl;

    // We only search FLYING_MAX_STEPS steps ahead
    if(n->depth >= FLYING_MAX_STEPS)
    {
      n = nearest;
      pathfound = true;
      break;
    }
    else if(closed.count() + open.count() > FLYING_MAX_NODES)
    {
      boWarning(500) << k_funcinfo << "Node count bigger than FLYING_MAX_NODES. Interrupting." << endl;
      n = nearest;
      pathfound = true;
      break;
    }

    // Check if it's the goal
    bofixed dist = QMAX(QABS(n->x - info->dest.x()), QABS(n->y - info->dest.y()));
    if(info->range >= 0)
    {
      if(dist <= info->range)
      {
        // This is one of the destination cells
//        boDebug(500) << "" << k_funcinfo << "goal cell found, breaking" << endl;
        pathfound = true;
        goalReached = true;
        break;
      }
    }
    else
    {
      // range -1 means to get as close as possible
      if(dist <= FLYING_MAX_GOAL_DIST)
      {
        // we're at dest point
        // Modify the node's pos a bit to make it match the dist
        n->x = info->dest.x();
        n->y = info->dest.y();
        pathfound = true;
        goalReached = true;
        break;
      }
    }

    // Add neighbor nodes to open
    for(bofixed r = -FLYING_MAX_TURN; r <= FLYING_MAX_TURN; r += FLYING_TURN_STEP)
    {
      BosonPathFlyingNode* n2 = new BosonPathFlyingNode;
      n2->rot = n->rot + r;
      n2->x = n->x + cos(Bo3dTools::deg2rad(n2->rot)) * FLYING_NODE_DIST;
      n2->y = n->y + sin(Bo3dTools::deg2rad(n2->rot)) * FLYING_NODE_DIST;
      n2->depth = n->depth + 1;
      n2->parent = n;

      // Make sure cell is in search area
      if((n2->x < 0) || (n2->x >= mMap->width()) || (n2->y < 0) || (n2->y >= mMap->height()))
      {
        // Discard this node
        delete n2;
        continue;
      }

      // TODO: do we want/need this for _air_ units?
      // Make sure cell is passable
      /*if(mSlopeMap[(int)(n2->y * mMap->width() + n2->x)] > 45)
      {
        delete n2;
        continue;
      }*/
      // And not occupied
      /*else if(cell(n2->x, n2->y)->isAirOccupied())
      {
        delete n2;
        continue;
      }*/


      // Calculate costs
      n2->g = n->g + flyingCost(n2->x, n2->y, n2->rot, info);
      // Small penalty for turning
      n2->g += FLYING_TURNING_PENALTY * QABS(n2->rot - n->rot);
      n2->h = flyingDistToGoal(n2->x, n2->y, n2->rot, info);

      /*boDebug(500) << "  " << "Node " << n2 << ": pos: (" << n2->x << "; " << n2->y << ")" <<
          "; rot = " << n2->rot << " (" << r << ")" <<
          ";  depth = " << n2->depth <<
          ";  g = " << n->g << " + " << n2->g - n->g << " = " << n2->g <<
          "; h = " << n2->h << endl;*/

      // Check if n2 is nearest node so far
      if((n2->h + n2->g * NEAREST_G_FACTOR) < (nearest->h + nearest->g * NEAREST_G_FACTOR))
      {
        nearest = n2;
      }

      // Add node to open
      open.add(n2);
    }
  }

  boDebug(500) << k_funcinfo << "n->depth: " << n->depth << "; nodes: open: " << open.count() <<
      "; closed: " << closed.count() << "; total: " << open.count() + closed.count() << endl;

  // Traceback path
  if(!pathfound && info->range >= 0)
  {
    boError(500) << k_funcinfo << "No path found!!!" << endl;
    info->result = NoPath;
  }
  else
  {
    PROFILE_METHOD_2(postprocprofiler, "Path postprocessing");
    if(!pathfound)
    {
      // We didn't find exact path to destination, so use nearest point instead
      n = nearest;
    }

    QValueList<BoVector2Fixed> temp;
    // Coordinate of the last node in the path (destination)
    // TODO: do we need to add 0.5 to those coords???
    temp.prepend(BoVector2Fixed(n->x, n->y));
    while(n->parent)
    {
      // Take next node
      n = n->parent;
      // We must prepend regions, not append them, because we go from destination
      //  to start here
      // TODO: do we need to add 0.5 to those coords???
      temp.prepend(BoVector2Fixed(n->x, n->y));
    }

    // If path was found but goal wasn't reached, then we found only a partial
    //  path.
    if(!goalReached && pathfound)
    {
      // We have reached next region
      info->result = OutOfRange;
    }
    else
    {
      // We have reached destination (or nearest possible point from it)
      info->result = GoalReached;
    }

    // Copy temp path to real path vector
    info->llpath.clear();
    info->llpath.reserve(temp.count());
    QValueList<BoVector2Fixed>::iterator it;
    for(it = temp.begin(); it != temp.end(); ++it)
    {
      info->llpath.append(*it);
    }
    boDebug(500) << k_funcinfo << "found path has " << info->llpath.count() << " steps" << endl;

#ifdef VISUALIZE_PATHS
    // Add LineVisualization stuff
    {
    PROFILE_METHOD_2(vizprofiler, "Vizualizations");
    {
      QValueList<BoVector3Fixed> points;
      for(unsigned int point = 0; point < info->llpath.count(); point++)
      {
        bofixed x = info->llpath[point].x();
        bofixed y = info->llpath[point].y();
//        boDebug(500) << "  " << k_funcinfo << "Adding lineviz for point (" << x << "; " << y << ")" << endl;
        points.append(BoVector3Fixed(x, -y, 0.0f));
      }
      bofixed pointSize = 3.0f;
      int timeout = 100;
      bofixed zOffset = 0.5f;
      BoVector4Float color(1.0f, 0.5f, 0.0f, 0.8f); // orange
      BosonPathVisualization::pathVisualization()->addLineVisualization(points, color, pointSize, timeout, zOffset);
    }
    {
      const bofixed pointSize = 1.0f;
      const int timeout = 80;
      const bofixed zOffset = 0.4f;
      const BoVector4Float opencolor(1.0f, 0.7f, 0.6f, 0.5f);
      int i = 0;
      QValueList<BosonPathFlyingNode*>::Iterator it;
      for(it = open.begin(); it != open.end(); ++it, i++)
      {
        BosonPathFlyingNode* node = *it;
        if(!node->parent)
        {
          continue;
        }
        QValueList<BoVector3Fixed> points;
        points.append(BoVector3Fixed(node->x, -node->y, 0.0f));
        points.append(BoVector3Fixed(node->parent->x, -node->parent->y, 0.0f));
        BosonPathVisualization::pathVisualization()->addLineVisualization(points, opencolor, pointSize, timeout, zOffset);
      }
      const BoVector4Float closedcolor(0.4f, 0.4f, 0.4f, 0.5f);
      for(it = closed.begin(); it != closed.end(); ++it, i++)
      {
        BosonPathFlyingNode* node = *it;
        if(!node->parent)
        {
          continue;
        }
        QValueList<BoVector3Fixed> points;
        points.append(BoVector3Fixed(node->x, -node->y, 0.0f));
        points.append(BoVector3Fixed(node->parent->x, -node->parent->y, 0.0f));
        BosonPathVisualization::pathVisualization()->addLineVisualization(points, closedcolor, pointSize, timeout, zOffset);
      }
    }
    }
#endif
  }

  // Delete the nodes in open and closed
  QValueList<BosonPathFlyingNode*>::Iterator it;
  for(it = open.begin(); it != open.end(); ++it)
  {
    delete *it;
  }
  for(it = closed.begin(); it != closed.end(); ++it)
  {
    delete *it;
  }
}

bofixed BosonPath::flyingDistToGoal(bofixed x, bofixed y, bofixed rot, BosonPathInfo* info)
{
  BoVector2Fixed todest(info->dest.x() - x, info->dest.y() - y);
  BoVector2Fixed tostart(info->start.x() - x, info->start.y() - y);

  /*bofixed dx1 = x - info->dest.x();
  bofixed dy1 = y - info->dest.y();
  bofixed dx2 = info->start.x() - x;
  bofixed dy2 = info->start.y() - y;
  bofixed cross = dx1 * dy2 - dx2 * dy1;
  if(cross < 0)
  {
    cross = -cross;
  }*/

  BoVector2Fixed heading(cos(Bo3dTools::deg2rad(rot)), sin(Bo3dTools::deg2rad(rot)));
  // How much does current heading differ from the one we need
  BoVector2Fixed todestnorm = todest / todest.length();
  bofixed headingdot = heading.x() * todestnorm.x() + heading.y() * todestnorm.y();
  // Estimation of extra distance we need to travel to get correct heading
  bofixed extradist = (1 - headingdot) * 4.7;

  /*boDebug(500) << "    " << "turncost = " << extradist << "*" << FLYING_TURNDIST_MULTPLIER <<
      " = " << extradist * FLYING_TURNDIST_MULTPLIER <<
      "; distcost = " << QMAX(QABS(todest.x()), QABS(todest.y())) << "*" << FLYING_DIST_MULTPLIER <<
      " = " << QMAX(QABS(todest.x()), QABS(todest.y())) * FLYING_DIST_MULTPLIER << endl;*/
  return extradist * FLYING_TURNDIST_MULTPLIER + QMAX(QABS(todest.x()), QABS(todest.y())) * FLYING_DIST_MULTPLIER;
}

bofixed BosonPath::flyingCost(bofixed x, bofixed y, bofixed rot, BosonPathInfo* info)
{
  return FLYING_BASE_COST;
}

void BosonPath::initOffsets()
{
  mXOffset[0] = 0;
  mYOffset[0] = 0;
  for(unsigned int i = 1; i <= 9; i++)
  {
    mXOffset[i] = xoffsets[i-1];
    mYOffset[i] = yoffsets[i-1];
  }
}

void BosonPath::initMoveDatas(BosonCanvas* canvas)
{
  PROFILE_METHOD;
  // TODO: delete current movedatas
  mMoveDatas.clear();
  canvas->clearMoveDatas();

  // Go through all units and create all possible movedatas
  QPtrListIterator<Player> playerit(*boGame->gamePlayerList());
  while(playerit.current())
  {
    Player* p = playerit.current();
    SpeciesTheme* theme = p->speciesTheme();

    QValueList<unsigned long int> unitpropids = theme->allMobiles();
    QValueList<unsigned long int>::Iterator it;
    for(it = unitpropids.begin(); it != unitpropids.end(); ++it)
    {
      const UnitProperties* prop = theme->unitProperties(*it);
      if(prop->isAircraft())
      {
        // Flying units are special. Skip it
        continue;
      }

      BosonMoveData::Type type = (prop->isLand() ? BosonMoveData::Land : BosonMoveData::Water);
      int size = (int)ceilf(QMAX(prop->unitWidth(), prop->unitHeight()));
      // Other parameters aren't used yet

      BosonMoveData* data = 0;
      for(unsigned int i = 0; i < mMoveDatas.count(); i++)
      {
        BosonMoveData* d = mMoveDatas[i];
        if((d->type == type) && (d->size == size) && (d->crushDamage == prop->crushDamage()) &&
            (d->maxSlope == prop->maxSlope()) && (d->waterDepth == prop->waterDepth()))
        {
          // This one matches
          data = d;
          break;
        }
      }
      if(!data)
      {
        // Gotta make a new MoveData
        data = new BosonMoveData;
        data->type = type;
        data->size = size;
        data->crushDamage = prop->crushDamage();
        data->maxSlope = prop->maxSlope();
        data->waterDepth = prop->waterDepth();

        data->edgedist1 = size / 2;
        data->edgedist2 = size - data->edgedist1 - 1;
        data->id = mMoveDatas.count();
        // Add it to list of MoveDatas
        mMoveDatas.append(data);
      }

      // Set unit's movedata to data
      canvas->insertMoveData(prop, data);
    }

    ++playerit;
  }

  boDebug(500) << k_funcinfo << "Created " << mMoveDatas.count() << " movedatas" << endl;
}

void BosonPath::initCellStatusArray()
{
  PROFILE_METHOD;
  int cells = mMap->width() * mMap->height();
  mCellStatus = new CellStatus[cells];
  for(int i = 0; i < cells; i++)
  {
    mCellStatus[i].flags = 0;
  }

  mCellStatusDirtyCount = 0;
  mCellStatusDirtySize = 2 * LOW_MAX_NODES;
  mCellStatusDirty = new int[mCellStatusDirtySize];
}

void BosonPath::initCellPassabilityMaps()
{
  PROFILE_METHOD;

  for(unsigned int i = 0; i < mMoveDatas.count(); i++)
  {
    BosonMoveData* movedata = mMoveDatas[i];
    // Create passable map for this movedata
    movedata->cellPassable = new bool[mMap->width() * mMap->height()];

    // Fill the map
    for(unsigned int y = 0; y < mMap->height(); y++)
    {
      for(unsigned int x = 0; x < mMap->width(); x++)
      {
        // Check if this cell is passable for current movedata
        unsigned int pos = y * mMap->width() + x;
        movedata->cellPassable[pos] = true;
        if(movedata->type == BosonMoveData::Land)
        {
          // Land unit
          // Check for slope
          if(mSlopeMap[pos] > movedata->maxSlope)
          {
            movedata->cellPassable[pos] = false;
          }
          // Check for water
          if(mMap->waterDepthAtCorner(x, y) > movedata->waterDepth)
          {
            movedata->cellPassable[pos] = false;
          }
        }
        else
        {
          // Water unit (ship)
          // Check for deep enough water
          if(mMap->waterDepthAtCorner(x, y) < movedata->waterDepth)
          {
            movedata->cellPassable[pos] = false;
          }
        }
      }
    }
  }
}

void BosonPath::initBlocks()
{
  PROFILE_METHOD;

  // Size of a single block, in cells
  mBlockSize = 8;
  // Number of blocks in x- and y-direction
  mBlocksCountX = (int)ceilf(mMap->width() / (float)mBlockSize);
  mBlocksCountY = (int)ceilf(mMap->height() / (float)mBlockSize);

  // Total number of blocks
  int blockcount = mBlocksCountX * mBlocksCountY;

  // Create the array of blocks
  mBlocks = new BlockInfo[blockcount];

  // Find out the block centers for all movedatas
  for(int i = 0; i < blockcount; i++)
  {
    mBlocks[i].centerx = new int[mMoveDatas.count()];
    mBlocks[i].centery = new int[mMoveDatas.count()];
  }
  // We need to first loop by movedata here, because cell statuses are cached
  //  per movedata, so we need to reset them before starting to process another
  //  movedata
  // Find block centers
  for(unsigned int i = 0; i < mMoveDatas.count(); i++)
  {
    for(int j = 0; j < blockcount; j++)
    {
      findBlockCenter(j, mMoveDatas[i]);
    }
    resetDirtyCellStatuses();
  }

  // Find the connections between the blocks
  mBlockConnectionsCount = blockcount * mMoveDatas.count() * 4;
  mBlockConnections = new bofixed[mBlockConnectionsCount];
  mBlockConnectionsDirty = new bool[blockcount * 4];
  for(int i = 0; i < blockcount; i++)
  {
    for(unsigned int j = 0; j < mMoveDatas.count(); j++)
    {
      findBlockConnections(i, mMoveDatas[j]);
    }
    for(int j = 0; j < 4; j++)
    {
      mBlockConnectionsDirty[i*4 + j] = false;
    }
  }

  /*for(unsigned int i = 0; i < mMoveDatas.count(); i++)
  {
    createBlockColormap(mMoveDatas[i]);
  }*/
}

void BosonPath::findBlockCenter(int blockpos, BosonMoveData* movedata)
{
  // TODO: add  bool* passable;  array to BlockInfo. It would store for
  //  each movedata whether the block can ever be passable (i.e. it
  //  would be cellPassable values for all cells in this block AND'ed
  //  together). Would make updating the blocks faster.
  PROFILE_METHOD;
  int left = QMAX((blockpos % mBlocksCountX) * mBlockSize, movedata->edgedist1);
  int top  = QMAX((blockpos / mBlocksCountX) * mBlockSize, movedata->edgedist1);
  int right = QMIN(left + mBlockSize, (int)(mMap->width()) - 1 - movedata->edgedist2);
  int bottom = QMIN(top + mBlockSize, (int)(mMap->height()) - 1 - movedata->edgedist2);

  int centerx = (left + right) / 2;
  int centery = (top + bottom) / 2;

  bofixed bestcost = 10000;
  int bestx = -1;
  int besty = -1;

  // Temporary info object
  BosonPathInfo info;
  info.movedata = movedata;

  for(int y = top; y <= bottom; y++)
  {
    for(int x = left; x <= right; x++)
    {
      bofixed cost = 0;
      if(nodeStatus(&info, x, y) & STATUS_CANTGO)
      {
        // Take the next cell
        continue;
      }

      // Add small penalty for being off the center of the block
      cost += QMAX(QABS(centerx - x), QABS(centery - y)) * BLOCK_COST_DIST_MULTIPLIER;

      if(cost < bestcost)
      {
        bestcost = cost;
        bestx = x;
        besty = y;
      }
    }
  }

  // Set block's center pos for this movedata (-1 means not passable)
  mBlocks[blockpos].centerx[movedata->id] = bestx;
  mBlocks[blockpos].centery[movedata->id] = besty;
}

void BosonPath::findBlockConnections(int blockpos, BosonMoveData* movedata)
{
  for(int dir = 1; dir < 5; dir++)
  {
    calculateBlockConnection(blockpos, movedata, dir);
  }
}

void BosonPath::calculateBlockConnection(int blockpos, BosonMoveData* movedata, int dir)
{
  PROFILE_METHOD;
  // Calculate this block's position (in block coordinates)
  int blockx = blockpos % mBlocksCountX;
  int blocky = blockpos / mBlocksCountX;

  // Calculate neighbor block's position
  int otherblockx = blockx + mXOffset[dir];
  int otherblocky = blocky + mYOffset[dir];
  int otherblockpos = otherblocky * mBlocksCountX + otherblockx;

  int connectionpos = movedata->id * mBlocksCountX * mBlocksCountY * 4 + blockpos * 4 + (dir - 1);

  //boDebug(500) << k_funcinfo << "From (" << blockx << "; " << blocky <<
  //    ") to (" << otherblockx << "; " << otherblocky <<
  //    ");  movedata: " << movedata->id << "; dir: " << dir << endl;
  mBlockConnections[connectionpos] = -1;
  // Make sure the other block is valid
  if((otherblockx < 0) || (otherblocky < 0) || (otherblockx >= mBlocksCountX) || (otherblocky >= mBlocksCountY))
  {
    return;
  }

  // Make sure both blocks are passable for current movedata
  if((mBlocks[blockpos].centerx[movedata->id] == -1) || (mBlocks[otherblockpos].centerx[movedata->id] == -1))
  {
    //boDebug(500) << "    " << "No passability: " << mBlocks[blockpos].centerx[movedata->id] <<
    //    "; " << mBlocks[otherblockpos].centerx[movedata->id] << endl;
    return;
  }

  // Create BosonPathInfo object
  BosonPathInfo info;
  info.needpath = false;
  info.movedata = movedata;
  info.range = 0;
  // Calculate starting point
  info.start.setX(mBlocks[blockpos].centerx[movedata->id]);
  info.start.setY(mBlocks[blockpos].centery[movedata->id]);
  // Calculate destination point
  info.dest.setX(mBlocks[otherblockpos].centerx[movedata->id]);
  info.dest.setY(mBlocks[otherblockpos].centery[movedata->id]);

  // Set area boundaries
  int left = QMIN(blockx, otherblockx) * mBlockSize;
  int top = QMIN(blocky, otherblocky) * mBlockSize;
  int right = QMIN(left + 2 * mBlockSize - 1, (int)(mMap->width()) - 1);
  int bottom = QMIN(top + 2 * mBlockSize - 1, (int)(mMap->height()) - 1);
  lowLevelSetAreaBoundary(left, top, right, bottom);

  // Try to find a path from start (this block) to destination (the other block)
  getLowLevelPath(&info);
  // lowLevelSetAreaBoundary() marks some cells as blocking. We need to reset
  //  them because otherwise next time the pathfinder is run, it would still
  //  consider them to blocking
  resetDirtyCellStatuses();

  //boDebug(500) << "    " << "PF result: passable: " << info.passable << "; cost: " << info.pathcost << endl;
  if(info.result != GoalReached)
  {
    // No path was found
    return;
  }

  // Set connection's cost
  mBlockConnections[connectionpos] = info.pathcost;
}

void BosonPath::setBlockConnectionDirty(int pos)
{
  if(mBlockConnectionsDirty[pos])
  {
    // Don't add the same connection multiple times
    return;
  }

  mDirtyConnections.append(pos);
  mBlockConnectionsDirty[pos] = true;
}

void BosonPath::createBlockColormap(BosonMoveData* movedata)
{
  PROFILE_METHOD;
#define SETCOLOR(x, y,  r, g, b) \
    colors[(y * mMap->width() + x) * 3 + 0] = r; \
    colors[(y * mMap->width() + x) * 3 + 1] = g; \
    colors[(y * mMap->width() + x) * 3 + 2] = b;

  BoColorMap* colormap = new BoColorMap(mMap->width(), mMap->height());
  mMap->addColorMap(colormap, QString("Blocks %1").arg(movedata->id));
  unsigned char* colors = new unsigned char[mMap->width() * mMap->height() * 3];

  for(int blocky = 0; blocky < mBlocksCountY; blocky++)
  {
    for(int blockx = 0; blockx < mBlocksCountX; blockx++)
    {
      int blockpos = blocky * mBlocksCountX + blockx;
      // Block's edge coordinates (inclusive)
      int left = blockx * mBlockSize;
      int top = blocky * mBlockSize;
      int right = QMIN(left + mBlockSize - 1, (int)(mMap->width()) - 1);
      int bottom = QMIN(top + mBlockSize - 1, (int)(mMap->height()) - 1);

      bool passable = (mBlocks[blockpos].centerx[movedata->id] != -1);

      for(int y = top; y <= bottom; y++)
      {
        for(int x = left; x <= right; x++)
        {
          if(passable)
          {
            SETCOLOR(x, y,  128, 255, 128);
          }
          else
          {
            SETCOLOR(x, y,  255, 128, 128);
          }
        }
      }

      // Mark block's center
      if(passable)
      {
        SETCOLOR(mBlocks[blockpos].centerx[movedata->id], mBlocks[blockpos].centery[movedata->id],  255, 255, 255);
      }

      // Mark connections
      int connectionpos = movedata->id * mBlocksCountX * mBlocksCountY * 4 + blockpos * 4;
      // Up
      if(mBlockConnections[connectionpos + 0] != -1)
      {
        SETCOLOR((left + right) / 2, top,  128, 128, 255);
      }
      // Up-right
      if(mBlockConnections[connectionpos + 0] != -1)
      {
        SETCOLOR(right, top,  128, 128, 255);
      }
      // Right
      if(mBlockConnections[connectionpos + 0] != -1)
      {
        SETCOLOR(right, (top + bottom) / 2,  128, 128, 255);
      }
      // Down-right
      if(mBlockConnections[connectionpos + 0] != -1)
      {
        SETCOLOR(right, bottom,  128, 128, 255);
      }
    }
  }
  colormap->update(colors);
  delete[] colors;
#undef SETCOLOR
}

bofixed* BosonPath::calculateSlopemap()
{
  PROFILE_METHOD;
//  boDebug(500) << k_funcinfo << endl;
  BoColorMap* slopeColormap = new BoColorMap(mMap->width(), mMap->height());
  mMap->addColorMap(slopeColormap, "Slopes");
  unsigned char* slopecolors = new unsigned char[mMap->width() * mMap->height() * 3];

  bofixed* slopemap = new bofixed[mMap->height() * mMap->width()];
  bofixed minh, maxh, slope;
  for(unsigned int y = 0; y < mMap->height(); y++)
  {
    for(unsigned int x = 0; x < mMap->width(); x++)
    {
      // Find min and max heights for that cell
      minh = 1000;
      maxh = -1000;
      for(unsigned int i = x; i <= x + 1; i++)
      {
        for(unsigned int j = y; j <= y + 1; j++)
        {
          minh = QMIN(minh, bofixed(mMap->heightAtCorner(i, j)));
          maxh = QMAX(maxh, bofixed(mMap->heightAtCorner(i, j)));
        }
      }
      // Calculate slope between min and max height.
      // For simplicity, we check only one pair of corners and use 1 as distance
      //  between them
      if(minh == maxh)
      {
        slope = 0;
      }
      else
      {
        slope = Bo3dTools::rad2deg(atan(maxh - minh));
      }

      slopemap[y * mMap->width() + x] = slope;

      unsigned char s = (unsigned char)(slope / 90 * 255);
      slopecolors[(y * mMap->width() + x) * 3 + 0] = (slope > 30 ? (slope > 45 ? 255 : 160) : s);
      slopecolors[(y * mMap->width() + x) * 3 + 1] = s;
      slopecolors[(y * mMap->width() + x) * 3 + 2] = s;
    }
  }
  slopeColormap->update(slopecolors);
  delete[] slopecolors;
//  boDebug(500) << k_funcinfo << "END" << endl;
  return slopemap;
}

bofixed* BosonPath::calculateForestmap()
{
  PROFILE_METHOD;
  BoColorMap* forestColormap = new BoColorMap(mMap->width(), mMap->height());
  mMap->addColorMap(forestColormap, "Forestation");
  unsigned char* forestcolors = new unsigned char[mMap->width() * mMap->height() * 3];

  bofixed* itemmap = new bofixed[mMap->height() * mMap->width()];

  // Pass 1: find all neutral items
  for(unsigned int y = 0; y < mMap->height(); y++)
  {
    for(unsigned int x = 0; x < mMap->width(); x++)
    {
      const BoItemList* items = cell(x, y)->items();
      itemmap[y * mMap->width() + x] = 0;
      for(BoItemList::ConstIterator it = items->begin(); it != items->end(); ++it)
      {
        if((*it)->owner()->isNeutralPlayer())
        {
          if(RTTI::isUnit((*it)->rtti()))
          {
            Unit* u = (Unit*)*it;
            if(u->plugin(UnitPlugin::ResourceMine))
            {
              // Resource mines are excluded
              continue;
            }
          }
          itemmap[y * mMap->width() + x] += 1;
        }
      }
    }
  }

  // Pass 2: blur the cost map
  bofixed* blurredmap = new bofixed[mMap->height() * mMap->width()];
//  float factors[] = { 0.199501, 0.176059, 0.121004, 0.064769, 0.027000 };
  float factors[] = { 0.2, 0.16, 0.12, 0.08, 0.04 };
  for(int y = 0; y < (int)mMap->height(); y++)
  {
    for(int x = 0; x < (int)mMap->width(); x++)
    {
      for(int ydelta = -4; ydelta <= 4; ydelta++)
      {
        for(int xdelta = -4; xdelta <= 4; xdelta++)
        {
          int newx = x + xdelta;
          int newy = y + ydelta;
          if(newx < 0 || newy < 0 || newx >= (int)mMap->width() || newy >= (int)mMap->height())
          {
            continue;
          }
          bofixed f = factors[QABS(xdelta)] * factors[QABS(ydelta)];
          blurredmap[y * mMap->width() + x] += itemmap[newy * mMap->width() + newx] * f;
        }
      }

      bofixed blurvalue = blurredmap[y * mMap->width() + x];
      unsigned char value = (unsigned char)(((blurvalue > MAX_SHOWN_FORESTED_VALUE) ? bofixed(MAX_SHOWN_FORESTED_VALUE) : blurvalue) / MAX_SHOWN_FORESTED_VALUE * 255);
      forestcolors[(y * mMap->width() + x) * 3 + 0] = (blurvalue >= MIN_FORESTED_VALUE ? 255 : value);
      forestcolors[(y * mMap->width() + x) * 3 + 1] = value;
      forestcolors[(y * mMap->width() + x) * 3 + 2] = value;
    }
  }

  forestColormap->update(forestcolors);
  delete[] forestcolors;

  delete[] itemmap;

  return blurredmap;
}


void BosonPath::cellsOccupiedStatusChanged(int x1, int y1, int x2, int y2)
{
  for(int x = x1; x < x2; x++)
  {
    for(int y = y1; y < y2; y++)
    {
    }
  }
}

void BosonPath::unitMovingStatusChanges(Unit* u, int oldstatus, int newstatus)
{
  PROFILE_METHOD;
  if((oldstatus != UnitBase::Standing) && (newstatus != UnitBase::Standing))
  {
    // Unit was moving and will continue to be moving. No need to do anything
    return;
  }
  else
  {
    // Unit either starts or stops moving
    // It should be suffient if we go through all cells that unit is on, and
    //  recalc their occupied status
    int x1, x2, y1, y2;  // Rect in which cells changed
    x1 = y1 = 1000000;
    x2 = y2 = -1000000;
    const QPtrVector<Cell>* cells = u->cells();
    for(unsigned int i = 0; i < cells->count(); i++)
    {
      Cell* c = cells->at(i);
      if(u->isFlying())
      {
        continue;
      }
      else
      {
        cellChanged(c);
      }
    }
  }
}

void BosonPath::cellChanged(Cell* c)
{
  PROFILE_METHOD;
  markBlockChanged(c);
}

void BosonPath::markBlockChanged(Cell* c)
{
  int blockx = c->x() / mBlockSize;
  int blocky = c->y() / mBlockSize;
  int blockpos = blocky * mBlocksCountX + blockx;

  // Set this block to be dirty
  if(mBlocks[blockpos].flags & STATUS_CHANGED)
  {
    // The block has changed already
    return;
  }
  else
  {
    mBlocks[blockpos].flags |= STATUS_CHANGED;
    mChangedBlocks.append(blockpos);
  }

  // Set the block's connections to be dirty
  setBlockConnectionDirty(blockpos*4 + 0);
  setBlockConnectionDirty(blockpos*4 + 1);
  setBlockConnectionDirty(blockpos*4 + 2);
  setBlockConnectionDirty(blockpos*4 + 3);

  // We also need to update block's lower, lower-left, left and upper-left
  //  connections
  // Lower neighbor
  if(blocky + 1 < mBlocksCountY)
  {
    int blockpos2 = (blocky + 1) * mBlocksCountX + blockx;
    setBlockConnectionDirty(blockpos2 * 4 + 0);
  }
  // Lower-left neighbor
  if((blocky + 1 < mBlocksCountY) && (blockx > 0))
  {
    int blockpos2 = (blocky + 1) * mBlocksCountX + (blockx - 1);
    setBlockConnectionDirty(blockpos2 * 4 + 1);
  }
  // Left neighbor
  if(blockx > 0)
  {
    int blockpos2 = blocky * mBlocksCountX + (blockx - 1);
    setBlockConnectionDirty(blockpos2 * 4 + 2);
  }
  // Upper-left neighbor
  if((blocky > 0) && (blockx > 0))
  {
    int blockpos2 = (blocky - 1) * mBlocksCountX + (blockx - 1);
    setBlockConnectionDirty(blockpos2 * 4 + 3);
  }
}

void BosonPath::updateChangedBlocks()
{
  if(mChangedBlocks.isEmpty())
  {
    return;
  }

  int blocksToUpdate = mChangedBlocks.count();
  int connectionsToUpdate = mDirtyConnections.count();

  {
  PROFILE_METHOD;

  // Update block centers
  while(!mChangedBlocks.isEmpty())
  {
    int pos = mChangedBlocks.first();
    mChangedBlocks.pop_front();
    if(!(mBlocks[pos].flags & STATUS_CHANGED))
    {
      continue;
    }

    for(unsigned int i = 0; i < mMoveDatas.count(); i++)
    {
      findBlockCenter(pos, mMoveDatas[i]);
      resetDirtyCellStatuses();
    }
    mBlocks[pos].flags &= ~STATUS_CHANGED;
  }

  // Update connections
  while(!mDirtyConnections.isEmpty())
  {
    int connectionpos = mDirtyConnections.first();
    mDirtyConnections.pop_front();
    //int connectionpos = blockpos * 4 + (dir - 1);
    int blockpos = connectionpos / 4;
    int dir = connectionpos % 4;

    for(unsigned int i = 0; i < mMoveDatas.count(); i++)
    {
      calculateBlockConnection(blockpos, mMoveDatas[i], dir+1);
    }
    mBlockConnectionsDirty[blockpos*4 + dir] = false;
  }
  long int elapsed = methodProfiler.popElapsed();
  boDebug(500) << k_funcinfo << "Updated " << blocksToUpdate << " blocks and " <<
      connectionsToUpdate << " connections in " << elapsed/1000.0f << " ms" << endl;
  }

  /*for(unsigned int i = 0; i < mMoveDatas.count(); i++)
  {
    createBlockColormap(mMoveDatas[i]);
  }*/
  resetDirtyCellStatuses();
}

bool BosonPath::cellForested(int x, int y) const
{
  return (mForestMap[y * mMap->width() + x] >= MIN_FORESTED_VALUE);
}

Cell* BosonPath::cell(int x, int y) const
{
  return mMap->cell(x, y);
}

bool BosonPath::isValidCell(int x, int y) const
{
  return mMap->isValidCell(x, y);
}

bool BosonPath::saveAsXML(QDomElement& root) const
{
  // Nothing to save atm
  return true;
}

bool BosonPath::loadFromXML(const QDomElement& root)
{
  // Nothing to load atm
  return true;
}

QString BosonPath::debugText(bofixed x, bofixed y)
{
  QString info;

  int cellx = (int)x;
  int celly = (int)y;
  int pos = celly * mMap->width() + cellx;
  Cell* cellUnderCursor = cell(cellx, celly);
  if(!cellUnderCursor)
  {
    info += "Nothing under cursor";
    return info;
  }

  info += QString("Cell pos: (%1; %2)\n").arg(cellUnderCursor->x()).arg(cellUnderCursor->y());
//  info += QString("  occupied: %1\n").arg(cellUnderCursor->isLandOccupied() ? "true" : "false");
//  info += QString("  airoccupied: %1\n").arg(cellUnderCursor->isAirOccupied() ? "true" : "false");
  info += QString("  slope: %1\n").arg(mSlopeMap[pos]);
  //info += QString("  forest: %1\n").arg(mForestMap[pos]);

  info += QString("Block pos: (%1; %2)\n").arg(cellx / mBlockSize).arg(celly / mBlockSize);
  info += QString("Passability for movedatas:\n");
  for(unsigned int i = 0; i < mMoveDatas.count(); i++)
  {
    BosonMoveData* movedata = mMoveDatas[i];
    info += QString("  %1 (%2; size %3): %4\n").arg(i).arg(movedata->type == BosonMoveData::Land ? "land" : "water").arg(movedata->size).arg(movedata->cellPassable[pos]);
  }
  info += QString("\n");

  int blockpos = (celly / mBlockSize) * mBlocksCountX + (cellx / mBlockSize);
  const BlockInfo& block = mBlocks[blockpos];
  info += QString("B centers for movedatas:\n");
  for(unsigned int i = 0; i < mMoveDatas.count(); i++)
  {
    if(block.centerx[i] == -1)
    {
      info += QString("  %1: none\n").arg(i);
    }
    else
    {
      info += QString("  %1: (%2; %3)\n").arg(i).arg(block.centerx[i]).arg(block.centery[i]);
    }
  }


  info += QString("B connections for movedatas:\n");
  for(unsigned int i = 0; i < mMoveDatas.count(); i++)
  {
    int connectionpos = i * mBlocksCountX * mBlocksCountY * 4 + blockpos * 4;
    info += QString("  %1:  %2; %3; %4; %5\n").arg(i).
        arg(mBlockConnections[connectionpos + 0]).arg(mBlockConnections[connectionpos + 1]).
        arg(mBlockConnections[connectionpos + 2]).arg(mBlockConnections[connectionpos + 3]);
  }

  return info;
}

QValueList<BoVector2Fixed> BosonPath::findLocations(Player* player, int x, int y, int n, int radius, ResourceType type)
{
  QValueList<BoVector2Fixed> locations;

  QValueList<BosonPathNode> open;
  BosonPathNode node, n2;
  int diameterplusone = 2 * radius + 1;
  bool* visited = new bool[diameterplusone * diameterplusone];
  // Init VISITED set to false
  for(int i = 0; i < diameterplusone * diameterplusone; i++)
  {
    visited[i] = false;
  }
#define VISITED(nx, ny)  visited[((ny) - y + radius) * diameterplusone + ((nx) - x + radius)]

  node.x = x;
  node.y = y;
  open.append(node);
  VISITED(node.x, node.y) = true;

  int found = 0;


  while(!open.isEmpty())
  {
    // Get first node of OPEN
    node = open.first();
    open.pop_front();

    // Check it's children and add them to OPEN list
    for(int dir = 1; dir < 9; dir++)
    {
      // First, set new node's position to be old's one
      n2.x = node.x + mXOffset[dir];
      n2.y = node.y + mYOffset[dir];

      // Check if new node is within given radius
      int dist = QMAX(QABS(x - n2.x), QABS(y - n2.y));
      if(dist > radius)
      {
        continue;
      }

      // Make sure that position is valid
      if((n2.x < 0) || (n2.y < 0) || (n2.x >= (int)mMap->width()) || (n2.y >= (int)mMap->height()))
      {
        continue;
      }

      // Check if cell is already in OPEN
      if(VISITED(n2.x, n2.y))
      {
        continue;
      }

      // Add node to OPEN
      open.append(n2);
      VISITED(n2.x, n2.y) = true;

      // Check if cell is explored or not
      if(!player->isExplored(n2.x, n2.y))
      {
        continue;
      }

      // If it's explored, maybe it's what we're looking for
      if(type == Minerals)
      {
        const BoItemList* items = cell(n2.x, n2.y)->items();
        for(BoItemList::ConstIterator it = items->begin(); it != items->end(); ++it)
        {
          if(!RTTI::isUnit((*it)->rtti()))
          {
            continue;
          }
          Unit* u = (Unit*)*it;
          if(u->isDestroyed())
          {
            continue;
          }
          if(!(u->visibleStatus(player->bosonId()) & (UnitBase::VS_Visible | UnitBase::VS_Earlier)))
          {
            continue;
          }
          ResourceMinePlugin* res = (ResourceMinePlugin*)u->plugin(UnitPlugin::ResourceMine);
          if(res && res->canProvideMinerals() && (res->minerals() != 0))
          {
            locations.append(BoVector2Fixed(n2.x, n2.y));
            found++;
          }
        }
      }
      else if(type == Oil)
      {
        const BoItemList* items = cell(n2.x, n2.y)->items();
        for(BoItemList::ConstIterator it = items->begin(); it != items->end(); ++it)
        {
          if(!RTTI::isUnit((*it)->rtti()))
          {
            continue;
          }
          Unit* u = (Unit*)*it;
          if(u->isDestroyed())
          {
            continue;
          }
          if(!(u->visibleStatus(player->bosonId()) & (UnitBase::VS_Visible | UnitBase::VS_Earlier)))
          {
            continue;
          }
          ResourceMinePlugin* res = (ResourceMinePlugin*)u->plugin(UnitPlugin::ResourceMine);
          if(res && res->canProvideOil() && (res->oil() != 0))
          {
            locations.append(BoVector2Fixed(n2.x, n2.y));
            found++;
          }
        }
      }
      else if(type == EnemyBuilding)
      {
        // TODO!
      }
      else if(type == EnemyUnit)
      {
        // TODO!
      }

      if(n > 0 && found >= n)
      {
        delete[] visited;
        return locations;
      }

    }
  }

  boDebug(500) << k_funcinfo << "Found only " << found << " of " << n << " locations" << endl;
  delete[] visited;
  return locations;

#undef VISITED
}






/*****  BosonPathInfo  *****/

bool BosonPathInfo::saveAsXML(QDomElement& root)
{
  // Most of these things aren't necessary as they are also stored elsewhere
  // If you need to save/load anything, uncomment as necessary and don't forget
  //  to change loadFromXML() as well

  // Save start/dest points and range
  //saveVector2AsXML(start, root, "start");
  //saveVector2AsXML(dest, root, "dest");
  //root.setAttribute("target", target ? (int)target->id() : (int)-1);
  //root.setAttribute("range", range);
  // Save last pf query result
  root.setAttribute("result", result);
  // Save llpath
  //root.setAttribute("llpathlength", llpath.count());
  //for(unsigned int i = 0; i < llpath.count(); i++
  //{
  //  saveVector2AsXML(llpath[i], root, QString("llpath-%1").arg(i));
  //}
  // hlpath doesn't have to be saved atm
  // Save misc stuff
  root.setAttribute("moveAttacking", moveAttacking ? 1 : 0);
  root.setAttribute("slowDownAtDest", slowDownAtDest ? 1 : 0);
  root.setAttribute("waiting", waiting);
  root.setAttribute("pathrecalced", pathrecalced);

  return true;
}

bool BosonPathInfo::loadFromXML(const QDomElement& root)
{
  bool ok;
  result = (BosonPath::Result)root.attribute("result").toInt(&ok);
  if(!ok)
  {
    boError(500) << k_funcinfo << "Invalid value for result attribute" << endl;
    return false;
  }
  moveAttacking = root.attribute("moveAttacking").toInt(&ok);
  if(!ok)
  {
    boError(500) << k_funcinfo << "Invalid value for moveAttacking attribute" << endl;
    return false;
  }
  slowDownAtDest = root.attribute("slowDownAtDest").toInt(&ok);
  if(!ok)
  {
    boError(500) << k_funcinfo << "Invalid value for slowDownAtDest attribute" << endl;
    return false;
  }
  waiting = root.attribute("waiting").toInt(&ok);
  if(!ok)
  {
    boError(500) << k_funcinfo << "Invalid value for waiting attribute" << endl;
    return false;
  }
  pathrecalced = root.attribute("pathrecalced").toInt(&ok);
  if(!ok)
  {
    boError(500) << k_funcinfo << "Invalid value for pathrecalced attribute" << endl;
    return false;
  }

  return true;
}



/*****  BosonPathVisualization  *****/

BosonPathVisualization::BosonPathVisualization(QObject* parent) : QObject(parent)
{

}

BosonPathVisualization::~BosonPathVisualization()
{

}

BosonPathVisualization* BosonPathVisualization::pathVisualization()
{
  if (mPathVisualization)
  {
    return mPathVisualization;
  }
  sd.setObject(mPathVisualization, new BosonPathVisualization(0));
  return mPathVisualization;
}

void BosonPathVisualization::addLineVisualization(const QValueList<BoVector3Fixed>& points, const BoVector4Float& color, bofixed pointSize, int timeout, bofixed zOffset)
{
  emit signalAddLineVisualization(points, color, pointSize, timeout, zOffset);
}

void BosonPathVisualization::addLineVisualization(const QValueList<BoVector3Fixed>& points, bofixed pointSize, int timeout, bofixed zOffset)
{
  addLineVisualization(points, BoVector4Float(1.0f, 1.0f, 1.0f, 1.0f), pointSize, timeout, zOffset);
}
/*
 * vim: et sw=2
 */
