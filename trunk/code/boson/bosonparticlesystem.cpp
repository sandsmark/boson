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
#include "bosonparticlesystemproperties.h"
#include "bodebug.h"

#include <GL/gl.h>


/*****  BoParticleManager  *****/

BoParticleManager::BoParticleManager()
{
  mParticlesDirty = true;
  mViewportDirty = true;
}

void BoParticleManager::draw(QPtrList<BosonParticleSystem>* systems, const BoVector3& camera)
{
  if(systems->count() == 0)
  {
    return;
  }
  QPtrListIterator<BosonParticleSystem> it(*systems);
  BosonParticleSystem* s;
  float x, y, z;
  BosonParticle* p;
  if(mParticlesDirty)
  {
    mCameraPos = camera;
    clear();
    // Add all particles to the list
    while((s = it.current()) != 0)
    {
      ++it;
      for(int i = 0; i < s->mMaxNum; i++)
      {
        if(s->mParticles[i].life > 0.0)
        {
          p = &(s->mParticles[i]);
          // Calculate distance from camera. Note that for performance reasons,
          //  we don't calculate actual distance, but square of it.
          x = p->pos.x() - mCameraPos.x();
          y = p->pos.y() - mCameraPos.y();
          z = p->pos.z() - mCameraPos.z();
          p->distance = (x*x + y*y + z*z);
          // Append to list
          append(p);
        }
      }
    }

    if(count() == 0)
    {
      return;
    }

    // Sort the list
    sort();
    mParticlesDirty = false;

    // Call pre-draw methods
    it.toFirst();
    while((s = it.current()) != 0)
    {
      ++it;
      s->preDraw();
    }
  }
  else
  {
    if(!mCameraPos.isEqual(camera))
    {
      // Shouldn't happen
      boWarning() << k_funcinfo << "mParticlesDirty is false, but camera has moved" << endl;
      mCameraPos = camera;
    }
  }

  /// Draw particles
  QPtrListIterator<BosonParticle> i(*this);
  //boDebug() << k_funcinfo << "Drawing " << i.count() << " particles" << endl;
  while((p = i.current()) != 0)
  {
    // To make things faster (again), we call BosonParticleSystem::draw() only
    //  when particle system of current particle changes.
    //boDebug() << k_funcinfo << "Drawing loop; current particle's distance: " << p->distance << endl;
    p->system->draw(i);
  }
  //boDebug() << k_funcinfo << "Drawing completed" << endl;
  glColor4f(1.0, 1.0, 1.0, 1.0); // Reset color
}

int BoParticleManager::compareItems(QPtrCollection::Item item1, QPtrCollection::Item item2)
//int BoParticleManager::compareItems(BosonParticle* item1, BosonParticle* item2)
{
  float d1, d2;
  d1 = ((BosonParticle*)item1)->distance;
  d2 = ((BosonParticle*)item2)->distance;
  //boDebug() << k_funcinfo << "d1: " << d1 << "; d2: " << d2 << endl;
  if(d1 > d2)
  {
    return -1;
  }
  else if(d1 == d2)
  {
    return 0;
  }
  else
  {
    return 1;
  }
}



/*****  BosonParticle  *****/

BosonParticle::BosonParticle()
{
  // Only reset life
  life = -1.0;
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
  tex = 0;
  system = 0l;
  distance = 0.0;
}

void BosonParticle::update(float elapsed)
{
  life -= elapsed;
  pos.addScaled(velo, elapsed);
}

/*****  BosonParticleSystem  *****/

BosonParticleSystem::BosonParticleSystem(int maxnum, int initialnum, float size,
    float createrate, bool align, float maxradius, BosonParticleTextureArray textures,
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
  mTextures = textures;
  mColor = color;
  mPos = pos;
  mParticleAge = particleage;
  mAge = age;
  mVelo = velo;
  mProp = prop;

  init(initialnum);
}

