/*
    This file is part of the Boson game
    Copyright (C) 2003-2005 Rivo Laks (rivolaks@hot.ee)

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

#include "../../../bomemory/bodummymemory.h"

#include <Python.h>
#include <marshal.h>
#include <compile.h>

#include <qstring.h>
#include <qfileinfo.h>
#include <qstringlist.h>
#include <qmap.h>

#include "bodebug.h"
#include "../../bo3dtools.h"
#include "../boevent.h"

// note: for this to work, getPythonLock() must be called first
#define CHECK_PYTHON_ERROR if(PyErr_Occurred() != NULL) \
        { \
          boError() << k_funcinfo << "(line: " << __LINE__ << ") Python error: " << endl; \
          PyErr_Print(); \
          PyErr_Clear(); \
        }


PyThreadState* PythonScript::mThreadState = 0;
int PythonScript::mScriptInstances = 0;
bool PythonScript::mScriptingInited = false;

/*****  BoScript methods table (these are accessible from scripts)  *****/
// Keep this up to date!
PyMethodDef PythonScript::mCallbacks[] = {
  // Events
  { (char*)"addEventHandler", py_addEventHandler, METH_VARARGS, 0 },
  { (char*)"removeEventHandler", py_removeEventHandler, METH_VARARGS, 0 },
  // Players
  { (char*)"areEnemies", py_areEnemies, METH_VARARGS, 0 },
  { (char*)"isEnemy", py_isEnemy, METH_VARARGS, 0 },
  { (char*)"allPlayers", py_allGamePlayers, METH_VARARGS, 0 },
  { (char*)"isNeutral", py_isNeutral, METH_VARARGS, 0 },
  { (char*)"powerGenerated", py_powerGenerated, METH_VARARGS, 0 },
  { (char*)"powerConsumed", py_powerConsumed, METH_VARARGS, 0 },
  { (char*)"powerGeneratedAfterConstructions", py_powerGeneratedAfterConstructions, METH_VARARGS, 0 },
  { (char*)"powerConsumedAfterConstructions", py_powerConsumedAfterConstructions, METH_VARARGS, 0 },
  { (char*)"isCellFogged", py_isCellFogged, METH_VARARGS, 0 },
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
  { (char*)"canPlaceProductionAt", py_canPlaceProductionAt, METH_VARARGS, 0 },
  { (char*)"placeProduction", py_placeProduction, METH_VARARGS, 0 },
  { (char*)"unitsOnCell", py_unitsOnCell, METH_VARARGS, 0 },
  { (char*)"unitsInRect", py_unitsInRect, METH_VARARGS, 0 },
  { (char*)"cellOccupied", py_cellOccupied, METH_VARARGS, 0 },
  { (char*)"unitPosition", py_unitPosition, METH_VARARGS, 0 },
  { (char*)"unitOwner", py_unitOwner, METH_VARARGS, 0 },
  { (char*)"unitType", py_unitType, METH_VARARGS, 0 },
  { (char*)"unitAdvanceWork", py_unitAdvanceWork, METH_VARARGS, 0 },
  { (char*)"unitSightRange", py_unitSightRange, METH_VARARGS, 0 },
  { (char*)"isUnitMobile", py_isUnitMobile, METH_VARARGS, 0 },
  { (char*)"isUnitTypeMobile", py_isUnitTypeMobile, METH_VARARGS, 0 },
  { (char*)"isUnitAircraft", py_isUnitAircraft, METH_VARARGS, 0 },
  { (char*)"isUnitTypeAircraft", py_isUnitTypeAircraft, METH_VARARGS, 0 },
  { (char*)"canUnitShoot", py_canUnitShoot, METH_VARARGS, 0 },
  { (char*)"canUnitTypeShoot", py_canUnitTypeShoot, METH_VARARGS, 0 },
  { (char*)"canUnitProduce", py_canUnitProduce, METH_VARARGS, 0 },
  { (char*)"hasUnitCompletedProduction", py_hasUnitCompletedProduction, METH_VARARGS, 0 },
  { (char*)"completedProductionType", py_completedProductionType, METH_VARARGS, 0 },
  { (char*)"canUnitMineMinerals", py_canUnitMineMinerals, METH_VARARGS, 0 },
  { (char*)"canUnitTypeMineMinerals", py_canUnitTypeMineMinerals, METH_VARARGS, 0 },
  { (char*)"canUnitMineOil", py_canUnitMineOil, METH_VARARGS, 0 },
  { (char*)"canUnitTypeMineOil", py_canUnitTypeMineOil, METH_VARARGS, 0 },
  { (char*)"productionTypes", py_productionTypes, METH_VARARGS, 0 },
  { (char*)"isUnitAlive", py_isUnitAlive, METH_VARARGS, 0 },
  { (char*)"allPlayerUnits", py_allPlayerUnits, METH_VARARGS, 0 },
  { (char*)"allPlayerUnitsCount", py_allPlayerUnitsCount, METH_VARARGS, 0 },
  { (char*)"playerUnitsOfType", py_playerUnitsOfType, METH_VARARGS, 0 },
  { (char*)"playerUnitsOfTypeCount", py_playerUnitsOfTypeCount, METH_VARARGS, 0 },
  { (char*)"allUnitsVisibleFor", py_allUnitsVisibleFor, METH_VARARGS, 0 },
  { (char*)"allEnemyUnitsVisibleFor", py_allEnemyUnitsVisibleFor, METH_VARARGS, 0 },
  // Camera
  { (char*)"setCameraRotation", py_setCameraRotation, METH_VARARGS, 0 },
  { (char*)"setCameraXRotation", py_setCameraXRotation, METH_VARARGS, 0 },
  { (char*)"setCameraDistance", py_setCameraDistance, METH_VARARGS, 0 },
  { (char*)"setCameraMoveMode", py_setCameraMoveMode, METH_VARARGS, 0 },
  { (char*)"setCameraInterpolationMode", py_setCameraInterpolationMode, METH_VARARGS, 0 },
  { (char*)"setCameraLookAt", py_setCameraLookAt, METH_VARARGS, 0 },
  { (char*)"setCameraPos", py_setCameraPos, METH_VARARGS, 0 },
  { (char*)"setCameraUp", py_setCameraUp, METH_VARARGS, 0 },
  { (char*)"setCameraLimits", py_setCameraLimits, METH_VARARGS, 0 },
  { (char*)"setCameraFreeMode", py_setCameraFreeMode, METH_VARARGS, 0 },
  { (char*)"addCameraLookAtPoint", py_addCameraLookAtPoint, METH_VARARGS, 0 },
  { (char*)"addCameraUpPoint", py_addCameraUpPoint, METH_VARARGS, 0 },
  { (char*)"addCameraPosPoint", py_addCameraPosPoint, METH_VARARGS, 0 },
  { (char*)"commitCameraChanges", py_commitCameraChanges, METH_VARARGS, 0 },
  { (char*)"cameraLookAt", py_cameraLookAt, METH_VARARGS, 0 },
  { (char*)"cameraPos", py_cameraPos, METH_VARARGS, 0 },
  { (char*)"cameraUp", py_cameraUp, METH_VARARGS, 0 },
  { (char*)"cameraRotation", py_cameraRotation, METH_VARARGS, 0 },
  { (char*)"cameraXRotation", py_cameraXRotation, METH_VARARGS, 0 },
  { (char*)"cameraDistance", py_cameraDistance, METH_VARARGS, 0 },
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
  { (char*)"addEffect", py_addEffect, METH_VARARGS, 0 },
  { (char*)"addEffectToUnit", py_addEffectToUnit, METH_VARARGS, 0 },
  { (char*)"advanceEffects", py_advanceEffects, METH_VARARGS, 0 },
  { (char*)"wind", py_wind, METH_VARARGS, 0 },
  { (char*)"setWind", py_setWind, METH_VARARGS, 0 },
  { (char*)"explorePlayer", py_explorePlayer, METH_VARARGS, 0 },
  { (char*)"exploreAllPlayers", py_exploreAllPlayers, METH_VARARGS, 0 },
  { (char*)"unfogPlayer", py_unfogPlayer, METH_VARARGS, 0 },
  { (char*)"unfogAllPlayers", py_unfogAllPlayers, METH_VARARGS, 0 },
  { (char*)"fogPlayer", py_fogPlayer, METH_VARARGS, 0 },
  { (char*)"fogAllPlayers", py_fogAllPlayers, METH_VARARGS, 0 },
  { (char*)"setAcceptUserInput", py_setAcceptUserInput, METH_VARARGS, 0 },
  { (char*)"addChatMessage", py_addChatMessage, METH_VARARGS, 0 },
  { (char*)"mapWidth", py_mapWidth, METH_VARARGS, 0 },
  { (char*)"mapHeight", py_mapHeight, METH_VARARGS, 0 },
  //{ (char*)"", py_, METH_VARARGS, 0 },
  { 0, 0, 0, 0 }
};


