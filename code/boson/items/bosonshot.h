/*
    This file is part of the Boson game
    Copyright (C) 2002-2004 The Boson Team (boson-devel@lists.sourceforge.net)

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

#ifndef BOSONSHOT_H
#define BOSONSHOT_H

#include "../bo3dtools.h"
#include "../rtti.h"
#include "bosonitem.h"
#include <qptrlist.h>

#include <kgame/kgameproperty.h>

class Unit;
class Player;
class BosonWeaponProperties;
class UnitProperties;

class QDomElement;
template<class T> class QPtrList;


/** @short Base class for shots
 *
 * This is the base class for all shots.
 * Shots are not only bullets/missiles but basically everything that can damage
 * units.
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonShot : public BosonItem
{
  public:
    enum PropertyIds
    {
        // BosonShot uses IDs from 512 to 1023
        // (derived classes use 1024 to 28671)
    };

    /**
     * Type of the shot
     **/
    enum Type
    {
      Bullet = 0,
      Missile,
      Explosion,
      Mine,
      Bomb,
      Fragment
    };

    /**
     * @param owner The player that shot. This is usually @ref Unit::owner of
     * the unit that is attacking.
     * @param canvas The @ref BosonCanvas object
     * @param prop The kind of weapon that fired this shot.
     **/
    BosonShot(Player* owner, BosonCanvas* canvas, const BosonWeaponProperties* prop);

    /**
     * Same as above, except that it meant to be used when you don't have weapon
     * properties, but still want to use a model
     **/
    BosonShot(Player* owner, BosonCanvas* canvas, BosonModel* model);

    BosonShot(Player* owner, BosonCanvas* canvas);
    ~BosonShot();

    virtual BosonModel* getModelForItem() const;
    virtual bool init();

    /**
     * @return Weapon properties of this shot if it has one.
     * Note that it's perfectly legal to return NULL pointer here, so you should
     * always check it before doing anything with it.
     **/
    inline const BosonWeaponProperties* properties() const  { return mProp; }

    /**
     * @return Damage made by this shot when it explodes.
     * If you supply weapon properties in ctor, this equals to properties()->damage().
     * Otherwise, you should re-implement it in derived classes
     **/
    virtual long int damage() const;
    /**
     * @return Damage range of this shot.
     * If you supply weapon properties in ctor, this equals to properties()->damageRange().
     * Otherwise, you should re-implement it in derived classes
     **/
    virtual bofixed damageRange() const;
    /**
     * @return Full damage range of this shot.
     * If you supply weapon properties in ctor, this equals to properties()->fullDamageRange().
     * Otherwise, you should re-implement it in derived classes
     **/
    virtual bofixed fullDamageRange() const;

    /**
     * @return If this shot is active.
     * Once shot has exploded, it becomes inactive. Inactive shots can be
     * deleted.
     **/
    inline bool isActive() const  { return mActive; }

    inline virtual int rtti() const  { return RTTI::Shot; }
    virtual int type() const = 0;

    /**
     * A shot is always moving, so this does a permanent @ref
     * advanceMoveInternal and @ref advanceMoveCheck.
     **/
    inline virtual void advanceFunction(unsigned int)
    {
      if(!isActive())
      {
        return;
      }
      advanceMoveInternal();
      advanceMoveCheck();
    }
    /**
     * See @ref advanceMoveFunction. This does exactly the same.
     **/
    inline virtual void advanceFunction2(unsigned int)
    {
      if(!isActive())
      {
        return;
      }
      advanceMoveInternal();
      advanceMoveCheck();
    }

    virtual bool saveAsXML(QDomElement& root);
    virtual bool loadFromXML(const QDomElement& root);

    virtual void explode();

  private:
    static void initStatic();

  protected:
    virtual const QColor* teamColor() const;

    inline void setActive(bool a)  { mActive = a; }
    inline void setProperties(const BosonWeaponProperties* p)  { mProp = p; }

    virtual void advanceMoveInternal() {}
    virtual void advanceMoveCheck();

    virtual void moveToTarget() {}

    bool mActive;
    const BosonWeaponProperties* mProp;
};


