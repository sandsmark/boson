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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#include "bosoneffect.h"

#include "../bomemory/bodummymemory.h"
#include "bosoneffectpropertiesparticle.h"
#include "bodebug.h"
#include "bolight.h"

#include <krandomsequence.h>
#include <kstaticdeleter.h>

#include <qstring.h>
#include <qdom.h>


// Static initialization stuff
KRandomSequence* BosonEffect::mRandom = 0;
static KStaticDeleter<KRandomSequence> sd;


void BosonEffect::initStatic(const QString& particletexdir)
{
  if (mRandom)
  {
    boError(150) << k_funcinfo << "called twice" << endl;
    return;
  }
  mRandom = new KRandomSequence(123456789);
  sd.setObject(mRandom);
  BosonEffectPropertiesParticle::initStatic(particletexdir);
}

float BosonEffect::getRandomFloat(float min, float max)
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

bofixed BosonEffect::getRandomBofixed(bofixed min, bofixed max)
{
  if(min == max)
  {
    return min;
  }
  else
  {
    return ((bofixed)(mRandom->getDouble())) * (max - min) + min;
  }
}


BosonEffect::BosonEffect(const BosonEffectProperties* prop)
{
  mGeneralProperties = prop;
  mActive = true;
  mStarted = false;
  mDelay = 0.0f;
  mOwnerId = 0;
  mUpdateCounter = 0;
  if(prop)
  {
    mDelay = prop->delay();
  }
}

BosonEffect::~BosonEffect()
{
}

void BosonEffect::doDelayedUpdates()
{
  if(mUpdateCounter > maxDelayedUpdates())
  {
    mUpdateCounter = maxDelayedUpdates();
  }
  while(mUpdateCounter > 0)
  {
    update(0.05f);
    mUpdateCounter--;
  }
}

void BosonEffect::markUpdate(float elapsed)
{
  if(supportsDelayedUpdates())
  {
    // TODO: make sure elapsed it 0.05f
    mUpdateCounter++;
  }
  else
  {
    update(elapsed);
  }
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
  }
  else
  {
    // Effects without properties can't be saved for now. So we return false,
    //  which prevents saving anything
    return false;
  }
  // Position and rotation
  BoVector3Fixed canvasPos = mPosition;
  canvasPos[1] *= -1; // AB: loading expects _canvas_ position of effects
  saveVector3AsXML(canvasPos, root, "Position");
  saveVector3AsXML(mRotation, root, "Rotation");

  // Misc
  root.setAttribute(QString::fromLatin1("Active"), mActive ? 1 : 0);
  root.setAttribute(QString::fromLatin1("Started"), mStarted ? 1 : 0);
  root.setAttribute(QString::fromLatin1("Delay"), mDelay);
  root.setAttribute(QString::fromLatin1("OwnerId"), mOwnerId);

  return true;
}

bool BosonEffect::loadFromXML(const QDomElement& root)
{
  bool ok;
  mActive = (root.attribute("Active").toInt(&ok));
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading Active attribute ('" << root.attribute("Active") << "')" << endl;
    return false;
  }
  mStarted = (root.attribute("Started").toInt(&ok));
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading Started attribute ('" << root.attribute("Started") << "')" << endl;
    return false;
  }
  mDelay = root.attribute("Delay").toFloat(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading Delay attribute ('" << root.attribute("Delay") << "')" << endl;
    return false;
  }
  mOwnerId = root.attribute("OwnerId").toUInt(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading OwnerId attribute ('" << root.attribute("OwnerId") << "')" << endl;
    return false;
  }

  if(!loadVector3FromXML(&mPosition, root, "Position"))
  {
    return false;
  }
  if(!loadVector3FromXML(&mRotation, root, "Rotation"))
  {
    return false;
  }

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

BosonEffectFog::BosonEffectFog(const BoVector4Float& color, float start, float end, float radius) :
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

  saveVector4AsXML(mColor, root, "Color");
  root.setAttribute(QString::fromLatin1("Start"), mStart);
  root.setAttribute(QString::fromLatin1("End"), mEnd);
  root.setAttribute(QString::fromLatin1("Radius"), mRadius);

  return true;
}

