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

#include "bosonfileconverter.h"

#include "bodebug.h"
#include "defines.h"
#include "boversion.h"

#include <qdatastream.h>
#include <qcolor.h>
#include <qvaluelist.h>
#include <qdom.h>
#include <qimage.h>
#include <qstringlist.h>

// version number as used by boson 0.8.128 (aka 0x00,0x08,0x80 - development
// version. got never released)
#define BOSONMAP_VERSION_0_8_128 0x01

// version number as used by boson 0.9
#define BOSONMAP_VERSION_0_9 0x01

// compatibility for boson 0.8
#define BO_COMPAT_0_8_TEXTURE_COUNT 3 // we use 3 textured by default for old maps (grass, desert, water).

bool BosonFileConverter::loadXMLDoc(QDomDocument* doc, const QString& xml) const
{
 QString errorMsg;
 int line = 0;
 int column = 0;
 if (!doc->setContent(xml, &errorMsg, &line, &column)) {
	boError() << k_funcinfo << "Parse errror in line=" << line << ",column="
			<< column << " errorMsg=" << errorMsg << endl;
	return false;
 }
 return true;
}

bool BosonFileConverter::convertMapFile_From_0_8_To_0_9(const QByteArray& map, QByteArray* newMap, QByteArray* texMap)
{
 QDataStream readStream(map, IO_ReadOnly);

 // read in the boson 0.8 map first.
 // read map geo
 Q_INT32 mapWidth;
 Q_INT32 mapHeight;
 readStream >> mapWidth;
 readStream >> mapHeight;

 if (mapWidth < 10 || mapWidth > 500) {
	boError() << k_funcinfo << "broken map file - invalid width: " << mapWidth << endl;
	return false;
 }
 if (mapHeight < 10 || mapHeight > 500) {
	boError() << k_funcinfo << "broken map file - invalid height: " << mapHeight << endl;
	return false;
 }

 // read cells
 int* groundTypes = new int[mapWidth * mapHeight];
 unsigned char* versions = new unsigned char[mapWidth * mapHeight];
 for (int i = 0; i < mapWidth; i++) {
	for (int j = 0; j < mapHeight; j++) {
		Q_INT32 g;
		Q_INT8 version;
		readStream >> g;
		if (g < 0 || g > (7 + (4 * (12 + 4 * 16)))) { // Cell::isValidGround() from boson 0.8
			boError() << k_funcinfo << "not a valid ground" << endl;
			return false;
		}
		readStream >> version;
		if (version > 4) {
			boError() << k_funcinfo << "invalid version for ground" << endl;
			return false;
		}
		groundTypes[i + j * mapWidth] = g;
		versions[i + j * mapWidth] = version;
	}
 }

  // the versions don't influence textures or terrain type or anything else. they
 // are just "looks-better" tiles, so that we didn't have to use the same tile
 // for all grass cells. but we don't need it for converting.
 delete[] versions;
 versions = 0;

 QByteArray mapBuffer;
 MapToTexMap_From_0_8_To_0_9 converter((unsigned int)mapWidth, (unsigned int)mapHeight);
 bool ret = converter.convert(groundTypes, &mapBuffer, texMap);
 if (!ret) {
	boError() << k_funcinfo << "unable to convert file" << endl;
 }
 // delete temporary variables. also versions, which is NULL here, in case we
 // remove the delete above one day.
 delete[] groundTypes;
 delete[] versions;


 // MapToTexMap converts the map to 0.8.128 only. further converting is
 // necessary.
 return convertMapFile_From_0_8_128_To_0_9(mapBuffer, newMap);
}

bool BosonFileConverter::convertMapFile_From_0_8_128_To_0_9(const QByteArray& map, QByteArray* mapXML, int* mapWidth, int* mapHeight)
{
 boDebug() << k_funcinfo << endl;
 QDataStream stream(map, IO_ReadOnly);
 QString magicCookie;
 Q_UINT32 mapVersion;
 Q_INT32 width;
 Q_INT32 height;
 QString groundTheme;

 stream >> magicCookie;
 stream >> mapVersion;
 if (magicCookie != QString::fromLatin1("BosonMap")) {
	boError() << k_funcinfo << "invalid magic cookie for map file: " << magicCookie << endl;
	return false;
 }
 if (mapVersion != BOSONMAP_VERSION_0_8_128) {
	boError() << k_funcinfo << "version must be " << BOSONMAP_VERSION_0_8_128 <<  "- is: " << mapVersion << endl;
	return false;
 }
 stream >> width;
 stream >> height;
 if (width < 10 || height < 10 || width > 500 || height > 500) {
	boError() << k_funcinfo << "invalid map geometry: " << width << "x" << height << endl;
	return false;
 }
 stream >> groundTheme;

 QDomDocument doc(QString::fromLatin1("BosonMap"));
 QDomElement root = doc.createElement(QString::fromLatin1("BosonMap"));
 root.setAttribute(QString::fromLatin1("Version"), BOSONMAP_VERSION_0_9);
 root.setAttribute(QString::fromLatin1("GroundTheme"), groundTheme);
 doc.appendChild(root);
 QDomElement geometry = doc.createElement(QString::fromLatin1("Geometry"));
 geometry.setAttribute(QString::fromLatin1("Width"), width);
 geometry.setAttribute(QString::fromLatin1("Height"), height);
 root.appendChild(geometry);

 if (mapWidth && mapHeight) {
	*mapWidth = width;
	*mapHeight = height;
 }
 *mapXML = doc.toCString();
 return true;
}

