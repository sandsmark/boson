/*
    This file is part of the Boson game
    Copyright (C) 2001-2005 Andreas Beckermann (b_mann@gmx.de)

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

#include "../bomemory/bodummymemory.h"
#include "player.h"
#include "bodebug.h"
#include "unit.h"
#include "unitproperties.h"
#include "bosonconfig.h"
#include "bosonprofiling.h"
#include "script/bosonscript.h"
#include "boson.h"
#include "boeventlistener.h"

#include <kgame/kgame.h>
#include <kstandarddirs.h>

#include <qpoint.h>

#include "bosoncomputerio.moc"


BosonComputerIO::BosonComputerIO() : KGameComputerIO()
{
 boDebug() << k_funcinfo << endl;
 // advance() method in script is called every advance call now, so it's
 //  script's job to to something useful only every aidelay() advance calls.
 boDebug() << k_funcinfo << "aidelay: " << boConfig->doubleValue("AIDelay") << endl;
 if (boConfig->doubleValue("AIDelay") != 0.0) {
	setReactionPeriod(1);
 }
 mEventListener = 0;
}

BosonComputerIO::~BosonComputerIO()
{
 delete mEventListener;
}

bool BosonComputerIO::initializeIO()
{
 delete mEventListener;
 mEventListener = 0;
 if (!player()) {
	BO_NULL_ERROR(player());
	return false;
 }

 // warning: p->game() is NULL at this point. using boGame here is ugly.
 boDebug() << k_funcinfo << endl;
 if (!boGame) {
	BO_NULL_ERROR(boGame);
	return false;
 }
 BoEventManager* manager = boGame->eventManager();

 // AB: note that the listener is neither loaded nor saved currently.
 //     only the script is loaded (initScript()) and saved (by the manager), but
 //     loadFromXML() is never called.
 //     TODO: is this a bug or do we intend this?
 mEventListener = new BoComputerPlayerEventListener((Player*)player(), manager, this);
 if (!mEventListener->initScript()) {
	boError() << k_funcinfo << "could not init script" << endl;
	return false;
 }
 return true;
}

void BosonComputerIO::reaction()
{
 if (boConfig->doubleValue("AIDelay") == 0.0) {
	return;
 }
 BosonProfiler p("BosonComputerIO::reaction()");
}

