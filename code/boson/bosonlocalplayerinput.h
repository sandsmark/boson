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
#include "bomath.h"

class BoSpecificAction;
class Unit;
class Player;
class HarvesterPlugin;
class ResourceMinePlugin;
class BoEventListener;

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
    enum _LocalPlayerInputRTTI {
      LocalPlayerInputRTTI = 125 // just any unique number
    };
  public:
    BosonLocalPlayerInput();
    virtual ~BosonLocalPlayerInput();

    virtual void initIO(KPlayer*);

    BoEventListener* eventListener() const  { return mEventListener; }

    virtual int rtti() const  { return LocalPlayerInputRTTI; }

    virtual void produceAction(const BoSpecificAction& action);


    void stopUnits(const QPtrList<Unit>& units);
    void layMine(const BoSpecificAction& action);
    void harvest(const HarvesterPlugin* harvester, const ResourceMinePlugin* mine);
    void moveWithoutAttack(const QPtrList<Unit>& units, bofixed x, bofixed y);
    void moveWithAttack(const QPtrList<Unit>& units, bofixed x, bofixed y);
    void build(ProductionType type, Unit* factory, bofixed x, bofixed y);
    void attack(const QPtrList<Unit>& units, Unit* target);
    void dropBomb(Unit* u, int weapon, bofixed x, bofixed y);
    void repair(const QPtrList<Unit>& units, Unit* repairyard);
    void refine(const QPtrList<Unit>& units, Unit* refinery);
    void follow(const QPtrList<Unit>& units, Unit* target);

    void placeUnit(Player* owner, unsigned long int unitType, bofixed x, bofixed y);
    void changeHeight(int x, int y, bofixed height);

  protected slots:
    virtual void slotAction(const BoSpecificAction& action);

  signals:
    void signalAction(const BoSpecificAction& action);

  private:
    BoEventListener* mEventListener;
};

#endif // BOSONLOCALPLAYERINPUT_H

/*
 * vim: et sw=2
 */