bool BosonFileConverter::convertScenario_From_0_8_To_0_9(const QByteArray& scenarioXML, QByteArray* playersXML, QByteArray* canvasXML, QByteArray* kgameXML)
{
 if (!playersXML) {
	BO_NULL_ERROR(playersXML);
	return false;
 }
 if (!canvasXML) {
	BO_NULL_ERROR(canvasXML);
	return false;
 }
 if (!kgameXML) {
	BO_NULL_ERROR(kgameXML);
	return false;
 }
 boDebug() << k_funcinfo << endl;
 QDomDocument doc(QString::fromLatin1("BosonScenario"));
 if (!loadXMLDoc(&doc, scenarioXML)) {
	boError() << k_funcinfo << "could not load scenario.xml properly" << endl;
	return false;
 }
 QDomElement root = doc.documentElement();
 if (root.childNodes().count() < 2) {
	boError() << k_funcinfo << "no scenario found in file" << endl;
	return false;
 }
 QDomElement scenarioSettings = root.namedItem(QString::fromLatin1("ScenarioSettings")).toElement();
 if (scenarioSettings.isNull()) {
	boError() << k_funcinfo << "ScenarioSettings not found" << endl;
	return false;
 }

 unsigned int minPlayers = 0;
 int maxPlayers = 0;
 if ((int)minPlayers > maxPlayers && maxPlayers > 0) {
	boError() << k_funcinfo << "invalid min/max players: min=" << minPlayers << " max=" << maxPlayers << endl;
	return false;
 }
 if (minPlayers > 10 || maxPlayers > 10) {
	boError() << k_funcinfo << "more than 10 players is not allowed" << endl;
	return false;
 }
 bool ok = false;
 minPlayers = scenarioSettings.attribute("MinPlayers").toUInt(&ok);
 if (!ok) {
	boError() << k_funcinfo << "MinPlayers is not a valid number" << endl;
	return false;
 }
 maxPlayers = scenarioSettings.attribute("MaxPlayers").toInt(&ok);
 if (!ok) {
	boError() << k_funcinfo << "MaxPlayers is not a valid number" << endl;
	return false;
 }

 QDomElement players = root.namedItem(QString::fromLatin1("ScenarioPlayers")).toElement();
 if (players.isNull()) {
	boError() << k_funcinfo << "ScenarioPlayers not found" << endl;
	return false;
 }
 class UnitNode {
 public:
	UnitNode()
	{
		type = 0;
		x = 0;
		y = 0;
		isFacility = false;

		constructionCompleted = 0;
		constructionStep = 0;
	}
	~UnitNode()
	{
		delete constructionCompleted;
		delete constructionStep;
	}
	unsigned int type;
	unsigned int x;
	unsigned int y;
	bool isFacility;

	// use defaults, when NULL otherwise the value here.
	bool* constructionCompleted;
	unsigned int* constructionStep;
 };
 class PlayerNode {
 public:
	PlayerNode()
	{
		isValid = false;
		playerNumber = 0;
		minerals = 0;
		oil = 0;
		unitCount = 0;
		units = 0;
	}
	~PlayerNode()
	{
		delete[] units;
	}
	bool isValid;
	unsigned int playerNumber;
	unsigned long int minerals;
	unsigned long int oil;
	unsigned int unitCount;
	UnitNode* units;
 };
 QDomNodeList list = players.elementsByTagName(QString::fromLatin1("Player"));
 PlayerNode* scenarioPlayers = new PlayerNode [list.count()];
 unsigned int scenarioPlayersCount = list.count();
 for (unsigned int i = 0; i < list.count(); i++) {
	QDomElement e = list.item(i).toElement();
	if (e.isNull()) {
		continue;
	}
	PlayerNode* p = &scenarioPlayers[i];
	p->playerNumber = e.attribute("PlayerNumber").toUInt(&ok);
	if (!ok) {
		boWarning() << k_funcinfo << "invalid PlayerNumber attribute " << endl;
		continue;
	}
	QDomElement m = e.namedItem("Minerals").toElement();
	if (!m.isNull()) {
		p->minerals  = m.text().toULong(&ok);
		if (!ok) {
			boWarning() << k_funcinfo << "invalid Minerals" << endl;
			p->minerals = 0;
		}
	}
	QDomElement o = e.namedItem("Oil").toElement();
	if (!o.isNull()) {
		p->oil = o.text().toULong(&ok);
		if (!ok) {
			boWarning() << k_funcinfo << "invalid Oil" << endl;
			p->oil = 0;
		}
	}
	QDomNodeList units = e.elementsByTagName(QString::fromLatin1("Unit"));
	p->units = new UnitNode[units.count()];
	p->unitCount = units.count();
	for (unsigned int j = 0; j < units.count(); j++) {
		QDomElement u = units.item(j).toElement();
		UnitNode* unit = &p->units[j];
		if (!u.hasAttribute("Type")) {
			unit->type = u.attribute("UnitType").toUInt(&ok);
		} else {
			unit->type = u.attribute("Type").toUInt(&ok);
		}
		if (!ok) {
			boWarning() << k_funcinfo << "invalid type value"  << endl;
			unit->type = 0;
			continue;
		}
		unit->x = u.attribute("x").toUInt(&ok);
		if (!ok) {
			boWarning() << k_funcinfo << "invalid x value"  << endl;
			unit->type = 0; // unit will be ignored with a 0 type
			continue;
		}
		unit->y = u.attribute("y").toUInt(&ok);
		if (!ok) {
			boWarning() << k_funcinfo << "invalid y value"  << endl;
			unit->type = 0; // unit will be ignored with a 0 type
			continue;
		}

		// AB: health, armor, shields sightrange and work can also
		// appear in the scenario files - but only if non-defaults
		// should get used.
		// since the editor of boson 0.8 did not include any way to
		// change these values it is very unlikely that any non-defaults
		// will appear in the files. so we do not support them here.

		// the same is valid for the "MaxSpeed" setting of mobile units.
		// but we need to support construction settings of facilities.
		if (u.hasAttribute("ConstructionCompleted")) {
			int c = u.attribute("ConstructionCompleted").toInt(&ok);
			if (ok) {
				unit->constructionCompleted = new bool;
				*unit->constructionCompleted = (bool)c;
			}
		}
		if (u.hasAttribute("ConstructionStep")) {
			unsigned int c = u.attribute("ConstructionStep").toUInt(&ok);
			if (ok) {
				unit->constructionStep = new unsigned int;
				*unit->constructionStep = c;
			}
		}
	}
	p->isValid = true;
 }

 QDomDocument playersDoc(QString::fromLatin1("Players"));
 {
	QDomElement root = playersDoc.createElement(QString::fromLatin1("Players"));
	playersDoc.appendChild(root);
	for (unsigned int i = 0; i < scenarioPlayersCount; i++) {
		PlayerNode* p = &scenarioPlayers[i];
		if (!p->isValid) {
			continue;
		}
		QDomElement player = playersDoc.createElement("Player");

		// AB: warning: we use the _number_ for the ID !
		// this can be very evil!
		// (same in canvasDoc below)
		player.setAttribute("Id", p->playerNumber);

		root.appendChild(player);
		QDomElement dataHandler = playersDoc.createElement("DataHandler");
		player.appendChild(dataHandler);
		QDomElement minerals = playersDoc.createElement("KGameProperty");
		dataHandler.appendChild(minerals);
		minerals.setAttribute("Id", 258);
		minerals.appendChild(playersDoc.createTextNode(QString::number(p->minerals)));

		QDomElement oil = playersDoc.createElement("KGameProperty");
		dataHandler.appendChild(oil);
		oil.setAttribute("Id", 259);
		oil.appendChild(playersDoc.createTextNode(QString::number(p->oil)));
	}
 }

 QDomDocument canvasDoc(QString::fromLatin1("Canvas"));
 {
	QDomElement root = canvasDoc.createElement(QString::fromLatin1("Canvas"));
	canvasDoc.appendChild(root);
	for (unsigned int i = 0; i < scenarioPlayersCount; i++) {
		PlayerNode* p = &scenarioPlayers[i];
		if (!p->isValid) {
			continue;
		}
		QDomElement items = canvasDoc.createElement("Items");

		// AB: warning: see above in playersDoc. we use the
		// _playerNumber_ as ID! can be very evil!
		items.setAttribute("OwnerId", p->playerNumber);

		root.appendChild(items);
		for (unsigned int j = 0; j < p->unitCount; j++) {
			UnitNode* u = &p->units[j];
			QDomElement item = canvasDoc.createElement("Item");
			items.appendChild(item);
			item.setAttribute("Id", 0); // 0 == boson assigns an id
			item.setAttribute("DataHandlerId", -1); // -1 == boson assigns an id
			item.setAttribute("Rtti", 200 + u->type);
			item.setAttribute("Type", u->type);
			item.setAttribute("Group", u->type);
			item.setAttribute("GroupType", u->type);
			item.setAttribute("x", (float)u->x * 48);
			item.setAttribute("y", (float)u->y * 48);
			item.setAttribute("z", 0.0f);
			item.setAttribute("Rotation", 0.0f);
			QDomElement dataHandler = canvasDoc.createElement("DataHandler");
			item.appendChild(dataHandler);
			if (u->isFacility && u->constructionCompleted) {
				// now _this_ is difficult.
				// canvas.xml does not support such a task, as
				// there is no such variable in Unit. we have to
				// use the constructionStep for this, but we
				// don't know which value we have to assign to
				// it!
				// we simply use a very high value and hope that
				// it will get reduced to the maximal value on
				// loading.
				if (!u->constructionStep) {
					u->constructionStep = new unsigned int;
				}
				*u->constructionStep = 500000;
			}
			if (u->isFacility && u->constructionStep) {
				QDomElement step = canvasDoc.createElement("KGameProperty");
				step.setAttribute("Id", 372);
				step.appendChild(canvasDoc.createTextNode(QString::number(*u->constructionStep)));
			}
		}
		QDomElement canvasDataHandler = canvasDoc.createElement("DataHandler");
		root.appendChild(canvasDataHandler);
	}
 }

 delete[] scenarioPlayers;

 QDomDocument kgameDoc(QString::fromLatin1("Boson"));
 QDomElement kgameRoot = kgameDoc.createElement(QString::fromLatin1("Boson"));
 kgameDoc.appendChild(kgameRoot);
 // AB: SAVEGAME is not really true..
 kgameRoot.setAttribute(QString::fromLatin1("Version"), (unsigned int)BOSON_SAVEGAME_FORMAT_VERSION_0_9);

 *playersXML = playersDoc.toCString();
 *canvasXML = canvasDoc.toCString();
 *kgameXML = kgameDoc.toCString();

 return true;
}

