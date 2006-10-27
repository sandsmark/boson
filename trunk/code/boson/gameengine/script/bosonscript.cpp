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

#include "bosonscript.h"

#include <bomemory/bodummymemory.h>
#include "../../bo3dtools.h"
#include "../boson.h"
#include "../bosonmessage.h"
#include "../bosonmessageids.h"
#include "../bosoncanvas.h"
#include "../bosoncollisions.h"
#include "../rtti.h"
#include "../unit.h"
#include "../boitemlist.h"
#include "../../bosonconfig.h"
#include "../unitproperties.h"
#include "../unitplugins/unitplugins.h"
#include "../../bosonprofiling.h"
#include "../bosonpath.h"
#include "../speciestheme.h"
#include "../playerio.h"
#include "../pluginproperties.h"
#include "../unitbase.h"
#include "../bosonplayfield.h"
#include "../bosonmap.h"
#include "bosonscriptinterface.h"
#include "bodebug.h"

#warning FIXME: remove
// FIXME: don't include player.h, or more precisely: don't provide access to
// it's methods from script functions. Player provides methods such as
// allUnits() that simply should not be used in AI (which is the main part of
// scripts).
// use PlayerIO whenever possible (and in most cases: add methods to it, if it's
// not).
//
// Even more important: the scripts should have access to the Player/PlayerIO
// object of the player with ID playerId() only. not of other players.
#include "../player.h"

#include "pythonscript.h"

#include <kgame/kgamemessage.h>
#include <kglobal.h>
#include <kstandarddirs.h>


#include <qdatastream.h>


BosonScript* BosonScript::mCurrentScript = 0;
BosonCanvas* BosonScript::mCanvas = 0;
Boson* BosonScript::mGame = 0;

BosonScript* BosonScript::newScriptParser(Language lang, int playerId)
{
  BosonScript* s = 0;
  if(lang == Python)
  {
    s = new PythonScript(playerId);
  }
  else
  {
    boDebug() << k_funcinfo << "Invalid script language: " << lang << endl;
    s = 0;
  }
  return s;
}

BosonScript::BosonScript(int playerId)
{
  mInterface = new BosonScriptInterface(0);
  mPlayerId = playerId;
}

BosonScript::~BosonScript()
{
  delete mInterface;
}

void BosonScript::sendInput(QDataStream& stream)
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return;
  }

  Player* p = scriptPlayer();

  if(!p)
  {
    boError() << k_funcinfo << "No player with id " << playerId() << endl;
    return;
  }

  p->forwardInput(stream);
}

void BosonScript::makeScriptCurrent(BosonScript* s)
{
  mCurrentScript = s;
}

int BosonScript::playerId() const
{
  return mPlayerId;
}

QString BosonScript::scriptsPath()
{
  QString path = KGlobal::dirs()->findResourceDir("data", "boson/themes/scripts/ai.py");
  if(path.isNull())
  {
    boWarning() << k_funcinfo << "No ai.py script file found!" << endl;
    return QString::null;
  }
  path += "boson/themes/scripts/";
  return path;
  // TODO: maybe cache the path?
}


/*****  Event methods  *****/
int BosonScript::addEventHandler(const QString& eventname, const QString& functionname, const QString& args)
{
  return interface()->addEventHandler(eventname, functionname, args);
}

void BosonScript::removeEventHandler(int id)
{
  interface()->removeEventHandler(id);
}


/*****  Player methods  *****/
bool BosonScript::areEnemies(int playerId1, int playerId2) const
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return false;  // What to return here?
  }

  PlayerIO* p1 = findPlayerIOByUserId(playerId1);

  if(!p1)
  {
    boError() << k_funcinfo << "No player with id " << playerId1 << endl;
    return false;
  }

  return p1->isPlayerEnemy(playerId2);
}

bool BosonScript::isEnemy(int p) const
{
  return areEnemies(playerId(), p);
}

QValueList<int> BosonScript::allGamePlayers() const
{
  QValueList<int> players;

  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return players;
  }

  QPtrListIterator<Player> it(*game()->gamePlayerList());
  for (; it.current(); ++it)
  {
    Player* p = it.current();
    players.append(p->bosonId());
  }

  return players;
}

bool BosonScript::isNeutral(int player) const
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return false;  // What to return here?
  }

  if(playerId() != 0)
  {
    PlayerIO* p = scriptPlayerIO();
    if(!p)
    {
      boError() << k_funcinfo << "NULL script player" << endl;
      return false;
    }
    return p->isPlayerNeutral(player);
  }
  else
  {
    Player* p = findPlayerByUserId(player);
    if(!p)
    {
      boError() << k_funcinfo << "No player with id " << player << endl;
      return false;
    }
    return p->isNeutralPlayer();
  }

  return false;
}

unsigned long int BosonScript::powerGenerated(int playerId) const
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return 0;
  }
#if 0
  if(!canAccessPlayer(playerId))
  {
    boWarning() << k_funcinfo << "player with ID " << playerId << " cannot be acessed from current script." << endl;
    return 0;
  }
#endif

  PlayerIO* p = findPlayerIOByUserId(playerId);

  if(!p)
  {
    boError() << k_funcinfo << "No player with id " << playerId << endl;
    return 0;
  }

  unsigned long int powerGenerated = 0;
  p->calculatePower(&powerGenerated, 0);
  return powerGenerated;
}

