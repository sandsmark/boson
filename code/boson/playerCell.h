/***************************************************************************
                          playerCell.h  -  description                              
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

#ifndef PLAYER_CELL_H 
#define PLAYER_CELL_H 

#include <QwSpriteField.h>

#include "../common/cell.h"

#include "sprites.h"

/** 
  * This class represents one cell of the main game board
  */

class playerCell : public Cell, public QwSprite
{


 public:
  playerCell(groundType g, int i, int j);

//  void setGroundType(groundType g, int i, int j);
/* Qw stuff */
  virtual int rtti() { return S_GROUND; }

 // void setGroundType(groundType g) { ground = g; }
  protected:

  private:
};

#endif // PLAYER_CELL_H
