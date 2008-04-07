/*
    This file is part of the Boson game
    Copyright (C) 2004-2005 Rivo Laks (rivolaks@hot.ee)

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/


#include "bosoneffectparticle.h"

#include "../bomemory/bodummymemory.h"
#include "botexture.h"
#include "bosoneffectpropertiesparticle.h"
#include "bodebug.h"

#include <math.h>

#include <qdom.h>
//Added by qt3to4:
#include <Q3PtrCollection>

#include <kcodecs.h>


/*****  BosonEffectParticle  *****/

BosonEffectParticle::BosonEffectParticle(const BosonEffectPropertiesParticle* prop) : BosonEffect(prop)
{
  mProperties = prop;
  mBoundingSphereRadius = 0.0f;
  mBlendFunc[0] = GL_SRC_ALPHA;
  mBlendFunc[1] = GL_ONE_MINUS_SRC_ALPHA;
  mParticleDist = 0.0f;
  mParticleCount = 0;
  mAlign = true;
}

bool BosonEffectParticle::saveAsXML(QDomElement& root) const
{
  if(!BosonEffect::saveAsXML(root))
  {
    return false;
  }

  root.setAttribute("BoundingSphereRadius", mBoundingSphereRadius);
  root.setAttribute("BlendFunc.src", mBlendFunc[0]);
  root.setAttribute("BlendFunc.dst", mBlendFunc[1]);
  root.setAttribute("ParticleDist", mParticleDist);
  root.setAttribute("ParticleCount", mParticleCount);
  root.setAttribute("Align", mAlign ? 1 : 0);
  saveVector3AsXML(mParticleDistVector, root, "ParticleDistVector");

  return true;
}

bool BosonEffectParticle::loadFromXML(const QDomElement& root)
{
  if(!BosonEffect::loadFromXML(root))
  {
    return false;
  }

  bool ok;
  mBoundingSphereRadius = root.attribute("BoundingSphereRadius").toFloat(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading BoundingSphereRadius attribute ('" << root.attribute("BoundingSphereRadius") << "')" << endl;
    return false;
  }
  mBlendFunc[0] = root.attribute("BlendFunc.src").toInt(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading BlendFunc.src attribute ('" << root.attribute("BlendFunc.src") << "')" << endl;
    return false;
  }
  mBlendFunc[1] = root.attribute("BlendFunc.dst").toInt(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading BlendFunc.dst attribute ('" << root.attribute("BlendFunc.dst") << "')" << endl;
    return false;
  }
  mParticleDist = root.attribute("ParticleDist").toFloat(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading ParticleDist attribute ('" << root.attribute("ParticleDist") << "')" << endl;
    return false;
  }
  mParticleCount = root.attribute("ParticleCount").toUInt(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading ParticleCount attribute ('" << root.attribute("ParticleCount") << "')" << endl;
    return false;
  }
  mAlign = (root.attribute("Align").toInt(&ok));
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading Align attribute ('" << root.attribute("Align") << "')" << endl;
    return false;
  }
  if(!loadVector3FromXML(&mParticleDistVector, root, "ParticleDistVector"))
  {
    return false;
  }

  return true;
}

void BosonEffectParticle::setPosition(const BoVector3Fixed& pos)
{
  mPositionFloat = pos.toFloat();
  BosonEffect::setPosition(pos);
}


/*****  BosonEffectParticleGeneric  *****/

BosonEffectParticleGeneric::BosonEffectParticleGeneric(const BosonEffectPropertiesParticleGeneric* prop,
    int maxnum, const BoTextureArray* textures) :
    BosonEffectParticle(prop)
{
  mParticles = new BosonGenericParticle[maxnum];
  mParticleCount = maxnum;
  mTextures = textures;

  // Init variables to defaults
  mAge = 3600.0f;
  mMass = 1.0f;
  mParticleDist = 0.0f;
  mMoveParticlesWithSystem = false;
  mCreateCache = 0.0f;
  mRate = 0.0f;
  mBlendFunc[0] = GL_SRC_ALPHA;
  mBlendFunc[1] = GL_ONE_MINUS_SRC_ALPHA;
  mRotated = false;
  mNum = 0;
  mMaxParticleSize = 0.0f;
  mMaxDelayedUpdates = 60;
}

BosonEffectParticleGeneric::~BosonEffectParticleGeneric()
{
  delete[] mParticles;
}

