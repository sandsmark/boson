/***************************************************************************
                          editorBigDisplay.h  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Tue Sep 21 01:18:00 CET 1999
                                           
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

#ifndef EDITORBIGDISPLAY_H 
#define EDITORBIGDISPLAY_H 

#include "visualBigDisplay.h"
#include "editorView.h"

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

	virtual void actionClicked(int, int);		// selecting, moving...

private slots:
	void setSelectedObject	(object_type, int);
	void setWho		(int w) { who = w; }

private:
	groundType	g;
	mobType		m;
	facilityType	f;
	object_type	otype;
	int		who;
};

#endif // EDITORBIGDISPLAY_H


