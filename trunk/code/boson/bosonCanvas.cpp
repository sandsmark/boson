/***************************************************************************
                          bosonCanvas.cpp  -  description                              
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

#include "bosonCanvas.h"
#include "boshot.h"
#include "game.h" 	// who_am_i
  

/*
 *  BOSON CANVAS
 */
bosonCanvas::bosonCanvas( QPixmap p, uint w, uint h)
	: visualCanvas(p,w,h)
{
	mobile.resize(149);
	facility.resize(149);
//	mobile.setAutoDelete(TRUE);
//	facility.setAutoDelete(TRUE);   

	cells = new Cell[w*h];
}


bosonCanvas::~bosonCanvas()
{
	delete [] cells;
}


void bosonCanvas::hideMob(destroyedMsg_t &m)
{
	playerMobUnit *u;

	u = mobile.find(m.key);
	if (u) u->doHide();
	else logf(LOG_ERROR, "bosonCanvas::unHideMob : can't find m.key");

	// XXX emit something for the minimap
}


void bosonCanvas::unHideMob(mobileMsg_t &m)
{
	playerMobUnit *u;

	assert(m.who < nb_player);
	u = mobile.find(m.key);
	if (u) u->doShow();
	else logf(LOG_ERROR, "bosonCanvas::unHideMob : can't find m.key");

	// XXX emit something for the minimap
}



void bosonCanvas::createMob(mobileMsg_t &m)
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


void bosonCanvas::destroyMob(destroyedMsg_t &m)
{
	playerMobUnit *mob ;
	
	mob = mobile.find(m.key);
	if (!mob) {
		logf(LOG_ERROR, "bosonCanvas::destroyMob : can't find m.key");
		return;
	}

	boCheck(m.x, mob->x());
	boCheck(m.y, mob->y());

	QPoint p  = mob->center();
	new boShot ( p.x(), p.y(), mob->z(), boShot::SHOT_UNIT);

	mob->destroy();
	boAssert( mobile.remove(m.key) == true );
}


void bosonCanvas::hideFix(destroyedMsg_t &m)
{
	playerFacility *f;

	f = facility.find(m.key);
	if (f) f->doHide();
	else logf(LOG_ERROR, "bosonCanvas::unHideFix : can't find m.key");

	// XXX emit something for the minimap
}

void bosonCanvas::unHideFix(facilityMsg_t &m)
{
	playerFacility *f;

	assert(m.who < (uint) nb_player);
	f = facility.find(m.key);
	if (f) f->doShow();
	else logf(LOG_ERROR, "bosonCanvas::unHideFix : can't find m.key");

	// XXX emit something for the minimap
}


void bosonCanvas::createFix(facilityMsg_t &m)
{
	playerFacility *f;

	assert(m.who < (uint) nb_player);

	f = new playerFacility(&m);
	facility.insert(m.key, f);

	emit updateFix(f);
	
	if ( FACILITY_CMDBUNKER == m.type && m.who == who_am_i)
		emit reCenterView(m.x, m.y);
}


void bosonCanvas::destroyFix(destroyedMsg_t &msg)
{
	playerFacility * f;
	
	f = facility.find(msg.key);
	if (!f) {
		logf(LOG_ERROR, "bosonCanvas::destroyFix : can't find msg.key");
		return;
	}
	boCheck(msg.x, f->x());
	boCheck(msg.y, f->y());

	QPoint p = f->center();
	new boShot ( p.x(), p.y(), f->z(), boShot::SHOT_FACILITY);

	f->destroy();
	boAssert( facility.remove(msg.key) == true);
}


void bosonCanvas::move(moveMsg_t &msg)
{
	playerMobUnit * m;
	
	m = mobile.find(msg.key);
	if (!m) {
		logf(LOG_ERROR, "bosonCanvas::move : can't find msg.key");
		return;
	}

	playerMobUnit::state_t	ns;

	ns.x = msg.newx;
	ns.y = msg.newy;
	m->s_moveTo(ns);
}

void bosonCanvas::requestAction(void)
{
	QIntDictIterator<playerMobUnit> mobIt(mobile);
	QIntDictIterator<playerFacility> fixIt(facility);

	for (mobIt.toFirst(); mobIt; ++mobIt)
		mobIt.current()->getWantedAction();
	for (fixIt.toFirst(); fixIt; ++fixIt)
		fixIt.current()->getWantedAction();
}

void bosonCanvas::shooted(powerMsg_t &m)
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
	} else	logf(LOG_ERROR, "bosonCanvas::shooted : unexpected key in powerMsg_t : %d", m.key);

}


void bosonCanvas::updateRess(unitRessMsg_t &m)
{
	playerMobUnit	* mob = mobile.find(m.key);
	
	if (mob) {
		mob->updateContain(m.contain);
	} else logf(LOG_ERROR, "bosonCanvas::updateRess : unexpected key in unitRessMsg_t : %d", m.key);
}


void bosonCanvas::shoot(shootMsg_t &m)
{
	playerMobUnit	* mob;
	playerFacility	* fix;
	QRect		r;
	double		_z;

	mob = mobile.find(m.target_key);
	fix = facility.find(m.target_key);
	if (!mob && !fix) {
		logf(LOG_ERROR, "bosonCanvas::shoot : unexpected target_key in shootMsg_t : %d", m.target_key);
		return;
	}
	
	if (!mob) {
		r  = fix->rect();
		_z = fix->z();

	} else {
		r  = mob->rect();
		_z = mob->z();
	}

	r.setSize( r.size()/2 );
	r.moveBy ( r.size().width()>>1, r.size().height()>>1 );

	new boShot(
			r.x() + rand()%r.width(),
			r.y() + rand()%r.height(),
			_z , 
			boShot::SHOT_SHOT);
}

void bosonCanvas::setCell(int i, int j, cell_t c)
{
	visualCanvas::setCell(i,j,c);
	cell(i,j).setGround( ground(c) );
}

