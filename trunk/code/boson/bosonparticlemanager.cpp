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

#include "bosonparticlemanager.h"

#include "bosonparticlesystem.h"
#include "bosontexturearray.h"
#include "bosonconfig.h"
#include "defines.h"
#include "speciestheme.h"
#include "bodebug.h"

#include <qstring.h>
#include <qimage.h>

#include <ksimpleconfig.h>
#include <kconfig.h>

#include <GL/gl.h>

/*****  BosonParticleManager  *****/

/*BosonTextureArray* BosonParticleManager::mTextures = 0;
KRandomSequence* BosonParticleManager::mRandom = 0;

void BosonParticleManager::loadTextures(QString texdir)
{
  // load textures
  QValueList<QImage> images;
  images.append(QImage(texdir + "/explosion.png"));
  images.append(QImage(texdir + "/smoke.png"));
  images.append(QImage(texdir + "/explosion.png"));
  images.append(QImage(texdir + "/explosion.png"));
  images.append(QImage(texdir + "/smoke.png"));
  images.append(QImage(texdir + "/explosion.png"));

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  mTextures = new BosonTextureArray(images, false);

  mRandom = new KRandomSequence(9376594);
}

BosonParticleSystem* BosonParticleManager::newSystem(BoVector3 pos, Type type)
{
  //cout << "PARTICLE: " << k_funcinfo << "called, type: " << type << endl;
  int maxnum = 0, initnum = 3, blendfunc = GL_ONE_MINUS_SRC_ALPHA;
  float rate = 0, size = 0, age = 0;
  BosonParticleSystem::OldExternalFunction initfunc = 0, updatefunc = &updateFadeOutParticle;

  if(type == Explosion)
  {
    maxnum = 400;
    initnum = 250;
    size = 1.0;
    age = 0.3;
    rate = 150;
    initfunc = &initExplosionParticle;
    updatefunc = &updateExplosionParticle;
    blendfunc = GL_ONE;
  }
  else if(type == Smoke)
  {
    maxnum = 200;
    rate = 35;
    size = 1.0;
    age = 20;
    initfunc = &initSmokeParticle;
  }
  else if(type == Shot)
  {
    maxnum = 100;
    initnum = 100;
    size = 0.5;
    initfunc = &initShotParticle;
    blendfunc = GL_ONE;
  }
  else if(type == Fire)
  {
    maxnum = 120;
    rate = 30;
    size = 1.0;
    age = 3600 * 24 * 30;  // 30 days ;-)
    initfunc = &initFireParticle;
    updatefunc = &updateFireParticle;
    blendfunc = GL_ONE;
  }
  else if(type == SmallSmoke)
  {
    maxnum = 125;
    rate = 25;
    size = 0.7;
    age = 3600 * 24 * 30;
    initfunc = &initSmallSmokeParticle;
  }
  else if(type == ShockWave)
  {
    maxnum = 500;
    initnum = 500;
    size = 0.8;
    initfunc = &initShockWaveParticle;
    blendfunc = GL_ONE;
  }

  BosonParticleSystem* s = new BosonParticleSystem(maxnum, rate, true,
      2, mTextures->texture((int)type - 1), initfunc, updatefunc);
  s->setPosition(pos);
  s->setSize(size);
  s->setAge(age);
  s->setBlendFunc(GL_SRC_ALPHA, blendfunc);
  s->createParticles(initnum);

  return s;
}

void BosonParticleManager::initExplosionParticle(BosonParticleSystem*, BosonParticle* particle)
{
//  boDebug() << "PARTICLE:" << "        " << k_funcinfo << "initing particle" << endl;
  particle->life = getFloat(0.5, 0.7);  // Particle's lifetime is between 0.5 and 0.7 seconds
  particle->maxage = particle->life;
  particle->velo = BoVector3(getFloat(-1.6, 1.6), getFloat(-1.6, 1.6), getFloat(0, 1.4));  // Random velocity (per second)
  particle->color = BoVector4(1.0, 0.5, 0.0, 0.25);  // Color of particle
  // Note that particle's position is relative to position of particle system
  particle->pos = BoVector3(getFloat(-0.1, 0.1), getFloat(-0.1, 0.1), getFloat(0.2, 0.7));  // We randomize position little bit
}

void BosonParticleManager::initSmokeParticle(BosonParticleSystem*, BosonParticle* particle)
{
  particle->life = getFloat(2.0, 3.0);
  particle->maxage = particle->life;
  particle->velo = BoVector3(getFloat(0.05, 0.55), getFloat(-0.15, 0.45), getFloat(0.4, 1.0));
  particle->color = BoVector4(0.2, 0.2, 0.2, 0.7);
  particle->pos = BoVector3(getFloat(-0.2, 0.2), getFloat(-0.2, 0.2), getFloat(-0.1, 0.25));
}

void BosonParticleManager::initSmallSmokeParticle(BosonParticleSystem*, BosonParticle* particle)
{
  particle->life = getFloat(1.5, 2.3);
  particle->maxage = particle->life;
  particle->velo = BoVector3(getFloat(0.05, 0.55), getFloat(-0.15, 0.45), getFloat(0.4, 1.0));
  //particle->color = BoVector4(0.7, 0.7, 0.7, 0.25);
//  particle->pos = BoVector3(getFloat(-0.1, 0.1), getFloat(-0.1, 0.1), getFloat(0.5, 0.6));
  particle->pos = BoVector3(getFloat(-0.15, 0.15), getFloat(-0.15, 0.15), getFloat(0.5, 0.65));
}

void BosonParticleManager::initShotParticle(BosonParticleSystem*, BosonParticle* particle)
{
  particle->life = getFloat(0.2, 0.4);
  particle->maxage = particle->life;
  particle->velo = BoVector3(getFloat(-1.0, 1.0), getFloat(-1.0, 1.0), getFloat(0, 1.0));
  particle->color = BoVector4(1.0, 0.07, 0.02, 0.25);
  particle->pos = BoVector3(getFloat(-0.1, 0.1), getFloat(-0.1, 0.1), getFloat(0.5, 0.6));
}

void BosonParticleManager::initFireParticle(BosonParticleSystem* s, BosonParticle* particle)
{
  particle->life = getFloat(1.5, 2.0);
  particle->maxage = particle->life;
  particle->velo = BoVector3(0.0, 0.0, s->velocity()[2] + getFloat(0.0, 0.4));
  particle->color = BoVector4(1.0, 0.3, 0.15, 0.3);
  particle->pos = BoVector3(getFloat(-0.05, 0.05), getFloat(-0.05, 0.05), getFloat(0.5, 0.6));
}

void BosonParticleManager::initShockWaveParticle(BosonParticleSystem*, BosonParticle* particle)
{
  particle->life = 0.5;
  particle->maxage = particle->life;
  BoVector3 velo(getFloat(-1, 1), getFloat(-1, 1), getFloat(-0.2, 0.2));  // Extremely big velocity
  velo.normalize();
  velo.scale(5);
  particle->velo = velo;
  particle->color = BoVector4(0.8, 0.8, 0.8, 0.25);
  particle->pos = BoVector3(getFloat(-0.2, 0.2), getFloat(-0.2, 0.2), getFloat(0.0, 0.2));
}

void BosonParticleManager::updateFadeOutParticle(BosonParticleSystem*, BosonParticle* particle)
{
  // Alpha will start decreasing when half of particle's life is over and will be 0 when it dies
  if((particle->life * 2) < particle->maxage)
  {
    particle->color.setW(0.25 * (particle->life * 2 / particle->maxage));
  }
}

void BosonParticleManager::updateFireParticle(BosonParticleSystem*, BosonParticle* particle)
{
  particle->color.setY(0.3 * (particle->life / particle->maxage));  // Green - makes fire more yellow
  particle->color.setZ(0.15 * (particle->life / particle->maxage));  // Blue - makes fire more white
  particle->color.setW(0.3 * (particle->life / particle->maxage));  // Alpha
}

void BosonParticleManager::updateExplosionParticle(BosonParticleSystem*, BosonParticle* particle)
{
  if((particle->life * 4) >= particle->maxage)
  {
    // Particle's age is at most 75% of maximum. Particle will get more red (explosion -> fire)
    particle->color.setY(0.5 * ((particle->life / particle->maxage - 0.25) / 3 * 4));
  }
  else
  {
    // Particle's age is more than 75% of maximum. Particle will get more grey (fire -> smoke)
    float factor = 1.0 - particle->life / particle->maxage * 4;  // at 75%, this is 0 and at the end, it's 1
    particle->color.setX(1.0 - 0.8 * factor);
    particle->color.setY(0.2 * factor);
    particle->color.setZ(0.2 * factor);
    particle->color.setW(0.3 - 0.3 * factor);  // Alpha
  }
}*/