unsigned long int BosonScript::powerConsumed(int playerId) const
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return 0;
  }

  PlayerIO* p = findPlayerIOByUserId(playerId);

  if(!p)
  {
    boError() << k_funcinfo << "No player with id " << playerId << endl;
    return 0;
  }

  unsigned long int powerConsumed = 0;
  p->calculatePower(0, &powerConsumed);
  return powerConsumed;
}

unsigned long int BosonScript::powerGeneratedAfterConstructions(int playerId) const
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return 0;
  }

  PlayerIO* p = findPlayerIOByUserId(playerId);

  if(!p)
  {
    boError() << k_funcinfo << "No player with id " << playerId << endl;
    return 0;
  }

  unsigned long int powerGenerated = 0;
  p->calculatePower(&powerGenerated, 0, true);
  return powerGenerated;
}

unsigned long int BosonScript::powerConsumedAfterConstructions(int playerId) const
{
  PlayerIO* p = findPlayerIOByUserId(playerId);

  if(!p)
  {
    boError() << k_funcinfo << "No player with id " << playerId << endl;
    return 0;
  }

  unsigned long int powerConsumed = 0;
  p->calculatePower(0, &powerConsumed, true);
  return powerConsumed;
}

bool BosonScript::isCellFogged(int playerId, int x, int y) const
{
  PlayerIO* p = findPlayerIOByUserId(playerId);

  if(!p)
  {
    boError() << k_funcinfo << "No player with id " << playerId << endl;
    return 0;
  }

  return p->isFogged(x, y);
}

bool BosonScript::isCellExplored(int playerId, int x, int y) const
{
  PlayerIO* p = findPlayerIOByUserId(playerId);

  if(!p)
  {
    boError() << k_funcinfo << "No player with id " << playerId << endl;
    return 0;
  }

  return p->isExplored(x, y);
}


/*****  Resource methods  *****/
unsigned long int BosonScript::minerals(int playerId) const
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return 0;
  }

  PlayerIO* p = findPlayerIOByUserId(playerId);

  if(!p)
  {
    boError() << k_funcinfo << "No player with id " << playerId << endl;
    return 0;
  }

  return p->minerals();
}

void BosonScript::addMinerals(int player, int amount)
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return;
  }
  if (playerId() != 0)
  {
    boError() << k_funcinfo << "player scripts are not allowed to cheat." << endl;
    return;
  }

  QByteArray b;
  QDataStream stream(b, IO_WriteOnly);
  stream << (Q_UINT32)player;
  stream << (Q_INT32)amount;
  game()->sendMessage(b, BosonMessageIds::IdModifyMinerals);
}

unsigned long int BosonScript::oil(int playerId) const
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return 0;
  }

  PlayerIO* p = findPlayerIOByUserId(playerId);

  if(!p)
  {
    boError() << k_funcinfo << "No player with id " << playerId << endl;
    return 0;
  }

  return p->oil();
}

void BosonScript::addOil(int player, int amount)
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return;
  }
  if (playerId() != 0)
  {
    boError() << k_funcinfo << "player scripts are not allowed to cheat." << endl;
    return;
  }

  QByteArray b;
  QDataStream stream(b, IO_WriteOnly);
  stream << (Q_UINT32)player;
  stream << (Q_INT32)amount;
  game()->sendMessage(b, BosonMessageIds::IdModifyOil);
}

QValueList<BoVector2Fixed> BosonScript::nearestMineralLocations(int x, int y, unsigned int n, unsigned int radius)
{
  PlayerIO* p = scriptPlayerIO();

  if(!p)
  {
    boError() << k_funcinfo << "No player with id " << playerId() << endl;
    return QValueList<BoVector2Fixed>();
  }

  return p->nearestMineralLocations(x, y, n, radius);
}

QValueList<BoVector2Fixed> BosonScript::nearestOilLocations(int x, int y, unsigned int n, unsigned int radius)
{
  PlayerIO* p = scriptPlayerIO();

  if(!p)
  {
    boError() << k_funcinfo << "No player with id " << playerId() << endl;
    return QValueList<BoVector2Fixed>();
  }

  return p->nearestOilLocations(x, y, n, radius);
}


/*****  Unit methods  *****/
void BosonScript::moveUnit(int id, float x, float y)
{
  QByteArray b;
  QDataStream stream(b, IO_WriteOnly);

  QValueList<Q_ULONG> moveUnits;
  moveUnits.append((Q_ULONG)id);
  BosonMessageMoveMove message(false, BoVector2Fixed(x, y), moveUnits);
  if(!message.save(stream))
  {
    boError() << k_funcinfo << "failed saving message " << message.messageId() << " to stream" << endl;
    return;
  }

  QDataStream msg(b, IO_ReadOnly);
  sendInput(msg);
}

void BosonScript::moveUnitWithAttacking(int id, float x, float y)
{
  QByteArray b;
  QDataStream stream(b, IO_WriteOnly);

  QValueList<Q_ULONG> moveUnits;
  moveUnits.append((Q_ULONG)id);
  BosonMessageMoveMove message(true, BoVector2Fixed(x, y), moveUnits);
  if(!message.save(stream))
  {
    boError() << k_funcinfo << "failed saving message " << message.messageId() << " to stream" << endl;
    return;
  }

  QDataStream msg(b, IO_ReadOnly);
  sendInput(msg);
}

