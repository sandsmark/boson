/*
    This file is part of the Boson game
    Copyright (C) 2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "botooltipcreator.h"
#include "bodebug.h"
#include "rtti.h"
#include "unit.h"
#include "unitplugins.h"
#include "speciestheme.h"
#include "unitproperties.h"
#include "upgradeproperties.h"

#include <qstring.h>

#include <klocale.h>

QString BoToolTipCreator::createToolTip(const BosonItem* ) const
{
 return QString::null;
}

QString BoToolTipCreatorBasic::createToolTip(const BosonItem* item) const
{
 if (!item) {
	return QString::null;
 }
 if (!RTTI::isUnit(item->rtti())) {
	return QString::null;
 }
 Unit* u = (Unit*)item;
 return u->name();
}

QString BoToolTipCreatorExtended::createToolTip(const BosonItem* item) const
{
 if (!item) {
	return QString::null;
 }
 if (!RTTI::isUnit(item->rtti())) {
	return QString::null;
 }
 QString tip;

 Unit* u = (Unit*)item;
 tip = i18n("%1\nHealth: %2").arg(u->name()).arg(u->health());
 if (u->isFacility()) {
	Facility* fac = (Facility*)u;
	if (!fac->isConstructionComplete()) {
		tip += i18n("\n%1% constructed").arg((int)fac->constructionProgress());
	} else {
		ProductionPlugin* production = (ProductionPlugin*)u->plugin(UnitPlugin::Production);
		if (production && production->hasProduction()) {
			QString text;
			unsigned long int id = production->currentProductionId();
			ProductionType type = production->currentProductionType();
			double p = production->productionProgress();
			if (type == ProduceUnit) {
				const UnitProperties* prop = production->speciesTheme()->unitProperties(id);
				text = i18n("\nProducing: %1 (%2%)").arg(prop->name()).arg((int)p);
			} else if (type == ProduceTech) {
				const UpgradeProperties* prop = production->speciesTheme()->technology(id);
				text = i18n("\nResearching: %1 (%2%)").arg(prop->upgradeName()).arg((int)p);
			}
			if (!text.isNull()) {
				tip += text;
			}
		}
	}
 } else {
	HarvesterPlugin* harvester = (HarvesterPlugin*)u->plugin(UnitPlugin::Harvester);
	if (harvester) {
		double p = (double)(harvester->resourcesMined() * 100) / (double)harvester->maxResources();
		if (harvester->canMineMinerals()) {
			tip += i18n("\nMineral filling: %1%").arg((int)p);
		} else if (harvester->canMineOil()) {
			tip += i18n("\nOil filling: %1%").arg((int)p);
		}
	}
 }
 return tip;
}

QString BoToolTipCreatorDebug::createToolTip(const BosonItem* item) const
{
 if (!item) {
	return QString::null;
 }
 if (!RTTI::isUnit(item->rtti())) {
	return QString::null;
 }
 QString tip;
 // TODO :)

 return tip;
}