void BosonEffectParticleGeneric::update(float elapsed)
{
  BosonEffect::update(elapsed);
  if(!hasStarted())
  {
    return;
  }
/*  boDebug(150) << k_funcinfo << " UPDATING; elapsed: " << elapsed << "; rate: " << mRate <<
      "; createCache: " << mCreateCache << "; will add " << elapsed * mRate <<
      " to create cache (total will be = " << mCreateCache + (elapsed * mRate) << ")" << endl;*/
  mCreateCache += (elapsed * mRate);

  if((mCreateCache < 1.0f) && (mNum <= 0))
  {
    return;
  }

  mNum = 0;
  mBoundingSphereRadius = 0.0f;
  bool createnew;

  // Check if new particles should be created
  if(mAge == -1.0f)
  {
    // -1 means to create new particles forever
    createnew = (mCreateCache >= 1.0f);
  }
  else if(mAge > 0.0f)
  {
    // system still producing new particles
    mAge -= elapsed;
    if(mAge < 0.0f)
    {
      mAge = 0.0f;
    }
    createnew = (mCreateCache >= 1.0f) && (mAge > 0.0f);
  }
  else
  {
    // system not producing new particles anymore
    createnew = false;
  }

  // Update particles
  for(unsigned int i = 0; i < mParticleCount; i++)
  {
    if(mParticles[i].life > 0.0f)
    {
      updateParticle(&mParticles[i], elapsed);
      // Check for death
      // For performance reasons particles aren't actually created/deleted.
      //  They're just marked as dead (aka inactive)
      if(mParticles[i].life > 0.0f)
      {
        mNum++;
      }
    }
    else if(createnew)
    {
      // Dead particle, re-create it
      initParticle(&mParticles[i]);
      mCreateCache -= 1.0f;
      mNum++;
      createnew = (mCreateCache >= 1.0f);
    }
  }

  // Particle update and init methods set mRadius to dot product for performance
  //  reasons. Convert it here.
  mBoundingSphereRadius = sqrtf(mBoundingSphereRadius) + mMaxParticleSize;
}

void BosonEffectParticleGeneric::start()
{
  BosonEffect::start();
  createParticles(((BosonEffectPropertiesParticleGeneric*)properties())->initialParticles());
}

void BosonEffectParticleGeneric::setRotation(const BoVector3Fixed& rotation)
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

    BoVector3Float newpos, oldpos;
    BoVector3Float fposition = mPosition.toFloat();
    // Rotate all particles
    for(unsigned int i = 0; i < mParticleCount; i++)
    {
      if(mParticles[i].life > 0.0f)
      {
        oldpos = mParticles[i].pos - fposition;
        transformmatrix.transform(&newpos, &oldpos);
        mParticles[i].pos = newpos + fposition;
      }
    }
  }

  mMatrix.loadIdentity();
  mMatrix.rotate(-rotation.z(), 0.0f, 0.0f, 1.0f);
  mMatrix.rotate(rotation.x(), 1.0f, 0.0f, 0.0f);
  mMatrix.rotate(rotation.y(), 0.0f, 1.0f, 0.0f);

  BosonEffect::setRotation(rotation.toFixed());

  mRotated = true;
}

void BosonEffectParticleGeneric::setPosition(const BoVector3Fixed& pos)
{
  if(mMoveParticlesWithSystem)
  {
    BoVector3Float diff = pos.toFloat() - positionFloat();
    // Move all particles by diff
    for(unsigned int i = 0; i < mParticleCount; i++)
    {
      if(mParticles[i].life > 0.0f)
      {
        mParticles[i].pos += diff;
      }
    }
  }
  BosonEffectParticle::setPosition(pos);
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
  mBoundingSphereRadius = sqrtf(mBoundingSphereRadius) + mMaxParticleSize;
}

