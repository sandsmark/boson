
#include "bosonscenario.h"

#include "unit.h"
#include "player.h"
#include "boson.h"

#include <kdebug.h>
#include <kstandarddirs.h>
#include <kfilterdev.h>
#include <ksimpleconfig.h>

#include <qfile.h>
#include <qdatastream.h>
#include <qdom.h>

#include "defines.h"

#define TAG_FIELD "bosonscenario_magic_0_6"
#define TAG_UNIT (0xba)

struct ScenarioUnit
{
	int unitType;
	int x;
	int y;
};


class BosonScenarioPrivate
{
public:
	BosonScenarioPrivate()
	{
		mUnits = 0;

		mMaxPlayers = 0;
		mMinPlayers = 0;
	}
	
	QString mFileName;

	int mMaxPlayers; // -1 == unlimited
	unsigned int mMinPlayers;

	QValueList<ScenarioUnit>* mUnits;
};

BosonScenario::BosonScenario()
{
 init();
}

BosonScenario::BosonScenario(const QString& fileName)
{
 init();
 loadScenario(fileName);
}

BosonScenario::~BosonScenario()
{
 if (d->mUnits) {
	d->mUnits->clear();
	delete d->mUnits;
 }
 delete d;
}

void BosonScenario::init()
{
 d = new BosonScenarioPrivate;
}

QString BosonScenario::defaultScenario()
{
 return locate("data", "boson/scenario/basic.bsc");
}

bool BosonScenario::loadScenario(const QString& fileName)
{
 kdDebug() << "BosonScenario::loadScenario " << fileName << endl;

 // open stream
 QIODevice* dev = KFilterDev::deviceForFile(fileName);
 if (!dev) {
	kdError() << "BosonScenario: NULL device for " << fileName << endl;
	return false;
 }
 if (!dev->open(IO_ReadOnly)) {
	kdError() << "BosonScenario: Could not open " << fileName << endl;
	delete dev;
	return false;
 }
 QByteArray buffer = dev->readAll();
 dev->close();
 delete dev;
 d->mFileName = fileName;

 QDataStream stream(buffer, IO_ReadOnly);
 if (verifyScenario(stream)) { // binary file
	if (!loadScenarioSettings(stream)) {
		kdError() << "Could not load scenario settings" << endl;
		return false;
	}
	if (!loadPlayers(stream)) {
		kdError() << "Could not load scenario players" << endl;
		return false;
	}
	kdDebug() << "loading done - save now..." << endl;
	return true;
 }
 // XML file
 QDomDocument doc("BosonScenario");
 QString errorMsg;
 int lineNo;
 int columnNo;
 if (!doc.setContent(buffer, false, &errorMsg, &lineNo, &columnNo)) {
	kdError() << "Parse error in line " << lineNo << ",column " << columnNo
			<< " error message: " << errorMsg << endl;
	return false;
 }
 QDomNodeList list;
 QDomElement root = doc.documentElement();
 list = root.elementsByTagName("ScenarioSettings");
 if (list.count() != 1) {
	kdError() << "Cannot have ScenarioSettings " << list.count() 
			<< " times"<< endl;
	return false;
 }
 QDomElement settings = list.item(0).toElement();
 if (settings.isNull()) {
	kdError() << "settings is not a QDomElement" << endl;
	return false;
 }
 if (!loadScenarioSettings(settings)) {
	kdError() << "Could not load scenario settings" << endl;
	return false;
 }

 list = root.elementsByTagName("ScenarioPlayers");
 if (list.count() != 1) {
	kdError() << "Cannot have ScenarioPlayers " << list.count() 
			<< " times"<< endl;
	return false;
 }
 QDomElement players = list.item(0).toElement();
 if (players.isNull()) {
	kdError() << "players is not a QDomElement" << endl;
	return false;
 }
 if (!loadPlayers(players)) {
	kdError() << "Could not load scenario players" << endl;
	return false;
 }

 return false;
}

