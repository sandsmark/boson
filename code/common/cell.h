/***************************************************************************
                          cell.h  -  description                              
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

#ifndef CELL_H 
#define CELL_H 

#include "common/groundType.h"

#ifndef byte
typedef unsigned char byte;
#endif

/** 
  * This class represents one cell of the main game board
  */
class Cell
{

public:
	Cell(groundType g = GROUND_UNKNOWN, byte it=0);

	groundType	getGroundType(void)	{ return ground; }
	byte		getItem()		{ return item; }

	void		setGroundType(groundType g)	{ ground = g ; }
	void		setItem(byte it)		{ item   = it ; }

protected:
	groundType	ground;
	byte		item;

private:
//	destroyedType	destroyed;
};

#endif // CELL_H
