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

#ifndef VISUAL_CELL_H 
#define VISUAL_CELL_H 

#include <QwSpriteField.h>

#include "../common/cell.h"

#include "sprites.h"

/** 
  * This class represents one cell of the main game board
  */

class visualCell : public Cell, public QwSprite
{


public:
  visualCell(groundType g, int i, int j);

/* Qw stuff */
  virtual int rtti() const { return S_GROUND + ground; }

};

#endif // VISUAL_CELL_H
