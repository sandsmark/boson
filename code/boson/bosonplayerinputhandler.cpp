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

#include <qptrstack.h>
#include <qdom.h>

#include <math.h>

BosonPlayerInputHandler::BosonPlayerInputHandler(Boson* game)
	: QObject(0, "bosonplayerinputhandler")
{
 mGame = game;
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
		BosonMessageMoveMove message;
		if (!message.load(stream)) {
			boError() << k_lineinfo << "message (" << message.messageId() << ") could not be read" << endl;
			break;
		}
		bool attack = message.mIsAttack;
		boDebug() << "MOVING: " << k_funcinfo << "attack: " << attack << endl;
		QPtrList<Unit> unitsToMove;
		for (QValueList<Q_ULONG>::iterator it = message.mItems.begin(); it != message.mItems.end(); ++it) {
			Q_ULONG unitId = *it;
//			boDebug() << "pos: " << mPos.x() << " " << mPos.y() << endl;
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
			unitsToMove.first()->moveTo(message.mPos, attack);
		} else {
			QPtrListIterator<Unit> it(unitsToMove);
			it.toFirst();
			while (it.current()) {
				it.current()->moveTo(message.mPos, attack);
				++it;
			}
		}
		break;
	}
	case BosonMessageIds::MoveAttack:
	{
		BosonMessageMoveAttack message;
		if (!message.load(stream)) {
			boError() << k_lineinfo << "message (" << message.messageId() << ") could not be read" << endl;
			break;
		}
		Unit* attackedUnit = findUnit(message.mAttackedUnitId, 0);
		if (!attackedUnit) {
			boError() << "Cannot attack NULL unit" << endl;
			return true;
		}
		for (QValueList<Q_ULONG>::iterator it = message.mItems.begin(); it != message.mItems.end(); ++it) {
			Q_ULONG unitId = *it;
			if (unitId == message.mAttackedUnitId) {
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
				boDebug() << unitId << " attacks " << message.mAttackedUnitId << endl;
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
		BosonMessageMoveStop message;
		if (!message.load(stream)) {
			boError() << k_lineinfo << "message (" << message.messageId() << ") could not be read" << endl;
			break;
		}
		QPtrList<Unit> unitsToStop;
		for (QValueList<Q_ULONG>::iterator it = message.mItems.begin(); it != message.mItems.end(); ++it) {
			Q_ULONG unitId = *it;
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
		BosonMessageMoveMine message;
		if (!message.load(stream)) {
			boError() << k_lineinfo << "message (" << message.messageId() << ") could not be read" << endl;
			break;
		}
		boDebug() << "MoveMine" << endl;
		Unit* u = findUnit(message.mHarvesterId, player);
		if (!u) {
			boError() << k_lineinfo << "cannot find harvester unit " << message.mHarvesterId << " for player " << player << endl;
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
			boDebug() << k_funcinfo << "unit " << message.mHarvesterId << "only owner can move units!" << endl;
			break;
		}
		if (u->isDestroyed()) {
			boDebug() << "cannot mine with destroyed units" << endl;
			break;
		}

		u = findUnit(message.mResourceMineId, 0);
		if (!u) {
			boError() << k_lineinfo << "cannot find resourcemine unit " << message.mResourceMineId << " for player " << player << endl;
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
		BosonMessageMoveRefine message;
		if (!message.load(stream)) {
			boError() << k_lineinfo << "message (" << message.messageId() << ") could not be read" << endl;
			break;
		}
		boDebug() << "MoveRefine" << endl;
		Player* refineryOwner = findPlayer(message.mRefineryOwner);
		if (!refineryOwner) {
			boError() << k_lineinfo << "Cannot find player " << message.mRefineryOwner << endl;
			break;
		}
		if (player->isEnemy(refineryOwner)) {
			boError() << k_lineinfo << "Cannot go to enemy refinery" << endl;
			break;
		}
		Unit* refineryUnit = findUnit(message.mRefineryId, refineryOwner);
		if (!refineryUnit) {
			boError() << k_lineinfo << "cannot find refinery " << message.mRefineryId << " for player " << message.mRefineryOwner << endl;
			break;
		}
		RefineryPlugin* refinery = (RefineryPlugin*)refineryUnit->plugin(UnitPlugin::Refinery);
		if (!refinery) {
			boWarning() << k_lineinfo << "refinery must be a refinery" << endl;
			break;
		}
		for (QValueList<Q_ULONG>::iterator it = message.mItems.begin(); it != message.mItems.end(); ++it) {
			Q_ULONG unitId = *it;
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

		BosonMessageMoveRepair message;
		if (!message.load(stream)) {
			boError() << k_lineinfo << "message (" << message.messageId() << ") could not be read" << endl;
			break;
		}

#if 0
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
#endif
		break;
	}
	case BosonMessageIds::MoveProduce:
	{
		BosonMessageMoveProduce message;
		if (!message.load(stream)) {
			boError() << k_lineinfo << "message (" << message.messageId() << ") could not be read" << endl;
			break;
		}

		Player* p = findPlayer(message.mOwner);
		if (!p) {
			boError() << k_lineinfo << "Cannot find player " << message.mOwner << endl;
			break;
		}
		Unit* factory = findUnit(message.mFactoryId, p);
		if (!factory) {
			boError() << "Cannot find unit " << message.mFactoryId << endl;
			break;
		}

		ProductionPlugin* production = (ProductionPlugin*)factory->plugin(UnitPlugin::Production);
		if (!production) {
			// maybe not yet fully constructed
			boWarning() << k_lineinfo << factory->id() << " cannot produce" << endl;
			break;
		}
		if (message.mType  <= 0) {
			boError() << k_lineinfo << "Invalid type " << message.mType << endl;
			break;
		}

		unsigned long int mineralCost = 0, oilCost = 0;

		if ((ProductionType)message.mProduceType == ProduceUnit) {
			// Produce unit
			const UnitProperties* prop = p->unitProperties(message.mType);
			if (!prop) {
				boError() << k_lineinfo << "NULL unit properties (EVIL BUG)" << endl;
				break;
			}
			mineralCost = prop->mineralCost();
			oilCost = prop->oilCost();
		} else if ((ProductionType)message.mProduceType == ProduceTech) {
			// Produce upgrade
			const UpgradeProperties* prop = p->speciesTheme()->technology(message.mType);
			if (!prop) {
				boError() << k_lineinfo << "NULL technology properties (EVIL BUG)" << endl;
				break;
			}
			mineralCost = prop->mineralCost();
			oilCost = prop->oilCost();
		} else {
			boError() << k_funcinfo << "Invalid productionType: " << message.mProduceType << endl;
		}

		// AB: event parameters.
		// all events in here contain a type (unittype or technology type) and a location.
		// the name of the event depends what is to be done (production
		// started or un-paused) and the productiontype (unit/tech). two
		// events that differ by productiontype only, differ in their
		// name by the "Unit" or "Technology" part only.
		QString eventTypeParameter = QString::number(message.mType);
		QCString productionTypeString;
		if ((ProductionType)message.mProduceType == ProduceUnit) {
			productionTypeString = "Unit";
		} else if ((ProductionType)message.mProduceType == ProduceTech) {
			productionTypeString = "Technology";
		} else {
			boError() << k_funcinfo << "Invalid productionType: " << message.mProduceType << endl;
		}
		BoVector3Fixed eventLocation(factory->x(), factory->y(), factory->z());

		if (factory->currentPluginType() != UnitPlugin::Production) {
			if ((production->currentProductionId() == message.mType) && (production->currentProductionType() == (ProductionType)message.mProduceType)) {
				// production was paused - continue it now
				factory->setPluginWork(UnitPlugin::Production);

				QCString name = "ContinueProductionOf" + productionTypeString + "WithType";
				BoEvent* event = new BoEvent(name, eventTypeParameter, QString::number(message.mFactoryId));
				event->setPlayerId(message.mOwner);
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
		production->addProduction((ProductionType)message.mProduceType, (unsigned long int)message.mType);

		BoEvent* event = new BoEvent(QCString("StartProductionOf") + productionTypeString + "WithType", eventTypeParameter, QString::number(message.mFactoryId));
		event->setPlayerId(message.mOwner);
		event->setLocation(eventLocation);
		mGame->queueEvent(event);
		break;
	}
	case BosonMessageIds::MoveProduceStop:
	{
		BosonMessageMoveProduceStop message;
		if (!message.load(stream)) {
			boError() << k_lineinfo << "message (" << message.messageId() << ") could not be read" << endl;
			break;
		}
		boDebug() << "MoveProduceStop" << endl;

		Player* p = findPlayer(message.mOwner);
		if (!p) {
			boError() << k_lineinfo << "Cannot find player " << message.mOwner << endl;
			break;
		}
		Unit* factory = findUnit(message.mFactoryId, p);
		if (!factory) {
			boError() << "Cannot find unit " << message.mFactoryId << endl;
			break;
		}

		ProductionPlugin* production = (ProductionPlugin*)factory->plugin(UnitPlugin::Production);
		if (!production) {
			// should not happen here!
			boError() << k_lineinfo << factory->id() << "cannot produce?!" << endl;
			break;
		}
		if (!production->contains((ProductionType)message.mProduceType, (unsigned long int)message.mType)) {
			boDebug() << k_lineinfo << "Production " << message.mProduceType << " with id "
					 << message.mType << " is not in production queue" << endl;
			return true;
		}
		if (message.mType <= 0) {
			boError() << k_lineinfo << "Invalid type " << message.mType << endl;
			break;
		}

		unsigned long int mineralCost = 0, oilCost = 0;

		if ((ProductionType)message.mProduceType == ProduceUnit) {
			const UnitProperties* prop = p->unitProperties(message.mType);
			if (!prop) {
				boError() << k_lineinfo << "NULL unit properties (EVIL BUG)" << endl;
				break;
			}
			mineralCost = prop->mineralCost();
			oilCost = prop->oilCost();
		} else if ((ProductionType)message.mProduceType == ProduceTech) {
			const UpgradeProperties* prop = p->speciesTheme()->technology(message.mType);
			if (!prop) {
				boError() << k_lineinfo << "NULL technology properties (EVIL BUG)" << endl;
				break;
			}
			mineralCost = prop->mineralCost();
			oilCost = prop->oilCost();
		} else {
			boError() << k_funcinfo << "Invalid productionType: " << message.mProduceType << endl;
		}


		// AB: event parameters.
		QString eventTypeParameter = QString::number(message.mType);
		QCString productionTypeString;
		if ((ProductionType)message.mProduceType == ProduceUnit) {
			productionTypeString = "Unit";
		} else if ((ProductionType)message.mProduceType == ProduceTech) {
			productionTypeString = "Technology";
		} else {
			boError() << k_funcinfo << "Invalid productionType: " << message.mProduceType << endl;
		}


		BoEvent* event = 0;
		if ((production->currentProductionId() == message.mType) && (production->currentProductionType() == (ProductionType)message.mProduceType)) {
			if (factory->currentPluginType() == UnitPlugin::Production) {
				// do not abort but just pause
				factory->setWork(Unit::WorkIdle);
				event = new BoEvent(QCString("PauseProductionOf") + productionTypeString + "WithType", eventTypeParameter, QString::number(message.mFactoryId));
			} else {
				p->setMinerals(p->minerals() + mineralCost);
				p->setOil(p->oil() + oilCost);
				production->removeProduction();
				event = new BoEvent(QCString("StopProductionOf") + productionTypeString + "WithType", eventTypeParameter, QString::number(message.mFactoryId));
			}
		} else {
			// not the current, but a queued production is stopped.

			//FIXME: money should be paid when the production is
			//actually started! (currently it is paid as soon as an
			//item is added to the queue)
			p->setMinerals(p->minerals() + mineralCost);
			p->setOil(p->oil() + oilCost);
			production->removeProduction((ProductionType)message.mProduceType, (unsigned long int)message.mType);
			event = new BoEvent(QCString("StopProductionOf") + productionTypeString + "WithType", eventTypeParameter, QString::number(message.mFactoryId));
		}
		if (event) {
			event->setPlayerId(message.mOwner);
			event->setLocation(BoVector3Fixed(factory->x(), factory->y(), factory->z()));
			mGame->queueEvent(event);
		}
		break;
	}
	case BosonMessageIds::MoveBuild:
	{
		BosonMessageMoveBuild message;
		if (!message.load(stream)) {
			boError() << k_lineinfo << "message (" << message.messageId() << ") could not be read" << endl;
			break;
		}
		boDebug() << "MoveBuild" << endl;

		// Only units are "built"
		if ((ProductionType)message.mProduceType != ProduceUnit) {
			boError() << k_funcinfo << "Invalid productionType: " << message.mProduceType << endl;
			break;
		}

		Player* p = findPlayer(message.mOwner);
		if (!p) {
			boError() << k_lineinfo << "Cannot find player " << message.mOwner << endl;
			break;
		}
		Unit* factory = findUnit(message.mFactoryId, p);
		if (!factory) {
			boError() << "Cannot find unit " << message.mFactoryId << endl;
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
		mGame->buildProducedUnit(production, unitType, message.mPos);
		break;
	}
	case BosonMessageIds::MoveFollow:
	{
		BosonMessageMoveFollow message;
		if (!message.load(stream)) {
			boError() << k_lineinfo << "message (" << message.messageId() << ") could not be read" << endl;
			break;
		}
		Unit* followUnit = findUnit(message.mFollowUnitId, 0);
		if (!followUnit) {
			boError() << "Cannot follow NULL unit" << endl;
			return true;
		}
		for (QValueList<Q_ULONG>::iterator it = message.mItems.begin(); it != message.mItems.end(); ++it) {
			Q_ULONG unitId = *it;
			if (unitId == message.mFollowUnitId) {
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
		BosonMessageMoveLayMine message;
		if (!message.load(stream)) {
			boError() << k_lineinfo << "message (" << message.messageId() << ") could not be read" << endl;
			break;
		}
		boDebug() << k_funcinfo << "MoveLayMine action" << endl;
		Q_UINT32 unitId, weaponId;
		for (unsigned int i = 0; i < message.mUnits.count(); i++) {
			unitId = message.mUnits[i];
			weaponId = message.mWeapons[i];
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
		BosonMessageMoveDropBomb message;
		if (!message.load(stream)) {
			boError() << k_lineinfo << "message (" << message.messageId() << ") could not be read" << endl;
			break;
		}
		boDebug() << k_funcinfo << "MoveDropBomb action" << endl;
		Q_UINT32 unitId, weaponId;

		for (unsigned int i = 0; i < message.mUnits.count(); i++) {
			unitId = message.mUnits[i];
			weaponId = message.mWeapons[i];
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
			b->bomb(weaponId, message.mPos);
		}
		boDebug() << k_funcinfo << "done" << endl;

		break;
	}
	case BosonMessageIds::MoveTeleport:
	{
		BosonMessageMoveTeleport message;
		if (!message.load(stream)) {
			boError() << k_lineinfo << "message (" << message.messageId() << ") could not be read" << endl;
			break;
		}

		Player* p = findPlayer(message.mOwner);
		if (!p) {
			boError() << k_lineinfo << "Cannot find player " << message.mOwner << endl;
			break;
		}
		Unit* u = findUnit(message.mUnitId, p);
		if (!u) {
			boError() << "Cannot find unit " << message.mUnitId << endl;
			break;
		}

		u->moveBy(message.mPos.x() - u->x(), message.mPos.y() - u->y(), 0);

		break;
	}
	case BosonMessageIds::MoveRotate:
	{
		BosonMessageMoveRotate message;
		if (!message.load(stream)) {
			boError() << k_lineinfo << "message (" << message.messageId() << ") could not be read" << endl;
			break;
		}

		Player* p = findPlayer(message.mOwner);
		if (!p) {
			boError() << k_lineinfo << "Cannot find player " << message.mOwner << endl;
			break;
		}
		Unit* u = findUnit(message.mUnitId, p);
		if (!u) {
			boError() << "Cannot find unit " << message.mUnitId << endl;
			break;
		}

		u->setRotation(message.mRotate);

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
		BosonMessageEditorMovePlaceUnit message;
		if (!message.load(stream)) {
			boError() << k_lineinfo << "message (" << message.messageId() << ") could not be read" << endl;
			break;
		}

		Unit* u = editorPlaceUnit(message.mOwner, message.mUnitType, message.mPos, message.mRotation);

		if (!u) {
			boError() << k_funcinfo << "placing unit failed" << endl;
			break;
		}

		BosonMessageEditorMoveUndoPlaceUnit undo(u->id(), message);
		emit signalEditorNewUndoMessage(undo);
		break;
	}
	case BosonMessageIds::MoveChangeTexMap:
	{
		BosonMessageEditorMoveChangeTexMap message;
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
		BosonMessageEditorMoveChangeHeight message;
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
		BosonMessageEditorMoveDeleteItems message;
		if (!message.load(stream)) {
			boError() << k_lineinfo << "message (" << message.messageId() << ") could not be read" << endl;
			break;
		}

		BosonMessageEditorMove* undo = createNewUndoDeleteItemsMessage(message);

		editorDeleteItems(message.mItems);

		emit signalEditorNewUndoMessage(*undo);
		delete undo;

		break;
	}
	case BosonMessageIds::MoveUndoPlaceUnit:
	{
		BosonMessageEditorMoveUndoPlaceUnit message;
		if (!message.load(stream)) {
			boError() << k_lineinfo << "message (" << message.messageId() << ") could not be read" << endl;
			break;
		}

		editorDeleteItems(message.mDeleteUnit.mItems);

		// AB: no additional information required here for redo
		emit signalEditorNewRedoMessage(message.mMessage);
		break;
	}
	case BosonMessageIds::MoveUndoDeleteItems:
	{
		BosonMessageEditorMoveUndoDeleteItems message;
		if (!message.load(stream)) {
			boError() << k_lineinfo << "message (" << message.messageId() << ") could not be read" << endl;
			break;
		}

		QValueList<Q_ULONG> redoDeleteItems;
		for (unsigned int i = 0; i < message.mUnits.count(); i++) {
			BosonMessageEditorMovePlaceUnit* p = message.mUnits[i];

			Unit* u = editorPlaceUnit(p->mOwner, p->mUnitType, p->mPos, p->mRotation);

			if (!u) {
				boError() << k_funcinfo << "placing unit failed" << endl;
				continue;
			}

			// AB: we even add this to redo if loading the unit from
			// xml fails
			redoDeleteItems.append(u->id());

			QString xml = message.mUnitsData[i];
			QDomDocument doc;
			if (!doc.setContent(xml)) {
				boError() << k_funcinfo << "invalid xml string" << endl;
				continue;
			}
			QDomElement root = doc.documentElement();
			if (!u->loadFromXML(root)) {
				boError() << k_funcinfo << "unable to load unit " << u->id() << " from XML" << endl;
				continue;
			}
		}


		// for redo we need the new unit IDs
		message.mMessage.mItems = redoDeleteItems;
		emit signalEditorNewRedoMessage(message.mMessage);
		break;
	}
	default:
		// did not process message
		return false;
 }
 // message processed
 return true;
}

void BosonPlayerInputHandler::editorDeleteItems(const QValueList<Q_ULONG>& items)
{
 BO_CHECK_NULL_RET(canvas());
 canvas()->deleteItems(items);
}

Unit* BosonPlayerInputHandler::editorPlaceUnit(Q_UINT32 owner, Q_UINT32 unitType, const BoVector2Fixed& pos, const bofixed& rotation)
{
 BO_CHECK_NULL_RET0(canvas());

 Player* p = 0;
 if (owner >= 1024) { // a KPlayer ID
	p = findPlayer(owner);
 } else {
	p = (Player*)mGame->playerList()->at(owner);
 }
 if (!p) {
	boError() << k_lineinfo << "Cannot find player " << owner << endl;
	return 0;
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
	return 0;
 }
 if (canvas()->collisions()->cellsOccupied(r)) {
	boWarning() << k_funcinfo << "Cells at (" << pos.x() << "; " << pos.y() << ") are occupied" << endl;
	return 0;
 }*/

 BoVector3Fixed pos3(pos.x(), pos.y(), 0.0f);
 Unit* u = (Unit*)canvas()->createNewItem(RTTI::UnitStart + unitType, p, ItemType(unitType), pos3);
 u->setRotation(rotation);
 u->updateRotation();
 if (u->isFacility()) {
	// Facilities will be fully constructed by default
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
 return u;
}



BosonMessageEditorMove* BosonPlayerInputHandler::createNewUndoDeleteItemsMessage(const BosonMessageEditorMoveDeleteItems& message) const
{
 QValueList<BosonMessageEditorMovePlaceUnit*> placeUnit;
 QValueList<QString> unitData;
 for (unsigned int i = 0; i < message.mItems.count(); i++) {
	unsigned long int id = message.mItems[i];
	BosonItem* item = canvas()->findItem(id);
	if (!item) {
		boError() << k_funcinfo << "cannot find item " << id << endl;
		continue;
	}
	if (!RTTI::isUnit(item->rtti())) {
		boWarning() << k_funcinfo << "undo deletion works for units only" << endl;
		continue;
	}
	Unit* u = (Unit*)item;
	BosonMessageEditorMovePlaceUnit* m;

	QDomDocument doc;
	QDomElement root = doc.createElement("Item");
	if (!u->saveAsXML(root)) {
		boError() << k_funcinfo << "unable to save unit as xml" << endl;
		continue;
	}
	doc.appendChild(root);

	m = new BosonMessageEditorMovePlaceUnit(u->type(), u->owner()->id(), BoVector2Fixed(u->x(), u->y()), u->rotation());
	placeUnit.append(m);
	QString xml = doc.toString();
	unitData.append(xml);
 }

 BosonMessageEditorMoveUndoDeleteItems* undo;
 undo = new BosonMessageEditorMoveUndoDeleteItems(placeUnit, unitData, message);
 while (!placeUnit.isEmpty()) {
	delete placeUnit[0];
	placeUnit.pop_front();
 }
 return undo;
}