/*****  Basic stuff  *****/

PythonScript::PythonScript(int playerId) : BosonScript(playerId)
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

  mMainModule = 0;
  mDict = 0;
  mScriptInstances++;
}

PythonScript::~PythonScript()
{
  boDebug() << k_funcinfo << endl;

  getPythonLock();
  CHECK_PYTHON_ERROR;

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
  makeScriptCurrent(this);
  PyEval_AcquireLock();
  PyThreadState_Swap(mInterpreter);
}

void PythonScript::freePythonLock()
{
  PyThreadState_Swap(0);
  PyEval_ReleaseLock();
  makeScriptCurrent(0);
}

bool PythonScript::loadScript(QString file)
{
  boDebug() << k_funcinfo << "file: " << file << endl;
  QFileInfo fi(file);

  if(!fi.exists())
  {
    boError() << k_funcinfo << "No such file: '" << fi.absFilePath() << "'. Aborting." << endl;
    return false;
  }
  QString filePath = fi.dirPath(true);

  QFile f(file);
  if(!f.open(IO_ReadOnly))
  {
    boError() << k_funcinfo << "Can't open file '" << file << "' for reading. Aborting." << endl;
    return false;
  }

  return loadScriptFromString(f.readAll());
}

bool PythonScript::loadScriptFromString(const QString& string)
{
  getPythonLock();
  CHECK_PYTHON_ERROR;

  mMainModule = PyImport_AddModule((char*)"__main__");
  mDict = PyModule_GetDict(mMainModule);

  QString code;
  code += "import sys\n";
  code += QString("sys.path.insert(0, '%1')\n").arg(BosonScript::scriptsPath());
  code += QString("sys.path.insert(0, '%1')\n").arg(BosonScript::scriptsPath() + "pythonlib");
  code += "sys.path.insert(0, '')\n";
  code += string;

  PyObject* obj = PyRun_String(code.ascii(), Py_file_input, mDict, mDict);
  if(!obj)
  {
    PyErr_Print();
    boError() << k_funcinfo << "obj is NULL" << endl;
    boError() << k_funcinfo << "Probably there's a parse error in the script. Aborting." << endl;
    freePythonLock();
    return false;
  }
  Py_DECREF(obj);

  mLoadedScripts += string;
  mLoadedScripts += '\n';

  CHECK_PYTHON_ERROR;
  freePythonLock();
  return true;
}

void PythonScript::callFunction(const QString& function)
{
  callFunction(function, 0);
}

void PythonScript::callFunction(const QString& function, PyObject* args)
{
  boDebug(700) << k_funcinfo << "function: " << function << endl;

  if(!mDict)
  {
    boError() << k_funcinfo << "No file loaded!" << endl;
    return;
  }

  getPythonLock();
  CHECK_PYTHON_ERROR;

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

  /*static int callscount = 0;
  callscount++;
  if((callscount % 10) == 0)
  {
    QFile f(QString("boscript-%1.save").arg(callscount));
    f.open(IO_WriteOnly);
    QDataStream stream(&f);
    if(!save(stream))
    {
      boError() << k_funcinfo  << "Couldn't save!" << endl;
    }
    f.close();
  }*/

  CHECK_PYTHON_ERROR;
  freePythonLock();
}

int PythonScript::callFunctionWithReturn(const QString& function)
{
  return callFunctionWithReturn(function, 0);
}

int PythonScript::callFunctionWithReturn(const QString& function, PyObject* args)
{
  // AB: this is duplicated code from callFunction().
  // probably we need a callFunctionInteral which returns the returned PyObject.
  // callFunction() would then simply Py_DECREF() it, and here we would also
  // parse it.
  boDebug(700) << k_funcinfo << "function: " << function << endl;

  if(!mDict)
  {
    boError() << k_funcinfo << "No file loaded!" << endl;
    return -1;
  }

  getPythonLock();
  CHECK_PYTHON_ERROR;

  PyObject* func = PyDict_GetItemString(mDict, (char*)function.ascii());
  if(!func)
  {
    PyErr_Print();
    boError() << k_funcinfo << "No such function: " << function << endl;
    freePythonLock();
    return -1;
  }

  int ret = -1;
  PyObject* pValue = PyObject_CallObject(func, args);
  if(!pValue)
  {
    PyErr_Print();
    boError() << k_funcinfo << "Error while calling function " << function << endl;
    ret = -1;
  }
  else
  {
    // AB: FIXME: the return value doesn't seem to work as intended.
    //     apparently we always get Py_None here.
    //     -> try this by changing "init()" in ai.py to return a value. we still
    //        get into the PyNone branch only.
    if(pValue == Py_None)
    {
      ret = 0;
    }
    else if(pValue->ob_type == &PyInt_Type)
    {
      PyArg_Parse(pValue, (char*)"i", &ret);
    }
    else
    {
      boDebug() << "unhandled return value type" << endl;
      ret = 0;
    }
    Py_DECREF(pValue);
  }

  CHECK_PYTHON_ERROR;
  freePythonLock();
  return ret;
}

