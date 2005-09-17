/*
    This file is part of the Boson game
    Copyright (C) 1999-2000 Thomas Capricelli (capricel@email.enst.fr)
    Copyright (C) 2001-2005 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2001-2005 Rivo Laks (rivolaks@hot.ee)

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

#include "speciestheme.h"

#include "../bomemory/bodummymemory.h"
#include "speciesdata.h"
#include "defines.h"
#include "unit.h"
#include "unitproperties.h"
#include "bosonconfig.h"
#include "bosonprofiling.h"
#include "upgradeproperties.h"
#include "bodebug.h"
#include "bosonweapon.h"
#include "sound/bosonaudiointerface.h"

#include <kstandarddirs.h>
#include <ksimpleconfig.h>
#include <klocale.h>

#include <qintdict.h>
#include <qdict.h>
#include <qdir.h>
#include <qdom.h>

class UpgradesContainer
{
public:
	UpgradesContainer()
	{
		mUpgrades.setAutoDelete(true);
	}
	~UpgradesContainer()
	{
		mConstUpgrades.setAutoDelete(false);
		mConstUpgrades.clear();
		mUpgrades.setAutoDelete(true);
		mUpgrades.clear();
	}

	void insertUpgrade(UpgradeProperties* upgrade)
	{
		BO_CHECK_NULL_RET(upgrade);
		mUpgrades.insert(upgrade->id(), upgrade);
		mConstUpgrades.insert(upgrade->id(), upgrade);
	}
	const QIntDict<const UpgradeProperties>& upgrades() const
	{
		return mConstUpgrades;
	}

	const UpgradeProperties* upgrade(unsigned long int id) const
	{
		return mConstUpgrades[id];
	}
private:
	QIntDict<UpgradeProperties> mUpgrades;
	QIntDict<const UpgradeProperties> mConstUpgrades;
};

class SpeciesThemePrivate
{
public:
	SpeciesThemePrivate()
	{
	}

	// data that cannot go to SpeciesData as it must be modifyable by some
	// reasons
	// (e.g. because of upgrades)
	QIntDict<UnitProperties> mUnitProperties;

	QDict< UpgradesContainer > mUpgrades;
};

static int defaultColorIndex = 0;
QRgb default_color[BOSON_MAX_PLAYERS + 1] = { // AB: + 1 because of neutral player
	qRgb(0,0,255),
	qRgb(0,255,0),
	qRgb(255,0,0),
	qRgb(255,255,0),
	qRgb(255,0,255),
	qRgb(0,255,255),
	qRgb(127,255,0),
	qRgb(255,0,127),
	qRgb(0,127,255),
	qRgb(0,127,127),
	qRgb(127,0,255),
};

SpeciesTheme::SpeciesTheme(const QString& speciesDir, const QColor& teamColor)
{
 d = new SpeciesThemePrivate;
 d->mUnitProperties.setAutoDelete(true);
 d->mUpgrades.setAutoDelete(true);

 if (!loadTheme(speciesDir, teamColor)) {
	boError() << "Theme " << speciesDir << " not properly loaded" << endl;
 }
}

SpeciesTheme::~SpeciesTheme()
{
 boDebug() << k_funcinfo << endl;
 reset();

 delete d;
 boDebug() << k_funcinfo << "done" << endl;
}

void SpeciesTheme::reset()
{
 d->mUnitProperties.setAutoDelete(true);
 d->mUnitProperties.clear();
 d->mUpgrades.setAutoDelete(true);
 d->mUpgrades.clear();
}

QColor SpeciesTheme::defaultColor()
{
 defaultColorIndex++;
 return QColor(default_color[defaultColorIndex - 1]);
}

bool SpeciesTheme::loadTheme(const QString& speciesDir, const QColor& teamColor)
{
 if (teamColor == QColor(0,0,0)) { // no color specified
	setTeamColor(defaultColor());
 } else {
	setTeamColor(teamColor);
 }
 mThemePath = speciesDir;
 boDebug() << k_funcinfo << "theme path: " << themePath() << endl;

 // don't preload units here as the species can still be changed in new game
 // dialog
 return true;
}

QCString SpeciesTheme::unitPropertiesMD5() const
{
 QCString string;
 QIntDictIterator<UnitProperties> it(d->mUnitProperties);
 while (it.current()) {
	if (string.isNull()) {
		string = it.current()->md5();
	} else {
		string += "\n";
		string += it.current()->md5();
	}
	++it;
 }
 return string;
}

bool SpeciesTheme::loadTechnologies()
{
 QFile f(themePath() + "index.technologies");
 if (!f.exists()) {
	boWarning(270) << k_funcinfo << "Technologies file (" << f.name() << ") does not exists. No technologies loaded" << endl;
	// We assume that this theme has no technologies and still return true
	return true;
 }
 KSimpleConfig cfg(f.name());
 QStringList techs = cfg.groupList();
 if (techs.isEmpty()) {
	boDebug(270) << k_funcinfo << "No technologies found in technologies file (" << f.name() << ")" << endl;
	return true;
 }
 for (QStringList::Iterator it = techs.begin(); it != techs.end(); ++it) {
	boDebug(270) << k_funcinfo << "Loading upgrade from group " << *it << endl;
	UpgradeProperties* tech = new UpgradeProperties("Technology", this);
	if (!tech->load(&cfg, *it)) {
		boError(270) << k_funcinfo << *it << " could not be loaded" << endl;
		delete tech;
		continue;
	}
	insertUpgrade(tech);
 }
 return true;
}

void SpeciesTheme::loadNewUnit(Unit* unit)
{
 PROFILE_METHOD
 if (!unit) {
	boError() << k_funcinfo << "NULL unit" << endl;
	return;
 }
 const UnitProperties* prop = unit->unitProperties(); //unitProperties(unit);
 if (!prop) {
	boError() << k_funcinfo << "NULL properties for " << unit->type() << endl;
	return;
 }

 unit->clearUpgrades();
 const QValueList<const UpgradeProperties*>* upgrades = unit->upgradesCollection().upgrades();
 for (QValueList<const UpgradeProperties*>::const_iterator it = upgrades->begin(); it != upgrades->end(); ++it) {
	unit->addUpgrade(*it);
 }

 unit->setHealth(unit->maxHealth());
 unit->setArmor(unit->maxArmor());
 unit->setShields(unit->maxShields());
 unit->setSightRange(unit->maxSightRange());

 if (prop->isMobile()) {
 } else if (prop->isFacility()) {
 }
}

bool SpeciesTheme::readUnitConfigs(bool full)
{
 // AB: at least the object models are touched here :(
 // they depend on teamcolor, so we won't be able to change teamcolor anymore!
 if (d->mUnitProperties.count() != 0) {
	boError(270) << "Cannot read unit configs again. Returning untouched..."
			<< endl;
	return true;
 }
 QDir dir(themePath());
 dir.cd(QString::fromLatin1("units"));
 QStringList dirList = dir.entryList(QDir::Dirs);
 QStringList list;
 for (unsigned int i = 0; i < dirList.count(); i++) {
	if (dirList[i] == QString::fromLatin1("..") ||
			dirList[i] == QString::fromLatin1(".")) {
		continue;
	}
	QString file = dir.path() + QString::fromLatin1("/") + dirList[i] +
			QString::fromLatin1("/index.unit");
	if (QFile::exists(file)) {
		list.append(file);
	}
 }

 if (list.isEmpty()) {
	boWarning(270) << "No Units found in this theme" << endl;
	return true;
 }
 for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it) {
	UnitProperties* prop = new UnitProperties(this);
	if (!prop->loadUnitType(*it, full)) {
		boError(270) << k_funcinfo << "could not load " << *it << endl;
		delete prop;
		return false;
	}
	if (!d->mUnitProperties.find(prop->typeId())) {
		d->mUnitProperties.insert(prop->typeId(), prop);
	} else {
		boError(270) << "UnitType " << prop->typeId() << " already there!"
				<< endl;
		delete prop;
		return false;
	}
 }
 return true;
}

const UnitProperties* SpeciesTheme::unitProperties(unsigned long int unitType) const
{
 if (unitType == 0) {
	boError() << k_funcinfo << "invalid unit type " << unitType << endl;
	return 0;
 }
 if (!d->mUnitProperties[unitType]) {
	boError() << k_lineinfo << "oops - no unit properties for " << unitType << endl;
	return 0;
 }
 return d->mUnitProperties[unitType];
}

UnitProperties* SpeciesTheme::nonConstUnitProperties(unsigned long int unitType) const
{
 if (unitType == 0) {
	boError() << k_funcinfo << "invalid unit type " << unitType << endl;
	return 0;
 }
 if (!d->mUnitProperties[unitType]) {
	boError() << k_lineinfo << "oops - no unit properties for " << unitType << endl;
	return 0;
 }
 return d->mUnitProperties[unitType];
}

bool SpeciesTheme::hasUnitProperties(unsigned long int unitType) const
{
 if (unitType == 0) {
	return false;
 }
 if (!d->mUnitProperties[unitType]) {
	return false;
 }
 return true;
}

const UpgradeProperties* SpeciesTheme::technology(unsigned long int techType) const
{
 return upgrade("Technology", techType);
}

const UpgradeProperties* SpeciesTheme::upgrade(const QString& type, unsigned long int id) const
{
 if (id == 0) {
	boError() << k_funcinfo << "invalid id 0" << endl;
	return 0;
 }
 const UpgradeProperties* upgrade = 0;
 UpgradesContainer* c = d->mUpgrades[type];
 if (c) {
	upgrade = c->upgrade(id);
 }
 if (!upgrade) {
	boError() << k_funcinfo << "NULL upgrade for type=" << type << " id=" << id << endl;
	return 0;
 }
 return upgrade;
}

QValueList<unsigned long int> SpeciesTheme::allFacilities() const
{
 QValueList<unsigned long int> list;
 QIntDictIterator<UnitProperties> it(d->mUnitProperties);
 while (it.current()) {
	if (it.current()->isFacility()) {
		list.append(it.current()->typeId());
	}
	++it;
 }
 return list;
}

QValueList<unsigned long int> SpeciesTheme::allMobiles() const
{
 QValueList<unsigned long int> list;
 QIntDictIterator<UnitProperties> it(d->mUnitProperties);
 while (it.current()) {
	if (it.current()->isMobile()) {
		list.append(it.current()->typeId());
	}
	++it;
 }
 return list;
}

const QIntDict<UnitProperties>* SpeciesTheme::allUnitsNonConst() const
{
 return &d->mUnitProperties;
}

QValueList<const UnitProperties*> SpeciesTheme::allUnits() const
{
 QValueList<const UnitProperties*> list;
 QIntDictIterator<UnitProperties> it(d->mUnitProperties);
 for (; it.current(); ++it) {
	list.append(it.current());
 }
 return list;
}

QStringList SpeciesTheme::allObjects(QStringList* files) const
{
 if (files) {
	*files = QStringList();
 }
 QString fileName = themePath() + QString::fromLatin1("objects/objects.boson");
 if (!KStandardDirs::exists(fileName)) {
	boDebug() << k_funcinfo << "no objects.boson file found" << endl;
	// We assume that this theme has no objects and don't complain
	return QStringList();
 }

 KSimpleConfig cfg(fileName);
 QStringList groups = cfg.groupList();

 // all groups must have a File entry
 QStringList objects;
 for (unsigned int i = 0; i < groups.count(); i++) {
		cfg.setGroup(groups[i]);
		if (!cfg.hasKey(QString::fromLatin1("File"))) {
			boError() << k_funcinfo << "group " << groups[i] << " has no File key" << endl;
		} else {
			objects.append(groups[i]);
			if (files) {
				QString file = cfg.readEntry(QString::fromLatin1("File"));
				files->append(file);
			}
		}
 }
 if (objects.isEmpty()) {
	boWarning() << k_funcinfo << "No objects found in objects file (" << fileName << ")" << endl;
 }
 return objects;
}

QValueList<unsigned long int> SpeciesTheme::productions(const QValueList<unsigned long int>& producers) const
{
 QValueList<unsigned long int> list;
 QIntDictIterator<UnitProperties> it(d->mUnitProperties);
 while (it.current()) {
	if (producers.contains(it.current()->producer())) {
		list.append(it.current()->typeId());
	}
	++it;
 }
 return list;
}

QValueList<unsigned long int> SpeciesTheme::technologies(const QValueList<unsigned long int>& producers) const
{
 QValueList<unsigned long int> list;
 if (!d->mUpgrades["Technology"]) {
	return list;
 }
 QIntDictIterator<const UpgradeProperties> it(d->mUpgrades["Technology"]->upgrades());
 while (it.current()) {
	if (producers.contains(it.current()->producer())) {
		list.append(it.current()->id());
	}
	++it;
 }
 return list;
}

QStringList SpeciesTheme::availableSpecies()
{
 QStringList list = KGlobal::dirs()->findAllResources("data",
		"boson/themes/species/*/index.species");
 if (list.isEmpty()) {
	boWarning() << "No species found!" << endl;
	return list;
 }
 return list;
}

