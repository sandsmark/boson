/***************************************************************************
                          editorMap.cpp  -  description                              
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
#include "../common/playField.h"
#include "../common/boconfig.h" // MAX_PLAYERS
#include "../map/map.h"

#include "editorMap.h"
  
editorMap::editorMap(uint w, uint h, QObject *parent, const char *name=0L)
	: physMap(w,h,parent,name)
{
int	i, j;
playField field("/opt/kde/share/apps/boson/map/basic.bpf");

assert(true == field.load() );
///orzel, was ugly.. should handle load()==false correctly 

map.width = field.map.width;
map.height = field.map.height;

/* creation of the ground map */
map.cells = new (visualCell *)[map.width];
for (i=0; i< map.width; i++)
	map.cells[i] = new (visualCell)[map.height];

/* initialisation */
for (i=0; i< map.width; i++)
	for (j=0; j< map.height; j++)
		map.cells[i][j].set( field.map.cells[i][j], i, j);


/* freeing of field.map.cells */
for (i=0; i< map.width; i++)
	delete [] field.map.cells[i];
delete [] field.map.cells;

}


/*
void editorMap::setCell(int i, int j, groundType g)
{
	boAssert(i>=0); boAssert(j>=0);
	boAssert(i<width()); boAssert(j<height());

	(void) new playerCell(g, i, j);

	emit newCell(i,j,g);
}
*/


void editorMap::createMob(mobileMsg_t &m)
{
	visualMobUnit *u;

	assert(m.who < BOSON_MAX_PLAYERS);

	u = new visualMobUnit(&m);
	mobile.insert(m.key, u);

	emit updateMobile(u);
}


void editorMap::destroyMob(destroyedMsg_t &m)
{
	visualMobUnit *mob ;
	
	mob = mobile.find(m.key);
	if (mob) {
		boAssert(m.x == mob->x());
		boAssert(m.y == mob->y());
		}
	else {
		logf(LOG_ERROR, "editorMap::destroyMob : can't find m.key");
		return;
		}

	boAssert( mobile.remove(m.key) == true );
}

void editorMap::createFix(facilityMsg_t &m)
{
	visualFacility *f;

	assert(m.who < BOSON_MAX_PLAYERS);

	f = new visualFacility(&m);
	facility.insert(m.key, f);

	emit updateFix(f);
}


void editorMap::destroyFix(destroyedMsg_t &m)
{
	visualFacility * f;
	
	f = facility.find(m.key);
	if (f) {
		boAssert(m.x == f->x());
		boAssert(m.y == f->x());
		}
	else {
		logf(LOG_ERROR, "editorMap::destroyFix : can't find m.key");
		return;
		}

	boAssert( facility.remove(m.key) == true);
}
