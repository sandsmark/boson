/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include "boson.h"

#include "defines.h"
#include "bosonmessage.h"
#include "player.h"
#include "unit.h"
#include "unitplugins.h"
#include "speciestheme.h"
#include "unitproperties.h"
#include "bosoncanvas.h"
#include "bosonscenario.h"
#include "bosonstatistics.h"
#include "bosonplayfield.h"
#include "global.h"
#include "upgradeproperties.h"

#include <kdebug.h>
#include <klocale.h>
#include <kgame/kgameio.h>

#include <qbuffer.h>
#include <qtimer.h>
#include <qdatetime.h>
#include <qptrlist.h>
#include <qptrqueue.h>
#include <qdom.h>
#include <qdatastream.h>

#include "boson.moc"

Boson* Boson::mBoson = 0;

#define ADVANCE_INTERVAL 250 // ms

// the advance message <-> advance call thing is pretty new. i want to have
// debug output from it in bug reports, so please do not define this.
// on the other hand i often need to test a few things where i don't need those
// messages. defining this lets easily disable the debug output.
#define NO_ADVANCE_DEBUG

// Saving format version (000005 = 00.00.05)
#define BOSON_SAVEGAME_FORMAT_VERSION 000007

class BoMessage
{
public:
	QByteArray byteArray;
	int msgid;
	Q_UINT32 receiver;
	Q_UINT32 sender;
	Q_UINT32 clientId;
};

class Boson::BosonPrivate
{
public:
	BosonPrivate()
	{
		mGameTimer = 0;
		mCanvas = 0;
		mPlayField = 0;
		mPlayer = 0;

		mAdvanceDividerCount = 0;

		mLoadingStatus = Boson::NotLoaded;
	}

	QTimer* mGameTimer;

// stuff for the message-delaying feature
	QTime mAdvanceReceived; // when the last advance *message* was received
	int mAdvanceDivider; // pretty much exactly gameSpeed()
	int mAdvanceDividerCount; // how many advance *calls* have been made since the last advance *message*
	int mAdvanceMessageWaiting;
	QPtrQueue<BoMessage> mDelayedMessages;
	bool mIsLocked;
	bool mDelayedWaiting; // FIXME bad name!

	BosonCanvas* mCanvas; // this pointer is anti-OO IMHO
	BosonPlayField* mPlayField;
	Player* mPlayer;
	QPtrList<KGameComputerIO> mComputerIOList;
	
	KGamePropertyInt mGameSpeed;

	KGameProperty<unsigned long int> mNextUnitId;
	KGameProperty<unsigned int> mAdvanceCount;
	KGamePropertyInt mAdvanceFlag;

	Boson::LoadingStatus mLoadingStatus;
};

Boson::Boson(QObject* parent) : KGame(BOSON_COOKIE, parent)
{
 setPolicy(PolicyClean);
 d = new BosonPrivate;

 d->mGameTimer = new QTimer(this);
 d->mAdvanceDivider = 1;
 d->mIsLocked = false;
 d->mDelayedWaiting = false;
 d->mAdvanceMessageWaiting = 0;

 mGameMode = true;

 connect(this, SIGNAL(signalNetworkData(int, const QByteArray&, Q_UINT32, Q_UINT32)),
		this, SLOT(slotNetworkData(int, const QByteArray&, Q_UINT32, Q_UINT32)));
 connect(this, SIGNAL(signalSave(QDataStream&)),
		this, SLOT(slotSave(QDataStream&)));
 connect(this, SIGNAL(signalLoad(QDataStream&)),
		this, SLOT(slotLoad(QDataStream&)));
 connect(this, SIGNAL(signalReplacePlayerIO(KPlayer*, bool*)),
		this, SLOT(slotReplacePlayerIO(KPlayer*, bool*)));
 connect(this, SIGNAL(signalPlayerJoinedGame(KPlayer*)),
		this, SLOT(slotPlayerJoinedGame(KPlayer*)));
 connect(this, SIGNAL(signalPlayerLeftGame(KPlayer*)),
		this, SLOT(slotPlayerLeftGame(KPlayer*)));
 connect(this, SIGNAL(signalAdvance(unsigned int, bool)),
		this, SLOT(slotAdvanceComputerPlayers(unsigned int, bool)));
 connect(dataHandler(), SIGNAL(signalPropertyChanged(KGamePropertyBase*)),
		this, SLOT(slotPropertyChanged(KGamePropertyBase*)));
 d->mGameSpeed.registerData(IdGameSpeed, dataHandler(),
		KGamePropertyBase::PolicyClean, "GameSpeed");
 d->mNextUnitId.registerData(IdNextUnitId, dataHandler(),
		KGamePropertyBase::PolicyLocal, "NextUnitId");
 d->mAdvanceCount.registerData(IdAdvanceCount, dataHandler(),
		KGamePropertyBase::PolicyLocal, "AdvanceCount");
 d->mAdvanceFlag.registerData(IdAdvanceFlag, dataHandler(),
		KGamePropertyBase::PolicyLocal, "AdvanceFlag");
 d->mNextUnitId.setLocal(0);
 d->mAdvanceCount.setLocal(0);
 d->mGameSpeed.setLocal(0);
 d->mAdvanceFlag.setLocal(0);
 d->mAdvanceCount.setEmittingSignal(false); // wo don't need it and it would be bad for performance.
}

Boson::~Boson()
{
 delete d->mGameTimer;
 delete d;
}

void Boson::initBoson()
{
 mBoson = new Boson(0);
}

void Boson::deleteBoson()
{
 delete mBoson;
 mBoson = 0;
}

void Boson::setCanvas(BosonCanvas* c)
{
 d->mCanvas = c;
}

void Boson::setPlayField(BosonPlayField* p)
{
 kdDebug() << k_funcinfo << endl;
 d->mPlayField = p;
}

Player* Boson::localPlayer()
{
 return d->mPlayer;
}

void Boson::setLocalPlayer(Player* p)
{
 d->mPlayer = p;
}

void Boson::quitGame()
{
// kdDebug() << k_funcinfo << endl;
// reset everything
 d->mGameTimer->stop();
 setGameStatus(KGame::End);
 d->mNextUnitId = 0;

 // remove all players from game
 removeAllPlayers();
// kdDebug() << k_funcinfo << " done" <<  endl;
}

void Boson::removeAllPlayers()
{
 QPtrList<KPlayer> list = *playerList();
 for (unsigned int i = 0; i < list.count(); i++) {
	removePlayer(list.at(i)); // might not be necessary - sends remove over network
	systemRemovePlayer(list.at(i), true); // remove immediately, even before network removing is received.
 }
}

