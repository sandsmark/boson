/*
    This file is part of the Boson game
    Copyright (C) 2006 Rivo Laks (rivolaks@hot.ee)

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

#include "unitorder.h"

#include "bodebug.h"
#include "bosonpath.h"



UnitOrder::UnitOrder()
{
  /*mSuborder = 0;
  mParent = 0;*/
}

UnitOrder::~UnitOrder()
{
  //delete mSuborder;
}



UnitMoveOrder::UnitMoveOrder(const BoVector2Fixed& pos, int range, bool attacking)
{
  mPos = pos;
  mRange = range;
  mWithAttacking = attacking;
}

UnitMoveOrder::~UnitMoveOrder()
{
}


UnitMoveToUnitOrder::UnitMoveToUnitOrder(Unit* target, int range, bool attacking) :
    UnitMoveOrder(BoVector2Fixed(), range, attacking)
{
  mTarget = target;
}

UnitMoveToUnitOrder::~UnitMoveToUnitOrder()
{
}


UnitAttackOrder::UnitAttackOrder(Unit* target, bool canmove)
{
  mTarget = target;
  mCanMove = canmove;
}

UnitAttackOrder::~UnitAttackOrder()
{
}


UnitAttackGroundOrder::UnitAttackGroundOrder(const BoVector2Fixed& pos)
{
  mPos = pos;
}

UnitAttackGroundOrder::~UnitAttackGroundOrder()
{
}



UnitTurnOrder::UnitTurnOrder(bofixed dir)
{
  mDirection = dir;
}

UnitTurnOrder::~UnitTurnOrder()
{
}



UnitTurnToUnitOrder::UnitTurnToUnitOrder(Unit* target)
{
  mTarget = target;
}

UnitTurnToUnitOrder::~UnitTurnToUnitOrder()
{
}



UnitFollowOrder::UnitFollowOrder(Unit* target, bofixed distance)
{
  mTarget = target;
  mDistance = distance;
}

UnitFollowOrder::~UnitFollowOrder()
{
}



UnitHarvestOrder::UnitHarvestOrder(Unit* at)
{
  mTarget = at;
}

UnitHarvestOrder::~UnitHarvestOrder()
{
}



UnitRefineOrder::UnitRefineOrder(Unit* at)
{
  mTarget = at;
}

UnitRefineOrder::~UnitRefineOrder()
{
}



UnitOrderData::UnitOrderData(UnitOrder* order)
{
  mSuborder = 0;
  mParent = 0;

  mOrder = order;
}

UnitOrderData::~UnitOrderData()
{
  delete mSuborder;

  delete mOrder;
}

UnitOrderData* UnitOrderData::createData(UnitOrder* order)
{
  UnitOrderData* data = 0;
  switch(order->type())
  {
    case UnitOrder::Move:
      data = new UnitMoveOrderData(order);
      break;
    case UnitOrder::MoveToUnit:
      data = new UnitMoveToUnitOrderData(order);
      break;
    case UnitOrder::AttackUnit:
    case UnitOrder::Follow:
    case UnitOrder::Turn:
    case UnitOrder::TurnToUnit:
    case UnitOrder::Harvest:
    case UnitOrder::Refine:
      data = new UnitOrderData(order);
      break;
    default:
      boError() << k_funcinfo << "Invalid order type " << order->type() << endl;
      break;
  }

  return data;
}


UnitMoveOrderData::UnitMoveOrderData(UnitOrder* order) : UnitOrderData(order)
{
  pathinfo = new BosonPathInfo;
  target = 0;
}

UnitMoveOrderData::~UnitMoveOrderData()
{
  delete pathinfo;
}


UnitMoveToUnitOrderData::UnitMoveToUnitOrderData(UnitOrder* order) : UnitMoveOrderData(order)
{
}

UnitMoveToUnitOrderData::~UnitMoveToUnitOrderData()
{
}

