/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include "unit.h" // FIXME this is for animation ids only

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
	QIntDict<QPixmap> mActionPixmaps;
	QIntDict<TeamColorData> mTeamData;
	QIntDict<BosonParticleSystemProperties> mParticleProperties;
	QDict<QPixmap> mUpgradePixmaps;
};

class SpeciesData::TeamColorData
{
public:
	TeamColorData()
	{
		mUnitModels.setAutoDelete(true);
		mSmallOverview.setAutoDelete(true);
		mBigOverview.setAutoDelete(true);
		mObjectModels.setAutoDelete(true);
	}
	~TeamColorData()
	{
		mUnitModels.clear();
		mSmallOverview.clear();
		mBigOverview.clear();
		mObjectModels.clear();
	}

	QIntDict<BosonModel> mUnitModels;
	QIntDict<QPixmap> mSmallOverview;
	QIntDict<QPixmap> mBigOverview;
	QDict<BosonModel> mObjectModels;
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
 d->mActionPixmaps.setAutoDelete(true);
 d->mParticleProperties.setAutoDelete(true);
 d->mTeamData.setAutoDelete(true);
 d->mThemePath = speciesPath;
}

SpeciesData::~SpeciesData()
{
 d->mActionPixmaps.clear();
 d->mParticleProperties.clear();
 d->mTeamData.clear();
 delete d;
}

void SpeciesData::initStatic()
{
 if (mSpeciesData) {
	return;
 }
 mSpeciesData = new QDict<SpeciesData>();
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

bool SpeciesData::loadActionPixmaps()
{
 if (d->mActionPixmaps.count() > 0) {
	return true;
 }
 // Keep this code in sync with UnitAction enum in global.h!
 // TODO: make this configurable (introduce index.boson or ui.boson in
 // theme path)
 QString actionPath = KGlobal::dirs()->findResourceDir("data", "boson/themes/ui/standard/attack.png");
 actionPath += "boson/themes/ui/standard/";
 boDebug() << k_funcinfo << "action Path: " << actionPath << endl;
 QPixmap* attack = new QPixmap(actionPath + "attack.png");
 if (attack->isNull()) {
	boError() << k_funcinfo << "NULL attack pixmap!" << endl;
	delete attack;
	return false;
 }
 d->mActionPixmaps.insert((int)ActionAttack, attack);

 QPixmap* move = new QPixmap(actionPath + "move.png");
 if (move->isNull()) {
	boError() << k_funcinfo << "NULL move pixmap!" << endl;
	delete move;
	return false;
 }
 d->mActionPixmaps.insert((int)ActionMove, move);

 QPixmap* stop = new QPixmap(actionPath + "stop.png");
 if (stop->isNull()) {
	boError() << k_funcinfo << "NULL stop pixmap!" << endl;
	delete stop;
	return false;
 }
 d->mActionPixmaps.insert((int)ActionStop, stop);

 QPixmap* follow = new QPixmap(actionPath + "follow.png");
 if (follow->isNull()) {
	boError() << k_funcinfo << "NULL follow pixmap!" << endl;
	delete follow;
	return false;
 }
 d->mActionPixmaps.insert((int)ActionFollow, follow);

 QPixmap* mine = new QPixmap(actionPath + "mine.png");
 if (mine->isNull()) {
	boError() << k_funcinfo << "NULL mine pixmap!" << endl;
	delete mine;
	return false;
 }
 d->mActionPixmaps.insert((int)ActionMine, mine);

 QPixmap* repair = new QPixmap(actionPath + "repair.png");
 if (repair->isNull()) {
	boError() << k_funcinfo << "NULL repair pixmap!" << endl;
	delete repair;
	return false;
 }
 d->mActionPixmaps.insert((int)ActionRepair, repair);
 return true;
}

QPixmap* SpeciesData::actionPixmap(UnitAction action) const
{
 return d->mActionPixmaps[action];
}

void SpeciesData::loadUnitModel(const UnitProperties* prop, const QColor& color)
{
 TeamColorData* data = teamColorData(color);
 if (!data) {
	boError() << k_funcinfo << "NULL teamcolor data" << endl;
	return;
 }
 if (data->mUnitModels[prop->typeId()]) {
	boWarning() << k_funcinfo << "Model already loaded" << endl;
	return;
 }
 if (!QFile::exists(prop->unitPath() + unitModelFile())) {
	boError() << k_funcinfo << "Cannot find " << unitModelFile() << " file for " << prop->typeId() << endl;
	return;
 }
 BosonModel* m = new BosonModel(prop->unitPath(), unitModelFile(),
		((float)prop->unitWidth()) * BO_GL_CELL_SIZE / BO_TILE_SIZE,
		((float)prop->unitHeight()) * BO_GL_CELL_SIZE / BO_TILE_SIZE);
 m->setLongNames(prop->longTextureNames());
 m->setTeamColor(color);
 m->loadModel();
 if (prop->isFacility()) {
	m->generateConstructionLists();
 }

 // now we load animation information. this is just which frame is used for
 // which animation mode - no frame/node/display list modifying needs to be
 // made.
 KSimpleConfig cfg(prop->unitPath() + QString::fromLatin1("index.unit"));
 cfg.setGroup("OpenGL");
 m->loadAnimationMode(Unit::AnimationIdle, &cfg, QString::fromLatin1("Idle"));
 m->loadAnimationMode(Unit::AnimationWreckage, &cfg, QString::fromLatin1("Wreckage"));


 m->finishLoading();
 data->mUnitModels.insert(prop->typeId(), m);
}

BosonModel* SpeciesData::unitModel(unsigned long int unitType, const QColor& teamColor) const
{
 TeamColorData* data = teamColorData(teamColor);
 if (!data) {
	boError() << k_funcinfo << "NULL teamcolor data" << endl;
	return 0;
 }
 return data->mUnitModels[unitType];
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
 TeamColorData* data = d->mTeamData[color.rgb()];
 delete data;
 d->mTeamData.remove(color.rgb());
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
		return false;
	}
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
		return false;
	}
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
	return false;
 }
 return data->mSmallOverview[unitType];
}