bool BosonEffectFog::loadFromXML(const QDomElement& root)
{
  if(!BosonEffect::loadFromXML(root))
  {
    return false;
  }

  bool ok;
  if(loadVector4FromXML(&mColor, root, "Color"))
  {
    return false;
  }
  mStart = root.attribute("Start").toFloat(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading Start attribute ('" << root.attribute("Start") << "')" << endl;
    return false;
  }
  mEnd = root.attribute("End").toFloat(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading End attribute ('" << root.attribute("End") << "')" << endl;
    return false;
  }
  mRadius = root.attribute("Radius").toFloat(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading Radius attribute ('" << root.attribute("Radius") << "')" << endl;
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

BoShader* BosonEffectFade::shader(int pass) const
{
  return mProperties->shader(pass);
}

int BosonEffectFade::downscale(int pass) const
{
 return mProperties->downscale(pass);
}

int BosonEffectFade::passes() const
{
 return mProperties->passes();
}

bool BosonEffectFade::saveAsXML(QDomElement& root) const
{
  if(!BosonEffect::saveAsXML(root))
  {
    return false;
  }

  root.setAttribute(QString::fromLatin1("TimeLeft"), mTimeLeft);
  saveVector4AsXML(mColor, root, "Color");
  saveVector4AsXML(mGeometry, root, "Geometry");
  root.setAttribute(QString::fromLatin1("BlendFunc.src"), mBlendFunc[0]);
  root.setAttribute(QString::fromLatin1("BlendFunc.dst"), mBlendFunc[1]);

  return true;
}

bool BosonEffectFade::loadFromXML(const QDomElement& root)
{
  if(!BosonEffect::loadFromXML(root))
  {
    return false;
  }

  bool ok;
  mTimeLeft = root.attribute("TimeLeft").toFloat(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading TimeLeft attribute ('" << root.attribute("TimeLeft") << "')" << endl;
    return false;
  }
  if(!loadVector4FromXML(&mColor, root, "Color"))
  {
    return false;
  }
  if(!loadVector4FromXML(&mGeometry, root, "Geometry"))
  {
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
  if(mLight)
  {
    BoLightManager::manager()->deleteLight(mLight->id());
  }
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
    if(mLight)
    {
      BoLightManager::manager()->deleteLight(mLight->id());
    }
    mLight = 0;
    return;
  }

  float factor = mTimeLeft / mProperties->life();  // This goes from 1 to 0 during effect's lifetime
  BoVector4Float ambient, diffuse, specular;
  BoVector3Float attenuation;
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

  mLight = BoLightManager::manager()->createLight();
  mLight->setEnabled(true);
  mLight->setDirectional(false);
  mLight->setPosition3(position().toFloat());
}

void BosonEffectLight::setPosition(const BoVector3Fixed& pos)
{
  // Add properties->position() to given pos
  BoVector3Fixed newpos = pos + mProperties->position();
  BosonEffect::setPosition(newpos);
  if(mLight)
  {
    mLight->setPosition3(newpos.toFloat());
  }
}

void BosonEffectLight::makeObsolete()
{
  if(mTimeLeft > 0)
  {
    return;
  }
  if(mLight)
  {
    BoLightManager::manager()->deleteLight(mLight->id());
  }
  mLight = 0;
  BosonEffect::makeObsolete();
}

bool BosonEffectLight::saveAsXML(QDomElement& root) const
{
  if(!BosonEffect::saveAsXML(root))
  {
    return false;
  }

  root.setAttribute(QString::fromLatin1("TimeLeft"), mTimeLeft);

  return true;
}

bool BosonEffectLight::loadFromXML(const QDomElement& root)
{
  if(!BosonEffect::loadFromXML(root))
  {
    return false;
  }

  bool ok;
  mTimeLeft = root.attribute("TimeLeft").toFloat(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading TimeLeft attribute ('" << root.attribute("TimeLeft") << "')" << endl;
    return false;
  }

  // Create light only if we've started
  if(!hasStarted())
  {
    return true;
  }

  // This creates BoLight object
  start();

  float factor = mTimeLeft / mProperties->life();  // This goes from 1 to 0 during effect's lifetime
  BoVector4Float ambient, diffuse, specular;
  BoVector3Float attenuation;
  ambient.setBlended(mProperties->startAmbient(), factor, mProperties->endAmbient(), 1.0 - factor);
  diffuse.setBlended(mProperties->startDiffuse(), factor, mProperties->endDiffuse(), 1.0 - factor);
  specular.setBlended(mProperties->startSpecular(), factor, mProperties->endSpecular(), 1.0 - factor);
  attenuation.setBlended(mProperties->startAttenuation(), factor, mProperties->endAttenuation(), 1.0 - factor);
  mLight->setAmbient(ambient);
  mLight->setDiffuse(diffuse);
  mLight->setSpecular(specular);
  mLight->setAttenuation(attenuation);

  return true;
}



/*****  BosonEffectBulletTrail  *****/

BosonEffectBulletTrail::BosonEffectBulletTrail(const BosonEffectPropertiesBulletTrail* prop, const BoVector3Fixed& pos) : BosonEffect(prop)
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

  BoVector3Fixed movevector = mPosition - mLastPos;
  float movelength = movevector.length();
  movevector.scale(1 / movelength); // Normalize movevector
  float length = getRandomFloat(mProperties->minLength(), mProperties->maxLength());  // Line's length
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
    float pos = getRandomFloat(0, movelength - length);
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

const BoVector4Float& BosonEffectBulletTrail::color() const
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

  saveVector3AsXML(mLastPos, root, "LastPos");
  saveVector3AsXML(mStart, root, "Start");
  saveVector3AsXML(mEnd, root, "End");
  root.setAttribute("Advanced", mAdvanced);
  root.setAttribute("ShouldMakeObsolete", mShouldMakeObsolete ? 1 : 0);

  return true;
}

bool BosonEffectBulletTrail::loadFromXML(const QDomElement& root)
{
  if(!BosonEffect::loadFromXML(root))
  {
    return false;
  }

  if(!loadVector3FromXML(&mLastPos, root, "LastPos"))
  {
    return false;
  }
  if(!loadVector3FromXML(&mStart, root, "Start"))
  {
    return false;
  }
  if(!loadVector3FromXML(&mEnd, root, "End"))
  {
    return false;
  }
  bool ok;
  mShouldMakeObsolete = (root.attribute("ShouldMakeObsolete").toInt(&ok));
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading ShouldMakeObsolete attribute ('" << root.attribute("ShouldMakeObsolete") << "')" << endl;
    return false;
  }
  mAdvanced = root.attribute("Advanced").toShort(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading Advanced attribute ('" << root.attribute("Advanced") << "')" << endl;
    return false;
  }

  return true;
}

/*
 * vim: et sw=2
 */