QString SpeciesTheme::defaultSpecies()
{
 return QString::fromLatin1("Human");
}

QString SpeciesTheme::speciesDirectory(const QString& identifier)
{
 QStringList l = availableSpecies();
 for (unsigned int i = 0; i < l.count(); i++) {
	KSimpleConfig cfg(l[i]);
	cfg.setGroup("Boson Species");
	if (cfg.readEntry("Identifier") == identifier) {
		QString d = l[i].left(l[i].length() - strlen("index.species"));
		return d;
	}
 }
 return QString::null;
}

QString SpeciesTheme::identifier() const
{
 KSimpleConfig cfg(themePath() + QString::fromLatin1("index.species"));
 cfg.setGroup("Boson Species");
 return cfg.readEntry("Identifier");
}

bool SpeciesTheme::setTeamColor(const QColor& color)
{
 mTeamColor = color;
 return true;
}

QValueList<QColor> SpeciesTheme::defaultColors()
{
 QValueList<QColor> colors;
 for (int i = 0; i < BOSON_MAX_PLAYERS; i++) {
	colors.append(QColor(default_color[i]));
 }
 return colors;
}

bool SpeciesTheme::saveGameDataAsXML(QDomElement& root) const
{
 root.setAttribute("Identifier", identifier());
 root.setAttribute("TeamColor", teamColor().rgb());

 QDomDocument doc = root.ownerDocument();
 QDomElement unitTypes = doc.createElement("UnitTypes");
 root.appendChild(unitTypes);
 for (QIntDictIterator<UnitProperties> it(d->mUnitProperties); it.current(); ++it) {
	const UnitProperties* prop = it.current();
	QDomElement type = doc.createElement("UnitType");
	type.setAttribute("Id", prop->typeId());
	unitTypes.appendChild(type);

	QDomElement upgradesTag = doc.createElement("Upgrades");
	if (!prop->saveUpgradesAsXML(upgradesTag)) {
		boError() << k_funcinfo << "unable to save upgrades for unit type " << prop->typeId() << endl;
		return false;
	}
	type.appendChild(upgradesTag);

	const QPtrList<PluginProperties>* plugins = prop->plugins();
	QDomElement pluginsTag = doc.createElement("PluginProperties");
	for (QPtrListIterator<PluginProperties> it(*plugins); it.current(); ++it) {
		QDomElement plugin = doc.createElement("Plugin");
		plugin.setAttribute("Id", QString::number(it.current()->pluginType()));
		QDomElement pluginUpgrades = doc.createElement("Upgrades");
		plugin.appendChild(pluginUpgrades);

		pluginsTag.appendChild(plugin);
	}
	type.appendChild(pluginsTag);
 }
 return true;
}

