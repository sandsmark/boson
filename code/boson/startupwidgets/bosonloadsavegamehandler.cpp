/*
    This file is part of the Boson game
    Copyright (C) 2006 Andreas Beckermann (b_mann@gmx.de)

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

#include "bosonloadsavegamehandler.h"
#include "bosonloadsavegamehandler.moc"

#include "../../bomemory/bodummymemory.h"
#include "bosonstartupnetwork.h"
#include "../gameengine/bosonplayfield.h"
#include "../gameengine/boson.h"
#include "../gameengine/player.h"
#include "../gameengine/speciestheme.h"
#include "../gameengine/bosoncomputerio.h"
#include "../gameview/bosonlocalplayerinput.h"
#include "bodebug.h"

#include <qmap.h>
#include <qdom.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>

class BosonLoadSaveGameHandlerPrivate
{
public:
	BosonLoadSaveGameHandlerPrivate()
	{
	}
};

BosonLoadSaveGameHandler::BosonLoadSaveGameHandler(BosonStartupNetwork* interface, QObject* parent)
		: QObject(parent)
{
 d = new BosonLoadSaveGameHandlerPrivate();
 mNetworkInterface = interface;
}

BosonLoadSaveGameHandler::~BosonLoadSaveGameHandler()
{
 delete d;
}


void BosonLoadSaveGameHandler::slotLoadGame(const QString& fileName)
{
 boDebug() << k_funcinfo << fileName << endl;
 BO_CHECK_NULL_RET(boGame);
 if (fileName.isEmpty()) {
	return;
 }

 // AB: WARNING this is important and does more than the name says!
 // -> e.g. deletes _AND_ resets/recreates Boson
 emit signalGameOver();

 QByteArray data = prepareLoadGame(fileName);

 bool ret = networkInterface()->sendLoadGame(data);
 if (!ret) {
	boError() << k_funcinfo << "unable to load game from file " << fileName << endl;
	return;
 }
}

void BosonLoadSaveGameHandler::slotSaveGame(const QString& fileName, const QString& description, bool forceOverwrite)
{
 boDebug() << k_funcinfo << endl;
 QString file = fileName;
 if (file.isNull()) {
	return;
 }
 if (file[0] != '/') {
	boError() << k_funcinfo << "filename must be absolute" << endl;
	KMessageBox::sorry(0, i18n("Cannot save at given path (%1). Filename must be absolute at this point. Internal error.").arg(fileName));
	return;
 }
 if (file.findRev('.') == -1) {
	file += ".bsg";
 }
 if (!forceOverwrite && KStandardDirs::exists(file)) {
	int r = KMessageBox::questionYesNo(0, i18n("File %1 already exists. Overwrite?").arg(file));
	if (r != KMessageBox::Yes) {
		return;
	}
 }

 boDebug() << k_funcinfo << file << endl;
 bool ok = boGame->saveToFile(file);

 if (ok) {
	emit signalCancelLoadSave();
 } else {
	KMessageBox::sorry(0, i18n("Error while saving!"));
 }
}

QByteArray BosonLoadSaveGameHandler::prepareLoadGame(const QString& loadingFileName)
{
 if (loadingFileName.isNull()) {
	boError(260) << k_funcinfo << "Cannot load game with NULL filename" << endl;
	return QByteArray();
 }
 BosonPlayField loadField;
 if (!loadField.preLoadPlayField(loadingFileName)) {
	boError(260) << k_funcinfo << "could not preload " << loadingFileName << endl;
	return QByteArray();
 }

 QMap<QString, QByteArray> files;
 if (!loadField.loadFromDiskToFiles(files)) {
	boError(260) << k_funcinfo << "could not load " << loadingFileName << endl;
	return QByteArray();
 }
 QByteArray playField = BosonPlayField::streamFiles(files);
 if (playField.size() == 0) {
	boError(260) << k_funcinfo << "empty playfield loaded from " << loadingFileName << endl;
	return QByteArray();
 }

 if (!files.contains("players.xml")) {
	boError(260) << k_funcinfo << "did not find players.xml" << endl;
	return QByteArray();
 }
 if (!addLoadGamePlayers(files["players.xml"])) {
	boError(260) << k_funcinfo << "adding players failed" << endl;
	return QByteArray();
 }

 boDebug(270) << k_funcinfo << "done" << endl;

 return playField;
}

// note: this method is _incompatible_ with network!!
// if we want loading games to be network compatible, we need to add the players
// _before_ loading the game.
bool BosonLoadSaveGameHandler::addLoadGamePlayers(const QString& playersXML)
{
 QDomDocument playersDoc;
 if (!playersDoc.setContent(playersXML)) {
	boError(260) << k_funcinfo << "error loading players.xml" << endl;
	return false;
 }
 QDomElement playersRoot = playersDoc.documentElement();
 if (boGame->allPlayerCount() != 0) {
	boError(260) << k_funcinfo << "no player are allowed at this point" << endl;
	return false;
 }
 QDomNodeList list = playersRoot.elementsByTagName("Player");
 if (list.count() == 0) {
	boError(260) << k_funcinfo << "no players in savegame" << endl;
	return false;
 }
 boDebug(260) << k_funcinfo << "adding " << list.count() << " players" << endl;
 for (unsigned int i = 0; i < list.count(); i++) {
	QDomElement p = list.item(i).toElement();
	bool ok = false;
	unsigned int id = p.attribute("PlayerId").toUInt(&ok);
	if (!ok) {
		boError(260) << k_funcinfo << "invalid PlayerId" << endl;
		return false;
	}
	QDomElement speciesTheme = p.namedItem("SpeciesTheme").toElement();
	if (speciesTheme.isNull()) {
		boError(260) << k_funcinfo << "NULL SpeciesTheme tag for player " << i<< endl;
		return false;
	}
	QString species = speciesTheme.attribute(QString::fromLatin1("Identifier"));
	QColor color;
	color.setRgb(speciesTheme.attribute(QString::fromLatin1("TeamColor")).toUInt(&ok));
	if (!ok) {
		boError(260) << k_funcinfo << "invalid teamcolor" << endl;
		return false;
	}
	if (species.isEmpty()) {
		boError(260) << k_funcinfo << "invalid SpeciesTheme" << endl;
		// TODO: check whether species actually exists and can get
		// loaded
		return false;
	}
	if (boGame->findPlayerByUserId(id)) {
		boError(260) << k_funcinfo << "id " << id << " already in the game" << endl;
		return false;
	}
	Player* player = new Player();

#warning TODO save and load IOs
	// AB: the IOs of the players should be saved into the file for
	// savegames. they should NOT be saved for playfield files!
	//
	// on loading we should
	// * use the IOs requested by the user (e.g. it should be possible to
	//   save a single player game and load it as a multiplayer game)
	// * if none requested load the IO from the file

	// as a replacement we currently simply add a localIO to the first
	// player and a computer IO to all other players
	if (id >= 128 && id < 256) {
		if (i == 0) {
			BosonLocalPlayerInput* io = new BosonLocalPlayerInput();
			player->addGameIO(io);
			if (!io->initializeIO()) {
				boError() << k_funcinfo << "unable to initialize (local) IO for player " << id << endl;
				player->removeGameIO(io, true);
			} else {
				boDebug() << k_funcinfo << "added local IO to player " << id << endl;
			}
		} else {
			BosonComputerIO* io = new BosonComputerIO();
			player->addGameIO(io);
			if (!io->initializeIO()) {
				boError() << k_funcinfo << "unable to initialize IO for player " << id << endl;
				player->removeGameIO(io, true);
			} else {
				boDebug() << k_funcinfo << "added computer IO to player " << id << endl;
			}
		}
	}

	player->setUserId(id);
	player->loadTheme(SpeciesTheme::speciesDirectory(species), color);

	boGame->bosonAddPlayer(player);
 }

 return true;
}


