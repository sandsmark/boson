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

#include <qdom.h>

#include <kmdcodec.h>

#include "bosontexturearray.h"
#include "bosoneffectpropertiesparticle.h"
#include "bodebug.h"


/*****  BosonEffectParticle  *****/

BosonEffectParticle::BosonEffectParticle(const BosonEffectPropertiesParticle* prop) : BosonEffect(prop)
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
  mParticleDistVector.saveAsXML(root, "ParticleDistVector");

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
  if(!mParticleDistVector.loadFromXML(root, "ParticleDistVector"))
  {
    return false;
  }

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
  BosonEffect::update(elapsed);
  if(!hasStarted())
  {
    return;
  }
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

    BoVector3Fixed newpos, oldpos;
    // Rotate all particles
    for(unsigned int i = 0; i < mParticleCount; i++)
    {
      if(mParticles[i].life > 0.0)
      {
        oldpos = mParticles[i].pos - mPosition;
        BoVector3Float _newpos, _oldpos;
        _oldpos = oldpos.toFloat();
        transformmatrix.transform(&_newpos, &_oldpos);
        newpos = _newpos.toFixed();
        mParticles[i].pos = newpos + mPosition;
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
    BoVector3Fixed diff = pos - mPosition;
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
  mBoundingSphereRadius = QMAX(mBoundingSphereRadius, (float)(particle->pos - mPosition).dotProduct());
}

void BosonEffectParticleGeneric::updateParticle(BosonGenericParticle* particle, float elapsed)
{
  particle->life -= elapsed;
  particle->pos.addScaled(particle->velo, elapsed);

  if(properties())
  {
    ((BosonEffectPropertiesParticleGeneric*)properties())->updateParticle(this, particle);
    mBoundingSphereRadius = QMAX(mBoundingSphereRadius, (float)(particle->pos - mPosition).dotProduct());
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
  QDataStream stream(ba, IO_WriteOnly);

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
    if(mParticles[i].life > 0.0)
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
  QDataStream stream(ba, IO_ReadOnly);

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
    int t = (int)((1.0 - factor) * ((int)mTextures->count() + 1)); // +1 for last texture to be shown
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
    int maxnum, const BosonTextureArray* textures, const BoVector3Fixed& pos) :
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
  BosonEffect::update(elapsed);
  if(!hasStarted())
  {
    return;
  }
/*  boDebug(150) << k_funcinfo << " UPDATING; elapsed: " << elapsed << "; rate: " << mRate <<
      "; createCache: " << mCreateCache << "; will add " << elapsed * mRate <<
      " to create cache (total will be = " << mCreateCache + (elapsed * mRate) << ")" << endl;*/

  // How much and how have we moved since last update?
  BoVector3Fixed movevector = mPosition - mLastPos;
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
  BosonEffect::setPosition(pos);
}

void BosonEffectParticleTrail::initParticle(BosonTrailParticle* particle, const BoVector3Fixed& pos)
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
  mBoundingSphereRadius = QMAX(mBoundingSphereRadius, (float)(particle->pos.toFixed() - mPosition).dotProduct());
}

void BosonEffectParticleTrail::updateParticle(BosonTrailParticle* particle, float elapsed)
{
  particle->life -= elapsed;
  particle->pos.addScaled(particle->velo, elapsed);

  if(properties())
  {
    ((BosonEffectPropertiesParticleTrail*)properties())->updateParticle(this, particle);
  }
  mBoundingSphereRadius = QMAX(mBoundingSphereRadius, (float)(particle->pos - mPosition).dotProduct());
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
  mOffset.saveAsXML(root, "Offset");
  mLastPos.saveAsXML(root, "LastPos");

  // Init byte array and data stream
  QByteArray ba;
  QDataStream stream(ba, IO_WriteOnly);

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
    if(mParticles[i].life > 0.0)
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
  mOffset.loadFromXML(root, "Offset");
  mLastPos.loadFromXML(root, "LastPos");

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
  QDataStream stream(ba, IO_ReadOnly);

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
    int t = (int)((1.0 - factor) * ((int)mTextures->count() + 1)); // +1 for last texture to be shown
    if(t >= (int)mTextures->count())
    {
      t = mTextures->count() - 1;
    }
    mParticles[i].tex = mTextures->texture(t);
  }

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

