/***************************************************************************
                          playerUnit.h  -  description                              
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

#ifndef PLAYER_UNIT_H 
#define PLAYER_UNIT_H 

#include <QwSpriteField.h>

#include "../common/unit.h"
#include "sprites.h"


enum mobUnitState {
	MUS_NONE,
	MUS_MOVING,
	MUS_,
	};

class playerMobUnit : public mobUnit, QwSprite
{
 Q_OBJECT

 public:
  
  playerMobUnit(mobileMsg_t *, QObject* parent=0, const char *name=0L);

  virtual	int _x(void) {return x();}
  virtual	int _y(void) {return y();}

  void	turnLeft(void);
  void	turnRight(void);

  int	getWantedMove(int &dx, int &dy);
  int	getWantedAction();

  virtual QRect	rect(void);
  
/* Server orders */
  void  s_moveBy(int dx, int dy);

/* Qw stuff */
  virtual int rtti() { return S_MOBILE; }

 public slots:
/* orders from user */
  void	u_goto(int, int); // not the same as QwSprite::moveTo
  void  u_stop(void);	

 private :
  uint		direction;	// [0-11] is the angle ...
  mobUnitState	state;

  /* moving */
  int 	dest_x, dest_y;
  int	asked_dx, asked_dy;

  /* toute la gestion du but, chemin pris, etc... */
};

class playerFacility : public Facility, public QwSprite
{

 public:
  playerFacility(facilityMsg_t *msg, QObject* parent=0L, const char *name=0L);
  virtual QRect	rect(void);
  virtual	int _x(void) {return x();}
  virtual	int _y(void) {return y();}

/* Server orders */
  void  s_setState(int );

/* Qw stuff */
  virtual int rtti() { return S_FACILITY; }
};

#endif // PLAYER_UNIT_H

