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
#include "unitbase.h"
#include "unitproperties.h"
#include "bosonmusic.h"
#include "bosonsound.h"

#include "defines.h"

#include <kstandarddirs.h>
#include <ksimpleconfig.h>
#include <kdebug.h>

#include <qcanvas.h>
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
	void killAlphaMask() 
	{
		if (data->alphapm) {
			delete data->alphapm;
			data->alphapm = 0;
		}
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
	QIntDict<QCanvasPixmapArray> mSprite;

	QIntDict<QCanvasPixmapArray> mFacilityBigShot;
	QIntDict<QCanvasPixmapArray> mMobileBigShot;

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
 d->mFacilityBigShot.setAutoDelete(true);
 d->mMobileBigShot.setAutoDelete(true);
 d->mSmallOverview.setAutoDelete(true);
 d->mBigOverview.setAutoDelete(true);
 d->mSprite.setAutoDelete(true);
 d->mCanChangeTeamColor = true;
 mShot = 0;
 
 if (!loadTheme(speciesDir, teamColor)) {
	kdError() << "Theme " << speciesDir << " not properly loaded" << endl;
 }
}

SpeciesTheme::~SpeciesTheme()
{
 reset();

 delete d;
}

void SpeciesTheme::reset()
{
 d->mSprite.clear();
 d->mSmallOverview.clear();
 d->mBigOverview.clear();
 d->mUnitProperties.clear();
 d->mFacilityBigShot.clear();
 d->mMobileBigShot.clear();
 if (mShot) {
	delete mShot;
	mShot = 0;
 }
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

 if (!loadShot()) {
	kdError() << "Could not load shot sequence" << endl;
 }

 // don't preload units here as the species can still be changed in new game
 // dialog
 return true;
}

bool SpeciesTheme::loadUnit(int type)
{
 const UnitProperties* prop = unitProperties(type);
 if (!prop) {
	kdError() << "Could not load unit type " << type << endl;
	return false;
 }
 QValueList<QPixmap> pixmapList;
 QString path = prop->unitPath();

// sprites first
 QString fileName = path + "field-%1.png";
 int pixmaps = prop->isFacility() ? PIXMAP_PER_FIX : PIXMAP_PER_MOBILE;
 for(int i = 0; i < pixmaps; i++) {
	QPixmap p; // created by loadUnitPixmap()
	QString number;
	number.sprintf("%04d", i);
	if (!loadUnitPixmap(fileName.arg(number), p, true, (pixmaps - 1 != i))) { // latest(destroyed) isn't team-colored
		kdError() << "Cannot load " << fileName.arg(number) << endl;
		return false;
	}
	pixmapList.push_back(p);
 }
 QCanvasPixmapArray* pixmapArray = new QCanvasPixmapArray(pixmapList);
 if (!pixmapArray->isValid()) {
	kdError() << "invalid array" << endl;
	return false;
 }
 d->mSprite.insert(prop->typeId(), pixmapArray);

// big overview 
 if (d->mBigOverview[type]) {
	kdError() << "BigOverview of " << type << " already there" << endl;
 } else {
	QPixmap* p = new QPixmap;
	if (!loadUnitPixmap(path + "overview-big.png", *p, false)) {
		kdError() << "SpeciesTheme : Can't load " << path + "overview-big.png" << endl;
		delete p;
		return false;
	}
	d->mBigOverview.insert(type, p);
 }

// small overview 
 if (d->mSmallOverview[type]) {
	kdError() << "SmallOverview of " << type << " already there" << endl;
 } else {
	OverviewPixmap* p = new OverviewPixmap;
	if (!loadUnitPixmap(path + "overview-small.png", *p, false)) {
		kdError() << "SpeciesTheme : Can't load " << path + "overview-small.png" << endl;
		return false;
	}
	p->killAlphaMask();
	d->mSmallOverview.insert(type, p);
 }

 BosonSound* sound = boMusic->bosonSound(themePath());
 sound->addUnitSounds(prop);
 return true;
}

