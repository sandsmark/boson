/***************************************************************************
                          unitType.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Sat Jan  9 19:35:36 CET 1999
                                           
    copyright            : (C) 1999-2000 by Thomas Capricelli                         
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

#include "unitType.h"
#include "bomap.h"

/*
	char 	*name;		// The name of the funny thing...
	const char	*desc;		// Description of the unit
	int	width;		// pixel-size
	int	height;		// pixel-size
	int	visibility;	// how far it can see
	int	range;		// how far it can send weapons
	uint	cost_mineral;	// how much mineral does it cost ?
	uint	cost_oil;	// how much oil does it cost ?

	int 	speed;		// how many tiles can it move during one jiffie
	uint	goFlag;		// where can it go ? 
*/

#define EX(s) ((s)*BO_TILE_SIZE)
mobileProperties_t mobileProp[] = {
	{"ship",
	 "Ship can shoot, and move on water",
	 EX(2), EX(2),  4, 5,	150, 20, 3, BO_GO_SEA},	// MOB_SHIP

	{"quad",
	 "Quad can go fast, shoot, and move on ground",
	 EX(1), EX(1),  3, 2,	 50, 20, 2, BO_GO_EARTH},	// MOB_QUAD

	{"oilharvester",
	 "Oil Harvester go slowly on ground, can't shoot. They harvest oil",
	 EX(1), EX(1),  2, 0,	100, 20, 1, BO_GO_EARTH},	// MOB_OIL_HARVESTER

	{"mineralharvester",
	 "Mineral Harvester go slowly on ground, can't shoot. They harvest mineral",
	 EX(1), EX(1),  2, 0,	100, 20, 1, BO_GO_EARTH},	// MOB_MINERAL_HARVESTER

	{"aircraft",
	 "Aircraft move quickly in the air, they can shoot",
	 EX(2), EX(2),  4, 4,	120, 20, 3, BO_GO_AIR},	// MOB_AIRCRAFT

	};
//       width          visibility   oil speed
//              height     range mineral    goFlag

/*
	char 	*name;		// Guess it, geek..
	const char	*desc;		// Description of the unit
	int	width;		// tile-size
	int	height;		// tile-size
	int	visibility;	// how far it can see
	int	range;		// how far it can send weapons
	uint	cost_mineral;	// how much mineral does it cost ?
	uint	cost_oil;	// how much oil does it cost ?
*/
	
facilityProperties_t facilityProp[] = {
	{"comsat"	,
	 "Not used yet",
	 EX(2), EX(2), 7,  0,	1000, 40},	// FACILITY_COMSAT

	{"helipad"	,
	 "Not used yet",
	 EX(2), EX(2), 6,  0,	500, 40},	// FACILITY_HELIPAD

	{"powerplant"	,
	 "This facility ljsadljasdljsdlfjsadlfjsadlfjdaslfjsdafjsldfjlasdfjlasdfjasdlfjasdlfsjfsafasd",
	 EX(2), EX(2), 3,  0,	500, 40},	// FACILITY_POWERPLANT

	{"warfactory"	,
	 "This is where you can build mobile units",
	 EX(2), EX(2), 3,  0,	1300, 40},	// FACILITY_WAR_FACTORY

	{"barracks"	,
	 "Not used yet",
	 EX(2), EX(2), 3,  0,	500, 40},	// FACILITY_BARRACKS

	{"cmdbunker"	,
	 "This is the main facility, from which you can build most of other facility",
	 EX(3), EX(3), 3,  0,	3000, 40},	// FACILITY_CMDBUNKER

	{"samsite"	,
	 "This facility can shoot quicly at other units",
	 EX(1), EX(1), 4, 10,	500, 40},	// FACILITY_SAMSITE

	{"oiltower"	,
	 "Not used yet",
	 EX(1), EX(1), 2,  0,	100, 40},	// FACILITY_OILTOWER

	{"refinery"	,
	 "Not used yet",
	 EX(2), EX(2), 3,  0,	500, 40},	// FACILITY_REFINERY

	{"repairpad"	,
	 "This facility is used to repair units (not done yet)",
	 EX(2), EX(2), 4,  0,	800, 40},	// FACILITY_REPAIRPAD

	{"turret"	,
	 "Turrets can shoot, but not that much",
	 EX(1), EX(1), 5,  5,	200, 40},	// FACILITY_TURRET

	};
//	                        mineral
//      width          visibility
//              height     range     oil
#undef EX

const int facilityPropNb = (sizeof(facilityProp)/sizeof(facilityProp [0]));
const int mobilePropNb = (sizeof(mobileProp)/sizeof(mobileProp [0]));

// 43334
// 32123
// 31013  <= 0 is the center, other figures tells the boDist();
// 32123
// 43334
int	boDist(int a, int b)
{
	return boGridDist(a/BO_TILE_SIZE, b/BO_TILE_SIZE);
}

int	boGridDist(int a, int b)
{
	// abs
	if (a<0) a=-a;
	if (b<0) b=-b;

	if ( 0==a && 2==b) return 3;
	if ( 0==b && 2==a) return 3;

	return a+b;
}


