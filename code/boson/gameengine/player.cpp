/*
    This file is part of the Boson game
    Copyright (C) 1999-2000 Thomas Capricelli (capricel@email.enst.fr)
    Copyright (C) 2001-2005 Andreas Beckermann (b_mann@gmx.de)

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

#include "player.h"

#include "../bomemory/bodummymemory.h"
#include "playerio.h"
#include "speciestheme.h"
#include "unit.h"
#include "unitproperties.h"
#include "unitplugins.h"
#include "bosonmessageids.h"
#include "bosonmap.h"
#include "bosonstatistics.h"
#include "boson.h"
#include "upgradeproperties.h"
#include "bosonitempropertyhandler.h"
#include "bosonpropertyxml.h"
#include "bosonprofiling.h"
#include "bodebug.h"
#include "bobincoder.h"
#include "boevent.h"

#include <kgame/kgame.h>
#include <kgame/kgamemessage.h>

#include <qbitarray.h>
#include <qdom.h>
#include <qtextstream.h>

#include "player.moc"


class PlayerPrivate
{
public:
	PlayerPrivate()
	{
		mUnitPropID = 0;
		mMap = 0;
		mFoggedRef = 0;

		mStatistics = 0;

		mPlayerIO = 0;
	}

	QPtrList<Unit> mUnits;

	BosonMap* mMap; // just a pointer
	int mUnitPropID; // used for KGamePropertyHandler

	QBitArray mExplored; // TODO: use KGameProperty
	unsigned short int* mFoggedRef;
	unsigned int mExploredCount; // helper variable, doesn't need to be saved
	unsigned int mUnfoggedCount; // helper variable, doesn't need to be saved
	KGameProperty<unsigned long int> mMinerals;
	KGameProperty<unsigned long int> mOil;
	KGamePropertyBool mIsNeutralPlayer;
	QMap<QString, unsigned long int> mAmmunition; // TODO: use KGameProperty somehow

	BosonStatistics* mStatistics;

	int mMobilesCount;
	int mFacilitiesCount;

	PlayerIO* mPlayerIO;

	QValueList<unsigned long int> mResearchedUpgrades;

	BoUpgradesCollection mUpgradesCollection;

	QValueList<const Unit*> mRadarUnits;
};

Player::Player(bool isNeutralPlayer) : KPlayer()
{
 boDebug() << k_funcinfo << endl;
 mSpecies = 0;
 d = new PlayerPrivate;
 d->mExploredCount = 0;
 d->mUnfoggedCount = 0;
 mPowerChargeForCurrentAdvanceCall = 0;
 setAsyncInput(true);
 connect(this, SIGNAL(signalNetworkData(int, const QByteArray&, Q_UINT32, KPlayer*)),
		this, SLOT(slotNetworkData(int, const QByteArray&, Q_UINT32, KPlayer*)));

 KGamePropertyBase* propName = dataHandler()->find(KGamePropertyBase::IdName);
 if (propName) {
	propName->setPolicy(KGamePropertyBase::PolicyClean);
 } else {
	boError() << k_funcinfo << "can't find name property" << endl;
 }
// TODO d->mFogged.registerData() or something like this
 mOutOfGame.registerData(IdOutOfGame, dataHandler(),
		KGamePropertyBase::PolicyLocal, "OutOfGame");
 mHasLost.registerData(IdHasLost, dataHandler(),
		KGamePropertyBase::PolicyLocal, "HasLost");
 mHasWon.registerData(IdHasWon, dataHandler(),
		KGamePropertyBase::PolicyLocal, "HasWon");
 d->mMinerals.registerData(IdMinerals, dataHandler(),
		KGamePropertyBase::PolicyLocal, "Minerals");
 d->mOil.registerData(IdOil, dataHandler(),
		KGamePropertyBase::PolicyLocal, "Oil");
 d->mIsNeutralPlayer.registerData(IdIsNeutralPlayer, dataHandler(),
		KGamePropertyBase::PolicyLocal, "IsNeutralPlayer");
 d->mIsNeutralPlayer = isNeutralPlayer;
 mOutOfGame = false;
 mHasLost = false;
 mHasWon = false;
 d->mPlayerIO = new PlayerIO(this);

 quitGame(); // this will reset some variables
}

Player::~Player()
{
 boDebug() << k_funcinfo << endl;
 quitGame(true);
 dataHandler()->clear(); // this must not be in quitGame()
 delete mSpecies;
 delete d->mPlayerIO;
 delete d;
 boDebug() << k_funcinfo << "done" << endl;
}

int Player::bosonId() const
{
 return userId();
}

bool Player::isGamePlayer() const
{
 if (bosonId() >= 128 && bosonId() <= 511) {
	return true;
 }
 return false;
}

bool Player::isActiveGamePlayer() const
{
 if (bosonId() >= 128 && bosonId() <= 255) {
	return true;
 }
 return false;
}

bool Player::isNeutralPlayer() const
{
 return d->mIsNeutralPlayer;
}

PlayerIO* Player::playerIO() const
{
 return d->mPlayerIO;
}

BosonMap* Player::map() const
{
 return d->mMap;
}

void Player::quitGame(bool destruct)
{
 d->mMobilesCount = 0;
 d->mFacilitiesCount = 0;
 d->mUnfoggedCount = 0;
 d->mExploredCount = 0;
 mOutOfGame = false;
 mHasWon = false;
 mHasLost = false;
 d->mMap = 0;
 d->mResearchedUpgrades.clear();

 d->mUnits.clear();
 delete d->mStatistics;
 d->mStatistics = 0;

 if (!destruct) {
	d->mStatistics = new BosonStatistics;
	d->mMinerals.setLocal(0);
	d->mOil.setLocal(0);
	d->mExplored.resize(0);
	delete[] d->mFoggedRef;
	d->mAmmunition.clear();
	d->mAmmunition.insert("Generic", 0);
 }
}

void Player::slotNetworkData(int msgid, const QByteArray& buffer, Q_UINT32 sender, KPlayer*)
{
 // there are only very few messages handled here. Only those that have
 // PolicyClean or PolicyDirty. All others are in slotUnitPropertiesChanged()

 QDataStream stream(buffer, IO_ReadOnly);
 bool issender = true;
 if (game()) {
	issender = sender == game()->gameId();
 }
// first check if the message is a property of a unit
 QPtrListIterator<Unit> it(d->mUnits);
 while (it.current() && !it.current()->dataHandler()->processMessage(stream, msgid + KGamePropertyBase::IdUser, issender)) {
	++it;
 }
 Unit* unit = it.current();
 if (unit) { // this was a unit property
	// nothing to do here anymore (revisions < r7629 did something here)
	return;
 }

 // this wasn't a unit property but a normal message
 switch (msgid) {
	case BosonMessageIds::IdChat:
		break;
	default:
		boWarning() << k_funcinfo << "Unknown message " << msgid << endl;
		break;
 }
}


void Player::loadTheme(const QString& species, const QColor& teamColor)
{
 if (species == QString::fromLatin1("Neutral")) {
	if (!isNeutralPlayer()) {
		boError() << k_funcinfo << "only neutral player can have neutral theme" << endl;
		return;
	}
 }
 SpeciesTheme* newTheme = new SpeciesTheme();
 if (!newTheme->loadTheme(species, teamColor)) {
	boError() << k_funcinfo << "cannot load theme " << species << endl;
	delete newTheme;
	return;
 }
 delete mSpecies;
 mSpecies = newTheme;
}

void Player::addUnit(Unit* unit, int dataHandlerId)
{
 d->mUnits.append(unit);
 if (dataHandlerId == -1) {
	dataHandlerId = BosonMessageIds::UnitPropertyHandler + d->mUnitPropID;
	d->mUnitPropID++;// used for ID of KGamePropertyHandler
 }
 unit->dataHandler()->registerHandler(dataHandlerId, this,
		SLOT(sendProperty(int, QDataStream&, bool*)),
		SLOT(slotUnitPropertyChanged(KGamePropertyBase*)));

 if (unit->isMobile()) {
	d->mMobilesCount++;
 } else {
	d->mFacilitiesCount++;
 }
}

void Player::unitDestroyed(Unit* unit)
{
 if (!unit) {
	boError() << k_funcinfo << "Cannot remove NULL unit" << endl;
	return;
 }
 d->mUnits.take(d->mUnits.findRef(unit));
 if (unit->isFacility()) {
	statistics()->addLostFacility(unit);
	d->mFacilitiesCount--;
 } else {
	statistics()->addLostMobileUnit(unit);
	d->mMobilesCount--;
 }
}

void Player::slotUnitPropertyChanged(KGamePropertyBase* prop)
{
 if (!prop) {
	boError() << k_funcinfo << "NULL property" << endl;
	return;
 }

 bool emitSignalUnitChanged = false;
 switch (prop->id()) {
	case UnitBase::IdHealthFactor:
	case UnitBase::IdArmorFactor:
	case UnitBase::IdShieldsFactor:
	case UnitBase::IdSightRangeFactor:
		// update BosonUnitView if the unit is selected.
		// not all of these IDs are displayed there. But perhaps they
		// will one day.
		emitSignalUnitChanged = true;
		break;
	default:
		// all other Unit IDs are not displayed in BosonUnitView so
		// there is no need to emit a signal for them.
		break;
 }
 if (!emitSignalUnitChanged) {
	// nothing to do here
	return;
 }

 // AB: we don't check for sender()->isA() here, for performance reasons.
 BosonItemPropertyHandler* p = (BosonItemPropertyHandler*)sender();
 if (!p->item()) {
	boError() << k_funcinfo << "NULL parent item for property handler" << endl;
	boDebug() << "player=" << bosonId() << ",propId=" << prop->id() << endl;
	return;
 }
 if (!RTTI::isUnit(p->item()->rtti())) {
	return;
 }
 Unit* unit = (Unit*)p->item();
 if (!unit) {
	boError() << k_funcinfo << "NULL unit" << endl;
	return;
 }
 if (emitSignalUnitChanged) {
	emit signalUnitChanged(unit);
 }
}

Unit* Player::findUnit(unsigned long int unitId) const
{
 QPtrListIterator<Unit> it(d->mUnits);
 while (it.current()) {
	if (it.current()->id() == unitId) {
		return it.current();
	}
	++it;
 }
 return 0;
}

bool Player::save(QDataStream& stream)
{
// we need save() and load() for the new game dialog. The units part should be
// unused, only the species theme should be necessary.
 boDebug() << k_funcinfo << endl;
 if (!KPlayer::save(stream)) {
	boError() << k_funcinfo << "Couldn't save KPlayer" << endl;
	return false;
 }

 // Save speciestheme
 if (speciesTheme()) {
	stream << speciesTheme()->identifier();
	stream << speciesTheme()->teamColor();
 } else {
	 stream << QString::null;
	 stream << QColor(0, 0, 0);
 }

 // Save unitpropID
 Q_UINT32 unitPropId = d->mUnitPropID;
 stream << unitPropId;


 // AB: see load()
 /*
 // Save fog
 stream << d->mFogged;

 // Save statistics
 d->mStatistics->save(stream);
 */

 /// TODO: save researched technologies!!!

 return true;
}

