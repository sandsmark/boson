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


#include "bosoneffect.h"

#include <krandomsequence.h>
#include <kstaticdeleter.h>

#include <qstring.h>
#include <qdom.h>

#include "bosoneffectpropertiesparticle.h"
#include "bodebug.h"
#include "bolight.h"


// Static initialization stuff
KRandomSequence* BosonEffect::mRandom = 0;
static KStaticDeleter<KRandomSequence> sd;


void BosonEffect::initStatic(const QString& particletexdir)
{
  boDebug() << k_funcinfo << "" << endl;
  if (mRandom)
  {
    boError(150) << k_funcinfo << "called twice" << endl;
    return;
  }
  mRandom = new KRandomSequence(123456789);
  sd.setObject(mRandom);
  BosonEffectPropertiesParticle::initStatic(particletexdir);
}

float BosonEffect::getFloat(float min, float max)
{
  if(min == max)
  {
    return min;
  }
  else
  {
    return ((float)(mRandom->getDouble())) * (max - min) + min;
  }
}


BosonEffect::BosonEffect(const BosonEffectProperties* prop)
{
  mGeneralProperties = prop;
  mActive = true;
  mStarted = false;
  mDelay = 0.0f;
  if(prop)
  {
    mDelay = prop->delay();
  }
}

BosonEffect::~BosonEffect()
{
}

void BosonEffect::update(float elapsed)
{
  // If effect is delayed, decrease delay
  if(!hasStarted())
  {
    mDelay -= elapsed;
    if(mDelay <= 0.0f)
    {
      mDelay = 0.0f;
      start();
    }
  }
}

void BosonEffect::start()
{
  mStarted = true;
}

bool BosonEffect::saveAsXML(QDomElement& root) const
{
  // Properties id
  if(mGeneralProperties)
  {
    root.setAttribute(QString::fromLatin1("PropId"), mGeneralProperties->id());
    // TODO: owner id. Maybe make effects global?
  }
  // Position
  root.setAttribute(QString::fromLatin1("Posx"), position().x());
  root.setAttribute(QString::fromLatin1("Posy"), position().y());
  root.setAttribute(QString::fromLatin1("Posz"), position().z());
  // Rotation
  root.setAttribute(QString::fromLatin1("Rotx"), rotation().x());
  root.setAttribute(QString::fromLatin1("Roty"), rotation().y());
  root.setAttribute(QString::fromLatin1("Rotz"), rotation().z());

  // Misc
  root.setAttribute(QString::fromLatin1("Active"), mActive ? 1 : 0);
  root.setAttribute(QString::fromLatin1("Started"), mStarted ? 1 : 0);
  root.setAttribute(QString::fromLatin1("Delay"), mDelay);

  return true;
}

bool BosonEffect::loadFromXML(const QDomElement& root)
{
  return true;
}



/*****  BosonEffectFog  *****/

BosonEffectFog::BosonEffectFog(const BosonEffectPropertiesFog* prop) : BosonEffect(prop)
{
  mProperties = prop;
  mColor = prop->color();
  mStart = prop->start();
  mEnd = prop->end();
  mRadius = prop->radius();
}

BosonEffectFog::BosonEffectFog(const BoVector4& color, float start, float end, float radius) :
    BosonEffect()
{
  mProperties = 0;
  mColor = color;
  mStart = start;
  mRadius = radius;
  mEnd = end;
}

bool BosonEffectFog::saveAsXML(QDomElement& root) const
{
  if(!BosonEffect::saveAsXML(root))
  {
    return false;
  }

  return true;
}

bool BosonEffectFog::loadFromXML(const QDomElement& root)
{
  if(!BosonEffect::loadFromXML(root))
  {
    return false;
  }

  return true;
}



/*****  BosonEffectFade  *****/

BosonEffectFade::BosonEffectFade(const BosonEffectPropertiesFade* prop) : BosonEffect(prop)
{
  mProperties = prop;
  mTimeLeft = prop->time();
  mColor = prop->startColor();
  mGeometry = prop->geometry();
  mBlendFunc[0] = prop->blendFunc()[0];
  mBlendFunc[1] = prop->blendFunc()[1];
}

void BosonEffectFade::update(float elapsed)
{
  BosonEffect::update(elapsed);
  if(!hasStarted())
  {
    return;
  }

  mTimeLeft -= elapsed;
  if(mTimeLeft <= 0)
  {
    mActive = false;
    return;
  }
  float factor = mTimeLeft / mProperties->time();  // This goes from 1 to 0 during effect's lifetime
  mColor.setBlended(mProperties->startColor(), factor, mProperties->endColor(), 1.0 - factor);
}

void BosonEffectFade::makeObsolete()
{
  if(mTimeLeft > 0)
  {
    return;
  }
  BosonEffect::makeObsolete();
}

bool BosonEffectFade::saveAsXML(QDomElement& root) const
{
  if(!BosonEffect::saveAsXML(root))
  {
    return false;
  }

  return true;
}

bool BosonEffectFade::loadFromXML(const QDomElement& root)
{
  if(!BosonEffect::loadFromXML(root))
  {
    return false;
  }

  return true;
}



/*****  BosonEffectLight  *****/