void BosonEffectParticleGeneric::initParticle(BosonGenericParticle* particle)
{
  // Note that most stuff isn't initialized here, it's done in
  //  BosonParticleSystemProperties
  particle->pos = positionFloat();
  if(!mTextures)
  {
    boError(150) << k_funcinfo << "NULL textures" << endl;
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
  mBoundingSphereRadius = qMax(mBoundingSphereRadius, (particle->pos - positionFloat()).dotProduct());
}

void BosonEffectParticleGeneric::updateParticle(BosonGenericParticle* particle, float elapsed)
{
  particle->life -= elapsed;
  particle->pos.addScaled(particle->velo, elapsed);

  if(properties())
  {
    ((BosonEffectPropertiesParticleGeneric*)properties())->updateParticle(this, particle, elapsed);
    mBoundingSphereRadius = qMax(mBoundingSphereRadius, (particle->pos - positionFloat()).dotProduct());
  }
}

bool BosonEffectParticleGeneric::saveAsXML(QDomElement& root) const
{
  if(!BosonEffectParticle::saveAsXML(root))
  {
    return false;
  }

  root.setAttribute("Num", mNum);
  root.setAttribute("Rate", mRate);
  root.setAttribute("CreateCache", mCreateCache);
  root.setAttribute("MoveParticlesWithSystem", mMoveParticlesWithSystem ? 1 : 0);
  root.setAttribute("Rotated", mRotated ? 1 : 0);
  root.setAttribute("Age", mAge);
  root.setAttribute("Mass", mMass);
  root.setAttribute("MaxParticleSize", mMaxParticleSize);

  // Init byte array and data stream
  QByteArray ba;
  QDataStream stream(&ba, QIODevice::WriteOnly);

  // Save matrix
  for(int i = 0; i < 4; i++)
  {
    for(int j = 0; j < 4; j++)
    {
      stream << mMatrix.element(i, j);
    }
  }

  // Save particles
  int particlessaved = 0;
  for(unsigned int i = 0; i < mParticleCount; i++)
  {
    if(mParticles[i].life > 0.0f)
    {
      stream << mParticles[i].velo;
      stream << mParticles[i].maxage;
      stream << mParticles[i].color;
      stream << mParticles[i].pos;
      stream << mParticles[i].size;
      stream << mParticles[i].life;
      stream << mParticles[i].distance;
      particlessaved++;
    }
  }
  if(particlessaved != mNum)
  {
    boError() << k_funcinfo << "mNum was invalid!!!" << endl;
    return false;
  }

  // Encode ba to base64
  QString base64data = KCodecs::base64Encode(ba);
  // Save as QDomText
  QDomDocument doc = root.ownerDocument();
  QDomElement data = doc.createElement("Data");
  QDomText domtext = doc.createTextNode(base64data);
  root.appendChild(data);
  data.appendChild(domtext);

  return true;
}

bool BosonEffectParticleGeneric::loadFromXML(const QDomElement& root)
{
  if(!BosonEffectParticle::loadFromXML(root))
  {
    return false;
  }

  bool ok;
  mNum = root.attribute("Num").toInt(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading Num attribute ('" << root.attribute("Num") << "')" << endl;
    return false;
  }
  mRate = root.attribute("Rate").toFloat(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading Rate attribute ('" << root.attribute("Rate") << "')" << endl;
    return false;
  }
  mCreateCache = root.attribute("CreateCache").toFloat(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading CreateCache attribute ('" << root.attribute("CreateCache") << "')" << endl;
    return false;
  }
  mMoveParticlesWithSystem = (root.attribute("MoveParticlesWithSystem").toInt(&ok));
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading mMoveParticlesWithSystem attribute ('" << root.attribute("mMoveParticlesWithSystem") << "')" << endl;
    return false;
  }
  mRotated = (root.attribute("Rotated").toInt(&ok));
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading Rotated attribute ('" << root.attribute("Rotated") << "')" << endl;
    return false;
  }
  mAge = root.attribute("Age").toFloat(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading Age attribute ('" << root.attribute("Age") << "')" << endl;
    return false;
  }
  mMass = root.attribute("Mass").toFloat(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading Mass attribute ('" << root.attribute("Mass") << "')" << endl;
    return false;
  }
  mMaxParticleSize = root.attribute("MaxParticleSize").toFloat(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading MaxParticleSize attribute ('" << root.attribute("MaxParticleSize") << "')" << endl;
    return false;
  }

  QDomElement dataelement = root.namedItem("Data").toElement();
  if(dataelement.isNull())
  {
    boError() << k_funcinfo << "Data element not found!" << endl;
    return false;
  }
  QString base64data = dataelement.text();
  if(base64data.isEmpty())
  {
    boDebug() << k_funcinfo << "Empty or invalid text in Data element!" << endl;
    return false;
  }
  QByteArray ba;
  KCodecs::base64Decode(base64data.utf8(), ba);  // Is utf8() safe to use here?

  // Init data stream
  QDataStream stream(ba);

  // Load matrix
  for(int i = 0; i < 4; i++)
  {
    for(int j = 0; j < 4; j++)
    {
      float e;
      stream >> e;
      mMatrix.setElement(i, j, e);
    }
  }

  // Load particles
  for(int i = 0; i < mNum; i++)
  {
    mParticles[i].system = this;
    stream >> mParticles[i].velo;
    stream >> mParticles[i].maxage;
    stream >> mParticles[i].color;
    stream >> mParticles[i].pos;
    stream >> mParticles[i].size;
    stream >> mParticles[i].life;
    stream >> mParticles[i].distance;

    // Update particle's texture (can't be saved because texture id can change)
    float factor = mParticles[i].life / mParticles[i].maxage;
    int t = (int)((1.0f - factor) * ((int)mTextures->count() + 1)); // +1 for last texture to be shown
    if(t >= (int)mTextures->count())
    {
      t = mTextures->count() - 1;
    }
    mParticles[i].tex = mTextures->texture(t);
  }

  return true;
}



/*****  BosonEffectParticleTrail  *****/

