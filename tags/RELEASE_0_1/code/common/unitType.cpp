/***************************************************************************
                          unitType.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Sat Jan  9 19:35:36 CET 1999
                                           
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

#include "unitType.h"

/*
	char 	*name;		// The name of the funny thing...
	uint	orders;		// which orders it's able to understand
	int	width;		// pixel-size
	int	height;		// pixel-size

	int	visibility;	// how far it can see
	int 	speed;		// how far it may travel (in pixels) during one jiffie
	uint	goFlag;		// where can it go ? 
*/

mobileProperties_t mobileProp[] = {
	{"ship",	0, 96, 96,  2, 3, BO_GO_SEA},	// MOB_SHIP
	{"quad",	0, 32, 32,  3, 4, BO_GO_EARTH},	// MOB_QUAD
	{"aircraft",	0, 72, 72,  3, 6, BO_GO_AIR},	// MOB_AIRCRAFT
	};

/*
	char 	*name;		// Guess it, geek..
	uint	orders;		// which orders it's able to understand
	int	width;		// tile-size
	int	height;		// tile-size
	int	visibility;	// how far it can see
*/
facilityProperties_t facilityProp[] = {
	{"comsat"	, 0, 2, 2, 7},			// FACILITY_COMSAT
	{"helipad"	, 0, 2, 2, 6},			// FACILITY_HELIPAD
	{"powerplant"	, 0, 2, 2, 3},			// FACILITY_POWERPLANT
	{"warfactory"	, 0, 2, 2, 3}, 			// FACILITY_WAR_FACTORY
	{"barracks"	, 0, 2, 2, 3}, 			// FACILITY_BARRACKS
	{"bunker"	, 0, 2, 2, 3}, 			// FACILITY_BUNKER
	{"samsite"	, 0, 1, 1, 4}, 			// FACILITY_SAMSITE
	{"oiltower"	, 0, 1, 1, 2}, 			// FACILITY_OILTOWER
	};

const int facilityPropNb = (sizeof(facilityProp)/sizeof(facilityProp [0]));
const int mobilePropNb = (sizeof(mobileProp)/sizeof(mobileProp [0]));



