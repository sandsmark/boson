/*
    This file is part of the Boson game
    Copyright (C) 2002-2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "unitproperties.h"
#include "bosonmodel.h"
#include "bosonparticlesystemproperties.h"
#include "bodebug.h"
#include "boaction.h"

#include <qintdict.h>
#include <qdict.h>
#include <qpixmap.h>
#include <qfile.h>
#include <qimage.h>

#include <kstaticdeleter.h>
#include <kstandarddirs.h>
#include <ksimpleconfig.h>

static KStaticDeleter< QDict<SpeciesData> > sd;
QDict<SpeciesData>* SpeciesData::mSpeciesData = 0;

class BosonModelFactory
{
public:
	BosonModelFactory() { }
	~BosonModelFactory() { }

	BosonModel* createUnitModel(const UnitProperties* prop);
	BosonModel* createObjectModel(const KSimpleConfig* config, const QString& themePath);
};

BosonModel* BosonModelFactory::createUnitModel(const UnitProperties* prop)
{
 BosonModel* m = new BosonModel(prop->unitPath(), SpeciesData::unitModelFile(),
		((float)prop->unitWidth()) * BO_GL_CELL_SIZE / BO_TILE_SIZE,
		((float)prop->unitHeight()) * BO_GL_CELL_SIZE / BO_TILE_SIZE);
 m->setLongNames(prop->longTextureNames());
 m->loadModel();
 if (prop->isFacility()) {
	m->generateConstructionFrames();
 }


 // now we load animation information. this is just which frame is used for
 // which animation mode - no frame/node/display list modifying needs to be
 // made.
 KSimpleConfig cfg(prop->unitPath() + QString::fromLatin1("index.unit"));
 cfg.setGroup("OpenGL");
 m->loadAnimationMode(UnitAnimationIdle, &cfg, QString::fromLatin1("Idle"));
 m->loadAnimationMode(UnitAnimationWreckage, &cfg, QString::fromLatin1("Wreckage"));


 m->finishLoading();

 return m;
}

BosonModel* BosonModelFactory::createObjectModel(const KSimpleConfig* config, const QString& themePath)
{
 float width = (float)config->readDoubleNumEntry(QString::fromLatin1("Width"), 1.0f);
 float height = (float)config->readDoubleNumEntry(QString::fromLatin1("Height"), 1.0f);
 QString file = config->readEntry(QString::fromLatin1("File"), QString::fromLatin1("missile.3ds"));


 BosonModel* m = new BosonModel(themePath + QString::fromLatin1("/objects/"), file, width, height);
 m->loadModel();

 m->finishLoading();

 return m;
}


/**
 * By any reason QPixmap uses the alpha mask if existing, even if a custom 
 * mask using setMask() is supplied. We use this hack to delete the alpha mask 
 * if existing, so we can use our custom mask in 
 * BosonCommandWidget::advanceProduction()
 **/
class OverviewPixmap : public QPixmap
{
public:
	OverviewPixmap() : QPixmap() {}
	OverviewPixmap(const QImage& image) : QPixmap(image) {}
	void killAlphaMask() 
	{
		delete data->alphapm;
		data->alphapm = 0;
	}
};


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
	QIntDict<BosonParticleSystemProperties> mParticleProperties;
	QDict<BoAction> mActions;
	QDict<QPixmap> mPixmaps;

	QIntDict<BosonModel> mUnitModels;
	QDict<BosonModel> mObjectModels;
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

	QIntDict<QPixmap> mSmallOverview;
	QIntDict<QPixmap> mBigOverview;
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
 d->mParticleProperties.setAutoDelete(true);
 d->mTeamData.setAutoDelete(true);
 d->mUnitModels.setAutoDelete(true);
 d->mObjectModels.setAutoDelete(true);
 d->mActions.setAutoDelete(true);
 d->mPixmaps.setAutoDelete(true);
 d->mThemePath = speciesPath;
}

SpeciesData::~SpeciesData()
{
 boDebug() << k_funcinfo << endl;
 d->mParticleProperties.clear();
 d->mTeamData.clear();
 d->mUnitModels.clear();
 d->mObjectModels.clear();
 d->mActions.clear();
 d->mPixmaps.clear();
 delete d;
 boDebug() << k_funcinfo << "done" << endl;
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
 boDebug() << k_funcinfo << speciesDir << endl;
 SpeciesData* species = mSpeciesData->find(speciesDir);
 if (!species) {
	species = new SpeciesData(speciesDir);
	mSpeciesData->insert(speciesDir, species);
 }
 return species;
}

QString SpeciesData::themePath() const
{
 return d->mThemePath;
}

