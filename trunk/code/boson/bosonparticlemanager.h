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

#ifndef BOSONPARTICLEMANAGER_H
#define BOSONPARTICLEMANAGER_H

#include <krandomsequence.h>

#include "bo3dtools.h"

class BosonTextureArray;
class BosonParticleSystem;
class BosonParticle;
class QString;

class BosonParticleManager
{
  public:
    static void loadTextures(QString texdir);

    enum Type { None = 0, Explosion = 1, Smoke = 2, Shot = 3, LastType };

    static BosonParticleSystem* newSystem(BoVector3 pos, Type type);

    inline static BosonParticleSystem* newExplosion(BoVector3 pos) { return newSystem(pos, Explosion); };
    inline static BosonParticleSystem* newExplosion(float x, float y, float z) { return newSystem(BoVector3(x, y, z), Explosion); };
    inline static BosonParticleSystem* newSmoke(BoVector3 pos) { return newSystem(pos, Smoke); };
    inline static BosonParticleSystem* newSmoke(float x, float y, float z) { return newSystem(BoVector3(x, y, z), Smoke); };
    inline static BosonParticleSystem* newShot(BoVector3 pos) { return newSystem(pos, Shot); };
    inline static BosonParticleSystem* newShot(float x, float y, float z) { return newSystem(BoVector3(x, y, z), Shot); };

    static void initExplosionParticle(BosonParticleSystem* system, BosonParticle* particle);
    static void updateExplosionParticle(BosonParticleSystem* system, BosonParticle* particle);

    static void initSmokeParticle(BosonParticleSystem* system, BosonParticle* particle);
    static void updateSmokeParticle(BosonParticleSystem* system, BosonParticle* particle);

    static void initShotParticle(BosonParticleSystem* system, BosonParticle* particle);
    static void updateShotParticle(BosonParticleSystem* system, BosonParticle* particle);
    
    inline static float getFloat(float min, float max) { return ((float)(mRandom->getDouble())) * (max - min) + min; };

  protected:
    static BosonTextureArray* mTextures;
    static KRandomSequence* mRandom;
};

#endif // BOSONPARTICLEMANAGER_H
