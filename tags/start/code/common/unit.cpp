/***************************************************************************
                          unit.cpp  -  description                              
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

#include "unit.h"
#include "../map/map.h"

Unit::Unit(QObject*parent, const char *name=0L)
	: QObject(parent, name)
{
//pos_x = 0; 
//pos_y = 0;
who = 0;
}

Unit::~Unit()
{
}

mobUnit::mobUnit(mobileMsg_t *msg, QObject* parent, const char *name=0L)
:Unit(parent,name)
{
key	= msg->key;
//pos_x	= msg->x;
//pos_y	= msg->y;
who	= msg->who;
type	= msg->type;
//printf("mobUnit, creation with type = %d\n", type);

isShown = TRUE; ///orzel : should be removed since Qw handles this now...
}

QRect mobUnit::rect(void)
{
register int
	w = mobileProp[type].width,
	h = mobileProp[type].height;
return QRect(-w/2 + _x(), -h/2 + _y(), w, h);
}

Facility::Facility(facilityMsg_t *msg, QObject* parent, const char *name=0L)
:Unit(parent,name)
{
key	= msg->key;
//pos_x	= msg->x;
//pos_y	= msg->y;
who	= msg->who;
type	= msg->type;
}


QRect Facility::rect(void)
{
return QRect(
		BO_TILE_SIZE * _x(),
		BO_TILE_SIZE * _y(),
		BO_TILE_SIZE * facilityProp[type].width,
		BO_TILE_SIZE * facilityProp[type].height);
}