void BosonScript::attack(int attackerId, int targetId)
{
  QByteArray b;
  QDataStream stream(b, IO_WriteOnly);

  QValueList<Q_ULONG> units;
  units.append((Q_ULONG)attackerId);
  BosonMessageMoveAttack message(targetId, units);
  if(!message.save(stream))
  {
    boError() << k_funcinfo << "failed saving message " << message.messageId() << " to stream" << endl;
    return;
  }

  QDataStream msg(b, IO_ReadOnly);
  sendInput(msg);
}

void BosonScript::stopUnit(int id)
{
  QByteArray b;
  QDataStream stream(b, IO_WriteOnly);

  QValueList<Q_ULONG> units;
  units.append((Q_ULONG)id);
  BosonMessageMoveStop message(units);
  if(!message.save(stream))
  {
    boError() << k_funcinfo << "failed saving message " << message.messageId() << " to stream" << endl;
    return;
  }

  QDataStream msg(b, IO_WriteOnly);
  sendInput(msg);
}

void BosonScript::mineUnit(int id, float x, float y)
{
  PlayerIO* p = scriptPlayerIO();
  BO_CHECK_NULL_RET(p);

  // First we have to find resource mine
  // TODO: it sucks to do this here, perhaps we could have something like a
  //  MoveMineAt message which orders unit to move at specific point, not at
  //  some unit (then Boson would take care of finding resource mine unit).
  Unit* resourceUnit = p->findUnitAt(BoVector3Fixed(x, y, 0.0));
  if(!resourceUnit)
  {
    boError() << k_funcinfo << "No units found at (" << x << "; " << y << ")" << endl;
    return;
  }
  ResourceMinePlugin* resource = (ResourceMinePlugin*)resourceUnit->plugin(UnitPlugin::ResourceMine);
  if(!resource)
  {
    // there is no mine at destination
    boError() << k_funcinfo << "No resource mine found at (" << x << "; " << y << ")" << endl;
    return;
  }

  QByteArray b;
  QDataStream stream(b, IO_WriteOnly);

  BosonMessageMoveMine message(id, resourceUnit->id());
  if(!message.save(stream))
  {
    boError() << k_funcinfo << "failed saving message " << message.messageId() << " to stream" << endl;
    return;
  }

  QDataStream msg(b, IO_ReadOnly);
  sendInput(msg);
}

void BosonScript::setUnitRotation(int id, float rotation)
{
  QByteArray b;
  QDataStream stream(b, IO_WriteOnly);

  BosonMessageMoveRotate message(id, playerId(), rotation);
  if(!message.save(stream))
  {
    boError() << k_funcinfo << "failed saving message " << message.messageId() << " to stream" << endl;
    return;
  }

  QDataStream msg(b, IO_ReadOnly);
  sendInput(msg);
}

void BosonScript::dropBomb(int id, int weapon, float x, float y)
{
  QByteArray b;
  QDataStream stream(b, IO_WriteOnly);

  QValueList<Q_ULONG> units;
  QValueList<Q_ULONG> weapons;
  units.append((Q_ULONG)id);
  weapons.append((Q_ULONG)weapon);
  BosonMessageMoveDropBomb message(BoVector2Fixed(x, y), units, weapons);
  if(!message.save(stream))
  {
    boError() << k_funcinfo << "failed saving message " << message.messageId() << " to stream" << endl;
    return;
  }

  QDataStream msg(b, IO_ReadOnly);
  sendInput(msg);
}

void BosonScript::produceUnit(int factory, int production)
{
  QByteArray b;
  QDataStream stream(b, IO_WriteOnly);

  BosonMessageMoveProduce message((Q_UINT32)ProduceUnit, playerId(), factory, production);
  if(!message.save(stream))
  {
    boError() << k_funcinfo << "failed saving message " << message.messageId() << " to stream" << endl;
    return;
  }

  QDataStream msg(b, IO_ReadOnly);
  sendInput(msg);
}

void BosonScript::spawnUnit(int type, float x, float y)
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return;
  }
  QByteArray b;
  QDataStream stream(b, IO_WriteOnly);

#warning AB: This is an editor message. it should not be here.
  boWarning() << k_funcinfo << "FIXME" << endl;
#if 0
  stream << (Q_UINT32)BosonMessageIds::MovePlaceUnit;
  stream << (Q_UINT32)player;
  stream << (Q_UINT32)type;
  stream << BoVector2Fixed(x, y);
#endif

  QDataStream msg(b, IO_ReadOnly);
  sendInput(msg);
}

void BosonScript::teleportUnit(int id, float x, float y)
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return;
  }
  QByteArray b;
  QDataStream stream(b, IO_WriteOnly);

  BosonMessageMoveTeleport message(id, playerId(), BoVector2Fixed(x, y));
  if(!message.save(stream))
  {
    boError() << k_funcinfo << "failed saving message " << message.messageId() << " to stream" << endl;
    return;
  }

  QDataStream msg(b, IO_ReadOnly);
  sendInput(msg);
}

