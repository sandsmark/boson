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
#include "../bosonprofiling.h"


PyObject* PythonScript::mDict = 0;

/*****  BoScript methods table (these are accessible from scripts)  *****/
// Keep this up to date!
PyMethodDef PythonScript::mCallbacks[] = {
  // Players
  { (char*)"areEnemies", py_areEnemies, METH_VARARGS, 0 },
  { (char*)"playerId", py_playerId, METH_VARARGS, 0 },
  { (char*)"allPlayers", py_allPlayers, METH_VARARGS, 0 },
  // Resources
  { (char*)"minerals", py_minerals, METH_VARARGS, 0 },
  { (char*)"oil", py_oil, METH_VARARGS, 0 },
  // Units
  { (char*)"moveUnit", py_moveUnit, METH_VARARGS, 0 },
  { (char*)"moveUnitWithAttacking", py_moveUnitWithAttacking, METH_VARARGS, 0 },
  { (char*)"attack", py_attack, METH_VARARGS, 0 },
  { (char*)"stopUnit", py_stopUnit, METH_VARARGS, 0 },
  { (char*)"unitsOnCell", py_unitsOnCell, METH_VARARGS, 0 },
  { (char*)"unitsInRect", py_unitsInRect, METH_VARARGS, 0 },
  { (char*)"cellOccupied", py_cellOccupied, METH_VARARGS, 0 },
  { (char*)"unitPosition", py_unitPosition, METH_VARARGS, 0 },
  { (char*)"unitOwner", py_unitOwner, METH_VARARGS, 0 },
  { (char*)"unitType", py_unitType, METH_VARARGS, 0 },
  { (char*)"isUnitMobile", py_isUnitMobile, METH_VARARGS, 0 },
  { (char*)"canUnitShoot", py_canUnitShoot, METH_VARARGS, 0 },
  { (char*)"isMyUnit", py_isMyUnit, METH_VARARGS, 0 },
  { (char*)"isUnitAlive", py_isUnitAlive, METH_VARARGS, 0 },
  { (char*)"allMyUnits", py_allMyUnits, METH_VARARGS, 0 },
  { (char*)"allPlayerUnits", py_allPlayerUnits, METH_VARARGS, 0 },
  // Camera
  { (char*)"moveCamera", py_moveCamera, METH_VARARGS, 0 },
  { (char*)"moveCameraBy", py_moveCameraBy, METH_VARARGS, 0 },
  { (char*)"setCameraRotation", py_setCameraRotation, METH_VARARGS, 0 },
  { (char*)"setCameraRadius", py_setCameraRadius, METH_VARARGS, 0 },
  { (char*)"setCameraZ", py_setCameraZ, METH_VARARGS, 0 },
  { (char*)"setCameraMoveMode", py_setCameraMoveMode, METH_VARARGS, 0 },
  { (char*)"commitCameraChanges", py_commitCameraChanges, METH_VARARGS, 0 },
  { (char*)"cameraPos", py_cameraPos, METH_VARARGS, 0 },
  { (char*)"cameraRotation", py_cameraRotation, METH_VARARGS, 0 },
  { (char*)"cameraRadius", py_cameraRadius, METH_VARARGS, 0 },
  { (char*)"cameraZ", py_cameraZ, METH_VARARGS, 0 },
  // AI
  { (char*)"aiDelay", py_aiDelay, METH_VARARGS, 0 },
  //{ (char*)"", py_, METH_VARARGS, 0 },
  { 0, 0, 0, 0 }
};


/*****  Basic stuff  *****/

