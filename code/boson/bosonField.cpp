/***************************************************************************
                          bosonField.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Thu Sep  9 01:27:00 CET 1999
                                           
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

#include <stdlib.h>	// rand

#include <assert.h>

//#include <kapp.h>
//#include <kmsgbox.h>

#include "../common/log.h"
#include "../common/boconfig.h" // MAX_PLAYERS
#include "../common/map.h"

#include "bosonField.h"
#include "boshot.h"
#include "game.h" 	// who_am_i
  
bosonField::bosonField(uint w, uint h, QObject *parent, const char *name=0L)
	: visualField(w,h,parent,name)
{
	mobile.resize(149);
	facility.resize(149);
	mobile.setAutoDelete(TRUE);
	facility.setAutoDelete(TRUE);   
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
		boCheck(m.x, mob->x());
		boCheck(m.y, mob->y());
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

	assert(m.who < vpp.nb_player);

	f = new playerFacility(&m);
	facility.insert(m.key, f);

	emit updateFix(f);
	
	if ( FACILITY_CMDBUNKER == m.type && m.who == who_am_i)
		emit reCenterView(m.x, m.y);
}


void bosonField::destroyFix(destroyedMsg_t &msg)
{
	playerFacility * f;
	
	f = facility.find(msg.key);
	if (f) {
		boCheck(msg.x, f->x());
		boCheck(msg.y, f->y());
	}
	else {
		logf(LOG_ERROR, "bosonField::destroyFix : can't find msg.key");
		return;
	}

	boAssert( facility.remove(msg.key) == true);
}


void bosonField::move(moveMsg_t &msg)
{
	playerMobUnit * m;
	
	m = mobile.find(msg.key);
	if (!m) {
		logf(LOG_ERROR, "bosonField::move : can't find msg.key");
		return;
	}

	m->s_moveBy(msg.dx, msg.dy, msg.direction);
}

void bosonField::requestAction(void)
{
	QIntDictIterator<playerMobUnit> mobIt(mobile);
	QIntDictIterator<playerFacility> fixIt(facility);

	for (mobIt.toFirst(); mobIt; ++mobIt)
		mobIt.current()->getWantedAction();
	for (fixIt.toFirst(); fixIt; ++fixIt)
		fixIt.current()->getWantedAction();
}

void bosonField::shoot(shootMsg_t &m)
{
	playerMobUnit	* mob;
	playerFacility	* fix;
	QRect		r;
	int		_z;

	mob = mobile.find(m.target_key);
	fix = facility.find(m.target_key);
	if (!mob && !fix) {
		logf(LOG_ERROR, "bosonField::shoot : unexpected target_key in shootMsg_t : %d", m.target_key);
		return;
	}
	
	if (!mob) {
		r  = fix->rect();
		_z = fix->z();

	} else {
		r  = mob->rect();
		_z = mob->z();
	}

	new boShot(
			r.x() + rand()%r.width(),
			r.y() + rand()%r.height(),
			_z );
}


