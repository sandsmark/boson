/***************************************************************************
                          groundTheme.h  -  description                              
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

#ifndef GROUND_THEME_H 
#define GROUND_THEME_H 

//#ifdef HAVE_CONFIG_H
//#include <config.h>
//#endif 

#include "../common/groundType.h"

class QString;
class QPixmap;
class QProgressDialog;
class QwSpritePixmapSequence;

/** 
  * This class handle drawing of the ground in the Boson player (client)
  */
class groundTheme
{
  public:
  groundTheme(char *themeName);
  ~groundTheme();

//	QPixmap		*getPixmap(groundType gt) { return pixmap[gt]; }
	QwSpritePixmapSequence
		*getPixmap(groundType gt) { return groundPix[gt]; }
	bool		isOk(void){ return allLoaded; }

  private:
	bool loadGround(int i, const QString &path, QProgressDialog &progress);
	bool loadTransition(int i, const QString &path, QProgressDialog &progress);

  bool		allLoaded;
  QPixmap	*no_pixmap;	///orzel : beurk .. no_pixmap == pixmap[-1]... a revoir
//  QPixmap	*pixmap[GROUND_LAST + 8 * 5]; ///orzel : max 5 transitions

  QwSpritePixmapSequence
		*groundPix[GROUND_LAST + TILES_PER_TRANSITION * 5]; ///orzel : max 5 transitions

};


#endif // GROUND_THEME_H
