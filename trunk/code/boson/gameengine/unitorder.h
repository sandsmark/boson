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
        // do NOT change numbers or insert something in between!
        // -> the type of an order is saved and thus must remain constant
        //    between releases
        Invalid = 0,
        Move = 1,
        MoveToUnit = 2,
        MoveInsideUnit = 3,
        AttackUnit = 4,
        AttackGround = 5,
        Turn = 6,
        TurnToUnit = 7,
        Follow = 8,
        Harvest = 9,
        Refine = 10,
        EnterUnit = 11
    };
    enum FinishStatus { Success = 1, Failure };

    virtual OrderType type() const = 0;
    virtual UnitBase::WorkType work() const = 0;

    /**
     * @return TRUE if this is a move order, otherwise FALSE.
     *
     * A move order is an order that <em>directly</em> handles moving (not
     * through a suborder), such as @ref UnitMoveOrder and
     * @ref UnitMoveToUnitOrder.
     *
     * A move order <em>MUST</em> provide a @ref UnitMoveOrderData dervided
     * class in @ref UnitOrderData::createData.
     **/
    virtual bool isMoveOrder() const { return false; }

    /**
     * @return 0 if @ref work is not @ref UnitBase::WorkPlugin. If @ref work is
     * @ref UnitBase::WorkPlugin, this is meant to return the @ref
     * UnitPlugin::pluginType that should be used. For example @ref
     * UnitHarvestOrder should use @ref UnitPlugins::Harvester here.
     **/
    virtual int workPluginType() const { return 0; }

    virtual UnitOrder* duplicate() const = 0;

    virtual bool saveAsXML(QDomElement& root);
    virtual bool loadFromXML(const QDomElement& root, BosonCanvas* canvas);

    static UnitOrder* createAndLoadFromXML(const QDomElement& root, BosonCanvas* canvas);
};


class UnitMoveOrder : public UnitOrder
{
  public:
    UnitMoveOrder(const BoVector2Fixed& pos, int range = -1, bool attacking = true);
    UnitMoveOrder();
    virtual ~UnitMoveOrder();

    virtual OrderType type() const  { return Move; }
    virtual UnitBase::WorkType work() const  { return UnitBase::WorkMove; }
    virtual bool isMoveOrder() const { return true; }
    virtual UnitOrder* duplicate() const  { return new UnitMoveOrder(*this); };

    virtual bool saveAsXML(QDomElement& root);
    virtual bool loadFromXML(const QDomElement& root, BosonCanvas* canvas);

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


class UnitMoveInsideUnitOrder : public UnitMoveOrder
{
  public:
    UnitMoveInsideUnitOrder(const BoVector2Fixed& pos);
    virtual ~UnitMoveInsideUnitOrder();

    virtual OrderType type() const  { return MoveInsideUnit; }
    virtual UnitOrder* duplicate() const  { return new UnitMoveInsideUnitOrder(*this); };

    virtual bool saveAsXML(QDomElement& root);
    virtual bool loadFromXML(const QDomElement& root, BosonCanvas* canvas);

  protected:
};


class UnitMoveToUnitOrder : public UnitMoveOrder
{
  public:
    UnitMoveToUnitOrder(Unit* target, int range = 0, bool attacking = false);
    UnitMoveToUnitOrder();
    virtual ~UnitMoveToUnitOrder();

    virtual OrderType type() const  { return MoveToUnit; }
    virtual bool isMoveOrder() const { return true; }
    virtual UnitOrder* duplicate() const  { return new UnitMoveToUnitOrder(*this); };

    virtual bool saveAsXML(QDomElement& root);
    virtual bool loadFromXML(const QDomElement& root, BosonCanvas* canvas);

    Unit* target() const  { return mTarget; }


  protected:
    Unit* mTarget;
};


class UnitAttackOrder : public UnitOrder
{
  public:
    UnitAttackOrder(Unit* target, bool canmove = true);
    UnitAttackOrder();
    virtual ~UnitAttackOrder();

    virtual OrderType type() const  { return AttackUnit; }
    virtual UnitBase::WorkType work() const  { return UnitBase::WorkAttack; }
    virtual UnitOrder* duplicate() const  { return new UnitAttackOrder(*this); };

    virtual bool saveAsXML(QDomElement& root);
    virtual bool loadFromXML(const QDomElement& root, BosonCanvas* canvas);

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
    UnitAttackGroundOrder();
    virtual ~UnitAttackGroundOrder();

    virtual OrderType type() const  { return AttackGround; }
    virtual UnitBase::WorkType work() const  { return UnitBase::WorkAttack; }
    virtual UnitOrder* duplicate() const  { return new UnitAttackGroundOrder(*this); };

    virtual bool saveAsXML(QDomElement& root);
    virtual bool loadFromXML(const QDomElement& root, BosonCanvas* canvas);

    inline const BoVector2Fixed& position() const  { return mPos; }
    inline void setPosition(const BoVector2Fixed& p)  { mPos = p; }


  protected:
    BoVector2Fixed mPos;
};


class UnitTurnOrder : public UnitOrder
{
  public:
    UnitTurnOrder(bofixed dir);
    UnitTurnOrder();
    virtual ~UnitTurnOrder();

    virtual OrderType type() const  { return Turn; }
    virtual UnitBase::WorkType work() const  { return UnitBase::WorkTurn; }
    virtual UnitOrder* duplicate() const  { return new UnitTurnOrder(*this); };

