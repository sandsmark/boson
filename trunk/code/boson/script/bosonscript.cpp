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

#include "bosonscript.h"

#include "../bo3dtools.h"
#include "../player.h"
#include "../boson.h"
#include "../bosonmessage.h"
#include "../bosoncanvas.h"
#include "../bosoncollisions.h"
#include "../rtti.h"
#include "../unit.h"
#include "../boitemlist.h"
#include "../bosonconfig.h"
#include "../unitproperties.h"
#include "../unitplugins.h"
#include "../bosonprofiling.h"
#include "../bosonpath.h"
#include "../speciestheme.h"
#include "../bosoneffectproperties.h"
#include "../bosoneffect.h"
#include "../playerio.h"
#include "../pluginproperties.h"
#include "../unitbase.h"
#include "../bosonplayfield.h"
#include "../bosonmap.h"
#include "bosonscriptinterface.h"
#include "bodebug.h"

#include "pythonscript.h"

#include <kgame/kgamemessage.h>
#include <kglobal.h>
#include <kstandarddirs.h>


#include <qdatastream.h>


BosonScript* BosonScript::mCurrentScript = 0;
BosonCanvas* BosonScript::mCanvas = 0;
Boson* BosonScript::mGame = 0;

BosonScript* BosonScript::newScriptParser(Language lang, Player* p)
{
  boDebug() << k_funcinfo << endl;
  BosonScript* s = 0;
  if(lang == Python)
  {
    s = new PythonScript(p);
  }
  else
  {
    boDebug() << k_funcinfo << "Invalid script language: " << lang << endl;
    s = 0;
  }
  return s;
}

BosonScript::BosonScript(Player* p)
{
  boDebug() << k_funcinfo << endl;
  mInterface = new BosonScriptInterface(0);
  mPlayer = p;
}

BosonScript::~BosonScript()
{
  boDebug() << k_funcinfo << endl;
  delete mInterface;
}

void BosonScript::sendInput(int playerId, QDataStream& stream)
{
  boDebug() << k_funcinfo << "PlayerID: " << playerId << endl;

  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return;
  }

  Player* p = (Player*)(game()->findPlayer(playerId));

  if(!p)
  {
    boError() << k_funcinfo << "No player with id " << playerId << endl;
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
  if(!player())
  {
    boError() << k_funcinfo << "NULL player" << endl;
    return -1;
  }

  return player()->id();
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
bool BosonScript::areEnemies(int playerId1, int playerId2)
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return false;  // What to return here?
  }

  Player* p1 = (Player*)(game()->findPlayer(playerId1));

  if(!p1)
  {
    boError() << k_funcinfo << "No player with id " << playerId1 << endl;
    return false;
  }

  Player* p2 = (Player*)(game()->findPlayer(playerId2));

  if(!p2)
  {
    boError() << k_funcinfo << "No player with id " << playerId1 << endl;
    return false;
  }

  return p1->isEnemy(p2);
}

QValueList<int> BosonScript::allPlayers()
{
  QValueList<int> players;

  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return players;
  }

  QPtrListIterator<KPlayer> it(*(game()->playerList()));
  for (; it.current(); ++it)
  {
    players.append(it.current()->id());
  }

  return players;
}

bool BosonScript::isNeutral(int playerId)
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return false;  // What to return here?
  }

  Player* p = (Player*)(game()->findPlayer(playerId));

  if(!p)
  {
    boError() << k_funcinfo << "No player with id " << playerId << endl;
    return false;
  }

  return p->isNeutralPlayer();
}

/*****  Resource methods  *****/
unsigned long int BosonScript::minerals(int playerId)
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return 0;
  }

  Player* p = (Player*)(game()->findPlayer(playerId));

  if(!p)
  {
    boError() << k_funcinfo << "No player with id " << playerId << endl;
    return 0;
  }

  return p->minerals();
}

void BosonScript::addMinerals(int playerId, int amount)
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return;
  }

  QByteArray b;
  QDataStream stream(b, IO_WriteOnly);
  stream << (Q_UINT32)playerId;
  stream << (Q_INT32)amount;
  game()->sendMessage(b, BosonMessage::IdModifyMinerals);
}

