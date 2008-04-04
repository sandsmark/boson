/*
    This file is part of the Boson game
    Copyright (C) 2004-2005 Andreas Beckermann (b_mann@gmx.de)

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

#include "bolocalplayereventlistener.h"
#include "bolocalplayereventlistener.moc"

#include "../../bomemory/bodummymemory.h"
#include "bodebug.h"
#include "../gameengine/boevent.h"
#include "../gameengine/boson.h"
#include "../gameengine/playerio.h"
#include "../gameengine/speciestheme.h"
#include "../speciesdata.h"
#include "../gameengine/unitproperties.h"
#include "../gameengine/script/bosonscript.h"
#include "../bosonprofiling.h"
#include "../bosonviewdata.h"

#include <klocale.h>


BoLocalPlayerEventListener::BoLocalPlayerEventListener(PlayerIO* io, BoEventManager* manager, QObject* parent)
	: BoEventListener(manager, parent)
{
 mPlayerIO = io;
}

BoLocalPlayerEventListener::~BoLocalPlayerEventListener()
{
}

BosonScript* BoLocalPlayerEventListener::createScriptParser() const
{
 int playerId = -1;
 if (!playerIO()) {
	BO_NULL_ERROR(playerIO());
	// really bad error.
	// fortunately it should be impossible anyway :)
 } else {
	playerId = playerIO()->playerId();
 }
 return BosonScript::newScriptParser(BosonScript::Python, playerId);
}

void BoLocalPlayerEventListener::processEvent(const BoEvent* event)
{
 PROFILE_METHOD
 BO_CHECK_NULL_RET(playerIO());
 BO_CHECK_NULL_RET(playerIO()->speciesTheme());
 if (event->playerId() != 0) {
	if (event->playerId() != playerIO()->playerId()) {
		return;
	}
 }
 if (event->name() == "UnitWithTypeProduced") {
	bool ok;
	unsigned long int unitType = event->data1().toULong(&ok);
	if (!ok) {
		boError(360) << k_funcinfo << "data1 parameter for UnitWithTypeProduced event is not a valid number: " << event->data1() << endl;
		return;
	}
	const UnitProperties* prop = playerIO()->unitProperties(unitType);
	if (!prop) {
		boError(360) << k_funcinfo << "cannot find unittype " << unitType << " specified in UnitWithTypeProduced" << endl;
		return;
	}
	if (prop->isFacility()) {
		boGame->slotAddChatSystemMessage(
				i18n("A %1 has been produced - place it on the map to start the construction!").arg(prop->name()),
				playerIO()->player());
	} else {
		// actually it has been placed already, but who cares
		// for so much preciseness ;)
		// --> this way we dont say something wrong when placing
		// the unit failed
		boGame->slotAddChatSystemMessage(
				i18n("A %1 has been produced and will be placed on the map now").arg(prop->name()),
				playerIO()->player());
	}
 } else if (event->name() == "LostMinimap") {
	boViewData->speciesData(playerIO()->speciesTheme())->playSound(SoundReportMinimapDeactivated);
	emit signalShowMiniMap(false);
 } else if (event->name() == "GainedMinimap") {
	boViewData->speciesData(playerIO()->speciesTheme())->playSound(SoundReportMinimapActivated);
	emit signalShowMiniMap(true);
 } else if (event->name() == "AllUnitsDestroyed") {
 }
}

bool BoLocalPlayerEventListener::canSee(const BoEvent* event) const
{
 return mPlayerIO->canSee(event->location());
}

