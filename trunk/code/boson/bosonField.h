/***************************************************************************
                          bosonField.h  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Thu Sep  9 01:27:00 CET 1999
                                           
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

#ifndef BOSONFIELD_H 
#define BOSONFIELD_H 

#include <qintdict.h>

#include "../common/msgData.h"
//#include "../common/unitType.h"
#include "../common/unit.h"	// Facility

#include "playerUnit.h"		// playerMobUnit
//#include "playerCell.h"
#include "visualField.h"

class QRect;
class QPainter;
class Cell;
class Unit;



/** 
  * This class encapsulate the "physical" idea of the map : size, contents..
  */
class bosonField : public visualField 
{
  Q_OBJECT

 public:
  bosonField(uint l, uint h, QObject *parent=0, const char *name=0L);

  void createMob(mobileMsg_t &);
  void destroyMob(destroyedMsg_t &);

  void createFix(facilityMsg_t &);
  void destroyFix(destroyedMsg_t &);

  void move(moveMsg_t &);
  void requestAction(boBuffer *);

/* concerning contents */
  playerFacility *getFacility(long key) { return facility.find(key); }

//private :
  QIntDict<playerMobUnit>	mobile;
  QIntDict<playerFacility>	facility;

};

#endif // BOSONFIELD_H


