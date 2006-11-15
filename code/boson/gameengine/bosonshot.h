/*
    This file is part of the Boson game
    Copyright (C) 2002-2004 Rivo Laks (rivolaks@hot.ee)

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
#include "rtti.h"
#include "bosonitem.h"
#include <qptrlist.h>

#include <kgame/kgameproperty.h>

class Unit;
class Player;
class BosonWeaponProperties;
class BosonWeapon;
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
        // BosonShot uses IDs from 4096 to 4159
        // derived classes may use 4160 to 8191.
        // WARNING: because we need to add every Id with a name to the
        // (static!!) property map, IDs must be unique among different classes.
        // Therefore we must not use the same ID twice, even if that's in
        // different classes.
        IdWeaponDamage = 4096,
        IdWeaponDamageRange = 4097,
        IdWeaponFullDamageRange = 4098,
        IdWeaponSpeed = 4099
    };

    /**
     * Type of the shot
     **/
    enum Type
    {
      Bullet = 0,
      Rocket,
      Explosion,
      Mine,
      Bomb,
      Fragment,
      Missile
    };

    /**
     * @param owner The player that shot. This is usually @ref Unit::owner of
     * the unit that is attacking.
     * @param canvas The @ref BosonCanvas object
     * @param prop The kind of weapon that fired this shot.
     **/
    BosonShot(Player* owner, BosonCanvas* canvas, const BosonWeaponProperties* prop);

    BosonShot(Player* owner, BosonCanvas* canvas);
    ~BosonShot();

    virtual QString getModelIdForItem() const;
    virtual bool init();

    /**
     * Apply the values from @p weapon to this shot. @p weapon is the weapon
     * shooting this shot, its values (like @ref damage) are derived to this
     * object.
     **/
    virtual void applyWeapon(const BosonWeapon* weapon);

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

    void setInactive();
    inline void setProperties(const BosonWeaponProperties* p)  { mProp = p; }

    virtual void advanceMoveInternal() {}
    virtual void advanceMoveCheck();

    virtual void moveToTarget() {}

  protected:
    bool mActive;
    const BosonWeaponProperties* mProp;

    // values from BosonWeapon. note that some classes may not use them at all
    // (e.g. reimplement damage() with their own values)
    KGameProperty<Q_INT32> mWeaponDamage;
    KGameProperty<bofixed> mWeaponDamageRange;
    KGameProperty<bofixed> mWeaponFullDamageRange;
    KGameProperty<bofixed> mWeaponSpeed;

  private:
    void initPrivate();
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
      // 4160 to 4223 (4160+63) allowed here
    };

    BosonShotBullet(Player* owner, BosonCanvas* canvas, const BosonWeaponProperties* prop);
    virtual bool init();

    // Bullets won't be saved becaused they will immediately "explode" and
    //  become inactive
    virtual bool saveAsXML(QDomElement&) { return true; }
    virtual bool loadFromXML(const QDomElement&)
    {
      setInactive();
      return true;
    }

    void setTarget(const BoVector3Fixed& target);

    inline virtual int type() const { return BosonShot::Bullet; }

  protected:
    virtual void moveToTarget();

  private:
    static void initStatic();

  private:
    BoVector3Fixed mTarget;
};


/**
 * @short Shot class for rockets
 *
 * Rockets is a shot that flies through the air from initial position to target.
 *
 * Note that rockets are not guided.
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonShotRocket : public BosonShot
{
  public:
    enum PropertyIds
    {
      // 4224 to 4287 (4224+63) allowed here
      IdTotalDist = 4224 + 0,
      IdPassedDist = 4224 + 1,
      IdZ = 4224 + 2,
      IdEffectVelocity = 4224 + 3,
      IdMaxHeight = 4224 + 4
    };

    BosonShotRocket(Player* owner, BosonCanvas* canvas, const BosonWeaponProperties* prop);
    ~BosonShotRocket();

    virtual bool saveAsXML(QDomElement& root);
    virtual bool loadFromXML(const QDomElement& root);

    void init(const BoVector3Fixed& pos, const BoVector3Fixed& target);

    inline virtual int type() const { return BosonShot::Rocket; }

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
    bofixed mLastDistToTarget;
};


/**
 * @short Shot class for missile
 *
 * Missile is a shot that flies trough the air, trying to reach it's target.
 *
 * Missiles are guided, that is, they will follow their targets.
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonShotMissile : public BosonShot
{
  public:
    enum PropertyIds
    {
      // 4544 to 4607 (4544+63) allowed here
      IdPassedDist = 4544 + 0,
    };

    BosonShotMissile(Player* owner, BosonCanvas* canvas, const BosonWeaponProperties* prop);
    ~BosonShotMissile();

    virtual bool saveAsXML(QDomElement& root);
    virtual bool loadFromXML(const QDomElement& root);

    void init(const BoVector3Fixed& pos, Unit* target);

    inline virtual int type() const { return BosonShot::Rocket; }

  protected:
    virtual void advanceMoveInternal();

    virtual void moveToTarget();

  private:
    static void initStatic();

  private:
    Unit* mTarget;
    BoVector3Fixed mTargetPos;
    KGameProperty<bofixed> mPassedDist;
    // This is _normalized_ velocity, i.e. just direction
    BoVector3Fixed mVelo;
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
      // 4288 to 4351 (4288+63) allowed here
      IdDamage = 4288 + 0,
      IdDamageRange = 4288 + 1,
      IdFullDamageRange  = 4288 + 2,
      IdDelay = 4288 + 3
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
    KGameProperty<Q_INT32> mDamage;
    KGameProperty<bofixed> mDamageRange;
    KGameProperty<bofixed> mFullDamageRange;
    KGameProperty<Q_INT32> mDelay;
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
      // 4352 to 4415 (4352+63) allowed here
      IdActivated = 4352 + 0,
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
      // 4416 to 4479 (4416+63) allowed here
      IdActivated = 4416 + 0,
    };

    BosonShotBomb(Player* owner, BosonCanvas* canvas, const BosonWeaponProperties* prop);

    virtual bool saveAsXML(QDomElement& root);
    virtual bool loadFromXML(const QDomElement& root);

    void init(const BoVector3Fixed& pos);

    void setHorizontalVelocity(const BoVector2Fixed& velo) { mHorizontalVelocity = velo; }

    inline virtual int type() const { return BosonShot::Bomb; }

  protected:
    virtual void advanceMoveInternal();

  private:
    static void initStatic();

  private:
    KGamePropertyBool mActivated;
    BoVector2Fixed mHorizontalVelocity;
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
    enum PropertyIds
    {
      // 4480 to 4543 (4480+63) allowed here
    };

    BosonShotFragment(Player* owner, BosonCanvas* canvas);
    ~BosonShotFragment();

    virtual QString getModelIdForItem() const;

    void activate(const BoVector3Fixed& pos, const UnitProperties* unitProperties);

    virtual bool saveAsXML(QDomElement& root);
    virtual bool loadFromXML(const QDomElement& root);

    virtual long int damage() const;
    virtual bofixed damageRange() const;
    virtual bofixed fullDamageRange() const;

    inline virtual int type() const { return BosonShot::Fragment; }

    const UnitProperties* unitProperties() const
    {
      return mUnitProperties;
    }

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
