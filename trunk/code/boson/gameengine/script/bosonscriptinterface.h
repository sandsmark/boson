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

#ifndef BOSONSCRIPTINTERFACE_H
#define BOSONSCRIPTINTERFACE_H

class BosonBigDisplayBase;
class Player;
class BoGameCamera;
class BoAutoGameCamera;
class BosonCanvas;
class Boson;
class BoLight;
class bofixed;
template<class T> class BoVector3;
template<class T> class BoVector4;
typedef BoVector3<bofixed> BoVector3Fixed;
typedef BoVector3<float> BoVector3Float;
typedef BoVector4<bofixed> BoVector4Fixed;
typedef BoVector4<float> BoVector4Float;

class QString;
class QDataStream;
class QPoint;

template<class T> class QValueList;

#include <qobject.h>

/**
 * Interface class between @ref BosonScript and the rest of the game. @ref
 * BosonScript owns an object of this class and can call methods in it. This
 * class then emits a signal. If no slot is connected to the signal, nothing
 * happens (i.e. the method is not implemented for that @ref BosonScript
 * object). Otherwise the slot will execute the command.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonScriptInterface : public QObject
{
  Q_OBJECT
  public:
    BosonScriptInterface(QObject* parent = 0, const char* name = 0);
    ~BosonScriptInterface();

    /*  Events  */
    int addEventHandler(const QString& eventname, const QString& functionname, const QString& args);
    void removeEventHandler(int id);

    /*  Light  */
     /**
      * @return -1 on error, otherwise the ID of the new light. See @ref BoLight
      **/
    int addLight();
    void removeLight(int id);
    BoVector4Float lightPos(int id);
    BoVector4Float lightAmbient(int id);
    BoVector4Float lightDiffuse(int id);
    BoVector4Float lightSpecular(int id);
    BoVector3Float lightAttenuation(int id);
    bool lightEnabled(int id);
    void setLightPos(int id, const BoVector4Float&);
    void setLightAmbient(int id, const BoVector4Float&);
    void setLightDiffuse(int id, const BoVector4Float&);
    void setLightSpecular(int id, const BoVector4Float&);
    void setLightAttenuation(int id, const BoVector3Float&);
    void setLightEnabled(int id, bool);

    /*  Camera  */
    BoVector3Float cameraPos();
    BoVector3Float cameraUp();
    BoVector3Float cameraLookAt();
    float cameraRotation();
    float cameraXRotation();
    float cameraDistance();
    void setUseCameraLimits(bool on);
    void setCameraFreeMovement(bool on);

    /*  AutoCamera  */
    void setCameraRotation(float);
    void setCameraXRotation(float);
    void setCameraDistance(float);
    void setCameraMoveMode(int mode);
    void setCameraInterpolationMode(int mode);
    void setCameraPos(const BoVector3Float&);
    void setCameraLookAt(const BoVector3Float&);
    void setCameraUp(const BoVector3Float&);
    void addCameraLookAtPoint(const BoVector3Float& pos, float time);
    void addCameraPosPoint(const BoVector3Float& pos, float time);
    void addCameraUpPoint(const BoVector3Float& up, float time);
    void commitCameraChanges(int ticks);
    void setAcceptUserInput(bool accept);

    /*  Effects  */
    void addEffect(unsigned int id, const BoVector3Fixed& pos, bofixed zrot);
    void addEffectToUnit(int unitid, unsigned int effectid, BoVector3Fixed offset, bofixed zrot);
    void advanceEffects(int ticks);
    void setWind(const BoVector3Float& wind);
    BoVector3Float wind();

  signals:
    /*  Events  */
    void signalAddEventHandler(const QString& eventname, const QString& functionname, const QString& args, int* id);
    void signalRemoveEventHandler(int id);

    /*  Light  */
    void signalAddLight(int* id);
    void signalRemoveLight(int id);
    void signalGetLightPos(int id, BoVector4Float*);
    void signalGetLightAmbient(int id, BoVector4Float*);
    void signalGetLightDiffuse(int id, BoVector4Float*);
    void signalGetLightSpecular(int id, BoVector4Float*);
    void signalGetLightAttenuation(int id, BoVector3Float*);
    void signalGetLightEnabled(int id, bool*);
    void signalSetLightPos(int id, const BoVector4Float&);
    void signalSetLightAmbient(int id, const BoVector4Float&);
    void signalSetLightDiffuse(int id, const BoVector4Float&);
    void signalSetLightSpecular(int id, const BoVector4Float&);
    void signalSetLightAttenuation(int id, const BoVector3Float&);
    void signalSetLightEnabled(int id, bool);

    /*  Camera */
    void signalGetCameraPos(BoVector3Float*);
    void signalGetCameraUp(BoVector3Float*);
    void signalGetCameraLookAt(BoVector3Float*);
    void signalGetCameraRotation(float*);
    void signalGetCameraXRotation(float*);
    void signalGetCameraDistance(float*);
    void signalSetUseCameraLimits(bool);
    void signalSetCameraFreeMovement(bool);

    /*  AutoCamera */
    void signalSetCameraPos(const BoVector3Float&);
    void signalSetCameraLookAt(const BoVector3Float&);
    void signalSetCameraUp(const BoVector3Float&);
    void signalAddCameraLookAtPoint(const BoVector3Float&, float);
    void signalAddCameraPosPoint(const BoVector3Float&, float);
    void signalAddCameraUpPoint(const BoVector3Float&, float);
    void signalSetCameraRotation(float);
    void signalSetCameraXRotation(float);
    void signalSetCameraDistance(float);
    void signalSetCameraMoveMode(int);
    void signalSetCameraInterpolationMode(int);
    void signalCommitCameraChanges(int);
    void signalSetAcceptUserInput(bool);

    /*  Effects  */
    void signalAddEffect(unsigned int id, const BoVector3Fixed& pos, bofixed zrot);
    void signalAddEffectToUnit(int unitid, unsigned int effectid, BoVector3Fixed offset, bofixed zrot);
    void signalAdvanceEffects(int ticks);
    void signalSetWind(const BoVector3Float& wind);
    void signalGetWind(BoVector3Float*);
};

#endif

/*
 * vim: et sw=2
 */
