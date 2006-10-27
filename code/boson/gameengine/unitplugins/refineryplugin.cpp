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
#include "refineryplugin.h"

#include "../../bomemory/bodummymemory.h"
#include "../defines.h"
#include "harvesterplugin.h"
#include "resourcemineplugin.h"
#include "unit.h"
#include "unitproperties.h"
#include "pluginproperties.h"
#include "player.h"
#include "playerio.h"
#include "bodebug.h"
#include "../bo3dtools.h"

#include <qdom.h>

#include <math.h>

RefineryPlugin::RefineryPlugin(Unit* owner)
		: UnitPlugin(owner)
{
}
RefineryPlugin::~RefineryPlugin()
{
}

bool RefineryPlugin::saveAsXML(QDomElement& root) const
{
 Q_UNUSED(root);
 return true;
}

bool RefineryPlugin::loadFromXML(const QDomElement& root)
{
 Q_UNUSED(root);
 return true;
}

void RefineryPlugin::advance(unsigned int)
{
}

bool RefineryPlugin::canRefineMinerals() const
{
 const RefineryProperties * prop = (RefineryProperties*)unit()->properties(PluginProperties::Refinery);
 if (!prop) {
	return false;
 }
 return prop->canRefineMinerals();
}

bool RefineryPlugin::canRefineOil() const
{
 const RefineryProperties * prop = (RefineryProperties*)unit()->properties(PluginProperties::Refinery);
 if (!prop) {
	return false;
 }
 return prop->canRefineOil();
}

void RefineryPlugin::unitDestroyed(Unit*)
{
}

void RefineryPlugin::itemRemoved(BosonItem*)
{
}

bool RefineryPlugin::isUsableTo(const HarvesterPlugin* harvester) const
{
 if (!harvester) {
	return false;
 }
 if (canRefineMinerals() && harvester->canMineMinerals()) {
	return true;
 }
 if (canRefineOil() && harvester->canMineOil()) {
	return true;
 }
 return false;
}

unsigned int RefineryPlugin::refineMinerals(unsigned int minerals)
{
 if (!player()) {
	BO_NULL_ERROR(player());
	return 0;
 }
 if (!unit()->requestPowerChargeForAdvance()) {
	return 0;
 }
 player()->setMinerals(player()->minerals() + minerals);
 return minerals;
}

unsigned int RefineryPlugin::refineOil(unsigned int oil)
{
 if (!player()) {
	BO_NULL_ERROR(player());
	return 0;
 }
 if (!unit()->requestPowerChargeForAdvance()) {
	return 0;
 }
 player()->setOil(player()->oil() + oil);
 return oil;
}