bool Player::load(QDataStream& stream)
{
 boDebug() << k_funcinfo << endl;
 if (!KPlayer::load(stream)) {
	boError() << k_funcinfo << "Couldn't load KPlayer" << endl;
	return false;
 }

 // Load speciestheme
 QString themeIdentifier;
 QColor teamColor;
 stream >> themeIdentifier;
 stream >> teamColor;
 if (themeIdentifier != QString::null) {
	loadTheme(SpeciesTheme::speciesDirectory(themeIdentifier), teamColor);
 } else {
	boError() << k_funcinfo << "NULL species theme identifier" << endl;
	return false;
 }

 // Load unitpropID
 Q_UINT32 unitPropId;
 stream >> unitPropId;
 d->mUnitPropID = unitPropId;

 // these are necessary for loading games only. for that we use loadFromXML()
 // now. note that Player::load() still gets called when a player enters a
 // network game - we may want to fix this in order to enable players to join an
 // already running game!
 /*
 // Load fog
 stream >> d->mFogged;

 // Load statistics
 d->mStatistics->load(stream);
 */

 return true;
}

QPtrList<Unit>* Player::allUnits() const
{
 return &(d->mUnits);
}

const UnitProperties* Player::unitProperties(unsigned long int unitType) const
{
 if (!speciesTheme()) {
	boError() << k_funcinfo << "NULL theme" << endl;
	return 0;
 }

 // TODO: remove this check as soon as the reason for the current crash on
 // building-placing (02/01/12) is found and fixed
 if (!speciesTheme()->unitProperties(unitType)) {
	boError() << k_lineinfo << "NULL unit properties (VERY EVIL BUG!!)" << endl;
 }
 return speciesTheme()->unitProperties(unitType);
}

