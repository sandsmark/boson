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
#include "cell.h"

#include "unitproperties.h"
#include "defines.h"

#include <kdebug.h>
#include <kstandarddirs.h>

#include <qcanvas.h>
#include <qbitmap.h>

QCanvasPixmapArray* Cell::mFogPixmap = 0;


Cell::Cell()
{
 mFog = 0;
 setGroundType(GroundUnknown);
 initStatic();
}

Cell::~Cell()
{
}

void Cell::setGroundType(GroundType t)
{
 mType = t;
}

void Cell::makeCell(int groundType, unsigned char version)
{
 setGroundType((GroundType)groundType);
 setVersion(version);
 if (groundType == GroundUnknown) {
	kdError() << "unknown ground?!" << endl;
 }
}

bool Cell::canGo(const UnitProperties* prop) const
{ // probably a time critical function!
 if (!prop) {
	kdError() << k_funcinfo << ": NULL unit properties" << endl;
	return false;
 }
 if (isPlain(groundType())) {
	return canGo(prop, (GroundType)groundType());
 } else if (isTrans(groundType())) {
	TransType g = (TransType)getTransRef(groundType());
	return (canGo(prop, from(g)) && canGo(prop, to(g)));
 } else {
	kdWarning() << "neither plain nor transition" << endl;
	return false;
 }
}

bool Cell::canGo(const UnitProperties* prop, GroundType ground)
{ // probably a time critical function!
 switch (ground) {
	case GroundGrass:
	case GroundDesert:
		if (prop->isFacility()) {
			return true;
		}
		return prop->canGoOnLand();
	case GroundGrassMineral:
	case GroundGrassOil:
		if (prop->isFacility()) { // not on minerals/oil
			return false;
		}
		return prop->canGoOnLand();
	case GroundDeepWater:
	case GroundWater:
		return prop->canGoOnWater();
	default:
		kdWarning() << "unknown groundType " << ground << endl;
		return false;
 }
 return false; // never reached, btw
}

bool Cell::isValidGround(int g)
{
 if (g < 0) {
	return false;
 }
 if (g > groundTilesNumber()) { 
	return false;
 }
 return true;
}

bool Cell::isTrans(int ground)
{
 return (ground >= GroundLast && ground < groundTilesNumber());
}
bool Cell::isPlain(int ground)
{
 return (ground >=0 && ground < GroundLast );
}

int Cell::groundTilesNumber()
{
 return GroundLast + TransLast * tilesPerTransition();
}

int Cell::tilesPerTransition()
{
 return smallTilesPerTransition() + 4 * bigTilesPerTransition();
}
int Cell::smallTilesPerTransition()
{
 return 12;
}

int Cell::bigTilesPerTransition()
{
 return 16;
}

Cell::GroundType Cell::from(TransType trans)
{
 switch (trans) {
	case TransGrassWater:
		return GroundGrass;
	case TransGrassDesert:
		return GroundGrass;
	case TransDesertWater:
		return GroundDesert;
	case TransDeepWater:
		return GroundDeepWater;
	default:
		kdError() << "Unknown trans " << (int)trans << endl;
		return GroundUnknown;
 }
}
Cell::GroundType Cell::to(TransType trans)
{
 switch (trans) {
	case TransGrassWater:
		return GroundWater;
	case TransGrassDesert:
		return GroundDesert;
	case TransDesertWater:
		return GroundWater;
	case TransDeepWater:
		return GroundWater;
	default:
		kdError() << "Unknown trans " << (int)trans
				<< endl;
		return GroundUnknown;
 }
}


int Cell::getTransRef(int g)
{
 return ((g - GroundLast) / tilesPerTransition());
}

int Cell::getTransNumber(TransType transRef, int transTile)
{
 return GroundLast + tilesPerTransition() * (int)transRef + transTile;
}

int Cell::getTransTile(int g)
{
 return ((g - Cell::GroundLast) % tilesPerTransition());
}

