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

#include "../map/map.h"
#include "serverCell.h"
#include "boserver.h"
#include "serverUnit.h"
#include "game.h"


void BosonServer::initMap(const char *mapfile)
{
int i,j;
playField field(mapfile);

mobile.resize(149);
facility.resize(149);
mobile.setAutoDelete(TRUE);
facility.setAutoDelete(TRUE);
key = 127; // why not ?

assert(true == field.load() );
///orzel, was ugly.. should handle load()==false correctly 

map.width = field.map.width;
map.height = field.map.height;

/* creation of the ground map */
map.cells = new (serverCell *)[map.width];
for (i=0; i< map.width; i++)
	map.cells[i] = new (serverCell)[map.height];

/* initialisation */
for (i=0; i< map.width; i++)
	for (j=0; j< map.height; j++)
		map.cells[i][j].setGroundType( field.map.cells[i][j]);


/* freeing of field.map.cells */
for (i=0; i< map.width; i++)
	delete [] field.map.cells[i];
delete [] field.map.cells;

/* checking */
for (int i=0; i< 3; i++)
	for (int j=0; j< 3; j++)
		boAssert(map.cells[i][j].known == 0l);


people	= field.people;
gpp.nbPlayer= field.nbPlayer;

assert(gpp.nbPlayer < 11);
assert(gpp.nbPlayer > 1);

}

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
if (u->inherits("mobUnit")) {
	x /= BO_TILE_SIZE;
	y /= BO_TILE_SIZE;
	}

im = QMAX(0, x-dist) - x;
iM = QMIN(map.width-1,	x+dist ) - x;
//iM = QMIN(map.width-1,	x+dist+u->getWidth()/BO_TILE_SIZE ) - x;
jm = QMAX(0, y-dist ) - y;
jM = QMIN(map.height-1,	y+dist ) - y;
//jM = QMIN(map.height-1	y+dist+u->getHeight()/BO_TILE_SIZE ) - y;

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
				gpp.player[u->who].buffer,
				MSG_MAP_DISCOVERED,
				sizeof(data.coo), &data);
			}
		}
}

void BosonServer::createMobUnit(uint who, uint x, uint y, mobType type)
{
bosonMsgData	data;
serverMobUnit	*u;
ulong	k = 0l;
int	i,j, i2,j2;

logf(LOG_GAME_HIGH, "BosonServer::createMobUnit called");

data.mobile.who = who; 
data.mobile.key = key;
data.mobile.x = x;
data.mobile.y = y;
data.mobile.type = type;
assert(who< BOSON_MAX_CONNECTION);
assert(gpp.player[who].socketState==SSS_CONNECT_OK);

u = new serverMobUnit(gpp.player[who].buffer, &data.mobile);

/* who is interested in knowing u's arrival */
x /= BO_TILE_SIZE;
y /= BO_TILE_SIZE;
/* puts("hop1");
printf("type is %d, width is %d\n", type, mobileProp[type].width);
printf("u= %p\n", u); */
i2 = (u->getWidth() + BO_TILE_SIZE -1 ) / BO_TILE_SIZE;
j2 = (u->getHeight() + BO_TILE_SIZE -1 )/ BO_TILE_SIZE;
/*i2 = j2 = 1; 
puts("hop2"); */
k = getPlayerMask(who);
for (i=0; i<i2; i++)
	for (j=0; j<j2; j++)
		k |= map.cells[x+i][y+j].known;
u->setKnown(k);

/* telling them */
u->sendToKnown(MSG_MOBILE_CREATED, sizeof(data.mobile), &data);

mobile.insert ( key++, u);
checkUnitVisibility(u);
}



