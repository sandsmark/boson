
#include "player.h"

#include "speciestheme.h"
#include "unit.h"
#include "bosonmessage.h"

#include <kgame/kgamepropertyhandler.h>
#include <kgame/kgame.h>
#include <kgame/kgamemessage.h>

#include <qcanvas.h>

#include "player.moc"

class PlayerPrivate
{
public:
	PlayerPrivate()
	{
		mUnitPropID = 0;
	
		mSpecies = 0;
	}

	SpeciesTheme* mSpecies;

	QPtrList<Unit> mUnits;

	int mUnitPropID; // used for KGamePropertyHandler
};

Player::Player() : KPlayer()
{
 d = new PlayerPrivate;
 d->mUnits.setAutoDelete(true);
 setAsyncInput(true);
 connect(this, SIGNAL(signalNetworkData(int, const QByteArray&, Q_UINT32, KPlayer*)),
		this, SLOT(slotNetworkData(int, const QByteArray&, Q_UINT32, KPlayer*)));
}

Player::~Player()
{
 kdDebug() << k_funcinfo << endl;
 d->mUnits.clear();
// kdDebug() << "clear handler" << endl;
 dataHandler()->clear();
// kdDebug() << "delete theme" << endl;
 if (d->mSpecies) {
	delete d->mSpecies;
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
		case UnitBase::IdType: //? Can this change at all?
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
	case BosonMessage::IdStopMoving:
	{
		unsigned long int id;
		double x;
		double y;
		double z;

		stream >> id;
		stream >> x;
		stream >> y;
		stream >> z;

		Unit* unit = findUnit(id);
		if (!unit) {
			kdError() << "Unit " << id << " not found" << endl;
			break;
		}
		if (unit->work() != UnitBase::WorkNone) {
			kdWarning() << "work != WorkNone" << endl;
		}
		unit->stopMoving(false); // do not send again
		unit->setX(x);
		unit->setY(y);
		unit->setZ(z);
		break;
	}
	default:
		kdWarning() << "Unknown message " << msgid << endl;
		break;
 }
}


void Player::loadTheme(const QString& species, const QRgb& teamColor)
{
 if (d->mSpecies) {
	delete d->mSpecies;
 }
 d->mSpecies = new SpeciesTheme(species, teamColor);
}

QCanvasPixmapArray* Player::pixmapArray(int unitType) const
{
 if (!d->mSpecies) {
	kdError() << k_funcinfo << ": NULL theme" << endl;
	return 0;
 }
 return d->mSpecies->pixmapArray(unitType);
}


void Player::addUnit(Unit* unit)
{
 d->mUnitPropID++;// used for ID of KGamePropertyHandler
 d->mUnits.append(unit);
 unit->setOwner(this); // already done in c'tor of Unit
 unit->dataHandler()->registerHandler(BosonMessage::UnitPropertyHandler + d->mUnitPropID, this,
		SLOT(sendProperty(int, QDataStream&, bool*)),
		SLOT(emitSignal(KGamePropertyBase*)));
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

SpeciesTheme* Player::speciesTheme() const
{
 return d->mSpecies;
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
//	KGamePropertyHandler* h = (KGamePropertyHandler*)sender();
	kdError() << k_funcinfo << ": NULL unit" << endl;
//	kdDebug() << h->id() << endl;
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
	case Unit::IdFix_ConstructionDelay:
		// these IDs are not to be displayed in BosonUnitView.
		break;
	case UnitBase::IdHealth:
	case UnitBase::IdArmor:
	case UnitBase::IdShields:
	case UnitBase::IdSpeed:
	case UnitBase::IdType: // FIXME: can this change at all? (currently not)
	case UnitBase::IdCost:
	case UnitBase::IdRange:
	case Unit::IdReloadState:
		// update BosonUnitView if the unit is selected.
		// not all of these IDs are displayed there. But perhaps they
		// will one day.
//		kdDebug() << "emit unit changed" << endl;
		emit signalUnitChanged(unit);
		break;
	default:
		kdDebug() << "Unknown property ID " << prop->id() << endl;
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
 if (!KPlayer::save(stream)) {
	kdError() << "Couldn't save player" << endl;
	return false;
 }
// kdDebug() << k_funcinfo < endl;
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
// Player::load() is unused!

 if (!KPlayer::load(stream)) {
	kdError() << "Couldn't load player" << endl;
	return false;
 }
// kdDebug() << k_funcinfo < endl;
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

void Player::sendStopMoving(Unit* unit)
{
 if (!game()) {
	return;
 }
 QByteArray buffer;
 QDataStream stream(buffer, IO_WriteOnly);
 stream << (unsigned long int)unit->id();
 stream << (double)unit->x();
 stream << (double)unit->y();
 stream << (double)unit->z();
 game()->sendMessage(buffer, BosonMessage::IdStopMoving, id());
}

const UnitProperties* Player::unitProperties(int unitType) const
{
 if (!speciesTheme()) {
	kdError() << k_funcinfo << ": NULL theme" << endl;
	return 0;
 }
 return speciesTheme()->unitProperties(unitType);
}
