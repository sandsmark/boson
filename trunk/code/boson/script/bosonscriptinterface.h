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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef BOSONSCRIPTINTERFACE_H
#define BOSONSCRIPTINTERFACE_H

class BoVector3;
class BosonBigDisplayBase;
class Player;
class BoGameCamera;
class BoAutoGameCamera;
class BosonCanvas;
class Boson;
class BoLight;
class BoVector4;

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

    /*  Light  */
     /**
      * @return -1 on error, otherwise the ID of the new light. See @ref BoLight
      **/
    int addLight();
    void removeLight(int id);
    BoVector4 lightPos(int id);
    BoVector4 lightAmbient(int id);
    BoVector4 lightDiffuse(int id);
    BoVector4 lightSpecular(int id);
    BoVector3 lightAttenuation(int id);
    bool lightEnabled(int id);
    void setLightPos(int id, const BoVector4&);
    void setLightAmbient(int id, const BoVector4&);
    void setLightDiffuse(int id, const BoVector4&);
    void setLightSpecular(int id, const BoVector4&);
    void setLightAttenuation(int id, const BoVector3&);
    void setLightEnabled(int id, bool);

    /*  Camera  */
    BoVector3 cameraPos();
    BoVector3 cameraUp();
    BoVector3 cameraLookAt();
    float cameraRotation();
    float cameraRadius();
    float cameraZ();
    void setUseCameraLimits(bool on);
    void setCameraFreeMovement(bool on);

    /*  AutoCamera  */
    void setCameraRotation(float);
    void setCameraRadius(float);
    void setCameraZ(float);
    void setCameraMoveMode(int mode);
    void setCameraPos(const BoVector3&);
    void setCameraLookAt(const BoVector3&);
    void setCameraUp(const BoVector3&);
    void commitCameraChanges(int ticks);


  signals:
    /*  Light  */
    void signalAddLight(int* id);
    void signalRemoveLight(int id);
    void signalGetLightPos(int id, BoVector4*);
    void signalGetLightAmbient(int id, BoVector4*);
    void signalGetLightDiffuse(int id, BoVector4*);
    void signalGetLightSpecular(int id, BoVector4*);
    void signalGetLightAttenuation(int id, BoVector3*);
    void signalGetLightEnabled(int id, bool*);
    void signalSetLightPos(int id, const BoVector4&);
    void signalSetLightAmbient(int id, const BoVector4&);
    void signalSetLightDiffuse(int id, const BoVector4&);
    void signalSetLightSpecular(int id, const BoVector4&);
    void signalSetLightAttenuation(int id, const BoVector3&);
    void signalSetLightEnabled(int id, bool);

    /*  Camera */
    void signalGetCameraPos(BoVector3*);
    void signalGetCameraUp(BoVector3*);
    void signalGetCameraLookAt(BoVector3*);
    void signalGetCameraRotation(float*);
    void signalGetCameraRadius(float*);
    void signalGetCameraZ(float*);
    void signalSetUseCameraLimits(bool);
    void signalSetCameraFreeMovement(bool);

    /*  AutoCamera */
    void signalSetCameraPos(const BoVector3&);
    void signalSetCameraLookAt(const BoVector3&);
    void signalSetCameraUp(const BoVector3&);
    void signalSetCameraRotation(float);
    void signalSetCameraRadius(float);
    void signalSetCameraZ(float);
    void signalSetCameraMoveMode(int);
    void signalCommitCameraChanges(int);
};

#endif

/*
 * vim: et sw=2
 */