BosonEffectParticleTrail::BosonEffectParticleTrail(const BosonEffectPropertiesParticleTrail* prop,
    int maxnum, const BoTextureArray* textures, const BoVector3Fixed& pos) :
    BosonEffectParticle(prop)
{
  mParticles = new BosonGenericParticle[maxnum];
  mParticleCount = maxnum;
  mTextures = textures;

  // Init variables to defaults
  mMass = 1.0f;
  mParticleDist = 0.0f;
  mCreateCache = 0.0f;
  mBlendFunc[0] = GL_SRC_ALPHA;
  mBlendFunc[1] = GL_ONE_MINUS_SRC_ALPHA;
  mRotated = false;
  mNum = 0;
  mObsolete = false;
  mMaxDelayedUpdates = 60;

  mLastPos = pos.toFloat();
  setPosition(pos);
}

BosonEffectParticleTrail::~BosonEffectParticleTrail()
{
  delete[] mParticles;
}

void BosonEffectParticleTrail::update(float elapsed)
{
  BosonEffect::update(elapsed);
  if(!hasStarted())
  {
    return;
  }
/*  boDebug(150) << k_funcinfo << " UPDATING; elapsed: " << elapsed << "; rate: " << mRate <<
      "; createCache: " << mCreateCache << "; will add " << elapsed * mRate <<
      " to create cache (total will be = " << mCreateCache + (elapsed * mRate) << ")" << endl;*/

  // How much and how have we moved since last update?
  BoVector3Float movevector = positionFloat() - mLastPos;
  float movedlength = movevector.length();
  //boDebug() << k_funcinfo << "movedlength: " << movedlength << "(" << mParticleCount << " particles)" << endl;

  // Where to create next particle?
  float createpos = 1.0f - mCreateCache;
  //boDebug() << k_funcinfo << "mCreateCache: " << mCreateCache << "createpos: " << createpos << endl;

  bool createnew = false;

  if(movedlength > 0.0f && !mObsolete)
  {
    // Scale movevector by (1 / movedlength), so that we can later do
    //  movevector * x  and have the position for x-th particle
    movevector = movevector / movedlength;

    // How many particles to create?
    mCreateCache += (movedlength / mSpacing);

    //boDebug() << k_funcinfo << "mCreateCache: " << mCreateCache << "mSpacing: " << mSpacing << endl;
    if((mCreateCache < 1.0f) && (mNum <= 0))
    {
      return;
    }

    // Check if new particles should be created
    createnew = (mCreateCache >= 1.0f);
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
  mBoundingSphereRadius = 0.0f;


  // Update particles
  for(unsigned int i = 0; i < mParticleCount; i++)
  {
    if(mParticles[i].life > 0.0f)
    {
      updateParticle(&mParticles[i], elapsed);
      // Check for death
      // For performance reasons particles aren't actually created/deleted.
      //  They're just marked as dead (aka inactive)
      if(mParticles[i].life > 0.0f)
      {
        mNum++;
      }
    }
    else if(createnew)
    {
      // Dead particle, re-create it
      initParticle(&mParticles[i], mLastPos + (movevector * createpos * mSpacing));
      mCreateCache -= 1.0f;
      createpos += 1.0f;
      mNum++;
      createnew = (mCreateCache >= 1.0f);
    }
  }

  if(mCreateCache >= 1.0f)
  {
    boWarning() << k_funcinfo << "propid: " << properties()->id() << "; mCreateCache = " << mCreateCache << " after update()! Increase MaxSpeed!" << endl;
  }

  // Particle update and init methods set mRadius to dot product for performance
  //  reasons. Convert it here.
  mBoundingSphereRadius = sqrtf(mBoundingSphereRadius) + mMaxParticleSize;

  mLastPos = positionFloat();
}

void BosonEffectParticleTrail::setRotation(const BoVector3Fixed& rotation)
{
  mRotated = true;

  mMatrix.loadIdentity();
  mMatrix.rotate(-rotation.z(), 0.0f, 0.0f, 1.0f);
  mMatrix.rotate(rotation.x(), 1.0f, 0.0f, 0.0f);
  mMatrix.rotate(rotation.y(), 0.0f, 1.0f, 0.0f);

  BosonEffect::setRotation(rotation);
}

void BosonEffectParticleTrail::setPosition(const BoVector3Fixed& pos)
{
  BosonEffectParticle::setPosition(pos);
}

void BosonEffectParticleTrail::initParticle(BosonGenericParticle* particle, const BoVector3Float& pos)
{
  // Note that most stuff isn't initialized here, it's done in
  //  BosonParticleSystemProperties
  particle->pos = pos;
  if(!mTextures)
  {
    boError(150) << k_funcinfo << "NULL textures" << endl;
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
  mBoundingSphereRadius = qMax(mBoundingSphereRadius, (particle->pos - positionFloat()).dotProduct());
}

void BosonEffectParticleTrail::updateParticle(BosonGenericParticle* particle, float elapsed)
{
  particle->life -= elapsed;
  particle->pos.addScaled(particle->velo, elapsed);

  if(properties())
  {
    ((BosonEffectPropertiesParticleTrail*)properties())->updateParticle(this, particle, elapsed);
  }
  mBoundingSphereRadius = qMax(mBoundingSphereRadius, (particle->pos - positionFloat()).dotProduct());
}

bool BosonEffectParticleTrail::saveAsXML(QDomElement& root) const
{
  if(!BosonEffectParticle::saveAsXML(root))
  {
    return false;
  }

  root.setAttribute("Num", mNum);
  root.setAttribute("CreateCache", mCreateCache);
  root.setAttribute("Rotated", mRotated ? 1 : 0);
  root.setAttribute("Mass", mMass);
  root.setAttribute("MaxParticleSize", mMaxParticleSize);
  root.setAttribute("Obsolete", mObsolete ? 1 : 0);
  root.setAttribute("Spacing", mSpacing);
  saveVector3AsXML(mOffset, root, "Offset");
  saveVector3AsXML(mLastPos, root, "LastPos");

  // Init byte array and data stream
  QByteArray ba;
  QDataStream stream(&ba, QIODevice::WriteOnly);

  // Save matrix
  for(int i = 0; i < 4; i++)
  {
    for(int j = 0; j < 4; j++)
    {
      stream << mMatrix.element(i, j);
    }
  }

  // Save particles
  int particlessaved = 0;
  for(unsigned int i = 0; i < mParticleCount; i++)
  {
    if(mParticles[i].life > 0.0f)
    {
      stream << mParticles[i].velo;
      stream << mParticles[i].maxage;
      stream << mParticles[i].color;
      stream << mParticles[i].pos;
      stream << mParticles[i].size;
      stream << mParticles[i].life;
      stream << mParticles[i].distance;
      particlessaved++;
    }
  }
  if(particlessaved != mNum)
  {
    boError() << k_funcinfo << "mNum was invalid!!!" << endl;
    return false;
  }

  // Encode ba to base64
  QString base64data = KCodecs::base64Encode(ba);
  // Save as QDomText
  QDomDocument doc = root.ownerDocument();
  QDomElement data = doc.createElement("Data");
  QDomText domtext = doc.createTextNode(base64data);
  root.appendChild(data);
  data.appendChild(domtext);

  return true;
}

bool BosonEffectParticleTrail::loadFromXML(const QDomElement& root)
{
  if(!BosonEffectParticle::loadFromXML(root))
  {
    return false;
  }

  bool ok;
  mNum = root.attribute("Num").toInt(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading Num attribute ('" << root.attribute("Num") << "')" << endl;
    return false;
  }
  mCreateCache = root.attribute("CreateCache").toFloat(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading CreateCache attribute ('" << root.attribute("CreateCache") << "')" << endl;
    return false;
  }
  mRotated = (root.attribute("Rotated").toInt(&ok));
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading Rotated attribute ('" << root.attribute("Rotated") << "')" << endl;
    return false;
  }
  mMass = root.attribute("Mass").toFloat(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading Mass attribute ('" << root.attribute("Mass") << "')" << endl;
    return false;
  }
  mMaxParticleSize = root.attribute("MaxParticleSize").toFloat(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading MaxParticleSize attribute ('" << root.attribute("MaxParticleSize") << "')" << endl;
    return false;
  }
  mObsolete = (root.attribute("Obsolete").toInt(&ok));
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading Obsolete attribute ('" << root.attribute("Obsolete") << "')" << endl;
    return false;
  }
  mSpacing = root.attribute("Spacing").toFloat(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading Spacing attribute ('" << root.attribute("Spacing") << "')" << endl;
    return false;
  }
  loadVector3FromXML(&mOffset, root, "Offset");
  loadVector3FromXML(&mLastPos, root, "LastPos");

  QDomElement dataelement = root.namedItem("Data").toElement();
  if(dataelement.isNull())
  {
    boError() << k_funcinfo << "Data element not found!" << endl;
    return false;
  }
  QString base64data = dataelement.text();
  if(base64data.isEmpty())
  {
    boDebug() << k_funcinfo << "Empty or invalid text in Data element!" << endl;
    return false;
  }
  QByteArray ba;
  KCodecs::base64Decode(base64data.utf8(), ba);  // Is utf8() safe to use here?

  // Init data stream
  QDataStream stream(ba);

  // Load matrix
  for(int i = 0; i < 4; i++)
  {
    for(int j = 0; j < 4; j++)
    {
      float e;
      stream >> e;
      mMatrix.setElement(i, j, e);
    }
  }

  // Load particles
  for(int i = 0; i < mNum; i++)
  {
    mParticles[i].system = this;
    stream >> mParticles[i].velo;
    stream >> mParticles[i].maxage;
    stream >> mParticles[i].color;
    stream >> mParticles[i].pos;
    stream >> mParticles[i].size;
    stream >> mParticles[i].life;
    stream >> mParticles[i].distance;

    // Update particle's texture (can't be saved because texture id can change)
    float factor = mParticles[i].life / mParticles[i].maxage;
    int t = (int)((1.0f - factor) * ((int)mTextures->count() + 1)); // +1 for last texture to be shown
    if(t >= (int)mTextures->count())
    {
      t = mTextures->count() - 1;
    }
    mParticles[i].tex = mTextures->texture(t);
  }

  return true;
}



