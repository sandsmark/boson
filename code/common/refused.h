/***************************************************************************
                          refused.h  -  description                    
                             -------------------                                         

    version              : $Id$
    begin                : Sat Apr 17 23:02:00 CET 1999
                                           
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

#ifndef REFUSED_H
#define REFUSED_H

/*
 * this enum explain why the connection to the game has been
 * refused by the server 
 */

/// should be move to connect.h, no ?

enum refusedType {
	REFUSED_PLAYING,	/* A game is currently running, not used yet */
	REFUSED_BAD_VERSION,	/* Your client version isn't compatible with the server's one, not used yet */
	REFUSED_PRIVATE,	/* this is a private game, you haven't been invited, not used yet */
	REFUSED_TOO_LOW,	/* your connection is too low, not used yet */
	REFUSED_,
	};

#endif /* REFUSED_H */
