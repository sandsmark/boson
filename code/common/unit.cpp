/***************************************************************************
                          unit.cpp  -  description                              
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

#include "unit.h"
#include "map.h"


/*
 * MOBILE
 */
QRect Unit::gridRect(void)
{
	QRect r = rect();

	r.moveTopLeft( r.topLeft() / BO_TILE_SIZE );
	r.setSize( (r.size() + QSize( BO_TILE_SIZE-1, BO_TILE_SIZE-1))/ BO_TILE_SIZE );

	return r;
}

/*
 * MOBILE
 */
mobUnit::mobUnit(mobileMsg_t *msg)
	:Unit( (unitMsg_t*) msg)
{
	type	= msg->type;
}


void mobUnit::fill(mobileMsg_t &msg)
{
	QRect r = rect();

	msg.who = who;
	msg.x   = r.x();
	msg.y   = r.y();
	msg.type= type;
}


QRect mobUnit::rect(void)
{
	register int
		w = mobileProp[type].width,
		h = mobileProp[type].height;
	return QRect(-w/2 , -h/2 , w, h);
}


/*
 * FACILITY
 */
Facility::Facility(facilityMsg_t *msg)
	:Unit( (unitMsg_t*) msg)
{
	type	= msg->type;
}


void Facility::fill(facilityMsg_t &msg)
{
	QRect r = rect();

	msg.who = who;
	msg.x   = r.x() / BO_TILE_SIZE;
	msg.y   = r.y() / BO_TILE_SIZE;
	msg.type= type;
}


