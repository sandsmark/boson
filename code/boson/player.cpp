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
#include "bosonmessage.h"
#include "bosonmap.h"

#include <kgame/kgamepropertyhandler.h>
#include <kgame/kgame.h>
#include <kgame/kgamemessage.h>

#include <qcanvas.h>
#include <qbitarray.h>

#include "player.moc"

class Player::PlayerPrivate
{
public:
	PlayerPrivate()
	{
		mUnitPropID = 0;
		mMap = 0;
	}

	QPtrList<Unit> mUnits;

	BosonMap* mMap; // just a pointer
	int mUnitPropID; // used for KGamePropertyHandler
	
	QBitArray mFogged; // TODO: use KGameProperty
	KGameProperty<unsigned long int> mMinerals;
	KGameProperty<unsigned long int> mOil;
};

Player::Player() : KPlayer()
{
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
 d->mMinerals.setLocal(0);
 d->mOil.setLocal(0);
}

Player::~Player()
{
 kdDebug() << k_funcinfo << endl;
 d->mUnits.clear();
// kdDebug() << "clear handler" << endl;
 dataHandler()->clear();
// kdDebug() << "delete theme" << endl;
 if (mSpecies) {
	delete mSpecies;
 }
 delete d;
// kdDebug() << "~Player done" << endl;
}

void Player::slotNetworkData(int msgid, const QByteArray& buffer, Q_UINT32 sender, KPlayer*)
{
// note: this is kind of obsolete. IdStopMoving is the only ID still used here -
// and that is rather for debugging. It is not needed. (Only historical
// reasons).
// The entire KGameProperty stuff is also unused as properties of a unit all
// have PolicyLocal. 

 QDataStream stream(buffer, IO_ReadOnly);
 bool issender = true;
 if (game()) {
	issender = sender == game()->gameId();
 }
// kdDebug() << "slotNetworkData" << endl;
// first check if the message is a property of a unit
 QPtrListIterator<Unit> it(d->mUnits);
 while(it.current() && !it.current()->dataHandler()->processMessage(stream, msgid, issender)) {
	++it;
 }
 Unit* unit = it.current();

 if (unit) { // this was a unit property
	// note this part is completely obsolete!
	QDataStream stream2(buffer, IO_ReadOnly);
	int propertyId;
	KGameMessage::extractPropertyHeader(stream2, propertyId);
	kdDebug() << "unit property" << endl;

	switch (propertyId) { // arghh - ID of datahandler()
		case UnitBase::IdWork:
		case Unit::IdDirection:
		case Unit::IdWaypoints:
			break;
		case UnitBase::IdSpeed:
		case UnitBase::IdHealth:
		case UnitBase::IdArmor:
		case UnitBase::IdShields:
		case UnitBase::IdCost: // can change during the game, too!!
			// currently unused - nevertheless display the value if
			// selected
			kdDebug() << "emit unit changed" << endl;
			emit signalUnitChanged(unit);
			break;
		default:
			break;
	}
	return;
 }

 // this wasn't a unit property but a normal message
 switch (msgid) {
	// none here currently
	default:
		kdWarning() << "Unknown message " << msgid << endl;
		break;
 }
}


void Player::loadTheme(const QString& species, const QRgb& teamColor)
{
 if (mSpecies) {
	delete mSpecies;
 }
 mSpecies = new SpeciesTheme(species, teamColor);
}

QCanvasPixmapArray* Player::pixmapArray(int unitType) const
{
 if (!speciesTheme()) {
	kdError() << k_funcinfo << ": NULL theme" << endl;
	return 0;
 }
 return speciesTheme()->pixmapArray(unitType);
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

}

void Player::unitDestroyed(Unit* unit)
{
 if (!unit) {
	kdError() << k_funcinfo << ": Cannot remove NULL unit" << endl;
	return;
 }
 d->mUnits.take(d->mUnits.findRef(unit));
}

