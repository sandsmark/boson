/*
    This file is part of the Boson game
    Copyright (C) 2001-2002 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "cell.h"
#include "unit.h"
#include "bosoncanvas.h"
#include "defines.h"
#include "player.h"
#include "bodebug.h"
#include "unitproperties.h"

#include <qpoint.h>
#include <sys/time.h> // only for debug

#define ERROR_COST 100000
#define MAX_PATH_COST 5000
#define FOGGED_COST 3

#define marking(x, y) mark[x - mStartx + SEARCH_STEPS][y - mStarty + SEARCH_STEPS]

// If you uncomment next line, you will enable in-line moving style.
//  With this moving style, unit movement is less agressive and units
//  wait until they can move, they don't search path around other units.
//  This is useful when moving big group of units through narrow places
//  on map, but in general, default style is better IMO
#define MOVE_IN_LINE

class BosonPath::PathNode
{
  public:
    PathNode() { x = 0; y = 0; g = 0; h = 0; level = -1; };
    inline void operator=(const BosonPath::PathNode& a);
    int x; // x-coordinate of cell
    int y; // y-coordinate of cell

    float g; // real cost - cost of all nodes up to this one (If this node is at goal, it's cost of full path)
    float h; // heuristic cost - distance between this node and goal
    short int level; // node is level steps away from start. It's needed to search only 10 steps of path at once
};

inline void BosonPath::PathNode::operator=(const BosonPath::PathNode& a)
{
  x = a.x;
  y = a.y;
  g = a.g;
  h = a.h;
  level = a.level;
}

// AB: that const once was added for g++-3.x -> i am sure it is used for nothing
// - but please test before removing... sometimes gcc is *really* strange...
inline const bool operator<(const BosonPath::PathNode& a, const BosonPath::PathNode& b)
{
  return (a.g + a.h) < (b.g + b.h);
}
inline const bool operator>(const BosonPath::PathNode& a, const BosonPath::PathNode& b)
{
  return (a.g + a.h) > (b.g + b.h);
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

/*
* !!! Please do not make big changes to code in this class unless you really now
*    what you are doing and if you make any bigger changes, send note to author!
*/

BosonPath::BosonPath(Unit* unit, int startx, int starty, int goalx, int goaly, int range)
{
  mUnit = unit;
  mStartx = startx;
  mStarty = starty;
  mGoalx = goalx;
  mGoaly = goaly;
  mRange = range;
  /// TODO: those variables needs tuning and *lots* of testing!
  mModifier = 6;
  mCrossDivider = 100;
  mMinCost = 3;
  mAbortPath = (SEARCH_STEPS * 2 + 1) * (SEARCH_STEPS * 2 + 1);

  //boDebug(500) << k_funcinfo << "start: " << mStartx << "," << mStarty << " goal: " << mGoalx << "," << mGoaly << " range: " << mRange << endl;
}

BosonPath::~BosonPath()
{
}

QValueList<QPoint> BosonPath::findPath(Unit* unit, int goalx, int goaly, int range)
{
  QValueList<QPoint> points;
  if (!unit)
  {
    boError(500) << k_funcinfo << "NULL unit" << endl;
    return points;
  }
  QPoint p = unit->boundingRect().center();
  BosonPath path(unit, p.x() / BO_TILE_SIZE, p.y() / BO_TILE_SIZE,
        goalx / BO_TILE_SIZE, goaly / BO_TILE_SIZE, range);
  if (!path.findPath())
  {
    boWarning(500) << "no path found" << endl;
  }
  points = path.path; // faster than manually coping all points
  return points;
}

