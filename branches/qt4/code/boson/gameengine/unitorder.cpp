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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "unitorder.h"

#include "bodebug.h"
#include "bosonpath.h"
#include "unitplugins/unitplugin.h"
#include "unit.h"
#include "bosoncanvas.h"

#include <qdom.h>



UnitOrder::UnitOrder()
{
}

UnitOrder::~UnitOrder()
{
}

bool UnitOrder::saveAsXML(QDomElement& root)
{
  root.setAttribute("OrderType", (int)type());
  return true;
}

bool UnitOrder::loadFromXML(const QDomElement&, BosonCanvas*)
{
  return true;
}

UnitOrder* UnitOrder::createAndLoadFromXML(const QDomElement& root, BosonCanvas* canvas)
{
  bool ok;
  int type = root.attribute("OrderType").toInt(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Invalid value for OrderType attribute" << endl;
    return 0;
  }

  UnitOrder* o = 0;

  switch(type)
  {
    case UnitOrder::Move:
      o = new UnitMoveOrder;
      break;
    case UnitOrder::MoveToUnit:
      o = new UnitMoveToUnitOrder;
      break;
    case UnitOrder::AttackUnit:
      o = new UnitAttackOrder;
      break;
    case UnitOrder::AttackGround:
      o = new UnitAttackGroundOrder;
      break;
    case UnitOrder::Follow:
      o = new UnitFollowOrder;
      break;
    case UnitOrder::Turn:
      o = new UnitTurnOrder;
      break;
    case UnitOrder::TurnToUnit:
      o = new UnitTurnToUnitOrder;
      break;
    case UnitOrder::Harvest:
      o = new UnitHarvestOrder;
      break;
    case UnitOrder::Refine:
      o = new UnitRefineOrder;
      break;
    default:
      boError() << k_funcinfo << "Loaded invalid order type " << type << endl;
      break;
  }

  if(o)
  {
    if(!o->loadFromXML(root, canvas))
    {
      delete o;
      return 0;
    }
  }

  return o;
}



UnitMoveOrder::UnitMoveOrder(const BoVector2Fixed& pos, int range, bool attacking) : UnitOrder()
{
  mPos = pos;
  mRange = range;
  mWithAttacking = attacking;
}

UnitMoveOrder::UnitMoveOrder() : UnitOrder()
{
}

UnitMoveOrder::~UnitMoveOrder()
{
}

bool UnitMoveOrder::saveAsXML(QDomElement& root)
{
  if(!UnitOrder::saveAsXML(root))
  {
    return false;
  }
  saveVector2AsXML(mPos, root, "Position");
  root.setAttribute("Range", mRange);
  root.setAttribute("WithAttacking", mWithAttacking ? 1 : 0);
  return true;
}

bool UnitMoveOrder::loadFromXML(const QDomElement& root, BosonCanvas* canvas)
{
  if(!UnitOrder::loadFromXML(root, canvas))
  {
    return false;
  }
  loadVector2FromXML(&mPos, root, "Position");

  bool ok;
  mRange = root.attribute("Range").toInt(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Invalid value for Range attribute" << endl;
    return false;
  }
  mWithAttacking = (bool)root.attribute("WithAttacking").toInt(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Invalid value for WithAttacking attribute" << endl;
    return false;
  }

  return true;
}


UnitMoveInsideUnitOrder::UnitMoveInsideUnitOrder(const BoVector2Fixed& pos)
  : UnitMoveOrder(pos, -1, false)
{
}

UnitMoveInsideUnitOrder::~UnitMoveInsideUnitOrder()
{
}

bool UnitMoveInsideUnitOrder::saveAsXML(QDomElement& root)
{
  if(!UnitMoveOrder::saveAsXML(root))
  {
    return false;
  }
  return true;
}

bool UnitMoveInsideUnitOrder::loadFromXML(const QDomElement& root, BosonCanvas* canvas)
{
  if(!UnitMoveOrder::loadFromXML(root, canvas))
  {
    return false;
  }
  return true;
}



UnitMoveToUnitOrder::UnitMoveToUnitOrder(Unit* target, int range, bool attacking) :
    UnitMoveOrder(BoVector2Fixed(), range, attacking)
{
  mTarget = target;
}

UnitMoveToUnitOrder::UnitMoveToUnitOrder() : UnitMoveOrder()
{
}