bool SpeciesTheme::loadGameDataFromXML(const QDomElement& root)
{
 QDomElement unitTypes = root.namedItem("UnitTypes").toElement();
 if (unitTypes.isNull()) {
	boError() << k_funcinfo << "NULL UnitTypes tag" << endl;
	return false;
 }
 for (QIntDictIterator<UnitProperties> it(d->mUnitProperties); it.current(); ++it) {
	it.current()->clearUpgrades();
 }
 for (QDomNode n = unitTypes.firstChild(); !n.isNull(); n = n.nextSibling()) {
	QDomElement type = n.toElement();
	if (type.isNull()) {
		continue;
	}
	if (type.tagName() != "UnitType") {
		continue;
	}
	bool ok = false;
	unsigned long int id = type.attribute("Id").toULong(&ok);
	if (!ok) {
		boError() << k_funcinfo << "invalid number for Id of UnitType" << endl;
		return false;
	}
	if (!hasUnitProperties(id)) {
		boDebug() << k_funcinfo << "Have no UnitProperties for " << id << " - ignoring." << endl;
		// AB: not an error. this may happen if the designer of a map
		//     has non-default unittypes installed.
		//     the unittypes are stored in the map then, even if they
		//     are unused by the map.
		continue;
	}
	UnitProperties* prop = nonConstUnitProperties(id);
	if (!prop) {
		boError() << k_funcinfo << "cannot find unitproperties for " << id << endl;
		return false;
	}
	QDomElement upgrades = type.namedItem("Upgrades").toElement();
	if (upgrades.isNull()) {
		boError() << k_funcinfo << "NULL Upgrades tag for UnitType " << id << endl;
		return false;
	}
	if (!prop->loadUpgradesFromXML(upgrades)) {
		boError() << k_funcinfo << "unable to load Upgrades of UnitType " << id << endl;
		return false;
	}

	QDomElement pluginsTag = type.namedItem("PluginProperties").toElement();
	if (pluginsTag.isNull()) {
		boError() << k_funcinfo << "NULL PluginProperties tag" << endl;
		return false;
	}
	for (QDomNode n2 = pluginsTag.firstChild(); !n2.isNull(); n2 = n2.nextSibling()) {
		QDomElement plugin = n2.toElement();
		if (plugin.isNull()) {
			continue;
		}
		if (plugin.tagName() == "Plugin") {
			continue;
		}
		bool ok = false;
		int id = plugin.attribute("Id").toInt(&ok);
		if (!ok) {
			boError() << k_funcinfo << "invalid value for Id attribute of Plugin" << endl;
			return false;
		}
		QDomElement upgrades = plugin.namedItem("Upgrades").toElement();
		if (upgrades.isNull()) {
			boError() << k_funcinfo << "NULL Upgrades tag of Plugin" << endl;
			return false;
		}
		PluginProperties* p = 0;
		for (QPtrListIterator<PluginProperties> it(*prop->plugins()); it.current() && !p; ++it) {
			if (it.current()->pluginType() == id) {
				p = it.current();
			}
		}
		if (!p) {
			boError() << k_funcinfo << "cannot find properties with ID " << id << endl;
			return false;
		}
		
	}
 }
 return true;
}

// AB: I did not choose "addUpgrade()" as name to avoid confusing with the
// addUpgrade() methods that are called when an upgrade is gained
void SpeciesTheme::insertUpgrade(UpgradeProperties* upgrade)
{
 if (!upgrade) {
	return;
 }
 UpgradesContainer* c = d->mUpgrades[upgrade->type()];
 if (!c) {
	c = new UpgradesContainer();
	d->mUpgrades.insert(upgrade->type(), c);
 }
 if (c->upgrade(upgrade->id())) {
	boError() << k_funcinfo << "tried to insert upgrade with type=" << upgrade->type() << " and id=" << upgrade->id() << " twice" << endl;
	delete upgrade;
	return;
 }
 c->insertUpgrade(upgrade);
}

