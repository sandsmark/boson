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


#include "bosoneffectpropertiesparticle.h"

#include "../bomemory/bodummymemory.h"
#include "botexture.h"
#include "bosoneffectparticle.h"
#include "bodebug.h"
#include "bosonconfig.h"

#include <KConfig>
#include <KConfigGroup>

#include <qstring.h>
#include <qdir.h>
#include <qstringlist.h>

#include <math.h>


/*****  BosonEffectPropertiesParticle  *****/

#warning this is never freed!
Q3Dict<BoTextureArray> BosonEffectPropertiesParticle::mTextureArrays;
QString BosonEffectPropertiesParticle::mTexturePath;
BoVector3Float BosonEffectPropertiesParticle::mWind(0.2, 0.1, 0.0);

BosonEffectPropertiesParticle::BosonEffectPropertiesParticle() :
    BosonEffectProperties()
{
}

bool BosonEffectPropertiesParticle::load(KConfig* cfg, const QString& groupName, bool inherited)
{
  if(!BosonEffectProperties::load(cfg, groupName, inherited))
  {
    return false;
  }

  // Nothing to do here (ATM)

  return true;
}

void BosonEffectPropertiesParticle::initStatic(const QString& texdir)
{
  mTexturePath = texdir;
}

const BoTextureArray* BosonEffectPropertiesParticle::getTextures(const QString& name)
{
  if(!mTextureArrays.find(name))
  {
    boDebug(150) << k_funcinfo << "Adding texture with name " << name << " to textures arrays map" << endl;
    QDir d(mTexturePath);
    QStringList files = d.entryList(name + "*.png", QDir::Files, QDir::Name);
    boDebug(150) << k_funcinfo << "Found " << files.count() << " suitable files" << endl;
    QStringList absFiles;
    for(QStringList::Iterator it = files.begin(); it != files.end(); it++)
    {
      absFiles.append(mTexturePath + "/" + *it);
    }
    BoTextureArray* t = new BoTextureArray(absFiles, BoTexture::Particle);

    mTextureArrays.insert(name, t);

    // AB: this makes boson crash on destruction, as the texture arrays access
    // the texture manager which accesses the BosonConfig object which already
    // has been destructed
    // to fix this:
    // REMOVE THE GLOBAL OBJECT !!!
#if 0
    mTextureArrays.setAutoDelete(true);
#endif
  }
  return mTextureArrays[name];
}


/*****  BosonEffectPropertiesParticleGeneric  *****/

BosonEffectPropertiesParticleGeneric::BosonEffectPropertiesParticleGeneric() :
    BosonEffectPropertiesParticle()
{
  reset();
}

void BosonEffectPropertiesParticleGeneric::reset()
{
  // Reset all variables to their default values
  mMinVelo.reset();
  mMaxVelo.reset();
  mMinPos.reset();
  mMaxPos.reset();
  mNormalizePos = false;
  mMinPosScale = 1.0f;
  mMaxPosScale = 1.0f;
  mNormalizeVelo = false;
  mMinVeloScale = 1.0f;
  mMaxVeloScale = 1.0f;
  mStartColor.reset();
  mEndColor.reset();
  mMinLife = 1.0f;
  mMaxLife = 1.0f;
  mMaxNum = 100;
  mInitNum = 0;
  mGLBlendFuncStr = "GL_ONE_MINUS_SRC_ALPHA";
  mGLSrcBlendFuncStr = "GL_SRC_ALPHA";
  mRate = 0.0f;
  mStartSize = 1.0f;
  mEndSize = 1.0f;
  mAge = 0.0f;
  mMass = 1.0f;
  mParticleDist = 0.0f;
  mAlign = true;
  mMoveParticlesWithSystem = false;
  mTextureName = "explosion";
}

