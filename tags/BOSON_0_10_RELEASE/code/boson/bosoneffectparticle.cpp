/*
    This file is part of the Boson game
    Copyright (C) 2004 The Boson Team (boson-devel@lists.sourceforge.net)

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


#include "bosoneffectparticle.h"

#include <math.h>

#include "bosontexturearray.h"
#include "bosoneffectpropertiesparticle.h"
#include "bodebug.h"


/*****  BosonEffectParticle  *****/

BosonEffectParticle::BosonEffectParticle(const BosonEffectPropertiesParticle* prop) : BosonEffect()
{
  mProperties = prop;
  mBoundingSphereRadius = 0;
  mBlendFunc[0] = GL_SRC_ALPHA;
  mBlendFunc[1] = GL_ONE_MINUS_SRC_ALPHA;
  mParticleDist = 0;
  mParticleCount = 0;
  mAlign = true;
}

bool BosonEffectParticle::saveAsXML(QDomElement& root) const
{
  // Save mBlendFunc
  // Save mParticleDist
  // Save mParticleCount
  return true;
}

bool BosonEffectParticle::loadFromXML(const QDomElement& root)
{
  return true;
}


/*****  BosonEffectParticleGeneric  *****/

BosonEffectParticleGeneric::BosonEffectParticleGeneric(const BosonEffectPropertiesParticleGeneric* prop,
    int maxnum, const BosonTextureArray* textures) :
    BosonEffectParticle(prop)
{
  mParticles = new BosonGenericParticle[maxnum];
  mParticleCount = maxnum;
  mTextures = textures;

  // Init variables to defaults
  mAge = 3600;
  mMass = 1.0;
  mParticleDist = 0.0;
  mMoveParticlesWithSystem = false;
  mCreateCache = 0.0;
  mRate = 0;
  mBlendFunc[0] = GL_SRC_ALPHA;
  mBlendFunc[1] = GL_ONE_MINUS_SRC_ALPHA;
  mRotated = false;
  mNum = 0;
  mMaxParticleSize = 0.0;
}

BosonEffectParticleGeneric::~BosonEffectParticleGeneric()
{
  delete[] mParticles;
}

