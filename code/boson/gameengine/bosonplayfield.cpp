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

#include "bosonplayfield.h"

#include "../bomemory/bodummymemory.h"
#include "boversion.h"
#include "bosonmap.h"
#include "fileconverter/bosonplayfieldconverter.h"
#include "bosondata.h"
#include "bodebug.h"
#include "bofile.h"
#include "bosonprofiling.h"
#include "bosoncampaign.h"
#include "bpfdescription.h"
#include "defines.h"

#include <qdom.h>
#include <qdatastream.h>
#include <qfileinfo.h>
#include <qfile.h>

#include <kstandarddirs.h>
#include <klocale.h>

#include "bosonplayfield.moc"


// the version of the stream that the ADMIN sends to start a game
#define BO_ADMIN_STREAM_VERSION (Q_UINT32)0x01


class BosonPlayFieldData : public BosonDataObject
{
public:
	BosonPlayFieldData(const QString& mapFile, BosonPlayField* field);

	virtual ~BosonPlayFieldData()
	{
		delete mPlayField;
	}

	virtual QString idString() const
	{
		return mId;
	}
	virtual void* pointer() const
	{
		return (void*)playField();
	}
	BosonPlayField* playField() const
	{
		return mPlayField;
	}

	virtual bool load();
private:
	BosonPlayField* mPlayField;
	QString mId;
};

BosonPlayFieldData::BosonPlayFieldData(const QString& mapFile, BosonPlayField* field)
	: BosonDataObject(mapFile)
{
 mPlayField = field;
 if (!mPlayField->preLoadPlayField(mapFile)) {
	boError() << k_funcinfo << "unable to load playField from " << mapFile << endl;
	mId = QString::null;
	return;
 }
 mId = mPlayField->identifier();
 if (mId.isEmpty()) {
	boError() << k_funcinfo << "no identifier in " << mapFile << endl;
	mId = QString::null;
	return;
 }
}

bool BosonPlayFieldData::load()
{
 return true;
}



BosonPlayFieldInformation::BosonPlayFieldInformation()
{
 mMapWidth = 0;
 mMapHeight = 0;
 mMinPlayers = 1;
 mMaxPlayers = 1;
}

BosonPlayFieldInformation::~BosonPlayFieldInformation()
{
}

bool BosonPlayFieldInformation::loadInformation(BPFFile* file)
{
 if (!file) {
	BO_NULL_ERROR(file);
	return false;
 }
 QByteArray mapXML;
 QByteArray playersXML;
 if (!file->hasMapDirectory()) {
	boError() << k_funcinfo << "file format not supported" << endl;
	return false;
 } else {
	// file format is >= boson 0.9
	mapXML = file->mapXMLData();
	playersXML = file->playersData();
 }
 QMap<QString, QByteArray> files;
 files.insert("map/map.xml", mapXML);
 files.insert("players.xml", playersXML);
 return loadInformation(files);
}

// AB: note that files must contain the files in the _current_ format. no
// conversion happens here
bool BosonPlayFieldInformation::loadInformation(const QMap<QString, QByteArray>& files)
{
 QByteArray mapXML = files["map/map.xml"];
 QByteArray playersXML = files["players.xml"];
 if (!loadPlayersInformation(playersXML)) {
	boError() << k_funcinfo << "unable to load players information" << endl;
	return false;
 }
 if (!loadMapInformation(mapXML)) {
	boError() << k_funcinfo << "unable to load map information" << endl;
	return false;
 }

 return true;
}

bool BosonPlayFieldInformation::loadPlayersInformation(const QByteArray& xml)
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
 mMinPlayers = 1;

 // count the Player tags that don't have a IsNeutral attribute (maximal one
 // exists - exactly one for files created with boson >= 0.10)
 mMaxPlayers = 0;
 QDomNodeList list = root.elementsByTagName("Player");
 for (unsigned int i = 0; i < list.count(); i++) {
	if (list.item(i).toElement().hasAttribute("IsNeutral")) {
		continue;
	}
	mMaxPlayers++;
 }


 return true;
}

bool BosonPlayFieldInformation::loadMapInformation(const QByteArray& xml)
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
 mMapWidth = geometry.attribute("Width").toUInt(&ok);
 if (!ok) {
	boError() << k_funcinfo << "invalid number for Width" << endl;
	return false;
 }
 mMapHeight = geometry.attribute("Height").toUInt(&ok);
 if (!ok) {
	boError() << k_funcinfo << "invalid number for Width" << endl;
	return false;
 }
 return true;
}


