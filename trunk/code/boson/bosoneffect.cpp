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

