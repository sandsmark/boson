/***************************************************************************
                          playerUnit.h  -  description                              
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

#ifndef PLAYERUNIT_H 
#define PLAYERUNIT_H 

#include "visualUnit.h"
#include "sprites.h"


enum mobUnitState {
	MUS_NONE,
	MUS_TURNING,
	MUS_MOVING,
	MUS_,
	};

class playerMobUnit : public visualMobUnit
{
 Q_OBJECT

 public:
  
  playerMobUnit(mobileMsg_t *, QObject* parent=0, const char *name=0L);

  int	getWantedMove(int &dx, int &dy, int &direction);
  int	getWantedAction();

/* Server orders */
  void  doMoveBy(int dx, int dy);
  void  s_moveBy(int dx, int dy, int direction);

 protected:
  int	getLeft(int a=1) {return (direction+12-a)%12; }
  int	getRight(int a=1) {return (direction+a)%12; }
  void	turnTo(int newdir);
  bool	checkMove(int dx, int dy);

 public slots:
/* orders from user */
  void	u_goto(int, int); // not the same as QwSprite::moveTo
  void  u_stop(void);	

 private :
  int		direction;	// [0-11] is the angle ...
  mobUnitState	state;

  /* moving */
  int 	dest_x, dest_y;
  int	asked_dx, asked_dy;

  /* toute la gestion du but, chemin pris, etc... */
};




class playerFacility : public visualFacility
{

 public:
  playerFacility(facilityMsg_t *msg, QObject* parent=0L, const char *name=0L);

/* Server orders */
  void		s_setState(int );

 private:
};

#endif // PLAYERUNIT_H

