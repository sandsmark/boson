#include "boson.h"

#include "bosonmap.h"
#include "bosonmessage.h"
#include "player.h"
#include "defines.h"
#include "visualunit.h"
#include "speciestheme.h"
#include "unitproperties.h"

#include <kdebug.h>
#include <klocale.h>

#include <qtimer.h>

#include "boson.moc"

class BosonPrivate
{
public:
	BosonPrivate()
	{
		mGameTimer = 0;
		mCanvas = 0;

	}

	QTimer* mGameTimer;
	QCanvas* mCanvas; // used for construcing a unit ONLY!! ( this pointer is anti-OO IMHO)
	
	KGamePropertyInt mGameSpeed;

	KGameProperty<unsigned long int> mNextUnitId;
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
 d->mGameSpeed.registerData(IdGameSpeed, dataHandler(),
		KGamePropertyBase::PolicyLocal, "GameSpeed"); // PolicyClean?
 d->mNextUnitId.registerData(IdNextUnitId, dataHandler(),
		KGamePropertyBase::PolicyLocal, "NextUnitId");
 d->mNextUnitId = 0;
}

Boson::~Boson()
{
 delete d;
}

void Boson::setCanvas(QCanvas* c)
{
 d->mCanvas = c;
}

void Boson::quitGame()
{
// kdDebug() << "Boson::quitGame" << endl;
// reset everything
 d->mGameTimer->stop();
 setGameStatus(KGame::End);
 d->mNextUnitId = 0;

 // remove all players from game
 for (unsigned int i = 0; i < playerCount(); i++) {
	removePlayer(playerList()->at(i));
 }
// kdDebug() << "Boson::quitGame done" << endl;
}

