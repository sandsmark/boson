/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "../bosonmap.h"
#include "../bosonscenario.h"
#include "bodebug.h"
#include "bpfdescription.h"
#include "bpffile.h"
#include "../defines.h"

#include <qdom.h>
#include <qdatastream.h>
#include <qfileinfo.h>

#include <kstandarddirs.h>
#include <kstaticdeleter.h>
#include <klocale.h>

#include "bosonplayfield.moc"

KStaticDeleter< QDict<BosonPlayField> > sd;
QDict<BosonPlayField>* BosonPlayField::mPlayFields = 0;


BosonPlayField::BosonPlayField(QObject* parent) : QObject(parent, "BosonPlayField")
{
 mMap = 0;
 mScenario = 0;
 mFile = 0;
 mPreLoaded = false;
 mLoaded = false;
 mDescription = new BPFDescription();

 initStatic();
}

BosonPlayField::~BosonPlayField()
{
 emit signalNewMap(0);
 delete mMap;
 delete mScenario;
 delete mDescription;
 delete mFile;
}

void BosonPlayField::initStatic()
{
 if (!mPlayFields) {
	sd.setObject(mPlayFields, new QDict<BosonPlayField>);
	mPlayFields->setAutoDelete(true);

	QTime t;
	t.start();
	// note: this might take some time, since we use gzip compressed files
	// which get extracted here.
	// TODO: free the memory once a map is being started!!
	preLoadAllPlayFields();
	boDebug() << k_funcinfo << t.elapsed() << endl;
 }
}

void BosonPlayField::preLoadAllPlayFields()
{
 // TODO: profiling!
 // TODO: ensure that UI doesn't block (i.e. call process events)
 QStringList list = KGlobal::dirs()->findAllResources("data", "boson/maps/*.bpf");
 if (list.isEmpty()) {
	boError() << k_funcinfo << "Cannot find any playfield?!" << endl;
	return;
 }
 for (unsigned int i = 0; i < list.count(); i++) {
	if (mPlayFields->find(BPFFile::fileNameToIdentifier(list[i]))) {
		continue;
	}
	boDebug() << k_funcinfo << list[i] << endl;
	BosonPlayField* playField = new BosonPlayField();
	bool ok = playField->preLoadPlayField(list[i]);
	if (!ok) {
		boError() << k_funcinfo << "Could not load " << list[i] << endl;
		delete playField;
		continue;
	}
	mPlayFields->insert(playField->mIdentifier, playField);
 }
 if (mPlayFields->count() == 0) {
	boError() << k_funcinfo << "no valid map found!" << endl;
 }
}

BosonPlayField* BosonPlayField::playField(const QString& identifier)
{
 return mPlayFields->find(identifier);
}

void BosonPlayField::clearAllPreLoadedPlayFields()
{
 if (!mPlayFields) {
	return;
 }
 mPlayFields->clear();
}


QString BosonPlayField::defaultPlayField()
{
 QStringList l = availablePlayFields();
 if (l.contains(DEFAULT_PLAYFIELD)) {
	return DEFAULT_PLAYFIELD;
 }
 if (l.count() == 0) {
	return QString::null;
 }
 boWarning() << k_funcinfo << "cannot find " << DEFAULT_PLAYFIELD << " map - using " << l[0] << " instead" << endl;
 return l[0];
}

QStringList BosonPlayField::availablePlayFields()
{
 QStringList list;
 QDictIterator<BosonPlayField> it(*mPlayFields);
 for (; it.current(); ++it) {
	list.append(it.currentKey());
 }
 return list;
}

QString BosonPlayField::playFieldName(const QString& id)
{
 BosonPlayField* f = mPlayFields->find(id);
 if (!f) {
	return QString::null;
 }
 return f->playFieldName();
}

QString BosonPlayField::playFieldComment(const QString& id)
{
 BosonPlayField* f = mPlayFields->find(id);
 if (!f) {
	return QString::null;
 }
 return f->playFieldComment();
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
 if (!loadDescriptionXML(mFile->descriptionData())) {
	boError() << k_funcinfo << "Could not load description file" << endl;
	return false;
 }
 mIdentifier = mFile->identifier();
 if (!loadScenarioXML(mFile->scenarioData())) {
	boError() << k_funcinfo << "Error loading scenario from " << file << endl;
	return false;
 }

 mPreLoaded = true;
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
 if (!mFile) {
	boError() << k_funcinfo << "NULL file" << endl;
	return false;
 }

 // this takes most loading time
 if (!loadMapXML(mFile->mapData(), mFile->heightMapData())) {
	boError() << k_funcinfo << "Error loading map from " << file << endl;
	return false;
 }
 mFile->close();
 delete mFile;
 mFile = 0;
 mLoaded = true;
 return true;
}

