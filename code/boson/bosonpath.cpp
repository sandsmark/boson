/*
    This file is part of the Boson game
    Copyright (C) 2001 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include <qcanvas.h>
#include <qpoint.h>
#include <qdatetime.h> // only for debug

#ifdef USE_STL
 #include <vector.h>
#endif

#define ERROR_COST 100000
#define MAX_PATH_COST 5000
#define FOGGED_COST 2.5
#define SEARCH_STEPS 10  // How many steps of path to find

class BosonPath::Marking
{
  public:
    Marking() { dir = DirNone; f = -1; g = -1; level = -1; }
    Direction dir;
    float f;
    float g;
    short int level;
};

class BosonPath::PathNode
{
  public:
    PathNode() { x = 0; y = 0; g = 0; h = 0; level = -1; };
    int x; // x-coordinate of cell
    int y; // y-coordinate of cell

// AB: I don't fully understand these both (I HATE short names) - I hope this
// description is correct
    float g; // path cost of this node?? -> "cost" of the cells (ALL cells that the unit has crossed up to here)
    float h; // distance of all cells the unit has crossed up to here
    short int level; // node is level steps away from start. It's needed to search only 10 steps of path at once
};

/** Describes found path
  * Possible values are:
  * NoPath - no path was found
  * FullPath - full path to goal was found
  * PartialPath - SEARCH_STEPS steps of path was found and no further was searched
  * AbortedPath - pathfinding was aborted, but some steps were found
  */
enum PathStyle {
  NoPath = 0,
  FullPath = 1,
  PartialPath = 2,
  AbortedPath = 3
};

inline bool operator < (const BosonPath::PathNode& a, const BosonPath::PathNode& b)
{
  return (a.g + a.h) < (b.g + b.h);
}

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
  mModifier = 3;
  mCrossDivider = 10;
  mMinCost = 3;
  mAbortPath = 2000;

  kdDebug() << k_funcinfo << "start: " << mStartx << "," << mStarty << " goal: " << mGoalx << "," << mGoaly << " range: " << mRange << endl;
}

BosonPath::~BosonPath()
{
}

QValueList<QPoint> BosonPath::findPath(Unit* unit, int goalx, int goaly, int range)
{
  QValueList<QPoint> points;
  if (!unit)
  {
    kdError() << k_funcinfo << "NULL unit" << endl;
    return points;
  }
  QPoint p = unit->boundingRect().center();
  BosonPath path(unit, p.x() / BO_TILE_SIZE, p.y() / BO_TILE_SIZE,
        goalx / BO_TILE_SIZE, goaly / BO_TILE_SIZE, range);
  if (!path.findPath())
  {
    kdWarning() << "no path found" << endl;
  }
  points = path.path; // faster than manually coping all points
  return points;
}

