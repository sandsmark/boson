/***************************************************************************
                          physMap.cpp  -  description                              
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

#include <assert.h>

#include <kapp.h>

#include "../common/log.h"
#include "../common/boconfig.h"
#include "../map/map.h"

#include "physMap.h"
#include "speciesTheme.h"
#include "groundTheme.h"
#include "game.h"
  
physMap::physMap(uint w, uint h, QObject *parent, const char *name=0L)
	: QObject(parent, name)
	, QwSpriteField (w * BO_TILE_SIZE ,h * BO_TILE_SIZE)
{

/* map geometry */
maxX = w; maxY = h;

/* Dictionaries */
mobile.resize(149);
facility.resize(149);
mobile.setAutoDelete(TRUE);
facility.setAutoDelete(TRUE);

/* Themes selection (should be moved thereafter) */
gameProperties.ground		= new groundTheme("ben");
gameProperties.species[1]	= new speciesTheme("Blue");
gameProperties.species[0]	= new speciesTheme("Red");

}


physMap::~physMap()
{
}


void physMap::setCell(int i, int j, groundType g)
{
	boAssert(i>=0); boAssert(j>=0);
	boAssert(i<width()); boAssert(j<height());

	(void) new playerCell(g, i, j);

	emit newCell(i,j,g);
}


void physMap::createMob(mobileMsg_t &m)
{
	playerMobUnit *u;

	assert(m.who < BOSON_MAX_PLAYERS);

	u = new playerMobUnit(&m);
	mobile.insert(m.key, u);

	emit updateMobile(u);
}


void physMap::createFix(facilityMsg_t &m)
{
	playerFacility *f;

	assert(m.who < BOSON_MAX_PLAYERS);

	f = new playerFacility(&m);
	facility.insert(m.key, f);

	emit updateFix(f);
}


void physMap::move(moveMsg_t &m)
{
	mobile.find(m.key)->s_moveBy(m.dx, m.dy);
}

void physMap::requestAction(boBuffer *buffer)
{
	QIntDictIterator<playerMobUnit> mobIt(mobile);
	//QIntDictIterator<playerFacility> fixIt(facility);
	int		dx, dy;
	bosonMsgData	data;

	for (mobIt.toFirst(); mobIt; ++mobIt) {
		if (mobIt.current()->getWantedMove(dx,dy)){
			data.move.key	= mobIt.currentKey();
			data.move.dx	= dx;
			data.move.dy	= dy;
			sendMsg(buffer, MSG_MOBILE_MOVE_R, sizeof(data.move), &data);
			}
		}
}
