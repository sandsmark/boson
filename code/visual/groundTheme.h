/***************************************************************************
                          groundTheme.h  -  description                              
                             -------------------                                         

    version              : $Id$
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

#ifndef GROUNDTHEME_H 
#define GROUNDTHEME_H 

//#ifdef HAVE_CONFIG_H
//#include <config.h>
//#endif 

#include "../common/groundType.h"

class QString;
class QBitArray;
class QPixmap;
class QwSpritePixmapSequence;

/** 
  * This class handle drawing of the ground in the Boson player (client)
  */
class groundTheme
{
public:
	groundTheme(char *themeName);

	QwSpritePixmapSequence *getPixmap(groundType gt);

private:
	bool loadGround		(int i, const QString &path);
	bool loadTransition	(int i);

	QString		*themePath;
	QBitArray	*transitions;

	QwSpritePixmapSequence **groundPix;

};


#endif // GROUNDTHEME_H