QCanvasPixmapArray* SpeciesTheme::pixmapArray(int unitType)
{
 QCanvasPixmapArray* array = d->mSprite[unitType];
 if (!array) {
	loadUnit(unitType);
	array = d->mSprite[unitType];
 }
 if (!array) {
	kdError() << k_funcinfo << ": Cannot find unit type " << unitType 
			<< endl;
	return 0;
 }
 return array;
}

QPixmap* SpeciesTheme::bigOverview(int unitType)
{
 QPixmap* pix = d->mBigOverview[unitType];
 if (!pix) {
	loadUnit(unitType);
	pix = d->mBigOverview[unitType];
 }
 if (!pix) {
	kdError() << k_funcinfo << ": Cannot find unit type " << unitType 
			<< endl;
	return 0;
 }
 return pix;
}

QPixmap* SpeciesTheme::smallOverview(int unitType)
{
 QPixmap* pix = d->mSmallOverview[unitType];
 if (!pix) {
	loadUnit(unitType);
	pix = d->mSmallOverview[unitType];
 }
 if (!pix) {
	kdError() << k_funcinfo << ": Cannot find unit type " << unitType 
			<< endl;
	return 0;
 }
 return pix;
}



bool SpeciesTheme::loadUnitPixmap(const QString &fileName, QPixmap &pix, bool withMask, bool with_team_color)
{
 d->mCanChangeTeamColor = false;
 
 QImage image(fileName);
 QImage *mask = 0;
 int x, y, w, h;
 unsigned char *yp = 0; //AB: what is this??
 QRgb *p = 0;
 static const QRgb background = qRgb(255,  0, 255) & RGB_MASK ;
 static const QRgb background2 = qRgb(248, 40, 240) & RGB_MASK ;

 w = image.width(); 
 h = image.height();

 if (image.depth() != 32) {
	kdError() << k_funcinfo << fileName << ": depth != 32" << endl;
 }
 if (w < 32) {
	kdError() << k_funcinfo << fileName << ": w < 32" << endl;
	return false;
 }
 if (h < 32) {
	kdError() << k_funcinfo << fileName << ": h < 32" << endl;
	return false;
 }

 if (image.isNull()) {
	kdError() << k_funcinfo << ": NULL image" << endl;
	return false;
 }

 if (withMask) {
	mask = new QImage ( w, h, 1, 2, QImage::LittleEndian);
	if (mask->isNull()) {
		kdError() << k_funcinfo << ": NULL mask" << endl;
		return false;
	}
	mask->setColor( 0, 0xffffff );
	mask->setColor( 1, 0 );
	mask->fill(0xff); 
 }
	
 if (withMask) {
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
				if ( (qRed(*p) > 0x80) && (qGreen(*p) < 0x70) && (qBlue(*p) < 0x70)) {
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
	kdError() << k_funcinfo << ": image is null" << endl;
	return false;
 }

 pix.convertFromImage(image);
	
 if (withMask) {
	QBitmap m;
	m.convertFromImage(*mask);
	pix.setMask( m );
 }
 if (mask) {
	delete mask;
 }
 return true;
}

bool SpeciesTheme::loadShotPixmap(const QString& fileName, QPixmap& pix)
{
 QImage image(fileName);
 const QRgb backGround = qRgb(255, 0, 255) & RGB_MASK;

 if (image.isNull()) {
	kdError() << k_funcinfo << ": Could not load " << fileName << endl;
	return false;
 }
 if (image.width() < 25) {
	kdError() << fileName << ": width < 25" << endl;
	return false;
 }
 if (image.height() < 25) {
	kdError() << fileName << ": height < 25" << endl;
	return false;
 }
 if (image.depth() != 32) {
	kdError() << fileName << ": depth != 32" << endl;
 }

 QImage mask(image.width(), image.height(), 1, 2, QImage::LittleEndian);
 if (mask.isNull()) {
	kdError() << k_funcinfo << ": NULL mask" << endl;
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
	kdError() << k_funcinfo << ": NULL unit" << endl;
	return;
 }
 const UnitProperties* prop = unitProperties(unit);
 if (!prop) {
	kdError() << k_funcinfo << ": NULL properties for " << unit->type() << endl;
	return;
 }
 unit->setHealth(prop->health());
 unit->setArmor(prop->armor());
 unit->setShields(prop->shields());
 unit->setRange(prop->range());
 unit->setDamage(prop->damage());
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
	UnitProperties* prop = new UnitProperties(*it);
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
	kdError() << k_funcinfo << ": NULL unit" << endl;
	return 0;
 }
 return unitProperties(unit->type());
}

