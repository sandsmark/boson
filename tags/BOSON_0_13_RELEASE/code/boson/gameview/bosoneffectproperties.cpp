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


#include "bosoneffectproperties.h"

#include "../../bomemory/bodummymemory.h"
#include "bosoneffectpropertiesparticle.h"
#include "../gameengine/speciestheme.h"
#include "../bosonconfig.h"
#include "bodebug.h"
#include "../speciesdata.h"
#include "../boshader.h"

#include <ksimpleconfig.h>
#include <kglobal.h>
#include <kstandarddirs.h>

#include <qstring.h>


/*****  BosonEffectPropertiesManager  *****/

BosonEffectPropertiesManager* BosonEffectPropertiesManager::mManager = 0;

void BosonEffectPropertiesManager::initStatic()
{
  if(!mManager)
  {
    mManager = new BosonEffectPropertiesManager();
  }
}

void BosonEffectPropertiesManager::deleteStatic()
{
  delete mManager;
  mManager = 0;
}

BosonEffectPropertiesManager::BosonEffectPropertiesManager()
{
  mEffectProperties.setAutoDelete(true);
}

BosonEffectPropertiesManager::~BosonEffectPropertiesManager()
{
  mEffectProperties.clear();
}

BosonEffectPropertiesManager* BosonEffectPropertiesManager::bosonEffectPropertiesManager()
{
  if(!mManager)
  {
    boError() << k_funcinfo << "initStatic() has not been called" << endl;
  }
  return mManager;
}

void BosonEffectPropertiesManager::loadEffectProperties()
{
  if(mEffectProperties.count() > 0)
  {
    // already loaded
    return;
  }

  QString path = KGlobal::dirs()->findResourceDir("data", "boson/themes/effects/effects.boson");
  if(path.isNull())
  {
    boWarning() << k_funcinfo << "No effects.boson file found!" << endl;
    return;
  }
  path += "boson/themes/effects/";
  QString fileName = path + "effects.boson";

  BosonEffect::initStatic(path + "particles/");

  KSimpleConfig cfg(fileName);
  QStringList effects = cfg.groupList();
  if(effects.isEmpty())
  {
    boWarning() << k_funcinfo << "No effects found in effects file (" << fileName << ")" << endl;
    return;
  }
  boDebug(150) << k_funcinfo << "Loading " << effects.count()
      << " effects from config file" << endl;
  QStringList::Iterator it;
  for(it = effects.begin(); it != effects.end(); ++it)
  {
    boDebug(150) << k_funcinfo << "Loading effect from group " << *it << endl;
    BosonEffectProperties* effectprop = loadEffectProperties(&cfg, *it);
    if(!effectprop)
    {
      // Error has already been given
      continue;
    }
    if(!mEffectProperties.find(effectprop->id()))
    {
      mEffectProperties.insert(effectprop->id(), effectprop);
    }
    else
    {
      boError(150) << k_funcinfo << "effect with id " << effectprop->id() << " already there!" << endl;
      delete effectprop;
    }
  }
  // BosonEffectProperties (more specifically, collection properties) need
  //  2-stage loading
  QIntDictIterator<BosonEffectProperties> eit(mEffectProperties);
  while(eit.current())
  {
    eit.current()->finishLoading(this);
    ++eit;
  }
}


const BosonEffectProperties* BosonEffectPropertiesManager::effectProperties(unsigned long int id) const
{
  if(id == 0)
  {
    // We don't print error here because 0 means "none" in configurations
    return 0;
  }
  if(!mEffectProperties[id])
  {
    boError() << k_funcinfo << "oops - no effect properties for " << id << endl;
    return 0;
  }
  return mEffectProperties[id];
}

BosonEffectProperties* BosonEffectPropertiesManager::loadEffectProperties(KSimpleConfig* cfg, const QString& group)
{
  cfg->setGroup(group);

  // Find out effect's type
  QString type = cfg->readEntry("Type", QString::null);

  // Loop until we have type
  while(type == QString::null)
  {
    // No type was given
    // Effect may be inheriting from another effect
    QString inherits = cfg->readEntry("Inherits", QString::null);
    if(inherits == QString::null)
    {
      // These properties inherit nothing and type is still null, so we have an
      //  error.
      boError() << k_funcinfo << "No type found for " << group << endl;
      return 0;
    }
    cfg->setGroup(inherits);
    type = cfg->readEntry("Type", QString::null);
  }

  // Create new properties object
  BosonEffectProperties* prop = newEffectProperties(type);
  if(!prop)
  {
    boError() << k_funcinfo << "Couldn't load effect properties for group '" << group << "'" << endl;
    return 0;
  }

  // Load properties
  prop->load(cfg, group);
  return prop;
}

