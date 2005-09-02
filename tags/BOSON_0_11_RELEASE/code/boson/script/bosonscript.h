/*
    This file is part of the Boson game
    Copyright (C) 2003-2005 Rivo Laks (rivolaks@hot.ee)

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


#include "../bomath.h"

class Player;
class Boson;
class BosonCanvas;
class Boson;
class BoEvent;
template<class T> class BoVector2;
template<class T> class BoVector3;
template<class T> class BoVector4;
template<class T> class BoRect;
typedef BoVector2<bofixed> BoVector2Fixed;
typedef BoVector3<bofixed> BoVector3Fixed;
typedef BoVector3<float> BoVector3Float;
typedef BoVector4<bofixed> BoVector4Fixed;
typedef BoVector4<float> BoVector4Float;
typedef BoRect<bofixed> BoRectFixed;

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

    static BosonScript* newScriptParser(Language lang, int playerId = -1);

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
    virtual bool loadScript(QString file) = 0;

    /**
     * Loads script from given string.
     **/
    virtual bool loadScriptFromString(const QString& string) = 0;

    /**
     * Calls "advance" function in the script.
     * Should be called every advance call
     **/
    virtual bool advance() = 0;
    /**
     * Calls "init" function in the script.
     * Should be called immeditely after the game is started
     **/
    virtual bool init() = 0;
    /**
     * Calls "setPlayerId" function in the script.
     * New player id of the player that owns this script is given to the
     *  function as a parameter.
     **/
    virtual void setPlayerId(int id) = 0;

    /**
     * Loads script and variables from stream.
     **/
    virtual bool load(QDataStream& stream) = 0;
    /**
     * Saves script and variables to specified stream.
     **/
    virtual bool save(QDataStream& stream) = 0;

    /**
     * Calls function with given name in the script
     **/
    virtual void callFunction(const QString& function) = 0;
    virtual int callFunctionWithReturn(const QString& function) = 0;

    /**
     * Executes given script line
     **/
    virtual void execLine(const QString& line) = 0;

    virtual void callEventHandler(const BoEvent* e, const QString& function, const QString& args) = 0;

    int playerId() const;


    /**
     * @return Path where script files are (ending with boson/themes/scripts/)
     **/
    static QString scriptsPath();


    static void setGame(Boson* g)  { mGame = g; }
    static void setCanvas(BosonCanvas* c)  { mCanvas = c; }

    static Boson* game()  { return mGame; };

    static BosonCanvas* canvas()  { return mCanvas; }


    // Events
    /**
     * Add an event handler.
     * @param eventname name of the event
     * @param functionname name of the function which will be called when
     *  specified event happens
     * @param args arguments that will be given to the function
     *
     * args can consist of 0 or more letters, every letter will be replaced
     *  with corresponding argument. Those letters can be:
     *  @li p - player id
     *  @li u - unit id
     *  @li l - location
     *  @li n - name of the event
     *  @li a - custom data 1
     *  @li b - custom data 2
     *
     * @return id of the event handler. This can be used to remove the event
     *  handler later.
     **/
    int addEventHandler(const QString& eventname, const QString& functionname, const QString& args);
    /**
     * Removes event handler with given id.
     **/
    void removeEventHandler(int id);

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
    /**
     * @return Whether player with given id is neutral player
     **/
    static bool isNeutral(int playerId);


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
    static QValueList<BoVector2Fixed> nearestMineralLocations(int playerId, int x, int y, unsigned int n, unsigned int radius);
    /**
     * Finds n nearest oil locations to point (x, y), that are visible to
     * player with id playerId.
     * At most radius tiles are searched.
     * If n is 0, all visible oil mines in given are returned.
     **/
    static QValueList<BoVector2Fixed> nearestOilLocations(int playerId, int x, int y, unsigned int n, unsigned int radius);


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
     * Places completed production (i.e. unit) of given factory to (x, y)
     * If unit with id factoryid is not factory, if it doesn't have a
     *  production or if the production isn't completed, it does nothing.
     **/
    static void placeProduction(int player, int factoryid, float x, float y);

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
    static BoVector2Fixed unitPosition(int id);
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
     * @return Whether unit with given type is mobile unit
     **/
    static bool isUnitTypeMobile(int playerid, int type);
    /**
     * @return Whether unit with id id is an aircraft
     **/
    static bool isUnitAircraft(int id);
    /**
     * @return Whether unit with given type is aircraft
     **/
    static bool isUnitTypeAircraft(int playerid, int type);
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
    void setCameraXRotation(float r);
    void setCameraDistance(float dist);
    void setCameraMoveMode(int mode);
    void setCameraInterpolationMode(int mode);
    void setCameraLookAt(const BoVector3Float& pos);
    void setCameraPos(const BoVector3Float& pos);
    void setCameraUp(const BoVector3Float& up);
    void addCameraLookAtPoint(const BoVector3Float& pos, float time);
    void addCameraPosPoint(const BoVector3Float& pos, float time);
    void addCameraUpPoint(const BoVector3Float& up, float time);
    void setCameraLimits(bool on);
    void setCameraFreeMode(bool on);
    void commitCameraChanges(int ticks);

    BoVector3Float cameraLookAt();
    BoVector3Float cameraPos();
    BoVector3Float cameraUp();
    float cameraRotation();
    float cameraXRotation();
    float cameraDistance();


    // Lights
    BoVector4Float lightPos(int id);
    BoVector4Float lightAmbient(int id);
    BoVector4Float lightDiffuse(int id);
    BoVector4Float lightSpecular(int id);
    BoVector3Float lightAttenuation(int id);
    bool lightEnabled(int id);

    void setLightPos(int id, BoVector4Float pos);
    void setLightAmbient(int id, BoVector4Float a);
    void setLightDiffuse(int id, BoVector4Float a);
    void setLightSpecular(int id, BoVector4Float a);
    void setLightAttenuation(int id, BoVector3Float a);
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
    static void addEffect(unsigned int id, BoVector3Fixed pos, bofixed zrot = 0);
    // To prevent including bo3dtools.h here
    static void addEffectToUnit(int unitid, unsigned int effectid);
    static void addEffectToUnit(int unitid, unsigned int effectid, BoVector3Fixed offset, bofixed zrot = 0);
    static void advanceEffects(int ticks);
    static void setWind(const BoVector3Float& wind);
    static const BoVector3Float& wind();
    static void unfogPlayer(int playerid);
    static void unfogAllPlayers();
    void setAcceptUserInput(bool accept);
    static void addChatMessage(const QString& from, const QString& message);


  protected:
    BosonScript(int playerId = -1);

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

    int mPlayerId;

    static BosonCanvas* mCanvas;
    static Boson* mGame;
};

#endif //BOSONSCRIPT_H

/*
 * vim: et sw=2
 */
