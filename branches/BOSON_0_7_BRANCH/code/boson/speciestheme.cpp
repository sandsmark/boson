/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2002 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "speciesdata.h"
#include "defines.h"
#include "unit.h"
#include "unitproperties.h"
#include "bosonconfig.h"
#include "bosonprofiling.h"
#include "bosonmodel.h"
#include "sound/bosonmusic.h"
#include "sound/bosonsound.h"
#include "upgradeproperties.h"
#include "bodebug.h"

#include <kstandarddirs.h>
#include <ksimpleconfig.h>

#include <qpixmap.h>
#include <qimage.h>
#include <qbitmap.h>
#include <qintdict.h>
#include <qdir.h>

class SpeciesTheme::SpeciesThemePrivate
{
public:
	SpeciesThemePrivate()
	{
	}

	QIntDict<UnitProperties> mUnitProperties; // they can't be placed into SpeciesData, since they can be modified by upgrades
	QIntDict<TechnologyProperties> mTechnologies; // can't be in SpeciesData - we need setResearched()

	bool mCanChangeTeamColor;
};

static int defaultColorIndex = 0;
QRgb default_color[BOSON_MAX_PLAYERS] = {
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
};

SpeciesTheme::SpeciesTheme(const QString& speciesDir, const QColor& teamColor)
{
 d = new SpeciesThemePrivate;
 d->mUnitProperties.setAutoDelete(true);
 d->mTechnologies.setAutoDelete(true);
 d->mCanChangeTeamColor = true;
 mSound = 0;
 mData = 0;

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

 mSound = boMusic->addSounds(themePath());

 // the initial values for the units - config files :-)
 //readUnitConfigs();

 // action pixmaps - it doesn't hurt
 if (!loadActionGraphics()) {
	boError() << "Couldn't load action pixmaps" << endl;
 }

 // don't preload units here as the species can still be changed in new game
 // dialog 
 return true;
}

bool SpeciesTheme::loadUnit(unsigned long int type)
{
 boProfiling->loadUnit();
 const UnitProperties* prop = unitProperties(type);
 if (!prop) {
	boError() << "Could not load unit type " << type << endl;
	boProfiling->loadUnitDone(type);
	return false;
 }
 // once we load the overview pixmaps the teamcolor can't be changed anymore
 finalizeTeamColor();
 bool ret = mData->loadUnitOverview(prop, teamColor());
 loadUnitModel(prop);

 if (!ret) {
	boProfiling->loadUnitDone(type);
	return false;
 }
 QStringList sounds;
 QMap<int, QString> unitSounds = prop->sounds();
 QMap<int,QString>::Iterator it = unitSounds.begin();
 for (; it != unitSounds.end(); ++it) {
	sounds.append(*it);
 }
 mSound->addUnitSounds(themePath(), sounds);
 boProfiling->loadUnitDone(type);
 return true;
}

bool SpeciesTheme::loadActionGraphics()
{
 return mData->loadActionPixmaps();
}

QPixmap* SpeciesTheme::techPixmap(unsigned long int techType)
{
 return upgradePixmapByName(technology(techType)->pixmapName());
}

QPixmap* SpeciesTheme::upgradePixmapByName(const QString& name)
{
 return mData->upgradePixmapByName(name);
}

bool SpeciesTheme::loadTechnologies()
{
 QFile f(themePath() + "index.technologies");
 if(!f.exists()) {
	boWarning() << k_funcinfo << "Technologies file (" << f.name() << ") does not exists. No technologies loaded" << endl;
	// We assume that this theme has no technologies and still return true
	return true;
 }
 KSimpleConfig cfg(f.name());
 QStringList techs = cfg.groupList();
 if(techs.isEmpty()) {
	boWarning() << k_funcinfo << "No technologies found in technologies file (" << f.name() << ")" << endl;
	return true;
 }
 QStringList::Iterator it;
 for(it = techs.begin(); it != techs.end(); ++it) {
	boDebug() << k_funcinfo << "Loading technology from group " << *it << endl;
	TechnologyProperties* tech = new TechnologyProperties;
	cfg.setGroup(*it);
	tech->load(&cfg);
	if (!d->mTechnologies.find(tech->id())) {
		d->mTechnologies.insert(tech->id(), tech);
	} else {
		boError() << k_funcinfo << "Technology with id " << tech->id() << " already there!" << endl;
	}
 }
 return true;
}

QString SpeciesTheme::unitModelFile()
{
 return SpeciesData::unitModelFile();
}

