/***************************************************************************
                         sprites.h  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Sat Apr 24 20:42:00 CET 1999
                                           
    copyright            : (C) 1999 by Thomas Capricelli                         
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

#ifndef SPRITES_H 
#define SPRITES_H 

#define	S_BASE		1000
#define	S_PART_UP	(S_BASE+10)
#define	S_PART_DOWN	(S_BASE+20)
#define	S_GROUND	(S_BASE+100)
#define	S_MOBILE	(S_BASE+200)
#define	S_FACILITY	(S_BASE+300)
#define	S_SHOT		(S_BASE+400)
#define	S_END		(S_BASE+500)

#define Z_MOBILE	(+100)
#define Z_FACILITY	(+50)
#define Z_GROUND	(+10)
#define Z_INVISIBLE	(-10)


#define IS_MOBILE(rtti)		( ((rtti) >= S_MOBILE  )  && ((rtti) < S_FACILITY) )
#define IS_FACILITY(rtti)	( ((rtti) >= S_FACILITY)  && ((rtti) < S_END     ) )
#define IS_UNIT(rtti)		( ((rtti) >= S_MOBILE  )  && ((rtti) < S_END     ) )

#endif // SPRITES_H