void Player::initMap(BosonMap* map, bool unexplored, bool fogged)
{
 d->mMap = map;
 if (!d->mMap) {
	d->mExplored.resize(0);
	return;
 }
 if (d->mExplored.size() != 0) {
	// the fog of war was initialized before. probably we are loading a
	// game.
	return;
 }
 int cellcount = d->mMap->width() * d->mMap->height();
 d->mExplored.fill(!unexplored, cellcount);
 d->mExploredCount = (unexplored ? 0 : cellcount);
 d->mFoggedRef = new unsigned short int[cellcount];
 for (int i = 0; i < cellcount; i++) {
	d->mFoggedRef[i] = (fogged ? 0 : 1);
 }
 d->mUnfoggedCount = (fogged ? 0 : cellcount);
}

void Player::addFogRef(int x, int y)
{
 if (!d->mMap || !d->mFoggedRef) {
	return;
 }

 unsigned int index = x + d->mMap->width() * y;
 d->mFoggedRef[index]++;
 if (d->mFoggedRef[index] == 1) {
	d->mUnfoggedCount++;
	explore(x, y);
	emit signalUnfog(x, y);
 }
}

void Player::removeFogRef(int x, int y)
{
 if (!d->mMap || !d->mFoggedRef) {
	return;
 }

 unsigned int index = x + d->mMap->width() * y;
 if (d->mFoggedRef[index] == 0) {
	boError() << k_funcinfo << "mFoggedRef is already 0 at (" << x << "; " << y << ")!" << endl;
	return;
 }
 d->mFoggedRef[index]--;
 if (d->mFoggedRef[index] == 0) {
	d->mUnfoggedCount--;
	emit signalFog(x, y);
 }
}

bool Player::isFogged(int x, int y) const
{
 // TODO: error checking
 /*if (x + d->mMap->width() * y >= d->mFogged.size()) {
	boError() << k_funcinfo << "x=" << x << ",y=" << y << " out of range ("
			<< d->mFogged.size() << ")" << endl;
	return true;
 }*/
 return d->mFoggedRef[x + d->mMap->width() * y] == 0;
}

void Player::explore(int x, int y)
{
 if (!d->mMap) {
	return;
 }
 unsigned int index = x + d->mMap->width() * y;
 if (index >= d->mExplored.size()) {
	boError() << k_funcinfo << "x=" << x << ",y=" << y << " out of range ("
			<< d->mExplored.size() << ")" << endl;
	return;
 }
 bool isExplored = d->mExplored.at(index);
//boDebug() << k_funcinfo << x << "," << y << endl;
 if (!isExplored) {
	d->mExplored.setBit(index);
	if (d->mExploredCount >= d->mExplored.size()) {
		boError() << k_funcinfo << "invalid explored count value " << d->mExploredCount << endl;
	} else {
		d->mExploredCount++;
	}
	// emit signal (actual fog on map + minimap)
	// TODO: any way to emit only for the local player?
	emit signalExplored(x, y);
 }
//boDebug() << k_funcinfo << "done " << endl;
}