    virtual bool saveAsXML(QDomElement& root);
    virtual bool loadFromXML(const QDomElement& root, BosonCanvas* canvas);

    inline bofixed direction() const  { return mDirection; }
    inline void setDirection(bofixed d)   { mDirection = d; }


  protected:
    bofixed mDirection;
};


class UnitTurnToUnitOrder : public UnitOrder
{
  public:
    UnitTurnToUnitOrder(Unit* target);
    UnitTurnToUnitOrder();
    virtual ~UnitTurnToUnitOrder();

    virtual OrderType type() const  { return TurnToUnit; }
    virtual UnitBase::WorkType work() const  { return UnitBase::WorkTurn; }
    virtual UnitOrder* duplicate() const  { return new UnitTurnToUnitOrder(*this); };

    virtual bool saveAsXML(QDomElement& root);
    virtual bool loadFromXML(const QDomElement& root, BosonCanvas* canvas);

    inline Unit* target() const  { return mTarget; }
    inline void setTarget(Unit* u)   { mTarget = u; }


  protected:
    Unit* mTarget;
};


class UnitFollowOrder : public UnitOrder
{
  public:
    UnitFollowOrder(Unit* target, bofixed distance = 2);
    UnitFollowOrder();
    virtual ~UnitFollowOrder();

    virtual OrderType type() const  { return Follow; }
    virtual UnitBase::WorkType work() const  { return UnitBase::WorkFollow; }
    virtual UnitOrder* duplicate() const  { return new UnitFollowOrder(*this); };

    virtual bool saveAsXML(QDomElement& root);
    virtual bool loadFromXML(const QDomElement& root, BosonCanvas* canvas);

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
    UnitHarvestOrder();
    virtual ~UnitHarvestOrder();

    virtual OrderType type() const  { return Harvest; }
    virtual UnitBase::WorkType work() const  { return UnitBase::WorkPlugin; }
    virtual int workPluginType() const;
    virtual UnitOrder* duplicate() const  { return new UnitHarvestOrder(*this); };

    virtual bool saveAsXML(QDomElement& root);
    virtual bool loadFromXML(const QDomElement& root, BosonCanvas* canvas);

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
    UnitRefineOrder();
    virtual ~UnitRefineOrder();

    virtual OrderType type() const  { return Refine; }
    virtual UnitBase::WorkType work() const  { return UnitBase::WorkPlugin; }
    virtual int workPluginType() const;
    virtual UnitOrder* duplicate() const  { return new UnitRefineOrder(*this); };

    virtual bool saveAsXML(QDomElement& root);
    virtual bool loadFromXML(const QDomElement& root, BosonCanvas* canvas);

    inline Unit* target() const  { return mTarget; }
    inline void setTarget(Unit* u)   { mTarget = u; }


  protected:
    Unit* mTarget;
};



/**
 * Order a unit to "enter" another unit, such as a plane that lands on an
 * airport or some unit that enters a repairyard or something like that.
 *
 * The unit being entered must have a @ref UnitStoragePlugin and it must be able
 * to hold this unit.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class UnitEnterUnitOrder : public UnitOrder
{
  public:
    UnitEnterUnitOrder(Unit* unit);
    virtual ~UnitEnterUnitOrder();

    virtual OrderType type() const  { return EnterUnit; }
    virtual UnitBase::WorkType work() const { return UnitBase::WorkPlugin; }
    virtual int workPluginType() const;
    virtual UnitOrder* duplicate() const { return new UnitEnterUnitOrder(*this); }

    /**
     * Change the "Enter" order into a "Leave" order.
     **/
    void setIsLeaveOrder(bool isLeave) { mIsLeaveOrder = isLeave; }
    /**
     * @return FALSE if this order is a "enter unit" order (the default) or TRUE
     * if it is a "leave unit" order.
     **/
    bool isLeaveOrder() const { return mIsLeaveOrder; }

    virtual bool saveAsXML(QDomElement& root);
    virtual bool loadFromXML(const QDomElement& root, BosonCanvas* canvas);

    inline Unit* target() const  { return mTarget; }
    inline void setTarget(Unit* u) { mTarget = u; }


  protected:
    Unit* mTarget;
    bool mIsLeaveOrder;
};


class UnitOrderData
{
  public:
    UnitOrderData(UnitOrder* order);
    virtual ~UnitOrderData();

    UnitOrder::OrderType type() const  { return mOrder ? mOrder->type() : UnitOrder::Invalid; }
    UnitOrder* order()  { return mOrder; }

    virtual bool saveAsXML(QDomElement& root);
    virtual bool loadFromXML(const QDomElement& root, BosonCanvas* canvas);

    static UnitOrderData* createData(UnitOrder* order);
    static UnitOrderData* createAndLoadFromXML(const QDomElement& root, BosonCanvas* canvas);


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
    virtual bool loadFromXML(const QDomElement& root, BosonCanvas* canvas);

    BosonPathInfo* pathinfo;
    Unit* target;
};

class UnitMoveToUnitOrderData : public UnitMoveOrderData
{
  public:
    UnitMoveToUnitOrderData(UnitOrder* order);
    virtual ~UnitMoveToUnitOrderData();

    virtual bool saveAsXML(QDomElement& root);
    virtual bool loadFromXML(const QDomElement& root, BosonCanvas* canvas);

    BoVector2Fixed lastTargetPos;
};


#endif

/*
 * vim: et sw=2
 */
