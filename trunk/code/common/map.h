/***************************************************************************
                          map.h  -  description                    
                             -------------------                                         

    version              : $Id$
    begin                : Sat Apr 17 23:02:00 CET 1999
                                           
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

#ifndef MAP_H
#define MAP_H


#include "common/groundType.h"

#define BO_TILE_SIZE 48

#define		BIG_W	32
#define		BIG_H	( (NB_GROUND_TILES*4 + BIG_W-1) / BIG_W )

/*
 * BIG_W		should be 4*n, cause this way it's easier to watch the generated file
 * NB_GROUND_TILES*4	is 1244 (on may, 14th 200), so BIG_H should be 39
 */

#endif /* MAP_H */