void PythonScript::execLine(const QString& line)
{
  boDebug(700) << k_funcinfo << "line: " << line << endl;
  getPythonLock();
  CHECK_PYTHON_ERROR;

  PyRun_SimpleString((char*)line.ascii());

  CHECK_PYTHON_ERROR;
  freePythonLock();
}

void PythonScript::callEventHandler(const BoEvent* e, const QString& function, const QString& args)
{
  if(!e)
  {
    boError() << k_funcinfo << "NULL event!" << endl;
    return;
  }
  if(function.isEmpty())
  {
    boError() << k_funcinfo << "function name is empty!" << endl;
    return;
  }

  // Break function name string into module and function name
  PyObject* dict = 0;
  QString funcname = function;
  // If it contains module name, it must contain dot.
  if(function.find('.') == -1)
  {
    // Function is in __main__ module
    dict = mDict;
  }
  else
  {
    // Find the module
    QStringList modulelist = QStringList::split('.', function);
    PyObject* lastdict = mDict;
    // There should be this many modules in the modulelist, the last string is
    //  name of the function.
    int modulecount = modulelist.count() - 1;
    int i = 0;
    // Recursively go through the modulelist and find the module where the
    //  function is.
    for(QStringList::Iterator it = modulelist.begin(); (it != modulelist.end()) && (i < modulecount); ++it, i++)
    {
      // Find object with given name in lastmodule
      PyObject* o = PyDict_GetItemString(lastdict, (char*)(*it).latin1());
      if(!o)
      {
        boError() << k_funcinfo << "Couldn't find object '" << *it << "'! Full function name was '" << function << "'" << endl;
        return;
      }
      // Make sure o is module
      if(!PyModule_Check(o))
      {
        boError() << k_funcinfo << "'" << *it << "' is not a module! Full function name was '" << function << "'" << endl;
        return;
      }
      lastdict = PyModule_GetDict(o);
    }

    dict = lastdict;
    funcname = modulelist.last();
  }
  // Make sure we have valid dict
  if(!dict)
  {
    boError() << k_funcinfo << "Couldn't find dict!" << endl;
    return;
  }

  getPythonLock();
  CHECK_PYTHON_ERROR;
  freePythonLock();


  // Create the argument list for the function call
  PyObject* funcargs = PyTuple_New(args.length());

  for(unsigned int i = 0; i < args.length(); i++)
  {
    PyObject* o = 0;
    if(args.at(i) == 'p')
    {
      o = PyInt_FromLong(e->playerId());
    }
    else if(args.at(i) == 'u')
    {
      o = PyInt_FromLong(e->unitId());
    }
    else if(args.at(i) == 'l')
    {
      PyObject* location = PyTuple_New(3);
      PyTuple_SetItem(location, 0, PyFloat_FromDouble(e->location().x()));
      PyTuple_SetItem(location, 1, PyFloat_FromDouble(e->location().y()));
      PyTuple_SetItem(location, 2, PyFloat_FromDouble(e->location().z()));
      o = location;
    }
    else if(args.at(i) == 'n')
    {
      o = PyString_FromString(e->name());
    }
    else if(args.at(i) == 'a')
    {
      o = PyString_FromString(e->data1().latin1());
    }
    else if(args.at(i) == 'b')
    {
      o = PyString_FromString(e->data2().latin1());
    }
    else
    {
      boError() << k_funcinfo << "Unknown format char '" << QString(args.at(i)) << "' in format string '" <<
          args << "'" << endl;
    }

    PyTuple_SetItem(funcargs, i, o);
  }


  // Make the function call
  // TODO: make a callFunction() method which takes module as an argument. Then
  //  we could get rid of duplicated code here.
  getPythonLock();
  CHECK_PYTHON_ERROR;

  PyObject* func = PyDict_GetItemString(dict, (char*)funcname.ascii());
  if(!func)
  {
    PyErr_Print();
    boError() << k_funcinfo << "No such function: " << funcname << endl;
    freePythonLock();
    return;
  }

  PyObject* pValue = PyObject_CallObject(func, funcargs);
  if(!pValue)
  {
    PyErr_Print();
    boError() << k_funcinfo << "Error while calling function " << funcname << endl;
  }
  else
  {
    Py_DECREF(pValue);
  }

  CHECK_PYTHON_ERROR;
  freePythonLock();

  Py_DECREF(funcargs);
}

bool PythonScript::init()
{
  getPythonLock();
  CHECK_PYTHON_ERROR;
  PyObject* args = PyTuple_New(1);
  PyTuple_SetItem(args, 0, PyInt_FromLong(playerId()));
  freePythonLock();
  int ret = callFunctionWithReturn("init", args);
  getPythonLock();
  Py_DECREF(args);
  CHECK_PYTHON_ERROR;
  freePythonLock();
  return (ret == 0);
}

void PythonScript::setPlayerId(int id)
{
  getPythonLock();
  CHECK_PYTHON_ERROR;
  PyObject* args = PyTuple_New(1);
  PyTuple_SetItem(args, 0, PyInt_FromLong(id));
  freePythonLock();
  callFunction("setPlayerId", args);
  getPythonLock();
  Py_DECREF(args);
  CHECK_PYTHON_ERROR;
  freePythonLock();
}

// TODO: check if the use of this variable is correct. if it is, integrate
// into the class.
QMap<PyObject*, PyObject*> g_seenModules;

