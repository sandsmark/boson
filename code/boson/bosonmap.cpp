
#include "bosonmap.h"

#include "unit.h"
#include "cell.h"
#include "player.h"
#include "boson.h"

#include <qfile.h>
#include <qdatastream.h>

#include <kdebug.h>
#include <kstandarddirs.h>

#include "defines.h"

#define TAG_FIELD "boeditor_magic_0_3"
#define TAG_FIELD_LEN 18 // len of above
#define TAG_CELL (0xde)
#define TAG_FIX (0xbe)
#define TAG_MOB (0xba)

struct MapUnit
{
	int unitType;
	int x;
	int y;
	int who;// aka owner
};

class BosonMapPrivate
{
public:
	BosonMapPrivate()
	{
		mMaxPlayers = 0;
		mMapWidth = 0;
		mMapHeight = 0;

		mFacilities = 0;
		mMobileUnits = 0;

		mMobileCount = 0;
		mFacilityCount = 0;

		mCells = 0;
	}
	
	QString mFileName;
	int mMaxPlayers; // -1 == unlimited
	unsigned int mMinPlayers;
	int mMapWidth;
	int mMapHeight;
	QString mWorldName;
	
	MapUnit* mFacilities;
	MapUnit* mMobileUnits;
	unsigned int mMobileCount;
	unsigned int mFacilityCount;

	Cell* mCells;
};

BosonMap::BosonMap()
{
 init();
}

BosonMap::BosonMap(const QString& fileName)
{
 init();
 loadMap(fileName);
}

BosonMap::~BosonMap()
{
 if (d->mFacilities) {
	delete[] d->mFacilities;
 }
 if (d->mMobileUnits) {
	delete[] d->mMobileUnits;
 }
 if (d->mCells) {
	delete[] d->mCells;
 }
 delete d;
}

void BosonMap::init()
{
 d = new BosonMapPrivate;
}

QString BosonMap::defaultMap()
{
 return locate("data", "boson/map/basic.bpf");
}

bool BosonMap::loadMap(const QString& fileName)
{
 kdDebug() << "BosonMap::loadMap " << fileName << endl;

 // open stream 
 QFile f(fileName);
 if (!f.open(IO_ReadOnly)){
	kdError() << "BosonMap: Can't open file " << fileName << endl;
	return false;
 }
 QDataStream stream(&f);
 bool ret = loadCompleteMap(stream);
 if (ret) {
	d->mFileName = fileName;
 }
 f.close();
 return ret;
}

bool BosonMap::loadCompleteMap(QDataStream& stream)
{
 if (!verifyMap (stream)) {
	kdError() << "Invalid map file" << endl;
	return false;
 }
 if (!loadMapGeo(stream)) {
	kdError() << "Error loading map geo" << endl;
	return false;
 }

// load mobile units into d->mMobileUnits
 if (!loadMobileUnits(stream)) {
	kdError() << "Error loading mobile units" << endl;
	return false;
 }

// load fix units into d->mFacilities
 if (!loadFacilities(stream)) {
	kdError() << "Error loading facilities" << endl;
	return false;
 }
 return true;
}

bool BosonMap::verifyMap(QDataStream& stream)
{
 char magic[ TAG_FIELD_LEN+4 ];  // paranoic spaces
 int i;

 // magic 
 // Qt marshalling for a string is 4-byte-len + data
 stream >> i;
 if (TAG_FIELD_LEN + 1 != i) {
	kdError() << "BosonMap::loadMap(): Magic doesn't match(len), check file name" << endl;
	return false;
 }

 for (i = 0; i < TAG_FIELD_LEN + 1; i++) {
	Q_INT8  b;
	stream >> b;
	magic[i] = b;
 }

 if (strncmp(magic, TAG_FIELD, TAG_FIELD_LEN) ) {
	kdError() << "BosonMap::loadMap(): Magic doesn't match(string), check file name" << endl;
	return false;
 }
 return true;
}

