/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include "bosoncomputerio.h"

#include "player.h"
#include "bodebug.h"
#include "unit.h"
#include "unitproperties.h"

#include <kgame/kgame.h>

#include <qpoint.h>

#include "bosoncomputerio.moc"

BosonComputerIO::BosonComputerIO() : KGameComputerIO()
{
 boDebug() << k_funcinfo << endl;
 setReactionPeriod(40);
 mUnit = -1;
 mTarget = 0l;
}

BosonComputerIO::BosonComputerIO(KPlayer* p) : KGameComputerIO(p)
{
 boDebug() << k_funcinfo << endl;
}

BosonComputerIO::~BosonComputerIO()
{
}

void BosonComputerIO::reaction()
{
 boDebug() << k_funcinfo << endl;

 if(!mTarget || mTarget->isDestroyed()) {
	mTarget = findTarget();
	if(!mTarget) {
		boDebug() << k_funcinfo << "No enemies left" << endl;
		return;
	}
 }

 QPtrList<Unit>* units = boPlayer()->allUnits();
 Unit* attacker = 0l;
 Unit* u;

 while(!attacker) {
	mUnit++;

	if(mUnit >= (int)units->count()) {
		mUnit = -1;
		return;
	}

	u = units->at(mUnit);
	if(u->isMobile() && u->unitProperties()->canShoot()) {
		attacker = u;
	}
 }

 boDebug() << k_funcinfo << "Sending " << mUnit << ". unit with id " << u->id() << " to attack" << endl;
 attacker->moveTo(QPoint((int)mTarget->x(), (int)mTarget->y()), true);
}

Unit* BosonComputerIO::findTarget()
{
 QPtrListIterator<KPlayer> it(*(boPlayer()->game()->playerList()));
 Unit* u = 0l;
 for (; it.current(); ++it) {
	Player* p = (Player*)it.current();
	if (boPlayer()->isEnemy(p)) {
		QPtrListIterator<Unit> it(*(p->allUnits()));
		// First try to find enemy's command center
		for (; it.current(); ++it) {
			if (it.current()->isDestroyed()) {
				continue;
			}
			// FIXME: command center id is hardcoded
			if (it.current()->unitProperties()->typeId() == 5) {
				return it.current();
			}
			if (!u) {
				u = it.current();
			}
		}
		// if there's no command center, find any other unit
		return u;
	}
 }
 return 0l;
}

