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
#include "cell.h"

#include <qstring.h>
#include <qptrvector.h>

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
 // debugging tooltips are not restricted in size, although we should *try* to
 // keep em short. but it is more important that all necessary data is
 // included...
 // especially data about position is *really* important. cells are very handy,
 // too
 QString tip = i18n("Rtti: %1").arg(item->rtti());
 tip += i18n("\nPosition: (%1,%2,%3)").arg(item->x()).arg(item->y()).arg(item->z());
 tip += i18n("\nSize: (%1,%2)").arg(item->width()).arg(item->height());

 QPtrVector<Cell>* cells = item->cellsConst();
 tip += i18n("\nCells: ");
 for (unsigned int i = 0; i < cells->count(); i++) {
	if (i != 0) {
		tip += i18n(",");
	}
	tip += i18n("(%1,%2)").arg(cells->at(i)->x()).arg(cells->at(i)->y());

 }
 tip += i18n("\nVelocity: (%1,%2,%3)").arg(item->xVelocity()).arg(item->yVelocity()).arg(item->zVelocity());

 if (!RTTI::isUnit(item->rtti())) {
	return tip;
 }

 Unit* u = (Unit*)item;
 tip += i18n("\nId : %1").arg(u->id());
 tip += i18n("\nName: %1").arg(u->name()); // AB: could be left out for debugging tooltips
 tip += i18n("\nHealth: %1").arg(u->health()); // AB: could be left out for debugging tooltips
 tip += i18n("\nWork: %1").arg(u->work());
 tip += i18n("\nAdvance Work: %1").arg(u->advanceWork());
 tip += i18n("\nTarget: %1").arg(u->target() ? QString::number(u->target()->id()) : i18n("None"));
 tip += i18n("\nDestination: %1,%2").arg(u->destinationX()).arg(u->destinationY());

 // AB: waypoints are important imho
 QValueList<QPoint> waypoints = u->waypointList();
 if (waypoints.count() == 0) {
	tip += i18n("\nNo waypoints");
 } else {
	QValueList<QPoint>::Iterator it;
	tip += i18n("\nWaypoints: ");
	for (it = waypoints.begin(); it != waypoints.end(); ++it) {
		if (it != waypoints.begin()) {
			tip += i18n(",");
		}
		tip += i18n("(%1,%2)").arg((*it).x()).arg((*it).y());
	}
 }

 QValueList<QPoint> pathpoints = u->pathPointList();
 if (pathpoints.count() == 0) {
	tip += i18n("\nNo pathpoints");
 } else {
	QValueList<QPoint>::Iterator it;
	tip += i18n("\nPathpoints: ");
	for (it = pathpoints.begin(); it != pathpoints.end(); ++it) {
		if (it != pathpoints.begin()) {
			tip += i18n(",");
		}
		tip += i18n("(%1,%2)").arg((*it).x()).arg((*it).y());
	}
 }

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
