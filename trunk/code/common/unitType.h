/***************************************************************************
                          unitType.h  -  description                              
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

#ifndef UNIT_TYPE_H 
#define UNIT_TYPE_H 

#include "groundType.h"

/*
 * mobiles should be from lower to higher
 */
enum mobType {
	MOB_SHIP = 0,
	MOB_QUAD,
	MOB_AIRCRAFT,
	MOB_
	};

enum facilityType {
	FACILITY_AIRFIELD = 0 ,
	FACILITY_COMSAT,
	FACILITY_HELIPAD,
	FACILITY_POWERPLANT,
	FACILITY_WAR_FACTORY,
	FACILITY_,
	};


/*
	GROUND_WATER = 0,
//	GROUND_DEEP_WATER = 1,
	GROUND_GRASS,
	GROUND_DESERT,
*/
#define GET_BIT(bb)	(1l<<(bb))
#define BO_GO_WATER		GET_BIT(GROUND_WATER)
//#define BO_GO_DEEP_WATER	GET_BIT(GROUND_DEEP_WATER)
#define BO_GO_GRASS		GET_BIT(GROUND_GRASS)
#define BO_GO_DESERT		GET_BIT(GROUND_DESERT)

#define BO_GO_SEA		(BO_GO_WATER/* | BO_GO_DEEP_WATER*/)
#define BO_GO_EARTH		(BO_GO_GRASS | BO_GO_DESERT)
#define BO_GO_AIR		(0xfffff)

#ifndef uint
typedef unsigned int uint;
#endif

/**
 * UNIT properties
 */

struct unitProperties_t {
/* from here.... */
	char 	*name;		// The name of the funny thing...
	uint	orders;		// which orders it's able to understand
	int	width;
	int	height;
	int	visibility;	// how far it can see
/* ... to here : shoudn't be changed, as general type assume this is here, in this order */
	};



/**
 * MOBILE properties
 */

//struct mobileProperties_t : public unitProperties_t {
struct mobileProperties_t {
/* from here.... */
	char 	*name;		// The name of the funny thing...
	uint	orders;		// which orders it's able to understand
	int	width;		// pixel-size
	int	height;		// pixel-size
	int	visibility;	// how far it can see
/* ... to here : shoudn't be changed, as general type assume this is here, in this order */

	int 	speed;		// how far it may travel (in pixels) during one jiffie
	uint	goFlag;		// where can it go ? 

/*
// still unused ///orzel

	int	weakness;	// how much does the power fail when hit by a unit weapon
	int	initialForce;	// when initialized
	int	hitForce;	// how many hit units towards the others
	int	dynPowerCons;	// by how many decrease the power for each time Unit if moving
	int	statPowerCons;	// by how many decrease the power for each time Unit if not moving
	int	visibility;	// how far can it see 
	int 	cost;
*/
	};

extern mobileProperties_t mobileProp[];
extern const int mobilePropNb;




/*
 * MOBILE properties
 */
struct facilityProperties_t  {
/* from here.... */
	char 	*name;		// Guess it, geek..
	uint	orders;		// which orders it's able to understand
	int	width;		// tile-size
	int	height;		// tile-size
	int	visibility;	// how far it can see
/* ... to here : shoudn't be changed, as general type assume this is here, in this order */

	};

extern facilityProperties_t facilityProp[];
extern const int facilityPropNb;
 
#endif // UNIT_TYPE_H

