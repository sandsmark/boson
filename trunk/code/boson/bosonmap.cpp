
#include "bosonmap.h"

#include "unit.h"
#include "cell.h"
#include "player.h"
#include "boson.h"

#include <qfile.h>
#include <qdatastream.h>
#include <qdom.h>

#include <kdebug.h>
#include <kstandarddirs.h>
#include <kfilterdev.h>

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

	int mMapWidth;
	int mMapHeight;
	
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
}

QString BosonMap::defaultMap()
{
//TODO create a list of all maps (search for .desktop files in boson/map/) and
//look if list contains "basic" - then check whether basic.bpf exists and return. 
// Otherwise look for the first .desktop file which has a .bpf file exisiting.
// Return that then.
 return locate("data", "boson/map/basic.bpf");
}

bool BosonMap::loadMap(const QString& fileName)
{
 kdDebug() << "BosonMap::loadMap " << fileName << endl;
 d->mFileName = fileName; // probably obsolete?
 
 // open stream 
 QIODevice* dev = KFilterDev::deviceForFile(fileName);
 if (!dev) {
	kdError() << "No file " << fileName << endl;
	return false;
 }
 if (!dev->open(IO_ReadOnly)){
	kdError() << "BosonMap: Can't open file " << fileName << endl;
	delete dev;
	return false;
 }
 QByteArray buffer = dev->readAll();
 delete dev;
 QDataStream stream(buffer, IO_ReadOnly);
 
 bool binary = false;
 if (verifyMap(stream)) { // check if this is binary
	binary = true;
 } else {
	binary = false;
 }
 bool ret = loadMap(buffer, binary);
 if (!ret) {
	kdError() << "Could not load file " << fileName << endl;
 }
 return ret;
}

bool BosonMap::loadMap(const QByteArray& buffer, bool binary)
{
 if (binary) {
	QDataStream stream(buffer, IO_ReadOnly);
	if (!verifyMap (stream)) { // we already did this so this cannot fail
		kdError() << "Invalid map file" << endl;
		return false;
	}
	if (!loadMapGeo(stream)) {
		kdError() << "Error loading map geo" << endl;
		return false;
	}
	if (!loadCells(stream)) {
		kdError() << "Error loading map cells" << endl;
		return false;
	}
	return true;
 }

 // load XML file
 QDomDocument doc("BosonMap");
 QString errorMsg;
 int lineNo;
 int columnNo;
 if (!doc.setContent(buffer, false, &errorMsg, &lineNo, &columnNo)) {
	kdError() << "Parse error in line " << lineNo << ",column " << columnNo
			<< " error message: " << errorMsg << endl;
	return false;
 }
 QDomNodeList list;
 QDomElement root = doc.documentElement();
 list = root.elementsByTagName("MapGeo");
 if (list.count() != 1) {
	kdError() << "XML error: cannot have tag MapGeo " 
			<< list.count() << " times" << endl;
	return false;
 }
 QDomElement geo = list.item(0).toElement();
 if (geo.isNull()) {
	kdError() << "XML error: geo is not an QDomElement" << endl;
	return false;
 }
 if (!loadMapGeo(geo)) {
	kdError() << "XML error: failed loading map geo" << endl;
	return false;
 }

 list = root.elementsByTagName("MapCells");
 if (list.count() != 1) {
	kdError() << "XML error: cannot have tag Map Geo " 
			<< list.count() << " times" << endl;
	return false;
 }
 QDomElement cells = list.item(0).toElement();
 if (geo.isNull()) {
	kdError() << "XML error: cells is not an QDomElement" << endl;
	return false;
 }
 if (!loadCells(cells)) {
	kdError() << "XML error: failed loading map geo" << endl;
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
//	kdError() << "BosonMap::loadMap(): Magic doesn't match(len), check file name" << endl;// not an error - probably an XML file
	return false;
 }

 for (i = 0; i < TAG_FIELD_LEN + 1; i++) {
	Q_INT8  b;
	stream >> b;
	magic[i] = b;
 }

 if (strncmp(magic, TAG_FIELD, TAG_FIELD_LEN) ) {
//	kdError() << "BosonMap::loadMap(): Magic doesn't match(string), check file name" << endl;
	return false;
 }
 return true;
}