bool BosonScenario::saveScenario(const QString& fileName, bool binary)
{
 if (binary) {
	QIODevice* dev = KFilterDev::deviceForFile(fileName);
	if (!dev) {
		kdError() << "BosonScenario: NULL device for " << fileName << endl;
		return false;
	}
	if (!dev->open(IO_WriteOnly)) {
		kdError() << "BosonScenario: Could not open " << fileName << endl;
		delete dev;
		return false;
	}
	QDataStream stream(dev);
	bool ret = saveScenario(stream);
	dev->close();
	delete dev;
	return ret;
 }

 // now save the file
 QIODevice* dev = KFilterDev::deviceForFile(fileName, "application/x-gzip");
 if (!dev) {
	kdError() << "BosonScenario: NULL device for " << fileName << endl;
	return false;
 }
 if (!dev->open(IO_WriteOnly)) {
	kdError() << "BosonScenario: Could not open " << fileName << endl;
	delete dev;
	return false;
 }
 saveXMLScenario(dev);
 dev->close();
 delete dev;
 return true;
}

bool BosonScenario::saveScenario(QDataStream& stream)
{
 if (!saveValidityHeader(stream)) {
	kdError() << "Could not write header" << endl;
	return false;
 }
 if (!saveScenarioSettings(stream)) {
	kdError() << "Could not write settings" << endl;
	return false;
 }
 if (!savePlayers(stream)) {
	kdError() << "Could not write players" << endl;
	return false;
 }
 return true;
}

bool BosonScenario::saveXMLScenario(QIODevice* dev)
{
 if (!dev) {
	kdError() << "NULL device" << endl;
	return false;
 }
 QDomDocument doc("BosonScenario");
 QDomElement root = doc.createElement("BosonScenario");
 doc.appendChild(root);

 QDomElement settings = doc.createElement("ScenarioSettings");
 root.appendChild(settings);
 
 if (!saveScenarioSettings(settings)) {
	kdError() << "Could not save scenario settings to XML" << endl;
	return false;
 }

 QDomElement players = doc.createElement("ScenarioPlayers");
 root.appendChild(players);
 if (!savePlayers(players)) {
	kdError() << "Could not save players to XML" << endl;
	return false;
 }

 QString xml = doc.toString();
 dev->writeBlock(xml.data(), xml.length()); // FIXME: or xml.latin1() // or something else?
 return true;
}

bool BosonScenario::saveScenarioSettings(QDomElement& node)
{
 QDomDocument doc = node.ownerDocument();
 node.setAttribute("MinPlayers", minPlayers());
 node.setAttribute("MaxPlayers", maxPlayers());
 return true;
}

bool BosonScenario::loadScenarioSettings(QDomElement& node)
{
 if (!node.hasAttribute("MinPlayers")) {
	kdError() << "Missing MinPlayers" << endl;
	return false;
 }
 if (!node.hasAttribute("MaxPlayers")) {
	kdError() << "Missing MaxPlayers" << endl;
	return false;
 }

 QByteArray buffer;
 QDataStream stream(buffer, IO_WriteOnly);

 // TODO: check if attrtibute actually was int
 Q_UINT32 min = node.attribute("MinPlayers").toUInt();
 Q_INT32 max = node.attribute("MaxPlayers").toInt();
 
 stream << min;
 stream << max;
 
 QDataStream readStream(buffer, IO_ReadOnly);
 loadScenarioSettings(readStream);
 return true;
}

bool BosonScenario::loadScenarioSettings(QDataStream& stream)
{
 Q_UINT32 minPlayers;
 Q_INT32 maxPlayers;
 stream >> minPlayers;
 stream >> maxPlayers;
 
 if (minPlayers < 1) {
	kdError() << "BosonScenario::loadScenarioSettings(): broken scenario file!" << endl;
	kdError() << "minPlayers < 1" << endl;
	return false;
 }
 if (maxPlayers > BOSON_MAX_PLAYERS) {
	kdError() << "BosonScenario::loadScenarioSettings(): broken scenario file!" << endl;
	kdError() << "maxPlayers > " << BOSON_MAX_PLAYERS << endl;
	return false;
 }
 if ((int)minPlayers > maxPlayers) {
	kdError() << "BosonScenario::loadScenarioSettings(): broken scenario file!" << endl;
	kdError() << "minPlayers > maxPlayers" << endl;
	return false;
 }
 d->mMinPlayers = minPlayers;
 d->mMaxPlayers = maxPlayers;
 d->mUnits = new QValueList<ScenarioUnit>[d->mMaxPlayers];
 return true;
}


