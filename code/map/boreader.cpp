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
	{MOB_ARROW,	1, BO_TILE_SIZE*22, BO_TILE_SIZE*15},
	{MOB_ARROW,	1, BO_TILE_SIZE*12, BO_TILE_SIZE*15},
	{MOB_TANK,	1, BO_TILE_SIZE*19, BO_TILE_SIZE*18},
	{MOB_TANK,	0, BO_TILE_SIZE*10, BO_TILE_SIZE*28},
	{MOB_QUAD,	0, BO_TILE_SIZE*15, BO_TILE_SIZE*26},
	{MOB_ARROW,	0, BO_TILE_SIZE*12, BO_TILE_SIZE*25},
	};


origPeople people = {
	sizeof(mobile)/sizeof(mobile[0]),
	sizeof(facility)/sizeof(facility[0]),
	mobile,
	facility 
	};

int main(void)
{

playField field("basic.bpf"); /// basic BosonPlayField...

if (!field.load()) {
	puts("Error : impossible to read file");
	exit(1);
	}

puts("Ok, file read, checking...");

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