bool BosonFileConverter::convertPlayField_From_0_9_To_0_9_1(QMap<QString, QByteArray>& files)
{
 QDomDocument kgameDoc(QString::fromLatin1("Boson"));
 if (!loadXMLDoc(&kgameDoc, files["kgame.xml"])) {
	boError() << k_funcinfo << "could not load kgame.xml" << endl;
	return false;
 }
 QDomElement kgameRoot = kgameDoc.documentElement();
 unsigned int version = kgameRoot.attribute("Version").toUInt();
 if (version != BOSON_SAVEGAME_FORMAT_VERSION_0_9) {
	boError() << k_funcinfo << "invalid version: " << version << endl;
	return false;
 }
 kgameRoot.setAttribute("Version", BOSON_SAVEGAME_FORMAT_VERSION_0_9_1);

 // we need to fix the Ids in the players.xml and canvas.xml files.
 // there are exactly two possibilities:
 // 1. the file is a playfield. then it is fixed already.
 // 2. the file is a savegame. then the file contains actual IDs, but we need
 // numbers.
 //
 // for code simplicity we ignore 1. -> it doesn't matter anway if we "fix
 // again".
 //
 // while we're on it we also remove the "OwnerId" attribute in canvas.xml. it
 // has been replaced by "Id"

 QDomDocument playersDoc(QString::fromLatin1("Players"));
 if (!loadXMLDoc(&playersDoc, files["players.xml"])) {
	boError() << k_funcinfo << "could not load players.xml" << endl;
	return false;
 }
 QDomElement playersRoot = playersDoc.documentElement();

 QDomNodeList playerList = playersRoot.elementsByTagName("Player");
 QMap<int, int> playerId2Number;
 for (unsigned int i = 0; i < playerList.count(); i++) {
	QDomElement p = playerList.item(i).toElement();
	bool ok = false;
	unsigned int id = p.attribute("Id").toUInt(&ok);
	if (!ok) {
		boError() << k_funcinfo << "invalid number for Id of Player " << i << endl;
		return false;
	}
	p.setAttribute("Id", i);
	playerId2Number.insert(id, i);
 }

 QDomDocument canvasDoc(QString::fromLatin1("Canvas"));
 if (!loadXMLDoc(&canvasDoc, files["canvas.xml"])) {
	boError() << k_funcinfo << "could not load canvas.xml" << endl;
	return false;
 }
 QDomElement canvasRoot = canvasDoc.documentElement();
 QDomNodeList itemsList = canvasRoot.elementsByTagName("Items");
 for (unsigned int i = 0; i < itemsList.count(); i++) {
	QDomElement e = itemsList.item(i).toElement();
	if (!e.hasAttribute("Id")) {
		// a .bpf file has both, Id and OwnerId (with OwnerId unused),
		// but .bsg files have OwnerId only
		e.setAttribute("Id", e.attribute("OwnerId"));
	}
	e.removeAttribute("OwnerId");
	bool ok = false;
	unsigned int id = e.attribute("Id").toUInt(&ok);
	if (!ok) {
		boError() << k_funcinfo << "invalid number for OwnerId of Items tag " << i << endl;
		return false;
	}
	if (!playerId2Number.contains(id)) {
		boError() << k_funcinfo << "unknown OwnerId " << id << " for Items tag " << i << endl;
		return false;
	}
	e.setAttribute("Id", playerId2Number[id]);
 }
 files.insert("kgame.xml", kgameDoc.toString().utf8());
 files.insert("players.xml", playersDoc.toString().utf8());
 files.insert("canvas.xml", canvasDoc.toString().utf8());
 return true;
}

