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

#ifndef PYTHONSCRIPT_H
#define PYTHONSCRIPT_H

#include "bosonscript.h"

typedef struct _object PyObject;
struct PyMethodDef;
typedef struct _ts PyThreadState;
class QString;

class PythonScript : public BosonScript
{
  public:
    PythonScript(Player* p);
    virtual ~PythonScript();

    virtual void loadScript(QString file);

    virtual void advance();
    virtual void init();

    virtual void callFunction(QString function);
    virtual void callFunction(QString function, PyObject* args);

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
    static PyObject* py_isUnitMobile(PyObject* self, PyObject* args);
    static PyObject* py_canUnitShoot(PyObject* self, PyObject* args);
    static PyObject* py_canUnitProduce(PyObject* self, PyObject* args);
    static PyObject* py_productionTypes(PyObject* self, PyObject* args);

    static PyObject* py_isUnitAlive(PyObject* self, PyObject* args);

    static PyObject* py_allPlayerUnits(PyObject* self, PyObject* args);


    // Camera
    static PyObject* py_moveCamera(PyObject* self, PyObject* args);
    static PyObject* py_moveCameraBy(PyObject* self, PyObject* args);
    static PyObject* py_setCameraRotation(PyObject* self, PyObject* args);
    static PyObject* py_setCameraRadius(PyObject* self, PyObject* args);
    static PyObject* py_setCameraZ(PyObject* self, PyObject* args);
    static PyObject* py_setCameraMoveMode(PyObject* self, PyObject* args);

    static PyObject* py_commitCameraChanges(PyObject* self, PyObject* args);

    static PyObject* py_cameraPos(PyObject* self, PyObject* args);
    static PyObject* py_cameraRotation(PyObject* self, PyObject* args);
    static PyObject* py_cameraRadius(PyObject* self, PyObject* args);
    static PyObject* py_cameraZ(PyObject* self, PyObject* args);


    // AI
    static PyObject* py_aiDelay(PyObject* self, PyObject* args);


    // Other
    static PyObject* py_startBenchmark(PyObject* self, PyObject* args);
    static PyObject* py_endBenchmark(PyObject* self, PyObject* args);
    static PyObject* py_setRandomSeed(PyObject* self, PyObject* args);

  protected:
    static PyObject* QValueListToPyList(QValueList<int>* list);

    static void initScripting();
    static void uninitScripting();

    void getPythonLock();
    void freePythonLock();

  private:
    PyObject* mDict;
    PyThreadState* mInterpreter;

    static PyMethodDef mCallbacks[];
    static bool mScriptingInited;
    static int mScriptInstances;
    static PyThreadState* mThreadState;
};

#endif //PYTHONSCRIPT_H