BosonModel* SpeciesTheme::unitModel(unsigned long int unitType)
{
 return mData->unitModel(unitType, teamColor());
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

QPixmap* SpeciesTheme::actionPixmap(UnitAction action)
{
 // check for NULL?
 return mData->actionPixmap(action);
}

void SpeciesTheme::loadNewUnit(UnitBase* unit)
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

 unit->setHealth(prop->health());
 unit->setArmor(prop->armor());
 unit->setShields(prop->shields());
// unit->setWeaponRange(prop->weaponRange()); // seems to cause a KGame error sometimes
// unit->setWeaponDamage(prop->weaponDamage());
 unit->setSightRange(prop->sightRange());

 if (prop->isMobile()) {
	unit->setSpeed(prop->speed());
 } else if (prop->isFacility()) {

 }
}

void SpeciesTheme::readUnitConfigs(bool full)
{
 // AB: at least the object models are touched here :(
 // they depend on teamcolor, so we won't be able to change teamcolor anymore!
 finalizeTeamColor();
 if (d->mUnitProperties.count() != 0) {
	boError() << "Cannot read unit configs again. Returning untouched..."
			<< endl;
	return;
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
	boError() << "No Units found in this theme" << endl;
	return;
 }
 for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it) {
	UnitProperties* prop = new UnitProperties(this, *it, full);
	if (!d->mUnitProperties.find(prop->typeId())) {
		d->mUnitProperties.insert(prop->typeId(), prop);
	} else {
		boError() << "UnitType " << prop->typeId() << " already there!"
				<< endl;
	}
 }
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

TechnologyProperties* SpeciesTheme::technology(unsigned long int techType) const
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

QValueList<const UnitProperties*> SpeciesTheme::allUnits() const
{
 QValueList<const UnitProperties*> list;
 QIntDictIterator<UnitProperties> it(d->mUnitProperties);
 for (; it.current(); ++it) {
	list.append(it.current());
 }
 return list;
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

/*QValueList<unsigned long int> SpeciesTheme::upgrades(QValueList<int> producers) const
{
 QPtrList<UpgradeProperties> list;
 QIntDictIterator<UnitProperties> it(d->mUnitProperties);
 while (it.current()) {
	QValueList<TechnologyProperties>::Iterator uit(it.current()->unresearchedUpgrades());
	while (*uit) {
		if (producers.contains((*uit)->producer())) {
			list.append(*uit);
		}
		++it;
	}
//	if (producers.contains(it.current()->producer())) {
//		list.append(it.current()->typeId());
//	}
	++it;
 }
 return list;
}*/

QValueList<unsigned long int> SpeciesTheme::technologies(QValueList<unsigned long int> producers) const
{
 QValueList<unsigned long int> list;
 QIntDictIterator<TechnologyProperties> it(d->mTechnologies);
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

void SpeciesTheme::loadUnitModel(const UnitProperties* prop)
{
 // once we load the model the teamcolor can't be changed anymore
 finalizeTeamColor();
 mData->loadUnitModel(prop, teamColor());
}

void SpeciesTheme::playSound(UnitBase* unit, UnitSoundEvent event)
{
 if (boConfig->disableSound()) {
	return;
 }
 if (!sound()) {
	return;
 }
 if (!boConfig->unitSoundActivated(event)) {
	return;
 }
 sound()->play(unit->unitProperties()->sound(event));
}

void SpeciesTheme::playSound(SoundEvent event)
{
 if (boConfig->disableSound()) {
	return;
 }
 if (!sound()) {
	return;
 }
 //TODO;
// if (!boConfig->soundActivated(event)) {
//	return;
// }
 sound()->play(event);
}

void SpeciesTheme::loadGeneralSounds()
{
// TODO: sound mapping!
// speciestheme designers should be able to rename the sounds for certain
// events, just like for unit sounds!
 if (boConfig->disableSound()) {
	return;
 }
 if (!sound()) {
	return;
 }
 QMap<int, QString> sounds;
 sounds.insert(SoundReportMinimapActivated, "report_minimap_activated");
 sounds.insert(SoundReportMinimapDeactivated, "report_minimap_deactivated");
 sound()->addSounds(themePath(), sounds);
}

QIntDict<TechnologyProperties> SpeciesTheme::technologyList() const
{
 return d->mTechnologies;
}

void SpeciesTheme::upgradeResearched(unsigned long int unitType, UpgradeProperties* upgrade)
{
 d->mUnitProperties[unitType]->upgradeResearched(upgrade);
}

void SpeciesTheme::loadParticleSystems()
{
 mData->loadParticleSystemProperties();
}

const BosonParticleSystemProperties* SpeciesTheme::particleSystemProperties(unsigned long int id)
{
 return mData->particleSystemProperties(id);
}

BosonModel* SpeciesTheme::objectModel(const QString& file)
{
 finalizeTeamColor();
 return mData->objectModel(file, teamColor());
}

void SpeciesTheme::finalizeTeamColor()
{
 if (d->mCanChangeTeamColor) {
	d->mCanChangeTeamColor = false;
	mData->addTeamColor(teamColor());
 }
}

