/***************************************************************************
                         connect.h  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Sat Jan  9 19:35:36 CET 1999
                                           
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

#ifndef CLIENT_CONNECT_H 
#define CLIENT_CONNECT_H 


enum playerSocketState {
	PSS_INIT ,
	PSS_WAIT_CONFIRM_INIT ,
	PSS_CONNECT_OK ,
	PSS_CONNECT_DOWN ,
	PSS_SYNC_ME ,
	PSS_SYNC_OTHER ,
	PSS_ 
	};

enum playerState {
	PS_INIT ,
	PS_WAIT_ANSWER ,
	PS_NO_CONNECT ,
	PS_WAIT_BEGIN ,
	PS_PLAYING ,
	PS_ 
	};


#endif // CLIENT_CONNECT_H 

