/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

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

#ifndef UNITGROUP_H
#define UNITGROUP_H

#include <qptrlist.h>

class Unit;
class QPoint;

/** This class stores pointers to units in one "group"
  *
  * At the moment groups are only used for movement: when you move multiple
  * units together, temporary group is created for them. Leader is assigned to
  * group and path is searched for only this unit. Other units are told to
  * follow leader
  *
  * In future, it may be possible to manually define some groups and then have
  * quick access to them through some keys.
  *
  * @author Rivo Laks <rivolaks@hot.ee>
  */
class UnitGroup
{
  public:
    UnitGroup(bool moving = false);
    ~UnitGroup();
    /** Adds unit to group */
    void addMember(Unit* unit);
    /** Removes unit from group */
    void removeMember(Unit* unit);
    void setLeader(Unit* leader);
    void setMovingGroup(bool moving);
    void leaderMoved(int x, int y);
    void leaderDestroyed();
    void leaderStopped();
    bool isLeader(Unit* unit);
  private:
    QPtrList<Unit> mMembers;
    Unit* mLeader;
    bool mMoving;
};

#endif
