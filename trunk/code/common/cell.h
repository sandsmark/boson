/***************************************************************************
                          cell.h  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Sat Jan  9 19:35:36 CET 1999
                                           
    copyright            : (C) 2000 by Thomas Capricelli                         
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

#ifndef CELL_H 
#define CELL_H 


#include "common/unitType.h"

/*
 * Cell
 */

/** provide meta-information on each cell */
class Cell {

public:
	Cell(void) { ground = g_unknown; flags=0u; }

	bool isKnown(void) { return flags&0x1; }	// 0x1 : known / unknown
	void setGround(groundType);
	/** tel if a given mobile can "go" on this cell */
	bool canGo(mobType type);

	bool building(void) { return flags & 0x2; }	// 0x2 : building
	void put_building(void) { flags |= 0x2; }
	void del_building(void) { flags &= (~0x2); }

	bool field_unit(void) { return flags & 0x3; }	// 0x3 : field unit
	void put_field_unit(void) { flags |= 0x3; }
	void del_field_unit(void) { flags &= (~0x3); }

	bool flying_unit(void) { return flags & 0x4; }	// 0x4 : flying unit
	void put_flying_unit(void) { flags |= 0x4; }
	void del_flying_unit(void) { flags &= (~0x4); }


private:
	enum {
		g_unknown,
		g_dwater,
		g_water,
		g_grass,
		g_desert
	} ground;
	uint  flags;
	// flags : set/reset
} ;


#endif // CELL_H

