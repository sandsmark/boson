/***************************************************************************
                          visualCanvas.h  -  description                              
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

#ifndef VISUALCANVAS_H 
#define VISUALCANVAS_H 


#include <qobject.h>
#include <qintdict.h>
#include <qcanvas.h>

#include "common/groundType.h"

class QRect;
class QPainter;
class speciesTheme;
class visualFacility;
class visualMobUnit;


/** 
  * This class encapsulate the "physical" idea of the map : size, contents..
  */
class visualCanvas : public QCanvas
{

	Q_OBJECT

public:
	visualCanvas( QPixmap,  uint , uint );

/* geometry ? , still public */
	int		maxX, maxY;	// size of the map
///orzel should be maxX * BO_TILE_SIZE = width(), maxY * BO_TILE_SIZE

	void setCell(int i, int j, cell_t c);
	/** find the unit at this position */
	QCanvasItem		*findUnitAt(int x, int y);
	groundType		groundAt(int x, int y) {return (groundType)tile( x/tileWidth(), y/tileHeight() );}
	
	virtual void resize (int, int);
protected:
	visualCanvas();		// to be used by editorField
	void initTheme(void);

signals:
	void newCell(int,int, groundType g);
	void updateMobile(visualMobUnit *); // for miniMap
	void updateFix(visualFacility *); // for miniMap

//	void	mobileDestroyed( int);
//	void	fixDestroyed( int);
};

#endif // VISUALCANVAS_H