BosonEffectProperties* BosonEffectPropertiesManager::newEffectProperties(const QString& type)
{
  if(type.left(8) == "Particle")
  {
    return newParticleEffectProperties(type);
  }
  else if(type == "Fog")
  {
    return new BosonEffectPropertiesFog();
  }
  else if(type == "Fade")
  {
    return new BosonEffectPropertiesFade();
  }
  else if(type == "Light")
  {
    return new BosonEffectPropertiesLight();
  }
  else if(type == "BulletTrail")
  {
    return new BosonEffectPropertiesBulletTrail();
  }
  else if(type == "Collection")
  {
    return new BosonEffectPropertiesCollection();
  }
  else
  {
    boError() << k_funcinfo << "Invalid type '" << type << "'" << endl;
    return 0;
  }
}

BosonEffectProperties* BosonEffectPropertiesManager::newParticleEffectProperties(const QString& type)
{
  if(type == "ParticleGeneric")
  {
    return new BosonEffectPropertiesParticleGeneric();
  }
  else if(type == "ParticleTrail")
  {
    return new BosonEffectPropertiesParticleTrail();
  }
  else if(type == "ParticleEnvironmental")
  {
    return new BosonEffectPropertiesParticleEnvironmental();
  }
  else
  {
    boError() << k_funcinfo << "Invalid type '" << type << "'" << endl;
    return 0;
  }
}


/*****  BosonEffectProperties  *****/

BosonEffectProperties::BosonEffectProperties()
{
  mId = 0;
  mDelay = 0;
}

BosonEffectProperties::~BosonEffectProperties()
{
}

bool BosonEffectProperties::load(KSimpleConfig* cfg, const QString& group, bool inherited)
{
  // Set correct group
  cfg->setGroup(group);
  // Unless we're loading inherited effect, load the id now (as the first thing)
  if(!inherited)
  {
    mId = cfg->readUnsignedLongNumEntry("Id", 0);
    if(mId == 0)
    {
      boError(150) << k_funcinfo << "Invalid or missing id in group " << cfg->group() << endl;
      return false;  // Loading failed
    }
  }

  mDelay = (float)(cfg->readDoubleNumEntry("Delay", mDelay));

  // Load inherited properties, if any
  QString inherits = cfg->readEntry("Inherits", QString::null);
  if(!inherits.isNull())
  {
    boDebug(150) << k_funcinfo << "Loading inhereted system from group " << inherits << endl;
    load(cfg, inherits, true);
    cfg->setGroup(group);
  }

  return true;
}

QPtrList<BosonEffectProperties> BosonEffectProperties::loadEffectProperties(KSimpleConfig* cfg, QString key)
{
  return loadEffectProperties(BosonConfig::readUnsignedLongNumList(cfg, key));
}

QPtrList<BosonEffectProperties> BosonEffectProperties::loadEffectProperties(const QValueList<unsigned long int>& ids)
{
  QPtrList<BosonEffectProperties> props;
  QValueList<unsigned long int>::const_iterator it;
  for(it = ids.begin(); it != ids.end(); it++)
  {
    props.append(boEffectPropertiesManager->effectProperties(*it));
  }
  return props;
}

QPtrList<BosonEffect> BosonEffectProperties::newEffects(const QPtrList<BosonEffectProperties>* properties,
    const BoVector3Fixed& pos, const BoVector3Fixed& rot)
{
  QPtrList<BosonEffect> list;
  QPtrListIterator<BosonEffectProperties> it(*properties);
  while(it.current())
  {
    // FIXME: bad? maybe make newEffect() return QPtrList for all effect properties?
    if(it.current()->type() == BosonEffect::Collection)
    {
      QPtrList<BosonEffect> effects = ((BosonEffectPropertiesCollection*)it.current())->newEffectsList(pos, rot);
      QPtrListIterator<BosonEffect> eit(effects);
      while(eit.current())
      {
        list.append(eit.current());
        ++eit;
      }
    }
    else
    {
      BosonEffect* e = it.current()->newEffect(pos, rot);
      if (e) {
        list.append(e);
      }
    }
    ++it;
  }
  return list;
}

