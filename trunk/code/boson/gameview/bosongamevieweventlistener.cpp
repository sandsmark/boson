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

#include "bosongamevieweventlistener.h"
#include "bosongamevieweventlistener.moc"

#include "../bosonprofiling.h"
#include "../gameengine/boevent.h"
#include "../gameengine/rtti.h"
#include "../gameengine/bosoncanvas.h"
#include "../gameengine/bosonitem.h"
#include "bodebug.h"

BosonGameViewEventListener::BosonGameViewEventListener(BoEventManager* manager, QObject* parent)
	: BoEventListener(manager, parent)
{
}

BosonGameViewEventListener::~BosonGameViewEventListener()
{
}

void BosonGameViewEventListener::setCanvas(const BosonCanvas* canvas)
{
 mCanvas = canvas;
}

void BosonGameViewEventListener::processEvent(const BoEvent* event)
{
 PROFILE_METHOD
 BO_CHECK_NULL_RET(mCanvas);
 if (event->name() == "FacilityWithTypeConstructed") {
	unsigned long int unitId = event->unitId();
	BosonItem* item = mCanvas->findItem(unitId);
	BO_CHECK_NULL_RET(item);
	if (!RTTI::isUnit(item->rtti())) {
		boError() << k_funcinfo << unitId << " is not a unit" << endl;
		return;
	}
	Unit* u = (Unit*)item;
	emit signalFacilityConstructed(u);
 }
}

