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
class QString;

class PythonScript : public BosonScript
{
  public:
    PythonScript();
    virtual ~PythonScript();

    virtual void loadScript(QString file);

    virtual void advance();
    virtual void init();

    virtual void callFunction(QString function);

    virtual void execLine(const QString& line);


    // Resources
    // Units
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


  private:
    static PyMethodDef mCallbacks[];
    static PyObject* mDict;
};

#endif //PYTHONSCRIPT_H
