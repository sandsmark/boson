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


PyThreadState* PythonScript::mThreadState = 0;
int PythonScript::mScriptInstances = 0;
bool PythonScript::mScriptingInited = false;

/*****  BoScript methods table (these are accessible from scripts)  *****/
// Keep this up to date!
PyMethodDef PythonScript::mCallbacks[] = {
  // Players
  { (char*)"areEnemies", py_areEnemies, METH_VARARGS, 0 },
  { (char*)"allPlayers", py_allPlayers, METH_VARARGS, 0 },
  // Resources
  { (char*)"minerals", py_minerals, METH_VARARGS, 0 },
  { (char*)"addMinerals", py_addMinerals, METH_VARARGS, 0 },
  { (char*)"oil", py_oil, METH_VARARGS, 0 },
  { (char*)"addOil", py_addOil, METH_VARARGS, 0 },
  { (char*)"nearestMineralLocations", py_nearestMineralLocations, METH_VARARGS, 0 },
  { (char*)"nearestOilLocations", py_nearestOilLocations, METH_VARARGS, 0 },
  // Units
  { (char*)"moveUnit", py_moveUnit, METH_VARARGS, 0 },
  { (char*)"moveUnitWithAttacking", py_moveUnitWithAttacking, METH_VARARGS, 0 },
  { (char*)"attack", py_attack, METH_VARARGS, 0 },
  { (char*)"stopUnit", py_stopUnit, METH_VARARGS, 0 },
  { (char*)"mineUnit", py_mineUnit, METH_VARARGS, 0 },
  { (char*)"setUnitRotation", py_setUnitRotation, METH_VARARGS, 0 },
  { (char*)"dropBomb", py_dropBomb, METH_VARARGS, 0 },
  { (char*)"produceUnit", py_produceUnit, METH_VARARGS, 0 },
  { (char*)"spawnUnit", py_spawnUnit, METH_VARARGS, 0 },
  { (char*)"teleportUnit", py_teleportUnit, METH_VARARGS, 0 },
  { (char*)"unitsOnCell", py_unitsOnCell, METH_VARARGS, 0 },
  { (char*)"unitsInRect", py_unitsInRect, METH_VARARGS, 0 },
  { (char*)"cellOccupied", py_cellOccupied, METH_VARARGS, 0 },
  { (char*)"unitPosition", py_unitPosition, METH_VARARGS, 0 },
  { (char*)"unitOwner", py_unitOwner, METH_VARARGS, 0 },
  { (char*)"unitType", py_unitType, METH_VARARGS, 0 },
  { (char*)"unitWork", py_unitWork, METH_VARARGS, 0 },
  { (char*)"isUnitMobile", py_isUnitMobile, METH_VARARGS, 0 },
  { (char*)"canUnitShoot", py_canUnitShoot, METH_VARARGS, 0 },
  { (char*)"canUnitProduce", py_canUnitProduce, METH_VARARGS, 0 },
  { (char*)"productionTypes", py_productionTypes, METH_VARARGS, 0 },
  { (char*)"isUnitAlive", py_isUnitAlive, METH_VARARGS, 0 },
  { (char*)"allPlayerUnits", py_allPlayerUnits, METH_VARARGS, 0 },
  { (char*)"allPlayerUnitsCount", py_allPlayerUnitsCount, METH_VARARGS, 0 },
  { (char*)"playerUnitsOfType", py_playerUnitsOfType, METH_VARARGS, 0 },
  { (char*)"playerUnitsOfTypeCount", py_playerUnitsOfTypeCount, METH_VARARGS, 0 },
  // Camera
  { (char*)"setCameraRotation", py_setCameraRotation, METH_VARARGS, 0 },
  { (char*)"setCameraRadius", py_setCameraRadius, METH_VARARGS, 0 },
  { (char*)"setCameraZ", py_setCameraZ, METH_VARARGS, 0 },
  { (char*)"setCameraMoveMode", py_setCameraMoveMode, METH_VARARGS, 0 },
  { (char*)"setCameraLookAt", py_setCameraLookAt, METH_VARARGS, 0 },
  { (char*)"setCameraPos", py_setCameraPos, METH_VARARGS, 0 },
  { (char*)"setCameraUp", py_setCameraUp, METH_VARARGS, 0 },
  { (char*)"setCameraLimits", py_setCameraLimits, METH_VARARGS, 0 },
  { (char*)"setCameraFreeMode", py_setCameraFreeMode, METH_VARARGS, 0 },
  { (char*)"commitCameraChanges", py_commitCameraChanges, METH_VARARGS, 0 },
  { (char*)"cameraLookAt", py_cameraLookAt, METH_VARARGS, 0 },
  { (char*)"cameraPos", py_cameraPos, METH_VARARGS, 0 },
  { (char*)"cameraUp", py_cameraUp, METH_VARARGS, 0 },
  { (char*)"cameraRotation", py_cameraRotation, METH_VARARGS, 0 },
  { (char*)"cameraRadius", py_cameraRadius, METH_VARARGS, 0 },
  { (char*)"cameraZ", py_cameraZ, METH_VARARGS, 0 },
  // Lights
  { (char*)"lightPos", py_lightPos, METH_VARARGS, 0 },
  { (char*)"lightAmbient", py_lightAmbient, METH_VARARGS, 0 },
  { (char*)"lightDiffuse", py_lightDiffuse, METH_VARARGS, 0 },
  { (char*)"lightSpecular", py_lightSpecular, METH_VARARGS, 0 },
  { (char*)"lightAttenuation", py_lightAttenuation, METH_VARARGS, 0 },
  { (char*)"lightEnabled", py_lightEnabled, METH_VARARGS, 0 },
  { (char*)"setLightPos", py_setLightPos, METH_VARARGS, 0 },
  { (char*)"setLightAmbient", py_setLightAmbient, METH_VARARGS, 0 },
  { (char*)"setLightDiffuse", py_setLightDiffuse, METH_VARARGS, 0 },
  { (char*)"setLightSpecular", py_setLightSpecular, METH_VARARGS, 0 },
  { (char*)"setLightAttenuation", py_setLightAttenuation, METH_VARARGS, 0 },
  { (char*)"setLightEnabled", py_setLightEnabled, METH_VARARGS, 0 },
  { (char*)"addLight", py_addLight, METH_VARARGS, 0 },
  { (char*)"removeLight", py_removeLight, METH_VARARGS, 0 },
  // AI
  { (char*)"aiDelay", py_aiDelay, METH_VARARGS, 0 },
  // Other
  { (char*)"startBenchmark", py_startBenchmark, METH_VARARGS, 0 },
  { (char*)"endBenchmark", py_endBenchmark, METH_VARARGS, 0 },
  { (char*)"setRandomSeed", py_setRandomSeed, METH_VARARGS, 0 },
  { (char*)"findPath", py_findPath, METH_VARARGS, 0 },
  { (char*)"addParticleSystem", py_addParticleSystem, METH_VARARGS, 0 },
  //{ (char*)"", py_, METH_VARARGS, 0 },
  { 0, 0, 0, 0 }
};


