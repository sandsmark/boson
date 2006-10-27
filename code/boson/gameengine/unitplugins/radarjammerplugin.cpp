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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#include "radarjammerplugin.h"

#include "../../bomemory/bodummymemory.h"
#include "../defines.h"
#include "unit.h"
#include "unitproperties.h"
#include "pluginproperties.h"
#include "player.h"
#include "upgradeproperties.h"
#include "bodebug.h"
#include "../bo3dtools.h"

#include <math.h>

RadarJammerPlugin::RadarJammerPlugin(Unit* owner)
		: UnitPlugin(owner)
{
 mTransmittedPower = 0.0f;
 mRange = 0;
}
RadarJammerPlugin::~RadarJammerPlugin()
{
}

bool RadarJammerPlugin::saveAsXML(QDomElement& root) const
{
 return true;
}

bool RadarJammerPlugin::loadFromXML(const QDomElement& root)
{
 return true;
}

void RadarJammerPlugin::advance(unsigned int)
{
}

void RadarJammerPlugin::unitDestroyed(Unit*)
{
}

void RadarJammerPlugin::itemRemoved(BosonItem*)
{
}

void RadarJammerPlugin::unitHealthChanged()
{
 const RadarJammerProperties * prop = (RadarJammerProperties*)unit()->properties(PluginProperties::RadarJammer);
 if (!prop) {
	mTransmittedPower = 0.0f;
	mRange = 0;
	return;
 }
 mTransmittedPower = prop->transmittedPower() * (float)unit()->healthFactor();
 // Maximum range of the jammer
 mRange = (bofixed)sqrt(mTransmittedPower / 0.5f);
}

