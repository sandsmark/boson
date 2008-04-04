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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef PLAYER_H
#define PLAYER_H


#include <qcstring.h>

#include <kgame/kplayer.h>
#include <kgame/kgameproperty.h>

class QDataStream;


class Player : public KPlayer
{
  Q_OBJECT
  public:
    enum PropertyIds {
      IdFogged = KGamePropertyBase::IdUser + 1,
      IdMinerals = KGamePropertyBase::IdUser + 2,
      IdOil = KGamePropertyBase::IdUser + 3,
      IdIsNeutralPlayer = KGamePropertyBase::IdUser + 4,
      IdOutOfGame = KGamePropertyBase::IdUser + 5,
      IdHasLost = KGamePropertyBase::IdUser + 6,
      IdHasWon = KGamePropertyBase::IdUser + 7
    };

    Player();
    ~Player();

    virtual bool load(QDataStream& stream);
    virtual bool save(QDataStream& stream);

    unsigned int unitCount() const  { return mUnitCount; }
    void setUnitCount(unsigned int c)  { mUnitCount = c; }

  public slots:
    void slotNetworkData(int msgid, const QByteArray& msg, Q_UINT32 sender, KPlayer*);

  private:
    KGameProperty<unsigned long int> mMinerals;
    KGameProperty<unsigned long int> mOil;
    KGamePropertyBool mIsNeutralPlayer;
    KGameProperty<Q_UINT8> mOutOfGame;
    KGameProperty<Q_UINT8> mHasLost;
    KGameProperty<Q_UINT8> mHasWon;

    unsigned int mUnitCount;
};

#endif
