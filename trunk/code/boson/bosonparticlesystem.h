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

#ifndef BOSONPARTICLESYSTEM_H
#define BOSONPARTICLESYSTEM_H

#include "bo3dtools.h"

class BosonParticle
{
  public:
    BosonParticle();
    BosonParticle(BoVector4 color, BoVector3 pos, BoVector3 velo, float size, float life = 0);
    ~BosonParticle();

    void update(float elapsed);
    void reset();

    BoVector4 color;  // Current color of particle
    BoVector3 pos;  // Current pos
    BoVector3 velo;  // Velocity per second
    float size;  // Current size
    float life;  // How many remaining seconds particles has left to live, before it dies
    float maxage;
};

class BosonParticleSystem
{
  public:
    typedef void (*ExternalFunction)(BosonParticleSystem* system, BosonParticle* particle);
    BosonParticleSystem(int maxnum, int initialnum, float size,
        float createrate, bool align, float maxradius, int texture,
        BoVector4 color, float particleage, float age, BoVector3 pos, BoVector3 velo,
        ExternalFunction initFunc = 0, ExternalFunction updateFunc = 0,
        ExternalFunction deleteFunc = 0);
    BosonParticleSystem(int maxnum, int initialnum, float createrate,
        bool align, float maxradius, int texture, ExternalFunction initFunc,
        ExternalFunction updateFunc, ExternalFunction deleteFunc = 0);
    virtual ~BosonParticleSystem();

    virtual void update(float elapsed);
    virtual void draw();

    virtual void initParticle(BosonParticle* particle);
    inline virtual void updateParticle(BosonParticle* particle) { if(mUpdateFunc) (*mUpdateFunc)(this, particle); };
    inline virtual void deleteParticle(BosonParticle* particle) { if(mDeleteFunc) (*mDeleteFunc)(this, particle); };

    inline void setPosition(BoVector3 v) { mPos = v; };
    inline BoVector3 position() { return mPos; };
    
    inline float size() { return mSize; };
    inline void setSize(float s) { mSize = s; };

    inline float age() { return mAge; };
    inline void setAge(float a) { mAge = a; };

    inline float boundingSphereRadius()  { return mRadius; };
    inline bool isActive()  { return (mNum > 0); };

    inline void setBlendFunc(int f1, int f2) { mBlendFunc[0] = f1; mBlendFunc[1] = f2; };

    inline int particleCount() { return mNum; };

    void createParticles(int count);

  protected:
    virtual void init(int initialnum);

  private:
    BosonParticle* mParticles;  // Array of particles
    int mMaxNum;  // Maximum number of particles
    int mNum;  // Current number of particle (aka number of active particles)
    float mSize;  // Size of particles
    float mCreateRate;  // Number of particles created per second
    float mCreateCache;  // Number of particles to create during next update
    bool mAlign;  // Whether to align particles to camera
    float mRadius;  // Radius of bounding sphere
    int mTexture;  // Texture of particles
    BoVector3 mPos;
    BoVector3 mVelo;
    float mAge;
    int mBlendFunc[2];

    BoVector4 mColor;  // Color of particle
    float mParticleAge;  // How many seconds particles live

    ExternalFunction mInitFunc;
    ExternalFunction mUpdateFunc;
    ExternalFunction mDeleteFunc;
};

#endif // BOSONPARTICLESYSTEM_H
