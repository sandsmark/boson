/***************************************************************************
                          bomap.h  -  description                    
                             -------------------                                         

    version              : $Id$
    begin                : Sat Apr 17 23:02:00 CET 1999
                                           
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

#ifndef BOMAP_H
#define BOMAP_H


#include "common/groundType.h"

#define BO_TILE_SIZE 48

/*
 * BIG_W,BIG_H
 * number of pixmap in widht/height of the big picture (earth.png)
 * should be 4*n, cause this way it's easier to watch the generated file
 * NB_GROUND_TILES*4	is 1244 (on may, 14th 2000), so BIG_H should be 39
 */
#define	BIG_W	32
#define	BIG_H	( (NB_GROUND_TILES*4 + BIG_W-1) / BIG_W )

// to get (x,y) position of a given ground 'g' in the big
// pixmap used to store all ground pixmap
#define GET_BIG_X(g) (( (g) % BIG_W) * BO_TILE_SIZE)
#define GET_BIG_Y(g) (( (g) / BIG_W) * BO_TILE_SIZE)



#endif /* BOMAP_H */
