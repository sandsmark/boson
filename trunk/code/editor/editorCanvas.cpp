/***************************************************************************
                          editorCanvas.cpp  -  description                              
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

#include "common/log.h"

#include "editorCanvas.h"
#include "visualCell.h"
  
editorCanvas::editorCanvas()
	: visualCanvas()
{
	mobiles.resize(149);
	facilities.resize(149);
	mobiles.setAutoDelete(TRUE);
	facilities.setAutoDelete(TRUE);   
	key = 253;

	cells_allocated = false;
}


bool editorCanvas::Load(QString filename)
{
	int i,j;
	mobileMsg_t	mob;
	facilityMsg_t	fix;
	Cell		c;

	freeRessources();

	if (!openRead(filename.data())) return false;
	

	puts("hop1");
	
	resize(map_width, map_height);
	puts("hop2");

	/* creation of the ground map */
	cells = new (visualCell *)[map_width];
	for (i=0; i< map_width; i++)
		cells[i] = new (visualCell)[map_height];
	cells_allocated = true;

	
	puts("hop3");
	/* initialisation */
	for (i=0; i< map_width; i++)
		for (j=0; j< map_height; j++) {
//			printf("loading cell %d,%d\n", i, j );
			boFile::load( c);
			// bypassing it until qcanvas-port-related bug is done
			continue;
			if (c.getGroundType() != GROUND_UNKNOWN) {
				cells[i][j].set(c.getGroundType(), i, j);
				cells[i][j]._setFrame(c.getItem());
			} else {
				// orzel : ugly, should be handled a _lot_ more nicely
				cells[i][j].set( GROUND_WATER, i, j);
				cells[i][j].setZ( Z_INVISIBLE);
				cells[i][j].set( GROUND_UNKNOWN);
				boAssert (
					(i>0 && IS_BIG_TRANS(cells[i-1][j].getGroundType())) ||
					(j>0 && IS_BIG_TRANS(cells[i][j-1].getGroundType())) ||
					(i>0 && j>0 && IS_BIG_TRANS(cells[i-1][j-1].getGroundType())) );
			}
		}
	puts("hop4");
	
	/* checking */
	for (int i=0; i< 3; i++)
		for (int j=0; j< 3; j++)
			boAssert(0 <= cells[i][j].getGroundType());

	puts("hop5");

	for (i=0; i<nbMobiles; i++) {
		boFile::load(mob);
		if (!isOk()) return false;
		createMobUnit(mob);
	}
	puts("hop6");

	for (i=0; i<nbFacilities; i++) {
		boFile::load(fix);
		if (!isOk()) return false;
		createFixUnit(fix);
	}

	puts("hop7");
	// ok, it's all right
	Close();
	modified = false;
	update();
	return isOk();
}


bool editorCanvas::Save(QString filename)
{
	int i,j;
	mobileMsg_t	mob;
	facilityMsg_t	fix;
	QIntDictIterator<visualMobUnit> mobIt(mobiles);
	QIntDictIterator<visualFacility> fixIt(facilities);

	nbMobiles = (int)mobiles.count();
	nbFacilities = (int)facilities.count();

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


bool editorCanvas::New(groundType fill_ground, uint w, uint h, const QString &name)
{
	int i,j;

	freeRessources();

	/* QCanvas configuratoin */
	resize(w,h);
	
	/* boFile configuration */
	nbPlayer = 2;  // XXX still hardcoded .......
	map_width = w;
	map_height = h;
	_worldName = name;

	/* creation of the ground map */
	cells = new (visualCell *)[map_width];
	for (i=0; i< map_width; i++)
		cells[i] = new (visualCell)[map_height];
	cells_allocated = true;
	
	/* initialisation */
	for (i=0; i< map_width; i++)
		for (j=0; j< map_height; j++) {
			cells[i][j].set( fill_ground, i, j);
			cells[i][j]._setFrame((3*i+5*j)%4);
		}

	modified = true;
	
	return true;
}


void editorCanvas::freeRessources()
{
	int i;

	if (!cells_allocated) return;

	/* freeing of cells */
	for (i=0; i< map_width; i++)
		delete [] cells[i];
	delete [] cells;

	/* freeing of mobiles */
	mobiles.clear();
	/* freeing of facilities */
	facilities.clear();
	modified = false;
	
	cells_allocated = false;
}


void editorCanvas::createMobUnit(mobileMsg_t &msg)
{
	visualMobUnit *m;

	msg.key = key++;

	m = new visualMobUnit(&msg);
	mobiles.insert(msg.key, m);

	modified = true;
//	emit updateMobile(m);
}


/*
void editorCanvas::destroyMobUnit(destroyedMsg_t &msg)
{
	visualMobUnit *mob ;
	
	mob = mobile.find(msg.key);
	if (mob) {
		boAssert(msg.x == mob->x());
		boAssert(msg.y == mob->y());
		}
	else {
		logf(LOG_ERROR, "editorCanvas::destroyMob : can't find msg.key");
		return;
		}

	boAssert( mobile.remove(msg.key) == true );
}
*/

void editorCanvas::createFixUnit(facilityMsg_t &msg)
{
	visualFacility *f;

	msg.key = key ++;
	msg.state = CONSTRUCTION_STEP - 1 ;

	f = new visualFacility(&msg);
	facilities.insert(msg.key, f);

	modified = true;
//	emit updateFix(f);
}


/*
void editorCanvas::destroyFixUnit(destroyedMsg_t &msg)
{
	visualFacility * f;
	
	f = facility.find(msg.key);
	if (f) {
		boAssert(msg.x == f->x());
		boAssert(msg.y == f->x());
		}
	else {
		logf(LOG_ERROR, "editorCanvas::destroyFix : can't find msg.key");
		return;
		}

	boAssert( facility.remove(msg.key) == true);
}
*/


void editorCanvas::deleteCell(int x, int y)
{
	groundType	oldg =  cells[x][y].getGroundType();
	

	/* deal with improper value */
	if ( x<0 || y<0) return;

// debugging recursive behaviour :-)
//	static int i = 20;
//	if (i--<0) exit(-1);


//	printf ("deleting %d,%d\n", x, y);


	/* actually delete the cell */
	modified = true;
	cells[x][y].set(GROUND_UNKNOWN);
	cells[x][y].setZ( Z_INVISIBLE);

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

void editorCanvas::setCell(int x, int y, groundType g )
{
//	printf ("setting %d,%d at %d\n", x, y, g);
	modified = true;
	if ( IS_BIG_TRANS(g) || IS_BIG_TRANS( cells[x][y].getGroundType() ) ) {
		deleteCell(x,y+1);
		deleteCell(x+1,y);
		deleteCell(x+1,y+1);
	}
	cells[x][y].set(g);
}

