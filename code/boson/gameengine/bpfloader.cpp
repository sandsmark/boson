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
		mPlayFieldInformation = 0;
		mDescription = 0;
		mIsLoaded = false;
	}
	~BPFPreviewPrivate()
	{
		delete mPlayFieldInformation;
		delete mDescription;
	}

	bool mIsLoaded;
	BosonPlayFieldInformation* mPlayFieldInformation;
	BPFDescription* mDescription;
	QString mIdentifier;
	QByteArray mMapPreviewPNGData;
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

bool BPFPreview::isLoaded() const
{
 return d->mIsLoaded;
}

const QString& BPFPreview::identifier() const
{
 return d->mIdentifier;
}

const BosonPlayFieldInformation* BPFPreview::information() const
{
 return d->mPlayFieldInformation;
}

const BPFDescription* BPFPreview::description() const
{
 return d->mDescription;
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
 if (!loadFilePreviewFromDiskToFiles(file, files)) {
	return BPFPreview();;
 }
 return loadFilePreviewFromFiles(files);
}

bool BPFLoader::loadFilePreviewFromDiskToFiles(const QString& file, QMap<QString,QByteArray>& destFiles)
{
 BPFFile boFile(file, true);
 if (!boFile.checkTar()) {
	boError() << k_funcinfo << "Oops - broken file " << file << endl;
	return false;
 }

 QByteArray mapXML; // for BosonPlayFieldInformation
 QByteArray playersXML; // for BosonPlayFieldInformation
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

BPFPreview BPFLoader::loadFilePreviewFromFiles(const QMap<QString, QByteArray>& files)
{
 BPFPreviewPrivate* data = new BPFPreviewPrivate;
 BPFPreview previewObject(data);

 if (!data->mPlayFieldInformation->loadInformation(files)) {
	boError() << k_funcinfo << "Could not load playfield information" << endl;
	return BPFPreview();
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

