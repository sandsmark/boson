/***************************************************************************
                          visualCell.cpp  -  description                              
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

#include "../common/map.h"

#include "groundTheme.h"
#include "visualCell.h"
#include "visual.h"


visualCell::visualCell(groundType g)
	: Cell(g)
	, QwSprite()
{
	// don't do anything until set() has been called, no even a z() !
}

visualCell::visualCell(groundType g, int i, int j)
	: Cell(g)
	, QwSprite()
{

	set(g,i,j);	// To be done first !
}

void visualCell::set(groundType g, int i, int j)
{
	// first of all, call setSequence, else you'll get a segfault
	set(g);
	moveTo(BO_TILE_SIZE * i , BO_TILE_SIZE * j);
	z(Z_GROUND);
}


void visualCell::set(groundType g)
{
	setGroundType(g);
	if (GROUND_UNKNOWN != g) {
		setSequence(::ground->getPixmap(g));
		show ();
	}
	else	hide();
}




