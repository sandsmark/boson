/*
    This file is part of the Boson game
    Copyright (C) 2003-2005 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bosonlocalplayerinput.h"
#include "bosonlocalplayerinput.moc"

#include "../boaction.h"
#include "../unit.h"
#include "../player.h"
#include "../boselection.h"
#include "../bosonmessage.h"
#include "../bosonmessageids.h"
#include "bodebug.h"
#include "../bosonweapon.h"
#include "../boeventlistener.h"
#include "../boson.h"

#include <qptrlist.h>
#include <qdatastream.h>
#include <qpoint.h>


// AB: because of the event listener the name "Input" is not correct anymore.
// this class still handles input, but also output (as it reports events)
BosonLocalPlayerInput::BosonLocalPlayerInput() : KGameIO()
{
  boDebug() << k_funcinfo << endl;
  mEventListener = 0;
}

BosonLocalPlayerInput::~BosonLocalPlayerInput()
{
  boDebug() << k_funcinfo << endl;
  delete mEventListener;
}

bool BosonLocalPlayerInput::initializeIO()
{
  delete mEventListener;
  mEventListener = 0;
  if (!player())
  {
    BO_NULL_ERROR(player());
    return false;
  }
  boDebug() << k_funcinfo << endl;
  if (game())
  {
    // note: a NULL game() _is_ possible on program startup.
    // that player IO will be deleted later though, it is never really used
    PlayerIO* io = ((Player*)player())->playerIO();
    BoEventManager* manager = ((Boson*)game())->eventManager();
    mEventListener = new BoLocalPlayerEventListener(io, manager, this);
    if (!mEventListener->initScript())
    {
      boError() << k_funcinfo << "could not init script" << endl;
      return false; // TODO: return false
    }
  }
  return true;
}

void BosonLocalPlayerInput::slotAction(const BoSpecificAction& action)
{
  boDebug() << k_funcinfo << "Action type: " << action.type() << endl;
  if (action.isProduceAction())
  {
    produceAction(action);
    return;
  }

  switch (action.type())
  {
    case ActionStop:
      stopUnits(action.allUnits());
      break;
    case ActionLayMine:
      layMine(action);
      break;
    default:
      // Other actions will be handled in BosonGameViewInput
      boDebug() << k_funcinfo << "Emitting signalAction" << endl;
      emit signalAction(action);
      break;
  }
}

void BosonLocalPlayerInput::produceAction(const BoSpecificAction& action)
{
  if(!action.unit())
  {
    boError() << k_funcinfo << "NULL unit" << endl;
    return;
  }
  if(!action.productionOwner())
  {
    boError() << k_funcinfo << "NULL production owner" << endl;
    return;
  }
  boDebug() << k_funcinfo << endl;
  QByteArray b;
  QDataStream stream(b, IO_WriteOnly);

  // FIXME: is there any way not to hardcode this?
  if(action.type() == ActionStopProduceUnit || action.type() == ActionStopProduceTech)
  {
    stream << (Q_UINT32)BosonMessageIds::MoveProduceStop;
  }
  else
  {
    stream << (Q_UINT32)BosonMessageIds::MoveProduce;
  }

  stream << (Q_UINT32)action.productionType();
  stream << (Q_UINT32)action.productionOwner()->id();
  stream << (Q_ULONG)action.unit()->id();
  stream << (Q_UINT32)action.productionId();

  QDataStream msg(b, IO_ReadOnly);
  sendInput(msg);
}

void BosonLocalPlayerInput::stopUnits(const QPtrList<Unit>& units)
{
  boDebug() << k_funcinfo << endl;
  if (units.isEmpty())
  {
    boError() << k_funcinfo << "No units!" << endl;
    return;
  }
  QPtrListIterator<Unit> it(units);
  QByteArray b;
  QDataStream stream(b, IO_WriteOnly);

  // tell the clients we want to move units:
  stream << (Q_UINT32)BosonMessageIds::MoveStop;
  // tell them how many units:
  stream << (Q_UINT32)units.count();
  while (it.current())
  {
    // tell them which unit to move:
    stream << (Q_ULONG)it.current()->id();
    ++it;
  }

  QDataStream msg(b, IO_WriteOnly);
  sendInput(msg);
}

void BosonLocalPlayerInput::layMine(const BoSpecificAction& action)
{
  if(!action.unit())
  {
    boError() << k_funcinfo << "NULL unit" << endl;
    return;
  }
  if(!action.weapon())
  {
    boError() << k_funcinfo << "NULL weapon" << endl;
    return;
  }

  boDebug() << k_funcinfo << endl;
  QByteArray b;
  QDataStream stream(b, IO_WriteOnly);
  stream << (Q_UINT32)BosonMessageIds::MoveLayMine;
  stream << (Q_UINT32)1;
  stream << (Q_ULONG)action.unit()->id();
  stream << (Q_ULONG)action.weapon()->id();

  QDataStream msg(b, IO_ReadOnly);
  sendInput(msg);
}

void BosonLocalPlayerInput::harvest(const HarvesterPlugin* harvester, const ResourceMinePlugin* mine)
{
  BO_CHECK_NULL_RET(harvester);
  BO_CHECK_NULL_RET(harvester->unit());
  BO_CHECK_NULL_RET(mine);
  BO_CHECK_NULL_RET(mine->unit());
  boDebug() << k_funcinfo << endl;
  QByteArray b;
  QDataStream stream(b, IO_WriteOnly);

  stream << (Q_UINT32)BosonMessageIds::MoveMine;
  stream << (Q_ULONG)harvester->unit()->id();
  stream << (Q_ULONG)mine->unit()->id();

  QDataStream msg(b, IO_ReadOnly);
  sendInput(msg);
}

void BosonLocalPlayerInput::moveWithoutAttack(const QPtrList<Unit>& units, bofixed x, bofixed y)
{
  boDebug() << k_funcinfo << endl;
  QByteArray b;
  QDataStream stream(b, IO_WriteOnly);

  QPtrListIterator<Unit> it(units);
  // tell the clients we want to move units:
  stream << (Q_UINT32)BosonMessageIds::MoveMove;
  // We want to move without attacking
  stream << (Q_UINT8)0;
  // tell them where to move to:
  stream << BoVector2Fixed(x, y);
  // tell them how many units:
  stream << (Q_UINT32)units.count();
  Unit* unit = 0;
  while (it.current())
  {
    if (!unit)
    {
      unit = it.current();
    }
    // tell them which unit to move:
    stream << (Q_ULONG)it.current()->id(); // MUST BE UNIQUE!
    ++it;
  }

  QDataStream msg(b, IO_ReadOnly);
  sendInput(msg);
}

void BosonLocalPlayerInput::moveWithAttack(const QPtrList<Unit>& units, bofixed x, bofixed y)
{
  boDebug() << k_funcinfo << endl;
  // FIXME: maybe moveWithAttack() and moveWithoutAttack() should be merged to
  //  single move() with bool attack param
  QByteArray b;
  QDataStream stream(b, IO_WriteOnly);

  QPtrListIterator<Unit> it(units);
  // tell the clients we want to move units:
  stream << (Q_UINT32)BosonMessageIds::MoveMove;
  // We want to move with attacking
  stream << (Q_UINT8)1;
  // tell them where to move to:
  stream << BoVector2Fixed(x, y);
  // tell them how many units:
  stream << (Q_UINT32)units.count();
  Unit* unit = 0;
  while (it.current())
  {
    if (!unit)
    {
      unit = it.current();
    }
    // tell them which unit to move:
    stream << (Q_ULONG)it.current()->id(); // MUST BE UNIQUE!
    ++it;
  }

  QDataStream msg(b, IO_ReadOnly);
  sendInput(msg);
}

void BosonLocalPlayerInput::build(ProductionType type, Unit* factory, bofixed x, bofixed y)
{
  boDebug() << k_funcinfo << endl;
  QByteArray b;
  QDataStream stream(b, IO_WriteOnly);

  stream << (Q_UINT32)BosonMessageIds::MoveBuild;
  stream << (Q_UINT32)type;
  stream << (Q_ULONG)factory->id();
  stream << (Q_UINT32)factory->owner()->id();
  stream << BoVector2Fixed(x, y);

  QDataStream msg(b, IO_ReadOnly);
  sendInput(msg);
}

void BosonLocalPlayerInput::attack(const QPtrList<Unit>& units, Unit* target)
{
  boDebug() << k_funcinfo << endl;
  QByteArray b;
  QDataStream stream(b, IO_WriteOnly);

  QPtrListIterator<Unit> it(units);
  // tell the clients we want to attack:
  stream << (Q_UINT32)BosonMessageIds::MoveAttack;
  // tell them which unit to attack:
  stream << (Q_ULONG)target->id();
  // tell them how many units attack:
  stream << (Q_UINT32)units.count();
  while (it.current())
  {
    // tell them which unit is going to attack:
    stream << (Q_ULONG)it.current()->id(); // MUST BE UNIQUE!
    ++it;
  }

  QDataStream msg(b, IO_ReadOnly);
  sendInput(msg);
}

void BosonLocalPlayerInput::dropBomb(Unit* u, int weapon, bofixed x, bofixed y)
{
  boDebug() << k_funcinfo << endl;
  QByteArray b;
  QDataStream stream(b, IO_WriteOnly);

  // tell the clients we want to drop bomb:
  stream << (Q_UINT32)BosonMessageIds::MoveDropBomb;
  // tell place
  stream << BoVector2Fixed(x, y);
  // tell them how many units attack:
  stream << (Q_UINT32)1;
  stream << (Q_UINT32)u->id();
  stream << (Q_UINT32)weapon;

  QDataStream msg(b, IO_ReadOnly);
  sendInput(msg);
}

void BosonLocalPlayerInput::repair(const QPtrList<Unit>& units, Unit* repairyard)
{
  boDebug() << k_funcinfo << endl;
  QByteArray b;
  QDataStream stream(b, IO_WriteOnly);

  QPtrListIterator<Unit> it(units);
  // tell the clients we want to repair:
  stream << (Q_UINT32)BosonMessageIds::MoveRepair;
  // the owner of the repairyard (can also be an allied
  // player - not localplayer only)
  stream << (Q_UINT32)repairyard->owner()->id();
  // tell them where to repair the units:
  stream << (Q_ULONG)repairyard->id();
  // tell them how many units to be repaired:
  stream << (Q_UINT32)units.count();
  while (it.current())
  {
    // tell them which unit is going be repaired:
    stream << (Q_ULONG)it.current()->id();
    ++it;
  }

  QDataStream msg(b, IO_ReadOnly);
  sendInput(msg);
}

void BosonLocalPlayerInput::refine(const QPtrList<Unit>& units, Unit* refinery)
{
  boDebug() << k_funcinfo << endl;
  QByteArray b;
  QDataStream stream(b, IO_WriteOnly);

  QPtrListIterator<Unit> it(units);
  stream << (Q_UINT32)BosonMessageIds::MoveRefine;
  // the owner of the refinery (can also be an allied
  // player - not localplayer only)
  stream << (Q_UINT32)refinery->owner()->id();
  // destination:
  stream << (Q_ULONG)refinery->id();
  // how many units go to the refinery
  stream << (Q_UINT32)units.count();
  while (it.current())
  {
    // tell them which unit goes there
    stream << (Q_ULONG)it.current()->id();
    ++it;
  }

  QDataStream msg(b, IO_ReadOnly);
  sendInput(msg);
}

void BosonLocalPlayerInput::follow(const QPtrList<Unit>& units, Unit* target)
{
  boDebug() << k_funcinfo << endl;
  QByteArray b;
  QDataStream stream(b, IO_WriteOnly);

  QPtrListIterator<Unit> it(units);
  // tell the clients we want to follow:
  stream << (Q_UINT32)BosonMessageIds::MoveFollow;
  // tell them which unit to follow:
  stream << (Q_ULONG)target->id();
  // tell them how many units follow:
  stream << (Q_UINT32)units.count();
  while (it.current())
  {
    // tell them which unit is going to follow:
    stream << (Q_ULONG)it.current()->id(); // MUST BE UNIQUE!
    ++it;
  }

  QDataStream msg(b, IO_ReadOnly);
  sendInput(msg);
}

void BosonLocalPlayerInput::placeUnit(Player* owner, unsigned long int unitType, bofixed x, bofixed y)
{
  boDebug() << k_funcinfo << endl;

  // editor message
  BosonMessageMovePlaceUnit message(unitType, owner->id(), BoVector2Fixed(x, y));

  QByteArray b;
  QDataStream stream(b, IO_WriteOnly);
  if (!message.save(stream))
  {
    boError() << k_funcinfo << "unable to save message (" << message.messageId() << ")" << endl;
    return;
  }

  QDataStream msg(b, IO_ReadOnly);
  sendInput(msg);
}

void BosonLocalPlayerInput::changeHeight(int x, int y, bofixed height)
{
  boDebug() << k_funcinfo << endl;

  // editor message
  QValueVector<Q_UINT32> cornersX(1);
  QValueVector<Q_UINT32> cornersY(1);
  QValueVector<bofixed> cornersHeight(1);
  cornersX[0] = x;
  cornersY[0] = y;
  cornersHeight[0] = height;
  BosonMessageMoveChangeHeight message(cornersX, cornersY, cornersHeight);

  QByteArray b;
  QDataStream stream(b, IO_WriteOnly);
  if (!message.save(stream))
  {
    boError() << k_funcinfo << "unable to save message (" << message.messageId() << ")" << endl;
    return;
  }

  QDataStream msg(b, IO_ReadOnly);
  sendInput(msg);
}

/*
 * vim: et sw=2
 */