const UnitProperties* SpeciesTheme::unitProperties(int unitType) const
{
 if (unitType < 0) {
	kdError() << "invalid unit type " << unitType << endl;
	return 0;
 }
 if (!d->mUnitProperties[unitType]) {
	kdError() << k_lineinfo << "oops - no unit properties for " << unitType << endl;
	return 0;
 }
 return d->mUnitProperties[unitType];
}

QValueList<int> SpeciesTheme::allFacilities() const
{
 QValueList<int> list;
 QIntDictIterator<UnitProperties> it(d->mUnitProperties);
 while (it.current()) {
	if (it.current()->isFacility()) {
		list.append(it.current()->typeId());
	}
	++it;
 }
 return list;
}

QValueList<int> SpeciesTheme::allMobiles() const
{
 QValueList<int> list;
 QIntDictIterator<UnitProperties> it(d->mUnitProperties);
 while (it.current()) {
	if (it.current()->isMobile()) {
		list.append(it.current()->typeId());
	}
	++it;
 }
 return list;
}

bool SpeciesTheme::loadShot()
{
 if (mShot) {
	return true;
 }
 QString fileName = themePath() + "explosions/shots/shot_00-%1.png"; // FIXME: shot.00 is hardcoded. is .01, .02 ... possible?

 QValueList<QPixmap> pixList;
 QPointArray points(SHOT_FRAMES);
 
 for (int i = 0; i < SHOT_FRAMES; i++) {
	QPixmap p;
	QString number;
	number.sprintf("%04d", i);
	if (!loadShotPixmap(fileName.arg(number), p)) { 
		kdError() << "Could not load " << fileName.arg(number) << endl;
		return false;
	}
	pixList.append(p);
	points.setPoint(i, p.width() >> 1, p.height() >> 1);
 }

 mShot = new QCanvasPixmapArray(pixList, points);
 return true;
}

bool SpeciesTheme::loadBigShot(bool isFacility, unsigned int version)
{
 if (isFacility) {
	if (d->mFacilityBigShot[version]) {
		return true;
	}
 } else {
	if (d->mMobileBigShot[version]) {
		return true;
	}
 }
 int frames = (isFacility ?  FACILITY_SHOT_FRAMES : MOBILE_SHOT_FRAMES);
 QString path = themePath() + "explosions/" + 
		(isFacility ? "facilities/" : "units/");
 QString v;
 v.sprintf("%02d", version);
 QString fileName = path + QString("expl_%1-").arg(v) + "%1.png";

 QValueList<QPixmap> pixList;
 QPointArray points(frames);
 
 for (int i = 0; i < frames; i++) {
	QPixmap p;
	QString number;
	number.sprintf("%04d", i);
	if (!loadShotPixmap(fileName.arg(number), p)) {
		kdError() << k_funcinfo << ": Could not load"
				<< fileName.arg(number) << endl;
		return false; }
	pixList.append(p);
	points.setPoint(i, p.width() >> 1, p.height() >> 1);
 }
 
 QCanvasPixmapArray* array = new QCanvasPixmapArray(pixList, points);
 if (isFacility) {
	d->mFacilityBigShot.insert(version, array);
 } else {
	d->mMobileBigShot.insert(version, array);
 }
 return true;
}

QCanvasPixmapArray* SpeciesTheme::bigShot(bool isFacility, unsigned int version) const
{
 if (isFacility) {
	return d->mFacilityBigShot.find(version);
 } else {
	return d->mMobileBigShot.find(version);
 }
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