bool BosonPlayField::loadDescriptionXML(const QByteArray& xml)
{
 if (xml.size() == 0) {
	boError() << k_funcinfo << "Oops - NULL description file" << endl;
	return false;
 }
 delete mDescription;
 mDescription = new BPFDescription(QString(xml));
 return true;
}

bool BosonPlayField::loadMapXML(const QByteArray& xml, const QByteArray& heightMapImage)
{
 if (xml.size() == 0) {
	boError() << k_funcinfo << "empty byte array for map.xml" << endl;
	return false;
 }
 if (heightMapImage.size() == 0) {
	boError() << k_funcinfo << "empty height map array" << endl;
	return false;
 }
 QDomDocument doc("BosonMap");
 QString errorMsg;
 int lineNo, columnNo;
 if (!doc.setContent(xml, &errorMsg, &lineNo, &columnNo)) {
	boError() << k_funcinfo << "Parse error in line " << lineNo << ",column " << columnNo
			<< " error message: " << errorMsg << endl;
	return false;
 }
 QDomElement root = doc.documentElement();

 if (root.childNodes().count() < 2) { // at least map geo and map cells
	boError() << k_funcinfo << "No map found in file" << endl;
	return false;
 }
 delete mMap;
 mMap = new BosonMap(this);
 bool ret = mMap->loadMap(root);
 if (!ret) {
	boError() << k_funcinfo << "Could not load map" << endl;
	return false;
 }
 boWarning() << k_funcinfo << "should load height map now" << endl;
 ret = mMap->loadHeightMapImage(heightMapImage);
 if (!ret) {
	boError() << k_funcinfo << "Could not load map (height map failed)" << endl;
	return false;
 }
 emit signalNewMap(mMap);
 return ret;
}

bool BosonPlayField::loadScenarioXML(const QByteArray& xml)
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
 QString description = saveDescriptionXML();
 if (description.isEmpty()) {
	boError() << k_funcinfo << "Unable to save description" << endl;
	return false;
 }
 QString map = saveMapXML();
 if (map.isEmpty()) {
	boError() << k_funcinfo << "Unable to save map" << endl;
	return false;
 }
 QString scenario = saveScenarioXML();
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

 QString topDir = fileInfo.fileName();
 if (topDir.right(7) == QString::fromLatin1(".tar.gz")) {
	// might be the case for debugging
	topDir = topDir.left(topDir.length() - 7);
 }
 if (topDir.findRev('.') > 0) {
	topDir.truncate(topDir.findRev('.'));
 }

 BPFFile* f = new BPFFile(fileName, false);
 QString user = f->directory()->user();
 QString group = f->directory()->group();
 f->writeFile(QString::fromLatin1("%1/map.xml").arg(topDir), user, group, map.length(), map.data());
 f->writeFile(QString::fromLatin1("%1/scenario.xml").arg(topDir), user, group, scenario.length(), scenario.data());
 f->writeFile(QString::fromLatin1("%1/heightmap.png").arg(topDir), user, group, heightMap.size(), heightMap.data());
 f->writeFile(QString::fromLatin1("%1/%2/description.xml").arg(topDir).arg(QString::fromLatin1("C")), user, group, description.length(), description.data());

 mMap->setModified(false);
 mScenario->setModified(false);
 f->close();
 delete f;
 return true;
}

QString BosonPlayField::saveDescriptionXML()
{
 if (!mDescription) {
	BO_NULL_ERROR(mDescription);
	return QString::null;
}
 return mDescription->toString();
}

QString BosonPlayField::saveMapXML()
{
 if (!mMap) {
	boError() << k_funcinfo << "NULL map" << endl;
	return QString::null;
 }
 QDomDocument doc("BosonMap");
 QDomElement root = doc.createElement("BosonMap");
 doc.appendChild(root);
 if (!mMap->saveMap(root)) {
	boError() << k_funcinfo << "Error saving map" << endl;
	return QString::null;
 }
 return doc.toString();
}

QString BosonPlayField::saveScenarioXML()
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
 delete mMap;
 mMap = new BosonMap(this);
 if (!mMap->loadMap(stream)) {
	boError() << k_funcinfo << "Unable to load map from stream" << endl;
	return false;
 }
 emit signalNewMap(mMap);
 return true;
}

void BosonPlayField::saveMap(QDataStream& stream)
{
 if (!mMap) {
	boError() << k_funcinfo << "NULL map" << endl;
	return;
 }
 if (!mMap->saveMap(stream)) {
	boError() << k_funcinfo << "Unable to save map" << endl;
	return;
 }
}

void BosonPlayField::saveDescription(QDataStream& stream)
{
 BO_CHECK_NULL_RET(mDescription);
 QString xml = mDescription->toString();
 if (xml.isEmpty()) {
	boError() << k_funcinfo << "empty description string!!" << endl;
	// don't return! this is *not* fatal!
 }
 stream << xml;
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
