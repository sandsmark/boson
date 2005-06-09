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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "bosonplayerinputhandler.h"
#include "bosonplayerinputhandler.moc"

#include "boson.h"
#include "bosonmessageids.h"
#include "bosonmessage.h"
#include "player.h"
#include "unit.h"
#include "unitplugins.h"
#include "speciestheme.h"
#include "unitproperties.h"
#include "bosoncanvas.h"
#include "global.h"
#include "upgradeproperties.h"
#include "bodebug.h"
#include "bosonweapon.h"
#include "boevent.h"

#include <klocale.h>

#include <math.h>

BosonPlayerInputHandler::BosonPlayerInputHandler(Boson* game) : QObject(0, "bosonplayerinputhandler")
{
 mGame = game;
 connect(this, SIGNAL(signalChangeTexMap(int, int, unsigned int, unsigned int*, unsigned char*)),
		mGame, SIGNAL(signalChangeTexMap(int, int, unsigned int, unsigned int*, unsigned char*)));
 connect(this, SIGNAL(signalChangeHeight(int, int, float)),
		mGame, SIGNAL(signalChangeHeight(int, int, float)));
}

BosonPlayerInputHandler::~BosonPlayerInputHandler()
{
}

BosonCanvas* BosonPlayerInputHandler::canvas() const
{
 return mGame->canvasNonConst();
}

Unit* BosonPlayerInputHandler::findUnit(unsigned long int id, Player* p) const
{
 return mGame->findUnit(id, p);
}

Player* BosonPlayerInputHandler::findPlayer(unsigned long int id) const
{
 return (Player*)mGame->findPlayer(id);
}

bool BosonPlayerInputHandler::playerInput(QDataStream& stream, Player* player)
{
 if (player->isOutOfGame()) {
	boWarning() << k_funcinfo << "Player must not send input anymore!!" << endl;
	return true;
 }
 if (!mGame->gameMode()) {
	// editor mode sends an additional entry safety id, just in case we
	// might have constructed a wrong display or so
	Q_UINT32 editor;
	stream >> editor;
	if (editor != BosonMessageIds::MoveEditor) {
		boError() << k_funcinfo << "Not an editor message, elthough we're in editor mode!" << endl;
		return true;
	}
 }
 Q_UINT32 msgid;
 stream >> msgid;
 if (mGame->gameMode()) {
	if (gamePlayerInput(msgid, stream, player)) {
		return true;
	}
 } else {
	if (editorPlayerInput(msgid, stream, player)) {
		return true;
	}
 }
 switch (msgid) {
	default:
		boWarning() << k_funcinfo << "unexpected playerInput " << msgid << endl;
		break;
 }
 return true;
}

