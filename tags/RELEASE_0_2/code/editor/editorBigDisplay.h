/***************************************************************************
                          editorBigDisplay.h  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Tue Sep 21 01:18:00 CET 1999
                                           
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

#ifndef EDITORBIGDISPLAY_H 
#define EDITORBIGDISPLAY_H 

#include "visualBigDisplay.h"

class QPopupMenu;
class visualCell;

/** 
  * Add all editor-specific 'bells and whistles' to the visual/visualBigDisplay 
  */
class editorBigDisplay : public visualBigDisplay 
{

	Q_OBJECT

public:
	editorBigDisplay(visualView *v, QWidget *parent=0, const char *name=0L, WFlags f=0);

protected:
	virtual void mousePressEvent(QMouseEvent *e);

private slots:
	void setCell(int);
	void setTransTile(int);
	void setTransRef(int);
	void setItem(int);

private:
	QPopupMenu	*popup;
	visualCell	*selectedCell;

};

#endif // EDITORBIGDISPLAY_H


