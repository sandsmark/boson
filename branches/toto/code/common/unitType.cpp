/***************************************************************************
                          unitType.cpp  -  description                              
                             -------------------                                         

    version              :                                   
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
	uint 	speed;		// how far it may travel (in pixels) during one jiffie
*/

mobileProperties_t mobileProp[] = {
	{"Arrow", 0, 32,32, 5, 10},	// MOB_ARROW
	{"Quad", 0, 64,64, 6, 4},	// MOB_QUAD
	{"Tank", 0, 48,48, 5, 7},	// MOB_TANK
/*	{"", 0, 2, 2}			// MOB_
*/
	};

facilityProperties_t facilityProp[] = {
	{"AirField", 0, 3, 2, 7},
	{"ComSat", 0, 2, 2, 7},
	{"HeliPad", 0, 2, 2, 7},
	{"PowerPlant", 0, 2, 2, 7},
	{"WarFactory", 0, 2, 2, 7},
/*	{"carre", 0, 2, 2},
	{"", 0, 2, 2} */
	};

const int facilityPropNb = (sizeof(facilityProp)/sizeof(facilityProp [0]));
const int mobilePropNb = (sizeof(mobileProp)/sizeof(mobileProp [0]));