void SpeciesData::loadUnitModel(const UnitProperties* prop, const QColor& teamColor)
{
 BO_CHECK_NULL_RET(prop);
 QString fileName = prop->unitPath() + unitModelFile();
 if (!KStandardDirs::exists(fileName)) {
	boError() << k_funcinfo << "Cannot find " << unitModelFile() << " file for " << prop->typeId() << endl;
	return;
 }
 BosonModel* m = d->mUnitModels[prop->typeId()];

 if (!m) {
	BosonModelFactory factory;
	m = factory.createUnitModel(prop);
	d->mUnitModels.insert(prop->typeId(), m);
 } else {
//	boDebug() << "model already loaded - loading an additional teamcolor only..." << endl;
	// we only need to load the display lists here, as everything else
	// doesn't depend on the teamcolor :)
 }
 m->createDisplayLists(&teamColor);
}

BosonModel* SpeciesData::unitModel(unsigned long int unitType) const
{
 return d->mUnitModels[unitType];
}

QString SpeciesData::unitModelFile()
{
 return QString::fromLatin1("unit.3ds");
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
	boError() << k_funcinfo << "teamcolor " << color.rgb() << " already present" << endl;
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
 TeamColorData* data = teamColorData(teamColor);
 if (!data) {
	boError() << k_funcinfo << "NULL teamcolor data" << endl;
	return false;
 }
 unsigned long int type = prop->typeId();
 QString path = prop->unitPath();

// big overview
 if (data->mBigOverview[type]) {
	boError() << k_funcinfo << "BigOverview of " << type << " already there" << endl;
 } else {
	QImage image;
	if (!loadUnitImage(teamColor, path + "overview-big.png", image)) {
		boError() << k_funcinfo << "Can't load " << path + "overview-big.png" << endl;
		image = QImage(100, 100, 32);
		image.fill(Qt::red.rgb());
	}
	// we use 100x100 images for big overviews.
	image = image.smoothScale(100, 100, QImage::ScaleMin);
	QPixmap* p = new QPixmap(image);
	data->mBigOverview.insert(type, p);
 }

// small overview
 if (data->mSmallOverview[type]) {
	boError() << k_funcinfo << "SmallOverview of " << type << " already there" << endl;
 } else {
	QImage image;
	if (!loadUnitImage(teamColor, path + "overview-small.png", image)) {
		boError() << k_funcinfo << "Can't load " << path + "overview-small.png" << endl;
		image = QImage(50, 50, 32);
		image.fill(Qt::red.rgb());
	}
	image = image.smoothScale(50, 50, QImage::ScaleMin);
	OverviewPixmap* p = new OverviewPixmap(image);
	p->killAlphaMask();
	data->mSmallOverview.insert(type, p);
 }

 return true;
}


bool SpeciesData::loadUnitImage(const QColor& teamColor, const QString &fileName, QImage &_image)
{
 QImage image(fileName);
 image.setAlphaBuffer(false);
 int x, y, w, h;
 QRgb *p = 0;
 static const QRgb background = qRgb(255,  0, 255) & RGB_MASK ;
 static const QRgb background2 = qRgb(248, 40, 240) & RGB_MASK ;

 w = image.width();
 h = image.height();

 if (image.depth() != 32) {
	boError() << k_funcinfo << fileName << "depth != 32" << endl;
 }
 if (w < 32) {
	boError() << k_funcinfo << fileName << "w < 32" << endl;
	return false;
 }
 if (h < 32) {
	boError() << k_funcinfo << fileName << "h < 32" << endl;
	return false;
 }

 if (image.isNull()) {
	boError() << k_funcinfo << "NULL image" << endl;
	return false;
 }

 for ( y = 0; y < h; y++ ) {
	p  = (QRgb *)image.scanLine(y);	// image
	for ( x = 0; x < w; x++, p++ ) {
		if ( (qRed(*p) > 0x90) && (qGreen(*p) < 0x60) && (qBlue(*p) < 0x60)) {
			*p = teamColor.rgb();
		}
	}
 }

 if (image.isNull() || w < 32 || h < 32)  {
	boError() << k_funcinfo << "image is null" << endl;
	return false;
 }

 /*
 QPixmap pix;
 pix.convertFromImage(image);

 if (withMask) {
	QBitmap m;
	m.convertFromImage(*mask);
	pix.setMask( m ); 
 }
 delete mask;
 _image = pix.convertToImage();
 */
 _image = image;
 return true;
}

QPixmap* SpeciesData::bigOverview(unsigned long int unitType, const QColor& teamColor) const
{
 TeamColorData* data = teamColorData(teamColor);
 if (!data) {
	boError() << k_funcinfo << "NULL teamcolor data" << endl;
	return 0;
 }
 return data->mBigOverview[unitType];
}

QPixmap* SpeciesData::smallOverview(unsigned long int unitType, const QColor& teamColor) const
{
 TeamColorData* data = teamColorData(teamColor);
 if (!data) {
	boError() << k_funcinfo << "NULL teamcolor data" << endl;
	return 0;
 }
 return data->mSmallOverview[unitType];
}


