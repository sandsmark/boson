/***************************************************************************
                          visualCell.h  -  description                              
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

#ifndef VISUALCELL_H 
#define VISUALCELL_H 

#include <QwSpriteField.h>

#include "../common/cell.h"

#include "sprites.h"

/** 
  * This class represents one cell of the main game board
  */

class visualCell : public Cell, public QwSprite
{


public:
	visualCell	(groundType g = GROUND_UNKNOWN);
	visualCell	(groundType g, int i, int j);

	void	set	(groundType g, int i, int j);
	void	set	(groundType g);
	void	setFrame(byte it) { setItem(it); frame(it); }
  
/* Qw stuff */
	virtual int rtti() const { return S_GROUND + ground; }

};

#endif // VISUALCELL_H