/***************************************************************************
                          visualBigDisplay.h  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Sat Jan  9 19:35:36 CET 1999
                                           
    copyright            : (C) 1999-2000 by Thomas Capricelli                         
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

	virtual QSize sizeHint() const { return QSize(100,100); } // minimum size

signals:
	void	relativeReCenterView (QPoint p);
	void	reSizeView (QSize s);

protected:
	/*
	 * put object 
	 */
	virtual void object_put(QPoint)=0;
	virtual void actionClicked(QPoint, int state)=0;	// selecting, moving...


	// display
	void drawRectSelect(QPoint p1, QPoint p2, QPainter &qp)	// XXX QRect somewhere ?
		{ qp.drawRect(p1.x(), p1.y(), p2.x()-p1.x(), p2.y()-p1.y()); }

/* events */
//  virtual void drawContents( QPainter*, int cx, int cy, int cw, int ch );

//  virtual void paintEvent(QPaintEvent *evt);
  virtual void viewportMousePressEvent(QMouseEvent *e);
  virtual void viewportMouseMoveEvent(QMouseEvent *e);
  virtual void viewportMouseReleaseEvent(QMouseEvent *e);
  virtual void resizeEvent(QResizeEvent *e); // do we receive this one ?

  visualTopLevel	*vtl;

	QPoint	oldPos;
	QPoint	selectPos;

};

#endif // VISUALBIGDISPLAY_H


