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
#include <qdir.h>
#include <qdom.h>

class SpeciesTheme::SpeciesThemePrivate
{
public:
	SpeciesThemePrivate()
	{
	}

	QIntDict<UnitProperties> mUnitProperties; // they can't be placed into SpeciesData, since they can be modified by upgrades
	QIntDict<UpgradeProperties> mTechnologies; // can't be in SpeciesData - we need setResearched()

	bool mCanChangeTeamColor;
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
 d->mTechnologies.setAutoDelete(true);
 d->mCanChangeTeamColor = true;
 mSound = 0;
 mData = 0;
 boDebug() << "teamcolor: " << teamColor.red() << " " << teamColor.green() << " " << teamColor.blue() << endl;

 // this MUST be called in c'tor, as it initializes mData!
 if (!loadTheme(speciesDir, teamColor)) {
	boError() << "Theme " << speciesDir << " not properly loaded" << endl;
 }
}

SpeciesTheme::~SpeciesTheme()
{
 // AB: do NOT delete mData here
 boDebug() << k_funcinfo << endl;
 reset();

 // for now we also remove our teamcolor object from the species data.
 // one day we should skip this, if possible. or just remove dependancy of most
 // data on the teamcolor.
 // we should keep the data in memory until a new game is started and drop the
 // objects that are not needed anymore
 mData->removeTeamColor(teamColor());

 delete d;
 boDebug() << k_funcinfo << "done" << endl;
}

void SpeciesTheme::reset()
{
 d->mUnitProperties.clear();
 d->mTechnologies.clear();
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
 boDebug() << "theme path: " << themePath() << endl;
 mData = SpeciesData::speciesData(themePath());
 mSound = boAudio->addSounds(themePath());

 // don't preload units here as the species can still be changed in new game
 // dialog
 return true;
}

bool SpeciesTheme::loadUnit(unsigned long int type)
{
 BosonProfiler prof("LoadUnit");
 const UnitProperties* prop = unitProperties(type);
 if (!prop) {
	boError(270) << "Could not load unit type " << type << endl;
	return false;
 }
 // once we load the overview pixmaps the teamcolor can't be changed anymore
 finalizeTeamColor();
 bool ret = mData->loadUnitOverview(prop, teamColor());

 // Unit's produce action is tricky because it needs overview pixmap which is
 //  not loaded when UnitProperties are being loaded. So we load it (and other
 //  actions) here
 nonConstUnitProperties(type)->loadActions();

 if (!loadUnitModel(prop)) {
	boError(270) << k_funcinfo << "unable to load model for unit " << type << endl;
	ret = false;
 }

 if (!ret) {
	return false;
 }
 // Load unit sounds
 BosonProfiler soundProfiling("UnitSound");
 QStringList sounds;
 QMap<int, QString> unitSounds = prop->sounds();
 QMap<int,QString>::Iterator it = unitSounds.begin();
 for (; it != unitSounds.end(); ++it) {
	sounds.append(*it);
 }
 // Load sounds of weapons of this unit
 if (prop->canShoot()) {
	QPtrListIterator<PluginProperties> it(*(prop->plugins()));
	while (it.current()) {
		if (it.current()->pluginType() == PluginProperties::Weapon) {
			QMap<int, QString> weaponSounds = ((BosonWeaponProperties*)it.current())->sounds();
			QMap<int, QString>::Iterator it = weaponSounds.begin();
			for (; it != weaponSounds.end(); ++it) {
				sounds.append(*it);
			}
		}
		++it;
	}
 }
 mSound->addUnitSounds(themePath(), sounds);
 soundProfiling.pop();
 return true;
}

