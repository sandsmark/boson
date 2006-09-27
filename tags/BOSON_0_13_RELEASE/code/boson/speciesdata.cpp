/*
    This file is part of the Boson game
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

#include "speciesdata.h"

#include "../bomemory/bodummymemory.h"
#include "gameengine/unitproperties.h"
#include "gameengine/unitbase.h"
#include "gameengine/bosonweapon.h"
#include "modelrendering/bosonmodel.h"
#include "bodebug.h"
#include "boaction.h"
#include "bosonconfig.h"
#include "bosonprofiling.h"
#include "sound/bosonaudiointerface.h"
#include "boufo/boufoimage.h"

#include <qintdict.h>
#include <qdict.h>
#include <qfile.h>
#include <qimage.h>
#include <qtextstream.h>

#include <kstaticdeleter.h>
#include <kstandarddirs.h>
#include <ksimpleconfig.h>
#include <ktempfile.h>

static KStaticDeleter< QDict<SpeciesData> > sd;
QDict<SpeciesData>* SpeciesData::mSpeciesData = 0;

static void killAlphaMask(QImage& img)
{
 if (img.isNull()) {
	return;
 }
 if (!img.hasAlphaBuffer()) {
	return;
 }
 if (img.depth() != 32) {
	return;
 }
 QRgb *p = 0;
 for (int y = 0; y < img.height(); y++) {
	p  = (QRgb*)img.scanLine(y);
	for (int x = 0; x < img.width(); x++, p++) {
		// set alpha to 255
		*p = (*p) | (0xFF000000);
	}
 }
}

class BosonModelFactory
{
public:
	BosonModelFactory() { }
	~BosonModelFactory() { }

	BosonModel* createUnitModel(const UnitProperties* prop, const QString& file);
	BosonModel* createObjectModel(const KSimpleConfig* config, const QString& themePath);
};

BosonModel* BosonModelFactory::createUnitModel(const UnitProperties* prop, const QString& file)
{
 BosonModel* m = new BosonModel(prop->unitPath(), file);
 QString configfile = prop->unitPath() + QString::fromLatin1("index.unit");
 if (!m->loadModel(configfile)) {
	boError() << k_funcinfo << "model loading failed" << endl;
	delete m;
	return 0;
 }


 // now we load animation information. this is just which frame is used for
 // which animation mode - no frame/node/display list modifying needs to be
 // made.
 KSimpleConfig cfg(configfile);
 cfg.setGroup("Model");
 m->loadAnimationMode(UnitAnimationIdle, &cfg, QString::fromLatin1("Idle"));
 m->loadAnimationMode(UnitAnimationWreckage, &cfg, QString::fromLatin1("Wreckage"));
 m->loadAnimationMode(UnitAnimationConstruction, &cfg, QString::fromLatin1("Construction"));

 if (prop->isFacility()) {
	m->generateConstructionAnimation(prop->constructionSteps());
 }

 return m;
}

BosonModel* BosonModelFactory::createObjectModel(const KSimpleConfig* config, const QString& themePath)
{
 // Object models doesn't have per-model config, but bobmfconfig requires it to
 //  set size of the model.
 // So we create a temporary config file and copy necessary data there.
 // TODO: separate BosonModelFactory into it's own file and make it also handle
 //  model caching?
 KTempFile tmpconfig;
 // We want the temporary file to be deleted when we're done.
 tmpconfig.setAutoDelete(true);

 // We write the config file manually, using QTextStream.
 // There's no point in creating KConfig object to write a single entry.
 QTextStream* ts = tmpconfig.textStream();
 if (!ts) {
	boError() << k_funcinfo << "Couldn't create textstream object for KTempFile" << endl;
	return 0;
 }
 (*ts) << "[Model]" << endl;
 (*ts) << "Size=" << config->readDoubleNumEntry(QString::fromLatin1("Width"), 1.0f) << endl;
 tmpconfig.close();

 // Load name of the model file
 QString file = config->readEntry(QString::fromLatin1("File"), QString::fromLatin1("missile.3ds"));


 BosonModel* m = new BosonModel(themePath + QString::fromLatin1("/objects/"), file);
 if (!m->loadModel(tmpconfig.name())) {
	boError() << k_funcinfo << "model loading failed" << endl;
	delete m;
	return 0;
 }

 return m;
}


class SpeciesData::SpeciesDataPrivate
{
public:
	SpeciesDataPrivate()
	{
	}
	// warning: OpenGL classes should not yet be here, as they depend on the
	// current OpenGL context!
	// we should separate our OpenGL context from BosonBigDisplay first
	QString mThemePath;
	QIntDict<TeamColorData> mTeamData;
	QDict<BoAction> mActions;
	QDict<BoUfoImage> mImages;

	QIntDict<BosonModel> mUnitModels;
	QDict<BosonModel> mObjectModels;

	BosonSoundInterface* mSound;
};

class SpeciesData::TeamColorData
{
public:
	TeamColorData()
	{
		mSmallOverview.setAutoDelete(true);
		mBigOverview.setAutoDelete(true);
	}
	~TeamColorData()
	{
		mSmallOverview.clear();
		mBigOverview.clear();
	}

	QIntDict<BoUfoImage> mSmallOverview;
	QIntDict<BoUfoImage> mBigOverview;
};

SpeciesData::SpeciesData(const QString& speciesPath)
{
 initStatic();
 if (mSpeciesData->find(speciesPath)) {
	boError() << k_funcinfo << speciesPath << " already present (we have a memory hole now)" << endl;
	// AB: we can't delete this here, as the pointer will be invalid (and
	// non-NULL) otherwise.
	return;
 }
 d = new SpeciesDataPrivate;
 d->mTeamData.setAutoDelete(true);
 d->mUnitModels.setAutoDelete(true);
 d->mObjectModels.setAutoDelete(true);
 d->mActions.setAutoDelete(true);
 d->mImages.setAutoDelete(true);
 d->mThemePath = speciesPath;
 d->mSound = boAudio->addSounds(themePath());
}

SpeciesData::~SpeciesData()
{
 d->mTeamData.clear();
 d->mUnitModels.clear();
 d->mObjectModels.clear();
 d->mActions.clear();
 d->mImages.clear();
 delete d;
}

void SpeciesData::initStatic()
{
 if (mSpeciesData) {
	return;
 }
 mSpeciesData = new QDict<SpeciesData>();
 mSpeciesData->setAutoDelete(true);
 sd.setObject(mSpeciesData);
}

SpeciesData* SpeciesData::speciesData(const QString& speciesDir)
{
 initStatic();
 SpeciesData* species = mSpeciesData->find(speciesDir);
 if (!species) {
	species = new SpeciesData(speciesDir);
	mSpeciesData->insert(speciesDir, species);
 }
 return species;
}

void SpeciesData::clearSpeciesData()
{
 if (!mSpeciesData) {
	return;
 }
 mSpeciesData->clear();
}

QString SpeciesData::themePath() const
{
 return d->mThemePath;
}

bool SpeciesData::loadUnit(const UnitProperties* prop, const QColor& teamColor)
{
 BosonProfiler prof("LoadUnit");
 if (!prop) {
	BO_NULL_ERROR(prop);
	return false;
 }
 if (!loadUnitOverview(prop, teamColor)) {
	return false;
 }

 if (!loadUnitModel(prop, teamColor)) {
	return false;
 }
 if (!loadUnitSounds(prop)) {
	return false;
 }
 return true;
}

bool SpeciesData::loadUnitModel(const UnitProperties* prop, const QColor& )
{
 if (!prop) {
	BO_NULL_ERROR(prop);
	return false;
 }
 if (boConfig->boolValue("ForceDisableModelLoading")) {
	return true;
 }
 BosonModel* m = d->mUnitModels[prop->typeId()];

 if (!m) {
	QStringList fileNames = unitModelFiles();
	bool found = false;
	QString file;
	for (QStringList::Iterator it = fileNames.begin(); it != fileNames.end(); ++it) {
		if (KStandardDirs::exists(prop->unitPath() + *it)) {
			file = *it;
			found = true;
			break;
		}
	}
	if (!found) {
		boError(270) << k_funcinfo << "Cannot find model file file for " << prop->typeId() << endl;
		return false;
	}

	BosonModelFactory factory;
	m = factory.createUnitModel(prop, file);
	if (m) {
		d->mUnitModels.insert(prop->typeId(), m);
	} else {
		boError(270) << k_funcinfo << "NULL model created" << endl;
		return false;
	}
 }
 return true;
}

BosonModel* SpeciesData::unitModel(unsigned long int unitType) const
{
 return d->mUnitModels[unitType];
}

QStringList SpeciesData::unitModelFiles()
{
 QStringList list;
 list.append("unit.3ds");
 list.append("unit.ac");
 list.append("unit.md2");
 return list;
}

BosonSoundInterface* SpeciesData::sound() const
{
 return d->mSound;
}

SpeciesData::TeamColorData* SpeciesData::teamColorData(const QColor& color) const
{
 TeamColorData* data = d->mTeamData[color.rgb()];
 return data;
}

void SpeciesData::addTeamColor(const QColor& color)
{
 TeamColorData* data = d->mTeamData[color.rgb()];
 if (data) {
	return;
 }
 data = new TeamColorData;
 d->mTeamData.insert(color.rgb(), data);
}

void SpeciesData::removeTeamColor(const QColor& color)
{
 d->mTeamData.remove(color.rgb()); // will delete the object!
}

bool SpeciesData::loadUnitOverview(const UnitProperties* prop, const QColor& teamColor)
{
 BosonProfiler prof("LoadUnitOverview");
 TeamColorData* data = teamColorData(teamColor);
 if (!data) {
	boError(270) << k_funcinfo << "NULL teamcolor data" << endl;
	return false;
 }
 unsigned long int type = prop->typeId();
 QString path = prop->unitPath();

// big overview
 if (!data->mBigOverview[type]) {
	QImage image;

	if (!loadUnitImage(teamColor, path + "overview-big.png", image)) {
		boError(270) << k_funcinfo << "Can't load " << path + "overview-big.png" << endl;
		image = QImage(100, 100, 32);
		image.fill(Qt::red.rgb());
	}
	if (image.width() != 100 || image.height() != 100) {
		image = image.smoothScale(100, 100, QImage::ScaleMin);
	}

	// AB: maybe we want to remove this in the future
	killAlphaMask(image);

	BoUfoImage* boufoImage = new BoUfoImage(image);
	data->mBigOverview.insert(type, boufoImage);
 }

// small overview
 if (!data->mSmallOverview[type]) {
	QImage image;

	if (!loadUnitImage(teamColor, path + "overview-small.png", image)) {
		boError(270) << k_funcinfo << "Can't load " << path + "overview-small.png" << endl;
		image = QImage(50, 50, 32);
		image.fill(Qt::red.rgb());
	}
	if (image.width() != 50 || image.height() != 50) {
		image = image.smoothScale(50, 50, QImage::ScaleMin);
	}

	// AB: maybe we want to remove this in the future
	killAlphaMask(image);

	BoUfoImage* boufoImage = new BoUfoImage(image);
	data->mSmallOverview.insert(type, boufoImage);
 }

 return true;
}


bool SpeciesData::loadUnitImage(const QColor& teamColor, const QString &fileName, QImage &_image)
{
 BosonProfiler prof("LoadUnitImage");
 boProfiling->push("QImage loading");
 QImage image(fileName);
 boProfiling->pop(); // "QImage loading"
// image.setAlphaBuffer(false);
 int x, y, w, h;
 QRgb *p = 0;
 static const QRgb background = qRgb(255,  0, 255) & RGB_MASK ;
 static const QRgb background2 = qRgb(248, 40, 240) & RGB_MASK ;

 w = image.width();
 h = image.height();

 if (image.isNull()) {
	boDebug(270) << k_funcinfo << fileName << ": NULL image" << endl;
	return false;
 }
 if (image.depth() != 32) {
	boError(270) << k_funcinfo << fileName << ": depth != 32" << endl;
	return false;
 }
 if (w < 32) {
	boError(270) << k_funcinfo << fileName << ": w < 32" << endl;
	return false;
 }
 if (h < 32) {
	boError(270) << k_funcinfo << fileName << ": h < 32" << endl;
	return false;
 }


 boProfiling->push("teamcolor");
 for ( y = 0; y < h; y++ ) {
	p  = (QRgb *)image.scanLine(y);	// image
	for ( x = 0; x < w; x++, p++ ) {
		if ( (qRed(*p) > 0x90) && (qGreen(*p) < 0x60) && (qBlue(*p) < 0x60)) {
			*p = teamColor.rgb();
		}
	}
 }
 boProfiling->pop(); // "teamcolor"

 if (image.isNull() || w < 32 || h < 32)  {
	boError(270) << k_funcinfo << "image is null" << endl;
	return false;
 }
 _image = image;
 return true;
}

BoUfoImage* SpeciesData::bigOverview(unsigned long int unitType, const QColor& teamColor) const
{
 TeamColorData* data = teamColorData(teamColor);
 if (!data) {
	boError() << k_funcinfo << "NULL teamcolor data" << endl;
	return 0;
 }
 return data->mBigOverview[unitType];
}

BoUfoImage* SpeciesData::smallOverview(unsigned long int unitType, const QColor& teamColor) const
{
 TeamColorData* data = teamColorData(teamColor);
 if (!data) {
	boError() << k_funcinfo << "NULL teamcolor data" << endl;
	return 0;
 }
 return data->mSmallOverview[unitType];
}

const BoAction* SpeciesData::action(const QString& name) const
{
 BoAction* action = d->mActions[name];
 if (!action) {
	boError() << k_funcinfo << "No action with name " << name << endl;
	return 0;
 }
 return action;
}

BosonModel* SpeciesData::objectModel(const QString& name) const
{
 BosonModel* m = d->mObjectModels[name];
 if (!m && !boConfig->boolValue("ForceDisableModelLoading")) {
	boError() << k_funcinfo << "No object with name " << name << endl;
	return 0;
 }
 return m;
}

bool SpeciesData::loadObjects(const QColor& teamColor)
{
 BosonProfiler prof("LoadObjects");
 QString fileName = themePath() + QString::fromLatin1("objects/objects.boson");
 if (!KStandardDirs::exists(fileName)) {
	boDebug(270) << k_funcinfo << "no objects.boson file found at " << fileName << endl;
	// We assume that this theme has no objects and don't complain
	return true;
 }

 KSimpleConfig cfg(fileName);
 QStringList objects = cfg.groupList();
 if (objects.isEmpty()) {
	boDebug(270) << k_funcinfo << "No objects found in objects file (" << fileName << ")" << endl;
	return true;
 }

 boDebug(270) << k_funcinfo << "Loading " << objects.count()
		<< " objects from config file" << endl;
 QStringList::Iterator it;
 for (it = objects.begin(); it != objects.end(); ++it) {
	boDebug(270) << k_funcinfo << "Loading object from group " << *it << endl;

	cfg.setGroup(*it);

	if (boConfig->boolValue("ForceDisableModelLoading")) {
		continue;
	}
	BosonModel* m = d->mObjectModels.find(*it);
	if (!m) {
		BosonModelFactory factory;
		m = factory.createObjectModel(&cfg, themePath());
		if (!m) {
			boError(270) << k_funcinfo  << "NULL model created" << endl;
			return false;
		}
		d->mObjectModels.insert(*it, m);
	} else {
		// nothing special to do here
	}
 }
 return true;
}

bool SpeciesData::loadActions()
{
 if (d->mActions.count() > 0) {
	// already loaded, probably for another player
	return true;
 }

 QString fileName = themePath() + QString::fromLatin1("actions.boson");
 if (!KStandardDirs::exists(fileName)) {
	boDebug(270) << k_funcinfo << "no actions.boson file found at " << fileName << endl;
	return true;
 }

 KSimpleConfig cfg(fileName);
 QStringList actions = cfg.groupList();
 if (actions.isEmpty()) {
	boWarning(270) << k_funcinfo << "No actions found in actions file (" << fileName << ")" << endl;
	return true;
 }

 boDebug(270) << k_funcinfo << "Loading " << actions.count()
		<< " actions from config file" << endl;
 QStringList::Iterator it;
 for (it = actions.begin(); it != actions.end(); ++it) {
	boDebug(270) << k_funcinfo << "Loading action from group " << *it << endl;
	BoAction* action = new BoAction(&cfg, *it, this);
	if (!d->mActions.find(action->id())) {
		d->mActions.insert(action->id(), action);
	} else {
		boError(270) << k_funcinfo << "action with id " << action->id() << " already there!" << endl;
	}
 }
 return true;
}

BoUfoImage* SpeciesData::image(const QString& name)
{
 if (!d->mImages[name]) {
	QImage img = QImage(themePath() + QString::fromLatin1("pixmaps/") + name);

	if (img.isNull()) {
		boError() << k_funcinfo << "Cannot find pixmap with name " << name << endl;
		return 0;
	}

	// AB: maybe we want to remove this in the future
	killAlphaMask(img);

	BoUfoImage* boufoImage = new BoUfoImage(img);
	d->mImages.insert(name, boufoImage);
 }
 return d->mImages[name];
}

void SpeciesData::playSound(UnitBase* unit, UnitSoundEvent event)
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

void SpeciesData::playSound(SoundEvent event)
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

void SpeciesData::playSound(const BosonWeaponProperties* weaponProp, WeaponSoundEvent event)
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

bool SpeciesData::loadGeneralSounds()
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

bool SpeciesData::loadUnitSounds(const UnitProperties* prop)
{
 BosonProfiler soundProfiling("UnitSound");
 QStringList sounds;
 QMap<int, QString> unitSounds = prop->sounds();
 QMap<int,QString>::const_iterator it = unitSounds.begin();
 for (; it != unitSounds.end(); ++it) {
	sounds.append(*it);
 }
 // Load sounds of weapons of this unit
 if (prop->canShoot()) {
	QPtrListIterator<PluginProperties> it(*(prop->plugins()));
	while (it.current()) {
		if (it.current()->pluginType() == PluginProperties::Weapon) {
			QMap<int, QString> weaponSounds = ((BosonWeaponProperties*)it.current())->sounds();
			QMap<int, QString>::const_iterator it = weaponSounds.begin();
			for (; it != weaponSounds.end(); ++it) {
				sounds.append(*it);
			}
		}
		++it;
	}
 }
 sound()->addUnitSounds(themePath(), sounds);
 return true;
}

QPtrList<BosonModel> SpeciesData::allLoadedModels() const
{
 QPtrList<BosonModel> models;
 for (QIntDictIterator<BosonModel> it(d->mUnitModels); it.current(); ++it) {
	models.append(it.current());
 }
 for (QDictIterator<BosonModel> it(d->mObjectModels); it.current(); ++it) {
	models.append(it.current());
 }
 return models;
}

QPtrList<BosonModel> SpeciesData::allLoadedModelsInAllSpecies()
{
 QPtrList<BosonModel> models;
 if (!mSpeciesData) {
	return models;
 }
 for (QDictIterator<SpeciesData> it(*mSpeciesData); it.current(); ++it) {
	QPtrList<BosonModel> list = it.current()->allLoadedModels();
	for (QPtrListIterator<BosonModel> it2(list); it2.current(); ++it2) {
		models.append(it2.current());
	}
 }
 return models;
}

