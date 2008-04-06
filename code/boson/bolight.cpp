/*
    This file is part of the Boson game
    Copyright (C) 2003 Rivo Laks (rivolaks@hot.ee)

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

#include "bolight.h"

#include "../bomemory/bodummymemory.h"
#include "bodebug.h"
#include "boshader.h"
#include <bogl.h>

#include <q3valuevector.h>
//Added by qt3to4:
#include <Q3ValueList>


/*****  BoLightManager  *****/

BoLightManager* BoLightManager::mLightManager = 0;

void BoLightManager::initStatic()
{
  if(mLightManager)
  {
    return;
  }
  mLightManager = new BoLightManager();
}

void BoLightManager::deleteStatic()
{
  delete mLightManager;
  mLightManager = 0;
}

BoLightManager* BoLightManager::manager()
{
  if(!mLightManager)
  {
    boError() << k_funcinfo << "initStatic() has not yet been called!" << endl;
    return 0;
  }
  return mLightManager;
}

BoLightManager::BoLightManager()
{
  init();
}

BoLightManager::~BoLightManager()
{
  // Delete all lights
  while(mAllLights->count() > 0)
  {
    deleteLight(mAllLights->first()->id());
  }

  delete mAllLights;
  delete mActiveLights;
  BoShader::setActiveLights(0);
}

void BoLightManager::init()
{
  glGetIntegerv(GL_MAX_LIGHTS, &mMaxActiveLights);

  mActiveLights = new Q3ValueVector<BoLight*>();
  mAllLights = new Q3ValueList<BoLight*>();
  mNextLightId = 0;
  BoShader::setActiveLights(0);
}

BoLight* BoLightManager::light(int id)
{
  for(Q3ValueList<BoLight*>::Iterator it = mAllLights->begin(); it != mAllLights->end(); it++)
  {
    if((*it)->id() == id)
    {
      return *it;
    }
  }
  return 0;
}

BoLight* BoLightManager::activeLight(int openglid)
{
  if(openglid < 0 || openglid >= (int)mActiveLights->count())
  {
    return 0;
  }
  return mActiveLights->at(openglid);
}

BoLight* BoLightManager::createLight()
{
  BoLight* light = new BoLight(mNextLightId++);
  mAllLights->append(light);

  // Make the light active if possible
  if((int)mActiveLights->count() < mMaxActiveLights)
  {
    light->setOpenGLId((int)mActiveLights->count());
    mActiveLights->push_back(light);
    BoShader::setActiveLights(activeLights());
  }
  return light;
}

void BoLightManager::deleteLight(int id)
{
  BoLight* l = 0;
  for(Q3ValueList<BoLight*>::Iterator it = mAllLights->begin(); it != mAllLights->end(); it++)
  {
    if((*it)->id() == id)
    {
      l = *it;
      mAllLights->erase(it);
      break;
    }
  }

  if(l)
  {
    int openglid = l->openGLId();
    delete l;

    if(openglid >= 0)
    {
      // Remove the light from the list of active lights
      if(openglid + 1 < (int)mActiveLights->count())
      {
        // The light is in the middle of the vector.
        BoLight* otherlight = mActiveLights->last();
        mActiveLights->at(openglid) = otherlight;
        otherlight->setOpenGLId(openglid);
      }
      mActiveLights->pop_back();

      if(mAllLights->count() > mActiveLights->count())
      {
        // Make another light active
        BoLight* otherlight;
        // Find first inactive light...
        for(Q3ValueList<BoLight*>::Iterator it = mAllLights->begin(); it != mAllLights->end(); it++)
        {
          if(!(*it)->isActive())
          {
            otherlight = *it;
            break;
          }
        }
        // ... and active it
        otherlight->setOpenGLId(mAllLights->count());
        mActiveLights->push_back(otherlight);
      }
      BoShader::setActiveLights(activeLights());
    }
  }
}

int BoLightManager::activeLights() const
{
  return mActiveLights->count();
}

void BoLightManager::cameraChanged()
{
  for (unsigned int i = 0; i < mActiveLights->count(); i++)
  {
    mActiveLights->at(i)->refreshPosition();
  }
}

void BoLightManager::updateAllStates()
{
  for (unsigned int i = 0; i < mActiveLights->count(); i++)
  {
    mActiveLights->at(i)->updateStates();
  }
}


/*****  BoLight  *****/

BoLight::BoLight(int id)
{
  mId = id;
  mOpenGLId = -1;

  // Disable
  mEnabled = false;

  mAttenuation.setX(1);
}

BoLight::~BoLight()
{
  setEnabled(false);
}