int pathSlow = 0, pathRange = 0, pathFast = 0;

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
  struct timeval time1, time2;
  gettimeofday(&time1, 0);
  if(findFastPath())
  {
    gettimeofday(&time2, 0);
    boDebug(500) << k_funcinfo << "TOTAL TIME ELAPSED (fast method): " << time2.tv_usec - time1.tv_usec << "microsec." << endl;
    pathFast++;
    return true;
  }
  else
  {
    if(rangeCheck())
    {
      gettimeofday(&time2, 0);
      boDebug(500) << k_funcinfo << "TOTAL TIME ELAPSED (range method): " << time2.tv_usec - time1.tv_usec << "microsec." << endl;
      pathRange++;
      return false;
    }
    bool a = findSlowPath();
    gettimeofday(&time2, 0);
    boDebug(500) << k_funcinfo << "TOTAL TIME ELAPSED (slow method): " << time2.tv_usec - time1.tv_usec << "microsec." << endl;
    pathSlow++;
    return a;
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
  int length = QMAX(4, mUnit->unitProperties()->sightRange());

  lastx = mStartx;
  lasty = mStarty;

  // FIXME: it'd be best if we'd search unit->sightRange() steps here, but we
  //  don't have unit available here currently and I'm too lazy to change it
  //  right now...
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
    if(cost(lastx, lasty) == ERROR_COST)
    {
      // Path can't be found using fast method
      //gettimeofday(&time2, 0);
      //boDebug(500) << k_funcinfo << "Can't find path using fast method. Time elapsed: "
      //    << time2.tv_usec - time1.tv_usec << " microsec." << endl;
      return false;
    }
  }

  // Compose path
  QPoint wp;
  int i;
  for(i = 0; i < steps; i++)
  {
    wp.setX(x[i] * BO_TILE_SIZE + BO_TILE_SIZE / 2);
    wp.setY(y[i] * BO_TILE_SIZE + BO_TILE_SIZE / 2);
    path.push_back(wp);
  }

  i = steps - 1;
  if((x[i] != mGoalx) || (y[i] != mGoaly))
  {
    // Only partial path was found
    wp.setX(-2);
    wp.setY(-2);
    path.push_back(wp);
  } else {
    path.push_back(QPoint(-1, -1));  // This means that end of path has been reached
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
#ifdef USE_STL
  vector<PathNode> open;
#else
  QValueList<PathNode> open;
#endif

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
  open.push_back(node);

  // mark values on 'virtual map'
  marking(node.x, node.y).f = node.g + node.h;
  marking(node.x, node.y).g = node.g;
  marking(node.x, node.y).level = 0; // same as node.level
  
  // Create second node
  PathNode n2;

  bool goalUnReachable = false;
  nearest = node;

  // Main loop
  while(! open.empty())
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
    if(mNodesRemoved >= mAbortPath)
    {
      boDebug(500) << k_funcinfo << "mNodesRemoved >= mAbortPath" << endl;
      // Pick best node from OPEN
#ifdef USE_STL
      for(vector<PathNode>::iterator i = open.begin(); i != open.end(); ++i)
#else
      for(QValueList<PathNode>::iterator i = open.begin(); i != open.end(); ++i)
#endif
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
      if(! mUnit->canvas()->onCanvas(n2.x * BO_TILE_SIZE, n2.y * BO_TILE_SIZE))
      {
        //boWarning() << k_lineinfo << "not on canvas" << endl;
        continue;
      }

      // Calculate costs of node
      float nodecost = cost(n2.x, n2.y);
      // If cost is ERROR_COST, then we can't go there
      if(nodecost == ERROR_COST)
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
        // Push node to OPEN
        open.push_back(n2);
#ifdef USE_STL
        push_heap(open.begin(), open.end(), comp);
#else
        qHeapSort(open);
#endif
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
#ifdef USE_STL
            vector<PathNode>::iterator find = open.begin();
#else
            QValueList<PathNode>::iterator find = open.begin();
#endif
            for(; find != open.end(); ++find)
            {
              if(((*find).x == n2.x) && ((*find).y == n2.y))
              {
                break;
              }
            }
            if (find == open.end()) 
            {
              boError(500) << "find != open.end()" << endl;
              break; // or what?
            }
            // Mark new direction from this node to previous one
            marking(n2.x, n2.y).dir = reverseDir(d);
            // Then modify costs and level of spot
            marking(n2.x, n2.y).g = n2.g;
            marking(n2.x, n2.y).f = n2.g + n2.h;
            marking(n2.x, n2.y).level = n2.level;
            // Replace cost and level of node that was in OPEN
            (*find).g = n2.g;
            (*find).level = n2.level;
#ifdef USE_STL
            push_heap(open.begin(), find + 1, comp);
#else
            qHeapSort(open.begin(), ++find);
#endif
          }
        }
      }
    }
  }



  // Pathfinding finished, but was the path found
  // We now check value of pathfound to see if path was found
