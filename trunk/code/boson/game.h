/***************************************************************************
                         game.h  -  description                              
                             -------------------                                         

    version              : $Id$
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

#ifndef BOSON_GAME_H 
#define BOSON_GAME_H 

#include "../visual/visual.h"


//class bosonProperties_t : public visualProperties_t {
class bosonProperties_t {

public:
	speciesTheme	*myspecies;
	int		who_am_i;
};


extern bosonProperties_t gpp;


#endif // BOSON_GAME_H 
