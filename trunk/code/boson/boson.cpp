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
#include "bodebug.h"

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

	QString debug(KGame* game)
	{
		if (!game) {
			return QString::null;
		}
		QString m = QString("msgid=%1").arg(msgid);
		QString r = QString("receiver=%3").arg(receiver);
		QString s = QString("sender=%2").arg(sender);
		if (KGameMessage::isPlayer(receiver)) {
			KPlayer* p = game->findPlayer(receiver);
			if (!p) {
				r += QString("(player cant be found)");
			} else {
				r += QString("(player %1)").arg(p->name());
			}
		} else if (KGameMessage::isGame(receiver)) {
			if (receiver == 0) {
				r += "(broadcast games)";
			} else if (receiver == game->gameId()) {
				r += "(local game)";
			} else {
				r += "(remote game)";
			}
		}

		if (KGameMessage::isPlayer(sender)) {
			KPlayer* p = game->findPlayer(receiver);
			if (!p) {
				r += QString("(player cant be found)");
			} else {
				r += QString("(player %1)").arg(p->name());
			}
		} else if (KGameMessage::isGame(sender)) {
			if (sender == 0) {
				r += "(broadcast games)";
			} else if (sender == game->gameId()) {
				s += "(local game)";
			} else {
				s += "(remote game)";
			}
		}

		QString mname = KGameMessage::messageId2Text(msgid);
		if (!mname.isEmpty()) {
			m += QString("(%1)").arg(mname);
		}

		QString d;
		d = m + " " + r + " " + s;

		d += debugMore(game);
		return d;
	}

	QString debugMore(KGame* game)
	{
		QString m;
		QDataStream s(byteArray, IO_ReadOnly);
		if (msgid == KGameMessage::IdGameProperty) {
			int propId;
			KGameMessage::extractPropertyHeader(s, propId);
			if (propId == KGamePropertyBase::IdCommand) {
				m = " is a property command";
				// we could use
				// KGameMessage::extractPropertyCommand() here
				// now
			} else {
				KGamePropertyBase* p;
				p = game->dataHandler()->dict().find(propId);
				if (!p) {
					m = QString(" property %1 can't be found").arg(propId);
				} else {
					m = QString(" property: %1(%2)").arg(propId).arg(game->dataHandler()->propertyName(propId));
				}
			}
		}
		return m;
	}
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
 boDebug() << k_funcinfo << endl;
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
 boDebug() << k_funcinfo << endl;
