/***************************************************************************
                          unitType.h  -  description                              
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

#ifndef UNIT_TYPE_H 
#define UNIT_TYPE_H 

enum mobType {
	MOB_QUAD,
	MOB_AIRCRAFT,
	MOB_SHIP,
	MOB_
	};

enum facilityType {
	FACILITY_AIRFIELD,
	FACILITY_COMSAT,
	FACILITY_HELIPAD,
	FACILITY_POWERPLANT,
	FACILITY_WAR_FACTORY,
	FACILITY_,
	};

#define BO_GO_FACILITY	0x001
#define BO_GO_SEA	0x010
#define BO_GO_PLAIN	0x020
#define BO_GO_ROCK	0x040
#define BO_GO_DESERT	0x080
#define BO_GO_AIR	(BO_GO_SEA | BO_GO_PLAIN | BO_GO_ROCK | BO_GO_DESERT)


#ifndef uint
typedef unsigned int uint;
#endif

/*
 * UNIT properties
 */

struct unitProperties_t {
/* from here.... */
	char 	*name;		// The name of the funny thing...
	uint	orders;		// which orders it's able to understand
/* ... to here : shoudn't be changed, as general type assume this is here, in this order */
	};



/*
 * MOBILE properties
 */

//struct mobileProperties_t : public unitProperties_t {
struct mobileProperties_t {
/* from here.... */
	char 	*name;		// The name of the funny thing...
	uint	orders;		// which orders it's able to understand
/* ... to here : shoudn't be changed, as general type assume this is here, in this order */
	int	width;		// pixel-size
	int	height;		// pixel-size
	int	visibility;	// how far it can see
	uint 	speed;		// how far it may travel (in pixels) during one jiffie

/*
// still unused ///orzel

	int	weakness;	// how much does the power fail when hit by a unit weapon
	int	initialForce;	// when initialized
	int	hitForce;	// how many hit units towards the others
	int	dynPowerCons;	// by how many decrease the power for each time Unit if moving
	int	statPowerCons;	// by how many decrease the power for each time Unit if not moving
	int	visibility;	// how far can it see 
	int 	cost;
	uint	goFlag;		// where can it go ? 
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
/* ... to here : shoudn't be changed, as general type assume this is here, in this order */
	int	width;		// tile-size
	int	height;		// tile-size
	int	visibility;	// how far it can see
	};

extern facilityProperties_t facilityProp[];
extern const int facilityPropNb;
 
#endif // UNIT_TYPE_H