void BosonEffectParticleGeneric::update(float elapsed)
{
/*  boDebug(150) << k_funcinfo << " UPDATING; elapsed: " << elapsed << "; rate: " << mRate <<
      "; createCache: " << mCreateCache << "; will add " << elapsed * mRate <<
      " to create cache (total will be = " << mCreateCache + (elapsed * mRate) << ")" << endl;*/
  mCreateCache += (elapsed * mRate);

  if((mCreateCache < 1.0) && (mNum <= 0))
  {
    return;
  }

  mNum = 0;
  mBoundingSphereRadius = 0.0;
  bool createnew;

  // Check if new particles should be created
  if(mAge == -1)
  {
    // -1 means to create new particles forever
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
  for(unsigned int i = 0; i < mParticleCount; i++)
  {
    if(mParticles[i].life > 0.0)
    {
      updateParticle(&mParticles[i], elapsed);
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
  mBoundingSphereRadius = sqrt(mBoundingSphereRadius) + mMaxParticleSize;
}

void BosonEffectParticleGeneric::setRotation(const BoVector3& rotation)
{
  if(!mRotated && rotation.isNull())
  {
    return;
  }

  if(mMoveParticlesWithSystem)
  {
    BoMatrix transformmatrix;
    // If the system was already rotated, we need to first "unrotate" it and
    //  then set new rotation, otherwise we'll add new rotation to the old one
    //  and get invalid results.
    // To do this, we first calculate inverse matrix of old rotation matrix and
    //  then add new rotation matrix to it and then transform particles by
    //  resulting matrix.
    if(mRotated)
    {
      transformmatrix.rotate(-mRotation.y(), 0.0f, 1.0f, 0.0f);
      transformmatrix.rotate(-mRotation.x(), 1.0f, 0.0f, 0.0f);
      transformmatrix.rotate(mRotation.z(), 0.0f, 0.0f, 1.0f);
      /*BoMatrix inverse;
      if(mMatrix.invert(&inverse))
      {
        bool isInverse = inverse.isEqual(transformmatrix);
        if(isInverse)
        {
          boDebug() << k_funcinfo << "tfm IS inverse matrix!!!" << endl;
        }
        else
        {
          boDebug() << k_funcinfo << "tfm is not inverse matrix" << endl;
        }
      }
      else
      {
        boDebug() << k_funcinfo << "couldn't get inverse matrix" << endl;
      }*/
      /*mMatrix.invert(&transformmatrix);*/
    }
    transformmatrix.rotate(-rotation.z(), 0.0f, 0.0f, 1.0f);
    transformmatrix.rotate(rotation.x(), 1.0f, 0.0f, 0.0f);
    transformmatrix.rotate(rotation.y(), 0.0f, 1.0f, 0.0f);

    BoVector3 newpos, oldpos;
    // Rotate all particles
    for(unsigned int i = 0; i < mParticleCount; i++)
    {
      if(mParticles[i].life > 0.0)
      {
        oldpos = mParticles[i].pos - mPosition;
        transformmatrix.transform(&newpos, &oldpos);
        mParticles[i].pos = newpos + mPosition;
      }
    }
  }

  mMatrix.loadIdentity();
  mMatrix.rotate(-rotation.z(), 0.0f, 0.0f, 1.0f);
  mMatrix.rotate(rotation.x(), 1.0f, 0.0f, 0.0f);
  mMatrix.rotate(rotation.y(), 0.0f, 1.0f, 0.0f);

  mRotation = rotation;

  mRotated = true;
}

void BosonEffectParticleGeneric::setPosition(const BoVector3& pos)
{
  if(mMoveParticlesWithSystem)
  {
    BoVector3 diff = pos - mPosition;
    // Move all particles by diff
    for(unsigned int i = 0; i < mParticleCount; i++)
    {
      if(mParticles[i].life > 0.0)
      {
        mParticles[i].pos += diff;
      }
    }
  }
  BosonEffect::setPosition(pos);
}

void BosonEffectParticleGeneric::createParticles(int count)
{
  for(unsigned int i = 0; i < mParticleCount; i++)
  {
    if(mParticles[i].life <= 0.0)
    {
      initParticle(&mParticles[i]);
      mNum++;
      if(mNum >= count)
      {
        break;
      }
    }
  }
  mBoundingSphereRadius = sqrt(mBoundingSphereRadius) + mMaxParticleSize;
}

void BosonEffectParticleGeneric::initParticle(BosonGenericParticle* particle)
{
  // Note that most stuff isn't initialized here, it's done in
  //  BosonParticleSystemProperties
  particle->pos = mPosition;
  if(!mTextures)
  {
    boError(150) << k_funcinfo << "NULL textures" << endl;
  }
  else if(!mTextures->isValid())
  {
    boError(150) << k_funcinfo << "invalid texture array" << endl;
  }
  else
  {
    particle->tex = mTextures->texture(0);
  }
  particle->system = this;
  if(properties())
  {
    ((BosonEffectPropertiesParticleGeneric*)properties())->initParticle(this, particle);
  }
  mBoundingSphereRadius = QMAX(mBoundingSphereRadius, (particle->pos - mPosition).dotProduct());
}

void BosonEffectParticleGeneric::updateParticle(BosonGenericParticle* particle, float elapsed)
{
  particle->life -= elapsed;
  particle->pos.addScaled(particle->velo, elapsed);

  if(properties())
  {
    ((BosonEffectPropertiesParticleGeneric*)properties())->updateParticle(this, particle);
    mBoundingSphereRadius = QMAX(mBoundingSphereRadius, (particle->pos - mPosition).dotProduct());
  }
}

bool BosonEffectParticleGeneric::saveAsXML(QDomElement& root) const
{
  return true;
}

bool BosonEffectParticleGeneric::loadFromXML(const QDomElement& root)
{
  return true;
}


/*****  BosonEffectParticleTrail  *****/

BosonEffectParticleTrail::BosonEffectParticleTrail(const BosonEffectPropertiesParticleTrail* prop,
    int maxnum, const BosonTextureArray* textures, const BoVector3& pos) :
    BosonEffectParticle(prop)
{
  mParticles = new BosonTrailParticle[maxnum];
  mParticleCount = maxnum;
  mTextures = textures;

  // Init variables to defaults
  mMass = 1.0;
  mParticleDist = 0.0;
  mCreateCache = 0.0;
  mBlendFunc[0] = GL_SRC_ALPHA;
  mBlendFunc[1] = GL_ONE_MINUS_SRC_ALPHA;
  mRotated = false;
  mNum = 0;
  mObsolete = false;

  mLastPos = pos;
  setPosition(pos);
}

BosonEffectParticleTrail::~BosonEffectParticleTrail()
{
  delete[] mParticles;
}

void BosonEffectParticleTrail::update(float elapsed)
{
/*  boDebug(150) << k_funcinfo << " UPDATING; elapsed: " << elapsed << "; rate: " << mRate <<
      "; createCache: " << mCreateCache << "; will add " << elapsed * mRate <<
      " to create cache (total will be = " << mCreateCache + (elapsed * mRate) << ")" << endl;*/

  // How much and how have we moved since last update?
  BoVector3 movevector = mPosition - mLastPos;
  float movedlength = movevector.length();
  //boDebug() << k_funcinfo << "movedlength: " << movedlength << "(" << mParticleCount << " particles)" << endl;

  // Where to create next particle?
  float createpos = 1.0 - mCreateCache;
  //boDebug() << k_funcinfo << "mCreateCache: " << mCreateCache << "createpos: " << createpos << endl;

  bool createnew = false;

  if(movedlength >= 0.0 && !mObsolete)
  {
    // Scale movevector by (1 / movedlength), so that we can later do
    //  movevector * x  and have the position for x-th particle
    movevector = movevector / movedlength;

    // How many particles to create?
    mCreateCache += (movedlength / mSpacing);

    //boDebug() << k_funcinfo << "mCreateCache: " << mCreateCache << "mSpacing: " << mSpacing << endl;
    if((mCreateCache < 1.0) && (mNum <= 0))
    {
      return;
    }

    // Check if new particles should be created
    createnew = (mCreateCache >= 1.0);
  }
  else
  {
    // We haven't moved since last update() or the system is obsolete (= don't
    //  create new particles)
    if(mNum <= 0)
    {
      return;
    }
    createnew = false;
  }

  mNum = 0;  // Number of active particles
  mBoundingSphereRadius = 0.0;


  // Update particles
  for(unsigned int i = 0; i < mParticleCount; i++)
  {
    if(mParticles[i].life > 0.0)
    {
      updateParticle(&mParticles[i], elapsed);
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
      initParticle(&mParticles[i], mLastPos + (movevector * createpos * mSpacing));
      mCreateCache -= 1.0;
      createpos += 1.0;
      mNum++;
      createnew = (mCreateCache >= 1.0);
    }
  }

  if(mCreateCache >= 1.0)
  {
    boWarning() << k_funcinfo << "mCreateCache = " << mCreateCache << " after update()! Increase MaxSpeed!" << endl;
  }

  // Particle update and init methods set mRadius to dot product for performance
  //  reasons. Convert it here.
  mBoundingSphereRadius = sqrt(mBoundingSphereRadius) + mMaxParticleSize;

  mLastPos = mPosition;
}

void BosonEffectParticleTrail::setRotation(const BoVector3& rotation)
{
  mRotated = true;

  mMatrix.loadIdentity();
  mMatrix.rotate(-rotation.z(), 0.0f, 0.0f, 1.0f);
  mMatrix.rotate(rotation.x(), 1.0f, 0.0f, 0.0f);
  mMatrix.rotate(rotation.y(), 0.0f, 1.0f, 0.0f);
}

void BosonEffectParticleTrail::setPosition(const BoVector3& pos)
{
  BosonEffect::setPosition(pos);
}

void BosonEffectParticleTrail::initParticle(BosonTrailParticle* particle, const BoVector3& pos)
{
  // Note that most stuff isn't initialized here, it's done in
  //  BosonParticleSystemProperties
  particle->pos = pos;
  if(!mTextures)
  {
    boError(150) << k_funcinfo << "NULL textures" << endl;
  }
  else if(!mTextures->isValid())
  {
    boError(150) << k_funcinfo << "invalid texture array" << endl;
  }
  else
  {
    particle->tex = mTextures->texture(0);
  }
  particle->system = this;
  if(properties())
  {
    ((BosonEffectPropertiesParticleTrail*)properties())->initParticle(this, particle);
  }
  mBoundingSphereRadius = QMAX(mBoundingSphereRadius, (particle->pos - mPosition).dotProduct());
}

void BosonEffectParticleTrail::updateParticle(BosonTrailParticle* particle, float elapsed)
{
  particle->life -= elapsed;
  particle->pos.addScaled(particle->velo, elapsed);

  if(properties())
  {
    ((BosonEffectPropertiesParticleTrail*)properties())->updateParticle(this, particle);
  }
  mBoundingSphereRadius = QMAX(mBoundingSphereRadius, (particle->pos - mPosition).dotProduct());
}

bool BosonEffectParticleTrail::saveAsXML(QDomElement& root) const
{
  return true;
}

bool BosonEffectParticleTrail::loadFromXML(const QDomElement& root)
{
  return true;
}


/*****  BoParticleList  *****/

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

