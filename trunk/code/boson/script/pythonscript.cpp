/*
    This file is part of the Boson game
    Copyright (C) 2003-2004 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include <marshal.h>
#include <compile.h>

#include <qstring.h>
#include <qfileinfo.h>
#include <qstringlist.h>

#include "bodebug.h"
#include "../bo3dtools.h"
#include "../boevent.h"


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
  { (char*)"isUnitAircraft", py_isUnitAircraft, METH_VARARGS, 0 },
  { (char*)"canUnitShoot", py_canUnitShoot, METH_VARARGS, 0 },
  { (char*)"canUnitTypeShoot", py_canUnitTypeShoot, METH_VARARGS, 0 },
  { (char*)"canUnitProduce", py_canUnitProduce, METH_VARARGS, 0 },
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
  { (char*)"addEffect", py_addEffect, METH_VARARGS, 0 },
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

  mMainModule = 0;
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

  mMainModule = PyImport_AddModule((char*)"__main__");
  mDict = PyModule_GetDict(mMainModule);

  QString code;
  code += "import sys\n";
  code += QString("sys.path.insert(0, '%1')\n").arg(BosonScript::scriptsPath());
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

  mLoadedScripts += string;
  mLoadedScripts += '\n';

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
    PyArg_Parse(pValue, (char*)"i", &ret);
    Py_DECREF(pValue);
  }

  freePythonLock();
  return ret;
}

void PythonScript::execLine(const QString& line)
{
  boDebug(700) << k_funcinfo << "line: " << line << endl;
  getPythonLock();

  PyRun_SimpleString((char*)line.ascii());

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

void PythonScript::setPlayerId(int id)
{
  PyObject* args = PyTuple_New(1);
  PyTuple_SetItem(args, 0, PyInt_FromLong(id));
  callFunction("setPlayerId", args);
}

bool PythonScript::save(QDataStream& stream)
{
  boDebug() << k_funcinfo << endl;

  getPythonLock();

  // Save main module to a PyObject
  PyObject* savedmodule = saveModule(mMainModule);

  // Save all items in savedict
  PyObject* data = PyMarshal_WriteObjectToString(savedmodule);
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
    if((strncmp(PyString_AsString(key), "__", 2) == 0) || (strcmp(PyString_AsString(key), "sys") == 0))
    {
      // Skip everything with name "__*" and "sys"
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
      PyDict_SetItem(submodules, key, saveModule(value));
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
  freePythonLock();

  delete[] data;

  boDebug() << k_funcinfo << "END" << endl;
  return true;
}


/*****  Event functions  *****/
PyObject* PythonScript::py_addEventHandler(PyObject*, PyObject* args)
{
  char* eventname = 0;
  char* funcname = 0;
  char* funcargs = 0;
  if(!PyArg_ParseTuple(args, (char*)"sss", &eventname, &funcname, &funcargs))
  {
    return 0;
  }

  int id = BosonScript::currentScript()->addEventHandler(eventname, funcname, funcargs);

  return Py_BuildValue((char*)"i", id);
}