bool BosonMap::loadMapGeo(QDataStream& stream)
{
 Q_UINT32 players;
 Q_UINT32 mobiles;
 Q_UINT32 facilities;
 Q_INT32 mapWidth;
 Q_INT32 mapHeight;
 QString worldName; // FIXME: i18n?!

 stream >> players;
 stream >> mapWidth;
 stream >> mapHeight;
 stream >> mobiles;
 stream >> facilities;
 stream >> worldName;

 // check 'realityness'
 if (players < 1) {
	kdError() << "BosonMap::loadMap(): broken map file!" << endl;
	kdError() << "players < 1" << endl;
	return false;
 }
 if (mapWidth < 10) {
	kdError() << "BosonMap::loadMap(): broken map file!" << endl;
	kdError() << "mapWidth < 10" << endl;
	return false;
 }
 if (mapHeight < 10) {
	kdError() << "BosonMap::loadMap(): broken map file!" << endl;
	kdError() << "mapHeight < 10" << endl;
	return false;
 }
 if (mobiles < 1) {
	kdError() << "BosonMap::loadMap(): broken map file!" << endl;
	kdError() << "Mobiles < 1" << endl;
	return false;
 }
 if (facilities < 1) {
	kdError() << "BosonMap::loadMap(): broken map file!" << endl;
	kdError() << "Facilities < 1" << endl;
	return false;
 }

 if (players > BOSON_MAX_PLAYERS) {
	kdError() << "BosonMap::loadMap(): broken map file!" << endl;
	kdError() << "players > " << BOSON_MAX_PLAYERS << endl;
	return false;
 }
 if (mapWidth > MAX_MAP_WIDTH) {
	kdError() << "BosonMap::loadMap(): broken map file!" << endl;
	kdError() << "mapWidth > " << MAX_MAP_WIDTH << endl;
	return false;
 }
 if (mapHeight > MAX_MAP_HEIGHT) {
	kdError() << "BosonMap::loadMap(): broken map file!" << endl;
	kdError() << "mapHeight > " << MAX_MAP_HEIGHT << endl;
	return false;
 }
 if (mobiles > MOBILES) {
	kdError() << "BosonMap::loadMap(): broken map file!" << endl;
	kdError() << "maxMobiles > " << MOBILES << endl;
	kdError() << mobiles << endl;
//	return false;
 }
 if (facilities > FACILITIES) {
	kdError() << "BosonMap::loadMap(): broken map file!" << endl;
	kdError() << "Facilities > " << FACILITIES << endl;
	return false;
 }

// map is ok - lets apply
 d->mMinPlayers = players; // minplayers is not yet used (players == the number of players in this map)
 d->mMaxPlayers = players; // maxplayers is not yet used (players == the number of players in this map)
 d->mMapWidth = mapWidth; // horizontal cell count
 d->mMapHeight = mapHeight; // vertical cell count
 d->mWorldName = worldName; // guess what?
 d->mFacilityCount = facilities; // no of facilities to be loaded from this map
 d->mMobileCount = mobiles; // no of mobiles to be loaded from this map

 if (d->mCells) {
//	kdDebug() << "cells created before!! try to delete..." << endl;
	delete[] d->mCells;
 }
 d->mCells = new Cell[width() * height()];

// load all cells:
 for (int i = 0; i < width(); i++) {
	for (int j = 0; j < height(); j++) {
		int groundType = 0;
		unsigned char version = 0;
		loadCell(stream, groundType, version);
		Cell* c = cell(i, j);
		if (!c) {
			kdError() << "NULL cell" << endl;
			continue;
		}
		if ((Cell::GroundType)groundType == Cell::GroundUnknown) {
			kdWarning() << "Unknown ground?! x=" << i << ",y=" 
					<< j << endl;
		}
		c->makeCell(groundType, version);
	}
 }

 return true;
}

