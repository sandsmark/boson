/***************************************************************************
                          bosonBigDisplay.h  -  description                              
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

#ifndef BOSONBIGDISPLAY_H 
#define BOSONBIGDISPLAY_H 

#include "visualBigDisplay.h"

class bosonTopLevel;

class bosonBigDisplay : public visualBigDisplay
{
  Q_OBJECT

public:
	bosonBigDisplay(bosonTopLevel *v, QWidget *parent=0, const char *name=0L, WFlags f=0);

protected:
	/* from display classes */
	virtual void actionClicked(QPoint, int state);	// selecting, moving...

	/*
	 * put object 
	 */
	virtual void	object_put(QPoint p);

private:
	bosonTopLevel *btl;


};

#endif // BOSONBIGDISPLAY_H


