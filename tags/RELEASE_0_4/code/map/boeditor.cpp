/***************************************************************************
                         boeditor.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Sat Feb 14 1999
                                           
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

#include <stdio.h>

// _do_ use the local boFile.h copy !
#include "boFile.h"
#include "common/msgData.h"
#include "common/groundType.h"
#include "common/cell.h"

#include "map.h"


#define MAP_WIDTH	200
#define MAP_HEIGHT	200
#define PF_NAME		"basic.bpf"

void createSeaMap(int width, int height);
void fillRect(int x, int y, int width, int height, groundType g);
void createIsland(int x, int y, int trans, int width=1, int height=1, bool inverted=false);

/* global variable */
groundType	**cells;
groundType	**ncells;
FILE *logfile = (FILE *) 0L;

/* 0 is red, 1 is blue*/
// struct facilityMsg_t	{ int who, key, x, y, state; facilityType type; };
facilityMsg_t nfacility[30];
facilityMsg_t facility[] = {
	{0,	0, 13, 27, 0, FACILITY_POWERPLANT},
	{0,	0, 15, 27, 0, FACILITY_HELIPAD},
	{0,	0, 17, 27, 0, FACILITY_COMSAT},
	{0,	0, 12, 29, 0, FACILITY_CMDBUNKER},
	{0,	0, 15, 29, 0, FACILITY_BARRACKS},
	{0,	0, 17, 29, 0, FACILITY_WAR_FACTORY},

	{0,	0, 18, 25, 0, FACILITY_SAMSITE},
	{0,	0, 19, 25, 0, FACILITY_OILTOWER},
	{0,	0, 18, 26, 0, FACILITY_OILTOWER},
	{0,	0, 19, 26, 0, FACILITY_SAMSITE},

	{1,	1, 16, 15, 0, FACILITY_COMSAT},
	{1,	1, 12, 17, 0, FACILITY_HELIPAD},
	{1,	1, 14, 17, 0, FACILITY_POWERPLANT},
	{1,	1, 16, 17, 0, FACILITY_WAR_FACTORY}, 

	{1,	1, 21, 18, 0, FACILITY_REFINERY}, 
	{1,	1, 20, 20, 0, FACILITY_REPAIRPAD}, 
	{1,	1, 20, 17, 0, FACILITY_TURRET}, 
	{1,	1, 23, 17, 0, FACILITY_TURRET}, 
	{1,	0, 23, 19, 0, FACILITY_CMDBUNKER},
	};

/* 0 is red, 1 is blue*/
//struct mobileMsg_t	{ int who, key, x, y; mobType type; };
mobileMsg_t nmobile[30];
mobileMsg_t mobile[] = {
	{1,	1, BO_TILE_SIZE*22, BO_TILE_SIZE*15, MOB_AIRCRAFT},
	{1,	1, BO_TILE_SIZE*13, BO_TILE_SIZE*16, MOB_QUAD},
	{1,	1, BO_TILE_SIZE*19, BO_TILE_SIZE*13, MOB_SHIP},
	{1,	1, BO_TILE_SIZE*16, BO_TILE_SIZE*33, MOB_QUAD},

	{0,	0, BO_TILE_SIZE*10, BO_TILE_SIZE*28, MOB_SHIP},
	{0,	0, BO_TILE_SIZE*14, BO_TILE_SIZE*26, MOB_QUAD},
	{0,	0, BO_TILE_SIZE*15, BO_TILE_SIZE*26, MOB_QUAD},
	{0,	0, BO_TILE_SIZE*16, BO_TILE_SIZE*26, MOB_QUAD},
	{0,	0, BO_TILE_SIZE*20, BO_TILE_SIZE*29, MOB_QUAD},
	{0,	0, BO_TILE_SIZE*20, BO_TILE_SIZE*20, MOB_QUAD},
	{0,	0, BO_TILE_SIZE*12, BO_TILE_SIZE*25, MOB_AIRCRAFT},
	};