void BosonScript::placeProduction(int factoryid, float x, float y)
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return;
  }

  PlayerIO* p = scriptPlayerIO();

  if(!p)
  {
    boError() << k_funcinfo << "No player with id " << playerId() << endl;
    return;
  }

  Unit* u = p->findUnit(factoryid);
  if(!u)
  {
    boError() << k_funcinfo << "No unit with id" << factoryid << endl;
    return;
  }

  ProductionPlugin* prod = (ProductionPlugin*)u->plugin(UnitPlugin::Production);
  if(!prod)
  {
    boError() << k_funcinfo << "Unit " << factoryid << " doesn't have production plugin!" << endl;
    return;
  }
  if(prod->completedProductionType() != ProduceUnit)
  {
    boError() << k_lineinfo << "not producing unit!" << endl;
    return;
  }
  int unitType = prod->completedProductionId();
  if (unitType <= 0) {
    // hope this is working...
    boWarning() << k_lineinfo << "not yet completed" << endl;
    return;
  }

  QByteArray b;
  QDataStream stream(b, IO_WriteOnly);

  BosonMessageMoveBuild message((Q_UINT32)ProduceUnit, p->playerId(), u->id(), BoVector2Fixed(x, y));
  if(!message.save(stream))
  {
    boError() << k_funcinfo << "failed saving message " << message.messageId() << " to stream" << endl;
    return;
  }

  QDataStream msg(b, IO_ReadOnly);
  sendInput(msg);
}

bool BosonScript::canPlaceProductionAt(int factoryid, int unitType, float x, float y)
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return false;
  }
  if(!canvas())
  {
    boError() << k_funcinfo << "NULL canvas" << endl;
    return false;
  }

  PlayerIO* p = scriptPlayerIO();

  if(!p)
  {
    boError() << k_funcinfo << "No player with id " << playerId() << endl;
    return false;
  }

  Unit* u = p->findUnit(factoryid);
  if(!u)
  {
    boError() << k_funcinfo << "No unit with id" << factoryid << endl;
    return false;
  }
  if(!u->speciesTheme())
  {
    BO_NULL_ERROR(u->speciesTheme());
    return false;
  }

  ProductionPlugin* prod = (ProductionPlugin*)u->plugin(UnitPlugin::Production);
  if(!prod)
  {
    boError() << k_funcinfo << "Unit " << factoryid << " doesn't have production plugin!" << endl;
    return false;
  }
  const UnitProperties* prop = u->speciesTheme()->unitProperties(unitType);
  if(!prop)
  {
    boError() << k_funcinfo << "no such unittype " << unitType << endl;
    return false;
  }
  return canvas()->canPlaceUnitAt(prop, BoVector2Fixed(x, y), prod);
}

QValueList<int> BosonScript::unitsOnCell(int x, int y) const
{
  QValueList<int> list;
  if(!canvas())
  {
    boError() << k_funcinfo << "NULL canvas" << endl;
    return list;
  }
  BosonCollisions* c = canvas()->collisions();
  BoItemList* l = c->collisionsAtCell(x, y);
  for(BoItemList::Iterator it = l->begin(); it != l->end(); ++it)
  {
    if(RTTI::isUnit((*it)->rtti()))
    {
      list.append(((Unit*)(*it))->id());
    }
  }
  return list;
}

QValueList<int> BosonScript::unitsInRect(int x1, int y1, int x2, int y2) const
{
  QValueList<int> list;
  if(!canvas())
  {
    boError() << k_funcinfo << "NULL canvas" << endl;
    return list;
  }
  BosonCollisions* c = canvas()->collisions();
  BoItemList* l = c->collisionsAtCells(BoRect2Fixed(BoVector2Fixed(x1, y1), BoVector2Fixed(x2, y2)));
  for(BoItemList::Iterator it = l->begin(); it != l->end(); ++it)
  {
    if(RTTI::isUnit((*it)->rtti()))
    {
      list.append(((Unit*)(*it))->id());
    }
  }
  return list;
}

bool BosonScript::cellOccupied(int x, int y) const
{
  if(!canvas())
  {
    boError() << k_funcinfo << "NULL canvas" << endl;
    return false;
  }
  return canvas()->cellOccupied(x, y);
}

BoVector2Fixed BosonScript::unitPosition(int id) const
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return BoVector2Fixed(-1, -1);
  }

  Unit* u = findUnit(id);
  if(!u)
  {
    boError() << k_funcinfo << "No unit with id" << id << endl;
    return BoVector2Fixed(-1, -1);
  }

  return BoVector2Fixed(u->x(), u->y());
}

int BosonScript::unitOwner(int id) const
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return -1;
  }

  Unit* u = findUnit(id);
  if(!u)
  {
    boError() << k_funcinfo << "No unit with id" << id << endl;
    return -1;
  }

  return u->owner()->bosonId();
}

int BosonScript::unitType(int id) const
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return -1;
  }

  Unit* u = findUnit(id);
  if(!u)
  {
    boError() << k_funcinfo << "No unit with id" << id << endl;
    return -1;
  }

  return u->unitProperties()->typeId();
}

int BosonScript::unitAdvanceWork(int id) const
{
  Unit* u = findUnit(id);
  if(!u)
  {
    boError() << k_funcinfo << "No unit with id" << id << endl;
    return -1;
  }

  return (int)u->advanceWork();
}

int BosonScript::unitSightRange(int id) const
{
  Unit* u = findUnit(id);
  if(!u)
  {
    boError() << k_funcinfo << "No unit with id" << id << endl;
    return 0;
  }

  return (int)u->sightRange();
}

bool BosonScript::isUnitMobile(int id) const
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return false;
  }

  Unit* u = findUnit(id);
  if(!u)
  {
    boError() << k_funcinfo << "No unit with id" << id << endl;
    return false;
  }

  return u->isMobile();
}

