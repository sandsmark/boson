/*
    This file is part of the Boson game
    Copyright (C) 2002-2003 The Boson Team (boson-devel@lists.sourceforge.net)

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
 BosonProfiler profiler(BosonProfiling::LoadPlayField);
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
 } else if (!file->hasMapDirectory() && !file->hasFile("scenario.xml")) {
	boWarning() << k_funcinfo << "converting from a savegame file from boson < 0.9" << endl;
	QMap<QString, QByteArray> files;
	files.insert("kgame.xml", file->kgameData());
	files.insert("players.xml", file->playersData());
	files.insert("canvas.xml", file->canvasData());
	files.insert("external.xml", file->externalData());
	files.insert("map", file->mapData());
	BosonFileConverter converter;
//	if (!converter.convertSaveGame_From_0_8_128_To_0_9(files)) {
	if (!converter.convertSaveGame_From_0_8_To_0_9(files)) {
		boError() << k_funcinfo << "unable to convert from savegame format" << endl;
		return false;
	}
	boDebug() << k_funcinfo << "conversion succeeded" << endl;
	playersXML = files["players.xml"];
	mapXML = files["map/map.xml"];
 } else {
	boDebug() << k_funcinfo << "file format is current" << endl;
	mapXML = file->mapXMLData();
	playersXML = file->playersData();
 }

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
 if (!doc.setContent(QCString(xml), &errorMsg, &line, &column)) {
	boError() << k_funcinfo << "unable to set XML content - error in line=" << line << ",column=" << column << " errorMsg=" << errorMsg << endl;
	return false;
 }
 QDomElement root = doc.documentElement();

 // AB: atm there is no usable way to retrieve a minplayers value. also atm we
 // don't have any use for minplayers anyway, as we don't have scenarios (i.e.
 // winning conditions) yet. so we use 1 as default.
 mMinPlayers = 1;

 mMaxPlayers = root.elementsByTagName("Player").count();


 return true;
}

bool BosonPlayFieldInformation::loadMapInformation(const QByteArray& xml)
{
 QString errorMsg;
 int line = 0, column = 0;
 QDomDocument doc(QString::fromLatin1("BosonMap"));
#warning FIXME
 // AB: why does QCString(xml) not work here when loading games?
 if (!doc.setContent(QString(xml), &errorMsg, &line, &column)) {
	boError() << k_funcinfo << "unable to set XML content - error in line=" << line << ",column=" << column << " errorMsg=" << errorMsg << endl;
	boDebug() << QString(xml) << endl;
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
 mFile = 0;
 mPreLoaded = false;
 mLoaded = false;
 mPlayFieldInformation = new BosonPlayFieldInformation();
 mDescription = new BPFDescription();
}

BosonPlayField::~BosonPlayField()
{
 boDebug() << k_funcinfo << endl;
 emit signalNewMap(0);
 delete mMap;
 delete mDescription;
 delete mFile;
 delete mPlayFieldInformation;
 boDebug() << k_funcinfo << "done" << endl;
}

bool BosonPlayField::preLoadAllPlayFields()
{
 boDebug() << k_funcinfo << endl;

 // TODO: ensure that UI doesn't block (i.e. call process events)
 BosonProfiler profiler(BosonProfiling::PreLoadPlayFields);
 if (BosonData::bosonData()->availablePlayFields().count() > 0) {
		boWarning() << k_funcinfo << "playFields already loaded" << endl;
	return true;
 }
 QStringList campaigns = findAvailableCampaigns();
 QStringList::Iterator campaignIt;
 for (campaignIt = campaigns.begin(); campaignIt != campaigns.end(); ++campaignIt) {
	QString campaignName = *campaignIt;
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
	BosonCampaign* campaign = new BosonCampaign(*campaignIt, *campaignIt);
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
		QString campaignName = *campaignIt;
		if (campaignName.isEmpty()) {
			campaignName = QString::fromLatin1("default");
		}
		boWarning() << k_funcinfo << "could not load any playfields for campaign " << campaignName << endl;
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
 QStringList l = boData->availablePlayFields();
 if (l.contains(DEFAULT_PLAYFIELD)) {
	return DEFAULT_PLAYFIELD;
 }
 if (l.count() == 0) {
	return QString::null;
 }
 boWarning() << k_funcinfo << "cannot find " << DEFAULT_PLAYFIELD << " map - using " << l[0] << " instead" << endl;
 return l[0];
}

bool BosonPlayField::preLoadPlayField(const QString& file)
{
 if (isPreLoaded()) {
	return true;
 }
 delete mFile;
 mFile = new BPFFile(file, true);
 if (!mFile->checkTar()) {
	boError() << k_funcinfo << "Oops - broken file " << file << endl;
	return false;
 }
 if (!mPlayFieldInformation->loadInformation(mFile)) {
	boError() << k_funcinfo << "Could not load playfield information" << endl;
	return false;
 }
 QByteArray descriptionXML = mFile->descriptionData();
 if (descriptionXML.size() == 0 && !mFile->hasMapDirectory()) {
	// AB: this might be a savegame (.bsg) file from boson < 0.9
	// the "map" file includes the description.xml here. boson 0.8 used the
	// same format for that file as boson 0.8.128
	QMap<QString, QByteArray> files;
	files.insert("kgame.xml", mFile->kgameData());
	files.insert("players.xml", mFile->playersData());
	files.insert("canvas.xml", mFile->canvasData());
	files.insert("external.xml", mFile->externalData());
	files.insert("map", mFile->mapData());
	BosonFileConverter converter;
	boWarning() << k_funcinfo << "trying to convert a savegame file..." << endl;
	if (!converter.convertSaveGame_From_0_8_128_To_0_9(files)) {
		boError() << k_funcinfo << "unable to convert from savegame format" << endl;
		return false;
	}
	boDebug() << k_funcinfo << "conversion succeeded" << endl;
	descriptionXML = files["C/descriptionXML"];
 }
 if (!loadDescriptionFromFile(descriptionXML)) {
	boError() << k_funcinfo << "Could not load description file" << endl;
	return false;
 }
 mIdentifier = mFile->identifier();

 mFileName = file;
 mPreLoaded = true;

 delete mFile;
 mFile = 0;
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

 // AB: C/description.xml is mandatory. all other languages are optional.
 if (!files.contains("C/description.xml")) {
	boError() << k_funcinfo << "no default description.xml found" << endl;
	return false;
 }

 if (!loadMapFromFile(files["map/map.xml"], files["map/heightmap.png"], files["map/texmap"])) {
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

bool BosonPlayField::loadMapFromFile(const QByteArray& mapXML, const QByteArray& heightMapImage, const QByteArray& texMap)
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


 boDebug() << k_funcinfo << "map loaded. emitting signal" << endl;
 emit signalNewMap(mMap);
 return ret;
}

bool BosonPlayField::savePlayField(const QString& fileName)
{
 // TODO: use KMessageBox here? or maybe add an errorMessage parameter which can
 // be displayed as a msg box in the calling method
 if (!mMap) {
	boError() << k_funcinfo << "NULL map" << endl;
	return false;
 }
 boError() << k_funcinfo << "saving to files is broken at the moment!" << endl;
 return false;
 QFileInfo fileInfo(fileName);

 if (!mDescription) {
	BO_NULL_ERROR(mDescription);
	return false;
 }
 if (mDescription->name().isEmpty()) {
	mDescription->setName(fileInfo.baseName());
 }
 QString description = saveDescriptionToFile();
 if (description.isEmpty()) {
	boError() << k_funcinfo << "Unable to save description" << endl;
	return false;
 }
 QByteArray map = saveMapToFile();
 if (map.isEmpty()) {
	boError() << k_funcinfo << "Unable to save map" << endl;
	return false;
 }
 boError() << k_funcinfo << "scenario saving is broken currently!" << endl;
 return false;
 QString scenario;
#if 0
 scenario = saveScenarioToFile();
 if (scenario.isEmpty()) {
	boError() << k_funcinfo << "Unable to save scenario" << endl;
	return false;
 }
#endif
 boDebug() << k_funcinfo << "Save height map" << endl;
 QByteArray heightMap = mMap->saveHeightMapImage();
 if (heightMap.size() == 0) {
	boError() << k_funcinfo << "Unable to save height map" << endl;
	return false;
 }
 boDebug() << k_funcinfo << "Save height map done" << endl;
 QByteArray texMap = saveTexMapToFile();
 if (map.isEmpty()) {
	boError() << k_funcinfo << "Unable to save texmap" << endl;
	return false;
 }

 BPFFile f(fileName, false);
 f.writeFile(QString::fromLatin1("map"), map);
 f.writeFile(QString::fromLatin1("texmap"), texMap);
 f.writeFile(QString::fromLatin1("scenario.xml"), scenario);
 f.writeFile(QString::fromLatin1("heightmap.png"), heightMap);
 f.writeFile(QString::fromLatin1("description.xml"), description, QString::fromLatin1("C"));

 mMap->setModified(false);
 return true;
}

QString BosonPlayField::saveDescriptionToFile()
{
 if (!mDescription) {
	BO_NULL_ERROR(mDescription);
	return QString::null;
}
 return mDescription->toString();
}

QByteArray BosonPlayField::saveMapToFile()
{
 if (!mMap) {
	boError() << k_funcinfo << "NULL map" << endl;
	return QByteArray();
 }
 return mMap->saveMapToFile();
}

QByteArray BosonPlayField::saveTexMapToFile()
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

bool BosonPlayField::loadMap(QDataStream& stream)
{
 boDebug() << k_funcinfo << endl;
 delete mMap;
 mMap = new BosonMap(this);
 if (!mMap->loadCompleteMap(stream)) {
	boError() << k_funcinfo << "Unable to load map from stream" << endl;
	return false;
 }
 emit signalNewMap(mMap);
 return true;
}

bool BosonPlayField::savePlayFieldForRemote(QDataStream& stream) const
{
 if (!saveMap(stream)) {
	boError() << k_funcinfo << "Could not save map" << endl;
	return false;
 }

 // I'm not too happy about saving the complete description
 // (including translations...) but i'm afraid we need them. in case of
 // conflicting maps the network version will be used, then we should be
 // able to provide the correct description, too
 if (!saveDescription(stream)) {
	boError() << k_funcinfo << "Could not save description" << endl;
	return false;
 }
 return true;
}

bool BosonPlayField::loadPlayFieldFromRemote(QDataStream& stream)
{
 if (!loadMap(stream)) {
	boError() << k_funcinfo << "Could not load map from stream" << endl;
	return false;
 }
 if (!loadDescription(stream)) {
	boError() << k_funcinfo << "Could not load description from stream" << endl;
	return false;
 }
 return true;
}

bool BosonPlayField::saveMap(QDataStream& stream) const
{
 if (!mMap) {
	boError() << k_funcinfo << "NULL map" << endl;
	return false;
 }
 if (!mMap->saveCompleteMap(stream)) {
	boError() << k_funcinfo << "Unable to save map" << endl;
	return false;
 }
 return true;
}

bool BosonPlayField::saveDescription(QDataStream& stream) const
{
 if (!mDescription) {
	BO_NULL_ERROR(mDescription);
	return false;
 }
 QString xml = mDescription->toString();
 if (xml.isEmpty()) {
	boError() << k_funcinfo << "empty description string!!" << endl;
	// don't return! this is *not* fatal!
 }
 stream << xml;
 return true;
}

bool BosonPlayField::loadDescription(QDataStream& stream)
{
 delete mDescription;
 QString xml;
 stream >> xml;
 mDescription = new BPFDescription(xml);
 return true;
}

void BosonPlayField::quit()
{
 emit signalNewMap(0);
 delete mMap;
 mMap = 0;
}

void BosonPlayField::changeMap(BosonMap* m)
{
 delete mMap;
 mMap = m;
 emit signalNewMap(mMap);
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
 mLoaded = true;
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
 if (!mFile) {
	boWarning() << k_funcinfo << "NULL file - recreating file pointer" << endl;
	mFile = new BPFFile(mFileName, true);
 }
 QByteArray heightMap = mFile->heightMapData();
 QByteArray texMap = mFile->texMapData();
 QByteArray mapXML = mFile->mapXMLData();
 QByteArray playersXML = mFile->playersData();
 QByteArray canvasXML = mFile->canvasData();
 QByteArray kgameXML = mFile->kgameData();
 if (!mFile->hasMapDirectory()) {
	boWarning() << k_funcinfo << "need to convert from an old file" << endl;
	BosonFileConverter converter;

	// convert map first
	if (texMap.size() == 0) {
		if (!converter.convertMapFile_From_0_8_To_0_9(mFile->mapData(), &mapXML, &texMap)) {
			boError() << k_funcinfo << "failed converting from boson 0.8" << endl;
			return false;
		}
	} else {
		if (!converter.convertMapFile_From_0_8_128_To_0_9(mFile->mapData(), &mapXML)) {
			boError() << k_funcinfo << "failed converting from boson 0.8.128" << endl;
			return false;
		}
	}

	// convert scenario
	QByteArray scenario = mFile->scenarioData();
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

 // FIXME: the should load the default description only! i.e. C/description.xml
 QByteArray descriptionXML = mFile->descriptionData();
 if (descriptionXML.size() == 0) {
	boError() << k_funcinfo << "empty default description.xml" << endl;
	return false;
 }

 QByteArray externalXML;
#if 0
 // TODO: not yet suported. will be.
 externalXML = mFile->externalData();
#endif
 destFiles.insert("map/texmap", texMap);
 destFiles.insert("map/heightmap.png", heightMap);
 destFiles.insert("map/map.xml", mapXML);
 destFiles.insert("players.xml", playersXML);
 destFiles.insert("canvas.xml", canvasXML);
 destFiles.insert("kgame.xml", kgameXML);
 destFiles.insert("C/description.xml", descriptionXML);
 if (externalXML.size() != 0) {
	// AB: externalXML is optional only. only for loading games.
	destFiles.insert("external.xml", externalXML);
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
 mapXML = saveMapToFile();
 if (mapXML.size() == 0) {
	boError() << k_funcinfo << "failed saving the map" << endl;
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

 files.insert("map/map.xml", mapXML);
 files.insert("map/heightmap.png", heightMap);
 files.insert("map/texmap", texMap);
 files.insert("C/description.xml", descriptionXML);
 boDebug() << k_funcinfo << "succeeded" << endl;
 return true;
}