/*****  BosonEffectParticleEnvironmental  *****/

BosonEffectParticleEnvironmental::BosonEffectParticleEnvironmental(const BosonEffectPropertiesParticleEnvironmental* prop,
    float density, float range, const BoTextureArray* textures,
    const BoVector3Fixed& pos) :
    BosonEffectParticle(prop)
{
  mDensity = density;
  mRange = range;
  // Calculate how many particles should be in total
  float num = mDensity * (mRange * mRange * mRange * 8);  // = mDensity * pow(mRange * 2, 3)
  mParticleCount = (int)(num * 1.10f);  // 10% is for safety

  mParticles = new BosonGenericParticle[mParticleCount];
  mTextures = textures;

  // Init variables to defaults
  mMass = 1.0f;
  mBlendFunc[0] = GL_SRC_ALPHA;
  mBlendFunc[1] = GL_ONE_MINUS_SRC_ALPHA;
  mNum = 0;
  mObsolete = false;
  mBoundingSphereRadius = 10.0f;  // Any value >0 should work

  // Init num particles at default position
  int i = 0;
  // Min/max coords of the box where to spawn particles
  BoVector3Float min(pos.x() - mRange, pos.y() - mRange, pos.z() - mRange);
  BoVector3Float max(pos.x() + mRange, pos.y() + mRange, pos.z() + mRange);

  // Spawn them
  while(num >= 1.0f)
  {
    // Spawn next particle
    // Use this dead particle to spawn new one.
    // Choose random position inside this box.
    mParticles[i].pos.setX(getRandomFloat(min.x(), max.x()));
    mParticles[i].pos.setY(getRandomFloat(min.y(), max.y()));
    mParticles[i].pos.setZ(getRandomFloat(min.z(), max.z()));
    initParticle(&mParticles[i], mParticles[i].pos);
    i++;
    num -= 1.0f;
  }

  BosonEffectParticle::setPosition(pos);
}