/*****  BosonParticleSystemProperties  *****/

/// Start of static initialization stuff
KRandomSequence* BosonParticleSystemProperties::mRandom = 0;
QMap<QString, GLuint> BosonParticleSystemProperties::mTextures;
QString BosonParticleSystemProperties::mTexturePath;

void BosonParticleSystemProperties::addTexture(QString name)
{
  if(!mTextures.contains(name))
  {
    boDebug() << k_funcinfo << "Adding texture with name " << name << " to textures map" << endl;
    GLuint tex;
    glGenTextures(1, &tex);
    QImage image(mTexturePath + "/" + name);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    BosonTextureArray::createTexture(image, tex, boConfig->modelTexturesMipmaps());
    mTextures.insert(name, tex);
  }
}

GLuint BosonParticleSystemProperties::texture(QString name)
{
  return mTextures[name];
}

void BosonParticleSystemProperties::init(QString texdir)
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
  /** Veeery ugly code
  mMinXVelo = (float)(cfg->readDoubleNumEntry("MinXVelo", 0));
  mMinYVelo = (float)(cfg->readDoubleNumEntry("MinYVelo", 0));
  mMinZVelo = (float)(cfg->readDoubleNumEntry("MinZVelo", 0));
  mMaxXVelo = (float)(cfg->readDoubleNumEntry("MaxXVelo", 0));
  mMaxYVelo = (float)(cfg->readDoubleNumEntry("MaxYVelo", 0));
  mMaxZVelo = (float)(cfg->readDoubleNumEntry("MaxZVelo", 0));
  mStartColor = BoVector4((float)(cfg->readDoubleNumEntry("StartColorR", 0)), (float)(cfg->readDoubleNumEntry("StartColorG", 0)),
      (float)(cfg->readDoubleNumEntry("StartColorB", 0)), (float)(cfg->readDoubleNumEntry("StartColorA", 0)));
  mEndColor = BoVector4((float)(cfg->readDoubleNumEntry("EndColorR", 0)), (float)(cfg->readDoubleNumEntry("EndColorG", 0)),
      (float)(cfg->readDoubleNumEntry("EndColorB", 0)), (float)(cfg->readDoubleNumEntry("EndColorA", 0)));
  */
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
  mSize = (float)(cfg->readDoubleNumEntry("Size", 1));
  mAge = (float)(cfg->readDoubleNumEntry("SystemLife", 0));
  mAlign = cfg->readBoolEntry("Align", true);
  mTextureName = cfg->readEntry("Texture", "default.png");
  addTexture(mTextureName);
}