bool SpeciesTheme::loadActions()
{
 mData->loadActions();
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

QPixmap* SpeciesTheme::pixmap(const QString& name)
{
 return mData->pixmap(name);
}

const BoAction* SpeciesTheme::action(const QString& name) const
{
 return mData->action(name);
}

bool SpeciesTheme::loadTechnologies()
{
 QFile f(themePath() + "index.technologies");
 if(!f.exists()) {
	boWarning(270) << k_funcinfo << "Technologies file (" << f.name() << ") does not exists. No technologies loaded" << endl;
	// We assume that this theme has no technologies and still return true
	return true;
 }
 KSimpleConfig cfg(f.name());
 QStringList techs = cfg.groupList();
 if(techs.isEmpty()) {
	boDebug(270) << k_funcinfo << "No technologies found in technologies file (" << f.name() << ")" << endl;
	return true;
 }
 QStringList::Iterator it;
 for(it = techs.begin(); it != techs.end(); ++it) {
	boDebug(270) << k_funcinfo << "Loading upgrade from group " << *it << endl;
	UpgradeProperties* tech = new UpgradeProperties(this);
	if (!tech->load(&cfg, *it)) {
		boError(270) << k_funcinfo << *it << " could not be loaded" << endl;
		delete tech;
		continue;
	}
	if (!d->mTechnologies.find(tech->id())) {
		d->mTechnologies.insert(tech->id(), tech);
	} else {
		boError(270) << k_funcinfo << "Technology with id " << tech->id() << " already there!" << endl;
		delete tech;
	}
 }
 return true;
}

bool SpeciesTheme::loadObjects()
{
 finalizeTeamColor(); // AB: this is obsolete, the models don't use the teamcolor anymore. removing it should be safe.
 return mData->loadObjects(teamColor());
}

QStringList SpeciesTheme::unitModelFiles()
{
 return SpeciesData::unitModelFiles();
}

BosonModel* SpeciesTheme::unitModel(unsigned long int unitType)
{
 return mData->unitModel(unitType);
}

QPixmap* SpeciesTheme::bigOverview(unsigned long int unitType)
{
 QPixmap* p = mData->bigOverview(unitType, teamColor());
 if (!p) {
	loadUnit(unitType);
	p = mData->bigOverview(unitType, teamColor());
	if (!p) {
		boError() << k_funcinfo << "Cannot find unit type " << unitType
				<< endl;
		return 0;
	}
 }
 return p;
}

QPixmap* SpeciesTheme::smallOverview(unsigned long int unitType)
{
 QPixmap* p = mData->smallOverview(unitType, teamColor());
 if (!p) {
	loadUnit(unitType);
	p = mData->smallOverview(unitType, teamColor());
	if (!p) {
		boError() << k_funcinfo << "Cannot find unit type " << unitType
				<< endl;
		return 0;
	}
 }
 return p;
}

QString SpeciesTheme::unitActionName(UnitAction action)
{
 switch (action) {
	case ActionAttack:
		return i18n("Attack");
	case ActionMove:
		return i18n("Move");
	case ActionStop:
		return i18n("Stop");
	case ActionFollow:
		return i18n("Follow");
	case ActionHarvest:
		return i18n("Harvest");
	case ActionRepair:
		return i18n("Repair");
	case ActionProduceUnit:
		return i18n("ProduceUnit");
	case ActionProduceTech:
		return i18n("ProduceTech");
	case ActionChangeHeight:
		return i18n("Change Height");
	default:
		break;
 }
 return QString::null;
}

void SpeciesTheme::loadNewUnit(Unit* unit)
{
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
 finalizeTeamColor();
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

UpgradeProperties* SpeciesTheme::technology(unsigned long int techType) const
{
 if (techType == 0) {
	boError() << k_funcinfo << "invalid technology type " << techType << endl;
	return 0;
 }
 if (!d->mTechnologies[techType]) {
	boError() << k_lineinfo << "oops - no technology properties for " << techType << endl;
	return 0;
 }
 return d->mTechnologies[techType];
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

QValueList<unsigned long int> SpeciesTheme::productions(QValueList<unsigned long int> producers) const
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

QValueList<unsigned long int> SpeciesTheme::technologies(QValueList<unsigned long int> producers) const
{
 QValueList<unsigned long int> list;
 QIntDictIterator<UpgradeProperties> it(d->mTechnologies);
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
 if (!d->mCanChangeTeamColor) {
	boWarning() << "Cannot change team color anymore!" << endl;
	return false;
 }
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

bool SpeciesTheme::loadUnitModel(const UnitProperties* prop)
{
 // once we load the model the teamcolor can't be changed anymore
 finalizeTeamColor();
 return mData->loadUnitModel(prop, teamColor());
}

void SpeciesTheme::playSound(UnitBase* unit, UnitSoundEvent event)
{
 if (boConfig->boolValue("ForceDisableSound")) {
	return;
 }
 if (!sound()) {
	return;
 }
 if (!boConfig->unitSoundActivated(event)) {
	return;
 }
 sound()->playSound(unit->unitProperties()->sound(event));
}

void SpeciesTheme::playSound(SoundEvent event)
{
 if (boConfig->boolValue("ForceDisableSound")) {
	return;
 }
 if (!sound()) {
	return;
 }
 //TODO;
// if (!boConfig->soundActivated(event)) {
//	return;
// }
 sound()->playSound(event);
}

void SpeciesTheme::playSound(const BosonWeaponProperties* weaponProp, WeaponSoundEvent event)
{
 if (boConfig->boolValue("ForceDisableSound")) {
	return;
 }
 if (!sound()) {
	return;
 }
 if (boConfig->boolValue("DeactivateWeaponSounds")) {
	return;
 }
 sound()->playSound(weaponProp->sound(event));
}

bool SpeciesTheme::loadGeneralSounds()
{
// TODO: sound mapping!
// speciestheme designers should be able to rename the sounds for certain
// events, just like for unit sounds!
 if (boConfig->boolValue("ForceDisableSound")) {
	return true;
 }
 if (!sound()) {
	return true;
 }
 QMap<int, QString> sounds;
 sounds.insert(SoundReportMinimapActivated, "report_minimap_activated");
 sounds.insert(SoundReportMinimapDeactivated, "report_minimap_deactivated");
 sound()->addSounds(themePath(), sounds);
 return true;
}

const QIntDict<UpgradeProperties>& SpeciesTheme::technologyList() const
{
 return d->mTechnologies;
}

BosonModel* SpeciesTheme::objectModel(const QString& name) const
{
 return mData->objectModel(name);
}

void SpeciesTheme::finalizeTeamColor()
{
 if (d->mCanChangeTeamColor) {
	d->mCanChangeTeamColor = false;
	mData->addTeamColor(teamColor());
 }
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
	type.appendChild(upgradesTag);

	if (!prop->saveUpgradesAsXML(upgradesTag)) {
		boError() << k_funcinfo << "unable to save upgrades for unit type " << prop->typeId() << endl;
		return false;
	}
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
 }
 return true;
}


