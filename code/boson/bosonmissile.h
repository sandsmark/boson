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

#ifndef BOSONMISSILE_H
#define BOSONMISSILE_H

#include "bo3dtools.h"

class Unit;
class Player;

class BosonMissile
{
  public:
    //enum Type { Bullet = 1, Missile };

    BosonMissile(Unit* attacker, float x, float y, float z);
    BosonMissile(Unit* attacker, Unit* target);

    void update();
    void draw();
    
    inline BoVector3 pos() { return mPos; };

    inline unsigned long int damageRange() { return mDamageRange; };
    inline long int damage() { return mDamage; };
    inline Player* owner() { return mOwner; };

    inline bool isActive() { return mActive; };

  protected:
    void init(Unit* attacker, float x, float y, float z);
    BoVector3 mVelo;
    BoVector3 mPos;
    unsigned int mSteps;
    unsigned long int mDamageRange;
    long int mDamage;
    bool mActive;
    Player* mOwner;
};

#endif // BOSONMISSILE_H
