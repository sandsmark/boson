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

#include "bosonmessage.h"
#include "player.h"
#include "defines.h"
#include "unit.h"
#include "speciestheme.h"
#include "unitproperties.h"
#include "bosoncanvas.h"
#include "bosonconfig.h"
#include "bosonscenario.h"
#include "bosonstatistics.h"

#include <kdebug.h>
#include <klocale.h>
#include <kgame/kgameio.h>

#include <qtimer.h>
#include <qptrlist.h>
#include <qdom.h>

#include "boson.moc"

class Boson::BosonPrivate
{
public:
	BosonPrivate()
	{
		mGameTimer = 0;
		mCanvas = 0;

	}

	QTimer* mGameTimer;
	QCanvas* mCanvas; // this pointer is anti-OO IMHO
	QPtrList<KGameComputerIO> mComputerIOList;
	
	KGamePropertyInt mGameSpeed;

	KGameProperty<unsigned long int> mNextUnitId;
	KGameProperty<unsigned int> mAdvanceCount;
};

Boson::Boson(QObject* parent) : KGame(BOSON_COOKIE, parent)
{
 setPolicy(PolicyClean);
 d = new BosonPrivate;

 d->mGameTimer = new QTimer(this);

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
 connect(this, SIGNAL(signalAdvance(unsigned int)),
		this, SLOT(slotAdvanceComputerPlayers(unsigned int)));
 d->mGameSpeed.registerData(IdGameSpeed, dataHandler(),
		KGamePropertyBase::PolicyLocal, "GameSpeed"); // PolicyClean?
 d->mNextUnitId.registerData(IdNextUnitId, dataHandler(),
		KGamePropertyBase::PolicyLocal, "NextUnitId");
 d->mAdvanceCount.registerData(IdAdvanceCount, dataHandler(),
		KGamePropertyBase::PolicyLocal, "AdvanceCount");
 d->mNextUnitId.setLocal(0);
 d->mAdvanceCount.setLocal(0);
 d->mGameSpeed.setLocal(0);
 d->mAdvanceCount.setEmittingSignal(false); // wo don't need it and it would be bad for performance.
}

Boson::~Boson()
{
 delete d->mGameTimer;
 delete d;
}

void Boson::setCanvas(QCanvas* c)
{
 d->mCanvas = c;
}

void Boson::quitGame()
{
// kdDebug() << k_funcinfo << endl;
// reset everything
 d->mGameTimer->stop();
 setGameStatus(KGame::End);
 d->mNextUnitId = 0;

 // remove all players from game
 QPtrList<KPlayer> list = *playerList();
 for (unsigned int i = 0; i < list.count(); i++) {
	removePlayer(list.at(i)); // might not be necessary - sends remove over network
	systemRemovePlayer(list.at(i), true); // remove immediately, even before network removing is received.
 }
// kdDebug() << k_funcinfo << " done" <<  endl;
}