bool BosonEffectPropertiesParticleGeneric::load(KConfig* cfg, const QString& groupName, bool inherited)
{
  if(!BosonEffectPropertiesParticle::load(cfg, groupName, inherited))
  {
    return false;
  }

  KConfigGroup group = cfg->group(groupName);

  mMinVelo = BosonConfig::readBoVector3FloatEntry(&group, "MinVelocity", mMinVelo);
  mMaxVelo = BosonConfig::readBoVector3FloatEntry(&group, "MaxVelocity", mMaxVelo);
  mMinPos = BosonConfig::readBoVector3FloatEntry(&group, "MinPos", mMinPos);
  mMaxPos = BosonConfig::readBoVector3FloatEntry(&group, "MaxPos", mMaxPos);
  mNormalizePos = group.readEntry("NormalizePos", mNormalizePos);
  if(mNormalizePos)
  {
    mMinPosScale = group.readEntry("MinPosScale", mMinPosScale);
    mMaxPosScale = group.readEntry("MaxPosScale", mMaxPosScale);
  }
  mNormalizeVelo = group.readEntry("NormalizeVelo", mNormalizeVelo);
  if(mNormalizeVelo)
  {
    mMinVeloScale = group.readEntry("MinVeloScale", mMinVeloScale);
    mMaxVeloScale = group.readEntry("MaxVeloScale", mMaxVeloScale);
  }
  mStartColor = BosonConfig::readBoVector4FloatEntry(&group, "StartColor", mStartColor);
  mEndColor = BosonConfig::readBoVector4FloatEntry(&group, "EndColor", mEndColor);
  mMinLife = group.readEntry("MinLife", mMinLife);
  mMaxLife = group.readEntry("MaxLife", mMaxLife);
  mMaxNum = group.readEntry("MaxNum", mMaxNum);
  mInitNum = group.readEntry("InitNum", mInitNum);
  mGLBlendFuncStr = group.readEntry("BlendFunc", mGLBlendFuncStr);
  mGLSrcBlendFuncStr = group.readEntry("SrcBlendFunc", mGLSrcBlendFuncStr);
  mRate = group.readEntry("Rate", mRate);
  mStartSize = group.readEntry("StartSize", mStartSize);
  mEndSize = group.readEntry("EndSize", mEndSize);
  mAge = group.readEntry("SystemLife", mAge);
  mMass = group.readEntry("Mass", mMass);
  mParticleDist = group.readEntry("ParticleDist", mParticleDist);
  mAlign = group.readEntry("Align", mAlign);
  mMoveParticlesWithSystem = group.readEntry("MoveParticlesWithSystem", mMoveParticlesWithSystem);
  mTextureName = group.readEntry("Texture", mTextureName);

  // If we're loading main properties (not inherited ones), load the textures
  //  now.
  if(!inherited)
  {
    mTextures = getTextures(mTextureName);

    mGLBlendFunc = Bo3dTools::string2GLBlendFunc(mGLBlendFuncStr);
    if(mGLBlendFunc == GL_INVALID_ENUM)
    {
      boError() << k_funcinfo << "Invalid OpenGL blend function (dst): '" << mGLBlendFuncStr << "'" << endl;
      mGLBlendFunc = GL_ONE_MINUS_SRC_ALPHA;
    }
    mGLSrcBlendFunc = Bo3dTools::string2GLBlendFunc(mGLSrcBlendFuncStr);
    if(mGLSrcBlendFunc == GL_INVALID_ENUM)
    {
      boError() << k_funcinfo << "Invalid OpenGL blend function (src): '" << mGLSrcBlendFuncStr << "'" << endl;
      mGLSrcBlendFunc = GL_SRC_ALPHA;
    }
  }
  return true;
}

BosonEffect* BosonEffectPropertiesParticleGeneric::newEffect(const BoVector3Fixed& pos, const BoVector3Fixed& rot) const
{
  BosonEffectParticleGeneric* e = new BosonEffectParticleGeneric(this, mMaxNum, mTextures);
  BoVector3Fixed worldpos = pos;
  worldpos.canvasToWorld();
  e->setPosition(worldpos);
  e->setAge(mAge);
  e->setMass(mMass);
  e->setParticleDist(mParticleDist);
  e->setBlendFunc(mGLSrcBlendFunc, mGLBlendFunc);
  e->setRate(mRate);
  e->setAlignParticles(mAlign);
  e->setMaxParticleSize(qMax(mStartSize, mEndSize));
  if(!rot.isNull())
  {
    e->setRotation(rot);
  }
  e->setMoveParticlesWithSystem(mMoveParticlesWithSystem);
  e->setMaxDelayedUpdates((int)ceilf(mMaxLife * 20));

  return e;
}

