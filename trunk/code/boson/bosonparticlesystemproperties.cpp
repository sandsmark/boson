/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bosonparticlesystemproperties.h"

#include "bosonparticlesystem.h"
#include "bosontexturearray.h"
#include "bosonconfig.h"
#include "defines.h"
#include "speciestheme.h"
#include "bodebug.h"

#include <qstring.h>
#include <qimage.h>
#include <qdir.h>

#include <ksimpleconfig.h>
#include <kconfig.h>

#include <GL/gl.h>

/// Start of static initialization stuff
QDict<BosonParticleTextureArray> BosonParticleSystemProperties::mTextureArrays;
QString BosonParticleSystemProperties::mTexturePath;

const BosonParticleTextureArray* BosonParticleSystemProperties::getTextures(const QString& name)
{
  if(!mTextureArrays.find(name))
  {
    boDebug(150) << k_funcinfo << "Adding texture with name " << name << " to textures arrays map" << endl;
    QDir d(mTexturePath);
    QStringList files = d.entryList(name + "*.png", QDir::Files, QDir::Name);
    boDebug(150) << k_funcinfo << "Found " << files.count() << " suitable files" << endl;
    BosonParticleTextureArray* t = new BosonParticleTextureArray(files.count());
    glGenTextures(t->mTextureCount, t->mTextureIds);
    
    int i = 0;
    for(QStringList::Iterator it = files.begin(); it != files.end(); it++)
    {
      BosonTextureArray::createTexture(mTexturePath + "/" + *it, t->mTextureIds[i], boConfig->modelTexturesMipmaps());
      boDebug(150) << k_funcinfo << "Created new texture from file " << *it  << "; id: " << i << "; OGL name: " << t->mTextureIds[i] << endl;
      i++;
    }
/*    GLuint tex;
    glGenTextures(1, &tex);
    QImage image(mTexturePath + "/" + name);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    BosonTextureArray::createTexture(image, tex, boConfig->modelTexturesMipmaps());*/
    mTextureArrays.insert(name, t);
    mTextureArrays.setAutoDelete(true);
  }
  return mTextureArrays[name];
}

void BosonParticleSystemProperties::initStatic(const QString& texdir)
{
  mTexturePath = texdir;
}
/// End of static initialization stuff (below this is real code ;-))


BosonParticleSystemProperties::BosonParticleSystemProperties(KSimpleConfig* cfg)
{
  // Load all values
  mId = cfg->readUnsignedLongNumEntry("Id", 0);
  if(mId == 0)
  {
    boError(150) << k_funcinfo << "Invalid id in group " << cfg->group() << endl;
  }
  mMinVelo = BoVector3::load(cfg, "MinVelocity");
  mMaxVelo = BoVector3::load(cfg, "MaxVelocity");
  mMinPos = BoVector3::load(cfg, "MinPos");
  mMaxPos = BoVector3::load(cfg, "MaxPos");
  mNormalize = cfg->readBoolEntry("Normalize", false);
  if(mNormalize)
  {
    mMinScale = (float)(cfg->readDoubleNumEntry("MinScale", 1));
    mMaxScale = (float)(cfg->readDoubleNumEntry("MaxScale", 1));
  }
  mStartColor = BoVector4::load(cfg, "StartColor");
  mEndColor = BoVector4::load(cfg, "EndColor");
  mMinLife = (float)(cfg->readDoubleNumEntry("MinLife", 0));
  mMaxLife = (float)(cfg->readDoubleNumEntry("MaxLife", 0));
  mMaxNum = cfg->readNumEntry("MaxNum", 100);
  mInitNum = cfg->readNumEntry("InitNum", 0);
/*  if(mInitNum == 0)
  {
    mInitNum = 10;  // Needed because of stupid bug
  }*/
  QString mGLBlendFuncStr = cfg->readEntry("BlendFunc", "GL_ONE_MINUS_SRC_ALPHA");
  if(mGLBlendFuncStr == "GL_ONE_MINUS_SRC_ALPHA")
  {
    mGLBlendFunc = GL_ONE_MINUS_SRC_ALPHA;
  }
  else if(mGLBlendFuncStr == "GL_ONE")
  {
    mGLBlendFunc = GL_ONE;
  }
  else
  {
    boError(150) << k_funcinfo << "Invalid BlendFunc entry in config file: " << mGLBlendFuncStr << endl;
    mGLBlendFunc = GL_ONE_MINUS_SRC_ALPHA;
  }
  mRate = (float)(cfg->readDoubleNumEntry("Rate", 0));
  mStartSize = (float)(cfg->readDoubleNumEntry("StartSize", 1));
  mEndSize = (float)(cfg->readDoubleNumEntry("EndSize", 1));
  mAge = (float)(cfg->readDoubleNumEntry("SystemLife", 0));
  mAlign = cfg->readBoolEntry("Align", true);
  QString textureName = cfg->readEntry("Texture", "explosion");
  mTextures = getTextures(textureName);
}

