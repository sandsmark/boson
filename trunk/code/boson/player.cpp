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

#include "player.h"

#include "speciestheme.h"
#include "unit.h"
#include "unitproperties.h"
#include "bosonmessage.h"
#include "bosonmap.h"
#include "bosonstatistics.h"
#include "boson.h"
#include "upgradeproperties.h"
#include "bodebug.h"

#include <kgame/kgamepropertyhandler.h>
#include <kgame/kgame.h>
#include <kgame/kgamemessage.h>

#include <qbitarray.h>

#include "player.moc"

class Player::PlayerPrivate
{
public:
	PlayerPrivate()
	{
		mUnitPropID = 0;
		mMap = 0;

		mStatistics = 0;
	}

	QPtrList<Unit> mUnits;

	BosonMap* mMap; // just a pointer
	int mUnitPropID; // used for KGamePropertyHandler

	QBitArray mFogged; // TODO: use KGameProperty
	KGameProperty<unsigned long int> mMinerals;
	KGameProperty<unsigned long int> mOil;

	BosonStatistics* mStatistics;

	int mMobilesCount;
	int mFacilitiesCount;
};

Player::Player() : KPlayer()
{
 boDebug() << k_funcinfo << endl;
 mSpecies = 0;
 d = new PlayerPrivate;
 d->mUnits.setAutoDelete(true);
 setAsyncInput(true);
 connect(this, SIGNAL(signalNetworkData(int, const QByteArray&, Q_UINT32, KPlayer*)),
		this, SLOT(slotNetworkData(int, const QByteArray&, Q_UINT32, KPlayer*)));
// TODO d->mFogged.registerData() or something like this
 d->mMinerals.registerData(IdMinerals, dataHandler(),
		KGamePropertyBase::PolicyLocal, "MineralCost");
 d->mOil.registerData(IdOil, dataHandler(),
		KGamePropertyBase::PolicyLocal, "OilCost");

 quitGame(); // this will reset some variables
}

Player::~Player()
{
 boDebug() << k_funcinfo << endl;
 quitGame(true);
 dataHandler()->clear(); // this must not be in quitGame()
 delete mSpecies;
 delete d;
 boDebug() << k_funcinfo << "done" << endl;
}

