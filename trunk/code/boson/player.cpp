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

#include "playerio.h"
#include "speciestheme.h"
#include "unit.h"
#include "unitproperties.h"
#include "bosonmessage.h"
#include "bosonmap.h"
#include "bosonstatistics.h"
#include "boson.h"
#include "upgradeproperties.h"
#include "items/bosonitempropertyhandler.h"
#include "bosonpropertyxml.h"
#include "bosonprofiling.h"
#include "bodebug.h"
#include "bobincoder.h"

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

		mPlayerIO = 0;
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

	PlayerIO* mPlayerIO;
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

void Player::addUnit(Unit* unit, int dataHandlerId)
{
 d->mUnits.append(unit);
 if (dataHandlerId == -1) {
	dataHandlerId = BosonMessage::UnitPropertyHandler + d->mUnitPropID;
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
 if (unit->unitProperties()->supportMiniMap()) {
	if (!hasMiniMap()) {
		speciesTheme()->playSound(SoundReportMinimapDeactivated);
		emit signalShowMiniMap(false);
	}
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
 BosonItemPropertyHandler* p = (BosonItemPropertyHandler*)sender();
 if (!p->item()) {
	boError() << k_funcinfo << "NULL parent item for property handler" << endl;
	boDebug() << "player=" << id() << ",propId=" << prop->id() << endl;
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
		if (!it.current()->isFacility()) {
			return true;
		} else {
			Facility* f = (Facility*)it.current();
			if (f->isConstructionComplete()) {
				return true;
			}
		}
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
 BosonProfiler profiler(BosonProfiling::SavePlayerToXML);

 // note: we need to save the index in the list, not the actual id()
 root.setAttribute(QString::fromLatin1("Id"), game()->playerList()->findRef(this));

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

 boDebug(260) << k_funcinfo << "load data handler" << endl;
 // load the data handler
 BosonCustomPropertyXML propertyXML;
 QDomElement handler = root.namedItem(QString::fromLatin1("DataHandler")).toElement();
 if (!propertyXML.loadFromXML(handler, dataHandler())) {
	boError(260) << k_funcinfo << "unable to load player data handler (player=" << this->id() << ")" << endl;
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
 QDomElement fog = doc.createElement(QString::fromLatin1("Fogged"));
 fog.appendChild(doc.createTextNode(BoBinCoder::toCoded(d->mFogged)));
 root.appendChild(fog);
 return true;
}

bool Player::loadFogOfWar(const QDomElement& root)
{
 QDomElement element = root.namedItem(QString::fromLatin1("Fogged")).toElement();
 if (element.isNull()) {
	boError() << k_funcinfo << "No Fogged tag found" << endl;
	d->mFogged.fill(true);
	return false;
 }
 QString text = element.text();
 if (text.isEmpty()) {
	boError() << k_funcinfo << "no content for Fogged tag found" << endl;
	d->mFogged.fill(true);
	return false;
 }
 boDebug() << k_funcinfo << "decoding" << endl;
 d->mFogged = BoBinCoder::toBinary(text);
 boDebug() << k_funcinfo << "decoded: " << d->mFogged.size() << endl;
 if (d->mMap) {
	if (d->mMap->width() * d->mMap->height() != d->mFogged.size()) {
		boError() << k_funcinfo << "fog of war loaded: " << d->mFogged.size() << " map size: " << d->mMap->width() << "x" << d->mMap->height() << endl;
		return false;
	}
 }
 return true;
}

void Player::writeGameLog(QTextStream& log)
{
 log << "Player: " << id() << endl;
 log << minerals() << " " << oil() << endl;
 log << mobilesCount() << " " << facilitiesCount() << endl;

 QPtrListIterator<Unit> it(d->mUnits);
 Unit* u;
 while (it.current()) {
	u = it.current();
	log << "Unit: " << u->id() << "  " << u->x() << " " << u->y() << " " << u->z() << "  " << u->rotation() <<
			"  " << u->speed() << "  " << u->health() << "  " << u->work() << " " << u->advanceWork() << endl;
	++it;
 }
 log << endl;
}

