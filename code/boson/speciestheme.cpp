/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include "unitbase.h"
#include "unitproperties.h"
#include "bosonmusic.h"
#include "bosonsound.h"
#include "bosonprofiling.h"
#include "bosontexturearray.h"
#include "bosonmodel.h"

#include <kstandarddirs.h>
#include <ksimpleconfig.h>
#include <kdebug.h>

#include <qpixmap.h>
#include <qimage.h>
#include <qbitmap.h>
#include <qintdict.h>
#include <qdir.h>
#include <qgl.h>

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
	QIntDict<QPixmap> mSmallOverview;
	QIntDict<QPixmap> mBigOverview;
	QIntDict<BosonTextureArray> mSpriteTextures;
	QIntDict<BosonModel> mUnitModels;

	// TODO: the shots have no OpenGL implementation yet!

	QIntDict<QPixmap> mActionPixmaps;


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
 d->mSmallOverview.setAutoDelete(true);
 d->mBigOverview.setAutoDelete(true);
 d->mActionPixmaps.setAutoDelete(true);
 d->mSpriteTextures.setAutoDelete(true);
 d->mUnitModels.setAutoDelete(true);
 d->mCanChangeTeamColor = true;

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
 d->mSpriteTextures.clear();
 d->mUnitModels.clear();
 d->mSmallOverview.clear();
 d->mBigOverview.clear();
 d->mUnitProperties.clear();
 d->mActionPixmaps.clear();
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

 boMusic->addSounds(themePath());

 // the initial values for the units - config files :-)
 readUnitConfigs();

 // action pixmaps - it doesn't hurt
 if (!loadActionGraphics()) {
	kdError() << "Couldn't load action pixmaps" << endl;
 }

 if (!loadShot()) {
	kdError() << "Could not load shot sequence" << endl;
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

// sprites first
 QString fileName = path + "field-%1.png";
 int images = prop->isFacility() ? PIXMAP_PER_FIX : PIXMAP_PER_MOBILE;
 for(int i = 0; i < images; i++) {
	QImage image;
	QString number;
	number.sprintf("%04d", i);
	if (!loadUnitImage(fileName.arg(number), image, true, (images - 1 != i))) { // latest(destroyed) isn't team-colored
		kdError() << "Cannot load " << fileName.arg(number) << endl;
		return false;
	}
	imageList.append(image);
 }
 loadUnitTextures(prop->typeId(), imageList);
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
 BosonSound* sound = boMusic->bosonSound(themePath());
 sound->addUnitSounds(prop);
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
 d->mActionPixmaps.insert((int)ActionAttack, attack);
 if (!attack) {
	kdError() << k_funcinfo << "NULL attack pixmap!" << endl;
	return false;
 }

 QPixmap* move = new QPixmap(actionPath + "move.png");
 d->mActionPixmaps.insert((int)ActionMove, move);
 if (!move) {
	kdError() << k_funcinfo << "NULL move pixmap!" << endl;
	return false;
 }

 QPixmap* stop = new QPixmap(actionPath + "stop.png");
 d->mActionPixmaps.insert((int)ActionStop, stop);
 if (!stop) {
	kdError() << k_funcinfo << "NULL stop pixmap!" << endl;
	return false;
 }
 return true;
}

int SpeciesTheme::unitWidth(unsigned long int unitType)
{
 BosonTextureArray* array = d->mSpriteTextures[unitType];
 if (!array) {
	loadUnit(unitType);
	d->mSpriteTextures[unitType];
	if (!array) {
		kdError() << "Cannot load data files for " << unitType << endl;
		return 0;
	}
 }
 return array->width(0);
}

int SpeciesTheme::unitHeight(unsigned long int unitType)
{
 BosonTextureArray* array = d->mSpriteTextures[unitType];
 if (!array) {
	loadUnit(unitType);
	array = d->mSpriteTextures[unitType];
	if (!array) {
		kdError() << "Cannot load data files for " << unitType << endl;
		return 0;
	}
 }
 return array->height(0);
}