bool BosonScenario::saveScenarioSettings(QDataStream& stream)
{
 stream << (Q_UINT32)minPlayers();
 stream << (Q_UINT32)maxPlayers();
 return true;
}

bool BosonScenario::loadPlayers(QDataStream& stream)
{
 if (!d->mUnits) {
	kdError() << "NULL units" << endl;
	return false;
 }
 for (int i = 0; i < d->mMaxPlayers; i++) {
	if (!loadPlayer(stream, i)) {
		kdError() << "Could not load player " << i << endl;
		return false;
	}
 }
 return true;
}

bool BosonScenario::loadPlayers(QDomElement& node)
{
 if (!d->mUnits) {
	kdError() << "NULL units" << endl;
	return false;
 }
 QDomNodeList list = node.elementsByTagName("Player");
 if (list.count() < minPlayers()) {
	kdError() << "Not enough players in file - expected " << minPlayers() 
			<< ", found " << list.count() << endl;
	return false;
 }
 if ((int)list.count() > maxPlayers()) {
	kdWarning() << "Too many players in file. Read only " << maxPlayers() 
			<< endl;
 }

 for (int unsigned i = 0; i < list.count(); i++) {
	QDomElement player = list.item(i).toElement();
	if (player.isNull()) {
		kdError() << i << " is not a QDomElement" << endl;
		return false;
	}
	if (!loadPlayer(player)) {
		kdError() << "Could not load player " << i << endl;
		return false;
	}
 }
 return true;
}

bool BosonScenario::loadPlayer(QDomElement& node)
{
 if (!node.hasAttribute("PlayerNumber")) {
	kdError() << "Missing PlayerNumber" << endl;
	return false;
 }
 QByteArray buffer;
 QDataStream stream(buffer, IO_WriteOnly);
 QDomNodeList list = node.elementsByTagName("Unit");
 stream << (Q_UINT32)list.count();
 for (unsigned int i = 0; i < list.count(); i++) {
	QDomElement unit = list.item(i).toElement();
	if (unit.isNull()) {
		kdError() << "Unit is not a QDomElement" << endl;
		return false;
	}
	if (!unit.hasAttribute("UnitType")) {
		kdError() << "Missing UnitType" << endl;
		return false;
	}
	if (!unit.hasAttribute("x")) {
		kdError() << "Missing x" << endl;
		return false;
	}
	if (!unit.hasAttribute("y")) {
		kdError() << "Missing y" << endl;
		return false;
	}
	// TODO: check if these were actually numbers
	Q_INT32 unitType;
	Q_INT32 x;
	Q_INT32 y;

	unitType = unit.attribute("UnitType").toInt();
	x = unit.attribute("x").toInt();
	y = unit.attribute("y").toInt();
	
	stream << (Q_INT32)TAG_UNIT;
	stream << unitType;
	stream << x;
	stream << y;
 }

 // TODO: check if this was a number
 unsigned int playerNumber = node.attribute("PlayerNumber").toUInt();

 QDataStream readStream(buffer, IO_ReadOnly);
 return loadPlayer(readStream, playerNumber);
}

bool BosonScenario::savePlayers(QDomElement& node)
{
 QDomDocument doc = node.ownerDocument();

 for (int i = 0; i < maxPlayers(); i++) {
	QDomElement player = doc.createElement("Player");
	node.appendChild(player);
	if (!savePlayer(player, i)) {
		kdError() << "Error saving units of player " << i << endl;
		return false;
	}
 }
 return true;
}

