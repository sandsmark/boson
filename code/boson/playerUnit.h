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


class selectPart_up;
class selectPart_down;

enum mobUnitState {
	MUS_NONE,
	MUS_TURNING,
	MUS_MOVING,
	MUS_,
	};

class playerMobUnit : public mobUnit, public QwSprite
{
 Q_OBJECT

 public:
  
  playerMobUnit(mobileMsg_t *, QObject* parent=0, const char *name=0L);
  ~playerMobUnit();

  virtual	int _x(void) {return x();}
  virtual	int _y(void) {return y();}

  int	getWantedMove(int &dx, int &dy, int &direction);
  int	getWantedAction();

/* attachement */
  void  select();
  void  unSelect();

/* Server orders */
  void  doMoveBy(int dx, int dy);
  void  s_moveBy(int dx, int dy, int direction);

/* Qw stuff */
  virtual int rtti() { return S_MOBILE+type; }

 protected:
  int	getLeft(int a=1) {return (direction+12-a)%12; }
  int	getRight(int a=1) {return (direction+a)%12; }
  void	turnTo(int newdir);

 public slots:
/* orders from user */
  void	u_goto(int, int); // not the same as QwSprite::moveTo
  void  u_stop(void);	

 private :
  int		direction;	// [0-11] is the angle ...
  mobUnitState	state;

/* attachement */
  selectPart_up *sp_up;
  selectPart_down *sp_down;

  /* moving */
  int 	dest_x, dest_y;
  int	asked_dx, asked_dy;

  /* toute la gestion du but, chemin pris, etc... */
};




class playerFacility : public Facility, public QwSprite
{

 public:
  playerFacility(facilityMsg_t *msg, QObject* parent=0L, const char *name=0L);
  ~playerFacility();
//  virtual QRect	rect(void);
  virtual int	_x(void) {return x();}
  virtual int	_y(void) {return y();}

/* attachement */
  void  select();
  void  unSelect();

/* Server orders */
  void		s_setState(int );

/* Qw stuff */
  virtual int	rtti() { return S_FACILITY+type; }

 private:
/* attachement */
  selectPart_up *sp_up;
  selectPart_down *sp_down;

};

#endif // PLAYER_UNIT_H

