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

#include "bosonscript.h"

#include "../bo3dtools.h"
#include "../bocamera.h"
#include "../boautocamera.h"
#include "../bosonbigdisplaybase.h"
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
#include "../bolight.h"
#include "../speciestheme.h"
#include "../bosonparticlesystemproperties.h"
#include "../playerio.h"
#include "../pluginproperties.h"
#include "bodebug.h"

#include "pythonscript.h"

#include <kgame/kgamemessage.h>

#include <qdatastream.h>
#include <qpoint.h>


BosonBigDisplayBase* BosonScript::mDisplay = 0;
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
  mPlayer = p;
}

BosonScript::~BosonScript()
{
  boDebug() << k_funcinfo << endl;
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

BoGameCamera* BosonScript::camera()
{
  if(!display())
  {
    boError() << k_funcinfo << "NULL display" << endl;
    return 0;
  }

  return display()->camera();
}

BoAutoGameCamera* BosonScript::autoCamera()
{
  if(!display())
  {
    boError() << k_funcinfo << "NULL display" << endl;
    return 0;
  }

  return display()->autoCamera();
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

BoLight* BosonScript::light(int id)
{
  if(!display())
  {
    boError() << k_funcinfo << "NULL display" << endl;
    return 0;
  }

  return display()->light(id);
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

QValueList<QPoint> BosonScript::nearestMineralLocations(int playerId, int x, int y, unsigned int n, unsigned int radius)
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return QValueList<QPoint>();
  }

  Player* p = (Player*)(game()->findPlayer(playerId));

  if(!p)
  {
    boError() << k_funcinfo << "No player with id " << playerId << endl;
    return QValueList<QPoint>();
  }

  return BosonPath::findLocations(p, x, y, n, radius, BosonPath::Minerals);
}

QValueList<QPoint> BosonScript::nearestOilLocations(int playerId, int x, int y, unsigned int n, unsigned int radius)
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return QValueList<QPoint>();
  }

  Player* p = (Player*)(game()->findPlayer(playerId));

  if(!p)
  {
    boError() << k_funcinfo << "No player with id " << playerId << endl;
    return QValueList<QPoint>();
  }

  return BosonPath::findLocations(p, x, y, n, radius, BosonPath::Oil);
}


/*****  Unit methods  *****/
void BosonScript::moveUnit(int player, int id, int x, int y)
{
  QByteArray b;
  QDataStream stream(b, IO_WriteOnly);

  // tell the clients we want to move units:
  stream << (Q_UINT32)BosonMessage::MoveMove;
  // We want to move without attacking
  stream << (Q_UINT8)0;
  // tell them where to move to:
  stream << QPoint(x, y);
  // tell them how many units:
  stream << (Q_UINT32)1;
  // Unit id
  stream << (Q_ULONG)id;

  QDataStream msg(b, IO_ReadOnly);
  sendInput(player, msg);
}