UnitMoveToUnitOrder::~UnitMoveToUnitOrder()
{
}

bool UnitMoveToUnitOrder::saveAsXML(QDomElement& root)
{
  if(!UnitOrder::saveAsXML(root))
  {
    return false;
  }
  root.setAttribute("TargetId", mTarget->id());
  return true;
}

bool UnitMoveToUnitOrder::loadFromXML(const QDomElement& root, BosonCanvas* canvas)
{
  if(!UnitOrder::loadFromXML(root, canvas))
  {
    return false;
  }

  bool ok;
  int targetid = root.attribute("TargetId").toInt(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Invalid value for TargetId attribute" << endl;
    return false;
  }
  mTarget = canvas->findUnit(targetid);
  return true;
}


UnitAttackOrder::UnitAttackOrder(Unit* target, bool canmove) : UnitOrder()
{
  mTarget = target;
  mCanMove = canmove;
}

UnitAttackOrder::UnitAttackOrder() : UnitOrder()
{
}

UnitAttackOrder::~UnitAttackOrder()
{
}

bool UnitAttackOrder::saveAsXML(QDomElement& root)
{
  if(!UnitOrder::saveAsXML(root))
  {
    return false;
  }
  root.setAttribute("TargetId", mTarget->id());
  root.setAttribute("CanMove", mCanMove ? 1 : 0);
  return true;
}

bool UnitAttackOrder::loadFromXML(const QDomElement& root, BosonCanvas* canvas)
{
  if(!UnitOrder::loadFromXML(root, canvas))
  {
    return false;
  }
  bool ok;
  int targetid = root.attribute("TargetId").toInt(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Invalid value for TargetId attribute" << endl;
    return false;
  }
  mTarget = canvas->findUnit(targetid);

  mCanMove = root.attribute("CanMove").toInt(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Invalid value for CanMove attribute" << endl;
    return false;
  }

  return true;
}


UnitAttackGroundOrder::UnitAttackGroundOrder(const BoVector2Fixed& pos) : UnitOrder()
{
  mPos = pos;
}

UnitAttackGroundOrder::UnitAttackGroundOrder() : UnitOrder()
{
}

UnitAttackGroundOrder::~UnitAttackGroundOrder()
{
}

bool UnitAttackGroundOrder::saveAsXML(QDomElement& root)
{
  if(!UnitOrder::saveAsXML(root))
  {
    return false;
  }
  saveVector2AsXML(mPos, root, "Position");
  return true;
}

bool UnitAttackGroundOrder::loadFromXML(const QDomElement& root, BosonCanvas* canvas)
{
  if(!UnitOrder::loadFromXML(root, canvas))
  {
    return false;
  }
  loadVector2FromXML(&mPos, root, "Position");
  return true;
}



UnitTurnOrder::UnitTurnOrder(bofixed dir) : UnitOrder()
{
  mDirection = dir;
}

UnitTurnOrder::UnitTurnOrder() : UnitOrder()
{
}

UnitTurnOrder::~UnitTurnOrder()
{
}

bool UnitTurnOrder::saveAsXML(QDomElement& root)
{
  if(!UnitOrder::saveAsXML(root))
  {
    return false;
  }
  root.setAttribute("Direction", mDirection);
  return true;
}

bool UnitTurnOrder::loadFromXML(const QDomElement& root, BosonCanvas* canvas)
{
  if(!UnitOrder::loadFromXML(root, canvas))
  {
    return false;
  }

  bool ok;
  mDirection = root.attribute("Direction").toFloat(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Invalid value for Direction attribute" << endl;
    return false;
  }
  return true;
}



UnitTurnToUnitOrder::UnitTurnToUnitOrder(Unit* target) : UnitOrder()
{
  mTarget = target;
}

UnitTurnToUnitOrder::UnitTurnToUnitOrder() : UnitOrder()
{
}

UnitTurnToUnitOrder::~UnitTurnToUnitOrder()
{
}

bool UnitTurnToUnitOrder::saveAsXML(QDomElement& root)
{
  if(!UnitOrder::saveAsXML(root))
  {
    return false;
  }
  root.setAttribute("TargetId", mTarget->id());
  return true;
}