int Cell::getBigTransNumber(TransType transRef, int transTile)
{
 return getTransNumber(transRef, smallTilesPerTransition() + 4 * transTile);
}

bool Cell::isSmallTrans(int g)
{
 return (isTrans(g) && getTransTile(g) < smallTilesPerTransition());
}

bool Cell::isBigTrans(int g)
{
 return (isTrans(g) && getTransTile(g) >= smallTilesPerTransition());
}
int Cell::smallTileNumber(int smallNo, TransType trans, bool inverted)
{
 int tileNo;
 switch (smallNo) {
	case 0:
		tileNo = getTransNumber(trans, inverted ? 
				TransUpLeftInverted : TransUpLeft);
		break;
	case 1:
		tileNo = getTransNumber(trans, inverted ? 
				TransDown : TransUp);
		break;
	case 2:
		tileNo = getTransNumber(trans, inverted ? 
				TransUpRightInverted : TransUpRight);
		break;
	case 3:
		tileNo = getTransNumber(trans,
				inverted ? TransRight : TransLeft);
		break;
	case 4:
		tileNo = getTransNumber(trans,
				inverted ? to(trans) : from(trans));
		break;
	case 5:
		tileNo = getTransNumber(trans,
				inverted ? TransLeft : TransRight);
		break;
	case 6:
		tileNo = getTransNumber(trans, inverted ? 
				TransDownLeftInverted : TransDownLeft);
		break;
	case 7:
		tileNo = getTransNumber(trans, inverted ? 
				TransUp : TransDown);
		break;
	case 8:
		tileNo = getTransNumber(trans, inverted ? 
				TransDownRightInverted : TransDownRight);
		break;
	default:
		kdError() << "Unknwon small tile " << smallNo << endl;
		return 0;
 }
 return tileNo;
}

int Cell::moveCost() const
{
 int cost = 0;
 switch (groundType()) {
	case GroundDeepWater:
		cost = 0;
		break;
	case GroundWater:
		cost = 0;
		break;
	case GroundGrass:
		cost = 0;
		break;
	case GroundDesert:
		cost = 1;
		break;
	case GroundGrassMineral:
		cost = 3;
		break;
	case GroundGrassOil:
		cost = 2;
		break;
	case GroundUnknown:
	default:
		kdWarning() << k_funcinfo << ": invalid ground" << endl;
		cost = 0;
		break;
 }
 return cost;
}

void Cell::fog(QCanvas* canvas, int x, int y)
{
 if (mFog) {
	return;
 }
 if (!mFogPixmap) {
	return;
 }
 mFog = new QCanvasSprite(mFogPixmap, canvas);
 mFog->move(x * BO_TILE_SIZE, y * BO_TILE_SIZE);
 mFog->setZ(Z_FOG_OF_WAR);
 mFog->show();
}

void Cell::unfog()
{
 if (!mFog) {
	return;
 }
 delete mFog;
 mFog = 0;
}

void Cell::initStatic()
{
 if (mFogPixmap) {
	return;
 }
 QString fogPath = locate("data", "boson/themes/fow.xpm");
 kdDebug() << fogPath << endl;
 mFogPixmap = new QCanvasPixmapArray(fogPath);
 if (!mFogPixmap->image(0) || 
		mFogPixmap->image(0)->width() != (BO_TILE_SIZE * 2) ||
		mFogPixmap->image(0)->height() != (BO_TILE_SIZE * 2)) {
	kdError() << k_funcinfo << "Cannot load fow.xpm" << endl;
	delete mFogPixmap;
	mFogPixmap = 0;
	return;
 }
 QBitmap mask(fogPath);
 if (mask.width() != (BO_TILE_SIZE * 2) || mask.height() != (BO_TILE_SIZE * 2)) {
	kdError() << k_funcinfo << "Can't create fow mask" << endl;
	delete mFogPixmap;
	mFogPixmap = 0;
	return;
 }
 mFogPixmap->image(0)->setMask(mask);
 mFogPixmap->image(0)->setOffset(BO_TILE_SIZE / 2, BO_TILE_SIZE / 2);
}

