/***************************************************************************
                          unit.h  -  description                              
                             -------------------                                         

    version              : $Id$
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

#ifndef UNIT_H 
#define UNIT_H 

#include <qobject.h>

#include "../common/unitType.h"
#include "../common/msgData.h"
#include "../common/map.h"


enum workType {
	WORK_NONE =0, 
	WORK_TRAINING, 

	WORK_LAST_FACILITY, 
	WORK_WOOD, 
	WORK_MINE, 
	WORK_ 
	};

/** 
  * This class is the base for all mobile units
  */
class Unit : public QObject
{

 Q_OBJECT

 public:
  Unit(QObject* parent=0, const char *name=0L);

  virtual	int	_x(void)=0;
  virtual	int	_y(void)=0;

  virtual	int	getWidth(void)=0;
  virtual	int	getHeight(void)=0;
  virtual	uint	getVisibility(void)=0;

  int		who;		// who is the owner ?
  int		key;
  protected:
  uint		power;		// Power : 0 => death
  uint		contain;	// how much contain (petrol, money);
  uint		countDown;	// work countDown;
  workType	work;		// work being done


};


class mobUnit : public Unit
{
 Q_OBJECT

 public:
  mobUnit(mobileMsg_t *msg, QObject* parent=0L, const char *name=0L);

  void		getPos(uint &xx, uint &yy) {xx = _x(); yy = _y(); }
  mobType	getType(void) {return type;}

  		int	goFlag(void) { return mobileProp[type].goFlag;}

  virtual	int	getWidth(void) { return mobileProp[type].width;}
  virtual	int	getHeight(void) { return mobileProp[type].height;}
  virtual	uint	getVisibility(void) {return mobileProp[type].visibility; }
  		QRect	rect(void);

 protected:
  mobType		type;
  uint	autonomy; 	// carburant ?
  bool	isShown;	// if not -> is carried by another one

};

class Facility : public Unit
{
 Q_OBJECT

 public:
  		Facility(facilityMsg_t *msg, QObject* parent=0L, const char *name=0L);

  facilityType	getType(void) {return type;}

  virtual	int	getWidth(void) { return facilityProp[type].width * BO_TILE_SIZE;}
  virtual	int	getHeight(void) { return facilityProp[type].height * BO_TILE_SIZE;}
  virtual	uint	getVisibility(void) {return facilityProp[type].visibility; }
  		QRect	rect(void);

 protected:
  facilityType	type;

};

#endif // UNIT_H


