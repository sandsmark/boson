/*
    This file is part of the Boson game
    Copyright (C) 2005 Rivo Laks (rivolaks@hot.ee)

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

#include "player.h"
#include "player.moc"

#include "bodebug.h"

#include <qcolor.h>

#include <kgame/kgamepropertyhandler.h>


Player::Player() : KPlayer()
{
  setAsyncInput(true);
  connect(this, SIGNAL(signalNetworkData(int, const QByteArray&, Q_UINT32, KPlayer*)),
      this, SLOT(slotNetworkData(int, const QByteArray&, Q_UINT32, KPlayer*)));

  KGamePropertyBase* propName = dataHandler()->find(KGamePropertyBase::IdName);
  if(propName)
  {
    propName->setPolicy(KGamePropertyBase::PolicyClean);
  }
  else
  {
    boError() << k_funcinfo << "can't find name property" << endl;
  }
  // TODO d->mFogged.registerData() or something like this
  mMinerals.registerData(IdMinerals, dataHandler(),
      KGamePropertyBase::PolicyLocal, "MineralCost");
  mOil.registerData(IdOil, dataHandler(),
      KGamePropertyBase::PolicyLocal, "OilCost");
  mIsNeutralPlayer.registerData(IdIsNeutralPlayer, dataHandler(),
      KGamePropertyBase::PolicyLocal, "IsNeutralPlayer");

  mUnitCount = 0;
}

Player::~Player()
{
}

void Player::slotNetworkData(int msgid, const QByteArray& msg, Q_UINT32 sender, KPlayer*)
{
}

bool Player::load(QDataStream& stream)
{
  boDebug() << k_funcinfo << endl;
  if (!KPlayer::load(stream))
  {
    boError() << k_funcinfo << "Couldn't load KPlayer" << endl;
    return false;
  }

  // Load speciestheme
  QString themeIdentifier;
  QColor teamColor;
  stream >> themeIdentifier;
  stream >> teamColor;

  // Load unitpropID
  Q_UINT32 unitPropId;
  stream >> unitPropId;

  return true;
}

bool Player::save(QDataStream& stream)
{
  boError() << k_funcinfo << endl;
  return true;
}

void Player::unitDestroyed()
{
  mUnitCount--;
}

void Player::unitProduced()
{
  mUnitCount++;
}

