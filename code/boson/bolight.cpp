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

#include <GL/gl.h>

#include <qvaluevector.h>


/*****  BoLightManager  *****/

bool BoLightManager::mInited = false;
QValueVector<bool> BoLightManager::mIds;

void BoLightManager::init()
{
  if(mInited)
  {
    return;
  }

  int maxlights;
  glGetIntegerv(GL_MAX_LIGHTS, &maxlights);
  boDebug() << k_funcinfo << maxlights << " lights are supported" << endl;

  mIds.resize(maxlights, false);

  mInited = true;
}

int BoLightManager::nextFreeId()
{
  init();

  for(unsigned int i = 0; i < mIds.size(); i++)
  {
    if(mIds[i] == false)
    {
      boDebug() << k_funcinfo << "Light " << i << " not used" << endl;
      return i;
    }
  }
  boDebug() << k_funcinfo << "All lights are already used" << endl;
  return -1;
}

void BoLightManager::setIdUsed(int id, bool used)
{
  init();

  mIds[id] = used;
}


/*****  BoLight  *****/

BoLight::BoLight()
{
  // Get id for this light
  mId = BoLightManager::nextFreeId();
  if(mId == -1)
  {
    return;  // All lights already in use
  }
  BoLightManager::setIdUsed(mId, true);

  // Disable
  mEnabled = false;
}

BoLight::~BoLight()
{
  if(mId == -1)
  {
    // Invalid light, nothing was inited, nothing has to be uninited
    return;
  }
  setEnabled(false);
  BoLightManager::setIdUsed(mId, false);
}

void BoLight::setAmbient(BoVector4 a)
{
  if(mAmbient == a)
  {
    return;
  }

  mAmbient = a;
  glLightfv(GL_LIGHT0 + mId, GL_AMBIENT, mAmbient.data());
}

void BoLight::setDiffuse(BoVector4 d)
{
  if(mDiffuse == d)
  {
    return;
  }

  mDiffuse = d;
  glLightfv(GL_LIGHT0 + mId, GL_DIFFUSE, mDiffuse.data());
}

void BoLight::setSpecular(BoVector4 s)
{
  if(mSpecular == s)
  {
    return;
  }

  mSpecular = s;
  glLightfv(GL_LIGHT0 + mId, GL_SPECULAR, mSpecular.data());
}

void BoLight::setPosition(BoVector4 pos)
{
  if(mPos == pos)
  {
    return;
  }

  mPos = pos;
  boDebug() << k_funcinfo << "Position changed to " << mPos.debugString(3) << endl;
  glLightfv(GL_LIGHT0 + mId, GL_POSITION, mPos.data());
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

