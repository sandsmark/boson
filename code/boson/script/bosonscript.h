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
class BoLight;
class BoVector4;

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
     * Loads script from given string.
     **/
    virtual void loadScriptFromString(const QString& string) = 0;

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
    virtual void callFunction(const QString& function) = 0;
    virtual int callFunctionWithReturn(const QString& function) = 0;

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
    static BoLight* light(int id);
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
    /**
     * Finds n nearest mineral locations to point (x, y), that are visible to
     * player with id playerId.
     * At most radius tiles are searched.
     * If n is 0, all visible mineral mines in given are returned.
     **/
    static QValueList<QPoint> nearestMineralLocations(int playerId, int x, int y, unsigned int n, unsigned int radius);
    /**
     * Finds n nearest oil locations to point (x, y), that are visible to
     * player with id playerId.
     * At most radius tiles are searched.
     * If n is 0, all visible oil mines in given are returned.
     **/
    static QValueList<QPoint> nearestOilLocations(int playerId, int x, int y, unsigned int n, unsigned int radius);


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
     * @return Work of unit with id id
     * Work shows what unit is currently doing - attacking, moving, etc, or just
     * standing.
     * Note that this only returns int "code" of work. Look at unitbase.h to
     * find out what different values mean.
     **/
    static int unitWork(int id);
    /**
     * @return Whether unit with id id is mobile unit
     **/
    static bool isUnitMobile(int id);
    /**
     * @return Whether unit with id id can shoot
     **/
    static bool canUnitShoot(int id);
    /**
     * @return Whether unit with given type can shoot
     **/
    static bool canUnitTypeShoot(int playerid, int type);
    /**
     * @return Whether unit with id id can produce (other units or upgrades)
     **/
    static bool canUnitProduce(int id);
    /**
     * @return Can unit with id id mine minerals
     **/
    static bool canUnitMineMinerals(int id);
    /**
     * @return Can player's unit with given type mine minerals
     **/
    static bool canUnitTypeMineMinerals(int playerid, int type);
    /**
     * @return Can unit with id id mine oil
     **/
    static bool canUnitMineOil(int id);
    /**
     * @return Can unit with given type mine minerals
     **/
    static bool canUnitTypeMineOil(int playerid, int type);

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
     * Warning: this method might be slow in case player has many units
     **/
    static QValueList<int> allPlayerUnits(int id);
    /**
     * @return How many units player with given id has
     *
     * If you don't need a list of units, this method is faster, than
     *  @ref allPlayerUnits, but it might still be a bit slow when player has
     *  many units
     **/
    static int allPlayerUnitsCount(int id);
    /**
     * @return List containing all units with given type, belonging to player
     *  with given id
     **/
    static QValueList<int> playerUnitsOfType(int playerid, int type);
    /**
     * @return how many units of given type player with given id has
     **/
    static int playerUnitsOfTypeCount(int playerid, int type);


    // Camera
    static void setCameraRotation(float r);
    static void setCameraRadius(float r);
    static void setCameraZ(float z);
    static void setCameraMoveMode(int mode);
    static void setCameraLookAt(const BoVector3& pos);
    static void setCameraPos(const BoVector3& pos);
    static void setCameraUp(const BoVector3& up);
    static void setCameraLimits(bool on);
    static void setCameraFreeMode(bool on);
    static void commitCameraChanges(int ticks);

    static BoVector3 cameraLookAt();
    static BoVector3 cameraPos();
    static BoVector3 cameraUp();
    static float cameraRotation();
    static float cameraRadius();
    static float cameraZ();


    // Lights
    static BoVector4 lightPos(int id);
    static BoVector4 lightAmbient(int id);
    static BoVector4 lightDiffuse(int id);
    static BoVector4 lightSpecular(int id);
    static BoVector3 lightAttenuation(int id);
    static bool lightEnabled(int id);

    static void setLightPos(int id, BoVector4 pos);
    static void setLightAmbient(int id, BoVector4 a);
    static void setLightDiffuse(int id, BoVector4 a);
    static void setLightSpecular(int id, BoVector4 a);
    static void setLightAttenuation(int id, BoVector3 a);
    static void setLightEnabled(int id, bool enable);

    static int addLight();
    static void removeLight(int id);


    // AI
    static float aiDelay();


    // Other
    static void startBenchmark();
    static void endBenchmark(const QString& name);
    static void setRandomSeed(long int seed);
    static void findPath(int x1, int y1, int x2, int y2);
    static void addEffect(unsigned int id, BoVector3 pos, float zrot = 0.0f);


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