BosonParticleSystemProperties::~BosonParticleSystemProperties()
{
}

BosonParticleSystem* BosonParticleSystemProperties::newSystem(BoVector3 pos, float rotation) const
{
  BosonParticleSystem* s = new BosonParticleSystem(mMaxNum, mRate, mAlign,
      mTextures, this);
  s->setPosition(BoVector3(pos[0] / BO_TILE_SIZE, -(pos[1] / BO_TILE_SIZE), pos[2] / BO_TILE_SIZE));
  s->setAge(mAge);
  s->setBlendFunc(GL_SRC_ALPHA, mGLBlendFunc);
  if(rotation != 0.0)
  {
    s->setRotation(rotation, 0, 0, 1);
  }
  s->createParticles(mInitNum);

  return s;
}

void BosonParticleSystemProperties::initParticle(BosonParticleSystem* s, BosonParticle* particle) const
{
  particle->life = getFloat(mMinLife, mMaxLife);
  particle->maxage = particle->life;
  particle->color = mStartColor;
  particle->size = mStartSize;
  if(s->isRotated())
  {
    BoVector3 pos(getFloat(mMinPos[0], mMaxPos[0]),
        getFloat(mMinPos[1], mMaxPos[1]), getFloat(mMinPos[2], mMaxPos[2]));
    BoVector3 velo(getFloat(mMinVelo[0], mMaxVelo[0]),
        getFloat(mMinVelo[1], mMaxVelo[1]), getFloat(mMinVelo[2], mMaxVelo[2]));
    s->matrix().transform(&pos, &pos);
    particle->pos += pos;
    s->matrix().transform(&(particle->velo), &velo);
  }
  else
  {
    particle->pos += BoVector3(getFloat(mMinPos[0], mMaxPos[0]),
        getFloat(mMinPos[1], mMaxPos[1]), getFloat(mMinPos[2], mMaxPos[2]));
    particle->velo = BoVector3(getFloat(mMinVelo[0], mMaxVelo[0]),
        getFloat(mMinVelo[1], mMaxVelo[1]), getFloat(mMinVelo[2], mMaxVelo[2]));
  }
  if(mNormalize)
  {
    particle->velo.scale(getFloat(mMinScale, mMaxScale) / particle->velo.length());
  }
}

void BosonParticleSystemProperties::updateParticle(BosonParticleSystem*, BosonParticle* particle) const
{
  float factor = particle->life / particle->maxage;  // This is 1 when particle is born and will be 0 by the time when it dies
  particle->color.setBlended(mStartColor, factor, mEndColor, 1.0 - factor);
  particle->size = mStartSize * factor + mEndSize * (1.0 - factor);
  // Note that we use our own texture array here, not the one stored in
  //  BosonParticleSystem (which is only used for drawing). It doesn't matter,
  //  because they are identical (in theory ;-)) anyway.
  int t = (int)((1.0 - factor) * (mTextures->mTextureCount + 1)); // +1 for last texture to be shown
  if(t >= (int)mTextures->mTextureCount)
  {
    t = mTextures->mTextureCount - 1;
  }
  particle->tex = mTextures->mTextureIds[t];
}

QPtrList<BosonParticleSystemProperties> BosonParticleSystemProperties::loadParticleSystemProperties(KSimpleConfig* cfg, QString key, SpeciesTheme* theme)
{
  return loadParticleSystemProperties(BosonConfig::readUnsignedLongNumList(cfg, key), theme);
}

QPtrList<BosonParticleSystemProperties> BosonParticleSystemProperties::loadParticleSystemProperties(QValueList<unsigned long int> ids, SpeciesTheme* theme)
{
  QPtrList<BosonParticleSystemProperties> props;
  QValueList<unsigned long int>::Iterator it;
  for(it = ids.begin(); it != ids.end(); it++)
  {
    props.append(theme->particleSystemProperties(*it));
  }
  return props;
}
