/***************************************************************************
                         serverMap.cpp  -  description                              
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

#include <assert.h>

#include <qpainter.h>

#include "common/map.h"

#include "boserver.h"
#include "serverUnit.h"
#include "game.h"


void BosonServer::initMap(const char *mapfile)
{
mobile.resize(149);
facility.resize(149);
mobile.setAutoDelete(TRUE);
facility.setAutoDelete(TRUE);
key = 127; // why not ?

	assert (openRead(mapfile));
	///orzel, was ugly.. should handle openRead()==false correctly 
	worldName = new QString(_worldName);
	assert (loadGround());
}



void  BosonServer::checkUnitVisibility(Unit *u)
{
int		i,j, im,jm, iM,jM;
serverCell	*c;
cooMsg_t        coo;
ulong		mask = getPlayerMask(u->who);
int		dist = u->getVisibility();

int		x = u->gridRect().x(),
		y = u->gridRect().y();

im = QMAX(0, x-dist) - x;
iM = QMIN(map_width-1,	x+dist ) - x;
jm = QMAX(0, y-dist ) - y;
jM = QMIN(map_height-1,	y+dist ) - y;

// printf("checkUnitVisibility : im iM jm jM : %d %d %d %d\n", im, iM, jm, jM);

dist *= dist;

for (i=im ; i<=iM; i++)
    for (j=jm ; j<=jM; j++)
	if ( i*i+j*j < dist) {
		assert(i+x<map_width);
		assert(j+y<map_height);
		c = &cell( i+x, j+y );
		if ( ! c->isKnownBy(mask)) {
			c->setKnown(mask);
//			printf("checkUnitVisibility : cell(%d,%d) set to %x\n", i+x, j+y, (int)mask);
			/* here, send a message for every changed state */
			coo.x = i+x;
			coo.y = j+y;
			coo.c = c->_cell;
			if (GROUND_UNKNOWN != c->ground() )
				sendMsg (
					player[u->who].buffer,
					MSG_MAP_DISCOVERED,
					MSG(coo) );
			}
		}
}

void BosonServer::createMobUnit(mobileMsg_t &data)
{
	serverMobUnit	*u;

	logf(LOG_GAME_HIGH, "BosonServer::createMobUnit called");

	data.key = key;
	assert(data.who< BOSON_MAX_CONNECTION);
	assert(player[data.who].socketState==SSS_CONNECT_OK);

	switch(data.type) {
		default:
			u = new serverMobUnit(player[data.who].buffer, &data);
			break;
		case MOB_MINERAL_HARVESTER:
		case MOB_OIL_HARVESTER:
			u = new serverHarvester(player[data.who].buffer, &data);
			break;
	};


	placeMob(u);
	mobile.insert ( key++, u);
	checkUnitVisibility(u);
}

/*
 * basic placing function
 * should be smarter by veryfing that (x,y) is free, and else
 * try 'around' that place.
 */
void BosonServer::placeMob(serverMobUnit *u)
{
	ulong		k = 0l;
	int		i,j;
	QRect		r = u->gridRect();

	/* who is interested in knowing u's arrival */
	k = getPlayerMask(u->who);
	for (i=0; i<r.width(); i++)
		for (j=0; j<r.height(); j++)
			k |= cell( r.x()+i, r.y()+j).known;
	u->setKnown(k);

	/* telling them */
	u->reportCreated();
}


void BosonServer::createFixUnit(facilityMsg_t &data)
{
	serverFacility	*f;

	logf(LOG_GAME_HIGH, "BosonServer::createFixUnit called");

	data.key	= key;
	data.state	= 0;
	assert(data.who< BOSON_MAX_CONNECTION);
	assert(player[data.who].socketState==SSS_CONNECT_OK);

	f = new serverFacility(player[data.who].buffer, &data);

	placeFix(f);

	facility.insert ( key++, f);
	checkUnitVisibility(f);
}


void BosonServer::placeFix(serverFacility * f)
{
	ulong		k;
	int		i,j;
	QRect		r = f->gridRect();

	/* who is interested in knowing f's arrival */
	k = getPlayerMask(f->who);
	for (i=0; i<r.width(); i++)
		for (j=0; j<r.height(); j++)
			k |= cell( r.x()+i, r.y()+j).known;
	f->setKnown(k);

	/* telling them */
	f->reportCreated();
}


void BosonServer::requestAction()
{
	QIntDictIterator<serverMobUnit> mobIt(mobile);
	QIntDictIterator<serverFacility> fixIt(facility);

	for (fixIt.toFirst(); fixIt; ++fixIt)
		fixIt.current()->getWantedAction();
	for (mobIt.toFirst(); mobIt; ++mobIt)
		mobIt.current()->getWantedAction();
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
	int i,j;
	ulong	k = 0l, k2;
	QRect	r = f->gridRect();

	for (i=0; i<r.width(); i++)
		for (j=0; j<r.height(); j++)
			k |= cell( r.x()+i, r.y()+j).known;

	i=0; // 'i' is now the player index
	k2 = f->known;
	while ( k != k2) {
		boAssert(i<3);
		/* until the state is coherent between unit and ground */
		if ( (k&1l) == (k2&1l) ) {
			/* same state for user i, it's ok*/
			k>>=1; k2>>=1; i++; continue;
			}
		if ( k&1l) /* in this case the mobile should be known, but isn't */
			f->reportUnHidden(i);
		else {
			boAssert( k2&1l );
			/* the unit isn't known anymore */
			f->reportHidden(i);
			}
		k>>=1; k2>>=1; i++; // let's continue
		} /* while */

}


void BosonServer::checkMobileKnown(serverMobUnit *m)
{
	int i,j;
	ulong	k = 0l, k2;
	QRect	r = m->gridRect();

	boAssert(r.x()>=0); boAssert(r.y()>=0);
	boAssert(r.x()<200); boAssert(r.y()<200);

	for (i=0; i<r.width(); i++)
		for (j=0; j<r.height(); j++)
			k |= cell( r.x()+i, r.y()+j).known;

	i=0;
	k2 = m->known;
	while ( k != k2) {
		boAssert(i<3);
		/* until the state is coherent between unit and ground */
		if ( (k&1l) == (k2&1l) ) {
			/* same state for user i, it's ok*/
			k>>=1; k2>>=1; i++; continue;
			}
		if ( k&1l) /* in this case the mobile should be known, but isn't */
			m->reportUnHidden(i);
		else {
			boAssert( k2&1l );
			/* the unit isn't known anymore */
			m->reportHidden(i);
			}
		k>>=1; k2>>=1; i++; // let's continue
		} /* while */

}

bool BosonServer::loadGround()
{
	int i,j;

	/* creation of the ground map */
	cells = new serverCell[map_width*map_height];

	/* initialisation */
	for (i=0; i< map_width; i++)
		for (j=0; j< map_height; j++) {
			load ( cell( i, j)._cell );
		}
	
	/* checking */
	for (int i=0; i< 3; i++)
		for (int j=0; j< 3; j++)
			boAssert(0 <= cell(i,j).ground());

	return isOk();
}


bool BosonServer::loadUnits()
{
	
	int		i;
	mobileMsg_t	mob;
	facilityMsg_t	fix;

	for (i=0; i<nbMobiles; i++) {
		load(mob);
		createMobUnit(mob);
	}

	for (i=0; i<nbFacilities; i++) {
		load(fix);
		createFixUnit(fix);
	}

	Close();
	return isOk();
}





