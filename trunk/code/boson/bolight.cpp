/*
    This file is part of the Boson game
    Copyright (C) 2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bolight.h"

#include "bodebug.h"
#include <bogl.h>

#include <qvaluevector.h>


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
  for(unsigned int i = 0; i < mLights->count(); i++)
  {
    delete (*mLights)[i];
  }
  mLights->clear();
  delete mLights;
}

void BoLightManager::init()
{
  int maxlights;
  glGetIntegerv(GL_MAX_LIGHTS, &maxlights);
  boDebug() << k_funcinfo << maxlights << " lights are supported" << endl;

  mLights = new QValueVector<BoLight*>();
  mLights->resize(maxlights, false);
}

int BoLightManager::nextFreeId()
{
  for(unsigned int i = 0; i < mLights->size(); i++)
  {
    if((*mLights)[i] == 0)
    {
      boDebug() << k_funcinfo << "Light " << i << " not used" << endl;
      return i;
    }
  }
  boDebug() << k_funcinfo << "All lights are already used" << endl;
  return -1;
}

void BoLightManager::setLight(int id, BoLight* light)
{
  (*mLights)[id] = light;
}

const QValueVector<BoLight*>* BoLightManager::lights()
{
  return mLights;
}

BoLight* BoLightManager::light(int id)
{
  return (*mLights)[id];
}

BoLight* BoLightManager::createLight()
{
  BoLight* light = new BoLight; // this also call BoLightManager::setLight()
  if(light->id() == -1)
  {
    // Light could not be created
    delete light;
    return 0;
  }
  return light;
}

void BoLightManager::deleteLight(int id)
{
  BoLight* l = (*mLights)[id];
  if(l)
  {
    (*mLights)[id] = 0;
    delete l;
  }
}

void BoLightManager::updateAllStates()
{
  for(unsigned int i = 0; i < mLights->size(); i++)
  {
    if((*mLights)[i] != 0)
    {
      (*mLights)[i]->updateStates();
    }
  }
}


/*****  BoLight  *****/

BoLight::BoLight()
{
  // Get id for this light
  mId = BoLightManager::manager()->nextFreeId();
  if(mId == -1)
  {
    return;  // All lights already in use
  }
  BoLightManager::manager()->setLight(mId, this);

  // Disable
  mEnabled = false;

  mAttenuation.setX(1);
}

BoLight::~BoLight()
{
  if(mId == -1)
  {
    // Invalid light, nothing was inited, nothing has to be uninited
    return;
  }
  setEnabled(false);
  BoLightManager::manager()->setLight(mId, 0);
}

void BoLight::setAmbient(const BoVector4Float& a)
{
  if(mAmbient == a)
  {
    return;
  }

  mAmbient = a;
  glLightfv(GL_LIGHT0 + mId, GL_AMBIENT, mAmbient.data());
}

void BoLight::setDiffuse(const BoVector4Float& d)
{
  if(mDiffuse == d)
  {
    return;
  }

  mDiffuse = d;
  glLightfv(GL_LIGHT0 + mId, GL_DIFFUSE, mDiffuse.data());
}

void BoLight::setSpecular(const BoVector4Float& s)
{
  if(mSpecular == s)
  {
    return;
  }

  mSpecular = s;
  glLightfv(GL_LIGHT0 + mId, GL_SPECULAR, mSpecular.data());
}

void BoLight::setPosition(const BoVector4Float& pos)
{
  if(mPos == pos)
  {
    return;
  }

  mPos = pos;
  glLightfv(GL_LIGHT0 + mId, GL_POSITION, mPos.data());
}

void BoLight::setConstantAttenuation(float a)
{
  if(constantAttenuation() == a)
  {
    return;
  }

  mAttenuation.setX(a);
  glLightf(GL_LIGHT0 + mId, GL_CONSTANT_ATTENUATION, a);
}

void BoLight::setLinearAttenuation(float a)
{
  if(linearAttenuation() == a)
  {
    return;
  }

  mAttenuation.setY(a);
  glLightf(GL_LIGHT0 + mId, GL_LINEAR_ATTENUATION, a);
}

void BoLight::setQuadraticAttenuation(float a)
{
  if(quadraticAttenuation() == a)
  {
    return;
  }

  mAttenuation.setZ(a);
  glLightf(GL_LIGHT0 + mId, GL_QUADRATIC_ATTENUATION, a);
}

void BoLight::setAttenuation(const BoVector3Float& a)
{
  if(attenuation() == a)
  {
    return;
  }

  mAttenuation = a;
  glLightf(GL_LIGHT0 + mId, GL_CONSTANT_ATTENUATION, a.x());
  glLightf(GL_LIGHT0 + mId, GL_LINEAR_ATTENUATION, a.y());
  glLightf(GL_LIGHT0 + mId, GL_QUADRATIC_ATTENUATION, a.z());
}

void BoLight::setEnabled(bool e)
{
  if(mEnabled == e)
  {
    return;
  }

  mEnabled = e;
  if(mEnabled)
  {
    glEnable(GL_LIGHT0 + mId);
  }
  else
  {
    glDisable(GL_LIGHT0 + mId);
  }
}

void BoLight::refreshPosition()
{
  glLightfv(GL_LIGHT0 + mId, GL_POSITION, mPos.data());
}

void BoLight::updateStates()
{
  if(mEnabled)
  {
    glEnable(GL_LIGHT0 + mId);
  }
  else
  {
    glDisable(GL_LIGHT0 + mId);
  }

  glLightfv(GL_LIGHT0 + mId, GL_AMBIENT, mAmbient.data());
  glLightfv(GL_LIGHT0 + mId, GL_DIFFUSE, mDiffuse.data());
  glLightfv(GL_LIGHT0 + mId, GL_SPECULAR, mSpecular.data());

  glLightf(GL_LIGHT0 + mId, GL_CONSTANT_ATTENUATION, mAttenuation.x());
  glLightf(GL_LIGHT0 + mId, GL_LINEAR_ATTENUATION, mAttenuation.y());
  glLightf(GL_LIGHT0 + mId, GL_QUADRATIC_ATTENUATION, mAttenuation.z());

  glLightfv(GL_LIGHT0 + mId, GL_POSITION, mPos.data());
}