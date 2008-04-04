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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "bosonscriptinterface.h"

#include "../../../bomemory/bodummymemory.h"
#include "bodebug.h"
#include "../../bo3dtools.h"

#include "bosonscriptinterface.moc"

BosonScriptInterface::BosonScriptInterface(QObject* parent, const char* name) : QObject(parent, name)
{
}

BosonScriptInterface::~BosonScriptInterface()
{
}

int BosonScriptInterface::addEventHandler(const QString& eventname, const QString& functionname, const QString& args)
{
  int id = -1;
  emit signalAddEventHandler(eventname, functionname, args, &id);
  return id;
}

void BosonScriptInterface::removeEventHandler(int id)
{
  emit signalRemoveEventHandler(id);
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

BoVector4Float BosonScriptInterface::lightPos(int id)
{
  BoVector4Float v;
  emit signalGetLightPos(id, &v);
  return v;
}

BoVector4Float BosonScriptInterface::lightAmbient(int id)
{
  BoVector4Float v;
  emit signalGetLightAmbient(id, &v);
  return v;
}

BoVector4Float BosonScriptInterface::lightDiffuse(int id)
{
  BoVector4Float v;
  emit signalGetLightDiffuse(id, &v);
  return v;
}

BoVector4Float BosonScriptInterface::lightSpecular(int id)
{
  BoVector4Float v;
  emit signalGetLightSpecular(id, &v);
  return v;
}

BoVector3Float BosonScriptInterface::lightAttenuation(int id)
{
  BoVector3Float v;
  emit signalGetLightAttenuation(id, &v);
  return v;
}

bool BosonScriptInterface::lightEnabled(int id)
{
  bool e;
  emit signalGetLightEnabled(id, &e);
  return e;
}



void BosonScriptInterface::setLightPos(int id, const BoVector4Float& v)
{
  emit signalSetLightPos(id, v);
}
void BosonScriptInterface::setLightAmbient(int id, const BoVector4Float& v)
{
  emit signalSetLightAmbient(id, v);
}

void BosonScriptInterface::setLightDiffuse(int id, const BoVector4Float& v)
{
  emit signalSetLightDiffuse(id, v);
}

void BosonScriptInterface::setLightSpecular(int id, const BoVector4Float& v)
{
  emit signalSetLightSpecular(id, v);
}

void BosonScriptInterface::setLightAttenuation(int id, const BoVector3Float& v)
{
  emit signalSetLightAttenuation(id, v);
}

void BosonScriptInterface::setLightEnabled(int id, bool e)
{
  emit signalSetLightEnabled(id, e);
}


BoVector3Float BosonScriptInterface::cameraPos()
{
  BoVector3Float v;
  emit signalGetCameraPos(&v);
  return v;
}

BoVector3Float BosonScriptInterface::cameraUp()
{
  BoVector3Float v;
  emit signalGetCameraUp(&v);
  return v;
}

BoVector3Float BosonScriptInterface::cameraLookAt()
{
  BoVector3Float v;
  emit signalGetCameraLookAt(&v);
  return v;
}

float BosonScriptInterface::cameraRotation()
{
  float v;
  emit signalGetCameraRotation(&v);
  return v;
}

float BosonScriptInterface::cameraXRotation()
{
  float v;
  emit signalGetCameraXRotation(&v);
  return v;
}

float BosonScriptInterface::cameraDistance()
{
  float v;
  emit signalGetCameraDistance(&v);
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

void BosonScriptInterface::setCameraXRotation(float v)
{
  emit signalSetCameraXRotation(v);
}

void BosonScriptInterface::setCameraDistance(float v)
{
  emit signalSetCameraDistance(v);
}

void BosonScriptInterface::setCameraMoveMode(int v)
{
  emit signalSetCameraMoveMode(v);
}

void BosonScriptInterface::setCameraInterpolationMode(int mode)
{
  emit signalSetCameraInterpolationMode(mode);
}

void BosonScriptInterface::setCameraPos(const BoVector3Float& v)
{
  emit signalSetCameraPos(v);
}

void BosonScriptInterface::setCameraLookAt(const BoVector3Float& v)
{
  emit signalSetCameraLookAt(v);
}

void BosonScriptInterface::setCameraUp(const BoVector3Float& v)
{
  emit signalSetCameraUp(v);
}

void BosonScriptInterface::addCameraLookAtPoint(const BoVector3Float& pos, float time)
{
  emit signalAddCameraLookAtPoint(pos, time);
}

void BosonScriptInterface::addCameraPosPoint(const BoVector3Float& pos, float time)
{
  emit signalAddCameraPosPoint(pos, time);
}

void BosonScriptInterface::addCameraUpPoint(const BoVector3Float& up, float time)
{
  emit signalAddCameraUpPoint(up, time);
}

void BosonScriptInterface::commitCameraChanges(int ticks)
{
  emit signalCommitCameraChanges(ticks);
}

void BosonScriptInterface::setAcceptUserInput(bool accept)
{
  emit signalSetAcceptUserInput(accept);
}

void BosonScriptInterface::addEffect(unsigned int id, const BoVector3Fixed& pos, bofixed zrot)
{
  emit signalAddEffect(id, pos, zrot);
}

void BosonScriptInterface::addEffectToUnit(int unitid, unsigned int effectid, BoVector3Fixed offset, bofixed zrot)
{
  emit signalAddEffectToUnit(unitid, effectid, offset, zrot);
}

void BosonScriptInterface::advanceEffects(int ticks)
{
  emit signalAdvanceEffects(ticks);
}

void BosonScriptInterface::setWind(const BoVector3Float& wind)
{
  emit signalSetWind(wind);
}

BoVector3Float BosonScriptInterface::wind()
{
  BoVector3Float w;
  emit signalGetWind(&w);
  return w;
}