bool PythonScript::save(QDataStream& stream)
{
  boDebug() << k_funcinfo << endl;

  getPythonLock();
  CHECK_PYTHON_ERROR;

  // Save main module to a PyObject
  g_seenModules.clear();
  PyObject* savedmodule = saveModule(mMainModule);
  g_seenModules.clear();

  // Save all items in savedict
#ifdef Py_MARSHAL_VERSION
  // Py_MARSHAL_VERSION is a macro that tells the version of marshal.c (Python >= 2.4)
  // Note that we use version 0 here to keep compatibility with savegames saved
  //  with older python version
  PyObject* data = PyMarshal_WriteObjectToString(savedmodule, 0);
#else
  // Python < 2.4 does not have Py_MARSHAL_VERSION
  PyObject* data = PyMarshal_WriteObjectToString(savedmodule);
#endif
  if(!data)
  {
    boError() << k_funcinfo << "null data!" << endl;
    PyErr_Print();
    freePythonLock();
    return false;
  }

  boDebug() << k_funcinfo << "Data string length is " << PyString_Size(data) << endl;

  stream << mLoadedScripts;
  // We can't just stream PyString_AsString(data), because it might contain
  //  NULL bytes.
  stream.writeBytes(PyString_AsString(data), PyString_Size(data));

  CHECK_PYTHON_ERROR;
  freePythonLock();

  boDebug() << k_funcinfo << "END" << endl;
  return true;
}

PyObject* PythonScript::saveModule(PyObject* module) const
{
  /**
   * Python module saving format:
   *
   * * PyDict
   *   * "variables" - PyDict
   *     * All variables in the module, saved in a PyDict
   *   * "submodules" - PyDict
   *     * All submodules in the module, saved in a PyDict. Note that this
   *       module can be empty, then module has no submodules.
   **/
  boDebug() << k_funcinfo << endl;


  // Find dictionary of the module
  PyObject* moduledict = PyModule_GetDict(module);

  // Top-level dictionary for this module. This is the one that contains two
  //  PyDicts with keys "variables" and "submodules"
  PyObject* maindict = PyDict_New();
  g_seenModules.insert(module, maindict);

  // Create a dict containing submodules of this module
  PyObject* submodules = PyDict_New();
  // Create a dict containing only variables (not functions)
  PyObject* variables = PyDict_New();

  // Iterate through all entries in this module's dict and add them to
  //  variables or submodules dict if possible.
  PyObject* key;
  PyObject* value;
  int pos = 0;
  while(PyDict_Next(moduledict, &pos, &key, &value))
  {
    // Check if value is any of the known types
    // Note that I'm using Andi's coding style here to keep the code small
    // Skip certain internal Python data
    if((strncmp(PyString_AsString(key), "__", 2) == 0) || (strcmp(PyString_AsString(key), "sys") == 0) ||
        (strcmp(PyString_AsString(key), "os") == 0) || (strcmp(PyString_AsString(key), "traceback") == 0))
    {
      // Skip everything with name "__*", "sys", "os" or "traceback"
      continue;
    }
    // This is basically taken from w_object() in Python/marshal.c
    if(value == NULL) {
    } else if(value == Py_None) {
    } else if(value == PyExc_StopIteration) {
    } else if(value == Py_Ellipsis) {
    } else if(value == Py_False) {
    } else if(value == Py_True) {
    } else if(PyInt_Check(value)) {
    } else if(PyLong_Check(value)) {
    } else if(PyFloat_Check(value)) {
    } else if(PyString_Check(value)) {
    } else if(PyTuple_Check(value)) {
    } else if(PyList_Check(value)) {
    } else if(PyDict_Check(value)) {
    } else if(PyCode_Check(value)) {
    } else if(PyObject_CheckReadBuffer(value)) {
    } else if(PyModule_Check(value)) {
      // Add module to submodules' list
      boDebug() << k_funcinfo << "Saving module '" << PyString_AsString(key) << "', exact: " << PyModule_CheckExact(value) << endl;
      if (g_seenModules.contains(value)) {
        boDebug() << k_funcinfo << "module already seen .. ignoring" << endl;
      } else {
        PyDict_SetItem(submodules, key, saveModule(value));
      }
      continue;
    } else {
      // This is unknown type (most likely a function)
      // Note that we also don't support complex and unicode objects because
      //  they might be not supported by Python.
      continue;
    }
    // Add the object to dict
    boDebug() << k_funcinfo << "Saving object '" << PyString_AsString(key) << "'" << endl;
    PyDict_SetItem(variables, key, value);
  }

  // Add variables and submodules to maindict
  PyDict_SetItemString(maindict, (char*)"__BO_variables", variables);
  PyDict_SetItemString(maindict, (char*)"__BO_submodules", submodules);

  boDebug() << k_funcinfo << "END" << endl;
  return maindict;
}

void PythonScript::loadModule(PyObject* module, PyObject* data)
{
  boDebug() << k_funcinfo << endl;
  // Find dictionary of the module
  PyObject* moduledict = PyModule_GetDict(module);

  // Get variable and submodule dicts
  PyObject* variables = PyDict_GetItemString(data, (char*)"__BO_variables");
  PyObject* submodules = PyDict_GetItemString(data, (char*)"__BO_submodules");

  // Make sure the dicts are valid
  if(!variables)
  {
    boError() << k_funcinfo << "Couldn't find variables dict!" << endl;
    return;
  }
  if(!submodules)
  {
    boError() << k_funcinfo << "Couldn't find submodules dict!" << endl;
    return;
  }

  // Load the variables
  boDebug() << k_funcinfo << "Loading and merging " << PyDict_Size(variables) << " variables" << endl;
  PyDict_Merge(moduledict, variables, true);

  // Load the submodules
  boDebug() << k_funcinfo << "Loading and merging " << PyDict_Size(submodules) << " submodules" << endl;
  PyObject* key;
  PyObject* value;
  int pos = 0;
  while(PyDict_Next(submodules, &pos, &key, &value))
  {
    // Check if current module already has module with this name
    PyObject* m = PyDict_GetItem(moduledict, key);
    if(m)
    {
      if(!PyModule_Check(m))
      {
        boError() << k_funcinfo << "Parent module has non-module item '" << PyString_AsString(key) << "'! Can't load module with same name!" << endl;
        continue;
      }
    }
    else
    {
      // Create new module
      m = PyModule_New(PyString_AsString(key));
      PyDict_SetItem(moduledict, key, m);
    }
    // Load the module
    boDebug() << k_funcinfo << "Loading module '" << PyString_AsString(key) << "'" << endl;
    loadModule(m, value);
  }

  boDebug() << k_funcinfo << "END" << endl;
}

bool PythonScript::load(QDataStream& stream)
{
  // TODO: make sure no script is loaded

  stream >> mLoadedScripts;
  char* data;
  unsigned int datalen;
  stream.readBytes(data, datalen);

  // Load dict object from data
  getPythonLock();
  CHECK_PYTHON_ERROR;
  PyObject* maindict = PyMarshal_ReadObjectFromString(data, datalen);
  if(!maindict)
  {
    boError() << k_funcinfo << "Could not load main dict!" << endl;
    return false;
  }
  // loadScriptFromString() also acquires the python lock, so we must free it
  //  here and re-acquire it later.
  freePythonLock();

  // Load script
  loadScriptFromString(mLoadedScripts);

  // Load data
  getPythonLock();
  loadModule(mMainModule, maindict);
  CHECK_PYTHON_ERROR;
  freePythonLock();

  delete[] data;

  boDebug() << k_funcinfo << "END" << endl;
  return true;
}


