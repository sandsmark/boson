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


#include "bosoneffectpropertiesparticle.h"

#include <ksimpleconfig.h>
#include <qstring.h>
#include <qdir.h>
#include <qstringlist.h>

#include "bosontexturearray.h"
#include "bosoneffectparticle.h"
#include "bodebug.h"
#include "bosonconfig.h"


/*****  BosonEffectPropertiesParticle  *****/

#warning this is never freed!
QDict<BosonTextureArray> BosonEffectPropertiesParticle::mTextureArrays;
QString BosonEffectPropertiesParticle::mTexturePath;

BosonEffectPropertiesParticle::BosonEffectPropertiesParticle() :
    BosonEffectProperties()
{
}

bool BosonEffectPropertiesParticle::load(KSimpleConfig* cfg, const QString& group, bool inherited)
{
  if(!BosonEffectProperties::load(cfg, group, inherited))
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

const BosonTextureArray* BosonEffectPropertiesParticle::getTextures(const QString& name)
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
    BosonTextureArray* t = new BosonTextureArray(absFiles, boConfig->modelTexturesMipmaps(), true);

    mTextureArrays.insert(name, t);
    mTextureArrays.setAutoDelete(true);
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
  mMinPosScale = 1.0;
  mMaxPosScale = 1.0;
  mNormalizeVelo = false;
  mMinVeloScale = 1.0;
  mMaxVeloScale = 1.0;
  mStartColor.reset();
  mEndColor.reset();
  mMinLife = 1.0;
  mMaxLife = 1.0;
  mMaxNum = 100;
  mInitNum = 0;
  mGLBlendFuncStr = "GL_ONE_MINUS_SRC_ALPHA";
  mGLSrcBlendFuncStr = "GL_SRC_ALPHA";
  mRate = 0;
  mStartSize = 1;
  mEndSize = 1;
  mAge = 0;
  mMass = 1.0;
  mParticleDist = 0.0;
  mAlign = true;
  mMoveParticlesWithSystem = false;
  mTextureName = "explosion";
}