bool Boson::playerInput(QDataStream& stream, KPlayer* p)
{
 Player* player = (Player*)p;
 if (player->isOutOfGame()) {
	kdWarning() << k_funcinfo << "Player must not send input anymore!!" << endl;
	return true;
 }
 if (!gameMode()) {
	// editor mode sends an additional entry safety id, just in case we
	// might have constructed a wrong display or so
	Q_UINT32 editor;
	stream >> editor;
	if (editor != BosonMessage::MoveEditor) {
		kdError() << k_funcinfo << "Not an editor message, elthough we're in editor mode!" << endl;
		return true;
	}
 }
 Q_UINT32 msgid;
 stream >> msgid;
 switch (msgid) {
	case BosonMessage::MoveMove:
	{
		QPoint pos;
		Q_UINT32 unitCount;
		stream >> pos;
		stream >> unitCount;
		QPtrList<Unit> unitsToMove;
		for (unsigned int i = 0; i < unitCount; i++) {
			Q_ULONG unitId;
			stream >> unitId;
//			kdDebug() << "pos: " << pos.x() << " " << pos.y() << endl;
			Unit* unit = findUnit(unitId, player);
			if (!unit) {
				kdDebug() << "unit " << unitId << " not found for this player" << endl;
				continue;
			}
			if (unit->owner() != player) {
				kdDebug() << "unit " << unitId << "only owner can move units!" << endl;
				continue;
			}
			if (unit->isDestroyed()) {
				kdDebug() << "cannot move destroyed units" << endl;
				continue;
			}
//			kdDebug() << "move " << unitId << endl;
			if (unit->unitProperties()->isMobile()) {
				unitsToMove.append(unit);
			}
		}
		if (unitsToMove.count() == 0) {
			kdWarning() << k_lineinfo << "no unit to move" << endl;
			break;
		}
		QPtrListIterator<Unit> it(unitsToMove);
		while (it.current()) {
			it.current()->stopMoving();
			++it;
		}
		if (unitsToMove.count() == 1) {
			unitsToMove.first()->moveTo(pos);
		} else {
			it.toFirst();
			while (it.current()) {
				it.current()->moveTo(pos);
				++it;
			}
		}
		break;
	}
	case BosonMessage::MoveAttack:
	{
		Q_ULONG attackedUnitId;
		Q_UINT32 unitCount;
		stream >> attackedUnitId;
		stream >> unitCount;
		Unit* attackedUnit = findUnit(attackedUnitId, 0);
		if (!attackedUnit) {
			kdError() << "Cannot attack NULL unit" << endl;
			return true;
		}
		for (unsigned int i = 0; i < unitCount; i++) {
			Q_ULONG unitId;
			stream >> unitId;
			if (unitId == attackedUnitId) {
				// can become possible one day - e.g. when
				// repairing a unit
				kdWarning() << "Can not (yet) attack myself" 
						<< endl;
				continue;
			}
			Unit* unit = findUnit(unitId, player);
			if (!unit) {
				kdDebug() << "unit " << unitId << " not found for this player" << endl;
				continue;
			}
			if (unit->isDestroyed()) {
				kdDebug() << "cannot attack with destroyed units" << endl;
				continue;
			}
			if (attackedUnit->isDestroyed()) {
				kdDebug() << "no sense in attacking destroyed units" << endl;
				continue;
			}
			if (unit->weaponRange() > 0 && unit->weaponDamage() > 0) {
				kdDebug() << unitId << " attacks " << attackedUnitId << endl;
				if (unit->weaponDamage() != 0) {
					unit->setTarget(attackedUnit);
					if (unit->target()) {
						unit->setWork(Unit::WorkAttack);
					}
				}
			}
		}
		break;
	}
	case BosonMessage::MoveStop:
	{
		Q_UINT32 unitCount;
		stream >> unitCount;
		QPtrList<Unit> unitsToStop;
		for (unsigned int i = 0; i < unitCount; i++) {
			Q_ULONG unitId;
			stream >> unitId;
			Unit* unit = findUnit(unitId, player);
			if (!unit) {
				kdDebug() << "unit " << unitId << " not found for this player" << endl;
				continue;
			}
			if (unit->owner() != player) {
				kdDebug() << "unit " << unitId << "only owner can stop units!" << endl;
				continue;
			}
			if (unit->isDestroyed()) {
				kdDebug() << "cannot stop destroyed units" << endl;
				continue;
			}
			unitsToStop.append(unit);
		}
		if (unitsToStop.count() == 0) {
			kdWarning() << k_lineinfo << "no unit to stop" << endl;
			break;
		}
		QPtrListIterator<Unit> it(unitsToStop);
		while (it.current()) {
			it.current()->stopAttacking();  // call stopAttacking() because it also sets unit's work to WorkNone... and it doesn't hurt
			++it;
		}
		break;
	}
	case BosonMessage::MoveMine:
	{
		kdDebug() << "MoveMine" << endl;
		Q_ULONG unitId;
		QPoint pos;
		stream >> unitId;
		stream >> pos;
		Unit* u = findUnit(unitId, player);
		if (!u) {
			kdError() << k_lineinfo << "cannot find unit " << unitId << " for player " << player << endl;
			break;
		}
		if (!u->isMobile()) {
			kdError() << k_lineinfo << "only mobile units can mine" << endl;
			break;
		}
		HarvesterPlugin* h = (HarvesterPlugin*)u->plugin(UnitPlugin::Harvester);
		if (!h) {
			kdError() << k_lineinfo << "only harvester can mine" << endl;
			break;
		}
		if (u->owner() != player) {
			kdDebug() << k_funcinfo << "unit " << unitId << "only owner can move units!" << endl;
			break;
		}
		if (u->isDestroyed()) {
			kdDebug() << "cannot mine with destroyed units" << endl;
			break;
		}
		h->mineAt(pos);
		break;
	}
	case BosonMessage::MoveRefine:
	{
		kdDebug() << "MoveRefine" << endl;
		Q_UINT32 refineryOwnerId;
		Q_ULONG refineryId;
		Q_UINT32 unitCount;
		stream >> refineryOwnerId;
		stream >> refineryId;
		stream >> unitCount;
		Player* refineryOwner = (Player*)findPlayer(refineryOwnerId);
		if (!refineryOwner) {
			kdError() << k_lineinfo << "Cannot find player " << refineryOwnerId << endl;
			break;
		}
		if (player->isEnemy(refineryOwner)) {
			kdError() << k_lineinfo << "Cannot go to enemy refinery" << endl;
			break;
		}
		Unit* refinery = findUnit(refineryId, refineryOwner);
		if (!refinery) {
			kdError() << k_lineinfo << "cannot find refinery " << refineryId << " for player " << refineryOwnerId << endl;
			break;
		}
#warning TODO
//		if (!refinery->plugin(UnitPlugin::Refinery)) {
		if (!refinery->isFacility()) { // FIXME do not depend on facility!
			kdWarning() << k_lineinfo << "refinery must be a facility" << endl;
			break;
		}
		for (unsigned int i = 0; i < unitCount; i++) {
			Q_ULONG unitId;
			stream >> unitId;
			Unit* u = findUnit(unitId, player);
			if (!u) {
				kdError() << k_lineinfo << "cannot find unit " << unitId << " for player " << player->id() << endl;
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
				kdError() << k_lineinfo << "must be a harvester" << endl;
				continue;
			}
			h->refineAt((Facility*)refinery);
		}
		break;
		
	}
	case BosonMessage::MoveRepair:
	{
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
		Player* repairOwner= (Player*)findPlayer(repairOwnerId);
		if (!repairOwner) {
			kdError() << k_lineinfo << "Cannot find player " << repairOwnerId << endl;
			break;
		}
		if (player->isEnemy(repairOwner)) {
			kdError() << k_lineinfo << "Cannot move to enemy repairyard" << endl;
			break;
		}
		Unit* repairYard = findUnit(repairId, repairOwner);
		if (!repairYard) {
			kdError() << k_lineinfo << "Cannot find " << repairId << " for player " << repairOwnerId << endl;
			break;
		}
		RepairPlugin* repair = repairYard->repairPlugin();
		if (!repair) {
			kdError() << k_lineinfo << "repairyard cannot repair?!" << endl;
			break;
		}
		for (unsigned int i = 0; i < unitCount; i++) {
			Q_ULONG unitId;
			stream >> unitId;
			Unit* u = findUnit(unitId, player);
			if (!u) {
				kdError() << k_lineinfo << "cannot find unit " << unitId << " for player " << player->id()  << endl;
				continue;
			}
			if (!u->isMobile()) {
				kdError() << k_lineinfo << "must be a mobile unit" << endl;
				continue;
			}
			repair->repair(u);
		}
		break;
	}
	case BosonMessage::MoveProduce:
	{
		Q_UINT32 productionType;
		Q_UINT32 owner;
		Q_ULONG factoryId;
		Q_UINT32 id;

		stream >> productionType;

		stream >> owner;
		stream >> factoryId;
		stream >> id;

		Player* p = (Player*)findPlayer(owner);
		if (!p) {
			kdError() << k_lineinfo << "Cannot find player " << owner << endl;
			break;
		}
		Unit* factory = findUnit(factoryId, p);
		if (!factory) {
			kdError() << "Cannot find unit " << factoryId << endl;
			break;
		}

		ProductionPlugin* production = (ProductionPlugin*)factory->plugin(UnitPlugin::Production);
		if (!production) {
			// maybe not yet fully constructed
			kdWarning() << k_lineinfo << factory->id() << " cannot produce" << endl;
			break;
		}
		if (id <= 0) {
			kdError() << k_lineinfo << "Invalid id " << id << endl;
			break;
		}

		unsigned long int mineralCost = 0, oilCost = 0;

		if ((ProductionType)productionType == ProduceUnit) {
			// Produce unit
			const UnitProperties* prop = p->unitProperties(id);
			if (!prop) {
				kdError() << k_lineinfo << "NULL unit properties (EVIL BUG)" << endl;
				break;
			}
			mineralCost = prop->mineralCost();
			oilCost = prop->oilCost();
		} else if ((ProductionType)productionType == ProduceTech) {
			// Produce technology
			const TechnologyProperties* prop = p->speciesTheme()->technology(id);
			if (!prop) {
				kdError() << k_lineinfo << "NULL technology properties (EVIL BUG)" << endl;
				break;
			}
			mineralCost = prop->mineralCost();
			oilCost = prop->oilCost();
		} else {
			kdError() << k_funcinfo << "Invalid productionType: " << productionType << endl;
		}

			if (factory->currentPluginType() != UnitPlugin::Production) {
				if ((production->currentProductionId() == id) && (production->currentProductionType() == (ProductionType)productionType)) {
					// production was stopped - continue it now
					factory->setPluginWork(UnitPlugin::Production);
					emit signalUpdateProduction(factory);
					break;
				}
			}

		if (p->minerals() < mineralCost) {
			if (p == localPlayer()) {
				slotAddChatSystemMessage(i18n("You have not enough minerals!"));

			}
			break;
		}
		if (p->oil() < oilCost) {
			if (p == localPlayer()) {
				slotAddChatSystemMessage(i18n("You have not enough oil!"));
			}
			break;
		}
		p->setMinerals(p->minerals() - mineralCost);
		p->setOil(p->oil() - oilCost);
		production->addProduction((ProductionType)productionType, (unsigned long int)id);
		emit signalUpdateProduction(factory);
		break;
	}
	case BosonMessage::MoveProduceStop:
	{
		kdDebug() << "MoveProduceStop" << endl;
		Q_UINT32 productionType;
		Q_UINT32 owner;
		Q_ULONG factoryId;
		Q_UINT32 id;

		stream >> productionType;

		stream >> owner;
		stream >> factoryId;
		stream >> id;

		Player* p = (Player*)findPlayer(owner);
		if (!p) {
			kdError() << k_lineinfo << "Cannot find player " << owner << endl;
			break;
		}
		Unit* factory = findUnit(factoryId, p);
		if (!factory) {
			kdError() << "Cannot find unit " << factoryId << endl;
			break;
		}

		ProductionPlugin* production = (ProductionPlugin*)factory->plugin(UnitPlugin::Production);
		if (!production) {
			// should not happen here!
			kdError() << k_lineinfo << factory->id() << "cannot produce?!" << endl;
			break;
		}
		if (!production->contains((ProductionType)productionType, (unsigned long int)id)) {
			kdDebug() << k_lineinfo << "Production " << productionType << " with id "
					 << id << " is not in production queue" << endl;
			return true;
		}
		if (id <= 0) {
			kdError() << k_lineinfo << "Invalid id " << id << endl;
			break;
		}

		unsigned long int mineralCost = 0, oilCost = 0;

		if ((ProductionType)productionType == ProduceUnit) {
			const UnitProperties* prop = p->unitProperties(id);
			if (!prop) {
				kdError() << k_lineinfo << "NULL unit properties (EVIL BUG)" << endl;
				break;
			}
			mineralCost = prop->mineralCost();
			oilCost = prop->oilCost();
		} else if ((ProductionType)productionType == ProduceTech) {
			const TechnologyProperties* prop = p->speciesTheme()->technology(id);
			if (!prop) {
				kdError() << k_lineinfo << "NULL technology properties (EVIL BUG)" << endl;
				break;
			}
			mineralCost = prop->mineralCost();
			oilCost = prop->oilCost();
		} else {
			kdError() << k_funcinfo << "Invalid productionType: " << productionType << endl;
		}

		if ((production->currentProductionId() == id) && (production->currentProductionType() == (ProductionType)productionType)) {
			if (factory->currentPluginType() == UnitPlugin::Production) {
				// do not abort but just pause
				factory->setWork(Unit::WorkNone);
				emit signalUpdateProduction(factory);
			} else {
				p->setMinerals(p->minerals() + mineralCost);
				p->setOil(p->oil() + oilCost);
				production->removeProduction();
				emit signalUpdateProduction(factory);
			}
		} else {
			// not the current, but a queued production is stopped.

			//FIXME: money should be paid when the production is
			//actually started! (currently it is paid as soon as an
			//item is added to the queue)
			p->setMinerals(p->minerals() + mineralCost);
			p->setOil(p->oil() + oilCost);
			production->removeProduction((ProductionType)productionType, (unsigned long int)id);
			emit signalUpdateProduction(factory);
		}
		break;
	}
	case BosonMessage::MoveBuild:
	{
		kdDebug() << "MoveBuild" << endl;
		Q_UINT32 productionType;
		Q_ULONG factoryId;
		Q_UINT32 owner;
		Q_INT32 x;
		Q_INT32 y;

		stream >> productionType;

		stream >> factoryId;
		stream >> owner;
		stream >> x;
		stream >> y;

		// Only units are "built"
		if ((ProductionType)productionType != ProduceUnit) {
			kdError() << k_funcinfo << "Invalid productionType: " << productionType << endl;
			break;
		}

		Player* p = (Player*)findPlayer(owner);
		if (!p) {
			kdError() << k_lineinfo << "Cannot find player " << owner << endl;
			break;
		}
		Unit* factory = findUnit(factoryId, p);
		if (!factory) {
			kdError() << "Cannot find unit " << factoryId << endl;
			break;
		}
		ProductionPlugin* production = (ProductionPlugin*)factory->plugin(UnitPlugin::Production);
		if (!production) {
			// should not happen here!
			kdError() << k_lineinfo << factory->id() << "cannot produce?!" << endl;
			break;
		}
		if (production->completedProductionType() != ProduceUnit) {
			kdError() << k_lineinfo << "not producing unit!" << endl;
			break;
		}
		int unitType = production->completedProductionId();
		kdDebug() << k_lineinfo
				<< "factory="
				<< factory->id()
				<< ",unitid="
				<< unitType
				<< endl;
		if (unitType <= 0) {
			// hope this is working...
			kdWarning() << k_lineinfo << "not yet completed" << endl;
			break;
		}
		buildProducedUnit(production, unitType, x, y);
		break;
	}
	case BosonMessage::MoveFollow:
	{
		Q_ULONG followUnitId;
		Q_UINT32 unitCount;
		stream >> followUnitId;
		stream >> unitCount;
		Unit* followUnit = findUnit(followUnitId, 0);
		if (!followUnit) {
			kdError() << "Cannot follow NULL unit" << endl;
			return true;
		}
		for (unsigned int i = 0; i < unitCount; i++) {
			Q_ULONG unitId;
			stream >> unitId;
			if (unitId == followUnitId) {
				kdWarning() << "Cannot follow myself" << endl;
				continue;
			}
			Unit* unit = findUnit(unitId, player);
			if (!unit) {
				kdDebug() << "unit " << unitId << " not found for this player" << endl;
				continue;
			}
			if (unit->isDestroyed()) {
				kdDebug() << "cannot follow with destroyed units" << endl;
				continue;
			}
			if (followUnit->isDestroyed()) {
				kdDebug() << "Cannot follow destroyed units" << endl;
				continue;
			}
			unit->setTarget(followUnit);
			if (unit->target()) {
				unit->setWork(Unit::WorkFollow);
			}
		}
		break;
	}
	case BosonMessage::MovePlaceUnit:
	{
		Q_UINT32 unitType;
		Q_UINT32 owner;
		Q_INT32 x;
		Q_INT32 y;

		stream >> owner;
		stream >> unitType;
		stream >> x;
		stream >> y;

		KPlayer* p = 0;
		if (owner >= 1024) { // a KPlayer ID
			p = findPlayer(owner);
		} else {
			p = playerList()->at(owner);
		}
		if (!p) {
			kdError() << k_lineinfo << "Cannot find player " << owner << endl;
			break;
		}

		addUnit(unitType, (Player*)p, x, y);
		break;
	}
	case BosonMessage::MovePlaceCell:
	{
		Q_INT32 tile;
		Q_UINT8 version;
		Q_INT8 isBigTrans;
		Q_INT32 x;
		Q_INT32 y;

		stream >> tile;
		stream >> version;
		stream >> isBigTrans;
		stream >> x;
		stream >> y;
		
		emit signalChangeCell(x, y, tile, (unsigned char)version);
		if (isBigTrans) {
			emit signalChangeCell(x + 1, y, tile + 1, (unsigned char)version);
			emit signalChangeCell(x, y + 1, tile + 2, (unsigned char)version);
			emit signalChangeCell(x + 1, y + 1, tile + 3, (unsigned char)version);
		}
		break;
	}
	default:
		kdWarning() << k_funcinfo << "unexpected playerInput " << msgid << endl;
		break;
 }
 return true;
}