void Player::unexplore(int x, int y)
{
 if (!d->mMap) {
	return;
 }
//boDebug() << k_funcinfo << x << "," << y << endl;
 unsigned int index = x + d->mMap->width() * y;
 if (index >= d->mExplored.size()) {
	boError() << k_funcinfo << "x=" << x << ",y=" << y << " out of range ("
			<< d->mExplored.size() << ")" << endl;
	return;
 }
 bool isExplored = d->mExplored.at(index);
 if (isExplored) {
	d->mExplored.clearBit(index);
	if (d->mExploredCount == 0) {
		boError() << k_funcinfo << "invalid fogged count value " << d->mExploredCount << endl;
	} else {
		d->mExploredCount--;
	}
	// emit signal (actual fog on map + minimap)
	// TODO: any way to emit only for the local player?
	emit signalUnexplored(x, y);
 }
//boDebug() << k_funcinfo << "done " << endl;
}

bool Player::isExplored(int x, int y) const
{
 unsigned int index = x + d->mMap->width() * y;
 if (index >= d->mExplored.size()) {
	boError() << k_funcinfo << "x=" << x << ",y=" << y << " out of range ("
			<< d->mExplored.size() << ")" << endl;
	return true;
 }
 return d->mExplored.at(index);
}

unsigned long int Player::minerals() const
{
 return d->mMinerals;
}

unsigned long int Player::oil() const
{
 return d->mOil;
}

unsigned long int Player::ammunition(const QString& type) const
{
 if (!d->mAmmunition.contains(type)) {
	return 0;
 }
 return d->mAmmunition[type];
}

void Player::setOil(unsigned long int o)
{
 d->mOil = o;
}

void Player::setMinerals(unsigned long int m)
{
 d->mMinerals = m;
}

bool Player::useMinerals(unsigned long int amount)
{
 if (minerals() < amount) {
	return false;
 } else {
	setMinerals(minerals() - amount);
	return true;
 }
}

bool Player::useOil(unsigned long int amount)
{
 if (oil() < amount) {
	return false;
 } else {
	setOil(oil() - amount);
	return true;
 }
}

bool Player::useResources(unsigned long int mineralamount, unsigned long int oilamount)
{
 if (minerals() < mineralamount || oil() < oilamount) {
	return false;
 } else {
	setMinerals(minerals() - mineralamount);
	setOil(oil() - oilamount);
	return true;
 }
}

void Player::setAmmunition(const QString& type, unsigned long int a)
{
 d->mAmmunition.insert(type, a);
}

unsigned long int Player::requestAmmunition(const QString& type, unsigned long int requested)
{
 boDebug(610) << k_funcinfo << "type=" << type << " requested=" << requested << endl;
 unsigned long int ammo = 0;
 if (ammunition(type) < requested) {
	ammo = ammunition(type);
 } else {
	ammo = requested;
 }
 setAmmunition(type, ammunition(type) - ammo);
 boDebug(610) << k_funcinfo << "giving: " << ammo << " have left: " << ammunition(type) << endl;

 if (ammo > requested) {
	boError() << k_funcinfo << "received more ammo than requested" << endl;
	ammo = requested;
 }
 if (ammo == requested) {
	return ammo;
 }

 boDebug(610) << k_funcinfo << "searching for ammo in AmmunitionStoragePlugins" << endl;
 // not enough ammo of this type in the global pool - search for globally
 // accessible ammo in ammunition storages
 for (QPtrListIterator<Unit> it(*allUnits()); it.current(); ++it) {
	if (it.current()->isDestroyed()) {
		continue;
	}
	AmmunitionStoragePlugin* p = (AmmunitionStoragePlugin*)it.current()->plugin(UnitPlugin::AmmunitionStorage);
	if (!p) {
		continue;
	}
	if (!p->mustBePickedUp(type)) {
		ammo += p->requestAmmunitionGlobally(type, requested - ammo);
	}
	if (ammo > requested) {
		boError() << k_funcinfo << "received more ammo than requested" << endl;
		ammo = requested;
	}
	if (ammo == requested) {
		boDebug(610) << k_funcinfo << "giving " << ammo << endl;
		return ammo;
	}
 }
 boDebug(610) << k_funcinfo << "can give " << ammo << " only" << endl;
 return ammo;
}

const QColor& Player::teamColor() const
{
 if (!speciesTheme()) {
	BO_NULL_ERROR(speciesTheme());
	return Qt::red;
 }
 return speciesTheme()->teamColor();
}

bool Player::hasMiniMap() const
{
 unsigned long int powerGenerated = 0;
 unsigned long int powerConsumed = 0;
 calculatePower(&powerGenerated, &powerConsumed);
 if (powerGenerated < powerConsumed) {
	return false;
 }
 QPtrListIterator<Unit> it(d->mUnits);
 while (it.current()) {
	if (it.current()->unitProperties()->supportMiniMap()) {
		if (!it.current()->isFacility()) {
			return true;
		} else {
			UnitConstruction* c = it.current()->construction();
			if (c->isConstructionComplete()) {
				return true;
			}
		}
	}
	++it;
 }
 return false;
}