QPtrList<BosonEffect> BosonEffectProperties::newEffects(const BosonEffectProperties* properties,
    const BoVector3Fixed& pos, const BoVector3Fixed& rot)
{
  QPtrList<BosonEffect> list;
  // FIXME: bad? maybe make newEffect() return QPtrList for all effect properties?
  if(properties->type() == BosonEffect::Collection)
  {
    QPtrList<BosonEffect> effects = ((BosonEffectPropertiesCollection*)properties)->newEffectsList(pos, rot);
    QPtrListIterator<BosonEffect> eit(effects);
    while(eit.current())
    {
      list.append(eit.current());
      ++eit;
    }
  }
  else
  {
    BosonEffect* e = properties->newEffect(pos, rot);
    if (e) {
      list.append(e);
    }
  }
  return list;
}



/*****  BosonEffectPropertiesFog  *****/

BosonEffectPropertiesFog::BosonEffectPropertiesFog() : BosonEffectProperties()
{
  reset();
}

void BosonEffectPropertiesFog::reset()
{
  // Reset all variables to their default values
  mColor.reset();
  mStart = 0;
  mEnd = 0;
  mRadius = 0;
}

bool BosonEffectPropertiesFog::load(KSimpleConfig* cfg, const QString& group, bool inherited)
{
  if(!BosonEffectProperties::load(cfg, group, inherited))
  {
    return false;
  }

  mColor = BosonConfig::readBoVector4FloatEntry(cfg, "Color", mColor);
  mStart = (float)(cfg->readDoubleNumEntry("Start", mStart));
  mEnd = (float)(cfg->readDoubleNumEntry("End", mEnd));
  mRadius = (float)(cfg->readDoubleNumEntry("Radius", mRadius));
  return true;
}

BosonEffect* BosonEffectPropertiesFog::newEffect(const BoVector3Fixed& pos, const BoVector3Fixed&) const
{
  BoVector3Fixed worldpos = pos;
  worldpos.canvasToWorld();
  BosonEffectFog* fog = new BosonEffectFog(this);
  fog->setPosition(worldpos);
  return fog;
}



/*****  BosonEffectPropertiesFade  *****/

BosonEffectPropertiesFade::BosonEffectPropertiesFade() : BosonEffectProperties()
{
  reset();
}

BosonEffectPropertiesFade::~BosonEffectPropertiesFade()
{
  for (int i = 0; i < mPasses; i++)
  {
    delete mShader[i];
  }
  delete[] mShader;
  delete[] mShaderFilename;
  delete[] mDownscaleFactor;
}

void BosonEffectPropertiesFade::reset()
{
  // Reset all variables to their default values
  mStartColor.reset();
  mEndColor.reset();
  mGeometry.reset();
  mTime = 0;
  mBlendFunc[0] = GL_SRC_ALPHA;
  mBlendFunc[1] = GL_ONE_MINUS_SRC_ALPHA;

  mPasses = 0;
  mShader = 0;
  mShaderFilename = 0;
  mDownscaleFactor = 0;
}