void BosonScript::moveUnitWithAttacking(int player, int id, int x, int y)
{
  QByteArray b;
  QDataStream stream(b, IO_WriteOnly);

  // tell the clients we want to move units:
  stream << (Q_UINT32)BosonMessage::MoveMove;
  // We want to move with attacking
  stream << (Q_UINT8)1;
  // tell them where to move to:
  stream << QPoint(x, y);
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

void BosonScript::mineUnit(int player, int id, int x, int y)
{
  QByteArray b;
  QDataStream stream(b, IO_WriteOnly);

  stream << (Q_UINT32)BosonMessage::MoveMine;
  stream << (Q_ULONG)id;
  stream << QPoint(x, y);

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

void BosonScript::dropBomb(int player, int id, int weapon, int x, int y)
{
  QByteArray b;
  QDataStream stream(b, IO_WriteOnly);

  stream << (Q_UINT32)BosonMessage::MoveDropBomb;
  // tell place
  stream << (Q_INT32)x;
  stream << (Q_INT32)y;
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

void BosonScript::spawnUnit(int player, int type, int x, int y)
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
  stream << (Q_INT32)x;
  stream << (Q_INT32)y;

  QDataStream msg(b, IO_ReadOnly);
  sendInput(player, msg);
}

void BosonScript::teleportUnit(int player, int id, int x, int y)
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
  stream << (Q_INT32)x;
  stream << (Q_INT32)y;

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
  BoItemList* l = c->collisionsAtCells(QRect(QPoint(x1, y1), QPoint(x2, y2)));
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

QPoint BosonScript::unitPosition(int id)
{
  if(!game())
  {
    boError() << k_funcinfo << "NULL game" << endl;
    return QPoint(-1, -1);
  }

  Unit* u = game()->findUnit(id, 0);
  if(!u)
  {
    boError() << k_funcinfo << "No unit with id" << id << endl;
    return QPoint(-1, -1);
  }

  return QPoint((int)u->x(), (int)u->y());
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

/*****  Camera methods  *****/

void BosonScript::moveCamera(const BoVector3& pos)
{
  autoCamera()->setLookAt(pos);
}

void BosonScript::moveCameraBy(const BoVector3& pos)
{
  autoCamera()->changeLookAt(pos);
}

void BosonScript::setCameraRotation(float r)
{
  autoCamera()->setRotation(r);
}

void BosonScript::setCameraRadius(float r)
{
  autoCamera()->setRadius(r);
}

void BosonScript::setCameraZ(float z)
{
  autoCamera()->setZ(z);
}

void BosonScript::setCameraMoveMode(int mode)
{
  boDebug() << k_funcinfo << "mode: " << mode << endl;
  autoCamera()->setMoveMode((BoAutoCamera::MoveMode)mode);
}

void BosonScript::commitCameraChanges(int ticks)
{
  autoCamera()->commitChanges(ticks);
}

BoVector3 BosonScript::cameraPos()
{
  return camera()->lookAt();
}

float BosonScript::cameraRotation()
{
  return camera()->rotation();
}

float BosonScript::cameraRadius()
{
  return camera()->radius();
}

float BosonScript::cameraZ()
{
  return camera()->z();
}

/*****  Light methods  *****/
BoVector4 BosonScript::lightPos(int id)
{
  BoLight* l = light(id);
  if(!l)
  {
    boError() << k_funcinfo << "No light with id " << id << endl;
    return BoVector4();
  }

  return l->position();
}

BoVector4 BosonScript::lightAmbient(int id)
{
  BoLight* l = light(id);
  if(!l)
  {
    boError() << k_funcinfo << "No light with id " << id << endl;
    return BoVector4();
  }

  return l->ambient();
}

BoVector4 BosonScript::lightDiffuse(int id)
{
  BoLight* l = light(id);
  if(!l)
  {
    boError() << k_funcinfo << "No light with id " << id << endl;
    return BoVector4();
  }

  return l->diffuse();
}

BoVector4 BosonScript::lightSpecular(int id)
{
  BoLight* l = light(id);
  if(!l)
  {
    boError() << k_funcinfo << "No light with id " << id << endl;
    return BoVector4();
  }

  return l->specular();
}

BoVector3 BosonScript::lightAttenuation(int id)
{
  BoLight* l = light(id);
  if(!l)
  {
    boError() << k_funcinfo << "No light with id " << id << endl;
    return BoVector3();
  }

  return l->attenuation();
}

bool BosonScript::lightEnabled(int id)
{
  BoLight* l = light(id);
  if(!l)
  {
    boError() << k_funcinfo << "No light with id " << id << endl;
    return false;
  }

  return l->isEnabled();
}

void BosonScript::setLightPos(int id, BoVector4 pos)
{
  BoLight* l = light(id);
  if(!l)
  {
    boError() << k_funcinfo << "No light with id " << id << endl;
    return;
  }

  l->setPosition(pos);
}

void BosonScript::setLightAmbient(int id, BoVector4 a)
{
  BoLight* l = light(id);
  if(!l)
  {
    boError() << k_funcinfo << "No light with id " << id << endl;
    return;
  }

  l->setAmbient(a);
}

void BosonScript::setLightDiffuse(int id, BoVector4 d)
{
  BoLight* l = light(id);
  if(!l)
  {
    boError() << k_funcinfo << "No light with id " << id << endl;
    return;
  }

  l->setDiffuse(d);
}

void BosonScript::setLightSpecular(int id, BoVector4 s)
{
  BoLight* l = light(id);
  if(!l)
  {
    boError() << k_funcinfo << "No light with id " << id << endl;
    return;
  }

  l->setSpecular(s);
}

void BosonScript::setLightAttenuation(int id, BoVector3 a)
{
  BoLight* l = light(id);
  if(!l)
  {
    boError() << k_funcinfo << "No light with id " << id << endl;
    return;
  }

  l->setAttenuation(a);
}

void BosonScript::setLightEnabled(int id, bool enable)
{
  BoLight* l = light(id);
  if(!l)
  {
    boError() << k_funcinfo << "No light with id " << id << endl;
    return;
  }

  l->setEnabled(enable);
}

int BosonScript::addLight()
{
  if(!display())
  {
    boError() << k_funcinfo << "NULL display" << endl;
    return -1;
  }
  BoLight* l = display()->newLight();
  if(!l)
  {
    return -1;
  }
  return l->id();
}

void BosonScript::removeLight(int id)
{
  if(!display())
  {
    boError() << k_funcinfo << "NULL display" << endl;
    return;
  }
  display()->removeLight(id);
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
 i.start = QPoint(x1, y1);
 i.dest = QPoint(x2, y2);
 boDebug() << k_funcinfo << "Let's go!" << endl;
 canvas()->pathfinder()->findPath(&i);
 boDebug() << k_funcinfo << "script path searching complete" << endl;
#else
 boWarning() << k_funcinfo << "findPath() is only available with new pathfinder!" << endl;
#endif
}

void BosonScript::addParticleSystem(int player, unsigned int id, BoVector3 pos, float rot)
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

  const BosonParticleSystemProperties* prop = p->speciesTheme()->particleSystemProperties(id);
  if(!prop)
  {
    boError() << k_funcinfo << "No particle system properties with id " << id << " for player " << player << endl;
    return;
  }
  BosonParticleSystem* s = prop->newSystem(pos, rot);
  BosonCanvas* c = boGame->canvasNonConst();
  c->addParticleSystem(s);
}