bool BosonScript::isUnitTypeMobile(int playerId, int type) const
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return false;
  }

  PlayerIO* p = findPlayerIOByUserId(playerId);

  if(!p)
  {
    boError() << k_funcinfo << "No player with id " << playerId << endl;
    return false;
  }

  const UnitProperties* prop = p->unitProperties(type);
  if(!prop)
  {
    boError() << k_funcinfo << "No unit properties with typeid " << type << " for player with id " << playerId << endl;
    return false;
  }

  return prop->isMobile();
}

bool BosonScript::isUnitAircraft(int id) const
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return false;
  }

  Unit* u = findUnit(id);
  if(!u)
  {
    boError() << k_funcinfo << "No unit with id " << id << ", or unit not visible for script player" << endl;
    return false;
  }

  return u->unitProperties()->isAircraft();
}

bool BosonScript::isUnitTypeAircraft(int playerId, int type) const
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return false;
  }

  PlayerIO* p = findPlayerIOByUserId(playerId);

  if(!p)
  {
    boError() << k_funcinfo << "No player with id " << playerId << endl;
    return false;
  }

  const UnitProperties* prop = p->unitProperties(type);
  if(!prop)
  {
    boError() << k_funcinfo << "No unit properties with typeid " << type << " for player with id " << playerId << endl;
    return false;
  }

  return prop->isAircraft();
}

bool BosonScript::canUnitShoot(int id) const
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return false;
  }

  Unit* u = findUnit(id);
  if(!u)
  {
    boError() << k_funcinfo << "No unit with id" << id << endl;
    return false;
  }

  return u->unitProperties()->canShoot();
}

bool BosonScript::canUnitTypeShoot(int playerId, int type) const
{
  QValueList<int> list;
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return false;
  }

  PlayerIO* p = findPlayerIOByUserId(playerId);

  if(!p)
  {
    boError() << k_funcinfo << "No player with id " << playerId << endl;
    return false;
  }

  const UnitProperties* prop = p->unitProperties(type);
  if(!prop)
  {
    boError() << k_funcinfo << "No unit properties with typeid " << type << " for player with id " << playerId << endl;
    return false;
  }

  return prop->canShoot();
}

bool BosonScript::canUnitProduce(int id) const
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return false;
  }

  Unit* u = findUnit(id);
  if(!u)
  {
    boError() << k_funcinfo << "No unit with id" << id << endl;
    return false;
  }

  return (u->plugin(UnitPlugin::Production));
}

bool BosonScript::hasUnitCompletedProduction(int id) const
{
  return (completedProductionType(id) > 0);
}

unsigned long int BosonScript::completedProductionType(int id) const
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return 0;
  }

  Unit* u = findUnit(id);
  if(!u)
  {
    boError() << k_funcinfo << "No unit with id" << id << endl;
    return 0;
  }

  ProductionPlugin* prod = (ProductionPlugin*)u->plugin(UnitPlugin::Production);
  if(!prod)
  {
    // cannot produce
    return 0;
  }
  return prod->completedProductionId();
}

bool BosonScript::canUnitMineMinerals(int id) const
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return false;
  }

  Unit* u = findUnit(id);
  if(!u)
  {
    boError() << k_funcinfo << "No unit with id" << id << endl;
    return false;
  }

  HarvesterPlugin* res = (HarvesterPlugin*)u->plugin(UnitPlugin::Harvester);
  if(res && res->canMineMinerals())
  {
    return true;
  }
  return false;
}

bool BosonScript::canUnitTypeMineMinerals(int playerId, int type) const
{
  QValueList<int> list;
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return false;
  }

  PlayerIO* p = findPlayerIOByUserId(playerId);

  if(!p)
  {
    boError() << k_funcinfo << "No player with id " << playerId << endl;
    return false;
  }

  const UnitProperties* prop = p->unitProperties(type);
  if(!prop)
  {
    boError() << k_funcinfo << "No unit properties with typeid " << type << " for player with id " << playerId << endl;
    return false;
  }

  const HarvesterProperties* harvester = (const HarvesterProperties*)prop->properties(PluginProperties::Harvester);
  if(harvester && harvester->canMineMinerals())
  {
    return true;
  }
  return false;
}

bool BosonScript::canUnitMineOil(int id) const
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return false;
  }

  Unit* u = findUnit(id);
  if(!u)
  {
    boError() << k_funcinfo << "No unit with id" << id << endl;
    return false;
  }

  HarvesterPlugin* res = (HarvesterPlugin*)u->plugin(UnitPlugin::Harvester);
  if(res && res->canMineOil())
  {
    return true;
  }
  return false;
}

bool BosonScript::canUnitTypeMineOil(int playerId, int type) const
{
  QValueList<int> list;
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return false;
  }

  PlayerIO* p = findPlayerIOByUserId(playerId);

  if(!p)
  {
    boError() << k_funcinfo << "No player with id " << playerId << endl;
    return false;
  }

  const UnitProperties* prop = p->unitProperties(type);
  if(!prop)
  {
    boError() << k_funcinfo << "No unit properties with typeid " << type << " for player with id " << playerId << endl;
    return false;
  }

  const HarvesterProperties* harvester = (const HarvesterProperties*)prop->properties(PluginProperties::Harvester);
  if(harvester && harvester->canMineOil())
  {
    return true;
  }
  return false;
}