BosonPlayField::BosonPlayField(QObject* parent) : QObject(parent, "BosonPlayField")
{
 mMap = 0;
 mPreLoaded = false;
 mPlayFieldInformation = new BosonPlayFieldInformation();
 mDescription = new BPFDescription();
}

BosonPlayField::~BosonPlayField()
{
 delete mMap;
 delete mDescription;
 delete mPlayFieldInformation;
}

bool BosonPlayField::preLoadAllPlayFields()
{
 boDebug() << k_funcinfo << endl;

 // TODO: ensure that UI doesn't block (i.e. call process events)
 BosonProfiler profiler("Preload all playfields");
 if (BosonData::bosonData()->availablePlayFields().count() > 0) {
	boWarning() << k_funcinfo << "playFields already loaded" << endl;
	return true;
 }
 QStringList campaigns = findAvailableCampaigns();
 QStringList::Iterator campaignIt;
 for (campaignIt = campaigns.begin(); campaignIt != campaigns.end(); ++campaignIt) {
	if (BosonData::bosonData()->availableCampaigns().contains(*campaignIt)) {
		if ((*campaignIt).isEmpty()) {
			boError() << k_funcinfo << "trying to insert default campaing (random maps) twice!" << endl;
		} else {
			boError() << k_funcinfo << "trying to insert campaign " << *campaignIt << " twice!" << endl;
		}
		continue;
	}
	QStringList list = findPlayFieldsOfCampaign(*campaignIt);
	QStringList::Iterator it;
	QString campaignName = *campaignIt;
	if ((*campaignIt).isEmpty()) {
		campaignName = i18n("Boson"); // default campaign
	}
	BosonCampaign* campaign = new BosonCampaign(*campaignIt, campaignName);
	for (it = list.begin(); it != list.end(); ++it) {
		BosonPlayField* playField = new BosonPlayField();
		// this will also preload the playfield!
		BosonPlayFieldData* data = new BosonPlayFieldData(*it, playField);
		if (data->idString().isEmpty()) {
			boError() << k_funcinfo << *it << " could not be loaded" << endl;
			delete data;
			continue;
		}
		if (!BosonData::bosonData()->insertPlayField(data)) {
			boWarning() << k_funcinfo << "could not insert playField "
					<< data->idString()
					<< " (maybe already inserted)" << endl;
			delete data;
			continue;
		}
		campaign->addPlayField(playField);
	}
	if (campaign->playFieldCount() == 0) {
		boWarning() << k_funcinfo << "could not load any playfields for campaign " << campaign->name() << endl;
	}
	if (!BosonData::bosonData()->insertCampaign(BosonCampaign::campaignDataObject(campaign))) {
		boWarning() << k_funcinfo << "error on inserting campaign " << *campaignIt << endl;
	}
 }

 if (BosonData::bosonData()->availablePlayFields().count() == 0) {
	boError() << k_funcinfo << "no playFields could be loaded!" << endl;
	return false;
 }
 return true;
}

void BosonPlayField::clearAllPreLoadedPlayFields()
{
boWarning() << k_funcinfo << "obsolete" << endl;
#if 0
 if (!mPlayFields) {
	return;
 }
 mPlayFields->clear();
#endif
}

QString BosonPlayField::defaultPlayField()
{
 QString _default = "weareunderattack.bpf";

 QStringList l = boData->availablePlayFields();
 if (l.contains(_default)) {
	return _default;
 }
 if (l.count() == 0) {
	return QString::null;
 }
 boWarning() << k_funcinfo << "cannot find " << _default<< " map - using " << l[0] << " instead" << endl;
 return l[0];
}

bool BosonPlayField::preLoadPlayField(const QString& file)
{
 if (isPreLoaded()) {
	return true;
 }
 BPFFile boFile(file, true);
 if (!boFile.checkTar()) {
	boError() << k_funcinfo << "Oops - broken file " << file << endl;
	return false;
 }
 if (!mPlayFieldInformation->loadInformation(&boFile)) {
	boError() << k_funcinfo << "Could not load playfield information" << endl;
	return false;
 }
 QMap<QString, QByteArray> descriptions = boFile.descriptionsData();
 QByteArray descriptionXML = descriptions["C/description.xml"];
 if (descriptionXML.size() == 0 && !boFile.hasMapDirectory()) {
	boError() << k_funcinfo << "old savegame format not supported." << endl;
	return false;
 }
 if (!loadDescriptionFromFile(descriptionXML)) {
	boError() << k_funcinfo << "Could not load description file" << endl;
	return false;
 }
 mIdentifier = boFile.identifier();
 mMapPreviewPNGData = boFile.fileData("map.png", "mappreview");

 mFileName = file;
 mPreLoaded = true;

 return true;
}