bool BosonPlayerInputHandler::gamePlayerInput(Q_UINT32 msgid, QDataStream& stream, Player* player)
{
 switch (msgid) {
	case BosonMessageIds::MoveMove:
	{
		bool attack;
		Q_UINT8 attackcode;
		BoVector2Fixed pos;
		Q_UINT32 unitCount;
		stream >> attackcode;
		if (attackcode == 0) {
			attack = false;
		} else {
			attack = true;
		}
		boDebug() << "MOVING: " << k_funcinfo << "attack: " << attack << endl;
		stream >> pos;
		stream >> unitCount;
		QPtrList<Unit> unitsToMove;
		for (unsigned int i = 0; i < unitCount; i++) {
			Q_ULONG unitId;
			stream >> unitId;
//			boDebug() << "pos: " << pos.x() << " " << pos.y() << endl;
			Unit* unit = findUnit(unitId, player);
			if (!unit) {
				boDebug() << "unit " << unitId << " not found for this player" << endl;
				continue;
			}
			if (unit->owner() != player) {
				boDebug() << "unit " << unitId << "only owner can move units!" << endl;
				continue;
			}
			if (unit->isDestroyed()) {
				boDebug() << "cannot move destroyed units" << endl;
				continue;
			}
//			boDebug() << "move " << unitId << endl;
			if (unit->unitProperties()->isMobile()) {
				unitsToMove.append(unit);
			}
		}
		if (unitsToMove.count() == 0) {
			boWarning() << k_lineinfo << "no unit to move" << endl;
			break;
		}
		if (unitsToMove.count() == 1) {
			unitsToMove.first()->moveTo(pos, attack);
		} else {
		QPtrListIterator<Unit> it(unitsToMove);
			it.toFirst();
			while (it.current()) {
				it.current()->moveTo(pos, attack);
				++it;
			}
		}
		break;
	}
	case BosonMessageIds::MoveAttack:
	{
		Q_ULONG attackedUnitId;
		Q_UINT32 unitCount;
		stream >> attackedUnitId;
		stream >> unitCount;
		Unit* attackedUnit = findUnit(attackedUnitId, 0);
		if (!attackedUnit) {
			boError() << "Cannot attack NULL unit" << endl;
			return true;
		}
		for (unsigned int i = 0; i < unitCount; i++) {
			Q_ULONG unitId;
			stream >> unitId;
			if (unitId == attackedUnitId) {
				// can become possible one day - e.g. when
				// repairing a unit
				boWarning() << "Can not (yet) attack myself"
						<< endl;
				continue;
			}
			Unit* unit = findUnit(unitId, player);
			if (!unit) {
				boDebug() << "unit " << unitId << " not found for this player" << endl;
				continue;
			}
			if (unit->isDestroyed()) {
				boDebug() << "cannot attack with destroyed units" << endl;
				continue;
			}
			if (attackedUnit->isDestroyed()) {
				boDebug() << "no sense in attacking destroyed units" << endl;
				continue;
			}
			if (unit->unitProperties()->canShoot()) {
				boDebug() << unitId << " attacks " << attackedUnitId << endl;
				unit->setTarget(attackedUnit);
				if (unit->target()) {
					unit->setWork(Unit::WorkAttack);
				}
			}
		}
		break;
	}
	case BosonMessageIds::MoveStop:
	{
		Q_UINT32 unitCount;
		stream >> unitCount;
		QPtrList<Unit> unitsToStop;
		for (unsigned int i = 0; i < unitCount; i++) {
			Q_ULONG unitId;
			stream >> unitId;
			Unit* unit = findUnit(unitId, player);
			if (!unit) {
				boDebug() << "unit " << unitId << " not found for this player" << endl;
				continue;
			}
			if (unit->owner() != player) {
				boDebug() << "unit " << unitId << "only owner can stop units!" << endl;
				continue;
			}
			if (unit->isDestroyed()) {
				boDebug() << "cannot stop destroyed units" << endl;
				continue;
			}
			unitsToStop.append(unit);
		}
		if (unitsToStop.count() == 0) {
			boWarning() << k_lineinfo << "no unit to stop" << endl;
			break;
		}
		QPtrListIterator<Unit> it(unitsToStop);
		while (it.current()) {
			it.current()->stopAttacking();  // call stopAttacking() because it also sets unit's work to WorkIdle ... and it doesn't hurt
			++it;
		}
		break;
	}
	case BosonMessageIds::MoveMine:
	{
		boDebug() << "MoveMine" << endl;
		Q_ULONG harvesterId;
		Q_ULONG resourceMineId;
		stream >> harvesterId;
		stream >> resourceMineId;
		Unit* u = findUnit(harvesterId, player);
		if (!u) {
			boError() << k_lineinfo << "cannot find harvester unit " << harvesterId << " for player " << player << endl;
			break;
		}
		if (!u->isMobile()) {
			boError() << k_lineinfo << "only mobile units can mine" << endl;
			break;
		}
		HarvesterPlugin* h = (HarvesterPlugin*)u->plugin(UnitPlugin::Harvester);
		if (!h) {
			boError() << k_lineinfo << "only harvester can mine" << endl;
			break;
		}
		if (u->owner() != player) {
			boDebug() << k_funcinfo << "unit " << harvesterId << "only owner can move units!" << endl;
			break;
		}
		if (u->isDestroyed()) {
			boDebug() << "cannot mine with destroyed units" << endl;
			break;
		}

		u = findUnit(resourceMineId, 0);
		if (!u) {
			boError() << k_lineinfo << "cannot find resourcemine unit " << resourceMineId << " for player " << player << endl;
			break;
		}
		ResourceMinePlugin* r = (ResourceMinePlugin*)u->plugin(UnitPlugin::ResourceMine);
		if (!r) {
			boError() << k_lineinfo << "can mine at resource mine only" << endl;
			break;
		}
		h->mineAt(r);
		break;
	}
	case BosonMessageIds::MoveRefine:
	{
		boDebug() << "MoveRefine" << endl;
		Q_UINT32 refineryOwnerId;
		Q_ULONG refineryId;
		Q_UINT32 unitCount;
		stream >> refineryOwnerId;
		stream >> refineryId;
		stream >> unitCount;
		Player* refineryOwner = findPlayer(refineryOwnerId);
		if (!refineryOwner) {
			boError() << k_lineinfo << "Cannot find player " << refineryOwnerId << endl;
			break;
		}
		if (player->isEnemy(refineryOwner)) {
			boError() << k_lineinfo << "Cannot go to enemy refinery" << endl;
			break;
		}
		Unit* refineryUnit = findUnit(refineryId, refineryOwner);
		if (!refineryUnit) {
			boError() << k_lineinfo << "cannot find refinery " << refineryId << " for player " << refineryOwnerId << endl;
			break;
		}
		RefineryPlugin* refinery = (RefineryPlugin*)refineryUnit->plugin(UnitPlugin::Refinery);
		if (!refinery) {
			boWarning() << k_lineinfo << "refinery must be a refinery " << endl;
			break;
		}
		for (unsigned int i = 0; i < unitCount; i++) {
			Q_ULONG unitId;
			stream >> unitId;
			Unit* u = findUnit(unitId, player);
			if (!u) {
				boError() << k_lineinfo << "cannot find unit " << unitId << " for player " << player->id() << endl;
				continue;
			}
			if (u->isDestroyed()) {
				continue;
			}
			if (u->owner() != player) {
				continue;
			}
			HarvesterPlugin* h = (HarvesterPlugin*)u->plugin(UnitPlugin::Harvester);
			if (!h) {
				boError() << k_lineinfo << "must be a harvester" << endl;
				continue;
			}
			h->refineAt(refinery);
		}
		break;
	}
	case BosonMessageIds::MoveRepair:
	{
		boWarning() << "MoveRepair is a TODO" << endl;
		break;
		// move mobile units to repairyard
		//
		// TODO there are several ways of repairing:
		// 1. move mobile units to repairyard
		// 2. move a mobile repairunit to damaged units
		// 3. repair facilities
		// can we use MoveRepair for all of them?
		// this is currently about 1. only
		Q_UINT32 repairOwnerId;
		Q_ULONG repairId;
		Q_UINT32 unitCount;
		stream >> repairOwnerId;
		stream >> repairId;
		stream >> unitCount;
		Player* repairOwner= findPlayer(repairOwnerId);
		if (!repairOwner) {
			boError() << k_lineinfo << "Cannot find player " << repairOwnerId << endl;
			break;
		}
		if (player->isEnemy(repairOwner)) {
			boError() << k_lineinfo << "Cannot move to enemy repairyard" << endl;
			break;
		}
		Unit* repairYard = findUnit(repairId, repairOwner);
		if (!repairYard) {
			boError() << k_lineinfo << "Cannot find " << repairId << " for player " << repairOwnerId << endl;
			break;
		}
		RepairPlugin* repair = (RepairPlugin*)repairYard->plugin(UnitPlugin::Repair);
		if (!repair) {
			boError() << k_lineinfo << "repairyard cannot repair?!" << endl;
			break;
		}
		for (unsigned int i = 0; i < unitCount; i++) {
			Q_ULONG unitId;
			stream >> unitId;
			Unit* u = findUnit(unitId, player);
			if (!u) {
				boError() << k_lineinfo << "cannot find unit " << unitId << " for player " << player->id()  << endl;
				continue;
			}
			if (!u->isMobile()) {
				boError() << k_lineinfo << "must be a mobile unit" << endl;
				continue;
			}
			repair->repair(u);
		}
		break;
	}
	case BosonMessageIds::MoveProduce:
	{
		Q_UINT32 productionType;
		Q_UINT32 owner;
		Q_ULONG factoryId;
		Q_UINT32 id;

		stream >> productionType;

		stream >> owner;
		stream >> factoryId;
		stream >> id; // TODO: rename to "type" (simple text replacement, but I am lazy atm)

		Player* p = findPlayer(owner);
		if (!p) {
			boError() << k_lineinfo << "Cannot find player " << owner << endl;
			break;
		}
		Unit* factory = findUnit(factoryId, p);
		if (!factory) {
			boError() << "Cannot find unit " << factoryId << endl;
			break;
		}

		ProductionPlugin* production = (ProductionPlugin*)factory->plugin(UnitPlugin::Production);
		if (!production) {
			// maybe not yet fully constructed
			boWarning() << k_lineinfo << factory->id() << " cannot produce" << endl;
			break;
		}
		if (id <= 0) {
			boError() << k_lineinfo << "Invalid id " << id << endl;
			break;
		}

		unsigned long int mineralCost = 0, oilCost = 0;

		if ((ProductionType)productionType == ProduceUnit) {
			// Produce unit
			const UnitProperties* prop = p->unitProperties(id);
			if (!prop) {
				boError() << k_lineinfo << "NULL unit properties (EVIL BUG)" << endl;
				break;
			}
			mineralCost = prop->mineralCost();
			oilCost = prop->oilCost();
		} else if ((ProductionType)productionType == ProduceTech) {
			// Produce upgrade
			const UpgradeProperties* prop = p->speciesTheme()->technology(id);
			if (!prop) {
				boError() << k_lineinfo << "NULL technology properties (EVIL BUG)" << endl;
				break;
			}
			mineralCost = prop->mineralCost();
			oilCost = prop->oilCost();
		} else {
			boError() << k_funcinfo << "Invalid productionType: " << productionType << endl;
		}

		// AB: event parameters.
		// all events in here contain a type (unittype or technology type) and a location.
		// the name of the event depends what is to be done (production
		// started or un-paused) and the productiontype (unit/tech). two
		// events that differ by productiontype only, differ in their
		// name by the "Unit" or "Technology" part only.
		QString eventTypeParameter = QString::number(id);
		QCString productionTypeString;
		if ((ProductionType)productionType == ProduceUnit) {
			productionTypeString = "Unit";
		} else if ((ProductionType)productionType == ProduceTech) {
			productionTypeString = "Technology";
		} else {
			boError() << k_funcinfo << "Invalid productionType: " << productionType << endl;
		}
		BoVector3Fixed eventLocation(factory->x(), factory->y(), factory->z());

		if (factory->currentPluginType() != UnitPlugin::Production) {
			if ((production->currentProductionId() == id) && (production->currentProductionType() == (ProductionType)productionType)) {
				// production was paused - continue it now
				factory->setPluginWork(UnitPlugin::Production);

				QCString name = "ContinueProductionOf" + productionTypeString + "WithType";
				BoEvent* event = new BoEvent(name, eventTypeParameter, QString::number(factoryId));
				event->setPlayerId(owner);
				event->setLocation(eventLocation);
				mGame->queueEvent(event);
				break;
			}
		}

		if (p->minerals() < mineralCost) {
			mGame->slotAddChatSystemMessage(i18n("You have not enough minerals!"), p);
			break;
		}
		if (p->oil() < oilCost) {
			mGame->slotAddChatSystemMessage(i18n("You have not enough oil!"), p);
			break;
		}
		p->setMinerals(p->minerals() - mineralCost);
		p->setOil(p->oil() - oilCost);
		production->addProduction((ProductionType)productionType, (unsigned long int)id);

		BoEvent* event = new BoEvent(QCString("StartProductionOf") + productionTypeString + "WithType", eventTypeParameter, QString::number(factoryId));
		event->setPlayerId(owner);
		event->setLocation(eventLocation);
		mGame->queueEvent(event);
		break;
	}
	case BosonMessageIds::MoveProduceStop:
	{
		boDebug() << "MoveProduceStop" << endl;
		Q_UINT32 productionType;
		Q_UINT32 owner;
		Q_ULONG factoryId;
		Q_UINT32 id;

		stream >> productionType;

		stream >> owner;
		stream >> factoryId;
		stream >> id; // TODO: rename to "type" (simple text replacement, but I am lazy atm)

		Player* p = findPlayer(owner);
		if (!p) {
			boError() << k_lineinfo << "Cannot find player " << owner << endl;
			break;
		}
		Unit* factory = findUnit(factoryId, p);
		if (!factory) {
			boError() << "Cannot find unit " << factoryId << endl;
			break;
		}

		ProductionPlugin* production = (ProductionPlugin*)factory->plugin(UnitPlugin::Production);
		if (!production) {
			// should not happen here!
			boError() << k_lineinfo << factory->id() << "cannot produce?!" << endl;
			break;
		}
		if (!production->contains((ProductionType)productionType, (unsigned long int)id)) {
			boDebug() << k_lineinfo << "Production " << productionType << " with id "
					 << id << " is not in production queue" << endl;
			return true;
		}
		if (id <= 0) {
			boError() << k_lineinfo << "Invalid id " << id << endl;
			break;
		}

		unsigned long int mineralCost = 0, oilCost = 0;

		if ((ProductionType)productionType == ProduceUnit) {
			const UnitProperties* prop = p->unitProperties(id);
			if (!prop) {
				boError() << k_lineinfo << "NULL unit properties (EVIL BUG)" << endl;
				break;
			}
			mineralCost = prop->mineralCost();
			oilCost = prop->oilCost();
		} else if ((ProductionType)productionType == ProduceTech) {
			const UpgradeProperties* prop = p->speciesTheme()->technology(id);
			if (!prop) {
				boError() << k_lineinfo << "NULL technology properties (EVIL BUG)" << endl;
				break;
			}
			mineralCost = prop->mineralCost();
			oilCost = prop->oilCost();
		} else {
			boError() << k_funcinfo << "Invalid productionType: " << productionType << endl;
		}


		// AB: event parameters.
		QString eventTypeParameter = QString::number(id);
		QCString productionTypeString;
		if ((ProductionType)productionType == ProduceUnit) {
			productionTypeString = "Unit";
		} else if ((ProductionType)productionType == ProduceTech) {
			productionTypeString = "Technology";
		} else {
			boError() << k_funcinfo << "Invalid productionType: " << productionType << endl;
		}


		BoEvent* event = 0;
		if ((production->currentProductionId() == id) && (production->currentProductionType() == (ProductionType)productionType)) {
			if (factory->currentPluginType() == UnitPlugin::Production) {
				// do not abort but just pause
				factory->setWork(Unit::WorkIdle);
				event = new BoEvent(QCString("PauseProductionOf") + productionTypeString + "WithType", eventTypeParameter, QString::number(factoryId));
			} else {
				p->setMinerals(p->minerals() + mineralCost);
				p->setOil(p->oil() + oilCost);
				production->removeProduction();
				event = new BoEvent(QCString("StopProductionOf") + productionTypeString + "WithType", eventTypeParameter, QString::number(factoryId));
			}
		} else {
			// not the current, but a queued production is stopped.

			//FIXME: money should be paid when the production is
			//actually started! (currently it is paid as soon as an
			//item is added to the queue)
			p->setMinerals(p->minerals() + mineralCost);
			p->setOil(p->oil() + oilCost);
			production->removeProduction((ProductionType)productionType, (unsigned long int)id);
			event = new BoEvent(QCString("StopProductionOf") + productionTypeString + "WithType", eventTypeParameter, QString::number(factoryId));
		}
		if (event) {
			event->setPlayerId(owner);
			event->setLocation(BoVector3Fixed(factory->x(), factory->y(), factory->z()));
			mGame->queueEvent(event);
		}
		break;
	}
	case BosonMessageIds::MoveBuild:
	{
		boDebug() << "MoveBuild" << endl;
		Q_UINT32 productionType;
		Q_ULONG factoryId;
		Q_UINT32 owner;
		BoVector2Fixed pos;

		stream >> productionType;

		stream >> factoryId;
		stream >> owner;
		stream >> pos;

		// Only units are "built"
		if ((ProductionType)productionType != ProduceUnit) {
			boError() << k_funcinfo << "Invalid productionType: " << productionType << endl;
			break;
		}

		Player* p = findPlayer(owner);
		if (!p) {
			boError() << k_lineinfo << "Cannot find player " << owner << endl;
			break;
		}
		Unit* factory = findUnit(factoryId, p);
		if (!factory) {
			boError() << "Cannot find unit " << factoryId << endl;
			break;
		}
		ProductionPlugin* production = (ProductionPlugin*)factory->plugin(UnitPlugin::Production);
		if (!production) {
			// should not happen here!
			boError() << k_lineinfo << factory->id() << "cannot produce?!" << endl;
			break;
		}
		if (production->completedProductionType() != ProduceUnit) {
			boError() << k_lineinfo << "not producing unit!" << endl;
			break;
		}
		int unitType = production->completedProductionId();
		boDebug() << k_lineinfo
				<< "factory="
				<< factory->id()
				<< ",unitid="
				<< unitType
				<< endl;
		if (unitType <= 0) {
			// hope this is working...
			boWarning() << k_lineinfo << "not yet completed" << endl;
			break;
		}
		mGame->buildProducedUnit(production, unitType, pos);
		break;
	}
	case BosonMessageIds::MoveFollow:
	{
		Q_ULONG followUnitId;
		Q_UINT32 unitCount;
		stream >> followUnitId;
		stream >> unitCount;
		Unit* followUnit = findUnit(followUnitId, 0);
		if (!followUnit) {
			boError() << "Cannot follow NULL unit" << endl;
			return true;
		}
		for (unsigned int i = 0; i < unitCount; i++) {
			Q_ULONG unitId;
			stream >> unitId;
			if (unitId == followUnitId) {
				boWarning() << "Cannot follow myself" << endl;
				continue;
			}
			Unit* unit = findUnit(unitId, player);
			if (!unit) {
				boDebug() << "unit " << unitId << " not found for this player" << endl;
				continue;
			}
			if (unit->isDestroyed()) {
				boDebug() << "cannot follow with destroyed units" << endl;
				continue;
			}
			if (followUnit->isDestroyed()) {
				boDebug() << "Cannot follow destroyed units" << endl;
				continue;
			}
			unit->setTarget(followUnit);
			if (unit->target()) {
				unit->setWork(Unit::WorkFollow);
			}
		}
		break;
	}
	case BosonMessageIds::MoveLayMine:
	{
		boDebug() << k_funcinfo << "MoveLayMine action" << endl;
		Q_UINT32 unitCount;
		Q_UINT32 unitId, weaponId;

		stream >> unitCount;

		for (unsigned int i = 0; i < unitCount; i++) {
			stream >> unitId;
			stream >> weaponId;
			boDebug() << k_funcinfo << "unit: " << unitId << "; weapon: " << weaponId << endl;

			Unit* unit = findUnit(unitId, 0);
			if (!unit) {
				boWarning() << "unit " << unitId << " not found" << endl;
				continue;
			}
			if (unit->isDestroyed()) {
				boWarning() << "cannot do anything with destroyed units" << endl;
				continue;
			}
			MiningPlugin* m = (MiningPlugin*)unit->plugin(UnitPlugin::Mining);
			if (!m) {
				boError() << k_lineinfo << "This unit has no mining plugin" << endl;
				break;
			}
			m->mine(weaponId);
		}
		boDebug() << k_funcinfo << "done" << endl;

		break;
	}
	case BosonMessageIds::MoveDropBomb:
	{
		boDebug() << k_funcinfo << "MoveDropBomb action" << endl;
		Q_UINT32 unitCount;
		BoVector2Fixed pos;
		Q_UINT32 unitId, weaponId;

		stream >> pos;
		stream >> unitCount;

		for (unsigned int i = 0; i < unitCount; i++) {
			stream >> unitId;
			stream >> weaponId;
			boDebug() << k_funcinfo << "unit: " << unitId << "; weapon: " << weaponId << endl;

			Unit* unit = findUnit(unitId, 0);
			if (!unit) {
				boWarning() << "unit " << unitId << " not found" << endl;
				continue;
			}
			if (unit->isDestroyed()) {
				boWarning() << "cannot do anything with destroyed units" << endl;
				continue;
			}
			BombingPlugin* b = (BombingPlugin*)unit->plugin(UnitPlugin::Bombing);
			if (!b) {
				boError() << k_lineinfo << "This unit has no bombing plugin" << endl;
				break;
			}
			b->bomb(weaponId, pos);
		}
		boDebug() << k_funcinfo << "done" << endl;

		break;
	}
	case BosonMessageIds::MoveTeleport:
	{
		Q_UINT32 unitId;
		Q_UINT32 owner;
		BoVector2Fixed pos;

		stream >> owner;
		stream >> unitId;
		stream >> pos;

		Player* p = findPlayer(owner);
		if (!p) {
			boError() << k_lineinfo << "Cannot find player " << owner << endl;
			break;
		}
		Unit* u = findUnit(unitId, p);
		if (!u) {
			boError() << "Cannot find unit " << unitId << endl;
			break;
		}

		u->moveBy(pos.x() - u->x(), pos.y() - u->y(), 0);

		break;
	}
	case BosonMessageIds::MoveRotate:
	{
		Q_UINT32 unitId;
		Q_UINT32 owner;
		bofixed rot;

		stream >> owner;
		stream >> unitId;
		stream >> rot;

		Player* p = findPlayer(owner);
		if (!p) {
			boError() << k_lineinfo << "Cannot find player " << owner << endl;
			break;
		}
		Unit* u = findUnit(unitId, p);
		if (!u) {
			boError() << "Cannot find unit " << unitId << endl;
			break;
		}

		u->setRotation(rot);

		break;
	}
	default:
		// did not process message
		return false;
 }
 // message processed
 return true;
}