unsigned long int BosonScript::oil(int playerId)
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return 0;
  }

  Player* p = (Player*)(game()->findPlayer(playerId));

  if(!p)
  {
    boError() << k_funcinfo << "No player with id " << playerId << endl;
    return 0;
  }

  return p->oil();
}

void BosonScript::addOil(int playerId, int amount)
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return;
  }

  QByteArray b;
  QDataStream stream(b, IO_WriteOnly);
  stream << (Q_UINT32)playerId;
  stream << (Q_INT32)amount;
  game()->sendMessage(b, BosonMessage::IdModifyOil);
}

QValueList<BoVector2Fixed> BosonScript::nearestMineralLocations(int playerId, int x, int y, unsigned int n, unsigned int radius)
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return QValueList<BoVector2Fixed>();
  }

  Player* p = (Player*)(game()->findPlayer(playerId));

  if(!p)
  {
    boError() << k_funcinfo << "No player with id " << playerId << endl;
    return QValueList<BoVector2Fixed>();
  }

  return BosonPath::findLocations(p, x, y, n, radius, BosonPath::Minerals);
}

QValueList<BoVector2Fixed> BosonScript::nearestOilLocations(int playerId, int x, int y, unsigned int n, unsigned int radius)
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return QValueList<BoVector2Fixed>();
  }

  Player* p = (Player*)(game()->findPlayer(playerId));

  if(!p)
  {
    boError() << k_funcinfo << "No player with id " << playerId << endl;
    return QValueList<BoVector2Fixed>();
  }

  return BosonPath::findLocations(p, x, y, n, radius, BosonPath::Oil);
}


/*****  Unit methods  *****/
void BosonScript::moveUnit(int player, int id, float x, float y)
{
  QByteArray b;
  QDataStream stream(b, IO_WriteOnly);

  // tell the clients we want to move units:
  stream << (Q_UINT32)BosonMessage::MoveMove;
  // We want to move without attacking
  stream << (Q_UINT8)0;
  // tell them where to move to:
  stream << BoVector2Fixed(x, y);
  // tell them how many units:
  stream << (Q_UINT32)1;
  // Unit id
  stream << (Q_ULONG)id;

  QDataStream msg(b, IO_ReadOnly);
  sendInput(player, msg);
}

void BosonScript::moveUnitWithAttacking(int player, int id, float x, float y)
{
  QByteArray b;
  QDataStream stream(b, IO_WriteOnly);

  // tell the clients we want to move units:
  stream << (Q_UINT32)BosonMessage::MoveMove;
  // We want to move with attacking
  stream << (Q_UINT8)1;
  // tell them where to move to:
  stream << BoVector2Fixed(x, y);
  // tell them how many units:
  stream << (Q_UINT32)1;
  // Unit id
  stream << (Q_ULONG)id;

  QDataStream msg(b, IO_ReadOnly);
  sendInput(player, msg);
}

void BosonScript::attack(int player, int attackerId, int targetId)
{
  QByteArray b;
  QDataStream stream(b, IO_WriteOnly);

  // tell the clients we want to attack:
  stream << (Q_UINT32)BosonMessage::MoveAttack;
  // tell them which unit to attack:
  stream << (Q_ULONG)targetId;
  // tell them how many units attack:
  stream << (Q_UINT32)1;
  stream << (Q_ULONG)attackerId;

  QDataStream msg(b, IO_ReadOnly);
  sendInput(player, msg);
}

void BosonScript::stopUnit(int player, int id)
{
  QByteArray b;
  QDataStream stream(b, IO_WriteOnly);

  // tell the clients we want to move units:
  stream << (Q_UINT32)BosonMessage::MoveStop;
  // tell them how many units:
  stream << (Q_UINT32)1;
  stream << (Q_ULONG)id;

  QDataStream msg(b, IO_WriteOnly);
  sendInput(player, msg);
}

void BosonScript::mineUnit(int player, int id, float x, float y)
{
  // First we have to find resource mine
  // TODO: it sucks to do this here, perhaps we could have something like a
  //  MoveMineAt message which orders unit to move at specific point, not at
  //  some unit (then Boson would take care of finding resource mine unit).
  Unit* resourceUnit = canvas()->findUnitAt(BoVector3Fixed(x, y, 0.0));
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

  stream << (Q_UINT32)BosonMessage::MoveMine;
  stream << (Q_ULONG)id;
  stream << (Q_ULONG)resourceUnit->id();

  QDataStream msg(b, IO_ReadOnly);
  sendInput(player, msg);
}

