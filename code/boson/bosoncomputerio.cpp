/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2003 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include "bosonconfig.h"
#include "bosonprofiling.h"
#include "script/bosonscript.h"

#include <kgame/kgame.h>
#include <kstandarddirs.h>

#include <qpoint.h>

#include "bosoncomputerio.moc"


BosonComputerIO::BosonComputerIO() : KGameComputerIO()
{
 boDebug() << k_funcinfo << endl;
 // advance() method in script is called every advance call now, so it's
 //  script's job to to something useful only every aidelay() advance calls.
 boDebug() << k_funcinfo << "aidelay: " << boConfig->aiDelay() << endl;
 if (boConfig->aiDelay() != 0.0) {
	setReactionPeriod(1);
 }
 mScript = 0;
}

BosonComputerIO::BosonComputerIO(KPlayer* p) : KGameComputerIO(p)
{
 boDebug() << k_funcinfo << endl;
}

BosonComputerIO::~BosonComputerIO()
{
 if (mScript) {
	delete mScript;
 }
}

void BosonComputerIO::initScript()
{
 if (!player()) {
	boError() << k_funcinfo << "No player set!!!" << endl;
	return;
 }

 boDebug() << k_funcinfo << "Player id: " << boPlayer()->id() << endl;

 mScript = BosonScript::newScriptParser(BosonScript::Python, boPlayer());
 mScript->loadScript(locate("data", "boson/scripts/boson-script.py"));
 mScript->init();
}

void BosonComputerIO::reaction()
{
 if (boConfig->aiDelay() == 0.0) {
	return;
 }
 static int id = boProfiling->requestEventId("BosonComputerIO::reaction()");
 BosonProfiler p(id);

 if (mScript) {
	mScript->advance();
 }
}
