
#include "bosonmap.h"

#include "unit.h"
#include "cell.h"
#include "player.h"
#include "boson.h"

#include <qfile.h>
#include <qdatastream.h>

#include <kgame/kgamepropertyhandler.h>
#include <kgame/kgameproperty.h>
#include <kdebug.h>
#include <kstandarddirs.h>

#include "defines.h"

#define TAG_FIELD "boeditor_magic_0_3"
#define TAG_FIELD_LEN 18 // len of above
#define TAG_CELL (0xde)

class BosonMapPrivate
{
public:
	BosonMapPrivate()
	{
		mCells = 0;
	}
	
	QString mFileName;

	KGamePropertyHandler mProperties;

	KGameProperty<int> mMapWidth;
	KGameProperty<int> mMapHeight;
	
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
 delete d;
}

void BosonMap::init()
{
 d = new BosonMapPrivate;
 // note we do not register d->mProperties anywhere as we don't use send() or
 // emitSignal from KGameProperty. They are just used for saving the file.
 d->mMapHeight.registerData(IdMapHeight, dataHandler(),
		KGamePropertyBase::PolicyLocal, "MapHeight");
 d->mMapWidth.registerData(IdMapWidth, dataHandler(),
		KGamePropertyBase::PolicyLocal, "MapWidth");
 d->mMapWidth.setLocal(0);
 d->mMapHeight.setLocal(0);
}

QString BosonMap::defaultMap()
{
//TODO create a list of all maps (search for .desktop files in boson/map/) and
//look if list contains "basic" - then check whether basic.bpf exists and return. 
// Otherwise look for the first .desktop file which has a .bpf file exisiting.
// Return that then.
 return locate("data", "boson/map/basic.bpf");
}

KGamePropertyHandler* BosonMap::dataHandler() const
{
 return &d->mProperties;
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
 Q_UINT32 players; // obsolete
 Q_INT32 mapWidth;
 Q_INT32 mapHeight;

 stream >> players; // obsolete

 stream >> mapWidth;
 stream >> mapHeight;
 
 stream >> players; // obsolete
 stream >> players; // obsolete
 QString x;
 stream >> x; // obsolete


 // check 'realityness'
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

// map is ok - lets apply
 d->mMapWidth = mapWidth; // horizontal cell count
 d->mMapHeight = mapHeight; // vertical cell count

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
		if (!loadCell(stream, groundType, version)) {
			return false;
		}
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

bool BosonMap::saveMap(const QString& fileName)
{
 kdDebug() << "BosonMap::saveMap()" << endl;
// TODO: check if file exists already
 // open stream 
 QFile f(fileName);
 if (!f.open(IO_WriteOnly)){
	kdError() << "BosonMap: Can't open file " << fileName << endl;
	return false;
 }
 QDataStream stream(&f);
 
 // magic 
 // Qt marshalling for a string is 4-byte-len + data
 stream << (Q_INT32)(TAG_FIELD_LEN - 1);

 for (int i = 0; i <= TAG_FIELD_LEN; i++) {
	stream << (Q_INT8)TAG_FIELD[i];
 }

 bool ret = saveMapGeo(stream);
 f.close();
 kdDebug() << "BosonMap::saveMap() done" << endl;
 return ret;
}

bool BosonMap::saveMapGeo(QDataStream& stream)
{
 if (!isValidGeo()) {
	kdError() << "Map geo is not valid" << endl;
	return false;
 }
 kdDebug() << "save map geo" << endl;
 stream << (Q_UINT32)1; // obsolete
 stream << (Q_INT32)width();
 stream << (Q_INT32)height();
 stream << (Q_UINT32)1; // obsolete
 stream << (Q_UINT32)1; // obsolete
 stream << QString(""); // obsolete

 for (int i = 0; i < width(); i++) {
	for (int j = 0; j < height(); j++) {
		Cell* c = cell(i, j);
		saveCell(stream, c);
	}
 }
 return true;
}

bool BosonMap::isValidGeo() const
{
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

 //TODO: check cells!

 return true;
}

bool BosonMap::loadCell(QDataStream& stream, int& groundType, unsigned char& b)
{
 Q_INT32 g;
 Q_INT8 version;
 stream >> g; // not groundType - first TAG_CELL
 if (g != TAG_CELL) {
	kdError() << "BosonMap::loadCell(): broken map file!" << endl;
	kdError() << "missing TAG_CELL!" << endl;
	return false;
 }

 stream >> g; // this is the groundType now.
 if(!Cell::isValidGround(g)) { 
	return false; 
 }
// if (g < 0 || ) { return false; }
 
 stream >> version;
 if (version > 4) {
	kdError() << "BosonMap::loadCell(): broken map file!" << endl;
	kdError() << "invalid cell!" << endl;
 }
 groundType = g;
 b = version;

 return true;
}

void BosonMap::saveCell(QDataStream& stream, Cell* c)
{
 int groundType = 0;
 unsigned char version = 0;
 if (!c) {
	kdError() << "BosonMap::saveCell(): NULL Cell" << endl;
	// do not abort - otherwise all clients receiving this
	// stream are completely broken as we expect
	// width()*height() cells
 } else {
	groundType = c->groundType();
	version = c->version();
 }
 stream << (Q_INT32)TAG_CELL;
 stream << (Q_INT32)groundType;
 stream << (Q_INT8)version;
}

int BosonMap::width() const
{
 return d->mMapWidth;
}

int BosonMap::height() const
{
 return d->mMapHeight;
}

QString BosonMap::worldName() const // FIXME?
{
 //return d->mWorldName;
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