void BosonScript::setUnitRotation(int player, int id, float rotation)
{
  QByteArray b;
  QDataStream stream(b, IO_WriteOnly);

  stream << (Q_UINT32)BosonMessage::MoveRotate;
  stream << (Q_UINT32)player;
  stream << (Q_ULONG)id;
  stream << rotation;

  QDataStream msg(b, IO_ReadOnly);
  sendInput(player, msg);
}

void BosonScript::dropBomb(int player, int id, int weapon, float x, float y)
{
  QByteArray b;
  QDataStream stream(b, IO_WriteOnly);

  stream << (Q_UINT32)BosonMessage::MoveDropBomb;
  // tell place
  stream << BoVector2Fixed(x, y);
  // tell them how many units attack:
  stream << (Q_UINT32)1;
  stream << (Q_UINT32)id;
  stream << (Q_UINT32)weapon;

  QDataStream msg(b, IO_ReadOnly);
  sendInput(player, msg);
}

void BosonScript::produceUnit(int player, int factory, int production)
{
  QByteArray b;
  QDataStream stream(b, IO_WriteOnly);

  stream << (Q_UINT32)BosonMessage::MoveProduce;
  stream << (Q_UINT32)ProduceUnit;
  stream << (Q_UINT32)player;
  stream << (Q_ULONG)factory;
  stream << (Q_UINT32)production;

  QDataStream msg(b, IO_ReadOnly);
  sendInput(player, msg);
}

void BosonScript::spawnUnit(int player, int type, float x, float y)
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return;
  }
  QByteArray b;
  QDataStream stream(b, IO_WriteOnly);

  stream << (Q_UINT32)BosonMessage::MovePlaceUnit;
  stream << (Q_UINT32)player;
  stream << (Q_UINT32)type;
  stream << BoVector2Fixed(x, y);

  QDataStream msg(b, IO_ReadOnly);
  sendInput(player, msg);
}

void BosonScript::teleportUnit(int player, int id, float x, float y)
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return;
  }
  QByteArray b;
  QDataStream stream(b, IO_WriteOnly);

  stream << (Q_UINT32)BosonMessage::MoveTeleport;
  stream << (Q_UINT32)player;
  stream << (Q_UINT32)id;
  stream << BoVector2Fixed(x, y);

  QDataStream msg(b, IO_ReadOnly);
  sendInput(player, msg);
}

void BosonScript::placeProduction(int player, int factoryid, float x, float y)
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return;
  }

  Player* p = (Player*)(game()->findPlayer(player));

  if(!p)
  {
    boError() << k_funcinfo << "No player with id " << player << endl;
    return;
  }

  Unit* u = game()->findUnit(factoryid, p);
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

  stream << (Q_UINT32)BosonMessage::MoveBuild;
  stream << (Q_UINT32)ProduceUnit;
  stream << (Q_ULONG)u->id();
  stream << (Q_UINT32)p->id();
  stream << BoVector2Fixed(x, y);

  QDataStream msg(b, IO_ReadOnly);
  sendInput(player, msg);
}

QValueList<int> BosonScript::unitsOnCell(int x, int y)
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

QValueList<int> BosonScript::unitsInRect(int x1, int y1, int x2, int y2)
{
  QValueList<int> list;
  if(!canvas())
  {
    boError() << k_funcinfo << "NULL canvas" << endl;
    return list;
  }
  BosonCollisions* c = canvas()->collisions();
  BoItemList* l = c->collisionsAtCells(BoRectFixed(BoVector2Fixed(x1, y1), BoVector2Fixed(x2, y2)));
  for(BoItemList::Iterator it = l->begin(); it != l->end(); ++it)
  {
    if(RTTI::isUnit((*it)->rtti()))
    {
      list.append(((Unit*)(*it))->id());
    }
  }
  return list;
}

bool BosonScript::cellOccupied(int x, int y)
{
  if(!canvas())
  {
    boError() << k_funcinfo << "NULL canvas" << endl;
    return false;
  }
  return canvas()->cellOccupied(x, y);
}