void BosonMap::saveValidityHeader(QDataStream& stream)
{ // TODO: use QString for this
 // magic 
 // Qt marshalling for a string is 4-byte-len + data

 stream << (Q_INT32)(TAG_FIELD_LEN - 1);

 for (int i = 0; i <= TAG_FIELD_LEN; i++) {
	stream << (Q_INT8)TAG_FIELD[i];
 }
}

bool BosonMap::loadMapGeo(QDataStream& stream)
{
 Q_INT32 mapWidth;
 Q_INT32 mapHeight;

 stream >> mapWidth;
 stream >> mapHeight;
 
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
 return true;
}

bool BosonMap::loadCells(QDataStream& stream)
{
 if (!d->mCells) {
	kdError() << "BosonMap::loadCells(): NULL cells" << endl;
	return false;
 }
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


bool BosonMap::saveMap(const QString& fileName, bool binary)
{
// TODO: check if file exists already
 kdDebug() << "BosonMap::saveMap()" << endl;
 if (!isValid()) {
	kdError() << "Cannot save invalid file" << endl;
	return false;
 }
// QFile file(fileName);
 if (binary) { // write a binary file
	QIODevice* dev = KFilterDev::deviceForFile(fileName, "application/x-gzip");
	if (!dev) {
		kdError() << "No file " << fileName << endl;
		return false;
	}
	if (!dev->open(IO_WriteOnly)){
		kdError() << "BosonMap: Can't open file " << fileName << endl;
		delete dev;
		return false;
	}
	QDataStream stream(dev);
 
	saveValidityHeader(stream);
	bool ret = saveMapGeo(stream);
	if (ret) {
		ret = saveCells(stream);
	}
//	file.close();
	dev->close();
	kdDebug() << "BosonMap::saveMap() done" << endl;
	delete dev;
	return ret;
 }

 QDomDocument doc("BosonMap");
 QDomElement root = doc.createElement("BosonMap");
 doc.appendChild(root);

 QDomElement geo = doc.createElement("MapGeo");
 root.appendChild(geo);

 if (!saveMapGeo(geo)) {
	kdError() << "Could not save map geo to " << fileName << endl;
	return false;
 }

 QDomElement cells = doc.createElement("MapCells");
 root.appendChild(cells);

 if (!saveCells(cells)) {
	kdError() << "Could not save cells into " << fileName << endl;
	return false;
 }

// now save the file
 QIODevice* dev = KFilterDev::deviceForFile(fileName, "application/x-gzip");
 if (!dev) {
	kdError() << "No file " << fileName << endl;
	return false;
 }
 if (!dev->open(IO_WriteOnly)){
	kdError() << "BosonMap: Can't open file " << fileName << endl;
	delete dev;
	return false;
 }
 QString xml = doc.toString();
 dev->writeBlock(xml.data(), xml.length()); // is this ok? is there a better way??
 dev->close();
 delete dev;
 return true;
}

bool BosonMap::saveMapGeo(QDomElement& node)
{
// TODO: maybe check for validity
 node.setAttribute("Width", width());
 node.setAttribute("Height", width());
 return true;
}

bool BosonMap::loadMapGeo(QDomElement& node)
{
 if (!node.hasAttribute("Width")) {
	kdError() << "Map width is mandatory!" << endl;
	return false;
 }
 if (!node.hasAttribute("Height")) {
	kdError() << "Map height is mandatory!" << endl;
	return false;
 }
 Q_INT32 width = node.attribute("Width").toInt();
 Q_INT32 height = node.attribute("Height").toInt();

// lets use the same function as for loading the binary file:
 QByteArray buffer;
 QDataStream stream(buffer, IO_WriteOnly);
 stream << width;
 stream << height;

 QDataStream readStream(buffer, IO_ReadOnly);
 return loadMapGeo(readStream);
}

bool BosonMap::loadCells(QDomElement& node)
{
 QDomNodeList list = node.elementsByTagName("Cell");
 if ((int)list.count() < width() * height()) {
	kdError() << "XML error: not enough cells" << endl;
	return false;
 }
 if ((int)list.count() != width() * height()) {
	kdWarning() << "Cell count doesn't match width * height" 
			<< endl;
 }
 
 QByteArray buffer;
 QDataStream stream(buffer, IO_WriteOnly);
 int* groundType = new int[width() * height()];
 unsigned char* version = new unsigned char[width() * height()];
 for (unsigned int i = 0; i < list.count(); i++) {
	QDomElement cell = list.item(i).toElement();
	if (cell.isNull()) {
		kdError() << "XML error: cell is not an QDomElement" << endl;
	} else {
		int x;
		int y;
		int g;
		unsigned char v;
		if (!loadCell(cell, x, y, g, v)) {
			kdError() << "XML error: could not load cell" << endl;
			continue;
		}
		groundType[x + y * width()] = g;
		version[x + y * width()] = v;
	}
 }
 for (int i = 0; i < width(); i++) {
	for (int j = 0; j < height(); j++) {
		saveCell(stream, groundType[i + j * width()], 
				version[i + j * width()]);
	}
 }

 delete[] groundType;
 delete[] version;

 QDataStream readStream(buffer, IO_ReadOnly);
 return loadCells(readStream);
}

bool BosonMap::loadCell(QDomElement& node, int& x, int& y, int& groundType, unsigned char& version)
{
 if (!node.hasAttribute("x")) {
	kdError() << "XML: attribute x is mandatory!" << endl;
	return false;
 }
 if (!node.hasAttribute("y")) {
	kdError() << "XML: attribute y is mandatory!" << endl;
	return false;
 }
 if (!node.hasAttribute("GroundType")) {
	kdError() << "XML: attribute GroundType is mandatory!" << endl;
	return false;
 }
 if (!node.hasAttribute("Version")) {
	kdError() << "XML: attribute Version is mandatory!" << endl;
	return false;
 }
 x = node.attribute("x").toInt();
 y = node.attribute("y").toInt();
 groundType = node.attribute("GroundType").toInt();
 version = (unsigned char)node.attribute("Version").toInt(); // not nice...

 if (x > width()) {
	kdError() << "XML: x > width" << endl;
	return false;
 }
 if (y > height()) {
	kdError() << "XML: y > height" << endl;
	return false;
 }
 return true;
}

bool BosonMap::saveCells(QDomElement& node)
{
 if (!d->mCells) {
	kdError() << "BosonMap::saveCells(): NULL cells" << endl;
	return false;
 }
 QDomDocument doc = node.ownerDocument();
 for (int i = 0; i < width(); i++) {
	for (int j = 0; j < height(); j++) {
		QDomElement c = doc.createElement("Cell");
		node.appendChild(c);
		saveCell(c, i, j, cell(i, j));
	}
 }
 return true;
}

bool BosonMap::saveCell(QDomElement& node, int x, int y, Cell* cell)
{
 if (!cell) {
	kdError() << "BosonMap::saveCell(): NULL cell" << endl;
	return false;
 }
 QDomDocument doc = node.ownerDocument();
 node.setAttribute("x", x);
 node.setAttribute("y", y);

// or should these be separate elements? I doubt this.
 node.setAttribute("GroundType", cell->groundType());
 node.setAttribute("Version", cell->version());
 return true;
}

bool BosonMap::saveMapGeo(QDataStream& stream)
{
 if (!isValid()) {
	kdError() << "Map geo is not valid" << endl;
	return false;
 }
 kdDebug() << "save map geo" << endl;
 stream << (Q_INT32)width();
 stream << (Q_INT32)height();
 return true;
}

bool BosonMap::saveCells(QDataStream& stream)
{
 for (int i = 0; i < width(); i++) {
	for (int j = 0; j < height(); j++) {
		Cell* c = cell(i, j);
		if (!c) {
			kdError() << "BosonMap::saveCell(): NULL Cell" << endl;
			// do not abort - otherwise all clients receiving this
			// stream are completely broken as we expect
			// width()*height() cells
			saveCell(stream, 0, 0);
		} else {
			saveCell(stream, c->groundType(), c->version());
		}
	}
 }
 return true;
}

bool BosonMap::isValid() const
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
 if (!d->mCells) {
	kdError() << "BosonMap::isValid(): NULL cells" << endl;
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
	kdError() << "invalid cell: version >= 4!" << endl;
	kdDebug() << version << endl;
 }
 groundType = g;
 b = version;

 return true;
}

void BosonMap::saveCell(QDataStream& stream, int groundType, unsigned char version)
{
 if (version > 4) {
	kdWarning() << "Invalid version " << version << endl;
	version = 0;
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

