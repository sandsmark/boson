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

#ifndef BOSONSCRIPT_H
#define BOSONSCRIPT_H

class BoVector3;
class BosonBigDisplayBase;
class Player;
class Boson;
class BoCamera;

class QString;
class QDataStream;

#define boScript BosonScript::bosonScript()


/**
 * Base class for scripting interfaces in Boson
 *
 * This class should provide all necessary methods for communication between
 * the script and main program.
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonScript
{
  public:
    enum Language { Python = 1 };

    static BosonScript* newScriptParser(Language lang);

    BosonScript();
    virtual ~BosonScript();

    static BosonScript* bosonScript()  { return mScript; };

    /**
     * Loads script from file.
     * File should be complete path to the script.
     **/
    virtual void loadScript(QString file) = 0;

    /**
     * Calls "advance" function in the script.
     * Should be called every advance call
     **/
    virtual void advance() = 0;
    /**
     * Calls "init" function in the script.
     * Should be called immeditely after the game is started
     **/
    virtual void init() = 0;

    /**
     * Calls function with given name in the script
     **/
    virtual void callFunction(QString function) = 0;

    /**
     * Executes given script line
     **/
     virtual void execLine(const QString& line) = 0;


    void setDisplay(BosonBigDisplayBase* d)  { mDisplay = d; };
    void setPlayer(Player* p)  { mPlayer = p; };

    BosonBigDisplayBase* display()  { return mDisplay; };
    Player* player()  { return mPlayer; };
    Boson* game();
    BoCamera* camera();


    // Players
    bool areEnemies(int playerId1, int playerId2);
    int playerId();
    // Resources
    // Units
    void moveUnit(int id, int x, int y);
    void moveUnitWithAttacking(int id, int x, int y);
    void attack(int attackerId, int targetId);
    void stopUnit(int id);
    // Camera
    void moveCamera(BoVector3 pos);
    void moveCameraBy(BoVector3 pos);
    void setCameraRotation(float r);
    void setCameraRadius(float r);
    void setCameraZ(float z);
    void setCameraMoveMode(int mode);
    void commitCameraChanges(int ticks);

    BoVector3 cameraPos();
    float cameraRotation();
    float cameraRadius();
    float cameraZ();

  protected:
    void sendInput(QDataStream& stream);

  private:
    static BosonScript* mScript;

    BosonBigDisplayBase* mDisplay;
    Player* mPlayer;
};

#endif //BOSONSCRIPT_H