BoVector2Fixed BosonScript::unitPosition(int id)
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return BoVector2Fixed(-1, -1);
  }

  Unit* u = game()->findUnit(id, 0);
  if(!u)
  {
    boError() << k_funcinfo << "No unit with id" << id << endl;
    return BoVector2Fixed(-1, -1);
  }

  return BoVector2Fixed(u->x(), u->y());
}

int BosonScript::unitOwner(int id)
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return -1;
  }

  Unit* u = game()->findUnit(id, 0);
  if(!u)
  {
    boError() << k_funcinfo << "No unit with id" << id << endl;
    return -1;
  }

  return u->owner()->id();
}

int BosonScript::unitType(int id)
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return -1;
  }

  Unit* u = game()->findUnit(id, 0);
  if(!u)
  {
    boError() << k_funcinfo << "No unit with id" << id << endl;
    return -1;
  }

  return u->unitProperties()->typeId();
}

int BosonScript::unitWork(int id)
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return -1;
  }

  Unit* u = game()->findUnit(id, 0);
  if(!u)
  {
    boError() << k_funcinfo << "No unit with id" << id << endl;
    return -1;
  }

  return (int)u->work();
}

bool BosonScript::isUnitMobile(int id)
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return false;
  }

  Unit* u = game()->findUnit(id, 0);
  if(!u)
  {
    boError() << k_funcinfo << "No unit with id" << id << endl;
    return false;
  }

  return u->isMobile();
}

bool BosonScript::isUnitTypeMobile(int playerid, int type)
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return false;
  }

  Player* p = (Player*)(game()->findPlayer(playerid));

  if(!p)
  {
    boError() << k_funcinfo << "No player with id " << playerid << endl;
    return false;
  }

  const UnitProperties* prop = p->speciesTheme()->unitProperties(type);
  if(!prop)
  {
    boError() << k_funcinfo << "No unit properties with typeid " << type << " for player with id " << playerid << endl;
    return false;
  }

  return prop->isMobile();
}

bool BosonScript::isUnitAircraft(int id)
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return false;
  }

  Unit* u = game()->findUnit(id, 0);
  if(!u)
  {
    boError() << k_funcinfo << "No unit with id" << id << endl;
    return false;
  }

  return u->unitProperties()->isAircraft();
}

bool BosonScript::isUnitTypeAircraft(int playerid, int type)
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return false;
  }

  Player* p = (Player*)(game()->findPlayer(playerid));

  if(!p)
  {
    boError() << k_funcinfo << "No player with id " << playerid << endl;
    return false;
  }

  const UnitProperties* prop = p->speciesTheme()->unitProperties(type);
  if(!prop)
  {
    boError() << k_funcinfo << "No unit properties with typeid " << type << " for player with id " << playerid << endl;
    return false;
  }

  return prop->isAircraft();
}

bool BosonScript::canUnitShoot(int id)
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return false;
  }

  Unit* u = game()->findUnit(id, 0);
  if(!u)
  {
    boError() << k_funcinfo << "No unit with id" << id << endl;
    return false;
  }

  return u->unitProperties()->canShoot();
}

bool BosonScript::canUnitTypeShoot(int playerid, int type)
{
  QValueList<int> list;
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return false;
  }

  Player* p = (Player*)(game()->findPlayer(playerid));

  if(!p)
  {
    boError() << k_funcinfo << "No player with id " << playerid << endl;
    return false;
  }

  const UnitProperties* prop = p->speciesTheme()->unitProperties(type);
  if(!prop)
  {
    boError() << k_funcinfo << "No unit properties with typeid " << type << " for player with id " << playerid << endl;
    return false;
  }

  return prop->canShoot();
}

bool BosonScript::canUnitProduce(int id)
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return false;
  }

  Unit* u = game()->findUnit(id, 0);
  if(!u)
  {
    boError() << k_funcinfo << "No unit with id" << id << endl;
    return false;
  }

  return (u->plugin(UnitPlugin::Production));
}

bool BosonScript::canUnitMineMinerals(int id)
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return false;
  }

  Unit* u = game()->findUnit(id, 0);
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

