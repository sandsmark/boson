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
#include "unitplugins.h"
#include "bowater.h"

#include <qptrqueue.h>

#include <kstaticdeleter.h>


//#include <sys/time.h> // only for debug

#include <math.h>


// this should be highest of all costs. If cell's cost is at least ERROR_COST,
//  there's no way unit can go to that cell
#define ERROR_COST 100000
// If path's cost is more than MAX_PATH_COST, searching is aborted
#define MAX_PATH_COST 5000
// This will be added to cell's cost if cell is fogged
#define FOGGED_COST 3
// This will be added to cell's cost if cell is occupied by a moving unit
#define OCCUPIED_BY_MOVING_COST 7
// This will be added to cell's cost if cell is occupied by a non-moving unit
#define OCCUPIED_BY_NONMOVING_COST ERROR_COST
// If cell's cost is more than this, path won't be handled by fast pathfinder
#define MAX_FAST_PF_COST 10
// This will be added to all costs
#define BASE_COST 5
// This will be multiplied by manhattan distance between goal and cell in dist()
#define DIST_MODIFIER 7
#define CROSSDIVIDER 100
#define ABORTPATH (SEARCH_STEPS * 2 + 1) * (SEARCH_STEPS * 2 + 1)


#define marking(x, y) mMark[x - mStartx + SEARCH_STEPS][y - mStarty + SEARCH_STEPS]


// If you uncomment next line, you will enable in-line moving style.
//  With this moving style, unit movement is less agressive and units
//  wait until they can move, they don't search path around other units.
//  This is useful when moving big group of units through narrow places
//  on map, but in general, default style is better IMO
#define MOVE_IN_LINE

#define NEWER_PF_STYLE

// If this is defined, BoLineVisualization will be used to show found paths
//#define VISUALIZE_PATHS


BosonPathVisualization* BosonPathVisualization::mPathVisualization = 0;

static KStaticDeleter<BosonPathVisualization> sd;

static long int totalelapsed = 0;
static int totalcalls = 0;


class PathNode
{
  public:
    PathNode() { x = 0; y = 0; g = 0; h = 0; level = -1; };
    inline void operator=(const PathNode& a);
    inline bool operator<(const PathNode& a);
    int x; // x-coordinate of cell
    int y; // y-coordinate of cell

    float g; // real cost - cost of all nodes up to this one (If this node is at goal, it's cost of full path)
    float h; // heuristic cost - distance between this node and goal
    short int level; // node is level steps away from start. It's needed to search only 10 steps of path at once
};

inline void PathNode::operator=(const PathNode& a)
{
  x = a.x;
  y = a.y;
  g = a.g;
  h = a.h;
  level = a.level;
}

inline bool PathNode::operator<(const PathNode& a)
{
  return (g + h) < (a.g + a.h);
}

/** Describes found path
  * Possible values are:
  * NoPath - no path was found
  * FullPath - full path to goal was found
  * PartialPath - SEARCH_STEPS steps of path was found and no further was searched
  * AbortedPath - pathfinding was aborted, but some steps were found
  * AlternatePath - goal was occupied, but path to nearby tile was found
  */
enum PathStyle {
  NoPath = 0,
  FullPath = 1,
  PartialPath = 2,
  AbortedPath = 3,
  AlternatePath = 4
};


BosonPath::BosonPath(Unit* unit, int startx, int starty, int goalx, int goaly, int range)
{
  mUnit = unit;
  mStartx = startx;
  mStarty = starty;
  mGoalx = goalx;
  mGoaly = goaly;
  mRange = range;
  /// TODO: those variables needs tuning and *lots* of testing!

  //boDebug(500) << k_funcinfo << "start: " << mStartx << "," << mStarty << " goal: " << mGoalx << "," << mGoaly << " range: " << mRange << endl;

  mNodesRemoved  = 0;
  mPathLength = 0;
  mPathCost = 0.0f;
  mRange = 0;
}

BosonPath::~BosonPath()
{
}

static int pathSlow = 0, pathRange = 0, pathFast = 0;

QValueList<BoVector2> BosonPath::findPath(BosonPathInfo* pathInfo)
{
  QValueList<BoVector2> points;
  if (!pathInfo || !pathInfo->unit)
  {
    boError(500) << k_funcinfo << "NULL unit" << endl;
    return points;
  }
  Unit* unit = pathInfo->unit;
  int goalx = (int)pathInfo->dest.x();
  int goaly = (int)pathInfo->dest.y();
  int range = pathInfo->range;
  BosonPath path(unit, (int)unit->centerX(), (int)unit->centerY(),
        goalx, goaly, range);
  if (!path.findPath())
  {
    boWarning(500) << "no path found" << endl;
  }
  boDebug() << k_funcinfo << "Total time elapsed: " << totalelapsed / 1000000.0 << " sec; calls: " << totalcalls <<
      ";  handled in (R/F/S): " << pathRange << "/" << pathFast << "/" << pathSlow << endl;
  points = path.path; // faster than manually coping all points

  pathInfo->llpath.clear();
  pathInfo->llpath.reserve(points.count());
  QValueList<BoVector2>::Iterator it;
  int i = 0;
  for (it = points.begin(); it != points.end(); ++it, i++) {
    pathInfo->llpath.append(*it);
  }
  return points;
}

