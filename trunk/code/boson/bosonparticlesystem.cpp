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

#include "bosonparticlesystem.h"

#include "bo3dtools.h"
#include "bosonparticlesystemproperties.h"
#include "bodebug.h"

#include <GL/gl.h>


/*****  BoParticleManager  *****/


int BoParticleList::compareItems(QPtrCollection::Item item1, QPtrCollection::Item item2)
{
  float d1, d2;
  d1 = ((BosonParticle*)item1)->distance;
  d2 = ((BosonParticle*)item2)->distance;
  if(d1 > d2)
  {
    return -1;
  }
  else if(d1 == d2)
  {
    return 0;
  }
  else
  {
    return 1;
  }
}



/*****  BosonParticle  *****/

BosonParticle::BosonParticle()
{
  // Only reset life
  life = -1.0;
}

BosonParticle::~BosonParticle()
{
}

void BosonParticle::reset()
{
  color.reset();
  pos.reset();
  velo.reset();
  size = 0;
  life = -1.0;
  maxage = 0;
  tex = 0;
  system = 0l;
  distance = 0.0;
}

void BosonParticle::update(float elapsed)
{
  life -= elapsed;
  pos.addScaled(velo, elapsed);
}

/*****  BosonParticleSystem  *****/

BosonParticleSystem::BosonParticleSystem(int maxnum, int initialnum, float size,
    float createrate, bool align, float maxradius, BosonParticleTextureArray textures,
    BoVector4 color, float particleage, float age, BoVector3 pos, BoVector3 velo,
    const BosonParticleSystemProperties* prop)
{
  // Set some variables first
  mMaxNum = maxnum;
  if(initialnum > maxnum)
  {
    initialnum = maxnum;
  }
  mSize = size;
  mCreateRate = createrate;
  mAlign = align;
  mRadius = maxradius;
  mTextures = textures;
  mColor = color;
  mPos = pos;
  mParticleAge = particleage;
  mAge = age;
  mVelo = velo;
  mProp = prop;

  init(initialnum);
}

BosonParticleSystem::BosonParticleSystem(int maxnum,
    float createrate, bool align, float maxradius, BosonParticleTextureArray textures,
    const BosonParticleSystemProperties* prop)
{
  //cout << k_funcinfo << "CREATING PARTICLE SYSTEM.  maxnum: " <<  maxnum <<
      //"; createrate: " << createrate << endl;
  // Set some variables first
  mMaxNum = maxnum;
  mCreateRate = createrate;
  mAlign = align;
  mRadius = maxradius;
  mTextures = textures;
  mSize = 0;
  mParticleAge = 0;
  mAge = 3600;
  mProp = prop;

  init(0);
}

void BosonParticleSystem::init(int initialnum)
{
  // Some variables
  mCreateCache = 0.0;
  mBlendFunc[0] = GL_SRC_ALPHA;
  mBlendFunc[1] = GL_ONE_MINUS_SRC_ALPHA;

  // Create particles
  mParticles = new BosonParticle[mMaxNum];

  // Create initial particles
  mNum = 0;
  createParticles(initialnum);
//  boDebug(150) << k_funcinfo << "Created " << mNum << " initial particles" << endl;
}

void BosonParticleSystem::createParticles(int count)
{
  if(count > mMaxNum) 
  {
    count = mMaxNum;
  }
  for(int i = 0; i < count; i++)
  {
    if(mParticles[i].life <= 0.0)
    {
      initParticle(&mParticles[i]);
      mNum++;
    }
  }
}

BosonParticleSystem::~BosonParticleSystem()
{
  delete[] mParticles;
}

void BosonParticleSystem::update(float elapsed)
{
/*  boDebug(150) << k_funcinfo << " UPDATING; elapsed: " << elapsed << "; createRate: " << mCreateRate <<
      "; createCache: " << mCreateCache << "; will add " << elapsed * mCreateRate <<
      " to create cache (total will be = " << mCreateCache + (elapsed * mCreateRate) << ")" << endl;*/
  mCreateCache += (elapsed * mCreateRate);

  if((mCreateCache < 1.0) && (mNum <= 0))
  {
    return;
  }

  mNum = 0;
  mAge -= elapsed;

  // Update particles
  for(int i = 0; i < mMaxNum; i++)
  {
    if(mParticles[i].life > 0.0)
    {
      mParticles[i].update(elapsed);
      updateParticle(&mParticles[i]);
      // Check for death
      if(mParticles[i].life <= 0.0)
      {
        // For performance reasons we actually don't create/delete particles. We just mark them dead
        //uninitParticle(&mParticles[i]);
      }
      else
      {
        mNum++;
      }
    }
  }

  // Create some new ones if needed
  if((mCreateCache >= 1.0) && (mAge >= 0.0))
  {
//    boDebug(150) << k_funcinfo << "createCache >= 1.0 (" << mCreateCache << "); trying to create new particles" << endl;
    for(int i = 0; (i < mMaxNum) && (mCreateCache >= 1.0); i++)
    {
      if(mParticles[i].life <= 0.0)
      {
        // Dead particle, re-create it
        initParticle(&mParticles[i]);
        mCreateCache -= 1.0;
        mNum++;
      }
    }
//    boDebug(150) << k_funcinfo << "Created " << created << " new particles; createCache is now " << mCreateCache << endl;
  }
}

void BosonParticleSystem::initParticle(BosonParticle* particle)
{
  particle->color = mColor;
  particle->life = mParticleAge;
  particle->pos.reset();
  particle->size = mSize;
  particle->velo = mVelo;
  particle->tex = mTextures.mTextureIds[0];
  particle->system = this;
  if(mProp)
  {
    mProp->initParticle(this, particle);
  }
}

void BosonParticleSystem::updateParticle(BosonParticle* particle)
{
  if(mProp)
  {
    mProp->updateParticle(this, particle);
  }
}

void BosonParticleSystem::moveParticles(BoVector3 v)
{
  // Move particles by inverse of v (to have effect of them staying in same place)
  BoVector3 inv(-v[0], -v[1], -v[2]);
  for(int i = 0; i < mMaxNum; i++)
  {
    if(mParticles[i].life > 0.0)
    {
      mParticles[i].pos.add(inv);
    }
  }
}

/*
 * vim: et sw=2
 */