void Boson::slotNetworkData(int msgid, const QByteArray& buffer, Q_UINT32 , Q_UINT32 )
{
 QDataStream stream(buffer, IO_ReadOnly);
 switch (msgid) {
	case BosonMessage::AddUnit:
	{
		Q_UINT32 owner;
		Q_UINT32 unitType;
		Q_INT32 x;
		Q_INT32 y;

		stream >> owner;
		stream >> unitType;
		stream >> x;
		stream >> y;
		
		KPlayer* p = 0;
		if (owner >= 1024) { // a KPlayer ID
			p = findPlayer(owner);
		} else {
			p = playerList()->at(owner);
		}
		if (!p) {
			kdError() << k_lineinfo << "Cannot find player " << owner << endl;
			break;
		}
		addUnit(unitType, (Player*)p, x, y);
		break;
	}
	case BosonMessage::AddUnitsXML:
	{
		Q_UINT32 owner;
		stream >> owner;

		QString xmlDocument;
		stream >> xmlDocument;

		KPlayer* p = 0;
		if (owner >= 1024) { // a KPlayer ID
			p = findPlayer(owner);
		} else {
			p = playerList()->at(owner);
		}
		if (!p) {
			kdError() << k_lineinfo << "Cannot find player " << owner << endl;
			break;
		}

		QDomDocument doc;
		doc.setContent(xmlDocument);
		QDomElement root = doc.documentElement();
		QDomNodeList list = root.elementsByTagName("Unit");
		for (unsigned int i = 0; i < list.count(); i++) {
			QDomElement e = list.item(i).toElement();
			addUnit(e, (Player*)p);
		}
		break;
	}
	case BosonMessage::AdvanceN:
	{
		int n = gameSpeed();
		d->mAdvanceDivider = n;
		d->mAdvanceDividerCount = 0;
		lock();
#ifndef NO_ADVANCE_DEBUG
		kdDebug() << "Advance - speed (calls per " << ADVANCE_INTERVAL 
				<< "ms)=" << gameSpeed() << " elapsed: " 
				<< d->mAdvanceReceived.elapsed() << endl;
#endif
		d->mAdvanceReceived.restart();
		slotReceiveAdvance();
		break;
	}
	case BosonMessage::InitMap:
		emit signalInitMap(buffer);
		break;
	case BosonMessage::IdInitFogOfWar:
		if (isRunning()) {
			return;
		}
		emit signalInitFogOfWar();
		break;
	case BosonMessage::IdStartScenario:
		if (isRunning()) {
			return;
		}
		emit signalStartScenario();
		break;
	case BosonMessage::IdNewGame:
		setGameMode(true);
		QTimer::singleShot(0, this, SIGNAL(signalStartNewGame()));
		break;
	case BosonMessage::IdNewEditor:
		setGameMode(false);
		QTimer::singleShot(0, this, SIGNAL(signalStartNewGame()));
		break;
	case BosonMessage::IdGameIsStarted:
		if (!isRunning()) {
			kdError() << "Received IdGameIstarted but it isn't" << endl;
			return;
		}
		emit signalGameStarted();
		break;
	case BosonMessage::ChangeSpecies:
	{
		Q_UINT32 id;
		QString species;
		Q_UINT32 color;
		stream >> id;
		stream >> species;
		stream >> color;
		Player* p = (Player*)findPlayer(id);
		if (!p) {
			kdError() << k_lineinfo << "Cannot find player " << id << endl;
			return;
		}
		p->loadTheme(SpeciesTheme::speciesDirectory(species), QColor(color));
		emit signalSpeciesChanged(p);
		break;
	}
	case BosonMessage::ChangeTeamColor:
	{
		Q_UINT32 id;
		Q_UINT32 color;
		stream >> id;
		stream >> color;
		Player* p = (Player*)findPlayer(id);
		if (!p) {
			kdError() << k_lineinfo << "Cannot find player " << id << endl;
			return;
		}
		if (!p->speciesTheme()) {
			kdError() << k_lineinfo << "NULL speciesTheme for " << id << endl;
			return;
		}
		if (p->speciesTheme()->setTeamColor(QColor(color))) {
			emit signalTeamColorChanged(p);
		} else {
			kdWarning() << k_lineinfo << "could not change color for " << id << endl;
		}
		break;
	}
	case BosonMessage::ChangePlayField:
	{
		QString field;
		stream >> field;
		emit signalPlayFieldChanged(field);
		break;
	}
	case BosonMessage::IdChat:
	{
		break;
	}
	default:
		kdWarning() << k_funcinfo << "unhandled msgid " << msgid << endl;
		break;
 }
}