// AB: correct (i.e. current) file format assumed. must be converted before this
// is called.
bool BosonPlayField::loadPlayField(const QMap<QString, QByteArray>& files)
{
 if (!files.contains("map/map.xml")) {
	boError() << k_funcinfo << "no map.xml found" << endl;
	return false;
 }
 if (!files.contains("map/heightmap.png")) {
	boError() << k_funcinfo << "no heightmap found" << endl;
	return false;
 }
 if (!files.contains("map/texmap")) {
	boError() << k_funcinfo << "no texmap found" << endl;
	return false;
 }
 if (!files.contains("map/water.xml")) {
	boError() << k_funcinfo << "no water.xml found" << endl;
	return false;
 }

 // AB: C/description.xml is mandatory. all other languages are optional.
 if (!files.contains("C/description.xml")) {
	boError() << k_funcinfo << "no default description.xml found" << endl;
	return false;
 }

 if (!mPlayFieldInformation->loadInformation(files)) {
	boError() << k_funcinfo << "Could not load playfield information" << endl;
	return false;
 }
 if (!loadMapFromFiles(files)) {
	boError() << k_funcinfo << "error loading the map" << endl;
	return false;
 }
 if (!loadDescriptionFromFile(files["C/description.xml"])) {
	boError() << k_funcinfo << "error loading the default description" << endl;
	return false;
 }

 // here we might parse files for keys that end with "description.xml". all
 // languages should be added that are present.

 // when we have winning conditions one day (probably in "scenario.xml"), we
 // should load it here as well.

 return true;
}

bool BosonPlayField::loadDescriptionFromFile(const QByteArray& xml)
{
 if (xml.size() == 0) {
	boError() << k_funcinfo << "Oops - NULL description file" << endl;
	return false;
 }
 delete mDescription;
 mDescription = new BPFDescription(QString(xml));
 return true;
}

bool BosonPlayField::loadMapFromFiles(const QMap<QString, QByteArray>& files)
{
 delete mMap;
 mMap = new BosonMap(this);
 if (!mMap->loadMapFromFiles(files)) {
	boError() << k_funcinfo << "unable to load map" << endl;
	return false;
 }
 return true;
}

QString BosonPlayField::saveDescriptionToFile() const
{
 if (!mDescription) {
	BO_NULL_ERROR(mDescription);
	return QString::null;
}
 return mDescription->toString();
}

QByteArray BosonPlayField::saveMapPreviewPNGToFile() const
{
 if (!mMap) {
	boError() << k_funcinfo << "NULL map" << endl;
	return QByteArray();
 }
 return mMap->saveMapPreviewPNGToFile();
}


void BosonPlayField::quit()
{
 delete mMap;
 mMap = 0;
}

void BosonPlayField::changeMap(BosonMap* m)
{
 delete mMap;
 mMap = m;
}

void BosonPlayField::changeDescription(BPFDescription* d)
{
 delete mDescription;
 mDescription = d;
}

bool BosonPlayField::modified() const
{
 if (mMap && mMap->modified()) {
	return true;
 }
 return false;
}

void BosonPlayField::deleteMap()
{
 delete mMap;
 mMap = 0;
}

void BosonPlayField::finalizeLoading()
{
 mPreLoaded = true;
}

QString BosonPlayField::playFieldName() const
{
 if (!mDescription) {
	BO_NULL_ERROR(mDescription);
	return QString::null;
 }
 return mDescription->name();
}

QString BosonPlayField::playFieldComment() const
{
 if (!mDescription) {
	BO_NULL_ERROR(mDescription);
	return QString::null;
 }
 return mDescription->comment();
}

bool BosonPlayField::importHeightMapImage(const QImage& image)
{
 if (!mMap) {
	BO_NULL_ERROR(mMap);
	return false;
 }
 return mMap->importHeightMapImage(image);
}

QByteArray BosonPlayField::exportHeightMap() const
{
 if (!mMap) {
	BO_NULL_ERROR(mMap);
	return QByteArray();
 }
 return mMap->saveHeightMapImage();
}

QByteArray BosonPlayField::exportTexMap(unsigned int texture) const
{
 if (!mMap) {
	BO_NULL_ERROR(mMap);
	return QByteArray();
 }
 return mMap->saveTexMapImage(texture);
}

QByteArray BosonPlayField::mapPreviewPNGData() const
{
 return mMapPreviewPNGData;
}


