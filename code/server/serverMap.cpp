/***************************************************************************
                         serverMap.cpp  -  description                              
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

#include <qpainter.h>
#include "boserver.h"
#include "../map/serverCell.h"
#include "serverUnit.h"
#include "../map/map.h"
#include "../map/playField.h"
//#include "../common/boconnect.h"

void BosonServer::initMap(const char *mapfile)
{
//uint i,j,d;

playField field(mapfile);

mobile.resize(149);
facility.resize(149);
mobile.setAutoDelete(TRUE);
facility.setAutoDelete(TRUE);
key = 127; // why not ?

assert(true == field.load() );

map	= field.map;
people	= field.people;
nbPlayer= field.nbPlayer;

assert(nbPlayer < 11);
assert(nbPlayer > 1);






#define CENTER (200/2)
#define R1 (40*2)
#define R2 (10*2)
/*

map.width = w; map.height = h;
map.cells = new (serverCell *)[w*h];

for (i=0; i < w; i++)
   for (j=0; j < h; j++) {
	d = (i-CENTER)*(i-CENTER) + (j-CENTER)*(j-CENTER);
	if (d>R1*R1) map.cells[coo2index(i,j)] = new serverCell(GROUND_SEA);
	else if (d>R2*R2) map.cells[coo2index(i,j)] = new serverCell(GROUND_PLAIN);
	else map.cells[coo2index(i,j)] = new serverCell(GROUND_DESERT);
	}
*/
}
#undef R2
#undef R1
#undef CENTER


void  BosonServer::checkUnitVisibility(Unit *u)
{
int		i,j, im,jm, iM,jM;
serverCell	*c;
bosonMsgData	data;
int		dist = u->getVisibility();
ulong		mask = getPlayerMask(u->who);

int		x = u->_x(),
		y = u->_y();

/* mobile don't need to fit the grid-layout */
//printf("checking ");
if (u->inherits("mobUnit")) {
//	printf("mobile ");
	x /= BO_TILE_SIZE;
	y /= BO_TILE_SIZE;
	}
//else printf("facility ");
//printf("at %d,%d\n", x, y);

im = QMAX(0, x-dist)	- x;
iM = QMIN(map.width-1, x+dist)	- x;
jm = QMAX(0, y-dist)	- y;
jM = QMIN(map.height-1, y+dist)	- y;

//printf("im iM jm jM : %d %d %d %d\n", im, iM, jm, jM);

dist *= dist;

for (i=im ; i<=iM; i++)
    for (j=jm ; j<=jM; j++)
	if ( i*i+j*j < dist) {
		assert(i+x<map.width);
		assert(j+y<map.height);
		c = &map.cells[i+x][j+y];
		if ( ! c->isKnownBy(mask)) {
			c->setKnown(mask);
			/* here, send a message for every changed state */
			data.coo.x = i+x;
			data.coo.y = j+y;
			data.coo.g = c->getGroundType();
			sendMsg (
				player[u->who].buffer,
				MSG_MAP_DISCOVERED,
				sizeof(data.coo), &data);
			}
		}
//	else printf("(%d,%d) not discovered\n", i, j);

///orzel : should we ? 
//player[0].buffer->flush();

}

void BosonServer::createMobUnit(uint who, uint x, uint y, mobType type)
{
bosonMsgData	data;
serverMobUnit	*u;

logf(LOG_GAME_HIGH, "BosonServer::createMobUnit called");

data.mobile.who = who; 
data.mobile.key = key;
data.mobile.x = x;
data.mobile.y = y;
data.mobile.type = type;
assert(who< BOSON_MAX_CONNECTION);
assert(player[who].socketState==SSS_CONNECT_OK);

printf("create : who = %d\n", who);
u = new serverMobUnit(player[who].buffer, &data.mobile);
printf("create : who = %d\n", who);

mobile.insert ( key++, u);
checkUnitVisibility(u);
}


void BosonServer::createFixUnit(uint who, uint x, uint y, facilityType type)
{
bosonMsgData	data;
serverFacility	*f;

logf(LOG_GAME_HIGH, "BosonServer::createFixUnit called");

data.facility.who	= who;
data.facility.key	= key;
data.facility.x		= x;
data.facility.y		= y;
data.facility.type	= type;
assert(who< BOSON_MAX_CONNECTION);
assert(player[who].socketState==SSS_CONNECT_OK);

f = new serverFacility(player[who].buffer, &data.facility);

facility.insert ( key++, f);
checkUnitVisibility(f);
}



void BosonServer::requestAction()
{
//QIntDictIterator<serverMobUnit> mobIt(mobile);
QIntDictIterator<serverFacility> fixIt(facility);

for (fixIt.toFirst(); fixIt; ++fixIt)
	fixIt.current()->getWantedAction();
}
