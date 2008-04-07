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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "bosonlocalplayerinput.h"
#include "bosonlocalplayerinput.moc"

#include "../bomemory/bodummymemory.h"
#include "../boaction.h"
#include "../gameengine/unit.h"
#include "../gameengine/unitplugins/harvesterplugin.h"
#include "../gameengine/unitplugins/resourcemineplugin.h"
#include "../gameengine/unitproperties.h"
#include "../gameengine/player.h"
#include "../gameengine/playerio.h"
#include "boselection.h"
#include "../gameengine/bosonmessage.h"
#include "../gameengine/bosonmessageids.h"
#include "bodebug.h"
#include "../gameengine/bosonweapon.h"
#include "../gameengine/boeventlistener.h"
#include "../gameengine/boson.h"
#include "bolocalplayereventlistener.h"

#include <q3ptrlist.h>
#include <qdatastream.h>
#include <qpoint.h>
#include <qpair.h>
#include <q3valuelist.h>

#include <krandomsequence.h>


// AB: because of the event listener the name "Input" is not correct anymore.
// this class still handles input, but also output (as it reports events)
BosonLocalPlayerInput::BosonLocalPlayerInput(bool gameMode) : KGameIO()
{
  mEventListener = 0;
  mGameMode = gameMode;
}

BosonLocalPlayerInput::~BosonLocalPlayerInput()
{
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
  if (!boGame)
  {
    BO_NULL_ERROR(boGame);
    return false;
  }
  else
  // warning: p->game() is NULL at this point. using boGame here is ugly.
  {
    // note: a NULL game() _is_ possible on program startup.
    // that player IO will be deleted later though, it is never really used
    PlayerIO* io = ((Player*)player())->playerIO();
    BoEventManager* manager = boGame->eventManager();

    if (mGameMode)
    {
      // AB: note that the event listener is neither loaded nor saved!
      //     -> only the script is loaded (by initScript()) and saved (by the
      //        manager), loadFromXML() is never called.
      //     TODO: is this a bug or do we intend this?
      mEventListener = new BoLocalPlayerEventListener(io, manager, this);
      connect(mEventListener, SIGNAL(signalShowMiniMap(bool)),
              this, SIGNAL(signalShowMiniMap(bool)));
      if (!mEventListener->initScript())
      {
        boError() << k_funcinfo << "could not init script" << endl;
        return false;
      }
    }
  }
  return true;
}

void BosonLocalPlayerInput::slotAction(const BoSpecificAction& action)
{
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
    return;
  }

  QByteArray b;
  QDataStream stream(&b, QIODevice::WriteOnly);
  bool ret = message->save(stream);
  int msgid = message->messageId();
  delete message;
  if (!ret)
  {
    boError() << k_funcinfo << "unable to save message (" << msgid << ")" << endl;
    return;
  }

  QDataStream msg(b);
  sendInput(msg);
}

void BosonLocalPlayerInput::stopUnits(const Q3PtrList<Unit>& units)
{
  boDebug() << k_funcinfo << endl;
  if (units.isEmpty())
  {
    boError() << k_funcinfo << "No units!" << endl;
    return;
  }
  Q3ValueList<quint32> stopUnits;
  Q3PtrListIterator<Unit> it(units);
  while (it.current())
  {
    stopUnits.append((quint32)it.current()->id());
    ++it;
  }

  BosonMessageMoveStop message(stopUnits);

  QByteArray b;
  QDataStream stream(&b, QIODevice::WriteOnly);
  if (!message.save(stream))
  {
    boError() << k_funcinfo << "unable to save message (" << message.messageId() << ")" << endl;
    return;
  }

  QDataStream msg(&b, QIODevice::WriteOnly);
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

  Q3ValueList<quint32> units;
  units.append(action.unit()->id());
  Q3ValueList<quint32> weapons;
  weapons.append(action.weapon()->id());
  BosonMessageMoveLayMine message(units, weapons);

  QByteArray b;
  QDataStream stream(&b, QIODevice::WriteOnly);
  if (!message.save(stream))
  {
    boError() << k_funcinfo << "unable to save message (" << message.messageId() << ")" << endl;
    return;
  }

  QDataStream msg(b);
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
  QDataStream stream(&b, QIODevice::WriteOnly);
  if (!message.save(stream))
  {
    boError() << k_funcinfo << "unable to save message (" << message.messageId() << ")" << endl;
    return;
  }

  QDataStream msg(b);
  sendInput(msg);
}