BosonEffectParticleEnvironmental::~BosonEffectParticleEnvironmental()
{
  delete[] mParticles;
}

void BosonEffectParticleEnvironmental::update(float elapsed)
{
  BosonEffect::update(elapsed);
  if(!hasStarted())
  {
    return;
  }

  // We need to spawn some new particles, because when particles move, some of
  //  them go outside the range, others move, and some areas won't have any
  //  particles anymore.
  //  This is very similar to moving the whole particle system: if average
  //  velocity of the particles is velo, then we can treat the situation as if
  //  the particle system was moved by -velo.

  // First update particles and obsolete those that have gone out of range.
  BoVector3Float pos = positionFloat();
  mNum = 0;
  for(unsigned int i = 0; i < mParticleCount; i++)
  {
    if(mParticles[i].life > 0.0f)
    {
      // Update particle
      updateParticle(&mParticles[i], elapsed);
      // Calculate dist between particle and particle system's center.
      float dist = 0.0f;
      dist = qMax(dist, qAbs(pos.x() - mParticles[i].pos.x()));
      dist = qMax(dist, qAbs(pos.y() - mParticles[i].pos.y()));
      dist = qMax(dist, qAbs(pos.z() - mParticles[i].pos.z()));
      if(dist > mRange)
      {
        // Particle is too far away, mark it as dead
        mParticles[i].life = -1.0f;
      }
      else
      {
        mNum++;
      }
    }
  }

  // Then spawn some new particles where necessary.
  // This is average velocity of all particles
  BoVector3Float velo = mParticleVelo + BosonEffectPropertiesParticle::wind() * mMass;
  particleBoxMoved(pos, pos - velo * elapsed);
}

void BosonEffectParticleEnvironmental::setPosition(const BoVector3Fixed& _pos)
{
  BoVector3Float oldpos = positionFloat();
  BoVector3Float pos = _pos.toFloat();
  if(oldpos == pos)
  {
    // Position didn't change. No need to do anything
    return;
  }
  BosonEffectParticle::setPosition(_pos);

  // Delete all particles that are too far away now
  for(unsigned int i = 0; i < mParticleCount; i++)
  {
    if(mParticles[i].life > 0.0f)
    {
      // Calculate dist between particle and particle system's center.
      float dist = 0;
      dist = qMax(dist, qAbs(pos.x() - mParticles[i].pos.x()));
      dist = qMax(dist, qAbs(pos.y() - mParticles[i].pos.y()));
      dist = qMax(dist, qAbs(pos.z() - mParticles[i].pos.z()));
      if(dist > mRange)
      {
        // Particle is too far away, mark it as dead
        mParticles[i].life = -1.0f;
        mNum--;
      }
    }
  }

  particleBoxMoved(oldpos, pos);
}

