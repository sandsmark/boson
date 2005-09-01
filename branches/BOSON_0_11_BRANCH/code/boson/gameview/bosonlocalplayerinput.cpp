/*
    This file is part of the Boson game
    Copyright (C) 2003-2005 Andreas Beckermann (b_mann@gmx.de)
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

#include "bosonlocalplayerinput.h"
#include "bosonlocalplayerinput.moc"

#include "../bomemory/bodummymemory.h"
#include "../boaction.h"
#include "../unit.h"
#include "../unitproperties.h"
#include "../player.h"
#include "../playerio.h"
#include "../boselection.h"
#include "../bosonmessage.h"
#include "../bosonmessageids.h"
#include "bodebug.h"
#include "../bosonweapon.h"
#include "../boeventlistener.h"
#include "../boson.h"
#include "bolocalplayereventlistener.h"

#include <qptrlist.h>
#include <qdatastream.h>
#include <qpoint.h>
#include <qpair.h>
#include <qvaluelist.h>

#include <krandomsequence.h>


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
    connect(mEventListener, SIGNAL(signalShowMiniMap(bool)),
            this, SIGNAL(signalShowMiniMap(bool)));
    if (!mEventListener->initScript())
    {
      boError() << k_funcinfo << "could not init script" << endl;
      return false;
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

  BosonMessage* message = 0;

  // FIXME: is there any way not to hardcode this?
  if(action.type() == ActionStopProduceUnit || action.type() == ActionStopProduceTech)
  {
    message = new BosonMessageMoveProduceStop(action.productionType(), action.productionOwner()->playerId(), action.unit()->id(), action.productionId());
  }
  else
  {
    message = new BosonMessageMoveProduce(action.productionType(), action.productionOwner()->playerId(), action.unit()->id(), action.productionId());
  }

  if (!message)
  {
    BO_NULL_ERROR(message);
  }

  QByteArray b;
  QDataStream stream(b, IO_WriteOnly);
  bool ret = message->save(stream);
  int msgid = message->messageId();
  delete message;
  if (!ret)
  {
    boError() << k_funcinfo << "unable to save message (" << msgid << ")" << endl;
    return;
  }

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
  QValueList<Q_ULONG> stopUnits;
  QPtrListIterator<Unit> it(units);
  while (it.current())
  {
    stopUnits.append((Q_ULONG)it.current()->id());
    ++it;
  }

  BosonMessageMoveStop message(stopUnits);

  QByteArray b;
  QDataStream stream(b, IO_WriteOnly);
  if (!message.save(stream))
  {
    boError() << k_funcinfo << "unable to save message (" << message.messageId() << ")" << endl;
    return;
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

  QValueList<Q_ULONG> units;
  units.append(action.unit()->id());
  QValueList<Q_ULONG> weapons;
  weapons.append(action.weapon()->id());
  BosonMessageMoveLayMine message(units, weapons);

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

void BosonLocalPlayerInput::harvest(const HarvesterPlugin* harvester, const ResourceMinePlugin* mine)
{
  BO_CHECK_NULL_RET(harvester);
  BO_CHECK_NULL_RET(harvester->unit());
  BO_CHECK_NULL_RET(mine);
  BO_CHECK_NULL_RET(mine->unit());
  boDebug() << k_funcinfo << endl;

  BosonMessageMoveMine message(harvester->unit()->id(), mine->unit()->id());

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

void BosonLocalPlayerInput::moveWithoutAttack(const QPtrList<Unit>& units, bofixed x, bofixed y)
{
  boDebug() << k_funcinfo << endl;
  QValueList<Q_ULONG> moveUnits;
  QPtrListIterator<Unit> it(units);
  while (it.current())
  {
    moveUnits.append(it.current()->id());
    ++it;
  }

  BosonMessageMoveMove message(false, BoVector2Fixed(x, y), moveUnits);

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

void BosonLocalPlayerInput::moveWithAttack(const QPtrList<Unit>& units, bofixed x, bofixed y)
{
  // FIXME: maybe moveWithAttack() and moveWithoutAttack() should be merged to
  //  single move() with bool attack param
  boDebug() << k_funcinfo << endl;
  QValueList<Q_ULONG> moveUnits;
  QPtrListIterator<Unit> it(units);
  while (it.current())
  {
    moveUnits.append(it.current()->id());
    ++it;
  }

  BosonMessageMoveMove message(true, BoVector2Fixed(x, y), moveUnits);

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

void BosonLocalPlayerInput::build(ProductionType type, Unit* factory, bofixed x, bofixed y)
{
  boDebug() << k_funcinfo << endl;

  BosonMessageMoveBuild message(type, factory->owner()->id(), factory->id(), BoVector2Fixed(x, y));

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

void BosonLocalPlayerInput::attack(const QPtrList<Unit>& units, Unit* target)
{
  boDebug() << k_funcinfo << endl;

  QValueList<Q_ULONG> attackUnits;
  QPtrListIterator<Unit> it(units);
  while (it.current())
  {
    attackUnits.append((Q_ULONG)it.current()->id());
    ++it;
  }
  BosonMessageMoveAttack message(target->id(), attackUnits);
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

void BosonLocalPlayerInput::dropBomb(Unit* u, int weapon, bofixed x, bofixed y)
{
  boDebug() << k_funcinfo << endl;

  QValueList<Q_ULONG> units;
  QValueList<Q_ULONG> weapons;
  units.append(u->id());
  weapons.append(weapon);
  BosonMessageMoveDropBomb message(BoVector2Fixed(x, y), units, weapons);

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

void BosonLocalPlayerInput::repair(const QPtrList<Unit>& units, Unit* repairyard)
{
  // TODO
}

void BosonLocalPlayerInput::refine(const QPtrList<Unit>& units, Unit* refinery)
{
  boDebug() << k_funcinfo << endl;

  QValueList<Q_ULONG> refineUnits;
  QPtrListIterator<Unit> it(units);
  while (it.current())
  {
    refineUnits.append((Q_ULONG)it.current()->id());
    ++it;
  }

  BosonMessageMoveRefine message(refinery->owner()->id(), refinery->id(), refineUnits);

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

void BosonLocalPlayerInput::follow(const QPtrList<Unit>& units, Unit* target)
{
  boDebug() << k_funcinfo << endl;

  QValueList<Q_ULONG> followUnits;
  QPtrListIterator<Unit> it(units);
  while (it.current())
  {
    followUnits.append((Q_ULONG)it.current()->id());
    ++it;
  }

  BosonMessageMoveFollow message(target->id(), followUnits);

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

void BosonLocalPlayerInput::placeUnit(Player* owner, unsigned long int unitType, bofixed x, bofixed y)
{
  boDebug() << k_funcinfo << endl;

  bofixed rotation = 0;
  if (owner->unitProperties(unitType))
  {
    if (owner->unitProperties(unitType)->isMobile())
    {
#warning FIXME: this will break network
      // TODO: we need a separate random object, that does not influence the
      // random numbers/the seed of the random object of the game
      //
      // atm this is not high priority, because placeUnit() can be called in
      // editor mode only anyway.
      rotation = bofixed(boGame->random()->getLong(359));
    }
  }
  BosonMessageEditorMovePlaceUnit message(unitType, owner->id(), BoVector2Fixed(x, y), rotation);

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

  QValueList< QPair<QPoint, bofixed> > heights;
  heights.append(QPair<QPoint, bofixed>(QPoint(x, y), height));
  changeHeight(heights);
}

void BosonLocalPlayerInput::changeHeight(const QValueList< QPair<QPoint, bofixed> >& heights)
{
  boDebug() << k_funcinfo << endl;

  QValueVector<Q_UINT32> cornersX(heights.count());
  QValueVector<Q_UINT32> cornersY(heights.count());
  QValueVector<bofixed> cornersHeight(heights.count());
  QValueList< QPair<QPoint, bofixed> >::const_iterator it;
  int i = 0;
  for (it = heights.begin(); it != heights.end(); ++it)
  {
    cornersX[i] = (*it).first.x();
    cornersY[i] = (*it).first.y();
    cornersHeight[i] = (*it).second;

    i++;
  }
  BosonMessageEditorMoveChangeHeight message(cornersX, cornersY, cornersHeight);

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