QValueList<int> BosonScript::productionTypes(int id) const
{
  QValueList<int> list;

  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return list;
  }

  Unit* u = findUnit(id);
  if(!u)
  {
    boError() << k_funcinfo << "No unit with id" << id << endl;
    return list;
  }

  ProductionProperties* production = (ProductionProperties*)u->properties(PluginProperties::Production);
  if(!production)
  {
    boError() << k_funcinfo << "Unit with id " << id << " cannot produce" << endl;
    return list;
  }

  // Add units to production list
  QValueList<unsigned long int> unitsList = u->speciesTheme()->productions(production->producerList());
  // Filter out things that player can't actually build (requirements aren't met yet)
  QValueList<unsigned long int>::Iterator it;
  it = unitsList.begin();
  for (; it != unitsList.end(); ++it) {
    if (u->ownerIO()->canBuild(*it)) {
      list.append(*it);
    }
  }

  return list;
}

bool BosonScript::isUnitAlive(int id) const
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return false;
  }

  Unit* u = findUnit(id);
  if(!u)
  {
    return false;
  }

  return !u->isDestroyed();
}

QValueList<int> BosonScript::allPlayerUnits(int id) const
{
  QValueList<int> list;
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return list;
  }

  PlayerIO* p = findPlayerIOByUserId(id);

  if(!p)
  {
    boError() << k_funcinfo << "No player with id " << id << endl;
    return list;
  }

  const QPtrList<Unit>& units = p->allMyLivingUnits();
  QPtrListIterator<Unit> it(units);
  while(it.current())
  {
    list.append(it.current()->id());
    ++it;
  }
  return list;
}

int BosonScript::allPlayerUnitsCount(int id) const
{
  int count = 0;
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return count;
  }

  PlayerIO* p = findPlayerIOByUserId(id);

  if(!p)
  {
    boError() << k_funcinfo << "No player with id " << id << endl;
    return count;
  }

  return p->allMyLivingUnits().count();
}

QValueList<int> BosonScript::playerUnitsOfType(int playerId, int type) const
{
  QValueList<int> list;
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return list;
  }

  PlayerIO* p = findPlayerIOByUserId(playerId);

  if(!p)
  {
    boError() << k_funcinfo << "No player with id " << playerId << endl;
    return list;
  }

  const QPtrList<Unit>& units = p->allMyLivingUnits();
  QPtrListIterator<Unit> it(units);
  while(it.current())
  {
    if((int)it.current()->type() == type)
    {
      list.append(it.current()->id());
    }
    ++it;
  }
  return list;
}

int BosonScript::playerUnitsOfTypeCount(int playerId, int type) const
{
  return playerUnitsOfType(playerId, type).count();
}

QValueList<int> BosonScript::allUnitsVisibleFor(int id) const
{
  QValueList<int> list;

  PlayerIO* p = findPlayerIOByUserId(id);

  if(!p)
  {
    boError() << k_funcinfo << "No player with id " << id << endl;
    return list;
  }

  const QPtrList<Unit>& units = p->allUnits();
  QPtrListIterator<Unit> it(units);
  while(it.current())
  {
    list.append(it.current()->id());
    ++it;
  }
  return list;
}

QValueList<int> BosonScript::allEnemyUnitsVisibleFor(int id) const
{
  QValueList<int> list;

  PlayerIO* p = findPlayerIOByUserId(id);

  if(!p)
  {
    boError() << k_funcinfo << "No player with id " << id << endl;
    return list;
  }

  const QPtrList<Unit>& units = p->allEnemyUnits();
  QPtrListIterator<Unit> it(units);
  while(it.current())
  {
    list.append(it.current()->id());
    ++it;
  }
  return list;
}


/*****  Camera methods  *****/

void BosonScript::setCameraRotation(float r)
{
  interface()->setCameraRotation(r);
}

void BosonScript::setCameraXRotation(float r)
{
  interface()->setCameraXRotation(r);
}

void BosonScript::setCameraDistance(float dist)
{
  interface()->setCameraDistance(dist);
}

void BosonScript::setCameraMoveMode(int mode)
{
  interface()->setCameraMoveMode(mode);
}

void BosonScript::setCameraInterpolationMode(int mode)
{
  interface()->setCameraInterpolationMode(mode);
}

void BosonScript::setCameraPos(const BoVector3Float& pos)
{
  interface()->setCameraPos(pos);
}

void BosonScript::setCameraLookAt(const BoVector3Float& pos)
{
  interface()->setCameraLookAt(pos);
}

void BosonScript::setCameraUp(const BoVector3Float& up)
{
  interface()->setCameraUp(up);
}

void BosonScript::addCameraLookAtPoint(const BoVector3Float& pos, float time)
{
  interface()->addCameraLookAtPoint(pos, time);
}

void BosonScript::addCameraPosPoint(const BoVector3Float& pos, float time)
{
  interface()->addCameraPosPoint(pos, time);
}

void BosonScript::addCameraUpPoint(const BoVector3Float& up, float time)
{
  interface()->addCameraUpPoint(up, time);
}

void BosonScript::setCameraLimits(bool on)
{
  interface()->setUseCameraLimits(on);
}

void BosonScript::setCameraFreeMode(bool on)
{
  interface()->setCameraFreeMovement(on);
}

