/***************************************************************************
                          groundType.h  -  description                              
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

#ifndef GROUND_TYPE_H 
#define GROUND_TYPE_H 

enum groundType {
	GROUND_FACILITY = -2,
	GROUND_UNKNOWN = -1,

	GROUND_GRASS = 0,
	GROUND_WATER = 1,
	GROUND_DESERT,

	GROUND_LAST,

	};

enum transType {
	TRANS_GW,
	TRANS_GD,
	};


enum transition_t {
//	TRANS_PLAIN,
	TRANS_UL=0,	// up left
	TRANS_UR=1,	// up right
	TRANS_DL,	// down left
	TRANS_DR,	// down right

	TRANS_UP,
	TRANS_DOWN,
	TRANS_LEFT,
	TRANS_RIGHT,

	TRANS_ULI,	// up left inverted
	TRANS_URI,	// up right inverted
	TRANS_DLI,	// down left inverted
	TRANS_DRI,	// down right inverted
	};


/*
 *  Numbering : 
 *	-1 is no_pixmap
 *	0  is GRASS
 *	....
 *	GROUND_LAST-1 is the last plain_pixmap
 *
 *	GROUND_LAST to
 * 	GROUND_LAST+7,  first transitions
 *
 *	GROUND_LAST+n*TILES_PER_TRANSITION to
 * 	GROUND_LAST+n*8+7,  (n-1)-eme transitions
 *
 */

#define TILES_PER_TRANSITION 12
#define GET_TRANS_NUMBER(transNb,transPart)  ((groundType)(GROUND_LAST+(TILES_PER_TRANSITION*(transNb)) + (transPart)))

/* Transition description */
struct groundTransProperties_t {
	char		*name;
	groundType	from, to;
	};
extern const groundTransProperties_t groundTransProp[];
extern const int groundTransPropNb;

/* Ground Properties */
struct groundProperties_t {
	char	*name;
	};
extern const groundProperties_t groundProp[GROUND_LAST];
extern const int groundPropNb;


/* destroyed type */
enum destroyedType {
	DESTROYED_NONE,
	DESTROYED_WEAK,
	DESTROYED_ENOUGH,
	DESTROYED_QUITE,
	DESTROYED_NORMAL,
	DESTROYED_STRONG,
	DESTROYED_
	};

#endif // GROUND_TYPE_H

