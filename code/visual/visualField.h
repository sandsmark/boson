/***************************************************************************
                          visualField.h  -  description                              
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

#ifndef VISUALFIELD_H 
#define VISUALFIELD_H 


#include <qobject.h>
#include <qintdict.h>

#include <QwSpriteField.h>

#include "common/groundType.h"

#include "visualUnit.h"		// visualMobUnit
#include "visualCell.h"

class QRect;
class QPainter;
class Cell;
//class Unit;
class groundTheme;
class speciesTheme;
class visualFacility;
class visualMobUnit;


/** 
  * This class encapsulate the "physical" idea of the map : size, contents..
  */
class visualField : public QObject, public QwSpriteField
{

	Q_OBJECT

public:
	visualField(uint l, uint h, QObject *parent=0, const char *name=0L);

/* geometry ? , still public */
	int		maxX, maxY;	// size of the map
///orzel should be maxX * BO_TILE_SIZE = width(), maxY * BO_TILE_SIZE

	void setCell(int i, int j, groundType g);
	/** find the unit at this position */
	QwSpriteFieldGraphic	*findUnitAt(int x, int y);
	groundType		findGroundAt(int x, int y);
	
	
	virtual void resize (int, int);
protected:
	visualField(QObject *parent=0, const char *name=0L); // to be used by editorField
	void init(void);

signals:
	void newCell(int,int, groundType g);
	void updateMobile(visualMobUnit *); // for miniMap
	void updateFix(visualFacility *); // for miniMap
};

#endif // VISUALFIELD_H