QStringList BosonPlayField::findAvailablePlayFields()
{
 QStringList list = BosonData::availableFiles(QString::fromLatin1("maps/*.bpf"));
 QStringList campaignMaps = BosonData::availableFiles(QString::fromLatin1("maps/*/*.bpf"));
 list += campaignMaps;
 return list;
}

QStringList BosonPlayField::findAvailableCampaigns()
{
 QStringList list;
 list.append(QString::fromLatin1(""));
 QStringList files = BosonData::availableFiles(QString::fromLatin1("maps/*/*.bpf"));
 QStringList::Iterator it = files.begin();
 for (; it != files.end(); ++it) {
	int slash = (*it).findRev('/');
	QString path = (*it).left(slash);
	slash = path.findRev('/');

	// WARNING: here we assume one directory level only! it must be
	// maps/*/*.bpf, _NOT_ maps/*/*/*.bpf or similar.
	QString campaign = path.mid(slash + 1);
	if (!list.contains(campaign)) {
		list.append(campaign);
	}
 }
 return list;
}

QStringList BosonPlayField::findPlayFieldsOfCampaign(const QString& campaign)
{
 // at this point we assume that "campaign" is the name of the campaign
 // directory! we do not (yet?) use an identifier here!
 QStringList list;
 if (campaign.isEmpty()) {
	list = BosonData::availableFiles(QString::fromLatin1("maps/*.bpf"));
 } else {
	list = BosonData::availableFiles(QString::fromLatin1("maps/%1/*.bpf").arg(campaign));
 }
 return list;
}

// load from harddisk to virtual files in the QMap.
bool BosonPlayField::loadFromDiskToFiles(QMap<QString, QByteArray>& destFiles)
{
 if (!destFiles.isEmpty()) {
	boError() << k_funcinfo << "destFiles must be empty" << endl;
	return false;
 }
 if (!isPreLoaded()) {
	boError() << k_funcinfo << "playfield not yet preloaded" << endl;
	return false;
 }
 BPFFile boFile(mFileName, true);
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

QByteArray BosonPlayField::loadFromDiskToStream(QMap<QString, QByteArray>* destFiles)
{
 if (destFiles && !destFiles->isEmpty()) {
	boError() << k_funcinfo << "destFiles must be empty" << endl;
	return QByteArray();
 }
 if (!isPreLoaded()) {
	boError() << k_funcinfo << "playfield not yet preloaded" << endl;
	return QByteArray();
 }
 QMap<QString, QByteArray> files;
 if (!loadFromDiskToFiles(files)) {
	boError() << k_funcinfo << "could not load playfield from disk" << endl;
	return QByteArray();
 }

 if (destFiles) {
	*destFiles = files;
 }
 return streamFiles(files);
}

QByteArray BosonPlayField::streamFiles(const QMap<QString, QByteArray>& files)
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

bool BosonPlayField::unstreamFiles(QMap<QString, QByteArray>& files, const QByteArray& buffer)
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

bool BosonPlayField::savePlayFieldToFiles(QMap<QString, QByteArray>& files)
{
 if (!mMap) {
	BO_NULL_ERROR(mMap);
	return false;
 }
 if (!mDescription) {
	BO_NULL_ERROR(mDescription);
	return false;
 }
 QMap<QString, QByteArray> mapFiles = mMap->saveMapToFiles();
 if (mapFiles.isEmpty()) {
	boError() << k_funcinfo << "saving the map failed" << endl;
	return false;
 }

 // AB: map/map.xml contains the file format version of the map, so it is
 //     absolutely mandatory
 if (!mapFiles.contains("map/map.xml")) {
	boError() << k_funcinfo << "map/map.xml was not saved" << endl;
	return false;
 }

 // TODO: _all_ descriptions, not just default one
 QByteArray descriptionXML = saveDescriptionToFile().utf8();
 if (descriptionXML.size() == 0) {
	boError() << k_funcinfo << "saving description failed" << endl;
	return false;
 }

 QByteArray mapPreviewPNG;
 mapPreviewPNG = saveMapPreviewPNGToFile();
 if (mapPreviewPNG.size() == 0) {
	boError() << k_funcinfo << "saving map preview failed" << endl;
	return false;
 }


 for (QMap<QString,QByteArray>::iterator it = mapFiles.begin(); it != mapFiles.end(); ++it) {
	files.insert(it.key(), it.data());
 }
 files.insert("mappreview/map.png", mapPreviewPNG);
 files.insert("C/description.xml", descriptionXML);
 return true;
}