BosonTextureArray* SpeciesTheme::textureArray(unsigned long int unitType)
{
 BosonTextureArray* array = d->mSpriteTextures[unitType];
 if (!array) {
	loadUnit(unitType);
	array = d->mSpriteTextures[unitType];
 }
 if (!array) {
	kdError() << k_funcinfo << "Cannot find unit type " << unitType 
			<< endl;
	return 0;
 }
 return array;
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

GLuint SpeciesTheme::textureNumber(unsigned long int unitType, int dir)
{
 BosonTextureArray* array = d->mSpriteTextures[unitType];
 if (!array) {
	loadUnit(unitType);
	array = d->mSpriteTextures[unitType];
	if (!array) {
		kdError() << k_funcinfo << "Cannot find texture for " << unitType << endl;
		return 0;
	}
 }
 if (!array->isValid()) {
	kdError() << k_funcinfo << "Oops - textures not allocated for " << unitType << endl;
	return 0;
 }
 if ((int)array->count() <= dir) {
	kdError() << k_funcinfo << "Texture " << dir << " not allocated for " << unitType << endl;
	return 0;
 }
 return array->texture(dir);
}

GLuint SpeciesTheme::displayList(unsigned long int unitType)
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
 return model->displayList();
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

bool SpeciesTheme::loadShotPixmap(const QString& fileName, QPixmap& pix)
{
 QImage image(fileName);
 const QRgb backGround = qRgb(255, 0, 255) & RGB_MASK;

 if (image.isNull()) {
	kdError() << k_funcinfo << "Could not load " << fileName << endl;
	return false;
 }
 if (image.width() < 25) {
	kdError() << fileName << "width < 25" << endl;
	return false;
 }
 if (image.height() < 25) {
	kdError() << fileName << "height < 25" << endl;
	return false;
 }
 if (image.depth() != 32) {
	kdError() << fileName << "depth != 32" << endl;
 }

 QImage mask(image.width(), image.height(), 1, 2, QImage::LittleEndian);
 if (mask.isNull()) {
	kdError() << k_funcinfo << "NULL mask" << endl;
	return false;
 }
 mask.setColor(0, 0xffffff);
 mask.setColor(1, 0);
 mask.fill(0xff);

 for (int y = 0; y < image.height(); y++) {
	unsigned char* yp = mask.scanLine(y);
	QRgb* p = (QRgb*)image.scanLine(y);
	for (int x = 0; x < image.width(); x++, p++) {
		if ( (*p & 0x00fff0ff) == backGround) { // set transparent
			*(yp + (x >> 3)) &= ~(1 << (x & 7));
			continue;
		}
	}

 }
 QBitmap m;
 pix.convertFromImage(image);
 m.convertFromImage(mask);
 pix.setMask(m);
 return true;
}

void SpeciesTheme::loadNewUnit(UnitBase* unit)
{
 if (!unit) {
	kdError() << k_funcinfo << "NULL unit" << endl;
	return;
 }
 const UnitProperties* prop = unitProperties(unit);
 if (!prop) {
	kdError() << k_funcinfo << "NULL properties for " << unit->type() << endl;
	return;
 }

 unit->setHealth(prop->health());
 unit->setArmor(prop->armor());
 unit->setShields(prop->shields());
// kdDebug() << k_funcinfo << "1"<<endl;
 unit->setWeaponRange(prop->weaponRange()); // seems to cause a KGame error sometimes
// kdDebug() << k_funcinfo << "2"<<endl;
 unit->setWeaponDamage(prop->weaponDamage());
 unit->setSightRange(prop->sightRange());

 // what will we do with the name?
 // maybe soemthing like unit->setProperties(prop);? so UnitBase::name() 
 // could return UnitProperties::name()
 // but don't use unit->setName("blah") as the name is equal for all units of
 // one type and therefore much memory consumption!

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
		kdError() << "UnitType " << prop->typeId() << "already there!" 
				<< endl;
	}
 }
}

const UnitProperties* SpeciesTheme::unitProperties(UnitBase* unit) const
{
 if (!unit) {
	kdError() << k_funcinfo << "NULL unit" << endl;
	return 0;
 }
 return unitProperties(unit->type());
}

const UnitProperties* SpeciesTheme::unitProperties(unsigned long int unitType) const
{
 if (unitType <= 0) {
	kdError() << k_funcinfo << "invalid unit type " << unitType << endl;
	return 0;
 }
 if (!d->mUnitProperties[unitType]) {
	kdError() << k_lineinfo << "oops - no unit properties for " << unitType << endl;
	return 0;
 }
 return d->mUnitProperties[unitType];
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

bool SpeciesTheme::loadShot()
{
 kdWarning() << k_funcinfo << "not yet implemented for OpenGL" << endl;
 return true;
}

bool SpeciesTheme::loadBigShot(bool isFacility, unsigned int version)
{
 kdWarning() << k_funcinfo << "not yet implemented for OpenGL" << endl;
 return true;
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

void SpeciesTheme::loadUnitTextures(unsigned long int unitType, QValueList<QImage> list)
{
 if (list.isEmpty()) {
	kdError() << k_funcinfo << "empty list" << endl;
	return;
 }
 BosonTextureArray* tex = new BosonTextureArray;
 tex->createTextures(list);
 d->mSpriteTextures.insert(unitType, tex);
}

void SpeciesTheme::loadUnitModel(const UnitProperties* prop)
{
 BosonModel* m = 0;
 if (QFile::exists(prop->unitPath() + QString::fromLatin1("unit.3ds"))) {
	m = new BosonModel(prop->unitPath(), QString::fromLatin1("unit.3ds"));
	m->setLongNames(prop->longTextureNames());
	m->loadModel();
 } else {
	// this should get removed!
	BosonTextureArray* array = textureArray(prop->typeId());
	GLuint list = createDisplayList(prop->typeId());
	m = new BosonModel(list, array->width(0), array->height(0));
 }
 d->mUnitModels.insert(prop->typeId(), m);
}

GLuint SpeciesTheme::createDisplayList(unsigned long int typeId)
{
 BosonTextureArray* texArray = textureArray(typeId);
 if (!texArray) {
	kdError() << k_funcinfo << "NULL textures" << endl;
	return 0;
 }
 float width = ((float)texArray->width(0)) * BO_GL_CELL_SIZE / BO_TILE_SIZE;
 float height = ((float)texArray->height(0)) * BO_GL_CELL_SIZE / BO_TILE_SIZE;
 #warning FIXME - directions are not supported
 GLuint tex = texArray->texture(0); // this doesn't support directions!!
 if (tex == 0) {
	kdWarning() << k_funcinfo << "invalid texture" << endl;
	return 0;
 }

 GLuint list = glGenLists(1);
 glNewList(list, GL_COMPILE);
	glBindTexture(GL_TEXTURE_2D, tex); // which texture to load

	glBegin(GL_QUADS);
		glTexCoord2f(0.0,0.0); glVertex3f(0.0, 0.0, 0.0);
		glTexCoord2f(1.0,0.0); glVertex3f(width, 0.0, 0.0);
		glTexCoord2f(1.0,1.0); glVertex3f(width, height, 0.0);
		glTexCoord2f(0.0,1.0); glVertex3f(0.0, height, 0.0);

	glEnd();
 glEndList();

 return list;
}


