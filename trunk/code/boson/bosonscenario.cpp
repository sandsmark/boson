
#include "bosonscenario.h"

#include "unit.h"
#include "player.h"
#include "boson.h"

#include <qfile.h>
#include <qdatastream.h>

#include <kgame/kgamepropertyhandler.h>
#include <kgame/kgameproperty.h>
#include <kdebug.h>
#include <kstandarddirs.h>

#include "defines.h"

#define TAG_FIELD "boeditor_magic_0_3" // obsolete
#define TAG_FIELD_LEN 18 // len of above // obsolete

#define TAG_FIX (0xbe) // TODO
#define TAG_MOB (0xba) // TODO
//#define TAG_UNIT (0xba) // TODO

struct ScenarioUnit
{
	int unitType;
	int x;
	int y;
};


class BosonScenarioPrivate
{
public:
	BosonScenarioPrivate()
	{
		mUnits = 0;
	}
	
	QString mFileName;

	KGamePropertyHandler mProperties;

	KGameProperty<int> mMaxPlayers; // -1 == unlimited
	KGameProperty<unsigned int> mMinPlayers;
//	KGameProperty<QString> mWorldName; // TODO: i18n - see TODO file

	QValueList<ScenarioUnit>* mUnits;
};

BosonScenario::BosonScenario()
{
 init();
}

BosonScenario::BosonScenario(const QString& fileName)
{
 init();
 loadScenario(fileName);
}

BosonScenario::~BosonScenario()
{
 if (d->mUnits) {
	d->mUnits->clear();
	delete d->mUnits;
 }
 delete d;
}

void BosonScenario::init()
{
 d = new BosonScenarioPrivate;
 // note we do not register d->mProperties anywhere as we don't use send() or
 // emitSignal from KGameProperty. They are just used for saving the file.
 d->mMaxPlayers.registerData(IdMaxPlayers, dataHandler(),
		KGamePropertyBase::PolicyLocal, "MaxPlayers");
 d->mMinPlayers.registerData(IdMinPlayers, dataHandler(),
		KGamePropertyBase::PolicyLocal, "MinPlayers");
// d->mWorldName.registerData(IdWorldName, dataHandler(),
//		KGamePropertyBase::PolicyLocal, "WorldName");
}

QString BosonScenario::defaultScenario()
{
 return locate("data", "boson/scenario/basic.bsc");
}

KGamePropertyHandler* BosonScenario::dataHandler() const
{
 return &d->mProperties;
}

bool BosonScenario::loadScenario(const QString& fileName)
{
 kdDebug() << "BosonScenario::loadScenario " << fileName << endl;

 // open stream 
 QFile f(fileName);
 if (!f.open(IO_ReadOnly)){
	kdError() << "BosonScenario: Can't open file " << fileName << endl;
	return false;
 }
 QDataStream stream(&f);
 bool ret = loadScenario(stream);
 if (ret) {
	d->mFileName = fileName;
 }
 f.close();
 return ret;
}

bool BosonScenario::loadScenario(QDataStream& stream)
{
/*
 if (!verifyScenario(stream)) {//TODO
	kdError() << "Invalid map file" << endl;
	return false;
 }*/

 Q_UINT32 minPlayers;
 Q_INT32 maxPlayers;
 stream >> minPlayers;
 stream >> maxPlayers;

 if (minPlayers < 1) {
	kdError() << "BosonScenario::loadCompleteScenario(): broken scenario file!" << endl;
	kdError() << "minPlayers < 1" << endl;
	return false;
 }
 if (maxPlayers > BOSON_MAX_PLAYERS) {
	kdError() << "BosonScenario::loadCompleteScenario(): broken scenario file!" << endl;
	kdError() << "maxPlayers > " << BOSON_MAX_PLAYERS << endl;
	return false;
 }
 if ((int)minPlayers > maxPlayers) {
	kdError() << "BosonScenario::loadCompleteScenario(): broken scenario file!" << endl;
	kdError() << "minPlayers > maxPlayers" << endl;
	return false;
 }
 d->mMinPlayers = minPlayers;
 d->mMaxPlayers = maxPlayers;


 d->mUnits = new QValueList<ScenarioUnit>[d->mMaxPlayers];
 for (int i = 0; i < d->mMaxPlayers; i++) {
	loadUnits(stream, i);
 }
 
 return true;
}

