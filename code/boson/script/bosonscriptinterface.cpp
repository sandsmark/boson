/*
    This file is part of the Boson game
    Copyright (C) 2004 Andreas Beckermann (b_mann@gmx.de)

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

#include "bosonscriptinterface.h"
#include "bosonscriptinterface.moc"

#include "bodebug.h"
#include "../bo3dtools.h"

BosonScriptInterface::BosonScriptInterface(QObject* parent, const char* name) : QObject(parent, name)
{
}

BosonScriptInterface::~BosonScriptInterface()
{
}

int BosonScriptInterface::addLight()
{
 int id = -2;
 emit signalAddLight(&id);
 if(id == -2)
 {
   // the slot should set id to -1 on error
   boError() << k_funcinfo << "signalAddLight() did not return anything useful" << endl;
   return -1;
 }
 return id;
}

void BosonScriptInterface::removeLight(int id)
{
  emit signalRemoveLight(id);
}

BoVector4 BosonScriptInterface::lightPos(int id)
{
  BoVector4 v;
  emit signalGetLightPos(id, &v);
  return v;
}

BoVector4 BosonScriptInterface::lightAmbient(int id)
{
  BoVector4 v;
  emit signalGetLightAmbient(id, &v);
  return v;
}

BoVector4 BosonScriptInterface::lightDiffuse(int id)
{
  BoVector4 v;
  emit signalGetLightDiffuse(id, &v);
  return v;
}

BoVector4 BosonScriptInterface::lightSpecular(int id)
{
  BoVector4 v;
  emit signalGetLightSpecular(id, &v);
  return v;
}

BoVector3 BosonScriptInterface::lightAttenuation(int id)
{
  BoVector3 v;
  emit signalGetLightAttenuation(id, &v);
  return v;
}

bool BosonScriptInterface::lightEnabled(int id)
{
  bool e;
  emit signalGetLightEnabled(id, &e);
  return e;
}



void BosonScriptInterface::setLightPos(int id, const BoVector4& v)
{
  emit signalSetLightPos(id, v);
}
void BosonScriptInterface::setLightAmbient(int id, const BoVector4& v)
{
  emit signalSetLightAmbient(id, v);
}

void BosonScriptInterface::setLightDiffuse(int id, const BoVector4& v)
{
  emit signalSetLightDiffuse(id, v);
}

void BosonScriptInterface::setLightSpecular(int id, const BoVector4& v)
{
  emit signalSetLightSpecular(id, v);
}

void BosonScriptInterface::setLightAttenuation(int id, const BoVector3& v)
{
  emit signalSetLightAttenuation(id, v);
}

void BosonScriptInterface::setLightEnabled(int id, bool e)
{
  emit signalSetLightEnabled(id, e);
}


BoVector3 BosonScriptInterface::cameraPos()
{
  BoVector3 v;
  emit signalGetCameraPos(&v);
  return v;
}

BoVector3 BosonScriptInterface::cameraUp()
{
  BoVector3 v;
  emit signalGetCameraUp(&v);
  return v;
}

BoVector3 BosonScriptInterface::cameraLookAt()
{
  BoVector3 v;
  emit signalGetCameraLookAt(&v);
  return v;
}

float BosonScriptInterface::cameraRotation()
{
  float v;
  emit signalGetCameraRotation(&v);
  return v;
}

float BosonScriptInterface::cameraRadius()
{
  float v;
  emit signalGetCameraRadius(&v);
  return v;
}

float BosonScriptInterface::cameraZ()
{
  float v;
  emit signalGetCameraZ(&v);
  return v;
}

void BosonScriptInterface::setUseCameraLimits(bool on)
{
  emit signalSetUseCameraLimits(on);
}

void BosonScriptInterface::setCameraFreeMovement(bool on)
{
  emit signalSetCameraFreeMovement(on);
}

void BosonScriptInterface::setCameraRotation(float v)
{
  emit signalSetCameraRotation(v);
}

void BosonScriptInterface::setCameraRadius(float v)
{
  emit signalSetCameraRadius(v);
}

void BosonScriptInterface::setCameraZ(float v)
{
  emit signalSetCameraZ(v);
}

void BosonScriptInterface::setCameraMoveMode(int v)
{
  emit signalSetCameraMoveMode(v);
}

void BosonScriptInterface::setCameraPos(const BoVector3& v)
{
  emit signalSetCameraPos(v);
}

void BosonScriptInterface::setCameraLookAt(const BoVector3& v)
{
  emit signalSetCameraLookAt(v);
}

void BosonScriptInterface::setCameraUp(const BoVector3& v)
{
  emit signalSetCameraUp(v);
}

void BosonScriptInterface::commitCameraChanges(int ticks)
{
  emit signalCommitCameraChanges(ticks);
}


