/***************************************************************************
                          miniMap.h  -  description                              
                             -------------------                                         

    version              :                                   
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


#include <qframe.h> ///orzel qwidget.h
//#include <qintdict.h>

//#include "../common/msgData.h"
//#include "../common/groundType.h"
//#include "../common/unitType.h"
//#include "../common/unit.h"	// Facility
#include "playerUnit.h"		// playerMobUnit

///// orzel : TEMPORAIRE que c'en est grave : 
//#define	MAX_PLAYER	2
//#define MAX_UNIT	20

class Cell;
class Unit;
class QPixmap;
class playerCell;
class viewMap;

/** 
  * This is the little map, which "zoom" the battle field
  */
class miniMap : public QWidget
{
  Q_OBJECT

 public:
  miniMap(viewMap *v, QWidget *parent=0, const char *name=0L);
  ~miniMap();

 signals:
  void	reCenterView(int x, int y);
  void  reSizeView(int l, int h);

 public slots:
  void newCell(int,int, groundType);
/*  void drawCell(int i, int j) {drawCell(i,j,0L); }
  void drawMobile(playerMobUnit *mob) {drawMobile(mob,0L); }
  void drawFix(Facility *fix) {drawFix(fix,0L); } */
//  void drawCell(int i, int j);
  void drawMobile(playerMobUnit *mob);
  void drawFix(playerFacility *fix);

 protected:
 /* void drawCell(int i, int j, QPainter *p=0L);
  void drawMobile(playerMobUnit *, QPainter *p=0L);
  void drawFix(Facility *, QPainter *p=0L); */
//  void drawRelative(int x, int y, QPixmap *p, QPainter *p=0L);
  void setPoint(int x, int y, const QColor &color, QPainter *p=0L);
//  void drawRectSelect(int, int, int, int, QPainter &);

/* events */
  virtual void paintEvent(QPaintEvent *evt);
  virtual void mousePressEvent(QMouseEvent *e);
//  virtual void mouseMoveEvent(QMouseEvent *e);
//  virtual void mouseReleaseEvent(QMouseEvent *e);
//  virtual void resizeEvent(QResizeEvent *e);

 private:

  viewMap	*view;
  QPixmap	*ground;
//  QPainter	*myPainter;
};

#endif // MINI_MAP_H