void BosonLocalPlayerInput::moveWithoutAttack(const Q3PtrList<Unit>& units, bofixed x, bofixed y)
{
  boDebug() << k_funcinfo << endl;
  if (units.isEmpty())
  {
    return;
  }
  Q3ValueList<quint32> moveUnits;
  Q3PtrListIterator<Unit> it(units);
  while (it.current())
  {
    moveUnits.append(it.current()->id());
    ++it;
  }

  BosonMessageMoveMove message(false, BoVector2Fixed(x, y), moveUnits);

  QByteArray b;
  QDataStream stream(&b, QIODevice::WriteOnly);
  if (!message.save(stream))
  {
    boError() << k_funcinfo << "unable to save message (" << message.messageId() << ")" << endl;
    return;
  }

  QDataStream msg(b);
  sendInput(msg);

  emit signalMoveUnitsTo(units, BoVector2Fixed(x, y), false);
}

void BosonLocalPlayerInput::moveWithAttack(const Q3PtrList<Unit>& units, bofixed x, bofixed y)
{
  // FIXME: maybe moveWithAttack() and moveWithoutAttack() should be merged to
  //  single move() with bool attack param
  boDebug() << k_funcinfo << endl;
  if (units.isEmpty())
  {
    return;
  }
  Q3ValueList<quint32> moveUnits;
  Q3PtrListIterator<Unit> it(units);
  while (it.current())
  {
    moveUnits.append(it.current()->id());
    ++it;
  }

  BosonMessageMoveMove message(true, BoVector2Fixed(x, y), moveUnits);

  QByteArray b;
  QDataStream stream(&b, QIODevice::WriteOnly);
  if (!message.save(stream))
  {
    boError() << k_funcinfo << "unable to save message (" << message.messageId() << ")" << endl;
    return;
  }

  QDataStream msg(b);
  sendInput(msg);

  emit signalMoveUnitsTo(units, BoVector2Fixed(x, y), true);
}

void BosonLocalPlayerInput::build(ProductionType type, Unit* factory, bofixed x, bofixed y)
{
  boDebug() << k_funcinfo << endl;
  if (!factory)
  {
    return;
  }

  BosonMessageMoveBuild message(type, factory->owner()->bosonId(), factory->id(), BoVector2Fixed(x, y));

  QByteArray b;
  QDataStream stream(&b, QIODevice::WriteOnly);
  if (!message.save(stream))
  {
    boError() << k_funcinfo << "unable to save message (" << message.messageId() << ")" << endl;
    return;
  }

  QDataStream msg(b);
  sendInput(msg);
}

void BosonLocalPlayerInput::attack(const Q3PtrList<Unit>& units, Unit* target)
{
  boDebug() << k_funcinfo << endl;
  if (!target)
  {
    return;
  }
  if (units.isEmpty())
  {
    return;
  }

  Q3ValueList<quint32> attackUnits;
  Q3PtrListIterator<Unit> it(units);
  while (it.current())
  {
    attackUnits.append((quint32)it.current()->id());
    ++it;
  }
  BosonMessageMoveAttack message(target->id(), attackUnits);
  QByteArray b;
  QDataStream stream(&b, QIODevice::WriteOnly);
  if (!message.save(stream))
  {
    boError() << k_funcinfo << "unable to save message (" << message.messageId() << ")" << endl;
    return;
  }

  QDataStream msg(b);
  sendInput(msg);

  emit signalAttackUnit(units, target);
}

void BosonLocalPlayerInput::dropBomb(Unit* u, int weapon, bofixed x, bofixed y)
{
  boDebug() << k_funcinfo << endl;
  if (!u)
  {
    return;
  }

  Q3ValueList<quint32> units;
  Q3ValueList<quint32> weapons;
  units.append(u->id());
  weapons.append(weapon);
  BosonMessageMoveDropBomb message(BoVector2Fixed(x, y), units, weapons);

  QByteArray b;
  QDataStream stream(&b, QIODevice::WriteOnly);
  if (!message.save(stream))
  {
    boError() << k_funcinfo << "unable to save message (" << message.messageId() << ")" << endl;
    return;
  }

  QDataStream msg(b);
  sendInput(msg);
}