bool Boson::isServer() const
{
 return isAdmin(); // or isMaster() ??
}

void Boson::startGame()
{
 setGameStatus(KGame::Run);
 if (isServer()) {
	connect(d->mGameTimer, SIGNAL(timeout()), this, SLOT(slotSendAdvance()));
 } else {
	kdWarning() << "is not server - cannot start the game!" << endl;
 }
}

void Boson::slotSendAdvance()
{
 sendMessage(10, BosonMessage::AdvanceN);
}

Unit* Boson::createUnit(unsigned long int unitType, Player* owner)
{
 if (!owner) {
	kdError() << k_funcinfo << "NULL owner" << endl;
	return 0;
 }
 SpeciesTheme* theme = owner->speciesTheme();
 if (!theme) {
	kdError() << k_funcinfo << "No theme for this player" << endl;
	return 0; // BAAAAD - will crash
 }
 const UnitProperties* prop = theme->unitProperties(unitType);
 if (!prop) {
	kdError() << "Unknown unitType " << unitType << endl;
	return 0;
 }

 Unit* unit = 0;
 if (prop->isMobile()) {
	unit = new MobileUnit(prop, owner, d->mCanvas);
 } else if (prop->isFacility()) {
	unit = new Facility(prop, owner, d->mCanvas);
 } else { // should be impossible
	kdError() << k_funcinfo << "invalid unit type " << unitType << endl;
	return 0;
 }
 owner->addUnit(unit); // can also be in Unit c'tor - is this clean?
 theme->loadNewUnit(unit);

 return unit;
}