bool BosonMap::saveMapGeo(QDataStream& stream)
{
 if (!isValidGeo()) {
	kdError() << "Map geo is not valid" << endl;
	return false;
 }
 kdDebug() << "save map geo" << endl;
 stream << (Q_UINT32)minPlayers(); // FIXME: currently minPlayers==maxPlayers
 stream << (Q_INT32)width();
 stream << (Q_INT32)height();
 stream << (Q_UINT32)d->mMobileCount;
 stream << (Q_UINT32)d->mFacilityCount;
 stream << (QString)worldName();

 for (int i = 0; i < width(); i++) {
	for (int j = 0; j < height(); j++) {
		Cell* c = cell(i, j);
		int groundType = 0;
		unsigned char version = 0;
		if (!c) {
			kdError() << "BosonMap::saveMapGeo(): NULL cell" << endl;
			// do not abort - otherwise all clients receiving this
			// stream are completely broken as we expect
			// width()*height() cells
		} else {
			groundType = c->groundType();
			version = c->version();
		}
		
		stream << (unsigned int)TAG_CELL;
		stream << groundType;
		stream << version;
	}
 }
 return true;
}

bool BosonMap::isValidGeo() const
{
 if (d->mMinPlayers != (uint)d->mMaxPlayers) {
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
 if (width() < 10) {
	kdError() << "width < 10" << endl;
	return false;
 }
 if (width() > MAX_MAP_WIDTH) {
	kdError() << "mapWidth > " << MAX_MAP_WIDTH << endl;
	return false;
 }
 if (height() < 10) {
	kdError() << "height < 10" << endl;
	return false;
 }
 if (height() > MAX_MAP_HEIGHT) {
	kdError() << "mapHeight > " << MAX_MAP_HEIGHT << endl;
	return false;
 }
 if (d->mMobileCount< 1) {
	kdError() << "Mobiles < 1" << endl;
	return false;
 }
 if (d->mMobileCount > MOBILES) {
	kdError() << "maxMobiles > " << MOBILES << endl;
	kdError() << d->mMobileCount << endl;
//	return false;
 }
 if (d->mFacilityCount < 1) {
	kdError() << "Facilities < 1" << endl;
	return false;
 }
 if (d->mFacilityCount > FACILITIES) {
	kdError() << "Facilities > " << FACILITIES << endl;
	return false;
 }

 //TODO: check cells!

 return true;
}

bool BosonMap::loadMobileUnits(QDataStream& stream)
{
 if (d->mMobileUnits) {
	kdError() << "Mobile units already loaded! loading again without deleting - memory hole!!" << endl;
 }
 if (d->mMobileCount < 1) {
	kdError() << "MobileCount < 1" << endl;
	return false;
 }
 d->mMobileUnits = new MapUnit[d->mMobileCount];

 for (unsigned int i = 0; i < d->mMobileCount; i++) {
	int j;
	int x, y, who;
	stream >> j;
	if (TAG_MOB != j) {
		kdError() << "TAG_MOB missing" << endl;
		return false;
	}
	stream >> j >> x >> y >> who;
	// j == unitType (mobile unit)
	// x,y == position on canvas
	// owner == ... owner!
	// we use another unitType format - facilities and mobile units share
	// the same enum. so our unittype looks like this:
	// groups land, air and water.

//	int unitType = (Unit::FacilityLast + 1) + j;
	int unitType = (9999 + 1) + j; // TODO: start a new map file format to avoid this

	d->mMobileUnits[i].unitType = unitType;
	d->mMobileUnits[i].x = x;
	d->mMobileUnits[i].y = y;
	d->mMobileUnits[i].who = who;
 }
 return true;
}

bool BosonMap::loadFacilities(QDataStream& stream)
{
 if (d->mFacilities) {
	kdError() << "Facilities already loaded! loading again without deleting - memory hole!!" << endl;
 }
 if (d->mFacilityCount < 1) {
	kdError() << "FacilityCount < 1" << endl;
	return false;
 }
 d->mFacilities = new MapUnit[d->mFacilityCount];

 for (unsigned int i = 0; i < d->mFacilityCount; i++) {
	int j;
	int x, y, who;
	stream >> j;
	if (TAG_FIX != j) {
		kdError() << "TAG_FIX missing" << endl;
		return false;
	}
	stream >> j >> x >> y >> who;
	// j == unitType (facilityType)
	// x,y == position on canvas
	// owner == ... owner!
	int unitType = j;
	d->mFacilities[i].unitType = unitType;
	d->mFacilities[i].x = x;
	d->mFacilities[i].y = y;
	d->mFacilities[i].who = who;
 }
 return true;
}

bool BosonMap::loadCell(QDataStream& stream, int& groundType, unsigned char& b)
{
 stream >> groundType; // not groundType - first TAG_CELL

 if (groundType != TAG_CELL) {
	kdError() << "BosonMap::loadCell(): broken map file!" << endl;
	kdError() << "missing TAG_CELL!" << endl;
	return false;
 }

 stream >> groundType; // this is the groundType now.
 if(!Cell::isValidGround(groundType)) { 
	return false; 
 }
// if (g < 0 || ) { return false; }
 
 stream >> b; // what is this?
 if (b > 4) {
	kdError() << "BosonMap::loadCell(): broken map file!" << endl;
	kdError() << "invalid cell!" << endl;
 }

 return true;
}

int BosonMap::width() const
{
 return d->mMapWidth;
}

int BosonMap::height() const
{
 return d->mMapHeight;
}

unsigned int BosonMap::minPlayers() const
{
 return d->mMinPlayers;
}

int BosonMap::maxPlayers() const
{
 return d->mMaxPlayers;
}

const QString& BosonMap::worldName() const
{
 return d->mWorldName;
}

void BosonMap::addPlayerUnits(Boson* boson, int playerNumber)
{
 if (!boson) {
	kdError() << "BosonMap::addPlayerUnits(): NULL game" << endl;
	return;
 }
 Player* p = (Player*)boson->playerList()->at(playerNumber);
 if (!p) {
	kdError() << "BosonMap::addPlayerUnits(): NULL player" << endl;
	return;
 }
 kdDebug() << "BosonMap::addPlayerUnits of player " << playerNumber 
		<< "==" << p->id() << endl;
 for (unsigned int i = 0; i < d->mFacilityCount; i++) {
	if (d->mFacilities[i].who == playerNumber) {
		addUnit(boson, p, 
				d->mFacilities[i].unitType, 
				d->mFacilities[i].x, 
				d->mFacilities[i].y);
	}
 }
 for (unsigned int i = 0; i < d->mMobileCount; i++) {
	if (d->mMobileUnits[i].who == playerNumber) {
		addUnit(boson, p, 
				d->mMobileUnits[i].unitType,
				d->mMobileUnits[i].x,
				d->mMobileUnits[i].y);
	}
 }
}

void BosonMap::addUnit(Boson* boson, Player* owner, int unitType, int x, int y)
{
 if (!boson) {
	kdError() << "BosonMap::addUnit(): NULL game" << endl;
			return;
 }
 if (!owner) {
	kdError() << "BosonMap::addUnit(): NULL player" << endl;
	return;
 }
 boson->slotConstructUnit(unitType, x, y, owner);
}

void BosonMap::startMap(Boson* boson)
{
 if (!isValidGeo()) {
	return;
 }
 if (!boson) {
	kdError() << "BosonMap::addPlayerUnits(): NULL game" << endl;
	return;
 }
 for (int i = 0; i < maxPlayers(); i++) {
	addPlayerUnits(boson, i);
 }
 if (d->mMobileUnits) {
	delete[] d->mMobileUnits;
	d->mMobileUnits = 0;
 }
 if (d->mFacilities) {
	delete[] d->mFacilities;
	d->mFacilities = 0;
 }
}

Cell* BosonMap::cell(int x, int y) const
{
 if (!d->mCells) {
	kdError() << "Cells not yet created" << endl;
	return 0;
 }
 if (x < 0 || x > width()) {
	kdError() << "Invalid Cell! x=" << x << endl;
	return 0;
 }
 if (y < 0 || x > height()) {
	kdError() << "Invalid Cell! y=" << y << endl;
	return 0;
 }
 return &d->mCells[ x + y * width() ];
}

