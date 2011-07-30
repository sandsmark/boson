/*
    This file is part of the Boson game
    Copyright (C) 2003 Andreas Beckermann (b_mann@gmx.de)

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

#include "botooltipcreator.h"

#include "../bomemory/bodummymemory.h"
#include "bodebug.h"
#include "gameengine/rtti.h"
#include "gameengine/unit.h"
#include "gameengine/unitplugins/harvesterplugin.h"
#include "gameengine/unitplugins/productionplugin.h"
#include "gameengine/speciestheme.h"
#include "gameengine/unitproperties.h"
#include "gameengine/upgradeproperties.h"
#include "gameengine/cell.h"
#include "bo3dtools.h"
#include "bosonpropertylist.h"

#include <qstring.h>
#include <q3ptrvector.h>
//Added by qt3to4:
#include <Q3ValueList>

#include <klocale.h>

QString BoToolTipCreator::createToolTip(const BosonItem* ) const
{
 return QString();
}

QString BoToolTipCreatorBasic::createToolTip(const BosonItem* item) const
{
 if (!item) {
	return QString();
 }
 if (!RTTI::isUnit(item->rtti())) {
	return QString();
 }
 Unit* u = (Unit*)item;
 return u->name();
}

QString BoToolTipCreatorExtended::createToolTip(const BosonItem* item) const
{
 if (!item) {
	return QString();
 }
 if (!RTTI::isUnit(item->rtti())) {
	return QString();
 }
 QString tip;

 Unit* u = (Unit*)item;
 tip = i18n("%1\nHealth: %2", u->name(), u->health());
 if (u->isFacility()) {
	UnitConstruction* c = u->construction();
	if (!c->isConstructionComplete()) {
		tip += i18n("\n%1% constructed", (int)c->constructionProgress());
	} else {
		ProductionPlugin* production = (ProductionPlugin*)u->plugin(UnitPlugin::Production);
		if (production && production->hasProduction()) {
			QString text;
			unsigned long int id = production->currentProductionId();
			ProductionType type = production->currentProductionType();
			double p = production->productionProgress();
			if (type == ProduceUnit) {
				const UnitProperties* prop = production->speciesTheme()->unitProperties(id);
				text = i18n("\nProducing: %1 (%2%)", prop->name(), (int)p);
			} else if (type == ProduceTech) {
				const UpgradeProperties* prop = production->speciesTheme()->technology(id);
				text = i18n("\nResearching: %1 (%2%)", prop->upgradeName(), (int)p);
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
			tip += i18n("\nMineral filling: %1%", (int)p);
		} else if (harvester->canMineOil()) {
			tip += i18n("\nOil filling: %1%", (int)p);
		}
	}
 }
 return tip;
}

QString BoToolTipCreatorDebug::createToolTip(const BosonItem* item) const
{
 if (!item) {
	return QString();
 }
 // debugging tooltips are not restricted in size, although we should *try* to
 // keep em short. but it is more important that all necessary data is
 // included...
 // especially data about position is *really* important. cells are very handy,
 // too
 QString tip = i18n("Rtti: %1", item->rtti());
 tip += i18n("\nCenter Position: (%1,%2,%3)", item->centerX(), item->centerY(), item->z());
 tip += i18n("\nRotation: (%1,%2,%3)", item->xRotation(), item->yRotation(), item->rotation());
 tip += i18n("\nSize: (%1,%2)", item->width(), item->height());

 Q3PtrVector<Cell>* cells = item->cellsConst();
 tip += i18n("\nCells: ");
 for (unsigned int i = 0; i < cells->count(); i++) {
	if (i != 0) {
		tip += i18n(",");
	}
	tip += i18n("(%1,%2)", cells->at(i)->x(), cells->at(i)->y());

 }
 tip += i18n("\nVelocity: (%1,%2,%3)", item->xVelocity(), item->yVelocity(), item->zVelocity());

 if (!RTTI::isUnit(item->rtti())) {
	return tip;
 }

 Unit* u = (Unit*)item;
 tip += i18n("\nId : %1", u->id());
 tip += i18n("\nName: %1", u->name()); // AB: could be left out for debugging tooltips
 tip += i18n("\nHealth: %1", u->health()); // AB: could be left out for debugging tooltips
 tip += i18n("\nAdvance Work: %1", u->advanceWork());

 QList<BoVector2Fixed> pathpoints = u->pathPointList();
 if (pathpoints.count() == 0) {
	tip += i18n("\nNo pathpoints");
 } else {
	QList<BoVector2Fixed>::Iterator it;
	tip += i18n("\nPathpoints: ");
	for (it = pathpoints.begin(); it != pathpoints.end(); ++it) {
		if (it != pathpoints.begin()) {
			tip += i18n(",");
		}
		tip += i18n("(%1,%2)", (*it).x(), (*it).y());
	}
 }

 if (u->isFacility()) {
	UnitConstruction* c = u->construction();
	if (!c->isConstructionComplete()) {
		tip += i18n("\n%1% constructed", (int)c->constructionProgress());
	} else {
		ProductionPlugin* production = (ProductionPlugin*)u->plugin(UnitPlugin::Production);
		if (production && production->hasProduction()) {
			QString text;
			unsigned long int id = production->currentProductionId();
			ProductionType type = production->currentProductionType();
			double p = production->productionProgress();
			if (type == ProduceUnit) {
				const UnitProperties* prop = production->speciesTheme()->unitProperties(id);
				text = i18n("\nProducing: %1 (%2%)", prop->name(), (int)p);
			} else if (type == ProduceTech) {
				const UpgradeProperties* prop = production->speciesTheme()->technology(id);
				text = i18n("\nResearching: %1 (%2%)", prop->upgradeName(), (int)p);
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
			tip += i18n("\nMineral filling: %1%", (int)p);
		} else if (harvester->canMineOil()) {
			tip += i18n("\nOil filling: %1%", (int)p);
		}
	}
 }

 return tip;
}