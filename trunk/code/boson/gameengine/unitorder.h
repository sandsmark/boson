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

#ifndef UNITORDER_H
#define UNITORDER_H


#include "../bo3dtools.h"
#include "unitbase.h"

class Unit;
class BosonPathInfo;



/**
 * Stores information about an order given to unit(s).
 *
 * Order might either be given by player through UI or by the unit itself to
 *  fulfill a more complex task. E.g. when unit is ordered to attack enemy, it
 *  might give itself suborder to move closer to the enemy.
 * Note that UnitOrder class stores only data which describes the order but not
 *  intermediate data which is computed by the unit to fulfill the order. E.g.
 *  in case of move order, destination point as well as range are stored here,
 *  but not the exact path to move to the destination point. Such data is
 *  stored in @ref UnitOrderData class instead.
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class UnitOrder
{
  public:
    UnitOrder();
    virtual ~UnitOrder();

    enum OrderType
    {
        Invalid = 0,
        Move,
        MoveToUnit,
        AttackUnit,
        AttackGround,
        Turn,
        TurnToUnit,
        Follow,
        Harvest,
        Refine
    };
    enum FinishStatus { Success = 1, Failure };

    virtual OrderType type() const = 0;
    virtual UnitBase::WorkType work() const = 0;

    /**
     * @return 0 if @ref work is not @ref UnitBase::WorkPlugin. If @ref work is
     * @ref UnitBase::WorkPlugin, this is meant to return the @ref
     * UnitPlugin::pluginType that should be used. For example @ref
     * UnitHarvestOrder should use @ref UnitPlugins::Harvester here.
     **/
    virtual int workPluginType() const { return 0; }

    virtual UnitOrder* duplicate() const = 0;

    virtual bool saveAsXML(QDomElement& root);
    virtual bool loadFromXML(const QDomElement& root);
};


class UnitMoveOrder : public UnitOrder
{
  public:
    UnitMoveOrder(const BoVector2Fixed& pos, int range = -1, bool attacking = true);
    virtual ~UnitMoveOrder();

    virtual OrderType type() const  { return Move; }
    virtual UnitBase::WorkType work() const  { return UnitBase::WorkMove; }
    virtual UnitOrder* duplicate() const  { return new UnitMoveOrder(*this); };

    virtual bool saveAsXML(QDomElement& root);
    virtual bool loadFromXML(const QDomElement& root);

    inline const BoVector2Fixed& position() const  { return mPos; }
    inline void setPosition(const BoVector2Fixed& p)  { mPos = p; }

    inline int range() const  { return mRange; }
    inline void setRange(int r)  { mRange = r; }

    inline bool withAttacking() const  { return mWithAttacking; }
    inline void setWithAttacking(bool a)  { mWithAttacking = a; }


  protected:
    BoVector2Fixed mPos;
    int mRange;
    bool mWithAttacking;
};


class UnitMoveToUnitOrder : public UnitMoveOrder
{
  public:
    UnitMoveToUnitOrder(Unit* target, int range = 0, bool attacking = false);
    virtual ~UnitMoveToUnitOrder();

    virtual OrderType type() const  { return MoveToUnit; }
    virtual UnitOrder* duplicate() const  { return new UnitMoveToUnitOrder(*this); };

    virtual bool saveAsXML(QDomElement& root);
    virtual bool loadFromXML(const QDomElement& root);

    Unit* target() const  { return mTarget; }


  protected:
    Unit* mTarget;
};


class UnitAttackOrder : public UnitOrder
{
  public:
    UnitAttackOrder(Unit* target, bool canmove = true);
    virtual ~UnitAttackOrder();

    virtual OrderType type() const  { return AttackUnit; }
    virtual UnitBase::WorkType work() const  { return UnitBase::WorkAttack; }
    virtual UnitOrder* duplicate() const  { return new UnitAttackOrder(*this); };

    virtual bool saveAsXML(QDomElement& root);
    virtual bool loadFromXML(const QDomElement& root);

    inline Unit* target() const  { return mTarget; }
    inline void setTarget(Unit* u)   { mTarget = u; }

    inline bool canMove() const  { return mCanMove; }
    inline void setCanMove(bool can)   { mCanMove = can; }


  protected:
    Unit* mTarget;
    bool mCanMove;
};


class UnitAttackGroundOrder : public UnitOrder
{
  public:
    UnitAttackGroundOrder(const BoVector2Fixed& pos);
    virtual ~UnitAttackGroundOrder();

    virtual OrderType type() const  { return AttackGround; }
    virtual UnitBase::WorkType work() const  { return UnitBase::WorkAttack; }
    virtual UnitOrder* duplicate() const  { return new UnitAttackGroundOrder(*this); };

    virtual bool saveAsXML(QDomElement& root);
    virtual bool loadFromXML(const QDomElement& root);

    inline const BoVector2Fixed& position() const  { return mPos; }
    inline void setPosition(const BoVector2Fixed& p)  { mPos = p; }


  protected:
    BoVector2Fixed mPos;
};


class UnitTurnOrder : public UnitOrder
{
  public:
    UnitTurnOrder(bofixed dir);
    virtual ~UnitTurnOrder();

    virtual OrderType type() const  { return Turn; }
    virtual UnitBase::WorkType work() const  { return UnitBase::WorkTurn; }
    virtual UnitOrder* duplicate() const  { return new UnitTurnOrder(*this); };