//  if((node.x == mGoalx) && (node.y == mGoaly) && (node.g < MAX_PATH_COST))
  if(pathfound != NoPath)
  {
    // Something was
    // Path cost is equal to cost of last node
    mPathCost = node.g;
    // Temporary array - needed because path is first stored from goal to start
    QValueList<QPoint> temp;
    
    // Construct waypoint and set it's pos to goal
    QPoint wp;
    int x, y;
    x = mGoalx;
    y = mGoaly;
    wp.setX(x * BO_TILE_SIZE + BO_TILE_SIZE / 2);
    wp.setY(y * BO_TILE_SIZE + BO_TILE_SIZE / 2);

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
      temp.push_back(wp);
      mPathLength++;
      d = marking(x, y).dir; // the direction to the next cell
      neighbor(x, y, d);
//      wp.setX(x * BO_TILE_SIZE + BO_TILE_SIZE / 2);
//      wp.setY(y * BO_TILE_SIZE + BO_TILE_SIZE / 2);
      wp.setX(x * BO_TILE_SIZE + mUnit->width() / 2);
      wp.setY(y * BO_TILE_SIZE + mUnit->height() / 2);
    }
    if (counter >= 100) 
    {
      boWarning(500) << k_lineinfo << "oops - counter >= 100" << endl;
    }

    // Write normal-ordered path to path
    // We first add waypoint with coordinates of center of tile unit currently
    //  is on. This helps to get rid of some collisions.
    //wp.setX(mStartx * BO_TILE_SIZE + BO_TILE_SIZE / 2);
    //wp.setY(mStarty * BO_TILE_SIZE + BO_TILE_SIZE / 2);
    //path.push_back(wp);
    for(int i = temp.size() - 1; i >= 0; --i)
    {
      wp.setX(temp[i].x());
      wp.setY(temp[i].y());
      path.push_back(wp);
    }

    // If no full path was found, then we add another point with coordinates
    //  -2; -2 to the path, indicating that this is just partial path.
    if(pathfound != FullPath && pathfound != AlternatePath)
    {
      path.push_back(QPoint(-2, -2));
    }
    else
    {
      // Point with coordinates -1; -1 means that end of the path has been
      //  reached and unit should stop
      path.push_back(QPoint(-1, -1));
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
    // If path wasn't found we add one point with coordinates -1; -1 to path.
    //  In Unit::advanceMove(), there is check for this and if coordinates are
    //  those, then moving is stopped
    path.push_back(QPoint(-1, -1));
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
    if(cost(mGoalx, mGoaly) != ERROR_COST)
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
      if(cost(x, mGoaly - range) != ERROR_COST)
      {
        mRange = QMAX(mRange, range);
        return false;
      }
      if(cost(x, mGoaly + range) != ERROR_COST)
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
      if(cost(mGoalx - range, y) != ERROR_COST)
      {
        mRange = QMAX(mRange, range);
        return false;
      }
      if(cost(mGoalx + range, y) != ERROR_COST)
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
  
  float dist = float(cross / mCrossDivider);
  dist += mModifier * QMAX(QABS(ax - bx), QABS(ay - by));
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
    co = FOGGED_COST + mMinCost;
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
      if(! c->canGo(mUnit->unitProperties()))
      {
        //boDebug() << k_lineinfo << "cannot go on " << x << "," << y << endl;
        co = ERROR_COST;
      }
      else
      {
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
        if(c->isOccupied(mUnit, true))
#endif
        {
          co = ERROR_COST;
        }
        else
        {
          co = c->moveCost() + mMinCost;
        }
      }
    }
  }
  marking(x, y).c = co;
  return co;
}

#ifdef USE_STL
void BosonPath::getFirst(vector<PathNode>& v, PathNode& n)
#else
void BosonPath::getFirst(QValueList<PathNode>& v, PathNode& n)
#endif
{
  n = v.front();
#ifdef USE_STL
  pop_heap(v.begin(), v.end(), comp);
  v.pop_back();
#else
  qHeapSort(v);
  v.pop_front();
#endif
}

Direction BosonPath::reverseDir(Direction d)
{
  return (Direction)(((int)d + 4) % 8);
}

void BosonPath::debug() const
{
 boDebug() << k_funcinfo << endl;
 if (!mUnit) {
	boError(500) << "NULL unit" << endl;
	return;
 }
 boDebug(500) << "unit: " << mUnit->id() << endl;
 boDebug(500) << "startx,starty = " << mStartx << "," << mStarty << endl;
 boDebug(500) << "goalx,goaly = " << mGoalx << "," << mGoaly << endl;
 boDebug(500) << "waypoints: " << path.size() << endl;
 int j = 0;
 for(QValueList<QPoint>::const_iterator i = path.begin(); i != path.end(); ++i, j++) {
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

/*
 * vim: et sw=2
 */