/*****  Basic stuff  *****/

PythonScript::PythonScript(Player* p) : BosonScript(p)
{
  boDebug() << k_funcinfo << endl;

  if(!mScriptingInited)
  {
    initScripting();
  }

  PyEval_AcquireLock();
  mInterpreter = Py_NewInterpreter();

  Py_InitModule((char*)"BoScript", mCallbacks);

  PyThreadState_Swap(0);
  PyEval_ReleaseLock();

  PyOS_FiniInterrupts();

  mDict = 0;
  mScriptInstances++;
}

PythonScript::~PythonScript()
{
  boDebug() << k_funcinfo << endl;

  getPythonLock();

  if(mDict)
  {
    Py_DECREF(mDict);
  }

  Py_EndInterpreter(mInterpreter);

  freePythonLock();

  mScriptInstances--;
  if(mScriptInstances == 0)
  {
    uninitScripting();
  }
}

void PythonScript::initScripting()
{
  boDebug() << k_funcinfo << endl;
  Py_Initialize();
  PyEval_InitThreads();
  mThreadState = PyThreadState_Get();
  PyEval_ReleaseLock();
  mScriptingInited = true;
}

void PythonScript::uninitScripting()
{
  boDebug() << k_funcinfo << endl;
  PyInterpreterState* mainState = mThreadState->interp;
  PyThreadState* myState = PyThreadState_New(mainState);
  PyThreadState_Swap(myState);
  PyEval_AcquireLock();
  Py_Finalize();
  mScriptingInited = false;
}