Unit* Boson::loadUnit(unsigned long int unitType, Player* owner)
{
 if (!owner) {
	kdError() << k_funcinfo << "NULL owner" << endl;
	return 0;
 }
 SpeciesTheme* theme = owner->speciesTheme();
 if (!theme) {
	kdError() << k_funcinfo << "No theme for this player" << endl;
	return 0; // BAAAAD - will crash
 }
 const UnitProperties* prop = theme->unitProperties(unitType);
 if (!prop) {
	kdError() << "Unknown unitType " << unitType << endl;
	return 0;
 }

 Unit* unit = 0;
 if (prop->isMobile()) {
	unit = new MobileUnit(prop, owner, d->mCanvas);
 } else if (prop->isFacility()) {
	unit = new Facility(prop, owner, d->mCanvas);
 } else { // should be impossible
	kdError() << k_funcinfo << "invalid unit type " << unitType << endl;
	return 0;
 }

 return unit;
}

unsigned long int Boson::nextUnitId()
{
 d->mNextUnitId = d->mNextUnitId + 1;
//kdDebug() << "next id: " << d->mNextUnitId << endl;
 return d->mNextUnitId;
}

Unit* Boson::findUnit(unsigned long int id, Player* searchIn) const
{
 if (searchIn) {
	return searchIn->findUnit(id);
 }
 QPtrListIterator<KPlayer> it(*playerList());
 while (it.current()) {
	Unit* unit = ((Player*)it.current())->findUnit(id);
	if (unit) {
		return unit;
	}
	++it;
 }
 return 0;
}

KPlayer* Boson::createPlayer(int rtti, int io, bool isVirtual)
{
 kdDebug() << k_funcinfo << "rtti=" << rtti << ",io=" << io
		<< ",isVirtual=" << isVirtual << endl;
 Player* p = new Player();
 p->setGame(this);
 if (d->mPlayField && d->mPlayField->map()) {
	// AB: this will never be reached. unused. can probably be removed.
	p->initMap(d->mPlayField->map(), boGame->gameMode());
 }
 connect(p, SIGNAL(signalUnitLoaded(Unit*, int, int)),
		this, SIGNAL(signalAddUnit(Unit*, int, int)));
 return p;
}

