#include "cell.h"

#include "unitproperties.h"

#include <kdebug.h>

Cell::Cell()
{
 setGroundType(GroundUnknown);
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
// kdDebug() << "Cell::makeCell() " <<  endl;
 setGroundType((GroundType)groundType);
 setVersion(version);
 if (groundType == GroundUnknown) {
	kdError() << "unknown ground?!" << endl;
 }
}

bool Cell::canGo(const UnitProperties* prop) const
{ // probably a time critical function!
 if (!prop) {
	kdError() << "Cell::canGo(): NULL unit properties" << endl;
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
		return GroundDesert;
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
 return 0;
}