void BosonEffectPropertiesParticleGeneric::initParticle(BosonEffectParticle* effect, BosonParticle* p) const
{
  BosonGenericParticle* particle = (BosonGenericParticle*)p;
  particle->life = BosonEffect::getRandomFloat(mMinLife, mMaxLife);
  particle->maxage = particle->life;
  particle->color = mStartColor;
  particle->size = mStartSize;
  BoVector3Float pos;  // particle's relative position to particle system
  pos.set(BosonEffect::getRandomFloat(mMinPos[0], mMaxPos[0]), BosonEffect::getRandomFloat(mMinPos[1], mMaxPos[1]),
      BosonEffect::getRandomFloat(mMinPos[2], mMaxPos[2]));
  if(mNormalizePos)
  {
    //boDebug(150) << k_funcinfo << "Normalizing particle pos (scale: " << mMinPosScale << " - " << mMaxPosScale << ")" << endl;
    //boDebug(150) << k_funcinfo << "Current pos: (" << particle->pos.x() << ", " << particle->pos.y() << ", " << particle->pos.z() << "); length: " << particle->pos.length() << endl;
    //float scale = getRandomFloat(mMinPosScale, mMaxPosScale);
    //boDebug(150) << k_funcinfo << "Scaling by " << scale << " / " << particle->pos.length() << " = " << scale / particle->pos.length() << endl;
    pos.scale(BosonEffect::getRandomFloat(mMinPosScale, mMaxPosScale) / pos.length());
    //boDebug(150) << k_funcinfo << "New pos: (" << particle->pos.x() << ", " << particle->pos.y() << ", " << particle->pos.z() << "); length: " << particle->pos.length() << endl;
  }
  BosonEffectParticleGeneric* e = (BosonEffectParticleGeneric*)effect;
  if(e->isRotated())
  {
    BoVector3Float tmp;
    e->matrix().transform(&tmp, &pos);
    pos = tmp;
    BoVector3Float velo(BosonEffect::getRandomFloat(mMinVelo[0], mMaxVelo[0]),
        BosonEffect::getRandomFloat(mMinVelo[1], mMaxVelo[1]), BosonEffect::getRandomFloat(mMinVelo[2], mMaxVelo[2]));
    e->matrix().transform(&tmp, &velo);
    particle->velo = tmp;
  }
  else
  {
    particle->velo.set(BosonEffect::getRandomFloat(mMinVelo[0], mMaxVelo[0]),
        BosonEffect::getRandomFloat(mMinVelo[1], mMaxVelo[1]), BosonEffect::getRandomFloat(mMinVelo[2], mMaxVelo[2]));
  }
  particle->pos += pos;
  if(mNormalizeVelo)
  {
    particle->velo.scale(BosonEffect::getRandomFloat(mMinVeloScale, mMaxVeloScale) / particle->velo.length());
  }
}

void BosonEffectPropertiesParticleGeneric::updateParticle(BosonEffectParticle* effect, BosonParticle* p, float elapsed) const
{
  BosonGenericParticle* particle = (BosonGenericParticle*)p;
  float factor = particle->life / particle->maxage;  // This is 1 when particle is born and will be 0 by the time when it dies
  particle->color.setBlended(mStartColor, factor, mEndColor, 1.0f - factor);
  particle->size = mStartSize * factor + mEndSize * (1.0f - factor);
  // Note that we use our own texture array here, not the one stored in
  //  BosonParticleSystem (which is only used for drawing). It doesn't matter,
  //  because they are identical (in theory ;-)) anyway.
  int t = (int)((1.0f - factor) * ((int)mTextures->count() + 1)); // +1 for last texture to be shown
  if(t >= (int)mTextures->count())
  {
    t = mTextures->count() - 1;
  }
  particle->tex = mTextures->texture(t);
  BosonEffectParticleGeneric* e = (BosonEffectParticleGeneric*)effect;
  particle->pos += (wind() * (e->mass() * elapsed));
}