// reset everything
 d->mGameTimer->stop();
 setGameStatus(KGame::End);
 d->mNextUnitId = 0;

 // remove all players from game
 removeAllPlayers();
 boDebug() << k_funcinfo << " done" <<  endl;
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
	boWarning() << k_funcinfo << "Player must not send input anymore!!" << endl;
	return true;
 }
 if (!gameMode()) {
	// editor mode sends an additional entry safety id, just in case we
	// might have constructed a wrong display or so
	Q_UINT32 editor;
	stream >> editor;
	if (editor != BosonMessage::MoveEditor) {
		boError() << k_funcinfo << "Not an editor message, elthough we're in editor mode!" << endl;
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
			it.current()->stopAttacking();  // call stopAttacking() because it also sets unit's work to WorkNone... and it doesn't hurt
			++it;
		}
		break;
	}
	case BosonMessage::MoveMine:
	{
		boDebug() << "MoveMine" << endl;
		Q_ULONG unitId;
		QPoint pos;
		stream >> unitId;
		stream >> pos;
		Unit* u = findUnit(unitId, player);
		if (!u) {
			boError() << k_lineinfo << "cannot find unit " << unitId << " for player " << player << endl;
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
			boDebug() << k_funcinfo << "unit " << unitId << "only owner can move units!" << endl;
			break;
		}
		if (u->isDestroyed()) {
			boDebug() << "cannot mine with destroyed units" << endl;
			break;
		}
		h->mineAt(pos);
		break;
	}
	case BosonMessage::MoveRefine:
	{
		boDebug() << "MoveRefine" << endl;
		Q_UINT32 refineryOwnerId;
		Q_ULONG refineryId;
		Q_UINT32 unitCount;
		stream >> refineryOwnerId;
		stream >> refineryId;
		stream >> unitCount;
		Player* refineryOwner = (Player*)findPlayer(refineryOwnerId);
		if (!refineryOwner) {
			boError() << k_lineinfo << "Cannot find player " << refineryOwnerId << endl;
			break;
		}
		if (player->isEnemy(refineryOwner)) {
			boError() << k_lineinfo << "Cannot go to enemy refinery" << endl;
			break;
		}
		Unit* refinery = findUnit(refineryId, refineryOwner);
		if (!refinery) {
			boError() << k_lineinfo << "cannot find refinery " << refineryId << " for player " << refineryOwnerId << endl;
			break;
		}
#warning TODO
//		if (!refinery->plugin(UnitPlugin::Refinery)) {
		if (!refinery->isFacility()) { // FIXME do not depend on facility!
			boWarning() << k_lineinfo << "refinery must be a facility" << endl;
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
		RepairPlugin* repair = repairYard->repairPlugin();
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
			// Produce technology
			const TechnologyProperties* prop = p->speciesTheme()->technology(id);
			if (!prop) {
				boError() << k_lineinfo << "NULL technology properties (EVIL BUG)" << endl;
				break;
			}
			mineralCost = prop->mineralCost();
			oilCost = prop->oilCost();
		} else {
			boError() << k_funcinfo << "Invalid productionType: " << productionType << endl;
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
		boDebug() << "MoveProduceStop" << endl;
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
			const TechnologyProperties* prop = p->speciesTheme()->technology(id);
			if (!prop) {
				boError() << k_lineinfo << "NULL technology properties (EVIL BUG)" << endl;
				break;
			}
			mineralCost = prop->mineralCost();
			oilCost = prop->oilCost();
		} else {
			boError() << k_funcinfo << "Invalid productionType: " << productionType << endl;
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
		boDebug() << "MoveBuild" << endl;
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
			boError() << k_funcinfo << "Invalid productionType: " << productionType << endl;
			break;
		}

		Player* p = (Player*)findPlayer(owner);
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
			boError() << k_lineinfo << "Cannot find player " << owner << endl;
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
		boWarning() << k_funcinfo << "unexpected playerInput " << msgid << endl;
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
			boError() << k_lineinfo << "Cannot find player " << owner << endl;
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
			boError() << k_lineinfo << "Cannot find player " << owner << endl;
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
		boDebug(300) << "Advance - speed (calls per " << ADVANCE_INTERVAL 
				<< "ms)=" << gameSpeed() << " elapsed: " 
				<< d->mAdvanceReceived.elapsed() << endl;
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
			boError() << "Received IdGameIstarted but it isn't" << endl;
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
			boError() << k_lineinfo << "Cannot find player " << id << endl;
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
			boError() << k_lineinfo << "Cannot find player " << id << endl;
			return;
		}
		if (!p->speciesTheme()) {
			boError() << k_lineinfo << "NULL speciesTheme for " << id << endl;
			return;
		}
		if (p->speciesTheme()->setTeamColor(QColor(color))) {
			emit signalTeamColorChanged(p);
		} else {
			boWarning() << k_lineinfo << "could not change color for " << id << endl;
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
		boWarning() << k_funcinfo << "unhandled msgid " << msgid << endl;
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
	boWarning() << "is not server - cannot start the game!" << endl;
 }
}

void Boson::slotSendAdvance()
{
 sendMessage(10, BosonMessage::AdvanceN);
}

Unit* Boson::createUnit(unsigned long int unitType, Player* owner)
{
 if (!owner) {
	boError() << k_funcinfo << "NULL owner" << endl;
	return 0;
 }
 SpeciesTheme* theme = owner->speciesTheme();
 if (!theme) {
	boError() << k_funcinfo << "No theme for this player" << endl;
	return 0; // BAAAAD - will crash
 }
 const UnitProperties* prop = theme->unitProperties(unitType);
 if (!prop) {
	boError() << "Unknown unitType " << unitType << endl;
	return 0;
 }

 Unit* unit = 0;
 if (prop->isMobile()) {
	unit = new MobileUnit(prop, owner, d->mCanvas);
 } else if (prop->isFacility()) {
	unit = new Facility(prop, owner, d->mCanvas);
 } else { // should be impossible
	boError() << k_funcinfo << "invalid unit type " << unitType << endl;
	return 0;
 }
 owner->addUnit(unit); // can also be in Unit c'tor - is this clean?
 theme->loadNewUnit(unit);

 return unit;
}