PythonScript::PythonScript()
{
  boDebug() << k_funcinfo << endl;
  Py_Initialize();

  Py_InitModule((char*)"BoScript", mCallbacks);

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
  //boDebug() << k_funcinfo << "filePath: " << filePath << endl;
  QString fileName = fi.baseName();
  //boDebug() << k_funcinfo << "fileName: " << fileName << endl;

  char pypath[4096];
  sprintf(pypath, "sys.path.insert(0, '%s')", filePath.ascii());
  PyRun_SimpleString((char*)"import sys");
  PyRun_SimpleString(pypath);
  PyRun_SimpleString((char*)"sys.path.insert(0, '')");


  PyObject* pName = PyString_FromString(fileName.ascii());
  if(!pName)
  {
    boError() << k_funcinfo << "pName is NULL" << endl;
  }
  PyObject* pModule = PyImport_Import(pName);
  if(!pModule)
  {
    PyErr_Print();
    boError() << k_funcinfo << "pModule is NULL" << endl;
    boError() << k_funcinfo << "Probably there's a parse error in the script. Aborting." << endl;
    boError() << k_funcinfo << "File was: '" << fi.absFilePath() << "'" << endl;
    return;
  }
  mDict = PyModule_GetDict(pModule);
}

void PythonScript::callFunction(QString function)
{
  boDebug(700) << k_funcinfo << "function: " << function << endl;
  if(!mDict)
  {
    boError() << k_funcinfo << "No file loaded!" << endl;
    return;
  }

  PyObject* func = PyDict_GetItemString(mDict, (char*)function.ascii());
  if(!func)
  {
    PyErr_Print();
    boError() << k_funcinfo << "No such function: " << function << endl;
    return;
  }

  PyObject* pValue = PyObject_CallObject(func, 0);
  if(!pValue)
  {
    PyErr_Print();
    boError() << k_funcinfo << "Error while calling function " << function << endl;
  }
  else
  {
    Py_DECREF(pValue);
  }
}

