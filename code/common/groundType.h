/***************************************************************************
                          groundType.h  -  description                              
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

#ifndef GROUNDTYPE_H 
#define GROUNDTYPE_H 

enum groundType {
	GROUND_UNKNOWN = 0,
	GROUND_DEEP_WATER,
	GROUND_WATER,
	GROUND_GRASS,
	GROUND_DESERT,

	GROUND_WATER_OIL,
	GROUND_GRASS_OIL,

	GROUND_LAST

	};

enum transType {
	TRANS_GW = 0,
	TRANS_GD,
	TRANS_DW,
	TRANS_DWD,
	TRANS_LAST
	};


enum transition_t {
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
	TRANS_DRI	// down right inverted
	};


/*
 *  Numbering : 
 *	0  is GROUND_UNKNOWN
 *	1  is GROUND_DEEP_WATER
 *	....
 *	GROUND_LAST-1 is the last plain_pixmap
 *
 *	GROUND_LAST to
 * 	GROUND_LAST+TILES_PER_TRANSITION-1,  first transitions
 *
 *	GROUND_LAST+  n  *TILES_PER_TRANSITION to
 * 	GROUND_LAST+(n+1)*TILES_PER_TRANSITION-1,  (n-1)-th transitions
 *
 */

#define SMALL_TILES_PER_TRANSITION	12
#define BIG_TILES_PER_TRANSITION	16
#define TILES_PER_TRANSITION		(SMALL_TILES_PER_TRANSITION+4*BIG_TILES_PER_TRANSITION)
// number of different QPixmap use for ground
#define NB_GROUND_TILES			(GROUND_LAST + TRANS_LAST * TILES_PER_TRANSITION)


// conversion ref/tiles -> number
#define GET_TRANS_NUMBER(transRef,transTile)	\
	( (groundType)  ( GROUND_LAST + (TILES_PER_TRANSITION*(transRef)) + (transTile) ))
#define GET_BIG_TRANS_NUMBER(transRef,transTile) \
	GET_TRANS_NUMBER(transRef, SMALL_TILES_PER_TRANSITION + 4*(transTile))

// conversion number -> ref/tiles
// only valid if g is really a transition 
#define GET_TRANS_REF(g)	(((g)-GROUND_LAST) / TILES_PER_TRANSITION )
#define GET_TRANS_TILE(g)	(((g)-GROUND_LAST) % TILES_PER_TRANSITION )

// self-explaining names
#define IS_TRANS(g)		( (g) >= GROUND_LAST && (g)< NB_GROUND_TILES )
#define IS_SMALL_TRANS(g)	( IS_TRANS((g)) && (GET_TRANS_TILE(g) <  SMALL_TILES_PER_TRANSITION))
#define IS_BIG_TRANS(g)		( IS_TRANS((g)) && (GET_TRANS_TILE(g) >= SMALL_TILES_PER_TRANSITION))
#define IS_PLAIN(g)		( (g) >= 0 && (g) < GROUND_LAST)
#define IS_VALID_GROUND(g)	( (g) >= 0 && (g) < NB_GROUND_TILES)



/***** cell_t ******/

	//
	//  cell_t needs to be a scalar type cause QCanvas uses a 'int'
	//  so we can't use a class..
	//
#ifndef byte
typedef unsigned char byte;
#endif

typedef unsigned int cell_t;
inline byte		tile(cell_t c) { return (byte) (c&0x03); }
inline groundType	ground(cell_t c) { return (groundType) (c>>2); }
inline cell_t		cell(groundType g, int t=0) { return (cell_t) (g<<2 | (t&0x3)); }
inline void		setGround(cell_t &c, groundType g) {  c&=0x3; c|= (g<<2); }
inline void		setTile(cell_t &c, byte t) {  c&=~0x3; c|= (t&0x3); }
/***** cell_t ******/



/* Transition description */
struct groundTransProperties_t {
	const char	*name;
	groundType	from, to;
	};
extern const groundTransProperties_t groundTransProp[];
extern const int groundTransPropNb;


/* Ground Properties */
struct groundProperties_t {
	const char	*name;
	};
extern const groundProperties_t groundProp[GROUND_LAST];
extern const int groundPropNb;


/* destroyed type */
///orzel : still unused...
enum destroyedType {
	DESTROYED_NONE,
	DESTROYED_WEAK,
	DESTROYED_ENOUGH,
	DESTROYED_QUITE,
	DESTROYED_NORMAL,
	DESTROYED_STRONG,
	DESTROYED_
	};

#endif // GROUNDTYPE_H