bool BosonScenario::savePlayers(QDataStream& stream)
{
 for (int i = 0; i < maxPlayers(); i++) {
	if (!savePlayer(stream, i)) {
		kdError() << "Error saving units of player " << i << endl;
		return false;
	}
 }
 return true;
}

bool BosonScenario::savePlayer(QDomElement& node, unsigned int playerNumber)
{
 QDomDocument doc = node.ownerDocument();
 node.setAttribute("PlayerNumber", playerNumber);

 if (!d->mUnits) {
	kdError() << "NULL units" << endl;
	return false;
 }
 if (d->mUnits->count() <= playerNumber) {
	kdError() << "Unknown player " << playerNumber << endl;
	return false;
 }
 QValueList<ScenarioUnit>::Iterator it;
 for (it = d->mUnits[playerNumber].begin(); it != d->mUnits[playerNumber].end(); ++it) {
	ScenarioUnit s = (*it);
	QDomElement unit = doc.createElement("Unit");
	node.appendChild(unit);
	unit.setAttribute("UnitType", s.unitType);
	unit.setAttribute("x", s.x);
	unit.setAttribute("y", s.y);
 }
 return true;
}

bool BosonScenario::saveValidityHeader(QDataStream& stream)
{
 // magic
 // Qt marshalling for a string is 4-byte-len + data

 int length = QString(TAG_FIELD).length();
 stream << (Q_INT32)(length - 1);

 for (int i = 0; i <= length; i++) {
	stream << (Q_INT8)TAG_FIELD[i];
 }
 return true;
}

bool BosonScenario::verifyScenario(QDataStream& stream)
{
 int length = QString(TAG_FIELD).length();
 // FIXME: use QString/QCString or so instead
 char* magic = new char[ length + 4 ];  // paranoic spaces
 int i;

 // magic 
 // Qt marshalling for a string is 4-byte-len + data
 stream >> i;
 if (length + 1 != i) {
//	kdError() << "BosonScenario::verifyScenario(): Magic doesn't match(len), check file name" << endl;
	delete[] magic;
	return false;
 }

 for (i = 0; i < length + 1; i++) {
	Q_INT8  b;
	stream >> b;
	magic[i] = b;
 }

 if (!QString::compare(magic, TAG_FIELD)) {
//	kdError() << "BosonScenario::verifyScenario(): Magic doesn't match(string), check file name" << endl;
	delete[] magic;
	return false;
 }
 delete[] magic;
 return true;
}

bool BosonScenario::loadPlayer(QDataStream& stream, unsigned int playerNumber)
{
 if (!d->mUnits) {
	kdError() << "BosonScenario::loadPlayer(): Unit array not yet created" 
			<< endl;
	return false;
 }
 if ((int)playerNumber >= maxPlayers()) {
	kdError() << "BosonScenario::loadPlayer(): don't know player " 
			<< playerNumber << endl;
	return false;
 }
 Q_UINT32 unitCount;
 stream >> unitCount;
 for (unsigned int i = 0; i < unitCount; i++) {
	Q_INT32 tag_unit;
	Q_INT32 unitType;
	Q_INT32 x;
	Q_INT32 y;

	stream >> tag_unit;
	stream >> unitType;
	stream >> x;
	stream >> y;

	if (tag_unit != TAG_UNIT) {
		kdError() << "Missing TAG_UNIT" << endl;
		return false;
	}
		
	ScenarioUnit s;
	s.unitType = unitType;
	s.x = x;
	s.y = y;
	d->mUnits[playerNumber].append(s);
 }
 return true;
}

bool BosonScenario::savePlayer(QDataStream& stream, unsigned int playerNumber)
{
 if (!d->mUnits) {
	kdError() << "NULL units" << endl;
	return false;
 }
 if (d->mUnits->count() >= playerNumber) {
	kdError() << "Unknown player " << playerNumber << endl;
	return false;
 }
 stream << (Q_UINT32)d->mUnits[playerNumber].count();
 QValueList<ScenarioUnit>::Iterator it;
 for (it = d->mUnits[playerNumber].begin(); it != d->mUnits[playerNumber].end(); ++it) {
	ScenarioUnit s = (*it);
	stream << (Q_INT32)TAG_UNIT;
	stream << (Q_INT32)s.unitType;
	stream << (Q_INT32)s.x;
	stream << (Q_INT32)s.y;
 }
 return true;
}