/**
 * @short Shot class for bullets
 *
 * Bullet is a shot with infinite speed, it immediately reaches it's target
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonShotBullet : public BosonShot
{
  public:
    enum PropertyIds
    {
    };

    BosonShotBullet(Player* owner, BosonCanvas* canvas, const BosonWeaponProperties* prop);
    virtual bool init();

    // Bullets won't be saved becaused they will immediately "explode" and
    //  become inactive
    virtual bool saveAsXML(QDomElement&) { return true; }
    virtual bool loadFromXML(const QDomElement&)
    {
      setActive(false);
      return true;
    }

    void setTarget(const BoVector3Fixed& target);

    inline virtual int type() const { return BosonShot::Bullet; }

    virtual void explode();

  protected:
    virtual void moveToTarget();

  private:
    static void initStatic();

  private:
    BoVector3Fixed mTarget;
};


/**
 * @short Shot class for missiles
 *
 * Missile is a shot that flies through the air from initial position to target
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonShotMissile : public BosonShot
{
  public:
    enum PropertyIds
    {
      IdTotalDist = 1024 + 0,
      IdPassedDist = 1024 + 1,
      IdZ = 1024 + 2,
      IdEffectVelocity = 1024 + 3,
      IdMaxHeight = 1024 + 4
    };

    BosonShotMissile(Player* owner, BosonCanvas* canvas, const BosonWeaponProperties* prop);
    ~BosonShotMissile();

    virtual bool saveAsXML(QDomElement& root);
    virtual bool loadFromXML(const QDomElement& root);

    void init(const BoVector3Fixed& pos, const BoVector3Fixed& target);

    inline virtual int type() const { return BosonShot::Missile; }

  protected:
    virtual void advanceMoveInternal();

    virtual void moveToTarget();

  private:
    static void initStatic();

  private:
    BoVector3Fixed mVelo;
    BoVector3Fixed mTarget;
    KGameProperty<bofixed> mTotalDist;
    KGameProperty<bofixed> mPassedDist;
    KGameProperty<bofixed> mZ;
    KGameProperty<bofixed> mMaxHeight;
    bofixed mEffectVelo;
};


/**
 * @short Shot class for explosion
 *
 * Explosion is a shot which will explode after given time.
 * It has no visual appearance.
 * Used when units explode (you can make nice chain reactions with it ;-))
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonShotExplosion : public BosonShot
{
  public:
    enum PropertyIds
    {
      IdDamage = 1024 + 0,
      IdDamageRange = 1024 + 1,
      IdFullDamageRange  = 1024 + 2,
      IdDelay = 1024 + 3
    };

    BosonShotExplosion(Player* owner, BosonCanvas* canvas);

    void activate(const BoVector3Fixed& pos, long int damange, bofixed damageRange, bofixed fulldamagerange, int delay);

    virtual bool saveAsXML(QDomElement& root);
    virtual bool loadFromXML(const QDomElement& root);

    inline virtual int type() const { return BosonShot::Explosion; }

    virtual long int damage() const  { return mDamage; }
    virtual bofixed damageRange() const  { return mDamageRange; }
    virtual bofixed fullDamageRange() const  { return mFullDamageRange; }

  protected:
    virtual void advanceMoveInternal();

  private:
    static void initStatic();

  private:
    KGameProperty<long int> mDamage;
    KGameProperty<bofixed> mDamageRange;
    KGameProperty<bofixed> mFullDamageRange;
    KGameProperty<int> mDelay;
};


/**
 * @short Shot class for mines
 *
 * Mine is an item that will explode when it collides with any other item.
 * Until then, mine doesn't move
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonShotMine : public BosonShot
{
  public:
    enum PropertyIds
    {
      IdActivated = 1024 + 0,
    };

    BosonShotMine(Player* owner, BosonCanvas* canvas, const BosonWeaponProperties* prop);

    virtual bool saveAsXML(QDomElement& root);
    virtual bool loadFromXML(const QDomElement& root);

    void init(const BoVector3Fixed& pos);

    inline virtual int type() const { return BosonShot::Mine; }

  protected:
    virtual void advanceMoveInternal();

  private:
    static void initStatic();

  private:
    KGamePropertyBool mActivated;
};


/**
 * @short Shot class for bombs
 *
 * Bomb is basically like a mine, except that it should be dropped from planes
 * and it also explodes when it touches ground
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonShotBomb : public BosonShot
{
  public:
    enum PropertyIds
    {
      IdActivated = 1024 + 0,
    };

    BosonShotBomb(Player* owner, BosonCanvas* canvas, const BosonWeaponProperties* prop);

    virtual bool saveAsXML(QDomElement& root);
    virtual bool loadFromXML(const QDomElement& root);

    void init(const BoVector3Fixed& pos);

    inline virtual int type() const { return BosonShot::Bomb; }

  protected:
    virtual void advanceMoveInternal();

  private:
    static void initStatic();

  private:
    KGamePropertyBool mActivated;
};


/**
 * @short Shot class for fragments
 *
 * Fragment is actually not a shot, it's an explosion fragment.
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonShotFragment : public BosonShot
{
  public:
    BosonShotFragment(Player* owner, BosonCanvas* canvas);
    ~BosonShotFragment();

    virtual BosonModel* getModelForItem() const;

    void activate(const BoVector3Fixed& pos, const UnitProperties* unitProperties);

    virtual bool saveAsXML(QDomElement& root);
    virtual bool loadFromXML(const QDomElement& root);

    virtual long int damage() const;
    virtual bofixed damageRange() const;
    virtual bofixed fullDamageRange() const;

    inline virtual int type() const { return BosonShot::Fragment; }

    virtual void explode();

  protected:
    virtual void advanceMoveInternal();

  private:
    static void initStatic();

  private:
    BoVector3Fixed mVelo;
    const UnitProperties* mUnitProperties;
};


#endif // BOSONSHOT_H
/*
 * vim:et sw=2
 */