void BosonScript::commitCameraChanges(int ticks)
{
  interface()->commitCameraChanges(ticks);
}

BoVector3Float BosonScript::cameraLookAt()
{
  return interface()->cameraLookAt();
}

BoVector3Float BosonScript::cameraPos()
{
  return interface()->cameraPos();
}

BoVector3Float BosonScript::cameraUp()
{
  return interface()->cameraUp();
}

float BosonScript::cameraRotation()
{
  return interface()->cameraRotation();
}

float BosonScript::cameraXRotation()
{
  return interface()->cameraXRotation();
}

float BosonScript::cameraDistance()
{
  return interface()->cameraDistance();
}

/*****  Light methods  *****/
BoVector4Float BosonScript::lightPos(int id)
{
  return interface()->lightPos(id);
}

BoVector4Float BosonScript::lightAmbient(int id)
{
  return interface()->lightAmbient(id);
}

BoVector4Float BosonScript::lightDiffuse(int id)
{
  return interface()->lightDiffuse(id);
}

BoVector4Float BosonScript::lightSpecular(int id)
{
  return interface()->lightSpecular(id);
}

BoVector3Float BosonScript::lightAttenuation(int id)
{
  return interface()->lightAttenuation(id);
}

bool BosonScript::lightEnabled(int id)
{
  return interface()->lightEnabled(id);
}

void BosonScript::setLightPos(int id, BoVector4Float pos)
{
  interface()->setLightPos(id, pos);
}

void BosonScript::setLightAmbient(int id, BoVector4Float a)
{
  interface()->setLightAmbient(id, a);
}

void BosonScript::setLightDiffuse(int id, BoVector4Float d)
{
  interface()->setLightDiffuse(id, d);
}

void BosonScript::setLightSpecular(int id, BoVector4Float s)
{
  interface()->setLightSpecular(id, s);
}

void BosonScript::setLightAttenuation(int id, BoVector3Float a)
{
  interface()->setLightAttenuation(id, a);
}

void BosonScript::setLightEnabled(int id, bool enable)
{
  interface()->setLightEnabled(id, enable);
}

int BosonScript::addLight()
{
  return interface()->addLight();
}

void BosonScript::removeLight(int id)
{
  interface()->removeLight(id);
}

/*****  AI methods  *****/
float BosonScript::aiDelay()
{
  return (float)boConfig->doubleValue("AIDelay");
}

/*****  Other methods  *****/
void BosonScript::startBenchmark()
{
// AB: benchmark feature got lost
//  boProfiling->startBenchmark();
}

void BosonScript::endBenchmark(const QString& name)
{
// AB: benchmark feature got lost
//  boProfiling->endBenchmark(name);
}

void BosonScript::setRandomSeed(long int seed)
{
  boGame->sendSystemMessage(seed, KGameMessage::IdSyncRandom);
}

void BosonScript::findPath(int x1, int y1, int x2, int y2)
{
 boDebug() << k_funcinfo << "Trying searching script path" << endl;
 BosonPathInfo i;
 i.start = BoVector2Fixed(x1, y1);
 i.dest = BoVector2Fixed(x2, y2);
 boDebug() << k_funcinfo << "Let's go!" << endl;
 canvas()->pathFinder()->findPath(&i);
 boDebug() << k_funcinfo << "script path searching complete" << endl;
}

void BosonScript::addEffect(unsigned int id, BoVector3Fixed pos, bofixed zrot)
{
  interface()->addEffect(id, pos, zrot);
}

void BosonScript::addEffectToUnit(int unitid, unsigned int effectid)
{
  interface()->addEffectToUnit(unitid, effectid, BoVector3Fixed(), 0);
}

void BosonScript::addEffectToUnit(int unitid, unsigned int effectid, BoVector3Fixed offset, bofixed zrot)
{
  interface()->addEffectToUnit(unitid, effectid, offset, zrot);
}

void BosonScript::advanceEffects(int ticks)
{
  interface()->advanceEffects(ticks);
}

void BosonScript::setWind(const BoVector3Float& wind)
{
  interface()->setWind(wind);
}

BoVector3Float BosonScript::wind()
{
  return interface()->wind();
}

void BosonScript::explorePlayer(int playerid)
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return;
  }
  if(!game()->playField())
  {
    boError() << k_funcinfo << "NULL playField" << endl;
    return;
  }
  BosonMap* map = game()->playField()->map();
  if(!map)
  {
    boError() << k_funcinfo << "NULL map" << endl;
    return;
  }

  Player* p = findPlayerByUserId(playerid);

  if(!p)
  {
    boError() << k_funcinfo << "No player with id " << playerid << endl;
    return;
  }

  // AB: this breaks network, unless it is executed on all clients. scripts
  // cannot ensure that.
  // this method should be removed.
  boWarning() << k_funcinfo << "exploring - may break network" << endl;
  internalExplorePlayer(map, p);
}

void BosonScript::exploreAllPlayers()
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return;
  }
  if(!game()->playField())
  {
    boError() << k_funcinfo << "NULL playField" << endl;
    return;
  }
  BosonMap* map = game()->playField()->map();
  if(!map)
  {
    boError() << k_funcinfo << "NULL map" << endl;
    return;
  }
  QPtrList<Player> list = *game()->allPlayerList();
  for(unsigned int i = 0; i < list.count(); i++)
  {
    Player* p = list.at(i);
    internalExplorePlayer(map, p);
  }
}

