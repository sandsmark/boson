/*
    This file is part of the Boson game
    Copyright (C) 2001 The Boson Team (boson-devel@lists.sourceforge.net)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef __BOSONPATH_H__
#define __BOSONPATH_H__

#include <qdatastream.h>

#include <vector.h>
#include <stl_heap.h>

#include <qvaluelist.h>

class Unit;
class Node;
class QPoint;

/**
 * Marks direction
 */
enum Direction 
{ 
  DirNorth = 0,
  DirNE = 1,
  DirEast = 2,
  DirSE = 3,
  DirSouth = 4,
  DirSW = 5,
  DirWest = 6,
  DirNW = 7,
  DirNone = 100 
};


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
    BosonPath(Unit* unit, int startx, int starty, int goalx, int goaly);
    ~BosonPath();
    
    /**
     * Finds path. Path is stored in vector @ref path
     * Note that if path wasn't found goal may be set to last waypoint
     * @return TRUE if path was found, FALSE otherwise
     */
    bool findPath();

    /**
     * @param unit The unit that will be moved
     * @param goalx the x <em>coordinate</em> of the goal. Not the cell!
     * @param goaly the y <em>coordinate</em> of the goal. Not the cell!
     **/
    static QValueList<QPoint> findPath(Unit* unit, int goalx, int goaly);
    
    /**
     * In this vector are waypoints of path
     */
    vector<QPoint> path;
    
    /**
     * Returns lenght of path (in tiles)
     */
    int pathLength() const { return mPathLength; };

    /**
     * Returns cost of path
     */
    float pathCost() const { return mPathCost; };

  private:
    float dist(int ax, int ay, int bx, int by);
    float cost(int x, int y);
    inline void getFirst(vector<Node>&, Node& n);
    inline void neighbor(int& x, int& y, Direction d);
    inline Direction reverseDir(Direction d);

    void debug() const;
  
  private:
    int mStartx;
    int mStarty;
    int mGoalx;
    int mGoaly;

    Unit* mUnit;
    greater<Node> comp;
    
    float mModifier;
    float mCrossDivider;
    float mMinCost;
    int mAbortPath;

    int mNodesRemoved;
    int mPathLength;
    float mPathCost;
};


#endif // __BOSONPATH_H__