/*****  BosonEffectPropertiesParticleTrail  *****/

BosonEffectPropertiesParticleTrail::BosonEffectPropertiesParticleTrail() :
    BosonEffectPropertiesParticle()
{
  reset();
}

void BosonEffectPropertiesParticleTrail::reset()
{
  // Reset all variables to their default values
  mSpacing = 10.0f / 48.0f;  // distance between 2 particles
  mMass = 0.5f;
  mMinOffset.reset();
  mMaxOffset.reset();
  mMinVelo.reset();
  mMaxVelo.reset();
  mStartColor.reset();
  mEndColor.reset();
  mStartSize = 0.5f;
  mEndSize = 1.0f;
  mParticleDist = 0.0f;
  mMinLife = 1.0f;
  mMaxLife = 1.0f;
  mMaxSpeed = 30.0f / 48.0f;
  mTextureName = "smoke";
  mGLBlendFuncStr = "GL_ONE_MINUS_SRC_ALPHA";
  mGLSrcBlendFuncStr = "GL_SRC_ALPHA";
}

bool BosonEffectPropertiesParticleTrail::load(KConfig* cfg, const QString& groupName, bool inherited)
{
  if(!BosonEffectPropertiesParticle::load(cfg, groupName, inherited))
  {
    return false;
  }

  KConfigGroup group = cfg->group(groupName);

  mSpacing = (float)(group.readEntry("Spacing", mSpacing)) / 48.0f;
  mMass = group.readEntry("Mass", mMass);
  mMinOffset = BosonConfig::readBoVector3FloatEntry(&group, "MinOffset", mMinOffset);
  mMaxOffset = BosonConfig::readBoVector3FloatEntry(&group, "MaxOffset", mMaxOffset);
  mMinVelo = BosonConfig::readBoVector3FloatEntry(&group, "MinVelocity", mMinVelo);
  mMaxVelo = BosonConfig::readBoVector3FloatEntry(&group, "MaxVelocity", mMaxVelo);
  mStartColor = BosonConfig::readBoVector4FloatEntry(&group, "StartColor", mStartColor);
  mEndColor = BosonConfig::readBoVector4FloatEntry(&group, "EndColor", mEndColor);
  mStartSize = group.readEntry("StartSize", mStartSize);
  mEndSize = group.readEntry("EndSize", mEndSize);
  mMaxSpeed = group.readEntry("MaxSpeed", mMaxSpeed);
  mMinLife = group.readEntry("MinLife", mMinLife);
  mMaxLife = group.readEntry("MaxLife", mMaxLife);
  mParticleDist = group.readEntry("ParticleDist", mParticleDist);
  mGLBlendFuncStr = group.readEntry("BlendFunc", mGLBlendFuncStr);
  mGLSrcBlendFuncStr = group.readEntry("SrcBlendFunc", mGLSrcBlendFuncStr);
  mTextureName = group.readEntry("Texture", mTextureName);

  // If we're loading main properties (not inherited ones), load the textures
  //  now.
  if(!inherited)
  {
    mTextures = getTextures(mTextureName);
    mGLBlendFunc = Bo3dTools::string2GLBlendFunc(mGLBlendFuncStr);
    if(mGLBlendFunc == GL_INVALID_ENUM)
    {
      boError() << k_funcinfo << "Invalid OpenGL blend function (dst): '" << mGLBlendFuncStr << "'" << endl;
      mGLBlendFunc = GL_ONE_MINUS_SRC_ALPHA;
    }
    mGLSrcBlendFunc = Bo3dTools::string2GLBlendFunc(mGLSrcBlendFuncStr);
    if(mGLSrcBlendFunc == GL_INVALID_ENUM)
    {
      boError() << k_funcinfo << "Invalid OpenGL blend function (src): '" << mGLSrcBlendFuncStr << "'" << endl;
      mGLSrcBlendFunc = GL_SRC_ALPHA;
    }
  }
  return true;
}

