/***************************************************************************
                          unitType.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Sat Jan  9 19:35:36 CET 1999
                                           
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

#include "unitType.h"
#include "map.h"

/*
	char 	*name;		// The name of the funny thing...
	int	width;		// pixel-size
	int	height;		// pixel-size
	int	visibility;	// how far it can see
	int	range;		// how far it can send weapons
	uint	cost_mineral;	// how much mineral does it cost ?
	uint	cost_oil;	// how much oil does it cost ?

	int 	speed;		// how far it may travel (in pixels) during one jiffie
	uint	goFlag;		// where can it go ? 
*/

mobileProperties_t mobileProp[] = {
	{"ship",	96, 96,  2, 100,	200, 100, 3, BO_GO_SEA},	// MOB_SHIP
	{"quad",	32, 32,  3, 70,		200, 100, 4, BO_GO_EARTH},	// MOB_QUAD
	{"aircraft",	72, 72,  3, 100,	200, 100, 6, BO_GO_AIR},	// MOB_AIRCRAFT
	};

/*
	char 	*name;		// Guess it, geek..
	int	width;		// tile-size
	int	height;		// tile-size
	int	visibility;	// how far it can see
	int	range;		// how far it can send weapons
	uint	cost_mineral;	// how much mineral does it cost ?
	uint	cost_oil;	// how much oil does it cost ?
*/
	
#define EX(s) (s*BO_TILE_SIZE)
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
	{"turret"	, EX(1), EX(1), 5, 90,	300, 100},	// FACILITY_TURRET
	};
#undef EX

const int facilityPropNb = (sizeof(facilityProp)/sizeof(facilityProp [0]));
const int mobilePropNb = (sizeof(mobileProp)/sizeof(mobileProp [0]));



