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

#include "defines.h"
#include "unit.h"
#include "unitproperties.h"
#include "bosonconfig.h"
#include "bosonprofiling.h"
#include "bosonmodel.h"
#include "sound/bosonmusic.h"
#include "sound/bosonsound.h"
#include "upgradeproperties.h"
#include "bosonparticlemanager.h"
#include "items/bosonshot.h"
#include "bosonweapon.h"

#include <kstandarddirs.h>
#include <ksimpleconfig.h>
#include <kdebug.h>

#include <qpixmap.h>
#include <qimage.h>
#include <qbitmap.h>
#include <qintdict.h>
#include <qdir.h>

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


class SpeciesTheme::SpeciesThemePrivate
{
public:
	SpeciesThemePrivate()
	{
	}

	QIntDict<UnitProperties> mUnitProperties;
	QIntDict<TechnologyProperties> mTechnologies;
	QIntDict<QPixmap> mSmallOverview;
	QIntDict<QPixmap> mBigOverview;
	QIntDict<BosonModel> mUnitModels;
	QDict<BosonModel> mObjectModels;

	QIntDict<QPixmap> mActionPixmaps;

	QIntDict<BosonParticleSystemProperties> mParticleProps;
	QIntDict<BosonShotProperties> mShotProps;
	QIntDict<BosonWeaponProperties> mWeaponProps;

	bool mCanChangeTeamColor;

	QMap<QString, QPixmap*> mUpgradePixmaps;
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
 d->mSmallOverview.setAutoDelete(true);
 d->mBigOverview.setAutoDelete(true);
 d->mActionPixmaps.setAutoDelete(true);
 d->mUnitModels.setAutoDelete(true);
 d->mObjectModels.setAutoDelete(true);
 d->mParticleProps.setAutoDelete(true);
 d->mShotProps.setAutoDelete(true);
 d->mWeaponProps.setAutoDelete(true);
 d->mCanChangeTeamColor = true;
 mSound = 0;

 if (!loadTheme(speciesDir, teamColor)) {
	kdError() << "Theme " << speciesDir << " not properly loaded" << endl;
 }
}

SpeciesTheme::~SpeciesTheme()
{
 kdDebug() << k_funcinfo << endl;
 reset();

 delete d;
 kdDebug() << k_funcinfo << "done" << endl;
}

void SpeciesTheme::reset()
{
 d->mUnitModels.clear();
 d->mObjectModels.clear();
 d->mSmallOverview.clear();
 d->mBigOverview.clear();
 d->mUnitProperties.clear();
 d->mTechnologies.clear();
 d->mActionPixmaps.clear();
 d->mParticleProps.clear();
 d->mShotProps.clear();
 d->mWeaponProps.clear();
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
 kdDebug() << "theme path: " << themePath() << endl;

 mSound = boMusic->addSounds(themePath());

 // the initial values for the units - config files :-)
 //readUnitConfigs();

 // action pixmaps - it doesn't hurt
 if (!loadActionGraphics()) {
	kdError() << "Couldn't load action pixmaps" << endl;
 }

 // don't preload units here as the species can still be changed in new game
 // dialog 
 return true;
}

bool SpeciesTheme::loadUnitGraphics(const UnitProperties* prop)
{
 unsigned long int type = prop->typeId();
 QValueList<QImage> imageList;
 QString path = prop->unitPath();

 loadUnitModel(prop);

// big overview
 if (d->mBigOverview[type]) {
	kdError() << "BigOverview of " << type << " already there" << endl;
 } else {
	QImage image;
	if (!loadUnitImage(path + "overview-big.png", image, false)) {
		kdError() << "SpeciesTheme : Can't load " << path + "overview-big.png" << endl;
		return false;
	}
	QPixmap* p = new QPixmap(image);
	d->mBigOverview.insert(type, p);
 }

// small overview
 if (d->mSmallOverview[type]) {
	kdError() << "SmallOverview of " << type << " already there" << endl;
 } else {
	QImage image;
	if (!loadUnitImage(path + "overview-small.png", image, false)) {
		kdError() << "SpeciesTheme : Can't load " << path + "overview-small.png" << endl;
		return false;
	}
	OverviewPixmap* p = new OverviewPixmap(image);
	p->killAlphaMask();
	d->mSmallOverview.insert(type, p);
 }

 return true;
}