bool BosonFileConverter::convertPlayField_From_0_9_1_To_0_10(QMap<QString, QByteArray>& files)
{
 QDomDocument kgameDoc(QString::fromLatin1("Boson"));
 if (!loadXMLDoc(&kgameDoc, files["kgame.xml"])) {
	boError() << k_funcinfo << "could not load kgame.xml" << endl;
	return false;
 }
 QDomElement kgameRoot = kgameDoc.documentElement();
 unsigned int version = kgameRoot.attribute("Version").toUInt();
 if (version != BOSON_SAVEGAME_FORMAT_VERSION_0_9_1) {
	boError() << k_funcinfo << "invalid version: " << version << endl;
	return false;
 }
 kgameRoot.setAttribute("Version", BOSON_SAVEGAME_FORMAT_VERSION_0_10);

 QDomDocument playersDoc(QString::fromLatin1("Players"));
 if (!loadXMLDoc(&playersDoc, files["players.xml"])) {
	boError() << k_funcinfo << "could not load players.xml" << endl;
	return false;
 }
 QDomElement playersRoot = playersDoc.documentElement();

 QDomNodeList playerList = playersRoot.elementsByTagName("Player");
 for (unsigned int i = 0; i < playerList.count(); i++) {
	QDomElement p = playerList.item(i).toElement();
	if (p.hasAttribute("IsNeutral")) {
		boError() << k_funcinfo << "IsNeutral attribute already present! cannot convert!" << endl;
		return false;
	}
 }
 QDomDocument canvasDoc(QString::fromLatin1("Canvas"));
 if (!loadXMLDoc(&canvasDoc, files["canvas.xml"])) {
	boError() << k_funcinfo << "could not load canvas.xml" << endl;
	return false;
 }
 QDomElement canvasRoot = canvasDoc.documentElement();
 QDomNodeList itemsList = canvasRoot.elementsByTagName("Items");
 // remove Item Type==7 (oiltower), which doesnt exist anymore
 for (unsigned int i = 0; i < itemsList.count(); i++) {
	QValueList<QDomElement> removeItems;
	QDomElement itemsTag = itemsList.item(i).toElement();
	QDomNodeList items = itemsTag.elementsByTagName(QString::fromLatin1("Item"));
	for (unsigned int j = 0; j < items.count(); j++) {
		QDomElement e = items.item(j).toElement();
		if (e.attribute("Type").compare("7") == 0) {
			removeItems.append(e);
		}
	}
	while (removeItems.count() != 0) {
		itemsTag.removeChild(removeItems.first());
		removeItems.pop_front();
	}
 }

 if (itemsList.count() != playerList.count()) {
	boError() << k_funcinfo << "itemsList.count() != playerList.count()" << endl;
	return false;
 }
 {
	QStringList ids;
	ids.append(QString::number(275)); // IdMoveDestX
	ids.append(QString::number(276)); // IdMoveDestY
	ids.append(QString::number(277)); // IdMoveRange
	ids.append(QString::number(279)); // IdMoveAttacking
	ids.append(QString::number(280)); // IdSearchPath
	ids.append(QString::number(281)); // IdSlowDownAtDestination
	ids.append(QString::number(323)); // IdMovingFailed
	ids.append(QString::number(324)); // IdPathRecalculated
	ids.append(QString::number(325)); // IdPathAge
	removePropertyIds_0_9_1(itemsList, ids);
 }

 QDomElement neutralPlayer = playersDoc.createElement(QString::fromLatin1("Player"));
 neutralPlayer.setAttribute("Id", playerList.count());
 neutralPlayer.setAttribute("IsNeutral", 1);
 {
	QDomElement dataHandler = playersDoc.createElement("DataHandler");
	neutralPlayer.appendChild(dataHandler);
	QDomElement minerals = playersDoc.createElement("KGameProperty");
	dataHandler.appendChild(minerals);
	minerals.setAttribute("Id", 258);
	minerals.appendChild(playersDoc.createTextNode(QString::number(0)));

	QDomElement oil = playersDoc.createElement("KGameProperty");
	dataHandler.appendChild(oil);
	oil.setAttribute("Id", 259);
	oil.appendChild(playersDoc.createTextNode(QString::number(0)));
 }
 playersRoot.appendChild(neutralPlayer);

 QDomElement neutralItems = canvasDoc.createElement("Items");
 neutralItems.setAttribute("Id", itemsList.count());
 canvasRoot.appendChild(neutralItems);

 files.insert("kgame.xml", kgameDoc.toString().utf8());
 files.insert("players.xml", playersDoc.toString().utf8());
 files.insert("canvas.xml", canvasDoc.toString().utf8());
 return true;
}

bool BosonFileConverter::convertPlayField_From_0_10_To_0_10_80(QMap<QString, QByteArray>& files)
{
 QDomDocument kgameDoc(QString::fromLatin1("Boson"));
 QDomDocument canvasDoc(QString::fromLatin1("Canvas"));
 QDomDocument playersDoc(QString::fromLatin1("Players"));
 if (!loadXMLDoc(&kgameDoc, files["kgame.xml"])) {
	boError() << k_funcinfo << "could not load kgame.xml" << endl;
	return false;
 }
 if (!loadXMLDoc(&canvasDoc, files["canvas.xml"])) {
	boError() << k_funcinfo << "could not load canvas.xml" << endl;
	return false;
 }
 if (!loadXMLDoc(&playersDoc, files["players.xml"])) {
	boError() << k_funcinfo << "could not load players.xml" << endl;
	return false;
 }
 QDomElement kgameRoot = kgameDoc.documentElement();
 QDomElement canvasRoot = canvasDoc.documentElement();
 QDomElement playersRoot = playersDoc.documentElement();

 // 0.10.80 is a development version that was never released.
 // this just exist because we have some maps in cvs with this format version
 // and need to convert them, too.
 kgameRoot.setAttribute("Version", BOSON_MAKE_SAVEGAME_FORMAT_VERSION(0x00, 0x02, 0x04));

 // all occurances of the player ID/index are now stored in an attribute named
 // "PlayerId"
 QDomNodeList playersList = playersRoot.elementsByTagName("Player");
 if (playersList.count() < 2) {
	boError() << k_funcinfo << "less than 2 Player tags found in file. This is an invalid file." << endl;
	return false;
 }
 for (unsigned int i = 0; i < playersList.count(); i++) {
	QDomElement e = playersList.item(i).toElement();
	if (e.isNull()) {
		boError() << k_funcinfo << "invalid Player tag" << endl;
		return false;
	}
	QString id = e.attribute("Id");
	e.removeAttribute("Id");
	e.setAttribute("PlayerId", id);
	if (i == playersList.count() - 1) {
		if (!e.hasAttribute("IsNeutral")) {
			boError() << k_funcinfo << "file format error: last player must be neutral player" << endl;
			return false;
		}
		bool ok = false;
		if (e.attribute("IsNeutral").toUInt(&ok) != 1 || !ok) {
			boError() << k_funcinfo << "IsNeutral attribute must be 1 if present" << endl;
			return false;
		}

	}
 }
 QDomNodeList itemsList = canvasRoot.elementsByTagName("Items");
 for (unsigned int i = 0; i < itemsList.count(); i++) {
	QDomElement e = itemsList.item(i).toElement();
	if (e.isNull()) {
		boError() << k_funcinfo << "invalid Items tag" << endl;
		return false;
	}
	QString id = e.attribute("Id");
	e.removeAttribute("Id");
	e.setAttribute("PlayerId", id);
 }


 // events/conditions:
 QDomElement eventManager = kgameDoc.createElement("EventManager");
 kgameRoot.appendChild(eventManager);
 QDomElement eventDataHandler = kgameDoc.createElement("DataHandler");
 eventManager.appendChild(eventDataHandler);
 QDomElement eventQueue = kgameDoc.createElement("EventQueue");
 eventManager.appendChild(eventQueue);

 QDomElement canvasEventListener = canvasDoc.createElement("EventListener");
 canvasRoot.appendChild(canvasEventListener);
 QDomElement canvasConditions = canvasDoc.createElement("Conditions");
 canvasEventListener.appendChild(canvasConditions);
{
	boWarning() << k_funcinfo << "Adding a dummy condition for testing" << endl;
	QDomElement cond = canvasDoc.createElement("Condition");
	canvasConditions.appendChild(cond);

	QDomElement events = canvasDoc.createElement("Events");
	QDomElement statuses = canvasDoc.createElement("StatusConditions");
	cond.appendChild(events);
	cond.appendChild(statuses);

	QDomElement matching = canvasDoc.createElement("EventMatching");
	QDomElement event = canvasDoc.createElement("Event");
	matching.setAttribute("IsLeft", QString::number(1));
	matching.setAttribute("IgnoreUnitId", true);
	matching.setAttribute("IgnorePlayerId", true);
	matching.setAttribute("IgnoreData1", true);
	matching.setAttribute("IgnoreData2", true);
	event.setAttribute("Name", "UnitWithTypeDestroyed");
	event.setAttribute("Id", 0);
	event.setAttribute("UnitId", 0);
	event.setAttribute("DelayedDelivery", 0);
	event.setAttribute("HasLocation", 0);
	event.setAttribute("Location.x", 0.0);
	event.setAttribute("Location.y", 0.0);
	event.setAttribute("Location.z", 0.0);
	event.setAttribute("Data1", "");
	event.setAttribute("Data2", "");
	matching.appendChild(event);
	events.appendChild(matching);

	QDomElement action = canvasDoc.createElement("Action");
	action.setAttribute("Type", "Event");
	cond.appendChild(action);
	QDomElement actionEvent = event.cloneNode().toElement();
	actionEvent.setAttribute("Name", "CustomStringEvent");
	actionEvent.setAttribute("Data1", "Foobar");
	action.appendChild(actionEvent);
}

 QDomElement effects = canvasDoc.createElement("Effects");
 canvasRoot.appendChild(effects);

 if (files["map/water.xml"].size() == 0) {
	boDebug() << k_funcinfo << "Adding dummy water.xml file" << endl;
	// Old file format - add dummy water.xml file
	if(!addDummyWaterXML_From_0_10_To_0_10_80(files["map/water.xml"]))
	{
		boError() << k_funcinfo << "Couldn't add dummy water.xml file" << endl;
		return false;
	}
 }

 files.insert("kgame.xml", kgameDoc.toString().utf8());
 files.insert("canvas.xml", canvasDoc.toString().utf8());
 files.insert("players.xml", playersDoc.toString().utf8());
 return true;
}

