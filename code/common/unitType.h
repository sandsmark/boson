/***************************************************************************
                          unitType.h  -  description                              
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

#ifndef UNITTYPE_H 
#define UNITTYPE_H 

#include "groundType.h"

/*
 * mobiles should be from lower to higher
 */

// order matters : the latest, the 'highest', so flyers belongs to the end..
enum mobType {
	MOB_SHIP = 0,
	MOB_QUAD,
	MOB_OIL_HARVESTER,
	MOB_MINERAL_HARVESTER,
	MOB_AIRCRAFT,
	MOB_LAST
	};


#define PIXMAP_PER_FIX		5
#define PIXMAP_PER_MOBILE	9

#define PIXMAP_FIX_DESTROYED	(PIXMAP_PER_FIX-1)
#define PIXMAP_MOBILE_DESTROYED	(PIXMAP_PER_MOBILE-1)

#define	CONSTRUCTION_STEPS	PIXMAP_FIX_DESTROYED
#define	DIRECTION_STEPS		PIXMAP_MOBILE_DESTROYED

enum facilityType {
	FACILITY_COMSAT = 0,
	FACILITY_HELIPAD,
	FACILITY_POWERPLANT,
	FACILITY_WAR_FACTORY,
	FACILITY_BARRACKS,
	FACILITY_CMDBUNKER,
	FACILITY_SAMSITE,
	FACILITY_OILTOWER,
	FACILITY_REFINERY,
	FACILITY_REPAIRPAD,
	FACILITY_TURRET,
	FACILITY_LAST
	};


/*
	GROUND_DEEP_WATER = 0,
	GROUND_WATER = 1,
	GROUND_GRASS,
	GROUND_DESERT,
*/
#define GET_BIT(bb)		(1l<<(bb))
#define BO_GO_WATER		GET_BIT(GROUND_WATER) | GET_BIT(GROUND_WATER_OIL)
#define BO_GO_DEEP_WATER	GET_BIT(GROUND_DEEP_WATER)
#define BO_GO_GRASS		GET_BIT(GROUND_GRASS) | GET_BIT(GROUND_GRASS_OIL)
#define BO_GO_DESERT		GET_BIT(GROUND_DESERT)

#define BO_GO_SEA		(BO_GO_WATER | BO_GO_DEEP_WATER)
#define BO_GO_EARTH		(BO_GO_GRASS | BO_GO_DESERT)
#define BO_GO_AIR		(0xfffff)

#ifndef uint
typedef unsigned int uint;
#endif

/*
 * speed/range handling, we define a new distance :)
 */
// dist = 1          up, left, right, down
// dist = 2 idem 1 + upper-left, uppert-right corner...
// dist = 3 idem 2 + two-tile up, two-tile left..
//
// 43334
// 32123
// 31013  <= 0 is the center, other figures tells the boDist();
// 32123
// 43334
int	boDist(int, int);
int	boGridDist(int, int);



/**
 * UNIT properties
 */

struct unitProperties_t {
/* from here.... */
	const char 	*name;		// The name of the funny thing...
	int	width;
	int	height;
	int	visibility;	// how far it can see
	int	range;		// how far it can send weapons
	uint	cost_mineral;	// how much mineral does it cost ?
	uint	cost_oil;	// how much oil does it cost ?
/* ... to here : shoudn't be changed, as general type assume this is here, in this order */
	};



/**
 * MOBILE properties
 */

//struct mobileProperties_t : public unitProperties_t {
struct mobileProperties_t {
/* from here.... */
	const char 	*name;		// The name of the funny thing...
	int	width;		// pixel-size
	int	height;		// pixel-size
	int	visibility;	// how far it can see
	int	range;		// how far it can send weapons
	uint	cost_mineral;	// how much mineral does it cost ?
	uint	cost_oil;	// how much oil does it cost ?
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
	const char 	*name;		// Guess it, geek..
	int	width;		// tile-size
	int	height;		// tile-size
	int	visibility;	// how far it can see
	int	range;		// how far it can send weapons
	uint	cost_mineral;	// how much mineral does it cost ?
	uint	cost_oil;	// how much oil does it cost ?
/* ... to here : shoudn't be changed, as general type assume this is here, in this order */

	};

extern facilityProperties_t facilityProp[];
extern const int facilityPropNb;
 
#endif // UNITTYPE_H