bool BosonScript::canUnitTypeMineMinerals(int playerid, int type)
{
  QValueList<int> list;
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return false;
  }

  Player* p = (Player*)(game()->findPlayer(playerid));

  if(!p)
  {
    boError() << k_funcinfo << "No player with id " << playerid << endl;
    return false;
  }

  const UnitProperties* prop = p->speciesTheme()->unitProperties(type);
  if(!prop)
  {
    boError() << k_funcinfo << "No unit properties with typeid " << type << " for player with id " << playerid << endl;
    return false;
  }

  const HarvesterProperties* harvester = (const HarvesterProperties*)prop->properties(PluginProperties::Harvester);
  if(harvester && harvester->canMineMinerals())
  {
    return true;
  }
  return false;
}

bool BosonScript::canUnitMineOil(int id)
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return false;
  }

  Unit* u = game()->findUnit(id, 0);
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

bool BosonScript::canUnitTypeMineOil(int playerid, int type)
{
  QValueList<int> list;
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return false;
  }

  Player* p = (Player*)(game()->findPlayer(playerid));

  if(!p)
  {
    boError() << k_funcinfo << "No player with id " << playerid << endl;
    return false;
  }

  const UnitProperties* prop = p->speciesTheme()->unitProperties(type);
  if(!prop)
  {
    boError() << k_funcinfo << "No unit properties with typeid " << type << " for player with id " << playerid << endl;
    return false;
  }

  const HarvesterProperties* harvester = (const HarvesterProperties*)prop->properties(PluginProperties::Harvester);
  if(harvester && harvester->canMineOil())
  {
    return true;
  }
  return false;
}

QValueList<int> BosonScript::productionTypes(int id)
{
  QValueList<int> list;

  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return list;
  }

  Unit* u = game()->findUnit(id, 0);
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

bool BosonScript::isUnitAlive(int id)
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return false;
  }

  Unit* u = game()->findUnit(id, 0);
  if(!u)
  {
    return false;
  }

  return !u->isDestroyed();
}

QValueList<int> BosonScript::allPlayerUnits(int id)
{
  QValueList<int> list;
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return list;
  }

  Player* p = (Player*)(game()->findPlayer(id));

  if(!p)
  {
    boError() << k_funcinfo << "No player with id " << id << endl;
    return list;
  }

  QPtrListIterator<Unit> it(*(p->allUnits()));
  while(it.current())
  {
    if(!it.current()->isDestroyed())
    {
      list.append(it.current()->id());
    }
    ++it;
  }
  return list;
}

int BosonScript::allPlayerUnitsCount(int id)
{
  int count = 0;
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return count;
  }

  Player* p = (Player*)(game()->findPlayer(id));

  if(!p)
  {
    boError() << k_funcinfo << "No player with id " << id << endl;
    return count;
  }

  QPtrListIterator<Unit> it(*(p->allUnits()));
  while(it.current())
  {
    if(!it.current()->isDestroyed())
    {
      count++;
    }
    ++it;
  }
  return count;
}
QValueList<int> BosonScript::playerUnitsOfType(int playerid, int type)
{
  QValueList<int> list;
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return list;
  }

  Player* p = (Player*)(game()->findPlayer(playerid));

  if(!p)
  {
    boError() << k_funcinfo << "No player with id " << playerid << endl;
    return list;
  }

  QPtrListIterator<Unit> it(*(p->allUnits()));
  while(it.current())
  {
    if(!it.current()->isDestroyed())
    {
      if((int)it.current()->type() == type)
      {
        list.append(it.current()->id());
      }
    }
    ++it;
  }
  return list;
}

int BosonScript::playerUnitsOfTypeCount(int playerid, int type)
{
  int count = 0;
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return count;
  }

  Player* p = (Player*)(game()->findPlayer(playerid));

  if(!p)
  {
    boError() << k_funcinfo << "No player with id " << playerid << endl;
    return count;
  }

  QPtrListIterator<Unit> it(*(p->allUnits()));
  while(it.current())
  {
    if(!it.current()->isDestroyed())
    {
      if((int)it.current()->type() == type)
      {
        count++;
      }
    }
    ++it;
  }
  return count;
}


/*****  Camera methods  *****/

void BosonScript::setCameraRotation(float r)
{
  interface()->setCameraRotation(r);
}

void BosonScript::setCameraRadius(float r)
{
  interface()->setCameraRadius(r);
}

