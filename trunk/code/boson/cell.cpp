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
 
 switch (groundType()) {
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
		return false;
 }
 return false; // never reached, btw
}