void Player::unchargeUnitsForAdvance()
{
 BosonProfiler prof("unchargeUnitsForAdvance");
 for (QPtrListIterator<Unit> it(d->mUnits); it.current(); ++it) {
	it.current()->unchargePowerForAdvance();
 }
}

void Player::updatePowerChargeForCurrentAdvanceCall()
{
 unsigned long int powerGenerated = 0;
 unsigned long int powerConsumed = 0;
 calculatePower(&powerGenerated, &powerConsumed);

 // the factor specifies how much the unit is influenced by the lack of power.
 // 1.0 means that it works at full speed, 0.0 means it is disabled.
 bofixed factor = 1.0f;
 if (powerGenerated >= powerConsumed) {
	// no lack of power.
	factor = 1;
 } else {
	factor = ((float)powerGenerated) / ((float)powerConsumed);

	// we use quadratic relationship currently
	// this means:
	// powerGenerated	speed
	// 90%			80%
	// 70%			50%
	// 30%			10%
	factor = factor * factor;

	// if factor < 0.08 (i.e. speed is less than 8% of full speed), we
	// disable the units completely.
	if (factor < 0.08f) {
		factor = 0;
	}
 }

 if (factor < 0 || factor > 1) {
	boError() << k_funcinfo << "invalid factor " << factor << endl;
	factor = 1;
 }
 mPowerChargeForCurrentAdvanceCall = factor;
}

void Player::calculatePower(unsigned long int* _powerGenerated, unsigned long int* _powerConsumed, bool includeUnconstructedFacilities) const
{
 // AB: this method takes O(n) !
 //     note that it would be very hard to improve this:
 //     * we cannot assume that only facilities generate/consume power
 //       (atm this is the case, but this may change)
 //     * we cannot maintain a list of units that generate/consume power, as
 //       this list may change on the fly: using upgrades even units that cannot
 //       generate/consume power may do so later
 unsigned int powerGenerated = 0;
 unsigned int powerConsumed = 0;
 for (QPtrListIterator<Unit> it(d->mUnits); it.current(); ++it) {
	powerConsumed += it.current()->powerConsumedByUnit();
	if (!includeUnconstructedFacilities) {
		if (it.current()->isFacility()) {
			UnitConstruction* c = it.current()->construction();
			if (!c->isConstructionComplete()) {
				continue;
			}
		}
	}
	powerGenerated += it.current()->powerGeneratedByUnit();
 }
 if (_powerGenerated) {
	*_powerGenerated = powerGenerated;
 }
 if (_powerConsumed) {
	*_powerConsumed = powerConsumed;
 }
}

void Player::facilityCompleted(Unit* fac)
{
 if (!fac) {
	boError() << k_funcinfo << "NULL facility" << endl;
	return;
 }

 BoVector3Fixed location(fac->x(), fac->y(), fac->z());
 BoEvent* constructedEvent = new BoEvent("FacilityWithTypeConstructed", QString::number(fac->type()));
 constructedEvent->setPlayerId(bosonId());
 constructedEvent->setUnitId(fac->id());
 constructedEvent->setLocation(location);
 boGame->queueEvent(constructedEvent);
}

void Player::setOutOfGame()
{
 if (!isActiveGamePlayer()) {
	// non-active players (e.g. neutral players) are never out of the game
	mOutOfGame = false;
	return;
 }
 mOutOfGame = true;
}

BosonStatistics* Player::statistics() const
{
 return d->mStatistics;
}

bool Player::isEnemy(const Player* p) const
{
 return isPlayerEnemy(p->bosonId());
}

bool Player::isPlayerEnemy(int id) const
{
 if (isPlayerAllied(id) || isPlayerNeutral(id)) {
	return false;
 }
 return true;
}

bool Player::isNeutral(const Player* p) const
{
 return isPlayerNeutral(p->bosonId());
}

bool Player::isPlayerNeutral(int id) const
{
 // one day we might implement advanced relationships between players, so that
 // for two players A,B there may be the following cases:
 // 1. A and B are allies
 // 2. A and B are enemies
 // 3. A and B are neutral to each other

 // atm only the "neutral player" is neutral to another player.
 if (id == 256) {
	return true;
 }
 return false;
}

bool Player::isAllied(const Player* p) const
{
 if (p == this) {
	return true;
 }

 // allied players are not implemented yet
 return false;
}

bool Player::isPlayerAllied(int id) const
{
 if (id == bosonId()) {
	return true;
 }

 // allied players are not implemented yet
 return false;
}

int Player::mobilesCount()
{
 return d->mMobilesCount;
}

int Player::facilitiesCount()
{
 return d->mFacilitiesCount;
}

bool Player::canBuild(unsigned long int unitType) const
{
 QValueList<unsigned long int> neededTypes = speciesTheme()->unitProperties(unitType)->requirements();
 if (neededTypes.isEmpty()) {
	return true;
 }
 QValueList<unsigned long int>::Iterator it;
 for (it = neededTypes.begin(); it != neededTypes.end(); ++it) {
	// FIXME: this is SLOW. Cache values and refresh them when new unit is
	//  created or destroyed
	if (!hasUnitWithType(*it)) {
		return false;
	}
 }
 return true;
}

