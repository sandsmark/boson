/***************************************************************************
                         boeditor.cpp  -  description                              
                             -------------------                                         

    version              :                                   
    begin                : Sat Feb 14 1999
                                           
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

//#include <qdatastream.h>
#include "playField.h"
#include "map.h"

#include "../common/groundType.h"
//#include "../common/cell.h"
#include "serverCell.h"



#define MAP_WIDTH	200
#define MAP_HEIGHT	200


origFacility facility[] = {
	{FACILITY_POWERPLANT,	0, 13, 29},
	{FACILITY_AIRFIELD,	1, 13, 15},
	{FACILITY_COMSAT,	1, 16, 15},
	{FACILITY_HELIPAD,	1, 12, 17},
	{FACILITY_POWERPLANT,	1, 14, 17},
	{FACILITY_WAR_FACTORY,	1, 16, 17}, 
	};

origMobile mobile[] = {
	{MOB_AIRCRAFT,	1, BO_TILE_SIZE*22, BO_TILE_SIZE*15},
	{MOB_QUAD,	1, BO_TILE_SIZE*12, BO_TILE_SIZE*15},
	{MOB_SHIP,	1, BO_TILE_SIZE*19, BO_TILE_SIZE*18},
	{MOB_SHIP,	0, BO_TILE_SIZE*10, BO_TILE_SIZE*28},
	{MOB_QUAD,	0, BO_TILE_SIZE*15, BO_TILE_SIZE*26},
	{MOB_AIRCRAFT,	0, BO_TILE_SIZE*12, BO_TILE_SIZE*25},
	};

origPeople people = {
	sizeof(mobile)/sizeof(mobile[0]),
	sizeof(facility)/sizeof(facility[0]),
	mobile,
	facility 
	};



void createSeaMap(int width, int height, playField &field);
void createIsland(int x, int y, int trans, playField &field, int width=1, int height=1, bool inverted=false);

int main(void)
{

playField field("basic.bpf"); /// basic BosonPlayField...
createSeaMap( MAP_WIDTH, MAP_HEIGHT, field);
createIsland(12, 15, TRANS_GW, field, 8, 15);
createIsland(15, 23, TRANS_GD, field, 2, 3, true);
createIsland(22, 15, TRANS_GW, field);

/* fill misc */
field.nbPlayer = 2;

/* fill people  */

field.people = people;

if (!field.write()) {
	puts("Error : impossible to create file");
	exit(1);
	}

puts("Ok, file created");

if (!field.load()) {
	puts("Error : impossible to re-read file");
	exit(1);
	}

puts("Ok, file re-read, checking...");

if (field.map.width != MAP_WIDTH)
	puts("width NOK");
if (field.map.height!= MAP_HEIGHT)
	puts("height NOK");
if (field.people.nbMobiles != sizeof(mobile)/sizeof(mobile[0]))
	puts("nbMobiles NOK");
if (field.people.nbFacilities != sizeof(facility)/sizeof(facility[0]))
	puts("nbFacilities NOK");


return 0;
}

void createSeaMap(int width, int height, playField &field)
{
int i,j;

printf("Filling the area with water\n");
/* fill map struct */
field.map.width = width;
field.map.height= height;
	field.map.cells = new (serverCell *)[field.map.width];
	for (i=0; i< field.map.width; i++)
		field.map.cells[i] = new (serverCell)[field.map.height];
		
	for (i=0; i< field.map.width; i++)
		for (j=0; j< field.map.height; j++)
			field.map.cells[i][j].setGroundType(GROUND_WATER);
	
}

void createIsland(int x, int y, int trans, playField &field, int width, int height, bool inverted)
{
int i, j;

printf("Drawing island in %d,%d, size %dx%d, %s %s\n",
	x, y, width, height,
	groundTransProp[trans].name,
	inverted?"(inverted)":""
	);


/* corners */
field.map.cells[x-1][y-1].setGroundType(GET_TRANS_NUMBER(trans, inverted?TRANS_ULI:TRANS_UL));
field.map.cells[x+width][y-1].setGroundType(GET_TRANS_NUMBER(trans, inverted?TRANS_URI:TRANS_UR));

field.map.cells[x-1][y+height].setGroundType(GET_TRANS_NUMBER(trans, inverted?TRANS_DLI:TRANS_DL));
field.map.cells[x+width][y+height].setGroundType(GET_TRANS_NUMBER(trans, inverted?TRANS_DRI:TRANS_DR));

/* horizontal borders */
for (i=0; i<width; i++) {
	field.map.cells[x+i][y-1].setGroundType(GET_TRANS_NUMBER(trans, inverted?TRANS_DOWN:TRANS_UP));
	field.map.cells[x+i][y+height].setGroundType(GET_TRANS_NUMBER(trans, inverted?TRANS_UP:TRANS_DOWN));
	}

/* vertical borders */
for (j=0; j<height; j++) {
	field.map.cells[x-1][y+j].setGroundType(GET_TRANS_NUMBER(trans, inverted?TRANS_RIGHT:TRANS_LEFT));
	field.map.cells[x+width][y+j].setGroundType(GET_TRANS_NUMBER(trans, inverted?TRANS_LEFT:TRANS_RIGHT));
	}

/* filling inside */
groundType plain = inverted?groundTransProp[trans].to:groundTransProp[trans].from;
//printf("filling with %d\n", plain);
for (i=0; i<width; i++)
	for (j=0; j<height; j++)
		field.map.cells[x+i][y+j].setGroundType(plain);
}