void PythonScript::execLine(const QString& line)
{
  boDebug(700) << k_funcinfo << "line: " << line << endl;
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

/*****  Player functions  *****/
PyObject* PythonScript::py_areEnemies(PyObject*, PyObject* args)
{
  int id1, id2;
  if(!PyArg_ParseTuple(args, (char*)"ii", &id1, &id2))
  {
    return 0;
  }

  bool enemies = boScript->areEnemies(id1, id2);

  return Py_BuildValue((char*)"i", enemies ? 1 : 0);
}

PyObject* PythonScript::py_playerId(PyObject*, PyObject*)
{
  return Py_BuildValue((char*)"i", boScript->playerId());
}

PyObject* PythonScript::py_allPlayers(PyObject*, PyObject*)
{
  QValueList<int> players = boScript->allPlayers();

  return QValueListToPyList(&players);
}


/*****  Resource functions  *****/
PyObject* PythonScript::py_minerals(PyObject*, PyObject*)
{
  return Py_BuildValue((char*)"l", (long int)(boScript->mineralsAmount()));
}

PyObject* PythonScript::py_oil(PyObject*, PyObject*)
{
  return Py_BuildValue((char*)"l", (long int)(boScript->oilAmount()));
}


/*****  Unit functions  *****/
PyObject* PythonScript::py_moveUnit(PyObject*, PyObject* args)
{
  int id, x, y;
  if(!PyArg_ParseTuple(args, (char*)"iii", &id, &x, &y))
  {
    return 0;
  }

  boScript->moveUnit(id, x, y);

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_moveUnitWithAttacking(PyObject*, PyObject* args)
{
  int id, x, y;
  if(!PyArg_ParseTuple(args, (char*)"iii", &id, &x, &y))
  {
    return 0;
  }

  boScript->moveUnitWithAttacking(id, x, y);

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_attack(PyObject*, PyObject* args)
{
  int id, target;
  if(!PyArg_ParseTuple(args, (char*)"ii", &id, &target))
  {
    return 0;
  }

  boScript->attack(id, target);

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_stopUnit(PyObject*, PyObject* args)
{
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  boScript->stopUnit(id);

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_unitsOnCell(PyObject*, PyObject* args)
{
  int x, y;
  if(!PyArg_ParseTuple(args, (char*)"ii", &x, &y))
  {
    return 0;
  }

  QValueList<int> units = boScript->unitsOnCell(x, y);

  return QValueListToPyList(&units);
}

PyObject* PythonScript::py_unitsInRect(PyObject*, PyObject* args)
{
  int x1, y1, x2, y2;
  if(!PyArg_ParseTuple(args, (char*)"iiii", &x1, &y1, &x2, &y2))
  {
    return 0;
  }

  QValueList<int> units = boScript->unitsInRect(x1, y1, x2, y2);

  return QValueListToPyList(&units);
}

PyObject* PythonScript::py_cellOccupied(PyObject*, PyObject* args)
{
  int x, y;
  if(!PyArg_ParseTuple(args, (char*)"ii", &x, &y))
  {
    return 0;
  }

  bool occupied = boScript->cellOccupied(x, y);

  return Py_BuildValue((char*)"i", occupied ? 1 : 0);
}

PyObject* PythonScript::py_unitPosition(PyObject*, PyObject* args)
{
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  QPoint pos = boScript->unitPosition(id);

  return Py_BuildValue((char*)"(ii)", pos.x(), pos.y());
}

PyObject* PythonScript::py_unitOwner(PyObject*, PyObject* args)
{
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", boScript->unitOwner(id));
}

PyObject* PythonScript::py_unitType(PyObject*, PyObject* args)
{
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", boScript->unitType(id));
}

PyObject* PythonScript::py_isUnitMobile(PyObject*, PyObject* args)
{
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", boScript->isUnitMobile(id) ? 1 : 0);
}

PyObject* PythonScript::py_canUnitShoot(PyObject*, PyObject* args)
{
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", boScript->canUnitShoot(id) ? 1 : 0);
}

PyObject* PythonScript::py_isMyUnit(PyObject*, PyObject* args)
{
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", boScript->isMyUnit(id) ? 1 : 0);
}

PyObject* PythonScript::py_isUnitAlive(PyObject*, PyObject* args)
{
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", boScript->isUnitAlive(id) ? 1 : 0);
}

PyObject* PythonScript::py_allMyUnits(PyObject*, PyObject*)
{
  // FIXME: current implementation of methods returning arrays is quite
  //  ineffiecient. First we usually get a list of units, then add ids of those
  //  units to id list and then covert id list to Python list. We could add ids
  //  directly to Python list.
  QValueList<int> units = boScript->allMyUnits();

  return QValueListToPyList(&units);
}

PyObject* PythonScript::py_allPlayerUnits(PyObject*, PyObject* args)
{
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  QValueList<int> units = boScript->allPlayerUnits(id);

  return QValueListToPyList(&units);
}


/*****  Camera functions  *****/

PyObject* PythonScript::py_moveCamera(PyObject*, PyObject* args)
{
  float x, y, z;
  if(!PyArg_ParseTuple(args, (char*)"fff", &x, &y, &z))
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
  if(!PyArg_ParseTuple(args, (char*)"fff", &x, &y, &z))
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
  if(!PyArg_ParseTuple(args, (char*)"f", &r))
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
  if(!PyArg_ParseTuple(args, (char*)"f", &r))
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
  if(!PyArg_ParseTuple(args, (char*)"f", &z))
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
  if(!PyArg_ParseTuple(args, (char*)"i", &m))
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
  if(!PyArg_ParseTuple(args, (char*)"i", &ticks))
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
  return Py_BuildValue((char*)"[f, f, f]", pos.x(), pos.y(), pos.z());
}

PyObject* PythonScript::py_cameraRotation(PyObject*, PyObject*)
{
  return Py_BuildValue((char*)"f", boScript->cameraRotation());
}

PyObject* PythonScript::py_cameraRadius(PyObject*, PyObject*)
{
  return Py_BuildValue((char*)"f", boScript->cameraRadius());
}

PyObject* PythonScript::py_cameraZ(PyObject*, PyObject*)
{
  return Py_BuildValue((char*)"f", boScript->cameraZ());
}


/*****  AI functions  *****/
PyObject* PythonScript::py_aiDelay(PyObject*, PyObject*)
{
  return Py_BuildValue((char*)"f", boScript->aiDelay());
}


PyObject* PythonScript::QValueListToPyList(QValueList<int>* list)
{
  PyObject* pylist = PyList_New(list->count());

  int i = 0;
  QValueList<int>::Iterator it;
  for(it = list->begin(); it != list->end(); ++it)
  {
    PyList_SetItem(pylist, i, PyInt_FromLong(*it));
    i++;
  }

  return pylist;
}