BosonEffect* BosonEffectPropertiesParticleTrail::newEffect(const BoVector3Fixed& pos, const BoVector3Fixed& rot) const
{
  // Calculate maximum number of particles that system can have
  int maxparticles = (int)(mMaxSpeed * mMaxLife / mSpacing) + 1;
  BoVector3Fixed worldpos = pos;
  worldpos.canvasToWorld();

  BosonEffectParticleTrail* e = new BosonEffectParticleTrail(this, maxparticles, mTextures, worldpos);
  e->setMass(mMass);
  e->setParticleDist(mParticleDist);
  e->setBlendFunc(mGLSrcBlendFunc, mGLBlendFunc);
  e->setSpacing(mSpacing);
  e->setMaxParticleSize(qMax(mStartSize, mEndSize));
  if(!rot.isNull())
  {
    e->setRotation(rot);
  }
  e->setMaxDelayedUpdates((int)ceilf(mMaxLife * 20));

  return e;
}

void BosonEffectPropertiesParticleTrail::initParticle(BosonEffectParticle* effect, BosonParticle* p) const
{
  BosonGenericParticle* particle = (BosonGenericParticle*)p;
  particle->life = BosonEffect::getRandomFloat(mMinLife, mMaxLife);
  particle->maxage = particle->life;
  particle->color = mStartColor;
  particle->size = mStartSize;
  BoVector3Float offset;  // particle's relative position to particle system
  offset.set(BosonEffect::getRandomFloat(mMinOffset[0], mMaxOffset[0]), BosonEffect::getRandomFloat(mMinOffset[1], mMaxOffset[1]),
      BosonEffect::getRandomFloat(mMinOffset[2], mMaxOffset[2]));
  BosonEffectParticleTrail* e = (BosonEffectParticleTrail*)effect;
  if(e->isRotated())
  {
    BoVector3Float offset2 = offset;
    BoVector3Float velo(BosonEffect::getRandomFloat(mMinVelo[0], mMaxVelo[0]),
        BosonEffect::getRandomFloat(mMinVelo[1], mMaxVelo[1]), BosonEffect::getRandomFloat(mMinVelo[2], mMaxVelo[2]));
    e->matrix().transform(&offset, &offset2);
    e->matrix().transform(&particle->velo, &velo);
  }
  else
  {
    particle->velo.set(BosonEffect::getRandomFloat(mMinVelo[0], mMaxVelo[0]),
        BosonEffect::getRandomFloat(mMinVelo[1], mMaxVelo[1]), BosonEffect::getRandomFloat(mMinVelo[2], mMaxVelo[2]));
  }
  particle->pos += offset;
}

void BosonEffectPropertiesParticleTrail::updateParticle(BosonEffectParticle* effect, BosonParticle* p, float elapsed) const
{
  BosonGenericParticle* particle = (BosonGenericParticle*)p;
  float factor = particle->life / particle->maxage;  // This is 1 when particle is born and will be 0 by the time when it dies
  particle->color.setBlended(mStartColor, factor, mEndColor, 1.0f - factor);
  particle->size = mStartSize * factor + mEndSize * (1.0f - factor);
  // Note that we use our own texture array here, not the one stored in
  //  BosonParticleSystem (which is only used for drawing). It doesn't matter,
  //  because they are identical (in theory ;-)) anyway.
  int t = (int)((1.0f - factor) * ((int)mTextures->count() + 1)); // +1 for last texture to be shown
  if(t >= (int)mTextures->count())
  {
    t = mTextures->count() - 1;
  }
  particle->tex = mTextures->texture(t);
  BosonEffectParticleTrail* e = (BosonEffectParticleTrail*)effect;
  particle->pos += (wind() * (e->mass() * elapsed));
}



/*****  BosonEffectPropertiesParticleEnvironmental  *****/

BosonEffectPropertiesParticleEnvironmental::BosonEffectPropertiesParticleEnvironmental() :
    BosonEffectPropertiesParticle()
{
  reset();
}

void BosonEffectPropertiesParticleEnvironmental::reset()
{
  // Reset all variables to their default values
  mMass = 0.5f;
  mMinVelo.reset();
  mMaxVelo.reset();
  mColor.reset();
  mSize = 0.5f;
  mParticleDist = 0.0f;
  mTextureName = "smoke";
  mGLBlendFuncStr = "GL_ONE_MINUS_SRC_ALPHA";
  mGLSrcBlendFuncStr = "GL_SRC_ALPHA";
  mRange = 5.0f;
  mDensity = 10.0f;
}

