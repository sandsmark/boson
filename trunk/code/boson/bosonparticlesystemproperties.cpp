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
KRandomSequence* BosonParticleSystemProperties::mRandom = 0;
QMap<QString, BosonParticleTextureArray> BosonParticleSystemProperties::mTextureArrays;
QString BosonParticleSystemProperties::mTexturePath;

BosonParticleTextureArray BosonParticleSystemProperties::getTextures(const QString& name)
{
  if(!mTextureArrays.contains(name))
  {
    boDebug() << k_funcinfo << "Adding texture with name " << name << " to textures arrays map" << endl;
    BosonParticleTextureArray t;
    QDir d(mTexturePath);
    QStringList files = d.entryList(name + "*.png", QDir::Files, QDir::Name);
    t.mTextureCount = files.count();
    boDebug() << k_funcinfo << "Found " << files.count() << " suitable files" << endl;
    t.mTextureIds = new GLuint[t.mTextureCount];
    glGenTextures(t.mTextureCount, t.mTextureIds);
    
    int i = 0;
    for(QStringList::Iterator it = files.begin(); it != files.end(); it++)
    {
      BosonTextureArray::createTexture(mTexturePath + "/" + *it, t.mTextureIds[i], boConfig->modelTexturesMipmaps());
      boDebug() << k_funcinfo << "Created new texture from file " << *it  << "; id: " << i << "; OGL name: " << t.mTextureIds[i] << endl;
      i++;
    }
/*    GLuint tex;
    glGenTextures(1, &tex);
    QImage image(mTexturePath + "/" + name);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    BosonTextureArray::createTexture(image, tex, boConfig->modelTexturesMipmaps());*/
    mTextureArrays.insert(name, t);
  }
  return mTextureArrays[name];
}

void BosonParticleSystemProperties::init(const QString& texdir)
{
  mTexturePath = texdir;
  mRandom = new KRandomSequence(123456789); // AB: this isn't random, but that won't matter for particles.
}
/// End of static initialization stuff (below this is real code ;-))


BosonParticleSystemProperties::BosonParticleSystemProperties(KSimpleConfig* cfg)
{
  // Load all values
  mId = cfg->readUnsignedLongNumEntry("Id", 0);
  if(mId == 0)
  {
    boError() << k_funcinfo << "Invalid id in group " << cfg->group() << endl;
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
  if(mInitNum == 0)
  {
    mInitNum = 10;  // Needed because of stupid bug
  }
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
    boError() << k_funcinfo << "Invalid BlendFunc entry in config file: " << mGLBlendFuncStr << endl;
    mGLBlendFunc = GL_ONE_MINUS_SRC_ALPHA;
  }
  mRate = (float)(cfg->readDoubleNumEntry("Rate", 0));
  mStartSize = (float)(cfg->readDoubleNumEntry("StartSize", 1));
  mEndSize = (float)(cfg->readDoubleNumEntry("EndSize", 1));
  mAge = (float)(cfg->readDoubleNumEntry("SystemLife", 0));
  mAlign = cfg->readBoolEntry("Align", true);
  QString textureName = cfg->readEntry("Texture", "explosion");
  mTextures = getTextures(textureName);
  boDebug() << k_funcinfo << "Number of textures found: " << mTextures.mTextureCount << endl;
  for(int i = 0; i < mTextures.mTextureCount; i++)
  {
    boDebug() << "    " << k_funcinfo << "OGL name of texture[" << i << "]: " << mTextures.mTextureIds[i] << endl;
  }
}

BosonParticleSystemProperties::~BosonParticleSystemProperties()
{
}

BosonParticleSystem* BosonParticleSystemProperties::newSystem(BoVector3 pos) const
{
  BosonParticleSystem* s = new BosonParticleSystem(mMaxNum, mRate, mAlign,
      5, mTextures, this);
  s->setPosition(BoVector3(pos[0] / BO_TILE_SIZE, -(pos[1] / BO_TILE_SIZE), pos[2] / BO_TILE_SIZE));
  s->setSize((mStartSize + mEndSize) / 2);  // Average of start and end sizes
  s->setAge(mAge);
  s->setBlendFunc(GL_SRC_ALPHA, mGLBlendFunc);
  s->createParticles(mInitNum);

  return s;
}

void BosonParticleSystemProperties::initParticle(BosonParticleSystem*, BosonParticle* particle) const
{
  particle->life = getFloat(mMinLife, mMaxLife);
  particle->maxage = particle->life;
  particle->velo = BoVector3(getFloat(mMinVelo[0], mMaxVelo[0]),
      getFloat(mMinVelo[1], mMaxVelo[1]), getFloat(mMinVelo[2], mMaxVelo[2]));
  if(mNormalize)
  {
    particle->velo.scale(getFloat(mMinScale, mMaxScale) / particle->velo.length());
  }
  particle->color = mStartColor;
  // Note that particle's position is relative to position of particle system
  particle->pos = BoVector3(getFloat(mMinPos[0], mMaxPos[0]),
      getFloat(mMinPos[1], mMaxPos[1]), getFloat(mMinPos[2], mMaxPos[2]));
}

void BosonParticleSystemProperties::updateParticle(BosonParticleSystem*, BosonParticle* particle) const
{
  float factor = particle->life / particle->maxage;  // This is 1 when particle is born and will be 0 by the time when it dies
  particle->color.setBlended(mStartColor, factor, mEndColor, 1.0 - factor);
  particle->size = mStartSize * factor + mEndSize * (1.0 - factor);
  // Note that we use our own texture array here, not the one stored in
  //  BosonParticleSystem (which is only used for drawing). It doesn't matter,
  //  because they are identical (in theory ;-)) anyway.
  int t = (int)((1.0 - factor) * (mTextures.mTextureCount + 1));
  if(t >= mTextures.mTextureCount)
  {
    t = mTextures.mTextureCount;
  }
  particle->tex = mTextures.mTextureIds[t]; // +1 for last texture to be shown
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
