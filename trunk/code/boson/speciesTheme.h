/***************************************************************************
                          speciesTheme.h  -  description                              
                             -------------------                                         

    version              :                                   
    begin                : Sat Jan  9 19:35:36 CET 1999
                                           
    copyright            : (C) 1999 by Thomas Capricelli                         
    email                : capricel@enst.fr                                     
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#ifndef SPECIES_THEME_H 
#define SPECIES_THEME_H 


//#include <QwSpriteField.h>

#include "../common/cell.h"
#include "../common/unitType.h"
#include "playerUnit.h"

class QPixmap;
class QwSpritePixmapSequence;

/** 
  * This class handles the drawing of the different units for a given specie(s?)
  */
class speciesTheme
{
 public:
  speciesTheme(char *themeName);
  ~speciesTheme();

  QwSpritePixmapSequence
	*getPixmap(facilityType unit) { return fixSprite[unit]; }

  QwSpritePixmapSequence
	*getPixmap(mobType unit) { return mobSprite[unit]; }

  QPixmap	*getBigOverview(mobType unit) { return mobBigOverview[unit]; }
  QPixmap	*getBigOverview(facilityType unit) { return fixBigOverview[unit];}

  QPixmap	*getBigOverview(playerMobUnit *u) { return getBigOverview(u->getType()); }
  QPixmap	*getBigOverview(Facility *u) { return getBigOverview(u->getType()); }

  QPixmap	*getSmallOverview(mobType unit) { return mobSmallOverview[unit]; }
  QPixmap	*getSmallOverview(facilityType unit) { return fixSmallOverview[unit];}

  QPixmap	*getSmallOverview(playerMobUnit *u) { return getSmallOverview(u->getType()); }
  QPixmap	*getSmallOverview(Facility *u) { return getSmallOverview(u->getType()); }

  bool		isOk(void) { return isLoaded; }

  protected:
  bool 		loadMob(int index, QString const &path);
  bool		loadFix(int i, QString const &path);

  private:
  bool		isLoaded;

///orzel: ugly, will be moved with dynamic allocation in constructors for those tabs
#define mobilePropNb	10
#define facilityPropNb	10

  QPixmap	*mobBigOverview[mobilePropNb];		// pixmaps for the control panel
  QPixmap	*fixBigOverview[facilityPropNb];	// pixmaps for the control panel
  QPixmap	*mobSmallOverview[mobilePropNb];	// pixmaps for the control panel
  QPixmap	*fixSmallOverview[facilityPropNb];	// pixmaps for the control panel

  QwSpritePixmapSequence
		*mobSprite[mobilePropNb],	// a mobile
		*fixSprite[facilityPropNb];	// facilities

#undef mobilePropNb
#undef facilityPropNb

};

#endif // SPECIES_THEME_H


