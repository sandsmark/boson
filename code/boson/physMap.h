/***************************************************************************
                          physMap.h  -  description                              
                             -------------------                                         

    version              :                                   
    begin                : Sat Jan  9 19:35:36 CET 1999
                                           
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

#ifndef PHYS_MAP_H 
#define PHYS_MAP_H 


#include <qobject.h>
#include <qintdict.h>

#include <QwSpriteField.h>
#include "../common/msgData.h"
#include "../common/groundType.h"
#include "../common/unitType.h"
#include "../common/unit.h"	// Facility
#include "playerUnit.h"		// playerMobUnit
#include "playerCell.h"
//#include "speciesTheme.h"

class QRect;
class QPainter;
class Cell;
class Unit;
class groundTheme;
class speciesTheme;

///// orzel : TEMPORAIRE que c'en est grave : 
/*#define	MAX_PLAYER	2
#define MAX_UNIT	20 */


/** 
  * This class encapsulate the "physical" idea of the map : size, contents..
  */
class physMap : public QObject, public QwSpriteField
{
  Q_OBJECT

 public:
  physMap(uint l, uint h, QObject *parent=0, const char *name=0L);
  ~physMap();

/* geometry ? , still public */
  int		maxX, maxY;	// size of the map
///orzel should be maxX * BO_TILE_SIZE = width(), maxY * BO_TILE_SIZE

/* modify contents */
  void setCell(int i, int j, groundType g);
  void createMob(mobileMsg_t &);
  void createFix(facilityMsg_t &);
  void move(moveMsg_t &);
  void requestAction(boBuffer *);

/* get content */
  playerFacility *getFacility(long key) { return facility.find(key); }

  signals:
  void newCell(int,int, groundType g);
  void updateMobile(playerMobUnit *); ///orzel : outdated by Qw ?
  void updateFix(playerFacility *); ///orzel : outdated by Qw ?


  protected:
/* Qw stuff */
/*  int	coo2index(int x, int y)
	{ return maxY*x+y;}
  void	index2coo(uint index, int &x, int &y)
	{ x = index / maxY; y = index % maxY; } */

//  private:
public: ///orzel : temp

  QIntDict<playerMobUnit>	mobile;
  QIntDict<playerFacility>	facility;

};

#endif // PHYS_MAP_H


