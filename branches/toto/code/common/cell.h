/***************************************************************************
                          cell.h  -  description                              
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

#ifndef CELL_H 
#define CELL_H 

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif 

#include "../common/groundType.h"

/** 
  * This class represents one cell of the main game board
  */
class Cell
{

 public:
  Cell(groundType g = GROUND_UNKNOWN);
  ~Cell();

  groundType	getGroundType(void) { return ground; }
/*  virtual*/	void setGroundType(groundType g) { ground =g ; }
  void		setFacility(void) { ground = GROUND_FACILITY; }

 protected:
  groundType	ground;

 private:
  destroyedType	destroyed;
};

#endif // CELL_H
