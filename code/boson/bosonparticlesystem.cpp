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

#include "bosonparticlesystem.h"

#include "bo3dtools.h"

#include <kdebug.h>

#include <GL/gl.h>

/*****  BosonParticle  *****/

BosonParticle::BosonParticle()
{
  reset();
}

BosonParticle::BosonParticle(BoVector4 c, BoVector3 p, BoVector3 v, float s, float l)
{
  color = c;
  pos = p;
  velo = v;
  size = s;
  life = l;
  maxage = l;
}

BosonParticle::~BosonParticle()
{
}

void BosonParticle::reset()
{
  color.reset();
  pos.reset();
  velo.reset();
  size = 0;
  life = -1.0;
  maxage = 0;
}

void BosonParticle::update(float elapsed)
{
  life -= elapsed;
  pos.addScaled(velo, elapsed);
}

/*****  BosonParticleSystem  *****/

BosonParticleSystem::BosonParticleSystem(int maxnum, int initialnum, float size,
    float createrate, bool align, float maxradius, int texture,
    BoVector4 color, float particleage, float age, BoVector3 pos, BoVector3 velo,
    ExternalFunction initFunc, ExternalFunction updateFunc)
{
  // Set some variables first
  mMaxNum = maxnum;
  if(initialnum > maxnum)
  {
    initialnum = maxnum;
  }
  mSize = size;
  mCreateRate = createrate;
  mAlign = align;
  mRadius = maxradius;
  mTexture = texture;
  mColor = color;
  mPos = pos;
  mInitFunc = initFunc;
  mUpdateFunc = updateFunc;
  mParticleAge = particleage;
  mAge = age;
  mVelo = velo;

  init(initialnum);
}

BosonParticleSystem::BosonParticleSystem(int maxnum,
    float createrate, bool align, float maxradius, int texture,
    ExternalFunction initFunc, ExternalFunction updateFunc)
{
  kdDebug() << k_funcinfo << "CREATING PARTICLE SYSTEM.  maxnum: " <<  maxnum <<
      "; createrate: " << createrate << endl;
  // Set some variables first
  mMaxNum = maxnum;
  mCreateRate = createrate;
  mAlign = align;
  mRadius = maxradius;
  mTexture = texture;
  mInitFunc = initFunc;
  mUpdateFunc = updateFunc;
  mSize = 0;
  mParticleAge = 0;
  mAge = 3600;

  init(0);
}

void BosonParticleSystem::init(int initialnum)
{
  // Some variables
  mCreateCache = 0.0;
  mBlendFunc[0] = GL_SRC_ALPHA;
  mBlendFunc[1] = GL_ONE_MINUS_SRC_ALPHA;

  // Create particles
  mParticles = new BosonParticle[mMaxNum];

  // Create initial particles
  mNum = 0;
  createParticles(initialnum);
//  kdDebug() << k_funcinfo << "Created " << mNum << " initial particles" << endl;
}

void BosonParticleSystem::createParticles(int count)
{
  if(count > mMaxNum) 
  {
    count = mMaxNum;
  }
  for(int i = 0; i < count; i++)
  {
    if(mParticles[i].life <= 0.0)
    {
      initParticle(&mParticles[i]);
      mNum++;
    }
  }
}

BosonParticleSystem::~BosonParticleSystem()
{
  delete[] mParticles;
}

void BosonParticleSystem::update(float elapsed)
{
/*  kdDebug() << k_funcinfo << " UPDATING; elapsed: " << elapsed << "; createRate: " << mCreateRate <<
      "; createCache: " << mCreateCache << "; will add " << elapsed * mCreateRate <<
      " to create cache (total will be = " << mCreateCache + (elapsed * mCreateRate) << ")" << endl;*/
  mCreateCache += (elapsed * mCreateRate);

  if((mCreateCache < 1.0) && (mNum <= 0))
  {
    return;
  }

  mNum = 0;
  mAge -= elapsed;
  
  // Update particles
  for(int i = 0; i < mMaxNum; i++)
  {
    if(mParticles[i].life > 0.0)
    {
      mParticles[i].update(elapsed);
      updateParticle(&mParticles[i]);
      // Check for death
      if(mParticles[i].life <= 0.0)
      {
        // For performance reasons we actually don't create/delete particles. We just mark them dead
        deleteParticle(&mParticles[i]);
      }
      else
      {
        mNum++;
      }
    }
  }

  // Create some new ones if needed
  if((mCreateCache >= 1.0) && (mAge >= 0.0))
  {
//    kdDebug() << k_funcinfo << "createCache >= 1.0 (" << mCreateCache << "); trying to create new particles" << endl;
    for(int i = 0; (i < mMaxNum) && (mCreateCache >= 1.0); i++)
    {
      if(mParticles[i].life <= 0.0)
      {
        // Dead particle, re-create it
        initParticle(&mParticles[i]);
        mCreateCache -= 1.0;
        mNum++;
      }
    }
//    kdDebug() << k_funcinfo << "Created " << created << " new particles; createCache is now " << mCreateCache << endl;
  }
}