Unit* Boson::loadUnit(unsigned long int unitType, Player* owner)
{
 if (!owner) {
	boError() << k_funcinfo << "NULL owner" << endl;
	return 0;
 }
 SpeciesTheme* theme = owner->speciesTheme();
 if (!theme) {
	boError() << k_funcinfo << "No theme for this player" << endl;
	return 0; // BAAAAD - will crash
 }
 const UnitProperties* prop = theme->unitProperties(unitType);
 if (!prop) {
	boError() << "Unknown unitType " << unitType << endl;
	return 0;
 }

 Unit* unit = 0;
 if (prop->isMobile()) {
	unit = new MobileUnit(prop, owner, d->mCanvas);
 } else if (prop->isFacility()) {
	unit = new Facility(prop, owner, d->mCanvas);
 } else { // should be impossible
	boError() << k_funcinfo << "invalid unit type " << unitType << endl;
	return 0;
 }

 return unit;
}

unsigned long int Boson::nextUnitId()
{
 d->mNextUnitId = d->mNextUnitId + 1;
//boDebug() << "next id: " << d->mNextUnitId << endl;
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
 boDebug() << k_funcinfo << "rtti=" << rtti << ",io=" << io
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
 boDebug() << k_funcinfo << " speed = " << speed << endl;
 if (d->mGameSpeed == speed) {
	return; // do not restart timer
 }
 if (speed < 0) {
	boError() << "Invalid speed value " << speed << endl;
	return;
 }
 if ((speed < MIN_GAME_SPEED || speed > MAX_GAME_SPEED) && speed != 0) {
	boWarning() << "unexpected speed " << speed << " - pausing" << endl;
	d->mGameSpeed = 0;
	return;
 }
 boDebug() << k_funcinfo << "Setting speed to " << speed << endl;
 d->mGameSpeed = speed;
}

