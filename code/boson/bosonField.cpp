/***************************************************************************
                          bosonField.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Thu Sep  9 01:27:00 CET 1999
                                           
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

#include <assert.h>

//#include <kapp.h>
//#include <kmsgbox.h>

#include "../common/log.h"
#include "../common/boconfig.h" // MAX_PLAYERS
#include "../common/map.h"

#include "bosonField.h"
  
bosonField::bosonField(uint w, uint h, QObject *parent, const char *name=0L)
	: visualField(w,h,parent,name)
{
}


/*
void bosonField::setCell(int i, int j, groundType g)
{
	boAssert(i>=0); boAssert(j>=0);
	boAssert(i<width()); boAssert(j<height());

	(void) new playerCell(g, i, j);

	emit newCell(i,j,g);
}
*/


void bosonField::createMob(mobileMsg_t &m)
{
	playerMobUnit *u;

	assert(m.who < BOSON_MAX_PLAYERS);

	u = new playerMobUnit(&m);
	mobile.insert(m.key, u);

	emit updateMobile(u);
}


void bosonField::destroyMob(destroyedMsg_t &m)
{
	playerMobUnit *mob ;
	
	mob = mobile.find(m.key);
	if (mob) {
		boAssert(m.x == mob->x());
		boAssert(m.y == mob->y());
		}
	else {
		logf(LOG_ERROR, "bosonField::destroyMob : can't find m.key");
		return;
		}

	boAssert( mobile.remove(m.key) == true );
}

void bosonField::createFix(facilityMsg_t &m)
{
	playerFacility *f;

	assert(m.who < BOSON_MAX_PLAYERS);

	f = new playerFacility(&m);
	facility.insert(m.key, f);

	emit updateFix(f);
}


void bosonField::destroyFix(destroyedMsg_t &m)
{
	playerFacility * f;
	
	f = facility.find(m.key);
	if (f) {
		boAssert(m.x == f->x());
		boAssert(m.y == f->x());
		}
	else {
		logf(LOG_ERROR, "bosonField::destroyFix : can't find m.key");
		return;
		}

	boAssert( facility.remove(m.key) == true);
}


void bosonField::move(moveMsg_t &m)
{
	mobile.find(m.key)->s_moveBy(m.dx, m.dy, m.direction);
}

void bosonField::requestAction(boBuffer *buffer)
{
	QIntDictIterator<playerMobUnit> mobIt(mobile);
	//QIntDictIterator<playerFacility> fixIt(facility);
	int		dx, dy, dir;
	bosonMsgData	data;

	for (mobIt.toFirst(); mobIt; ++mobIt) {
		if (mobIt.current()->getWantedMove(dx,dy,dir)){
			data.move.key	= mobIt.currentKey();
			data.move.dx	= dx;
			data.move.dy	= dy;
			data.move.direction = dir;
			sendMsg(buffer, MSG_MOBILE_MOVE_R, sizeof(data.move), &data);
			}
		}
}
