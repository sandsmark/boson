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

#include <qcanvas.h>
#include <qpoint.h>

#define ERROR_COST 100000
#define MAX_PATH_COST 5000

class Marking
{
  public:
    Marking() { dir = DirNone; f = -1; g = -1; };
    Direction dir;
    float f;
    float g;
};

class Node
{
  public:
    Node() { x = 0; y = 0; g = 0; h = 0; };
    int x;
    int y;
    float g;
    float h;
};

bool operator < (const Node& a, const Node& b)
{
  return (a.g + a.h) < (b.g + b.h);
}


BosonPath::BosonPath(Unit* unit, int startx, int starty, int goalx, int goaly)
{
  mUnit = unit;
  mStartx = startx;
  mStarty = starty;
  mGoalx = goalx;
  mGoaly = goaly;
  mModifier = 5;
  mCrossDivider = 1000;
  mMinCost = 5;
  mAbortPath = 2000;

  kdDebug() << k_funcinfo << "start: " << mStartx << "," << mStarty << " goal: " << mGoalx << "," << mGoaly << endl;
}

BosonPath::~BosonPath()
{
}

QValueList<QPoint> BosonPath::findPath(Unit* unit, int goalx, int goaly)
{
  QValueList<QPoint> points;
  if (!unit) 
  {
    kdError() << k_funcinfo << "NULL unit" << endl;
    return points;
  }
  QPoint p = unit->boundingRect().center();
  BosonPath path(unit, p.x() / BO_TILE_SIZE, p.y() / BO_TILE_SIZE, 
        goalx / BO_TILE_SIZE, goaly / BO_TILE_SIZE);
  if (!path.findPath())
  {
    kdWarning() << "no path found" << endl;
    return points;
  }
  for (vector<QPoint>::iterator it = path.path.begin(); it != path.path.end(); ++it)
  {
    points.append(*it);
  }
  return points;
}

bool BosonPath::findPath()
{
  mNodesRemoved = 0;
  mPathLength = 0;
  mPathCost = 0;
  bool pathfound = false;
  Marking mark[mUnit->canvas()->width() / BO_TILE_SIZE][mUnit->canvas()->height() / BO_TILE_SIZE];
  vector<Node> open;

  // Create first (main) node
  Node node;

  // It will be at start
  node.x = mStartx;
  node.y = mStarty;

  // Real cost will be 0 (we haven't moved yet)
  node.g = 0;
  // Calculate heuristic (distance) cost
  node.h = dist(mStartx, mStarty, mGoalx, mGoaly);

  // add node to OPEN
  open.push_back(node);

  // mark values on 'virtual map'
  mark[node.x][node.y].f = node.g + node.h;
  mark[node.x][node.y].g = node.g;
  
  // Create second node
  Node n2;

  // Main loop
  while(! open.empty())
  {
    if (node.x != mGoalx || node.y != mGoaly)
    { // this is usually the case - except if we cannot go on the intended goal
      getFirst(open, node);
      // if f < 0 then it's not in OPEN
      mark[node.x][node.y].f = -1;
      mNodesRemoved++;
    }


    // First check if we're at goal already
    if((node.x == mGoalx) && (node.y == mGoaly))
    {
      kdDebug() << k_lineinfo << "path found :-)" << endl;
      pathfound = true;
      break;
    }
    
    kdDebug() << k_lineinfo << "removed nodes: " << mNodesRemoved << endl;

    // Check if we've gone too long with searching
    if(mNodesRemoved >= mAbortPath)
    {
      kdDebug() << k_funcinfo << "mNodesRemoved >= mAbortPath" << endl;
      // Pick best node from OPEN
      for(vector<Node>::iterator i = open.begin(); i != open.end(); ++i)
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
      pathfound = false;
      kdDebug() << k_lineinfo << "path NOT found" << endl;
      break;
    }

    for(int dir = 0; dir < 8; dir++)
    {
      Direction d = (Direction)dir;
      // First, set new node's position to be old's one
      n2.x = node.x;
      n2.y = node.y;
//      kdDebug() << k_lineinfo << n2.x << " " << n2.y << endl;
      // then call method to modify position accordingly to direction
      neighbor(n2.x, n2.y, d);
//      kdDebug() << k_lineinfo << "neighbor: " << n2.x << " " << n2.y << endl;

      // Make sure that position is valid
      if(! mUnit->canvas()->onCanvas(n2.x * BO_TILE_SIZE, n2.y * BO_TILE_SIZE))
      {
        kdWarning() << k_lineinfo << ": not on canvas" << endl;
        continue;
      }

      // Calculate costs of node
      float nodecost = cost(n2.x, n2.y);
      // If cost is ERROR_COST, then we can't go there
      if(nodecost == ERROR_COST)
      {
        if(n2.x == mGoalx && n2.y == mGoaly)
        {
          // change our goal
          mGoalx = node.x;
          mGoaly = node.y;
          kdDebug() << "oops - cannot go on target cell :-(" << endl;
          kdDebug() << "new goal x=" << mGoalx << ",y=" << mGoaly << endl;
	  continue;
	}
        kdDebug() << k_lineinfo << "ERROR_COST" << endl;
        continue;
      }
      else
      {
//        kdDebug() << k_lineinfo << "can go on cell" << endl;
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
        // Push node to OPEN
        open.push_back(n2);
        push_heap(open.begin(), open.end(), comp);
      }
      else
      {
        // Node is in OPEN or CLOSED
        if(mark[n2.x][n2.y].f != -1)
        {
          // It's in OPEN
          if(n2.g < mark[n2.x][n2.y].g)
          {
            // Our current node has lower cost than the one, that was here, so
            //  we modify the path
            // First, find this node in OPEN
            vector<Node>::iterator find = open.end();
            for(find = open.begin(); find != open.end(); ++find)
            {
              if(((*find).x == n2.x) && ((*find).y == n2.y))
              {
                break;
              }
            }
            assert(find != open.end());
            // Mark new direction from this node to previous one
            mark[n2.x][n2.y].dir = reverseDir(d);
            // Then modify costs of spot
            mark[n2.x][n2.y].g = n2.g;
            mark[n2.x][n2.y].f = n2.g + n2.h;
            // Replace cost of node that was in OPEN
            (*find).g = n2.g;
            push_heap(open.begin(), find + 1, comp);
          }
        }
      }
    }
  }
  // Pathfinding finished, but was the path found
  if((node.x == mGoalx) && (node.y == mGoaly) && (node.g < MAX_PATH_COST))
  {
    // Something was
    // Path cost is equal to cost of last node
    mPathCost = node.g;
    // Temporary array - needed because path is first stored from goal to start
    vector<QPoint> temp;
    
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
//    kdDebug() << k_lineinfo << " before loop: x:" << x << "; y:" << y << endl;
    while(((x != mStartx) || (y != mStarty)) && counter < 100)
    {
      counter++;
      // Add waypoint
      temp.push_back(wp);
      mPathLength++;
      d = mark[x][y].dir;
//      kdDebug() << k_lineinfo << " loop: x:" << x << "; y:" << y << "; d:" << (int)d << endl;
      neighbor(x, y, d);
      wp.setX(x * BO_TILE_SIZE + BO_TILE_SIZE / 2);
      wp.setY(y * BO_TILE_SIZE + BO_TILE_SIZE / 2);
//      kdDebug() << k_lineinfo << " loop: x:" << x << "; y:" << y << "; d:" << (int)d << endl;
    }
    if (counter >= 100) 
    {
      kdError() << k_lineinfo << "oops - counter >= 100" << endl;
    }
//    kdDebug() << "loop done" << endl;

    // Write normal-ordered path to path
    for(int i = temp.size() - 1; i >= 0; --i)
    {
//      kdDebug() << "add path" << endl;
      wp.setX(temp[i].x());
      wp.setY(temp[i].y());
      path.push_back(wp);
    }
  }
  else
  {
    kdDebug() << k_funcinfo << "path not found" << endl;
    kdDebug() << "node.x=" << node.x << ",goalx=" << mGoalx << endl;
    kdDebug() << "node.y=" << node.y << ",goaly=" << mGoaly << endl;
    kdDebug() << "node.g=" << node.g << ",MAX_PATH_COST=" << MAX_PATH_COST << endl;
    // Path wasn't found
  }

//  kdDebug() << k_funcinfo << "end" << endl;
//  debug();
  return pathfound;
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
  dist += mModifier * max(abs(ax - bx), abs(ay - by));
  return dist;
}