void BosonParticleSystem::draw()
{
//  kdDebug() << "PARTICLE:" << "        " << k_funcinfo << "drawing " << mNum << " particles" << endl;
  // Complex method. Parts of this method are taken from Plib project (plib.sf.net)
  // Return if there are no living particles
  if(mNum <= 0)
  {
    return;
  }

  // Some variables first
  BoVector3 nw, ne, sw, se;  // Coordinates of particle vertexes
  float size = mSize / 2.0;
  
  // Align if needed
  if(mAlign)
  {
    float mat[4][4];
    glGetFloatv(GL_MODELVIEW_MATRIX, (float*)mat);
    BoVector3 x, y;
    
    x.set(mat[0][0] * size, mat[1][0] * size, mat[2][0] * size);
    y.set(mat[0][1] * size, mat[1][1] * size, mat[2][1] * size);
    
    nw.set(-x[0] - y[0], -x[1] - y[1], -x[2] - y[2]);
    ne.set(x[0] - y[0], x[1] - y[1], x[2] - y[2]);
    se.set(x[0] + y[0], x[1] + y[1], x[2] + y[2]);
    sw.set(-x[0] + y[0], -x[1] + y[1], -x[2] + y[2]);
  }
  else
  {
    nw.set(-size, -size, 0);
    ne.set(size, -size, 0);
    se.set(size, size, 0);
    sw.set(-size, size, 0);
  }

  // Update particles
//  int num = 0;
  glPushMatrix();
//  kdDebug() << "PARTICLE:" << "        " << k_funcinfo << "translating by (" << mPos[0] << ", " << mPos[1] << ", " << mPos[2] << ")" << endl;
  glTranslatef(mPos[0], mPos[1], mPos[2]);
  glBindTexture(GL_TEXTURE_2D, mTexture);
  glBlendFunc(mBlendFunc[0], mBlendFunc[1]);
  glBegin(GL_QUADS);
  for(int i = 0; i < mMaxNum; i++)
  {
    // Don't draw dead particles
    if(mParticles[i].life <= 0.0)
    {
      continue;
    }
    BoVector3 a, b, c, d;  // Vertex positions

    a.setScaledSum(mParticles[i].pos, nw, mParticles[i].size);
    b.setScaledSum(mParticles[i].pos, ne, mParticles[i].size);
    c.setScaledSum(mParticles[i].pos, se, mParticles[i].size);
    d.setScaledSum(mParticles[i].pos, sw, mParticles[i].size);

    glColor4fv(mParticles[i].color.data());
    if(i == 20)
/*    kdDebug() << "PARTICLE:" << "        " << k_funcinfo << "drawing 20. particle; vertex coordinates: "
    << "(" << a[0] << "; " << a[1] << "; " << a[2] << ");  "
    << "(" << b[0] << "; " << b[1] << "; " << b[2] << ");  "
    << "(" << c[0] << "; " << c[1] << "; " << c[2] << ");  "
    << "(" << d[0] << "; " << d[1] << "; " << d[2] << ");  "
    << "  alpha: " << mParticles[i].color[3] << endl;*/
    glTexCoord2f(0.0, 1.0);  glVertex3fv(a.data());
    glTexCoord2f(1.0, 1.0);  glVertex3fv(b.data());
    glTexCoord2f(1.0, 0.0);  glVertex3fv(c.data());
    glTexCoord2f(0.0, 0.0);  glVertex3fv(d.data());
//    num++;
  }
  glColor4f(1.0, 1.0, 1.0, 1.0); // Reset color
  glEnd();
  glPopMatrix();
//  kdDebug() << "PARTICLE:" << "        " << k_funcinfo << "drawn " << num << " particles" << endl;
}

void BosonParticleSystem::initParticle(BosonParticle* particle)
{
  particle->color = mColor;
  particle->life = mParticleAge;
  particle->pos.reset();
  particle->size = mSize;
  particle->velo = mVelo;
  if(mInitFunc)
  {
    (*mInitFunc)(this, particle);
  }
}