bool UnitTurnToUnitOrder::loadFromXML(const QDomElement& root, BosonCanvas* canvas)
{
  if(!UnitOrder::loadFromXML(root, canvas))
  {
    return false;
  }

  bool ok;
  int targetid = root.attribute("TargetId").toInt(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Invalid value for TargetId attribute" << endl;
    return false;
  }
  mTarget = canvas->findUnit(targetid);
  return true;
}



UnitFollowOrder::UnitFollowOrder(Unit* target, bofixed distance) : UnitOrder()
{
  mTarget = target;
  mDistance = distance;
}

UnitFollowOrder::UnitFollowOrder() : UnitOrder()
{
}

UnitFollowOrder::~UnitFollowOrder()
{
}

bool UnitFollowOrder::saveAsXML(QDomElement& root)
{
  if(!UnitOrder::saveAsXML(root))
  {
    return false;
  }
  root.setAttribute("TargetId", mTarget->id());
  root.setAttribute("Distance", mDistance);
  return true;
}

bool UnitFollowOrder::loadFromXML(const QDomElement& root, BosonCanvas* canvas)
{
  if(!UnitOrder::loadFromXML(root, canvas))
  {
    return false;
  }

  bool ok;
  int targetid = root.attribute("TargetId").toInt(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Invalid value for TargetId attribute" << endl;
    return false;
  }
  mTarget = canvas->findUnit(targetid);

  mDistance = root.attribute("Distance").toFloat(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Invalid value for Distance attribute" << endl;
    return false;
  }
  return true;
}



UnitHarvestOrder::UnitHarvestOrder(Unit* at) : UnitOrder()
{
  mTarget = at;
}

UnitHarvestOrder::UnitHarvestOrder() : UnitOrder()
{
}

UnitHarvestOrder::~UnitHarvestOrder()
{
}

int UnitHarvestOrder::workPluginType() const
{
  return UnitPlugin::Harvester;
}

bool UnitHarvestOrder::saveAsXML(QDomElement& root)
{
  if(!UnitOrder::saveAsXML(root))
  {
    return false;
  }
  root.setAttribute("TargetId", mTarget->id());
  return true;
}

bool UnitHarvestOrder::loadFromXML(const QDomElement& root, BosonCanvas* canvas)
{
  if(!UnitOrder::loadFromXML(root, canvas))
  {
    return false;
  }
  bool ok;
  int targetid = root.attribute("TargetId").toInt(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Invalid value for TargetId attribute" << endl;
    return false;
  }
  mTarget = canvas->findUnit(targetid);
  return true;
}



UnitRefineOrder::UnitRefineOrder(Unit* at) : UnitOrder()
{
  mTarget = at;
}

UnitRefineOrder::UnitRefineOrder() : UnitOrder()
{
}

UnitRefineOrder::~UnitRefineOrder()
{
}

int UnitRefineOrder::workPluginType() const
{
  return UnitPlugin::Harvester;
}

bool UnitRefineOrder::saveAsXML(QDomElement& root)
{
  if(!UnitOrder::saveAsXML(root))
  {
    return false;
  }
  root.setAttribute("TargetId", mTarget->id());
  return true;
}

bool UnitRefineOrder::loadFromXML(const QDomElement& root, BosonCanvas* canvas)
{
  if(!UnitOrder::loadFromXML(root, canvas))
  {
    return false;
  }
  bool ok;
  int targetid = root.attribute("TargetId").toInt(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Invalid value for TargetId attribute" << endl;
    return false;
  }
  mTarget = canvas->findUnit(targetid);
  return true;
}



UnitEnterUnitOrder::UnitEnterUnitOrder(Unit* unit)
{
  mTarget = unit;
  mIsLeaveOrder = false;
}

UnitEnterUnitOrder::~UnitEnterUnitOrder()
{
}

int UnitEnterUnitOrder::workPluginType() const
{
  return UnitPlugin::EnterUnit;
}

bool UnitEnterUnitOrder::saveAsXML(QDomElement& root)
{
  if(!UnitOrder::saveAsXML(root))
  {
    return false;
  }
  root.setAttribute("TargetId", mTarget->id());
  root.setAttribute("IsLeaveOrder", mIsLeaveOrder ? 1 : 0);
  return true;
}