bool SpeciesTheme::loadUnit(unsigned long int type)
{
 boProfiling->loadUnit();
 const UnitProperties* prop = unitProperties(type);
 if (!prop) {
	kdError() << "Could not load unit type " << type << endl;
	boProfiling->loadUnitDone(type);
	return false;
 }
 bool ret = loadUnitGraphics(prop);

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
 // Keep this code in sync with UnitAction enum in global.h!
 // TODO: make this configurable (introduce index.desktop or ui.desktop in
 //  theme path)
 QString actionPath = KGlobal::dirs()->findResourceDir("data", "boson/themes/ui/standard/attack.png");
 actionPath += "boson/themes/ui/standard/";
 kdDebug() << k_funcinfo << "action Path: " << actionPath << endl;

 QPixmap* attack = new QPixmap(actionPath + "attack.png");
 if (attack->isNull()) {
	kdError() << k_funcinfo << "NULL attack pixmap!" << endl;
	delete attack;
	return false;
 }
 d->mActionPixmaps.insert((int)ActionAttack, attack);

 QPixmap* move = new QPixmap(actionPath + "move.png");
 if (move->isNull()) {
	kdError() << k_funcinfo << "NULL move pixmap!" << endl;
	delete move;
	return false;
 }
 d->mActionPixmaps.insert((int)ActionMove, move);

 QPixmap* stop = new QPixmap(actionPath + "stop.png");
 if (stop->isNull()) {
	kdError() << k_funcinfo << "NULL stop pixmap!" << endl;
	delete stop;
	return false;
 }
 d->mActionPixmaps.insert((int)ActionStop, stop);

 QPixmap* follow = new QPixmap(actionPath + "follow.png");
 if (follow->isNull()) {
	kdError() << k_funcinfo << "NULL follow pixmap!" << endl;
	delete follow;
	return false;
 }
 d->mActionPixmaps.insert((int)ActionFollow, follow);

 QPixmap* mine = new QPixmap(actionPath + "mine.png");
 if (mine->isNull()) {
	kdError() << k_funcinfo << "NULL mine pixmap!" << endl;
	delete mine;
	return false;
 }
 d->mActionPixmaps.insert((int)ActionMine, mine);

 QPixmap* repair = new QPixmap(actionPath + "repair.png");
 if (repair->isNull()) {
	kdError() << k_funcinfo << "NULL repair pixmap!" << endl;
	delete repair;
	return false;
 }
 d->mActionPixmaps.insert((int)ActionRepair, repair);

 return true;
}

QPixmap* SpeciesTheme::techPixmap(unsigned long int techType)
{
 return upgradePixmapByName(technology(techType)->pixmapName());
}

QPixmap* SpeciesTheme::upgradePixmapByName(QString name)
{
 if(!d->mUpgradePixmaps[name]) {
	QPixmap* p = new QPixmap(themePath() + "pixmaps/" + name);
	if(p->isNull()) {
		kdError() << k_funcinfo << "Cannot find pixmap with name " << name << endl;
		// Will crash?
	}
	d->mUpgradePixmaps[name] = p;
 }
 return d->mUpgradePixmaps[name];
}

bool SpeciesTheme::loadTechnologies()
{
 QFile f(themePath() + "technologies.desktop");
 if(!f.exists()) {
	kdWarning() << k_funcinfo << "Technologies file (" << f.name() << ") does not exists. No technologies loaded" << endl;
	// We assume that this theme has no technologies and still return true
	return true;
 }
 KSimpleConfig cfg(f.name());
 QStringList techs = cfg.groupList();
 if(techs.isEmpty()) {
	kdWarning() << k_funcinfo << "No technologies found in technologies file (" << f.name() << ")" << endl;
	return true;
 }
 QStringList::Iterator it;
 for(it = techs.begin(); it != techs.end(); ++it) {
	kdDebug() << k_funcinfo << "Loading technology from group " << *it << endl;
	TechnologyProperties* tech = new TechnologyProperties;
	cfg.setGroup(*it);
	tech->load(&cfg);
	if (!d->mTechnologies.find(tech->id())) {
		d->mTechnologies.insert(tech->id(), tech);
	} else {
		kdError() << k_funcinfo << "Technology with id " << tech->id() << " already there!" << endl;
	}
 }
 return true;
}

