/***************************************************************************
                          miniMap.h  -  description                              
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

#ifndef MINI_MAP_H 
#define MINI_MAP_H 

#include "../common/groundType.h"

#include <qframe.h> ///orzel qwidget.h

class Cell;
class Unit;
class QPixmap;
class playerCell;
class viewMap;
class playerMobUnit;
class playerFacility;

/** 
  * This is the little map, which "zoom" the battle field
  */
class miniMap : public QWidget
{

  Q_OBJECT

public:
  miniMap(viewMap *v, QWidget *parent=0, const char *name=0L);

signals:
  void	reCenterView(int x, int y);
  void  reSizeView(int l, int h);

public slots:
  void newCell(int,int, groundType);
  void drawMobile(playerMobUnit *mob);
  void drawFix(playerFacility *fix);

protected:
  void setPoint(int x, int y, const QColor &color, QPainter *p=0L);

/* events */
  virtual void paintEvent(QPaintEvent *evt);
  virtual void mousePressEvent(QMouseEvent *e);

private:

  viewMap	*view;
  QPixmap	*ground;

};

#endif // MINI_MAP_H