int main(void)
{
int	i, j;
Cell	c;

boFile *field = new boFile(); /// basic BosonPlayField...

printf("\nMap creation...\n");

createSeaMap( MAP_WIDTH, MAP_HEIGHT);
createIsland( 12, 15, TRANS_GW, 15, 20);
createIsland( 15, 23, TRANS_GD, 2, 3, true);
createIsland( 30, 15, TRANS_GW);

/* oil field */
fillRect(3,3,3,3, GROUND_WATER_OIL);
fillRect(22,25,3,5, GROUND_GRASS_OIL);

/* fill header */
field->nbPlayer		= 2;
field->map_width	= MAP_WIDTH;
field->map_height	= MAP_HEIGHT;
field->nbMobiles	= sizeof(mobile)/sizeof(mobile[0]);
field->nbFacilities	= sizeof(facility)/sizeof(facility[0]);
field->worldName	= "Test world 1";

printf("Creation .................Ok\nWrite to disk.............");

field->openWrite(PF_NAME);

	/* initialisation */
	for (i=0; i< MAP_WIDTH; i++)
		for (j=0; j< MAP_HEIGHT; j++) {
			c.setGroundType(cells[i][j]);
			c.setItem( ((i+7*j)%4));
			field->write(c);
		}
	
	for (i=0; i< field->nbMobiles; i++)
		field->write(mobile[i]);

	for (i=0; i< field->nbFacilities; i++)
		field->write(facility[i]);

field->Close();

if (!field->isOk()) {
	puts("Error : impossible to create file");
	exit(1);
	}

printf("Ok\nTest : reload from disk...");

delete field;

/* re-read, check coherency */
field = new boFile();

field->openRead(PF_NAME);

	/* allocation */
	printf("\tAllocating cells\n");
		ncells = new (groundType *)[field->map_width];
		for (i=0; i< field->map_width; i++)
			ncells[i] = new (groundType)[field->map_height];
	/* initialisation */
	for (i=0; i< field->map_width; i++)
		for (j=0; j< field->map_height; j++) {
			field->load (c);
			ncells[i][j] = c.getGroundType();
		}
	
	for (i=0; i< field->nbMobiles; i++)
		field->load(nmobile[i]);

	for (i=0; i< field->nbFacilities; i++)
		field->load(nfacility[i]);

field->Close();

if (!field->isOk()) {
	puts("Error : impossible to re-read file");
	exit(1);
	}

printf("Ok\nTest : checking header......");

if (field->map_width != MAP_WIDTH)
	puts("width NOK");
if (field->map_height!= MAP_HEIGHT)
	puts("height NOK");
if (field->nbMobiles != sizeof(mobile)/sizeof(mobile[0]))
	puts("nbMobiles NOK");
if (field->nbFacilities != sizeof(facility)/sizeof(facility[0]))
	puts("nbFacilities NOK");

printf("Ok\nTest : checking ground......");
for (i=0; i< field->map_width; i++)
	for (j=0; j< field->map_height; j++)
		if (ncells[i][j] != cells[i][j]) printf("cells[%d][%d] failed\n", i, j);
printf("Ok\nTest : checking mobiles......");
for (i=0; i< field->nbMobiles; i++) {
	if (mobile[i].x != nmobile[i].x) printf("mobile[%d] failed with x(%d,%d)\n", i, mobile[i].x, nmobile[i].x);
	if (mobile[i].y != nmobile[i].y)  printf("mobile[%d] failed with y(%d,%d)\n", i, mobile[i].y, nmobile[i].y);
	if (mobile[i].who != nmobile[i].who)  printf("mobile[%d] failed with who(%d,%d)\n", i, mobile[i].who, nmobile[i].who);
	if (mobile[i].type != nmobile[i].type)  printf("mobile[%d] failed with type(%d,%d)\n", i, mobile[i].type, nmobile[i].type);
	}
printf("Ok\nTest : checking facilities......");
for (i=0; i< field->nbFacilities; i++) {
	if (facility[i].x != nfacility[i].x) printf("facility[%d] failed with x(%d,%d)\n", i, facility[i].x, nfacility[i].x);
	if (facility[i].y != nfacility[i].y)  printf("facility[%d] failed with y(%d,%d)\n", i, facility[i].y, nfacility[i].y);
	if (facility[i].who != nfacility[i].who)  printf("facility[%d] failed with who(%d,%d)\n", i, facility[i].who, nfacility[i].who);
	if (facility[i].type != nfacility[i].type)  printf("facility[%d] failed with type(%d,%d)\n", i, facility[i].type, nfacility[i].type);
	}

printf("Ok\n");
return 0;
}


void createSeaMap(int width, int height)
{
int i,j;

printf("\tAllocating cells\n");
	cells = new (groundType *)[width];
	for (i=0; i< width; i++)
		cells[i] = new (groundType)[height];
		
printf("\tFilling the area with water\n");
	for (i=0; i< width; i++)
		for (j=0; j< height; j++)
			cells[i][j] = GROUND_WATER;
	
}



void createIsland(int x, int y, int trans, int width, int height, bool inverted)
{
	int i, j;
	
	printf("\tDrawing \'rectangle\' in %d,%d, size %dx%d, %s %s\n",
		x, y, width, height,
		groundTransProp[trans].name,
		inverted?"(inverted)":""
		);

	/* corners */
	cells[x-1][y-1] = GET_TRANS_NUMBER(trans, inverted?TRANS_ULI:TRANS_UL);
	cells[x+width][y-1] = GET_TRANS_NUMBER(trans, inverted?TRANS_URI:TRANS_UR);
	
	cells[x-1][y+height] = GET_TRANS_NUMBER(trans, inverted?TRANS_DLI:TRANS_DL);
	cells[x+width][y+height] = GET_TRANS_NUMBER(trans, inverted?TRANS_DRI:TRANS_DR);
	
	/* horizontal borders */
	for (i=0; i<width; i++) {
		cells[x+i][y-1] = GET_TRANS_NUMBER(trans, inverted?TRANS_DOWN:TRANS_UP);
		cells[x+i][y+height] = GET_TRANS_NUMBER(trans, inverted?TRANS_UP:TRANS_DOWN);
		}
	
	/* vertical borders */
	for (j=0; j<height; j++) {
		cells[x-1][y+j] = GET_TRANS_NUMBER(trans, inverted?TRANS_RIGHT:TRANS_LEFT);
		cells[x+width][y+j] = GET_TRANS_NUMBER(trans, inverted?TRANS_LEFT:TRANS_RIGHT);
		}
	
	/* filling inside */
	groundType plain = inverted?groundTransProp[trans].to:groundTransProp[trans].from;
	//printf("filling with %d\n", plain);
	fillRect(x,y,width,height,plain);
}

void fillRect(int x, int y, int width, int height, groundType g)
{
	int i, j;

	for (i=0; i<width; i++)
		for (j=0; j<height; j++)
			cells[x+i][y+j] = g;
}