bool Player::canResearchTech(unsigned long int id) const
{
 // Check for technologies
 QValueList<unsigned long int> neededTechs = speciesTheme()->technology(id)->requiredTechnologies();
 if (!neededTechs.isEmpty()) {
	QValueList<unsigned long int>::Iterator it;
	for (it = neededTechs.begin(); it != neededTechs.end(); ++it) {
		if (!hasTechnology(*it)) {
			return false;
		}
	}
 }

 // Check for units
 QValueList<unsigned long int> neededUnits = speciesTheme()->technology(id)->requiredUnits();
 if (!neededUnits.isEmpty()) {
	QValueList<unsigned long int>::Iterator it;
	for (it = neededUnits.begin(); it != neededUnits.end(); ++it) {
		if (!hasUnitWithType(*it)) {
			return false;
		}
	}
 }

 return true;
}

bool Player::hasUnitWithType(unsigned long int type) const
{
 QPtrListIterator<Unit> it(d->mUnits);
 while (it.current()) {
	if (it.current()->type() == type) {
		if (it.current()->isMobile()) {
			return true;
		}
		UnitConstruction* c = it.current()->construction();
		if (c->isConstructionComplete()) {
			return true;
		}
	}
	++it;
 }
 return false;
}

bool Player::hasTechnology(unsigned long int id) const
{
 if (d->mResearchedUpgrades.contains(id)) {
	return true;
 }
 return false;
}

const UpgradeProperties* Player::technologyProperties(unsigned long int type) const
{
 return speciesTheme()->technology(type);
}

void Player::technologyResearched(ProductionPlugin* plugin, unsigned long int type)
{
 BO_CHECK_NULL_RET(plugin);
 BO_CHECK_NULL_RET(plugin->unit());
 boDebug() << k_funcinfo << "type: " << type << endl;
 // Check if it isn't researched already
 if (d->mResearchedUpgrades.contains(type)) {
	boError() << k_funcinfo << "upgrade " << type << " already researched!" << endl;
	return;
 }

 const UpgradeProperties* prop = speciesTheme()->technology(type);
 if (!prop) {
	boError() << k_funcinfo << "NULL technology " << type << endl;
	return;
 }
 d->mResearchedUpgrades.append(type);

 addUpgrade(prop);

 BoEvent* event = new BoEvent("TechnologyWithTypeResearched", QString::number(type), QString::number(plugin->unit()->id()));
 event->setPlayerId(bosonId());
 event->setLocation(BoVector3Fixed(plugin->unit()->x(), plugin->unit()->y(), plugin->unit()->z()));
 ((Boson*)game())->queueEvent(event);
}

bool Player::advanceFlag() const
{
 return ((Boson*)game())->advanceFlag();
}

bool Player::saveAsXML(QDomElement& root)
{
 PROFILE_METHOD
 if (!game()) {
	boError() << k_funcinfo << "NULL game" << endl;
	return false;
 }
 if (!speciesTheme()) {
	BO_NULL_ERROR(speciesTheme());
	return false;
 }

 root.setAttribute(QString::fromLatin1("PlayerId"), bosonId());

 if (bosonId() == 256) {
	root.setAttribute(QString::fromLatin1("IsNeutral"), 1);
 }

 root.setAttribute(QString::fromLatin1("NetworkPriority"), networkPriority());

 QDomDocument doc = root.ownerDocument();

 // store the dataHandler()
 BosonCustomPropertyXML propertyXML;
 QDomElement handler = doc.createElement(QString::fromLatin1("DataHandler"));
 if (!propertyXML.saveAsXML(handler, dataHandler())) {
	boError() << k_funcinfo << "Unable to save datahandler of player " << bosonId() << endl;
	return false;
 }
 root.appendChild(handler);

 QDomElement upgrades = doc.createElement("Upgrades");
 d->mUpgradesCollection.saveAsXML(upgrades);
 root.appendChild(upgrades);

 QDomElement speciesThemeTag = doc.createElement("SpeciesTheme");
 if (!speciesTheme()->saveGameDataAsXML(speciesThemeTag)) {
	boError() << k_funcinfo << "Unable to save SpeciesTheme" << endl;
	return false;
 }
 root.appendChild(speciesThemeTag);

 // Save unitpropID
 root.setAttribute(QString::fromLatin1("UnitPropId"), (unsigned int)d->mUnitPropID);

 // Save fog
 if (!saveFogOfWar(root)) {
	return false;
 }

 if (!saveAmmunition(root)) {
	return false;
 }

 // Save statistics
 QDomElement statistics = doc.createElement(QString::fromLatin1("Statistics"));
 d->mStatistics->save(statistics);
 root.appendChild(statistics);

 return true;
}

