/*
    This file is part of the Boson game
    Copyright (C) 2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "boson.h"
#include "bosonprofiling.h"
#include "bosonplayfield.h"
#include "bosonpropertyxml.h"
#include "bosoncanvas.h"
#include "bofile.h"
#include "player.h"
#include "bodebug.h"

// FIXME do not include
#include <startupwidgets/bosonloadingwidget.h>

#include <kgame/kgamemessage.h>
#include <kgame/kgamepropertyhandler.h>
#include <klocale.h>

#include <qdom.h>
#include <qdatastream.h>
#include <qptrqueue.h>

// Saving format version
#define BOSON_SAVEGAME_FORMAT_VERSION_MAJOR 0x00
#define BOSON_SAVEGAME_FORMAT_VERSION_MINOR 0x01
#define BOSON_SAVEGAME_FORMAT_VERSION_RELEASE 0x12
#define BOSON_SAVEGAME_FORMAT_VERSION \
	BOSON_MAKE_SAVEGAME_FORMAT_VERSION \
		( \
		BOSON_SAVEGAME_FORMAT_VERSION_MAJOR, \
		BOSON_SAVEGAME_FORMAT_VERSION_MINOR, \
		BOSON_SAVEGAME_FORMAT_VERSION_RELEASE \
		)

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

	BosonSaveLoad::LoadingStatus mLoadingStatus;
	QPtrQueue<SaveLoadError> mErrorQueue;
};

BosonSaveLoad::BosonSaveLoad(Boson* boson) : QObject(boson, "bosonsaveload")
{
 d = new BosonSaveLoadPrivate;
 d->mLoadingStatus = NotLoaded;
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

 connect(this, SIGNAL(signalLoadExternalStuff(QDataStream&)),
		d->mBoson, SIGNAL(signalLoadExternalStuff(QDataStream&)));
 connect(this, SIGNAL(signalSaveExternalStuff(QDataStream&)),
		d->mBoson, SIGNAL(signalSaveExternalStuff(QDataStream&)));
 connect(this, SIGNAL(signalLoadExternalStuffFromXML(const QDomElement&)),
		d->mBoson, SIGNAL(signalLoadExternalStuffFromXML(const QDomElement&)));
 connect(this, SIGNAL(signalSaveExternalStuffAsXML(QDomElement&)),
		d->mBoson, SIGNAL(signalSaveExternalStuffAsXML(QDomElement&)));


 connect(this, SIGNAL(signalLoadingPlayersCount(int)),
		d->mBoson, SIGNAL(signalLoadingPlayersCount(int)));
 connect(this, SIGNAL(signalLoadingPlayer(int)),
		d->mBoson, SIGNAL(signalLoadingPlayer(int)));
 connect(this, SIGNAL(signalLoadingType(int)),
		d->mBoson, SIGNAL(signalLoadingType(int)));

 connect(this, SIGNAL(signalLoadPlayerData(Player*)),
		d->mBoson, SIGNAL(signalLoadPlayerData(Player*)));
 connect(this, SIGNAL(signalInitMap(const QByteArray&)),
		d->mBoson, SIGNAL(signalInitMap(const QByteArray&)));
}

void BosonSaveLoad::systemAddPlayer(KPlayer* p)
{
 if (p && d->mBoson) {
	d->mBoson->systemAddPlayer_(this, p);
 }
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

BosonSaveLoad::LoadingStatus BosonSaveLoad::loadingStatus() const
{
 return d->mLoadingStatus;
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
 unsigned int version = root.attribute(QString::fromLatin1("SaveGameVersion")).toUInt(&ok);
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

bool BosonSaveLoad::saveToFile(Player* localPlayer, const QString& file)
{
 boDebug() << k_funcinfo << file << endl;
 if (!d->mBoson) {
	boError() << k_funcinfo << "NULL boson object" << endl;
	return false;
 }
 if (!d->mCanvas) {
	BO_NULL_ERROR(d->mCanvas);
	return false;
 }
 BosonProfiler profiler(BosonProfiling::SaveGameToXML);
 if (d->mBoson->playerCount() < 1) {
	boError() << k_funcinfo << "no players in game. cannot save" << endl;
	return false;
 }
 if (d->mBoson->gameStatus() == KGame::Init) {
	boError() << k_funcinfo << "Running game must not be in Init state" << endl;
	return false;
 }
 if (!d->mPlayField) {
	BO_NULL_ERROR(d->mPlayField);
	return false;
 }
 boProfiling->start(BosonProfiling::SaveKGameToXML);
 QString kgameXML = saveKGameAsXML();
 boProfiling->stop(BosonProfiling::SaveKGameToXML);
 if (kgameXML.isNull()) {
	return false;
 }

 boProfiling->start(BosonProfiling::SavePlayersToXML);
 QString playersXML = savePlayersAsXML(localPlayer);
 boProfiling->stop(BosonProfiling::SavePlayersToXML);
 if (playersXML.isNull()) {
	return false;
 }

 boProfiling->start(BosonProfiling::SaveCanvasToXML);
 QString canvasXML = saveCanvasAsXML();
 boProfiling->stop(BosonProfiling::SaveCanvasToXML);
 if (canvasXML.isNull()) {
	return false;
 }

 boProfiling->start(BosonProfiling::SaveExternalToXML);
 QString externalXML = saveExternalAsXML();
 boProfiling->stop(BosonProfiling::SaveExternalToXML);
 if (externalXML.isNull()) {
	return false;
 }



 // we store the map as binary. XML would take a lot of space and time to save
 // and load.
 QByteArray map;
 QDataStream stream(map, IO_WriteOnly);
 boProfiling->start(BosonProfiling::SavePlayFieldToXML);
 d->mPlayField->savePlayFieldForRemote(stream); // we emulate a network stream.
 boProfiling->stop(BosonProfiling::SavePlayFieldToXML);

 BosonProfiler writeProfiler(BosonProfiling::SaveGameToXMLWriteFile);
 BSGFile f(file, false);
 if (!f.writeFile(QString::fromLatin1("kgame.xml"), kgameXML)) {
	boError() << k_funcinfo << "Could not write kgame.xml to " << file << endl;
	return false;
 }
 if (!f.writeFile(QString::fromLatin1("players.xml"), playersXML)) {
	boError() << k_funcinfo << "Could not write players.xml to " << file << endl;
	return false;
 }
 if (!f.writeFile(QString::fromLatin1("canvas.xml"), canvasXML)) {
	boError() << k_funcinfo << "Could not write canvas.xml to " << file << endl;
	return false;
 }
 if (!f.writeFile(QString::fromLatin1("external.xml"), externalXML)) {
	boError() << k_funcinfo << "Could not write external.xml to " << file << endl;
	return false;
 }
 if (!f.writeFile(QString::fromLatin1("map"), map)) {
	boError() << k_funcinfo << "Could not write map to " << file << endl;
	return false;
 }
 writeProfiler.stop(); // in case we add something below one day :)

 return true;
}

QString BosonSaveLoad::saveKGameAsXML()
{
 QDomDocument doc(QString::fromLatin1("Boson"));
 QDomElement root = doc.createElement(QString::fromLatin1("Boson")); // XML file for the Boson object
 doc.appendChild(root);
 root.setAttribute(QString::fromLatin1("SaveGameVersion"), BOSON_SAVEGAME_FORMAT_VERSION);

 // store the dataHandler()
 BosonCustomPropertyXML propertyXML;
 QDomElement handler = doc.createElement(QString::fromLatin1("DataHandler"));
 if (!propertyXML.saveAsXML(handler, d->mBoson->dataHandler())) { // AB: we should exclude gameStatus from this! we should stay in KGame::Init! -> add a BosonPropertyXML::remove() or so
	boError() << k_funcinfo << "unable to save KGame data handler" << endl;
	return QString::null;
 }
 root.appendChild(handler);

 // here we could add additional data for the Boson object.
 // but I believe all data should get into the datahandler as far as possible

 return doc.toString();
}

QString BosonSaveLoad::savePlayersAsXML(Player* localPlayer)
{
 QDomDocument doc(QString::fromLatin1("Players"));
 QDomElement root = doc.createElement(QString::fromLatin1("Players"));
 doc.appendChild(root);

 if (!d->mBoson) {
	BO_NULL_ERROR(d->mBoson);
	return doc.toString();
 }

 KGame::KGamePlayerList* list = d->mBoson->playerList();
 for (KPlayer* p = list->first(); p; p = list->next()) {
	// KGame also stored ID, RTTI and KPlayer::calcIOValue() here.
	// I believe we won't need them. ID might be useful for network games,
	// but we load from a file here.
	QDomElement element = doc.createElement(QString::fromLatin1("Player"));
	if (!((Player*)p)->saveAsXML(element)) {
		boError() << k_funcinfo << "Unable to save player " << p->id() << endl;
		continue;
	}
	root.appendChild(element);
 }

 // save the ID of the local player
 if (localPlayer) {
	root.setAttribute(QString::fromLatin1("LocalPlayerId"), (unsigned int)localPlayer->id());
 } else {
	// this might be the case for network games.
	// (when a player enters the game it is saved on the ADMIN and loaded on
	// the new client).
	// But currently we don't use XML for that. Maybe we will never do.
	boWarning() << k_funcinfo << "NULL local player" << endl;
 }

 return doc.toString();
}

QString BosonSaveLoad::saveCanvasAsXML()
{
 QDomDocument doc(QString::fromLatin1("Canvas"));
 QDomElement root = doc.createElement(QString::fromLatin1("Canvas")); // XML file for canvas
 doc.appendChild(root);

 if (d->mCanvas) {
	d->mCanvas->saveAsXML(root);
 }

 return doc.toString();
}

QString BosonSaveLoad::saveExternalAsXML()
{
 QDomDocument doc(QString::fromLatin1("External"));
 QDomElement root = doc.createElement(QString::fromLatin1("External")); // XML file for external data
 doc.appendChild(root);

 emit signalSaveExternalStuffAsXML(root);

 return doc.toString();
}


bool BosonSaveLoad::loadFromFile(const QString& file)
{
 boDebug(260) << k_funcinfo << endl;
 d->mLoadingStatus = LoadingInProgress;
 BSGFile f(file, true);
 if (!f.checkTar()) {
	boError(260) << k_funcinfo << "Could not load from " << file << endl;
	addLoadError(SaveLoadError::LoadBSGFileError, i18n("tar archive invalid"));
	d->mLoadingStatus = BSGFileError;
	return false;
 }

 // kgame.xml is mandatory in all boson savegame versions. We can get the
 // savegame format version from there (which is also mandatory) so we load this
 // first.
 QString kgameXML;
 kgameXML = QString(f.kgameData());
 if (kgameXML.isEmpty()) {
	boError(260) << k_funcinfo << "Empty kgameXML" << endl;
	addLoadError(SaveLoadError::LoadBSGFileError, i18n("empty kgame.xml"));
	d->mLoadingStatus = BSGFileError;
	return false;
 }
 unsigned int version = savegameFormatVersion(kgameXML);
 if (version > latestSavegameVersion()) {
	boError(260) << "version " << version << " is too recent. Can read up to " << latestSavegameVersion() << " only." << endl;
	d->mLoadingStatus = InvalidVersion;
	addLoadError(SaveLoadError::LoadBSGFileError, i18n("savegame version (%1) is too recent. Can read up to %2 only.").arg(version).arg(latestSavegameVersion()));
	return false;
 }
 if (version < BOSON_SAVEGAME_FORMAT_VERSION_0_8) {
	boError(260) << "version " << version << " is too old. Must be at least from boson 0.8" << endl;
	addLoadError(SaveLoadError::LoadBSGFileError, i18n("savegame version (%1) is too old. Must be at least %2.").arg(version).arg(BOSON_SAVEGAME_FORMAT_VERSION_0_8));
	d->mLoadingStatus = InvalidVersion;
	return false;
 }
 if (version != latestSavegameVersion()) {
	boDebug(260) << "version " << version << " must be converted first. trying to convert now" << endl;

	return false;
 }

 QString playersXML;
 QString canvasXML;
 QString externalXML;
 QByteArray map;

 playersXML = QString(f.playersData());
 canvasXML = QString(f.canvasData());
 externalXML = QString(f.externalData());
 map = f.mapData();

 if (playersXML.isEmpty()) {
	boError(260) << k_funcinfo << "Empty playersXML" << endl;
	addLoadError(SaveLoadError::LoadBSGFileError, i18n("empty file: players.xml"));
	d->mLoadingStatus = BSGFileError;
	return false;
 }
 if (canvasXML.isEmpty()) {
	boError(260) << k_funcinfo << "Empty canvasXML" << endl;
	addLoadError(SaveLoadError::LoadBSGFileError, i18n("empty file: canvas.xml"));
	d->mLoadingStatus = BSGFileError;
	return false;
 }
 if (externalXML.isEmpty()) {
	boError(260) << k_funcinfo << "Empty externalXML" << endl;
	addLoadError(SaveLoadError::LoadBSGFileError, i18n("empty file: external.xml"));
	d->mLoadingStatus = BSGFileError;
	return false;
 }
 if (map.isEmpty()) {
	boError(260) << k_funcinfo << "Empty map" << endl;
	addLoadError(SaveLoadError::LoadBSGFileError, i18n("empty file: map"));
	d->mLoadingStatus = BSGFileError;
	return false;
 }

 bool ok = d->mBoson->reset();
 if (!ok) {
	addLoadError(SaveLoadError::General, i18n("KGame::reset() failed"));
	d->mLoadingStatus = KGameError;
	return false;
 }
 if (d->mBoson->playerCount() != 0) {
	boError(260) << k_funcinfo << "not all players removed!" << endl;
	addLoadError(SaveLoadError::LoadBSGFileError, i18n("bug: not all players removed from game"));
	d->mLoadingStatus = BSGFileError;
	return false;
 }

 if (!loadKGameFromXML(kgameXML)) {
	return false;
 }

 // KGame::loadgame() also loads KGame::d->mUniquePlayerNumber !!
 // KGame::loadgame() also loads a seed for KGame::random() (not so important)

 if (!loadPlayersFromXML(playersXML)) {
	return false;
 }

 boDebug(260) << k_funcinfo << d->mBoson->playerCount() << " players loaded" << endl;

 // now load the map (must happen before loading units)
 // AB: can we move this before loading the players?
 emit signalLoadingType(BosonLoadingWidget::ReceiveMap);
 emit signalInitMap(map);


 // Load player data (e.g. unit configs, unit models, ...
 // TODO: since we use XML for savegames now we should be able to include this
 // to regular startup code in BosonStarting!
 KPlayer* p;
 int current = 0;
 for (p = d->mBoson->playerList()->first(); p; p = d->mBoson->playerList()->next(), current++) {
	emit signalLoadingPlayer(current);
	emit signalLoadPlayerData((Player*)p);
	emit signalLoadingType(BosonLoadingWidget::LoadSavedUnits);
 }

 boDebug(260) << k_funcinfo << "loading units" << endl;

 // construct a list of all players first
 // WARNING: HACK!
 // we are assuming that the players in the playerList() are inserted in exactly
 // the same order as they appear in the XML file.
 // we can assume so, as this code was splitted from loadPlayersXML().
 // we will soon move units to canvas.xml, so this hack will be removed soon.
 QMap<Player*, QDomElement> player2Element;
 QDomDocument doc(QString::fromLatin1("Boson"));
 if (!loadXMLDoc(&doc, playersXML)) {
	addLoadError(SaveLoadError::LoadInvalidXML, i18n("parsing error while reading players.xml"));
	d->mLoadingStatus = InvalidXML;
	return false;
 }
 QDomElement root = doc.documentElement();
 QDomNodeList list = root.elementsByTagName(QString::fromLatin1("Player"));
 for (unsigned int i = 0; i < list.count(); i++) {
	QDomElement player = list.item(i).toElement();
	if (player.isNull()) {
		boError(260) << k_funcinfo << "NULL player node" << endl;
		continue;
	}
	KPlayer* kp = d->mBoson->playerList()->first();
	for (unsigned int j = 0; kp != 0 && j != i; j++) {
		kp = d->mBoson->playerList()->next();
	}
	Player* p = (Player*)kp;
	if (!p) {
		boError(260) << k_funcinfo << "Oops - NULL player found for XML element" << endl;
		continue;
	}
	player2Element.insert(p, player);
 }


 QMap<Player*, QDomElement>::Iterator it;
 for (it = player2Element.begin(); it != player2Element.end(); ++it) {
	Player* p = it.key();
	if (!p->speciesTheme()) {
		boError() << k_funcinfo << "NULL speciesTheme" << endl;
		continue;
	}
	QDomElement e = it.data();
	p->loadUnitsFromXML(e);
 }

 // Load canvas (shots)
 if (!loadCanvasFromXML(canvasXML)) {
	addLoadError(SaveLoadError::General, i18n("error while loading canvas"));
	return false;
 }

 // Load external stuff (camera)
 if (!loadExternalFromXML(externalXML)) {
	addLoadError(SaveLoadError::General, i18n("error while loading external data"));
	return false;
 }

 d->mLoadingStatus = LoadingCompleted;
 return true;
}

bool BosonSaveLoad::loadKGameFromXML(const QString& xml)
{
 boDebug() << k_funcinfo << endl;
 QDomDocument doc(QString::fromLatin1("Boson"));
 if (!loadXMLDoc(&doc, xml)) {
	addLoadError(SaveLoadError::LoadInvalidXML, i18n("Parsing error in kgame.xml"));
	d->mLoadingStatus = InvalidXML;
	return false;
 }
 QDomElement root = doc.documentElement();
 bool ok = false;
 unsigned int version = root.attribute(QString::fromLatin1("SaveGameVersion")).toUInt(&ok);
 if (!ok) {
	boError() << k_funcinfo << "savegame version not a valid number: " << root.attribute(QString::fromLatin1("SaveGameVersion")) << endl;
	addLoadError(SaveLoadError::LoadInvalidXML, i18n("savegame version in kgame.xml is not a valid number"));
	d->mLoadingStatus = InvalidXML;
	return false;
 }
 if (version != latestSavegameVersion()) {
	boError() << "savegame version " << version << " not supported (latest is " << latestSavegameVersion() << ")" << endl;
	addLoadError(SaveLoadError::LoadInvalidXML, i18n("savegame version %1 is not supported").arg(version));
	d->mLoadingStatus = InvalidVersion;
	return false;
 }

 // load the datahandler
 QDomElement handler = root.namedItem(QString::fromLatin1("DataHandler")).toElement();
 if (handler.isNull()) {
	boError() << k_funcinfo << "No DataHandler tag found" << endl;
	addLoadError(SaveLoadError::LoadInvalidXML, i18n("No DataHandler tag found in kgame.xml"));
	d->mLoadingStatus = InvalidXML;
	return false;
 }
 BosonCustomPropertyXML propertyXML;
 if (!propertyXML.loadFromXML(handler, d->mBoson->dataHandler())) {
	boError() << k_funcinfo << "unable to load KGame data handler" << endl;
	addLoadError(SaveLoadError::LoadInvalidXML, i18n("Error while loading DataHandler of kgame.xml"));
	d->mLoadingStatus = InvalidXML;
	return false;
 }
#warning dont load gamestatus!
 // FIXME: the property handler also loads KGame::gameStatus(), but it must
 // remain in Init state until we completed loading!
 // this is a workaround for this problem:
 {
	// set gameStatus to Init. Will be set to Run later
	QByteArray b;
	QDataStream s(b, IO_WriteOnly);
	KGameMessage::createPropertyHeader(s, KGamePropertyBase::IdGameStatus);
	s << (int)KGame::Init;
	QDataStream readStream(b, IO_ReadOnly);
	d->mBoson->dataHandler()->processMessage(readStream, d->mBoson->dataHandler()->id(), false);
 }

 return true;
}

bool BosonSaveLoad::loadPlayersFromXML(const QString& playersXML)
{
 // now load the players (not the units!)
 QDomDocument doc(QString::fromLatin1("Boson"));
 if (!loadXMLDoc(&doc, playersXML)) {
	addLoadError(SaveLoadError::LoadInvalidXML, i18n("Parsing error in players.xml"));
	d->mLoadingStatus = InvalidXML;
	return false;
 }
 QDomElement root = doc.documentElement();
 if (!root.hasAttribute(QString::fromLatin1("LocalPlayerId"))) {
	boError(260) << k_funcinfo << "missing attribute: LocalPlayerId" << endl;
	addLoadError(SaveLoadError::LoadInvalidXML, i18n("Missing attribute LocalPlayerId in players.xml"));
	d->mLoadingStatus = InvalidXML;
	return false;
 }
 bool ok = false;
 unsigned int localId = root.attribute(QString::fromLatin1("LocalPlayerId")).toUInt(&ok);
 if (!ok) {
	boError(260) << k_funcinfo << "invalid LocalPlayerId: " << root.attribute(QString::fromLatin1("LocalPlayerId")) << endl;
	addLoadError(SaveLoadError::LoadInvalidXML, i18n("Invalid value for LocalPlayerId in players.xml"));
	d->mLoadingStatus = InvalidXML;
	return false;
 }
 QDomNodeList list = root.elementsByTagName(QString::fromLatin1("Player"));
 if (list.count() < 1) {
	boError(260) << k_funcinfo << "no Player tags in file" << endl;
	addLoadError(SaveLoadError::LoadInvalidXML, i18n("No Player Tag in players.xml"));
	d->mLoadingStatus = InvalidXML;
	return false;
 }
 boDebug(260) << k_funcinfo << "loading " << list.count() << " players" << endl;
 emit signalLoadingPlayersCount(list.count());
 Player* localPlayer = 0;
 for (unsigned int i = 0; i < list.count(); i++) {
	boDebug(260) << k_funcinfo << "creating player " << i << endl;
	QDomElement player = list.item(i).toElement();
	if (player.isNull()) {
		boError(260) << k_funcinfo << "NULL player node" << endl;
		continue;
	}
	bool ok = false;
	unsigned int id = player.attribute(QString::fromLatin1("Id")).toUInt(&ok);
	if (!ok) {
		boError(260) << k_funcinfo << "Id tag of player " << i << " not a valid number" << endl;
		addLoadError(SaveLoadError::LoadPlayersError, i18n("Id of player %1 not a valid number").arg(i));
		continue;
	}
	if (d->mBoson->findPlayer(id)) {
		addLoadError(SaveLoadError::LoadPlayersError, i18n("Player with Id %1 appears twice! Ignoring second player with this Id. File is broken."));
		continue;
	}
	Player* p = (Player*)d->mBoson->createPlayer(0, 0, false); // we ignore all params anyway.
	if (id == localId) {
		localPlayer = p;
	}
	p->loadFromXML(player);
	systemAddPlayer((KPlayer*)p);

 }
 if (!localPlayer) {
	boWarning(260) << k_funcinfo << "local player NOT found" << endl;
	addLoadError(SaveLoadError::LoadPlayersError, i18n("No local player found"));
 }
 d->mBoson->setLocalPlayer(localPlayer);

 return true;
}

bool BosonSaveLoad::loadCanvasFromXML(const QString& xml)
{
 boDebug() << k_funcinfo << endl;
 QDomDocument doc(QString::fromLatin1("Canvas"));
 if (!loadXMLDoc(&doc, xml)) {
	addLoadError(SaveLoadError::LoadInvalidXML, i18n("Parsing error in canvas.xml"));
	d->mLoadingStatus = InvalidXML;
	return false;
 }
 QDomElement root = doc.documentElement();

 if (!d->mCanvas->loadFromXML(root)) {
	addLoadError(SaveLoadError::LoadInvalidXML, i18n("error while loading canvas.xml"));
	d->mLoadingStatus = InvalidXML;
	return false;
 }

 return true;
}

bool BosonSaveLoad::loadExternalFromXML(const QString& xml)
{
 boDebug() << k_funcinfo << endl;
 // Load external stuff (camera)
 QDomDocument doc(QString::fromLatin1("External"));
 if (!loadXMLDoc(&doc, xml)) {
	addLoadError(SaveLoadError::LoadInvalidXML, i18n("Parsing error in external.xml"));
	d->mLoadingStatus = InvalidXML;
	return false;
 }
 QDomElement root = doc.documentElement();

 emit signalLoadExternalStuffFromXML(root);

 return true;
}

