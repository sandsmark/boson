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
#include "bodebug.h"

#include "pythonscript.h"

#include <qdatastream.h>
#include <qpoint.h>


BosonScript* BosonScript::mScript = 0;


BosonScript* BosonScript::newScriptParser(Language lang)
{
  boDebug() << k_funcinfo << endl;
  if(mScript)
  {
    boError() << k_funcinfo << "script already created" << endl;
    return mScript;
  }
  BosonScript* s = 0;
  if(lang == Python)
  {
    s = new PythonScript();
  }
  else
  {
    boDebug() << k_funcinfo << "Invalid script language: " << lang << endl;
    s = 0;
  }
  mScript = s;
  return s;
}

BosonScript::BosonScript()
{
  boDebug() << k_funcinfo << endl;
  mDisplay = 0;
  mPlayer = 0;
  mCanvas = 0;
}

BosonScript::~BosonScript()
{
  boDebug() << k_funcinfo << endl;
  mScript = 0;
}

void BosonScript::deleteBosonScript()
{
 delete mScript;
 mScript = 0;
}

void BosonScript::sendInput(QDataStream& stream)
{
  if(!player())
  {
    boError() << k_funcinfo << "Player not set!" << endl;
    return;
  }

  player()->forwardInput(stream);
}

Boson* BosonScript::game() const
{
  if(!player())
  {
    boError() << k_funcinfo << "NULL player" << endl;
    return 0;
  }

  return (Boson*)player()->game();
}

BoCamera* BosonScript::camera() const
{
  if(!display())
  {
    boError() << k_funcinfo << "NULL display" << endl;
    return 0;
  }

  return display()->camera();
}

/*****  Player methods  *****/

bool BosonScript::areEnemies(int playerId1, int playerId2) const
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

int BosonScript::playerId() const
{
  if(!player())
  {
    boError() << k_funcinfo << "NULL player" << endl;
    return -1;
  }

  return player()->id();
}

QValueList<int> BosonScript::allPlayers() const
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
unsigned long int BosonScript::mineralsAmount() const
{
  if(!player())
  {
    boError() << k_funcinfo << "NULL player" << endl;
    return 0;
  }
  return player()->minerals();
}

unsigned long int BosonScript::oilAmount() const
{
  if(!player())
  {
    boError() << k_funcinfo << "NULL player" << endl;
    return 0;
  }
  return player()->oil();
}

/*****  Unit methods  *****/
void BosonScript::moveUnit(int id, int x, int y)
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
  sendInput(msg);
}

void BosonScript::moveUnitWithAttacking(int id, int x, int y)
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
  sendInput(msg);
}

void BosonScript::attack(int attackerId, int targetId)
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
  sendInput(msg);
}

void BosonScript::stopUnit(int id)
{
  QByteArray b;
  QDataStream stream(b, IO_WriteOnly);

  // tell the clients we want to move units:
  stream << (Q_UINT32)BosonMessage::MoveStop;
  // tell them how many units:
  stream << (Q_UINT32)1;
  stream << (Q_ULONG)id;

  QDataStream msg(b, IO_WriteOnly);
  sendInput(msg);
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
  BoItemList* l = c->collisionsAtCells(QRect(x1, y1, x2, y2));
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

bool BosonScript::isMyUnit(int id)
{
  if(!player())
  {
    boError() << k_funcinfo << "NULL player" << endl;
    return false;
  }

  return (player()->findUnit(id));
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

QValueList<int> BosonScript::allMyUnits()
{
  QValueList<int> list;
  if(!player())
  {
    boError() << k_funcinfo << "NULL player" << endl;
    return list;
  }

  QPtrListIterator<Unit> it(*(player()->allUnits()));
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

void BosonScript::moveCamera(BoVector3 pos)
{
  camera()->setLookAt(pos);
}

void BosonScript::moveCameraBy(BoVector3 pos)
{
  camera()->changeLookAt(pos);
}

void BosonScript::setCameraRotation(float r)
{
  camera()->setRotation(r);
}

void BosonScript::setCameraRadius(float r)
{
  camera()->setRadius(r);
}

void BosonScript::setCameraZ(float z)
{
  camera()->setZ(z);
}

void BosonScript::setCameraMoveMode(int mode)
{
  boDebug() << k_funcinfo << "mode: " << mode << endl;
  camera()->setMoveMode((BoCamera::MoveMode)mode);
}

void BosonScript::commitCameraChanges(int ticks)
{
  camera()->commitChanges(ticks);
}

BoVector3 BosonScript::cameraPos() const
{
  return camera()->lookAt();
}

float BosonScript::cameraRotation() const
{
  return camera()->rotation();
}

float BosonScript::cameraRadius() const
{
  return camera()->radius();
}

float BosonScript::cameraZ() const
{
  return camera()->z();
}

/*****  AI methods  *****/
float BosonScript::aiDelay() const
{
  return boConfig->aiDelay();
}

