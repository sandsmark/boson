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

#include "common/map.h"

editorCanvas::editorCanvas(QPixmap p)
	: visualCanvas(p, 50, 50) // XXXX hardcoded until QCanvas allow on-the-fly creation
{
	mobiles.resize(149);
	facilities.resize(149);
	mobiles.setAutoDelete(TRUE);
	facilities.setAutoDelete(TRUE);   
	key = 253;
}


bool editorCanvas::Load(QString filename)
{
	int i,j;
	mobileMsg_t	mob;
	facilityMsg_t	fix;
	cell_t		c;

	freeRessources();

	if (!openRead(filename.data())) return false;
	

	/* initialisation */
	for (i=0; i< map_width; i++)
		for (j=0; j< map_height; j++) {
//			printf("loading cell %d,%d\n", i, j );
			boFile::load( c);
//			printf("(%3d,%3d) : ", i, j);
			setCell( i, j, c);
//			printf("%d\n", c);
/*			if (c.getGroundType() != GROUND_UNKNOWN)
			else {
				// orzel : ugly, should be handled a _lot_ more nicely
				cells[i][j].set( GROUND_WATER, i, j);
				cells[i][j].setZ( Z_INVISIBLE);
				cells[i][j].set( GROUND_UNKNOWN);
				boAssert (
					(i>0 && IS_BIG_TRANS(cells[i-1][j].getGroundType())) ||
					(j>0 && IS_BIG_TRANS(cells[i][j-1].getGroundType())) ||
					(i>0 && j>0 && IS_BIG_TRANS(cells[i-1][j-1].getGroundType())) );
			}
*/
		}
	
	/* checking */
	for (int i=0; i< 3; i++)
		for (int j=0; j< 3; j++)
			boAssert ( ground (tile(i,j)) );


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
			write( (cell_t) tile(i,j));
	
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

/*
	// XXX orzel : temp, until GUI is really functionnal 
	QString themePath = KGlobal::instance()->dirs()->findResourceDir("data", "boson/map/basic.bpf");
	themePath	+= "boson/themes/grounds/earth.bmp";
	printf("loading groundTheme : %s\n", themePath.latin1() );
	QPixmap *p = new QPixmap(themePath);
	if (p->isNull() ) {
		printf("can't load earth.jpeg\n");
		exit(1);
	}

	::visualCanvas(*p,w,h);
*/
	
	/* boFile configuration */
	nbPlayer = 2;  // XXX still hardcoded .......
	map_width = w;
	map_height = h;
	_worldName = name;

	/* initialisation */
	for (i=0; i< map_width; i++)
		for (j=0; j< map_height; j++)
			esetCell( i, j, cell ( fill_ground, (3*i+5*j)%4 ));

	modified = true;
	
	return true;
}


void editorCanvas::freeRessources()
{
	/* freeing of mobiles */
	mobiles.clear();
	/* freeing of facilities */
	facilities.clear();
	modified = false;
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
	groundType	oldg =  ground ( tile(x,y) );
	

	/* deal with improper value */
	if ( x<0 || y<0) return;

// debugging recursive behaviour :-)
//	static int i = 20;
//	if (i--<0) exit(-1);


//	printf ("deleting %d,%d\n", x, y);


	/* actually delete the cell */
	modified = true;
	esetCell(x,y, cell(GROUND_UNKNOWN, 0) );

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
		if ( IS_BIG_TRANS ( ground(tile(x-1,y))))
			deleteCell(x-1,y);
		else if ( IS_BIG_TRANS ( ground(tile(x,y-1))))
			deleteCell(x,y-1);
		else if ( IS_BIG_TRANS ( ground(tile(x-1,y-1))))
			deleteCell(x-1,y-1);
		return;
	}
}

void editorCanvas::esetCell(int x, int y, cell_t c)
{
//	printf ("setting %d,%d at %d\n", x, y, g);
	modified = true;
	if ( IS_BIG_TRANS( ground(c) ) || IS_BIG_TRANS( ground(tile(x,y))) ) {
		deleteCell(x,y+1);
		deleteCell(x+1,y);
		deleteCell(x+1,y+1);
	}
	visualCanvas::setCell(x,y,c);
}