bool BosonEffectPropertiesParticleGeneric::load(KSimpleConfig* cfg, const QString& group, bool inherited)
{
  if(!BosonEffectPropertiesParticle::load(cfg, group, inherited))
  {
    return false;
  }

  mMinVelo = BosonConfig::readBoVector3Entry(cfg, "MinVelocity", mMinVelo);
  mMaxVelo = BosonConfig::readBoVector3Entry(cfg, "MaxVelocity", mMaxVelo);
  mMinPos = BosonConfig::readBoVector3Entry(cfg, "MinPos", mMinPos);
  mMaxPos = BosonConfig::readBoVector3Entry(cfg, "MaxPos", mMaxPos);
  mNormalizePos = cfg->readBoolEntry("NormalizePos", mNormalizePos);
  if(mNormalizePos)
  {
    mMinPosScale = (float)(cfg->readDoubleNumEntry("MinPosScale", mMinPosScale));
    mMaxPosScale = (float)(cfg->readDoubleNumEntry("MaxPosScale", mMaxPosScale));
  }
  mNormalizeVelo = cfg->readBoolEntry("NormalizeVelo", mNormalizeVelo);
  if(mNormalizeVelo)
  {
    mMinVeloScale = (float)(cfg->readDoubleNumEntry("MinVeloScale", mMinVeloScale));
    mMaxVeloScale = (float)(cfg->readDoubleNumEntry("MaxVeloScale", mMaxVeloScale));
  }
  mStartColor = BosonConfig::readBoVector4Entry(cfg, "StartColor", mStartColor);
  mEndColor = BosonConfig::readBoVector4Entry(cfg, "EndColor", mEndColor);
  mMinLife = (float)(cfg->readDoubleNumEntry("MinLife", mMinLife));
  mMaxLife = (float)(cfg->readDoubleNumEntry("MaxLife", mMaxLife));
  mMaxNum = cfg->readNumEntry("MaxNum", mMaxNum);
  mInitNum = cfg->readNumEntry("InitNum", mInitNum);
  mGLBlendFuncStr = cfg->readEntry("BlendFunc", mGLBlendFuncStr);
  mGLSrcBlendFuncStr = cfg->readEntry("SrcBlendFunc", mGLSrcBlendFuncStr);
  mRate = (float)(cfg->readDoubleNumEntry("Rate", mRate));
  mStartSize = (float)(cfg->readDoubleNumEntry("StartSize", mStartSize));
  mEndSize = (float)(cfg->readDoubleNumEntry("EndSize", mEndSize));
  mAge = (float)(cfg->readDoubleNumEntry("SystemLife", mAge));
  mMass = (float)(cfg->readDoubleNumEntry("Mass", mMass));
  mParticleDist = (float)(cfg->readDoubleNumEntry("ParticleDist", mParticleDist));
  mAlign = cfg->readBoolEntry("Align", mAlign);
  mMoveParticlesWithSystem = cfg->readBoolEntry("MoveParticlesWithSystem", mMoveParticlesWithSystem);
  mTextureName = cfg->readEntry("Texture", mTextureName);

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

BosonEffect* BosonEffectPropertiesParticleGeneric::newEffect(const BoVector3& pos, const BoVector3& rot) const
{
  BosonEffectParticleGeneric* e = new BosonEffectParticleGeneric(this, mMaxNum, mTextures);
  BoVector3 worldpos = pos;
  worldpos.canvasToWorld();
  e->setPosition(worldpos);
  e->setAge(mAge);
  e->setMass(mMass);
  e->setParticleDist(mParticleDist);
  e->setBlendFunc(mGLSrcBlendFunc, mGLBlendFunc);
  e->setRate(mRate);
  e->setAlignParticles(mAlign);
  e->setMaxParticleSize(QMAX(mStartSize, mEndSize));
  if(!rot.isNull())
  {
    e->setRotation(rot);
  }
  e->setMoveParticlesWithSystem(mMoveParticlesWithSystem);

  return e;
}

void BosonEffectPropertiesParticleGeneric::initParticle(BosonEffectParticle* effect, BosonParticle* p) const
{
  BosonGenericParticle* particle = (BosonGenericParticle*)p;
  particle->life = BosonEffect::getFloat(mMinLife, mMaxLife);
  particle->maxage = particle->life;
  particle->color = mStartColor;
  particle->size = mStartSize;
  BoVector3 pos;  // particle's relative position to particle system
  pos.set(BosonEffect::getFloat(mMinPos[0], mMaxPos[0]), BosonEffect::getFloat(mMinPos[1], mMaxPos[1]),
      BosonEffect::getFloat(mMinPos[2], mMaxPos[2]));
  if(mNormalizePos)
  {
    //boDebug(150) << k_funcinfo << "Normalizing particle pos (scale: " << mMinPosScale << " - " << mMaxPosScale << ")" << endl;
    //boDebug(150) << k_funcinfo << "Current pos: (" << particle->pos.x() << ", " << particle->pos.y() << ", " << particle->pos.z() << "); length: " << particle->pos.length() << endl;
    //float scale = getFloat(mMinPosScale, mMaxPosScale);
    //boDebug(150) << k_funcinfo << "Scaling by " << scale << " / " << particle->pos.length() << " = " << scale / particle->pos.length() << endl;
    pos.scale(BosonEffect::getFloat(mMinPosScale, mMaxPosScale) / pos.length());
    //boDebug(150) << k_funcinfo << "New pos: (" << particle->pos.x() << ", " << particle->pos.y() << ", " << particle->pos.z() << "); length: " << particle->pos.length() << endl;
  }
  BosonEffectParticleGeneric* e = (BosonEffectParticleGeneric*)effect;
  if(e->isRotated())
  {
    BoVector3 pos2 = pos;
    BoVector3 velo(BosonEffect::getFloat(mMinVelo[0], mMaxVelo[0]),
        BosonEffect::getFloat(mMinVelo[1], mMaxVelo[1]), BosonEffect::getFloat(mMinVelo[2], mMaxVelo[2]));
    e->matrix().transform(&pos, &pos2);
    e->matrix().transform(&(particle->velo), &velo);
  }
  else
  {
    particle->velo.set(BosonEffect::getFloat(mMinVelo[0], mMaxVelo[0]),
        BosonEffect::getFloat(mMinVelo[1], mMaxVelo[1]), BosonEffect::getFloat(mMinVelo[2], mMaxVelo[2]));
  }
  particle->pos += pos;
  if(mNormalizeVelo)
  {
    particle->velo.scale(BosonEffect::getFloat(mMinVeloScale, mMaxVeloScale) / particle->velo.length());
  }
  particle->velo += (wind() * e->mass());
}

void BosonEffectPropertiesParticleGeneric::updateParticle(BosonEffectParticle* effect, BosonParticle* p) const
{
  BosonGenericParticle* particle = (BosonGenericParticle*)p;
  float factor = particle->life / particle->maxage;  // This is 1 when particle is born and will be 0 by the time when it dies
  particle->color.setBlended(mStartColor, factor, mEndColor, 1.0 - factor);
  particle->size = mStartSize * factor + mEndSize * (1.0 - factor);
  // Note that we use our own texture array here, not the one stored in
  //  BosonParticleSystem (which is only used for drawing). It doesn't matter,
  //  because they are identical (in theory ;-)) anyway.
  int t = (int)((1.0 - factor) * ((int)mTextures->count() + 1)); // +1 for last texture to be shown
  if(t >= (int)mTextures->count())
  {
    t = mTextures->count() - 1;
  }
  particle->tex = mTextures->texture(t);
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
  mSpacing = 10;  // distance between 2 particles
  mMass = 0.5;
  mMinOffset.reset();
  mMaxOffset.reset();
  mMinVelo.reset();
  mMaxVelo.reset();
  mStartColor.reset();
  mEndColor.reset();
  mStartSize = 0.5;
  mEndSize = 1.0;
  mParticleDist = 0;
  mMinLife = 1.0;
  mMaxLife = 1.0;
  mMaxSpeed = 30.0;  // in canvas coords
  mTextureName = "smoke";
  mGLBlendFuncStr = "GL_ONE_MINUS_SRC_ALPHA";
  mGLSrcBlendFuncStr = "GL_SRC_ALPHA";
}

bool BosonEffectPropertiesParticleTrail::load(KSimpleConfig* cfg, const QString& group, bool inherited)
{
  if(!BosonEffectPropertiesParticle::load(cfg, group, inherited))
  {
    return false;
  }

  mSpacing = (float)(cfg->readDoubleNumEntry("Spacing", mSpacing)) / BO_TILE_SIZE;
  mMass = (float)(cfg->readDoubleNumEntry("Mass", mMass));
  mMinOffset = BosonConfig::readBoVector3Entry(cfg, "MinOffset", mMinOffset);
  mMaxOffset = BosonConfig::readBoVector3Entry(cfg, "MaxOffset", mMaxOffset);
  mMinVelo = BosonConfig::readBoVector3Entry(cfg, "MinVelocity", mMinVelo);
  mMaxVelo = BosonConfig::readBoVector3Entry(cfg, "MaxVelocity", mMaxVelo);
  mStartColor = BosonConfig::readBoVector4Entry(cfg, "StartColor", mStartColor);
  mEndColor = BosonConfig::readBoVector4Entry(cfg, "EndColor", mEndColor);
  mStartSize = (float)(cfg->readDoubleNumEntry("StartSize", mStartSize));
  mEndSize = (float)(cfg->readDoubleNumEntry("EndSize", mEndSize));
  // * 20  because in units' config files is speed/tick, but here we want speed/sec
  mMaxSpeed = (float)(cfg->readDoubleNumEntry("MaxSpeed", mMaxSpeed)) / BO_TILE_SIZE * 20;
  mMinLife = (float)(cfg->readDoubleNumEntry("MinLife", mMinLife));
  mMaxLife = (float)(cfg->readDoubleNumEntry("MaxLife", mMaxLife));
  mParticleDist = (float)(cfg->readDoubleNumEntry("ParticleDist", mParticleDist));
  mGLBlendFuncStr = cfg->readEntry("BlendFunc", mGLBlendFuncStr);
  mGLSrcBlendFuncStr = cfg->readEntry("SrcBlendFunc", mGLSrcBlendFuncStr);
  mTextureName = cfg->readEntry("Texture", mTextureName);

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

BosonEffect* BosonEffectPropertiesParticleTrail::newEffect(const BoVector3& pos, const BoVector3& rot) const
{
  // Calculate maximum number of particles that system can have
  int maxparticles = (int)(mMaxSpeed * mMaxLife / mSpacing) + 1;
  BoVector3 worldpos = pos;
  worldpos.canvasToWorld();

  BosonEffectParticleTrail* e = new BosonEffectParticleTrail(this, maxparticles, mTextures, worldpos);
  e->setMass(mMass);
  e->setParticleDist(mParticleDist);
  e->setBlendFunc(mGLSrcBlendFunc, mGLBlendFunc);
  e->setSpacing(mSpacing);
  e->setMaxParticleSize(QMAX(mStartSize, mEndSize));
  if(!rot.isNull())
  {
    e->setRotation(rot);
  }

  return e;
}

void BosonEffectPropertiesParticleTrail::initParticle(BosonEffectParticle* effect, BosonParticle* p) const
{
  BosonTrailParticle* particle = (BosonTrailParticle*)p;
  particle->life = BosonEffect::getFloat(mMinLife, mMaxLife);
  particle->maxage = particle->life;
  particle->color = mStartColor;
  particle->size = mStartSize;
  BoVector3 offset;  // particle's relative position to particle system
  offset.set(BosonEffect::getFloat(mMinOffset[0], mMaxOffset[0]), BosonEffect::getFloat(mMinOffset[1], mMaxOffset[1]),
      BosonEffect::getFloat(mMinOffset[2], mMaxOffset[2]));
  BosonEffectParticleTrail* e = (BosonEffectParticleTrail*)effect;
  if(e->isRotated())
  {
    BoVector3 offset2 = offset;
    BoVector3 velo(BosonEffect::getFloat(mMinVelo[0], mMaxVelo[0]),
        BosonEffect::getFloat(mMinVelo[1], mMaxVelo[1]), BosonEffect::getFloat(mMinVelo[2], mMaxVelo[2]));
    e->matrix().transform(&offset, &offset2);
    e->matrix().transform(&(particle->velo), &velo);
  }
  else
  {
    particle->velo.set(BosonEffect::getFloat(mMinVelo[0], mMaxVelo[0]),
        BosonEffect::getFloat(mMinVelo[1], mMaxVelo[1]), BosonEffect::getFloat(mMinVelo[2], mMaxVelo[2]));
  }
  particle->pos += offset;
  particle->velo += (wind() * e->mass());
}

void BosonEffectPropertiesParticleTrail::updateParticle(BosonEffectParticle* effect, BosonParticle* p) const
{
  BosonTrailParticle* particle = (BosonTrailParticle*)p;
  float factor = particle->life / particle->maxage;  // This is 1 when particle is born and will be 0 by the time when it dies
  particle->color.setBlended(mStartColor, factor, mEndColor, 1.0 - factor);
  particle->size = mStartSize * factor + mEndSize * (1.0 - factor);
  // Note that we use our own texture array here, not the one stored in
  //  BosonParticleSystem (which is only used for drawing). It doesn't matter,
  //  because they are identical (in theory ;-)) anyway.
  int t = (int)((1.0 - factor) * ((int)mTextures->count() + 1)); // +1 for last texture to be shown
  if(t >= (int)mTextures->count())
  {
    t = mTextures->count() - 1;
  }
  particle->tex = mTextures->texture(t);
}