bool BosonFileConverter::addDummyWaterXML_From_0_10_To_0_10_80(QByteArray& waterXML)
{
 QDomDocument doc(QString::fromLatin1("Water"));
 QDomElement root = doc.createElement(QString::fromLatin1("Water"));
 doc.appendChild(root);
 waterXML = doc.toCString();
 return true;
}

bool BosonFileConverter::convertPlayField_From_0_10_80_To_0_11(QMap<QString, QByteArray>& files)
{
 QDomDocument kgameDoc(QString::fromLatin1("Boson"));
 QDomDocument canvasDoc(QString::fromLatin1("Canvas"));
 QDomDocument playersDoc(QString::fromLatin1("Players"));
 if (!loadXMLDoc(&kgameDoc, files["kgame.xml"])) {
	boError() << k_funcinfo << "could not load kgame.xml" << endl;
	return false;
 }
 if (!loadXMLDoc(&canvasDoc, files["canvas.xml"])) {
	boError() << k_funcinfo << "could not load canvas.xml" << endl;
	return false;
 }
 QDomElement kgameRoot = kgameDoc.documentElement();
 QDomElement canvasRoot = canvasDoc.documentElement();

#if BOSON_VERSION_MICRO < 0x80 || BOSON_VERSION_MINOR >= 0x11
#error replace the following by BOSON_SAVEGAME_FORMAT_VERSION_0_11
#endif
#define BO_VERSION BOSON_MAKE_SAVEGAME_FORMAT_VERSION(0x00, 0x02, 0x05)
 kgameRoot.setAttribute("Version", BO_VERSION);
#undef BO_VERSION

 QDomNodeList itemsList = canvasRoot.elementsByTagName("Items");
 for (unsigned int i = 0; i < itemsList.count(); i++) {
	QDomElement e = itemsList.item(i).toElement();
	if (e.isNull()) {
		boError() << k_funcinfo << "invalid Items tag" << endl;
		return false;
	}

	// BO_TILE_SIZE is no more
	QDomNodeList items = e.elementsByTagName("Item");
	for (unsigned int j = 0; j < items.count(); j++) {
		QDomElement item = items.item(j).toElement();
		if (item.isNull()) {
			boError() << k_funcinfo << "invalid Item tag" << endl;
			return false;
		}
		bool ok;
		float x = (float)item.attribute("x").toInt(&ok);
		if (!ok) {
			boError() << k_funcinfo << "x attribute is not a valid integer" << endl;
			return false;
		}
		float y = (float)item.attribute("y").toInt(&ok);
		if (!ok) {
				boError() << k_funcinfo << "y attribute is not a valid integer" << endl;
				return false;
		}
		x /= 48.0f;
		y /= 48.0f;

#warning this will be changed! -> we need to use fixed point values
		item.setAttribute("x", x);
		item.setAttribute("y", y);
	}
 }
 files.insert("kgame.xml", kgameDoc.toString().utf8());
 files.insert("canvas.xml", canvasDoc.toString().utf8());
 return true;
}

void BosonFileConverter::removePropertyIds_0_9_1(const QDomNodeList& itemsList, const QStringList& ids)
{
 for (unsigned int i = 0; i < itemsList.count(); i++) {
	QDomElement items = itemsList.item(i).toElement();
	QDomNodeList dataHandler = items.elementsByTagName(QString::fromLatin1("DataHandler"));
	for (unsigned int j = 0; j < dataHandler.count(); j++) {
		QDomElement handler = dataHandler.item(j).toElement();
		QDomNodeList properties = handler.elementsByTagName(QString::fromLatin1("KGameProperty"));
		for (int k = 0; k < (int)properties.count(); k++) {
			QDomElement e = properties.item(k).toElement();
			QString id = e.attribute("Id");
			if (ids.contains(id)) {
				handler.removeChild(e);
				k--;
			}
		}
	}
 }
}


