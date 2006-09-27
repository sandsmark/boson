/*
    This file is part of the Boson game
    Copyright (C) 2002-2005 Rivo Laks (rivolaks@hot.ee)
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

#include "bocreatenewmap.h"

#include "../gameengine/bosonmap.h"
#include "../gameengine/bosonplayfield.h"
#include "../bosondata.h"
#include "../gameengine/bpfdescription.h"
#include "../gameengine/bosongroundtheme.h"
#include "../gameengine/bosonsaveload.h"
#include "../defines.h"
#include "../gameengine/boson.h"
#include "../gameengine/boeventmanager.h"
#include "bodebug.h"

#include <kmessagebox.h>
#include <klocale.h>

#include <qdom.h>
#include <qmap.h>

BoCreateNewMap::BoCreateNewMap()
{
 mWidth = 0;
 mHeight = 0;
 mGroundTheme = 0;
 mGroundFilling = 0;
 mPlayerCount = 0;
}

BoCreateNewMap::~BoCreateNewMap()
{
}

void BoCreateNewMap::setSize(unsigned int width, unsigned int height)
{
 mWidth = width;
 mHeight = height;
}

void BoCreateNewMap::setGroundTheme(BosonGroundTheme* t)
{
 mGroundTheme = t;
}

void BoCreateNewMap::setGroundFilling(unsigned int f)
{
 mGroundFilling = f;
}

void BoCreateNewMap::setPlayerCount(unsigned int c)
{
 mPlayerCount = c;
}

void BoCreateNewMap::setName(const QString& name)
{
 mMapName = name;
}

QByteArray BoCreateNewMap::createNewMap()
{
 boDebug() << k_funcinfo << endl;
 if (!mGroundTheme) {
	BO_NULL_ERROR(mGroundTheme);
	return QByteArray();
 }
 if (mMapName.isEmpty()) {
	boError() << k_funcinfo << "an empty map name is not allowed" << endl;
	KMessageBox::sorry(0, i18n("Cannot create a map without a name. Please enter a map name."));
	return QByteArray();
 }
 if (!BosonMap::isValidMapGeo(mWidth, mHeight)) {
	boError() << k_funcinfo << "invalid map geo" << endl;
	KMessageBox::sorry(0, i18n("The desired map geo is not valid\nWidth=%1\nHeight=%2").arg(mWidth).arg(mHeight));
	return QByteArray();
 }
 BosonPlayField playField;
 BosonMap* map = new BosonMap(0);
 playField.changeMap(map); // takes ownership and will delete it!
 if (!map->createNewMap(mWidth, mHeight, mGroundTheme)) {
	boError() << k_funcinfo << "map could not be created" << endl;
	return QByteArray();
 }

 if (mGroundFilling >= mGroundTheme->groundTypeCount()) {
	boError() << k_funcinfo << "invalid groundtype for filling " << mGroundFilling << endl;
	KMessageBox::sorry(0, i18n("Could not fill the map with texture %1 - only %2 textures in groundTheme %3").arg(mGroundFilling).arg(mGroundTheme->groundTypeCount()).arg(mGroundTheme->identifier()));
	return QByteArray();
 }
 map->fill(mGroundFilling);

 // we add dummy players in order to save them into the bytearray.
 if (mPlayerCount < 2) {
	boError() << k_funcinfo << "playerCount < 2 does not make sense" << endl;
	KMessageBox::sorry(0, i18n("Max Players is an invalid value: %1").arg(mPlayerCount));
	return QByteArray();
 }
 if (mPlayerCount > BOSON_MAX_PLAYERS) {
	boError() << k_funcinfo << "mPlayerCount > " << BOSON_MAX_PLAYERS << " is not allowed" << endl;
	KMessageBox::sorry(0, i18n("Max Players is an invalid value: %1 (must be < %2)").arg(mPlayerCount).arg(BOSON_MAX_PLAYERS));
	return QByteArray();
 }

 BosonSaveLoad* save = new BosonSaveLoad(boGame);
 save->setPlayField(&playField);
 boDebug() << k_funcinfo << "saving to playfield completed" << endl;
 QMap<QString, QByteArray> files;
 if (!save->saveToFiles(files)) {
	delete save;
	boError() << k_funcinfo << "error occured while saving" << endl;
	KMessageBox::sorry(0, i18n("An error occured while saving the a game to a stream"));
	return QByteArray();
 }
 boDebug() << k_funcinfo << "saving completed" << endl;
 playField.changeMap(0); // deletes the map!

 QDomDocument playersDoc(QString::fromLatin1("Players"));
 QDomDocument canvasDoc(QString::fromLatin1("Canvas"));
 QDomElement playersRoot = playersDoc.createElement(QString::fromLatin1("Players"));
 QDomElement canvasRoot = canvasDoc.createElement(QString::fromLatin1("Canvas"));
 playersDoc.appendChild(playersRoot);
 canvasDoc.appendChild(canvasRoot);
 canvasRoot.appendChild(canvasDoc.createElement(QString::fromLatin1("DataHandler")));
 canvasRoot.appendChild(canvasDoc.createElement(QString::fromLatin1("Effects")));
 for (unsigned int i = 0; i < mPlayerCount + 1; i++) {
	QDomElement p = playersDoc.createElement(QString::fromLatin1("Player"));
	p.appendChild(playersDoc.createElement(QString::fromLatin1("Upgrades")));
	playersRoot.appendChild(p);
	QDomElement speciesTheme = playersDoc.createElement(QString::fromLatin1("SpeciesTheme"));
	p.appendChild(speciesTheme);
	speciesTheme.appendChild(playersDoc.createElement(QString::fromLatin1("UnitTypes")));

	QDomElement items = canvasDoc.createElement(QString::fromLatin1("Items"));
	canvasRoot.appendChild(items);

	if (i < mPlayerCount) {
		p.setAttribute("PlayerId", 128 + i);
		items.setAttribute("PlayerId", 128 + i);
	} else {
		p.setAttribute("PlayerId", 256);
		p.setAttribute("IsNeutral", 1);
		items.setAttribute("PlayerId", 256);
	}
 }
 QDomElement canvasEventListener = canvasDoc.createElement(QString::fromLatin1("EventListener"));
 canvasRoot.appendChild(canvasEventListener);
 canvasEventListener.appendChild(canvasDoc.createElement(QString::fromLatin1("Conditions")));
 files.insert("players.xml", playersDoc.toCString());
 files.insert("canvas.xml", canvasDoc.toCString());

 BPFDescription desc;
 desc.setName(mMapName);
 files.insert("C/description.xml", desc.toString().utf8());
 files.insert("scripts/eventlistener/game.py", QByteArray());
 files.insert("scripts/eventlistener/localplayer.py", createNewLocalPlayerScript());
 files.insert("scripts/eventlistener/gamevieweventlistener.py", QByteArray());
 QByteArray eventListenerXML = createEmptyEventListenerXML();
 for (unsigned int i = 0; i < mPlayerCount + 1; i++) {
	unsigned int id = 0;
	if (i <= mPlayerCount) {
		id = 128 + i;
	} else {
		// neutral player
		id = 256;
	}
	QByteArray script = createNewAIScript(id);
	files.insert(QString("scripts/eventlistener/ai-player_%1.py").arg(id), script);

	files.insert(QString("eventlistener/ai-player_%1.xml").arg(id), eventListenerXML);
 }
 files.insert("eventlistener/gameview.xml", eventListenerXML);
 files.insert("eventlistener/commandframe.xml", eventListenerXML);
 files.insert("eventlistener/localplayer.xml", eventListenerXML);
 files.insert("eventlistener/canvas.xml", eventListenerXML);

 QByteArray b = BosonPlayField::streamFiles(files);
 boDebug() << k_funcinfo << "files got streamed" << endl;
 return b;
}

QByteArray BoCreateNewMap::createNewLocalPlayerScript() const
{
 QByteArray localPlayerPy;
 QString script =
	"import dayandnight\n"
	"import wind\n"
	"\n"
	"player = -1\n"
	"\n"
	"def init(id):\n"
	"  global player\n"
	"  player = id\n"
	"  dayandnight.init()\n"
	"  wind.init()\n"
	"\n"
	"def setPlayerId(id):\n"
	"  global player\n"
	"  player = id\n"
	"\n"
	"def advance():\n"
	"  dayandnight.advance()\n"
	"  wind.advance()\n";
 localPlayerPy.duplicate(script.latin1(), script.length());
 return localPlayerPy;
}

QByteArray BoCreateNewMap::createNewAIScript(unsigned int playerId) const
{
 Q_UNUSED(playerId);
 QByteArray aiPy;
 QString script =
	"import ai\n"
	"\n"
	"def init(id):\n"
	"  ai.init(id)\n"
	"\n"
	"def setPlayerId(id):\n"
	"  ai.setPlayerId(id)\n"
	"\n"
	"def advance():\n"
	"  ai.advance()\n";
 aiPy.duplicate(script.latin1(), script.length());
 return aiPy;
}

QByteArray BoCreateNewMap::createEmptyEventListenerXML() const
{
 return BoEventManager::createEmptyEventListenerXML();
}