void PythonScript::getPythonLock()
{
  PyEval_AcquireLock();
  PyThreadState_Swap(mInterpreter);
}

void PythonScript::freePythonLock()
{
  PyThreadState_Swap(0);
  PyEval_ReleaseLock();
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

  getPythonLock();

  char pypath[4096];
  sprintf(pypath, "sys.path.insert(0, '%s')", filePath.ascii());
  PyRun_SimpleString((char*)"import sys");
  PyRun_SimpleString(pypath);
  PyRun_SimpleString((char*)"sys.path.insert(0, '')");


  PyObject* pName = PyString_FromString(fileName.ascii());
  if(!pName)
  {
    boError() << k_funcinfo << "pName is NULL" << endl;
    Py_DECREF(pName);
    freePythonLock();
    return;
  }
  PyObject* pModule = PyImport_Import(pName);
  Py_DECREF(pName);
  if(!pModule)
  {
    PyErr_Print();
    boError() << k_funcinfo << "pModule is NULL" << endl;
    boError() << k_funcinfo << "Probably there's a parse error in the script. Aborting." << endl;
    boError() << k_funcinfo << "File was: '" << fi.absFilePath() << "'" << endl;
    freePythonLock();
    return;
  }
  mDict = PyModule_GetDict(pModule);

  freePythonLock();
}

void PythonScript::callFunction(QString function)
{
  callFunction(function, 0);
}

void PythonScript::callFunction(QString function, PyObject* args)
{
  boDebug(700) << k_funcinfo << "function: " << function << endl;

  if(!mDict)
  {
    boError() << k_funcinfo << "No file loaded!" << endl;
    return;
  }

  getPythonLock();

  PyObject* func = PyDict_GetItemString(mDict, (char*)function.ascii());
  if(!func)
  {
    PyErr_Print();
    boError() << k_funcinfo << "No such function: " << function << endl;
    freePythonLock();
    return;
  }

  PyObject* pValue = PyObject_CallObject(func, args);
  if(!pValue)
  {
    PyErr_Print();
    boError() << k_funcinfo << "Error while calling function " << function << endl;
  }
  else
  {
    Py_DECREF(pValue);
  }

  freePythonLock();
}

void PythonScript::execLine(const QString& line)
{
  boDebug(700) << k_funcinfo << "line: " << line << endl;
  getPythonLock();

  PyRun_SimpleString((char*)line.ascii());

  freePythonLock();
}

void PythonScript::advance()
{
  callFunction("advance");
}

void PythonScript::init()
{
  PyObject* args = PyTuple_New(1);
  PyTuple_SetItem(args, 0, PyInt_FromLong(playerId()));
  callFunction("init", args);
}

/*****  Player functions  *****/
PyObject* PythonScript::py_areEnemies(PyObject*, PyObject* args)
{
  int id1, id2;
  if(!PyArg_ParseTuple(args, (char*)"ii", &id1, &id2))
  {
    return 0;
  }

  bool enemies = BosonScript::areEnemies(id1, id2);

  return Py_BuildValue((char*)"i", enemies ? 1 : 0);
}

PyObject* PythonScript::py_allPlayers(PyObject*, PyObject*)
{
  QValueList<int> players = BosonScript::allPlayers();

  return QValueListToPyList(&players);
}


/*****  Resource functions  *****/
PyObject* PythonScript::py_minerals(PyObject*, PyObject* args)
{
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", (int)(BosonScript::minerals(id)));
}