bool Player::loadFromXML(const QDomElement& root)
{
 boDebug(260) << k_funcinfo << bosonId() << endl;
 // this does NOT load the units!

 bool ok = false;
 if (!game()) {
	boError() << k_funcinfo << "NULL game" << endl;
	return false;
 }
 if (root.hasAttribute("IsNeutral")) {
	if (root.attribute("IsNeutral").toUInt(&ok) != 1) {
		boError() << k_funcinfo << "invalid file format: IsNeutral must be 1 if it is present" << endl;
		return false;
	} else if (!ok) {
		boError() << k_funcinfo << "invalid file format: IsNeutral must be a valid number if it is present" << endl;
		return false;
	}
 }

 boDebug(260) << k_funcinfo << "load data handler" << endl;
 // load the data handler
 BosonCustomPropertyXML propertyXML;
 QDomElement handler = root.namedItem(QString::fromLatin1("DataHandler")).toElement();
 if (!propertyXML.loadFromXML(handler, dataHandler())) {
	boError(260) << k_funcinfo << "unable to load player data handler (player=" << this->bosonId() << ")" << endl;
	return false;
 }

 clearUpgrades();
 QDomElement upgrades = root.namedItem("Upgrades").toElement();
 if (upgrades.isNull()) {
	boError(260) << k_funcinfo << "no Upgrades tag found" << endl;
	return false;
 }
 if (!d->mUpgradesCollection.loadFromXML(speciesTheme(), upgrades)) {
	boError(260) << k_funcinfo << "errror loading upgrades" << endl;
	return false;
 }

 QDomElement speciesThemeTag = root.namedItem("SpeciesTheme").toElement();
 if (speciesThemeTag.isNull()) {
	boError(260) << k_funcinfo << "NULL SpeciesTheme tag" << endl;
	return false;
 }
 if (!speciesTheme()->loadGameDataFromXML(speciesThemeTag)) {
	boError(260) << k_funcinfo << "Unable to load SpeciesTheme" << endl;
	return false;
 }

 if (root.hasAttribute(QString::fromLatin1("UnitPropId"))) {
	d->mUnitPropID = root.attribute(QString::fromLatin1("UnitPropId")).toUInt(&ok);
	if (!ok) {
		boError(260) << k_funcinfo << "Invalid UnitPropId value" << endl;
		return false;
	}
 }


 // Load fog
 if (!root.namedItem(QString::fromLatin1("Fogged")).isNull()) {
	boDebug(260) << k_funcinfo << "loading fow" << endl;
	if (!loadFogOfWar(root)) {
		boError(260) << k_funcinfo << "loading fog of war failed" << endl;
		return false;
	}
 }

 if (!loadAmmunition(root)) {
	return false;
 }

 // Load statistics
 QDomElement statistics = root.namedItem(QString::fromLatin1("Statistics")).toElement();
 if (!statistics.isNull()) {
	d->mStatistics->load(statistics);
 }

 boDebug(260) << k_funcinfo << "done" << endl;
 return true;
}

bool Player::saveFogOfWar(QDomElement& root) const
{
 // AB: I've tried several ways of doing this but couldn't find the solution. so
 // now we simply stream every single bit as a complete byte. not good, but
 // works
 // also note that i don't want to do the bit magic myself, as i want to avoid
 // this trouble with strings/xml and e.g. \0
 QDomDocument doc = root.ownerDocument();
 QDomElement explored = doc.createElement(QString::fromLatin1("Explored"));
 explored.appendChild(doc.createTextNode(BoBinCoder::toCoded(d->mExplored)));
 root.appendChild(explored);

 QDomElement fog = doc.createElement(QString::fromLatin1("Fogged"));
 int fogsize = d->mMap->width() * d->mMap->height() * sizeof(unsigned short int);
 fog.appendChild(doc.createTextNode(BoBinCoder::toCoded((const char*)d->mFoggedRef, fogsize)));
 root.appendChild(fog);
 return true;
}

bool Player::loadFogOfWar(const QDomElement& root)
{
 QDomElement element = root.namedItem(QString::fromLatin1("Explored")).toElement();
 if (element.isNull()) {
	boError() << k_funcinfo << "No Explored tag found" << endl;
	d->mExplored.fill(true);
	d->mExploredCount = d->mExplored.size();
	return false;
 }
 QString text = element.text();
 if (text.isEmpty()) {
	boError() << k_funcinfo << "no content for Explored tag found" << endl;
	d->mExplored.fill(true);
	d->mExploredCount = d->mExplored.size();
	return false;
 }
 boDebug() << k_funcinfo << "decoding" << endl;
 d->mExplored = BoBinCoder::toBinary(text);
 d->mExploredCount = 0;
 for (unsigned int i = 0; i < d->mExplored.size(); i++) {
	if (d->mExplored.at(i)) {
		d->mExploredCount++;
	}
 }
 boDebug() << k_funcinfo << "decoded: " << d->mExplored.size() << endl;
 if (d->mMap) {
	if (d->mMap->width() * d->mMap->height() != d->mExplored.size()) {
		boError() << k_funcinfo << "exploration fog loaded: " << d->mExplored.size() << " map size: " << d->mMap->width() << "x" << d->mMap->height() << endl;
		return false;
	}
 }

 QDomElement fogelement = root.namedItem(QString::fromLatin1("Fogged")).toElement();
 if (fogelement.isNull()) {
	boError() << k_funcinfo << "No Fogged tag found" << endl;
	return false;
 }
 QString fogtext = fogelement.text();
 if (fogtext.isEmpty()) {
	boError() << k_funcinfo << "no content for Fogged tag found" << endl;
	return false;
 }
 boDebug() << k_funcinfo << "decoding" << endl;
 d->mFoggedRef = new unsigned short int[d->mMap->width() * d->mMap->height()];
 int cellCount = d->mMap->width() * d->mMap->height();
 BoBinCoder::toCharArray(fogtext, (char*)d->mFoggedRef, cellCount * sizeof(unsigned short int));

 d->mUnfoggedCount = 0;
 for (int i = 0; i < cellCount; i++) {
	if (d->mFoggedRef[i] >= 1) {
		d->mUnfoggedCount++;
	}
 }
 return true;
}

