/*
    This file is part of the Boson game
    Copyright (C) 2002-2006 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2002-2006 Rivo Laks (rivolaks@hot.ee)

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
#include "unitplugin.h"

#include "../../bomemory/bodummymemory.h"
#include "../defines.h"
#include "unit.h"
#include "player.h"
#include "bo3dtools.h"
#include "bodebug.h"

#include <math.h>

UnitPlugin::UnitPlugin(Unit* unit)
{
 mUnit = unit;
}

UnitPlugin::~UnitPlugin()
{
}

SpeciesTheme* UnitPlugin::speciesTheme() const
{
 return unit()->speciesTheme();
}

Player* UnitPlugin::player() const
{
 return unit()->owner();
}

const UnitProperties* UnitPlugin::unitProperties() const
{
 return unit()->unitProperties();
}

const PluginProperties* UnitPlugin::properties(int propertyType) const
{
 return unit()->properties(propertyType);
}

KGamePropertyHandler* UnitPlugin::dataHandler() const
{
 return unit()->dataHandler();
}

BosonCanvas* UnitPlugin::canvas() const
{
 return unit()->canvas();
}

Boson* UnitPlugin::game() const
{
 if (!player()) {
	return 0;
 }
 return (Boson*)player()->game();
}

const BoUpgradesCollection& UnitPlugin::upgradesCollection() const
{
 return unit()->upgradesCollection();
}

bool UnitPlugin::isNextTo(const Unit* u) const
{
 // warning: we measure center of _this_ unit to center of the other unit only.
 // this will cause problems for units that occupy multiple cells (e.g.
 // refineries)
 if (!u) {
	return false;
 }
 if (!unit()) {
	BO_NULL_ERROR(unit());
	return false;
 }

 bofixed distx, disty;
 distx = (int)(u->centerX() - unit()->centerX());
 disty = (int)(u->centerY() - unit()->centerY());
 distx = QABS(distx);
 disty = QABS(disty);
 // We might get some precision trouble with floats, so we do this:
 distx = QMAX(distx - 0.1, bofixed(0));
 disty = QMAX(disty - 0.1, bofixed(0));

 bofixed allowedx, allowedy;
 allowedx = ceilf(unit()->width()) / 2 + ceilf(u->width()) / 2;
 allowedy = ceilf(unit()->height()) / 2 + ceilf(u->height()) / 2;

 if (distx <= allowedx && disty <= allowedy) {
	return true;
 }
 return false;
}

