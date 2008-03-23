/*
    This file is part of the Boson game
    Copyright (C) 2002-2008 Andreas Beckermann (b_mann@gmx.de)

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "bpfloader.h"

#include "../bomemory/bodummymemory.h"
#include "boversion.h"
#include "fileconverter/bosonplayfieldconverter.h"
#include "bodebug.h"
#include "bofile.h"
#include "bpfdescription.h"
#include "bosonplayfield.h"
#include "defines.h"

// AB: we use explicitly sharing. TODO: Qt4: use QExplicitlySharedDataPointer
class BPFPreviewPrivate : public QShared
{
public:
	BPFPreviewPrivate() : QShared()
	{
		mDescription = 0;
		mIsLoaded = false;

		mMapWidth = 0;
		mMapHeight = 0;
		mMinPlayers = 1;
		mMaxPlayers = 1;
	}
	~BPFPreviewPrivate()
	{
		delete mDescription;
	}

	bool mIsLoaded;
	BPFDescription* mDescription;
	QString mIdentifier;
	QByteArray mMapPreviewPNGData;

	unsigned int mMapWidth;
	unsigned int mMapHeight;
	unsigned int mMinPlayers;
	int mMaxPlayers;
};

BPFPreview::BPFPreview()
{
 d = new BPFPreviewPrivate;
 d->ref();
}

BPFPreview::BPFPreview(const BPFPreview& p)
{
 d = p.d;
 d->ref();
}

BPFPreview::BPFPreview(BPFPreviewPrivate* copy)
{
 d = copy;
 d->ref();
}

BPFPreview::~BPFPreview()
{
 if (d->deref()) {
	delete d;
 }
}

BPFPreview& BPFPreview::operator=(const BPFPreview& p)
{
 if (d->deref()) {
	delete d;
 }
 d = p.d;
 d->ref();
 return *this;
}

BPFPreview BPFPreview::loadFilePreviewFromFiles(const QMap<QString, QByteArray>& files)
{
 BPFPreviewPrivate* data = new BPFPreviewPrivate;
 BPFPreview previewObject(data);

 if (!files.contains("map/map.xml")) {
	boError() << k_funcinfo << "map/map.xml not found" << endl;
	return false;
 }
 if (!files.contains("players.xml")) {
	boError() << k_funcinfo << "players.xml not found" << endl;
	return false;
 }
 if (!loadFilePreviewPlayersInformation(data, files["players.xml"])) {
	boError() << k_funcinfo << "unable to load players information" << endl;
	return false;
 }
 if (!loadFilePreviewMapInformation(data, files["map/map.xml"])) {
	boError() << k_funcinfo << "unable to load map information" << endl;
	return false;
 }

 QByteArray descriptionXML = files["C/description.xml"];
 if (descriptionXML.size() == 0) {
	boError() << k_funcinfo << "Oops - NULL description file" << endl;
	return BPFPreview();
 }
 delete data->mDescription;
 data->mDescription = new BPFDescription(QString(descriptionXML));
 data->mMapPreviewPNGData = files["mappreview/map.png"];
 QByteArray identifierFile = files["identifier"];
 QDataStream identifierStream(identifierFile, IO_ReadOnly);
 identifierStream >> data->mIdentifier;

 data->mIsLoaded = true;

 return previewObject;
}

bool BPFPreview::loadFilePreviewPlayersInformation(BPFPreviewPrivate* data, const QByteArray& xml)
{
 QString errorMsg;
 int line = 0, column = 0;
 QDomDocument doc(QString::fromLatin1("Players"));
 if (!doc.setContent(QString(xml), &errorMsg, &line, &column)) {
	boError() << k_funcinfo << "unable to set XML content - error in line=" << line << ",column=" << column << " errorMsg=" << errorMsg << endl;
	return false;
 }
 QDomElement root = doc.documentElement();

 // AB: atm there is no usable way to retrieve a minplayers value. also atm we
 // don't have any use for minplayers anyway, as we don't have scenarios (i.e.
 // winning conditions) yet. so we use 1 as default.
 data->mMinPlayers = 1;

 // count the Player tags that don't have a IsNeutral attribute (maximal one
 // exists - exactly one for files created with boson >= 0.10)
 data->mMaxPlayers = 0;
 QDomNodeList list = root.elementsByTagName("Player");
 for (unsigned int i = 0; i < list.count(); i++) {
	if (list.item(i).toElement().hasAttribute("IsNeutral")) {
		continue;
	}
	data->mMaxPlayers++;
 }


 return true;
}

bool BPFPreview::loadFilePreviewMapInformation(BPFPreviewPrivate* data, const QByteArray& xml)
{
 QString errorMsg;
 int line = 0, column = 0;
 QDomDocument doc(QString::fromLatin1("BosonMap"));
 if (!doc.setContent(QString(xml), &errorMsg, &line, &column)) {
	boError() << k_funcinfo << "unable to set XML content - error in line=" << line << ",column=" << column << " errorMsg=" << errorMsg << endl;
	return false;
 }
 QDomElement root = doc.documentElement();
 QDomElement geometry = root.namedItem("Geometry").toElement();
 if (geometry.isNull()) {
	boError() << k_funcinfo << "no Geometry tag found" << endl;
	return false;
 }
 bool ok = false;
 data->mMapWidth = geometry.attribute("Width").toUInt(&ok);
 if (!ok) {
	boError() << k_funcinfo << "invalid number for Width" << endl;
	return false;
 }
 data->mMapHeight = geometry.attribute("Height").toUInt(&ok);
 if (!ok) {
	boError() << k_funcinfo << "invalid number for Width" << endl;
	return false;
 }
 return true;
}

bool BPFPreview::isLoaded() const
{
 return d->mIsLoaded;
}

const QString& BPFPreview::identifier() const
{
 return d->mIdentifier;
}

const BPFDescription* BPFPreview::description() const
{
 return d->mDescription;
}

BPFDescription* BPFPreview::description()
{
 return d->mDescription;
}

QByteArray BPFPreview::mapPreviewPNGData() const
{
 return d->mMapPreviewPNGData;
}

unsigned int BPFPreview::mapWidth() const
{
 return d->mMapWidth;
}

unsigned int BPFPreview::mapHeight() const
{
 return d->mMapHeight;
}

unsigned int BPFPreview::minPlayers() const
{
 return d->mMinPlayers;
}

int BPFPreview::maxPlayers() const
{
 return d->mMaxPlayers;
}


BPFLoader::BPFLoader()
{
}

BPFLoader::~BPFLoader()
{
}

BPFPreview BPFLoader::loadFilePreview(const QString& file)
{
 QMap<QString, QByteArray> files;
 if (!BPFLoader::loadFilePreviewFromDiskToFiles(file, files)) {
	return BPFPreview();;
 }
 return BPFPreview::loadFilePreviewFromFiles(files);
}

bool BPFLoader::loadFilePreviewFromDiskToFiles(const QString& file, QMap<QString,QByteArray>& destFiles)
{
 BPFFile boFile(file, true);
 if (!boFile.checkTar()) {
	boError() << k_funcinfo << "Oops - broken file " << file << endl;
	return false;
 }

 QByteArray mapXML;
 QByteArray playersXML;
 if (!boFile.hasMapDirectory()) {
	boError() << k_funcinfo << "file format not supported" << endl;
	return false;
 } else {
	// file format is >= boson 0.9
	mapXML = boFile.mapXMLData();
	playersXML = boFile.playersData();
 }

 QMap<QString, QByteArray> descriptions = boFile.descriptionsData();
 QByteArray descriptionXML = descriptions["C/description.xml"];
 if (!boFile.hasMapDirectory() && descriptions["C/description.xml"].size() == 0) {
	boError() << k_funcinfo << "old savegame format not supported." << endl;
	return false;
 }
 if (!descriptions.contains("C/description.xml")) {
	boError() << k_funcinfo << "no C/description.xml file found" << endl;
	return false;
 }
 QByteArray mapPreviewPNG = boFile.fileData("map.png", "mappreview");


 destFiles.insert("map/map.xml", mapXML);
 destFiles.insert("players.xml", playersXML);
 if (mapPreviewPNG.size() != 0) {
	// mappreview is optional. it's being displayed in the newgame widget.
	destFiles.insert("mappreview/map.png", mapPreviewPNG);
 }
 for (QMap<QString, QByteArray>::iterator it = descriptions.begin(); it != descriptions.end(); ++it) {
	destFiles.insert(it.key(), it.data());
 }

 destFiles.insert("identifier", createIdentifier(boFile, destFiles));


#warning TODO: convert file to current format (if it isnt already)

 return true;
}


bool BPFLoader::loadFromDiskToFiles(const QString& fileName, QMap<QString, QByteArray>& destFiles)
{
 if (!destFiles.isEmpty()) {
	boError() << k_funcinfo << "destFiles must be empty" << endl;
	return false;
 }
 BPFFile boFile(fileName, true);
 QByteArray heightMap = boFile.heightMapData();
 QByteArray texMap = boFile.texMapData();
 QByteArray mapXML = boFile.mapXMLData();
 QByteArray waterXML = boFile.waterXMLData();
 QByteArray playersXML = boFile.playersData();
 QByteArray canvasXML = boFile.canvasData();
 QByteArray kgameXML = boFile.kgameData();
 QByteArray mapPreviewPNG = boFile.fileData("map.png", "mappreview");
 if (!boFile.hasMapDirectory()) {
	boError() << k_funcinfo << "file has no map directory - file format too old (< Boson 0.9). Cannot convert this." << endl;
	return false;
 }

 if (texMap.size() == 0) {
	boError() << k_funcinfo << "empty texmap" << endl;
	return false;
 }
 if (heightMap.size() == 0) {
	boError() << k_funcinfo << "empty heightMap" << endl;
	return false;
 }
 if (mapXML.size() == 0) {
	boError() << k_funcinfo << "empty mapXML" << endl;
	return false;
 }
 if (kgameXML.size() == 0) {
	boError() << k_funcinfo << "empty kgameXML" << endl;
	return false;
 }

 QMap<QString, QByteArray> descriptions = boFile.descriptionsData();
 if (!descriptions.contains("C/description.xml")) {
	boError() << k_funcinfo << "no C/description.xml file found" << endl;
	return false;
 }
 if (descriptions["C/description.xml"].size() == 0) {
	boError() << k_funcinfo << "empty default description.xml" << endl;
	return false;
 }
 for (QMap<QString, QByteArray>::iterator it = descriptions.begin(); it != descriptions.end(); ++it) {
	destFiles.insert(it.key(), it.data());
 }

 QMap<QString, QByteArray> scripts = boFile.scriptsData();
 for (QMap<QString, QByteArray>::iterator it = scripts.begin(); it != scripts.end(); ++it) {
	destFiles.insert(it.key(), it.data());
 }

 QMap<QString, QByteArray> eventListener = boFile.eventListenerData();
 for (QMap<QString, QByteArray>::iterator it = eventListener.begin(); it != eventListener.end(); ++it) {
	destFiles.insert(it.key(), it.data());
 }

 QByteArray externalXML = boFile.externalData();
 destFiles.insert("map/texmap", texMap);
 destFiles.insert("map/heightmap.png", heightMap);
 destFiles.insert("map/map.xml", mapXML);
 destFiles.insert("map/water.xml", waterXML);
 destFiles.insert("players.xml", playersXML);
 destFiles.insert("canvas.xml", canvasXML);
 destFiles.insert("kgame.xml", kgameXML);
 if (externalXML.size() != 0) {
	// AB: externalXML is optional only. only for loading games.
	destFiles.insert("external.xml", externalXML);
 }
 if (mapPreviewPNG.size() != 0) {
	// mappreview is optional. it's being displayed in the newgame widget.
	destFiles.insert("mappreview/map.png", mapPreviewPNG);
 }
 destFiles.insert("identifier", createIdentifier(boFile, destFiles));

 BosonPlayFieldConverter converter;
 if (!converter.convertFilesToCurrentFormat(destFiles)) {
	boError() << k_funcinfo << "conversion to current file format failed" << endl;
	return false;
 }


 if (destFiles.isEmpty()) {
	boWarning() << k_funcinfo << "failed loading playfield from file" << endl;
	return false;
 }
 return true;
}

QByteArray BPFLoader::loadFromDiskToStream(const QString& file)
{
 QMap<QString, QByteArray> files;
 if (!BPFLoader::loadFromDiskToFiles(file, files)) {
	boError() << k_funcinfo << "could not load playfield from disk" << endl;
	return QByteArray();
 }

 return streamFiles(files);
}

QByteArray BPFLoader::streamFiles(const QMap<QString, QByteArray>& files)
{
 QByteArray buffer;
 if (files.isEmpty()) {
	boError() << k_funcinfo << "nothing to stream" << endl;
	return buffer;
 }
 QDataStream stream(buffer, IO_WriteOnly);
 stream << QCString("boplayfield");
 stream << (Q_UINT32)0x00; // version tag. probably not needed AB: maybe use BO_ADMIN_STREAM_VERSION
 stream << files;
 stream << QCString("boplayfield_end");
 return buffer;
}

bool BPFLoader::unstreamFiles(QMap<QString, QByteArray>& files, const QByteArray& buffer)
{
 QDataStream stream(buffer, IO_ReadOnly);
 // magic cookie
 QCString magic;
 QCString magicEnd;
 Q_UINT32 version;
 stream >> magic;
 if (magic != QCString("boplayfield")) {
	boError() << k_funcinfo << "magic cookie does not match" << endl;
	return false;
 }
 stream >> version;
 if (version != 0x00) {
	boError() << k_funcinfo << "invalid version" << endl;
	return false;
 }
 stream >> files;
 stream >> magicEnd;
 if (magicEnd != QCString("boplayfield_end")) {
	boError() << k_funcinfo << "magic end-cookie does not match" << endl;
	return false;
 }
 return true;
}


QByteArray BPFLoader::createIdentifier(const BPFFile& boFile, const QMap<QString, QByteArray>& files)
{
 Q_UNUSED(files); // currently not used. it is atm unsure if we will use it at some point
 // FIXME: currently we use the filename as identifier, however since playfields can
 //        be in subdirectories, this is NOT a unique identifier.
 //        maybe use MD5 of file?
 QByteArray buffer;
 QDataStream stream(buffer, IO_WriteOnly);
 stream << QString(boFile.identifier());
 return buffer;
}