bool Boson::playerInput(QDataStream& stream, KPlayer* p)
{
 Player* player = (Player*)p;
 Q_UINT32 msgid;
 stream >> msgid;
 kdDebug() << "playerInput " << msgid << endl;
 switch (msgid) {
	case BosonMessage::MoveMove:
	{
//		kdDebug() << "MoveMove" << endl;
		QPoint pos;
		Q_UINT32 unitCount;
		stream >> pos;
		stream >> unitCount;
		kdDebug() << "unitCount: " << unitCount << endl;
		for (unsigned int i = 0; i < unitCount; i++) {
			Q_ULONG unitId;
			stream >> unitId;
//			kdDebug() << "pos: " << pos.x() << " " << pos.y() << endl;
			VisualUnit* unit = findUnit(unitId, player);
			if (!unit) {
				kdDebug() << "unit " << unitId << " not found for this player" << endl;
				break;
			}
			kdDebug() << "move " << unitId << endl;
			if (unit->owner() != player) {
				kdDebug() << "unit " << unitId << ": only owner can move units!" << endl;
				break;
			}
			if (unit->unitProperties()->isMobile()) {
				unit->moveTo(pos);
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
		VisualUnit* attackedUnit = findUnit(attackedUnitId, 0);
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
			VisualUnit* unit = findUnit(unitId, player);
			if (!unit) {
				kdDebug() << "unit " << unitId << " not found for this player" << endl;
				break;
			}
			if (unit->range() > 0 && unit->damage() > 0) {
				kdDebug() << unitId << " attacks " << attackedUnitId << endl;
				unit->setTarget(attackedUnit);
				unit->setWork(Unit::WorkAttack);
				unit->setAnimated(true);
			}
		}
		break;
	}
	default:
		kdWarning() << "unexpected playerInput " << msgid << endl;
		break;
 }
 return true;
}

void Boson::slotLoadUnit(int unitType, unsigned long int id, Player* owner)
{
 if (!owner) {
	kdError() << "Cannot find player " << owner << endl;
	return;
 }
 kdDebug() << "slotLoadUnit" << endl;
 VisualUnit* unit = createUnit(unitType, owner);
 unit->setId(id);
 emit signalAddUnit(unit, 0, 0); // FIXME x=0, y=0 ??
}

void Boson::slotNetworkData(int msgid, const QByteArray& buffer, Q_UINT32 , Q_UINT32 )
{
 QDataStream stream(buffer, IO_ReadOnly);
 switch (msgid) {
	case BosonMessage::AddUnit:
	{
		Q_INT32 unitType;
		Q_INT32 x;
		Q_INT32 y;
		Q_INT32 owner;

		stream >> unitType;
		stream >> x;
		stream >> y;
		stream >> owner;
		
		KPlayer* p = 0;
		if (owner >= 1024) { // a KPlayer ID
			p = findPlayer(owner);
		} else {
			p = playerList()->at(owner);
		}
		if (!p) {
			kdError() << "Cannot find player " << owner << endl;
			break;
		}
		VisualUnit* unit = createUnit(unitType, (Player*)p);
		unit->setId(nextUnitId());
		emit signalAddUnit(unit, x, y);
		break;
	}
	case BosonMessage::Advance:
		emit signalAdvance();
		break;
	case BosonMessage::InitMap:
		emit signalInitMap(buffer);
		break;
	default:
		kdWarning() << "unhandled msgid " << msgid << endl;
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
	d->mGameTimer->start(gameSpeed());
 } else {
	kdWarning() << "is not server - cannot start the game!" << endl;
 }
}

void Boson::slotSendAdvance()
{
 sendMessage(0, BosonMessage::Advance);
}

VisualUnit* Boson::createUnit(int unitType, Player* owner)
{
 if (!owner) {
	kdError() << "NULL owner" << endl;
	return 0;
 }
 SpeciesTheme* theme = owner->speciesTheme();
 if (!theme) {
	kdError() << "No theme for this player" << endl;
	return 0; // BAAAAD
 }
 const UnitProperties* prop = theme->unitProperties(unitType);
 if (!prop) {
	kdError() << "Unknown unitType " << unitType << endl;
	return 0;
 }

 VisualUnit* unit = 0;
 if (prop->isMobile()) {
	unit = new VisualMobileUnit(unitType, owner, d->mCanvas);
 } else if (prop->isFacility()) {
	unit = new VisualFacility(unitType, owner, d->mCanvas);
 } else { // should be impossible
	kdError() << "Internal Error at unit type " << unitType << endl;
	return 0;
 }
 owner->addUnit(unit); // can also be in VisualUnit c'tor - is this clean?
 theme->loadNewUnit(unit);

 return unit;
}

unsigned long int Boson::nextUnitId()
{
 d->mNextUnitId = d->mNextUnitId + 1;
//kdDebug() << "next id: " << d->mNextUnitId << endl;
 return d->mNextUnitId;
}

VisualUnit* Boson::findUnit(unsigned long int id, Player* searchIn) const
{
 if (searchIn) {
	return searchIn->findUnit(id);
 }
 QPtrListIterator<KPlayer> it(*playerList());
 while (it.current()) {
	VisualUnit* unit = ((Player*)it.current())->findUnit(id);
	if (unit) {
		return unit;
	}
	++it;
 }
 return 0;
}

KPlayer* Boson::createPlayer(int rtti, int io, bool isVirtual)
{
 kdDebug() << "Boson::createPlayer(): rtti=" << rtti << ",io=" << io 	
		<< ",isVirtual=" << isVirtual << endl;
 Player* p = new Player();
// connect(p, SIGNAL(signalCreateUnit(VisualUnit*&, int, Player*)), 
//		this, SLOT(slotCreateUnit(VisualUnit*&, int, Player*)));
 connect(p, SIGNAL(signalLoadUnit(int, unsigned long int, Player*)), 
		this, SLOT(slotLoadUnit(int, unsigned long int, Player*)));
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
 if (speed > MIN_GAME_SPEED || speed < MAX_GAME_SPEED) {
	kdWarning() << "unexpected speed " << speed << endl;
 }
// kdDebug() << "Boson::setGameSpeed " << speed << endl;
 d->mGameSpeed = speed;
 if (d->mGameTimer->isActive()) {
	d->mGameTimer->stop();
	d->mGameTimer->start(gameSpeed());
 }
}

/*
void Boson::slotCreateUnit(VisualUnit*& unit, int unitType, Player* owner)
{ // request from a Player to create a unit
 unit = createUnit(unitType, owner);
}*/

void Boson::slotSave(QDataStream& stream)
{ // save non-KGameProperty datas here
// stream <<
}

void Boson::slotLoad(QDataStream& stream)
{
 kdDebug() << "next id: " << d->mNextUnitId << endl;
}

void Boson::slotConstructUnit(int unitType, VisualUnit* facility, Player* owner)
{
 if (!facility) { 
	kdError() << "NULL facility" << endl;
	return;
 }
 slotConstructUnit(unitType, facility->x(), facility->y(), owner);
}

void Boson::slotConstructUnit(int unitType, int x, int y, Player* owner)
{ // used by the editor directly
 if (!owner) {
	kdWarning() << "NULL owner! using first player" << endl;
	owner = (Player*)playerList()->at(0);
 }
 if (!owner) { // no player here
	kdError() << "NULL owner" << endl;
	return;
 }

 slotAddUnit(unitType, x, y, owner->id());
}

void Boson::slotAddUnit(int unitType, int x, int y, int owner)
{ // kind of obsolete - move stuff to slotConstructUnit
 if (!isServer()) {
	return;
 }

 QByteArray buffer;
 QDataStream stream(buffer, IO_WriteOnly);
 stream << (Q_INT32)unitType;
 stream << (Q_INT32)x;
 stream << (Q_INT32)y;
 stream << (Q_INT32)owner;
 sendMessage(buffer, BosonMessage::AddUnit);
}

void Boson::slotReplacePlayerIO(KPlayer* player, bool* remove)
{
 *remove = false;
 if (!player) {
	kdError() << "NULL player" << endl;
	return;
 }
 if (!isAdmin()) {
	kdError() << "Boson::slotReplacePlayerIO(): only ADMIN can do this" << endl; 
	return;
 }
 kdDebug() << "slotReplacePlayer()" << endl;
}