bool UnitEnterUnitOrder::loadFromXML(const QDomElement& root, BosonCanvas* canvas)
{
  if(!UnitOrder::loadFromXML(root, canvas))
  {
    return false;
  }
  bool ok;
  int targetid = root.attribute("TargetId").toInt(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Invalid value for TargetId attribute" << endl;
    return false;
  }
  mTarget = canvas->findUnit(targetid);
  mIsLeaveOrder = (bool)root.attribute("IsLeaveOrder").toInt(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Invalid value for IsLeaveOrder attribute" << endl;
    return false;
  }
  return true;
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
    case UnitOrder::MoveInsideUnit:
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
    case UnitOrder::EnterUnit:
      data = new UnitOrderData(order);
      break;
    default:
      boError() << k_funcinfo << "Invalid order type " << order->type() << endl;
      break;
  }

  return data;
}

UnitOrderData* UnitOrderData::createAndLoadFromXML(const QDomElement& root, BosonCanvas* canvas)
{
  QDomElement orderxml = root.namedItem("Order").toElement();
  if(orderxml.isNull())
  {
    boError() << k_funcinfo << "NULL Order element!" << endl;
    return 0;
  }

  UnitOrder* order = UnitOrder::createAndLoadFromXML(orderxml, canvas);
  if(!order)
  {
    return 0;
  }

  UnitOrderData* orderdata = createData(order);
  if(!orderdata->loadFromXML(root, canvas))
  {
    delete orderdata;
    return 0;
  }

  return orderdata;
}

bool UnitOrderData::saveAsXML(QDomElement& root)
{
  QDomDocument doc = root.ownerDocument();
  if(mSuborder)
  {
    QDomElement suborderxml = doc.createElement("SuborderData");
    root.appendChild(suborderxml);
    if(!mSuborder->saveAsXML(suborderxml))
    {
      return false;
    }
  }
  QDomElement orderxml = doc.createElement("Order");
  root.appendChild(orderxml);
  if(!mOrder->saveAsXML(orderxml))
  {
    return false;
  }

  return true;
}

bool UnitOrderData::loadFromXML(const QDomElement& root, BosonCanvas* canvas)
{
  QDomElement suborderxml = root.namedItem("SuborderData").toElement();
  if(!suborderxml.isNull())
  {
    UnitOrderData* sub = createAndLoadFromXML(suborderxml, canvas);
    if(!sub)
    {
      return false;
    }
    setSuborder(sub);
  }
  return true;
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

bool UnitMoveOrderData::saveAsXML(QDomElement& root)
{
  if(!UnitOrderData::saveAsXML(root))
  {
    return false;
  }

  QDomDocument doc = root.ownerDocument();
  QDomElement pathinfoxml = doc.createElement("PathInfo");
  root.appendChild(pathinfoxml);
  if(!pathinfo->saveAsXML(pathinfoxml))
  {
    return false;
  }
  root.setAttribute("TargetId", target ? (int)target->id() : (int)-1);
  return true;
}

bool UnitMoveOrderData::loadFromXML(const QDomElement& root, BosonCanvas* canvas)
{
  if(!UnitOrderData::loadFromXML(root, canvas))
  {
    return false;
  }

  bool ok;
  int targetid = root.attribute("TargetId").toInt(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Invalid value for TargetId attribute" << endl;
    return false;
  }
  target = (targetid == -1) ? 0 : canvas->findUnit(targetid);

  QDomElement pathinfoxml = root.namedItem("PathInfo").toElement();
  if(pathinfoxml.isNull())
  {
    boError() << k_funcinfo << "Missing PathInfo element!" << endl;
    return false;
  }
  if(!pathinfo->loadFromXML(pathinfoxml))
  {
    return false;
  }

  return true;
}


UnitMoveToUnitOrderData::UnitMoveToUnitOrderData(UnitOrder* order) : UnitMoveOrderData(order)
{
}

UnitMoveToUnitOrderData::~UnitMoveToUnitOrderData()
{
}

bool UnitMoveToUnitOrderData::saveAsXML(QDomElement& root)
{
  if(!UnitOrderData::saveAsXML(root))
  {
    return false;
  }
  saveVector2AsXML(lastTargetPos, root, "LastTargetPos");
  return true;
}

bool UnitMoveToUnitOrderData::loadFromXML(const QDomElement& root, BosonCanvas* canvas)
{
  if(!UnitOrderData::loadFromXML(root, canvas))
  {
    return false;
  }
  loadVector2FromXML(&lastTargetPos, root, "LastTargetPos");
  return true;
}

/*
 * vim: et sw=2
 */
