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

#include "common/log.h"
#include "common/boconfig.h" // MAX_PLAYERS
#include "common/map.h"

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

void bosonField::hideMob(destroyedMsg_t &m)
{
	playerMobUnit *u;

	u = mobile.find(m.key);
	if (u) u->doHide();
	else logf(LOG_ERROR, "bosonField::unHideMob : can't find m.key");

	// XXX emit something for the minimap
}



void bosonField::unHideMob(mobileMsg_t &m)
{
	playerMobUnit *u;

	assert(m.who < nb_player);
	u = mobile.find(m.key);
	if (u) u->doShow();
	else logf(LOG_ERROR, "bosonField::unHideMob : can't find m.key");

	// XXX emit something for the minimap
}



void bosonField::createMob(mobileMsg_t &m)
{
	playerMobUnit *u;

	assert(m.who < nb_player);

	switch (m.type) {
		default:
		case MOB_MINERAL_HARVESTER:
			u = new playerMobUnit(&m);
			break;
		case MOB_OIL_HARVESTER:
			u = new harvesterUnit(&m);
			break;
	};

	mobile.insert(m.key, u);

	emit updateMobile(u);
}


void bosonField::destroyMob(destroyedMsg_t &m)
{
	playerMobUnit *mob ;
	
	mob = mobile.find(m.key);
	if (!mob) {
		logf(LOG_ERROR, "bosonField::destroyMob : can't find m.key");
		return;
	}

	boCheck(m.x, mob->x());
	boCheck(m.y, mob->y());

	QRect r  = mob->rect();
	new boShot ( r.x() + (r.width()>>1), r.y() + (r.height()>>1), mob->z(), true);

	boAssert( mobile.remove(m.key) == true );
}


void bosonField::hideFix(destroyedMsg_t &m)
{
	playerFacility *f;

	f = facility.find(m.key);
	if (f) f->doHide();
	else logf(LOG_ERROR, "bosonField::unHideFix : can't find m.key");

	// XXX emit something for the minimap
}

void bosonField::unHideFix(facilityMsg_t &m)
{
	playerFacility *f;

	assert(m.who < nb_player);
	f = facility.find(m.key);
	if (f) f->doShow();
	else logf(LOG_ERROR, "bosonField::unHideFix : can't find m.key");

	// XXX emit something for the minimap
}


void bosonField::createFix(facilityMsg_t &m)
{
	playerFacility *f;

	assert(m.who < nb_player);

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
	if (!f) {
		logf(LOG_ERROR, "bosonField::destroyFix : can't find msg.key");
		return;
	}
	boCheck(msg.x, f->x());
	boCheck(msg.y, f->y());

	QRect r  = f->rect();
	new boShot ( r.x() + (r.width()>>1), r.y() + (r.height()>>1), f->z(), true);

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

	m->s_moveTo(msg.newx, msg.newy, msg.direction);
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

void bosonField::shooted(powerMsg_t &m)
{
	playerMobUnit	* mob;
	playerFacility	* fix;

	mob = mobile.find(m.key);
	fix = facility.find(m.key);

	if (mob) {
		/* shooting a mobile */
		mob->shooted(m.power);
	} else if (fix) {
		/* shooting a facility */
		fix->shooted(m.power);
	} else	logf(LOG_ERROR, "bosonField::shooted : unexpected key in powerMsg_t : %d", m.key);

}


void bosonField::updateRess(unitRessMsg_t &m)
{
	playerMobUnit	* mob = mobile.find(m.key);
	
	if (mob) {
		mob->updateContain(m.contain);
	} else logf(LOG_ERROR, "bosonField::updateRess : unexpected key in unitRessMsg_t : %d", m.key);
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


