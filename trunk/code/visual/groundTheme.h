/***************************************************************************
                          groundTheme.h  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Sat Jan  9 19:35:36 CET 1999
                                           
    copyright            : (C) 1999 by Thomas Capricelli                         
    email                : orzel@yalbi.com                                     
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

#include <qstring.h>
#include "../common/groundType.h"

class QBitArray;
class QPixmap;
class QwSpritePixmapSequence;

/** 
 * This class is a ground-pixmap loader / cache
 */
class groundTheme
{
public:
	groundTheme(char *themeName);
	~groundTheme();

	QwSpritePixmapSequence *getPixmap(groundType gt);

private:
	void loadGround		(int i, const QString &path);
	void loadTransition	(groundType gt);

	QString		*themePath;
	QString		transName[TRANS_LAST];
	QBitArray	*pixLoaded;

	QwSpritePixmapSequence **groundPix;

};

#endif // GROUNDTHEME_H