BosonModel* SpeciesTheme::unitModel(unsigned long int unitType)
{
 BosonModel* model = d->mUnitModels[unitType];
 if (!model) {
	loadUnit(unitType);
	model = d->mUnitModels[unitType];
 }
 if (!model) {
	kdError() << k_funcinfo << "Cannot load display list for " << unitType 
			<< endl;
	return 0;
 }
 return model;
}

QPixmap* SpeciesTheme::bigOverview(unsigned long int unitType)
{
 QPixmap* pix = d->mBigOverview[unitType];
 if (!pix) {
	loadUnit(unitType);
	pix = d->mBigOverview[unitType];
 }
 if (!pix) {
	kdError() << k_funcinfo << "Cannot find unit type " << unitType 
			<< endl;
	return 0;
 }
 return pix;
}

QPixmap* SpeciesTheme::smallOverview(unsigned long int unitType)
{
 QPixmap* pix = d->mSmallOverview[unitType];
 if (!pix) {
	loadUnit(unitType);
	pix = d->mSmallOverview[unitType];
 }
 if (!pix) {
	kdError() << k_funcinfo << "Cannot find unit type " << unitType 
			<< endl;
	return 0;
 }
 return pix;
}

QPixmap* SpeciesTheme::actionPixmap(UnitAction action)
{
 // check for NULL?
 return d->mActionPixmaps[(int)action];
}

bool SpeciesTheme::loadUnitImage(const QString &fileName, QImage &_image, bool withMask, bool with_team_color)
{
 d->mCanChangeTeamColor = false;

 QImage image(fileName);
 image.setAlphaBuffer(false);
 QImage *mask = 0;
 int x, y, w, h;
 unsigned char *yp = 0;
 QRgb *p = 0;
 static const QRgb background = qRgb(255,  0, 255) & RGB_MASK ;
 static const QRgb background2 = qRgb(248, 40, 240) & RGB_MASK ;

 w = image.width();
 h = image.height();

 if (image.depth() != 32) {
	kdError() << k_funcinfo << fileName << "depth != 32" << endl;
 }
 if (w < 32) {
	kdError() << k_funcinfo << fileName << "w < 32" << endl;
	return false;
 }
 if (h < 32) {
	kdError() << k_funcinfo << fileName << "h < 32" << endl;
	return false;
 }

 if (image.isNull()) {
	kdError() << k_funcinfo << "NULL image" << endl;
	return false;
 }

 if (withMask) {
	mask = new QImage ( w, h, 1, 2, QImage::LittleEndian);
	if (mask->isNull()) {
		kdError() << k_funcinfo << "NULL mask" << endl;
		return false;
	}
	mask->setColor( 0, 0xffffff );
	mask->setColor( 1, 0 );
	mask->fill(0xff); 
 }
 if (withMask) {
	const int teamRed = teamColor().red();
	const int teamGreen = teamColor().green();
	const int teamBlue = teamColor().blue();
	for ( y = 0; y < h; y++ ) {
		yp = mask->scanLine(y);	// mask
		p  = (QRgb *)image.scanLine(y);	// image
		for ( x = 0; x < w; x++, p++ ) {
			if ( (*p & 0x00fff0ff) == background ) {// set transparent 
				*(yp + (x >> 3)) &= ~(1 << (x & 7));
				continue;
			}
			if ( (*p & 0x00f8f8f8) == background2) {// set transparent 
				*(yp + (x >> 3)) &= ~(1 << (x & 7));
				continue;
			}
			if (with_team_color) {
				if (qAlpha(*p) < 255) {
					continue;
					// alpha == 0 means "fill completely
					// with team color", alpha == 255 means
					// "don't fill with team color".

					// basically a gradient from *p to teamColor.rgb()

#if 1
					// mostly from KPixmapEffect::gradient():
					int red = qRed(*p);
					int green = qGreen(*p);
					int blue = qBlue(*p);

					int dred = -teamRed + red;
					int dgreen = -teamGreen + green;
					int dblue = -teamBlue + blue;

					int rl = teamRed << 16;
					int gl = teamGreen << 16;
					int bl = teamBlue << 16;

					int rcdelta = ((1<<16) / 255) * dred;
					int gcdelta = ((1<<16) / 255) * dgreen;
					int bcdelta = ((1<<16) / 255) * dblue;

					rl += rcdelta * qAlpha(*p);
					gl += gcdelta * qAlpha(*p);
					bl += bcdelta * qAlpha(*p);

					*p = qRgb(rl>>16, gl>>16, bl>>16);
#else
					// this is exactly the same as above -
					// but alpha==255 means "no teamcolor"
					// (as above) and alpha==254 means "full
					// team color"! necessary for testing
					// :-(
					int red = qRed(*p);
					int green = qGreen(*p);
					int blue = qBlue(*p);

					int dred = teamRed - red;
					int dgreen = teamGreen - green;
					int dblue = teamBlue - blue;

					int rl = red << 16;
					int gl = green << 16;
					int bl = blue << 16;

					int rcdelta = ((1<<16) / 255) * dred;
					int gcdelta = ((1<<16) / 255) * dgreen;
					int bcdelta = ((1<<16) / 255) * dblue;

					rl += rcdelta * qAlpha(*p);
					gl += gcdelta * qAlpha(*p);
					bl += bcdelta * qAlpha(*p);

//					*p = qRgb(rl>>16, gl>>16, bl>>16);
					*p = qRgba(rl>>16, gl>>16, bl>>16, 0);

#endif
				} else if ( ((qRed(*p) > 0x80) &&
						(qGreen(*p) < 0x70) &&
						(qBlue(*p) < 0x70))) {
//					continue; // FIXME: this should not be used.. but somehow I have problems with the above, currently :(
					*p = teamColor().rgb();
				}
			}
		}
	}
 } else {
	for ( y = 0; y < h; y++ ) {
		p  = (QRgb *)image.scanLine(y);	// image
		for ( x = 0; x < w; x++, p++ ) {
			if ( (qRed(*p) > 0x90) && (qGreen(*p) < 0x60) && (qBlue(*p) < 0x60)) {
				*p = teamColor().rgb();
			}
		}
	}
 }

 if (image.isNull() || w < 32 || h < 32)  {
	kdError() << k_funcinfo << "image is null" << endl;
	return false;
 }

 // FIXME: I have a few problems with our mask..
 // there is no QImage::setAlphaBuffer(QImage) or so :-(
 QPixmap pix;
 pix.convertFromImage(image);

 if (withMask) {
	QBitmap m;
	m.convertFromImage(*mask);
	pix.setMask( m ); 
 }
 delete mask;
 _image = pix.convertToImage();
 return true;
}

