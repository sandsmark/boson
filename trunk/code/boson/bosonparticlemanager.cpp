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

#include <qstring.h>
#include <qimage.h>

#include <kdebug.h>

#include <GL/gl.h>

#include "bosonparticlesystem.h"
#include "bosontexturearray.h"

class BosonParticleData
{
  public:
    float maxage;
};

BosonTextureArray* BosonParticleManager::mTextures = 0;
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

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  mTextures = new BosonTextureArray(images, false);
  
  mRandom = new KRandomSequence(9376594);
}

BosonParticleSystem* BosonParticleManager::newSystem(BoVector3 pos, Type type)
{
//  kdDebug() << "PARTICLE:" << "        " << k_funcinfo << "called, type: " << type << "pos: (" << x << ", " << y << ", " << z  << ")" << endl;
  int maxnum = 0, initnum = 0, blendfunc = GL_ONE_MINUS_SRC_ALPHA;
  float rate = 0, size = 0, age = 0;
  BosonParticleSystem::ExternalFunction initfunc = 0, updatefunc = &updateFadeOutParticle;

  if(type == Explosion)
  {
    maxnum = 200;
    initnum = 200;
    size = 1.0;
    initfunc = &initExplosionParticle;
    blendfunc = GL_ONE;
  }
  else if(type == Smoke)
  {
    maxnum = 200;
    initnum = 0;
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
    maxnum = 200;
    initnum = 0;
    rate = 30;
    size = 1.0;
    age = 3600 * 24 * 30;  // 30 days ;-)
    initfunc = &initFireParticle;
    updatefunc = &updateFireParticle;
    blendfunc = GL_ONE;
  }
  else if(type == SmallSmoke)
  {
    maxnum = 75;
    initnum = 0;
    rate = 25;
    size = 0.7;
    age = 3600 * 24 * 30;
    initfunc = &initSmallSmokeParticle;
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
//  kdDebug() << "PARTICLE:" << "        " << k_funcinfo << "initing particle" << endl;
  particle->life = getFloat(0.5, 0.7);  // Particle's lifetime is between 0.5 and 0.7 seconds
  particle->maxage = particle->life;
  particle->velo = BoVector3(getFloat(-1.6, 1.6), getFloat(-1.6, 1.6), getFloat(0, 1.4));  // Random velocity (per second)
  particle->color = BoVector4(1.0, 0.07, 0.02, 0.25);  // Color of particle
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
  particle->color = BoVector4(0.7, 0.7, 0.7, 0.7);
  particle->pos = BoVector3(getFloat(-0.1, 0.1), getFloat(-0.1, 0.1), getFloat(0.5, 0.6));
}

void BosonParticleManager::initShotParticle(BosonParticleSystem*, BosonParticle* particle)
{
  particle->life = getFloat(0.2, 0.4);
  particle->maxage = particle->life;
  particle->velo = BoVector3(getFloat(-1.0, 1.0), getFloat(-1.0, 1.0), getFloat(0, 1.0));
  particle->color = BoVector4(1.0, 0.07, 0.02, 0.25);
  particle->pos = BoVector3(getFloat(-0.1, 0.1), getFloat(-0.1, 0.1), getFloat(0.5, 0.6));
}

void BosonParticleManager::initFireParticle(BosonParticleSystem*, BosonParticle* particle)
{
  particle->life = getFloat(1.5, 2.0);
  particle->maxage = particle->life;
  particle->velo = BoVector3(0.0, 0.0, getFloat(0.8, 1.2));
  particle->color = BoVector4(1.0, 0.3, 0.15, 0.3);
  particle->pos = BoVector3(getFloat(-0.05, 0.05), getFloat(-0.05, 0.05), getFloat(0.5, 0.6));
}

void BosonParticleManager::updateFireParticle(BosonParticleSystem*, BosonParticle* particle)
{
  particle->color.setY(0.3 * (particle->life / particle->maxage));  // Green - makes fire more yellow
  particle->color.setZ(0.15 * (particle->life / particle->maxage));  // Blue - makes fire more white
  particle->color.setW(0.3 * (particle->life / particle->maxage));  // Alpha
}

void BosonParticleManager::updateFadeOutParticle(BosonParticleSystem*, BosonParticle* particle)
{
  // Alpha will start decreasing when half of particle's life is over and will be 0 when it dies
  if((particle->life * 2) < particle->maxage)
  {
    particle->color.setW(0.25 * (particle->life * 2 / particle->maxage));
  }
}