bool BosonEffectPropertiesFade::load(KSimpleConfig* cfg, const QString& group, bool inherited)
{
  if(!BosonEffectProperties::load(cfg, group, inherited))
  {
    return false;
  }

  mStartColor = BosonConfig::readBoVector4FloatEntry(cfg, "StartColor", mStartColor);
  mEndColor = BosonConfig::readBoVector4FloatEntry(cfg, "EndColor", mEndColor);
  mGeometry = BosonConfig::readBoVector4FixedEntry(cfg, "Geometry", mGeometry);
  mTime = (float)(cfg->readDoubleNumEntry("Time", mTime));
  int glblendfunc = Bo3dTools::string2GLBlendFunc(cfg->readEntry("BlendFunc", QString::null));
  if(glblendfunc != GL_INVALID_ENUM)
  {
    mBlendFunc[1] = glblendfunc;
  }
  glblendfunc = Bo3dTools::string2GLBlendFunc(cfg->readEntry("SrcBlendFunc", QString::null));
  if(glblendfunc != GL_INVALID_ENUM)
  {
    mBlendFunc[0] = glblendfunc;
  }

  if(inherited)
  {
    // Pass-dependant stuff and number of passes do not support inheritance at this point
    return true;
  }

  // Load the number of passes
  mPasses = cfg->readNumEntry("Passes", mPasses);
  if(mPasses < 0 || mPasses > 10)
  {
    boError() << k_funcinfo << "Passes count of out bounds: " << mPasses << ", resetting to 0" << endl;
    mPasses = 0;
  }

  // If we don't use shader, then that's it
  if(mPasses == 0)
  {
    return true;
  }

  // Create arrays for everything which depends on the number of passes
  mShader = new BoShader*[mPasses];
  mShaderFilename = new QString[mPasses];
  mDownscaleFactor = new int[mPasses];
  for(int i = 0; i < mPasses; i++)
  {
    mShader[i] = 0;
  }

  // And load pass-dependand values
  mShaderFilename[0] = cfg->readEntry("Shader");
  mDownscaleFactor[0] = cfg->readNumEntry("Downscale", 1);

  for(int i = 1; i < mPasses; i++)
  {
    mShaderFilename[i] = cfg->readEntry(QString("Shader%1").arg(i+1));
    mDownscaleFactor[i] = cfg->readNumEntry(QString("Downscale%1").arg(i+1), 1);
  }

  // Load shaders
  for(int i = 0; i < mPasses; i++)
  {
    if(mShaderFilename[i].isEmpty())
    {
      boError() << k_funcinfo << "Empty shader filename for pass " << i+1 << endl;
      return false;
    }
    boDebug(150) << k_funcinfo << "Loading shader of pass " << i+1 << ": " << mShaderFilename[i] << endl;
    mShader[i] = new BoShader(mShaderFilename[i]);
    // TODO: check if shader was loaded correctly and return false if it wasn't
  }

  return true;
}

BosonEffect* BosonEffectPropertiesFade::newEffect(const BoVector3Fixed& pos, const BoVector3Fixed&) const
{
  BosonEffectFade* fade = new BosonEffectFade(this);
  fade->setPosition(pos);
  return fade;
}



/*****  BosonEffectPropertiesLight  *****/

BosonEffectPropertiesLight::BosonEffectPropertiesLight() : BosonEffectProperties()
{
  reset();
}

void BosonEffectPropertiesLight::reset()
{
  // Reset all variables to their default values
  mStartAmbientColor.reset();
  mStartDiffuseColor.reset();
  mStartSpecularColor.reset();
  mEndAmbientColor.reset();
  mEndDiffuseColor.reset();
  mEndSpecularColor.reset();
  mStartAttenuation.reset();
  mEndAttenuation.reset();
  mPosition.reset();
  mLife = 0.0f;
}

bool BosonEffectPropertiesLight::load(KSimpleConfig* cfg, const QString& group, bool inherited)
{
  if(!BosonEffectProperties::load(cfg, group, inherited))
  {
    return false;
  }

  mStartAmbientColor = BosonConfig::readBoVector4FloatEntry(cfg, "StartAmbient", mStartAmbientColor);
  mStartDiffuseColor = BosonConfig::readBoVector4FloatEntry(cfg, "StartDiffuse", mStartDiffuseColor);
  mStartSpecularColor = BosonConfig::readBoVector4FloatEntry(cfg, "StartSpecular", mStartSpecularColor);
  mEndAmbientColor = BosonConfig::readBoVector4FloatEntry(cfg, "EndAmbient", mEndAmbientColor);
  mEndDiffuseColor = BosonConfig::readBoVector4FloatEntry(cfg, "EndDiffuse", mEndDiffuseColor);
  mEndSpecularColor = BosonConfig::readBoVector4FloatEntry(cfg, "EndSpecular", mEndSpecularColor);
  mStartAttenuation = BosonConfig::readBoVector3FloatEntry(cfg, "StartAttenuation", mStartAttenuation);
  mEndAttenuation = BosonConfig::readBoVector3FloatEntry(cfg, "EndAttenuation", mEndAttenuation);
  mPosition = BosonConfig::readBoVector3FixedEntry(cfg, "Position", mPosition);
  mLife = (float)(cfg->readDoubleNumEntry("Life", mLife));
  return true;
}

BosonEffect* BosonEffectPropertiesLight::newEffect(const BoVector3Fixed& pos, const BoVector3Fixed&) const
{
  BoVector3Fixed worldpos = pos;
  worldpos.canvasToWorld();
  BosonEffectLight* light = new BosonEffectLight(this);
  light->setPosition(worldpos);
  return light;
}