void BosonScript::setCameraZ(float z)
{
  interface()->setCameraZ(z);
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

float BosonScript::cameraRadius()
{
  return interface()->cameraRadius();
}

float BosonScript::cameraZ()
{
  return interface()->cameraZ();
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
  return boConfig->aiDelay();
}

/*****  Other methods  *****/
void BosonScript::startBenchmark()
{
  boProfiling->startBenchmark();
}

void BosonScript::endBenchmark(const QString& name)
{
  boProfiling->endBenchmark(name);
}

void BosonScript::setRandomSeed(long int seed)
{
  boGame->sendSystemMessage(seed, KGameMessage::IdSyncRandom);
}

void BosonScript::findPath(int x1, int y1, int x2, int y2)
{
#ifdef PATHFINDER_TNG
 boDebug() << k_funcinfo << "Trying searching script path" << endl;
 BosonPathInfo i;
 i.start = BoVector2Fixed(x1, y1);
 i.dest = BoVector2Fixed(x2, y2);
 boDebug() << k_funcinfo << "Let's go!" << endl;
 canvas()->pathfinder()->findPath(&i);
 boDebug() << k_funcinfo << "script path searching complete" << endl;
#else
 boWarning() << k_funcinfo << "findPath() is only available with new pathfinder!" << endl;
#endif
}

void BosonScript::addEffect(unsigned int id, BoVector3Fixed pos, bofixed zrot)
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return;
  }

  const BosonEffectProperties* prop = boEffectPropertiesManager->effectProperties(id);
  if(!prop)
  {
    boError() << k_funcinfo << "No effect properties with id " << id << endl;
    return;
  }
  QPtrList<BosonEffect> list = BosonEffectProperties::newEffects(prop, pos, BoVector3Fixed(0, 0, zrot));
  BosonCanvas* c = boGame->canvasNonConst();
  c->addEffects(list);
}

void BosonScript::addEffectToUnit(int unitid, unsigned int effectid)
{
  addEffectToUnit(unitid, effectid, BoVector3Fixed());
}

void BosonScript::addEffectToUnit(int unitid, unsigned int effectid, BoVector3Fixed offset, bofixed zrot)
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return;
  }

  Unit* u = game()->findUnit(unitid, 0);
  if(!u)
  {
    boError() << k_funcinfo << "No unit with id" << unitid << endl;
    return;
  }

  const BosonEffectProperties* prop = boEffectPropertiesManager->effectProperties(effectid);
  if(!prop)
  {
    boError() << k_funcinfo << "No effect properties with id " << effectid << endl;
    return;
  }
  BoVector3Fixed pos(u->centerX(), u->centerY(), u->z());
  pos += offset;
  BoVector3Fixed rot(u->xRotation(), u->yRotation(), u->rotation());
  rot.setZ(rot.z() + zrot);
  QPtrList<BosonEffect> list = BosonEffectProperties::newEffects(prop, pos, rot);
  QPtrListIterator<BosonEffect> it(list);
  while(it.current())
  {
    u->addEffect(*it);
    ++it;
  }
}

void BosonScript::advanceEffects(int ticks)
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return;
  }

  QPtrListIterator<BosonEffect> it(*game()->canvas()->effects());
  while(it.current())
  {
    BosonEffect* e = it.current();
    for(int i = 0; i < ticks; i++)
    {
      // This code is taken from BoCanvasAdvance::updateEffects()
      if(!e->hasStarted())
      {
        e->update(0.05f);
      }
      else
      {
        e->markUpdate(0.05f);
      }
    }
    ++it;
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

  Player* p = (Player*)(game()->findPlayer(playerid));

  if(!p)
  {
    boError() << k_funcinfo << "No player with id " << playerid << endl;
    return;
  }
  for(unsigned int x = 0; x < map->width(); x++)
  {
    for(unsigned int y = 0; y < map->height(); y++)
    {
      p->unfog(x, y);
    }
  }
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
  QPtrList<KPlayer> list = *game()->playerList();
  for(unsigned int i = 0; i < list.count(); i++)
  {
    Player* p = (Player*)list.at(i);
    for(unsigned int x = 0; x < map->width(); x++)
    {
      for(unsigned int y = 0; y < map->height(); y++)
      {
        p->unfog(x, y);
      }
    }
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


/*
 * vim: et sw=2
 */

