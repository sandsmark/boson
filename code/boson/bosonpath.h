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
#ifndef BOSONPATH_H
#define BOSONPATH_H

#include "global.h"

#include <qvaluelist.h>

// Defines whether to use STL (Standard Template Library) or QTL (Qt Template Library)
// If there is no STL implementation for your compiler, you can use QTL, but I
//  recommend using STL as it is little bit faster (should be at least)
// AB: I recommend QTL, since there are *many* compiler problems less! speedup
// with STL is extremely low and QTL is way more stable, concerning compilation
// (and easier to use).
#include <config.h>
#if defined(HAVE_HP_STL) || defined(HAVE_SGI_STL)
 #define USE_STL 1
#endif

#ifdef USE_STL
 #include <vector.h>
 #include <queue>
#endif

class Unit;
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
    static QValueList<QPoint> findPath(Unit* unit, int goalx, int goaly, int range = 0);

    /**
     * Returns lenght of path (in tiles)
     */
    int pathLength() const { return mPathLength; };

    /**
     * Returns cost of path
     */
    float pathCost() const { return mPathCost; };

  public:
    class PathNode;

    /**
     * In this list are waypoints of path
     */
    QValueList<QPoint> path;

  private:
    class Marking;
    /// TODO: QTL impl.
#ifdef USE_STL
    greater<PathNode> comp;
#endif
    float dist(int ax, int ay, int bx, int by);
    float cost(int x, int y);
#ifdef USE_STL
    void getFirst(vector<PathNode>&, PathNode& n);
#else
    void getFirst(QValueList<PathNode>&, PathNode& n);
#endif
    void neighbor(int& x, int& y, Direction d);
    Direction reverseDir(Direction d);
    bool inRange(int x, int y);

    void debug() const;

    bool findFastPath();
    bool findSlowPath();

  private:
    int mStartx;
    int mStarty;
    int mGoalx;
    int mGoaly;

    Unit* mUnit;

    float mModifier;
    float mCrossDivider;
    float mMinCost;
    int mAbortPath;

    int mNodesRemoved;
    int mPathLength;
    float mPathCost;
    int mRange;
};


#endif // BOSONPATH_H
