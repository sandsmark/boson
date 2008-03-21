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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "bosonplayerinputhandler.h"
#include "bosonplayerinputhandler.moc"

#include "../../bomemory/bodummymemory.h"
#include "boson.h"
#include "bosonmessageids.h"
#include "bosonmessage.h"
#include "player.h"
#include "unit.h"
#include "unitplugins/unitplugins.h"
#include "speciestheme.h"
#include "unitproperties.h"
#include "bosoncanvas.h"
#include "../global.h"
#include "bodebug.h"
#include "bosonweapon.h"
#include "boevent.h"
#include "unitorder.h"

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

Player* BosonPlayerInputHandler::findPlayerByKGameId(unsigned long int id) const
{
 return (Player*)mGame->findPlayerByKGameId(id);
}

Player* BosonPlayerInputHandler::findPlayerByUserId(int id) const
{
 return (Player*)mGame->findPlayerByUserId(id);
}

bool BosonPlayerInputHandler::playerInput(QDataStream& stream, Player* player)
{
 if (player->isOutOfGame()) {
	boWarning() << k_funcinfo << "Player must not send input anymore!!" << endl;
	return true;
 }
 if (player->bosonId() <= 127) {
	boWarning() << k_funcinfo << "watching players are not allowed to send any input" << endl;
	return true;
 }
 if (player->bosonId() >= 512) {
	// AB: players with ID >= 512 are reserved for future use, so they
	// should not exist at all here. but they certainly shouldn't send any
	// input.
	boWarning() << k_funcinfo << "players with ID >= 512 are not allowed to send any input" << endl;
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
			boError(380) << k_lineinfo << "message (" << message.messageId() << ") could not be read" << endl;
			break;
		}
		bool attack = message.mIsAttack;
		boDebug(380) << "MOVING: " << k_funcinfo << "attack: " << attack << endl;
		QPtrList<Unit> unitsToMove;
		for (QValueList<Q_ULONG>::iterator it = message.mItems.begin(); it != message.mItems.end(); ++it) {
			Q_ULONG unitId = *it;
//			boDebug() << "pos: " << mPos.x() << " " << mPos.y() << endl;
			Unit* unit = findUnit(unitId, player);
			if (!unit) {
				boDebug(380) << "unit " << unitId << " not found for this player" << endl;
				continue;
			}
			if (unit->owner() != player) {
				boDebug(380) << "unit " << unitId << "only owner can move units!" << endl;
				continue;
			}
			if (unit->isDestroyed()) {
				boDebug(380) << "cannot move destroyed units" << endl;
				continue;
			}
//			boDebug() << "move " << unitId << endl;
			if (unit->unitProperties()->isMobile()) {
				unitsToMove.append(unit);
			}
		}
		if (unitsToMove.count() == 0) {
			break;
		}
		giveOrder(unitsToMove, UnitMoveOrder(message.mPos, -1, attack));
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
		QPtrList<Unit> unitsToAttackWith;
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
				unitsToAttackWith.append(unit);
			}
		}
		giveOrder(unitsToAttackWith, UnitAttackOrder(attackedUnit));
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
			if (unit->isInsideUnit()) {
				continue;
			}
			unitsToStop.append(unit);
		}
		if (unitsToStop.count() == 0) {
			break;
		}
		QPtrListIterator<Unit> it(unitsToStop);
		while (it.current()) {
			// TODO: what about delayed orders?
			it.current()->clearOrders();
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
//		boDebug() << "MoveMine" << endl;
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

		Unit* mineunit = findUnit(message.mResourceMineId, 0);
		if (!mineunit) {
			boError() << k_lineinfo << "cannot find resourcemine unit " << message.mResourceMineId << " for player " << player << endl;
			break;
		}
		ResourceMinePlugin* r = (ResourceMinePlugin*)mineunit->plugin(UnitPlugin::ResourceMine);
		if (!r) {
			boError() << k_lineinfo << "can mine at resource mine only" << endl;
			break;
		}
		giveOrder(u, UnitHarvestOrder(mineunit));
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
		Player* refineryOwner = findPlayerByUserId(message.mRefineryOwner);
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
		QPtrList<Unit> units;
		for (QValueList<Q_ULONG>::iterator it = message.mItems.begin(); it != message.mItems.end(); ++it) {
			Q_ULONG unitId = *it;
			Unit* u = findUnit(unitId, player);
			if (!u) {
				boError() << k_lineinfo << "cannot find unit " << unitId << " for player " << player->bosonId() << endl;
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
			units.append(u);
		}
		giveOrder(units, UnitRefineOrder(refineryUnit));
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
		Player* repairOwner= findPlayerByUserId(repairOwnerId);
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
				boError() << k_lineinfo << "cannot find unit " << unitId << " for player " << player->bosonId()  << endl;
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

		Player* p = findPlayerByUserId(message.mOwner);
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
			boError() << k_lineinfo << "Invalid unit type " << message.mType << endl;
			break;
		}

		if (factory->currentPluginType() != UnitPlugin::Production) {
			if ((ProductionType)message.mProduceType == production->currentProductionType() &&
					message.mType == production->currentProductionId()) {
				production->unpauseProduction();
				break;
			}
		}
		if (!production->canCurrentlyProduce((ProductionType)message.mProduceType, (unsigned long int)message.mType)) {
			boError() << k_lineinfo << "Unit "
					<< message.mFactoryId
					<< " can't produce type " <<
					(unsigned long int)message.mType
					<< " of producetype "
					<< message.mProduceType << endl;
			break;
		}
		production->addProduction((ProductionType)message.mProduceType, (unsigned long int)message.mType);

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

		Player* p = findPlayerByUserId(message.mOwner);
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
			break;
		}
		if (message.mType <= 0) {
			boError() << k_lineinfo << "Invalid type " << message.mType << endl;
			break;
		}

		if ((production->currentProductionId() == message.mType) && (production->currentProductionType() == (ProductionType)message.mProduceType)) {
			if (factory->currentPluginType() == UnitPlugin::Production) {
				// do not abort but just pause
				production->pauseProduction();
				break;
			}
		}

		production->abortProduction((ProductionType)message.mProduceType, message.mType);

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

		Player* p = findPlayerByUserId(message.mOwner);
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
		if (followUnit->isDestroyed()) {
			boDebug() << "Cannot follow destroyed units" << endl;
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
			unit->addToplevelOrder(new UnitFollowOrder(followUnit));
		}
		break;
	}
	case BosonMessageIds::MoveEnterUnit:
	{
		BosonMessageMoveEnterUnit message;
		if (!message.load(stream)) {
			boError() << k_lineinfo << "message (" << message.messageId() << ") could not be read" << endl;
			break;
		}
		Unit* enterUnit = findUnit(message.mEnterUnitId, 0);
		if (!enterUnit) {
			boError() << k_lineinfo << "Cannot enter NULL unit" << endl;
			return true;
		}
		if (enterUnit->isDestroyed()) {
			boDebug() << k_lineinfo << "Cannot enter destroyed units" << endl;
			return true;
		}
		UnitStoragePlugin* enterStorage = (UnitStoragePlugin*)enterUnit->plugin(UnitPlugin::UnitStorage);

		if (!enterStorage) {
			boDebug() << k_lineinfo << "Cannot enter unit without UnitStorage plugin" << endl;
			return true;
		}

		for (QValueList<Q_ULONG>::iterator it = message.mItems.begin(); it != message.mItems.end(); ++it) {
			Q_ULONG unitId = *it;
			if (unitId == message.mEnterUnitId) {
				boWarning() << "Cannot enter myself" << endl;
				continue;
			}
			Unit* unit = findUnit(unitId, player);
			if (!unit) {
				boDebug() << "unit " << unitId << " not found for this player" << endl;
				continue;
			}
			if (unit->isDestroyed()) {
				boDebug() << "cannot enter with destroyed units" << endl;
				continue;
			}

			if (!enterStorage->canStore(unit)) {
				continue;
			}

			boDebug() << k_lineinfo << unit->id() << " entering " << enterUnit->id() << endl;
			unit->addToplevelOrder(new UnitEnterUnitOrder(enterUnit));
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

		Player* p = findPlayerByUserId(message.mOwner);
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

		Player* p = findPlayerByUserId(message.mOwner);
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
		emit signalEditorNewUndoMessage(undo, message.isRedo());
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

		// save the original values (for undo)
		QValueVector<bofixed> originalHeights(message.mCellCornersX.count());
		for (Q_UINT32 i = 0; i < message.mCellCornersX.count(); i++) {
			Q_INT32 cornerX = message.mCellCornersX[i];
			Q_INT32 cornerY = message.mCellCornersY[i];
			bofixed height = canvas()->heightAtCorner(cornerX, cornerY);
			originalHeights[i] = height;
		}

		editorChangeHeight(message.mCellCornersX,
				message.mCellCornersY,
				message.mCellCornersHeight);

		BosonMessageEditorMoveChangeHeight originalHeightsMessage(message.mCellCornersX, message.mCellCornersY, originalHeights);
		BosonMessageEditorMoveUndoChangeHeight undo(originalHeightsMessage, message);
		emit signalEditorNewUndoMessage(undo, message.isRedo());
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

		emit signalEditorNewUndoMessage(*undo, message.isRedo());
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
	case BosonMessageIds::MoveUndoChangeHeight:
	{
		BosonMessageEditorMoveUndoChangeHeight message;
		if (!message.load(stream)) {
			boError() << k_lineinfo << "message (" << message.messageId() << ") could not be read" << endl;
			break;
		}

		editorChangeHeight(message.mOriginalHeights.mCellCornersX,
				message.mOriginalHeights.mCellCornersY,
				message.mOriginalHeights.mCellCornersHeight);

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

void BosonPlayerInputHandler::giveOrder(const QPtrList<Unit>& units, const UnitOrder& order, bool replace)
{
 QPtrListIterator<Unit> it(units);
 while (it.current()) {
	giveOrder(it.current(), order, replace);
	++it;
 }
}

void BosonPlayerInputHandler::giveOrder(Unit* unit, const UnitOrder& order_, bool replace)
{
 BO_CHECK_NULL_RET(unit);
 UnitOrder* order = order_.duplicate();

 if (unit->isInsideUnit()) {
	bool needToLeave = true;
	if (order->type() == UnitOrder::EnterUnit) {
		UnitEnterUnitOrder* o = (UnitEnterUnitOrder*)order;
		if (o->isLeaveOrder()) {
			needToLeave = false;
		}
	}

	if (needToLeave) {
		UnitEnterUnitOrder leave(unit);
		leave.setIsLeaveOrder(true);
		giveOrder(unit, leave, replace);
		replace = false;
	}
 }

 if (replace) {
	unit->replaceToplevelOrders(order);
 } else {
	unit->addToplevelOrder(order);
 }
}

void BosonPlayerInputHandler::editorDeleteItems(const QValueList<Q_ULONG>& items)
{
 BO_CHECK_NULL_RET(canvas());
 canvas()->deleteItems(items);
}

Unit* BosonPlayerInputHandler::editorPlaceUnit(Q_UINT32 owner, Q_UINT32 unitType, const BoVector2Fixed& pos, const bofixed& rotation)
{
 BO_CHECK_NULL_RET0(canvas());

 Player* p = findPlayerByUserId(owner);
 if (!p) {
	boError() << k_lineinfo << "Cannot find player " << owner << endl;
	return 0;
 }

 // First make sure unit can be placed there
 // We must only check if cells aren't occupied and if unit can go there,
 //  because everything else should be checked before placing anything. But
 //  occupied status of cell might have changed already.
 /*const UnitProperties* prop = p->speciesTheme()->unitProperties(unitType);
 bofixed width = prop->unitWidth();
 bofixed height = prop->unitHeight();
 BoRect2Fixed r(pos, pos + BoVector2Fixed(width, height));
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
	UnitConstruction* c = u->construction();

	// Facilities will be fully constructed by default
	c->setConstructionStep(c->constructionSteps());
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

	m = new BosonMessageEditorMovePlaceUnit(u->type(), u->owner()->bosonId(), BoVector2Fixed(u->x(), u->y()), u->rotation());
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


void BosonPlayerInputHandler::editorChangeHeight(const QValueVector<Q_UINT32>& cellCornersX, const QValueVector<Q_UINT32>& cellCornersY, const QValueVector<bofixed>& cellCornersHeight)
{
 if (cellCornersX.count() != cellCornersY.count() || cellCornersX.count() != cellCornersHeight.count()) {
	boError() << k_funcinfo << "invalid sizes" << endl;
	return;
 }

 // TODO: unify. either store float or bofixed, storing float in
 // the map and bofixed in the message is not good.
 QValueList< QPair<QPoint, float> > heights;
 for (Q_UINT32 i = 0; i < cellCornersX.count(); i++) {
	// note: cornerX == mapWidth() and cornerY == mapHeight()
	// are valid!
	Q_INT32 cornerX = cellCornersX[i];
	Q_INT32 cornerY = cellCornersY[i];
	bofixed height = cellCornersHeight[i];
	if (cornerX < 0 || (unsigned int)cornerX > canvas()->mapWidth()) {
		boError() << k_funcinfo << "invalid x coordinate " << cornerX << endl;
		continue;
	}
	if (cornerY < 0 || (unsigned int)cornerY > canvas()->mapHeight()) {
		boError() << k_funcinfo << "invalid y coordinate " << cornerY << endl;
		continue;
	}
	QPoint p(cornerX, cornerY);
	heights.append(QPair<QPoint, float>(p, height));
 }

 canvas()->setHeightsAtCorners(heights);

 for (Q_UINT32 i = 0; i < cellCornersX.count(); i++) {
	Q_INT32 cornerX = cellCornersX[i];
	Q_INT32 cornerY = cellCornersY[i];
	float height = canvas()->heightAtCorner(cornerX, cornerY);

	// TODO: find out whether we actually still need this
	// signal. if we do, find out whether we can use a list
	// instead.
	// this is currently terribly inefficient for many
	// corners at once.
	emit signalChangeHeight(cornerX, cornerY, height);
 }
}

