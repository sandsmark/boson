/*
    This file is part of the Boson game
    Copyright (C) 2003-2005 Andreas Beckermann (b_mann@gmx.de)

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

#include "bosonsaveload.h"
#include "bosonsaveload.moc"

#include "../bomemory/bodummymemory.h"
#include "boson.h"
#include "boversion.h"
#include "bosonprofiling.h"
#include "bosonplayfield.h"
#include "bosonpropertyxml.h"
#include "bosoncanvas.h"
#include "bofile.h"
#include "player.h"
#include "bodebug.h"
#include "bosonfileconverter.h"
#include "boeventmanager.h"

#include <kgame/kgamemessage.h>
#include <kgame/kgamepropertyhandler.h>
#include <klocale.h>

#include <qdom.h>
#include <qdatastream.h>
#include <qptrqueue.h>
#include <qvaluelist.h>
#include <qregexp.h>

SaveLoadError::SaveLoadError(ErrorType type, const QString& text, const QString& caption)
{
 mType = type;
 mCaption = caption;
 mText = text;
}

QString SaveLoadError::message() const
{
 QString m;
 switch (type()) {
	case Unknown:
		m = i18n("Unknown error");
		break;
	case General:
		m = i18n("General error");
		break;
	case LoadBSGFileError:
		m = i18n(".bsg file error");
		break;
	case LoadInvalidXML:
		m = i18n("Invalid XML");
		break;
	default:
		m = i18n("Unknown error type (%1)").arg((int)type());
 }
 if (!mText.isEmpty()) {
	m = i18n("%1\n\nAdditional information:\n%2").arg(m).arg(mText);
 }
 return m;
}

LoadError::LoadError(ErrorType type, const QString& text, const QString& caption)
	: SaveLoadError(type, text, caption.isNull() ? i18n("Loading error") : caption)
{
}

SaveError::SaveError(ErrorType type, const QString& text, const QString& caption)
	: SaveLoadError(type, text, caption.isNull() ? i18n("Saving error") : caption)
{
}


class BosonSaveLoadPrivate
{
public:
	BosonSaveLoadPrivate()
	{
		mBoson = 0;

		mPlayField = 0;
		mCanvas = 0;
	}
	Boson* mBoson;
	BosonPlayField* mPlayField;
	BosonCanvas* mCanvas;

	QPtrQueue<SaveLoadError> mErrorQueue;
};

BosonSaveLoad::BosonSaveLoad(Boson* boson) : QObject(boson, "bosonsaveload")
{
 d = new BosonSaveLoadPrivate;
 d->mBoson = boson;
 d->mErrorQueue.setAutoDelete(true);

 initBoson();
 d->mBoson->initSaveLoad(this);
}

BosonSaveLoad::~BosonSaveLoad()
{
 d->mErrorQueue.clear();
 delete d;
}

void BosonSaveLoad::initBoson()
{
 if (!d->mBoson) {
	return;
 }

 connect(this, SIGNAL(signalLoadExternalStuffFromXML(const QDomElement&)),
		d->mBoson, SIGNAL(signalLoadExternalStuffFromXML(const QDomElement&)));
 connect(this, SIGNAL(signalSaveExternalStuffAsXML(QDomElement&)),
		d->mBoson, SIGNAL(signalSaveExternalStuffAsXML(QDomElement&)));
}

void BosonSaveLoad::setCanvas(BosonCanvas* canvas)
{
 d->mCanvas = canvas;
}

void BosonSaveLoad::setPlayField(BosonPlayField* playField)
{
 d->mPlayField = playField;
}

void BosonSaveLoad::addError(SaveLoadError* error)
{
 d->mErrorQueue.enqueue(error);
}

void BosonSaveLoad::addLoadError(SaveLoadError::ErrorType type, const QString& text, const QString& caption)
{
 addError(new LoadError(type, text, caption));
}

void BosonSaveLoad::addSaveError(SaveLoadError::ErrorType type, const QString& text, const QString& caption)
{
 addError(new SaveError(type, text, caption));
}

unsigned long int BosonSaveLoad::latestSavegameVersion()
{
 return BOSON_SAVEGAME_FORMAT_VERSION;
}

unsigned long int BosonSaveLoad::savegameFormatVersion(const QString& kgameXML)
{
 if (kgameXML.isEmpty()) {
	return 0;
 }
 QDomDocument doc(QString::fromLatin1("Boson"));
 if (!loadXMLDoc(&doc, kgameXML)) {
	return 0;
 }
 QDomElement root = doc.documentElement();
 bool ok = false;
 unsigned int version = root.attribute(QString::fromLatin1("Version")).toUInt(&ok);
 if (!ok) {
	return 0;
 }
 return version;
}

bool BosonSaveLoad::loadXMLDoc(QDomDocument* doc, const QString& xml)
{
 // helper function that outputs parsing errors
 QString errorMsg;
 int lineNo, columnNo;
 if (!doc->setContent(xml, &errorMsg, &lineNo, &columnNo)) {
	boError() << k_funcinfo << "Parse error in line " << lineNo << ",column " << columnNo
			<< " error message: " << errorMsg << endl;
	return false;
 }
 return true;
}

bool BosonSaveLoad::saveToFiles(QMap<QString, QByteArray>& files)
{
 boDebug() << k_funcinfo << endl;
 if (!files.isEmpty()) {
	boError() << k_funcinfo << "files list must be empty" << endl;
	return false;
 }
 if (!d->mBoson) {
	boError() << k_funcinfo << "NULL boson object" << endl;
	return false;
 }
 BosonProfiler profiler("SaveKGameToXML");
 if (!d->mPlayField) {
	BO_NULL_ERROR(d->mPlayField);
	return false;
 }
 if (!d->mPlayField->savePlayFieldToFiles(files)) {
	boError() << k_funcinfo << "saving the playfield failed" << endl;
	return false;
 }
 QByteArray kgameXML = saveKGameAsXML();
 if (kgameXML.isNull()) {
	return false;
 }

 QByteArray playersXML = savePlayersAsXML();
 if (playersXML.isNull()) {
	return false;
 }

 QByteArray canvasXML = saveCanvasAsXML();
 if (canvasXML.isNull()) {
	return false;
 }

 QByteArray externalXML = saveExternalAsXML();
 if (externalXML.isNull()) {
	return false;
 }

 if (!saveEventListenerScripts(&files)) {
	boError() << k_funcinfo << "could not save event listener scripts" << endl;
	return false;
 }

 files.insert("kgame.xml", kgameXML);
 files.insert("players.xml", playersXML);
 files.insert("canvas.xml", canvasXML);
 files.insert("external.xml", externalXML);

 if (!convertPlayerIdsToIndices(files)) {
	return false;
 }

 return true;
}

bool BosonSaveLoad::savePlayFieldToFiles(QMap<QString, QByteArray>& files)
{
 // first we save a complete game.
 BosonProfiler prof("SavePlayFieldToFiles()");
 bool ret = saveToFiles(files);
 if (!ret) {
	boError() << k_funcinfo << "saving failed" << endl;
	return ret;
 }
 ret = convertSaveGameToPlayField(files);
 return ret;
}

bool BosonSaveLoad::saveToFile(const QMap<QString, QByteArray>& files, const QString& file)
{
 boDebug() << k_funcinfo << file << endl;
 PROFILE_METHOD
 QByteArray kgameXML = files["kgame.xml"];
 QByteArray playersXML = files["players.xml"];
 QByteArray canvasXML = files["canvas.xml"];
 QByteArray externalXML = files["external.xml"];
 QByteArray mapXML = files["map/map.xml"];
 QByteArray waterXML = files["map/water.xml"];
 QByteArray heightMap = files["map/heightmap.png"];
 QByteArray texMap = files["map/texmap"];
 QByteArray descriptionXML = files["C/description.xml"];
 if (kgameXML.size() == 0) {
	boError() << k_funcinfo << "no kgameXML found" << endl;
	return false;
 }
 if (canvasXML.size() == 0) {
	boError() << k_funcinfo << "no canvasXML found" << endl;
	return false;
 }
 if (playersXML.size() == 0) {
	boError() << k_funcinfo << "no playersXML found" << endl;
	return false;
 }
 if (canvasXML.size() == 0) {
	boError() << k_funcinfo << "no canvasXML found" << endl;
	return false;
 }
 if (externalXML.size() == 0) {
	// do nothing - is optional only.
 }
 if (mapXML.size() == 0) {
	boError() << k_funcinfo << "no mapXML found" << endl;
	return false;
 }
 if (waterXML.size() == 0) {
	boError() << k_funcinfo << "no waterXML found" << endl;
	return false;
 }
 if (heightMap.size() == 0) {
	boError() << k_funcinfo << "no heightMap found" << endl;
	return false;
 }
 if (texMap.size() == 0) {
	boError() << k_funcinfo << "no texMap found" << endl;
	return false;
 }
 if (descriptionXML.size() == 0) {
	boError() << k_funcinfo << "no descriptionXML found" << endl;
	return false;
 }
 BPFFile f(file, false);
 if (!f.writeFile(QString::fromLatin1("kgame.xml"), QString(kgameXML))) {
	boError() << k_funcinfo << "Could not write kgame.xml to " << file << endl;
	return false;
 }
 if (!f.writeFile(QString::fromLatin1("players.xml"), QString(playersXML))) {
	boError() << k_funcinfo << "Could not write players.xml to " << file << endl;
	return false;
 }
 if (!f.writeFile(QString::fromLatin1("canvas.xml"), QString(canvasXML))) {
	boError() << k_funcinfo << "Could not write canvas.xml to " << file << endl;
	return false;
 }
 if (!f.writeFile(QString::fromLatin1("external.xml"), QString(externalXML))) {
	boError() << k_funcinfo << "Could not write external.xml to " << file << endl;
	return false;
 }
 if (!f.writeFile(QString::fromLatin1("map.xml"), QString(mapXML), QString::fromLatin1("map"))) {
	boError() << k_funcinfo << "Could not write map to " << file << endl;
	return false;
 }
 if (!f.writeFile(QString::fromLatin1("water.xml"), QString(waterXML), QString::fromLatin1("map"))) {
	boError() << k_funcinfo << "Could not write water to " << file << endl;
	return false;
 }
 if (!f.writeFile(QString::fromLatin1("heightmap.png"), heightMap, QString::fromLatin1("map"))) {
	boError() << k_funcinfo << "Could not write map to " << file << endl;
	return false;
 }
 if (!f.writeFile(QString::fromLatin1("texmap"), texMap, QString::fromLatin1("map"))) {
	boError() << k_funcinfo << "Could not write map to " << file << endl;
	return false;
 }
 if (!f.writeFile(QString::fromLatin1("description.xml"), QString(descriptionXML), QString::fromLatin1("C"))) {
	boError() << k_funcinfo << "Could not write map to " << file << endl;
	return false;
 }

 QStringList scripts = QStringList(files.keys()).grep("scripts");
 for (QStringList::iterator it = scripts.begin(); it != scripts.end(); ++it) {
	QString path = *it;
	int lastSlash = path.findRev('/');
	QString dir = path.left(lastSlash);
	QString baseName = path.right(path.length() - (lastSlash + 1));
	if (!f.writeFile(baseName, files[path], dir)) {
		boError() << k_funcinfo << "Could not write " << baseName << " to " << file << endl;
		return false;
	}
 }
 return true;
}

QCString BosonSaveLoad::saveKGameAsXML()
{
 PROFILE_METHOD
 QDomDocument doc(QString::fromLatin1("Boson"));
 QDomElement root = doc.createElement(QString::fromLatin1("Boson")); // XML file for the Boson object
 doc.appendChild(root);
 root.setAttribute(QString::fromLatin1("Version"), BOSON_SAVEGAME_FORMAT_VERSION);

 // store the dataHandler()
 BosonCustomPropertyXML propertyXML;
 QDomElement handler = doc.createElement(QString::fromLatin1("DataHandler"));
 if (!propertyXML.saveAsXML(handler, d->mBoson->dataHandler())) { // AB: we should exclude gameStatus from this! we should stay in KGame::Init! -> add a BosonPropertyXML::remove() or so
	boError() << k_funcinfo << "unable to save KGame data handler" << endl;
	return QCString();
 }
 // IdGameStatus must _not_ be saved. it must remain in Init state when loading,
 // until loading is completed.
 propertyXML.removeProperty(handler, KGamePropertyBase::IdGameStatus);
 root.appendChild(handler);


 QDomElement eventManager = doc.createElement(QString::fromLatin1("EventManager"));
 root.appendChild(eventManager);
 if (!d->mBoson->eventManager()->saveAsXML(eventManager)) {
	boError() << k_funcinfo << "unable to save the event manager" << endl;
	return false;
 }

 return doc.toCString();
}

QCString BosonSaveLoad::savePlayersAsXML()
{
 PROFILE_METHOD
 QDomDocument doc(QString::fromLatin1("Players"));
 QDomElement root = doc.createElement(QString::fromLatin1("Players"));
 doc.appendChild(root);

 if (!d->mBoson) {
	BO_NULL_ERROR(d->mBoson);
	return QCString();
 }

 KGame::KGamePlayerList* list = d->mBoson->playerList();
 boDebug() << k_funcinfo << "saving " << list->count() << " players" << endl;
 for (KPlayer* p = list->first(); p; p = list->next()) {
	// KGame also stored ID, RTTI and KPlayer::calcIOValue() here.
	// I believe we won't need them. ID might be useful for network games,
	// but we load from a file here.
	QDomElement element = doc.createElement(QString::fromLatin1("Player"));
	if (!((Player*)p)->saveAsXML(element)) {
		boError() << k_funcinfo << "Unable to save player " << p->id() << endl;
		return QCString();
	}
	root.appendChild(element);
 }

 return doc.toCString();
}

QCString BosonSaveLoad::saveCanvasAsXML()
{
 PROFILE_METHOD
 QDomDocument doc(QString::fromLatin1("Canvas"));
 QDomElement root = doc.createElement(QString::fromLatin1("Canvas")); // XML file for canvas
 doc.appendChild(root);

 if (d->mCanvas) {
	d->mCanvas->saveAsXML(root);
 } else {
	boDebug() << k_funcinfo << "NULL canvas - saving nothing" << endl;
	QDomElement handler = doc.createElement(QString::fromLatin1("DataHandler"));
	root.appendChild(handler);
 }

 return doc.toCString();
}

QCString BosonSaveLoad::saveExternalAsXML()
{
 PROFILE_METHOD
 QDomDocument doc(QString::fromLatin1("External"));
 QDomElement root = doc.createElement(QString::fromLatin1("External")); // XML file for external data
 doc.appendChild(root);

 emit signalSaveExternalStuffAsXML(root);

 return doc.toCString();
}


bool BosonSaveLoad::startFromFiles(const QMap<QString, QByteArray>& files)
{
 boDebug(270) << k_funcinfo << endl;

 QByteArray playersXML = files["players.xml"];
 QByteArray canvasXML = files["canvas.xml"];
 QByteArray kgameXML = files["kgame.xml"];
 QByteArray externalXML = files["external.xml"];

  if (kgameXML.isEmpty()) {
	boError(270) << k_funcinfo << "Empty kgameXML" << endl;
	addLoadError(SaveLoadError::LoadBSGFileError, i18n("empty file: kgame.xml"));
	return false;
  }
  if (playersXML.isEmpty()) {
	boError(270) << k_funcinfo << "Empty playersXML" << endl;
	addLoadError(SaveLoadError::LoadBSGFileError, i18n("empty file: players.xml"));
	return false;
 }
 if (canvasXML.isEmpty()) {
	boError(270) << k_funcinfo << "Empty canvasXML" << endl;
	addLoadError(SaveLoadError::LoadBSGFileError, i18n("empty file: canvas.xml"));
	return false;
 }

 if (!loadKGameFromXML(kgameXML)) {
	return false;
 }
 if (!loadPlayersFromXML(playersXML)) {
	return false;
 }
 boDebug(270) << k_funcinfo << d->mBoson->playerCount() << " players loaded" << endl;

 boDebug(270) << k_funcinfo << "loading units" << endl;

 // Load canvas (items - i.e. units and shots)
 if (!loadCanvasFromXML(canvasXML)) {
	addLoadError(SaveLoadError::General, i18n("error while loading canvas"));
	return false;
 }
 if (externalXML.size() != 0) {
	// external.xml is optional only, it's valid that it's missing
	if (!loadExternalFromXML(externalXML)) {
		addLoadError(SaveLoadError::General, i18n("error while loading external data"));
		return false;
	}
 }

 if (!loadEventListenerScripts(files)) {
	boError(270) << k_funcinfo << "could not load eventlistener scripts" << endl;
	addLoadError(SaveLoadError::General, i18n("error while loading event listener scripts"));
	return false;
 }

 return true;
}

bool BosonSaveLoad::loadKGameFromXML(const QString& kgameXML)
{
 QDomDocument doc(QString::fromLatin1("Boson"));
 if (!loadXMLDoc(&doc, kgameXML)) {
	addLoadError(SaveLoadError::LoadInvalidXML, i18n("Parsing error in kgame.xml"));
	return false;
 }
 QDomElement root = doc.documentElement();

 BosonCustomPropertyXML propertyXML;
 QDomElement handler = root.namedItem(QString::fromLatin1("DataHandler")).toElement();
 if (!handler.isNull()) {
	// note: a missing DataHandler is valid.
	 BosonPropertyXML::removeProperty(handler, KGamePropertyBase::IdGameStatus);
	if (!propertyXML.loadFromXML(handler, d->mBoson->dataHandler())) {
		boError(270) << k_funcinfo << "unable to load KGame DataHandler" << endl;
		return false;
	}
 }

 QDomElement eventManager = root.namedItem(QString::fromLatin1("EventManager")).toElement();
 if (eventManager.isNull()) {
	boError(270) << k_funcinfo << "no EventManager tag" << endl;
	return false;
 }
 if (!d->mBoson->eventManager()->loadFromXML(eventManager)) {
	boError(270) << k_funcinfo << "unable to load EventManager" << endl;
	return false;
 }
 return true;
}

bool BosonSaveLoad::loadPlayersFromXML(const QString& playersXML)
{
 boDebug(270) << k_funcinfo << endl;
 QDomDocument doc(QString::fromLatin1("Players"));
 if (!loadXMLDoc(&doc, playersXML)) {
	addLoadError(SaveLoadError::LoadInvalidXML, i18n("Parsing error in players.xml"));
	return false;
 }
 QDomElement root = doc.documentElement();
 QDomNodeList list = root.elementsByTagName(QString::fromLatin1("Player"));
 if (list.count() < 1) {
	boError(270) << k_funcinfo << "no Player tags in file" << endl;
	addLoadError(SaveLoadError::LoadInvalidXML, i18n("No Player Tag in players.xml"));
	return false;
 }
 for (unsigned int i = 0; i < d->mBoson->playerCount(); i++) {
	Player* p = (Player*)d->mBoson->playerList()->at(i);
	QDomElement player;
	for (unsigned int j = 0; j < list.count() && player.isNull(); j++) {
		QDomElement e = list.item(j).toElement();
		bool ok = false;
		unsigned int id = e.attribute(QString::fromLatin1("PlayerId")).toUInt(&ok);
		if (!ok) {
			boError(270) << k_funcinfo << "missing or invalid PlayerId attribute for Player tag " << j << endl;
			continue;
		}
		if (p->id() != id) {
			continue;
		}
		player = e;
	}
	if (player.isNull()) {
		boError(270) << k_funcinfo << "no Player tag found for player with id " << p->id() << endl;
		return false;
	}
	if (i == d->mBoson->playerList()->count() - 1) {
		boDebug(270) << k_funcinfo << "loading neutral player" << endl;
		if (!player.hasAttribute("IsNeutral")) {
			boError(270) << k_funcinfo << "file format error: missing IsNeutral attribute for neutral player" << endl;
			return false;
		}
		bool ok = false;
		if (player.attribute("IsNeutral").toUInt(&ok) != 1) {
			boError(270) << k_funcinfo << "IsNeutral attribute must be 1, if present!" << endl;
			return false;
		} else if (!ok) {
			boError(270) << k_funcinfo << "invalid IsNeutral attribute (must be 1)!" << endl;
			return false;
		}
	}
	if (!p->loadFromXML(player)) {
		boError(270) << k_funcinfo << "failed loading player " << i << endl;
	}
 }
 return true;
}

bool BosonSaveLoad::loadCanvasFromXML(const QString& xml)
{
 boDebug(270) << k_funcinfo << endl;
 QDomDocument doc(QString::fromLatin1("Canvas"));
 if (!loadXMLDoc(&doc, xml)) {
	addLoadError(SaveLoadError::LoadInvalidXML, i18n("Parsing error in canvas.xml"));
	return false;
 }
 QDomElement root = doc.documentElement();

 if (!d->mCanvas->loadFromXML(root)) {
	addLoadError(SaveLoadError::LoadInvalidXML, i18n("error while loading canvas.xml"));
	return false;
 }

 return true;
}

bool BosonSaveLoad::loadExternalFromXML(const QString& xml)
{
 boDebug(260) << k_funcinfo << endl;
 // Load external stuff (camera)
 QDomDocument doc(QString::fromLatin1("External"));
 if (!loadXMLDoc(&doc, xml)) {
	addLoadError(SaveLoadError::LoadInvalidXML, i18n("Parsing error in external.xml"));
	return false;
 }
 QDomElement root = doc.documentElement();

 emit signalLoadExternalStuffFromXML(root);

 return true;
}

bool BosonSaveLoad::convertSaveGameToPlayField(QMap<QString, QByteArray>& files)
{
 BosonProfiler prof("convertSaveGameToPlayField()");
 // now we remove / change what does not belong there
 QDomDocument kgameDoc("Boson");
 if (!loadXMLDoc(&kgameDoc, QString(files["kgame.xml"]))) {
	boError() << k_funcinfo << "invalid kgame.xml file saved" << endl;
	return false;
 }
 QDomElement kgameRoot = kgameDoc.documentElement();
 kgameRoot.removeChild(kgameRoot.namedItem("DataHandler"));

 QDomDocument playersDoc("Players");
 if (!loadXMLDoc(&playersDoc, QString(files["players.xml"]))) {
	boError() << k_funcinfo << "invalid players.xml file saved" << endl;
	return false;
 }
 QDomElement playersRoot = playersDoc.documentElement();
 QDomNodeList playerList = playersRoot.elementsByTagName("Player");
 if (playerList.count() == 0) {
	boError() << k_funcinfo << "no players in game" << endl;
	return false;
 }
 for (unsigned int i = 0; i < playerList.count(); i++) {
	QDomElement p = playerList.item(i).toElement();
	p.removeAttribute("NetworkPriority");
	p.removeAttribute("UnitPropId");
	p.removeChild(p.namedItem("Statistics"));
	p.removeChild(p.namedItem("Fogged"));

	QDomElement speciesThemeTag = p.namedItem("SpeciesTheme").toElement();
	speciesThemeTag.removeAttribute("Identifier");
	speciesThemeTag.removeAttribute("TeamColor");
	bool ok = false;
	unsigned int id = p.attribute("PlayerId").toUInt(&ok);
	if (!ok) {
		boError() << k_funcinfo << "invalid value for PlayerId attribute of Player tag" << endl;
		return false;
	}
	if (id >= playerList.count()) {
		boError() << k_funcinfo << "invalid PlayerId for Player tag!" << endl;
		return false;
	}
	if (id != i) {
		boError() << k_funcinfo << "unexpected PlayerId " << id << " for Player tag - expected " << i << endl;
	}
	QDomElement handler = p.namedItem("DataHandler").toElement();
	if (handler.isNull()) {
		continue;
	}
	BosonPropertyXML::removeProperty(handler, KGamePropertyBase::IdGroup);
	BosonPropertyXML::removeProperty(handler, KGamePropertyBase::IdUserId);
	BosonPropertyXML::removeProperty(handler, KGamePropertyBase::IdAsyncInput);
	BosonPropertyXML::removeProperty(handler, KGamePropertyBase::IdTurn);
	BosonPropertyXML::removeProperty(handler, KGamePropertyBase::IdName);
 }

 QDomDocument canvasDoc("Canvas");
 if (!loadXMLDoc(&canvasDoc, QString(files["canvas.xml"]))) {
	boError() << k_funcinfo << "invalid canvas.xml file saved" << endl;
	return false;
 }
 QDomElement canvasRoot = canvasDoc.documentElement();
 canvasRoot.removeChild(canvasRoot.namedItem("Pathfinder"));
 canvasRoot.removeChild(canvasRoot.namedItem("Effects"));
 canvasRoot.appendChild(canvasDoc.createElement("Effects"));

 QByteArray kgameXML = kgameDoc.toCString();
 QByteArray playersXML = playersDoc.toCString();
 QByteArray canvasXML = canvasDoc.toCString();
 files.remove("external.xml");
 files.insert("kgame.xml", kgameXML);
 files.insert("players.xml", playersXML);
 files.insert("canvas.xml", canvasXML);

 QStringList list = files.keys();
 list = list.grep(QRegExp("^scripts\\/.*\\/data\\/"));
 for (QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
	files.remove(*it);
 }
 return true;
}


bool BosonSaveLoad::loadEventListenerScripts(const QMap<QString, QByteArray>& files)
{
 if (!boGame) {
	return false;
 }
 if (!boGame->eventManager()) {
	return false;
 }
 return boGame->eventManager()->loadListenerScripts(files);
}

bool BosonSaveLoad::saveEventListenerScripts(QMap<QString, QByteArray>* files)
{
 if (!boGame) {
	return false;
 }
 if (!boGame->eventManager()) {
	return false;
 }
 return boGame->eventManager()->saveListenerScripts(files);
}

bool BosonSaveLoad::convertPlayerIdsToIndices(QMap<QString, QByteArray>& files) const
{
 QStringList removeFiles;
 QMap<QString, QByteArray> addFiles;
 QRegExp hasPlayerId("-player_([0-9]+)");
 for (QMap<QString, QByteArray>::iterator it = files.begin(); it != files.end(); ++it) {
	int pos = hasPlayerId.search(it.key());
	if (pos < 0) {
		continue;
	}
	QString number = hasPlayerId.cap(1);
	bool ok;
	unsigned int id = number.toUInt(&ok);
	if (!ok) {
		boError() << k_funcinfo << it.key() << " does not contain a valid number" << endl;
		return false;
	}
	Player* p = (Player*)boGame->findPlayer(id);
	if (!p) {
		boError() << k_funcinfo << "no player with id " << id << " in game" << endl;
		return false;
	}
	QString file = it.key();
	QByteArray b = it.data();
	int index = boGame->playerList()->findRef(p);
	file.replace(hasPlayerId, QString("-player_%1").arg(index));
	removeFiles.append(it.key());
	addFiles.insert(file, b);
 }
 for (QStringList::iterator it = removeFiles.begin(); it != removeFiles.end(); ++it) {
	files.remove(*it);
 }
 for (QMap<QString, QByteArray>::iterator it = addFiles.begin(); it != addFiles.end(); ++it) {
	files.insert(it.key(), it.data());
 }
 return true;
}

