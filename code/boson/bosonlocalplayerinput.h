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

#ifndef BOSONLOCALPLAYERINPUT_H
#define BOSONLOCALPLAYERINPUT_H

#include <kgame/kgameio.h>

#include "global.h"

class BosonCommandFrameBase;
class BoSpecificAction;
class Unit;
class Player;

template<class T> class QPtrList;



/**
 * @short Class that handles all localplayer input
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonLocalPlayerInput : public KGameIO
{
  Q_OBJECT
  public:
    BosonLocalPlayerInput();
    virtual ~BosonLocalPlayerInput();

    virtual int rtti() const  { return 125; }; // just any unique number

    virtual void produceAction(const BoSpecificAction& action);


    void stopUnits(QPtrList<Unit> units);
    void layMine(const BoSpecificAction& action);
    void harvest(Unit* u, int x, int y);
    void moveWithoutAttack(QPtrList<Unit> units, int x, int y);
    void moveWithAttack(QPtrList<Unit> units, int x, int y);
    void build(ProductionType type, Unit* factory, int x, int y);
    void attack(QPtrList<Unit> units, Unit* target);
    void dropBomb(Unit* u, int weapon, int x, int y);
    void repair(QPtrList<Unit> units, Unit* repairyard);
    void refine(QPtrList<Unit> units, Unit* refinery);
    void follow(QPtrList<Unit> units, Unit* target);

    void placeUnit(Player* owner, unsigned long int unitType, int x, int y);
    void changeHeight(int x, int y, float height);

    virtual void setCommandFrame(BosonCommandFrameBase* cmdframe);


  protected slots:
    virtual void slotAction(const BoSpecificAction& action);

  signals:
    void signalAction(const BoSpecificAction& action);

  private:
    BosonCommandFrameBase* mCmdFrame;
};

#endif // BOSONLOCALPLAYERINPUT_H
