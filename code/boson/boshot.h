/***************************************************************************
                         boshot.h  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Thu Dec 16 14:35:00 CET 1999
                                           
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

#ifndef BOSHOT_H 
#define BOSHOT_H 

#include <qobject.h>		// timer
#include <QwSpriteField.h>	// grahism
#include "sprites.h"		// rtti S_SHOT

#define SHOT_FRAMES	18
#define BIG_SHOT_FRAMES	16

class boShot : public QObject,  public QwSprite
{
	Q_OBJECT
public:
	boShot(int _x, int _y, int _z, bool isBig=false);
/* Qw stuff */
	virtual int	rtti() const { return S_SHOT; }
protected:
	void  timerEvent( QTimerEvent * );
private:
	static	QwSpritePixmapSequence  *shotSequ;
	static	QwSpritePixmapSequence  *bigShotSequ;
	int	counter;
	int	maxCounter;
	
};

#endif // BOSHOT_H 