BosonParticleSystem::BosonParticleSystem(int maxnum,
    float createrate, bool align, float maxradius, BosonParticleTextureArray textures,
    const BosonParticleSystemProperties* prop)
{
  //cout << k_funcinfo << "CREATING PARTICLE SYSTEM.  maxnum: " <<  maxnum <<
      //"; createrate: " << createrate << endl;
  // Set some variables first
  mMaxNum = maxnum;
  mCreateRate = createrate;
  mAlign = align;
  mRadius = maxradius;
  mTextures = textures;
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

//#define NEW_TRANFORM_CODE

void BosonParticleSystem::preDraw()
{
  // TODO: perf: this method should have an argument 'bool camerachanged' and we
  //  should recalculate this stuff only if it's true.
  // Align if needed
#ifndef NEW_TRANFORM_CODE
  float size = mSize / 2.0;
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
#endif
}

void BosonParticleSystem::draw(QPtrListIterator<BosonParticle>& it)
{
//  boDebug() << "PARTICLE:" << "        " << k_funcinfo << "drawing " << mNum << " particles" << endl;
  // Complex method. Parts of this method are taken from Plib project (plib.sf.net)
  // Return if there are no living particles
  if(mNum <= 0)
  {
    return;
  }

#ifdef NEW_TRANSFORM_CODE
  BoMatrix matrix(GL_MODELVIEW_MATRIX);  // FIXME: this must be cached too!
#endif

  // Matrix transformations first
  // TODO: we must in the future use matrix inside BosonParticleSystem for
  //  particle coordinate calculations. This would enable us to reenable wind
  //  and maybe we'd also get rid of these things here
  glPushMatrix();
  glTranslatef(mPos[0], mPos[1], mPos[2]);
  glRotatef(mRot[0], 1.0, 0.0, 0.0);
  glRotatef(mRot[1], 0.0, 1.0, 0.0);
  glRotatef(mRot[2], 0.0, 0.0, 1.0);

  // Update particles
//  int num = 0;
//  boDebug() << "PARTICLE:" << "        " << k_funcinfo << "translating by (" << mPos[0] << ", " << mPos[1] << ", " << mPos[2] << ")" << endl;
  // TODO: perf: how much would it make things faster if we'd cache current OGL
  //  blend function somewhere and only change it if necessary?
  glBlendFunc(mBlendFunc[0], mBlendFunc[1]);
  
  // FIXME: between glBegin() and glEnd() there should be as little code as
  // possible, i.e. try to get around the loop and so.
  // FIXME: maybe we can use vertex arrays here?
  GLuint lasttex = 0;
  glBegin(GL_QUADS);
  BosonParticle* p;
  while((p = it.current()) != 0)
  {
    // Don't draw other systems' particles
    if(p->system != this)
    {
      break;
    }
    // Don't draw dead particles
    if(p->life <= 0.0)
    {
      continue;
    }

    BoVector3 a, b, c, d;  // Vertex positions

#ifdef NEW_TRANFORM_CODE
    BoVector3 center;
    matrix.transform(&center, &(p->pos));

    a = center + BoVector3(-(p->size), p->size, 0);
    b = center + BoVector3(p->size, p->size, 0);
    c = center + BoVector3(p->size, -(p->size), 0);
    d = center + BoVector3(-(p->size), -(p->size), 0);
#else
    a.setScaledSum(p->pos, nw, p->size);
    b.setScaledSum(p->pos, ne, p->size);
    c.setScaledSum(p->pos, se, p->size);
    d.setScaledSum(p->pos, sw, p->size);
#endif

    // TODO: perf: we would also store last used texture in BoParticleManager
    //  and only change it if necessary (also see above about blend func)
    if(lasttex != p->tex)
    {
      // This is ugly but it makes things faster
      //boDebug() << k_funcinfo << "Different texture: last: " << lasttex << "; new: " << p->tex << endl;
      lasttex = p->tex;
      glEnd();
      glBindTexture(GL_TEXTURE_2D, p->tex);
      glBegin(GL_QUADS);
    }
    glColor4fv(p->color.data());
    glTexCoord2f(0.0, 1.0);  glVertex3fv(a.data());
    glTexCoord2f(1.0, 1.0);  glVertex3fv(b.data());
    glTexCoord2f(1.0, 0.0);  glVertex3fv(c.data());
    glTexCoord2f(0.0, 0.0);  glVertex3fv(d.data());
//    num++;
    ++it;
  }
  glEnd();

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
  particle->tex = mTextures.mTextureIds[0];
  particle->system = this;
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
