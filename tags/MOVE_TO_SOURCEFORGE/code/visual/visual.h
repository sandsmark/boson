/***************************************************************************
                         visual.h  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Wed Sep 08 00:40:00 CET 1999
                                           
    copyright            : (C) 1999-2000 by Thomas Capricelli                         
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

#ifndef VISUAL_H 
#define VISUAL_H 

#include "common/boconfig.h"
#include "visualCanvas.h"

//class QPixmap;
class speciesTheme;
class visualCanvas;
class QString;

extern	visualCanvas	*vcanvas;
extern	speciesTheme	*species[BOSON_MAX_PLAYERS];
extern	QString		*dataPath;

	

#endif // VISUAL_H 