int Boson::gameSpeed() const
{
 return d->mGameSpeed;
}

void Boson::slotSetGameSpeed(int speed)
{
 kdDebug() << k_funcinfo << " speed = " << speed << endl;
 if (d->mGameSpeed == speed) {
	return; // do not restart timer
 }
 if (speed < 0) {
	kdError() << "Invalid speed value " << speed << endl;
	return;
 }
 if ((speed < MIN_GAME_SPEED || speed > MAX_GAME_SPEED) && speed != 0) {
	kdWarning() << "unexpected speed " << speed << " - pausing" << endl;
	d->mGameSpeed = 0;
	return;
 }
 kdDebug() << k_funcinfo << "Setting speed to " << speed << endl;
 d->mGameSpeed = speed;
}

void Boson::slotPropertyChanged(KGamePropertyBase* p)
{
 switch (p->id()) {
	case IdGameSpeed:
		kdDebug() << k_funcinfo << "speed has changed, new speed: " << gameSpeed() << endl;
		if (isServer()) {
			if (d->mGameSpeed == 0) {
				if (d->mGameTimer->isActive()) {
					kdDebug() << "pausing" << endl;
					d->mGameTimer->stop();
				}
			} else {
				if (!d->mGameTimer->isActive()) {
					kdDebug() << "start timer - ms="
							<< ADVANCE_INTERVAL
							<< endl;
					d->mGameTimer->start(ADVANCE_INTERVAL);
				}
			}
		}
		break;
	default:
		break;
 }
}

void Boson::slotSave(QDataStream& /*stream*/)
{ // save non-KGameProperty datas here
// stream <<
}

void Boson::slotLoad(QDataStream& /*stream*/)
{
// kdDebug() << "next id: " << d->mNextUnitId << endl;
}

void Boson::slotSendAddUnit(unsigned long int unitType, int x, int y, Player* owner)
{ // used by the editor directly
 if (!isServer()) {
	return;
 }
 if (!owner) {
	kdWarning() << k_funcinfo << "NULL owner! using first player" << endl;
	owner = (Player*)playerList()->at(0);
 }
 if (!owner) { // no player here
	kdError() << k_funcinfo << "NULL owner" << endl;
	return;
 }

 QByteArray buffer;
 QDataStream stream(buffer, IO_WriteOnly);
 stream << (Q_UINT32)owner->id();
 stream << (Q_UINT32)unitType;
 stream << (Q_INT32)x;
 stream << (Q_INT32)y;
 sendMessage(buffer, BosonMessage::AddUnit);
}

void Boson::sendAddUnits(const QString& xmlDocument, Player* owner)
{
 if (!isServer()) {
	return;
 }
 if (!owner) {
	kdWarning() << k_funcinfo << "NULL owner! using first player" << endl;
	owner = (Player*)playerList()->at(0);
 }
 if (!owner) { // no player here
	kdError() << k_funcinfo << "NULL owner" << endl;
	return;
 }
 QByteArray buffer;
 QDataStream stream(buffer, IO_WriteOnly);
 stream << (Q_UINT32)owner->id();
 stream << xmlDocument;
 sendMessage(buffer, BosonMessage::AddUnitsXML);
}

void Boson::slotReplacePlayerIO(KPlayer* player, bool* remove)
{
 *remove = false;
 if (!player) {
	kdError() << k_funcinfo << "NULL player" << endl;
	return;
 }
 if (!isAdmin()) {
	kdError() << k_funcinfo << "only ADMIN can do this" << endl; 
	return;
 }
// kdDebug() << k_funcinfo << endl;
}

bool Boson::buildProducedUnit(ProductionPlugin* factory, unsigned long int unitType, int x, int y)
{
 if (!factory) {
	kdError() << k_funcinfo << "NULL factory plugin cannot produce" << endl;
	return false;
 }
 Player* p = factory->player();
 if (!p) {
	kdError() << k_funcinfo << "NULL owner" << endl;
	return false;
 }
 if (!((BosonCanvas*)d->mCanvas)->canPlaceUnitAt(p->unitProperties(unitType), 
			QPoint(x * BO_TILE_SIZE, y * BO_TILE_SIZE), 0)) {
	kdDebug() << k_funcinfo << "Cannot create unit here" << endl;
	return false;
 }
 Unit* unit = addUnit(unitType, p, x, y);
 if (!unit) {
	kdError() << k_funcinfo << "NULL unit" << endl;
	return false;
 }
 if (unit->isFacility()) {
	p->statistics()->addProducedFacility((Facility*)unit, factory);
 } else {
	p->statistics()->addProducedMobileUnit((MobileUnit*)unit, factory);
 }

 // the current production is done.
 factory->removeProduction();
 emit signalUpdateProduction(factory->unit());
 return true;
}

Unit* Boson::addUnit(unsigned long int unitType, Player* p, int x, int y)
{
 if (x < 0 || (unsigned int)x >= d->mCanvas->mapWidth()) {
	kdError() << k_funcinfo << "Invalid x-coordinate " << x << endl;
	return 0;
 }
 if (y < 0 || (unsigned int)y >= d->mCanvas->mapHeight()) {
	kdError() << k_funcinfo << "Invalid y-coordinate " << y << endl;
	return 0;
 }
 if (!p) {
	kdError() << k_funcinfo << "NULL player" << endl;
	return 0;
 }
 Unit* unit = createUnit(unitType, (Player*)p);
 unit->setId(nextUnitId());
 emit signalAddUnit(unit, x * BO_TILE_SIZE, y * BO_TILE_SIZE);
 return unit;
}

Unit* Boson::addUnit(QDomElement& node, Player* p)
{
 unsigned long int unitType = 0;
 unsigned int x = 0;
 unsigned int y = 0;
 if (!BosonScenario::loadBasicUnit(node, unitType, x, y)) {
	kdError() << k_funcinfo << "Received invalid XML file from server!!!! (very bad)" << endl;
	return 0;
 }
 Unit* unit = createUnit(unitType, (Player*)p);
 unit->setId(nextUnitId());
 if (!BosonScenario::loadUnit(node, unit)) {
	kdWarning() << k_funcinfo << "Received broken XML file from server. It may be that network is broken now!" << endl;
	// don't return - the error should be on every client so with some luck
	// the player will never know that we had a problem here. Just a few
	// (non-critical) values were not loaded.
 }
 
 emit signalAddUnit(unit, x * BO_TILE_SIZE, y * BO_TILE_SIZE);
 return unit;
}

