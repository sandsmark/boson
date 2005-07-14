/***************************************************************************
                         game.h  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Sat Jun 19 01:53:00 CET 1999
                                           
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

#ifndef SERVER_GAME_H 
#define SERVER_GAME_H 

#include "player.h"
#include "../common/boconfig.h"

class QString;

class gameProperties_t {
public:
  QString	*worldName;
  Player	player[BOSON_MAX_CONNECTION];
  uint		jiffies;
  uint		nbPlayer;
  uint		nbConnected;
};


extern gameProperties_t gpp;


#endif // SERVER_GAME_H 