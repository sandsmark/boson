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
#include "radarplugin.h"

#include "../../bomemory/bodummymemory.h"
#include "../defines.h"
#include "unit.h"
#include "pluginproperties.h"
#include "bodebug.h"
#include "../bo3dtools.h"

#include <math.h>

RadarPlugin::RadarPlugin(Unit* owner)
		: UnitPlugin(owner)
{
 mTransmittedPower = 0.0f;
 mRange = 0;
}
RadarPlugin::~RadarPlugin()
{
}

bool RadarPlugin::saveAsXML(QDomElement& root) const
{
 return true;
}

bool RadarPlugin::loadFromXML(const QDomElement& root)
{
 return true;
}

void RadarPlugin::advance(unsigned int)
{
}

void RadarPlugin::unitDestroyed(Unit*)
{
}

void RadarPlugin::itemRemoved(BosonItem*)
{
}

float RadarPlugin::minReceivedPower() const
{
 const RadarProperties * prop = (RadarProperties*)unit()->properties(PluginProperties::Radar);
 if (!prop) {
	return 0.0f;
 }
 return prop->minReceivedPower();
}

bool RadarPlugin::detectsLandUnits() const
{
 const RadarProperties * prop = (RadarProperties*)unit()->properties(PluginProperties::Radar);
 if (!prop) {
	return false;
 }
 return prop->detectsLandUnits();
}

bool RadarPlugin::detectsAirUnits() const
{
 const RadarProperties * prop = (RadarProperties*)unit()->properties(PluginProperties::Radar);
 if (!prop) {
	return false;
 }
 return prop->detectsAirUnits();
}

void RadarPlugin::unitHealthChanged()
{
 const RadarProperties * prop = (RadarProperties*)unit()->properties(PluginProperties::Radar);
 if (!prop) {
	mTransmittedPower = 0.0f;
	mRange = 0;
	return;
 }
 mTransmittedPower = prop->transmittedPower() * (float)unit()->healthFactor();
 // Maximum range of the radar
 // We calculate maximum distance so that an object with size = 4.0 is still
 //  detected by the radar
 mRange = (bofixed)powf((mTransmittedPower * 4.0f) / prop->minReceivedPower(), 0.333f);
}