/*****  BosonEffectPropertiesBulletTrail  *****/

BosonEffectPropertiesBulletTrail::BosonEffectPropertiesBulletTrail() : BosonEffectProperties()
{
  reset();
}

void BosonEffectPropertiesBulletTrail::reset()
{
  // Reset all variables to their default values
  mColor.reset();
  mMinLength = 0.0f;
  mMaxLength = 0.0f;
  mWidth = 1.0f;
  mProbability = 1.0f;
}

bool BosonEffectPropertiesBulletTrail::load(KSimpleConfig* cfg, const QString& group, bool inherited)
{
  if(!BosonEffectProperties::load(cfg, group, inherited))
  {
    return false;
  }

  mColor = BosonConfig::readBoVector4FloatEntry(cfg, "Color", mColor);
  mMinLength = (float)(cfg->readDoubleNumEntry("MinLength", mMinLength));
  mMaxLength = (float)(cfg->readDoubleNumEntry("MaxLength", mMaxLength));
  mWidth = (float)(cfg->readDoubleNumEntry("Width", mWidth));
  mProbability = (float)(cfg->readDoubleNumEntry("Probability", mProbability));

  return true;
}

BosonEffect* BosonEffectPropertiesBulletTrail::newEffect(const BoVector3Fixed& pos, const BoVector3Fixed&) const
{
  if(mProbability < 1.0f)
  {
    if(BosonEffect::getRandomFloat(0.0f, 1.0f) <= mProbability)
    {
      return 0;
    }
  }

  BoVector3Fixed worldpos = pos;
  worldpos.canvasToWorld();
  BosonEffectBulletTrail* line = new BosonEffectBulletTrail(this, worldpos);
  return line;
}



/*****  BosonEffectPropertiesCollection  *****/

BosonEffectPropertiesCollection::BosonEffectPropertiesCollection() : BosonEffectProperties()
{
  reset();
}

void BosonEffectPropertiesCollection::reset()
{
  mEffects.clear();
  mEffectIds.clear();
}

bool BosonEffectPropertiesCollection::load(KSimpleConfig* cfg, const QString& group, bool inherited)
{
  // FIXME: inheritance doesn't work here
  // TODO: maybe make inheritance so that every inheriting effect _appends_ it's
  //  ids to parent's ids, not replace them.
  if(!BosonEffectProperties::load(cfg, group, inherited))
  {
    return false;
  }

  mEffectIds = BosonConfig::readUnsignedLongNumList(cfg, "Effects");
  // We can't add BosonEffectProperties objects to the list, because they might
  //  be not loaded yet. We'll load them in finishLoading()

  return true;
}

bool BosonEffectPropertiesCollection::finishLoading(const BosonEffectPropertiesManager* manager)
{
  QValueList<unsigned long int>::Iterator it;
  for(it = mEffectIds.begin(); it != mEffectIds.end(); it++)
  {
    mEffects.append(manager->effectProperties(*it));
  }
  return true;
}

BosonEffect* BosonEffectPropertiesCollection::newEffect(const BoVector3Fixed& pos, const BoVector3Fixed&) const
{
  // BosonEffectPropertiesCollection is special, newEffectsList() should be used
  //  instead of this method
  boWarning() << k_funcinfo << "Use newEffectsList() instead!" << endl;
  return 0;
}

QPtrList<BosonEffect> BosonEffectPropertiesCollection::newEffectsList(const BoVector3Fixed& pos, const BoVector3Fixed& rot) const
{
  QPtrList<BosonEffect> list;
  QPtrListIterator<BosonEffectProperties> it(mEffects);
  while(it.current())
  {
    // We can't just do 'list.append(it.current()->newEffect(pos, rot));'
    //  because it.current() may also be effects collection.
    // TODO: support delay!
    //  maybe increase delay of created effect by delay of collection effect
    QPtrList<BosonEffect> e = newEffects(it.current(), pos, rot);
    if(e.count() == 1)
    {
      list.append(e.first());
    }
    else if(e.count() > 0)
    {
      QPtrListIterator<BosonEffect> eit(e);
      while(eit.current())
      {
        list.append(eit.current());
        ++eit;
      }
    }
    ++it;
  }
  return list;
}