inline void BosonPath::neighbor(int& x, int& y, Direction d)
{
  if((d == DirNE) || (d == DirNorth) || (d == DirNW))
  {
    y--;
  }
  else if((d == DirSE) || (d == DirSouth) || (d == DirSW))
  {
    y++;
  }
  if((d == DirNW) || (d == DirWest) || (d == DirSW))
  {
    x--;
  }
  else if((d == DirNE) || (d == DirEast) || (d == DirSE))
  {
    x++;
  }
}

float BosonPath::cost(int x, int y)
{
  Cell* c = mUnit->boCanvas()->cell(x, y);
  // Check if we can go to that tile, if we can't, return -1
  if(! c->canGo(mUnit->unitProperties()))
  {
    kdDebug() << k_lineinfo << ": cannot go on " << x << "," << y << endl;
    return ERROR_COST;
  }

  // Check if there are units on that tile (slow method?)
  QCanvasItemList list = mUnit->canvas()->collisions(QRect(x * BO_TILE_SIZE,
      y * BO_TILE_SIZE, BO_TILE_SIZE, BO_TILE_SIZE));
  if(! list.isEmpty())
  {
    bool flying = mUnit->isFlying();
    for(QCanvasItemList::Iterator it = list.begin(); it != list.end(); ++it)
    {
      if(RTTI::isUnit((*it)->rtti()))
      {
        Unit* unit = (Unit*)*it;
        if(unit != mUnit && unit->isFlying() == flying && !unit->isDestroyed())
        {
          kdDebug() << k_lineinfo << ": unit on cell " << x << "," << y << ": " << unit->id() << endl;
          return ERROR_COST;
        }
      }
    }
  }

  float cost = c->moveCost();
  return cost + mMinCost;
}

inline void BosonPath::getFirst(vector<Node>& v, Node& n)
{
  n = v.front();
  pop_heap(v.begin(), v.end(), comp);
  v.pop_back();
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
 for(vector<QPoint>::const_iterator i = path.begin(); i != path.end(); ++i, j++) { // FIXME should be const
	kdDebug() << "waypoint " << j << ":" << endl;
	kdDebug() << "x,y=" << (*i).x() << "," << (*i).y() << endl;
 }
 kdDebug() << k_funcinfo << "(end)" << endl;
}

