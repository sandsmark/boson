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


BosonEffect::BosonEffect()
{
  mActive = true;
}

BosonEffect::~BosonEffect()
{
}



/*****  BosonEffectFog  *****/

/*BosonEffectFog::BosonEffectFog(BosonEffectPropertiesFog* prop) : BosonEffect()
{
}*/

BosonEffectFog::BosonEffectFog(const BoVector4& color, float start, float end, float radius) :
    BosonEffect()
{
  mColor = color;
  mStart = start;
  mRadius = radius;
  mEnd = end;
}

bool BosonEffectFog::saveAsXML(QDomElement& root) const
{
  return true;
}

bool BosonEffectFog::loadFromXML(const QDomElement& root)
{
  return true;
}



/*****  BosonEffectFade  *****/

BosonEffectFade::BosonEffectFade(const BosonEffectPropertiesFade* prop) : BosonEffect()
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
  mActive = false;
}

bool BosonEffectFade::saveAsXML(QDomElement& root) const
{
  return true;
}

bool BosonEffectFade::loadFromXML(const QDomElement& root)
{
  return true;
}



/*****  BosonEffectLight  *****/

BosonEffectLight::BosonEffectLight(const BosonEffectPropertiesLight* prop) : BosonEffect()
{
  mProperties = prop;
  mTimeLeft = prop->life();

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
  }
}

BosonEffectLight::~BosonEffectLight()
{
  delete mLight;
}

void BosonEffectLight::update(float elapsed)
{
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
  mActive = false;
  delete mLight;
  mLight = 0;
}

bool BosonEffectLight::saveAsXML(QDomElement& root) const
{
  return true;
}

bool BosonEffectLight::loadFromXML(const QDomElement& root)
{
  return true;
}