void Boson::slotPropertyChanged(KGamePropertyBase* p)
{
 switch (p->id()) {
	case IdGameSpeed:
		boDebug() << k_funcinfo << "speed has changed, new speed: " << gameSpeed() << endl;
		if (isServer()) {
			if (d->mGameSpeed == 0) {
				if (d->mGameTimer->isActive()) {
					boDebug() << "pausing" << endl;
					d->mGameTimer->stop();
				}
			} else {
				if (!d->mGameTimer->isActive()) {
					boDebug() << "start timer - ms="
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
// boDebug() << "next id: " << d->mNextUnitId << endl;
}

void Boson::slotSendAddUnit(unsigned long int unitType, int x, int y, Player* owner)
{ // used by the editor directly
 if (!isServer()) {
	return;
 }
 if (!owner) {
	boWarning() << k_funcinfo << "NULL owner! using first player" << endl;
	owner = (Player*)playerList()->at(0);
 }
 if (!owner) { // no player here
	boError() << k_funcinfo << "NULL owner" << endl;
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
	boWarning() << k_funcinfo << "NULL owner! using first player" << endl;
	owner = (Player*)playerList()->at(0);
 }
 if (!owner) { // no player here
	boError() << k_funcinfo << "NULL owner" << endl;
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
	boError() << k_funcinfo << "NULL player" << endl;
	return;
 }
 if (!isAdmin()) {
	boError() << k_funcinfo << "only ADMIN can do this" << endl; 
	return;
 }
// boDebug() << k_funcinfo << endl;
}

bool Boson::buildProducedUnit(ProductionPlugin* factory, unsigned long int unitType, int x, int y)
{
 if (!factory) {
	boError() << k_funcinfo << "NULL factory plugin cannot produce" << endl;
	return false;
 }
 Player* p = factory->player();
 if (!p) {
	boError() << k_funcinfo << "NULL owner" << endl;
	return false;
 }
 if (!((BosonCanvas*)d->mCanvas)->canPlaceUnitAt(p->unitProperties(unitType), 
			QPoint(x * BO_TILE_SIZE, y * BO_TILE_SIZE), 0)) {
	boDebug() << k_funcinfo << "Cannot create unit here" << endl;
	return false;
 }
 Unit* unit = addUnit(unitType, p, x, y);
 if (!unit) {
	boError() << k_funcinfo << "NULL unit" << endl;
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
	boError() << k_funcinfo << "Invalid x-coordinate " << x << endl;
	return 0;
 }
 if (y < 0 || (unsigned int)y >= d->mCanvas->mapHeight()) {
	boError() << k_funcinfo << "Invalid y-coordinate " << y << endl;
	return 0;
 }
 if (!p) {
	boError() << k_funcinfo << "NULL player" << endl;
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
	boError() << k_funcinfo << "Received invalid XML file from server!!!! (very bad)" << endl;
	return 0;
 }
 Unit* unit = createUnit(unitType, (Player*)p);
 unit->setId(nextUnitId());
 if (!BosonScenario::loadUnit(node, unit)) {
	boWarning() << k_funcinfo << "Received broken XML file from server. It may be that network is broken now!" << endl;
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
// boDebug() << "count = " << d->mComputerIOList.count() << endl;
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
		boDebug() << k_funcinfo <<  endl;
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
	boDebug(300) << k_funcinfo << "delayed messages: "
			<< d->mDelayedMessages.count() << endl;
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
		boWarning() << k_funcinfo << "more than 20 messages delayed!!" << endl;
		next = 0;
	}
//	boDebug() << "next: " << next << endl;
	QTimer::singleShot(next,  this, SLOT(slotReceiveAdvance()));
	d->mAdvanceDividerCount++;
 } else {
	boError() << k_funcinfo << "count > divider --> This must never happen!!" << endl;
 }
}

void Boson::networkTransmission(QDataStream& stream, int msgid, Q_UINT32 r, Q_UINT32 s, Q_UINT32 clientId)
{
 if (d->mIsLocked || d->mDelayedWaiting) {
	BoMessage* m = new BoMessage;
	m->byteArray = ((QBuffer*)stream.device())->readAll();
	m->msgid = msgid;
	m->receiver = r;
	m->sender = s;
	m->clientId = clientId;
	boDebug() << k_funcinfo << "delayed " << m->debug(this) << endl;
	d->mDelayedMessages.enqueue(m);
	d->mDelayedWaiting = true;
	switch (msgid - KGameMessage::IdUser) {
		case BosonMessage::AdvanceN:
			d->mAdvanceMessageWaiting++;
			boDebug() << k_funcinfo << "advance message got delayed" << endl;
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
 boDebug(300) << k_funcinfo << endl;
 d->mIsLocked = true;
}

void Boson::unlock()
{
 boDebug(300) << k_funcinfo << endl;
 d->mIsLocked = false;
 while (!d->mDelayedMessages.isEmpty() && !d->mIsLocked) {
	slotProcessDelayed();
 }
}

void Boson::slotProcessDelayed() // TODO: rename: processDelayed()
{
 BoMessage* m = d->mDelayedMessages.dequeue();
 if (!m) {
	boWarning() << k_funcinfo << "no message here" << endl;
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
 boDebug() << k_funcinfo << endl;
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
	boDebug() << k_funcinfo << "Saveing started game" << endl;
	stream << (Q_INT8)true;
	// Save map
	d->mPlayField->saveMap(stream);

	// Save local player (only id)
	stream << d->mPlayer->id();
 } else {
	stream << (Q_INT8)false;
	boDebug() << k_funcinfo << "Saveing not-yet started game" << endl;
 }

 // Save KGame stuff
#if !HAVE_KGAME_SAVEGAME
 boWarning() << k_funcinfo << "Saving without KGame::savegame() is untested! (KDE 3.1 has KGame::savegame())" << endl;
 if (!KGame::save(stream, saveplayers)) {
#else
 if (!KGame::savegame(stream, network, saveplayers)) {
#endif
	boError() << k_funcinfo << "Can't save KGame!" << endl;
	return false;
 }

 // units. they must be loaded *after* the players, so it also needs to be saved
 // later
 QPtrListIterator<KPlayer> it(*playerList());
 for (; it.current(); ++it) {
	if (!((Player*)it.current())->saveUnits(stream)) {
		boError() << k_funcinfo << "Error when saving units" << endl;
	}
 }

 boDebug() << k_funcinfo << " done" << endl;
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
 boDebug() << k_funcinfo << endl;

 d->mLoadingStatus = LoadingInProgress;

 // Load magic data
 Q_UINT8 a, b1, b2, b3;
 Q_INT32 c, v;
 stream >> a >> b1 >> b2 >> b3;
 if ((a != 128) || (b1 != 'B' || b2 != 'S' || b3 != 'G')) {
	// Error - not Boson SaveGame
	boError() << k_funcinfo << "This file is not Boson SaveGame" << endl;
	d->mLoadingStatus = InvalidFileFormat;
	return false;
 }
 stream >> c;
 if (c != cookie()) {
	// Error - wrong cookie
	boError() << k_funcinfo << "Invalid cookie in header" << endl;
	d->mLoadingStatus = InvalidCookie;
	return false;
 }
 stream >> v;
 if (v != BOSON_SAVEGAME_FORMAT_VERSION) {
	// Error - older version
	// TODO: It may be possible to load this version
	boError() << k_funcinfo << "Unsupported format version" << endl;
	d->mLoadingStatus = InvalidVersion;
	return false;
 }

 Q_UINT32 localId = 0;
 Q_INT8 started; // network players usually connect before starting a game
 stream >> started;
 QByteArray mapBuffer;
 if (started) {
	boDebug() << k_funcinfo << "Loading a previously saved game" << endl;
	// Load map
	BosonPlayField* f = new BosonPlayField;
	if (!f->loadMap(stream)) {
		boError() << k_funcinfo << "Could not load map" << endl;
		return false;
	}
	QDataStream mapstream(mapBuffer, IO_WriteOnly);
	f->saveMap(mapstream);
	delete f;

	// Load local player's id
	stream >> localId;
 }

 // Load KGame stuff
 boDebug() << "calling KGame::loadgame" << endl;
 if (!KGame::loadgame(stream, network, reset)) {
	// KGame loading error
	boError() << k_funcinfo << "KGame loading error" << endl;
	d->mLoadingStatus = KGameError;
	return false;
 }

 // KGame::loadgame() also loads the gamestatus. some functions depend on KGame
 // to be in Init status as long as it is still loading, so we set it manually
 // here. we can't do this using KGame::setStatus(), as the policy is clean, but
 // we need Init state *now*. Changing policy would also change our property
 // policies (we use both clean and local policies, so this would not work).
 {
	// set gameStatus to Init. Will be set to Run later
	QByteArray b;
	QDataStream s(b, IO_WriteOnly);
	KGameMessage::createPropertyHeader(s, KGamePropertyBase::IdGameStatus);
	s << (int)KGame::Init;
	QDataStream readStream(b, IO_ReadOnly);
	dataHandler()->processMessage(readStream, dataHandler()->id(), false);
 }

 if (started) {
	// AB: needs to be emitted after KGame::loadgame() which adds the
	// players
	emit signalInitMap(mapBuffer);
 }

 // units. they must be loaded *after* the players
 QPtrListIterator<KPlayer> it(*playerList());
 for (; it.current(); ++it) {
	// Order of calls below is very important!!! Don't change this unless you're sure you know what you're doing!!!
	((Player*)it.current())->speciesTheme()->loadParticleSystems();
	((Player*)it.current())->speciesTheme()->readUnitConfigs();
//	loadUnitDatas(((Player*)it.current()), progress);
	((Player*)it.current())->speciesTheme()->loadTechnologies();
	if (!((Player*)it.current())->loadUnits(stream)) {
		boError() << k_funcinfo << "Error when loading units" << endl;
		return false;
	}
 }

 boDebug() << k_funcinfo << "kgame loading successful" << endl;

 if (started) { // AB (02/09/04): by any reason this was "!started" before - can't work, as localId is not initialized then. is this fix correct?
	// Set local player
	d->mPlayer = (Player*)findPlayer(localId);
 }

 d->mLoadingStatus = LoadingCompleted;
 boDebug() << k_funcinfo << " done" << endl;
 return true;
}

Boson::LoadingStatus Boson::loadingStatus() const
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

