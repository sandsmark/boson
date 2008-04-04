/*
    This file is part of the Boson game
    Copyright (C) 2005 Andreas Beckermann (b_mann@gmx.de)

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

#include "bocommandframeeventlistener.h"
#include "bocommandframeeventlistener.moc"

#include "../../../bomemory/bodummymemory.h"
#include "../../gameengine/playerio.h"
#include "../../gameengine/boevent.h"
#include <bodebug.h>

class BoCommandFrameEventListenerPrivate
{
public:
	BoCommandFrameEventListenerPrivate()
	{
	}
};

BoCommandFrameEventListener::BoCommandFrameEventListener(PlayerIO* io, BoEventManager* manager, QObject* parent)
	: BoEventListener(manager, parent)
{
 d = new BoCommandFrameEventListenerPrivate;
 mPlayerIO = io;
}

BoCommandFrameEventListener::~BoCommandFrameEventListener()
{
 delete d;
}

bool BoCommandFrameEventListener::canSee(const BoEvent* event) const
{
 if (!playerIO()) {
	BO_NULL_ERROR(playerIO());
	return false;
 }
 return playerIO()->canSee(event->location());
}

void BoCommandFrameEventListener::processEvent(const BoEvent* event)
{
 if (event->playerId() == 0) {
	return;
 }
 if (event->playerId() != playerIO()->playerId()) {
	return;
 }
 if (event->name() == "FacilityWithTypeConstructed") {
	// production options may have changed - e.g. a new factory got
	// constructed
	emit signalFacilityConstructed(event->unitId());

	emit signalUpdateProductionOptions();
 } else if (event->name() == "ProducedUnitWithTypePlaced") {
	// FIXME: the original implementation did this in the MobileUnit c'tor.
	// why is that necessary for mobile units?
	// or is that rather of theoretical use? (-> a mobile unit with a
	// factory plugin)
	emit signalUpdateProductionOptions();

	// might be a production completed
	emit signalUpdateProduction(event->data2().toULong());
 } else if (event->name() == "TechnologyWithTypeResearched") {
	emit signalUpdateSelection();
	emit signalUpdateProductionOptions();

	// might be a production completed
	emit signalUpdateProduction(event->data2().toULong());
 } else if (event->name() == "UnitWithTypeDestroyed") {
	emit signalUnitDestroyed(event->unitId());
	emit signalUpdateProductionOptions();
 } else if (event->name() == "StartProductionOfUnitWithType" ||
		event->name() == "StartProductionOfTechnologyWithType" ||
		event->name() == "PauseProductionOfUnitWithType" ||
		event->name() == "PauseProductionOfTechnologyWithType" ||
		event->name() == "ContinueProductionOfUnitWithType" ||
		event->name() == "ContinueProductionOfTechnologyWithType" ||
		event->name() == "StopProductionOfUnitWithType" ||
		event->name() == "StopProductionOfTechnologyWithType") {
	emit signalUpdateProduction(event->data2().toULong());
 }
}

