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

//#include <qpainter.h>
#include <kapp.h>
#include <assert.h>
#include "../common/log.h"
#include "../map/map.h"
#include "physMap.h"
//#include "playerUnit.h"
#include "speciesTheme.h"
#include "groundTheme.h"
#include "game.h"
  
physMap::physMap(uint w, uint h, QObject *parent, const char *name=0L)
	: QObject(parent, name)
	, QwSpriteField (w * BO_TILE_SIZE ,h * BO_TILE_SIZE)
{

/* map geometry */
maxX = w; maxY = h;

/* Cells initialization */
//uint i;
//cells = new (playerCell *)[w*h];
//for (i=0; i < w*h; i++) cells[i] = new playerCell();

/* Dictionaries */
mobile.resize(149);
facility.resize(149);
mobile.setAutoDelete(TRUE);
facility.setAutoDelete(TRUE);

/* Themes selection (should be moved thereafter) */
gameProperties.ground		= new groundTheme("ben");

gameProperties.species[1]	=
gameProperties.myspecies	= ///orzel : hum....
gameProperties.species[0]	= new speciesTheme("ben");

}

physMap::~physMap()
{
}

void physMap::setCell(int i, int j, groundType g) //, bool redraw)
{
boAssert(i>=0); boAssert(j>=0);
boAssert(i<width()); boAssert(j<height());

//cells[coo2index(i,j)]->setGroundType(g);

(void) new playerCell(g, i, j);

emit newCell(i,j,g);
/* if (redraw){
	emit updateCell(i,j);
	} */
//drawCell(i,j);
}

void physMap::createMob(mobileMsg_t &m)
{
playerMobUnit *u;

assert(m.who < MAX_PLAYER);

u = new playerMobUnit(&m);
mobile.insert(m.key, u);

//drawMobile(u);
emit updateMobile(u);
}

void physMap::createFix(facilityMsg_t &m)
{
playerFacility *f;

assert(m.who < MAX_PLAYER);

f = new playerFacility(&m);
facility.insert(m.key, f);

/* map cleaning */
/*
int	i,j;
for(i=0; i< facilityProp[m.type].width; i++)
	for(j=0; j< facilityProp[m.type].height; j++)
		cells[coo2index(m.x+i,m.y+j)]->setGroundType(GROUND_FACILITY);
*/

//drawFix(f);
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

/*!
Reimplements QwSpriteField::drawBackground to draw the image
as the background.
*/
/*
void physMap::drawBackground(QPainter& painter, const QRect& area)
{
	int x,y;

//	painter.drawRect(area);

printf("drawBackground %d.%d %dx%d\n", area.x(), area.y(), area.width(), area.height());
//	return;
	for (x=area.x()/BO_TILE_SIZE;
		x<(area.x()+area.width()+BO_TILE_SIZE-1)/BO_TILE_SIZE; x++) {
	for (y=area.y()/BO_TILE_SIZE;
		y<(area.y()+area.height()+BO_TILE_SIZE-1)/BO_TILE_SIZE; y++) {
		painter.drawPixmap(x*BO_TILE_SIZE, y*BO_TILE_SIZE, *gameProperties.ground->getPixmap(getGround(x,y)));
		}
	}
}
*/