void BoLight::setAmbient(const BoVector4Float& a)
{
  if(mAmbient == a)
  {
    return;
  }

  mAmbient = a;
  if(isActive())
  {
    glLightfv(GL_LIGHT0 + mOpenGLId, GL_AMBIENT, mAmbient.data());
  }
}

void BoLight::setDiffuse(const BoVector4Float& d)
{
  if(mDiffuse == d)
  {
    return;
  }

  mDiffuse = d;
  if(isActive())
  {
    glLightfv(GL_LIGHT0 + mOpenGLId, GL_DIFFUSE, mDiffuse.data());
  }
}

void BoLight::setSpecular(const BoVector4Float& s)
{
  if(mSpecular == s)
  {
    return;
  }

  mSpecular = s;
  if(isActive())
  {
    glLightfv(GL_LIGHT0 + mOpenGLId, GL_SPECULAR, mSpecular.data());
  }
}

void BoLight::setPosition(const BoVector4Float& pos)
{
  if(mPos == pos)
  {
    return;
  }

  mPos = pos;
  if(isActive())
  {
    glLightfv(GL_LIGHT0 + mOpenGLId, GL_POSITION, mPos.data());
  }
}

void BoLight::setConstantAttenuation(float a)
{
  if(constantAttenuation() == a)
  {
    return;
  }

  mAttenuation.setX(a);
  if(isActive())
  {
    glLightf(GL_LIGHT0 + mOpenGLId, GL_CONSTANT_ATTENUATION, a);
  }
}

void BoLight::setLinearAttenuation(float a)
{
  if(linearAttenuation() == a)
  {
    return;
  }

  mAttenuation.setY(a);
  if(isActive())
  {
    glLightf(GL_LIGHT0 + mOpenGLId, GL_LINEAR_ATTENUATION, a);
  }
}

void BoLight::setQuadraticAttenuation(float a)
{
  if(quadraticAttenuation() == a)
  {
    return;
  }

  mAttenuation.setZ(a);
  if(isActive())
  {
    glLightf(GL_LIGHT0 + mOpenGLId, GL_QUADRATIC_ATTENUATION, a);
  }
}

void BoLight::setAttenuation(const BoVector3Float& a)
{
  if(attenuation() == a)
  {
    return;
  }

  mAttenuation = a;
  if(isActive())
  {
    glLightf(GL_LIGHT0 + mOpenGLId, GL_CONSTANT_ATTENUATION, a.x());
    glLightf(GL_LIGHT0 + mOpenGLId, GL_LINEAR_ATTENUATION, a.y());
    glLightf(GL_LIGHT0 + mOpenGLId, GL_QUADRATIC_ATTENUATION, a.z());
  }
}

void BoLight::setEnabled(bool e)
{
  if(mEnabled == e)
  {
    return;
  }

  mEnabled = e;
  if(isActive())
  {
    if(mEnabled)
    {
      glEnable(GL_LIGHT0 + mOpenGLId);
    }
    else
    {
      glDisable(GL_LIGHT0 + mOpenGLId);
    }
  }
}

void BoLight::refreshPosition()
{
  if(isActive())
  {
    glLightfv(GL_LIGHT0 + mOpenGLId, GL_POSITION, mPos.data());
  }
}

void BoLight::setOpenGLId(int id)
{
  if(isActive() && id < 0)
  {
    glDisable(GL_LIGHT0 + mOpenGLId);
  }

  mOpenGLId = id;
  updateStates();
}

void BoLight::updateStates()
{
  if(!isActive())
  {
    return;
  }

  if(mEnabled)
  {
    glEnable(GL_LIGHT0 + mOpenGLId);
  }
  else
  {
    glDisable(GL_LIGHT0 + mOpenGLId);
  }

  glLightfv(GL_LIGHT0 + mOpenGLId, GL_AMBIENT, mAmbient.data());
  glLightfv(GL_LIGHT0 + mOpenGLId, GL_DIFFUSE, mDiffuse.data());
  glLightfv(GL_LIGHT0 + mOpenGLId, GL_SPECULAR, mSpecular.data());

  glLightf(GL_LIGHT0 + mOpenGLId, GL_CONSTANT_ATTENUATION, mAttenuation.x());
  glLightf(GL_LIGHT0 + mOpenGLId, GL_LINEAR_ATTENUATION, mAttenuation.y());
  glLightf(GL_LIGHT0 + mOpenGLId, GL_QUADRATIC_ATTENUATION, mAttenuation.z());

  glLightfv(GL_LIGHT0 + mOpenGLId, GL_POSITION, mPos.data());
}
