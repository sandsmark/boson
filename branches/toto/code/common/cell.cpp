/***************************************************************************
                          cell.cpp  -  description                              
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

#include "cell.h"

/*
Cell::Cell(groundType g, QObject*parent, const char *name=0L)
	: QObject(parent, name)
*/
Cell::Cell(groundType g)
//Cell::Cell(int pix)
{
ground = g;
destroyed = DESTROYED_NONE ;
}

Cell::~Cell()
{
}

/*
void Cell::setGroundType(groundType g)
{
ground = g;
} */