bool MapToTexMap_From_0_8_To_0_9::convert(int* groundTypes, QByteArray* newMap, QByteArray* texMap)
{
 bool ret = true;
 QDataStream writeMapStream(*newMap, IO_WriteOnly);
 QDataStream writeTexMapStream(*texMap, IO_WriteOnly);

 writeMapStream << BOSONMAP_MAP_MAGIC_COOKIE;
 writeMapStream << (Q_UINT32)BOSONMAP_VERSION_0_9;
 writeTexMapStream << BOSONMAP_TEXMAP_MAGIC_COOKIE;
 writeTexMapStream << (Q_UINT32)BOSONMAP_VERSION_0_9;

 // now we can start to convert.
 writeMapStream << (Q_UINT32)mMapWidth;
 writeMapStream << (Q_UINT32)mMapHeight;
 writeMapStream << QString::fromLatin1("earth");
 writeTexMapStream << (Q_UINT32)BO_COMPAT_0_8_TEXTURE_COUNT;


 unsigned char* tex = new unsigned char[(mMapWidth + 1) * (mMapHeight + 1) * BO_COMPAT_0_8_TEXTURE_COUNT];
 convertToTexMap_From_0_8_To_0_9(groundTypes, tex);
 boDebug() << "writing texmap to stream" << endl;
 for (unsigned int i = 0; i < BO_COMPAT_0_8_TEXTURE_COUNT && ret; i++) {
	for (unsigned int x = 0; x < (unsigned int)mMapWidth + 1; x++) {
		for (unsigned int y = 0; y < (unsigned int)mMapHeight + 1; y++) {
			writeTexMapStream << (Q_UINT8)tex[texMapArrayPos(i, x, y)];
		}
	}
 }
 boDebug() << "wrote texmap to stream" << endl;
 delete[] tex;
 tex = 0;

 boDebug() << k_funcinfo << "done" << endl;
 return ret;
}


void MapToTexMap_From_0_8_To_0_9::addGroundType(int groundType, int* grass, int* desert, int* water)
{
 if (!isPlain(groundType)) {
	boError() << k_funcinfo << "not a plain groundtype: " << groundType << endl;
	return;
 }
 switch (groundType) {
	case 2: // GroundWater
		(*water) += 1;
		break;
	case 3: // GroundGrass
		(*grass) += 1;
		break;
	case 4: // GroundDesert
		(*desert) += 1;
		break;
	default:
		boWarning() << k_funcinfo << "unrecognized plain tile: " << groundType << endl;
		return;
 }
}

int MapToTexMap_From_0_8_To_0_9::from(int groundType) const
{
 if (!isValidGroundType(groundType) || isPlain(groundType)) {
	return 0;
 }
 int transRef = getTransRef(groundType);
 int from = 0;
 switch (transRef) {
	case 0: // TransGrassWater
		from = 3; // GroundGrass
		break;
	case 1: // TransGrassDesert
		from = 3; // GroundGrass
		break;
	case 2: // TransDesertWater
		from = 4; // GroundDesert
		break;
	case 3: // TransDeepWater
		boError() << k_funcinfo << "TransDeepWater should have been filtered out already! - groundType: " << groundType << endl;
		return 0;
		break;
	default:
		boError() << k_funcinfo << "invalid transref " << transRef << " - groundType: " << groundType << endl;
		return 0;
 }
 return from;
}

int MapToTexMap_From_0_8_To_0_9::to(int groundType) const
{
 if (!isValidGroundType(groundType) || isPlain(groundType)) {
	return 0;
 }
 int transRef = getTransRef(groundType);
 int to = 0;
 switch (transRef) {
	case 0: // TransGrassWater
		to = 2; // GroundWater
		break;
	case 1: // TransGrassDesert
		to = 4; // GroundDesert
		break;
	case 2: // TransDesertWater
		to = 2; // GroundWater
		break;
	case 3: // TransDeepWater
		boError() << k_funcinfo << "TransDeepWater should have been filtered out already! - groundType: " << groundType << endl;
		return 0;
		break;
	default:
		boError() << k_funcinfo << "invalid transref " << transRef << " - groundType: " << groundType << endl;
		return 0;
 }
 return to;
}

void MapToTexMap_From_0_8_To_0_9::addCornerTopLeft(int groundType, int* grass, int* desert, int* water)
{
 if (!isValidGroundType(groundType)) {
	boError() << k_funcinfo << "not a valid groundType: " << groundType << endl;
	return;
 }
 if (groundType >= 0 && groundType < 7) { // Cell::isPlain()
	addGroundType(groundType, grass, desert, water);
	return;
 }
 int from = this->from(groundType);
 int to = this->to(groundType);

 // the Cell::Transition names refer to the "to" part of the transition. e.g.
 // for desert_water and "TransUpLeft", we have desert in the lower right corner
 // of the cell only, the "UpLeft" part is water.
 int trans = getTransTile(groundType);
 switch (trans) {
	case 0: // TransUpLeft
		addGroundType(to, grass, desert, water);
		break;
	case 1: // TransUpRight
		addGroundType(to, grass, desert, water);
		break;
	case 2: // TransDownLeft
		addGroundType(to, grass, desert, water);
		break;
	case 3: // TransDownRight
		addGroundType(from, grass, desert, water);
		break;
	case 4: // TransUp
		addGroundType(to, grass, desert, water);
		break;
	case 5: // TransDown
		addGroundType(from, grass, desert, water);
		break;
	case 6: // TransLeft
		addGroundType(to, grass, desert, water);
		break;
	case 7: // TransRight
		addGroundType(from, grass, desert, water);
		break;
	case 8: // TransUpLeftInverted
		addGroundType(from, grass, desert, water);
		break;
	case 9: // TransUpRightInverted
		addGroundType(from, grass, desert, water);
		break;
	case 10: // TransDownLeftInverted
		addGroundType(from, grass, desert, water);
		break;
	case 11: // TransDownRightInverted
		addGroundType(to, grass, desert, water);
		break;
	default:
		// this is probably a bigtile. big tiles are not supported here
		// - they should have been split into plain tiles and small
		// transitions already!
		boError() << k_funcinfo << "invalid transition - groundType: " << groundType << endl;
		return;
 }
}

