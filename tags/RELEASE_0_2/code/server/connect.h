/***************************************************************************
                          connect.h  -  description                    
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

#ifndef SERVERCONNECT_H 
#define SERVERCONNECT_H 

//#ifdef HAVE_CONFIG_H
//#include <config.h>
//#endif 

enum serverSocketState {
	SSS_NO_CONNECT = -1,
	SSS_INIT = 1,
	SSS_CONNECT_OK,
	SSS_CONNECT_DOWN,
	SSS_SYNC_ME,
	SSS_SYNC_OTHER,
	SSS_SYNC_
	};


enum serverState {
	SS_INIT,
	SS_WAITING,
	SS_PLAYING,
	SS_
	};

#endif // SERVERCONNECT_H