void Boson::slotPlayerJoinedGame(KPlayer* p)
{
 if (!p) {
	return;
 }
 KGameIO* io = p->findRttiIO(KGameIO::ComputerIO);
 if (io) {
	// note the IO is added on only *one* client!
	d->mComputerIOList.append((KGameComputerIO*)io);
 }
 slotAddChatSystemMessage(i18n("Player %1 - %2 joined").arg(p->id()).arg(p->name()));
}

void Boson::slotPlayerLeftGame(KPlayer* p)
{
 if (!p) {
	return;
 }
 KGameIO* io = p->findRttiIO(KGameIO::ComputerIO);
 if (io) {
	d->mComputerIOList.removeRef((KGameComputerIO*)io);
 }
 slotAddChatSystemMessage(i18n("Player %1 - %2 left the game").arg(p->id()).arg(p->name()));
}

void Boson::slotAdvanceComputerPlayers(unsigned int /*advanceCount*/, bool /*advanceFlag*/)
{
 // we use this to "advance" the computer player. This is a completely new concept
 // introduced to KGameIO just for boson. See KGaneComputerIO documentation for
 // more. Basically this means - let the computer do something.
 QPtrListIterator<KGameComputerIO> it(d->mComputerIOList);
// kdDebug() << "count = " << d->mComputerIOList.count() << endl;
 while (it.current()) {
	it.current()->advance();
	++it;
 }
}

QValueList<QColor> Boson::availableTeamColors() const
{
 QValueList<QColor> colors = SpeciesTheme::defaultColors();
 QPtrListIterator<KPlayer> it(*playerList());
 while (it.current()) {
	if (((Player*)it.current())->speciesTheme()) {
		kdDebug() << k_funcinfo <<  endl;
		colors.remove(((Player*)it.current())->speciesTheme()->teamColor());
	}
	++it;
 }
 return colors;
}

void Boson::slotReceiveAdvance()
{
 bool flag = advanceFlag();
 // we need to toggle the flag *now*, in case one of the Unit::advance*()
 // methods changes the advance function. this change must not appear to the
 // currently used function, but to the other one.
 toggleAdvanceFlag();
 emit signalAdvance(d->mAdvanceCount, flag);

 d->mAdvanceCount = d->mAdvanceCount + 1; // this advance count is important for Unit e.g. - but not used in this function.
 if (d->mAdvanceCount > MAXIMAL_ADVANCE_COUNT) {
	d->mAdvanceCount = 0;
 }

 // we also have "mAdvanceDividerCount". the mAdvanceCount is important in Unit,
 // mAdvanceDividerCount is limited to boson only. some explanations:
 // Only a single advance message is sent over network every ADVANCE_INTERVAL
 // ms, independant from the game speed.
 // This single advance message results in a certain number of advance calls
 // (btw: in codes, comments, logs and so on I always make a different between
 // "advance message" and "advance calls" as explained here). mAdvanceDivider is
 // this number of advance calls to-be-generated and is reset to the gameSpeed()
 // when the message is received.
 // The mAdvanceDividerCount is reset to 0 once the advance message is received.
 //
 // The code below tries to make one advance call every
 // ADVANCE_INTERVAL/mAdvanceDivider ms. This means in the ideal case all
 // advance calls from this and from the next advance message would be in the
 // same interval.
 //
 // Please remember that there are several additional tasks that need to be done
 // - e.g. unit moving, OpenGL rendering, ... so 
 if (d->mAdvanceDividerCount + 1 == d->mAdvanceDivider)  {
#ifndef NO_ADVANCE_DEBUG
	kdDebug() << k_funcinfo << "delayed messages: "
			<< d->mDelayedMessages.count() << endl;
#endif
	unlock();
 } else if (d->mAdvanceDividerCount + 1 < d->mAdvanceDivider) {
	int next;
	if (d->mAdvanceMessageWaiting == 0) {
		int t = ADVANCE_INTERVAL * d->mAdvanceDividerCount / d->mAdvanceDivider;// time that should have been elapsed
		int diff = QMAX(5, d->mAdvanceReceived.elapsed() - t + 5); // we are adding 5ms "safety" diff
		next = QMAX(0, ADVANCE_INTERVAL / d->mAdvanceDivider - diff);
	} else {
		next = 0;
	}
	if (d->mDelayedMessages.count() > 20) {
		kdWarning() << k_funcinfo << "more than 20 messages delayed!!" << endl;
		next = 0;
	}
//	kdDebug() << "next: " << next << endl;
	QTimer::singleShot(next,  this, SLOT(slotReceiveAdvance()));
	d->mAdvanceDividerCount++;
 } else {
	kdError() << k_funcinfo << "count > divider --> This must never happen!!" << endl;
 }
}

void Boson::networkTransmission(QDataStream& stream, int msgid, Q_UINT32 r, Q_UINT32 s, Q_UINT32 clientId)
{
 if (d->mIsLocked || d->mDelayedWaiting) {
	kdDebug() << k_funcinfo << "delaying " << msgid<< endl;
	BoMessage* m = new BoMessage;
	m->byteArray = ((QBuffer*)stream.device())->readAll();
	m->msgid = msgid;
	m->receiver = r;
	m->sender = s;
	m->clientId = clientId;
	d->mDelayedMessages.enqueue(m);
	d->mDelayedWaiting = true;
	switch (msgid - KGameMessage::IdUser) {
		case BosonMessage::AdvanceN:
			d->mAdvanceMessageWaiting++;
			kdDebug() << k_funcinfo << "advance message got delayed" << endl;
			break;
		default:
			break;
	}
	return;
 }
 KGame::networkTransmission(stream, msgid, r, s, clientId);
}

void Boson::lock()
{
#ifndef NO_ADVANCE_DEBUG
 kdDebug() << k_funcinfo << endl;
#endif
 d->mIsLocked = true;
}

void Boson::unlock()
{
#ifndef NO_ADVANCE_DEBUG
 kdDebug() << k_funcinfo << endl;
#endif
 d->mIsLocked = false;
 while (!d->mDelayedMessages.isEmpty() && !d->mIsLocked) {
	slotProcessDelayed();
 }
}

void Boson::slotProcessDelayed() // TODO: rename: processDelayed()
{
 BoMessage* m = d->mDelayedMessages.dequeue();
 if (!m) {
	kdWarning() << k_funcinfo << "no message here" << endl;
	return;
 }
 QDataStream s(m->byteArray, IO_ReadOnly);
 d->mDelayedWaiting = false;
 switch (m->msgid - KGameMessage::IdUser) {
	case BosonMessage::AdvanceN:
		d->mAdvanceMessageWaiting--;
		break;
	default:
		break;
 }
 networkTransmission(s, m->msgid, m->receiver, m->sender, m->clientId);
 d->mDelayedWaiting = !d->mDelayedMessages.isEmpty();
 delete m;
}

