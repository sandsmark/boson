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

Unit::Unit(int _who, QObject*parent, const char *name)
	: QObject(parent, name)
{
	who		= _who;
	
	countDown	= 0;
	work		= WORK_NONE;
}


/*
 * MOBILE
 */

mobUnit::mobUnit(mobileMsg_t *msg, QObject* parent, const char *name)
:Unit(msg->who, parent,name)
{
	type	= msg->type;

	isShown = TRUE; ///orzel : should be removed since Qw handles this now...
}

void mobUnit::fill(mobileMsg_t &msg)
{
	msg.who = who;
	msg.x   = _x();
	msg.y   = _y();
	msg.type= type;
}

QRect mobUnit::rect(void)
{
	register int
		w = mobileProp[type].width,
		h = mobileProp[type].height;
	return QRect(-w/2 + _x(), -h/2 + _y(), w, h);
}


/*
 * FACILITY
 */

Facility::Facility(facilityMsg_t *msg, QObject* parent, const char *name)
:Unit(msg->who, parent,name)
{
	type	= msg->type;
}


QRect Facility::rect(void)
{
	return QRect( _x(), _y(),
		facilityProp[type].width,
		facilityProp[type].height);
}


void Facility::fill(facilityMsg_t &msg)
{
	msg.who = who;
	msg.x   = _x() / BO_TILE_SIZE;
	msg.y   = _y() / BO_TILE_SIZE;
	msg.type= type;
}


