/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001 The Boson Team (boson-devel@lists.sourceforge.net)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#include "speciestheme.h"
#include "unitbase.h"
#include "unitproperties.h"

#include "defines.h"

#include <kstandarddirs.h>
#include <kdebug.h>
#include <ksimpleconfig.h>

#include <qcanvas.h>
#include <qpixmap.h>
#include <qvaluelist.h>
#include <qimage.h>
#include <qbitmap.h>
#include <qintdict.h>
#include <qdir.h>

class SpeciesThemePrivate
{
public:
	SpeciesThemePrivate()
	{
		mShot = 0;
	}


	QString mThemePath;
	QRgb mTeamColor;

	QIntDict<UnitProperties> mUnitProperties;
	QIntDict<QPixmap> mSmallOverview;
	QIntDict<QPixmap> mBigOverview;
	QIntDict<QCanvasPixmapArray> mSprite;

	QCanvasPixmapArray* mShot;
	QIntDict<QCanvasPixmapArray> mFacilityBigShot;
	QIntDict<QCanvasPixmapArray> mMobileBigShot;

	unsigned int mMobileCount;
	unsigned int mFacilityCount;
};

static int defaultColorIndex = 0;
QRgb    default_color[BOSON_MAX_PLAYERS] = {
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

SpeciesTheme::SpeciesTheme(const QString& speciesDir, QRgb teamColor)
{
 d = new SpeciesThemePrivate;
 d->mUnitProperties.setAutoDelete(true);
 d->mFacilityBigShot.setAutoDelete(true);
 d->mMobileBigShot.setAutoDelete(true);
 d->mSmallOverview.setAutoDelete(true);
 d->mBigOverview.setAutoDelete(true);
 d->mSprite.setAutoDelete(true);
 
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
 d->mMobileCount = 0;
 d->mFacilityCount = 0;
 d->mSprite.clear();
 d->mSmallOverview.clear();
 d->mBigOverview.clear();
 d->mUnitProperties.clear();
 d->mFacilityBigShot.clear();
 d->mMobileBigShot.clear();
 if (d->mShot) {
	delete d->mShot;
	d->mShot = 0;
 }
}

QRgb SpeciesTheme::teamColor() const
{
 return d->mTeamColor;
}

QRgb SpeciesTheme::defaultColor()
{
 defaultColorIndex++;
 return default_color[defaultColorIndex - 1];
}

const QString& SpeciesTheme::themePath() const
{
 return d->mThemePath;
}

bool SpeciesTheme::loadTheme(const QString& speciesDir, QRgb teamColor)
{
 if (teamColor == qRgb(0,0,0)) { // no color specified
	d->mTeamColor = defaultColor();
 } else {
	d->mTeamColor = teamColor;
 }
 d->mThemePath += speciesDir;
 kdDebug() << "theme path: " << d->mThemePath << endl;

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
 QString fileName = path + "field.%1.bmp";
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
	if (!loadUnitPixmap(path + "overview.big.bmp", *p, false)) {
		kdError() << "SpeciesTheme : Can't load " << path + "overview.big.bmp" << endl;
		delete p;
		return false;
	}
	d->mBigOverview.insert(type, p);
 }

// small overview 
 if (d->mSmallOverview[type]) {
	kdError() << "SmallOverview of " << type << " already there" << endl;
 } else {
	QPixmap* p = new QPixmap;
	if (!loadUnitPixmap(path + "overview.small.bmp", *p, false)) {
		kdError() << "SpeciesTheme : Can't load " << path + "overview.small.bmp" << endl;
		return false;
	}
	d->mSmallOverview.insert(type, p);
 }
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
	kdError() << k_funcinfo << ": depth != 32" << endl;
 }
 if (w < 32) {
	kdError() << k_funcinfo << ": w < 32" << endl;
	return false;
 }
 if (h < 32) {
	kdError() << k_funcinfo << ": h < 32" << endl;
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
					*p = teamColor();
				}
			}
		}
	}
 } else {
	for ( y = 0; y < h; y++ ) {
		p  = (QRgb *)image.scanLine(y);	// image
		for ( x = 0; x < w; x++, p++ ) {
			if ( (qRed(*p) > 0x90) && (qGreen(*p) < 0x60) && (qBlue(*p) < 0x60)) {
				*p = teamColor();
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
	kdError() << "Shot pixmap width < 25" << endl;
	return false;
 }
 if (image.height() < 25) {
	kdError() << "Shot pixmap height < 25" << endl;
	return false;
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
 unit->setCost(prop->prize());
 unit->setRange(prop->range());
 unit->setDamage(prop->damage());
 unit->setReload(prop->reload());

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
	if (d->mUnitProperties.find(prop->typeId())) {
		kdError() << "UnitType " << prop->typeId() << "already there!" 
				<< endl;
	} else {
		d->mUnitProperties.insert(prop->typeId(), prop);
		if (prop->isFacility()) {
			d->mMobileCount++;
		}
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
	kdError() << "oops - no unit properties for " << unitType << endl;
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
 if (d->mShot) {
	return true;
 }
 QString fileName = themePath() + "explosions/shots/shot.00.%1.bmp"; // FIXME: shot.00 is hardcoded. is .01, .02 ... possible?

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

 d->mShot = new QCanvasPixmapArray(pixList, points);
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
 QString fileName = path + QString("expl.%1.").arg(v) + "%1.bmp";

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

QCanvasPixmapArray* SpeciesTheme::shot() const
{
 return d->mShot;
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

