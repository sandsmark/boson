/***************************************************************************
                          serverCell.h  -  description                              
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

#ifndef SERVER_CELL_H 
#define SERVER_CELL_H 

#include "../common/cell.h"
#include "../server/knownBy.h"

/** 
  * This is a cell in the server point of view
  */
class serverCell : public knownBy, public Cell
{

 public:
  serverCell(groundType g = GROUND_UNKNOWN);

};

#endif // SERVER_CELL_H