void BosonEffectParticleEnvironmental::particleBoxMoved(const BoVector3Float& oldpos, const BoVector3Float& newpos)
{
  // Find out the area where to spawn new particles
  // If oldbox is the old bounding box of the system, and newbox is the new
  //  one, then this area would be (newbox - oldbox). This area consists of
  //  at most 3 axis-aligned boxes.
  class MyBox
  {
    public:
      MyBox() { empty = true; volume = 0.0f; }
      void updateVolume()  { volume = (max.x() - min.x()) * (max.y() - min.y()) * (max.z() - min.z()); }

      BoVector3Float min;
      BoVector3Float max;
      float volume;
      bool empty;
  };

  // These boxes will contain areas which are in newbox, but not in oldbox.
  MyBox boxes[3];

  // Make boxes for old and new positions
  MyBox oldbox;
  oldbox.min.set(oldpos.x() - mRange, oldpos.y() - mRange, oldpos.z() - mRange);
  oldbox.max.set(oldpos.x() + mRange, oldpos.y() + mRange, oldpos.z() + mRange);
  MyBox newbox;
  newbox.min.set(newpos.x() - mRange, newpos.y() - mRange, newpos.z() - mRange);
  newbox.max.set(newpos.x() + mRange, newpos.y() + mRange, newpos.z() + mRange);

  // Transform oldbox, so that it's completely inside newbox. That will make
  //  some things easier.
  oldbox.min.set(qMax(oldbox.min.x(), newbox.min.x()), qMax(oldbox.min.y(), newbox.min.y()),
      qMax(oldbox.min.z(), newbox.min.z()));
  oldbox.max.set(qMin(oldbox.max.x(), newbox.max.x()), qMin(oldbox.max.y(), newbox.max.y()),
      qMin(oldbox.max.z(), newbox.max.z()));


  // Calculate z-difference
  if(oldpos.z() != newpos.z())
  {
    boxes[2].empty = false;
    boxes[2].min = newbox.min;
    boxes[2].max = newbox.max;
    if(oldpos.z() < newpos.z())
    {
      // Difference box is above old box
      boxes[2].min.setZ(oldbox.max.z());
    }
    else
    {
      // Difference box is below old box
      boxes[2].max.setZ(oldbox.min.z());
    }
    boxes[2].updateVolume();
  }

  // Calculate x-difference
  if(oldpos.x() != newpos.x())
  {
    boxes[0].empty = false;
    boxes[0].min = newbox.min;
    boxes[0].max = newbox.max;
    if(oldpos.x() < newpos.x())
    {
      // Difference box is right of old box
      boxes[0].min.setX(oldbox.max.x());
    }
    else
    {
      // Difference box is left of old box
      boxes[0].max.setX(oldbox.min.x());
    }
    // This makes sure no area is included twice in those boxes
    boxes[0].min.setZ(oldbox.min.z());
    boxes[0].max.setZ(oldbox.max.z());
    boxes[0].updateVolume();
  }

  // Calculate y-difference
  if(oldpos.y() != newpos.y())
  {
    boxes[1].empty = false;
    boxes[1].min = newbox.min;
    boxes[1].max = newbox.max;
    if(oldpos.y() < newpos.y())
    {
      // Difference box is in front of old box
      boxes[1].min.setY(oldbox.max.y());
    }
    else
    {
      // Difference box is behind of old box
      boxes[1].max.setY(oldbox.min.y());
    }
    // This makes sure no area is included twice in those boxes
    boxes[1].min.setZ(oldbox.min.z());
    boxes[1].max.setZ(oldbox.max.z());
    boxes[1].min.setX(oldbox.min.x());
    boxes[1].max.setX(oldbox.max.x());
    boxes[1].updateVolume();
  }


  // Spawn particles in new boxes
  unsigned int particlepos = 0;
  for(int i = 0; i < 3; i++)
  {
    if(boxes[i].empty)
    {
      continue;
    }
    // Calculate how many particles to spawn
    float num = mDensity * boxes[i].volume;
    while(num >= 1.0f)
    {
      // Spawn next particle
      for(; particlepos < mParticleCount; particlepos++)
      {
        if(mParticles[particlepos].life <= 0.0f)
        {
          // Use this dead particle to spawn new one.
          // Choose random position inside this box.
          mParticles[particlepos].pos.setX(getRandomFloat(boxes[i].min.x(), boxes[i].max.x()));
          mParticles[particlepos].pos.setY(getRandomFloat(boxes[i].min.y(), boxes[i].max.y()));
          mParticles[particlepos].pos.setZ(getRandomFloat(boxes[i].min.z(), boxes[i].max.z()));
          initParticle(&mParticles[particlepos], mParticles[particlepos].pos);
          mNum++;
          // Break out of for-loop
          break;
        }
      }
      num -= 1.0f;
    }
  }
}