void MapToTexMap_From_0_8_To_0_9::addCornerTopRight(int groundType, int* grass, int* desert, int* water)
{
 if (!isValidGroundType(groundType)) {
	boError() << k_funcinfo << "not a valid groundType: " << groundType << endl;
	return;
 }
 if (groundType >= 0 && groundType < 7) { // Cell::isPlain()
	addGroundType(groundType, grass, desert, water);
	return;
 }
 int from = this->from(groundType);
 int to = this->to(groundType);

 // the Cell::Transition names refer to the "to" part of the transition. e.g.
 // for desert_water and "TransUpLeft", we have desert in the lower right corner
 // of the cell only, the "UpLeft" part is water.
 int trans = getTransTile(groundType);
 switch (trans) {
	case 0: // TransUpLeft
		addGroundType(to, grass, desert, water);
		break;
	case 1: // TransUpRight
		addGroundType(to, grass, desert, water);
		break;
	case 2: // TransDownLeft
		addGroundType(from, grass, desert, water);
		break;
	case 3: // TransDownRight
		addGroundType(to, grass, desert, water);
		break;
	case 4: // TransUp
		addGroundType(to, grass, desert, water);
		break;
	case 5: // TransDown
		addGroundType(from, grass, desert, water);
		break;
	case 6: // TransLeft
		addGroundType(from, grass, desert, water);
		break;
	case 7: // TransRight
		addGroundType(to, grass, desert, water);
		break;
	case 8: // TransUpLeftInverted
		addGroundType(from, grass, desert, water);
		break;
	case 9: // TransUpRightInverted
		addGroundType(from, grass, desert, water);
		break;
	case 10: // TransDownLeftInverted
		addGroundType(to, grass, desert, water);
		break;
	case 11: // TransDownRightInverted
		addGroundType(from, grass, desert, water);
		break;
	default:
		// this is probably a bigtile. big tiles are not supported here
		// - they should have been split into plain tiles and small
		// transitions already!
		boError() << k_funcinfo << "invalid transition - groundType: " << groundType << endl;
		return;
 }
}

void MapToTexMap_From_0_8_To_0_9::addCornerBottomLeft(int groundType, int* grass, int* desert, int* water)
{
 if (!isValidGroundType(groundType)) {
	boError() << k_funcinfo << "not a valid groundType: " << groundType << endl;
	return;
 }
 if (groundType >= 0 && groundType < 7) { // Cell::isPlain()
	addGroundType(groundType, grass, desert, water);
	return;
 }
 int from = this->from(groundType);
 int to = this->to(groundType);

 // the Cell::Transition names refer to the "to" part of the transition. e.g.
 // for desert_water and "TransUpLeft", we have desert in the lower right corner
 // of the cell only, the "UpLeft" part is water.
 int trans = getTransTile(groundType);
 switch (trans) {
	case 0: // TransUpLeft
		addGroundType(to, grass, desert, water);
		break;
	case 1: // TransUpRight
		addGroundType(from, grass, desert, water);
		break;
	case 2: // TransDownLeft
		addGroundType(to, grass, desert, water);
		break;
	case 3: // TransDownRight
		addGroundType(to, grass, desert, water);
		break;
	case 4: // TransUp
		addGroundType(from, grass, desert, water);
		break;
	case 5: // TransDown
		addGroundType(to, grass, desert, water);
		break;
	case 6: // TransLeft
		addGroundType(to, grass, desert, water);
		break;
	case 7: // TransRight
		addGroundType(from, grass, desert, water);
		break;
	case 8: // TransUpLeftInverted
		addGroundType(from, grass, desert, water);
		break;
	case 9: // TransUpRightInverted
		addGroundType(to, grass, desert, water);
		break;
	case 10: // TransDownLeftInverted
		addGroundType(from, grass, desert, water);
		break;
	case 11: // TransDownRightInverted
		addGroundType(from, grass, desert, water);
		break;
	default:
		// this is probably a bigtile. big tiles are not supported here
		// - they should have been split into plain tiles and small
		// transitions already!
		boError() << k_funcinfo << "invalid transition - groundType: " << groundType << endl;
		return;
 }
}

void MapToTexMap_From_0_8_To_0_9::addCornerBottomRight(int groundType, int* grass, int* desert, int* water)
{
 if (!isValidGroundType(groundType)) {
	boError() << k_funcinfo << "not a valid groundType: " << groundType << endl;
	return;
 }
 if (groundType >= 0 && groundType < 7) { // Cell::isPlain()
	addGroundType(groundType, grass, desert, water);
	return;
 }
 int from = this->from(groundType);
 int to = this->to(groundType);

 // the Cell::Transition names refer to the "to" part of the transition. e.g.
 // for desert_water and "TransUpLeft", we have desert in the lower right corner
 // of the cell only, the "UpLeft" part is water.
 int trans = getTransTile(groundType);
 switch (trans) {
	case 0: // TransUpLeft
		addGroundType(from, grass, desert, water);
		break;
	case 1: // TransUpRight
		addGroundType(to, grass, desert, water);
		break;
	case 2: // TransDownLeft
		addGroundType(to, grass, desert, water);
		break;
	case 3: // TransDownRight
		addGroundType(to, grass, desert, water);
		break;
	case 4: // TransUp
		addGroundType(from, grass, desert, water);
		break;
	case 5: // TransDown
		addGroundType(to, grass, desert, water);
		break;
	case 6: // TransLeft
		addGroundType(from, grass, desert, water);
		break;
	case 7: // TransRight
		addGroundType(to, grass, desert, water);
		break;
	case 8: // TransUpLeftInverted
		addGroundType(to, grass, desert, water);
		break;
	case 9: // TransUpRightInverted
		addGroundType(from, grass, desert, water);
		break;
	case 10: // TransDownLeftInverted
		addGroundType(from, grass, desert, water);
		break;
	case 11: // TransDownRightInverted
		addGroundType(from, grass, desert, water);
		break;
	default:
		// this is probably a bigtile. big tiles are not supported here
		// - they should have been split into plain tiles and small
		// transitions already!
		boError() << k_funcinfo << "invalid transition - groundType: " << groundType << endl;
		return;
 }
}

