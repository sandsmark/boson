/***************************************************************************
                          visualBigDisplay.h  -  description                              
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

#ifndef VISUALBIGDISPLAY_H 
#define VISUALBIGDISPLAY_H 


#include <qframe.h> ///orzel qwidget
#include <qintdict.h>
#include <qpainter.h>

#include <QwSpriteField.h>

//#include "../common/msgData.h"
#include "../common/groundType.h"
#include "../common/unitType.h"
#include "../common/unit.h"	// Facility

#include "speciesTheme.h"
#include "visualView.h"
//#include "orderWin.h"
//#include "game.h"

class Cell;
class Unit;
class groundTheme;
class QPixmap;
class visualCell;
class visualView;
class orderWin;


/** 
  * This class handles all operations concerning the game Board/Map
  */
class visualBigDisplay : public QWidget, public QwAbsSpriteFieldView
{
  Q_OBJECT

public:
  visualBigDisplay(/*orderWin *,*/ visualView *v, QWidget *parent=0, const char *name=0L, WFlags f=0);
  ~visualBigDisplay();


	/* from display classes */
	virtual void actionClicked(int, int)=0;		// selecting, moving...

signals:
	void	relativeReCenterView (int x, int y);
	void	reSizeView (int l, int h);

protected:

/* Qw virtual functions */
  virtual QRect viewArea() const;
  virtual bool preferDoubleBuffering() const {return true;}
  virtual void beginPainter(QPainter &);
  virtual void flush(const QRect& area);
  //virtual void updateGeometries();

/* display */
  void drawCell(int i, int j, QPainter *p=0L);
  void drawRelative(int x, int y, QPixmap *p, QPainter *p=0L);
  void drawRectSelect(int x1, int y1, int x2, int y2, QPainter &qp) {qp.drawRect(x1, y1, x2-x1, y2-y1);}

/* events */
  virtual void paintEvent(QPaintEvent *evt);
  virtual void mousePressEvent(QMouseEvent *e);
  virtual void mouseMoveEvent(QMouseEvent *e);
  virtual void mouseReleaseEvent(QMouseEvent *e);
  virtual void resizeEvent(QResizeEvent *e);

  visualView	*view;

  int selectX, selectY;
  int oldX, oldY;

};

#endif // VISUALBIGDISPLAY_H


