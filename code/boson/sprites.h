/***************************************************************************
                         sprites.h  -  description                              
                             -------------------                                         

    version              :                                   
    begin                : Sat Apr 24 20:42:00 CET 1999
                                           
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

#ifndef SPRITES_H 
#define SPRITES_H 

#define	S_BASE		1234
#define	S_GROUND	(S_BASE+100)
#define	S_MOBILE	(S_BASE+200)
#define	S_FACILITY	(S_BASE+300)


#define Z_SELECT	(+20)
#define Z_FLYING	(+10)
#define Z_MOBILE	(+5)
#define Z_SAILING	(+2)
#define Z_FACILITY	(+0)
#define Z_GROUND	(-1)

#endif // SPRITES_H