/*****  Event functions  *****/
PyObject* PythonScript::py_addEventHandler(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  char* eventname = 0;
  char* funcname = 0;
  char* funcargs = 0;
  if(!PyArg_ParseTuple(args, (char*)"sss", &eventname, &funcname, &funcargs))
  {
    return 0;
  }

  int id = currentScript()->addEventHandler(eventname, funcname, funcargs);

  return Py_BuildValue((char*)"i", id);
}

PyObject* PythonScript::py_removeEventHandler(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  currentScript()->removeEventHandler(id);

  Py_INCREF(Py_None);
  return Py_None;
}

/*****  Player functions  *****/
PyObject* PythonScript::py_areEnemies(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id1, id2;
  if(!PyArg_ParseTuple(args, (char*)"ii", &id1, &id2))
  {
    return 0;
  }

  bool enemies = currentScript()->areEnemies(id1, id2);

  return Py_BuildValue((char*)"i", enemies ? 1 : 0);
}

PyObject* PythonScript::py_isEnemy(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  bool enemy = currentScript()->isEnemy(id);

  return Py_BuildValue((char*)"i", enemy ? 1 : 0);
}

PyObject* PythonScript::py_allGamePlayers(PyObject*, PyObject*)
{
  BO_CHECK_NULL_RET0(currentScript());
  QValueList<int> players = currentScript()->allGamePlayers();

  return QValueListToPyList(&players);
}

PyObject* PythonScript::py_isNeutral(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", currentScript()->isNeutral(id) ? 1 : 0);
}

PyObject* PythonScript::py_powerGenerated(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", currentScript()->powerGenerated(id));
}

PyObject* PythonScript::py_powerConsumed(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", currentScript()->powerConsumed(id));
}

PyObject* PythonScript::py_powerGeneratedAfterConstructions(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", currentScript()->powerGeneratedAfterConstructions(id));
}

PyObject* PythonScript::py_powerConsumedAfterConstructions(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", currentScript()->powerConsumedAfterConstructions(id));
}

PyObject* PythonScript::py_isCellFogged(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int player, x, y;
  if(!PyArg_ParseTuple(args, (char*)"iii", &player, &x, &y))
  {
    return 0;
  }

  bool fogged = currentScript()->isCellFogged(player, x, y);
  return Py_BuildValue((char*)"i", fogged ? 1 : 0);
}


/*****  Resource functions  *****/
PyObject* PythonScript::py_minerals(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", (int)(currentScript()->minerals(id)));
}