QValueList<BoVector2> BosonPath::findLocations(Player* player, int x, int y, int n, int radius, ResourceType type)
{
  QValueList<BoVector2> locations;

  QValueList<PathNode> open;
  PathNode node, n2;
  bool* visited = new bool[(2 * radius + 1) * (2 * radius + 1)];
  // Init VISITED set to false
  for(int i = 0; i < (2 * radius + 1) * (2 * radius + 1); i++)
  {
    visited[i] = false;
  }
#define VISITED(nx, ny)  visited[(ny - y + radius) * (2 * radius + 1) + (nx - x + radius)]

  const BosonCanvas* canvas = boGame->canvas(); // FIXME: is this good?

  node.x = x;
  node.y = y;
  node.level = 0;
  open.append(node);
  VISITED(node.x, node.y) = true;

  int found = 0;


  while(!open.isEmpty())
  {
    // Get first node of OPEN
    getFirst(open, node);

    // Check it's children and add them to OPEN list
    for(int dir = 0; dir < 8; dir++)
    {
      // Find next child node:
      Direction d = (Direction)dir;
      // First, set new node's position to be old's one
      n2.x = node.x;
      n2.y = node.y;
      // then call method to modify position accordingly to direction
      neighbor(n2.x, n2.y, d);

      // Check if new node is within given radius
      n2.level = QMAX(QABS(x - n2.x), QABS(y - n2.y));
      if(n2.level > radius)
      {
        continue;
      }

      // Make sure that position is valid
      if(!canvas->onCanvas(n2.x, n2.y))
      {
        //boWarning() << k_lineinfo << "not on canvas" << endl;
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

      // Check if cell is fogged or not
      if(player->isFogged(n2.x, n2.y))
      {
        continue;
      }

      // If it's not fogged, maybe it's what we're looking for
      if(type == Minerals)
      {
        const BoItemList* items = canvas->cell(n2.x, n2.y)->items();
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
          ResourceMinePlugin* res = (ResourceMinePlugin*)u->plugin(UnitPlugin::ResourceMine);
          if(res && res->canProvideMinerals() && (res->minerals() != 0))
          {
            locations.append(BoVector2(n2.x, n2.y));
            found++;
          }
        }
      }
      else if(type == Oil)
      {
        const BoItemList* items = canvas->cell(n2.x, n2.y)->items();
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
          ResourceMinePlugin* res = (ResourceMinePlugin*)u->plugin(UnitPlugin::ResourceMine);
          if(res && res->canProvideOil() && (res->oil() != 0))
          {
            locations.append(BoVector2(n2.x, n2.y));
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

  boDebug() << k_funcinfo << "Found only " << found << " of " << n << " locations" << endl;
  delete[] visited;
  return locations;

#undef VISITED
}


bool BosonPath::findPath()
{
  // We now have 3 different pathfinding methods: fast and slow pathfinders and
  //  range checker.
  // Fast method searches with dumb but very fast algorithm and fails if there
  //  is any units on the way
  // Range checker checks if it's possible to get closer to target. This will
  //  speed up pathfinding for multiple units
  // Slow method is the one used before. It should always find some path and it
  //  can find path around other units. It's at least about 10 times slower
  //  though (with a simple path)
  BosonProfiler profiler(BosonProfiling::FindPath);
  totalcalls++;
  if(findFastPath())
  {
    long int elapsed = profiler.stop();
    totalelapsed += elapsed;
    boDebug(500) << k_funcinfo << "TOTAL TIME ELAPSED (fast method): " << elapsed << " microsec." << endl;
    pathFast++;
    return true;
  }
  else
  {
    if(rangeCheck())
    {
      long int elapsed = profiler.stop();
      totalelapsed += elapsed;
      boDebug(500) << k_funcinfo << "TOTAL TIME ELAPSED (range method): " << elapsed << " microsec." << endl;
      pathRange++;
      return false;
    }
    bool ret = findSlowPath();
    long int elapsed = profiler.stop();
    totalelapsed += elapsed;
    boDebug(500) << k_funcinfo << "TOTAL TIME ELAPSED (slow method): " << elapsed << " microsec. nodes removed: " << mNodesRemoved << endl;
    pathSlow++;
    return ret;
  }
}

bool BosonPath::findFastPath()
{
  //struct timeval time1, time2;
  //gettimeofday(&time1, 0);

  int x[SEARCH_STEPS];
  int y[SEARCH_STEPS];
  int lastx, lasty;
  int tox, toy;
  bool movebyx, movebyy;
  int steps;
  // Find path inside unit's sight range only, but at least 4 steps and not more
  //  than SEARCH_STEPS in any case
  int length = QMIN(SEARCH_STEPS, QMAX(4, mUnit->sightRange()));

  lastx = mStartx;
  lasty = mStarty;

  for(steps = 0; steps < length; steps++)
  {
    // Calculate, how many steps to go in each direction
    tox = QABS(lastx - mGoalx);
    toy = QABS(lasty - mGoaly);

    // Check if we're at goal already
    if(tox == 0 && toy == 0)
    {
      break;
    }

    // Check by which direction to move
    if(tox > toy)
    {
      // More steps to go by x direction: move by x direction
      movebyx = true;
      movebyy = false;
    }
    else if(tox < toy)
    {
      // More steps to go by y direction: move by y direction
      movebyx = false;
      movebyy = true;
    }
    else
    {
      // Same amount of steps in both directions: move diagonally
      movebyx = true;
      movebyy = true;
    }

    // Move
    if(movebyx)
    {
      if(lastx < mGoalx)
      {
        lastx++;  // Go right
      }
      else
      {
        lastx--;  // Go left
      }
    }
    if(movebyy)
    {
      if(lasty < mGoaly)
      {
        lasty++;  // Go down
      }
      else
      {
        lasty--;  // Go up
      }
    }

    // Set waypoint
    x[steps] = lastx;
    y[steps] = lasty;

    // Check if we can move to waypoint
    // If cost is bigger than MAX_FAST_PF_COST, then we return false and it will
    //  be handled in slow pathfinder instead.
    if(! mUnit->canvas()->onCanvas(lastx, lasty))
    {
      return false;
    }
    if(cost(lastx, lasty) > MAX_FAST_PF_COST)
    {
      // Path can't be found using fast method
      //gettimeofday(&time2, 0);
      //boDebug(500) << k_funcinfo << "Can't find path using fast method. Time elapsed: "
      //    << time2.tv_usec - time1.tv_usec << " microsec." << endl;
      return false;
    }
  }

  // Compose path
  BoVector2 wp;
  int i;
  for(i = 0; i < steps; i++)
  {
    wp.setX(x[i] + 1.0f / 2);
    wp.setY(y[i] + 1.0f / 2);
    path.append(wp);
  }

  i = steps - 1;
  if((x[i] == mGoalx) && (y[i] == mGoaly))
  {
    // Full path was found. If partial path was found, pathfinder will be
    //  automatically called again when there's no waypoint left.
    path.append(BoVector2(PF_END_CODE, PF_END_CODE));  // This means that end of path has been reached
  }

  //gettimeofday(&time2, 0);
  //boDebug(500) << k_funcinfo << "Path found (using fast method)! Time elapsed: " <<
  //    time2.tv_usec - time1.tv_usec << "microsec." << endl;

  return true;
}

bool BosonPath::findSlowPath()
{
  //struct timeval time1, time2;
  //gettimeofday(&time1, 0);

  mNodesRemoved = 0;
  mPathLength = 0;
  mPathCost = 0;
  PathStyle pathfound = NoPath;
  QValueList<PathNode> open;

  // Create first (main) node
  PathNode node;

  // It will be at start
  node.x = mStartx;
  node.y = mStarty;
  node.level = 0;

  // Nearest node to goal. This is used when goal is occupied and we want to
  //  move as near as possible to it
  PathNode nearest;

  // Real cost will be 0 (we haven't moved yet)
  node.g = 0;
  // Calculate heuristic (distance) cost
  node.h = dist(mStartx, mStarty, mGoalx, mGoaly);

  // add node to OPEN
  open.append(node);

  // mark values on 'virtual map'
  marking(node.x, node.y).f = node.g + node.h;
  marking(node.x, node.y).g = node.g;
  marking(node.x, node.y).level = 0; // same as node.level

  // Create second node
  PathNode n2;

  bool goalUnReachable = false;
  nearest = node;

  // Main loop
  while(!open.empty())
  {
    // First check if we're at goal already
    if(inRange(node.x, node.y))
    {
      mGoalx = node.x;
      mGoaly = node.y;
      pathfound = FullPath;
      break;
    }
    else
    { // this is usually the case - except if we cannot go on the intended goal
      getFirst(open, node);
      // if f < 0 then it's not in OPEN
      marking(node.x, node.y).f = -1;
      mNodesRemoved++;
    }

    // Break if SEARCH_STEPS steps of path is found
    if(node.level >= SEARCH_STEPS)
    {
      if(goalUnReachable)
      {
        // If goal is unreachable and we've searched long enough, then we
        //  should stop searching and go to closest tile to goal
        node = nearest;
        pathfound = AlternatePath;
      }
      else
      {
        pathfound = PartialPath;
      }
      mGoalx = node.x;
      mGoaly = node.y;
      break;
    }

    // Check if we've gone too long with searching
    if(mNodesRemoved >= ABORTPATH)
    {
      boDebug(500) << k_funcinfo << "mNodesRemoved >= ABORTPATH" << endl;
      // Pick best node from OPEN
      QValueList<PathNode>::iterator i;
      for(i = open.begin(); i != open.end(); ++i)
      {
        if(((*i).g + (*i).h) < (node.g + node.h))
        {
          node = *i;
        }
      }
      // Set goal to where we ended
      mGoalx = node.x;
      mGoaly = node.y;
      // and abort
      pathfound = AbortedPath;
      break;
    }

    for(int dir = 0; dir < 8; dir++)
    {
      Direction d = (Direction)dir;
      // First, set new node's position to be old's one
      n2.x = node.x;
      n2.y = node.y;
      // then call method to modify position accordingly to direction
      neighbor(n2.x, n2.y, d);
      // new node's level = old node's level + 1
      n2.level = node.level + 1;

      // Make sure that position is valid
      if(! mUnit->canvas()->onCanvas(n2.x, n2.y))
      {
        //boWarning() << k_lineinfo << "not on canvas" << endl;
        continue;
      }

      // Calculate costs of node
      float nodecost = cost(n2.x, n2.y);
      // If cost is ERROR_COST, then we can't go there
      if(nodecost >= ERROR_COST)
      {
        /// TODO: this may lead to problems when all cells in range have ERROR_COST
        if(n2.x == mGoalx && n2.y == mGoaly && mRange == 0)
        {
          goalUnReachable = true;
        }
        //boDebug() << k_lineinfo << "ERROR_COST" << endl;
        continue;
      }
      else // we can go on this cell
      {
        n2.g = node.g + nodecost;
      }

      n2.h = dist(n2.x, n2.y, mGoalx, mGoaly);

      if((n2.h + n2.g * 0.2) < (nearest.h + nearest.g * 0.2))
      {
        nearest = n2;
      }

      // if g == -1 then it isn't visited yet
      if(marking(n2.x, n2.y).g == -1)
      {
        // First, mark the spot
        // direction of Marking always points to _previous_ element in path
        marking(n2.x, n2.y).dir = reverseDir(d);
        // Store costs
        marking(n2.x, n2.y).f = n2.g + n2.h;
        marking(n2.x, n2.y).g = n2.g;
        marking(n2.x, n2.y).level = n2.level;
        // Add node to OPEN
        addNode(open, n2);
      }
      else
      {
        // PathNode is in OPEN or CLOSED
        if(marking(n2.x, n2.y).f != -1)
        {
          // It's in OPEN
          if(n2.g < marking(n2.x, n2.y).g)
          {
            // Our current node has lower cost than the one, that was here, so
            //  we modify the path
            // First, find this node in OPEN
            QValueList<PathNode>::iterator find;
            for(find = open.begin(); find != open.end(); ++find)
            {
              if(((*find).x == n2.x) && ((*find).y == n2.y))
              {
                break;
              }
            }
            if (find == open.end())
            {
              boError(500) << "find == open.end()" << endl;
              break; // or what?
            }
            // Mark new direction from this node to previous one
            marking(n2.x, n2.y).dir = reverseDir(d);
            // Then modify costs and level of spot
            marking(n2.x, n2.y).g = n2.g;
            marking(n2.x, n2.y).f = n2.g + n2.h;
            marking(n2.x, n2.y).level = n2.level;
            // Replace cost and level of node that was in OPEN
            open.erase(find);
            addNode(open, n2);
          }
        }
      }
    }
  }



  // Pathfinding finished, but was the path found
  // We now check value of pathfound to see if path was found
  if(pathfound != NoPath)
  {
    // Something was
    // Path cost is equal to cost of last node
    mPathCost = node.g;
    // Temporary array - needed because path is first stored from goal to start
    QValueList<BoVector2> temp;

    // Construct waypoint and set it's pos to goal
    BoVector2 wp;
    int x, y;
    x = mGoalx;
    y = mGoaly;
    wp.setX(x + 1.0f / 2);
    wp.setY(y + 1.0f / 2);

    Direction d = DirNone;

    // Add waypoints to temporary path in reversed direction (goal to start)
    // We don't add start
    int counter = 0;  // failsave
    // the directions pointing to the cells are in marking(x1, y1) -> x1,y1 starts
    // at x,y (aka mGoalx,mGoaly) nad go to mStartx,mStarty
    while(((x != mStartx) || (y != mStarty)) && counter < 100)
    {
      counter++;
      // Add waypoint
      temp.append(wp);
      mPathLength++;
      d = marking(x, y).dir; // the direction to the next cell
      neighbor(x, y, d);
      wp.setX(x + mUnit->width() / 2);
      wp.setY(y + mUnit->height() / 2);
    }
    if (counter >= 100)
    {
      boWarning(500) << k_lineinfo << "oops - counter >= 100" << endl;
    }

    // Write normal-ordered path to path
    for(int i = temp.size() - 1; i >= 0; --i)
    {
      wp.setX(temp[i].x());
      wp.setY(temp[i].y());
      path.append(wp);
    }

#ifdef VISUALIZE_PATHS
  {
    QValueList<BoVector3> points;
    QValueList<BoVector2>::iterator it;
    for(it = path.begin(); it != path.end(); ++it)
    {
      points.append(BoVector3((*it).x(), -(*it).y(), 0.0f));
    }
    BoVector4 color(0.5f, 0.5f, 0.5f, 1.0f);
    float pointSize = 2.0f;
    int timeout = 100;
    float zOffset = 0.5f;
    BosonPathVisualization::pathVisualization()->addLineVisualization(points, color, pointSize, timeout, zOffset);
  }
#endif


    if(pathfound == FullPath || pathfound == AlternatePath)
    {
      // Point with coordinates PF_END_CODE; PF_END_CODE means that end of the path has been
      //  reached and unit should stop
      path.append(BoVector2(PF_END_CODE, PF_END_CODE));
    }
    //gettimeofday(&time2, 0);
    //boDebug(500) << k_funcinfo << "Path found (using slow method)! Time elapsed: " <<
    //    time2.tv_usec - time1.tv_usec << "microsec." << endl;
  }
  else
  {
    boDebug(500) << k_funcinfo << "path not found" << endl;
    boDebug(500) << "node.x=" << node.x << ",goalx=" << mGoalx << endl;
    boDebug(500) << "node.y=" << node.y << ",goaly=" << mGoaly << endl;
    boDebug(500) << "node.g=" << node.g << ",MAX_PATH_COST=" << MAX_PATH_COST << endl;
    // Path wasn't found
    // If path wasn't found we add one point with coordinates PF_END_CODE; PF_END_CODE to path.
    //  In Unit::advanceMove(), there is check for this and if coordinates are
    //  those, then moving is stopped
    path.append(BoVector2(PF_END_CODE, PF_END_CODE));
    //gettimeofday(&time2, 0);
    //boDebug(500) << k_funcinfo << "Time elapsed: " << time2.tv_usec - time1.tv_usec << "microsec." << endl;
  }

  return (pathfound != NoPath);
}

bool BosonPath::rangeCheck()
{
  // This method checks if it's possible to go to better place than where unit
  //  currently is
  // First check if unit is in range
  if(inRange(mStartx, mStarty))
  {
    return true;
  }

  // If unit's not in range, it might still be possible that it can't get closer
  //  to goal than it already is. This is usually the case when you move many
  //  units at once. Then one unit will go to goal, but others will try to find path to
  //  goal and waste time.
  // First quick check if goal is occupied if range is 0
  int dist = QMAX(QABS(mStartx - mGoalx), QABS(mStarty - mGoaly));
  // If distance to goal is more than SEARCH_STEPS, we won't search complete
  //  path and we don't need this method
  if(dist > SEARCH_STEPS)
  {
    return false;
  }
  if(mRange == 0)
  {
    if(cost(mGoalx, mGoaly) < ERROR_COST)
    {
      return false;
    }
    else if(dist == 1)
    {
      // Goal is occupied and unit's next to it - can't get any closer
      return true;
    }
  }

  int w = mUnit->canvas()->mapWidth();
  int h = mUnit->canvas()->mapHeight();
  int x, y;
  for(int range = 1; range < dist; range++)
  {
    // We must not look too far, otherwise we have crash, because marking array
    //  isn't big enough
    if(dist + range > SEARCH_STEPS)
    {
      return false;
    }
    // Bad duplicated code. But it's faster this way
    // First check upper and lower sides of "rectangle"
    for(x = mGoalx - range; x <= mGoalx + range; x++)
    {
      if((x < 0) || (x >= w))
      {
        continue;
      }
      if(cost(x, mGoaly - range) < ERROR_COST)
      {
        mRange = QMAX(mRange, range);
        return false;
      }
      if(cost(x, mGoaly + range) < ERROR_COST)
      {
        mRange = QMAX(mRange, range);
        return false;
      }
    }
    // Then right and left sides. Note that corners are already checked
    for(y = mGoaly - range + 1; y < mGoaly + range; y++)
    {
      if((y < 0) || (y >= h))
      {
        continue;
      }
      if(cost(mGoalx - range, y) < ERROR_COST)
      {
        mRange = QMAX(mRange, range);
        return false;
      }
      if(cost(mGoalx + range, y) < ERROR_COST)
      {
        mRange = QMAX(mRange, range);
        return false;
      }
    }
  }
  return true;
}

float BosonPath::dist(int ax, int ay, int bx, int by)
{
  // Cost is bigger when point a is not near straight line between start and
  //  goal
  double dx1 = ax - bx;
  double dy1 = ay - by;
  double dx2 = mStartx - bx;
  double dy2 = mStarty - by;
  double cross = dx1 * dy2 - dx2 * dy1;
  if(cross < 0)
  {
    cross = -cross;
  }

  float dist = float(cross / CROSSDIVIDER);
  dist += DIST_MODIFIER * QMAX(QABS(ax - bx), QABS(ay - by));
  return dist;
}

void BosonPath::neighbor(int& x, int& y, Direction d)
{
  if((d == NorthEast) || (d == North) || (d == NorthWest))
  {
    y--;
  }
  else if((d == SouthEast) || (d == South) || (d == SouthWest))
  {
    y++;
  }
  if((d == NorthWest) || (d == West) || (d == SouthWest))
  {
    x--;
  }
  else if((d == NorthEast) || (d == East) || (d == SouthEast))
  {
    x++;
  }
}

float BosonPath::cost(int x, int y)
{
  // Use cached value if possible
  if(marking(x, y).c != -1)
  {
    return marking(x, y).c;
  }

  float co;
  // Check at the very beginning if tile is fogged - if it is, we return one value and save time
  if(mUnit->owner()->isFogged(x, y))
  {
    //boDebug() << "Tile at (" << x << ", " << y << ") is fogged, returning FOGGED_COST" << endl;
    co = FOGGED_COST + BASE_COST;
  }
  else
  {
    // Cell is visible, check if it's ok
    Cell* c = mUnit->canvas()->cell(x, y);
    if(!c)
    {
      boError(500) << k_funcinfo << "NULL cell" << endl;
      co = ERROR_COST;
    }
    else
    {
      // cell is ok
      // Check if we can go to that tile, if we can't, cost will be ERROR_COST
      if(!c->passable())
      {
        co = ERROR_COST;
      }
      else
      {
        if(boWaterManager->cellPassable(x, y))
        {
          if(!mUnit->unitProperties()->canGoOnLand())
          {
            return ERROR_COST;
          }
        }
        else
        {
          if(!mUnit->unitProperties()->canGoOnWater())
          {
            return ERROR_COST;
          }
        }
#ifndef NEWER_PF_STYLE
#ifndef MOVE_IN_LINE
        // If we are close to our starting point or to our goal, then consider cell
        //  to be occupied even if only moving units are on it (we assume they can't
        //  move away fast enough)
        bool includeMoving = false;
        if(QMAX(QABS(x - mStartx), QABS(y - mStarty)) <= 2) // Change 2 to 1???
        {
          includeMoving = true;
        }
        if(c->isOccupied(mUnit, includeMoving))
#else
        if(c->isOccupied(mUnit, false))
#endif  // MOVE_IN_LINE
        {
          co = ERROR_COST;
        }
        else
        {
          co = c->moveCost() + BASE_COST;
        }
#else  // NEWER_PF_STYLE
        co = c->moveCost() + BASE_COST;
        // Modify cost if cell is occupied
        bool hasany, hasmoving;
        c->isOccupied(mUnit, hasmoving, hasany);
        if(hasany)
        {
          if(!hasmoving)
          {
            // Cell is occupied by non-moving unit
            // TODO: non-moving here doesn't necessarily mean that unit won't
            //  move away from there. We also set moving status to non-moving
            //  for units which are attacking the enemy while moving, but after
            //  enemy is dead, they'll move on.
            co += OCCUPIED_BY_NONMOVING_COST;
          }
          else
          {
            // Cell is occupied by moving unit.
            co += OCCUPIED_BY_MOVING_COST;
          }
        }
#endif  // NEWER_PF_STYLE
      }
    }
  }
  marking(x, y).c = co;
  return co;
}

void BosonPath::getFirst(QValueList<PathNode>& v, PathNode& n)
{
  // List is already sorted. If we remove first item, it will remain sorted.
  n = v.first();
  v.erase(v.begin());
}

void BosonPath::addNode(QValueList<PathNode>& list, const PathNode& n)
{
  // Add n to the correct position in the list, so that list is kept sorted
  //  (given that it was sorted before)
  QValueList<PathNode>::iterator i;
  for(i = list.begin(); i != list.end(); ++i)
  {
    if((n.g + n.h) <= ((*i).g + (*i).h))
    {
      list.insert(i, n);
      break;
    }
  }
  if(i == list.end()) {
    list.append(n);
  }
}

Direction BosonPath::reverseDir(Direction d)
{
  return (Direction)(((int)d + 4) % 8);
}

void BosonPath::debug() const
{
  boDebug() << k_funcinfo << endl;
  if(!mUnit) {
    boError(500) << "NULL unit" << endl;
    return;
  }
  boDebug(500) << "unit: " << mUnit->id() << endl;
  boDebug(500) << "startx,starty = " << mStartx << "," << mStarty << endl;
  boDebug(500) << "goalx,goaly = " << mGoalx << "," << mGoaly << endl;
  boDebug(500) << "waypoints: " << path.size() << endl;
  int j = 0;
  for(QValueList<BoVector2>::const_iterator i = path.begin(); i != path.end(); ++i, j++) {
    boDebug(500) << "waypoint " << j << ":" << endl;
    boDebug(500) << "x,y=" << (*i).x() << "," << (*i).y() << endl;
  }
  boDebug() << k_funcinfo << "(end)" << endl;
}

bool BosonPath::inRange(int x, int y)
{
  /// TODO: maybe use different check (not manhattan dist.)
  if(QABS(x - mGoalx) > mRange || QABS(y - mGoaly) > mRange)
  {
    return false;
  }
  return true;
}



/***********************************************************
*****           N E W   P A T H F I N D E R
***********************************************************/

const int xoffsets[] = {  0,  1,  1,  1,  0, -1, -1, -1};
const int yoffsets[] = { -1, -1,  0,  1,  1,  1,  0, -1};


#define TNG_HIGH_DIST_MULTIPLIER 1.5f
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

#define REVERSEDIR(d) ((d + 4) % 8)



/*****  BosonPathSector  *****/
BosonPathSector::BosonPathSector()
{
  regions.setAutoDelete(true);
  regions.resize(2);
  x = 0;
  y = 0;
  w = 0;
  h = 0;
  pathfinder = 0;
}

void BosonPathSector::setPathfinder(BosonPath2* pf)
{
  pathfinder = pf;
}

void BosonPathSector::setGeometry(int _x, int _y, int _w, int _h)
{
  x = _x;
  y = _y;
  w = _w;
  h = _h;
}

bool BosonPathSector::hasCell(int cx, int cy)
{
  return (cx >= x) && (cx < x + w) && (cy >= y) && (cy < y + h);
}

void BosonPathSector::reinitRegions()
{
//  boDebug(510) << k_funcinfo << endl;
  // First we have to current regions
  // Set region pointer to NULL for all cells in this sector
  for(int ay = y; ay < (y + h); ay++)
  {
    for(int ax = x; ax < (x + w); ax++)
    {
      pathfinder->cell(ax, ay)->setRegion(0);
    }
  }
  // Delete all regions we have atm
//  boDebug(510) << k_funcinfo << "clearing " << regions.count() << " regions" << endl;
  regions.clear();
  // And find new ones
//  boDebug(510) << k_funcinfo << "initing new regions" << endl;
  initRegions();
//  boDebug(510) << k_funcinfo << "END" << endl;
}

void BosonPathSector::initRegions()
{
  for(int ay = y; ay < (y + h); ay++)
  {
    for(int ax = x; ax < (x + w); ax++)
    {
      if(!pathfinder->cellRegion(ax, ay))
      {
//        boDebug(510) << k_funcinfo << "Cell at (" << ax << "; " << ay << ") doesn't have region" << endl;
        // This cell has no region yet
        if(pathfinder->cellPassability(ax, ay) == BosonPath2::NotPassable)
        {
          // Not passable - continue
          continue;
        }
        else if(pathfinder->cellOccupied(ax, ay))
        {
          // Cell is occupied - continue
          continue;
        }
        // Create new region for this cell
        BosonPathRegion* r = new BosonPathRegion(this);
        r->findCells(ax, ay);
        if(regions.size() == regions.count())
        {
          regions.resize(regions.size() + 4);
        }
        regions.insert(regions.count(), r);
      }
      /*else
      {
        boDebug(510) << k_funcinfo << "Cell at (" << ax << "; " << ay << ") has region " << pathfinder->cellRegion(ax, ay) << " with id " << pathfinder->cellRegion(ax, ay)->id << endl;
      }*/
    }
  }
}

void BosonPathSector::updateRegions()
{
}


/*****  BosonPathRegion  *****/
BosonPathRegion::BosonPathRegion(BosonPathSector* s)
{
  sector = s;
  neighbors.reserve(8);
  passabilityType = BosonPath2::NotPassable;
  cellsCount = 0;
  cost = 0.0f;
  centerx = 0.0f;
  centery = 0.0f;
  group = 0;
  parent = 0;
  id = sector->pathfinder->addRegion(this);
}

BosonPathRegion::~BosonPathRegion()
{
//  boDebug(510) << k_funcinfo << "id: " << id << "; this: " << this << endl;
  // Remove ourselves from neighbor lists of other regions
//  boDebug(510) << k_funcinfo << "removing from " << neighbors.count() << " neighbors" << endl;
  for(unsigned int i = 0; i < neighbors.count(); i++)
  {
    neighbors[i].region->removeNeighbor(this);
  }
  // Remove from group
  if(group)
  {
    group->regions.remove(this);
  }
  // And remove ourselves from pathfinder's regions list
//  boDebug(510) << k_funcinfo << "removing self from regions list" << endl;
  sector->pathfinder->removeRegion(this);
//  boDebug(510) << k_funcinfo << "END" << endl;
}

void BosonPathRegion::findCells(int x, int y)
{
//  boDebug(510) << k_funcinfo << "Starting at (" << x << "; " << y << ")" << endl;
  QValueVector<BosonPathNode> open(sector->w * sector->h);
  bool* visited = new bool[sector->w * sector->h];
  for(int i = 0; i < sector->w * sector->h; i++)
  {
    visited[i] = false;
  }
#define VISITED(a, b)  visited[sector->w * (b - sector->y) + (a - sector->x)]

  passabilityType = sector->pathfinder->cellPassability(x, y);
  BosonPathNode n(x, y);
  BosonPathNode n2;
  int i;
  cellsCount = 0;
  cost = 0.0f;
  centerx = 0.0f;
  centery = 0.0f;

  sector->pathfinder->cell(x, y)->setRegion(this);
  open.push_back(n);
  VISITED(x, y) = true;
  cellsCount++;
  centerx += n.x;
  centery += n.y;
  cost += sector->pathfinder->cellCost(n.x, n.y);

  while(!open.isEmpty())
  {
    n = open.back();
    open.erase(open.end() - 1);

    for(i = 0; i < 8; i++)
    {
      n2.x = n.x + xoffsets[i];
      n2.y = n.y + yoffsets[i];

      // This also validates position, so we won't get invalid cells
      if(!sector->hasCell(n2.x, n2.y))
      {
        // Cell is in another sector (or not on the map)
        continue;
      }
      else if(VISITED(n2.x, n2.y))
      {
        // Already visited
        continue;
      }
      else if(sector->pathfinder->cellRegion(n2.x, n2.y))
      {
        // Cell is already in a region
        continue;
      }
      else if(sector->pathfinder->cellPassability(n2.x, n2.y) != passabilityType)
      {
        // Cell isn't passable
        continue;
      }
      else if(sector->pathfinder->cellOccupied(n2.x, n2.y))
      {
        // Cell is occupied
        continue;
      }

      // Cell is now in this region
      sector->pathfinder->cell(n2.x, n2.y)->setRegion(this);

      open.push_back(n2);
      VISITED(n2.x, n2.y) = true;
      cellsCount++;
      centerx += n2.x + 0.5f;  // "+ 0.5f" because we want center of the cell
      centery += n2.y + 0.5f;  // "+ 0.5f" because we want center of the cell
      cost += sector->pathfinder->cellCost(n2.x, n2.y);
    }
  }

  // Calculate average center point of the region
  centerx /= cellsCount;
  centery /= cellsCount;
  // Calculate cost
  cost /= sqrt((float)cellsCount);

//  boDebug(510) << k_funcinfo << "Region " << id << ": cells: " << cellsCount << "; center: (" <<
//      centerx << "; " << centery << "); cost: " << cost << endl;

  delete[] visited;
#undef VISITED
}

void BosonPathRegion::findBorderCells()
{
  boError(510) << k_funcinfo << "DON'T USE THIS!!! (OBSOLETED)" << endl;
  // FIXME: I believe this can be done _much_ faster
  for(unsigned int i = 0; i < neighbors.count(); i++)
  {
    // Find number of cells in this region that have neighbor cells in another
    //  region. You can move to that other region via those cells
    for(int y = sector->y; y < (sector->y + sector->h); y++)
    {
      for(int x = sector->x; x < (sector->x + sector->w); x++)
      {
        if(sector->pathfinder->cellRegion(x, y) != this)
        {
          // Not our cell
          continue;
        }

        // Check if any of this cell's neighbors belongs to other region
        for(int n = 0; n < 8; n++)
        {
          int x2, y2;
          x2 = x + xoffsets[n];
          y2 = y + yoffsets[n];

          if(!sector->pathfinder->isValidCell(x2, y2))
          {
            continue;
          }

          if(sector->pathfinder->cellRegion(x2, y2) == neighbors[i].region)
          {
            // This cell has neighbor in other region, so it's border cell
            neighbors[i].bordercells++;
            break;
          }
        }
      }
    }
  }
}

void BosonPathRegion::calculateCosts()
{
  for(unsigned int i = 0; i < neighbors.count(); i++)
  {
    calculateCosts(i);
  }
}

void BosonPathRegion::calculateCosts(BosonPathRegion* neighbor)
{
  for(unsigned int i = 0; i < neighbors.count(); i++)
  {
    if(neighbors[i].region == neighbor)
    {
      calculateCosts(i);
    }
  }
}

void BosonPathRegion::calculateCosts(unsigned int index)
{
  if(index >= neighbors.count())
  {
    boError(510) << k_funcinfo << "Invalid neighbor index: " << index << "; count: " << neighbors.count() << endl;
    return;
  }

  // THIS IS SLOOOOW!!! cellsOccupiedStatusChanged() for a single cell took
  //  about 15ms on average and about 13ms of it was spent recalculating region
  //  costs. So now we don't use this anymore.
  /* // Reset number of border cells first
  neighbors[index].bordercells = 0;
  // Find number of cells in this region that have neighbor cells in another
  //  region. You can move to that other region via those cells
  for(int y = sector->y; y < (sector->y + sector->h); y++)
  {
    for(int x = sector->x; x < (sector->x + sector->w); x++)
    {
      if(sector->pathfinder->cellRegion(x, y) != this)
      {
        // Not our cell
        continue;
      }

      // Check if any of this cell's neighbors belongs to any of our neighbor
      //  regions
      for(int n = 0; n < 8; n++)
      {
        int x2, y2;
        x2 = x + xoffsets[n];
        y2 = y + yoffsets[n];
        bool found = false;

        if(!sector->pathfinder->isValidCell(x2, y2))
        {
          continue;
        }

        if(sector->pathfinder->cellRegion(x2, y2) == neighbors[index].region)
        {
          // This cell has neighbor in other region, so it's border cell
          neighbors[index].bordercells++;
          found = true;
          break;
        }
        if(found)
        {
          // This cell already has neighbor. We don't allow multiple neighbors per
          //  cell, otherwise we'd mess up counts
          break;
        }
      }
    }
  }*/

  // Calculate 'neighbor cost' for neighbor.
  // It's cost of going from this region to neighbor region. The more border
  // cells this region has with us, the less the cost will be.
  //neighbors[index].cost = (float)TNG_MAX_BORDER_COST / neighbors[index].bordercells;
  neighbors[index].cost = (float)TNG_MAX_BORDER_COST / neighbors[index].bordercells;
//  boDebug(510) << k_funcinfo << "Passage cost from " << id << " to " << neighbors[index].region->id <<
//      " is " << neighbors[index].cost << " (" << neighbors[index].bordercells << " border cells)" << endl;
/*  for(unsigned int i = 0; i < neighbors.count(); i++)
  {
    // Find this region in neighbor's neighbor list
    for(unsigned int j = 0; j < neighbors[i].region->neighbors.count(); j++)
    {
      if(neighbors[i].region->neighbors[j].region == this)
      {
       // We take minimum of border tiles count for this region and for neighbor
        neighbors[i].bordercells = QMIN(neighbors[i].bordercells, neighbors[i].region->neighbors[j].bordercells);
        boDebug(510) << k_funcinfo << "Passage cost from " << id << " to " << neighbors[i].region->id <<
            " is " << neighbors[i].cost << "(" << neighbors[i].bordercells << " border cells)" << endl;
      }
    }
  }*/
}

void BosonPathRegion::addNeighbor(BosonPathRegion* r)
{
  // Check if we already have this neighbor in the list
  for(unsigned int i = 0; i < neighbors.count(); i++)
  {
    if(neighbors[i].region == r)
    {
      // We already have this neighbor. Increase border cell count
      neighbors[i].bordercells++;
      return;
    }
  }

  Neighbor n;
  n.region = r;
  float dx = QABS(r->centerx - centerx);
  float dy = QABS(r->centery - centery);
  n.cost = sqrt(dx * dx + dy * dy);
  n.bordercells = 1;
  neighbors.append(n);
}

void BosonPathRegion::removeNeighbor(BosonPathRegion* r)
{
//  boDebug(510) << k_funcinfo << "ids: r: " << r->id << "; this: " << id << ";;  pointers: r: " << r << "; this: " << this << endl;
#ifdef REMOVE_NEIGHBOR_USE_IT
  QValueVector<Neighbor>::iterator it;
  int i = 0;
  for(it = neighbors.begin(); it != neighbors.end(); ++it)
  {
    if((*it).region == r)
    {
//      boDebug(510) << k_funcinfo << "Found neighbor region at " << i << "(" << *it << ")" << endl;
      // Remove this neighbor
      // We want to keep this vector linear (i.e. no holes in the middle), so we
      //  have to move the last element to the position of the removed one
      if(it != neighbors.end() - 1)
      {
//        boDebug(510) << k_funcinfo << "neighbor isn't the last, moving another region" << endl;
        neighbors.insert(it, neighbors.last());
      }
      // Delete last item
//      boDebug(510) << k_funcinfo << "erasing last region" << endl;
      neighbors.erase(neighbors.end() - 1);
      return;
    }
    i++;
  }
#else
  for(unsigned int i = 0; i < neighbors.count(); i++)
  {
    if(neighbors[i].region == r)
    {
//      boDebug(510) << k_funcinfo << "Found neighbor region at " << i << endl;
      // Remove this neighbor
      // We want to keep this vector linear (i.e. no holes in the middle), so we
      //  have to move the last element to the position of the removed one
      if((neighbors.count() > 1) && (i < neighbors.count() - 1))
      {
//        boDebug(510) << k_funcinfo << "neighbor isn't the last, moving another region from " << neighbors.count() - 1 << " to " << i << endl;
        neighbors[i] = neighbors[neighbors.count() - 1];
      }
//      boDebug(510) << k_funcinfo << "erasing last region" << endl;
      neighbors.pop_back();
      return;
    }
  }
#endif
  boWarning(510) << k_funcinfo << "No such neighbor found: " << r << endl;
}


/*****  BosonPath2  *****/
BosonPath2::BosonPath2(BosonMap* map)
{
  boDebug(510) << k_funcinfo << endl;
  mMap = map;
  mSectors = 0;
  mSectorWidth = 0;
  mSectorHeight = 0;
  mRegionIdUsed = 0;
  boDebug(510) << k_funcinfo << "END" << endl;
}

BosonPath2::~BosonPath2()
{
}

void BosonPath2::init()
{
  boDebug(510) << k_funcinfo << endl;
  static int profilerId = -boProfiling->requestEventId("Stupid profiling name");
  BosonProfiler profiler(profilerId);
  initCellPassability();
  initSectors();
  initRegions();
  findRegionNeighbors(0, 0, mMap->width() - 1, mMap->height() - 1);
  initRegionCosts(mRegions);
  initRegionGroups(mRegions);
  long int elapsed = profiler.stop();
  boDebug(510) << k_funcinfo << "END, elapsed: " << elapsed / 1000.0 << " ms" << endl;
}

void BosonPath2::findPath(BosonPathInfo* info)
{
  boDebug(510) << k_funcinfo << endl;
  BosonProfiler profiler(BosonProfiling::FindPath);
  totalcalls++;

  if(info->flying)
  {
    // Flying units are special and have their own pathfinding function
    // First use range-check method
    if(rangeCheck(info))
    {
      findFlyingUnitPath(info);
    }
    else
    {
      // Not possible to get any closer to destination
      info->passable = false;
      info->llpath.clear();
      info->llpath.reserve(1);
      info->llpath.append(BoVector2(PF_END_CODE, PF_END_CODE));
    }
  }
  else
  {
    // Land unit
    // Update start and goal region(s)
    findHighLevelGoal(info);
    // We _must_ have start region. Otherwise something is very broken
    if(!info->startRegion)
    {
      long int elapsed = profiler.stop();
      totalelapsed += elapsed;
      boDebug(500) << k_funcinfo << "ELAPSED (failed 1): " << elapsed << " microsec." << endl;
      boDebug(510) << k_funcinfo << "No start region!" << endl;
      info->passable = false;
      return;
    }
    if((info->range > 0) && info->possibleDestRegions.isEmpty())
    {
      // If range is not 0, we need to get exactly to that range. Otherwise we
      //  won't even search for path
      info->passable = false;
      long int elapsed = profiler.stop();
      totalelapsed += elapsed;
      boDebug(500) << k_funcinfo << "ELAPSED (failed 2): " << elapsed << " microsec." << endl;
      return;
    }

    // Find high-level path
    findHighLevelPath(info);
    if(!info->hlpath)
    {
      // No high-level path was found
      long int elapsed = profiler.stop();
      totalelapsed += elapsed;
      boDebug(500) << k_funcinfo << "ELAPSED (failed 3): " << elapsed << " microsec." << endl;
      boError(510) << k_funcinfo << "No HL path found!" << endl;
      info->passable = false;
      return;
    }

    // Search low-level path
    findLowLevelPath(info);
  }

  long int elapsed = profiler.stop();
  totalelapsed += elapsed;
  boDebug(500) << k_funcinfo << "ELAPSED (success!!!): " << elapsed << " microsec." << endl;
//  boDebug() << k_funcinfo << "ENDm Total time elapsed: " << totalelapsed / 1000000.0 << " sec; calls: " << totalcalls << endl;
}

void BosonPath2::findHighLevelPath(BosonPathInfo* info)
{
//  boDebug(510) << k_funcinfo << endl;
  // First check if unit already has high-level path
  if(info->hlpath)
  {
    // Highlevel path is there - check if it's still valid
    if(info->hlpath->valid)
    {
      // It's valid, so we don't have to calculate anything
//      boDebug(510) << k_funcinfo << "old path is valid - return" << endl;
      return;
    }
    else
    {
      // Obsolete path - remove it
      releaseHighLevelPath(info->hlpath);
      info->hlpath = 0;
      info->hlstep = 0;
    }
  }

  // Maybe we have such a path in the cache?
  // TODO: search not only for full paths, but also for path segments, e.g. if
  //  we want to get from sector B to sector C, it would be fine if we have a
  //  path that start from sector A, goes through B and then C and ends in D
  // FIXME: we don't need to get exactly to dest, we just have to get to given
  //  range
  info->hlpath = findCachedHighLevelPath(info);
  if(info->hlpath)
  {
    QString path;
    for(unsigned int i = 0; i < info->hlpath->path.count(); i++)
    {
      BosonPathRegion* r = info->hlpath->path[i];
      path += QString("REG(id: %1; cost: %2; center: (%3; %4); cells: %5),   ").arg(r->id).arg(r->cost).arg(r->centerx).arg(r->centery).arg(r->cellsCount);
    }
    boDebug(510) << k_funcinfo << "Using cached HL path:  " << path << endl;
    // Cached highlevel path was found. Return.
    info->hlpath->users++;
    info->hlstep = 0;
//    boDebug(510) << k_funcinfo << "using cached path - return" << endl;
    return;
  }

  // No cached path was found either - search new one
  searchHighLevelPath(info);
  if(info->hlpath)
  {
    QString path;
    for(unsigned int i = 0; i < info->hlpath->path.count(); i++)
    {
      BosonPathRegion* r = info->hlpath->path[i];
      path += QString("REG(id: %1; cost: %2; center: (%3; %4); cells: %5),   ").arg(r->id).arg(r->cost).arg(r->centerx).arg(r->centery).arg(r->cellsCount);
    }
    boDebug(510) << k_funcinfo << "Found HL path:  " << path << endl;
  }
//  boDebug(510) << k_funcinfo << "END" << endl;
}

void BosonPath2::findLowLevelPath(BosonPathInfo* info)
{
  long int tm_initarea, tm_initmaps, tm_initmisc, tm_mainloop, tm_copypath = 0, tm_viz = 0, tm_uninit;
  boDebug(510) << k_funcinfo << "HL path has " << info->hlpath->path.count() << " steps, using step " << info->hlstep << " atm" << endl;
  static int profilerId = -boProfiling->requestEventId("Stupid profiling name");
  BosonProfiler pr(profilerId);
  BosonPathRegion* currentregion = info->hlpath->path[info->hlstep];
  BosonPathRegion* nextRegion = 0;
  if(info->hlstep + 1 == info->hlpath->path.count())
  {
    // This is the last region, destination is inside this region
  }
  else
  {
    nextRegion = info->hlpath->path[info->hlstep + 1];
  }

  // List of open nodes
  BosonPathHeap<BosonPathNode> open;

  // Search area's coordinates and size. Note that we need 1-cell border around
  //  current region, because we also add destination node (which is in next
  //  region) to open.
  int areax = QMAX(currentregion->sector->x - 1, 0);
  int areay = QMAX(currentregion->sector->y - 1, 0);
  int areaw = QMIN(currentregion->sector->w + 2, (int)mMap->width() - areax);
  int areah = QMIN(currentregion->sector->h + 2, (int)mMap->height() - areay);
  tm_initarea = pr.elapsed();
//  boDebug(510) << k_funcinfo << "Area is: (" << areax << "x" << areay << "+" << areaw << "x" << areah << ")" << endl;
  // Create list of nodes that are in visited and of those in open.
  // This is used for performance reasons, it's faster than seacrhing open for
  //  every node.
  unsigned int maxnodes = areaw * areah;
//  boDebug(510) << k_funcinfo << "Creating visited and inopen bool maps for " << maxnodes << " nodes" << endl;
  bool* visited = new bool[maxnodes];
//  boDebug(510) << k_funcinfo << "Created visited @ " << visited << endl;
  bool* inopen = new bool[maxnodes];
#define VISITED(x, y) visited[(y - areay) * areaw + (x - areax)]
#define INOPEN(x, y) inopen[(y - areay) * areaw + (x - areax)]
  for(unsigned int i = 0; i < maxnodes; i++)
  {
    visited[i] = false;
    inopen[i] = false;
  }
  // Direction to the parent node, used to traceback path
  char* parentdirections = new char[maxnodes];
#define PARENTDIR(x, y) parentdirections[(y - areay) * areaw + (x - areax)]
  // We don't init parentdirections, because for each visited cell, we set
  //  direction anyway, and we don't use directions for unvisited cells (unless
  //  there's a bug somewhere)
  tm_initmaps = pr.elapsed();


  // Find cell that the unit is on atm.
  BosonPathNode first, n, n2;
  if(info->unit)
  {
    first.x = (int)(info->unit->x() + info->unit->width() / 2);
    first.y = (int)(info->unit->y() + info->unit->height() / 2);
  }
  else
  {
    first.x = (int)info->start.x();
    first.y = (int)info->start.y();
  }
  first.g = 0;
  first.h = lowLevelDistToGoal(first.x, first.y, info);

//  boDebug(510) << "    " << k_funcinfo << "OPEN_ADD: " << "pos: (" << first.x << "; " << first.y <<
//      "); g: " << first.g << "; h: " << first.h << endl;
  open.add(first);
  VISITED(first.x, first.y) = true;
  INOPEN(first.x, first.y) = true;

  // Is the path found?
  bool pathfound = false;

  // When range is 0 and we can't get exactly to destination point, we will go
  //  to nearest possible point
  BosonPathNode nearest = first;
  tm_initmisc = pr.elapsed();


  // Main loop
  while(!open.isEmpty())
  {
    // Take first node from open
    open.takeFirst(n);
    INOPEN(n.x, n.y) = false;
//    boDebug(510) << "    " << k_funcinfo << "OPEN_TAKE: " << "pos: (" << n.x << "; " << n.y <<
//        "); g: " << n.g << "; h: " << n.h << ";  open.count(): " << open.count() << endl;

    // Check if it's the goal
    if(nextRegion)
    {
      if(cellRegion(n.x, n.y) == nextRegion)
      {
        // This cell is in the next region
//        boDebug(510) << "" << k_funcinfo << "next region found, braking" << endl;
        pathfound = true;
        break;
      }
    }
    else
    {
      if((QMAX(QABS(n.x - info->dest.x()), QABS(n.y - info->dest.y())) - info->range) < PF_TNG_EPSILON)
      {
        // QMAX(...) <= info->range
        // This is the destination cell
//        boDebug(510) << "" << k_funcinfo << "goal cell found, braking" << endl;
        pathfound = true;
        break;
      }
    }

    // Add all neighbors of the cell to open
    for(unsigned char i = 0; i < 8; i++)
    {
      n2.x = n.x + xoffsets[i];
      n2.y = n.y + yoffsets[i];

      // Make sure cell is in search area
      if((n2.x < areax) || (n2.x >= areax + areaw) || (n2.y < areay) || (n2.y >= areay + areah))
      {
        continue;
      }
      // Make sure cell's passability is what we need
      if(cellPassability(n2.x, n2.y) != info->passability)
      {
        continue;
      }
      // Check if it's in correct region
      BosonPathRegion* r = cellRegion(n2.x, n2.y);
      if(r != currentregion)
      {
        if(!nextRegion || (r != nextRegion))
        {
          // It's not in the next region
          continue;
        }
      }


      if(!VISITED(n2.x, n2.y))
      {
        // Not visited yet - calculate costs
        n2.g = n.g + lowLevelCost(n2.x, n2.y, info);
        n2.h = lowLevelDistToGoal(n2.x, n2.y, info);

        // Check if n2 is nearest node so far
        if(((n2.h + n2.g * TNG_NEAREST_G_FACTOR) - (nearest.h + nearest.g * TNG_NEAREST_G_FACTOR)) < -PF_TNG_EPSILON)
        {
          nearest = n2;
        }

        // Add node to open
//        boDebug(510) << "        " << k_funcinfo << "OPEN_ADD: " << "pos: (" << n2.x << "; " << n2.y <<
//            "); g: " << n2.g << "; h: " << n2.h << endl;
        open.add(n2);
        VISITED(n2.x, n2.y) = true;
        INOPEN(n2.x, n2.y) = true;
        // This direction points to the parent, i.e. previous element in the
        //  path, so we need to reverse direction
        PARENTDIR(n2.x, n2.y) = REVERSEDIR(i);
      }
      else if(INOPEN(n2.x, n2.y))
      {
        // Node is already in open - change cost
        QValueList<BosonPathNode>::iterator it;
        // Find node in open
        for(it = open.begin(); it != open.end(); ++it)
        {
          if((n2.x == (*it).x) && (n2.y == (*it).y))
          {
            break;
          }
        }
        if(it == open.end())
        {
          boError(510) << k_funcinfo << "No region with pos (" << n2.x << "; " << n2.y << ") found in OPEN!!!" << endl;
          continue;
        }

        // Calculate costs
        n2.g = n.g + lowLevelCost(n2.x, n2.y, info);
        if((*it).g < n2.g)
        {
          // Old path is better - leave it untouched
          continue;
        }
        // Distance won't change
        n2.h = (*it).h;

        // Check if n2 is nearest node so far
        if(((n2.h + n2.g * TNG_NEAREST_G_FACTOR) - (nearest.h + nearest.g * TNG_NEAREST_G_FACTOR)) < -PF_TNG_EPSILON)
        {
          nearest = n2;
        }

//        boDebug(510) << "        " << k_funcinfo << "OPEN_REPLACE: " << "pos: (" << n2.x << "; " << n2.y <<
//            "); g_old: " << (*it).g << "; g_new: " << n2.g << "; h: " << n2.h << endl;
        // Delete old node from open
        open.remove(it);
        // And add new one
        open.add(n2);
        PARENTDIR(n2.x, n2.y) = REVERSEDIR(i);
      }
    }
  }

  tm_mainloop = pr.elapsed();
//  boDebug(510) << k_funcinfo << "path searching finished" << endl;

  // Traceback path
  if(!pathfound && info->range > 0)
  {
    // Low-level path should always be found
    boError(510) << k_funcinfo << "No low-level path found!!!" << endl;
    info->passable = false;
  }
  else
  {
    if(!pathfound)
    {
      // We didn't find exact path to destination, so use nearest point instead
      n = nearest;
//      boDebug(510) << k_funcinfo << "Using NEAREST node at (" << n.x << "; " << n.y << "); g: " <<
//          n.g << "; h: " << n.h << endl;
    }
    // First copy path to list. We use temporary list here because vector doesn't
    //  have prepend() and path's length isn't known
    // Note that low-level path must be in canvas-coords, so we convert cell
    //  coords to canvas ones here
    // FIXME: we can get path length, e.g. by adding level var to nodes
    QValueList<BoVector2> temp;
    // Coordinates of original nodes
    QValueList<QPoint> orignodes;
    // Coordinate of the last node in the path (destination)
    int x = n.x;
    int y = n.y;
    orignodes.append(QPoint(x, y));
    QPoint p;
    BoVector2 canvas; // Point in canvas coords
    p.setX(x);
    p.setY(y);
    canvas.setX(p.x() + 1.0f / 2);
    canvas.setY(p.y() + 1.0f / 2);
    temp.prepend(canvas);
//    boDebug(510) << k_funcinfo << "Added first point at (" << x << "; " << y << ")" << endl;
    while((x != first.x) || (y != first.y))
    {
      // Take next region
//      boDebug(510) << k_funcinfo << "parent direction is " << PARENTDIR(p.x(), p.y()) << endl;
      x += xoffsets[(int)PARENTDIR(p.x(), p.y())];
      y += yoffsets[(int)PARENTDIR(p.x(), p.y())];
      orignodes.append(QPoint(x, y));
      p.setX(x);
      p.setY(y);
      canvas.setX(p.x() + 1.0f / 2);
      canvas.setY(p.y() + 1.0f / 2);
      // We must prepend regions, not append them, because we go from destination
      //  to start here
      temp.prepend(canvas);
//      boDebug(510) << k_funcinfo << "Added point (" << canvas.x() << "; " << canvas.y() << ")" << endl;
    }


    QValueList<QPoint>::iterator nit;
    QString nodedebug;
    for(nit = orignodes.begin(); nit != orignodes.end(); ++nit)
    {
      int x = (*nit).x();
      int y = (*nit).y();
      nodedebug += QString("N(pos: (%1; %2); g: %3; h: %4),   ").arg(x).arg(y).arg(lowLevelCost(x, y, info)).arg(lowLevelDistToGoal(x, y, info));
    }
    boDebug(510) << k_funcinfo << "Found path: " << nodedebug << endl;

    // We add special point to the end of the path, telling unit what to do when
    //  it has reached the end of this path
    if(nextRegion && pathfound)
    {
      // We have reached next region
      temp.append(BoVector2(PF_NEXT_REGION, PF_NEXT_REGION));
    }
    else
    {
      // We have reached destination (or nearest possible point from it)
      temp.append(BoVector2(PF_END_CODE, PF_END_CODE));
    }

    // Copy temp path to real path vector
//    boDebug(510) << k_funcinfo << "copying path" << endl;
    info->llpath.clear();
    info->llpath.reserve(temp.count());
    QValueList<BoVector2>::iterator it;
    for(it = temp.begin(); it != temp.end(); ++it)
    {
      info->llpath.append(*it);
    }
//    boDebug(510) << k_funcinfo << "found path has " << info->llpath.count() << " steps" << endl;

    // Set passable flag to true
    info->passable = true;
    tm_copypath = pr.elapsed();


#ifdef VISUALIZE_PATHS
    // Add LineVisualization stuff
    {
      QValueList<BoVector3> points;
      for(unsigned int point = 0; point < info->llpath.count(); point++)
      {
        float x = info->llpath[point].x();
        float y = info->llpath[point].y();
//        boDebug(510) << "  " << k_funcinfo << "Adding lineviz for point (" << x << "; " << y << ")" << endl;
        points.append(BoVector3(x, -y, 0.0f));
      }
      float pointSize = 3.0f;
      int timeout = 100;
      float zOffset = 0.5f;
      BoVector4 color(1.0f, 0.5f, 0.0f, 0.8f); // orange
      BosonPathVisualization::pathVisualization()->addLineVisualization(points, color, pointSize, timeout, zOffset);
    }
#endif
    tm_viz = pr.elapsed();
  }

//  boDebug(510) << k_funcinfo << "Deleting visited @ " << visited << endl;
  delete[] visited;
  delete[] inopen;
  delete[] parentdirections;
  tm_uninit = pr.stop();

  boDebug(510) << k_funcinfo << "Took " << tm_uninit << " usec:" << endl <<
      "    area init: " << tm_initarea << ";  maps init: " << tm_initmaps - tm_initarea << ";  misc init: " << tm_initmisc - tm_initmaps << endl <<
      "    main loop: " << tm_mainloop - tm_initmisc << endl <<
      "    path copy: " << tm_copypath - tm_mainloop << ";  viz: " << tm_viz - tm_copypath << ";  maps uninit: " << tm_uninit - tm_viz << endl;

#undef VISITED
#undef INOPEN
#undef PARENTDIR
}

void BosonPath2::findFlyingUnitPath(BosonPathInfo* info)
{
  if(!info->flying)
  {
//    boDebug() << k_funcinfo << "Called for non-flying unit" << endl;
    return;
  }

  // We use usual A* pathfinder for flying units. It's quite similar to the old
  //  pathfinder and it only searches TNG_FLYING_STEPS steps ahead. As there
  //  usually aren't too many flying units, it should be enough.

  long int tm_initarea, tm_initmaps, tm_initmisc, tm_mainloop, tm_copypath = 0, tm_viz = 0, tm_uninit;
  BosonProfiler pr('P' + 'F' + '_' + 'T' + 'N' + 'G' + ' ' + 'l' + 'o' + 'f' + 'l' + 'y');

  // List of open nodes
  BosonPathHeap<BosonPathFlyingNode> open;

  // Unit's position
  int unitx = (int)info->start.x();
  int unity = (int)info->start.y();

  // Search area's coordinates and size.
  int areax = QMAX(unitx - TNG_FLYING_STEPS, 0);
  int areay = QMAX(unity - TNG_FLYING_STEPS, 0);
//  int areaw = QMIN(TNG_FLYING_STEPS * 2 + 1, (int)mMap->width() - areax);
//  int areah = QMIN(TNG_FLYING_STEPS * 2 + 1, (int)mMap->height() - areay);
  int areaw = QMIN(unitx + TNG_FLYING_STEPS + 1, (int)mMap->width()) - areax;
  int areah = QMIN(unity + TNG_FLYING_STEPS + 1, (int)mMap->height()) - areay;
  tm_initarea = pr.elapsed();
//  boDebug(510) << k_funcinfo << "Area is: (" << areax << "x" << areay << "+" << areaw << "x" << areah << ")" << endl;
  // Create list of nodes that are in visited and of those in open.
  // This is used for performance reasons, it's faster than seacrhing open for
  //  every node.
  unsigned int maxnodes = areaw * areah;
//  boDebug(510) << k_funcinfo << "Creating visited and inopen bool maps for " << maxnodes << " nodes" << endl;
  bool* visited = new bool[maxnodes];
//  boDebug(510) << k_funcinfo << "Created visited @ " << visited << endl;
  bool* inopen = new bool[maxnodes];
#define VISITED(x, y) visited[(y - areay) * areaw + (x - areax)]
#define INOPEN(x, y) inopen[(y - areay) * areaw + (x - areax)]
  for(unsigned int i = 0; i < maxnodes; i++)
  {
    visited[i] = false;
    inopen[i] = false;
  }
  // Direction to the parent node, used to traceback path
  char* parentdirections = new char[maxnodes];
#define PARENTDIR(x, y) parentdirections[(y - areay) * areaw + (x - areax)]
  // We don't init parentdirections, because for each visited cell, we set
  //  direction anyway, and we don't use directions for unvisited cells (unless
  //  there's a bug somewhere)
  tm_initmaps = pr.elapsed();


  // Find cell that the unit is on atm.
  BosonPathFlyingNode first, n, n2;
  first.x = unitx;
  first.y = unity;
  first.depth = 0;
  first.g = 0;
  first.h = lowLevelDistToGoal(first.x, first.y, info);
  // We set first cell's parent dir to 8, then turning penalty will be added to
  //  it's all neighbors (instead of all but one), so they'll all be equal
  PARENTDIR(first.x, first.y) = 8;

//  boDebug(510) << "    " << k_funcinfo << "OPEN_ADD: " << "pos: (" << first.x << "; " << first.y <<
//      "); g: " << first.g << "; h: " << first.h << endl;
  open.add(first);
  VISITED(first.x, first.y) = true;
  INOPEN(first.x, first.y) = true;

  // Is the path found?
  bool pathfound = false;
  bool goalReached = false;

  // When range is 0 and we can't get exactly to destination point, we will go
  //  to nearest possible point
  BosonPathFlyingNode nearest = first;
  tm_initmisc = pr.elapsed();

  // Destination in cell coordinates
  int destcellx = (int)info->dest.x();
  int destcelly = (int)info->dest.y();


  // Main loop
  while(!open.isEmpty())
  {
    // Take first node from open
    open.takeFirst(n);
    INOPEN(n.x, n.y) = false;
//    boDebug(510) << "    " << k_funcinfo << "OPEN_TAKE: " << "pos: (" << n.x << "; " << n.y <<
//        "); g: " << n.g << "; h: " << n.h << ";  open.count(): " << open.count() << endl;

    // We only search TNG_FLYING_STEPS steps ahead
    if(n.depth >= TNG_FLYING_STEPS)
    {
        pathfound = true;
        break;
    }
    // Check if it's the goal
    if(info->range > 0)
    {
      if(QMAX(QABS(n.x - destcellx), QABS(n.y - destcelly)) <= info->range)
      {
        // This is one of the destination cells
//        boDebug(510) << "" << k_funcinfo << "goal cell found, braking" << endl;
        pathfound = true;
        goalReached = true;
        break;
      }
    }
    else
    {
      // range 0 means to get as close as possible
      if((n.x == destcellx) && (n.y == destcelly))
      {
        // we're at dest cell
        pathfound = true;
        goalReached = true;
        break;
      }
    }

    // Add all neighbors of the cell to open
    for(unsigned char i = 0; i < 8; i++)
    {
      n2.x = n.x + xoffsets[i];
      n2.y = n.y + yoffsets[i];
      n2.depth = n.depth + 1;

      // Make sure cell is in search area
      if((n2.x < areax) || (n2.x >= areax + areaw) || (n2.y < areay) || (n2.y >= areay + areah))
      {
        // Shouldn't happen.
        // Note: this happens when border of the map is reached (n2 is not on
        //  the map anymore)
        boError() << k_funcinfo << "Cell (" << n2.x << "; " << n2.y << ") not in search area!" << endl;
        continue;
      }

      // Make sure cell is passable
      if(!cell(n2.x, n2.y)->passable())
      {
        continue;
      }


      if(!VISITED(n2.x, n2.y))
      {
        // Not visited yet - calculate costs
        n2.g = n.g + lowLevelCostAir(n2.x, n2.y, info);
        // Penalty for turning
        if(PARENTDIR(n.x, n.y) != REVERSEDIR(i))
        {
          // Direction is different from last direction
          n2.g += TNG_FLYING_TUNRING_COST;
        }
        n2.h = lowLevelDistToGoal(n2.x, n2.y, info);

        // Check if n2 is nearest node so far
        if((n2.h + n2.g * TNG_NEAREST_G_FACTOR) < (nearest.h + nearest.g * TNG_NEAREST_G_FACTOR))
        {
          nearest = n2;
        }

        // Add node to open
//        boDebug(510) << "        " << k_funcinfo << "OPEN_ADD: " << "pos: (" << n2.x << "; " << n2.y <<
//            "); g: " << n2.g << "; h: " << n2.h << endl;
        open.add(n2);
        VISITED(n2.x, n2.y) = true;
        INOPEN(n2.x, n2.y) = true;
        // This direction points to the parent, i.e. previous element in the
        //  path, so we need to reverse direction
        PARENTDIR(n2.x, n2.y) = REVERSEDIR(i);
      }
      else if(INOPEN(n2.x, n2.y))
      {
        // Node is already in open - change cost
        QValueList<BosonPathFlyingNode>::iterator it;
        // Find node in open
        for(it = open.begin(); it != open.end(); ++it)
        {
          if((n2.x == (*it).x) && (n2.y == (*it).y))
          {
            break;
          }
        }
        if(it == open.end())
        {
          boError(510) << k_funcinfo << "No region with pos (" << n2.x << "; " << n2.y << ") found in OPEN!!!" << endl;
          continue;
        }

        // Calculate costs
        n2.g = n.g + lowLevelCostAir(n2.x, n2.y, info);
        if(PARENTDIR(n.x, n.y) != REVERSEDIR(i))
        {
          // Direction is different from last direction
          n2.g += TNG_FLYING_TUNRING_COST;
        }
        if((*it).g < n2.g)
        {
          // Old path is better - leave it untouched
          continue;
        }
        // Distance won't change
        n2.h = (*it).h;

        // Check if n2 is nearest node so far
        if((n2.h + n2.g * TNG_NEAREST_G_FACTOR) < (nearest.h + nearest.g * TNG_NEAREST_G_FACTOR))
        {
          nearest = n2;
        }

//        boDebug(510) << "        " << k_funcinfo << "OPEN_REPLACE: " << "pos: (" << n2.x << "; " << n2.y <<
//            "); g_old: " << (*it).g << "; g_new: " << n2.g << "; h: " << n2.h << endl;
        // Delete old node from open
        open.remove(it);
        // And add new one
        open.add(n2);
        PARENTDIR(n2.x, n2.y) = REVERSEDIR(i);
      }
    }
  }

  tm_mainloop = pr.elapsed();
//  boDebug(510) << k_funcinfo << "path searching finished" << endl;

  // Traceback path
  if(!pathfound && info->range > 0)
  {
    boError(510) << k_funcinfo << "No path found!!!" << endl;
    info->passable = false;
  }
  else
  {
    if(!pathfound)
    {
      // We didn't find exact path to destination, so use nearest point instead
      n = nearest;
//      boDebug(510) << k_funcinfo << "Using NEAREST node at (" << n.x << "; " << n.y << "); g: " <<
//          n.g << "; h: " << n.h << endl;
    }
    // First copy path to list. We use temporary list here because vector doesn't
    //  have prepend() and path's length isn't known
    // Note that low-level path must be in canvas-coords, so we convert cell
    //  coords to canvas ones here
    // FIXME: we can get path length, e.g. by adding level var to nodes
    QValueList<BoVector2> temp;
    // Coordinate of the last node in the path (destination)
    int x = n.x;
    int y = n.y;
    QPoint p;
    BoVector2 canvas; // Point in canvas coords
    p.setX(x);
    p.setY(y);
    canvas.setX(p.x() + 1.0f / 2);
    canvas.setY(p.y() + 1.0f / 2);
    temp.prepend(canvas);
//    boDebug(510) << k_funcinfo << "Added first point at (" << x << "; " << y << ")" << endl;
    while((x != first.x) || (y != first.y))
    {
      // Take next region
//      boDebug(510) << k_funcinfo << "parent direction is " << PARENTDIR(p.x(), p.y()) << endl;
      x += xoffsets[(int)PARENTDIR(p.x(), p.y())];
      y += yoffsets[(int)PARENTDIR(p.x(), p.y())];
      p.setX(x);
      p.setY(y);
      canvas.setX(p.x() + 1.0f / 2);
      canvas.setY(p.y() + 1.0f / 2);
      // We must prepend regions, not append them, because we go from destination
      //  to start here
      temp.prepend(canvas);
//      boDebug(510) << k_funcinfo << "Added point (" << canvas.x() << "; " << canvas.y() << ")" << endl;
    }

    // If path was found but goal wasn't reached, then we found only a partial
    //  path. We add PF_NEXT_REGION point, which will make pathfinder be called
    //  again once end of this path is reached.
    if(!goalReached && pathfound)
    {
      // We have reached next region
      temp.append(BoVector2(PF_NEXT_REGION, PF_NEXT_REGION));
    }
    else
    {
      // We have reached destination (or nearest possible point from it)
      temp.append(BoVector2(PF_END_CODE, PF_END_CODE));
    }

    // Copy temp path to real path vector
//    boDebug(510) << k_funcinfo << "copying path" << endl;
    info->llpath.clear();
    info->llpath.reserve(temp.count());
    QValueList<BoVector2>::iterator it;
    for(it = temp.begin(); it != temp.end(); ++it)
    {
      info->llpath.append(*it);
    }
//    boDebug(510) << k_funcinfo << "found path has " << info->llpath.count() << " steps" << endl;

    // Set passable flag to true
    info->passable = true;
    tm_copypath = pr.elapsed();


#ifdef VISUALIZE_PATHS
    // Add LineVisualization stuff
    {
      QValueList<BoVector3> points;
      for(unsigned int point = 0; point < info->llpath.count(); point++)
      {
        float x = info->llpath[point].x();
        float y = info->llpath[point].y();
//        boDebug(510) << "  " << k_funcinfo << "Adding lineviz for point (" << x << "; " << y << ")" << endl;
        points.append(BoVector3(x, -y, 0.0f));
      }
      float pointSize = 3.0f;
      int timeout = 100;
      float zOffset = 0.5f;
      BoVector4 color(1.0f, 0.5f, 0.0f, 0.8f); // orange
      BosonPathVisualization::pathVisualization()->addLineVisualization(points, color, pointSize, timeout, zOffset);
    }
#endif
    tm_viz = pr.elapsed();
  }

//  boDebug(510) << k_funcinfo << "Deleting visited @ " << visited << endl;
  delete[] visited;
  delete[] inopen;
  delete[] parentdirections;
  tm_uninit = pr.stop();

  boDebug(510) << k_funcinfo << "Took " << tm_uninit << " usec:" << endl <<
      "    area init: " << tm_initarea << ";  maps init: " << tm_initmaps - tm_initarea << ";  misc init: " << tm_initmisc - tm_initmaps << endl <<
      "    main loop: " << tm_mainloop - tm_initmisc << endl <<
      "    path copy: " << tm_copypath - tm_mainloop << ";  viz: " << tm_viz - tm_copypath << ";  maps uninit: " << tm_uninit - tm_viz << endl;

#undef VISITED
#undef INOPEN
#undef PARENTDIR
}

bool BosonPath2::rangeCheck(BosonPathInfo* info)
{
  // ONLY FOR FLYING UNITS!
  // This method checks if it's possible to go to better place than where unit
  //  currently is.
  // It returns true if it is possible to get to better place, otherwise false
  // First check if unit is in range
  int dist = (int)QMAX(QABS(info->dest.x() - info->start.x()), QABS(info->dest.y() - info->start.y()));
  if(dist <= info->range)
  {
    return false;
  }

  // If unit's not in range, it might still be possible that it can't get closer
  //  to goal than it already is. This is usually the case when you move many
  //  units at once. Then one unit will go to goal, but others will try to find path to
  //  goal and waste time.
  // First quick check if goal is occupied if range is 0
  // If distance to goal is more than TNG_RANGE_STEPS, we won't search complete
  //  path and we don't need this method
  if(dist > TNG_RANGE_STEPS)
  {
    return true;
  }
  // Destination in cell coordinates
  int destCellX = (int)info->dest.x();
  int destCellY = (int)info->dest.y();
  if(info->range == 0)
  {
    if(lowLevelCostAir(destCellX, destCellY, info) < PF_TNG_COST_STANDING_UNIT)
    {
      // It is still possible to get to destination point
      return true;
    }
    else if(dist == 1)
    {
      // Goal is occupied and unit's next to it - can't get any closer
      return false;
    }
  }

  int w = mMap->width();
  int h = mMap->height();
  int x, y;
  for(int range = 1; range < dist; range++)
  {
    // We must not look too far, otherwise we have crash, because marking array
    //  isn't big enough
    if(dist + range > TNG_RANGE_STEPS)
    {
      return true;
    }
    // Bad duplicated code. But it's faster this way
    // First check upper and lower sides of "rectangle"
    for(x = destCellX - range; x <= destCellX + range; x++)
    {
      if((x < 0) || (x >= w))
      {
        continue;
      }
      if(destCellY - range >= 0)
      {
        if(lowLevelCostAir(x, destCellY - range, info) < PF_TNG_COST_STANDING_UNIT)
        {
          info->range = QMAX(info->range, range);
          return true;
        }
      }
      if(destCellY + range < h)
      {
        if(lowLevelCostAir(x, destCellY + range, info) < PF_TNG_COST_STANDING_UNIT)
        {
          info->range = QMAX(info->range, range);
          return true;
        }
      }
    }
    // Then right and left sides. Note that corners are already checked
    for(y = destCellY - range + 1; y < destCellY + range; y++)
    {
      if((y < 0) || (y >= h))
      {
        continue;
      }
      if(destCellX - range >= 0)
      {
        if(lowLevelCostAir(destCellX - range, y, info) < PF_TNG_COST_STANDING_UNIT)
        {
          info->range = QMAX(info->range, range);
          return true;
        }
      }
      if(destCellX + range < w)
      {
        if(lowLevelCostAir(destCellX + range, y, info) < PF_TNG_COST_STANDING_UNIT)
        {
          info->range = QMAX(info->range, range);
          return true;
        }
      }
    }
  }
  return false;
}

void BosonPath2::cellsOccupiedStatusChanged(int x1, int y1, int x2, int y2)
{
  boDebug(510) << k_funcinfo << "area: (" << x1 << "x" << y1 << "-" << x2 << "x" << y2 << ")" << endl;
  long int tm_calcsectorarea, tm_regionlist, tm_hlpathinvalidate, tm_regionreinit, tm_recalcneighbor,
      tm_regionlists2, tm_recalcregioncosts, tm_regiongroups, tm_end;
  static int profilerId = -boProfiling->requestEventId("Stupid profiling name");
  BosonProfiler profiler(profilerId);

  // Calculate changed area in sector coords
  int sx1, sy1, sx2, sy2;
  sx1 = x1 / mSectorWidth;
  sy1 = y1 / mSectorHeight;
  sx2 = x2 / mSectorWidth;
  sy2 = y2 / mSectorHeight;
//  boDebug(510) << k_funcinfo << "sector area: (" << sx1 << "x" << sy1 << "-" << sx2 << "x" << sy2 << ")" << endl;
  tm_calcsectorarea = profiler.elapsed();

  // First we need to make list of all to-be-deleted regions to revalidate
  //  high-level paths later
  QPtrList<BosonPathRegion> oldregions;
  BosonPathSector* s;
  for(int y = sy1; y <= sy2; y++)
  {
    for(int x = sx1; x <= sx2; x++)
    {
      s = sector(x, y);
      for(unsigned int i = 0; i < s->regions.count(); i++)
      {
        oldregions.append(s->regions[i]);
      }
    }
  }
  tm_regionlist = profiler.elapsed();

  // Validate high-level paths. If high-level path contains any of to-be-deleted
  //  regions, it will be considered to be invalid
  QPtrListIterator<BosonPathHighLevelPath> it(mHLPathCache);
  QPtrList<BosonPathHighLevelPath> invalidpaths;
//  boDebug(510) << k_funcinfo << "Validating " << it.count() << " HL paths" << endl;
  BosonPathHighLevelPath* p;
  while(it.current())
  {
    p = it.current();
    for(unsigned int i = 0; i < p->path.count(); i++)
    {
      if(oldregions.containsRef(p->path[i]))
      {
        invalidpaths.append(p);
      }
    }
    ++it;
  }
  while(!invalidpaths.isEmpty())
  {
    p = invalidpaths.take(0);
    // This path is now invalid
    p->valid = false;
    if(p->users == 0)
    {
      // Paths with no users shouldn't be in the cache
      boWarning(510) << k_funcinfo << "Path without users found in cache!" << endl;
      removeHighLevelPath(p);
      delete p;
    }
  }
  tm_hlpathinvalidate = profiler.elapsed();

  // Reinit regions in changed sectors
  // FIXME: probably this can be done _much_ faster, e.g. check first if
  //  recalculation is actually needed.
  unsigned int regionscount = 0;
  for(int y = sy1; y <= sy2; y++)
  {
    for(int x = sx1; x <= sx2; x++)
    {
//      boDebug(510) << k_funcinfo << "reiniting regions for sector at (" << x << "; " << y << ")" << endl;
      sector(x, y)->reinitRegions();
      regionscount += sector(x, y)->regions.count();
    }
  }
  tm_regionreinit = profiler.elapsed();

  // Recalculate region neighbors
  int nx1, ny1, nx2, ny2;
  /*nx1 = sx1 * mSectorWidth - 1;
  ny1 = sy1 * mSectorHeight - 1;
  nx2 = (sx2 + 1) * mSectorWidth;
  ny2 = (sy2 + 1) * mSectorHeight;*/

//  findRegionNeighbors(nx1, ny1, nx2, ny2);
  nx1 = QMAX(sx1 * ((int)mSectorWidth) - 1, 0);
  ny1 = QMAX(sy1 * ((int)mSectorHeight) - 1, 0);
  nx2 = QMIN((sx2 + 1) * ((int)mSectorWidth), ((int)mMap->width()) - 1);
  ny2 = QMIN((sy2 + 1) * ((int)mSectorHeight), ((int)mMap->height()) - 1);

//  boDebug(510) << k_funcinfo << "finding neighbors in area: (" << nx1 << "x" << ny1 << "-" << nx2 << "x" << ny2 << ")" << endl;
  findRegionNeighbors(QMAX(sx1 * (int)mSectorWidth - 1, 0), QMAX(sy1 * (int)mSectorHeight - 1, 0),
      QMIN((sx2 + 1) * (int)mSectorWidth, (int)mMap->width() - 1), QMIN((sy2 + 1) * (int)mSectorHeight, (int)mMap->height() - 1));
  tm_recalcneighbor = profiler.elapsed();

  // Recalculate neighbor costs
  QPtrVector<BosonPathRegion> regions(regionscount);  // All regions in affected sectors
  for(int y = sy1; y <= sy2; y++)
  {
    for(int x = sx1; x <= sx2; x++)
    {
//      boDebug(510) << k_funcinfo << "adding regions in sector at (" << x << "; " << y << ") to regions" << endl;
      for(unsigned int i = 0; i < sector(x, y)->regions.count(); i++)
      {
        regions.insert(regions.count(), sector(x, y)->regions[i]);
      }
    }
  }
  // We need to recalculate neighbor costs not only for affected sectors, but
  //  also for their neighbors
  QPtrVector<BosonPathRegion> neighbors(regionscount / 2);
  BosonPathRegion* n;
  for(unsigned int i = 0; i < regions.count(); i++)
  {
    for(unsigned int j = 0; j < regions[i]->neighbors.count(); j++)
    {
      n = regions[i]->neighbors[j].region;
      if(!regions.containsRef(n))
      {
        // n is not in affected regions' list
        if(!neighbors.containsRef(n))
        {
          // It's not in neighbor's list yet either. Add it there
          if(neighbors.size() == neighbors.count())
          {
            neighbors.resize(neighbors.size() + 5);
          }
          neighbors.insert(neighbors.count(), n);
        }
      }
    }
  }
  tm_regionlists2 = profiler.elapsed();
//  boDebug(510) << k_funcinfo << "recalculating costs for regions" << endl;
  initRegionCosts(regions);
  initRegionCosts(neighbors);
  tm_recalcregioncosts = profiler.elapsed();

  // Reinit region groups info
  initRegionGroups(regions);
  tm_regiongroups = profiler.elapsed();

  // Take care of eyecandy, too
//  boDebug(510) << k_funcinfo << "recalculating colormap" << endl;
  colorizeRegions();
  tm_end = profiler.stop();
  boDebug(510) << k_funcinfo << "END, took " <<  tm_end / 1000.0 << " ms in total:" << endl <<
      "sectarea: " << tm_calcsectorarea << ";  reglist: " << tm_regionlist - tm_calcsectorarea <<
      ";  hlpathinvalidate: " << tm_hlpathinvalidate - tm_regionlist << ";  regreinit: " << tm_regionreinit - tm_hlpathinvalidate << endl <<
      "recalcneighbor: " << tm_recalcneighbor - tm_regionreinit << ";  reglists2: " << tm_regionlists2 - tm_recalcneighbor <<
      ";  recalcregcosts: " << tm_recalcregioncosts - tm_regionlists2 << "; reggroups: " << tm_regiongroups - tm_recalcregioncosts <<
      ";  colorize: " << tm_end - tm_regiongroups << endl;
}

void BosonPath2::releaseHighLevelPath(BosonPathHighLevelPath* hlpath)
{
  if(!hlpath)
  {
    boError(510) << k_funcinfo << "NULL hlpath" << endl;
    return;
  }
  if(hlpath->users == 0)
  {
    boError(510) << k_funcinfo << "hlpath has no users!" << endl;
  }
  else
  {
    // Decrease number of path's users
    hlpath->users--;
  }

  // If it has no users left, remove it from the cache and delete it
  if(hlpath->users == 0)
  {
    removeHighLevelPath(hlpath);
    delete hlpath;
  }
}

void BosonPath2::initSectors()
{
//  boDebug(510) << k_funcinfo << endl;
  // Calculate number of sectors we'll have
  // TODO: make dynamic, depending on the map size
  mSectorWidth = 10;
  mSectorHeight = 10;
  // this is not the best approach, but works
  int sectorscount = ((mMap->width() / mSectorWidth) + 1) * ((mMap->height() / mSectorHeight) + 1);
//  boDebug(510) << k_funcinfo << "there will be " << sectorscount << " sectors (map is " <<
//      mMap->width() << "x" << mMap->height() << ")" << endl;

  // Create sector array
  mSectors = new BosonPathSector[sectorscount];
//  boDebug(510) << k_funcinfo << "Sectors are at " << mSectors << endl;

  // Create sectors
  for(unsigned int y = 0; y * mSectorHeight < mMap->height(); y++)
  {
    for(unsigned int x = 0; x * mSectorWidth < mMap->width(); x++)
    {
      // C++ doesn't allow any intialization when creating an array with new, so
      //  we must use this a bit hackish solution
//      boDebug(510) << k_funcinfo << "Initing sector " << sector(x, y) << " at ("<< x << "; " << y << ")" << endl;
      sector(x, y)->setPathfinder(this);
      sector(x, y)->setGeometry(x * mSectorWidth, y * mSectorHeight,
          QMIN(mSectorWidth, mMap->width() - x * mSectorWidth), QMIN(mSectorHeight, mMap->height() - y * mSectorWidth));
    }
  }
//  boDebug(510) << k_funcinfo << "END" << endl;
}

void BosonPath2::initRegions()
{
//  boDebug(510) << k_funcinfo << endl;
  // Init ids pool
  // It should be possible to have height*width / 4 regions at most
  // +1 is because of possible modulo
  int maxregcount = mMap->width() * mMap->height() / 4 + 1;
//  boDebug(510) << k_funcinfo << "there can be " << maxregcount << " regions at most" << endl;
  mRegionIdUsed = new bool[maxregcount];
  for(int i = 0; i < maxregcount; i++)
  {
    mRegionIdUsed[i] = false;
  }

  // Allocate some space for regions
  // It assumes every sector has 1.2 regions on average
  int regioncount = (int)(((mMap->width() / mSectorWidth) + 1) * ((mMap->height() / mSectorHeight) + 1) * 1.2);
//  boDebug(510) << k_funcinfo << "allocating space for " << regioncount << " regions" << endl;
  mRegions.resize(regioncount);

  for(unsigned int y = 0; y * mSectorHeight < mMap->height(); y++)
  {
    for(unsigned int x = 0; x * mSectorWidth < mMap->width(); x++)
    {
//      boDebug(510) << k_funcinfo << "initing regions for sector at (" << x << "; " << y << ")" << endl;
      sector(x, y)->initRegions();
    }
  }
//  boDebug(510) << k_funcinfo << "END" << endl;
}

void BosonPath2::initCellPassability()
{
#define RAD2DEG (180.0/M_PI)
//  boDebug(510) << k_funcinfo << endl;
  const float maxPassableSlope = 45.0f; // If slope <= this, then it's passable, otherwise not
  float minh, maxh, slope;
  for(unsigned int y = 0; y < mMap->height(); y++)
  {
    for(unsigned int x = 0; x < mMap->width(); x++)
    {
      // Find min and max heights for that cell
      minh = 1000.0f;
      maxh = -1000.0f;
      for(unsigned int i = x; i <= x + 1; i++)
      {
        for(unsigned int j = y; j <= y + 1; j++)
        {
          minh = QMIN(minh, mMap->heightAtCorner(i, j));
          maxh = QMAX(maxh, mMap->heightAtCorner(i, j));
        }
      }
      // Calculate slope between min and max height.
      // For simplicity, we check only one pair of corners and use 1 as distance
      //  between them
      if(minh == maxh)
      {
        slope = 0.0f;
      }
      else
      {
        slope = atan(maxh - minh) * RAD2DEG;
      }

      if(slope > maxPassableSlope)
      {
//        boDebug(510) << k_funcinfo << "Cell at (" << x << "; " << y << ") won't be passable, slope: " << slope << endl;
      }

      cell(x, y)->setPassable(slope <= maxPassableSlope);
    }
  }
//  boDebug(510) << k_funcinfo << "END" << endl;
#undef RAD2DEG
}

void BosonPath2::initRegionGroups(QPtrVector<BosonPathRegion>& regions)
{
  // Go through all regions
  for(unsigned int i = 0; i < regions.count(); i++)
  {
    if(regions[i]->group != 0)
    {
      // This region already belongs to a group
      continue;
    }

    // Create new group for this region
    BosonPathRegionGroup* group = new BosonPathRegionGroup;
//    boDebug(510) << k_funcinfo << "GROUP_NEW: created new group @ " << group << endl;
    group->passabilityType = regions[i]->passabilityType;
    findRegionsInGroup(group, regions[i]);
  }
}

void BosonPath2::findRegionsInGroup(BosonPathRegionGroup* group, BosonPathRegion* start)
{
//  boDebug(510) << k_funcinfo << endl;
  QPtrList<BosonPathRegion> open;

  BosonPathRegion* n;
  BosonPathRegion* n2;

  start->group = group;
  group->regions.append(start);
  open.append(start);

  while(!open.isEmpty())
  {
    n = open.take(0);

    for(unsigned int i = 0; i < n->neighbors.count(); i++)
    {
      n2 = n->neighbors[i].region;

      if(n2->passabilityType != group->passabilityType)
      {
        // n2 doesn't fit into this group (different passability types)
        continue;
      }

      if(n2->group != group)
      {
        // n2 should belong to this group, but doesn't
//        boDebug(510) << k_funcinfo << "Trying to add region " << n2->id << " to group @ " << group <<
//            " (atm @ " << n2->group << ")" << endl;
        if(!n2->group)
        {
          // n2 had no group, put it into this group
          n2->group = group;
          group->regions.append(n2);
          open.append(n2);
        }
        else
        {
          // n2's group is similar to this one, so let's join them
          // We remove this group and put all regions of this group to n2's
          //  group. This ensures that older regions won't be removed when
          //  reiniting regions.
          // TODO: we could compare regions count of groups to decide which
          //  group will be removed (bigger should stay)
          // Put all regions in group to n2's group
//          boDebug(510) << k_funcinfo << "GROUP_JOIN: joining " << group <<
//              " into " << n2->group << endl;
//          boDebug(510) << k_funcinfo << "src group (" << group << ") has " <<
//              group->regions.count() << " regions" << endl;
//          boDebug(510) << k_funcinfo << "dest group (" << n2->group << ") has " <<
//              n2->group->regions.count() << " regions" << endl;
          QPtrListIterator<BosonPathRegion> it(group->regions);
          while(it.current())
          {
            n2->group->regions.append(*it);
            (*it)->group = n2->group;
            ++it;
          }
          // Delete group
          delete group;
          // Change group pointer to point to n2's group (we'll continue adding
          //  regions to that group)
          group = n2->group;
          open.append(n2);
        }
      }
    }
  }
}

void BosonPath2::findRegionNeighbors(int x1, int y1, int x2, int y2)
{
//  boDebug(510) << k_funcinfo << endl;
  BosonPathRegion* r;
  BosonPathRegion* r2;
  unsigned int i;
  int nx, ny; // neighbor coords
  for(int y = y1; y <= y2; y++)
  {
    for(int x = x1; x <= x2; x++)
    {
      r = cellRegion(x, y);
      if(!r)
      {
        // Probably not passable cell
        continue;
      }
      for(i = 0; i < 8; i++)
      {
        nx = x + xoffsets[i];
        ny = y + yoffsets[i];

        // Make sure new pos is valid
        if((nx < x1) || (nx > x2) || (ny < y1) || (ny > y2))
        {
          continue;
        }

        r2 = cellRegion(nx, ny);
        if(!r2)
        {
          // Cell doesn't belong to any region. Probably it isn't passable
          continue;
        }

        if(r != r2)
        {
          // r and r2 are neighbors
          r->addNeighbor(r2);
          r2->addNeighbor(r);
        }
      }
    }
  }
//  boDebug(510) << k_funcinfo << "END" << endl;
}

void BosonPath2::initRegionCosts(QPtrVector<BosonPathRegion>& regions)
{
  // It calculates cost and number of "border cells" for each region/neighbor
  //  pair. Border cells are cells that have neighbor cell in another region
  //  (they're at border of the region)
  // Pass 1: find border cells
  /*for(unsigned int i = 0; i < regions.count(); i++)
  {
    regions[i]->findBorderCells();
  }*/
  // Pass 2: calculate costs
  for(unsigned int i = 0; i < regions.count(); i++)
  {
    regions[i]->calculateCosts();
  }
}

void BosonPath2::colorizeRegions()
{
  long int e1, e2, e3;
  static int profilerId = -boProfiling->requestEventId("Stupid profiling name");
  BosonProfiler p(profilerId);
  // We colorize regions with 6 colors
  const unsigned char colors[7][3] = {
    { 255,   0,   0 },
    {   0, 255,   0 },
    {   0,   0, 255 },
    { 255, 255,   0 },
    { 255,   0, 255 },
    {   0, 255, 255 },
    {   0,   0,   0 }   // Special color for cells not belonging to any region
  };

  // Step 1: take all regions and put them into stack. We must only insert
  //  regions with <= 5 non-inserted neighbors.
  bool* inserted = new bool[mRegions.count()];
  for(unsigned int i = 0; i < mRegions.count(); i++)
  {
    inserted[i] = false;
  }
  QPtrQueue<BosonPathRegion> queue;
  unsigned int added = 0;
  int current = 0;
  while(added < mRegions.count())
  {
    if(current >= (int)mRegions.count())
    {
      current = 0;
    }
    if(inserted[current])
    {
      current++;
      continue;
    }
    // Check if mRegions[current] is suitable
    if(mRegions[current]->neighbors.count() <= 5)
    {
      // Only up to 5 neighbors - add
      queue.enqueue(mRegions[current]);
      added++;
      inserted[current] = true;
    }
    else
    {
      // It may be that some regions have already been inserted to queue
      int uninsertedregions = 0;  // FIXME: variable name
      for(unsigned int i = 0; i < mRegions[current]->neighbors.count(); i++)
      {
        if(!inserted[mRegions[current]->neighbors[i].region->id])
        {
          uninsertedregions++;
        }
      }
      if(uninsertedregions <= 5)
      {
        // Add it to queue
        queue.enqueue(mRegions[current]);
        added++;
        inserted[current] = true;
      }
    }
    current++;
  }

  delete[] inserted;

  e1 = p.elapsed();

  // Step 2: dequeue regions and give them color that none of their neighbors
  //  has
  // Color indices actually
  int* colorindices = new int[mRegions.count()];
  for(unsigned int i = 0; i < mRegions.count(); i++)
  {
    colorindices[i] = -1;  // -1 = "no color yet"
  }

  while(!queue.isEmpty())
  {
    BosonPathRegion* r = queue.dequeue();
    // For every color, check if any of r's neighbors already have this color
    // TODO: this can probably be done faster, e.g. by having bool map and if
    //  some neighbor has color x, then you set item x in bool map to false
    //  (not available). And then, at the end,  check which color is true
    //  (available)
    for(int i = 0; i < 6; i++)
    {
      bool colortaken = false;
      for(unsigned int j = 0; j < r->neighbors.count(); j++)
      {
        if(colorindices[r->neighbors[j].region->id] == i)
        {
          colortaken = true;
          break;
        }
      }
      if(!colortaken)
      {
        // Free color - use it
        colorindices[r->id] = i;
        // Take next region (break into while loop)
        break;
      }
    }
  }

  e2 = p.elapsed();

  // Step 3: every region should have color now. So we take every cell and tell
  //  colorMap which color it should have
  //boDebug(510) << k_funcinfo << "Starting step 3" << endl;
  BoColorMap* colormap = mMap->colorMap();
  //boDebug(510) << k_funcinfo << "colormap is at " << colormap << endl;
  for(unsigned int y = 0; y < mMap->height(); y++)
  {
    for(unsigned int x = 0; x < mMap->width(); x++)
    {
      //boDebug(510) << k_funcinfo << "Starting work for location at (" << x << "; " << y << ")" << endl;
      BosonPathRegion* r = cellRegion(x, y);
      //boDebug(510) << k_funcinfo << "Region for that cell is: " << r << endl;
      if(!r)
      {
        // Now what?
        // Make it black. Black is colors[6] atm (special one)
        //boDebug(510) << k_funcinfo << "NULL region, setting color to (" << (int)colors[6][0] << "; " << (int)colors[6][1] << "; " << (int)colors[6][2] << ")" << endl;
        colormap->setColorAt(x, y, colors[6]);
      }
      else
      {
        //boDebug(510) << k_funcinfo << "Region has id " << r->id << "; corresponding colorindex is " << colorindices[r->id] << endl;
        if(colorindices[r->id] < 0 || colorindices[r->id] > 5)
        {
          //boError(510) << k_funcinfo << "Region for cell at (" << x << "; " << y << ") with id " << r->id << " has wrong color " << colorindices[r->id] << endl;
        }
        //boDebug(510) << k_funcinfo << "setting color to (" << (int)colors[colorindices[r->id]][0] << "; " << (int)colors[colorindices[r->id]][1] << "; " << (int)colors[colorindices[r->id]][2] << ")" << endl;
        colormap->setColorAt(x, y, colors[colorindices[r->id]]);
      }
      //boDebug(510) << k_funcinfo << "DONE for that location" << endl;
    }
  }
  //boDebug(510) << k_funcinfo << "END of step 3" << endl;

  delete[] colorindices;

  e3 = p.stop();

//  boDebug(510) << k_funcinfo << "DONE; total elapsed: " << e3 / 1000.0 << " ms (p1: " << e1 / 1000.0 <<
//      " ms;  p2: " << (e2 - e1) / 1000.0 << " ms;  p3: " << (e3 - e2) / 1000.0 << " ms)" << endl;
}

void BosonPath2::removeHighLevelPath(BosonPathHighLevelPath* path)
{
//  boDebug(510) << k_funcinfo << endl;
  if(path->users > 0)
  {
    boError(510) << k_funcinfo << "Highlevel path still has " << path->users << " users!" << endl;
    return;
  }

  if(!mHLPathCache.removeRef(path))
  {
    boError(510) << k_funcinfo << "No such item in the list?" << endl;
  }
//  boDebug(510) << k_funcinfo << "END" << endl;
}

BosonPathHighLevelPath* BosonPath2::findCachedHighLevelPath(BosonPathInfo* info)
{
//  boDebug(510) << k_funcinfo << endl;
  QPtrListIterator<BosonPathHighLevelPath> it(mHLPathCache);
  while(it.current())
  {
    if(it.current()->valid && (it.current()->passability == info->passability) &&
        (it.current()->startRegion == info->startRegion) && (info->possibleDestRegions.contains(it.current()->destRegion)))
    {
      return it.current();
    }
    ++it;
  }

  // No cached path was found (no error msg, it's ok)
  return 0;
}

void BosonPath2::addCachedHighLevelPath(BosonPathHighLevelPath* path)
{
//  boDebug(510) << k_funcinfo << endl;
  mHLPathCache.append(path);
}

void BosonPath2::searchHighLevelPath(BosonPathInfo* info)
{
  boDebug(510) << k_funcinfo << endl;
  long int tm_initmaps, tm_initmisc, tm_mainloop, tm_copypath = 0, tm_viz = 0, tm_uninit;
  static int profilerId = -boProfiling->requestEventId("Stupid profiling name");
  BosonProfiler pr(profilerId);
  // List of open nodes
  BosonPathHeap<BosonPathHighLevelNode> open;

  // List of nodes that are visited and of those in open
  // This is used for performance reasons, it's faster than seacrhing open for
  //  every node.
//  boDebug(510) << k_funcinfo << "Creating visited and inopen bool maps for " << mRegions.count() << " active regions" << endl;
  bool* visited = new bool[mRegions.count()];
  bool* inopen = new bool[mRegions.count()];
  for(unsigned int i = 0; i < mRegions.count(); i++)
  {
    visited[i] = false;
    inopen[i] = false;
  }
  tm_initmaps = pr.elapsed();

  BosonPathHighLevelNode n, n2;
  // Add starting node to open
  n.region = info->startRegion;
  n.region->parent = 0;
  n.g = 0.0f;
  n.h = highLevelDistToGoal(n.region, info);
  visited[n.region->id] = true;
  inopen[n.region->id] = true;
//  boDebug(510) << "    " << k_funcinfo << "OPEN_ADD: " << "regid: " << n.region->id <<
//      "; g: " << n.g << "; h: " << n.h << ";  f: " << n.g + n.h << endl;
  open.add(n);

  // Is the path found?
  bool pathfound = false;

  // When range is 0 and we can't get exactly to destination region, we will go
  //  to nearest region
  BosonPathHighLevelNode nearest = n;
  tm_initmisc = pr.elapsed();

  // Main loop
  while(!open.isEmpty())
  {
    // Take first node from open
    open.takeFirst(n);
    inopen[n.region->id] = false;
//    boDebug(510) << "    " << k_funcinfo << "OPEN_TAKE: " << "regid: " << n.region->id <<
//        "; g: " << n.g << "; h: " << n.h << ";  f: " << n.g + n.h << ";  open.count(): " << open.count() << endl;

    // Check if it's the goal
    if(info->possibleDestRegions.contains(n.region))
    {
      // We've got our goal
//      boDebug(510) << "" << k_funcinfo << "goal found, braking" << endl;
      pathfound = true;
      break;
    }

    // Add all neighbors of the node to open
    for(unsigned int i = 0; i < n.region->neighbors.count(); i++)
    {
      n2.region = n.region->neighbors[i].region;

      // Check if this node is suitable for us
      if(n2.region->passabilityType != info->passability)
      {
        // Unit cannot move in this region
        visited[n2.region->id] = true;
        continue;
      }

      if(!visited[n2.region->id])
      {
        // Calculate costs
//        n2.g = n.g + highLevelCost(n2.region, info);
        n2.g = n.g + n2.region->cost + n.region->neighbors[i].cost;
        n2.h = highLevelDistToGoal(n2.region, info);
        n2.region->parent = n.region;

        // Check if n2 is nearest node so far
        if((n2.h + n2.g * TNG_NEAREST_G_FACTOR) < (nearest.h + nearest.g * TNG_NEAREST_G_FACTOR))
        {
          nearest = n2;
        }

        // Add node to open
//        boDebug(510) << "        " << k_funcinfo << "OPEN_ADD: " << "regid: " << n2.region->id <<
//            "; g: " << n2.g << "; h: " << n2.h << ";  f: " << n2.g + n2.h << endl;
        open.add(n2);
        visited[n2.region->id] = true;
        inopen[n2.region->id] = true;
      }
      else if(inopen[n2.region->id])
      {
        // Node is already in open - change cost
        QValueList<BosonPathHighLevelNode>::iterator it;
        // Find node in open
        for(it = open.begin(); it != open.end(); ++it)
        {
          if(n2.region == (*it).region)
          {
            break;
          }
        }
        if(it == open.end())
        {
          boError(510) << k_funcinfo << "No region with id " << n2.region->id << " found in OPEN!!!" << endl;
          continue;
        }

        // Calculate costs
//        n2.g = n.g + highLevelCost(n2.region, info);
        n2.g = n.g + n2.region->cost + n.region->neighbors[i].cost;
        if((*it).g < n2.g)
        {
          // Old path is better - leave it untouched
        }
        n2.h = highLevelDistToGoal(n2.region, info);

        // Check if n2 is nearest node so far
        if((n2.h + n2.g * TNG_NEAREST_G_FACTOR) < (nearest.h + nearest.g * TNG_NEAREST_G_FACTOR))
        {
          nearest = n2;
        }
        n2.region->parent = n.region;

//        boDebug(510) << "        " << k_funcinfo << "OPEN_REPLACE: " << "regid: " << n2.region->id <<
//            "; g-old: " << (*it).g << "; g: " << n2.g << "; h: " << n2.h << ";  f: " << n2.g + n2.h << endl;
        // Delete it from open
        open.remove(it);
        // And re-add
        open.add(n2);
      }
    }
  }

  tm_mainloop = pr.elapsed();
//  boDebug(510) << k_funcinfo << "path searching finished" << endl;

  // Traceback path
  if(!pathfound && info->range > 0)
  {
    boError(510) << k_funcinfo << "No path found!" << endl;
    info->passable = false;
    info->hlpath = 0;
  }
  else
  {
    if(!pathfound)
    {
      // We didn't find exact path to destination, so use nearest point instead
      n = nearest;
  //    boDebug(510) << k_funcinfo << "Using NEAREST node with region id " << n.region->id << "); g: " <<
  //        n.g << "; h: " << n.h << endl;
    }

    // First copy path to list. We use temporary list here because vector doesn't
    //  have prepend() and path's length isn't known
    // FIXME: we can get path length, e.g. by adding level var to nodes
    QPtrList<BosonPathRegion> temp;
    BosonPathRegion* r = n.region;
    temp.prepend(r);
    while(r != info->startRegion)
    {
      // Take next region
      r = r->parent;
      // We must prepend regions, not append them, because we go from destination
      //  to start here
      temp.prepend(r);
    }

    // Create new highlevel path
    BosonPathHighLevelPath* path = new BosonPathHighLevelPath;
    path->startRegion = info->startRegion;
    path->destRegion = n.region;
    path->valid = true;
    path->users = 1;
    // Copy temp path to real path vector
    path->path.resize(temp.count());
    QPtrListIterator<BosonPathRegion> it(temp);
    while(it.current())
    {
      path->path.insert(path->path.count(), it.current());
      ++it;
    }
  //  boDebug(510) << k_funcinfo << "found path has " << path->path.count() << " steps" << endl;

    // Add path to cache
    addCachedHighLevelPath(path);

    // Update info
    info->hlpath = path;
    info->hlstep = 0;
    info->passable = true;
    info->destRegion = n.region;
    tm_copypath = pr.elapsed();

#ifdef VISUALIZE_PATHS
    // Add LineVisualization stuff
    {
      QValueList<BoVector3> points;
      for(unsigned int point = 0; point < path->path.count(); point++)
      {
        float x = path->path[point]->centerx;
        float y = path->path[point]->centery;
        points.append(BoVector3(x, -y, 0.0f));
      }
      float pointSize = 5.0f;
      int timeout = 200;
      float zOffset = 0.5f;
      BosonPathVisualization::pathVisualization()->addLineVisualization(points, pointSize, timeout, zOffset);
    }
#endif
    tm_viz = pr.elapsed();
  }

  delete[] visited;
  delete[] inopen;
  tm_uninit = pr.stop();

  boDebug(510) << k_funcinfo << "END, Took " << tm_uninit << " usec:" << endl <<
      "    maps init: " << tm_initmaps << ";  misc init: " << tm_initmisc - tm_initmaps << endl <<
      "    main loop: " << tm_mainloop - tm_initmisc << endl <<
      "    path copy: " << tm_copypath - tm_mainloop << ";  viz: " << tm_viz - tm_copypath << ";  maps uninit: " << tm_uninit - tm_viz << endl;
}

void BosonPath2::findHighLevelGoal(BosonPathInfo* info)
{
  info->possibleDestRegions.clear();
  info->startRegion = cellRegion(info->start);
  if(!info->startRegion)
  {
    boDebug(510) << k_funcinfo << "No start region! Start point: (" << info->start.x() << "; " << info->start.y() <<
        "); in cell coords: (" << (info->start).x() << "; " << (info->start).y() << ")" << endl;
    return;
  }
//  info->destRegion = cellRegion(info->dest);
  static int profilerId = -boProfiling->requestEventId("Stupid profiling name");
  BosonProfiler pr(profilerId);
  BosonPathRegionGroup* startGrp = info->startRegion->group;

  int destx = (int)info->dest.x();
  int desty = (int)info->dest.y();

  // Check if going to goal is possible
  // Goal area
  int minx = QMAX(destx - info->range, 0);
  int miny = QMAX(desty - info->range, 0);
  int maxx = QMIN(destx + info->range, (int)mMap->width() - 1);
  int maxy = QMIN(desty + info->range, (int)mMap->height() - 1);

  // Go through all the cells in goal area and check if it's possible to get
  //  there
  bool connected = false;
  if(!info->flying && info->canMoveOnLand && info->canMoveOnWater)
  {
    // TODO: add support for such units
//    boDebug() << k_funcinfo << "Took " << pr.stop() << " usec" << endl;
    boError(510) << k_funcinfo << "Land units that can move on both water and land aren't supported yet!!!" << endl;
    return;
  }
  // Find unit's passability type.
  if(info->canMoveOnWater)
  {
    info->passability = Water;
  }
  else
  {
    info->passability = Land;
  }
  QPtrList<BosonPathRegion> destRegions;
  BosonPathRegion* r;
  for(int y = miny; y <= maxy; y++)
  {
    for(int x = minx; x <= maxx; x++)
    {
      r = cellRegion(x, y);
      if(!r)
      {
        // Occupied or unpassable cell
        continue;
      }
      if(r->passabilityType != info->passability)
      {
        // Wrong passability
        continue;
      }
      if(r->group == startGrp)
      {
        // If both regions are in same group, then they're connected
        connected = true;
        // Add r to list of dest regions
        if(!destRegions.containsRef(r))
        {
          destRegions.append(r);
        }
      }
    }
  }
  if(!connected)
  {
//    boDebug() << k_funcinfo << "Took " << pr.stop() << " usec" << endl;
    boDebug(510) << k_funcinfo << "No path to destination area found!" << endl;
    return;
  }
  info->possibleDestRegions.resize(destRegions.count());
  QPtrListIterator<BosonPathRegion> it(destRegions);
  while(it.current())
  {
    info->possibleDestRegions.insert(info->possibleDestRegions.count(), it.current());
    ++it;
  }
//  boDebug() << k_funcinfo << "Took " << pr.stop() << " usec" << endl;
  boDebug(510) << k_funcinfo << "Found " << info->possibleDestRegions.count() << " possible dest regions" << endl;
}

float BosonPath2::highLevelDistToGoal(BosonPathRegion* r, BosonPathInfo* info)
{
  // We use start and destination _points_, not _regions_, because points are
  //  more accurate, than region centers (we may start/end at the edge of the
  //  region, not at it's center)
  // If this is the start region, we use dist between start and end points
  if(r == info->startRegion)
  {
    return TNG_HIGH_DIST_MULTIPLIER * QMAX(QABS(info->start.x() - info->dest.x()), QABS(info->start.y() - info->dest.y()));
  }
  /*else if(r == info->destRegion)
  {
    // For dest region, dist is 0
    return 0.0f;
  }*/
  // Cost is bigger when point a is not near straight line between start and
  //  goal
  float dx1 = r->centerx - info->dest.x();
  float dy1 = r->centery - info->dest.y();
  float dx2 = info->start.x() - r->centerx;
  float dy2 = info->start.y() - r->centery;
  float cross = dx1 * dy2 - dx2 * dy1;
  /*double dx1 = ax - bx;
  double dy1 = ay - by;
  double dx2 = mStartx - bx;
  double dy2 = mStarty - by;
  float cross = (r->center.x - info->destRegion->center.x) * (info->startRegion->center.y - r->center.y) - dx2 * dy1;*/
  if(cross < 0)
  {
    cross = -cross;
  }

  return (cross / HIGH_CROSS_DIVIDER) + TNG_HIGH_DIST_MULTIPLIER * QMAX(QABS(dx1), QABS(dy1));
}

float BosonPath2::highLevelCost(BosonPathRegion* r, BosonPathInfo*)
{
  return r->cost;
}

float BosonPath2::lowLevelDistToGoal(int x, int y, BosonPathInfo* info)
{
  /*float dx1 = x - info->dest.x();
  float dy1 = y - info->dest.y();
  float dx2 = info->start.x() - x;
  float dy2 = info->start.y() - y;
  float cross = dx1 * dy2 - dx2 * dy1;*/
  int dx1 = x - (int)info->dest.x();
  int dy1 = y - (int)info->dest.y();
  int dx2 = (int)info->start.x() - x;
  int dy2 = (int)info->start.y() - y;
  int cross = dx1 * dy2 - dx2 * dy1;
  if(cross < 0)
  {
    cross = -cross;
  }

  return (cross / (float)LOW_CROSS_DIVIDER) + TNG_LOW_DIST_MULTIPLIER * QMAX(QABS(dx1), QABS(dy1));
}

float BosonPath2::lowLevelCost(int x, int y, BosonPathInfo* info)
{
  return TNG_LOW_BASE_COST + cell(x, y)->passageCostLand();
}

float BosonPath2::lowLevelCostAir(int x, int y, BosonPathInfo* info)
{
  return TNG_LOW_BASE_COST + cell(x, y)->passageCostAir();
}

void BosonPath2::neighbor(int& x, int& y, Direction d)
{
  if((d == NorthEast) || (d == North) || (d == NorthWest))
  {
    y--;
  }
  else if((d == SouthEast) || (d == South) || (d == SouthWest))
  {
    y++;
  }
  if((d == NorthWest) || (d == West) || (d == SouthWest))
  {
    x--;
  }
  else if((d == NorthEast) || (d == East) || (d == SouthEast))
  {
    x++;
  }
}

BosonPathSector* BosonPath2::sector(int x, int y)
{
  return &mSectors[y * ((mMap->width() / mSectorWidth) + 1) + x];
}

BosonPathRegion* BosonPath2::cellRegion(int x, int y)
{
  return mMap->cell(x, y)->region();
}

BosonPath2::PassabilityType BosonPath2::cellPassability(int x, int y)
{
  if(!cell(x, y)->passable())
  {
    return NotPassable;
  }
  else if(boWaterManager->cellPassable(x, y))
  {
    // Cell is passable by land units
    return Land;
  }
  else
  {
    return Water;
  }
}

bool BosonPath2::cellOccupied(int x, int y)
{
  return cell(x, y)->isLandOccupied();
}

float BosonPath2::cellCost(int x, int y)
{
  return cell(x, y)->passageCostLand();
}

Cell* BosonPath2::cell(int x, int y)
{
  return mMap->cell(x, y);
}

bool BosonPath2::isValidCell(int x, int y)
{
  return mMap->isValidCell(x, y);
}

int BosonPath2::addRegion(BosonPathRegion* r)
{
//  boDebug(510) << k_funcinfo << endl;
  mRegionIdUsed[mRegions.count()] = true;
  if(mRegions.size() <= mRegions.count())
  {
    // We need more space
//    boDebug(510) << k_funcinfo << "resizing regions vector from " << mRegions.size() << " to " << mRegions.size() + 20 << endl;
    mRegions.resize(mRegions.size() + 20);
  }
//  boDebug(510) << k_funcinfo << "inserting region " << r << " to " << mRegions.count() << endl;
  mRegions.insert(mRegions.count(), r);
//  boDebug(510) << k_funcinfo << "new region will get id " << mRegions.count() - 1 << endl;
  return mRegions.count() - 1;
}

void BosonPath2::removeRegion(BosonPathRegion* r)
{
//  boDebug(510) << k_funcinfo << endl;
  mRegions.take(r->id);  // We don't want to delete region, remove() would delete it
//  boDebug(510) << k_funcinfo << "removed region with id " << r->id << "; count is: " <<
//      mRegions.count() << "; next free id is: " << mRegions.count() << endl;
  if((mRegions.count() > 0) && (r->id < (int)mRegions.count()))
  {
    // Move last region to freed position. Note that last region is at count(),
    //  not at count() - 1, because there's now a null element in the middle of
    //  the vector
//    boDebug(510) << k_funcinfo << "Moving region " << mRegions.at(mRegions.count()) << " from " << mRegions.count() << " to " << r->id << endl;
    BosonPathRegion* r2 = mRegions.take(mRegions.count());
    mRegions.insert(r->id, r2);
    // Assign freed id to it
    r2->id = r->id;
    // FIXME: Equal ids are bad, I know, but r _should_ be deleted soon. Maybe
    //  assign id -1 to it?
  }
  // We have new free id
//  mRegions.count()--;
}



/*****  BosonPathInfo  *****/


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

void BosonPathVisualization::addLineVisualization(const QValueList<BoVector3>& points, const BoVector4& color, float pointSize, int timeout, float zOffset)
{
  emit signalAddLineVisualization(points, color, pointSize, timeout, zOffset);
}

void BosonPathVisualization::addLineVisualization(const QValueList<BoVector3>& points, float pointSize, int timeout, float zOffset)
{
  addLineVisualization(points, BoVector4(1.0f, 1.0f, 1.0f, 1.0f), pointSize, timeout, zOffset);
}
/*
 * vim: et sw=2
 */
