/***************************************************************************
                       editor/mainView.h -  description 
                             -------------------                                         

    version              : $Id$
    begin                : Mon Apr 19 23:56:00 CET 1999
                                           
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

#ifndef MAINVIEW_H
#define MAINVIEW_H

#include <qwidget.h>

class	visualMiniDisplay;
class	editorBigDisplay;
class	visualView;
class	editorCanvas;

class mainView : public QWidget 
{
	Q_OBJECT
public:
	mainView(QWidget *parent=0, const char *name=0);

public slots:
	void slotEditDestroy(void);
protected:
  virtual void keyReleaseEvent (QKeyEvent * e );

private:
/* the map object we are playing in */
    visualMiniDisplay	*mini;
    editorBigDisplay	*big;
    visualView		*view;
};


#endif     // MAINVIEW_H
