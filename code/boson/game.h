/***************************************************************************
                         game.h  -  description                              
                             -------------------                                         

    version              :                                   
    begin                : Wen Apr 14 20:22:00 CET 1999
                                           
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

#ifndef GAME_H 
#define GAME_H 

class groundTheme;
class speciesTheme;

class gameProperties_t {

public:
  groundTheme	*ground;
  speciesTheme	*species[2];
  speciesTheme	*myspecies;
  int		who_am_i;
	
};


extern gameProperties_t gameProperties;


#endif // GAME_H 
