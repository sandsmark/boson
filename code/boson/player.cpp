
#include "player.h"

#include "speciestheme.h"
#include "bosonmessage.h"
#include "visualunit.h"

#include <kgame/kgamepropertyhandler.h>
#include <kgame/kgame.h>
#include <kgame/kgamemessage.h>

#include <qcanvas.h>
//#include <qintdict.h>
//#include <qptrdict.h>

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

	QPtrList<VisualUnit> mUnits;

	int mUnitPropID; // used for KGamePropertyHandler
};

Player::Player() : KPlayer()
{
 d = new PlayerPrivate;
 d->mUnits.setAutoDelete(true);
 setAsyncInput(true);
// loadTheme("human");
 connect(this, SIGNAL(signalNetworkData(int, const QByteArray&, Q_UINT32, KPlayer*)),
		this, SLOT(slotNetworkData(int, const QByteArray&, Q_UINT32, KPlayer*)));
}

Player::~Player()
{
 kdDebug() << "~Player" << endl;
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
 QPtrListIterator<VisualUnit> it(d->mUnits);
 while(it.current() && !it.current()->dataHandler()->processMessage(stream, msgid, issender)) {
	++it;
 }
 VisualUnit* unit = it.current();

 if (unit) { // this was a unit property
	// note this part is completely obsolete!
	QDataStream stream2(buffer, IO_ReadOnly);
	int propertyId;
	KGameMessage::extractPropertyHeader(stream2, propertyId);
	kdDebug() << "unit property" << endl;

	switch (propertyId) { // arghh - ID of datahandler()
		case Unit::IdWork:
		case VisualUnit::IdDirection:
		case VisualUnit::IdWaypoints:
			break;
		case Unit::IdSpeed:
		case Unit::IdHealth:
		case Unit::IdArmor:
		case Unit::IdShields:
		case Unit::IdType: //? Can this change at all?
		case Unit::IdCost: // can change during the game, too!!
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

		VisualUnit* unit = findUnit(id);
		if (!unit) {
			kdError() << "Unit " << id << " not found" << endl;
			break;
		}
		if (unit->work() != Unit::WorkNone) {
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
	kdError() << "Player::pixmapArray(): NULL theme" << endl;
	return 0;
 }
 return d->mSpecies->pixmapArray(unitType);
}


void Player::addUnit(VisualUnit* unit)
{
 d->mUnitPropID++;// used for ID of KGamePropertyHandler
 d->mUnits.append(unit);
 unit->setOwner(this); // already done in c'tor of VisualUnit
 unit->dataHandler()->registerHandler(BosonMessage::UnitPropertyHandler + d->mUnitPropID, this,
		SLOT(sendProperty(int, QDataStream&, bool*)),
		SLOT(emitSignal(KGamePropertyBase*)));
 connect(unit->dataHandler(), SIGNAL(signalPropertyChanged(KGamePropertyBase*)), 
		this, SLOT(slotUnitPropertyChanged(KGamePropertyBase*)));

}

void Player::unitDestroyed(VisualUnit* unit)
{
 if (!unit) {
	kdError() << "Player::unitDestroyed(): Cannot remove NULL unit" << endl;
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
	kdError() << "NULL property" << endl;
	return;
 }

// VERY EVIL HACK!!!
 VisualUnit* unit;
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
	kdError() << "Player::slotUnitPropertyChanged(): NULL unit" << endl;
//	kdDebug() << h->id() << endl;
	kdDebug() << "player=" << id() << ",propId=" << prop->id() << ",units=" << d->mUnits.count() << endl;
	return;
 }

 switch(prop->id()) {
	case Unit::IdWork:
	case Unit::IdId:
	case Unit::IdDamage:
	case Unit::IdReload:
	case VisualUnit::IdDirection:
	case VisualUnit::IdWaypoints:
	case VisualUnit::IdFix_ConstructionState:
	case VisualUnit::IdFix_ConstructionDelay:
		// these IDs are not to be displayed in BosonUnitView.
		break;
	case Unit::IdHealth:
	case Unit::IdArmor:
	case Unit::IdShields:
	case Unit::IdSpeed:
	case Unit::IdType: // FIXME: can this change at all? (currently not)
	case Unit::IdCost:
	case Unit::IdRange:
	case VisualUnit::IdReloadState:
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

VisualUnit* Player::findUnit(unsigned long int unitId) const
{
 QPtrListIterator<VisualUnit> it(d->mUnits);
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
// kdDebug() << "Player::save()" << endl;
 Q_UINT32 unitCount = d->mUnits.count();
 stream << unitCount;
 for (unsigned int i = 0; i < unitCount; i++) {
//	kdDebug() << "save unit " << i << endl;
	VisualUnit* unit = d->mUnits.at(i);
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
 if (!KPlayer::load(stream)) {
	kdError() << "Couldn't load player" << endl;
	return false;
 }
// kdDebug() << "Player::load()" << endl;
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
/*
	VisualUnit* unit = 0;
	emit signalCreateUnit(unit, type, this);

	unit->load(stream);
	addUnit(unit);// FIXME: d->mUnitPropId*/

// id is changed 3 (three) times ?!?!? 
	emit signalLoadUnit(type, id, this);
	VisualUnit* unit = findUnit(id);
	unit->load(stream);
	if (unit->id() != id) {
		kdWarning() << "hu??" << endl;
		unit->setId(id);
	}
 }
 return true;
}

QPtrList<VisualUnit> Player::allUnits() const
{
 return d->mUnits;
}

void Player::sendStopMoving(VisualUnit* unit)
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
	kdError() << "NULL theme" << endl;
	return 0;
 }
 return speciesTheme()->unitProperties(unitType);
}
