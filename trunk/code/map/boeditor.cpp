/***************************************************************************
                         boeditor.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
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

#include <stdio.h>

#include "../common/playField.h"
#include "../common/groundType.h"
#include "../common/cell.h"

#include "map.h"


#define MAP_WIDTH	200
#define MAP_HEIGHT	200
#define PF_NAME		"basic.bpf"

void createSeaMap(int width, int height, playField &field);
void createIsland(int x, int y, int trans, playField &field, int width=1, int height=1, bool inverted=false);



/* 0 is red, 1 is blue*/
origFacility facility[] = {
	{FACILITY_POWERPLANT,	0, 13, 27},
	{FACILITY_HELIPAD,	0, 15, 27},
	{FACILITY_COMSAT,	0, 17, 27},
	{FACILITY_CMDBUNKER,	0, 12, 29},
	{FACILITY_BARRACKS,	0, 15, 29},
	{FACILITY_WAR_FACTORY,	0, 17, 29},

	{FACILITY_SAMSITE,	0, 18, 25},
	{FACILITY_OILTOWER,	0, 19, 25},
	{FACILITY_OILTOWER,	0, 18, 26},
	{FACILITY_SAMSITE,	0, 19, 26},

	{FACILITY_COMSAT,	1, 16, 15},
	{FACILITY_HELIPAD,	1, 12, 17},
	{FACILITY_POWERPLANT,	1, 14, 17},
	{FACILITY_WAR_FACTORY,	1, 16, 17}, 

	{FACILITY_REFINERY,	1, 21, 18}, 
	{FACILITY_REPAIRPAD,	1, 20, 20}, 
	{FACILITY_TURRET,	1, 20, 17}, 
	{FACILITY_TURRET,	1, 23, 17}, 
	};

/* 0 is red, 1 is blue*/
origMobile mobile[] = {
	{MOB_AIRCRAFT,	1, BO_TILE_SIZE*22, BO_TILE_SIZE*15},
	{MOB_QUAD,	1, BO_TILE_SIZE*13, BO_TILE_SIZE*16},
	{MOB_SHIP,	1, BO_TILE_SIZE*19, BO_TILE_SIZE*13},
	{MOB_QUAD,	1, BO_TILE_SIZE*16, BO_TILE_SIZE*33},

	{MOB_SHIP,	0, BO_TILE_SIZE*10, BO_TILE_SIZE*28},
	{MOB_QUAD,	0, BO_TILE_SIZE*14, BO_TILE_SIZE*26},
	{MOB_QUAD,	0, BO_TILE_SIZE*15, BO_TILE_SIZE*26},
	{MOB_QUAD,	0, BO_TILE_SIZE*16, BO_TILE_SIZE*26},
	{MOB_QUAD,	0, BO_TILE_SIZE*20, BO_TILE_SIZE*29},
	{MOB_QUAD,	0, BO_TILE_SIZE*20, BO_TILE_SIZE*20},
	{MOB_AIRCRAFT,	0, BO_TILE_SIZE*12, BO_TILE_SIZE*25},
	};

origPeople people = {
	sizeof(mobile)/sizeof(mobile[0]),
	sizeof(facility)/sizeof(facility[0]),
	mobile,
	facility 
	};




int main(void)
{

playField *field = new playField(PF_NAME); /// basic BosonPlayField...

printf("\nMap creation...\n");
createSeaMap( MAP_WIDTH, MAP_HEIGHT, *field);
createIsland(12, 15, TRANS_GW, *field,15, 20);
createIsland(15, 23, TRANS_GD, *field, 2, 3, true);
createIsland(30, 15, TRANS_GW, *field);

/* fill misc */
field->nbPlayer = 2;

/* fill people  */
field->people = people;

printf("Creation .................Ok\nWrite to disk.............");
if (!field->write()) {
	puts("Error : impossible to create file");
	exit(1);
	}

printf("Ok\nTest : reload from disk...");

delete field;

/* re-read, check coherency */
field = new playField(PF_NAME);

if (!field->load()) {
	puts("Error : impossible to re-read file");
	exit(1);
	}

printf("Ok\nTest : checking data......");

if (field->map.width != MAP_WIDTH)
	puts("width NOK");
if (field->map.height!= MAP_HEIGHT)
	puts("height NOK");
if (field->people.nbMobiles != sizeof(mobile)/sizeof(mobile[0]))
	puts("nbMobiles NOK");
if (field->people.nbFacilities != sizeof(facility)/sizeof(facility[0]))
	puts("nbFacilities NOK");

printf("Ok\n\n");

return 0;
}



void createSeaMap(int width, int height, playField &field)
{
int i,j;

printf("\tFilling the area with water\n");
/* fill map struct */
field.map.width = width;
field.map.height= height;
	field.map.cells = new (groundType *)[field.map.width];
	for (i=0; i< field.map.width; i++)
		field.map.cells[i] = new (groundType)[field.map.height];
		
	for (i=0; i< field.map.width; i++)
		for (j=0; j< field.map.height; j++)
			field.map.cells[i][j] = GROUND_WATER;
	
}



void createIsland(int x, int y, int trans, playField &field, int width, int height, bool inverted)
{
int i, j;

printf("\tDrawing \'rectangle\' in %d,%d, size %dx%d, %s %s\n",
	x, y, width, height,
	groundTransProp[trans].name,
	inverted?"(inverted)":""
	);

/* corners */
field.map.cells[x-1][y-1] = GET_TRANS_NUMBER(trans, inverted?TRANS_ULI:TRANS_UL);
field.map.cells[x+width][y-1] = GET_TRANS_NUMBER(trans, inverted?TRANS_URI:TRANS_UR);

field.map.cells[x-1][y+height] = GET_TRANS_NUMBER(trans, inverted?TRANS_DLI:TRANS_DL);
field.map.cells[x+width][y+height] = GET_TRANS_NUMBER(trans, inverted?TRANS_DRI:TRANS_DR);

/* horizontal borders */
for (i=0; i<width; i++) {
	field.map.cells[x+i][y-1] = GET_TRANS_NUMBER(trans, inverted?TRANS_DOWN:TRANS_UP);
	field.map.cells[x+i][y+height] = GET_TRANS_NUMBER(trans, inverted?TRANS_UP:TRANS_DOWN);
	}

/* vertical borders */
for (j=0; j<height; j++) {
	field.map.cells[x-1][y+j] = GET_TRANS_NUMBER(trans, inverted?TRANS_RIGHT:TRANS_LEFT);
	field.map.cells[x+width][y+j] = GET_TRANS_NUMBER(trans, inverted?TRANS_LEFT:TRANS_RIGHT);
	}

/* filling inside */
groundType plain = inverted?groundTransProp[trans].to:groundTransProp[trans].from;
//printf("filling with %d\n", plain);
for (i=0; i<width; i++)
	for (j=0; j<height; j++)
		field.map.cells[x+i][y+j] = plain;
}
