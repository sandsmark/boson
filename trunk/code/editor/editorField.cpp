/***************************************************************************
                          editorField.cpp  -  description                              
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

//#include <assert.h>

//#include <kapp.h>
//#include <kmsgbox.h>

#include "../common/log.h"
//#include "../common/boconfig.h" // MAX_PLAYERS
//#include "../common/map.h"

#include "editorField.h"
  
editorField::editorField(uint w, uint h, QObject *parent, const char *name=0L)
	: visualField(w,h,parent,name)
{
	mobiles.resize(149);
	facilities.resize(149);
	mobiles.setAutoDelete(TRUE);
	facilities.setAutoDelete(TRUE);   
	key = 253;
}


bool editorField::load(QString filename)
{
	int i,j;
	mobileMsg_t	mob;
	facilityMsg_t	fix;
	Cell		c;
	static	bool	cells_allocated = false;

	if (cells_allocated) freeRessources();

	if (!openRead(filename.data())) return false;

	/* creation of the ground map */
	cells = new (visualCell *)[map_width];
	for (i=0; i< map_width; i++)
		cells[i] = new (visualCell)[map_height];

	cells_allocated = true;
	
	/* initialisation */
	for (i=0; i< map_width; i++)
		for (j=0; j< map_height; j++) {
			boFile::load( c);
			if (c.getGroundType() != GROUND_UNKNOWN) {
				cells[i][j].set(c.getGroundType(), i, j);
				cells[i][j].setFrame(c.getItem());
			} else {
				// orzel : ugly, should be handled a _lot_ more nicely
				cells[i][j].set( GROUND_WATER, i, j);
				cells[i][j].z( Z_INVISIBLE);
				cells[i][j].set( GROUND_UNKNOWN);
			}
		}
	
	/* checking */
	for (int i=0; i< 3; i++)
		for (int j=0; j< 3; j++)
			boAssert(0 <= cells[i][j].getGroundType());


	for (i=0; i<nbMobiles; i++) {
		boFile::load(mob);
		if (!isOk()) return false;
		createMobUnit(mob);
	}

	for (i=0; i<nbFacilities; i++) {
		boFile::load(fix);
		if (!isOk()) return false;
		createFixUnit(fix);
	}

	// ok, it's all right
	Close();
	modified = false;
	update();
	return isOk();
}


bool editorField::save(QString filename)
{
	int i,j;
	mobileMsg_t	mob;
	facilityMsg_t	fix;
	QIntDictIterator<visualMobUnit> mobIt(mobiles);
	QIntDictIterator<visualFacility> fixIt(facilities);

	boAssert (nbMobiles == mobiles.count());
	boAssert (nbFacilities == facilities.count());

	if (!openWrite(filename.data())) return false;

	/* initialisation */
	for (i=0; i< map_width; i++)
		for (j=0; j< map_height; j++)
			write(cells[i][j]);
	
	for (mobIt.toFirst(); mobIt; ++mobIt) {
		mobIt.current()->fill(mob);
		boFile::write(mob);
		}

	for (fixIt.toFirst(); fixIt; ++fixIt) {
		fixIt.current()->fill(fix);
		boFile::write(fix);
		}


	// ok, it's all right
	Close();
	modified = false;
	return isOk();
}


void editorField::freeRessources()
{
	int i;

	/* freeing of cells */
	for (i=0; i< map_width; i++)
		delete [] cells[i];
	delete [] cells;

	/* freeing of mobiles */
	mobiles.clear();
	/* freeing of facilities */
	facilities.clear();
}


void editorField::createMobUnit(mobileMsg_t &msg)
{
	visualMobUnit *m;

	msg.key = key++;

	m = new visualMobUnit(&msg);
	mobiles.insert(msg.key, m);

//	emit updateMobile(m);
}


/*
void editorField::destroyMobUnit(destroyedMsg_t &msg)
{
	visualMobUnit *mob ;
	
	mob = mobile.find(msg.key);
	if (mob) {
		boAssert(msg.x == mob->x());
		boAssert(msg.y == mob->y());
		}
	else {
		logf(LOG_ERROR, "bosonField::destroyMob : can't find msg.key");
		return;
		}

	boAssert( mobile.remove(msg.key) == true );
}
*/

void editorField::createFixUnit(facilityMsg_t &msg)
{
	visualFacility *f;

	msg.key = key ++;
	msg.state = CONSTRUCTION_STEP - 1 ;

	f = new visualFacility(&msg);
	facilities.insert(msg.key, f);

//	emit updateFix(f);
}


/*
void editorField::destroyFixUnit(destroyedMsg_t &msg)
{
	visualFacility * f;
	
	f = facility.find(msg.key);
	if (f) {
		boAssert(msg.x == f->x());
		boAssert(msg.y == f->x());
		}
	else {
		logf(LOG_ERROR, "bosonField::destroyFix : can't find msg.key");
		return;
		}

	boAssert( facility.remove(msg.key) == true);
}
*/


void editorField::deleteCell(int x, int y)
{
	groundType	oldg =  cells[x][y].getGroundType();
	

	/* deal with improper value */
	if ( x<0 || y<0) return;

// debugging recursive behaviour :-)
//	static int i = 20;
//	if (i--<0) exit(-1);


//	printf ("deleting %d,%d\n", x, y);


	/* actually delete the cell */
	cells[x][y].set(GROUND_UNKNOWN);
	cells[x][y].z( Z_INVISIBLE);

	/* deal with big tiles */
	if ( IS_BIG_TRANS( oldg )) {
		/* sanity check */
		if ( x+1>=maxX || y+1>=maxY) {
			logf(LOG_ERROR, "deleteCell : out of bound big tile...");
			return;
		}
		/* delte children */
		deleteCell(x,y+1);
		deleteCell(x+1,y);
		deleteCell(x+1,y+1);
	}
	
	/* deal with alread UNKNOWN tiles */
	if ( GROUND_UNKNOWN == oldg ) {
		if ( IS_BIG_TRANS (cells[x-1][y].getGroundType()) )
			deleteCell(x-1,y);
		else if ( IS_BIG_TRANS (cells[x][y-1].getGroundType()) )
			deleteCell(x,y-1);
		else if ( IS_BIG_TRANS (cells[x-1][y-1].getGroundType()) )
			deleteCell(x-1,y-1);
		return;
	}
}

void editorField::setCell(int x, int y, groundType g )
{
//	printf ("setting %d,%d at %d\n", x, y, g);
	if ( IS_BIG_TRANS(g) || IS_BIG_TRANS( cells[x][y].getGroundType() ) ) {
		deleteCell(x,y+1);
		deleteCell(x+1,y);
		deleteCell(x+1,y+1);
	}
	cells[x][y].set(g);
}

