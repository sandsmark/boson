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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
#include "boeventmanager.h"

#include <kgame/kgamemessage.h>
#include <kgame/kgamepropertyhandler.h>
#include <klocale.h>

#include <qdom.h>
#include <qdatastream.h>
#include <qptrqueue.h>
#include <qvaluelist.h>
#include <qregexp.h>

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
};

BosonSaveLoad::BosonSaveLoad(Boson* boson) : QObject(boson, "bosonsaveload")
{
 d = new BosonSaveLoadPrivate;
 d->mBoson = boson;

 initBoson();
 d->mBoson->initSaveLoad(this);
}

BosonSaveLoad::~BosonSaveLoad()
{
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
 files.insert("kgame.xml", kgameXML);

 QByteArray playersXML = savePlayersAsXML();
 if (playersXML.isNull()) {
	return false;
 }
 files.insert("players.xml", playersXML);

 QByteArray canvasXML = saveCanvasAsXML();
 if (canvasXML.isNull()) {
	return false;
 }
 files.insert("canvas.xml", canvasXML);

 QByteArray externalXML = saveExternalAsXML();
 if (externalXML.isNull()) {
	return false;
 }
 files.insert("external.xml", externalXML);

 if (!saveEventListenerScripts(&files)) {
	boError() << k_funcinfo << "could not save event listener scripts" << endl;
	return false;
 }
 if (!saveEventListenersXML(&files)) {
	boError() << k_funcinfo << "could not save event listener scripts" << endl;
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
 QStringList writtenFiles;
 QByteArray kgameXML = files["kgame.xml"];
 QByteArray playersXML = files["players.xml"];
 QByteArray canvasXML = files["canvas.xml"];
 QByteArray externalXML = files["external.xml"];
 QByteArray mapXML = files["map/map.xml"];
 QByteArray waterXML = files["map/water.xml"];
 QByteArray heightMap = files["map/heightmap.png"];
 QByteArray texMap = files["map/texmap"];
 QByteArray descriptionXML = files["C/description.xml"];
 QByteArray mapPreviewPNG = files["mappreview/map.png"];
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
 writtenFiles.append("kgame.xml");
 if (!f.writeFile(QString::fromLatin1("players.xml"), QString(playersXML))) {
	boError() << k_funcinfo << "Could not write players.xml to " << file << endl;
	return false;
 }
 writtenFiles.append("players.xml");
 if (!f.writeFile(QString::fromLatin1("canvas.xml"), QString(canvasXML))) {
	boError() << k_funcinfo << "Could not write canvas.xml to " << file << endl;
	return false;
 }
 writtenFiles.append("canvas.xml");
 if (!f.writeFile(QString::fromLatin1("external.xml"), QString(externalXML))) {
	boError() << k_funcinfo << "Could not write external.xml to " << file << endl;
	return false;
 }
 writtenFiles.append("external.xml");
 if (!f.writeFile(QString::fromLatin1("map.xml"), QString(mapXML), QString::fromLatin1("map"))) {
	boError() << k_funcinfo << "Could not write map to " << file << endl;
	return false;
 }
 writtenFiles.append("map/map.xml");
 if (!f.writeFile(QString::fromLatin1("water.xml"), QString(waterXML), QString::fromLatin1("map"))) {
	boError() << k_funcinfo << "Could not write water to " << file << endl;
	return false;
 }
 writtenFiles.append("map/water.xml");
 if (!f.writeFile(QString::fromLatin1("heightmap.png"), heightMap, QString::fromLatin1("map"))) {
	boError() << k_funcinfo << "Could not write map to " << file << endl;
	return false;
 }
 writtenFiles.append("map/heightmap.png");
 if (!f.writeFile(QString::fromLatin1("texmap"), texMap, QString::fromLatin1("map"))) {
	boError() << k_funcinfo << "Could not write map to " << file << endl;
	return false;
 }
 writtenFiles.append("map/texmap");
 if (!f.writeFile(QString::fromLatin1("description.xml"), QString(descriptionXML), QString::fromLatin1("C"))) {
	boError() << k_funcinfo << "Could not write map to " << file << endl;
	return false;
 }
 writtenFiles.append("C/description.xml");
 if (!f.writeFile(QString::fromLatin1("map.png"), mapPreviewPNG, QString::fromLatin1("mappreview"))) {
	boError() << k_funcinfo << "Could not write mappreview to " << file << endl;
	return false;
 }
 writtenFiles.append("mappreview/map.png");

 QStringList scripts = QStringList(files.keys()).grep(QRegExp("^scripts"));
 for (QStringList::iterator it = scripts.begin(); it != scripts.end(); ++it) {
	QString path = *it;
	int lastSlash = path.findRev('/');
	QString dir = path.left(lastSlash);
	QString baseName = path.right(path.length() - (lastSlash + 1));
	if (!f.writeFile(baseName, files[path], dir)) {
		boError() << k_funcinfo << "Could not write " << baseName << " to " << file << endl;
		return false;
	}
	writtenFiles.append(*it);
 }

 QStringList eventListener = QStringList(files.keys()).grep(QRegExp("^eventlistener"));
 for (QStringList::iterator it = eventListener.begin(); it != eventListener.end(); ++it) {
	QString path = *it;
	int lastSlash = path.findRev('/');
	QString dir = path.left(lastSlash);
	QString baseName = path.right(path.length() - (lastSlash + 1));
	bool ret = false;
	if (baseName.endsWith(".xml")) {
		ret = f.writeFile(baseName, QString(files[path]), dir);
	} else {
		ret = f.writeFile(baseName, files[path], dir);
	}
	if (!ret) {
		boError() << k_funcinfo << "Could not write " << baseName << " to " << file << endl;
		return false;
	}
	writtenFiles.append(*it);
 }


 QStringList allFiles = files.keys();
 for (QStringList::iterator it = allFiles.begin(); it != allFiles.end(); ++it) {
	if (!writtenFiles.contains(*it)) {
		boWarning() << k_funcinfo << "file not written: " << *it << endl;
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

 QPtrList<Player> list = *d->mBoson->gamePlayerList();
 boDebug() << k_funcinfo << "saving " << list.count() << " players" << endl;
 for (Player* p = list.first(); p; p = list.next()) {
	// KGame also stored ID, RTTI and KPlayer::calcIOValue() here.
	// I believe we won't need them. ID might be useful for network games,
	// but we load from a file here.
	QDomElement element = doc.createElement(QString::fromLatin1("Player"));
	if (!p->saveAsXML(element)) {
		boError() << k_funcinfo << "Unable to save player " << p->bosonId() << endl;
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


bool BosonSaveLoad::loadKGameFromXML(const QMap<QString, QByteArray>& files)
{
 PROFILE_METHOD
 QString kgameXML = QString(files["kgame.xml"]);
 if (kgameXML.length() == 0) {
	boError(270) << k_funcinfo << "Empty kgameXML" << endl;
	return false;
 }
 QDomDocument doc(QString::fromLatin1("Boson"));
 if (!loadXMLDoc(&doc, kgameXML)) {
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

bool BosonSaveLoad::loadPlayersFromXML(const QMap<QString, QByteArray>& files)
{
 PROFILE_METHOD
 boDebug(270) << k_funcinfo << endl;
 QString playersXML = QString(files["players.xml"]);
 if (playersXML.length() == 0) {
	boError(270) << k_funcinfo << "Empty playersXML" << endl;
	return false;
 }
 QDomDocument doc(QString::fromLatin1("Players"));
 if (!loadXMLDoc(&doc, playersXML)) {
	return false;
 }
 QDomElement root = doc.documentElement();
 QDomNodeList list = root.elementsByTagName(QString::fromLatin1("Player"));
 if (list.count() < 1) {
	boError(270) << k_funcinfo << "no Player tags in file" << endl;
	return false;
 }
 for (unsigned int i = 0; i < d->mBoson->gamePlayerCount(); i++) {
	Player* p = (Player*)d->mBoson->gamePlayerList()->at(i);
	QDomElement player;
	for (unsigned int j = 0; j < list.count() && player.isNull(); j++) {
		QDomElement e = list.item(j).toElement();
		bool ok = false;
		int id = e.attribute(QString::fromLatin1("PlayerId")).toInt(&ok);
		if (!ok) {
			boError(270) << k_funcinfo << "missing or invalid PlayerId attribute for Player tag " << j << endl;
			continue;
		}
		if (p->bosonId() != (int)id) {
			continue;
		}
		player = e;
	}
	if (player.isNull()) {
		boError(270) << k_funcinfo << "no Player tag found for player with id " << p->bosonId() << endl;
		return false;
	}
	if (p->bosonId() == 256) {
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
		return false;
	}
 }
 return true;
}

bool BosonSaveLoad::loadCanvasFromXML(const QMap<QString, QByteArray>& files)
{
 PROFILE_METHOD
 boDebug(270) << k_funcinfo << endl;
 QString xml = QString(files["canvas.xml"]);
 if (xml.length() == 0) {
	boError(270) << k_funcinfo << "Empty canvas.xml" << endl;
	return false;
 }
 QDomDocument doc(QString::fromLatin1("Canvas"));
 if (!loadXMLDoc(&doc, xml)) {
	return false;
 }
 QDomElement root = doc.documentElement();

 if (!d->mCanvas->loadFromXML(root)) {
	return false;
 }

 return true;
}

bool BosonSaveLoad::loadExternalFromXML(const QMap<QString, QByteArray>& files)
{
 PROFILE_METHOD
 boDebug(270) << k_funcinfo << endl;
 QString xml = QString(files["external.xml"]);

 // external.xml is optional only, it's valid that it's missing
 // if it is, we use a dummy document. this is meant to make sure that all
 // pointers are set correctly, even if we don't use them (e.g. canvas pointers
 // in all widgets that require it)
 QDomDocument doc(QString::fromLatin1("External"));
 if (xml.length() != 0) {
	boDebug(260) << k_funcinfo << endl;
	if (!loadXMLDoc(&doc, xml)) {
		return false;
	}
 } else {
	QDomElement root = doc.createElement(QString::fromLatin1("External"));
	doc.appendChild(root);

	QDomElement unitGroups = doc.createElement("UnitGroups");
	root.appendChild(unitGroups);

	QDomElement effects = doc.createElement("Effects");
	root.appendChild(effects);

	QDomElement displays = doc.createElement("Displays");
	root.appendChild(displays);
	QDomElement display = doc.createElement("Display");
	displays.appendChild(display);

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
	p.removeChild(p.namedItem("Explored"));

	QDomElement speciesThemeTag = p.namedItem("SpeciesTheme").toElement();
	speciesThemeTag.removeAttribute("Identifier");
	speciesThemeTag.removeAttribute("TeamColor");
	bool ok = false;
	unsigned int id = p.attribute("PlayerId").toUInt(&ok);
	if (!ok) {
		boError() << k_funcinfo << "invalid value for PlayerId attribute of Player tag" << endl;
		return false;
	}
	if (id < 128) {
		boError() << k_funcinfo << "invalid PlayerId for Player tag!" << endl;
		return false;
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
 PROFILE_METHOD
 if (!boGame) {
	return false;
 }
 if (!boGame->eventManager()) {
	return false;
 }
 if (!boGame->eventManager()->copyEventListenerScripts(files)) {
	boError() << k_funcinfo << "unable to copy event listener scripts" << endl;
	return false;
 }
 return boGame->eventManager()->loadAllEventListenerScripts();
}

bool BosonSaveLoad::saveEventListenerScripts(QMap<QString, QByteArray>* files)
{
 if (!boGame) {
	return false;
 }
 if (!boGame->eventManager()) {
	return false;
 }
 return boGame->eventManager()->saveAllEventListenerScripts(files);
}

bool BosonSaveLoad::loadEventListenersXML(const QMap<QString, QByteArray>& files)
{
 PROFILE_METHOD
 if (!boGame) {
	return false;
 }
 if (!boGame->eventManager()) {
	return false;
 }
 if (!boGame->eventManager()->copyEventListenerXML(files)) {
	boError() << k_funcinfo << "unable to copy event listeners XML" << endl;
	return false;
 }
 return boGame->eventManager()->loadAllEventListenersXML();
}

bool BosonSaveLoad::saveEventListenersXML(QMap<QString, QByteArray>* files)
{
 if (!boGame) {
	return false;
 }
 if (!boGame->eventManager()) {
	return false;
 }
 return boGame->eventManager()->saveAllEventListenersXML(files);
}

