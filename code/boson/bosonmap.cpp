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

#include "bosonmap.h"

#include "cell.h"

#include <qfile.h>
#include <qdatastream.h>
#include <qdom.h>

#include <kdebug.h>
#include <kstandarddirs.h>
#include <kfilterdev.h>
#include <ksimpleconfig.h>

#include "defines.h"

#include "bosonmap.moc"

#define TAG_FIELD "boeditor_magic_0_6"
#define TAG_FIELD_LEN 18 // len of above
#define TAG_CELL (0xde)

class BosonMap::BosonMapPrivate
{
public:
	BosonMapPrivate()
	{
		mCells = 0;
	}
	
	QString mFileName;

	Cell* mCells;

	bool mIsChanged;
};

BosonMap::BosonMap(QObject* parent) : QObject(parent)
{
 init();
}

BosonMap::BosonMap(const QString& fileName, QObject* parent) : QObject(parent)
{
 init();
 loadMap(fileName);
}

BosonMap::~BosonMap()
{
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
//TODO create a list of all maps (search for .desktop files in boson/map/) and
//look if list contains "basic" - then check whether basic.bpf exists and return. 
// Otherwise look for the first .desktop file which has a .bpf file exisiting.
// Return that then.
// UPDATE (01/12/22): only the identifier should be returned.

 return QString::fromLatin1("Basic");
}

bool BosonMap::loadMap(const QString& fileName)
{
// kdDebug() << k_funcinfo << endl;
 d->mFileName = fileName; // probably obsolete?
 
 // open stream 
 QIODevice* dev = KFilterDev::deviceForFile(fileName);
 if (!dev) {
	kdError() << k_funcinfo << ": No file " << fileName << endl;
	return false;
 }
 if (!dev->open(IO_ReadOnly)){
	kdError() << k_funcinfo << ": Can't open file " << fileName << endl;
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
//	kdError() << k_funcinfo << ": Magic doesn't match(len), check file name" << endl;// not an error - probably an XML file
	return false;
 }

 for (i = 0; i < TAG_FIELD_LEN + 1; i++) {
	Q_INT8  b;
	stream >> b;
	magic[i] = b;
 }

 if (strncmp(magic, TAG_FIELD, TAG_FIELD_LEN) ) {
//	kdError() << k_funcinfo << ": Magic doesn't match(string), check file name" << endl;
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
	kdError() << k_funcinfo << ": broken map file!" << endl;
	kdError() << "mapWidth < 10" << endl;
	return false;
 }
 if (mapHeight < 10) {
	kdError() << k_funcinfo << ": broken map file!" << endl;
	kdError() << "mapHeight < 10" << endl;
	return false;
 }
 if (mapWidth > MAX_MAP_WIDTH) {
	kdError() << k_funcinfo << ": broken map file!" << endl;
	kdError() << "mapWidth > " << MAX_MAP_WIDTH << endl;
	return false;
 }
 if (mapHeight > MAX_MAP_HEIGHT) {
	kdError() << k_funcinfo << ": broken map file!" << endl;
	kdError() << "mapHeight > " << MAX_MAP_HEIGHT << endl;
	return false;
 }

// map is ok - lets apply
 mMapWidth = mapWidth; // horizontal cell count
 mMapHeight = mapHeight; // vertical cell count

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
	kdError() << k_funcinfo << ": NULL cells" << endl;
	return false;
 }