void Player::quitGame(bool destruct)
{
 boDebug() << k_funcinfo << endl;
 d->mMobilesCount = 0;
 d->mFacilitiesCount = 0;
 mOutOfGame = false;
 d->mMap = 0;

 boDebug() << k_funcinfo << "clearing units" << endl;
 d->mUnits.clear();
 boDebug() << k_funcinfo << "units cleared" << endl;
 delete d->mStatistics;
 d->mStatistics = 0;

 if (!destruct) {
	d->mStatistics = new BosonStatistics;
	d->mMinerals.setLocal(0);
	d->mOil.setLocal(0);
	d->mFogged.resize(0);
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
 while(it.current() && !it.current()->dataHandler()->processMessage(stream, msgid + KGamePropertyBase::IdUser, issender)) {
	++it;
 }
 Unit* unit = it.current();
 if (unit) { // this was a unit property
	// there are only very few messages which are handled here! try to avoid
	// this!
	QDataStream stream2(buffer, IO_ReadOnly);
	int propertyId = 0;
	KGameMessage::extractPropertyHeader(stream2, propertyId);
	switch (propertyId) {
		case KGamePropertyBase::IdCommand: // KGamePropertyList or KGamePropertyArray
		{
			int cmd = 0;
			KGameMessage::extractPropertyCommand(stream2, propertyId, cmd);
			switch (propertyId) {
				case Unit::IdWaypoints:
					// waypoints have PolicyClean, so they 
					// send a message which is handled here.
					break;
				default:
					break;
			}
			break;
		}
		case Unit::IdWork:
		//...
		default:
			// completely unused
			break;
	}
	return;
 }

 // this wasn't a unit property but a normal message
 switch (msgid) {
	// nothing done here currently
	default:
		boWarning() << k_funcinfo << "Unknown message " << msgid << endl;
		break;
 }
}


void Player::loadTheme(const QString& species, const QColor& teamColor)
{
 delete mSpecies;
 mSpecies = new SpeciesTheme(species, teamColor);
}

void Player::addUnit(Unit* unit)
{
 d->mUnitPropID++;// used for ID of KGamePropertyHandler
 d->mUnits.append(unit);
 unit->setOwner(this); // already done in c'tor of Unit
 unit->dataHandler()->registerHandler(BosonMessage::UnitPropertyHandler + d->mUnitPropID, this,
		SLOT(sendProperty(int, QDataStream&, bool*)),
		SIGNAL(signalUnitPropertyChanged(KGamePropertyBase*)));
 connect(unit->dataHandler(), SIGNAL(signalPropertyChanged(KGamePropertyBase*)), 
		this, SLOT(slotUnitPropertyChanged(KGamePropertyBase*)));

 if (unit->isMobile()) {
	d->mMobilesCount++;
 }
 else {
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
 if (!hasMiniMap()) {
	speciesTheme()->playSound(SoundReportMinimapDeactivated);
	emit signalShowMiniMap(false);
 }
}

void Player::slotUnitPropertyChanged(KGamePropertyBase* prop)
{
 if (!prop) {
	boError() << k_funcinfo << "NULL property" << endl;
	return;
 }

// VERY EVIL HACK!!!
 Unit* unit;
 bool found = false;
 for (unit = d->mUnits.first(); unit && !found; unit = d->mUnits.next()) {
	if (unit->dataHandler() == (KGamePropertyHandler*)sender()) {
		found = true;
//		boDebug() << "found unit " << unit->id() << endl;
		break;
	}
 }
// (evil hack end)

 if (!unit) {
	boError() << k_funcinfo << "NULL unit" << endl;
	boDebug() << "player=" << id() << ",propId=" << prop->id() << ",units=" << d->mUnits.count() << endl;
	return;
 }

 switch(prop->id()) {
	case UnitBase::IdHealth:
	case UnitBase::IdArmor:
	case UnitBase::IdShields:
	case UnitBase::IdSightRange:
		// update BosonUnitView if the unit is selected.
		// not all of these IDs are displayed there. But perhaps they
		// will one day.
		emit signalUnitChanged(unit);
		break;
	default:
		// all other Unit IDs are not displayed in BosonUnitView so
		// there is no need to emit a signal for them.
		break;
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

 // Save fog
 stream << d->mFogged;

 // Save statistics
 d->mStatistics->save(stream);

 /// TODO: save researched technologies!!!

 return true;
}

bool Player::saveUnits(QDataStream& stream)
{
 // Save units
 Q_UINT32 unitCount = d->mUnits.count();
 stream << unitCount;
 for (unsigned int i = 0; i < unitCount; i++) {
	Unit* unit = d->mUnits.at(i);
	// we need the type first!
	stream << (unsigned long int)unit->type();
	stream << (unsigned long int)unit->id();
	stream << (double)unit->x();
	stream << (double)unit->y();

	stream << (int)unit->dataHandler()->id();

	if (!unit->save(stream)) {
		boError() << k_funcinfo << "Error while saving unit with id=" << unit->id() << endl;
		return false;
	}
 }
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

 // Load fog
 stream >> d->mFogged;

 // Save statistics
 d->mStatistics->load(stream);

 return true;
}

bool Player::loadUnits(QDataStream& stream)
{
 // Load units
 Q_UINT32 unitCount;
 stream >> unitCount;
 for (unsigned int i = 0; i < unitCount; i++) {
	unsigned long int type;
	unsigned long int id;
	double x, y;
	int dataHandlerID;
	stream >> type;
	stream >> id;
	stream >> x;
	stream >> y;
	stream >> dataHandlerID;

	// Create unit with Boson
	Unit* unit = ((Boson*)game())->loadUnit(type, this);

	if (!unit) {
		boError() << k_funcinfo << "NULL unit loaded" << endl;
		// we cannot load properly anymore.
		return false;
	}

	// Set additional properties
	d->mUnits.append(unit);
	unit->dataHandler()->registerHandler(dataHandlerID, this,
			SLOT(sendProperty(int, QDataStream&, bool*)),
			SIGNAL(signalUnitPropertyChanged(KGamePropertyBase*)));
	connect(unit->dataHandler(), SIGNAL(signalPropertyChanged(KGamePropertyBase*)),
			this, SLOT(slotUnitPropertyChanged(KGamePropertyBase*)));
	unit->setId(id);

	// Emit signal for canvas and minimap and BosonWidget
	emit signalUnitLoaded(unit, (int)x, (int)y);
	// Call unit's loading methods
	if (!unit->load(stream)) {
		boError() << k_funcinfo << "Error while loading unit with id=" << id << endl;
		return false;
	}

	// Increase unit count
	if (unit->isMobile()) {
		d->mMobilesCount++;
	} else {
		d->mFacilitiesCount++;
	}
 }
 return true;
}

QPtrList<Unit> Player::allUnits() const
{
 return d->mUnits;
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

void Player::initMap(BosonMap* map, bool fogged)
{
 d->mMap = map;
 if (!d->mMap) {
	d->mFogged.resize(0);
	return;
 }
 if (d->mFogged.size() != 0) {
	// the fog of war was initialized before. probably we are loading a
	// game.
	return;
 }
 d->mFogged.fill(fogged, d->mMap->width() * d->mMap->height());
}

void Player::fog(int x, int y)
{
 if (!d->mMap) {
	return;
 }
 if (x + d->mMap->width() * y >= d->mFogged.size()) {
	boError() << k_funcinfo << "x=" << x << ",y=" << y << " out of range ("
			<< d->mFogged.size() << ")" << endl;
	return;
 }
//boDebug() << k_funcinfo << x << "," << y << endl;
 d->mFogged.setBit(x + d->mMap->width() * y);
 // emit signal (actual fog on map + minimap)
 // TODO: any way to emit only for the local player?
 emit signalFog(x, y);
//boDebug() << k_funcinfo << "done " << endl;
}

void Player::unfog(int x, int y)
{
 if (!d->mMap) {
	return;
 }
//boDebug() << k_funcinfo << x << "," << y << endl;
 if (x + d->mMap->width() * y >= d->mFogged.size()) {
	boError() << k_funcinfo << "x=" << x << ",y=" << y << " out of range ("
			<< d->mFogged.size() << ")" << endl;
	return;
 }
 d->mFogged.clearBit(x + d->mMap->width() * y);
 // emit signal (actual fog on map + minimap)
 // TODO: any way to emit only for the local player?
 emit signalUnfog(x, y);
//boDebug() << k_funcinfo << "done " << endl;
}

bool Player::isFogged(int x, int y) const
{
 if (x + d->mMap->width() * y >= d->mFogged.size()) {
	boError() << k_funcinfo << "x=" << x << ",y=" << y << " out of range ("
			<< d->mFogged.size() << ")" << endl;
	return true;
 }
 return d->mFogged.at(x + d->mMap->width() * y);
}

unsigned long int Player::minerals() const
{
 return d->mMinerals;
}

unsigned long int Player::oil() const
{
 return d->mOil;
}

void Player::setOil(unsigned long int o)
{
 d->mOil = o;
}

void Player::setMinerals(unsigned long int m)
{
 d->mMinerals = m;
}

const QColor& Player::teamColor() const
{
 return speciesTheme()->teamColor();
}

bool Player::hasMiniMap() const
{
 QPtrListIterator<Unit> it(d->mUnits);
 while (it.current()) {
	if (it.current()->unitProperties()->supportMiniMap()) {
		return true;
	}
	++it;
 }
 return false;
}

void Player::facilityCompleted(Facility* fac)
{
 if (!fac) {
	boError() << k_funcinfo << "NULL facility" << endl;
	return;
 }
 if (fac->unitProperties()->supportMiniMap()) {
	speciesTheme()->playSound(SoundReportMinimapActivated);
	emit signalShowMiniMap(true);
 }
}

bool Player::checkOutOfGame()
{
 // TODO: make more clever.
 // e.g. currently every small unit, like harvesters, needs to be destroyed...
 mOutOfGame = (d->mUnits.count() == 0);
 return isOutOfGame();
}

BosonStatistics* Player::statistics() const
{
 return d->mStatistics;
}

bool Player::isEnemy(Player* p) const
{
 if (p == this) {
	return false;
 }
 return true;
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
		if (it.current()->isMobile() || ((Facility*)it.current())->isConstructionComplete()) {
			return true;
		}
	}
	++it;
 }
 return false;
}

bool Player::hasTechnology(unsigned long int id) const
{
 TechnologyProperties* tech = speciesTheme()->technologyList().find(id);
 if (!tech) {
	return false;
 }
 if (!tech->isResearched()) {
	return false;
 }
 return true;
}

void Player::technologyResearched(ProductionPlugin*, unsigned long int id)
{
 boDebug() << k_funcinfo << "id: " << id << endl;
 // Check if it isn't researched already
 QIntDictIterator<TechnologyProperties> it(speciesTheme()->technologyList());
 while (it.current()) {
	if (((unsigned long int)(it.currentKey()) == id) && (it.current()->isResearched())) {
		boError() << k_funcinfo << "Technology " << it.current() << " already researched!" << endl;
		return;
	}
	++it;
 }

 TechnologyProperties* prop = speciesTheme()->technology(id);
 prop->setResearched(true);
 prop->apply(this);

 // Iterate through upgrades, applying as needed
 QValueList<unsigned long int> unitIds = speciesTheme()->allFacilities();  // FIXME: this is dirty
 unitIds += speciesTheme()->allMobiles();
 QValueList<unsigned long int>::iterator pit;
 for (pit = unitIds.begin(); pit != unitIds.end(); pit++) {
	UpgradeProperties* upgrade;
	QPtrList<UpgradeProperties> ulist = speciesTheme()->nonConstUnitProperties(*pit)->unresearchedUpgrades();
	for (upgrade = ulist.first(); upgrade; upgrade = ulist.next()) {
		if (upgrade->canBeResearched(this)) {
			boDebug() << k_funcinfo << "    Applying upgrade for UP " << *pit << " with id " << upgrade->id() << endl;
			upgrade->setResearched(true);
			upgrade->apply(this);
			speciesTheme()->nonConstUnitProperties(*pit)->upgradeResearched( upgrade);
		}
	}
 }
 ((Boson*)game())->slotUpdateProductionOptions();
}

bool Player::advanceFlag() const
{
 return ((Boson*)game())->advanceFlag();
}