bool BosonPlayerInputHandler::editorPlayerInput(Q_UINT32 msgid, QDataStream& stream, Player* player)
{
 switch (msgid) {
	case BosonMessageIds::MovePlaceUnit:
	{
		BosonMessageMovePlaceUnit message;
		if (!message.load(stream)) {
			boError() << k_lineinfo << "message (" << message.messageId() << ") could not be read" << endl;
			break;
		}
		Q_UINT32 unitType = message.mUnitType;
		Q_UINT32 owner = message.mOwner;
		BoVector2Fixed pos = message.mPos;

		Player* p = 0;
		if (owner >= 1024) { // a KPlayer ID
			p = findPlayer(owner);
		} else {
			p = (Player*)mGame->playerList()->at(owner);
		}
		if (!p) {
			boError() << k_lineinfo << "Cannot find player " << owner << endl;
			break;
		}

		// First make sure unit can be placed there
		// We must only check if cells aren't occupied and if unit can go there,
		//  because everything else should be checked before placing anything. But
		//  occupied status of cell might have changed already.
		const UnitProperties* prop = p->speciesTheme()->unitProperties(unitType);
		/*bofixed width = prop->unitWidth();
		bofixed height = prop->unitHeight();
		BoRectFixed r(pos, pos + BoVector2Fixed(width, height));
		if (!canvas()->canGo(prop, r)) {
			boWarning() << k_funcinfo << "Unit with type " << unitType << " can't go to (" << pos.x() << "; " << pos.y() << ")" << endl;
			break;
		}
		if (canvas()->collisions()->cellsOccupied(r)) {
			boWarning() << k_funcinfo << "Cells at (" << pos.x() << "; " << pos.y() << ") are occupied" << endl;
			break;
		}*/

		BoVector3Fixed pos3(pos.x(), pos.y(), 0.0f);
		Unit* u = (Unit*)canvas()->createNewItem(RTTI::UnitStart + unitType, p, ItemType(unitType), pos3);
		// Facilities will be fully constructed by default
		if (u->isFacility()) {
			((Facility*)u)->setConstructionStep(((Facility*)u)->constructionSteps());
		}
		// Resource mines will have 20000 minerals / 10000 oil by default
		if (u->plugin(UnitPlugin::ResourceMine)) {
			ResourceMinePlugin* res = (ResourceMinePlugin*)u->plugin(UnitPlugin::ResourceMine);
			if (res->canProvideMinerals()) {
				res->setMinerals(20000);
			}
			if (res->canProvideOil()) {
				res->setOil(10000);
			}
		}
		break;
	}
	case BosonMessageIds::MoveChangeTexMap:
	{
		BosonMessageMoveChangeTexMap message;
		if (!message.load(stream)) {
			boError() << k_lineinfo << "message (" << message.messageId() << ") could not be read" << endl;
			break;
		}
		for (unsigned int i = 0; i < message.mCellCornersX.count(); i++) {
			int x = message.mCellCornersX[i];
			int y = message.mCellCornersY[i];
			Q_UINT32 texCount = message.mCellCornersTextureCount[i];
			Q_UINT32* textures = new Q_UINT32[texCount];
			Q_UINT8* alpha = new Q_UINT8[texCount];
			for (unsigned int j = 0; j < texCount; j++) {
				textures[j] = (message.mCellCornerTextures[i])[j];
				alpha[j] = (message.mCellCornerAlpha[i])[j];
			}
			emit signalChangeTexMap(x, y, texCount, textures, alpha);
			delete[] textures;
			delete[] alpha;
		}
		break;
	}
	case BosonMessageIds::MoveChangeHeight:
	{
		boDebug() << k_lineinfo << "change height" << endl;
		BosonMessageMoveChangeHeight message;
		if (!message.load(stream)) {
			boError() << k_lineinfo << "message (" << message.messageId() << ") could not be read" << endl;
			break;
		}
		Q_UINT32 count;
		for (Q_UINT32 i = 0; i < message.mCellCornersX.count(); i++) {
			// note: cornerX == mapWidth() and cornerY == mapHeight()
			// are valid!
			Q_INT32 cornerX = message.mCellCornersX[i];
			Q_INT32 cornerY = message.mCellCornersY[i];
			bofixed height = message.mCellCornersHeight[i];
			if (cornerX < 0 || (unsigned int)cornerX > canvas()->mapWidth()) {
				boError() << k_funcinfo << "invalid x coordinate " << cornerX << endl;
				continue;
			}
			if (cornerY < 0 || (unsigned int)cornerY > canvas()->mapHeight()) {
				boError() << k_funcinfo << "invalid y coordinate " << cornerY << endl;
				continue;
			}
			boDebug() << k_funcinfo << "new height at " << cornerX << "," << cornerY << " is " << height << endl;
			canvas()->setHeightAtCorner(cornerX, cornerY, height);
			emit signalChangeHeight(cornerX, cornerY, height);
		}
		break;
	}
	case BosonMessageIds::MoveDeleteItems:
	{
		BosonMessageMoveDeleteItems message;
		if (!message.load(stream)) {
			boError() << k_lineinfo << "message (" << message.messageId() << ") could not be read" << endl;
			break;
		}
		canvas()->deleteItems(message.mItems);
		break;
	}
	default:
		// did not process message
		return false;
 }
 // message processed
 return true;
}