bool Player::saveAmmunition(QDomElement& root) const
{
 QDomElement ammunition = root.namedItem("AmmunitionPool").toElement();
 if (!ammunition.isNull()) {
	root.removeChild(ammunition);
 }
 QDomDocument doc = root.ownerDocument();
 ammunition = doc.createElement("AmmunitionPool");
 root.appendChild(ammunition);

 for (QMap<QString, unsigned long int>::const_iterator it = d->mAmmunition.begin(); it != d->mAmmunition.end(); ++it) {
	QString type = it.key();
	unsigned long int value = it.data();
	QDomElement ammo = doc.createElement("Ammunition");
	ammo.setAttribute("Type", type);
	ammo.setAttribute("Value", value);
	ammunition.appendChild(ammo);
 }
 return true;
}

bool Player::loadAmmunition(const QDomElement& root)
{
 d->mAmmunition.clear();
 d->mAmmunition.insert("Generic", 0);
 QDomElement ammunition = root.namedItem("AmmunitionPool").toElement();
 if (ammunition.isNull()) {
	// nothing to do
	return true;
 }
 for (QDomNode n = ammunition.firstChild(); !n.isNull(); n = n.nextSibling()) {
	QDomElement e = n.toElement();
	if (e.isNull()) {
		continue;
	}
	if (e.tagName() != "Ammunition") {
		continue;
	}
	QString type = e.attribute("Type");
	if (type.isEmpty()) {
		boError() << k_funcinfo << "invalid Type attribute" << endl;
		return false;
	}
	bool ok;
	unsigned long int value = e.attribute("Value").toULong(&ok);
	if (!ok) {
		boError() << k_funcinfo << "invalid Value attribute for type=" << type << endl;
		return false;
	}
	d->mAmmunition.insert(type, value);
 }
 return true;
}

void Player::writeGameLog(QTextStream& log)
{
 log << "Player: " << bosonId() << endl;
 log << minerals() << " " << oil() << " " << ammunition("Generic") << endl;
 log << mobilesCount() << " " << facilitiesCount() << endl;

 QPtrListIterator<Unit> it(d->mUnits);
 Unit* u;
 while (it.current()) {
	u = it.current();
	log << "Unit: " << u->id() << "  " << u->x() << " " << u->y() << " " << u->z() << "  " << u->rotation() <<
			"  " << u->speed() << "  " << u->health() << " " << u->advanceWork() << endl;
	++it;
 }
 log << endl;
}

unsigned int Player::unfoggedCells() const
{
 return d->mUnfoggedCount;
}

unsigned int Player::exploredCells() const
{
 return d->mExploredCount;
}

void Player::clearUpgrades()
{
 d->mUpgradesCollection.clearUpgrades();
}

void Player::addUpgrade(const UpgradeProperties* upgrade)
{
 BO_CHECK_NULL_RET(upgrade);
 d->mUpgradesCollection.addUpgrade(upgrade);

 for (QPtrListIterator<Unit> it(d->mUnits); it.current(); ++it) {
	if (upgrade->appliesTo(it.current())) {
		it.current()->addUpgrade(upgrade);
	}
 }

 const QIntDict<UnitProperties>* units = speciesTheme()->allUnitsNonConst();
 for (QIntDictIterator<UnitProperties> it(*units); it.current(); ++it) {
	if (upgrade->appliesTo(it.current())) {
		it.current()->addUpgrade(upgrade);
	}
 }
}

void Player::removeUpgrade(unsigned long int id)
{
 removeUpgrade(d->mUpgradesCollection.findUpgrade(id));
}

void Player::removeUpgrade(const UpgradeProperties* upgrade)
{
 d->mUpgradesCollection.removeUpgrade(upgrade);

 for (QPtrListIterator<Unit> it(d->mUnits); it.current(); ++it) {
	it.current()->removeUpgrade(upgrade);
 }

 const QIntDict<UnitProperties>* units = speciesTheme()->allUnitsNonConst();
 for (QIntDictIterator<UnitProperties> it(*units); it.current(); ++it) {
	it.current()->removeUpgrade(upgrade);
 }
}

const QValueList<const UpgradeProperties*>* Player::upgrades() const
{
 return d->mUpgradesCollection.upgrades();
}

const QValueList<const Unit*>* Player::radarUnits() const
{
 return &d->mRadarUnits;
}

void Player::addRadar(Unit* u)
{
 d->mRadarUnits.append(u);
}

void Player::removeRadar(Unit* u)
{
 d->mRadarUnits.remove(u);
}

void Player::networkTransmission(QDataStream& stream, int msgid, Q_UINT32 sender)
{
 // TODO: maybe do this in KPlayer ?
 if (sender == 0) {
	boError() << k_funcinfo << "invalid sender 0 for msgid=" << msgid << endl;
	return;
 }
 if (sender != KGameMessage::rawGameId(kgameId())) {
	boError() << k_funcinfo
			<< "client " << sender
			<< " tried to control player on client "
			<< KGameMessage::rawGameId(kgameId())
			<< endl;
	return;
 }
 KPlayer::networkTransmission(stream, msgid, sender);
}


