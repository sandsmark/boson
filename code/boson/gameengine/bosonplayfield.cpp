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
#include "bosonplayfield.moc"

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
#include "bpfloader.h"
#include "defines.h"

#include <qdom.h>
#include <qdatastream.h>
#include <qfileinfo.h>
#include <qfile.h>

#include <kstandarddirs.h>
#include <klocale.h>

class BosonPlayFieldPreviewData : public BosonDataObject
{
public:
	BosonPlayFieldPreviewData(const QString& mapFile, BPFPreview* preview);

	virtual ~BosonPlayFieldPreviewData()
	{
		delete mPreview;
	}

	virtual QString idString() const
	{
		return mId;
	}
	virtual void* pointer() const
	{
		return (void*)preview();
	}
	BPFPreview* preview() const
	{
		return mPreview;
	}

	virtual bool load();
private:
	BPFPreview* mPreview;
	QString mId;
};

BosonPlayFieldPreviewData::BosonPlayFieldPreviewData(const QString& file, BPFPreview* preview)
	: BosonDataObject(file)
{
 mPreview = preview;
 *mPreview = BPFLoader::loadFilePreview(file);
 if (!mPreview->isLoaded()) {
	boError() << k_funcinfo << "unable to load playFieldPreview from " << file << endl;
	mId = QString::null;
	return;
 }
 mId = mPreview->identifier();
 if (mId.isEmpty()) {
	boError() << k_funcinfo << "no identifier in " << file << endl;
	mId = QString::null;
	return;
 }
}

bool BosonPlayFieldPreviewData::load()
{
 return true;
}



BosonPlayField::BosonPlayField(QObject* parent) : QObject(parent)
{
 mMap = 0;
 mDescription = new BPFDescription();
}

BosonPlayField::~BosonPlayField()
{
 delete mMap;
 delete mDescription;
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
		BPFPreview* playFieldPreview = new BPFPreview();
		// this will also load the preview!
		BosonPlayFieldPreviewData* previewData = new BosonPlayFieldPreviewData(*it, playFieldPreview);
		if (previewData->idString().isEmpty()) {
			boError() << k_funcinfo << *it << " could not be loaded" << endl;
			delete previewData;
			continue;
		}
		if (!BosonData::bosonData()->insertPlayFieldPreview(previewData)) {
			boWarning() << k_funcinfo << "could not insert playFieldPreview "
					<< previewData->idString()
					<< " (maybe already inserted)" << endl;
			delete previewData;
			continue;
		}

		campaign->addPlayField(playFieldPreview);
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

bool BosonPlayField::loadPlayFieldFromFiles(const QMap<QString, QByteArray>& files)
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

 delete mDescription;
 mDescription = new BPFDescription(QString(files["C/description.xml"]));

 if (!loadMapFromFiles(files)) {
	boError() << k_funcinfo << "error loading the map" << endl;
	return false;
 }

 // here we might parse files for keys that end with "description.xml". all
 // languages should be added that are present.

 // when we have winning conditions one day (probably in "scenario.xml"), we
 // should load it here as well.

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
 if (!description()) {
	BO_NULL_ERROR(description());
	return QString::null;
}
 return description()->toString();
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

QString BosonPlayField::playFieldName() const
{
 if (!description()) {
	BO_NULL_ERROR(description());
	return QString::null;
 }
 return description()->name();
}

QString BosonPlayField::playFieldComment() const
{
 if (!description()) {
	BO_NULL_ERROR(description());
	return QString::null;
 }
 return description()->comment();
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
	int slash = (*it).lastIndexOf('/');
	QString path = (*it).left(slash);
	slash = path.lastIndexOf('/');

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

bool BosonPlayField::savePlayFieldToFiles(QMap<QString, QByteArray>& files)
{
 if (!mMap) {
	BO_NULL_ERROR(mMap);
	return false;
 }
 if (!description()) {
	BO_NULL_ERROR(description());
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
 QByteArray descriptionXML = saveDescriptionToFile().toUtf8();
 if (descriptionXML.size() == 0) {
	boError() << k_funcinfo << "saving description failed" << endl;
	return false;
 }

 QByteArray mapPreviewPNG = saveMapPreviewPNGToFile();
 if (mapPreviewPNG.size() == 0) {
	boError() << k_funcinfo << "saving map preview failed" << endl;
	return false;
 }


 for (QMap<QString,QByteArray>::iterator it = mapFiles.begin(); it != mapFiles.end(); ++it) {
	files.insert(it.key(), it.value());
 }
 files.insert("mappreview/map.png", mapPreviewPNG);
 files.insert("C/description.xml", descriptionXML);
 return true;
}

const BPFDescription* BosonPlayField::description() const
{
 return mDescription;
}

void BosonPlayField::setModifiedDescription(BPFDescription* description)
{
 delete mDescription;
 mDescription = description;
}

