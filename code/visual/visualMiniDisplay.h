/***************************************************************************
                          visualMiniDisplay.h  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Sat Jan  9 19:35:36 CET 1999
                                           
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

#ifndef VISUALMINIDISPLAY_H 
#define VISUALMINIDISPLAY_H 

#include "common/groundType.h"

#include <qframe.h> ///orzel qwidget.h

class Cell;
class Unit;
class QPixmap;
class visualCell;
class visualTopLevel;
class visualMobUnit;
class visualFacility;

/** 
  * This is the little map, which "zoom" the battle field
  */
class visualMiniDisplay : public QWidget
{

  Q_OBJECT

public:
  visualMiniDisplay(visualTopLevel *, QWidget *parent=0, const char *name=0L);

signals:
  void	reCenterView(int x, int y);
  void  reSizeView(int l, int h);

public slots:
  void newCell(int,int, groundType);
  void drawMobile(visualMobUnit *mob);
  void drawFix(visualFacility *fix);

protected:
  void setPoint(int x, int y, const QColor &color, QPainter *p=0L);

/* events */
  virtual void paintEvent(QPaintEvent *evt);
  virtual void mousePressEvent(QMouseEvent *e);

private:

  visualTopLevel	*vtl;
  QPixmap	*ground;

};

#endif // VISUALMINIDISPLAY_H