bool MapToTexMap_From_0_8_To_0_9::convertToTexMap_From_0_8_To_0_9(int* groundTypes, unsigned char* texMap)
{
 boDebug() << k_funcinfo << endl;
 if (mMapWidth == 0 || mMapHeight == 0) {
	return false;
 }
 if (!groundTypes) {
	return false;
 }
 if (!texMap) {
	return false;
 }

 // initialize all values to 0 first
 for (unsigned int i = 0; i < BO_COMPAT_0_8_TEXTURE_COUNT; i++) {
	for (unsigned int x = 0; x < mMapWidth + 1; x++) {
		for (unsigned int y = 0; y < mMapHeight + 1; y++) {
			setTexMapAlpha(i, x, y, 0, texMap);
		}
	}
 }

 // fixing groundTypes a bit...
 // we have to simplify a lot before i am able to write conversion code...
 fixGroundTypes(groundTypes);

 const unsigned int texGrass = 0;
 const unsigned int texDesert = 1;
 const unsigned int texWater = 2;

 for (unsigned int x = 0; x < mMapWidth + 1; x++) {
	for (unsigned int y = 0; y < mMapHeight + 1; y++) {
		// if a cell doesn't exist for a corner it'll have an invalid
		// groundType
		int topLeft = -1;
		int topRight = -1;
		int bottomLeft = -1;
		int bottomRight = -1;
		if (x > 0 && y > 0) {
			topLeft = groundTypes[(x - 1) + (y - 1) * mMapWidth];
		}
		if (x < mMapWidth && y < mMapHeight) {
			bottomRight = groundTypes[x + y * mMapWidth];
		}
		if (x < mMapWidth && y > 0) {
			topRight = groundTypes[x + (y - 1) * mMapWidth];
		}
		if (x > 0 && y < mMapWidth) {
			bottomLeft = groundTypes[(x - 1) + y * mMapWidth];
		}

		int grass = 0;
		int desert = 0;
		int water = 0;

		// collect informations about the (x,y) in the texMap.
		// note: the cell named "topLeft" for the (x,y) corner,
		// considers the corner as "bottomRight"!

		// the top-left cell of this corner
		if (isValidGroundType(topLeft)) {
			addCornerBottomRight(topLeft, &grass, &desert, &water);
		}

		// the top-right cell of this corner
		if (isValidGroundType(topRight)) {
			addCornerBottomLeft(topRight, &grass, &desert, &water);
		}

		// the bottom-left cell of this corner
		if (isValidGroundType(bottomLeft)) {
			addCornerTopRight(bottomLeft, &grass, &desert, &water);
		}

		// the bottom-right cell of this corner
		if (isValidGroundType(bottomRight)) {
			addCornerTopLeft(bottomRight, &grass, &desert, &water);
		}

		// now we know how many grass/desert/water cells are adjacent to
		// this cell. note, that in a corner the groundType of a cell is
		// unique - even for transitions!
		// with this information we can compute alpha values for the
		// texmap.
		int cells = grass + desert + water; // will always be for, except at the borders!
		if (cells > 4 || cells <= 0) {
			boError() << k_funcinfo << "more than 4 or less than 1 adjacent cells is impossible for a corner!" << endl;
			grass = 0;
			desert = 0;
			water = 4;
			cells = 4;
		}
		unsigned char grassAlpha = 0;
		unsigned char desertAlpha = 0;
		unsigned char waterAlpha = 0;

		grassAlpha = (unsigned char)(255 * grass / cells);
		desertAlpha = (unsigned char)(255 * desert / cells);
		waterAlpha = (unsigned char)(255 * water / cells);

		setTexMapAlpha(texGrass, x, y, grassAlpha, texMap);
		setTexMapAlpha(texDesert, x, y, desertAlpha, texMap);
		setTexMapAlpha(texWater, x, y, waterAlpha, texMap);
	}
 }

 return true;
}

void MapToTexMap_From_0_8_To_0_9::fixGroundTypes(int* groundTypes)
{
 const int tilesPerTransition = 12 + 4 * 16;
 const int groundTilesNumber = 7 + 4 * tilesPerTransition;
 for (unsigned int x = 0; x < mMapWidth; x++) {
	for (unsigned int y = 0; y < mMapHeight; y++) {
		int g = groundTypes[x + y * mMapWidth];
		if (g < 7) { // Cell::isPlain()
			if (g == 0) { // GroundUnknown.
				// we cannot use this. converting to
				// GroundWater.
				// most units can't cross water, so this is the
				// closest type.
				g = 2; // GroundWater
			} else if (g == 1) { // GroundDeepWater
				// we make no difference between deep water and
				// water
				g = 2; // GroundWater
			} else if (g == 5 || g == 6) { // GroundGrass[Mineral|Oil]
				// mineral / oil won't be in the groundType.
				// we lose this information at this point, but
				// that won't hurt too much as mining is mostly
				// experimental anyway.
				g = 3; // GroundGrass
			}
		} else if (g >= 7 && g < groundTilesNumber) { // Cell::isTrans()
			int transType = (g - 7) / tilesPerTransition; // Cell::getTransRef()
			if (transType < 0 || transType >= 4) {
				boError() << k_funcinfo << "invalid transType " << transType << " for groundType " << g << " at " << x << "," << y << endl;
				g = 2; // same as for GroundUnknown.
			} else if (transType == 3) { // TransDeepWater
				// deep water is equal to water for us.
				// so we have an easy job here: the transition
				// becomes a plain (water) tile.
				g = 2;
			}

			int transTile = (g - 7) % tilesPerTransition; // Cell::getTransTile()
			if (transTile >= 12) { // Cell::isBigTrans()
				// a big transition consists of 4 tiles. for our
				// conversion algorithm we'll be able to convert
				// these into small transitions and plain tiles.
				int from = this->from(g);
				int to = this->to(g);
				int trans = transTile - 12; // bigtile number (12==smallTilesPerTransition)
				trans /= 4; // which one
				trans += 12;

				// want docs for this?
				// look into ground/earth/*_*/ and paint some
				// pictures for these transitions. it's close to
				// impossible to explain this, or keep these
				// information in text.
				// the big tiles are the earth/*_*/*_*_nn-*.png
				// with nn >= 13. internally we work with nn - 1
				//
				// note that grass_desert and grass_water use
				// the same format (i.e. their "to" part are
				// treated equally), whereas for desert_water
				// the "to" and "from" part is flipped.
				if (transType == 2) { // TransDesertWater
					// flip from/to for desert_water
					int tmp = from;
					from = to;
					to = tmp;
				}
				int big[4]; // topleft,topright,bottomleft,bottomright
				big[0] = big[1] = big[2] = big[3] = to;
				switch (trans) {
					case 12:
						big[3] = from;
						break;
					case 13:
						big[2] = from;
						break;
					case 14:
						big[1] = from;
						break;
					case 15:
						big[0] = from;
						break;
					case 16:
						big[0] = big[1] = big[2] = from;
						break;
					case 17:
						big[0] = big[1] = big[3] = from;
						break;
					case 18:
						big[0] = big[2] = big[3] = from;
						break;
					case 19:
						big[1] = big[2] = big[3] = from;
						break;
					case 20:
					case 21:
						big[2] = big[3] = from;
						break;
					case 22:
					case 23:
						big[0] = big[1] = from;
						break;
					case 24:
					case 26: // 24 and 26, not 25! not a typo!
						big[0] = big[2] = from;
						break;
					case 25:
					case 27:
						big[1] = big[3] = from;
						break;
					default:
						boError() << k_funcinfo << "invalid trans number for bigtile: " << trans << " groundType: " << g << endl;
						g = 2;
						continue;
				}

				int pos = transTile - ((trans - 12) * 4 + 12);
				if (pos < 0 || pos >= 4) {
					boError() << k_funcinfo << "invalid position: " << pos << " groundType = " << g << endl;
					g = 2;
					continue;
				}
				g = big[pos];
			}

		} else {
			boWarning() << k_funcinfo << "invalid groundtype at " << x << " " << y << endl;
			g = 2; // AB: same as for GroundUnknown above.
		}

		groundTypes[x + y * mMapWidth] = g;
	}
 }
}

void MapToTexMap_From_0_8_To_0_9::setTexMapAlpha(unsigned int texture, unsigned int x, unsigned int y, unsigned char alpha, unsigned char* texMap)
{
 unsigned char* tex = &texMap[texMapArrayPos(texture, x, y)];
 *tex = alpha;
}

