/*
    This file is part of the Boson game
    Copyright (C) 2001-2003 The Boson Team (boson-devel@lists.sourceforge.net)

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


#define SEARCH_STEPS 25  // How many steps of path to find


class Unit;
class Player;

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

    enum ResourceType { Minerals, Oil, EnemyBuilding, EnemyUnit };
    static QValueList<QPoint> findLocations(Player* player, int x, int y, int n, int radius, ResourceType type);

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
    QValueList<QPoint> path;

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

    class BosonPath::Marking
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


#endif // BOSONPATH_H

/*
 * vim: et sw=2
 */
