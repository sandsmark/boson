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

#include "visual.h"
#include "connect.h"

class bosonField;
class KSocket;
class boBuffer;

#ifndef uint
typedef unsigned int uint;
#endif

	extern speciesTheme		*myspecy;
	extern int			who_am_i;
/* The map which handle grouds and units*/
	extern bosonField		*field;
/* synchronization */
	extern uint			jiffies;
/* deal with the communication layer */
	extern KSocket			*Socket;
	extern playerSocketState	socketState;
	extern playerState		State;
	extern boBuffer			*buffer;

#endif // BOSON_GAME_H 