PyObject* PythonScript::py_addMinerals(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id, amount;
  if(!PyArg_ParseTuple(args, (char*)"ii", &id, &amount))
  {
    return 0;
  }

  currentScript()->addMinerals(id, amount);

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_oil(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", (int)(currentScript()->oil(id)));
}

PyObject* PythonScript::py_addOil(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id, amount;
  if(!PyArg_ParseTuple(args, (char*)"ii", &id, &amount))
  {
    return 0;
  }

  currentScript()->addOil(id, amount);

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_nearestMineralLocations(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int x, y, n, radius;
  if(!PyArg_ParseTuple(args, (char*)"iiii", &x, &y, &n, &radius))
  {
    return 0;
  }

  QValueList<BoVector2Fixed> locations = currentScript()->nearestMineralLocations(x, y, n, radius);

  PyObject* pylist = PyList_New(locations.count());

  int i = 0;
  QValueList<BoVector2Fixed>::Iterator it;
  for(it = locations.begin(); it != locations.end(); ++it)
  {
    // We use tuples for positions
    PyObject* pos = PyTuple_New(2);
    PyTuple_SetItem(pos, 0, PyFloat_FromDouble((float)(*it).x()));
    PyTuple_SetItem(pos, 1, PyFloat_FromDouble((float)(*it).y()));
    PyList_SetItem(pylist, i, pos);
    i++;
  }

  return pylist;
}

PyObject* PythonScript::py_nearestOilLocations(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int x, y, n, radius;
  if(!PyArg_ParseTuple(args, (char*)"iiii", &x, &y, &n, &radius))
  {
    return 0;
  }

  QValueList<BoVector2Fixed> locations = currentScript()->nearestOilLocations(x, y, n, radius);

  PyObject* pylist = PyList_New(locations.count());

  int i = 0;
  QValueList<BoVector2Fixed>::Iterator it;
  for(it = locations.begin(); it != locations.end(); ++it)
  {
    // We use tuples for positions
    PyObject* pos = PyTuple_New(2);
    PyTuple_SetItem(pos, 0, PyFloat_FromDouble((float)(*it).x()));
    PyTuple_SetItem(pos, 1, PyFloat_FromDouble((float)(*it).y()));
    PyList_SetItem(pylist, i, pos);
    i++;
  }

  return pylist;
}


/*****  Unit functions  *****/
PyObject* PythonScript::py_moveUnit(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id;
  float x, y;
  if(!PyArg_ParseTuple(args, (char*)"iff", &id, &x, &y))
  {
    return 0;
  }

  currentScript()->moveUnit(id, x, y);

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_moveUnitWithAttacking(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id;
  float x, y;
  if(!PyArg_ParseTuple(args, (char*)"iff", &id, &x, &y))
  {
    return 0;
  }

  currentScript()->moveUnitWithAttacking(id, x, y);

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_attack(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id, target;
  if(!PyArg_ParseTuple(args, (char*)"ii", &id, &target))
  {
    return 0;
  }

  currentScript()->attack(id, target);

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_stopUnit(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  currentScript()->stopUnit(id);

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_mineUnit(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id;
  float x, y;
  if(!PyArg_ParseTuple(args, (char*)"iff", &id, &x, &y))
  {
    return 0;
  }

  currentScript()->mineUnit(id, x, y);

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_setUnitRotation(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id;
  float rot;
  if(!PyArg_ParseTuple(args, (char*)"if", &id, &rot))
  {
    return 0;
  }

  currentScript()->setUnitRotation(id, rot);

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_dropBomb(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id, weapon;
  float x, y;
  if(!PyArg_ParseTuple(args, (char*)"iiff", &id, &weapon, &x, &y))
  {
    return 0;
  }

  currentScript()->dropBomb(id, weapon, x, y);

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_produceUnit(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int factory, production;
  if(!PyArg_ParseTuple(args, (char*)"ii", &factory, &production))
  {
    return 0;
  }

  currentScript()->produceUnit(factory, production);

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_spawnUnit(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int type;
  float x, y;
  if(!PyArg_ParseTuple(args, (char*)"iff", &type, &x, &y))
  {
    return 0;
  }

  currentScript()->spawnUnit(type, x, y);

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_teleportUnit(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id;
  float x, y;
  if(!PyArg_ParseTuple(args, (char*)"iff", &id, &x, &y))
  {
    return 0;
  }

  currentScript()->teleportUnit(id, x, y);

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_canPlaceProductionAt(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int factoryid, unitType;
  float x, y;
  if(!PyArg_ParseTuple(args, (char*)"iiff", &factoryid, &unitType, &x, &y))
  {
    return 0;
  }

  bool can = currentScript()->canPlaceProductionAt(factoryid, unitType, x, y);

  return Py_BuildValue((char*)"i", can ? 1 : 0);
}

PyObject* PythonScript::py_placeProduction(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int factoryid;
  float x, y;
  if(!PyArg_ParseTuple(args, (char*)"iff", &factoryid, &x, &y))
  {
    return 0;
  }

  currentScript()->placeProduction(factoryid, x, y);

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_unitsOnCell(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int x, y;
  if(!PyArg_ParseTuple(args, (char*)"ii", &x, &y))
  {
    return 0;
  }

  QValueList<int> units = currentScript()->unitsOnCell(x, y);

  return QValueListToPyList(&units);
}

PyObject* PythonScript::py_unitsInRect(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int x1, y1, x2, y2;
  if(!PyArg_ParseTuple(args, (char*)"iiii", &x1, &y1, &x2, &y2))
  {
    return 0;
  }

  QValueList<int> units = currentScript()->unitsInRect(x1, y1, x2, y2);

  return QValueListToPyList(&units);
}

PyObject* PythonScript::py_cellOccupied(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int x, y;
  if(!PyArg_ParseTuple(args, (char*)"ii", &x, &y))
  {
    return 0;
  }

  bool occupied = currentScript()->cellOccupied(x, y);

  return Py_BuildValue((char*)"i", occupied ? 1 : 0);
}

PyObject* PythonScript::py_unitPosition(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  BoVector2Fixed pos = currentScript()->unitPosition(id);

  return Py_BuildValue((char*)"(ff)", (float)pos.x(), (float)pos.y());
}

PyObject* PythonScript::py_unitOwner(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", currentScript()->unitOwner(id));
}

PyObject* PythonScript::py_unitType(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", currentScript()->unitType(id));
}

PyObject* PythonScript::py_unitAdvanceWork(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", currentScript()->unitAdvanceWork(id));
}

PyObject* PythonScript::py_unitSightRange(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", currentScript()->unitSightRange(id));
}

PyObject* PythonScript::py_isUnitMobile(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", currentScript()->isUnitMobile(id) ? 1 : 0);
}

PyObject* PythonScript::py_isUnitTypeMobile(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int playerid, type;
  if(!PyArg_ParseTuple(args, (char*)"ii", &playerid, &type))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", currentScript()->isUnitTypeMobile(playerid, type) ? 1 : 0);
}

PyObject* PythonScript::py_isUnitAircraft(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", currentScript()->isUnitAircraft(id) ? 1 : 0);
}

PyObject* PythonScript::py_isUnitTypeAircraft(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int playerid, type;
  if(!PyArg_ParseTuple(args, (char*)"ii", &playerid, &type))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", currentScript()->isUnitTypeAircraft(playerid, type) ? 1 : 0);
}

PyObject* PythonScript::py_canUnitShoot(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", currentScript()->canUnitShoot(id) ? 1 : 0);
}

PyObject* PythonScript::py_canUnitTypeShoot(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int playerid, type;
  if(!PyArg_ParseTuple(args, (char*)"ii", &playerid, &type))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", currentScript()->canUnitTypeShoot(playerid, type) ? 1 : 0);
}

PyObject* PythonScript::py_canUnitProduce(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", currentScript()->canUnitProduce(id) ? 1 : 0);
}

PyObject* PythonScript::py_hasUnitCompletedProduction(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", currentScript()->hasUnitCompletedProduction(id) ? 1 : 0);
}

PyObject* PythonScript::py_completedProductionType(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", currentScript()->completedProductionType(id));
}

PyObject* PythonScript::py_canUnitMineMinerals(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", currentScript()->canUnitMineMinerals(id) ? 1 : 0);
}

PyObject* PythonScript::py_canUnitTypeMineMinerals(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int playerid, type;
  if(!PyArg_ParseTuple(args, (char*)"ii", &playerid, &type))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", currentScript()->canUnitTypeMineMinerals(playerid, type) ? 1 : 0);
}

PyObject* PythonScript::py_canUnitMineOil(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", currentScript()->canUnitMineOil(id) ? 1 : 0);
}

PyObject* PythonScript::py_canUnitTypeMineOil(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int playerid, type;
  if(!PyArg_ParseTuple(args, (char*)"ii", &playerid, &type))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", currentScript()->canUnitTypeMineOil(playerid, type) ? 1 : 0);
}

PyObject* PythonScript::py_productionTypes(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  QValueList<int> list = currentScript()->productionTypes(id);

  return QValueListToPyList(&list);
}

PyObject* PythonScript::py_isUnitAlive(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", currentScript()->isUnitAlive(id) ? 1 : 0);
}

PyObject* PythonScript::py_allPlayerUnits(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  // FIXME: current implementation of methods returning arrays is quite
  //  ineffiecient. First we usually get a list of units, then add ids of those
  //  units to id list and then covert id list to Python list. We could add ids
  //  directly to Python list.
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  QValueList<int> units = currentScript()->allPlayerUnits(id);

  return QValueListToPyList(&units);
}

PyObject* PythonScript::py_allPlayerUnitsCount(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", currentScript()->allPlayerUnitsCount(id));
}

PyObject* PythonScript::py_playerUnitsOfType(PyObject*, PyObject* args)
{
  int id, type;
  if(!PyArg_ParseTuple(args, (char*)"ii", &id, &type))
  {
    return 0;
  }

  QValueList<int> units = currentScript()->playerUnitsOfType(id, type);

  return QValueListToPyList(&units);
}

PyObject* PythonScript::py_playerUnitsOfTypeCount(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id, type;
  if(!PyArg_ParseTuple(args, (char*)"ii", &id, &type))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", currentScript()->playerUnitsOfTypeCount(id, type));
}

PyObject* PythonScript::py_allUnitsVisibleFor(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  // FIXME: current implementation of methods returning arrays is quite
  //  ineffiecient. First we usually get a list of units, then add ids of those
  //  units to id list and then covert id list to Python list. We could add ids
  //  directly to Python list.
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  QValueList<int> units = currentScript()->allUnitsVisibleFor(id);

  return QValueListToPyList(&units);
}

PyObject* PythonScript::py_allEnemyUnitsVisibleFor(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  // FIXME: current implementation of methods returning arrays is quite
  //  ineffiecient. First we usually get a list of units, then add ids of those
  //  units to id list and then covert id list to Python list. We could add ids
  //  directly to Python list.
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  QValueList<int> units = currentScript()->allEnemyUnitsVisibleFor(id);

  return QValueListToPyList(&units);
}



/*****  Camera functions  *****/

PyObject* PythonScript::py_setCameraRotation(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  float r;
  if(!PyArg_ParseTuple(args, (char*)"f", &r))
  {
    return 0;
  }
  currentScript()->setCameraRotation(r);
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_setCameraXRotation(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  float r;
  if(!PyArg_ParseTuple(args, (char*)"f", &r))
  {
    return 0;
  }
  currentScript()->setCameraXRotation(r);
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_setCameraDistance(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  float dist;
  if(!PyArg_ParseTuple(args, (char*)"f", &dist))
  {
    return 0;
  }
  currentScript()->setCameraDistance(dist);
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_setCameraMoveMode(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int m;
  if(!PyArg_ParseTuple(args, (char*)"i", &m))
  {
    return 0;
  }
  currentScript()->setCameraMoveMode(m);
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_setCameraInterpolationMode(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int m;
  if(!PyArg_ParseTuple(args, (char*)"i", &m))
  {
    return 0;
  }
  currentScript()->setCameraInterpolationMode(m);
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_setCameraLookAt(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  float x, y, z;
  if(!PyArg_ParseTuple(args, (char*)"fff", &x, &y, &z))
  {
    return 0;
  }
  currentScript()->setCameraLookAt(BoVector3Float(x, y, z));
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_setCameraPos(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  float x, y, z;
  if(!PyArg_ParseTuple(args, (char*)"fff", &x, &y, &z))
  {
    return 0;
  }
  currentScript()->setCameraPos(BoVector3Float(x, y, z));
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_setCameraUp(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  float x, y, z;
  if(!PyArg_ParseTuple(args, (char*)"fff", &x, &y, &z))
  {
    return 0;
  }
  currentScript()->setCameraUp(BoVector3Float(x, y, z));
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_setCameraLimits(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int on;
  if(!PyArg_ParseTuple(args, (char*)"i", &on))
  {
    return 0;
  }
  currentScript()->setCameraLimits((bool)on);
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_setCameraFreeMode(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int on;
  if(!PyArg_ParseTuple(args, (char*)"i", &on))
  {
    return 0;
  }
  currentScript()->setCameraFreeMode((bool)on);
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_addCameraLookAtPoint(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  float x, y, z, time;
  if(!PyArg_ParseTuple(args, (char*)"ffff", &x, &y, &z, &time))
  {
    return 0;
  }
  currentScript()->addCameraLookAtPoint(BoVector3Float(x, y, z), time);
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_addCameraUpPoint(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  float x, y, z, time;
  if(!PyArg_ParseTuple(args, (char*)"ffff", &x, &y, &z, &time))
  {
    return 0;
  }
  currentScript()->addCameraUpPoint(BoVector3Float(x, y, z), time);
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_addCameraPosPoint(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  float x, y, z, time;
  if(!PyArg_ParseTuple(args, (char*)"ffff", &x, &y, &z, &time))
  {
    return 0;
  }
  currentScript()->addCameraPosPoint(BoVector3Float(x, y, z), time);
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_commitCameraChanges(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int ticks;
  if(!PyArg_ParseTuple(args, (char*)"i", &ticks))
  {
    return 0;
  }
  currentScript()->commitCameraChanges(ticks);
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_cameraLookAt(PyObject*, PyObject*)
{
  BO_CHECK_NULL_RET0(currentScript());
  BoVector3Float pos = currentScript()->cameraLookAt();
  return Py_BuildValue((char*)"[f, f, f]", pos.x(), pos.y(), pos.z());
}

PyObject* PythonScript::py_cameraPos(PyObject*, PyObject*)
{
  BO_CHECK_NULL_RET0(currentScript());
  BoVector3Float pos = currentScript()->cameraPos();
  return Py_BuildValue((char*)"[f, f, f]", pos.x(), pos.y(), pos.z());
}

PyObject* PythonScript::py_cameraUp(PyObject*, PyObject*)
{
  BO_CHECK_NULL_RET0(currentScript());
  BoVector3Float pos = currentScript()->cameraUp();
  return Py_BuildValue((char*)"[f, f, f]", pos.x(), pos.y(), pos.z());
}

PyObject* PythonScript::py_cameraRotation(PyObject*, PyObject*)
{
  BO_CHECK_NULL_RET0(currentScript());
  return Py_BuildValue((char*)"f", currentScript()->cameraRotation());
}

PyObject* PythonScript::py_cameraXRotation(PyObject*, PyObject*)
{
  BO_CHECK_NULL_RET0(currentScript());
  return Py_BuildValue((char*)"f", currentScript()->cameraXRotation());
}

PyObject* PythonScript::py_cameraDistance(PyObject*, PyObject*)
{
  BO_CHECK_NULL_RET0(currentScript());
  return Py_BuildValue((char*)"f", currentScript()->cameraDistance());
}


/*****  Light functions  *****/
PyObject* PythonScript::py_lightPos(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  BoVector4Float pos = currentScript()->lightPos(id);
  return Py_BuildValue((char*)"(ffff)", pos.x(), pos.y(), pos.z(), pos.w());
}

PyObject* PythonScript::py_lightAmbient(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  BoVector4Float a = currentScript()->lightAmbient(id);
  return Py_BuildValue((char*)"(ffff)", a.x(), a.y(), a.z(), a.w());
}

PyObject* PythonScript::py_lightDiffuse(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  BoVector4Float d = currentScript()->lightDiffuse(id);
  return Py_BuildValue((char*)"(ffff)", d.x(), d.y(), d.z(), d.w());
}

PyObject* PythonScript::py_lightSpecular(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  BoVector4Float s = currentScript()->lightSpecular(id);
  return Py_BuildValue((char*)"(ffff)", s.x(), s.y(), s.z(), s.w());
}

PyObject* PythonScript::py_lightAttenuation(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  BoVector3Float a = currentScript()->lightAttenuation(id);
  return Py_BuildValue((char*)"(fff)", a.x(), a.y(), a.z());
}

PyObject* PythonScript::py_lightEnabled(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", currentScript()->lightEnabled(id) ? 1 : 0);
}


PyObject* PythonScript::py_setLightPos(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id;
  float x, y, z, w;
  if(!PyArg_ParseTuple(args, (char*)"i(ffff)", &id, &x, &y, &z, &w))
  {
    return 0;
  }

  currentScript()->setLightPos(id, BoVector4Float(x, y, z, w));

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_setLightAmbient(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id;
  float r, g, b, a;
  if(!PyArg_ParseTuple(args, (char*)"i(ffff)", &id, &r, &g, &b, &a))
  {
    return 0;
  }

  currentScript()->setLightAmbient(id, BoVector4Float(r, g, b, a));

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_setLightDiffuse(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id;
  float r, g, b, a;
  if(!PyArg_ParseTuple(args, (char*)"i(ffff)", &id, &r, &g, &b, &a))
  {
    return 0;
  }

  currentScript()->setLightDiffuse(id, BoVector4Float(r, g, b, a));

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_setLightSpecular(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id;
  float r, g, b, a;
  if(!PyArg_ParseTuple(args, (char*)"i(ffff)", &id, &r, &g, &b, &a))
  {
    return 0;
  }

  currentScript()->setLightSpecular(id, BoVector4Float(r, g, b, a));

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_setLightAttenuation(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id;
  float c, l, q;
  if(!PyArg_ParseTuple(args, (char*)"i(fff)", &id, &c, &l, &q))
  {
    return 0;
  }

  currentScript()->setLightAttenuation(id, BoVector3Float(c, l, q));

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_setLightEnabled(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id, enable;
  if(!PyArg_ParseTuple(args, (char*)"ii", &id, &enable))
  {
    return 0;
  }

  currentScript()->setLightEnabled(id, enable);

  Py_INCREF(Py_None);
  return Py_None;
}


PyObject* PythonScript::py_addLight(PyObject*, PyObject*)
{
  BO_CHECK_NULL_RET0(currentScript());
  return Py_BuildValue((char*)"i", currentScript()->addLight());
}

PyObject* PythonScript::py_removeLight(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  currentScript()->removeLight(id);
  Py_INCREF(Py_None);
  return Py_None;
}



/*****  AI functions  *****/
PyObject* PythonScript::py_aiDelay(PyObject*, PyObject*)
{
  BO_CHECK_NULL_RET0(currentScript());
  return Py_BuildValue((char*)"f", currentScript()->aiDelay());
}

/*****  Other functions  *****/
PyObject* PythonScript::py_startBenchmark(PyObject*, PyObject*)
{
  BO_CHECK_NULL_RET0(currentScript());
  currentScript()->startBenchmark();
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_endBenchmark(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  char* name = 0;
  if(!PyArg_ParseTuple(args, (char*)"|s", &name))
  {
    return 0;
  }
  currentScript()->endBenchmark(QString(name));
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_setRandomSeed(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int seed;
  if(!PyArg_ParseTuple(args, (char*)"i", &seed))
  {
    return 0;
  }
  currentScript()->setRandomSeed(seed);
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_findPath(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int x1, y1, x2, y2;
  if(!PyArg_ParseTuple(args, (char*)"iiii", &x1, &y1, &x2, &y2))
  {
    return 0;
  }
  currentScript()->findPath(x1, y1, x2, y2);
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_addEffect(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int id;
  float x, y, z, rot = 0.0f;
  if(!PyArg_ParseTuple(args, (char*)"i(fff)|f", &id, &x, &y, &z, &rot))
  {
    return 0;
  }
  currentScript()->addEffect(id, BoVector3Fixed(x, y, z), rot);
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_addEffectToUnit(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int unitid, effectid;
  float x = 0, y = 0, z = 0, zrot = 0;
  if(!PyArg_ParseTuple(args, (char*)"ii|(fff)f", &unitid, &effectid, &x, &y, &z, &zrot))
  {
    return 0;
  }
  currentScript()->addEffectToUnit(unitid, effectid, BoVector3Fixed(x, y, z), zrot);
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_advanceEffects(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int ticks;
  if(!PyArg_ParseTuple(args, (char*)"i", &ticks))
  {
    return 0;
  }
  currentScript()->advanceEffects(ticks);
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_wind(PyObject*, PyObject*)
{
  BO_CHECK_NULL_RET0(currentScript());
  BO_CHECK_NULL_RET0(currentScript());
  const BoVector3Float& wind = currentScript()->wind();
  return Py_BuildValue((char*)"[f, f, f]", wind.x(), wind.y(), wind.z());
}

PyObject* PythonScript::py_setWind(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  float x = 0, y = 0, z = 0;
  if(!PyArg_ParseTuple(args, (char*)"(fff)", &x, &y, &z))
  {
    return 0;
  }
  currentScript()->setWind(BoVector3Float(x, y, z));
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_explorePlayer(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int playerid;
  if(!PyArg_ParseTuple(args, (char*)"i", &playerid))
  {
    return 0;
  }
  currentScript()->explorePlayer(playerid);
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_exploreAllPlayers(PyObject*, PyObject*)
{
  BO_CHECK_NULL_RET0(currentScript());
  currentScript()->exploreAllPlayers();
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_unfogPlayer(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int playerid;
  if(!PyArg_ParseTuple(args, (char*)"i", &playerid))
  {
    return 0;
  }
  currentScript()->unfogPlayer(playerid);
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_unfogAllPlayers(PyObject*, PyObject*)
{
  BO_CHECK_NULL_RET0(currentScript());
  currentScript()->unfogAllPlayers();
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_fogPlayer(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int playerid;
  if(!PyArg_ParseTuple(args, (char*)"i", &playerid))
  {
    return 0;
  }
  currentScript()->fogPlayer(playerid);
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_fogAllPlayers(PyObject*, PyObject*)
{
  BO_CHECK_NULL_RET0(currentScript());
  currentScript()->fogAllPlayers();
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_setAcceptUserInput(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  int accept;
  if(!PyArg_ParseTuple(args, (char*)"i", &accept))
  {
    return 0;
  }

  currentScript()->setAcceptUserInput(accept);

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_addChatMessage(PyObject* self, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  char* from = 0;
  char* message = 0;
  if(!PyArg_ParseTuple(args, (char*)"ss", &from, &message))
  {
    return 0;
  }
  currentScript()->addChatMessage(QString(from), QString(message));
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_mapWidth(PyObject* self, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  return Py_BuildValue((char*)"i", currentScript()->mapWidth());
}

PyObject* PythonScript::py_mapHeight(PyObject* self, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  return Py_BuildValue((char*)"i", currentScript()->mapHeight());
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


/*
 * vim: et sw=2
 */
