/***************************************************************************
                         selectPart.h  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Sat Jun 26 16:23:00 CET 1999
                                           
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

#ifndef SELECTPART_H 
#define SELECTPART_H 


#include <QwSpriteField.h>
#include "sprites.h"
#include "common/unit.h"


#define PART_NB		(MAX_POWER+1)


///orzel : could be made with only one class with yet another arg  : PART_UP / PART_DOWN 


class selectPart_up : public QwSprite
{
public:
		selectPart_up(int frame, int z);
  static void	initStatic();

/* Qw stuff */
  virtual int	rtti() const { return S_PART_UP; }

private:
  static QwSpritePixmapSequence  *qsps;
	
};



class selectPart_down : public QwSprite
{
public:
		selectPart_down(int frame, int z);
  static void	initStatic();

/* Qw stuff */
  virtual int	rtti() const { return S_PART_DOWN; }

private:
  static QwSpritePixmapSequence  *qsps;
	
};

#endif // SELECTPART_H 
