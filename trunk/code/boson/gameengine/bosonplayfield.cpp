/*
    This file is part of the Boson game
    Copyright (C) 2002-2005 Andreas Beckermann (b_mann@gmx.de)

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

#include "bosonplayfield.h"

#include "../bomemory/bodummymemory.h"
#include "boversion.h"
#include "bosonmap.h"
#include "bosonfileconverter.h"
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
 if (!file->hasMapDirectory() && file->hasFile("scenario.xml")) {
	// a missing map/ directory means that we have an old file. convert it
	// to out new format.
	// this might be pretty slow!
	boWarning() << k_funcinfo << "loading old file format in " << file->fileName() << endl;
	QByteArray scenarioXML = file->scenarioData();
	BosonFileConverter converter;
	QByteArray canvasXML;
	QByteArray kgameXML;
	if (!converter.convertScenario_From_0_8_To_0_9(scenarioXML, &playersXML, &canvasXML, &kgameXML)) {
		boError() << k_funcinfo << "failed converting from 0.8 file format" << endl;
		return false;
	}
	if (playersXML.size() == 0) {
		boError() << k_funcinfo << "empty playersXML after successfull conversion. code bug?" << endl;
		return false;
	}

	mapXML = file->mapXMLData();
	if (mapXML.size() == 0) {
		QByteArray map = file->mapData();
		if (map.size() == 0) {
			boError() << k_funcinfo << "neither map nor map.xml found. broken file." << endl;
			return false;
		}
		if (!file->hasFile("texmap")) {
			boWarning() << k_funcinfo << "converting map from boson 0.8 - this will take some time!" << endl;
			QByteArray texMap;
			if (!converter.convertMapFile_From_0_8_To_0_9(map, &mapXML, &texMap)) {
				boError() << k_funcinfo << "failed converting from 0.8 file" << endl;
				return false;
			}
		} else {
			boDebug() << k_funcinfo << "converting map from boson 0.8.128 to map.xml" << endl;
			if (!converter.convertMapFile_From_0_8_128_To_0_9(map, &mapXML)) {
				boError() << k_funcinfo << "failed converting from 0.8.128 file" << endl;
				return false;
			}
		}
	}
 } else if (!file->hasMapDirectory()) {
	boError() << k_funcinfo << "file format not supported" << endl;
	return false;
 } else {
	// file format is >= boson 0.9
//	boDebug() << k_funcinfo << "file format is current" << endl;
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
 boDebug() << k_funcinfo << endl;
 delete mMap;
 delete mDescription;
 delete mPlayFieldInformation;
 boDebug() << k_funcinfo << "done" << endl;
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
			boDebug() << k_funcinfo << "could not insert playField "
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
 if (!loadMapFromFile(files["map/map.xml"], files["map/heightmap.png"], files["map/texmap"], files["map/water.xml"])) {
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

bool BosonPlayField::loadMapFromFile(const QByteArray& mapXML, const QByteArray& heightMapImage, const QByteArray& texMap, const QByteArray& waterXML)
{
 boDebug() << k_funcinfo << endl;
 if (mapXML.size() == 0) {
	boError() << k_funcinfo << "empty byte array for mapXML" << endl;
	return false;
 }
 if (heightMapImage.size() == 0) {
	boError() << k_funcinfo << "empty height map array" << endl;
	return false;
 }
 if (texMap.size() == 0) {
	boError() << k_funcinfo << "empty texmap array" << endl;
	return false;
 }
 if (waterXML.size() == 0) {
	boError() << k_funcinfo << "empty byte array for waterXML" << endl;
	return false;
 }
 delete mMap;
 mMap = new BosonMap(this);
 bool ret = mMap->loadMapFromFile(mapXML);
 if (!ret) {
	boError() << k_funcinfo << "Could not load map" << endl;
	return false;
 }
// boDebug() << k_funcinfo << endl;
 boDebug() << k_funcinfo << "loading tex map" << endl;
 QDataStream texMapStream(texMap, IO_ReadOnly);
 ret = mMap->loadTexMap(texMapStream);
 if (!ret) {
	boError() << k_funcinfo << "Could not load map (texmap failed)" << endl;
	return false;
 }
 boDebug() << k_funcinfo << "generating cells" << endl;
 ret = mMap->generateCellsFromTexMap();
 if (!ret) {
	boError() << k_funcinfo << "Could not load map (cell generation failed)" << endl;
	return false;
 }

 boDebug() << k_funcinfo << "loading height map image" << endl;
 ret = mMap->loadHeightMapImage(heightMapImage);
 if (!ret) {
	boError() << k_funcinfo << "Could not load map (height map failed)" << endl;
	return false;
 }
 ret = mMap->loadWaterFromFile(waterXML);
 if (!ret) {
	boError() << k_funcinfo << "Could not load water" << endl;
	return false;
 }

 return ret;
}

QString BosonPlayField::saveDescriptionToFile() const
{
 if (!mDescription) {
	BO_NULL_ERROR(mDescription);
	return QString::null;
}
 return mDescription->toString();
}

QByteArray BosonPlayField::saveMapToFile() const
{
 if (!mMap) {
	boError() << k_funcinfo << "NULL map" << endl;
	return QByteArray();
 }
 return mMap->saveMapToFile();
}

QByteArray BosonPlayField::saveWaterToFile() const
{
 if (!mMap) {
	boError() << k_funcinfo << "NULL map" << endl;
	return QByteArray();
 }
 return mMap->saveWaterToFile();
}

QByteArray BosonPlayField::saveTexMapToFile() const
{
 if (!mMap) {
	boError() << k_funcinfo << "NULL map" << endl;
	return QByteArray();
 }
 QByteArray file;
 QDataStream stream(file, IO_WriteOnly);
 if (!mMap->saveTexMap(stream)) {
	boError() << k_funcinfo << "Error saving texmap" << endl;
	return QByteArray();
 }
 return file;
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
 boDebug() << k_funcinfo << endl;
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
	boWarning() << k_funcinfo << "need to convert from an old file" << endl;
	BosonFileConverter converter;

	// convert map first
	if (texMap.size() == 0) {
		if (!converter.convertMapFile_From_0_8_To_0_9(boFile.mapData(), &mapXML, &texMap)) {
			boError() << k_funcinfo << "failed converting from boson 0.8" << endl;
			return false;
		}
	} else {
		if (!converter.convertMapFile_From_0_8_128_To_0_9(boFile.mapData(), &mapXML)) {
			boError() << k_funcinfo << "failed converting from boson 0.8.128" << endl;
			return false;
		}
	}

	// convert scenario
	QByteArray scenario = boFile.scenarioData();
	converter.convertScenario_From_0_8_To_0_9(scenario, &playersXML, &canvasXML, &kgameXML);
	boDebug() << k_funcinfo << "conversion completed" << endl;
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

 if (!convertFilesToCurrentFormat(destFiles)) {
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
 QByteArray mapXML;
 QByteArray heightMap;
 QByteArray texMap;
 QByteArray waterXML;
 mapXML = saveMapToFile();
 if (mapXML.size() == 0) {
	boError() << k_funcinfo << "failed saving the map" << endl;
	return false;
 }
 waterXML = saveWaterToFile();
 if (waterXML.size() == 0) {
	boError() << k_funcinfo << "failed saving water" << endl;
	return false;
 }
 heightMap = mMap->saveHeightMapImage();
 if (heightMap.size() == 0) {
	boError() << k_funcinfo << "failed saving the heightmap" << endl;
	return false;
 }
 texMap = saveTexMapToFile();
 if (texMap.size() == 0) {
	boError() << k_funcinfo << "failed saving the texmap" << endl;
	return false;
 }
 boDebug() << k_funcinfo << "saving map succeeded" << endl;

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

 files.insert("map/map.xml", mapXML);
 files.insert("map/water.xml", waterXML);
 files.insert("map/heightmap.png", heightMap);
 files.insert("map/texmap", texMap);
 files.insert("mappreview/map.png", mapPreviewPNG);
 files.insert("C/description.xml", descriptionXML);
 boDebug() << k_funcinfo << "succeeded" << endl;
 return true;
}

bool BosonPlayField::convertFilesToCurrentFormat(QMap<QString, QByteArray>& destFiles)
{
 if (destFiles.isEmpty()) {
	boError() << k_funcinfo << "no files availble" << endl;
	return false;
 }
 // AB: this was added after 0.9, so at this point the files in destFiles are
 // _at least_ from boson 0.9.

 if (!destFiles.contains("map/texmap")) {
	boError() << k_funcinfo << "no map/texmap there." << endl;
	return false;
 }
 if (!destFiles.contains("map/heightmap.png")) {
	boError() << k_funcinfo << "no map/heightmap.png there." << endl;
	return false;
 }
 if (!destFiles.contains("map/map.xml")) {
	boError() << k_funcinfo << "no map/map.xml there." << endl;
	return false;
 }
 if (!destFiles.contains("players.xml")) {
	boError() << k_funcinfo << "no players.xml there." << endl;
	return false;
 }
 if (!destFiles.contains("canvas.xml")) {
	boError() << k_funcinfo << "no canvas.xml there." << endl;
	return false;
 }
 if (!destFiles.contains("kgame.xml")) {
	boError() << k_funcinfo << "no kgame.xml there." << endl;
	return false;
 }
 if (!destFiles.contains("C/description.xml")) {
	boError() << k_funcinfo << "no C/description.xml there." << endl;
	return false;
 }

 // AB: all other files are optional for boson 0.9
 QDomDocument kgameDoc(QString::fromLatin1("Boson"));
 if (!kgameDoc.setContent(QString(destFiles["kgame.xml"]))) {
	boError() << k_funcinfo << "unable to load kgame.xml" << endl;
	return false;
 }
 QDomElement kgameRoot = kgameDoc.documentElement();
 bool ok = false;
 unsigned int version = kgameRoot.attribute("Version").toUInt(&ok);
 if (!ok) {
	boError() << k_funcinfo << "Version attribute in kgame.xml is not a valid number" << endl;
	return false;
 }
 if (version < BOSON_SAVEGAME_FORMAT_VERSION_0_9) {
	boError() << k_funcinfo << "this function expects at least file format from 0.9 (" << BOSON_SAVEGAME_FORMAT_VERSION_0_9 << ") - found: " << version << endl;
	return false;
 }

 // AB: this is the point where you should insert your conversion code !
 bool handled = false;
 if (!convertFilesToCurrentFormat(destFiles, version, &handled)) {
	boError() << k_funcinfo << "conversion failed" << endl;
	return false;
 }

 if (handled) {
	// the files got converted to a different format.
	// -> call this function again and check whether we can convert any
	// further (e.g. 0.9 -> 0.9.1 in the first call, 0.9.1->0.10 in the 2nd
	// or so)

	 // first check whether the version got changed (prevent infinite loop)
	if (!kgameDoc.setContent(QString(destFiles["kgame.xml"]))) {
		boError() << k_funcinfo << "unable to load kgame.xml after conversion" << endl;
		return false;
	}
	QDomElement kgameRoot = kgameDoc.documentElement();
	ok = false;
	unsigned int newVersion = kgameRoot.attribute("Version").toUInt(&ok);
	if (!ok) {
		boError() << k_funcinfo << "Version attribute in kgame.xml is not a valid number" << endl;
		return false;
	}
	if (newVersion == version) {
		boError() << k_funcinfo << "format " << version << " got converted, but version has not been changed" << endl;
		return false;
	}

	// check whether we can convert any further
	return convertFilesToCurrentFormat(destFiles);
 }

 return true;
}

bool BosonPlayField::convertFilesToCurrentFormat(QMap<QString, QByteArray>& destFiles, unsigned int version, bool* handled)
{
 // AB: this is where the conversion should be done!
 // (of course - most actual conversion code will be in BosonFileConverter)

 *handled = true;
 bool ret = false;
 switch (version) {
	case BOSON_SAVEGAME_FORMAT_VERSION_0_9:
	{
		boDebug() << k_funcinfo << "converting from 0.9 to 0.9.1 format" << endl;
		BosonFileConverter converter;
		if (!converter.convertPlayField_From_0_9_To_0_9_1(destFiles)) {
			boError() << k_funcinfo << "could not convert from boson 0.9 to boson 0.9.1 file format" << endl;
			ret = false;
		} else {
			ret = true;
		}
		break;
	}
	case BOSON_SAVEGAME_FORMAT_VERSION_0_9_1:
	{
		boDebug() << k_funcinfo << "converting from 0.9.1 to 0.10 format" << endl;
		BosonFileConverter converter;
		if (!converter.convertPlayField_From_0_9_1_To_0_10(destFiles)) {
			boError() << k_funcinfo << "could not convert from boson 0.9 to boson 0.9.1 file format" << endl;
			ret = false;
		} else {
			ret = true;
		}
		break;
	}
	case BOSON_SAVEGAME_FORMAT_VERSION_0_10:
	{
		boDebug() << k_funcinfo << "converting from 0.10 to 0.10.80 format" << endl;
		BosonFileConverter converter;
		if (!converter.convertPlayField_From_0_10_To_0_10_80(destFiles)) {
			boError() << k_funcinfo << "could not convert from boson 0.10 to boson 0.10.80 file format" << endl;
			ret = false;
		} else {
			ret = true;
		}
		break;
	}
	case BOSON_MAKE_SAVEGAME_FORMAT_VERSION(0x00, 0x02, 0x04): // development version ("0.10.80")
	{
		boDebug() << k_funcinfo << "converting from 0.10.80 to 0.10.81 format" << endl;
		BosonFileConverter converter;
		if (!converter.convertPlayField_From_0_10_80_To_0_10_81(destFiles)) {
			boError() << k_funcinfo << "could not convert from boson 0.10.80 to boson 0.10.81 file format" << endl;
			ret = false;
		} else {
			ret = true;
		}
		break;
	}
	case BOSON_MAKE_SAVEGAME_FORMAT_VERSION(0x00, 0x02, 0x05): // development version ("0.10.81")
	{
		boDebug() << k_funcinfo << "converting from 0.10.81 to 0.10.82 format" << endl;
		BosonFileConverter converter;
		if (!converter.convertPlayField_From_0_10_81_To_0_10_82(destFiles)) {
			boError() << k_funcinfo << "could not convert from boson 0.10.81 to boson 0.10.82 file format" << endl;
			ret = false;
		} else {
			ret = true;
		}
		break;
	}
	case BOSON_MAKE_SAVEGAME_FORMAT_VERSION(0x00, 0x02, 0x06): // development version ("0.10.82")
	{
		boDebug() << k_funcinfo << "converting from 0.10.82 to 0.10.83 format" << endl;
		BosonFileConverter converter;
		if (!converter.convertPlayField_From_0_10_82_To_0_10_83(destFiles)) {
			boError() << k_funcinfo << "could not convert from boson 0.10.82 to boson 0.10.83 file format" << endl;
			ret = false;
		} else {
			ret = true;
		}
		break;
	}
	case BOSON_MAKE_SAVEGAME_FORMAT_VERSION(0x00, 0x02, 0x07): // development version ("0.10.83")
	{
		boDebug() << k_funcinfo << "converting from 0.10.83 to 0.10.84 format" << endl;
		BosonFileConverter converter;
		if (!converter.convertPlayField_From_0_10_83_To_0_10_84(destFiles)) {
			boError() << k_funcinfo << "could not convert from boson 0.10.83 to boson 0.10.84 file format" << endl;
			ret = false;
		} else {
			ret = true;
		}
		break;
	}
	case BOSON_MAKE_SAVEGAME_FORMAT_VERSION(0x00, 0x02, 0x08): // development version ("0.10.84")
	{
		boDebug() << k_funcinfo << "converting from 0.10.84 to 0.10.85 format" << endl;
		BosonFileConverter converter;
		if (!converter.convertPlayField_From_0_10_84_To_0_10_85(destFiles)) {
			boError() << k_funcinfo << "could not convert from boson 0.10.84 to boson 0.10.85 file format" << endl;
			ret = false;
		} else {
			ret = true;
		}
		break;
	}
	case BOSON_MAKE_SAVEGAME_FORMAT_VERSION(0x00, 0x02, 0x09): // development version ("0.10.85")
	{
		boDebug() << k_funcinfo << "converting from 0.10.85 to 0.11 format" << endl;
		BosonFileConverter converter;
		if (!converter.convertPlayField_From_0_10_85_To_0_11(destFiles)) {
			boError() << k_funcinfo << "could not convert from boson 0.10.85 to boson 0.11 file format" << endl;
			ret = false;
		} else {
			ret = true;
		}
		break;
	}
	case BOSON_SAVEGAME_FORMAT_VERSION_0_11:
	{
		boDebug() << k_funcinfo << "converting from 0.11 to 0.11.80 format" << endl;
		BosonFileConverter converter;
		if (!converter.convertPlayField_From_0_11_To_0_11_80(destFiles)) {
			boError() << k_funcinfo << "could not convert from boson 0.11 to boson 0.11.80 file format" << endl;
			ret = false;
		} else {
			ret = true;
		}
		break;
	}
	case BOSON_MAKE_SAVEGAME_FORMAT_VERSION(0x00, 0x03, 0x00): // development version ("0.11.80")
	{
		boDebug() << k_funcinfo << "converting from 0.11.80 to 0.11.81 format" << endl;
		BosonFileConverter converter;
		if (!converter.convertPlayField_From_0_11_80_To_0_11_81(destFiles)) {
			boError() << k_funcinfo << "could not convert from boson 0.11.80 to boson 0.11.81 file format" << endl;
			ret = false;
		} else {
			ret = true;
		}
		break;
	}
	case BOSON_MAKE_SAVEGAME_FORMAT_VERSION(0x00, 0x03, 0x01): // development version ("0.11.81")
	{
		const char* from = "0.11.81";
		const char* to = "0.12";
		boDebug() << k_funcinfo << "converting from " << from << " to " << to << " format" << endl;
		BosonFileConverter converter;
		if (!converter.convertPlayField_From_0_11_81_To_0_12(destFiles)) {
			boError() << k_funcinfo << "could not convert from boson " << from << " to boson " << to << " file format" << endl;
			ret = false;
		} else {
			ret = true;
		}
		break;
	}
	default:
		*handled = false;
		ret = true;
		break;
 }
 return ret;
}

