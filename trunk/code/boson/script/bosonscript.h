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
class BoGameCamera;
class BoAutoGameCamera;
class BosonCanvas;
class Boson;

class QString;
class QDataStream;
class QPoint;

template<class T> class QValueList;


/**
 * Base class for scripting interfaces in Boson
 *
 * This class provides methods for communication between the main program and
 * the script.
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonScript
{
  public:
    enum Language { Python = 1 };

    static BosonScript* newScriptParser(Language lang, Player* p);

    virtual ~BosonScript();

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


    Player* player() const  { return mPlayer; }
    int playerId() const;


    static void setDisplay(BosonBigDisplayBase* d)  { mDisplay = d; }
    static void setGame(Boson* g)  { mGame = g; }
    static void setCanvas(BosonCanvas* c)  { mCanvas = c; }

    static BosonBigDisplayBase* display()  { return mDisplay; }
    static Boson* game()  { return mGame; };

    static BoAutoGameCamera* autoCamera();
    static BoGameCamera* camera();
    static BosonCanvas* canvas()  { return mCanvas; }


    // Players
    /**
     * @return True if players with ids playerId1 and playerId2 are enemies,
     *  otherwise false
     **/
    static bool areEnemies(int playerId1, int playerId2);
    /**
     * @return List containing ids of all players in the game
     **/
    static QValueList<int> allPlayers();


    // Resources
    /**
     * @return Amount of minerals local player has
     **/
    static unsigned long int minerals(int playerId);
    /**
     * Add given amount of minerals for player with id playerId
     **/
    static void addMinerals(int playerId, int amount);
    /**
     * @return Amount of oil local player has
     **/
    static unsigned long int oil(int playerId);
    /**
     * Add given amount of oil for player with id playerId
     **/
    static void addOil(int playerId, int amount);


    // Units
    /**
     * Moves unit with id id to (x, y)
     * Unit will go directly to given position and will ignore any units on the
     * way
     **/
    static void moveUnit(int player, int id, int x, int y);
    /**
     * Moves unit with id id to (x, y)
     * Unit will shoot at any enemy units on the way
     **/
    static void moveUnitWithAttacking(int player, int id, int x, int y);
    /**
     * Unit with id attackerId will attack unit targetId
     **/
    static void attack(int player, int attackerId, int targetId);
    /**
     * Stops unit with id id from doing anything
     **/
    static void stopUnit(int player, int id);
    /**
     * Sends unit with id id to mine at (x, y)
     **/
    static void mineUnit(int player, int id, int x, int y);
    /**
     * Sets unit's rotation to rotation
     **/
    static void setUnitRotation(int player, int id, float rotation);
    /**
     * Drops bomb with given unit with specified weapon to given pos
     **/
    static void dropBomb(int player, int id, int weapon, int x, int y);
    /**
     * Produces unit with type production in factory with id factory
     **/
    static void produceUnit(int player, int factory, int production);
    /**
     * Spawns unit owned by player, with type type, at (x, y)
     **/
    static void spawnUnit(int player, int type, int x, int y);
    /**
     * Teleports (immediately moves) unit with id id, owned by player to (x, y)
     **/
    static void teleportUnit(int player, int id, int x, int y);

    /**
     * @return List of units on cell (x, y)
     **/
    static QValueList<int> unitsOnCell(int x, int y);
    /**
     * @return List of units in rectangle (x1, y1, x2 ,y2)
     * Note that coordinates are cell coordinates.
     **/
    static QValueList<int> unitsInRect(int x1, int y1, int x2, int y2);
    /**
     * @return Whether there are any units on cell (x, y)
     **/
    static bool cellOccupied(int x, int y);

    /**
     * @return Position of unit with id id
     **/
    static QPoint unitPosition(int id);
    /**
     * @return Id of the owner of unit with id id
     **/
    static int unitOwner(int id);
    /**
     * @return Type of unit with id id
     **/
    static int unitType(int id);
    /**
     * @return Whether unit with id id is mobile unit
     **/
    static bool isUnitMobile(int id);
    /**
     * @return Whether unit with id id can shoot
     **/
    static bool canUnitShoot(int id);
    /**
     * @return Whether unit with id id can produce (other units or upgrades)
     **/
    static bool canUnitProduce(int id);

    /**
     * @return List of unit types unit with id id can produce
     **/
    static QValueList<int> productionTypes(int id);

    /**
     * @return Whether unit with id id exists and is not destroyed
     **/
    static bool isUnitAlive(int id);

    /**
     * @return List containing ids of all units of player with id id
     **/
    static QValueList<int> allPlayerUnits(int id);


    // Camera
    static void moveCamera(const BoVector3& pos);
    static void moveCameraBy(const BoVector3& pos);
    static void setCameraRotation(float r);
    static void setCameraRadius(float r);
    static void setCameraZ(float z);
    static void setCameraMoveMode(int mode);
    static void commitCameraChanges(int ticks);

    static BoVector3 cameraPos();
    static float cameraRotation();
    static float cameraRadius();
    static float cameraZ();


    // AI
    static float aiDelay();


    // Other
    static void startBenchmark();
    static void endBenchmark();


  protected:
    BosonScript(Player* p);

    static void sendInput(int player, QDataStream& stream);

  private:
    Player* mPlayer;

    static BosonBigDisplayBase* mDisplay;
    static BosonCanvas* mCanvas;
    static Boson* mGame;
};

#endif //BOSONSCRIPT_H
