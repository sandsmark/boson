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

#include "pythonscript.h"

#include <Python.h>

#include <qstring.h>
#include <qfileinfo.h>

#include "bodebug.h"
#include "../bo3dtools.h"


PyObject* PythonScript::mDict = 0;

// BoScript methods table (these are accessible from scripts)
// Keep this up to date!
PyMethodDef PythonScript::mCallbacks[] = {
  { "moveCamera", py_moveCamera, METH_VARARGS, "Move camera" },
  { "moveCameraBy", py_moveCameraBy, METH_VARARGS, "" },
  { "setCameraRotation", py_setCameraRotation, METH_VARARGS, "" },
  { "setCameraRadius", py_setCameraRadius, METH_VARARGS, "" },
  { "setCameraZ", py_setCameraZ, METH_VARARGS, "" },
  { "setCameraMoveMode", py_setCameraMoveMode, METH_VARARGS, "" },
  { "commitCameraChanges", py_commitCameraChanges, METH_VARARGS, "" },
  { "cameraPos", py_cameraPos, METH_VARARGS, "" },
  { "cameraRotation", py_cameraRotation, METH_VARARGS, "" },
  { "cameraRadius", py_cameraRadius, METH_VARARGS, "" },
  { "cameraZ", py_cameraZ, METH_VARARGS, "" },
  //{ "", py_, METH_VARARGS, "" },
  { 0, 0, 0, 0 }
};


PythonScript::PythonScript()
{
  boDebug() << k_funcinfo << endl;
  Py_Initialize();

  PyImport_AddModule("BoScript");
  Py_InitModule("BoScript", mCallbacks);

  mDict = 0;
}

PythonScript::~PythonScript()
{
  boDebug() << k_funcinfo << endl;
  Py_Finalize();
}

void PythonScript::loadScript(QString file)
{
  boDebug() << k_funcinfo << "file: " << file << endl;
  QFileInfo fi(file);

  if(!fi.exists())
  {
    boError() << k_funcinfo << "No such file: '" << fi.absFilePath() << "'. Aborting." << endl;
    return;
  }

  QString filePath = fi.dirPath(true);
  boDebug() << k_funcinfo << "filePath: " << filePath << endl;
  QString fileName = fi.baseName();
  boDebug() << k_funcinfo << "fileName: " << fileName << endl;

  char pypath[4096];
  sprintf(pypath, "sys.path.insert(0, '%s')", filePath.ascii());
  PyRun_SimpleString("import sys");
  PyRun_SimpleString(pypath);
  PyRun_SimpleString("sys.path.insert(0, '')");


  PyObject* pName = PyString_FromString(fileName.ascii());
  if(!pName)
  {
    boError() << k_funcinfo << "pName is NULL" << endl;
  }
  PyObject* pModule = PyImport_Import(pName);
  if(!pModule)
  {
    boError() << k_funcinfo << "pModule is NULL" << endl;
    boError() << k_funcinfo << "Probably there's a parse error in the script. Aborting." << endl;
    boError() << k_funcinfo << "File was: '" << fi.absFilePath() << "'" << endl;
  }
  mDict = PyModule_GetDict(pModule);
}

void PythonScript::callFunction(QString function)
{
  boDebug() << k_funcinfo << "function: " << function << endl;
  if(!mDict)
  {
    boError() << k_funcinfo << "No file loaded!" << endl;
    return;
  }

  PyObject* func = PyDict_GetItemString(mDict, (char*)function.ascii());
  if(!func)
  {
    boError() << k_funcinfo << "No such function: " << function << endl;
    return;
  }

  PyObject_CallObject(func, 0);
}

void PythonScript::execLine(const QString& line)
{
  boDebug() << k_funcinfo << "line: " << line << endl;
  PyRun_SimpleString((char*)line.ascii());
}

void PythonScript::advance()
{
  callFunction("advance");
}

void PythonScript::init()
{
  callFunction("init");
}


PyObject* PythonScript::py_moveCamera(PyObject*, PyObject* args)
{
  float x, y, z;
  if(!PyArg_ParseTuple(args, "fff", &x, &y, &z))
  {
    return 0;
  }
  boScript->moveCamera(BoVector3(x, y, z));
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_moveCameraBy(PyObject*, PyObject* args)
{
  float x, y, z;
  if(!PyArg_ParseTuple(args, "fff", &x, &y, &z))
  {
    return 0;
  }
  boScript->moveCameraBy(BoVector3(x, y, z));
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_setCameraRotation(PyObject*, PyObject* args)
{
  float r;
  if(!PyArg_ParseTuple(args, "f", &r))
  {
    return 0;
  }
  boScript->setCameraRotation(r);
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_setCameraRadius(PyObject*, PyObject* args)
{
  float r;
  if(!PyArg_ParseTuple(args, "f", &r))
  {
    return 0;
  }
  boScript->setCameraRadius(r);
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_setCameraZ(PyObject*, PyObject* args)
{
  float z;
  if(!PyArg_ParseTuple(args, "f", &z))
  {
    return 0;
  }
  boScript->setCameraZ(z);
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_setCameraMoveMode(PyObject*, PyObject* args)
{
  int m;
  if(!PyArg_ParseTuple(args, "i", &m))
  {
    return 0;
  }
  boScript->setCameraMoveMode(m);
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_commitCameraChanges(PyObject*, PyObject* args)
{
  int ticks;
  if(!PyArg_ParseTuple(args, "i", &ticks))
  {
    return 0;
  }
  boScript->commitCameraChanges(ticks);
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_cameraPos(PyObject*, PyObject*)
{
  BoVector3 pos = boScript->cameraPos();
  return Py_BuildValue("[f, f, f]", pos.x(), pos.y(), pos.z());
}

PyObject* PythonScript::py_cameraRotation(PyObject*, PyObject*)
{
  float r = boScript->cameraRotation();
  return Py_BuildValue("f", r);
}

PyObject* PythonScript::py_cameraRadius(PyObject*, PyObject*)
{
  float r = boScript->cameraRadius();
  return Py_BuildValue("f", r);
}

PyObject* PythonScript::py_cameraZ(PyObject*, PyObject*)
{
  float r = boScript->cameraZ();
  return Py_BuildValue("f", r);
}