void BosonEffectParticleEnvironmental::initParticle(BosonGenericParticle* particle, const BoVector3Float& pos)
{
  // Note that most stuff isn't initialized here, it's done in
  //  BosonParticleSystemProperties
  particle->pos = pos;
  if(!mTextures)
  {
    boError(150) << k_funcinfo << "NULL textures" << endl;
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
}

void BosonEffectParticleEnvironmental::updateParticle(BosonGenericParticle* particle, float elapsed)
{
  particle->pos.addScaled(particle->velo, elapsed);

  if(properties())
  {
    ((BosonEffectPropertiesParticleTrail*)properties())->updateParticle(this, particle, elapsed);
  }
}

bool BosonEffectParticleEnvironmental::saveAsXML(QDomElement& root) const
{
  if(!BosonEffectParticle::saveAsXML(root))
  {
    return false;
  }

  root.setAttribute("Num", mNum);
  root.setAttribute("Mass", mMass);
  root.setAttribute("Obsolete", mObsolete ? 1 : 0);
  root.setAttribute("Range", mRange);
  root.setAttribute("Density", mDensity);
  saveVector3AsXML(mParticleVelo, root, "ParticleVelo");

  // Init byte array and data stream
  QByteArray ba;
  QDataStream stream(&ba, QIODevice::WriteOnly);

  // Save particles
  int particlessaved = 0;
  for(unsigned int i = 0; i < mParticleCount; i++)
  {
    if(mParticles[i].life > 0.0f)
    {
      stream << mParticles[i].velo;
      stream << mParticles[i].maxage;
      stream << mParticles[i].color;
      stream << mParticles[i].pos;
      stream << mParticles[i].size;
      stream << mParticles[i].life;
      stream << mParticles[i].distance;
      particlessaved++;
    }
  }
  if(particlessaved != mNum)
  {
    boError() << k_funcinfo << "mNum was invalid!!!" << endl;
    return false;
  }

  // Encode ba to base64
  QString base64data = KCodecs::base64Encode(ba);
  // Save as QDomText
  QDomDocument doc = root.ownerDocument();
  QDomElement data = doc.createElement("Data");
  QDomText domtext = doc.createTextNode(base64data);
  root.appendChild(data);
  data.appendChild(domtext);

  return true;
}

bool BosonEffectParticleEnvironmental::loadFromXML(const QDomElement& root)
{
  if(!BosonEffectParticle::loadFromXML(root))
  {
    return false;
  }

  bool ok;
  mNum = root.attribute("Num").toInt(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading Num attribute ('" << root.attribute("Num") << "')" << endl;
    return false;
  }
  mMass = root.attribute("Mass").toFloat(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading Mass attribute ('" << root.attribute("Mass") << "')" << endl;
    return false;
  }
  mObsolete = (root.attribute("Obsolete").toInt(&ok));
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading Obsolete attribute ('" << root.attribute("Obsolete") << "')" << endl;
    return false;
  }
  mRange = root.attribute("Range").toFloat(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading Range attribute ('" << root.attribute("Range") << "')" << endl;
    return false;
  }
  mDensity = root.attribute("Density").toFloat(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading Density attribute ('" << root.attribute("Density") << "')" << endl;
    return false;
  }
  loadVector3FromXML(&mParticleVelo, root, "ParticleVelo");

  QDomElement dataelement = root.namedItem("Data").toElement();
  if(dataelement.isNull())
  {
    boError() << k_funcinfo << "Data element not found!" << endl;
    return false;
  }
  QString base64data = dataelement.text();
  if(base64data.isEmpty())
  {
    boDebug() << k_funcinfo << "Empty or invalid text in Data element!" << endl;
    return false;
  }
  QByteArray ba;
  KCodecs::base64Decode(base64data.utf8(), ba);  // Is utf8() safe to use here?

  // Init data stream
  QDataStream stream(ba);

  // Load particles
  for(int i = 0; i < mNum; i++)
  {
    mParticles[i].system = this;
    stream >> mParticles[i].velo;
    stream >> mParticles[i].maxage;
    stream >> mParticles[i].color;
    stream >> mParticles[i].pos;
    stream >> mParticles[i].size;
    stream >> mParticles[i].life;
    stream >> mParticles[i].distance;

    // Update particle's texture (can't be saved because texture id can change)
    // Env. particles always use only the first texture atm
    mParticles[i].tex = mTextures->texture(0);
  }

  return true;
}



/*****  BoParticleList  *****/

int BoParticleList::compareItems(Q3PtrCollection::Item item1, Q3PtrCollection::Item item2)
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