PyObject* PythonScript::py_removeEventHandler(PyObject*, PyObject* args)
{
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  BosonScript::currentScript()->removeEventHandler(id);

  Py_INCREF(Py_None);
  return Py_None;
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

  QValueList<BoVector2Fixed> locations = BosonScript::nearestMineralLocations(player, x, y, n, radius);

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
  int player, x, y, n, radius;
  if(!PyArg_ParseTuple(args, (char*)"iiiii", &player, &x, &y, &n, &radius))
  {
    return 0;
  }

  QValueList<BoVector2Fixed> locations = BosonScript::nearestOilLocations(player, x, y, n, radius);

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
  int player, id;
  float x, y;
  if(!PyArg_ParseTuple(args, (char*)"iiff", &player, &id, &x, &y))
  {
    return 0;
  }

  BosonScript::moveUnit(player, id, x, y);

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_moveUnitWithAttacking(PyObject*, PyObject* args)
{
  int player, id;
  float x, y;
  if(!PyArg_ParseTuple(args, (char*)"iiff", &player, &id, &x, &y))
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
  int player, id;
  float x, y;
  if(!PyArg_ParseTuple(args, (char*)"iiff", &player, &id, &x, &y))
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
  int player, id, weapon;
  float x, y;
  if(!PyArg_ParseTuple(args, (char*)"iiiff", &player, &id, &weapon, &x, &y))
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
  int player, type;
  float x, y;
  if(!PyArg_ParseTuple(args, (char*)"iiff", &player, &type, &x, &y))
  {
    return 0;
  }

  BosonScript::spawnUnit(player, type, x, y);

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_teleportUnit(PyObject*, PyObject* args)
{
  int player, id;
  float x, y;
  if(!PyArg_ParseTuple(args, (char*)"iiff", &player, &id, &x, &y))
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

  BoVector2Fixed pos = BosonScript::unitPosition(id);

  return Py_BuildValue((char*)"(ff)", (float)pos.x(), (float)pos.y());
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

PyObject* PythonScript::py_isUnitAircraft(PyObject*, PyObject* args)
{
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", BosonScript::isUnitAircraft(id) ? 1 : 0);
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

PyObject* PythonScript::py_canUnitTypeShoot(PyObject*, PyObject* args)
{
  int playerid, type;
  if(!PyArg_ParseTuple(args, (char*)"ii", &playerid, &type))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", BosonScript::canUnitTypeShoot(playerid, type) ? 1 : 0);
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

PyObject* PythonScript::py_canUnitMineMinerals(PyObject*, PyObject* args)
{
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", BosonScript::canUnitMineMinerals(id) ? 1 : 0);
}

PyObject* PythonScript::py_canUnitTypeMineMinerals(PyObject*, PyObject* args)
{
  int playerid, type;
  if(!PyArg_ParseTuple(args, (char*)"ii", &playerid, &type))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", BosonScript::canUnitTypeMineMinerals(playerid, type) ? 1 : 0);
}

PyObject* PythonScript::py_canUnitMineOil(PyObject*, PyObject* args)
{
  int id;
  if(!PyArg_ParseTuple(args, (char*)"i", &id))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", BosonScript::canUnitMineOil(id) ? 1 : 0);
}

PyObject* PythonScript::py_canUnitTypeMineOil(PyObject*, PyObject* args)
{
  int playerid, type;
  if(!PyArg_ParseTuple(args, (char*)"ii", &playerid, &type))
  {
    return 0;
  }

  return Py_BuildValue((char*)"i", BosonScript::canUnitTypeMineOil(playerid, type) ? 1 : 0);
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

PyObject* PythonScript::py_setCameraRadius(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  float r;
  if(!PyArg_ParseTuple(args, (char*)"f", &r))
  {
    return 0;
  }
  currentScript()->setCameraRadius(r);
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* PythonScript::py_setCameraZ(PyObject*, PyObject* args)
{
  BO_CHECK_NULL_RET0(currentScript());
  float z;
  if(!PyArg_ParseTuple(args, (char*)"f", &z))
  {
    return 0;
  }
  currentScript()->setCameraZ(z);
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

PyObject* PythonScript::py_cameraRadius(PyObject*, PyObject*)
{
  BO_CHECK_NULL_RET0(currentScript());
  return Py_BuildValue((char*)"f", currentScript()->cameraRadius());
}

PyObject* PythonScript::py_cameraZ(PyObject*, PyObject*)
{
  BO_CHECK_NULL_RET0(currentScript());
  return Py_BuildValue((char*)"f", currentScript()->cameraZ());
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

PyObject* PythonScript::py_addEffect(PyObject*, PyObject* args)
{
  int id;
  float x, y, z, rot = 0.0f;
  if(!PyArg_ParseTuple(args, (char*)"i(fff)|f", &id, &x, &y, &z, &rot))
  {
    return 0;
  }
  BosonScript::addEffect(id, BoVector3Fixed(x, y, z), rot);
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


/*
 * vim: et sw=2
 */