bool BosonPath::findPath()
{
  QTime time;
  time.start();
  mNodesRemoved = 0;
  mPathLength = 0;
  mPathCost = 0;
  PathStyle pathfound = NoPath;
  Marking mark[mUnit->canvas()->width() / BO_TILE_SIZE][mUnit->canvas()->height() / BO_TILE_SIZE];
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

  // Real cost will be 0 (we haven't moved yet)
  node.g = 0;
  // Calculate heuristic (distance) cost
  node.h = dist(mStartx, mStarty, mGoalx, mGoaly);

  // add node to OPEN
  open.push_back(node);

  // mark values on 'virtual map'
  mark[node.x][node.y].f = node.g + node.h;
  mark[node.x][node.y].g = node.g;
  mark[node.x][node.y].level = 0; // same as node.level
  
  // Create second node
  PathNode n2;


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
      mark[node.x][node.y].f = -1;
      mNodesRemoved++;
    }

    // Break if SEARCH_STEPS steps of path is found
    if(node.level >= SEARCH_STEPS)
    {
      mGoalx = node.x;
      mGoaly = node.y;
      pathfound = PartialPath;
      break;
    }
    
    // Check if we've gone too long with searching
    if(mNodesRemoved >= mAbortPath)
    {
      kdDebug() << k_funcinfo << "mNodesRemoved >= mAbortPath" << endl;
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
        //kdWarning() << k_lineinfo << ": not on canvas" << endl;
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
          // change our goal
          mGoalx = node.x;
          mGoaly = node.y;
          kdDebug() << "oops - cannot go on target cell :-(" << endl;
          kdDebug() << "new goal x=" << mGoalx << ",y=" << mGoaly << endl;
          continue;
        }
        //kdDebug() << k_lineinfo << "ERROR_COST" << endl;
        continue;
      }
      else // we can go on this cell
      {
        n2.g = node.g + nodecost;
      }

      n2.h = dist(n2.x, n2.y, mGoalx, mGoaly);
      
      // if g == -1 then it isn't visited yet
      if(mark[n2.x][n2.y].g == -1)
      {
        // First, mark the spot
        // direction of Marking always points to _previous_ element in path
        mark[n2.x][n2.y].dir = reverseDir(d);
        // Store costs
        mark[n2.x][n2.y].f = n2.g + n2.h;
        mark[n2.x][n2.y].g = n2.g;
        mark[n2.x][n2.y].level = n2.level;
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
        if(mark[n2.x][n2.y].f != -1)
        {
          // It's in OPEN
          if(n2.g < mark[n2.x][n2.y].g)
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
              kdError() << "find != open.end()" << endl;
              break; // or what?
            }
            // Mark new direction from this node to previous one
            mark[n2.x][n2.y].dir = reverseDir(d);
            // Then modify costs and level of spot
            mark[n2.x][n2.y].g = n2.g;
            mark[n2.x][n2.y].f = n2.g + n2.h;
            mark[n2.x][n2.y].level = n2.level;
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
    // the directions pointing to the cells are in mark[x1][y1] -> x1,y1 starts
    // at x,y (aka mGoalx,mGoaly) nad go to mStartx,mStarty
    while(((x != mStartx) || (y != mStarty)) && counter < 100)
    {
      counter++;
      // Add waypoint
      temp.push_back(wp);
      mPathLength++;
      d = mark[x][y].dir; // the direction to the next cell
      neighbor(x, y, d);
      wp.setX(x * BO_TILE_SIZE + BO_TILE_SIZE / 2);
      wp.setY(y * BO_TILE_SIZE + BO_TILE_SIZE / 2);
    }
    if (counter >= 100) 
    {
      kdWarning() << k_lineinfo << "oops - counter >= 100" << endl;
    }

    // Write normal-ordered path to path
    for(int i = temp.size() - 1; i >= 0; --i)
    {
      wp.setX(temp[i].x());
      wp.setY(temp[i].y());
      path.push_back(wp);
    }

    // If no full path was found, then we add another point with coordinates
    //  -2; -2 to the path, indicating that this is just partial path.
    if(pathfound != FullPath)
      path.push_back(QPoint(-2, -2));
  }
  else
  {
    kdDebug() << k_funcinfo << "path not found" << endl;
    kdDebug() << "node.x=" << node.x << ",goalx=" << mGoalx << endl;
    kdDebug() << "node.y=" << node.y << ",goaly=" << mGoaly << endl;
    kdDebug() << "node.g=" << node.g << ",MAX_PATH_COST=" << MAX_PATH_COST << endl;
    // Path wasn't found
    // If path wasn't found we add one point with coordinates -1; -1 to path.
    //  In Unit::advanceMove(), there is check for this and if coordinates are
    //  those, then moving is stopped
    path.push_back(QPoint(-1, -1));
  }

  kdDebug() << "BosonPath::findPath() : path found, took " << time.elapsed() << " ms" << endl;
  return (pathfound != NoPath);
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

inline void BosonPath::neighbor(int& x, int& y, Direction d)
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
  // Check at the very beginning if tile is fogged - if it is, we return one value and save time
  if(mUnit->owner()->isFogged(x, y))
  {
    //kdDebug() << "Tile at (" << x << ", " << y << ") is fogged, returning FOGGED_COST" << endl;
    return FOGGED_COST + mMinCost;
  }

  Cell* c = mUnit->boCanvas()->cell(x, y);
  if (!c) {
    kdError() << k_funcinfo << "NULL cell" << endl;
    return ERROR_COST;
  }

  // Check if we can go to that tile, if we can't, return ERROR_COST
  if(! c->canGo(mUnit->unitProperties()))
  {
    //kdDebug() << k_lineinfo << ": cannot go on " << x << "," << y << endl;
    return ERROR_COST;
  }

  if(c->isOccupied(mUnit, false)) 
  {
    return ERROR_COST;
  }
  float cost = c->moveCost();

  return cost + mMinCost;
}

#ifdef USE_STL
inline void BosonPath::getFirst(vector<PathNode>& v, PathNode& n)
#else
inline void BosonPath::getFirst(QValueList<PathNode>& v, PathNode& n)
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

inline Direction BosonPath::reverseDir(Direction d)
{
  return (Direction)(((int)d + 4) % 8);
}

void BosonPath::debug() const
{
 kdDebug() << k_funcinfo << endl;
 if (!mUnit) {
	kdError() << "NULL unit" << endl;
	return;
 }
 kdDebug() << "unit: " << mUnit->id() << endl;
 kdDebug() << "startx,starty = " << mStartx << "," << mStarty << endl;
 kdDebug() << "goalx,goaly = " << mGoalx << "," << mGoaly << endl;
 kdDebug() << "waypoints: " << path.size() << endl;
 int j = 0;
 for(QValueList<QPoint>::const_iterator i = path.begin(); i != path.end(); ++i, j++) {
	kdDebug() << "waypoint " << j << ":" << endl;
	kdDebug() << "x,y=" << (*i).x() << "," << (*i).y() << endl;
 }
 kdDebug() << k_funcinfo << "(end)" << endl;
}

inline bool BosonPath:: inRange(int x, int y)
{
  /// TODO: maybe use different check (not manhattan dist.)
  if(QABS(x - mGoalx) > mRange || QABS(y - mGoaly) > mRange)
    return false;
  return true;
}
