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
#include "unitpropertyhandler.h"
#include "bosonpropertyxml.h"
#include "bodebug.h"

#include <kgame/kgame.h>
#include <kgame/kgamemessage.h>

#include <qbitarray.h>
#include <qdom.h>
#include <qtextstream.h>

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

 KGamePropertyBase* propName = dataHandler()->find(KGamePropertyBase::IdName);
 if (propName) {
	propName->setPolicy(KGamePropertyBase::PolicyClean);
 } else {
	boError() << k_funcinfo << "can't find name property" << endl;
 }
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
 while (it.current() && !it.current()->dataHandler()->processMessage(stream, msgid + KGamePropertyBase::IdUser, issender)) {
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
	case BosonMessage::IdChat:
		break;
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

 bool emitSignalUnitChanged = false;
 switch (prop->id()) {
	case UnitBase::IdHealth:
	case UnitBase::IdArmor:
	case UnitBase::IdShields:
	case UnitBase::IdSightRange:
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
 UnitPropertyHandler* p = (UnitPropertyHandler*)sender();
 if (!p->unit()) {
	boError() << k_funcinfo << "NULL parent unit for property handler" << endl;
	boDebug() << "player=" << id() << ",propId=" << prop->id() << endl;
	return;
 }
 Unit* unit = (Unit*)p->unit();
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

bool Player::loadUnits(QDataStream& stream)
{
 // Load units
 Q_UINT32 unitCount;
 stream >> unitCount;
 for (unsigned int i = 0; i < unitCount; i++) {
	unsigned long int type;
	unsigned long int id;
	int dataHandlerID;
	stream >> type;
	stream >> id;
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
			SLOT(slotUnitPropertyChanged(KGamePropertyBase*)));
	unit->setId(id);

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
 UpgradeProperties* tech = speciesTheme()->technologyList().find(id);
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
 QIntDictIterator<UpgradeProperties> it(speciesTheme()->technologyList());
 while (it.current()) {
	if (((unsigned long int)(it.currentKey()) == id) && (it.current()->isResearched())) {
		boError() << k_funcinfo << "upgrade " << it.current() << " already researched!" << endl;
		return;
	}
	++it;
 }

 UpgradeProperties* prop = speciesTheme()->technology(id);
 prop->setResearched(true);
 prop->apply(this);

 ((Boson*)game())->slotUpdateProductionOptions();
 // TODO: also update unit view
}

bool Player::advanceFlag() const
{
 return ((Boson*)game())->advanceFlag();
}

bool Player::saveAsXML(QDomElement& root)
{
 // AB: probably we'll never use this... (KGame should take care of the id)
 root.setAttribute(QString::fromLatin1("Id"), id());

 root.setAttribute(QString::fromLatin1("NetworkPriority"), networkPriority());

 QDomDocument doc = root.ownerDocument();

 // store the dataHandler()
 BosonCustomPropertyXML propertyXML;
 QDomElement handler = doc.createElement(QString::fromLatin1("DataHandler"));
 if (!propertyXML.saveAsXML(handler, dataHandler())) {
	boError() << k_funcinfo << "Unable to save datahandler of player " << id() << endl;
	return false;
 }
 root.appendChild(handler);

 // save units
 QDomElement units = doc.createElement(QString::fromLatin1("Units"));
 QPtrListIterator<Unit> it(d->mUnits);
 for (; it.current(); ++it) {
	Unit* u = it.current();
	QDomElement unit = doc.createElement(QString::fromLatin1("Unit"));
	if (!u->saveAsXML(unit)) {
		boError() << k_funcinfo << "Could not save unit " << u->id() << endl;
		continue;
	}
	units.appendChild(unit);
 }
 root.appendChild(units);

 // Save speciestheme
 if (speciesTheme()) {
	root.setAttribute(QString::fromLatin1("SpeciesTheme"), speciesTheme()->identifier());
	root.setAttribute(QString::fromLatin1("TeamColor"), (unsigned int)speciesTheme()->teamColor().rgb());
 } else {
	// the attributes won't be there
	boWarning() << k_funcinfo << "NULL speciestheme while saving into XML" << endl;
 }

 // Save unitpropID
 root.setAttribute(QString::fromLatin1("UnitPropId"), (unsigned int)d->mUnitPropID);

 // Save fog
 saveFogOfWar(root);

 // Save statistics
 QDomElement statistics = doc.createElement(QString::fromLatin1("Statistics"));
 d->mStatistics->save(statistics);
 root.appendChild(statistics);

 return true;
}

bool Player::loadFromXML(const QDomElement& root)
{
 boDebug(260) << k_funcinfo << endl;
 // this does NOT load the units!

 bool ok = false;
 int id = root.attribute(QString::fromLatin1("Id")).toInt(&ok);
 if (!ok) {
	boError(260) << k_funcinfo << "Id was no valid number" << endl;
	return false;
 }
 Q_UNUSED(id);
// setId(id); // AB: KGame should take care of this. we should not need this. (remember that we depend on KGame::d->mUniquePlayerNumber to be in sync!)
 int networkPriority = root.attribute(QString::fromLatin1("NetworkPriority")).toInt(&ok);
 if (!ok) {
	boError(260) << k_funcinfo << "NetworkPriority was no valid number" << endl;
	return false;
 }
 setNetworkPriority(networkPriority);

 boDebug(260) << k_funcinfo << "load data handler" << endl;
 // load the data handler
 BosonCustomPropertyXML propertyXML;
 QDomElement handler = root.namedItem(QString::fromLatin1("DataHandler")).toElement();
 if (!propertyXML.loadFromXML(handler, dataHandler())) {
	boError(260) << k_funcinfo << "unable to load player data handler (player=" << this->id() << ")" << endl;
	return false;
 }

 // note: this does NOT load the units.


 boDebug(260) << k_funcinfo << "loading speciesTheme" << endl;
 QString speciesIdentifier;
 QColor color;
 if (root.hasAttribute(QString::fromLatin1("SpeciesTheme"))) {
	speciesIdentifier = root.attribute(QString::fromLatin1("SpeciesTheme"));
 }
 if (root.hasAttribute(QString::fromLatin1("TeamColor"))) {
	unsigned int c = root.attribute(QString::fromLatin1("TeamColor")).toUInt(&ok);
	if (!ok) {
		boError(260) << k_funcinfo << "Invalid TeamColor value" << endl;
	} else {
		color.setRgb(c);
	}
 }
 if (speciesIdentifier.isNull()) {
	// migh be valid if we ever use this for network loading, too!
	boError(260) << k_funcinfo << "No SpeciesTheme" << endl;
	return false;
 } else {
	boDebug(260) << k_funcinfo << "speciesTheme: " << speciesIdentifier << endl;
	// TODO: check whether this theme actually exists and could be loaded
	loadTheme(SpeciesTheme::speciesDirectory(speciesIdentifier), color);
 }

 d->mUnitPropID = root.attribute(QString::fromLatin1("UnitPropId")).toUInt(&ok);
 if (!ok) {
	boError(260) << k_funcinfo << "Invalid UnitPropId value" << endl;
	return false;
 }


 // Load fog
 boDebug(260) << k_funcinfo << "loading fow" << endl;
 loadFogOfWar(root);

 // Load statistics
 QDomElement statistics = root.namedItem(QString::fromLatin1("Statistics")).toElement();
 if (statistics.isNull()) {
	boWarning() << k_funcinfo << "No Statistics tag" << endl;
	// dont return
 } else {
	d->mStatistics->load(statistics);
 }

 boDebug(260) << k_funcinfo << "done" << endl;
 return true;
}

bool Player::loadUnitsFromXML(const QDomElement& root)
{
 if (root.isNull()) {
	boError(260) << k_funcinfo << "NULL root node" << endl;
	return false;
 }
 QDomElement units = root.namedItem(QString::fromLatin1("Units")).toElement();
 if (units.isNull()) {
	boWarning(260) << k_funcinfo << "no units for player " << id() << endl;
	return true;
 }
 QDomNodeList list = units.elementsByTagName(QString::fromLatin1("Unit"));
 if (list.count() == 0) {
	boWarning(260) << k_funcinfo << "no units for player " << id() << endl;
	return true;
 }
 for (unsigned int i = 0; i < list.count(); i++) {
	QDomElement e = list.item(i).toElement();
	if (e.isNull()) {
		boError(260) << k_funcinfo << i << " is not an element" << endl;
		return false;
	}
	if (!e.hasAttribute(QString::fromLatin1("UnitType"))) {
		boError(260) << k_funcinfo << "missing attribute: UnitType for Unit " << i << endl;
		continue;
	}
	if (!e.hasAttribute(QString::fromLatin1("Id"))) {
		boError(260) << k_funcinfo << "missing attribute: Id for Unit " << i << endl;
		continue;
	}
	if (!e.hasAttribute(QString::fromLatin1("DataHandlerId"))) {
		boError(260) << k_funcinfo << "missing attribute: DataHandlerId for Unit " << i << endl;
		continue;
	}
	bool ok = false;
	unsigned long int type;
	unsigned long int id;
	int dataHandlerId;
	type = e.attribute(QString::fromLatin1("UnitType")).toULong(&ok);
	if (!ok) {
		boError(260) << k_funcinfo << "Invalid UnitType number for Unit " << i << endl;
		continue;
	}
	id = e.attribute(QString::fromLatin1("Id")).toULong(&ok);
	if (!ok) {
		boError(260) << k_funcinfo << "Invalid Id number for Unit " << i << endl;
		continue;
	}
	dataHandlerId = e.attribute(QString::fromLatin1("DataHandlerId")).toInt(&ok);
	if (!ok) {
		boError(260) << k_funcinfo << "Invalid DataHandlerId number for Unit " << i << endl;
		continue;
	}

	// Create unit with Boson
	Unit* unit = ((Boson*)game())->loadUnit(type, this);

	// Set additional properties
	d->mUnits.append(unit);
	unit->dataHandler()->registerHandler(dataHandlerId, this,
			SLOT(sendProperty(int, QDataStream&, bool*)),
			SLOT(slotUnitPropertyChanged(KGamePropertyBase*)));
	unit->setId(id);

	// Call unit's loading methods
	if (!unit->loadFromXML(e)) {
		boWarning(260) << k_funcinfo << "Could not load unit " << id << " correctly" << endl;
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

void Player::saveFogOfWar(QDomElement& root) const
{
 // AB: I've tried several ways of doing this but couldn't find the solution. so
 // now we simply stream every single bit as a complete byte. not good, but
 // works
 // also note that i don't want to do the bit magic myself, as i want to avoid
 // this trouble with strings/xml and e.g. \0
 QDomDocument doc = root.ownerDocument();
 QDomElement fog = doc.createElement(QString::fromLatin1("Fogged"));
 QString text;
 QTextOStream s(&text);
 s << (Q_UINT32)d->mFogged.size();
 s << ' ';
 for (unsigned int i = 0; i < d->mFogged.size(); i++) {
	s << (char)(d->mFogged.testBit(i) ? '1' : '0');
 }
 QDomCDATASection fow = doc.createCDATASection(text);
 fog.appendChild(fow);
 root.appendChild(fog);
}

void Player::loadFogOfWar(const QDomElement& root)
{
 QDomElement element = root.namedItem(QString::fromLatin1("Fogged")).toElement();
 if (element.isNull()) {
	boError() << k_funcinfo << "No Fogged tag found" << endl;
	d->mFogged.fill(true);
	return;
 }
 QString text = element.text();
 if (text.isEmpty()) {
	boError() << k_funcinfo << "no content for Fogged tag found" << endl;
	d->mFogged.fill(true);
	return;
 }
 QTextIStream s(&text);
 Q_UINT32 size;
 s >> size;
 char space;
 s >> space;
 d->mFogged.resize(size);
 for (unsigned int i = 0; i < size; i++) {
	char bit;
	s >> bit;
	d->mFogged.setBit(i, bit == '1');
 }
}