void SpeciesTheme::loadNewUnit(UnitBase* unit)
{
 if (!unit) {
	kdError() << k_funcinfo << "NULL unit" << endl;
	return;
 }
 const UnitProperties* prop = unit->unitProperties(); //unitProperties(unit);
 if (!prop) {
	kdError() << k_funcinfo << "NULL properties for " << unit->type() << endl;
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

void SpeciesTheme::readUnitConfigs()
{
 if (d->mUnitProperties.count() != 0) {
	kdError() << "Cannot read unit configs again. Returning untouched..."
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
			QString::fromLatin1("/index.desktop");
	if (QFile::exists(file)) {
		list.append(file);
	}
 }

 if (list.isEmpty()) {
	kdError() << "No Units found in this theme" << endl;
	return;
 }
 for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it) {
	UnitProperties* prop = new UnitProperties(this, *it);
	if (!d->mUnitProperties.find(prop->typeId())) {
		d->mUnitProperties.insert(prop->typeId(), prop);
	} else {
		kdError() << "UnitType " << prop->typeId() << " already there!"
				<< endl;
	}
 }
}

const UnitProperties* SpeciesTheme::unitProperties(unsigned long int unitType) const
{
 if (unitType == 0) {
	kdError() << k_funcinfo << "invalid unit type " << unitType << endl;
	return 0;
 }
 if (!d->mUnitProperties[unitType]) {
	kdError() << k_lineinfo << "oops - no unit properties for " << unitType << endl;
	return 0;
 }
 return d->mUnitProperties[unitType];
}

UnitProperties* SpeciesTheme::nonConstUnitProperties(unsigned long int unitType) const
{
 if (unitType == 0) {
	kdError() << k_funcinfo << "invalid unit type " << unitType << endl;
	return 0;
 }
 if (!d->mUnitProperties[unitType]) {
	kdError() << k_lineinfo << "oops - no unit properties for " << unitType << endl;
	return 0;
 }
 return d->mUnitProperties[unitType];
}

TechnologyProperties* SpeciesTheme::technology(unsigned long int techType) const
{
 if (techType == 0) {
	kdError() << k_funcinfo << "invalid technology type " << techType << endl;
	return 0;
 }
 if (!d->mTechnologies[techType]) {
	kdError() << k_lineinfo << "oops - no technology properties for " << techType << endl;
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

QValueList<unsigned long int> SpeciesTheme::productions(QValueList<int> producers) const
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

QValueList<unsigned long int> SpeciesTheme::technologies(QValueList<int> producers) const
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
		"boson/themes/species/*/index.desktop");
 if (list.isEmpty()) {
	kdWarning() << "No species found!" << endl;
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
		QString d = l[i].left(l[i].length() - strlen("index.desktop"));
		return d;
	}
 }
 return QString::null;
}

