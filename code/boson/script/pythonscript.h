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

#ifndef PYTHONSCRIPT_H
#define PYTHONSCRIPT_H

#include "bosonscript.h"

#include <qstring.h>
#include <qdatastream.h>

typedef struct _object PyObject;
struct PyMethodDef;
typedef struct _ts PyThreadState;


class PythonScript : public BosonScript
{
  public:
    PythonScript(Player* p);
    virtual ~PythonScript();

    virtual void loadScript(QString file);
    virtual void loadScriptFromString(const QString& string);

    virtual void advance();
    virtual void init();

    virtual bool load(QDataStream& stream);
    virtual bool save(QDataStream& stream) const;

    virtual void callFunction(const QString& function);
    void callFunction(const QString& function, PyObject* args);
    virtual int callFunctionWithReturn(const QString& function);
    int callFunctionWithReturn(const QString& function, PyObject* args);

    virtual void execLine(const QString& line);


    // Players
    static PyObject* py_areEnemies(PyObject* self, PyObject* args);
    static PyObject* py_allPlayers(PyObject* self, PyObject* args);


    // Resources
    static PyObject* py_minerals(PyObject* self, PyObject* args);
    static PyObject* py_addMinerals(PyObject* self, PyObject* args);
    static PyObject* py_oil(PyObject* self, PyObject* args);
    static PyObject* py_addOil(PyObject* self, PyObject* args);
    static PyObject* py_nearestMineralLocations(PyObject* self, PyObject* args);
    static PyObject* py_nearestOilLocations(PyObject* self, PyObject* args);


    // Units
    static PyObject* py_moveUnit(PyObject* self, PyObject* args);
    static PyObject* py_moveUnitWithAttacking(PyObject* self, PyObject* args);
    static PyObject* py_attack(PyObject* self, PyObject* args);
    static PyObject* py_stopUnit(PyObject* self, PyObject* args);
    static PyObject* py_mineUnit(PyObject* self, PyObject* args);
    static PyObject* py_setUnitRotation(PyObject* self, PyObject* args);
    static PyObject* py_dropBomb(PyObject* self, PyObject* args);
    static PyObject* py_produceUnit(PyObject* self, PyObject* args);
    static PyObject* py_spawnUnit(PyObject* self, PyObject* args);
    static PyObject* py_teleportUnit(PyObject* self, PyObject* args);

    static PyObject* py_unitsOnCell(PyObject* self, PyObject* args);
    static PyObject* py_unitsInRect(PyObject* self, PyObject* args);
    static PyObject* py_cellOccupied(PyObject* self, PyObject* args);

    static PyObject* py_unitPosition(PyObject* self, PyObject* args);
    static PyObject* py_unitOwner(PyObject* self, PyObject* args);
    static PyObject* py_unitType(PyObject* self, PyObject* args);
    static PyObject* py_unitWork(PyObject* self, PyObject* args);
    static PyObject* py_isUnitMobile(PyObject* self, PyObject* args);
    static PyObject* py_canUnitShoot(PyObject* self, PyObject* args);
    static PyObject* py_canUnitTypeShoot(PyObject* self, PyObject* args);
    static PyObject* py_canUnitProduce(PyObject* self, PyObject* args);
    static PyObject* py_canUnitMineMinerals(PyObject* self, PyObject* args);
    static PyObject* py_canUnitTypeMineMinerals(PyObject* self, PyObject* args);
    static PyObject* py_canUnitMineOil(PyObject* self, PyObject* args);
    static PyObject* py_canUnitTypeMineOil(PyObject* self, PyObject* args);
    static PyObject* py_productionTypes(PyObject* self, PyObject* args);

    static PyObject* py_isUnitAlive(PyObject* self, PyObject* args);

    static PyObject* py_allPlayerUnits(PyObject* self, PyObject* args);
    static PyObject* py_allPlayerUnitsCount(PyObject* self, PyObject* args);
    static PyObject* py_playerUnitsOfType(PyObject* self, PyObject* args);
    static PyObject* py_playerUnitsOfTypeCount(PyObject* self, PyObject* args);


    // Camera
    static PyObject* py_setCameraRotation(PyObject* self, PyObject* args);
    static PyObject* py_setCameraRadius(PyObject* self, PyObject* args);
    static PyObject* py_setCameraZ(PyObject* self, PyObject* args);
    static PyObject* py_setCameraMoveMode(PyObject* self, PyObject* args);
    static PyObject* py_setCameraLookAt(PyObject* self, PyObject* args);
    static PyObject* py_setCameraPos(PyObject* self, PyObject* args);
    static PyObject* py_setCameraUp(PyObject* self, PyObject* args);
    static PyObject* py_setCameraLimits(PyObject* self, PyObject* args);
    static PyObject* py_setCameraFreeMode(PyObject* self, PyObject* args);

    static PyObject* py_commitCameraChanges(PyObject* self, PyObject* args);

    static PyObject* py_cameraLookAt(PyObject* self, PyObject* args);
    static PyObject* py_cameraPos(PyObject* self, PyObject* args);
    static PyObject* py_cameraUp(PyObject* self, PyObject* args);
    static PyObject* py_cameraRotation(PyObject* self, PyObject* args);
    static PyObject* py_cameraRadius(PyObject* self, PyObject* args);
    static PyObject* py_cameraZ(PyObject* self, PyObject* args);


    // Lights
    static PyObject* py_lightPos(PyObject* self, PyObject* args);
    static PyObject* py_lightAmbient(PyObject* self, PyObject* args);
    static PyObject* py_lightDiffuse(PyObject* self, PyObject* args);
    static PyObject* py_lightSpecular(PyObject* self, PyObject* args);
    static PyObject* py_lightAttenuation(PyObject* self, PyObject* args);
    static PyObject* py_lightEnabled(PyObject* self, PyObject* args);

    static PyObject* py_setLightPos(PyObject* self, PyObject* args);
    static PyObject* py_setLightAmbient(PyObject* self, PyObject* args);
    static PyObject* py_setLightDiffuse(PyObject* self, PyObject* args);
    static PyObject* py_setLightSpecular(PyObject* self, PyObject* args);
    static PyObject* py_setLightAttenuation(PyObject* self, PyObject* args);
    static PyObject* py_setLightEnabled(PyObject* self, PyObject* args);

    static PyObject* py_addLight(PyObject* self, PyObject* args);
    static PyObject* py_removeLight(PyObject* self, PyObject* args);


    // AI
    static PyObject* py_aiDelay(PyObject* self, PyObject* args);


    // Other
    static PyObject* py_startBenchmark(PyObject* self, PyObject* args);
    static PyObject* py_endBenchmark(PyObject* self, PyObject* args);
    static PyObject* py_setRandomSeed(PyObject* self, PyObject* args);
    static PyObject* py_findPath(PyObject* self, PyObject* args);
    static PyObject* py_addEffect(PyObject* self, PyObject* args);

  protected:
    static PyObject* QValueListToPyList(QValueList<int>* list);

    static void initScripting();
    static void uninitScripting();

    void getPythonLock();
    void freePythonLock();

  private:
    PyObject* mDict;
    PyThreadState* mInterpreter;
    QString mLoadedScripts;

    static PyMethodDef mCallbacks[];
    static bool mScriptingInited;
    static int mScriptInstances;
    static PyThreadState* mThreadState;
};

#endif //PYTHONSCRIPT_H
