/***************************************************************************
                          bosonCanvas.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Thu Sep  9 01:27:00 CET 1999
                                           
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

#include <stdlib.h>	// rand
#include <assert.h>

// Arts / sound stuff
//#include <arts/qiomanager.h>
#include <arts/soundserver.h>                                                                                                                                     

#include "common/log.h"
#include "common/boconfig.h" // MAX_PLAYERS

#include "visual/speciesTheme.h"

#include "bosonCanvas.h"
#include "boshot.h"
#include "game.h" 	// who_am_i
  

Arts::SimpleSoundServer		*soundserver;

/*
 *  BOSON CANVAS
 */
bosonCanvas::bosonCanvas( QPixmap p, uint w, uint h, uint np)
	: visualCanvas(p,w,h)
	, CellMap (w, h)
{
	nb_player = np;

	//
	// units containers
	//
	mobile.resize(149);
	facility.resize(149);

	my_fix = my_mobiles = 0;



	//
	// species themes
	//
	boAssert(nb_player<BOSON_MAX_PLAYERS);
	boAssert(nb_player>1);
	for (uint i=0; i<nb_player; i++) {
		species[i]	= new speciesTheme("human");
		/* XXX todo: test if the theme has been loaded
			if (!species[1]->isOk()) KMsgBox::message(0l,
				i18n("Pixmap loading error"),
				i18n("Error while loading \"blue\" specie theme,\nsome images will show up awfully"),
				KMsgBox::EXCLAMATION);
		*/
	}

	//
	// ping initialisation, not relevant
	//
	last_sync = time(NULL);
	radar_pulse = 0;

	//
	// cells
	//
	cells = new bosonCell[w*h];

	//
	// sound initialisation 
	//
	//new Arts::Dispatcher(new Arts::QIOManager); // using this one, the second boson segfaults in Arts::Reference()
	new Arts::Dispatcher();
	soundserver =  new Arts::SimpleSoundServer ;
	//logf(LOG_INFO, "Soundserver created");
        *soundserver =  Arts::Reference("global:Arts_SimpleSoundServer"); 	//soundserver
	//logf(LOG_INFO, "Soundserver assigned");

	if(soundserver->isNull())
		logf(LOG_ERROR, "can't open artsd connection for sound, SOUND DISABLED");
	else	logf(LOG_INFO, "Sound initialised.");
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

	assert( m.who < nb_player);
	u = mobile.find(m.key);
	if (u) u->doShow();
	else logf(LOG_ERROR, "bosonCanvas::unHideMob : can't find m.key");

	// XXX emit something for the minimap
}



void bosonCanvas::createMob(mobileMsg_t &m)
{
	playerMobUnit *u;

	assert( m.who < nb_player);

	switch (m.type) {
		default:
			u = new playerMobUnit(&m);
			break;
		case MOB_MINERAL_HARVESTER:
		case MOB_OIL_HARVESTER:
			u = new harvesterUnit(&m);
			break;
	};

	mobile.insert(m.key, u);

	emit updateMobile(u);

	if (m.who == who_am_i)
		emit mobileNbUpdated(++my_mobiles);
}


void bosonCanvas::destroyMob(destroyedMsg_t &m)
{
	playerMobUnit *mob ;
	
	mob = mobile.find(m.key);
	if (!mob) {
		logf(LOG_ERROR, "bosonCanvas::destroyMob : can't find m.key");
		return;
	}

	if ( QPoint(m.x, m.y) != mob->gridRect().topLeft() )
		logf(LOG_WARNING, "Assertion failed file %s, line %d", __FILE__, __LINE__);

	QPoint p  = mob->center();
	new boShot ( p.x(), p.y(), mob->z(), boShot::SHOT_UNIT);

	if (mob->who == who_am_i)
		emit mobileNbUpdated(--my_mobiles);

	mob->s_destroy();
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

	assert( m.who < nb_player);
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

	if (m.who == who_am_i)
		emit facilityNbUpdated(++my_fix);
}


void bosonCanvas::destroyFix(destroyedMsg_t &msg)
{
	playerFacility * f;
	
	f = facility.find(msg.key);
	if (!f) {
		logf(LOG_ERROR, "bosonCanvas::destroyFix : can't find msg.key");
		return;
	}

	if ( QPoint(msg.x, msg.y) != f->gridRect().topLeft() )
		logf(LOG_WARNING, "Assertion failed file %s, line %d", __FILE__, __LINE__);

	QPoint p = f->center();
	new boShot ( p.x(), p.y(), f->z(), boShot::SHOT_FACILITY);

	if (f->who == who_am_i)
		emit facilityNbUpdated(--my_fix);

	f->s_destroy();
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

	m->s_moveTo(QPoint(msg.newx, msg.newy));
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
		mob->s_shooted(m.power);
	} else if (fix) {
		/* shooting a facility */
		fix->s_shooted(m.power);
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
}

void bosonCanvas::play(char *filename)
{
	if(soundserver->isNull()) return; // no sound available

	// XXX make a (char **) cache to speed this up
	// XXX human should really be handled by speciesTheme..
	QString path = *dataPath + "themes/sounds/human/" + filename;
//	logf(LOG_INFO, "(sound) playing : %s", path.latin1());
	soundserver->play(path.latin1());
}

/*
 * server-originated messages handling
 */

void bosonCanvas::msg_map_discovered(cooMsg_t coo)
{
	int i, j;

	logf(LOG_GAME_LOW, "received MSG_MAP_DISCOVERED : (%d,%d) = %d", coo.x, coo.y, (int)coo.c );

	setCell(coo.x, coo.y, coo.c);

	// handles this cell and all neighbour cells
	for(i=-1; i<2; i++)
		for(j=-1; j<2; j++)
			handleFOW(coo.x+i,coo.y+j);

}


void bosonCanvas::handleFOW(int x, int y)
{
	int i, j;

	if (!isValid(x,y)) return;

	if (cell(x,y).isFogged()) {
		if (cellKnown(x,y)) cell(x,y).unFog();
		return;
	}
	// the cell is unFogged after this line


	// we 'fog' cells that
	//  1) are unknown
	//  _and_
	//  2) have some known ground next to it
	if (cellKnown(x,y)) return;
	for(i=x-1; i<x+2; i++)
		for(j=y-1; j<y+2; j++) {
			if (!isValid(i,j)) break;
			if (cellKnown(i,j)) {
				cell(x,y).fog(x,y);
				return;
			} // if
		} // for
}




