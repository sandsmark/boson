/***************************************************************************
                         game.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Mon Nov  6 23:47:47 EST 2000
                                           
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


#include "game.h"

/*
 * visual/visual.h
 */
visualCanvas		*vcanvas=0;
speciesTheme		*species[BOSON_MAX_PLAYERS] = {0l, 0l, 0l, 0l, 0l  , 0l, 0l, 0l, 0l, 0l};
QString			*dataPath;



/*
 * editor/game.h
 */
editorCanvas	*ecanvas;
QPixmap		*bigBackground;