QString SpeciesTheme::identifier() const
{
 KSimpleConfig cfg(themePath() + QString::fromLatin1("index.desktop"));
 cfg.setGroup("Boson Species");
 return cfg.readEntry("Identifier");
}

bool SpeciesTheme::setTeamColor(const QColor& color)
{
 if (!d->mCanChangeTeamColor) {
	kdWarning() << "Cannot change team color anymore!" << endl;
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
 if (d->mUnitModels[prop->typeId()]) {
	kdWarning() << k_funcinfo << "Model already loaded" << endl;
	return;
 }
 if (!QFile::exists(prop->unitPath() + QString::fromLatin1("unit.3ds"))) {
	kdError() << k_funcinfo << "Cannot find unit.3ds file for " << prop->typeId() << endl;
	return;
 }
 BosonModel* m = new BosonModel(prop->unitPath(), QString::fromLatin1("unit.3ds"),
		((float)prop->unitWidth()) * BO_GL_CELL_SIZE / BO_TILE_SIZE,
		((float)prop->unitHeight()) * BO_GL_CELL_SIZE / BO_TILE_SIZE);
 m->setLongNames(prop->longTextureNames());
 m->setTeamColor(teamColor());
 m->loadModel();
 if (prop->isFacility()) {
	m->generateConstructionLists();
 }

 // now we load animation information. this is just which frame is used for
 // which animation mode - no frame/node/display list modifying needs to be
 // made.
 KSimpleConfig cfg(prop->unitPath() + QString::fromLatin1("index.desktop"));
 cfg.setGroup("OpenGL");
 m->loadAnimationMode(Unit::AnimationIdle, &cfg, QString::fromLatin1("Idle"));
 m->loadAnimationMode(Unit::AnimationWreckage, &cfg, QString::fromLatin1("Wreckage"));


 m->finishLoading();
 d->mUnitModels.insert(prop->typeId(), m);
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

BosonModel* SpeciesTheme::objectModel(QString file)
{
 if(!d->mObjectModels[file]) {
	// Model isn't loaded yet. Load it now
	BosonModel* m = new BosonModel(themePath() + "/objects/", file, 0.3, 0.3);
	m->setTeamColor(teamColor());
	m->loadModel();
	d->mObjectModels.insert(file, m);
 }
 return d->mObjectModels[file];
}

void SpeciesTheme::loadParticleSystems()
{
 BosonParticleSystemProperties::init(themePath() + "/particles");
 QFile f(themePath() + "particles/particles.boson");
 if(!f.exists()) {
	kdWarning() << k_funcinfo << "Particle systems file (" << f.name() << ") does not exists. No particle systems loaded!" << endl;
	// We assume that this theme has no particles and still return true
	return;
 }
 KSimpleConfig cfg(f.name());
 QStringList particles = cfg.groupList();
 if(particles.isEmpty()) {
	kdWarning() << k_funcinfo << "No particle systems found in particles file (" << f.name() << ")" << endl;
	return;
 }
 kdDebug() << k_funcinfo << "Loading " << particles.count() << " particle systems from config file" << endl;
 QStringList::Iterator it;
 for(it = particles.begin(); it != particles.end(); ++it) {
	kdDebug() << k_funcinfo << "Loading particle system from group " << *it << endl;
	cfg.setGroup(*it);
	BosonParticleSystemProperties* particleprop = new BosonParticleSystemProperties(&cfg);
	if (!d->mParticleProps.find(particleprop->id())) {
		d->mParticleProps.insert(particleprop->id(), particleprop);
	} else {
		kdError() << k_funcinfo << "particle system with id " << particleprop->id() << " already there!" << endl;
	}
 }
 return;
}

void SpeciesTheme::loadShots()
{
 QFile f(themePath() + "shots.boson");
 if(!f.exists()) {
	kdWarning() << k_funcinfo << "Shots file (" << f.name() << ") does not exists. No shots loaded!" << endl;
	// We assume that this theme has no shots and still return true
	return;
 }
 KSimpleConfig cfg(f.name());
 QStringList shots = cfg.groupList();
 if(shots.isEmpty()) {
	kdWarning() << k_funcinfo << "No shots found in shots file (" << f.name() << ")" << endl;
	return;
 }
 kdDebug() << k_funcinfo << "Loading " << shots.count() << " shots from config file" << endl;
 QStringList::Iterator it;
 for(it = shots.begin(); it != shots.end(); ++it) {
	kdDebug() << k_funcinfo << "Loading shot from group " << *it << endl;
	cfg.setGroup(*it);
	BosonShotProperties* shotprop = new BosonShotProperties(this, &cfg);
	if (!d->mShotProps.find(shotprop->id())) {
		d->mShotProps.insert(shotprop->id(), shotprop);
	} else {
		kdError() << k_funcinfo << "shot with id " << shotprop->id() << " already there!" << endl;
	}
 }
 return;
}

void SpeciesTheme::loadWeapons()
{
 QFile f(themePath() + "weapons.boson");
 if(!f.exists()) {
	kdWarning() << k_funcinfo << "Weapons file (" << f.name() << ") does not exists. No weapons loaded!" << endl;
	// We assume that this theme has no weapons and still return true
	return;
 }
 KSimpleConfig cfg(f.name());
 QStringList weapons = cfg.groupList();
 if(weapons.isEmpty()) {
	kdWarning() << k_funcinfo << "No weapons found in weapons file (" << f.name() << ")" << endl;
	return;
 }
 kdDebug() << k_funcinfo << "Loading " << weapons.count() << " weapons from config file" << endl;
 QStringList::Iterator it;
 for(it = weapons.begin(); it != weapons.end(); ++it) {
	kdDebug() << k_funcinfo << "Loading weapon from group " << *it << endl;
	cfg.setGroup(*it);
	BosonWeaponProperties* weaponprop = new BosonWeaponProperties(&cfg, this);
	if (!d->mWeaponProps.find(weaponprop->id())) {
		d->mWeaponProps.insert(weaponprop->id(), weaponprop);
	} else {
		kdError() << k_funcinfo << "weapon with id " << weaponprop->id() << " already there!" << endl;
	}
 }
 return;
}

BosonParticleSystemProperties* SpeciesTheme::particleSystemProperties(unsigned long int id)
{
 if (id == 0) {
	// We don't print error here because 0 means "none" in configurations
	return 0;
 }
 if (!d->mParticleProps[id]) {
	kdError() << k_funcinfo << "oops - no particle system properties for " << id << endl;
	return 0;
 }
 return d->mParticleProps[id];
}

BosonShotProperties* SpeciesTheme::shotProperties(unsigned long int id)
{
 if (id == 0) {
	kdError() << k_funcinfo << "Invalid shot id!" << endl;
	return 0;
 }
 if (!d->mShotProps[id]) {
	kdError() << k_funcinfo << "oops - no shot properties for " << id << endl;
	return 0;
 }
 return d->mShotProps[id];
}

BosonWeaponProperties* SpeciesTheme::weaponProperties(unsigned long int id)
{
 if (id == 0) {
	kdError() << k_funcinfo << "Invalid weapon id!" << endl;
	return 0;
 }
 if (!d->mWeaponProps[id]) {
	kdError() << k_funcinfo << "oops - no weapon properties for " << id << endl;
	return 0;
 }
 return d->mWeaponProps[id];
}
