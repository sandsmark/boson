/***************************************************************************
                          visualCell.cpp  -  description                              
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

#include "../map/map.h"

#include "groundTheme.h"
#include "visualCell.h"
#include "visual.h"


visualCell::visualCell(groundType g, int i, int j)
	: Cell(g)
	, QwSprite(vpp.ground->getPixmap(g))
{

	z(Z_GROUND);
	moveTo(BO_TILE_SIZE * i , BO_TILE_SIZE * j);
}