bool BosonEffectPropertiesParticleEnvironmental::load(KConfig* cfg, const QString& groupName, bool inherited)
{
  if(!BosonEffectPropertiesParticle::load(cfg, groupName, inherited))
  {
    return false;
  }

  KConfigGroup group = cfg->group(groupName);

  mMass = group.readEntry("Mass", mMass);
  mMinVelo = BosonConfig::readBoVector3FloatEntry(&group, "MinVelocity", mMinVelo);
  mMaxVelo = BosonConfig::readBoVector3FloatEntry(&group, "MaxVelocity", mMaxVelo);
  mColor = BosonConfig::readBoVector4FloatEntry(&group, "Color", mColor);
  mSize = group.readEntry("Size", mSize);
  mParticleDist = group.readEntry("ParticleDist", mParticleDist);
  mGLBlendFuncStr = group.readEntry("BlendFunc", mGLBlendFuncStr);
  mGLSrcBlendFuncStr = group.readEntry("SrcBlendFunc", mGLSrcBlendFuncStr);
  mRange = group.readEntry("Range", mRange);
  mDensity = group.readEntry("Density", mDensity);
  mTextureName = group.readEntry("Texture", mTextureName);

  // If we're loading main properties (not inherited ones), load the textures
  //  now.
  if(!inherited)
  {
    mTextures = getTextures(mTextureName);
    mGLBlendFunc = Bo3dTools::string2GLBlendFunc(mGLBlendFuncStr);
    if(mGLBlendFunc == GL_INVALID_ENUM)
    {
      boError() << k_funcinfo << "Invalid OpenGL blend function (dst): '" << mGLBlendFuncStr << "'" << endl;
      mGLBlendFunc = GL_ONE_MINUS_SRC_ALPHA;
    }
    mGLSrcBlendFunc = Bo3dTools::string2GLBlendFunc(mGLSrcBlendFuncStr);
    if(mGLSrcBlendFunc == GL_INVALID_ENUM)
    {
      boError() << k_funcinfo << "Invalid OpenGL blend function (src): '" << mGLSrcBlendFuncStr << "'" << endl;
      mGLSrcBlendFunc = GL_SRC_ALPHA;
    }
  }
  return true;
}

BosonEffect* BosonEffectPropertiesParticleEnvironmental::newEffect(const BoVector3Fixed& pos, const BoVector3Fixed& /*rot*/) const
{
  BoVector3Fixed worldpos = pos;
  worldpos.canvasToWorld();

  BosonEffectParticleEnvironmental* e = new BosonEffectParticleEnvironmental(this, mDensity, mRange, mTextures, worldpos);
  e->setMass(mMass);
  e->setParticleDist(mParticleDist);
  e->setBlendFunc(mGLSrcBlendFunc, mGLBlendFunc);
  e->setParticleVelo((mMinVelo + mMaxVelo) / 2);

  return e;
}

void BosonEffectPropertiesParticleEnvironmental::initParticle(BosonEffectParticle* effect, BosonParticle* p) const
{
  BosonGenericParticle* particle = (BosonGenericParticle*)p;
  particle->life = 1.0f;  // Not used, this just tells that the particle is active
  particle->maxage = 1.0f;  // Not used
  particle->color = mColor;
  particle->size = mSize;
  particle->velo.set(BosonEffect::getRandomFloat(mMinVelo[0], mMaxVelo[0]),
      BosonEffect::getRandomFloat(mMinVelo[1], mMaxVelo[1]), BosonEffect::getRandomFloat(mMinVelo[2], mMaxVelo[2]));
}

void BosonEffectPropertiesParticleEnvironmental::updateParticle(BosonEffectParticle* effect, BosonParticle* p, float elapsed) const
{
  BosonGenericParticle* particle = (BosonGenericParticle*)p;
  BosonEffectParticleEnvironmental* e = (BosonEffectParticleEnvironmental*)effect;
  particle->pos += (wind() * (e->mass() * elapsed));
}