void SpeciesData::loadParticleSystemProperties()
{
 if (d->mParticleProperties.count() > 0) {
	// already loaded, probably for another player
	return;
 }
 BosonParticleSystemProperties::initStatic(themePath() + "/particles");
 QString fileName = themePath() + QString::fromLatin1("particles/particles.boson");
 if (!KStandardDirs::exists(fileName)) {
	boDebug() << k_funcinfo << "no particles.boson file found" << endl;
	// We assume that this theme has no particles and don't complain
	return;
 }
 KSimpleConfig cfg(fileName);
 QStringList particles = cfg.groupList();
 if (particles.isEmpty()) {
	boWarning() << k_funcinfo << "No particle systems found in particles "
			<< "file (" << fileName << ")" << endl;
	return;
 }
 boDebug(150) << k_funcinfo << "Loading " << particles.count()
		<< " particle systems from config file" << endl;
 QStringList::Iterator it;
 for (it = particles.begin(); it != particles.end(); ++it) {
	boDebug(150) << k_funcinfo << "Loading particle system from group " << *it << endl;
	BosonParticleSystemProperties* particleprop = new BosonParticleSystemProperties(&cfg, *it);
	if (!d->mParticleProperties.find(particleprop->id())) {
		d->mParticleProperties.insert(particleprop->id(), particleprop);
	} else {
		boError(150) << k_funcinfo << "particle system with id " << particleprop->id() << " already there!" << endl;
	}
 }
}

const BosonParticleSystemProperties* SpeciesData::particleSystemProperties(unsigned long int id) const
{
 if (id == 0) {
	// We don't print error here because 0 means "none" in configurations
	return 0;
 }
 if (!d->mParticleProperties[id]) {
	boError() << k_funcinfo << "oops - no particle system properties for " << id << endl;
	return 0;
 }
 return d->mParticleProperties[id];
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
 if (!m) {
	boError() << k_funcinfo << "No object with name " << name << endl;
	return 0;
 }
 return m;
}

void SpeciesData::loadObjects(const QColor& teamColor)
{
 QString fileName = themePath() + QString::fromLatin1("objects/objects.boson");
 if (!KStandardDirs::exists(fileName)) {
	boDebug() << k_funcinfo << "no objects.boson file found at " << fileName << endl;
	// We assume that this theme has no objects and don't complain
	return;
 }

 KSimpleConfig cfg(fileName);
 QStringList objects = cfg.groupList();
 if (objects.isEmpty()) {
	boWarning() << k_funcinfo << "No objects found in objects file (" << fileName << ")" << endl;
	return;
 }

 boDebug() << k_funcinfo << "Loading " << objects.count()
		<< " objects from config file" << endl;
 QStringList::Iterator it;
 for (it = objects.begin(); it != objects.end(); ++it) {
	boDebug() << k_funcinfo << "Loading object from group " << *it << endl;

	cfg.setGroup(*it);

	BosonModel* m = d->mObjectModels.find(*it);
	if (!m) {
		BosonModelFactory factory;
		m = factory.createObjectModel(&cfg, themePath());
		d->mObjectModels.insert(*it, m);
	} else {
		// nothing special to do here - just load the additional display
		// lists for the new teamcolor.
	}
	m->createDisplayLists(&teamColor);
 }
}

void SpeciesData::loadActions()
{
 if (d->mActions.count() > 0) {
	// already loaded, probably for another player
	return;
 }

 QString fileName = themePath() + QString::fromLatin1("actions.boson");
 if (!KStandardDirs::exists(fileName)) {
	boDebug() << k_funcinfo << "no actions.boson file found at " << fileName << endl;
	return;
 }

 KSimpleConfig cfg(fileName);
 QStringList actions = cfg.groupList();
 if (actions.isEmpty()) {
	boWarning() << k_funcinfo << "No actions found in objects file (" << fileName << ")" << endl;
	return;
 }

 boDebug() << k_funcinfo << "Loading " << actions.count()
		<< " actions from config file" << endl;
 QStringList::Iterator it;
 for (it = actions.begin(); it != actions.end(); ++it) {
	boDebug() << k_funcinfo << "Loading action from group " << *it << endl;
	BoAction* action = new BoAction(&cfg, *it, this);
	if (!d->mActions.find(action->id())) {
		d->mActions.insert(action->id(), action);
	} else {
		boError() << k_funcinfo << "action with id " << action->id() << " already there!" << endl;
	}
 }
}

QPixmap* SpeciesData::pixmap(const QString& name)
{
 if (!d->mPixmaps[name]) {
	QPixmap* p = new QPixmap(themePath() + QString::fromLatin1("pixmaps/") + name);
	if (p->isNull()) {
		boError() << k_funcinfo << "Cannot find pixmap with name " << name << endl;
		return 0;
	}
	d->mPixmaps.insert(name, p);
 }
 return d->mPixmaps[name];
}

