/***************************************************************************
                          unit.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Sat Jan  9 19:35:36 CET 1999
                                           
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

#include "unit.h"
#include "bomap.h"


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
	:Unit( msg)
{
	type	= msg->type;
}


void mobUnit::fill(mobileMsg_t &msg)
{
	QRect r = gridRect();

	msg.who = who;
	msg.x   = r.x();
	msg.y   = r.y();
	msg.type= type;
}

/*
 * FACILITY
 */
Facility::Facility(facilityMsg_t *msg)
	:Unit( msg)
{
	type	= msg->type;
}


void Facility::fill(facilityMsg_t &msg)
{
	QRect r = gridRect();

	msg.who = who;
	msg.x   = r.x();
	msg.y   = r.y();
	msg.type= type;
}


