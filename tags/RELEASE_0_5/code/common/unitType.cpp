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
	{"ship",		EX(2), EX(2),  4, 5,	200, 100, 3, BO_GO_SEA},	// MOB_SHIP
	{"quad",		EX(1), EX(1),  3, 2,	200, 100, 2, BO_GO_EARTH},	// MOB_QUAD
	{"oilharvester",	EX(1), EX(1),  2, 0,	200, 100, 1, BO_GO_EARTH},	// MOB_OIL_HARVESTER
	{"mineralharvester",	EX(1), EX(1),  2, 0,	200, 100, 1, BO_GO_EARTH},	// MOB_MINERAL_HARVESTER
	{"aircraft",		EX(2), EX(2),  4, 4,	200, 100, 3, BO_GO_AIR},	// MOB_AIRCRAFT
	};
//	                        width          visibility    oil  speed
//	                               height     range mineral      goFlag

/*
	char 	*name;		// Guess it, geek..
	int	width;		// tile-size
	int	height;		// tile-size
	int	visibility;	// how far it can see
	int	range;		// how far it can send weapons
	uint	cost_mineral;	// how much mineral does it cost ?
	uint	cost_oil;	// how much oil does it cost ?
*/
	
facilityProperties_t facilityProp[] = {
	{"comsat"	, EX(2), EX(2), 7,  0,	300, 100},	// FACILITY_COMSAT
	{"helipad"	, EX(2), EX(2), 6,  0,	300, 100},	// FACILITY_HELIPAD
	{"powerplant"	, EX(2), EX(2), 3,  0,	300, 100},	// FACILITY_POWERPLANT
	{"warfactory"	, EX(2), EX(2), 3,  0,	300, 100},	// FACILITY_WAR_FACTORY
	{"barracks"	, EX(2), EX(2), 3,  0,	300, 100},	// FACILITY_BARRACKS
	{"cmdbunker"	, EX(3), EX(3), 3,  0,	300, 100},	// FACILITY_CMDBUNKER
	{"samsite"	, EX(1), EX(1), 4,  0,	300, 100},	// FACILITY_SAMSITE
	{"oiltower"	, EX(1), EX(1), 2,  0,	300, 100},	// FACILITY_OILTOWER
	{"refinery"	, EX(2), EX(2), 3,  0,	300, 100},	// FACILITY_REFINERY
	{"repairpad"	, EX(2), EX(2), 4,  0,	300, 100},	// FACILITY_REPAIRPAD
	{"turret"	, EX(1), EX(1), 5,  5,	300, 100},	// FACILITY_TURRET
	};
//	                                        mineral
//	                  width         visibility
//	                         height     range     oil
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