void SpeciesData::loadParticleSystemProperties()
{
 if (d->mParticleProperties.count() > 0) {
	// already loaded, probably for another player
	return;
 }
 BosonParticleSystemProperties::init(themePath() + "/particles");
 QFile f(themePath() + "particles/particles.boson");
 if (!f.exists()) {
	boDebug() << k_funcinfo << "no particles.boson file found" << endl;
	// We assume that this theme has no particles and don't complain
	return;
 }
 KSimpleConfig cfg(f.name());
 QStringList particles = cfg.groupList();
 if (particles.isEmpty()) {
	boWarning() << k_funcinfo << "No particle systems found in particles file (" << f.name() << ")" << endl;
	return;
 }
 boDebug() << k_funcinfo << "Loading " << particles.count() << " particle systems from config file" << endl;
 QStringList::Iterator it;
 for (it = particles.begin(); it != particles.end(); ++it) {
	boDebug() << k_funcinfo << "Loading particle system from group " << *it << endl;
	cfg.setGroup(*it);
	BosonParticleSystemProperties* particleprop = new BosonParticleSystemProperties(&cfg);
	if (!d->mParticleProperties.find(particleprop->id())) {
		d->mParticleProperties.insert(particleprop->id(), particleprop);
	} else {
		boError() << k_funcinfo << "particle system with id " << particleprop->id() << " already there!" << endl;
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

QPixmap* SpeciesData::upgradePixmapByName(const QString& name)
{
 if (!d->mUpgradePixmaps[name]) {
	QPixmap* p = new QPixmap(themePath() + QString::fromLatin1("pixmaps/") + name);
	if (p->isNull()) {
		boError() << k_funcinfo << "Cannot find pixmap with name " << name << endl;
		// Will crash?
	}
	d->mUpgradePixmaps.insert(name, p);
 }
 return d->mUpgradePixmaps[name];
}

BosonModel* SpeciesData::objectModel(const QString& file, const QColor& color)
{
 TeamColorData* data = teamColorData(color);
 if (!data) {
	boError() << k_funcinfo << "NULL teamcolor data" << endl;
	return 0;
 }
 if (!data->mObjectModels[file]) {
	// Model isn't loaded yet. Load it now
	BosonModel* m = new BosonModel(themePath() + QString::fromLatin1("/objects/"), file, 0.4, 0.4);
	m->setTeamColor(color);
	m->loadModel();
	data->mObjectModels.insert(file, m);
 }
 return data->mObjectModels[file];
}