void BosonServer::createFixUnit(uint who, uint x, uint y, facilityType type)
{
bosonMsgData	data;
serverFacility	*f;
int		i,j, i2, j2;
ulong		k;

logf(LOG_GAME_HIGH, "BosonServer::createFixUnit called");

data.facility.who	= who;
data.facility.key	= key;
data.facility.x		= x;
data.facility.y		= y;
data.facility.type	= type;
data.facility.state	= 0;
assert(who< BOSON_MAX_CONNECTION);
assert(gpp.player[who].socketState==SSS_CONNECT_OK);

f = new serverFacility(gpp.player[who].buffer, &data.facility);

/* who is interested in knowing u's arrival */
i2 = facilityProp[type].width;
j2 = facilityProp[type].height;

k = getPlayerMask(who);
for (i=0; i<i2; i++)
	for (j=0; j<j2; j++)
		k |= map.cells[x+i][y+j].known;
f->setKnown(k);

/* telling them */
for ( i=0; k; i++,k>>=1) {
	boAssert(i<4);
	if (k&1l) sendMsg (
		gpp.player[i].buffer,	MSG_FACILITY_CREATED,
		sizeof(data.facility),	&data);
	}

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

void BosonServer::checkKnownState()
{
QIntDictIterator<serverFacility> fixIt(facility);
QIntDictIterator<serverMobUnit> mobIt(mobile);

for (fixIt.toFirst(); fixIt; ++fixIt)
	checkFixKnown(fixIt.current());

for (mobIt.toFirst(); mobIt; ++mobIt)
	checkMobileKnown(mobIt.current());

}

void BosonServer::checkFixKnown(serverFacility *f)
{
	int x,y;
	int i,j, i2, j2;
	ulong	k = 0l, k2;

	x = f->_x();
	y = f->_y();
	///orzel : ugly
	i2 = f->getWidth() / BO_TILE_SIZE;
	j2 = f->getHeight() / BO_TILE_SIZE;

	for (i=0; i<i2; i++)
		for (j=0; j<j2; j++)
			k |= map.cells[x+i][y+j].known;

	i=0;
	k2 = f->known;
	while ( k != k2) {
		boAssert(i<3);
		/* until the state is coherent between unit and ground */
		if ( (k&1l) == (k2&1l) ) {
			/* same state for user i, it's ok*/
			k>>=1; k2>>=1; i++; continue;
			}
		if ( k&1l) {
			/* in this case the mobile should be known, but isn't */
puts("discovering machin");
			f->reportCreated( gpp.player[i].buffer);
			f->setKnown(getPlayerMask(i));
			}
		else {
			/* the unit isn't known anymore */
			f->reportDestroyed( gpp.player[i].buffer);
			f->unSetKnown(getPlayerMask(i));
			}
		k>>=1; k2>>=1; i++; // let's continue
		} /* while */

}


void BosonServer::checkMobileKnown(serverMobUnit *m)
{
	int x,y;
	int i,j, i2, j2;
	ulong	k = 0l, k2;


	x = m->_x() / BO_TILE_SIZE;
	y = m->_y() / BO_TILE_SIZE;
	i2 = (m->getWidth() + BO_TILE_SIZE -1 ) / BO_TILE_SIZE;
	j2 = (m->getHeight() + BO_TILE_SIZE -1 )/ BO_TILE_SIZE;

	for (i=0; i<i2; i++)
		for (j=0; j<j2; j++)
			k |= map.cells[x+i][y+j].known;

	i=0;
	k2 = m->known;
	while ( k != k2) {
		boAssert(i<3);
		/* until the state is coherent between unit and ground */
		if ( (k&1l) == (k2&1l) ) {
			/* same state for user i, it's ok*/
			k>>=1; k2>>=1; i++; continue;
			}
		if ( k&1l) {
			/* in this case the mobile should be known, but isn't */
			m->reportCreated( gpp.player[i].buffer);
			m->setKnown(getPlayerMask(i));
			}
		else {
			/* the unit isn't known anymore */
			m->reportDestroyed( gpp.player[i].buffer);
			m->unSetKnown(getPlayerMask(i));
			}
		k>>=1; k2>>=1; i++; // let's continue
		} /* while */

}