bool BosonScenario::isValid() const
{
 if (d->mMinPlayers != (uint)d->mMaxPlayers) { // FIXME
	kdError() << "internal error" << endl;
	return false;
 }
 if (d->mMinPlayers < 1) {
	kdError() << "minplayers < " << 1 << endl;
	return false;
 }
 if (d->mMinPlayers > BOSON_MAX_PLAYERS) {
	kdError() << "minplayers > " << BOSON_MAX_PLAYERS << endl;
	return false;
 }
 if (!d->mUnits) {
	kdError() << "no units!" << endl;
	return false;
 }

 return true;
}

unsigned int BosonScenario::minPlayers() const
{
 return d->mMinPlayers;
}

int BosonScenario::maxPlayers() const
{
 return d->mMaxPlayers;
}

void BosonScenario::addPlayerUnits(Boson* boson, int playerNumber)
{
 if (!boson) {
	kdError() << "BosonScenario::addPlayerUnits(): NULL game" << endl;
	return;
 }
 Player* p = (Player*)boson->playerList()->at(playerNumber);
 if (!p) {
	kdError() << "BosonScenario::addPlayerUnits(): NULL player" << endl;
	return;
 }
 kdDebug() << "BosonScenario::addPlayerUnits of player " << playerNumber 
		<< "==" << p->id() << endl;
 if ((int)d->mUnits->count() < playerNumber) {
	kdError() << "BosonScenario::addPlayerUnits(): don't have player " 
			<< playerNumber << endl;
	return;
 }
 unsigned int unitCount = d->mUnits[playerNumber].count();
 for (unsigned int i = 0; i < unitCount; i++) {
	ScenarioUnit s = (d->mUnits[playerNumber])[i]; // ieek
	addUnit(boson, p, s.unitType, s.x, s.y);
 }
}

void BosonScenario::addUnit(Boson* boson, Player* owner, int unitType, int x, int y)
{
 if (!boson) {
	kdError() << "BosonScenario::addUnit(): NULL game" << endl;
			return;
 }
 if (!owner) {
	kdError() << "BosonScenario::addUnit(): NULL player" << endl;
	return;
 }
 boson->slotSendAddUnit(unitType, x, y, owner);
}

void BosonScenario::startScenario(Boson* boson)
{
 if (!isValid()) {
	return;
 }
 if (!boson) {
	kdError() << "BosonScenario::addPlayerUnits(): NULL game" << endl;
	return;
 }
 for (int i = 0; i < maxPlayers(); i++) {
	addPlayerUnits(boson, i);
 }
}

QStringList BosonScenario::availableScenarios()
{
 QStringList list = KGlobal::dirs()->findAllResources("data",
		"boson/scenario/*.desktop");
 if (list.isEmpty()) {
	kdError() << "Cannot find any scenario?!" << endl;
	return list;
 }
 QStringList validList;
 for (unsigned int i = 0; i < list.count(); i++) {
	QString fileName = list[i].left(list[i].length() -  strlen(".desktop"));
	fileName += QString::fromLatin1(".bsc");
	if (QFile::exists(fileName)) {
		validList.append(list[i]);
	}
 }
 return validList;
}

QStringList BosonScenario::availableScenarios(const QString& mapIdentifier)
{
 QStringList list = availableScenarios();
 QStringList validScenarios; // scenarios for mapIdentifier
 for (unsigned int i = 0; i < list.count(); i++) {
	KSimpleConfig cfg(list[i]);
	cfg.setGroup("Boson Scenario");
	if (cfg.readEntry("Map", "Unknown") == mapIdentifier) {
		validScenarios.append(list[i]);
	}
 }
 return validScenarios;
}