void Player::slotUnitPropertyChanged(KGamePropertyBase* prop)
{
 if (!prop) {
	kdError() << k_funcinfo << ": NULL property" << endl;
	return;
 }

// VERY EVIL HACK!!!
 Unit* unit;
 bool found = false;
 for (unit = d->mUnits.first(); unit && !found; unit = d->mUnits.next()) {
	if (unit->dataHandler() == (KGamePropertyHandler*)sender()) {
		found = true;
//		kdDebug() << "found unit " << unit->id() << endl;
		break;
	}
 }
// (evil hack end)

 if (!unit) {
	kdError() << k_funcinfo << ": NULL unit" << endl;
	kdDebug() << "player=" << id() << ",propId=" << prop->id() << ",units=" << d->mUnits.count() << endl;
	return;
 }

 switch(prop->id()) {
	case UnitBase::IdWork:
	case UnitBase::IdId:
	case UnitBase::IdDamage:
	case UnitBase::IdReload:
	case Unit::IdDirection:
	case Unit::IdWaypoints:
	case Unit::IdFix_ConstructionState:
//	case Unit::IdFix_ConstructionDelay: // obsolete
	case Unit::IdFix_Productions:
	case Unit::IdFix_ProductionState:
		// these IDs are not to be displayed in BosonUnitView.
		break;
	case UnitBase::IdHealth:
	case UnitBase::IdArmor:
	case UnitBase::IdShields:
	case UnitBase::IdSpeed:
	case UnitBase::IdCost:
	case UnitBase::IdRange:
	case UnitBase::IdSightRange:
	case Unit::IdReloadState:
		// update BosonUnitView if the unit is selected.
		// not all of these IDs are displayed there. But perhaps they
		// will one day.
//		kdDebug() << "emit unit changed" << endl;
		emit signalUnitChanged(unit);
		break;
	default:
		kdDebug() << k_funcinfo << "Unknown property ID " << prop->id() << endl;
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
 kdDebug() << k_funcinfo << endl;
 if (!KPlayer::save(stream)) {
	kdError() << "Couldn't save player" << endl;
	return false;
 }
 if (speciesTheme()) {
	stream << speciesTheme()->identifier();
	stream << speciesTheme()->teamColor();
 } else {
	 stream << QString::null;
	 stream << (QRgb)0;
 }

// the stuff below this should (!) be unused.
 Q_UINT32 unitCount = d->mUnits.count();
 stream << unitCount;
 for (unsigned int i = 0; i < unitCount; i++) {
//	kdDebug() << "save unit " << i << endl;
	Unit* unit = d->mUnits.at(i);
	// we need the type first!
	stream << (int)unit->type();
	stream << (unsigned long int)unit->id();
//	stream << (double)unit->x();
//	stream << (double)unit->y();

	unit->save(stream);
 }
 return true;
}

bool Player::load(QDataStream& stream)
{
// kdDebug() << k_funcinfo < endl;
 if (!KPlayer::load(stream)) {
	kdError() << "Couldn't load player" << endl;
	return false;
 }
 QString themeIdentifier;
 QRgb teamColor;
 stream >> themeIdentifier;
 stream >> teamColor;
 if (themeIdentifier != QString::null) {
	loadTheme(SpeciesTheme::speciesDirectory(themeIdentifier), teamColor);
 }

 Q_UINT32 unitCount;
 stream >> unitCount;
 //perhaps:
 d->mUnitPropID = 0;
 for (unsigned int i = 0; i < unitCount; i++) {
//	kdDebug() << "load unit " << i << endl;
	int type;
	unsigned long int id;
	stream >> type;
	stream >> id;

	emit signalLoadUnit(type, id, this);
	Unit* unit = findUnit(id);
	unit->load(stream);
 }
 return true;
}

QPtrList<Unit> Player::allUnits() const
{
 return d->mUnits;
}

const UnitProperties* Player::unitProperties(int unitType) const
{
 if (!speciesTheme()) {
	kdError() << k_funcinfo << ": NULL theme" << endl;
	return 0;
 }
 return speciesTheme()->unitProperties(unitType);
}

void Player::initMap(BosonMap* map)
{
 d->mMap = map;
 if (!map) {
	d->mFogged.resize(0);
	return;
 }
 d->mFogged.fill(true, map->width() * map->height());
}

void Player::fog(int x, int y)
{
 if (!d->mMap) {
	return;
 }
//kdDebug() << k_funcinfo << x << "," << y << endl;
 d->mFogged.setBit(x + d->mMap->width() * y);
 // emit signal (actual fog on map + minimap)
 // TODO: any way to emit only for the local player?
 emit signalFog(x, y);
//kdDebug() << k_funcinfo << "done " << endl;
}

void Player::unfog(int x, int y)
{
 if (!d->mMap) {
	return;
 }
//kdDebug() << k_funcinfo << x << "," << y << endl;
 d->mFogged.clearBit(x + d->mMap->width() * y);
 // emit signal (actual fog on map + minimap)
 // TODO: any way to emit only for the local player?
 emit signalUnfog(x, y);
//kdDebug() << k_funcinfo << "done " << endl;
}

bool Player::isFogged(int x, int y) const
{
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

