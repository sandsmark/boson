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
#include <math.h>


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

BosonParticleSystem::BosonParticleSystem(int maxnum,
    float createrate, bool align, const BosonParticleTextureArray* textures,
    const BosonParticleSystemProperties* prop)
{
  // Set some variables first
  mMaxNum = maxnum;
  mCreateRate = createrate;
  mAlign = align;
  mTextures = textures;
  mAge = 3600;
  mMass = 1.0;
  mProp = prop;
  mMoveParticlesWithSystem = false;

  init(0);
}

void BosonParticleSystem::init(int initialnum)
{
  // Some variables
  mRadius = 0.0;
  mCreateCache = 0.0;
  mBlendFunc[0] = GL_SRC_ALPHA;
  mBlendFunc[1] = GL_ONE_MINUS_SRC_ALPHA;
  mRotated = false;

  // Create particles
  mParticles = new BosonParticle[mMaxNum];

  // Create initial particles
  mNum = 0;
  if(initialnum)
  {
    createParticles(initialnum);
  }
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
  mRadius = sqrt(mRadius);
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
  mRadius = 0.0;
  bool createnew;
  if(mAge == -1)
  {
    // -1 means forever
    createnew = (mCreateCache >= 1.0);
  }
  else if(mAge > 0.0)
  {
    // system still producing new particles
    mAge -= elapsed;
    if(mAge < 0.0)
    {
      mAge = 0.0;
    }
    createnew = (mCreateCache >= 1.0) && (mAge > 0.0);
  }
  else
  {
    // system not producing new particles anymore
    createnew = false;
  }

  // Update particles
  for(int i = 0; i < mMaxNum; i++)
  {
    if(mParticles[i].life > 0.0)
    {
      mParticles[i].update(elapsed);
      updateParticle(&mParticles[i]);
      // Check for death
      // For performance reasons particles aren't actually created/deleted.
      //  They're just marked as dead (aka inactive)
      if(mParticles[i].life > 0.0)
      {
        mNum++;
      }
    }
    else if(createnew)
    {
      // Dead particle, re-create it
      initParticle(&mParticles[i]);
      mCreateCache -= 1.0;
      mNum++;
      createnew = (mCreateCache >= 1.0);
    }
  }

  // Particle update and init methods set mRadius to dot product for performance
  //  reasons. Convert it here.
  mRadius = sqrt(mRadius);
}

void BosonParticleSystem::initParticle(BosonParticle* particle)
{
  // Note that most stuff isn't initialized here, it's done in
  //  BosonParticleSystemProperties
  particle->pos = mPos;
  if(!mTextures)
  {
    boError(150) << k_funcinfo << "NULL textures" << endl;
  }
  else if(!mTextures->mTextureIds)
  {
    boError(150) << k_funcinfo << "NULL texture array" << endl;
  }
  else
  {
    particle->tex = mTextures->mTextureIds[0];
  }
  particle->system = this;
  if(mProp)
  {
    mProp->initParticle(this, particle);
  }
  mRadius = QMAX(mRadius, (particle->pos - mPos).dotProduct());
}

void BosonParticleSystem::updateParticle(BosonParticle* particle)
{
  if(mProp)
  {
    mProp->updateParticle(this, particle);
    mRadius = QMAX(mRadius, (particle->pos - mPos).dotProduct());
  }
}

void BosonParticleSystem::setPosition(BoVector3 p)
{
  if(mMoveParticlesWithSystem)
  {
    BoVector3 diff = p - mPos;
    // Move all particles by diff
    for(int i = 0; i < mMaxNum; i++)
    {
      if(mParticles[i].life > 0.0)
      {
        mParticles[i].pos += diff;
      }
    }
  }
  mPos = p;
}

void BosonParticleSystem::setRotation(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
  mRotated = true; mMatrix.rotate(angle, x, y, z);
  if(mMoveParticlesWithSystem)
  {
    BoMatrix m;
    m.rotate(angle, x, y, z);
    BoVector3 newpos, oldpos;
    // Rotate all particles
    for(int i = 0; i < mMaxNum; i++)
    {
      if(mParticles[i].life > 0.0)
      {
        oldpos = mParticles[i].pos - mPos;
        m.transform(&newpos, &oldpos);
        mParticles[i].pos = newpos + mPos;
      }
    }
  }
}


/*
 * vim: et sw=2
 */