// load all cells:
 for (unsigned int i = 0; i < width(); i++) {
	for (unsigned int j = 0; j < height(); j++) {
		int groundType = 0;
		unsigned char version = 0;
		if (!loadCell(stream, groundType, version)) {
			return false;
		}
		Cell* c = cell(i, j);
		if (!c) {
			kdError() << k_funcinfo << ": NULL cell" << endl;
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
// kdDebug() << k_funcinfo << endl;
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
		kdError() << k_funcinfo << ": Can't open file " << fileName 
				<< endl;
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
	kdDebug() << k_funcinfo << " done" << endl;
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
	kdError() << k_funcinfo << ": Can't open file " << fileName << endl;
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
 node.setAttribute("Height", height());
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
 if (list.count() < width() * height()) {
	kdError() << "XML error: not enough cells" << endl;
	return false;
 }
 if (list.count() != width() * height()) {
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
 for (unsigned int i = 0; i < width(); i++) {
	for (unsigned int j = 0; j < height(); j++) {
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

 if (x >= (int)width()) {
	kdError() << k_lineinfo << ": x >= width" << endl;
	return false;
 }
 if (y >= (int)height()) {
	kdError() << k_lineinfo << ": y >= height" << endl;
	return false;
 }
 return true;
}

bool BosonMap::saveCells(QDomElement& node)
{
 if (!d->mCells) {
	kdError() << k_funcinfo << ": NULL cells" << endl;
	return false;
 }
 QDomDocument doc = node.ownerDocument();
 for (unsigned int i = 0; i < width(); i++) {
	for (unsigned int j = 0; j < height(); j++) {
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
	kdError() << k_funcinfo << ": NULL cell" << endl;
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
// kdDebug() << k_funcinfo << endl;
 stream << (Q_INT32)width();
 stream << (Q_INT32)height();
 return true;
}

bool BosonMap::saveCells(QDataStream& stream)
{
 for (unsigned int i = 0; i < width(); i++) {
	for (unsigned int j = 0; j < height(); j++) {
		Cell* c = cell(i, j);
		if (!c) {
			kdError() << k_funcinfo << ": NULL Cell" << endl;
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
	kdError() << k_funcinfo << ": NULL cells" << endl;
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
	kdError() << k_funcinfo << ": broken map file!" << endl;
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
	kdError() << k_funcinfo << ": broken map file!" << endl;
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

Cell* BosonMap::cell(int x, int y) const
{
 if (!d->mCells) {
	kdError() << "Cells not yet created" << endl;
	return 0;
 }
 if (x < 0 || x >= (int)width()) {
	kdWarning() << "Invalid Cell! x=" << x << endl;
	return 0;
 }
 if (y < 0 || y >= (int)height()) {
	kdWarning() << "Invalid Cell! y=" << y << endl;
	return 0;
 }
 return &d->mCells[ x + y * width() ];
}

QStringList BosonMap::availableMaps()
{
 QStringList list = KGlobal::dirs()->findAllResources("data", 
		"boson/map/*.desktop");
 if (list.isEmpty()) {
	kdError() << "Cannot find any map?!" << endl;
	return list;
 }
 QStringList validList;
 for (unsigned int i = 0; i < list.count(); i++) {
	QString fileName = list[i].left(list[i].length() -  strlen(".desktop"));
	fileName += QString::fromLatin1(".bpf");
	if (QFile::exists(fileName)) {
		validList.append(list[i]);
	}
 }
 return validList;
}

void BosonMap::changeCell(int x, int y, int groundType, unsigned char b)
{
//kdDebug() << x << " -> " << y << endl;
//kdDebug() << width() << " " << height() << endl;
 Cell* c = cell(x, y);
 if (!c) {
	kdError() << "Invalid cell x=" << x << ",y=" << y << endl;
	return;
 }
 if (c->groundType() != groundType || c->version() != b) {
	d->mIsChanged = false;
	c->makeCell(groundType, b);
 }
}

QString BosonMap::mapFileName(const QString& identifier)
{
 QStringList l = availableMaps();
 for (unsigned int i = 0; i < l.count(); i++) {
	KSimpleConfig cfg(l[i]);
	cfg.setGroup("Boson Map");
	if (cfg.readEntry("Identifier") == identifier) {
		QString m = l[i].left(l[i].length() - strlen(".desktop"));
		m += QString::fromLatin1(".bpf");
		if (QFile::exists(m)) {
			return m;
		} else {
			kdError() << "Cannot find " << m << " for valid .desktop file" << endl;
		}
	}
 }
 kdWarning() << "no map file found for " << identifier << endl;
 return QString::null;
}