void BosonScript::unfogPlayer(int playerid)
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return;
  }
  if(!game()->playField())
  {
    boError() << k_funcinfo << "NULL playField" << endl;
    return;
  }
  BosonMap* map = game()->playField()->map();
  if(!map)
  {
    boError() << k_funcinfo << "NULL map" << endl;
    return;
  }

  Player* p = findPlayerByUserId(playerid);

  if(!p)
  {
    boError() << k_funcinfo << "No player with id " << playerid << endl;
    return;
  }

  // AB: this breaks network, unless it is executed on all clients. scripts
  // cannot ensure that.
  // this method should be removed.
  boWarning() << k_funcinfo << "unfogging - may break network" << endl;
  internalUnfogPlayer(map, p);
}

void BosonScript::unfogAllPlayers()
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return;
  }
  if(!game()->playField())
  {
    boError() << k_funcinfo << "NULL playField" << endl;
    return;
  }
  BosonMap* map = game()->playField()->map();
  if(!map)
  {
    boError() << k_funcinfo << "NULL map" << endl;
    return;
  }
  QPtrList<Player> list = *game()->allPlayerList();
  for(unsigned int i = 0; i < list.count(); i++)
  {
    Player* p = list.at(i);
    internalUnfogPlayer(map, p);
  }
}

void BosonScript::fogPlayer(int playerid)
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return;
  }
  if(!game()->playField())
  {
    boError() << k_funcinfo << "NULL playField" << endl;
    return;
  }
  BosonMap* map = game()->playField()->map();
  if(!map)
  {
    boError() << k_funcinfo << "NULL map" << endl;
    return;
  }

  Player* p = findPlayerByUserId(playerid);

  if(!p)
  {
    boError() << k_funcinfo << "No player with id " << playerid << endl;
    return;
  }

  // AB: this breaks network, unless it is executed on all clients. scripts
  // cannot ensure that.
  // this method should be removed.
  boWarning() << k_funcinfo << "fogging - may break network" << endl;
  internalFogPlayer(map, p);
}

void BosonScript::fogAllPlayers()
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return;
  }
  if(!game()->playField())
  {
    boError() << k_funcinfo << "NULL playField" << endl;
    return;
  }
  BosonMap* map = game()->playField()->map();
  if(!map)
  {
    boError() << k_funcinfo << "NULL map" << endl;
    return;
  }
  QPtrList<Player> list = *game()->allPlayerList();
  for(unsigned int i = 0; i < list.count(); i++)
  {
    Player* p = list.at(i);
    internalFogPlayer(map, p);
  }
}

void BosonScript::setAcceptUserInput(bool accept)
{
  interface()->setAcceptUserInput(accept);
}

void BosonScript::addChatMessage(const QString& from, const QString& message)
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return;
  }

  game()->slotAddChatSystemMessage(from, message);
}

int BosonScript::mapWidth() const
{
  if(!canvas())
  {
    boError() << k_funcinfo << "NULL canvas" << endl;
    return 0;
  }
  return canvas()->mapWidth();
}

int BosonScript::mapHeight() const
{
  if(!canvas())
  {
    boError() << k_funcinfo << "NULL canvas" << endl;
    return 0;
  }
  return canvas()->mapHeight();
}


Player* BosonScript::findPlayerByUserId(int id) const
{
  if(playerId() != 0)
  {
    if(id != playerId())
    {
      boError() << k_funcinfo << "script is not allowed to retrieve Player object of player " << id << ". only " << playerId() << " can be retrieved." << endl;
      return 0;
    }
  }
  BO_CHECK_NULL_RET0(game());
  return (Player*)(game()->findPlayerByUserId(playerId()));
}

PlayerIO* BosonScript::findPlayerIOByUserId(int id) const
{
  Player* p = findPlayerByUserId(id);
  if(p)
  {
    return p->playerIO();
  }
  return 0;
}

Player* BosonScript::scriptPlayer() const
{
  return findPlayerByUserId(playerId());
}

PlayerIO* BosonScript::scriptPlayerIO() const
{
  return findPlayerIOByUserId(playerId());
}

Unit* BosonScript::findUnit(unsigned long int id) const
{
  Unit* u = 0;
  if(playerId() == 0)
  {
    // script may access everything
    u = game()->findUnit(id, 0);
  }
  else
  {
    PlayerIO* p = scriptPlayerIO();
    if(p)
    {
      u = p->findUnit(id);
    }
  }
  return u;
}

void BosonScript::internalExplorePlayer(BosonMap* map, Player* p)
{
  for(unsigned int x = 0; x < map->width(); x++)
  {
    for(unsigned int y = 0; y < map->height(); y++)
    {
      p->explore(x, y);
    }
  }
}

void BosonScript::internalUnfogPlayer(BosonMap* map, Player* p)
{
  for(unsigned int x = 0; x < map->width(); x++)
  {
    for(unsigned int y = 0; y < map->height(); y++)
    {
      p->addFogRef(x, y);
    }
  }
}

void BosonScript::internalFogPlayer(BosonMap* map, Player* p)
{
  for(unsigned int x = 0; x < map->width(); x++)
  {
    for(unsigned int y = 0; y < map->height(); y++)
    {
      p->removeFogRef(x, y);
    }
  }
}

/*
 * vim: et sw=2
 */

