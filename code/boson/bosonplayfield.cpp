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
#include "bosonscenario.h"
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


BosonPlayField::BosonPlayField(QObject* parent) : QObject(parent, "BosonPlayField")
{
 mMap = 0;
 mScenario = 0;
 mFile = 0;
 mPreLoaded = false;
 mLoaded = false;
 mDescription = new BPFDescription();
}

BosonPlayField::~BosonPlayField()
{
 boDebug() << k_funcinfo << endl;
 emit signalNewMap(0);
 delete mMap;
 delete mScenario;
 delete mDescription;
 delete mFile;
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
	if (!campaign->playFieldCount() == 0) {
		boWarning() << k_funcinfo << "could not load any playfields for campaign " << *campaignIt << endl;
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
 if (!loadDescriptionFromFile(mFile->descriptionData())) {
	boError() << k_funcinfo << "Could not load description file" << endl;
	return false;
 }
 mIdentifier = mFile->identifier();
 if (!loadScenarioFromFile(mFile->scenarioData())) {
	boError() << k_funcinfo << "Error loading scenario from " << file << endl;
	return false;
 }

 mFileName = file;
 mPreLoaded = true;

 delete mFile;
 mFile = 0;

 // we have a problem.
 // the minimap for the startupwidgets requires the cells to be loaded which
 // basically means that we have to load *all* data (except heightmaps) for the
 // startup widgets.
 // so "preLoadPlayField()" now loads the *complete* playfield. this is *really*
 // bad, as it takes too much memory!
 // we could solve this by
 // - load on demand only
 //   --> long loading times (about 0.2-1 seconds) when the player selects a map
 //   in the startup widgets
 // - clear the preloaded maps once a game is started
 //   --> we'd have to preload again when another game is started. anyway I
 //   think this is the best solution. TODO
#if 0
 loadPlayField(QString::null); // we don't need to provide the filename again.
#endif
 return true;
}

bool BosonPlayField::loadPlayField(const QString& file)
{
 if (isLoaded()) {
	// no need to load again :-)
	boDebug() << k_funcinfo << "playfield " << file << " has already been loaded" << endl;
	return true;
 }
 boDebug() << k_funcinfo << endl;
 if (!preLoadPlayField(file)) {
	return false;
 }
 if (!file.isNull()) {
	if (file != mFileName) {
		boError() << k_funcinfo << "file " << mFileName
				<< " was preloaded - but we are trying to load "
				<< file << " now!" << endl;
		return false;
	}
 }

 delete mFile;
 mFile = new BPFFile(mFileName, true);

 if (!mFile) {
	boError() << k_funcinfo << "NULL file" << endl;
	return false;
 }

 if (!loadMapFromFile(mFile->mapData(), mFile->heightMapData(), mFile->texMapData())) {
	boError() << k_funcinfo << "Error loading map from " << file << endl;
	return false;
 }
 delete mFile;
 mFile = 0;
 mLoaded = true;
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

bool BosonPlayField::loadMapFromFile(const QByteArray& map, const QByteArray& heightMapImage, const QByteArray& texMap)
{
 boDebug() << k_funcinfo << endl;
 if (texMap.size() == 0) {
	// there is no texmap in the file. Probably a map from boson <= 0.8
	// convert it to our new format first.
	boDebug() << k_funcinfo << "no texmap in file. trying to convert from boson 0.8 file format..." << endl;
	QByteArray newMap;
	QByteArray newTexMap;

	BosonFileConverter converter;
	converter.convertMapFile_From_0_8_To_0_9(map, &newMap, &newTexMap);

	if (newTexMap.size() == 0) {
		boError() << k_funcinfo << "empty texmap" << endl;
		return false;
	}
	return loadMapFromFile(newMap, heightMapImage, newTexMap);
 }
 if (map.size() == 0) {
	boError() << k_funcinfo << "empty byte array for map" << endl;
	return false;
 }
 if (heightMapImage.size() == 0) {
	boError() << k_funcinfo << "empty height map array" << endl;
	return false;
 }
 // note: an empty texMap is fine - we will generate one (compatibility mode
 // for boson < 0.9)
 delete mMap;
 mMap = new BosonMap(this);
 bool ret = mMap->loadMapFromFile(map);
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

bool BosonPlayField::loadScenarioFromFile(const QByteArray& xml)
{
 if (xml.size() == 0) {
	boError() << k_funcinfo << "empty byte array" << endl;
	return false;
 }
 QDomDocument doc("BosonScenario");
 QString errorMsg;
 int lineNo, columnNo;
 if (!doc.setContent(xml, &errorMsg, &lineNo, &columnNo)) {
	boError() << k_funcinfo << "Parse error in line " << lineNo << ",column " << columnNo
			<< " error message: " << errorMsg << endl;
	return false;
 }
 QDomElement root = doc.documentElement();

 if (root.childNodes().count() < 2) { // at least scenario settings and one player (will always be more)
	boError() << k_funcinfo << "No scenario found in file" << endl;
	return false;
 }
 delete mScenario;
 mScenario = new BosonScenario();
 bool ret = mScenario->loadScenario(root);
 if (!ret) {
	boError() << k_funcinfo << "Could not load scenario" << endl;
	return false;
 }
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
 if (!mScenario) {
	boError() << k_funcinfo << "NULL scenario" << endl;
	return false;
 }
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
 QString scenario = saveScenarioToFile();
 if (scenario.isEmpty()) {
	boError() << k_funcinfo << "Unable to save scenario" << endl;
	return false;
 }
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
 mScenario->setModified(false);
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
 QByteArray file;
 QDataStream stream(file, IO_WriteOnly);
 if (!mMap->saveMapToFile(stream)) {
	boError() << k_funcinfo << "Error saving map" << endl;
	return QByteArray();
 }
 return file;
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

QString BosonPlayField::saveScenarioToFile()
{
 if (!mScenario) {
	boError() << k_funcinfo << "NULL scenario" << endl;
	return QString::null;
 }
 QDomDocument doc("BosonScenario");
 QDomElement root = doc.createElement("BosonScenario");
 doc.appendChild(root);
 if (!mScenario->saveScenario(root)) {
	boError() << k_funcinfo << "Error saving scenario" << endl;
	return QString::null;
 }
 return doc.toString();
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

bool BosonPlayField::savePlayFieldForRemote(QDataStream& stream)
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

bool BosonPlayField::saveMap(QDataStream& stream)
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

bool BosonPlayField::saveDescription(QDataStream& stream)
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
 delete mScenario;
 mScenario = 0;
}

void BosonPlayField::applyScenario(Boson* boson)
{
 delete mScenario;
 mScenario = new BosonScenario();
 mScenario->applyScenario(boson);
}

void BosonPlayField::changeScenario(BosonScenario* s)
{
 delete mScenario;
 mScenario = s;
}

void BosonPlayField::changeMap(BosonMap* m)
{
 delete mMap;
 mMap = m;
 emit signalNewMap(mMap);
}

bool BosonPlayField::modified() const
{
 if (mMap && mMap->modified()) {
	return true;
 }
 if (mScenario && mScenario->modified()) {
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
 list.append(QString::null);
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

