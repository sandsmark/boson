/***************************************************************************
                          bosonCell.h  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Fri Nov 10 20:48:25 CET 2000
                                           
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

#ifndef BOSONCELL_H 
#define BOSONCELL_H 

#include "common/cell.h"

class fowSprite;

/** 
  * This is a cell from the boson client point of view
  */
class bosonCell : public Cell
{
public:
	bosonCell() { fow = 0l;}

	bool	isFogged(void) { return  fow!=0; }
	void	unFog(void);
	void	fog(int x, int y);
private:
	fowSprite	*fow;
};

#endif	//BOSONCELL_H 
