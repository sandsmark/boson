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
class Player;
class Boson;
class BosonCanvas;
class Boson;
class BoVector4;
class BoVector2;

class QString;
class QDataStream;

template<class T> class QValueList;

class BosonScriptInterface;

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

    BosonScriptInterface* interface() const
    {
      return mInterface;
    }

    /**
     * @return NULL usually. Only when the interpreter is currently running this
     * returns the BosonScript object the interpreter runs in. See @ref
     * makeScriptCurrent
     **/
    static BosonScript* currentScript()
    {
      return mCurrentScript;
    }


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


    static void setGame(Boson* g)  { mGame = g; }
    static void setCanvas(BosonCanvas* c)  { mCanvas = c; }

    static Boson* game()  { return mGame; };

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
    static QValueList<BoVector2> nearestMineralLocations(int playerId, int x, int y, unsigned int n, unsigned int radius);
    /**
     * Finds n nearest oil locations to point (x, y), that are visible to
     * player with id playerId.
     * At most radius tiles are searched.
     * If n is 0, all visible oil mines in given are returned.
     **/
    static QValueList<BoVector2> nearestOilLocations(int playerId, int x, int y, unsigned int n, unsigned int radius);


    // Units
    /**
     * Moves unit with id id to (x, y)
     * Unit will go directly to given position and will ignore any units on the
     * way
     **/
    static void moveUnit(int player, int id, float x, float y);
    /**
     * Moves unit with id id to (x, y)
     * Unit will shoot at any enemy units on the way
     **/
    static void moveUnitWithAttacking(int player, int id, float x, float y);
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
    static void mineUnit(int player, int id, float x, float y);
    /**
     * Sets unit's rotation to rotation
     **/
    static void setUnitRotation(int player, int id, float rotation);
    /**
     * Drops bomb with given unit with specified weapon to given pos
     **/
    static void dropBomb(int player, int id, int weapon, float x, float y);
    /**
     * Produces unit with type production in factory with id factory
     **/
    static void produceUnit(int player, int factory, int production);
    /**
     * Spawns unit owned by player, with type type, at (x, y)
     **/
    static void spawnUnit(int player, int type, float x, float y);
    /**
     * Teleports (immediately moves) unit with id id, owned by player to (x, y)
     **/
    static void teleportUnit(int player, int id, float x, float y);

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
    static BoVector2 unitPosition(int id);
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
    void setCameraRotation(float r);
    void setCameraRadius(float r);
    void setCameraZ(float z);
    void setCameraMoveMode(int mode);
    void setCameraLookAt(const BoVector3& pos);
    void setCameraPos(const BoVector3& pos);
    void setCameraUp(const BoVector3& up);
    void setCameraLimits(bool on);
    void setCameraFreeMode(bool on);
    void commitCameraChanges(int ticks);

    BoVector3 cameraLookAt();
    BoVector3 cameraPos();
    BoVector3 cameraUp();
    float cameraRotation();
    float cameraRadius();
    float cameraZ();


    // Lights
    BoVector4 lightPos(int id);
    BoVector4 lightAmbient(int id);
    BoVector4 lightDiffuse(int id);
    BoVector4 lightSpecular(int id);
    BoVector3 lightAttenuation(int id);
    bool lightEnabled(int id);

    void setLightPos(int id, BoVector4 pos);
    void setLightAmbient(int id, BoVector4 a);
    void setLightDiffuse(int id, BoVector4 a);
    void setLightSpecular(int id, BoVector4 a);
    void setLightAttenuation(int id, BoVector3 a);
    void setLightEnabled(int id, bool enable);

    int addLight();
    void removeLight(int id);


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

    /**
     * Most script interpreters use a C interface and therefore require
     * callbacks to be functions (i.e. no methods). However, very often we need
     * a pointer back to the actual BosonScript object that the interpreter runs
     * in.
     *
     * For this purpose makeScriptCurrent() exists. You <em>must</em> call this
     * <em>before</em> you execute any line with the interpreter. When you the
     * interpreter is done, you are supposed to call makeScriptCurrent(0).
     *
     * That call will make sure that @ref currentScript is always valid.
     **/
    static void makeScriptCurrent(BosonScript*);

  private:
    static BosonScript* mCurrentScript;
    BosonScriptInterface* mInterface;

    Player* mPlayer;

    static BosonCanvas* mCanvas;
    static Boson* mGame;
};

#endif //BOSONSCRIPT_H

/*
 * vim: et sw=2
 */
