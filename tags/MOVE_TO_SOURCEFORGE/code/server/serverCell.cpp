/***************************************************************************
                       serverCell.cpp  -  description                    
                             -------------------                                         

    version              : $Id$
    begin                : Sat Nov  4 03:49:47 EST 2000
                                           
    copyright            : (C) 2000 by Thomas Capricelli                         
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

#include "serverCell.h"
#include "serverUnit.h"
 
serverCellMap::serverCellMap(void)
	:CellMap( -1, -1)
{
}

 
bool serverCellMap::findFreePos(int &x, int &y, mobType t)
{
	int i, j;

	if (testFreePos(x,y,t)) return true; // for *.bpf-loaded mobiles, already in place

	for (i=-5; i<6; i++) // X position we try
		for (j=-5; j<6; j++) // Y position we try
			if (testFreePos(x+i, y+j, t)) {
				x+=i; y+=j;
				return true;
			}
	return false;
}

bool serverCellMap::testFreePos(int x, int y, mobType t)
{
	int k, l;
	int w = mobileProp[t].width/48; // really BO_TILE_SIZE
	int h = mobileProp[t].height/48; // really BO_TILE_SIZE
	int goFlag = mobileProp[t].goFlag;

	for (k=0; k<w; k++) // object Width
		for (l=0; l<h; l++) // object Height
			if  (!isValid(x+k,y+l) || !cell(x+k,y+l).canGo(goFlag))
				return false;
	return true;
}


void serverCellMap::reportMob(serverMobUnit *u)
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


void serverCellMap::reportFix(serverFacility * f)
{
	ulong		k;
	int		i,j;
	QRect		r = f->gridRect();

	/* who is interested in knowing f's arrival */
	k = getPlayerMask(f->who);
	for (i=0; i<r.width(); i++)
		for (j=0; j<r.height(); j++) {
			k |= cell( r.x()+i, r.y()+j).known;
			cell( r.x()+i, r.y()+j).put_building();
		}
	f->setKnown(k);

	/* telling them */
	f->reportCreated();
}


