/***************************************************************************
                         boshot.h  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Thu Dec 16 14:35:00 CET 1999
                                           
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

#ifndef BOSHOT_H 
#define BOSHOT_H 

#include <qobject.h>		// timer
#include <qbitarray.h>
#include <qcanvas.h>		// graphism
#include "sprites.h"		// rtti S_SHOT

#define SHOT_FRAMES		18

#define UNITS_SHOT_FRAMES	16
#define FIX_SHOT_FRAMES		16

#define UNITS_SHOTS_NB		4
#define FIX_SHOTS_NB		4



class boShot : public QObject,  public QCanvasSprite
{
	Q_OBJECT
public:
	enum shot_style { SHOT_SHOT, SHOT_UNIT, SHOT_FACILITY };

	boShot(int _x, int _y, int _z, shot_style s);
	/* QCanvas stuff */
	virtual int	rtti() const { return S_SHOT; }

protected:
	void	timerEvent( QTimerEvent * );

private:
	static	QBitArray		qba_units;			// which unit explosions are already loaded
	static	QCanvasPixmapArray	*unitSequ[UNITS_SHOTS_NB];	// explosions sequences for units
	static	QBitArray		qba_fix;			// which facilities explosions are already loaded
	static	QCanvasPixmapArray	*fixSequ[FIX_SHOTS_NB];		// explosions sequences for facilities
	static	QCanvasPixmapArray	*shotSequ;			// explosions sequence for small shots

	static	bool loadBig(shot_style style, int version);		// load one unit/facilitye pixmaps set

	int	counter;
	int	maxCounter;
	
};

#endif // BOSHOT_H 