BosonEffectLight::BosonEffectLight(const BosonEffectPropertiesLight* prop) : BosonEffect(prop)
{
  mProperties = prop;
  mTimeLeft = prop->life();
  mLight = 0;
}

BosonEffectLight::~BosonEffectLight()
{
  delete mLight;
}

void BosonEffectLight::update(float elapsed)
{
  BosonEffect::update(elapsed);
  if(!hasStarted())
  {
    return;
  }

  mTimeLeft -= elapsed;
  if(mTimeLeft <= 0)
  {
    mActive = false;
    delete mLight;
    mLight = 0;
    return;
  }
  if(!mLight)
  {
    return;
  }

  float factor = mTimeLeft / mProperties->life();  // This goes from 1 to 0 during effect's lifetime
  BoVector4 ambient, diffuse, specular;
  BoVector3 attenuation;
  ambient.setBlended(mProperties->startAmbient(), factor, mProperties->endAmbient(), 1.0 - factor);
  diffuse.setBlended(mProperties->startDiffuse(), factor, mProperties->endDiffuse(), 1.0 - factor);
  specular.setBlended(mProperties->startSpecular(), factor, mProperties->endSpecular(), 1.0 - factor);
  attenuation.setBlended(mProperties->startAttenuation(), factor, mProperties->endAttenuation(), 1.0 - factor);
  mLight->setAmbient(ambient);
  mLight->setDiffuse(diffuse);
  mLight->setSpecular(specular);
  mLight->setAttenuation(attenuation);
}

void BosonEffectLight::start()
{
  BosonEffect::start();

  mLight = new BoLight();
  if(mLight->id() == -1)
  {
    // Id -1 means that light could not be created. Probably maximum number of
    //  lights is already in use
    delete mLight;
    mLight = 0;
    mActive = false;
    mTimeLeft = 0.0f;
  }
  if(mLight)
  {
    mLight->setEnabled(true);
    mLight->setDirectional(false);
    mLight->setPosition3(position());
  }
}

void BosonEffectLight::setPosition(const BoVector3& pos)
{
  // Add properties->position() to given pos
  BoVector3 newpos = pos + mProperties->position();
  BosonEffect::setPosition(newpos);
  if(mLight)
  {
    mLight->setPosition3(newpos);
  }
}

void BosonEffectLight::makeObsolete()
{
  if(mTimeLeft > 0)
  {
    return;
  }
  delete mLight;
  mLight = 0;
  BosonEffect::makeObsolete();
}

bool BosonEffectLight::saveAsXML(QDomElement& root) const
{
  if(!BosonEffect::saveAsXML(root))
  {
    return false;
  }

  return true;
}

bool BosonEffectLight::loadFromXML(const QDomElement& root)
{
  if(!BosonEffect::loadFromXML(root))
  {
    return false;
  }

  return true;
}



/*****  BosonEffectBulletTrail  *****/

BosonEffectBulletTrail::BosonEffectBulletTrail(const BosonEffectPropertiesBulletTrail* prop, const BoVector3& pos) : BosonEffect(prop)
{
  mProperties = prop;
  mLastPos = pos;
  setPosition(pos);
  mAdvanced = 0;
  mShouldMakeObsolete = false;
}

BosonEffectBulletTrail::~BosonEffectBulletTrail()
{
}

void BosonEffectBulletTrail::update(float elapsed)
{
  BosonEffect::update(elapsed);
  if(!hasStarted())
  {
    return;
  }

  BoVector3 movevector = mPosition - mLastPos;
  float movelength = movevector.length();
  movevector.scale(1 / movelength); // Normalize movevector
  float length = getFloat(mProperties->minLength(), mProperties->maxLength());  // Line's length
  if(length > movelength)
  {
    length = movelength;
  }

  if(length == movelength)
  {
    mStart = mLastPos;
    mEnd = mPosition;
  }
  else
  {
    // Distance of start from mLastPos
    float pos = getFloat(0, movelength - length);
    mStart = mLastPos + movevector * pos;
//    mEnd = mPosition - movevector * (pos - (movelength - length));
    mEnd = mLastPos + movevector * (pos + length);
  }

  mLastPos = mPosition;

  if(mAdvanced < 2)
  {
    mAdvanced++;
    if(mShouldMakeObsolete && mAdvanced >= 2)
    {
      // Make effect obsolete now
      makeObsolete();
    }
  }
}

void BosonEffectBulletTrail::makeObsolete()
{
  // For bullet trails, line will be made obsolete even before update() is
  //  called, which would result in effect being immediately deleted. So we
  //  don't make it obsolete unless update() has been called at least 2 times
  //  already.
  if(mAdvanced < 2)
  {
    mShouldMakeObsolete = true;
    return;
  }

  BosonEffect::makeObsolete();
}

const BoVector4& BosonEffectBulletTrail::color() const
{
  return mProperties->color();
}

float BosonEffectBulletTrail::width() const
{
  return mProperties->width();
}

bool BosonEffectBulletTrail::saveAsXML(QDomElement& root) const
{
  if(!BosonEffect::saveAsXML(root))
  {
    return false;
  }

  return true;
}

bool BosonEffectBulletTrail::loadFromXML(const QDomElement& root)
{
  if(!BosonEffect::loadFromXML(root))
  {
    return false;
  }

  return true;
}