    virtual bool saveAsXML(QDomElement& root);
    virtual bool loadFromXML(const QDomElement& root);

    inline bofixed direction() const  { return mDirection; }
    inline void setDirection(bofixed d)   { mDirection = d; }


  protected:
    bofixed mDirection;
};


class UnitTurnToUnitOrder : public UnitOrder
{
  public:
    UnitTurnToUnitOrder(Unit* target);
    virtual ~UnitTurnToUnitOrder();

    virtual OrderType type() const  { return TurnToUnit; }
    virtual UnitBase::WorkType work() const  { return UnitBase::WorkTurn; }
    virtual UnitOrder* duplicate() const  { return new UnitTurnToUnitOrder(*this); };

    virtual bool saveAsXML(QDomElement& root);
    virtual bool loadFromXML(const QDomElement& root);

    inline Unit* target() const  { return mTarget; }
    inline void setTarget(Unit* u)   { mTarget = u; }


  protected:
    Unit* mTarget;
};


class UnitFollowOrder : public UnitOrder
{
  public:
    UnitFollowOrder(Unit* target, bofixed distance = 2);
    virtual ~UnitFollowOrder();

    virtual OrderType type() const  { return Follow; }
    virtual UnitBase::WorkType work() const  { return UnitBase::WorkFollow; }
    virtual UnitOrder* duplicate() const  { return new UnitFollowOrder(*this); };

    virtual bool saveAsXML(QDomElement& root);
    virtual bool loadFromXML(const QDomElement& root);

    inline Unit* target() const  { return mTarget; }
    inline void setTarget(Unit* u)   { mTarget = u; }

    inline bofixed distance() const  { return mDistance; }
    inline void setDistance(bofixed dist)   { mDistance = dist; }


  protected:
    Unit* mTarget;
    bofixed mDistance;
};


class UnitHarvestOrder : public UnitOrder
{
  public:
    UnitHarvestOrder(Unit* at);
    virtual ~UnitHarvestOrder();

    virtual OrderType type() const  { return Harvest; }
    virtual UnitBase::WorkType work() const  { return UnitBase::WorkPlugin; }
    virtual int workPluginType() const;
    virtual UnitOrder* duplicate() const  { return new UnitHarvestOrder(*this); };

    virtual bool saveAsXML(QDomElement& root);
    virtual bool loadFromXML(const QDomElement& root);

    inline Unit* target() const  { return mTarget; }
    inline void setTarget(Unit* u)   { mTarget = u; }


  protected:
    Unit* mTarget;
};


// TODO: see if this can be merged with harvest order
class UnitRefineOrder : public UnitOrder
{
  public:
    UnitRefineOrder(Unit* at);
    virtual ~UnitRefineOrder();

    virtual OrderType type() const  { return Refine; }
    virtual UnitBase::WorkType work() const  { return UnitBase::WorkPlugin; }
    virtual int workPluginType() const;
    virtual UnitOrder* duplicate() const  { return new UnitRefineOrder(*this); };

    virtual bool saveAsXML(QDomElement& root);
    virtual bool loadFromXML(const QDomElement& root);

    inline Unit* target() const  { return mTarget; }
    inline void setTarget(Unit* u)   { mTarget = u; }


  protected:
    Unit* mTarget;
};



/**
 * Stores unit-specific data for an order.
 *
 * Difference between @ref UnitOrder and this class is that @ref UnitOrder
 *  objects can be shared between units and thus mustn't contain unit-specific
 *  data such as path in case of moving. Such data is instead stored in this
 *  class.
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class UnitOrderData
{
  public:
    UnitOrderData(UnitOrder* order);
    virtual ~UnitOrderData();

    UnitOrder::OrderType type() const  { return mOrder ? mOrder->type() : UnitOrder::Invalid; }
    UnitOrder* order()  { return mOrder; }

    virtual bool saveAsXML(QDomElement& root);
    virtual bool loadFromXML(const QDomElement& root);

    static UnitOrderData* createData(UnitOrder* order);


    UnitOrderData* suborder() const  { return mSuborder; }
    UnitOrderData* parent() const  { return mParent; }
    // TODO: ok to delete current suborder?
    void setSuborder(UnitOrderData* o)  { delete mSuborder; o->mParent = this; mSuborder = o; }
    void suborderDone()  { delete mSuborder; mSuborder = 0; }

    UnitOrderData* currentOrder()  { return mSuborder ? mSuborder->currentOrder() : this; }


  protected:
    UnitOrder* mOrder;

    UnitOrderData* mSuborder;
    UnitOrderData* mParent;
};

class UnitMoveOrderData : public UnitOrderData
{
  public:
    UnitMoveOrderData(UnitOrder* order);
    virtual ~UnitMoveOrderData();

    virtual bool saveAsXML(QDomElement& root);
    virtual bool loadFromXML(const QDomElement& root);

    BosonPathInfo* pathinfo;
    Unit* target;
};

class UnitMoveToUnitOrderData : public UnitMoveOrderData
{
  public:
    UnitMoveToUnitOrderData(UnitOrder* order);
    virtual ~UnitMoveToUnitOrderData();

    virtual bool saveAsXML(QDomElement& root);
    virtual bool loadFromXML(const QDomElement& root);

    BoVector2Fixed lastTargetPos;
};


#endif

/*
 * vim: et sw=2
 */
