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


#include "bosoneffectproperties.h"


#include <ksimpleconfig.h>

#include <qstring.h>

#include "bosoneffectpropertiesparticle.h"
#include "speciestheme.h"
#include "bosonconfig.h"
#include "bodebug.h"
#include "speciesdata.h"


/*****  BosonEffectPropertiesFactory  *****/

BosonEffectProperties* BosonEffectPropertiesFactory::loadEffectProperties(KSimpleConfig* cfg, const QString& group)
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

BosonEffectProperties* BosonEffectPropertiesFactory::newEffectProperties(const QString& type)
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

BosonEffectProperties* BosonEffectPropertiesFactory::newParticleEffectProperties(const QString& type)
{
  if(type == "ParticleGeneric")
  {
    return new BosonEffectPropertiesParticleGeneric();
  }
  else if(type == "ParticleTrail")
  {
    return new BosonEffectPropertiesParticleTrail();
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

QPtrList<BosonEffectProperties> BosonEffectProperties::loadEffectProperties(KSimpleConfig* cfg, QString key, SpeciesTheme* theme)
{
  return loadEffectProperties(BosonConfig::readUnsignedLongNumList(cfg, key), theme);
}

QPtrList<BosonEffectProperties> BosonEffectProperties::loadEffectProperties(QValueList<unsigned long int> ids, SpeciesTheme* theme)
{
  QPtrList<BosonEffectProperties> props;
  QValueList<unsigned long int>::Iterator it;
  for(it = ids.begin(); it != ids.end(); it++)
  {
    props.append(theme->effectProperties(*it));
  }
  return props;
}

QPtrList<BosonEffect> BosonEffectProperties::newEffects(const QPtrList<BosonEffectProperties>* properties,
    const BoVector3& pos, const BoVector3& rot)
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
    const BoVector3& pos, const BoVector3& rot)
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

  mColor = BosonConfig::readBoVector4Entry(cfg, "Color", mColor);
  mStart = (float)(cfg->readDoubleNumEntry("Start", mStart));
  mEnd = (float)(cfg->readDoubleNumEntry("End", mEnd));
  mRadius = (float)(cfg->readDoubleNumEntry("Radius", mRadius));
  return true;
}

BosonEffect* BosonEffectPropertiesFog::newEffect(const BoVector3& pos, const BoVector3&) const
{
  BosonEffectFog* fog = new BosonEffectFog(mColor, mStart, mEnd);
  fog->setPosition(pos);
  return fog;
}



/*****  BosonEffectPropertiesFade  *****/

BosonEffectPropertiesFade::BosonEffectPropertiesFade() : BosonEffectProperties()
{
  reset();
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
}

bool BosonEffectPropertiesFade::load(KSimpleConfig* cfg, const QString& group, bool inherited)
{
  if(!BosonEffectProperties::load(cfg, group, inherited))
  {
    return false;
  }

  mStartColor = BosonConfig::readBoVector4Entry(cfg, "StartColor", mStartColor);
  mEndColor = BosonConfig::readBoVector4Entry(cfg, "EndColor", mEndColor);
  mGeometry = BosonConfig::readBoVector4Entry(cfg, "Geometry", mGeometry);
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
  return true;
}

BosonEffect* BosonEffectPropertiesFade::newEffect(const BoVector3& pos, const BoVector3&) const
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

  mStartAmbientColor = BosonConfig::readBoVector4Entry(cfg, "StartAmbient", mStartAmbientColor);
  mStartDiffuseColor = BosonConfig::readBoVector4Entry(cfg, "StartDiffuse", mStartDiffuseColor);
  mStartSpecularColor = BosonConfig::readBoVector4Entry(cfg, "StartSpecular", mStartSpecularColor);
  mEndAmbientColor = BosonConfig::readBoVector4Entry(cfg, "EndAmbient", mEndAmbientColor);
  mEndDiffuseColor = BosonConfig::readBoVector4Entry(cfg, "EndDiffuse", mEndDiffuseColor);
  mEndSpecularColor = BosonConfig::readBoVector4Entry(cfg, "EndSpecular", mEndSpecularColor);
  mStartAttenuation = BosonConfig::readBoVector3Entry(cfg, "StartAttenuation", mStartAttenuation);
  mEndAttenuation = BosonConfig::readBoVector3Entry(cfg, "EndAttenuation", mEndAttenuation);
  mPosition = BosonConfig::readBoVector3Entry(cfg, "Position", mPosition);
  mLife = (float)(cfg->readDoubleNumEntry("Life", mLife));
  return true;
}

BosonEffect* BosonEffectPropertiesLight::newEffect(const BoVector3& pos, const BoVector3&) const
{
  BoVector3 worldpos = pos;
  worldpos.canvasToWorld();
  BosonEffectLight* light = new BosonEffectLight(this);
  light->setPosition(worldpos);
  return light;
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

bool BosonEffectPropertiesCollection::finishLoading(const SpeciesData* theme)
{
  // Properties objects haven't been loaded yet, see comment in load()
  QValueList<unsigned long int>::Iterator it;
  for(it = mEffectIds.begin(); it != mEffectIds.end(); it++)
  {
    mEffects.append(theme->effectProperties(*it));
  }
  return true;
}

BosonEffect* BosonEffectPropertiesCollection::newEffect(const BoVector3& pos, const BoVector3&) const
{
  // BosonEffectPropertiesCollection is special, newEffectsList() should be used
  //  instead of this method
  boWarning() << k_funcinfo << "Use newEffectsList() instead!" << endl;
  return 0;
}

QPtrList<BosonEffect> BosonEffectPropertiesCollection::newEffectsList(const BoVector3& pos, const BoVector3& rot) const
{
  QPtrList<BosonEffect> list;
  QPtrListIterator<BosonEffectProperties> it(mEffects);
  while(it.current())
  {
    // We can't just do 'list.append(it.current()->newEffect(pos, rot));'
    //  because it.current() may also be effects collection.
    QPtrList<BosonEffect> e = newEffects(it.current(), pos, rot);
    if(e.count() == 1)
    {
      list.append(e.first());
    }
    else
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

