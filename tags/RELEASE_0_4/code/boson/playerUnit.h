/***************************************************************************
                          playerUnit.h  -  description                              
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

#ifndef PLAYERUNIT_H 
#define PLAYERUNIT_H 

#include "visualUnit.h"
#include "sprites.h"

enum mobUnitState {
	MUS_NONE,
	MUS_TURNING,
	MUS_MOVING,
	MUS_MOVING_WAIT,
	MUS_,
	};
	
	
class playerMobUnit : public visualMobUnit
{

	Q_OBJECT

 public:
  
  playerMobUnit(mobileMsg_t *, QObject* parent=0, const char *name=0L);
  ~playerMobUnit();

  void	getWantedAction();
 
  void	shooted(int _power);
/* Server orders */
	void  doMoveTo(int newx, int newy);
  void  s_moveTo(int newx, int newy, int direction);

 protected:
  int	getLeft(int a=1) {return (direction+12-a)%12; }
  int	getRight(int a=1) {return (direction+a)%12; }
  void	turnTo(int newdir);
   bool	checkMove(int dx, int dy);
   	bool near(int distance);

	virtual bool	getWantedMove(bosonMsgData *);
	virtual bool	getWantedShoot(bosonMsgData *);
	
signals:
	void dying(Unit *);
	void sig_moveTo(int newx, int newy);

public slots:
	/* orders from user */
	virtual void	u_goto(int, int); // not the same as QwSprite::moveTo
  void  u_stop(void);	
  void  u_attack(Unit *);
  	void targetDying(Unit *);
  	void targetMoveTo(int, int);

 private :
  void	do_goto(int, int);
  int		direction;	// [0-11] is the angle ...
  mobUnitState	state;

/* moving */
	int 	dest_x, dest_y;
	int	asked_x, asked_y;
	int	present_dx, present_dy;
	mobUnitState	asked_state;

/* attack */
	Unit 	*target;
	int	shoot_timer;

};


class harvesterUnit : public playerMobUnit
{
	Q_OBJECT
		
public:
	harvesterUnit(mobileMsg_t *m, QObject* parent=0, const char *name=0L) : playerMobUnit(m,parent,name)
		{ hstate = standBy; base_x = m->x; base_y = m->y; }
	virtual bool	getWantedMove(bosonMsgData *);
	virtual bool	getWantedShoot(bosonMsgData *);
	virtual void	u_goto(int, int);
	
	enum harvestState { standBy, goingTo, comingBack, harvesting };
	
private:
	groundType	underlyingGround(void);

	harvestState	hstate;
	int		base_x, base_y;
	int		harvest_x, harvest_y;
};






class playerFacility : public visualFacility
{

	Q_OBJECT

 public:
  playerFacility(facilityMsg_t *msg, QObject* parent=0L, const char *name=0L);
  ~playerFacility();

	void	getWantedAction();

/* Server orders */
	void	s_setState(int );
	void	shooted(int _power);

public slots:
  	void	targetDying(Unit *);
  	void	targetMoveTo(int, int);
signals:
	void	dying(Unit *);
};

#endif // PLAYERUNIT_H