bool Boson::save(QDataStream& stream, bool saveplayers)
{
#if HAVE_KGAME_SAVEGAME
 return KGame::save(stream, saveplayers);
#else
// KDE 3.0 didn't have KGame::savegame() - we provide our own savegame()
// version, but the KGame code is in KGame::save(). we need to call that from
// Boson::save(), instead of savegame()
 return savegame(stream, false, saveplayers);
#endif
}

bool Boson::savegame(QDataStream& stream, bool network, bool saveplayers)
{
 kdDebug() << k_funcinfo << endl;
 // KGame::load() doesn't emit signalLoadPrePlayers in KDE 3.0.x, so we have to
 //  rewrite some code to be able to load map before players (because players
 //  need map)

 // First write some magic data
 // For filetype detection
 stream << (Q_UINT8)128;
 stream << (Q_UINT8)'B' << (Q_UINT8)'S' << (Q_UINT8)'G';  // BSG = Boson SaveGame
 // Magic cookie
 stream << (Q_UINT32)cookie();
 // Version information (for future format changes and backwards compatibility)
 stream << (Q_UINT32)BOSON_SAVEGAME_FORMAT_VERSION;

 if (gameStatus() != KGame::Init) {
	kdDebug() << k_funcinfo << "Saveing started game" << endl;
	stream << (Q_INT8)true;
	// Save map
	d->mPlayField->saveMap(stream);

	// Save local player (only id)
	stream << d->mPlayer->id();
 } else {
	stream << (Q_INT8)false;
	kdDebug() << k_funcinfo << "Saveing not-yet started game" << endl;
 }

 // Save KGame stuff
#if !HAVE_KGAME_SAVEGAME
 kdWarning() << k_funcinfo << "Saving without KGame::savegame() is untested! (KDE 3.1 has KGame::savegame())" << endl;
 if (!KGame::save(stream, saveplayers)) {
#else
 if (!KGame::savegame(stream, network, saveplayers)) {
#endif
	kdError() << k_funcinfo << "Can't save KGame!" << endl;
	return false;
 }

 // units. they must be loaded *after* the players, so it also needs to be saved
 // later
 QPtrListIterator<KPlayer> it(*playerList());
 for (; it.current(); ++it) {
	if (!((Player*)it.current())->saveUnits(stream)) {
		kdError() << k_funcinfo << "Error when saving units" << endl;
	}
 }

 kdDebug() << k_funcinfo << " done" << endl;
 return true;
}

bool Boson::load(QDataStream& stream, bool reset)
{
// we can't use this directly cause of a KGame bug :-(
 return loadgame(stream, false, reset);
}

bool Boson::loadgame(QDataStream& stream, bool network, bool reset)
{
 // network is false for normal game-loading
 kdDebug() << k_funcinfo << endl;
 d->mLoadingStatus = LoadingInProgress;

 // Load magic data
 Q_UINT8 a, b1, b2, b3;
 Q_INT32 c, v;
 stream >> a >> b1 >> b2 >> b3;
 if ((a != 128) || (b1 != 'B' || b2 != 'S' || b3 != 'G')) {
	// Error - not Boson SaveGame
	kdError() << k_funcinfo << "This file is not Boson SaveGame" << endl;
	d->mLoadingStatus = InvalidFileFormat;
	return false;
 }
 stream >> c;
 if (c != cookie()) {
	// Error - wrong cookie
	kdError() << k_funcinfo << "Invalid cookie in header" << endl;
	d->mLoadingStatus = InvalidCookie;
	return false;
 }
 stream >> v;
 if (v != BOSON_SAVEGAME_FORMAT_VERSION) {
	// Error - older version
	// TODO: It may be possible to load this version
	kdError() << k_funcinfo << "Unsupported format version" << endl;
	d->mLoadingStatus = InvalidVersion;
	return false;
 }

 Q_UINT32 localId = 0;
 Q_INT8 started; // network players usually connect before starting a game
 stream >> started;
 QByteArray mapBuffer;
 if (started) {
	kdDebug() << k_funcinfo << "Loading a previously saved game" << endl;
	// Load map
	BosonPlayField* f = new BosonPlayField;
	if (!f->loadMap(stream)) {
		kdError() << k_funcinfo << "Could not load map" << endl;
		return false;
	}
	QDataStream mapstream(mapBuffer, IO_WriteOnly);
	f->saveMap(mapstream);
	delete f;

	// Load local player's id
	stream >> localId;
 }

 // Load KGame stuff
 kdDebug() << "calling KGame::loadgame" << endl;
 if (!KGame::loadgame(stream, network, reset)) {
	// KGame loading error
	kdError() << k_funcinfo << "KGame loading error" << endl;
	d->mLoadingStatus = KGameError;
	return false;
 }

 // KGame::loadgame() also loads the gamestatus. some functions depend on KGame
 // to be in Init status as long as it is still loading, so we set it manually
 // here. we first need to set PolicyLocal, to make the change take immediate
 // effect
 KGame::GamePolicy oldPolicy = boGame->policy();
 boGame->setPolicy(KGame::PolicyLocal);
 boGame->setGameStatus(KGame::Init);
 boGame->setPolicy(oldPolicy);

 if (started) {
	// AB: needs to be emitted after KGame::loadgame() which adds the
	// players
	emit signalInitMap(mapBuffer);
 }

 // units. they must be loaded *after* the players
 QPtrListIterator<KPlayer> it(*playerList());
 for (; it.current(); ++it) {
	if (!((Player*)it.current())->loadUnits(stream)) {
		kdError() << k_funcinfo << "Error when loading units" << endl;
	}
 }

 kdDebug() << k_funcinfo << "kgame loading successful" << endl;

 if (started) { // AB (02/09/04): by any reason this was "!started" before - can't work, as localId is not initialized then. is this fix correct?
	// Set local player
	d->mPlayer = (Player*)findPlayer(localId);
 }

 // Set game status to Init. It will be set to Run in TopWidget
 setGameStatus(Init);

 d->mLoadingStatus = LoadingCompleted;
 kdDebug() << k_funcinfo << " done" << endl;
 return true;
}

Boson::LoadingStatus Boson::loadingStatus()
{
 return d->mLoadingStatus;
}

void Boson::toggleAdvanceFlag()
{
 d->mAdvanceFlag = !d->mAdvanceFlag;
}

bool Boson::advanceFlag() const
{
 return d->mAdvanceFlag;
}

void Boson::slotUpdateProductionOptions()
{
 emit signalUpdateProductionOptions();
}

void Boson::slotAddChatSystemMessage(const QString& fromName, const QString& text)
{
 // just forward it to BosonWidgetBase
 emit signalAddChatSystemMessage(fromName, text);
}

void Boson::slotAddChatSystemMessage(const QString& text)
{
 slotAddChatSystemMessage(i18n("Boson"), text);
}

