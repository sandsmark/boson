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
	Cell(void) { flags=(cell_flags)0u; }

	/** tel if a given mobile can "go" on this cell */
	bool canGo(uint goFlag, groundType g);

	enum cell_flags { // one bit/flag
		building_f = 	0x02,
		field_unit_f =	0x04,
		flying_unit_f =	0x08,
		request_f =	0x10,
		request_flying_f =	0x20
	} flags;

	void	setFlag(cell_flags f) { flags = (cell_flags) (flags|f); } //  { flags |= f;} gives warning ??
	void	unsetFlag(cell_flags f) { flags = (cell_flags) (flags&~f); }

	bool building(void) { return flags & building_f; }	// building_f : building
	void put_building(void) { setFlag(building_f); }
	void del_building(void) { unsetFlag(building_f); }

	bool field_unit(void) { return flags & field_unit_f; }	// field_unit_f : field unit
	void put_field_unit(void) { setFlag(field_unit_f); }
	void del_field_unit(void) { unsetFlag(field_unit_f); }

	bool flying_unit(void) { return flags & flying_unit_f; }	// flying_unit_f : flying unit
	void put_flying_unit(void) { setFlag(flying_unit_f); }
	void del_flying_unit(void) { unsetFlag(flying_unit_f); }


private:
	bool canGoOnGround( groundType g, uint goFlag);
} ;


#endif // CELL_H

