/*
    This file is part of the Boson game
    Copyright (C) 2002-2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

class Unit;
class Player;
class BosonParticleSystem;
class BosonParticleSystemProperties;
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
    /**
     * Type of the shot
     **/
    enum Type { Bullet = 0, Missile, Explosion, Mine, Bomb, Fragment };

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

    /**
     * @return Owner of this shot, i.e. the player whose unit fired the shot.
     * Used for statistics
     **/
    Player* owner() const { return mOwner; }
    /**
     * @return Weapon properties of this shot if it has one.
     * Note that it's perfectly legal to return NULL pointer here, so you should
     * always check it before doing anything with it.
     **/
    inline const BosonWeaponProperties* properties() const  { return mProp; };

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
    virtual float damageRange() const;
    /**
     * @return Full damage range of this shot.
     * If you supply weapon properties in ctor, this equals to properties()->fullDamageRange().
     * Otherwise, you should re-implement it in derived classes
     **/
    virtual float fullDamageRange() const;

    /**
     * @return If this shot is active.
     * Once shot has exploded, it becomes inactive. Inactive shots can be
     * deleted.
     **/
    inline bool isActive() const  { return mActive; };

    inline virtual int rtti() const  { return RTTI::Shot; }
    virtual int type() const = 0;

    /**
     * A shot is always moving, so this does a permanent @ref
     * advanceMoveInternal and @ref advanceMoveCheck.
     **/
    inline virtual void advanceFunction(unsigned int)
    {
      if(!isActive()) { return; }
      advanceMoveInternal();
      advanceMoveCheck();
    }
    /**
     * See @ref advanceMoveFunction. This does exactly the same.
     **/
    inline virtual void advanceFunction2(unsigned int)
    {
      if(!isActive()) { return; }
      advanceMoveInternal();
      advanceMoveCheck();
    }

    virtual bool saveAsXML(QDomElement& root);
    virtual bool loadFromXML(const QDomElement& root);
    virtual bool save(QDataStream& stream);
    virtual bool load(QDataStream& stream);

    virtual void explode();

  protected:
    virtual const QColor* teamColor() const;

    inline void setActive(bool a)  { mActive = a; };
    inline void setProperties(const BosonWeaponProperties* p)  { mProp = p; };

    virtual void advanceMoveInternal() {};
    virtual void advanceMoveCheck();

    virtual void moveToTarget() {};

  private:
    void init();

    bool mActive;
    Player* mOwner;
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
    BosonShotBullet(Player* owner, BosonCanvas* canvas, const BosonWeaponProperties* prop, BoVector3 target);
    BosonShotBullet(Player* owner, BosonCanvas* canvas, const BosonWeaponProperties* prop);

    // Bullets won't be saved becaused they will immediately "explode" and
    //  become inactive
    virtual bool saveAsXML(QDomElement&) { return true; };
    virtual bool loadFromXML(const QDomElement&) { setActive(false); return true; };
    virtual bool save(QDataStream&) { return true; }
    virtual bool load(QDataStream&) { setActive(false); return true; }

    inline virtual int type() const { return BosonShot::Bullet; };

  protected:
    virtual void moveToTarget();

  private:
    BoVector3 mTarget;
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
    BosonShotMissile(Player* owner, BosonCanvas* canvas, const BosonWeaponProperties* prop, BoVector3 pos, BoVector3 target);
    BosonShotMissile(Player* owner, BosonCanvas* canvas, const BosonWeaponProperties* prop);

    virtual bool saveAsXML(QDomElement& root);
    virtual bool loadFromXML(const QDomElement& root);
    virtual bool save(QDataStream& stream);
    virtual bool load(QDataStream& stream);

    virtual QPtrList<BosonParticleSystem>* particleSystems() const  { return mFlyParticleSystems; };

    inline virtual int type() const { return BosonShot::Missile; };

  protected:
    virtual void advanceMoveInternal();

    virtual void moveToTarget();

  private:
    BoVector3 mVelo;
    BoVector3 mTarget;
    float mTotalDist;
    float mPassedDist;
    float mZ;
    float mParticleVelo;
    float mMaxHeight;
    QPtrList<BosonParticleSystem>* mFlyParticleSystems;
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
    BosonShotExplosion(Player* owner, BosonCanvas* canvas, BoVector3 pos, long int damage, float damagerange, float fulldamagerange, int delay);
    BosonShotExplosion(Player* owner, BosonCanvas* canvas);

    virtual bool saveAsXML(QDomElement& root);
    virtual bool loadFromXML(const QDomElement& root);
    virtual bool save(QDataStream& stream);
    virtual bool load(QDataStream& stream);

    inline virtual int type() const { return BosonShot::Explosion; };

    virtual long int damage() const  { return mDamage; };
    virtual float damageRange() const  { return mDamageRange; };
    virtual float fullDamageRange() const  { return mFullDamageRange; };

  protected:
    virtual void advanceMoveInternal();

  private:
    long int mDamage;
    float mDamageRange;
    float mFullDamageRange;
    int mDelay;
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
    BosonShotMine(Player* owner, BosonCanvas* canvas, const BosonWeaponProperties* prop, BoVector3 pos);
    BosonShotMine(Player* owner, BosonCanvas* canvas, const BosonWeaponProperties* prop);

    virtual bool saveAsXML(QDomElement& root);
    virtual bool loadFromXML(const QDomElement& root);
    virtual bool save(QDataStream& stream);
    virtual bool load(QDataStream& stream);

    inline virtual int type() const { return BosonShot::Mine; };

  protected:
    virtual void advanceMoveInternal();

  private:
    bool mActivated;
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
    BosonShotBomb(Player* owner, BosonCanvas* canvas, const BosonWeaponProperties* prop, BoVector3 pos);
    BosonShotBomb(Player* owner, BosonCanvas* canvas, const BosonWeaponProperties* prop);

    virtual bool saveAsXML(QDomElement& root);
    virtual bool loadFromXML(const QDomElement& root);
    virtual bool save(QDataStream& stream);
    virtual bool load(QDataStream& stream);

    inline virtual int type() const { return BosonShot::Bomb; };

  protected:
    virtual void advanceMoveInternal();

  private:
    bool mActivated;
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
    BosonShotFragment(Player* owner, BosonCanvas* canvas, BosonModel* model, BoVector3 pos,
        const UnitProperties* unitproperties);
    BosonShotFragment(Player* owner, BosonCanvas* canvas, BosonModel* model);

    virtual bool saveAsXML(QDomElement& root);
    virtual bool loadFromXML(const QDomElement& root);
    virtual bool save(QDataStream& stream);
    virtual bool load(QDataStream& stream);

    virtual long int damage() const;
    virtual float damageRange() const;
    virtual float fullDamageRange() const;

    inline virtual int type() const { return BosonShot::Fragment; };

    virtual QPtrList<BosonParticleSystem>* particleSystems() const  { return mParticleSystems; };

    virtual void explode();

  protected:
    virtual void advanceMoveInternal();

  private:
    QPtrList<BosonParticleSystem>* mParticleSystems;
    BoVector3 mVelo;
    const UnitProperties* mUnitProperties;
};


#endif // BOSONSHOT_H
/*
 * vim:et sw=2
 */
