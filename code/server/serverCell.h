/***************************************************************************
                          serverCell.h  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Sat Jan  9 19:35:36 CET 1999
                                           
    copyright            : (C) 1999-2000 by Thomas Capricelli                         
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

#ifndef SERVERCELL_H 
#define SERVERCELL_H 

#include "common/cell.h"
#include "knownBy.h"

class BosonServer;
class serverMobUnit;
class serverFacility;

/** 
  * This is a cell in the server point of view
  */
class serverCell : public knownBy, public Cell
{
	friend	BosonServer; /* to write in _cell */

public:
	serverCell() { _cell = makeCell(GROUND_UNKNOWN); }

	bool		canGo(int goFlag) { return Cell::canGo(goFlag, ground() ); }
	groundType	ground(void) { return ::ground(_cell); }
//	byte		version(void) { return version(_cell); }
//	void		operator= (cell_t c) { _cell = c; }
private:
	cell_t	_cell;
};


class serverCellMap : public CellMap
{
public:
       	serverCellMap(void);
       	virtual ~serverCellMap(void){ if (_width!=-1) delete [] cells; }

	virtual enum groundType	groundAt(QPoint p)  { return cell(p.x(), p.y()).ground() ;}
	/** return the cell at (i,j) */
	virtual Cell	&ccell(int i, int j) { return cell(i,j); }		// simple cast
protected:
	// called from serverMap.cpp
	/** initialise the object */
       	void initCellMap(int w, int h) { cells = new serverCell[w*h]; _width = w; _height = h; }
	/** convenient access function for serverCell instead of Cell */
	serverCell	&cell(int x, int y) {return cells[ x + y * _width ]; }
	bool		findFreePos(int &x, int &y, enum mobType t);
	bool		testFreePos(int x, int y, enum mobType t);
  
	void		reportMob(serverMobUnit *u);
	void		reportFix(serverFacility * f);

	serverCell	*cells;
}; 
#endif // SERVERCELL_H