bool BosonScenario::verifyScenario(QDataStream& stream)
{
 char magic[ TAG_FIELD_LEN+4 ];  // paranoic spaces
 int i;

 // magic 
 // Qt marshalling for a string is 4-byte-len + data
 stream >> i;
 if (TAG_FIELD_LEN + 1 != i) {
	kdError() << "BosonScenario::verifyScenario(): Magic doesn't match(len), check file name" << endl;
	return false;
 }

 for (i = 0; i < TAG_FIELD_LEN + 1; i++) {
	Q_INT8  b;
	stream >> b;
	magic[i] = b;
 }

 if (strncmp(magic, TAG_FIELD, TAG_FIELD_LEN) ) {
	kdError() << "BosonScenario::verifyScenario(): Magic doesn't match(string), check file name" << endl;
	return false;
 }
 return true;
}

bool BosonScenario::loadUnits(QDataStream& stream, unsigned int playerNumber)
{
 if (!d->mUnits) {
	kdError() << "BosonScenario::loadUnits(): Unit array not yet created" 
			<< endl;
	return false;
 }
 if (playerNumber >= maxPlayers()) {
	kdError() << "BosonScenario::loadUnits(): don't know player " 
			<< playerNumber << endl;
	return false;
 }
 Q_UINT32 unitCount;
 stream >> unitCount;
 for (unsigned int i = 0; i < unitCount; i++) {
	Q_INT32 unitType;
	Q_INT32 x;
	Q_INT32 y;

//TODO: TAG_UNIT
	stream >> unitType;
	stream >> x;
	stream >> y;
		
	ScenarioUnit s;
	s.unitType = unitType;
	s.x = x;
	s.y = y;
	d->mUnits[playerNumber].append(s);
 }
 return true;
}

bool BosonScenario::isValid() const
{
 if (d->mMinPlayers != (uint)d->mMaxPlayers) { // FIXME
	kdError() << "internal error" << endl;
	return false;
 }
 if (d->mMinPlayers < 1) {
	kdError() << "minplayers < " << 1 << endl;
	return false;
 }
 if (d->mMinPlayers > BOSON_MAX_PLAYERS) {
	kdError() << "minplayers > " << BOSON_MAX_PLAYERS << endl;
	return false;
 }
 if (!d->mUnits) {
	kdError() << "no units!" << endl;
	return false;
 }

 return true;
}

unsigned int BosonScenario::minPlayers() const
{
 return d->mMinPlayers;
}

int BosonScenario::maxPlayers() const
{
 return d->mMaxPlayers;
}
/*
QString BosonScenario::worldName() const
{
 return d->mWorldName;
}*/

void BosonScenario::addPlayerUnits(Boson* boson, int playerNumber)
{
 if (!boson) {
	kdError() << "BosonScenario::addPlayerUnits(): NULL game" << endl;
	return;
 }
 Player* p = (Player*)boson->playerList()->at(playerNumber);
 if (!p) {
	kdError() << "BosonScenario::addPlayerUnits(): NULL player" << endl;
	return;
 }
 kdDebug() << "BosonScenario::addPlayerUnits of player " << playerNumber 
		<< "==" << p->id() << endl;
 if ((int)d->mUnits->count() < playerNumber) {
	kdError() << "BosonScenario::addPlayerUnits(): don't have player " 
			<< playerNumber << endl;
	return;
 }
 unsigned int unitCount = d->mUnits[playerNumber].count();
 for (unsigned int i = 0; i < unitCount; i++) {
	ScenarioUnit s = (d->mUnits[playerNumber])[i]; // ieek
	addUnit(boson, p, s.unitType, s.x, s.y);
 }
}

void BosonScenario::addUnit(Boson* boson, Player* owner, int unitType, int x, int y)
{
 if (!boson) {
	kdError() << "BosonScenario::addUnit(): NULL game" << endl;
			return;
 }
 if (!owner) {
	kdError() << "BosonScenario::addUnit(): NULL player" << endl;
	return;
 }
 boson->slotSendAddUnit(unitType, x, y, owner);
}

void BosonScenario::startScenario(Boson* boson)
{
 if (!isValid()) {
	return;
 }
 if (!boson) {
	kdError() << "BosonScenario::addPlayerUnits(): NULL game" << endl;
	return;
 }
 for (int i = 0; i < maxPlayers(); i++) {
	addPlayerUnits(boson, i);
 }
}

