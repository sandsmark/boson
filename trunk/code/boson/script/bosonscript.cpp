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

#include "bosonscript.h"

#include "../bo3dtools.h"
#include "../bocamera.h"
#include "../bosonbigdisplaybase.h"
#include "bodebug.h"

#include "pythonscript.h"


BosonScript* BosonScript::mScript = 0;


BosonScript* BosonScript::newScriptParser(Language lang)
{
  boDebug() << k_funcinfo << endl;
  BosonScript* s;
  if(lang == Python)
  {
    s = new PythonScript();
  }
  else
  {
    boDebug() << k_funcinfo << "Invalid script language: " << lang << endl;
    return 0;
  }
  return s;
}

BosonScript::BosonScript()
{
  boDebug() << k_funcinfo << endl;
  mScript = this;
}

BosonScript::~BosonScript()
{
  boDebug() << k_funcinfo << endl;
  mScript = 0;
}


void BosonScript::moveCamera(BoVector3 pos)
{
  mDisplay->camera()->setLookAt(pos);
}

void BosonScript::moveCameraBy(BoVector3 pos)
{
  mDisplay->camera()->changeLookAt(pos);
}

void BosonScript::setCameraRotation(float r)
{
  mDisplay->camera()->setRotation(r);
}

void BosonScript::setCameraRadius(float r)
{
  mDisplay->camera()->setRadius(r);
}

void BosonScript::setCameraZ(float z)
{
  mDisplay->camera()->setZ(z);
}

void BosonScript::setCameraMoveMode(int mode)
{
  boDebug() << k_funcinfo << "mode: " << mode << endl;
  mDisplay->camera()->setMoveMode((BoCamera::MoveMode)mode);
}

void BosonScript::commitCameraChanges(int ticks)
{
  mDisplay->camera()->commitChanges(ticks);
}

BoVector3 BosonScript::cameraPos()
{
  return mDisplay->camera()->lookAt();
}

float BosonScript::cameraRotation()
{
  return mDisplay->camera()->rotation();
}

float BosonScript::cameraRadius()
{
  return mDisplay->camera()->radius();
}

float BosonScript::cameraZ()
{
  return mDisplay->camera()->z();
}
