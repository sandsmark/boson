/***************************************************************************
                          editorBigDisplay.h  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Tue Sep 21 01:18:00 CET 1999
                                           
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

#ifndef EDITORBIGDISPLAY_H 
#define EDITORBIGDISPLAY_H 

#include "visualBigDisplay.h"
#include "editorTopLevel.h"

class QPopupMenu;

/** 
  * Add all editor-specific 'bells and whistles' to the visual/visualBigDisplay 
  */
class editorBigDisplay : public visualBigDisplay 
{

	Q_OBJECT

public:
	editorBigDisplay(editorTopLevel *v, QWidget *parent=0, const char *name=0L, WFlags f=0);

	virtual void actionClicked(QPoint p, int state);	// selecting, moving...

private slots:
	void setSelectedObject	(object_type, int);
	void setWho		(uint w) { who = w; }

private:
	cell_t		c;
	mobType		m;
	facilityType	f;
	object_type	otype;
	uint		who;
};

#endif // EDITORBIGDISPLAY_H


