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


/**
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonShot : public BosonItem
{
  public:
    /**
     * @param prop The kind of weapon that fired this shot
     * @param owner The player that shot. This is usually @ref Unit::owner of
     * the unit that is attacking.
     * @param canvas The @ref BosonCanvas object
     **/
    BosonShot(const BosonWeaponProperties* prop, Player* owner, BosonCanvas* canvas, BoVector3 pos, BoVector3 target);
    /**
     * Constructs new BosonShot without initializing it.
     * You should call @ref loadFromXML after it.
     **/
    BosonShot(const BosonWeaponProperties* prop, Player* owner, BosonCanvas* canvas);

//    inline BoVector3 pos()  { return mPos; }

    Player* owner() const { return mOwner; }
    inline const BosonWeaponProperties* properties() const  { return mProp; };

    inline bool isActive() const  { return mActive; };
    inline virtual int rtti() const  { return RTTI::Shot; }

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

    void advanceMoveInternal();
    void advanceMoveCheck();

    bool saveAsXML(QDomElement& root);
    bool loadFromXML(const QDomElement& root);
    void save(QDataStream& stream);
    void load(QDataStream& stream);

    virtual QPtrList<BosonParticleSystem>* particleSystems() const  { return mFlyParticleSystems; };

    virtual void explode();

  protected:
    virtual const QColor* teamColor() const;

  private:
    BoVector3 mVelo;
    BoVector3 mTarget;
    bool mActive;
    float mTotalDist;
    float mPassedDist;
    float mZ;
    float mParticleVelo;
    float mMaxHeight;
    Player* mOwner;
    const BosonWeaponProperties* mProp;
    QPtrList<BosonParticleSystem>* mFlyParticleSystems;
};

#endif // BOSONSHOT_H
/*
 * vim:et sw=2
 */