BosonParticleSystemProperties::~BosonParticleSystemProperties()
{
}

BosonParticleSystem* BosonParticleSystemProperties::newSystem(float x, float y, float z)
{
  BosonParticleSystem* s = new BosonParticleSystem(mMaxNum, mRate, mAlign,
      5, texture(mTextureName), this);
  s->setPosition(BoVector3(x / BO_TILE_SIZE, -(y / BO_TILE_SIZE), z / BO_TILE_SIZE));
  s->setSize(mSize);
  s->setAge(mAge);
  s->setBlendFunc(GL_SRC_ALPHA, mGLBlendFunc);
  s->createParticles(mInitNum);

  return s;
}

void BosonParticleSystemProperties::initParticle(BosonParticleSystem*, BosonParticle* particle)
{
  particle->life = getFloat(mMinLife, mMaxLife);
  particle->maxage = particle->life;
  particle->velo = BoVector3(getFloat(mMinVelo[0], mMaxVelo[0]),
      getFloat(mMinVelo[1], mMaxVelo[1]), getFloat(mMinVelo[2], mMaxVelo[2]));
  particle->velo.add(wind());
  if(mNormalize)
  {
    particle->velo.scale(getFloat(mMinScale, mMaxScale) / particle->velo.length());
  }
  particle->color = mStartColor;
  // Note that particle's position is relative to position of particle system
  particle->pos = BoVector3(getFloat(mMinPos[0], mMaxPos[0]),
      getFloat(mMinPos[1], mMaxPos[1]), getFloat(mMinPos[2], mMaxPos[2]));
}

void BosonParticleSystemProperties::updateParticle(BosonParticleSystem*, BosonParticle* particle)
{
  float factor = particle->life / particle->maxage;  // This is 1 when particle is born and will be 0 by the time when it dies
  particle->color.setBlended(mStartColor, factor, mEndColor, 1.0 - factor);
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
