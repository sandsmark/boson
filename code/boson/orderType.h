/***************************************************************************
                          orderType.h  -  description                              
                             -------------------                                         

    version              :                                   
    begin                : Wen Apr  7 23:39:00 CET 1999
                                           
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

#ifndef ORDER_TYPE_H 
#define ORDER_TYPE_H 

enum orderType {

/* mobile */
	ORDER_MOVE	= 0x00001,	/* move to XX */
	ORDER_STOP	= 0x00002,	/* stop moving */
	ORDER_ATTACK	= 0x00004,	/* attack in position XX (vehicle or ground coo) */
	ORDER_REPLY	= 0x00008,	/* Go back to base */

	ORDER_EXPAND	= 0x00010,	/* expand into what is supposed to expand (mobile->fix) */
/* common */
	ORDER_DESTROY	= 0x00011,	/* autodestruction */
/* fix */
	ORDER_BUILD	= 0x10001,	/* build something new... */
	ORDER_PAUSE	= 0x10002,	/* pause while upgrading or building */
	ORDER_UPGRADE	= 0x10004,	/* upgrade possibilities */
	ORDER_REPAIR	= 0x10008,	/* repair the building */
	};

#endif // ORDER_TYPE_H