bool Boson::playerInput(QDataStream& stream, KPlayer* p)
{
 Player* player = (Player*)p;
 if (player->isOutOfGame()) {
	kdWarning() << k_funcinfo << "Player must not send input anymore!!" << endl;
	return true;
 }
 Q_UINT32 msgid;
 stream >> msgid;
 switch (msgid) {
	case BosonMessage::MoveMove:
	{
		QPoint pos;
		Q_UINT32 unitCount;
		Q_UINT32 mode;
		stream >> mode;
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
				kdDebug() << "unit " << unitId << ": only owner can move units!" << endl;
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
		if(unitsToMove.count() == 1) {
			unitsToMove.first()->moveTo(pos);
		} else {
			if(mode == GroupMoveFollow) {
				Unit* unit = unitsToMove.take(0);
				unit->moveTo(pos);
				unit->setGroupLeader(true);
				emit signalNewGroup(unit, unitsToMove);
			} else if (mode == GroupMoveOld) {
				it.toFirst();
				while (it.current()) {
					it.current()->moveTo(pos);
					++it;
				}
			} else if (mode == GroupMoveNew) {
				kdDebug() << k_funcinfo << "mode == GroupMoveNew" << endl;
				Unit* leader = unitsToMove.take(0);
				leader->moveTo(pos);
				Unit* unit;
				QPoint pos2;

				it.toFirst();
				while (it.current()) {
					unit = it.current();
					unit->moveTo(pos);
					pos2.setX(pos.x() + (unit->x() - leader->x()));
					pos2.setY(pos.y() + (unit->y() - leader->y()));
					unit->moveTo(pos2);
					++it;
				}
			} else {
				kdError() << k_funcinfo << "wrong GroupMoveMode " << mode << endl;
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
				unit->setTarget(attackedUnit);
			}
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
		MobileUnit* u = (MobileUnit*)findUnit(unitId, player);
		if (!((Unit*)u)->isMobile()) {
			kdError() << k_lineinfo << "only mobile units can mine" << endl;
			break;
		}
		if (u->owner() != player) {
			kdDebug() << "unit " << unitId << "only owner can move units!" << endl;
			break;
		}
		if (u->isDestroyed()) {
			kdDebug() << "cannot mine with destroyed units" << endl;
			break;
		}
		u->mineAt(pos);
		break;
	}
	case BosonMessage::MoveProduce:
	{
		Q_UINT32 owner;
		Q_ULONG factoryId;
		Q_INT32 unitType;
		stream >> owner;
		stream >> factoryId;
		stream >> unitType;
		
		Player* p = (Player*)findPlayer(owner);
		if (!p) {
			kdError() << k_lineinfo << "Cannot find player " << owner << endl;
			break;
		}
		Facility* factory = (Facility*)findUnit(factoryId, p);
		if (!factory) {
			kdError() << "Cannot find unit " << factoryId << endl;
			break;
		}
		if (!((Unit*)factory)->isFacility()) {
			kdError() << k_lineinfo << factoryId << " is not a facility" << endl;
			break;
		}
		if (unitType < 0) {
			kdError() << k_lineinfo << "Invalid unitType " << unitType << endl;
			break;
		}
		const UnitProperties* prop = p->unitProperties(unitType);
		if (!prop) {
			kdError() << k_lineinfo << "NULL properties (EVIL BUG)" << endl;
			break;
		}
		if (!factory->isConstructionComplete()) {
			kdWarning() << "Factory " << factoryId << " not yet constructed" << endl;
			break;
		}
		if (factory->work() != Unit::WorkProduce) {
			if (factory->currentProduction() == unitType) {
				// production was stopped - continue it now
				factory->setWork(Unit::WorkProduce);
				emit signalUpdateProduction(factory);
				break;
			}
		}
		if (p->minerals() < prop->mineralCost()) {
			emit signalNotEnoughMinerals(p);
			break;
		}
		if (p->oil() < prop->oilCost()) {
			emit signalNotEnoughOil(p);
			break;
		}
		p->setMinerals(p->minerals() - prop->mineralCost());
		p->setOil(p->oil() - prop->oilCost());
		factory->addProduction(unitType);
		emit signalUpdateProduction(factory);
		break;
	}
	case BosonMessage::MoveProduceStop:
	{
		kdDebug() << "MoveProduceStop" << endl;
		Q_UINT32 owner;
		Q_ULONG factoryId;
		Q_INT32 unitType;
		stream >> owner;
		stream >> factoryId;
		stream >> unitType;
		
		Player* p = (Player*)findPlayer(owner);
		if (!p) {
			kdError() << k_lineinfo << "Cannot find player " << owner << endl;
			break;
		}
		Facility* factory = (Facility*)findUnit(factoryId, p);
		if (!factory) {
			kdError() << "Cannot find unit " << factoryId << endl;
			break;
		}
		if (!((Unit*)factory)->isFacility()) {
			kdError() << k_lineinfo << factoryId << " is not a facility" << endl;
			break;
		}
		if (unitType < 0) {
			kdError() << k_lineinfo << "Invalid unitType " << unitType << endl;
			break;
		}
		const UnitProperties* prop = p->unitProperties(unitType);
		if (!prop) {
			kdError() << k_lineinfo << "NULL properties (EVIL BUG)" << endl;
			break;
		}

		if (factory->currentProduction() == unitType) {
			if (factory->work() == Unit::WorkProduce) {
				// do not abort but just pause
				factory->setWork(Unit::WorkNone);
				emit signalUpdateProduction(factory);
			} else {
				p->setMinerals(p->minerals() + prop->mineralCost());
				p->setOil(p->oil() + prop->oilCost());
				factory->removeProduction();
				emit signalUpdateProduction(factory);
			}
		} else {
			//FIXME: money should be paid when the production is
			//actually started! (currently it is paid as soon as an
			//item is added to the queue)
			p->setMinerals(p->minerals() + prop->mineralCost());
			p->setOil(p->oil() + prop->oilCost());
			factory->removeProduction(unitType);
			emit signalUpdateProduction(factory);
		}
		break;
	}
	case BosonMessage::MoveBuild:
	{
		kdDebug() << "MoveBuild" << endl;
		Q_ULONG factoryId;
		Q_UINT32 owner;
		Q_INT32 x;
		Q_INT32 y;
		stream >> factoryId;
		stream >> owner;
		stream >> x;
		stream >> y;
		
		Player* p = (Player*)findPlayer(owner);
		if (!p) {
			kdError() << k_lineinfo << "Cannot find player " << owner << endl;
			break;
		}
		Facility* factory = (Facility*)findUnit(factoryId, p);
		if (!factory) {
			kdError() << "Cannot find unit " << factoryId << endl;
			break;
		}
		if (!((Unit*)factory)->isFacility()) {
			kdError() << k_lineinfo << factoryId << " is not a facility" << endl;
			break;
		}
		int unitType = factory->completedProduction();
		kdDebug() << k_lineinfo 
				<< "factory=" 
				<< factory->id() 
				<< ",unitid=" 
				<< unitType 
				<< endl;
		if (unitType < 0) {
			// hope this is working...
			kdWarning() << k_lineinfo << "not yet completed" << endl;
			break;
		}
		buildProducedUnit(factory, unitType, x, y);
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
		Q_INT32 owner;
		Q_INT32 unitType;
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
		Q_INT32 owner;
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
	case BosonMessage::Advance:
//		lock();
//		for (unsigned i = 0; i < 5; i++) {
			emit signalAdvance(d->mAdvanceCount);
			d->mAdvanceCount = d->mAdvanceCount + 1;
			if (d->mAdvanceCount >= MAXIMAL_ADVANCE_COUNT) {
				d->mAdvanceCount = 0;
			}
//			qApp->processEvents(5);
//		}
//		unlock();
		break;
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
	if (gameSpeed() != 0) {
		if (!d->mGameTimer->isActive()) {
			d->mGameTimer->start(gameSpeed());
		} else {
			kdWarning() << "timer was already active!" << endl;
		}
	}
 } else {
	kdWarning() << "is not server - cannot start the game!" << endl;
 }
}

void Boson::slotSendAdvance()
{
 sendMessage(0, BosonMessage::Advance);
}

Unit* Boson::createUnit(int unitType, Player* owner)
{
 if (!owner) {
	kdError() << k_funcinfo << ": NULL owner" << endl;
	return 0;
 }
 SpeciesTheme* theme = owner->speciesTheme();
 if (!theme) {
	kdError() << k_funcinfo << ": No theme for this player" << endl;
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
 kdDebug() << k_funcinfo << ": rtti=" << rtti << ",io=" << io 	
		<< ",isVirtual=" << isVirtual << endl;
 Player* p = new Player();
// connect(p, SIGNAL(signalLoadUnit(int, unsigned long int, Player*)), 
//		this, SLOT(slotLoadUnit(int, unsigned long int, Player*)));
 return p;
}

int Boson::gameSpeed() const
{
 return d->mGameSpeed;
}

void Boson::slotSetGameSpeed(int speed)
{
 if (d->mGameSpeed == speed) {
	return; // do not restart timer
 }
 if (speed < 0) {
	kdError() << "Invalid speed value " << speed << endl;
	return;
 }
 if ((speed > MIN_GAME_SPEED || speed < MAX_GAME_SPEED) && speed != 0) {
	kdWarning() << "unexpected speed " << speed << endl;
 }
// kdDebug() << k_funcinfo << ": " << speed << endl;
 d->mGameSpeed = speed;
 if (d->mGameTimer->isActive()) {
	d->mGameTimer->stop();
	if (d->mGameSpeed != 0) {
		d->mGameTimer->start(gameSpeed());
	}
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

void Boson::slotSendAddUnit(int unitType, int x, int y, Player* owner)
{ // used by the editor directly
 if (!isServer()) {
	return;
 }
 if (!owner) {
	kdWarning() << k_funcinfo << ": NULL owner! using first player" << endl;
	owner = (Player*)playerList()->at(0);
 }
 if (!owner) { // no player here
	kdError() << k_funcinfo << ": NULL owner" << endl;
	return;
 }

 QByteArray buffer;
 QDataStream stream(buffer, IO_WriteOnly);
 stream << (Q_INT32)owner->id();
 stream << (Q_INT32)unitType;
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
	kdWarning() << k_funcinfo << ": NULL owner! using first player" << endl;
	owner = (Player*)playerList()->at(0);
 }
 if (!owner) { // no player here
	kdError() << k_funcinfo << ": NULL owner" << endl;
	return;
 }
 QByteArray buffer;
 QDataStream stream(buffer, IO_WriteOnly);
 stream << (Q_INT32)owner->id();
 stream << xmlDocument;
 sendMessage(buffer, BosonMessage::AddUnitsXML);
}

void Boson::slotReplacePlayerIO(KPlayer* player, bool* remove)
{
 *remove = false;
 if (!player) {
	kdError() << k_funcinfo << ": NULL player" << endl;
	return;
 }
 if (!isAdmin()) {
	kdError() << k_funcinfo << ": only ADMIN can do this" << endl; 
	return;
 }
// kdDebug() << k_funcinfo << endl;
}

bool Boson::buildProducedUnit(Facility* factory, int unitType, int x, int y)
{
 if (!factory) {
	kdError() << k_funcinfo << ": NULL factory cannot produce" << endl;
	return false;
 }
 Player* p = factory->owner();
 if (!p) {
	kdError() << k_funcinfo << ": NULL owner" << endl;
	return false;
 }
 QCanvasPixmapArray* a = p->speciesTheme()->pixmapArray(unitType);
 if (!a) {
	kdError() << k_funcinfo << ": NULL pximap array for " << unitType 
			<< endl;
	return false;
 }

 QRect rect = (QRect(
		x * BO_TILE_SIZE,
		y * BO_TILE_SIZE,
		a->image(0)->width(),
		a->image(0)->height()));
 if (!((BosonCanvas*)d->mCanvas)->canGo(p->unitProperties(unitType), rect)) {
	kdWarning() << "Unit can not be produced here" << endl;
	return false;
 }
// qt bug (confirmed). will be fixed in 3.1
#if QT_VERSION >= 310
 QCanvasItemList list = d->mCanvas->collisions(QRect(
		x * BO_TILE_SIZE,
		y * BO_TILE_SIZE, 
		a->image(0)->width(),
		a->image(0)->height()));
#else
 QCanvasItemList list = d->mCanvas->collisions(QRect(
		x * BO_TILE_SIZE,
		y * BO_TILE_SIZE, 
		a->image(0)->width() - 1,
		a->image(0)->height() - 1));
#endif
 QCanvasItemList::Iterator it;
 for (it = list.begin(); it != list.end(); ++it) {
	if (!RTTI::isUnit((*it)->rtti())) {
		continue; // this item is not interesting here
	}
	Unit* unit = (Unit*)*it;
	if (!unit->isDestroyed() && !unit->isFlying()) {
		kdDebug() << "Cannot create unit here" << endl;
		return false;
	}
 }
 Unit* unit = addUnit(unitType, p, x, y);
 if (unit->isFacility()) {
	p->statistics()->addProducedFacility((Facility*)unit, factory);
 } else {
	p->statistics()->addProducedMobileUnit((MobileUnit*)unit, factory);
 }
 
 // the current production is done.
 factory->removeProduction();
 emit signalUpdateProduction(factory);
 return true;
}

Unit* Boson::addUnit(int unitType, Player* p, int x, int y)
{
 Unit* unit = createUnit(unitType, (Player*)p);
 unit->setId(nextUnitId());
 emit signalAddUnit(unit, x, y);
 return unit;
}

Unit* Boson::addUnit(QDomElement& node, Player* p)
{
 int unitType = 0;
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
 
 emit signalAddUnit(unit, x, y);
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
}

void Boson::slotAdvanceComputerPlayers(unsigned int /*advanceCount*/)
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
//		kdError() << k_funcinfo << "NULL speciesTheme for " << it.current()->id() << endl;
		colors.remove(((Player*)it.current())->speciesTheme()->teamColor());
	}
	++it;
 }
 return colors;
}