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
#include "bosonparticlemanager.h"
#include "bodebug.h"
#include "boson.h"
#include "player.h"

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
    const BosonParticleSystemProperties* prop)
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
  mParticleAge = particleage;
  mAge = age;
  mVelo = velo;
  mProp = prop;

  init(initialnum);
}

BosonParticleSystem::BosonParticleSystem(int maxnum,
    float createrate, bool align, float maxradius, int texture,
    const BosonParticleSystemProperties* prop)
{
  //cout << k_funcinfo << "CREATING PARTICLE SYSTEM.  maxnum: " <<  maxnum <<
      //"; createrate: " << createrate << endl;
  // Set some variables first
  mMaxNum = maxnum;
  mCreateRate = createrate;
  mAlign = align;
  mRadius = maxradius;
  mTexture = texture;
  mSize = 0;
  mParticleAge = 0;
  mAge = 3600;
  mProp = prop;

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
//  boDebug() << k_funcinfo << "Created " << mNum << " initial particles" << endl;
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
/*  boDebug() << k_funcinfo << " UPDATING; elapsed: " << elapsed << "; createRate: " << mCreateRate <<
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
        //uninitParticle(&mParticles[i]);
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
//    boDebug() << k_funcinfo << "createCache >= 1.0 (" << mCreateCache << "); trying to create new particles" << endl;
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
//    boDebug() << k_funcinfo << "Created " << created << " new particles; createCache is now " << mCreateCache << endl;
  }
}

void BosonParticleSystem::draw()
{
//  boDebug() << "PARTICLE:" << "        " << k_funcinfo << "drawing " << mNum << " particles" << endl;
  // Complex method. Parts of this method are taken from Plib project (plib.sf.net)
  // Return if there are no living particles
  if(mNum <= 0)
  {
    return;
  }

  // Some variables first
  BoVector3 nw, ne, sw, se;  // Coordinates of particle vertexes
  float size = mSize / 2.0;
  
  // Matrix transformations first
  glPushMatrix();
  glTranslatef(mPos[0], mPos[1], mPos[2]);
  glRotatef(mRot[0], 1.0, 0.0, 0.0);
  glRotatef(mRot[1], 0.0, 1.0, 0.0);
  glRotatef(mRot[2], 0.0, 0.0, 1.0);

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
//  boDebug() << "PARTICLE:" << "        " << k_funcinfo << "translating by (" << mPos[0] << ", " << mPos[1] << ", " << mPos[2] << ")" << endl;
  glBindTexture(GL_TEXTURE_2D, mTexture);
  glBlendFunc(mBlendFunc[0], mBlendFunc[1]);
  
  Player* p = boGame->localPlayer();

  // FIXME: between glBegin() and glEnd() there should be as little code as
  // possible, i.e. try to get around the loop and so.
  // FIXME: can't we use a display list here?
  // FIXME: maybe we can use vertex arrays here?
  glBegin(GL_QUADS);
  for(int i = 0; i < mMaxNum; i++)
  {
    // Don't draw dead particles
    if(mParticles[i].life <= 0.0)
    {
      continue;
    }
    // FIXME: this is extremely bad here but we must not draw particles on fogged areas
    if(p->isFogged(mParticles[i].pos[0] + mPos[0], -(mParticles[i].pos[1] + mPos[1])))
    {
      continue;
    }

    BoVector3 a, b, c, d;  // Vertex positions

    a.setScaledSum(mParticles[i].pos, nw, mParticles[i].size);
    b.setScaledSum(mParticles[i].pos, ne, mParticles[i].size);
    c.setScaledSum(mParticles[i].pos, se, mParticles[i].size);
    d.setScaledSum(mParticles[i].pos, sw, mParticles[i].size);

    glColor4fv(mParticles[i].color.data());
    glTexCoord2f(0.0, 1.0);  glVertex3fv(a.data());
    glTexCoord2f(1.0, 1.0);  glVertex3fv(b.data());
    glTexCoord2f(1.0, 0.0);  glVertex3fv(c.data());
    glTexCoord2f(0.0, 0.0);  glVertex3fv(d.data());
//    num++;
  }
  glEnd();

  glColor4f(1.0, 1.0, 1.0, 1.0); // Reset color
  glPopMatrix();
//  boDebug() << "PARTICLE:" << "        " << k_funcinfo << "drawn " << num << " particles" << endl;
}

void BosonParticleSystem::initParticle(BosonParticle* particle)
{
  particle->color = mColor;
  particle->life = mParticleAge;
  particle->pos.reset();
  particle->size = mSize;
  particle->velo = mVelo;
  if(mProp)
  {
    mProp->initParticle(this, particle);
  }
}

void BosonParticleSystem::updateParticle(BosonParticle* particle)
{
  if(mProp)
  {
    mProp->updateParticle(this, particle);
  }
}

void BosonParticleSystem::moveParticles(BoVector3 v)
{
  // Move particles by inverse of v (to have effect of them staying in same place)
  BoVector3 inv(-v[0], -v[1], -v[2]);
  for(int i = 0; i < mMaxNum; i++)
  {
    if(mParticles[i].life > 0.0)
    {
      mParticles[i].pos.add(inv);
    }
  }
}

/*
 * vim: et sw=2
 */
