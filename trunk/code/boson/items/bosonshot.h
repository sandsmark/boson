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
class BosonWeaponProperties;

class QDomElement;


/** @short Base class for shots
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonShot : public BosonItem
{
  public:
    enum Type { Bullet = 0, Missile, Explosion, Mine, Bomb };

    /**
     * @param owner The player that shot. This is usually @ref Unit::owner of
     * the unit that is attacking.
     * @param canvas The @ref BosonCanvas object
     * @param prop The kind of weapon that fired this shot. Can be skipped
     **/
    BosonShot(Player* owner, BosonCanvas* canvas, const BosonWeaponProperties* prop = 0);

    Player* owner() const { return mOwner; }
    inline const BosonWeaponProperties* properties() const  { return mProp; };

    virtual long int damage() const;
    virtual float damageRange() const;
    virtual float fullDamageRange() const;

    inline bool isActive() const  { return mActive; };

    inline virtual int rtti() const  { return RTTI::Shot; }
    inline virtual int type() const = 0;

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
    virtual void save(QDataStream& stream);
    virtual void load(QDataStream& stream);

    virtual void explode();

  protected:
    virtual const QColor* teamColor() const;

    inline void setActive(bool a)  { mActive = a; };
    inline void setProperties(const BosonWeaponProperties* p)  { mProp = p; };

    virtual void advanceMoveInternal() {};
    virtual void advanceMoveCheck();

    virtual void moveToTarget() {};

  private:
    bool mActive;
    Player* mOwner;
    const BosonWeaponProperties* mProp;
};


class BosonShotBullet : public BosonShot
{
  public:
    BosonShotBullet(Player* owner, BosonCanvas* canvas, const BosonWeaponProperties* prop, BoVector3 target);
    BosonShotBullet(Player* owner, BosonCanvas* canvas, const BosonWeaponProperties* prop);

    // Bullets won't be saved becaused they will immediately "explode" and
    //  become inactive
    virtual bool saveAsXML(QDomElement&) { return true; };
    virtual bool loadFromXML(const QDomElement&) { setActive(false); return true; };
    virtual void save(QDataStream&) {};
    virtual void load(QDataStream&) { setActive(false); };

    inline virtual int type() const { return BosonShot::Bullet; };

  protected:
    virtual void moveToTarget();

  private:
    BoVector3 mTarget;
};


class BosonShotMissile : public BosonShot
{
  public:
    BosonShotMissile(Player* owner, BosonCanvas* canvas, const BosonWeaponProperties* prop, BoVector3 pos, BoVector3 target);
    BosonShotMissile(Player* owner, BosonCanvas* canvas, const BosonWeaponProperties* prop);

    virtual bool saveAsXML(QDomElement& root);
    virtual bool loadFromXML(const QDomElement& root);
    virtual void save(QDataStream& stream);
    virtual void load(QDataStream& stream);

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


class BosonShotExplosion : public BosonShot
{
  public:
    BosonShotExplosion(Player* owner, BosonCanvas* canvas, BoVector3 pos, long int damage, float damagerange, float fulldamagerange, int delay);
    BosonShotExplosion(Player* owner, BosonCanvas* canvas);

    virtual bool saveAsXML(QDomElement& root);
    virtual bool loadFromXML(const QDomElement& root);
    virtual void save(QDataStream& stream);
    virtual void load(QDataStream& stream);

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


class BosonShotMine : public BosonShot
{
  public:
    BosonShotMine(Player* owner, BosonCanvas* canvas, const BosonWeaponProperties* prop, BoVector3 pos);
    BosonShotMine(Player* owner, BosonCanvas* canvas, const BosonWeaponProperties* prop);

    virtual bool saveAsXML(QDomElement& root);
    virtual bool loadFromXML(const QDomElement& root);
    virtual void save(QDataStream& stream);
    virtual void load(QDataStream& stream);

    inline virtual int type() const { return BosonShot::Mine; };

  protected:
    virtual void advanceMoveInternal();

  private:
    bool mActivated;
};


class BosonShotBomb : public BosonShot
{
  public:
    BosonShotBomb(Player* owner, BosonCanvas* canvas, const BosonWeaponProperties* prop, BoVector3 pos);
    BosonShotBomb(Player* owner, BosonCanvas* canvas, const BosonWeaponProperties* prop);

    virtual bool saveAsXML(QDomElement& root);
    virtual bool loadFromXML(const QDomElement& root);
    virtual void save(QDataStream& stream);
    virtual void load(QDataStream& stream);

    inline virtual int type() const { return BosonShot::Bomb; };

  protected:
    virtual void advanceMoveInternal();
};


#endif // BOSONSHOT_H
/*
 * vim:et sw=2
 */