PyObject* PythonScript::py_addMinerals(PyObject*, PyObject* args)
{
  int id, amount;
  if(!PyArg_ParseTuple(args, (char*)"ii", &id, &amount))
  {
    return 0;
  }

  BosonScript::addMinerals(id, amount);

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_oil(PyObject*, PyObject* args)
{
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", (int)(BosonScript::oil(id)));
}

PyObject* PythonScript::py_addOil(PyObject*, PyObject* args)
{
  int id, amount;
  if(!PyArg_ParseTuple(args, (char*)"ii", &id, &amount))
  {
    return 0;
  }

  BosonScript::addOil(id, amount);

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_nearestMineralLocations(PyObject*, PyObject* args)
{
  int player, x, y, n, radius;
  if(!PyArg_ParseTuple(args, (char*)"iiiii", &player, &x, &y, &n, &radius))
  {
    return 0;
  }

  QValueList<QPoint> locations = BosonScript::nearestMineralLocations(player, x, y, n, radius);

  PyObject* pylist = PyList_New(locations.count());

  int i = 0;
  QValueList<QPoint>::Iterator it;
  for(it = locations.begin(); it != locations.end(); ++it)
  {
    // We use tuples for positions
    PyObject* pos = PyTuple_New(2);
    PyTuple_SetItem(pos, 0, PyInt_FromLong((*it).x()));
    PyTuple_SetItem(pos, 1, PyInt_FromLong((*it).y()));
    PyList_SetItem(pylist, i, pos);
    i++;
  }

  return pylist;
}

PyObject* PythonScript::py_nearestOilLocations(PyObject*, PyObject* args)
{
  int player, x, y, n, radius;
  if(!PyArg_ParseTuple(args, (char*)"iiiii", &player, &x, &y, &n, &radius))
  {
    return 0;
  }

  QValueList<QPoint> locations = BosonScript::nearestOilLocations(player, x, y, n, radius);

  PyObject* pylist = PyList_New(locations.count());

  int i = 0;
  QValueList<QPoint>::Iterator it;
  for(it = locations.begin(); it != locations.end(); ++it)
  {
    // We use tuples for positions
    PyObject* pos = PyTuple_New(2);
    PyTuple_SetItem(pos, 0, PyInt_FromLong((*it).x()));
    PyTuple_SetItem(pos, 1, PyInt_FromLong((*it).y()));
    PyList_SetItem(pylist, i, pos);
    i++;
  }

  return pylist;
}


/*****  Unit functions  *****/
PyObject* PythonScript::py_moveUnit(PyObject*, PyObject* args)
{
  int player, id, x, y;
  if(!PyArg_ParseTuple(args, (char*)"iiii", &player, &id, &x, &y))
  {
    return 0;
  }

  BosonScript::moveUnit(player, id, x, y);

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_moveUnitWithAttacking(PyObject*, PyObject* args)
{
  int player, id, x, y;
  if(!PyArg_ParseTuple(args, (char*)"iiii", &player, &id, &x, &y))
  {
    return 0;
  }

  BosonScript::moveUnitWithAttacking(player, id, x, y);

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_attack(PyObject*, PyObject* args)
{
  int player, id, target;
  if(!PyArg_ParseTuple(args, (char*)"iii", &player, &id, &target))
  {
    return 0;
  }

  BosonScript::attack(player, id, target);

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_stopUnit(PyObject*, PyObject* args)
{
  int player, id;
  if(!PyArg_ParseTuple(args, (char*)"ii", &player, &id))
  {
    return 0;
  }

  BosonScript::stopUnit(player, id);

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_mineUnit(PyObject*, PyObject* args)
{
  int player, id, x, y;
  if(!PyArg_ParseTuple(args, (char*)"iiii", &player, &id, &x, &y))
  {
    return 0;
  }

  BosonScript::mineUnit(player, id, x, y);

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_setUnitRotation(PyObject*, PyObject* args)
{
  int player, id;
  float rot;
  if(!PyArg_ParseTuple(args, (char*)"iif", &player, &id, &rot))
  {
    return 0;
  }

  BosonScript::setUnitRotation(player, id, rot);

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_dropBomb(PyObject*, PyObject* args)
{
  int player, id, weapon, x, y;
  if(!PyArg_ParseTuple(args, (char*)"iiiii", &player, &id, &weapon, &x, &y))
  {
    return 0;
  }

  BosonScript::dropBomb(player, id, weapon, x, y);

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_produceUnit(PyObject*, PyObject* args)
{
  int player, factory, production;
  if(!PyArg_ParseTuple(args, (char*)"iii", &player, &factory, &production))
  {
    return 0;
  }

  BosonScript::produceUnit(player, factory, production);

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_spawnUnit(PyObject*, PyObject* args)
{
  int player, type, x, y;
  if(!PyArg_ParseTuple(args, (char*)"iiii", &player, &type, &x, &y))
  {
    return 0;
  }

  BosonScript::spawnUnit(player, type, x, y);

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_teleportUnit(PyObject*, PyObject* args)
{
  int player, id, x, y;
  if(!PyArg_ParseTuple(args, (char*)"iiii", &player, &id, &x, &y))
  {
    return 0;
  }

  BosonScript::teleportUnit(player, id, x, y);

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

  QValueList<int> units = BosonScript::unitsOnCell(x, y);

  return QValueListToPyList(&units);
}

PyObject* PythonScript::py_unitsInRect(PyObject*, PyObject* args)
{
  int x1, y1, x2, y2;
  if(!PyArg_ParseTuple(args, (char*)"iiii", &x1, &y1, &x2, &y2))
  {
    return 0;
  }

  QValueList<int> units = BosonScript::unitsInRect(x1, y1, x2, y2);

  return QValueListToPyList(&units);
}

PyObject* PythonScript::py_cellOccupied(PyObject*, PyObject* args)
{
  int x, y;
  if(!PyArg_ParseTuple(args, (char*)"ii", &x, &y))
  {
    return 0;
  }

  bool occupied = BosonScript::cellOccupied(x, y);

  return Py_BuildValue((char*)"i", occupied ? 1 : 0);
}

PyObject* PythonScript::py_unitPosition(PyObject*, PyObject* args)
{
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  QPoint pos = BosonScript::unitPosition(id);

  return Py_BuildValue((char*)"(ii)", pos.x(), pos.y());
}

PyObject* PythonScript::py_unitOwner(PyObject*, PyObject* args)
{
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", BosonScript::unitOwner(id));
}

PyObject* PythonScript::py_unitType(PyObject*, PyObject* args)
{
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", BosonScript::unitType(id));
}

PyObject* PythonScript::py_unitWork(PyObject*, PyObject* args)
{
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", BosonScript::unitWork(id));
}

PyObject* PythonScript::py_isUnitMobile(PyObject*, PyObject* args)
{
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", BosonScript::isUnitMobile(id) ? 1 : 0);
}

PyObject* PythonScript::py_canUnitShoot(PyObject*, PyObject* args)
{
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", BosonScript::canUnitShoot(id) ? 1 : 0);
}

PyObject* PythonScript::py_canUnitProduce(PyObject*, PyObject* args)
{
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", BosonScript::canUnitProduce(id) ? 1 : 0);
}

PyObject* PythonScript::py_productionTypes(PyObject*, PyObject* args)
{
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  QValueList<int> list = BosonScript::productionTypes(id);

  return QValueListToPyList(&list);
}

PyObject* PythonScript::py_isUnitAlive(PyObject*, PyObject* args)
{
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", BosonScript::isUnitAlive(id) ? 1 : 0);
}

PyObject* PythonScript::py_allPlayerUnits(PyObject*, PyObject* args)
{
  // FIXME: current implementation of methods returning arrays is quite
  //  ineffiecient. First we usually get a list of units, then add ids of those
  //  units to id list and then covert id list to Python list. We could add ids
  //  directly to Python list.
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  QValueList<int> units = BosonScript::allPlayerUnits(id);

  return QValueListToPyList(&units);
}

PyObject* PythonScript::py_allPlayerUnitsCount(PyObject*, PyObject* args)
{
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", BosonScript::allPlayerUnitsCount(id));
}

PyObject* PythonScript::py_playerUnitsOfType(PyObject*, PyObject* args)
{
  int id, type;
  if(!PyArg_ParseTuple(args, (char*)"ii", &id, &type))
  {
    return 0;
  }

  QValueList<int> units = BosonScript::playerUnitsOfType(id, type);

  return QValueListToPyList(&units);
}

PyObject* PythonScript::py_playerUnitsOfTypeCount(PyObject*, PyObject* args)
{
  int id, type;
  if(!PyArg_ParseTuple(args, (char*)"ii", &id, &type))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", BosonScript::playerUnitsOfTypeCount(id, type));
}



/*****  Camera functions  *****/

PyObject* PythonScript::py_setCameraRotation(PyObject*, PyObject* args)
{
  float r;
  if(!PyArg_ParseTuple(args, (char*)"f", &r))
  {
    return 0;
  }
  BosonScript::setCameraRotation(r);
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
  BosonScript::setCameraRadius(r);
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
  BosonScript::setCameraZ(z);
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
  BosonScript::setCameraMoveMode(m);
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_setCameraLookAt(PyObject*, PyObject* args)
{
  float x, y, z;
  if(!PyArg_ParseTuple(args, (char*)"fff", &x, &y, &z))
  {
    return 0;
  }
  BosonScript::setCameraLookAt(BoVector3(x, y, z));
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_setCameraPos(PyObject*, PyObject* args)
{
  float x, y, z;
  if(!PyArg_ParseTuple(args, (char*)"fff", &x, &y, &z))
  {
    return 0;
  }
  BosonScript::setCameraPos(BoVector3(x, y, z));
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_setCameraUp(PyObject*, PyObject* args)
{
  float x, y, z;
  if(!PyArg_ParseTuple(args, (char*)"fff", &x, &y, &z))
  {
    return 0;
  }
  BosonScript::setCameraUp(BoVector3(x, y, z));
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_setCameraLimits(PyObject*, PyObject* args)
{
  int on;
  if(!PyArg_ParseTuple(args, (char*)"i", &on))
  {
    return 0;
  }
  BosonScript::setCameraLimits((bool)on);
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_setCameraFreeMode(PyObject*, PyObject* args)
{
  int on;
  if(!PyArg_ParseTuple(args, (char*)"i", &on))
  {
    return 0;
  }
  BosonScript::setCameraFreeMode((bool)on);
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
  BosonScript::commitCameraChanges(ticks);
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_cameraLookAt(PyObject*, PyObject*)
{
  BoVector3 pos = BosonScript::cameraLookAt();
  return Py_BuildValue((char*)"[f, f, f]", pos.x(), pos.y(), pos.z());
}

PyObject* PythonScript::py_cameraPos(PyObject*, PyObject*)
{
  BoVector3 pos = BosonScript::cameraPos();
  return Py_BuildValue((char*)"[f, f, f]", pos.x(), pos.y(), pos.z());
}

PyObject* PythonScript::py_cameraUp(PyObject*, PyObject*)
{
  BoVector3 pos = BosonScript::cameraUp();
  return Py_BuildValue((char*)"[f, f, f]", pos.x(), pos.y(), pos.z());
}

PyObject* PythonScript::py_cameraRotation(PyObject*, PyObject*)
{
  return Py_BuildValue((char*)"f", BosonScript::cameraRotation());
}

PyObject* PythonScript::py_cameraRadius(PyObject*, PyObject*)
{
  return Py_BuildValue((char*)"f", BosonScript::cameraRadius());
}

PyObject* PythonScript::py_cameraZ(PyObject*, PyObject*)
{
  return Py_BuildValue((char*)"f", BosonScript::cameraZ());
}


/*****  Light functions  *****/
PyObject* PythonScript::py_lightPos(PyObject*, PyObject* args)
{
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  BoVector4 pos = BosonScript::lightPos(id);
  return Py_BuildValue((char*)"(ffff)", pos.x(), pos.y(), pos.z(), pos.w());
}

PyObject* PythonScript::py_lightAmbient(PyObject*, PyObject* args)
{
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  BoVector4 a = BosonScript::lightAmbient(id);
  return Py_BuildValue((char*)"(ffff)", a.x(), a.y(), a.z(), a.w());
}

PyObject* PythonScript::py_lightDiffuse(PyObject*, PyObject* args)
{
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  BoVector4 d = BosonScript::lightDiffuse(id);
  return Py_BuildValue((char*)"(ffff)", d.x(), d.y(), d.z(), d.w());
}

PyObject* PythonScript::py_lightSpecular(PyObject*, PyObject* args)
{
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  BoVector4 s = BosonScript::lightSpecular(id);
  return Py_BuildValue((char*)"(ffff)", s.x(), s.y(), s.z(), s.w());
}

PyObject* PythonScript::py_lightAttenuation(PyObject*, PyObject* args)
{
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  BoVector3 a = BosonScript::lightAttenuation(id);
  return Py_BuildValue((char*)"(fff)", a.x(), a.y(), a.z());
}

PyObject* PythonScript::py_lightEnabled(PyObject*, PyObject* args)
{
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", BosonScript::lightEnabled(id) ? 1 : 0);
}


PyObject* PythonScript::py_setLightPos(PyObject*, PyObject* args)
{
  int id;
  float x, y, z, w;
  if(!PyArg_ParseTuple(args, (char*)"i(ffff)", &id, &x, &y, &z, &w))
  {
    return 0;
  }

  BosonScript::setLightPos(id, BoVector4(x, y, z, w));

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_setLightAmbient(PyObject*, PyObject* args)
{
  int id;
  float r, g, b, a;
  if(!PyArg_ParseTuple(args, (char*)"i(ffff)", &id, &r, &g, &b, &a))
  {
    return 0;
  }

  BosonScript::setLightAmbient(id, BoVector4(r, g, b, a));

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_setLightDiffuse(PyObject*, PyObject* args)
{
  int id;
  float r, g, b, a;
  if(!PyArg_ParseTuple(args, (char*)"i(ffff)", &id, &r, &g, &b, &a))
  {
    return 0;
  }

  BosonScript::setLightDiffuse(id, BoVector4(r, g, b, a));

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_setLightSpecular(PyObject*, PyObject* args)
{
  int id;
  float r, g, b, a;
  if(!PyArg_ParseTuple(args, (char*)"i(ffff)", &id, &r, &g, &b, &a))
  {
    return 0;
  }

  BosonScript::setLightSpecular(id, BoVector4(r, g, b, a));

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_setLightAttenuation(PyObject*, PyObject* args)
{
  int id;
  float c, l, q;
  if(!PyArg_ParseTuple(args, (char*)"i(fff)", &id, &c, &l, &q))
  {
    return 0;
  }

  BosonScript::setLightAttenuation(id, BoVector3(c, l, q));

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_setLightEnabled(PyObject*, PyObject* args)
{
  int id, enable;
  if(!PyArg_ParseTuple(args, (char*)"ii", &id, &enable))
  {
    return 0;
  }

  BosonScript::setLightEnabled(id, enable);

  Py_INCREF(Py_None);
  return Py_None;
}


PyObject* PythonScript::py_addLight(PyObject*, PyObject*)
{
  return Py_BuildValue((char*)"i", BosonScript::addLight());
}

PyObject* PythonScript::py_removeLight(PyObject*, PyObject* args)
{
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  BosonScript::removeLight(id);
  Py_INCREF(Py_None);
  return Py_None;
}



/*****  AI functions  *****/
PyObject* PythonScript::py_aiDelay(PyObject*, PyObject*)
{
  return Py_BuildValue((char*)"f", BosonScript::aiDelay());
}

/*****  Other functions  *****/
PyObject* PythonScript::py_startBenchmark(PyObject*, PyObject*)
{
  BosonScript::startBenchmark();
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_endBenchmark(PyObject*, PyObject* args)
{
  char* name = 0;
  if(!PyArg_ParseTuple(args, (char*)"|s", &name))
  {
    return 0;
  }
  BosonScript::endBenchmark(QString(name));
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_setRandomSeed(PyObject*, PyObject* args)
{
  int seed;
  if(!PyArg_ParseTuple(args, (char*)"i", &seed))
  {
    return 0;
  }
  BosonScript::setRandomSeed(seed);
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_findPath(PyObject*, PyObject* args)
{
  int x1, y1, x2, y2;
  if(!PyArg_ParseTuple(args, (char*)"iiii", &x1, &y1, &x2, &y2))
  {
    return 0;
  }
  BosonScript::findPath(x1, y1, x2, y2);
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_addParticleSystem(PyObject*, PyObject* args)
{
  int player, id;
  float x, y, z, rot = 0.0f;
  if(!PyArg_ParseTuple(args, (char*)"ii(fff)|f", &player, &id, &x, &y, &z, &rot))
  {
    return 0;
  }
  BosonScript::addParticleSystem(player, id, BoVector3(x, y, z), rot);
  Py_INCREF(Py_None);
  return Py_None;
}

/*****  Non-script functions  *****/

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