void BosonLocalPlayerInput::repair(const Q3PtrList<Unit>& units, Unit* repairyard)
{
  if (!repairyard)
  {
    return;
  }
  if (units.isEmpty())
  {
    return;
  }
  // TODO
}

void BosonLocalPlayerInput::refine(const Q3PtrList<Unit>& units, Unit* refinery)
{
  BO_CHECK_NULL_RET(refinery);
  boDebug() << k_funcinfo << endl;
  if (!refinery)
  {
    return;
  }
  if (units.isEmpty())
  {
    return;
  }

  Q3ValueList<quint32> refineUnits;
  Q3PtrListIterator<Unit> it(units);
  while (it.current())
  {
    refineUnits.append((quint32)it.current()->id());
    ++it;
  }

  BosonMessageMoveRefine message(refinery->owner()->bosonId(), refinery->id(), refineUnits);

  QByteArray b;
  QDataStream stream(&b, QIODevice::WriteOnly);
  if (!message.save(stream))
  {
    boError() << k_funcinfo << "unable to save message (" << message.messageId() << ")" << endl;
    return;
  }

  QDataStream msg(b);
  sendInput(msg);
}

void BosonLocalPlayerInput::follow(const Q3PtrList<Unit>& units, Unit* target)
{
  boDebug() << k_funcinfo << endl;
  if (!target)
  {
    return;
  }
  if (units.isEmpty())
  {
    return;
  }

  Q3ValueList<quint32> followUnits;
  Q3PtrListIterator<Unit> it(units);
  while (it.current())
  {
    followUnits.append((quint32)it.current()->id());
    ++it;
  }

  BosonMessageMoveFollow message(target->id(), followUnits);

  QByteArray b;
  QDataStream stream(&b, QIODevice::WriteOnly);
  if (!message.save(stream))
  {
    boError() << k_funcinfo << "unable to save message (" << message.messageId() << ")" << endl;
    return;
  }

  QDataStream msg(b);
  sendInput(msg);
}

void BosonLocalPlayerInput::enterUnit(const Q3PtrList<Unit>& units, Unit* target)
{
  boDebug() << k_funcinfo << endl;
  if (!target)
  {
    return;
  }
  if (units.isEmpty())
  {
    return;
  }

  Q3ValueList<quint32> enterUnits;
  for (Q3PtrListIterator<Unit> it(units); it.current(); ++it)
  {
    enterUnits.append((quint32)it.current()->id());
  }

  BosonMessageMoveEnterUnit message(target->id(), enterUnits);

  QByteArray b;
  QDataStream stream(&b, QIODevice::WriteOnly);
  if (!message.save(stream))
  {
    boError() << k_funcinfo << "unable to save message (" << message.messageId() << ")" << endl;
    return;
  }

  QDataStream msg(b);
  sendInput(msg);
}

void BosonLocalPlayerInput::placeUnit(Player* owner, quint32 unitType, bofixed x, bofixed y)
{
  BO_CHECK_NULL_RET(owner);
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
  BosonMessageEditorMovePlaceUnit message(unitType, owner->bosonId(), BoVector2Fixed(x, y), rotation);

  QByteArray b;
  QDataStream stream(&b, QIODevice::WriteOnly);
  if (!message.save(stream))
  {
    boError() << k_funcinfo << "unable to save message (" << message.messageId() << ")" << endl;
    return;
  }

  QDataStream msg(b);
  sendInput(msg);
}

void BosonLocalPlayerInput::changeHeight(int x, int y, bofixed height)
{
  Q3ValueList< QPair<QPoint, bofixed> > heights;
  heights.append(QPair<QPoint, bofixed>(QPoint(x, y), height));
  changeHeight(heights);
}

void BosonLocalPlayerInput::changeHeight(const Q3ValueList< QPair<QPoint, bofixed> >& heights)
{
  Q3ValueVector<quint32> cornersX(heights.count());
  Q3ValueVector<quint32> cornersY(heights.count());
  Q3ValueVector<bofixed> cornersHeight(heights.count());
  Q3ValueList< QPair<QPoint, bofixed> >::const_iterator it;
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
  QDataStream stream(&b, QIODevice::WriteOnly);
  if (!message.save(stream))
  {
    boError() << k_funcinfo << "unable to save message (" << message.messageId() << ")" << endl;
    return;
  }

  QDataStream msg(b);
  sendInput(msg);
}

/*
 * vim: et sw=2
 */
