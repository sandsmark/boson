/***************************************************************************
                          visualBigDisplay.h  -  description                              
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

#ifndef VISUALBIGDISPLAY_H 
#define VISUALBIGDISPLAY_H 

#include <qintdict.h>
#include <qpainter.h>
#include <qcanvas.h>

#include "common/groundType.h"
#include "common/unitType.h"
#include "common/unit.h"	// Facility

#include "speciesTheme.h"
//#include "visualTopLevel.h"

class Unit;
class QPixmap;
class visualTopLevel;
class orderWin;


/** 
  * This class handles all operations concerning the game Board/Map
  */
class visualBigDisplay : public QCanvasView
{
  Q_OBJECT

public:
  visualBigDisplay(/*orderWin *,*/ visualTopLevel *, QWidget *parent=0, const char *name=0L, WFlags f=0);
  ~visualBigDisplay();


	/* from display classes */
	virtual void actionClicked(int, int, int state)=0;	// selecting, moving...

signals:
	void	relativeReCenterView (int x, int y);
	void	reSizeView (int l, int h);

protected:
	// display
	void drawRectSelect(int x1, int y1, int x2, int y2, QPainter &qp)
		{ qp.drawRect(x1, y1, x2-x1, y2-y1); }

/* events */
//  virtual void drawContents( QPainter*, int cx, int cy, int cw, int ch );

//  virtual void paintEvent(QPaintEvent *evt);
  virtual void viewportMousePressEvent(QMouseEvent *e);
  virtual void viewportMouseMoveEvent(QMouseEvent *e);
  virtual void viewportMouseReleaseEvent(QMouseEvent *e);
  virtual void resizeEvent(QResizeEvent *e); // do we receive this one ?

  visualTopLevel	*vtl;

  int selectX, selectY;
  int oldX, oldY;

};

#endif // VISUALBIGDISPLAY_H


