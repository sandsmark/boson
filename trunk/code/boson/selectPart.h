/***************************************************************************
                         selectPart.h  -  description                              
                             -------------------                                         

    version              :                                   
    begin                : Sat Jun 26 16:23:00 CET 1999
                                           
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

#ifndef SELECTPART_H 
#define SELECTPART_H 


#include <QwSpriteField.h>


#define PART_NB		10

class selectPart : public QwSprite
{
public:
		selectPart();
  static void	drawSelectBox(QPainter &painter, QColor, QColor);
};



class selectPart_up : public selectPart
{
public:
		selectPart_up(int a);
  static void	initStatic();

private:
  static QwSpritePixmapSequence  *qsps;
	
};



class selectPart_down : public selectPart
{
public:
		selectPart_down(int);
  static void	initStatic();

private:
  static QwSpritePixmapSequence  *qsps;
	
};

#endif // SELECTPART_H 
