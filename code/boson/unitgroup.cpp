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

#include "unitgroup.h"

#include "unit.h"
#include <qpoint.h>

UnitGroup::UnitGroup(bool moving)
{
  mMoving = moving;

  mDelete = false;
}

UnitGroup::~UnitGroup()
{
}

void UnitGroup::addMember(Unit* unit)
{
  mMembers.append(unit);
}

void UnitGroup::removeMember(Unit* unit)
{
  mMembers.remove(unit);
}

void UnitGroup::setLeader(Unit* unit)
{
  mLeader = unit;
}

void UnitGroup::leaderMoved(int x, int y)
{
  QPtrListIterator<Unit> it(mMembers);
  Unit* member;
  while((member = it.current()) != 0)
  {
    ++it;
    ((MobileUnit*)member)->leaderMoved(x, y);
  }
}

void UnitGroup::leaderDestroyed()
{
  mLeader = mMembers.take(0);
  mLeader->setGroupLeader(true);
}

bool UnitGroup::isLeader(Unit* unit)
{
  if(unit->id() == mLeader->id()) 
  {
    return true;
  }
  return false;
}

void UnitGroup::leaderStopped()
{
  QPtrListIterator<Unit> it(mMembers);
  Unit* member;
  while((member = it.current()) != 0)
  {
    ++it;
    member->stopMoving();
  }
}

void UnitGroup::advanceGroupMove()
{
  kdDebug() << k_funcinfo << endl;
  mLeader->advanceMove();
  mLeader->advanceMoveCheck();
  QPtrListIterator<Unit> it(mMembers);
  while(it.current())
  {
    it.current()->advanceGroupMove(mLeader);
    it.current()->advanceMoveCheck();
    ++it;
  }
}
